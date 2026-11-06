#pragma once

#include "game/system/RaceConfig.hh"

#include <sp/settings/Settings.hh>

namespace SP {

// Time Trial practice tools (save states, item wheel) must never work online. They need an
// offline Time Trial: the TimeAttack game mode, and no racer that comes from the network.
inline bool IsOfflineTimeTrial() {
    auto *raceConfig = System::RaceConfig::Instance();
    if (raceConfig == nullptr) {
        return false;
    }
    auto &scenario = raceConfig->raceScenario();
    if (scenario.gameMode != System::RaceConfig::GameMode::TimeAttack) {
        return false;
    }
    for (u32 i = 0; i < 12; i++) {
        if (scenario.players[i].type == System::RaceConfig::Player::Type::Online) {
            return false;
        }
    }
    return true;
}

inline bool ItemWheelAllowed() {
    return IsOfflineTimeTrial() &&
            Settings::getSetting<Settings::Setting::YButton>() == Settings::YButton::ItemWheel;
}

inline bool SaveStatesAllowed() {
    return IsOfflineTimeTrial() &&
            Settings::getSetting<Settings::Setting::SaveStates>() == Settings::SaveStates::Enabled;
}

} // namespace SP
