#include "CtrlRaceNameBalloon.hh"
#include "CtrlRaceResult.hh"

#include "game/net/NetManager.hh"
#include "game/system/RaceConfig.hh"

#include <sp/net/mkw_server/packets/LoungeRanks.hh>

namespace UI {

namespace {

// Rank pictures in the tag (tools/extract_name_tag.py), in Lounge rank order.
const char *const RANK_PANES[] = {"rank_iron", "rank_copper", "rank_bronze", "rank_silver",
        "rank_gold", "rank_platinum", "rank_emerald", "rank_ruby", "rank_diamond", "rank_pearl",
        "rank_master", "rank_grandmaster"};

bool isOnlineRace() {
    auto mode = static_cast<s32>(System::RaceConfig::Instance()->raceScenario().gameMode);
    return mode >= 7 && mode <= 10;
}

} // namespace

// Shows player playerId's Lounge rank picture in place of the star rank (handle_text), if
// they chose to show it. Layouts without the rank pictures (CPU tags) are left alone.
static void showLoungeRank(LayoutUIControl &control, nw4r::lyt::Pane *firstRankPane, u32 playerId) {
    if (firstRankPane == nullptr) {
        return;
    }
    s32 rank = -1;
    auto *net = Net::NetManager::Instance();
    if (net != nullptr && playerId < 12 && isOnlineRace()) {
        rank = MKWServer::loungeRankOfAid(net->aidOfPlayer(playerId));
    }
    for (u32 r = 0; r < std::size(RANK_PANES); r++) {
        control.setPaneVisible(RANK_PANES[r], static_cast<s32>(r) == rank);
    }
    if (rank >= 0) {
        control.setPaneVisible("handle_text", false);
    }
}

void CtrlRaceNameBalloon::refresh(u32 playerId) {
    REPLACED(refresh)(playerId);
    showLoungeRank(*this, m_mainLayout.findPaneByName(RANK_PANES[0]), playerId);
}

void CtrlRaceResult::refreshRankIconsColor(u32 rank, u32 playerId) {
    REPLACED(refreshRankIconsColor)(rank, playerId);
    showLoungeRank(*this, m_mainLayout.findPaneByName(RANK_PANES[0]), playerId);
}

} // namespace UI
