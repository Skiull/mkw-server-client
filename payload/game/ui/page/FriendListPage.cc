#include "game/ui/page/FriendListPage.hh"

#include "game/system/Mii.hh"
#include "game/ui/SectionManager.hh"

#include <sp/net/ExtendedFriends.hh>

#include <cstring>

extern "C" void qsort(void *base, size_t n, size_t width, int (*cmp)(const void *, const void *));

namespace UI {

namespace {

int compareSorted(const void *a, const void *b) {
    return FriendListPage::CompareFriendIndexes(const_cast<u32 *>(static_cast<const u32 *>(a)),
            const_cast<u32 *>(static_cast<const u32 *>(b)));
}

bool vanillaSlotOccupied(System::FriendList *list, u32 idx) {
    if (!list || idx >= VANILLA_FRIEND_COUNT) {
        return false;
    }
    u32 status = list->GetFriendStatus(idx);
    return status != 0 && status != 0xffffffffu;
}

} // namespace

u32 FriendListPage::s_sortedFriendsIdx[MAX_FRIEND_COUNT];
u32 FriendListPage::s_sortedCount = 0;

System::FriendList *FriendListPage::friendList() const {
    auto *sectionMgr = SectionManager::Instance();
    if (!sectionMgr) {
        return nullptr;
    }
    auto *section = sectionMgr->currentSection();
    if (!section) {
        return nullptr;
    }
    return *reinterpret_cast<System::FriendList *const *>(reinterpret_cast<const u8 *>(section) + 0x404);
}

u32 FriendListPage::pageCount() const {
    if (s_sortedCount == 0) {
        return 1;
    }
    return (s_sortedCount + BUTTON_COUNT - 1) / BUTTON_COUNT;
}

void FriendListPage::rebuildSortedIndices() {
    s_sortedCount = 0;
    auto *list = friendList();
    if (list) {
        for (u32 i = 0; i < VANILLA_FRIEND_COUNT; i++) {
            if (vanillaSlotOccupied(list, i)) {
                s_sortedFriendsIdx[s_sortedCount++] = i;
            }
        }
    }
    for (u32 i = 0; i < SP::ExtendedFriends::EXTRA_COUNT; i++) {
        if (SP::ExtendedFriends::s_slots[i].occupied()) {
            s_sortedFriendsIdx[s_sortedCount++] = VANILLA_FRIEND_COUNT + i;
        }
    }

    if (s_sortedCount > 1) {
        qsort(s_sortedFriendsIdx, s_sortedCount, sizeof(u32), compareSorted);
    }

    // Keep member [30] in sync for leftover vanilla reads (first 30 only).
    u32 *member = memberSortedFriendsIdx();
    memset(member, 0, MEMBER_SORTED_COUNT * sizeof(u32));
    u32 copy = s_sortedCount < MEMBER_SORTED_COUNT ? s_sortedCount : MEMBER_SORTED_COUNT;
    memcpy(member, s_sortedFriendsIdx, copy * sizeof(u32));
}

void FriendListPage::refreshFriendButtons() {
    const u32 page = curPageIdx();
    auto *buttons = friendButtons();
    for (u32 b = 0; b < BUTTON_COUNT; b++) {
        u32 listPos = page * BUTTON_COUNT + b;
        if (listPos < s_sortedCount) {
            buttons[b].setVisible(true);
            buttons[b].SetMii(s_sortedFriendsIdx[listPos]);
        } else {
            buttons[b].setVisible(false);
        }
    }
}

void FriendListPage::rebuildRosterView() {
    rebuildSortedIndices();

    u32 pages = pageCount();
    if (curPageIdx() >= pages) {
        curPageIdx() = pages > 0 ? pages - 1 : 0;
    }

    updateMiiGroup();
    updatePageCounter();
    refreshFriendButtons();
}

void FriendListPage::onInit() {
    REPLACED(onInit)();
    // Stock inits MiiGroup for 30. Expand so indices 30..63 have mii slots.
    miiGroup()->init(MAX_FRIEND_COUNT, 0x1, nullptr);
}

void FriendListPage::onActivate() {
    SP::ExtendedFriends::SetUiSafeMode(true);
    REPLACED(onActivate)();
    SP::ExtendedFriends::SetUiSafeMode(false);

    curPageIdx() = 0;
    rebuildRosterView();
}

void FriendListPage::afterCalc() {
    u32 &freq = updateFrequency();
    if (freq == 0) {
        freq = 300;
    }
    if (--freq == 0) {
        freq = 300;
        rebuildRosterView();
    }
}

void FriendListPage::onRightArrowPress(void * /*control*/, u32 /*hudSlotId*/) {
    u32 pages = pageCount();
    if (pages == 0) {
        return;
    }
    curPageIdx() = (curPageIdx() + 1) % pages;
    updatePageCounter();
    refreshFriendButtons();
}

void FriendListPage::onLeftArrowPress(void * /*control*/, u32 /*hudSlotId*/) {
    u32 pages = pageCount();
    if (pages == 0) {
        return;
    }
    curPageIdx() = curPageIdx() == 0 ? pages - 1 : curPageIdx() - 1;
    updatePageCounter();
    refreshFriendButtons();
}

void FriendListPage::addNewFriend(u32 idx) {
    if (idx < VANILLA_FRIEND_COUNT) {
        SP::ExtendedFriends::SetUiSafeMode(true);
        REPLACED(addNewFriend)(idx);
        SP::ExtendedFriends::SetUiSafeMode(false);
    }
    rebuildRosterView();
}

void FriendListPage::updatePageCounter() {
    // Stock counter uses GetFriendCount() for total pages — report the real roster size.
    SP::ExtendedFriends::SetReportRealRosterCount(true);
    REPLACED(updatePageCounter)();
    SP::ExtendedFriends::SetReportRealRosterCount(false);
}

void FriendListPage::updateMiiGroup() {
    auto *list = friendList();
    auto *group = miiGroup();
    if (!list || !group) {
        return;
    }

    for (u32 i = 0; i < s_sortedCount; i++) {
        u32 friendIdx = s_sortedFriendsIdx[i];
        const System::FriendListEntry *entry = list->GetFriendEntry(friendIdx);
        if (!entry) {
            continue;
        }
        // FriendData.friendMii at +0x1a within FriendData at friendInfo+8.
        const auto *raw = reinterpret_cast<const System::RawMii *>(entry->friendInfo + 8 + 0x1a);
        group->insertFromRaw(friendIdx, raw);
    }
}

} // namespace UI
