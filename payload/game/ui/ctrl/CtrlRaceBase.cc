// See LICENSE-mkw-sp

#include "CtrlRaceBase.hh"

#include "game/system/RaceConfig.hh"

#include <sp/settings/Settings.hh>

namespace UI {

CtrlRaceBase::CtrlRaceBase() = default;

CtrlRaceBase::~CtrlRaceBase() = default;

// HUD Team Colors setting.
void CtrlRaceBase::setPaneColor(const char *paneName, bool teamColors) {
    auto &raceScenario = System::RaceConfig::Instance()->raceScenario();
    if (raceScenario.localPlayerCount < 2) {
        teamColors = true;
    }

    if (teamColors) {
        auto setting = Settings::getSetting<Settings::Setting::HUDTeamColors>();
        if (setting == Settings::HUDTeamColors::Disable) {
            teamColors = false;
        }
    }

    REPLACED(setPaneColor)(paneName, teamColors);
}

// HUD Labels setting.
void CtrlRaceBase::calcLabelVisibility(const char *paneName) {
    bool visible = System::RaceConfig::Instance()->raceScenario().localPlayerCount <= 2;
    auto setting = Settings::getSetting<Settings::Setting::HUDLabels>();
    if (setting == Settings::HUDLabels::Hide) {
        visible = false;
    }
    if (auto *pane = m_mainLayout.findPaneByName(paneName)) {
        pane->m_visible = visible;
    }
}

} // namespace UI
