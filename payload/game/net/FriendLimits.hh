#pragma once

// Vanilla rksys.dat / NetManager layout holds 30 friends. This client raises the
// usable cap to 64 (DWC's friendIdxList is already 64). Slots 0..29 stay in the vanilla
// NetManager array and rksys.dat; slots 30..63 live in NetManager::m_extraFriends and
// /mkw-sp/friends-L#.bin (see sp/net/ExtendedFriends.hh).
#define VANILLA_FRIEND_COUNT 30
#define MAX_FRIEND_COUNT 64
#define EXTRA_FRIEND_COUNT (MAX_FRIEND_COUNT - VANILLA_FRIEND_COUNT)
