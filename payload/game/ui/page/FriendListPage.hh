#pragma once

#include "game/net/FriendLimits.hh"
#include "game/system/FriendList.hh"
#include "game/ui/MiiGroup.hh"
#include "game/ui/ctrl/Button.hh"
#include "game/ui/page/Page.hh"

#include <cstddef>

namespace UI {

// Game FriendButton is PushButton (0x254) + fields to 0x264 (Pulsar).
class FriendListFriendButton : public PushButton {
public:
    // 0x805d3980
    void SetMii(u32 friendIdx);

    MiiGroup *miiGroup; // 0x254
    u32 friendIdx; // 0x258
    u32 status; // 0x25c
    bool isSelected; // 0x260
    u8 _261[0x264 - 0x261];
};
static_assert(sizeof(FriendListFriendButton) == 0x264);
static_assert(offsetof(FriendListFriendButton, friendIdx) == 0x258);

// Pages::FriendList (Pulsar). Keep object size 0x1a64 — sortedFriendsIdx stays [30].
// Extended roster uses s_sortedFriendsIdx[64] and REPLACE hooks for paging.
class FriendListPage : public Page {
public:
    static constexpr u32 BUTTON_COUNT = 5;
    static constexpr u32 MEMBER_SORTED_COUNT = VANILLA_FRIEND_COUNT;

    // 0x805d446c
    REPLACE void onInit() override;
    void REPLACED(onInit)();

    // 0x805d4834
    REPLACE void onActivate() override;
    void REPLACED(onActivate)();

    // 0x805d4c10 — AfterControlUpdate
    REPLACE void afterCalc() override;
    void REPLACED(afterCalc)();

    // 0x805d4ee4
    REPLACE void onRightArrowPress(void *control, u32 hudSlotId);
    void REPLACED(onRightArrowPress)(void *control, u32 hudSlotId);

    // 0x805d5014
    REPLACE void onLeftArrowPress(void *control, u32 hudSlotId);
    void REPLACED(onLeftArrowPress)(void *control, u32 hudSlotId);

    // 0x805d4c8c
    REPLACE void addNewFriend(u32 idx);
    void REPLACED(addNewFriend)(u32 idx);

    // 0x805d5330
    REPLACE void updatePageCounter();
    void REPLACED(updatePageCounter)();

    // 0x805d53f8
    REPLACE void updateMiiGroup();
    void REPLACED(updateMiiGroup)();

    // 0x805d518c — stock qsort comparator
    static int CompareFriendIndexes(u32 *idx1, u32 *idx2);

    void rebuildRosterView();

    FriendListFriendButton *friendButtons() {
        return reinterpret_cast<FriendListFriendButton *>(bytes() + 0x268);
    }
    MiiGroup *miiGroup() {
        return reinterpret_cast<MiiGroup *>(bytes() + 0x18e0);
    }
    u32 *memberSortedFriendsIdx() {
        return reinterpret_cast<u32 *>(bytes() + 0x1978);
    }
    u32 &curPageIdx() {
        return *reinterpret_cast<u32 *>(bytes() + 0x19f4);
    }
    u32 &updateFrequency() {
        return *reinterpret_cast<u32 *>(bytes() + 0x19f8);
    }

    static u32 s_sortedFriendsIdx[MAX_FRIEND_COUNT];
    static u32 s_sortedCount;

private:
    u8 *bytes() {
        return reinterpret_cast<u8 *>(this);
    }
    const u8 *bytes() const {
        return reinterpret_cast<const u8 *>(this);
    }

    System::FriendList *friendList() const;
    u32 pageCount() const;
    void rebuildSortedIndices();
    void refreshFriendButtons();
};

} // namespace UI
