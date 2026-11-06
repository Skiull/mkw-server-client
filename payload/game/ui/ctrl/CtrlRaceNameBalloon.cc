#include "CtrlRaceNameBalloon.hh"
#include "CtrlRaceResult.hh"

#include "game/net/NetManager.hh"
#include "game/system/RaceConfig.hh"

#include <sp/net/mkw_server/packets/LoungeRanks.hh>
#include <sp/settings/Settings.hh>

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

// Player Tags setting (from mkw-sp, see LICENSE-mkw-sp).

BalloonManager::BalloonManager()
    : m_namePositions(static_cast<Vec3 *>(::operator new(sizeof(Vec3) * 12))) {}

BalloonManager::~BalloonManager() {
    ::operator delete(m_namePositions);
}

void BalloonManager::dt(s32 type) {
    if (type > 0) {
        delete this;
    } else {
        this->~BalloonManager();
    }
}

void BalloonManager::init(u8 localPlayerId) {
    m_localPlayerId = localPlayerId;
    for (size_t i = 0; i < std::size(m_playerIds); i++) {
        m_playerIds[i] = -1;
    }
    for (size_t i = 0; i < std::size(m_nameIsEnabled); i++) {
        m_nameIsEnabled[i] = false;
    }
}

void BalloonManager::addNameControl(CtrlRaceNameBalloon * /* nameControl */) {
    m_nameCount++;
}

void BalloonManager::calc() {
    const auto &raceScenario = System::RaceConfig::Instance()->raceScenario();
    if (raceScenario.gameMode != System::RaceConfig::GameMode::TimeAttack) {
        u32 totalNameCount = Settings::getSetting<Settings::Setting::PlayerTags>();
        u32 localPlayerCount = raceScenario.localPlayerCount;
        if (localPlayerCount == 0) {
            localPlayerCount = 1;
        }
        u32 nameCount = totalNameCount / localPlayerCount;
        nameCount += m_localPlayerId < (totalNameCount % localPlayerCount);
        if (nameCount != m_nameCount) {
            for (u32 i = 0; i < m_nameCount; i++) {
                m_playerIds[i] = -1;
                m_nameIsEnabled[i] = false;
            }
            m_nameCount = nameCount;
        }
    }

    REPLACED(calc)();
}

} // namespace UI
