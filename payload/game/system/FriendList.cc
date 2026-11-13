#include "game/system/FriendList.hh"

#include "game/net/FriendInfo.hh"
#include "game/net/FriendLimits.hh"
#include "game/net/FriendManager.hh"
#include "game/net/NetManager.hh"
#include "game/system/SaveManager.hh"

#include <sp/net/ExtendedFriends.hh>

#include <cstring>

namespace System {

namespace {

// Parallel FriendListEntry mirrors for slots 30..63 (page / GetFriendEntry).
FriendListEntry s_extraEntries[EXTRA_FRIEND_COUNT];

void ensureExtraVtable(FriendListEntry &entry, const FriendList *list) {
    u32 *vtable = reinterpret_cast<u32 *>(&entry.friendInfo[0]);
    if (*vtable != 0) {
        return;
    }
    if (!list) {
        return;
    }
    const FriendListEntry *donor = list->REPLACED(GetFriendEntry)(0);
    if (!donor) {
        return;
    }
    // Friend vtable (+ optional pad) — enough for SetFriend / accessors.
    memcpy(entry.friendInfo, donor->friendInfo, 8);
}

void syncExtraEntry(u32 extraIndex, const FriendList *list) {
    if (extraIndex >= EXTRA_FRIEND_COUNT) {
        return;
    }
    FriendListEntry &entry = s_extraEntries[extraIndex];
    ensureExtraVtable(entry, list);

    const auto &slot = SP::ExtendedFriends::s_slots[extraIndex];
    entry.friendCode = slot.friendCode();
    // FriendData lives at Friend+0x8 == friendInfo+0x8.
    memcpy(entry.friendInfo + 8, slot.main, SP::ExtendedFriends::MAIN_SIZE);
    entry.friendStatus = slot.occupied() ? 1u : 0u;
}

} // namespace

int FriendList::GetFriendCount() const {
    const int vanilla = REPLACED(GetFriendCount)();
    const int extra = static_cast<int>(SP::ExtendedFriends::OccupiedCount());

    // Vanilla page fill still uses sortedFriendsIdx[30] — hide extras there.
    if (SP::ExtendedFriends::UiSafeMode()) {
        return vanilla;
    }

    // Page counter / extended UI wants the true total.
    if (SP::ExtendedFriends::ReportRealRosterCount()) {
        return vanilla + extra;
    }

    // Stock "roster full" gates compare against 29/30. Lie while extended slots remain.
    if (vanilla >= VANILLA_FRIEND_COUNT) {
        if (extra >= static_cast<int>(EXTRA_FRIEND_COUNT)) {
            return MAX_FRIEND_COUNT;
        }
        return VANILLA_FRIEND_COUNT - 1;
    }

    return vanilla + extra;
}

u32 FriendList::AddFriend(const u64 &friendCode) {
    u32 existing = GetFriendIdx(friendCode);
    if (existing != 0xffffffffu) {
        return existing;
    }

    auto *mgr = Net::FriendRosterManager::Instance();
    if (mgr) {
        s32 vanillaFree = mgr->REPLACED(GetFirstFreeFriendIdx)();
        if (vanillaFree >= 0) {
            return REPLACED(AddFriend)(friendCode);
        }
    } else {
        return REPLACED(AddFriend)(friendCode);
    }

    u32 extra = SP::ExtendedFriends::FirstFreeSlot();
    if (extra >= SP::ExtendedFriends::EXTRA_COUNT) {
        return 0xffffffffu;
    }

    if (!SP::ExtendedFriends::AddFriendCode(extra, friendCode)) {
        return 0xffffffffu;
    }

    syncExtraEntry(extra, this);

    if (auto *save = SaveManager::Instance()) {
        s16 license = save->currentLicenseIndex();
        if (license >= 0 && license < 4) {
            SP::ExtendedFriends::Save(static_cast<u32>(license));
        }
    }

    if (mgr) {
        SP::ExtendedFriends::RebuildDwcList(mgr);
    }

    return VANILLA_FRIEND_COUNT + extra;
}

void FriendList::RemoveFriend(u32 idx) {
    if (idx < VANILLA_FRIEND_COUNT) {
        REPLACED(RemoveFriend)(idx);
        if (auto *mgr = Net::FriendRosterManager::Instance()) {
            SP::ExtendedFriends::RebuildDwcList(mgr);
        }
        return;
    }

    u32 extra = idx - VANILLA_FRIEND_COUNT;
    if (extra >= SP::ExtendedFriends::EXTRA_COUNT) {
        return;
    }

    SP::ExtendedFriends::ClearSlot(extra);
    memset(&s_extraEntries[extra], 0, sizeof(s_extraEntries[extra]));

    if (auto *save = SaveManager::Instance()) {
        s16 license = save->currentLicenseIndex();
        if (license >= 0 && license < 4) {
            SP::ExtendedFriends::Save(static_cast<u32>(license));
        }
    }

    if (auto *mgr = Net::FriendRosterManager::Instance()) {
        SP::ExtendedFriends::RebuildDwcList(mgr);
    }
}

u32 FriendList::GetFriendIdx(const u64 &friendCode) {
    u32 vanilla = REPLACED(GetFriendIdx)(friendCode);
    if (vanilla != 0xffffffffu) {
        return vanilla;
    }

    s32 extra = SP::ExtendedFriends::IndexOfFriendCode(friendCode);
    if (extra < 0) {
        return 0xffffffffu;
    }
    return VANILLA_FRIEND_COUNT + static_cast<u32>(extra);
}

const FriendListEntry *FriendList::GetFriendEntry(u32 idx) const {
    if (idx < VANILLA_FRIEND_COUNT) {
        return REPLACED(GetFriendEntry)(idx);
    }

    u32 extra = idx - VANILLA_FRIEND_COUNT;
    if (extra >= EXTRA_FRIEND_COUNT) {
        return nullptr;
    }
    syncExtraEntry(extra, this);
    return &s_extraEntries[extra];
}

u32 FriendList::GetFriendStatus(u32 idx) const {
    if (idx < VANILLA_FRIEND_COUNT) {
        return REPLACED(GetFriendStatus)(idx);
    }

    u32 extra = idx - VANILLA_FRIEND_COUNT;
    if (extra >= EXTRA_FRIEND_COUNT) {
        return 0xffffffffu;
    }
    if (!SP::ExtendedFriends::s_slots[extra].occupied()) {
        return 0;
    }
    syncExtraEntry(extra, this);
    return s_extraEntries[extra].friendStatus;
}

void FriendList::UpdateStatuses() {
    REPLACED(UpdateStatuses)();

    auto *net = Net::NetManager::Instance();
    if (!net) {
        return;
    }

    // Mirror NetManager joinable status into extra FriendList entries for the roster UI.
    for (u32 e = 0; e < EXTRA_FRIEND_COUNT; e++) {
        if (!SP::ExtendedFriends::s_slots[e].occupied()) {
            s_extraEntries[e].friendStatus = 0;
            continue;
        }
        syncExtraEntry(e, this);
        u32 rosterIdx = VANILLA_FRIEND_COUNT + e;
        auto joinable = net->getFriendJoinableStatus(rosterIdx);
        // Stock UpdateStatuses writes values FriendButton / sorting understand; joinable
        // status is what the friend-info / join path uses for the same index.
        s_extraEntries[e].friendStatus = static_cast<u32>(joinable);
    }
}

} // namespace System
