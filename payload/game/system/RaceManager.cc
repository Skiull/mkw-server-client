#include "RaceManager.hh"

#include "game/item/ItemDirector.hh"
#include "game/item/PowManager.hh"
#include "game/system/RaceConfig.hh"

#include <sp/SaveStateManager.hh>
#include <sp/TimeTrialCheats.hh>

extern "C" {
#include <revolution.h>
#include <revolution/kpad.h>
}

namespace System {

// Hop dodge practice sets off one POW Block per button press, not one per frame.
static bool s_powButtonHeld = false;

void RaceManager::Player::calc() {
    REPLACED(calc)();

    // Check if user wants to load state, rotate items, or use pow.
    auto *raceConfig = System::RaceConfig::Instance();
    auto gameMode = raceConfig->raceScenario().gameMode;
    // These settings are exclusive to TTs. We check for playerId to prevent unintended behavior
    // from ghosts.
    if (gameMode == System::RaceConfig::GameMode::TimeAttack && m_playerId == 0 &&
            SP::IsOfflineTimeTrial()) {
        auto *playerPadProxy = m_padProxy;
        auto buttons = playerPadProxy->currentRaceInputState().rawButtons;
        auto controller = playerPadProxy->pad()->getControllerId();

        bool shouldUpdateItem = false;
        bool isLoadButtonPressed = false;
        bool isPowButtonPressed = false;
        switch (controller) {
        case Registry::Controller::WiiWheel:
            shouldUpdateItem = (buttons & PAD_BUTTON_START) == PAD_BUTTON_START;
            isLoadButtonPressed = (buttons & WPAD_CL_BUTTON_HOME) == WPAD_CL_BUTTON_HOME;
            isPowButtonPressed = (buttons & (WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT)) != 0;
            break;
        case Registry::Controller::WiiRemoteAndNunchuck:
            shouldUpdateItem = (buttons & WPAD_BUTTON_DOWN) == WPAD_BUTTON_DOWN;
            isLoadButtonPressed = (buttons & WPAD_CL_BUTTON_DOWN) == WPAD_CL_BUTTON_DOWN;
            isPowButtonPressed = (buttons & (WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT)) != 0;
            break;
        case Registry::Controller::Classic:
            shouldUpdateItem = (buttons & KPAD_CL_TRIGGER_ZL) == KPAD_CL_TRIGGER_ZL;
            isLoadButtonPressed = (buttons & KPAD_CL_TRIGGER_ZR) == KPAD_CL_TRIGGER_ZR;
            isPowButtonPressed = (buttons & KPAD_CL_BUTTON_X) == KPAD_CL_BUTTON_X;

            break;
        case Registry::Controller::GameCube:
            shouldUpdateItem = (buttons & PAD_BUTTON_Y) == PAD_BUTTON_Y;
            isLoadButtonPressed = (buttons & PAD_TRIGGER_Z) == PAD_TRIGGER_Z;
            isPowButtonPressed = (buttons & PAD_BUTTON_X) == PAD_BUTTON_X;
            break;
        case Registry::Controller::None:
            return;
        }

        auto *itemDirector = Item::ItemDirector::Instance();
        if (itemDirector && SP::ItemWheelAllowed()) {
            itemDirector->processInput(shouldUpdateItem);
        }

        auto *saveStateManager = SP::SaveStateManager::Instance();
        if (saveStateManager && SP::SaveStatesAllowed()) {
            saveStateManager->processInput(isLoadButtonPressed);
        }

        // Hop dodge practice: a POW Block that hits only the player (not the ghosts).
        auto *powManager = Item::PowManager::Instance();
        if (powManager && SP::HopDodgePracticeAllowed() && isPowButtonPressed &&
                !s_powButtonHeld) {
            powManager->beginPow(0);
            for (u32 i = 0; i < 12; i++) {
                powManager->setAffected(i, i == 0);
            }
            SP::MarkPracticeUsed();
        }
        s_powButtonHeld = isPowButtonPressed;
    }
}

} // namespace System

void RaceManager_startCountdown() {
    if (auto *raceManager = System::RaceManager::Instance()) {
        raceManager->startCountdown();
    } else {
        // Shouldn't happen but log it if it does.
        SP_LOG("RaceManager_startCountdown() called but RaceManager is nullptr!");
    }
}
