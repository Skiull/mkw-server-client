#include "CtrlRaceNameBalloon.hh"
#include "CtrlRaceResult.hh"

#include "game/net/NetManager.hh"
#include "game/system/RaceConfig.hh"
#include "game/ui/page/RacePage.hh"

#include <sp/TimeTrialCheats.hh>

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
    refreshGhostText(playerId);
}

// Ghost tag content (port of mkw-sp's TAGhostTagContent), offline Time Trials only. The game
// has just written the ghost's Mii name; replace it with its time or date if chosen.
void CtrlRaceNameBalloon::refreshGhostText(u32 playerId) {
    if (playerId >= 12 || !SP::IsOfflineTimeTrial()) {
        return;
    }
    const auto &scenario = System::RaceConfig::Instance()->raceScenario();
    if (scenario.players[playerId].type != System::RaceConfig::Player::Type::Ghost) {
        return;
    }
    auto content = Settings::getSetting<Settings::Setting::TAGhostTagContent>();
    if (content == Settings::TAGhostTagContent::Name) {
        return;
    }
    // Player 0 is the ghost in a replay, otherwise the first ghost races as player 1. The game
    // keeps one ghost file per race.
    bool isReplay = scenario.players[0].type == System::RaceConfig::Player::Type::Ghost;
    u32 index = isReplay ? playerId : playerId - 1;
    if (index != 0 || scenario.ghostBuffer == nullptr) {
        return;
    }
    const u8 *header = scenario.ghostBuffer[0];
    if (header[0] != 'R' || header[1] != 'K' || header[2] != 'G' || header[3] != 'D') {
        return;
    }
    MessageInfo info{};
    if (content == Settings::TAGhostTagContent::Date) {
        // Bits from the top of 0x8: vehicle 6, character 6, year 7, month 4, day 5.
        u32 word = header[8] << 24 | header[9] << 16 | header[10] << 8 | header[11];
        info.intVals[0] = 2000 + (word >> 13 & 0x7f);
        info.intVals[1] = word >> 9 & 0xf;
        info.intVals[2] = word >> 4 & 0x1f;
        setMessage("chara_name", 10145, &info);
        return;
    }
    // Finish time at 0x4: minutes 7 bits, seconds 7, milliseconds 10.
    info.intVals[0] = header[4] >> 1;
    info.intVals[1] = (header[4] & 1) << 6 | header[5] >> 2;
    info.intVals[2] = (header[5] & 3) << 8 | header[6];
    bool leadingZeroes = content == Settings::TAGhostTagContent::Time;
    setMessage("chara_name", leadingZeroes ? 1400 : 10144, &info);
}

// Ghost tag visibility (port of mkw-sp's TAGhostTagVisibility), offline Time Trials only.
void CtrlRaceNameBalloon::calcSelf() {
    REPLACED(calcSelf)();
    if (!getVisible() || !SP::IsOfflineTimeTrial()) {
        return;
    }
    switch (Settings::getSetting<Settings::Setting::TAGhostTagVisibility>()) {
    case Settings::TAGhostTagVisibility::None:
        setVisible(false);
        break;
    case Settings::TAGhostTagVisibility::Watched: {
        auto *racePage = RacePage::Instance();
        if (racePage != nullptr && static_cast<u32>(m_playerId) != racePage->watchedPlayerId()) {
            setVisible(false);
        }
        break;
    }
    default:
        break;
    }
}

void CtrlRaceResult::refreshRankIconsColor(u32 rank, u32 playerId) {
    REPLACED(refreshRankIconsColor)(rank, playerId);
    showLoungeRank(*this, m_mainLayout.findPaneByName(RANK_PANES[0]), playerId);
}

} // namespace UI
