#pragma once

#include <Common.hh>

// Lounge ranks on name tags (wfc-server qr2/lounge_rank.go): which aids show their MKW Lounge
// rank, and the player's own Ranks setting sent to the server.
namespace nw4r::lyt {
class Pane;
}

namespace MKWServer {
#define LOUNGE_RANKS 0x524E4B53 // "RNKS"
constexpr u32 LOUNGE_RANKS_SIZE = 12 + 2 + 12 * 2;

void processLoungeRanksPacket(const u8 *data);

// The Lounge rank (index into the rank icons) aid shows, or -1.
s32 loungeRankOfAid(u8 aid);

bool sendRankSetting(bool showLoungeRank);

// In a mogi room each player's LR is shown in place of their VR (MogiShowLR, called every
// frame) and the "VR" labels read "LR" (TextBox_setMessage hook, sp/net/mkw_server/LR.S).
bool inMogiRoom();
void leftMogiRoom();
void mogiShowLR();
// In mogi rooms, draws an LR text pane in its Lounge rank colour (Check Members, message 5105).
void colorPaneByLR(nw4r::lyt::Pane *pane, s32 lr);
} // namespace MKWServer
