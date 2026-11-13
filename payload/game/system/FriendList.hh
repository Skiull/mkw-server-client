#pragma once

#include <Common.hh>

#include <game/net/FriendLimits.hh>

#include <cstddef>

namespace System {

// One FriendListEntry (Pulsar). Friend is vtable + pad + FriendData[0x1c0].
struct FriendListEntry {
    u64 friendCode;
    u8 friendInfo[0x1c8]; // Friend at +0x8 of entry; size to friendStatus
    u32 friendStatus;
    u8 _1d4[4];
};
static_assert(sizeof(FriendListEntry) == 0x1d8);
static_assert(offsetof(FriendListEntry, friendStatus) == 0x1d0);

// Game FriendList (Pulsar UI/FriendList.hpp). Size 0x38d0; entries[30] stay vanilla.
// Overflow friends go to SP::ExtendedFriends (slots 30..63).
class FriendList {
public:
    // 0x805d24c0
    REPLACE int GetFriendCount() const;
    int REPLACED(GetFriendCount)() const;

    // 0x805d255c — returns new index, or 0xffffffff when full.
    REPLACE u32 AddFriend(const u64 &friendCode);
    u32 REPLACED(AddFriend)(const u64 &friendCode);

    // 0x805d2700
    REPLACE void RemoveFriend(u32 idx);
    void REPLACED(RemoveFriend)(u32 idx);

    // 0x805d2778 — returns index or 0xffffffff if missing.
    REPLACE u32 GetFriendIdx(const u64 &friendCode);
    u32 REPLACED(GetFriendIdx)(const u64 &friendCode);

    // 0x805d2874
    REPLACE const FriendListEntry *GetFriendEntry(u32 idx) const;
    const FriendListEntry *REPLACED(GetFriendEntry)(u32 idx) const;

    // 0x805d28c8 — -1 none, 0 open, else status bitfield.
    REPLACE u32 GetFriendStatus(u32 idx) const;
    u32 REPLACED(GetFriendStatus)(u32 idx) const;

    // 0x805d2a50 — copy NetManager FriendInfo into FriendList entries for the UI.
    REPLACE void UpdateStatuses();
    void REPLACED(UpdateStatuses)();

    FriendListEntry *entries() {
        return reinterpret_cast<FriendListEntry *>(reinterpret_cast<u8 *>(this) + 0x0);
    }
    const FriendListEntry *entries() const {
        return reinterpret_cast<const FriendListEntry *>(reinterpret_cast<const u8 *>(this) + 0x0);
    }

private:
    u8 _000[0x38d0 - 0x000];
};
static_assert(sizeof(FriendList) == 0x38d0);

} // namespace System
