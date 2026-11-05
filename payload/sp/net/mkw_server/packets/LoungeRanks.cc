#include "LoungeRanks.hh"

#include "game/net/NetManager.hh"
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

// The player's own VR, while their LR is sent in its place (0: not replaced).
static u16 s_ownVR = 0;

void leftMogiRoom() {
    s_mogi = false;
    auto *users = Net::UserHandler::Instance();
    if (s_ownVR != 0 && users != nullptr) {
        users->setOwnVR(s_ownVR);
    }
    s_ownVR = 0;
}

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
    // Their own row reads the record they send: put their LR there too (back on leaving).
    auto *net = Net::NetManager::Instance();
    if (net != nullptr) {
        u8 me = net->ownAid();
        // Capped at 9999: other players' games drop records with a VR above it.
        u16 lr = s_lrs[me] < 9999 ? s_lrs[me] : 9999;
        if (me < 12 && s_lrs[me] != 0 && users->myVR() != lr) {
            if (s_ownVR == 0) {
                s_ownVR = users->myVR();
            }
            users->setOwnVR(lr);
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
