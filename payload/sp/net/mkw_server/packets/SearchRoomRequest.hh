#pragma once

#include <Common.hh>

#include <sp/net/mkw_server/packets/MatchRequestHeader.hh>
namespace MKWServer {
struct SearchRoomRequestPacket {
    MatchRequestHeader header;
    SearchRegion region : 8;
    GameMode gameMode : 8;
    u8 replayIndex; // SEARCH_REGION_REPLAY: which of the player's replays (0: newest).
    u8 _13[0x18 - 0x13];
};
static_assert(sizeof(SearchRoomRequestPacket) == 0x18);
} // namespace MKWServer