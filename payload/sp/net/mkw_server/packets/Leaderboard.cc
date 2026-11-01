#include "Leaderboard.hh"

#include <string.h>

namespace MKWServer {

static LeaderboardPagePacket s_page{};
static u32 s_serial = 0;

void processLeaderboardPagePacket(const u8 *data) {
    memcpy(&s_page, data, sizeof(s_page));
    s_serial++;
}

const LeaderboardPagePacket &latestLeaderboardPage() {
    return s_page;
}

u32 leaderboardPageSerial() {
    return s_serial;
}

} // namespace MKWServer
