#include "LoungeRanks.hh"

#include "game/net/records/User.hh"

#include <string.h>

namespace MKWServer {

static u8 s_ranks[12] = {};
static bool s_mogi = false;
static u16 s_lrs[12] = {};

void processLoungeRanksPacket(const u8 *data) {
    memcpy(s_ranks, data, sizeof(s_ranks));
    s_mogi = data[12] != 0;
    memcpy(s_lrs, data + 14, sizeof(s_lrs)); // Big endian, like the console.
}

s32 loungeRankOfAid(u8 aid) {
    if (aid >= sizeof(s_ranks) || s_ranks[aid] == 0) {
        return -1;
    }
    return s_ranks[aid] - 1;
}

bool inMogiRoom() {
    return s_mogi;
}

void leftMogiRoom() {
    s_mogi = false;
}

// The player's own VR isn't replaced: the record they send must stay a valid VR (the game
// drops other players' records with VR over 9999, and they'd never connect).

void mogiShowLR() {
    auto *users = Net::UserHandler::Instance();
    if (!s_mogi || users == nullptr) {
        return;
    }
    for (u8 aid = 0; aid < 12; aid++) {
        if (s_lrs[aid] != 0) {
            users->setReceivedVR(aid, s_lrs[aid]);
        }
    }
}

extern "C" u32 MogiRemapMessage(u32 messageId) {
    if (!s_mogi) {
        return messageId;
    }
    switch (messageId) {
    case 4202: // "{n}VR"
        return 10034;
    case 9700: // "VR"
    case 1343:
        return 10024; // "LR"
    default:
        return messageId;
    }
}

} // namespace MKWServer
