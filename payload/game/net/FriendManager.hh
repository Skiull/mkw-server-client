#pragma once

#include <Common.hh>

#include <game/net/FriendLimits.hh>
#include <revolution/dwc/DWCFriend.h>
#include <revolution/dwc/DWCLogin.h>

namespace Net {

// RKNet::FriendMgr (Pulsar). Size 0x3e8; s_instance at 0x809c2110.
// friends[30][2] stays vanilla-sized -- slots 30..63 live in SP::ExtendedFriends.
class FriendRosterManager {
public:
    static FriendRosterManager *Instance() {
        return s_instance;
    }

    // 0x80663940 — first free AccFriendData index, or -1.
    // Extended: also returns VANILLA_FRIEND_COUNT + extra when 0..29 are full.
    REPLACE s32 GetFirstFreeFriendIdx() const;
    s32 REPLACED(GetFirstFreeFriendIdx)() const;

    // 0x806632fc — reload AccFriendData 0..29 from rksys.
    REPLACE void ReadDataFromRKSYS();
    void REPLACED(ReadDataFromRKSYS)();

    // Contiguous view of primary AccFriendData for indices 0..29 (friends[i][0]).
    // Layout is friends[30][2] at +0x9c per Pulsar; [i][0] stride is 0x18.
    const DWCFriendData *primaryFriend(u32 index) const {
        if (index >= VANILLA_FRIEND_COUNT) {
            return nullptr;
        }
        const u8 *base = reinterpret_cast<const u8 *>(this) + 0x9c;
        return reinterpret_cast<const DWCFriendData *>(base + index * 0x18);
    }

    DWCFriendData *primaryFriend(u32 index) {
        if (index >= VANILLA_FRIEND_COUNT) {
            return nullptr;
        }
        u8 *base = reinterpret_cast<u8 *>(this) + 0x9c;
        return reinterpret_cast<DWCFriendData *>(base + index * 0x18);
    }

private:
    static FriendRosterManager *s_instance;

    u8 _000[0x3e8 - 0x000];
};
static_assert(sizeof(FriendRosterManager) == 0x3e8);

} // namespace Net
