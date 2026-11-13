#pragma once

#include <Common.hh>

#include <revolution/dwc/DWCFriend.h>
#include <revolution/dwc/DWCLogin.h>

// 0x800d0d68 — merge ExtendedFriends into the DWC friend list (up to 64).
// C linkage: DWC is C, so symbols.txt has the plain name.
extern "C" {
REPLACE void DWC_InitFriendsMatch(void *unused, DWCAccUserData *userdata, int productID,
        const char *gameName, const char *secretKey, int sendBufSize, int recvBufSize,
        DWCFriendData friendList[], int friendListLen);
void REPLACED(DWC_InitFriendsMatch)(void *unused, DWCAccUserData *userdata, int productID,
        const char *gameName, const char *secretKey, int sendBufSize, int recvBufSize,
        DWCFriendData friendList[], int friendListLen);
}

// 0x800ec764
EXTERN_C void DWC_CreateFriendKeyToken(DWCFriendData *dest, u32 unused, u64 friendCode);

// 0x800ce190 — returns DWC friend-presence status; writes game StatusData into statusData.
EXTERN_C u8 DWC_GetFriendStatusData(const DWCFriendData *friendData, char *statusData, int *size);

// 0x800ce24c
EXTERN_C BOOL DWC_SetOwnStatusData(const char *statusData, u32 size);

// 0x800eb870
EXTERN_C BOOL DWC_IsBuddyFriendData(const DWCFriendData *friendData);
