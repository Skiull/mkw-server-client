// Credits: CLF78: Midnight-Variety-Pack, licensed under the Mozilla Public License 2.0.
// See license in LICENSE-Midnight-Variety-Pack.

#pragma once

#include <Common.h>

#include <revolution/gamespy/gp/gp.h>

#include <stddef.h>

typedef struct {
    u32 flags;
    u8 _4[0xc - 0x4];
} DWCAccFlag;
static_assert(sizeof(DWCAccFlag) == 0xc);

typedef struct {
    u32 id_data;
    u32 userId;
    u32 playerId;
} DWCLoginId;
static_assert(sizeof(DWCLoginId) == 0xc);

typedef struct {
    u32 id_data;
    u32 friendkey[2];
} DWCFriendKey;
static_assert(sizeof(DWCFriendKey) == 0xc);

typedef struct {
    u32 id_data;
    int id;
    u8 _8[0xc - 0x8];
} DWCGsProfileId;
static_assert(sizeof(DWCGsProfileId) == 0xc);

typedef union {
    DWCAccFlag flags;
    DWCLoginId login_id;
    DWCFriendKey friend_key;
    DWCGsProfileId gs_profile_id;
} DWCFriendData;
static_assert(sizeof(DWCFriendData) == 0xc);

typedef struct {
    u8 _00[0x04 - 0x00];
    GPConnection *gpConnection;
    u8 _08[0x18 - 0x08];
    int friendListLen; // 0x18
    DWCFriendData *friendList; // 0x1c
    u8 _20[0x60 - 0x20];
} DWCFriendControl;
static_assert(sizeof(DWCFriendControl) == 0x60);
static_assert(offsetof(DWCFriendControl, friendListLen) == 0x18);
static_assert(offsetof(DWCFriendControl, friendList) == 0x1c);
