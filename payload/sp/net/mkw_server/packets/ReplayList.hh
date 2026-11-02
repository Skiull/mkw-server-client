#pragma once

#include <Common.hh>

#include <sp/net/mkw_server/packets/MatchRequestHeader.hh>

// The Replays menu: wfc-server sends the newest recordings of races this player was in
// (wfc-server qr2/replay_list.go). Picking one searches region SEARCH_REGION_REPLAY with its
// index.
namespace MKWServer {
#define REPLAY_LIST 0x52504C53 // "RPLS"

struct ReplayListEntry {
    u8 players;
    u8 _1;
    wchar_t course[24]; // Not terminated when 24 long.
    wchar_t when[24];
} __attribute__((packed));
static_assert(sizeof(ReplayListEntry) == 98);

struct ReplayListPacket {
    u8 count;
    u8 _1[3];
    ReplayListEntry entries[12];
} __attribute__((packed));
static_assert(sizeof(ReplayListPacket) == 4 + 12 * 98);

void processReplayListPacket(const u8 *data);

// The latest list received, and a counter that changes whenever one arrives.
const ReplayListPacket &latestReplayList();
u32 replayListSerial();

bool sendReplayListRequest();

// Index of the replay picked in the Replays menu, sent with the room search.
extern u8 s_replayIndex;
} // namespace MKWServer
