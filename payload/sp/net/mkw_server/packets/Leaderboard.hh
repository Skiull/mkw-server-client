#pragma once

#include <Common.hh>

#include <sp/net/mkw_server/packets/MatchRequestHeader.hh>

// The in-game leaderboard: wfc-server sends one page (10 players) of the MKW Lounge Regular
// Track ladder on request (wfc-server qr2/leaderboard.go).
namespace MKWServer {
#define LEADERBOARD_PAGE 0x4C444244 // "LDBD"

struct LeaderboardRequestPacket {
    MatchRequestHeader header;
    u16 page;
    u8 _12[0x18 - 0x12];
};
static_assert(sizeof(LeaderboardRequestPacket) == 0x18);

struct LeaderboardRow {
    u32 position; // 0: empty row.
    u16 lr;
    u8 rank; // Index into the rank icons (iron .. grandmaster).
    u8 color[3];
    u8 country;    // Wii country code, as the game's flag panes are named. 0: no flag.
    u8 controller; // 0: none, 1 Wii Wheel, 2 Nunchuk, 3 Classic, 4 GameCube.
    wchar_t name[16]; // Not terminated when 16 long.
} __attribute__((packed));
static_assert(sizeof(LeaderboardRow) == 44);

struct LeaderboardPagePacket {
    u16 page;
    u16 pageCount;
    LeaderboardRow rows[10];
} __attribute__((packed));
static_assert(sizeof(LeaderboardPagePacket) == 4 + 440);

void processLeaderboardPagePacket(const u8 *data);

// The latest page received, and a counter that changes whenever one arrives.
const LeaderboardPagePacket &latestLeaderboardPage();
u32 leaderboardPageSerial();

bool sendLeaderboardRequest(u16 page);
} // namespace MKWServer
