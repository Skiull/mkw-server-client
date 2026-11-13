#include "game/net/FriendManager.hh"

#include <sp/net/ExtendedFriends.hh>

namespace Net {

s32 FriendRosterManager::GetFirstFreeFriendIdx() const {
    s_instance = const_cast<FriendRosterManager *>(this);
    s32 vanilla = REPLACED(GetFirstFreeFriendIdx)();
    if (vanilla >= 0) {
        return vanilla;
    }

    u32 extra = SP::ExtendedFriends::FirstFreeSlot();
    if (extra < SP::ExtendedFriends::EXTRA_COUNT) {
        return static_cast<s32>(VANILLA_FRIEND_COUNT + extra);
    }
    return -1;
}

void FriendRosterManager::ReadDataFromRKSYS() {
    s_instance = this;
    REPLACED(ReadDataFromRKSYS)();
    // Extras should be on SD (Loaded from NetManager::init / license select).
    SP::ExtendedFriends::RebuildDwcList(this);
}

} // namespace Net
