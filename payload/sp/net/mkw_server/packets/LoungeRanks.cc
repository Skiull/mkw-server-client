#include "LoungeRanks.hh"

#include <nw4r/lyt/lyt_pane.hh>

#include "game/net/NetManager.hh"
#include "game/net/records/User.hh"
#include "game/system/SaveManager.hh"
#include "game/system/Mii.hh"
#include "game/ui/MessageInfo.hh"

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
    auto *save = System::SaveManager::Instance();
    u16 *vr = save ? save->currentLicenseVR() : nullptr;
    if (s_ownVR != 0 && vr != nullptr) {
        *vr = s_ownVR;
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
    // Their own LR goes in their license's VR, which the game sends as their VR (VRCap.S lets
    // it pass 9999). Their VR is put back on leaving.
    auto *net = Net::NetManager::Instance();
    auto *save = System::SaveManager::Instance();
    u16 *vr = save ? save->currentLicenseVR() : nullptr;
    if (net != nullptr && vr != nullptr) {
        u8 me = net->ownAid();
        u16 lr = me < 12 ? s_lrs[me] : 0;
        if (lr != 0 && *vr != lr) {
            if (s_ownVR == 0) {
                s_ownVR = *vr;
            }
            *vr = lr;
        }
    }
}

// Lounge rank colours by LR, as on the leaderboard (wfc-server qr2/leaderboard.go).
static const struct {
    u16 minLR;
    u8 r, g, b;
} RANK_COLORS[] = {{0, 0x81, 0x78, 0x76}, {1250, 0xcf, 0x68, 0x53}, {2500, 0xcd, 0x7f, 0x32},
        {4000, 0xb7, 0xbe, 0xc4}, {5500, 0xff, 0xd7, 0x00}, {7000, 0x3f, 0xab, 0xb8},
        {8250, 0x87, 0xf1, 0x9b}, {9500, 0xf1, 0x38, 0x3a}, {11000, 0xb9, 0xf2, 0xff},
        {12000, 0xe5, 0xb3, 0xc7}, {13000, 0xcf, 0xe2, 0xf3}, {14000, 0x97, 0x17, 0x17}};


static void colorByLR(void *textBox, s32 lr) {
    u32 i = 0;
    while (i + 1 < sizeof(RANK_COLORS) / sizeof(RANK_COLORS[0]) && lr >= RANK_COLORS[i + 1].minLR) {
        i++;
    }
    // As on the leaderboard: the rank colour in the text colours (at 0xdc), the material's
    // fill colour (TEV colour 1, orange here) white.
    u8 *colors = reinterpret_cast<u8 *>(textBox) + 0xdc;
    for (u32 k = 0; k < 2; k++) {
        colors[k * 4 + 0] = RANK_COLORS[i].r;
        colors[k * 4 + 1] = RANK_COLORS[i].g;
        colors[k * 4 + 2] = RANK_COLORS[i].b;
        colors[k * 4 + 3] = 255;
    }
    auto *pane = reinterpret_cast<nw4r::lyt::Pane *>(textBox);
    if (auto *material = pane->getMaterial()) {
        material->tevColors[1].r = 255;
        material->tevColors[1].g = 255;
        material->tevColors[1].b = 255;
        material->tevColors[1].a = 255;
    }
}

void colorPaneByLR(nw4r::lyt::Pane *pane, s32 lr) {
    if (s_mogi && pane != nullptr) {
        colorByLR(pane, lr);
    }
}

extern "C" u32 MogiRemapMessage(void * /* textBox */, u32 messageId, const UI::MessageInfo * /* info */) {
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
