// See LICENSE-mkw-sp

#include "CtrlRaceBase.hh"

#include "game/system/RaceConfig.hh"

#include <sp/settings/Settings.hh>

namespace UI {

CtrlRaceBase::CtrlRaceBase() = default;

CtrlRaceBase::~CtrlRaceBase() = default;

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
