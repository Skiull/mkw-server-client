#include "revolution/dwc/DWCFriendsMatch.hh"

#include <sp/net/ExtendedFriends.hh>

void DWC_InitFriendsMatch(void *unused, DWCAccUserData *userdata, int productID,
        const char *gameName, const char *secretKey, int sendBufSize, int recvBufSize,
        DWCFriendData friendList[], int friendListLen) {
    SP::ExtendedFriends::RebuildDwcList(friendList, friendListLen);
    REPLACED(DWC_InitFriendsMatch)(unused, userdata, productID, gameName, secretKey, sendBufSize,
            recvBufSize, SP::ExtendedFriends::s_dwcList,
            static_cast<int>(SP::ExtendedFriends::s_dwcListLen));
}
