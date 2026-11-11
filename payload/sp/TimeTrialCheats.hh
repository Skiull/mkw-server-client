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

inline bool ItemBoxesAllowed() {
    return IsOfflineTimeTrial() &&
            Settings::getSetting<Settings::Setting::TAItemBoxes>() == Settings::TAItemBoxes::Enabled;
}

inline bool LongerTCsAllowed() {
    return IsOfflineTimeTrial() &&
            Settings::getSetting<Settings::Setting::TALongerTCs>() == Settings::TALongerTCs::Enabled;
}

inline bool HopDodgePracticeAllowed() {
    return IsOfflineTimeTrial() &&
            Settings::getSetting<Settings::Setting::TAHopDodgePractice>() ==
            Settings::TAHopDodgePractice::Enabled;
}

// Grid spot and grid size for the player's start, or false for the normal Time Trial start.
inline bool StartingSpotChanged(u32 *spot, u32 *layout) {
    if (!IsOfflineTimeTrial()) {
        return false;
    }
    u32 s = Settings::getSetting<Settings::Setting::TAStartingSpot>();
    u32 l = Settings::getSetting<Settings::Setting::TAStartingSpotsLayout>();
    if (s <= 1 && l <= 1) {
        return false;
    }
    *spot = s < 1 ? 1 : s;
    *layout = l < *spot ? *spot : l; // The spot has to exist in the grid
    return true;
}

// Like mkw-sp: a run that used a practice tool (item wheel, save states, item boxes, longer
// Thunder Clouds, hop dodge practice) can't save its ghost. Reset when a race scene is created.
void ResetPracticeUsed();
void MarkPracticeUsed();
bool PracticeUsed();

} // namespace SP

extern "C" {
// Read by the item box patches (game/item/ItemBoxes.S), set when a race scene is created.
extern u8 TimeTrialCheats_itemBoxes;
}
