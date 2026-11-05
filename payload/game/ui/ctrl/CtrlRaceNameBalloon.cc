#include "CtrlRaceNameBalloon.hh"

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

void CtrlRaceNameBalloon::refresh(u32 playerId) {
    REPLACED(refresh)(playerId);
    // CPU tags (chara_name_only) have no rank pictures.
    if (m_mainLayout.findPaneByName(RANK_PANES[0]) == nullptr) {
        return;
    }

    s32 rank = -1;
    auto *net = Net::NetManager::Instance();
    if (net != nullptr && playerId < 12 && isOnlineRace()) {
        rank = MKWServer::loungeRankOfAid(net->aidOfPlayer(playerId));
    }
    for (u32 r = 0; r < std::size(RANK_PANES); r++) {
        setPaneVisible(RANK_PANES[r], static_cast<s32>(r) == rank);
    }
    if (rank >= 0) {
        setPaneVisible("handle_text", false);
    }
}

} // namespace UI
