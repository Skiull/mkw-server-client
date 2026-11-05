#include "LoungeRanks.hh"

#include <string.h>

namespace MKWServer {

static u8 s_ranks[12] = {};

void processLoungeRanksPacket(const u8 *data) {
    memcpy(s_ranks, data, sizeof(s_ranks));
}

s32 loungeRankOfAid(u8 aid) {
    if (aid >= sizeof(s_ranks) || s_ranks[aid] == 0) {
        return -1;
    }
    return s_ranks[aid] - 1;
}

} // namespace MKWServer
