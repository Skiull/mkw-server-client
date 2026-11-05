#pragma once

#include <Common.hh>

// Lounge ranks on name tags (wfc-server qr2/lounge_rank.go): which aids show their MKW Lounge
// rank, and the player's own Ranks setting sent to the server.
namespace MKWServer {
#define LOUNGE_RANKS 0x524E4B53 // "RNKS"
constexpr u32 LOUNGE_RANKS_SIZE = 12;

void processLoungeRanksPacket(const u8 *data);

// The Lounge rank (index into the rank icons) aid shows, or -1.
s32 loungeRankOfAid(u8 aid);

bool sendRankSetting(bool showLoungeRank);
} // namespace MKWServer
