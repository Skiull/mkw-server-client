#include "RaceMenuPage.hh"

#include "game/system/RaceConfig.hh"
#include "game/ui/Section.hh"
#include "game/ui/SectionManager.hh"
#include "game/ui/page/SettingsPage.hh"

#include <sp/SaveStateManager.hh>
#include <sp/TimeTrialCheats.hh>

namespace UI {

namespace {

// An offline VS, Time Trial or battle race: no racer comes from the network. Section.cc adds
// our Settings page to the offline race sections only. Online races keep the game's own pause
// menu: the race carries on underneath it, and leaving it for another page could leave this
// console's menu state out of step with the room.
bool IsOfflineRace() {
    auto *raceConfig = System::RaceConfig::Instance();
    if (raceConfig == nullptr) {
        return false;
    }
    auto &scenario = raceConfig->raceScenario();
    using GameMode = System::RaceConfig::GameMode;
    if (scenario.gameMode != GameMode::OfflineVS && scenario.gameMode != GameMode::TimeAttack &&
            scenario.gameMode != GameMode::OfflineBT) {
        return false;
    }
    for (u32 i = 0; i < 12; i++) {
        if (scenario.players[i].type == System::RaceConfig::Player::Type::Online) {
            return false;
        }
    }
    return true;
}

using B = RaceMenuPage::ButtonId;

const B taButtons[] = {B::Continue1, B::Restart1, B::SaveState, B::LoadState, B::ChangeCourse,
        B::ChangeCharacter, B::Settings, B::Quit1};
const B taButtonsNoSS[] = {
        B::Continue1, B::Restart1, B::ChangeCourse, B::ChangeCharacter, B::Settings, B::Quit1};
const B vsButtons[] = {B::Continue1, B::Restart1, B::Settings, B::Quit1};

bool UseTaButtons() {
    return IsOfflineRace() && SP::IsOfflineTimeTrial();
}

} // namespace

PageId RaceMenuPage::s_settingsOpenedFrom = PageId::None;

void RaceMenuPage::OnSettingsActivated() {
    if (s_settingsOpenedFrom == PageId::None) {
        return;
    }
    auto *section = SectionManager::Instance()->currentSection();
    if (auto *page = section->page(s_settingsOpenedFrom)) {
        // Clear its replacement, so that it doesn't send us back here.
        static_cast<RaceMenuPage *>(page)->setReplacement(PageId::None);
    }
    s_settingsOpenedFrom = PageId::None;
}

bool RaceMenuPage::hasSPButtons() {
    // Our ids reuse the game's: only handle them when this page shows one of our lists.
    const ButtonId *buttons = getButtons();
    return buttons == taButtons || buttons == taButtonsNoSS || buttons == vsButtons;
}

void RaceMenuPage::onButtonFront(PushButton *button, u32 localPlayerId) {
    if (hasSPButtons()) {
        switch (static_cast<ButtonId>(button->m_index)) {
        case ButtonId::Settings:
            if (IsOfflineRace()) {
                onSettingsButtonFront(button);
            }
            return;
        case ButtonId::SaveState:
            if (SP::SaveStatesAllowed()) {
                if (auto *saveStateManager = SP::SaveStateManager::Instance()) {
                    saveStateManager->save();
                }
            }
            return;
        case ButtonId::LoadState:
            if (SP::SaveStatesAllowed()) {
                if (auto *saveStateManager = SP::SaveStateManager::Instance()) {
                    saveStateManager->reload();
                }
            }
            return;
        default:
            break;
        }
    }
    REPLACED(onButtonFront)(button, localPlayerId);
}

void RaceMenuPage::onSettingsButtonFront(PushButton *button) {
    auto *section = SectionManager::Instance()->currentSection();
    auto *menuSettingsPage = section->page<PageId::MenuSettings>();
    if (menuSettingsPage == nullptr) {
        return; // Not one of the sections that Section.cc adds the page to
    }
    menuSettingsPage->configure(nullptr, id());
    s_settingsOpenedFrom = id();
    setReplacement(PageId::MenuSettings);
    startReplace(Anim::Next, button->getDelay());
}

s32 TaRaceMenuPage::getButtonCount() {
    if (!UseTaButtons()) {
        return REPLACED(getButtonCount)();
    }
    return SP::SaveStatesAllowed() ? std::size(taButtons) : std::size(taButtonsNoSS);
}

const RaceMenuPage::ButtonId *TaRaceMenuPage::getButtons() {
    if (!UseTaButtons()) {
        return REPLACED(getButtons)();
    }
    return SP::SaveStatesAllowed() ? taButtons : taButtonsNoSS;
}

const char *TaRaceMenuPage::getResFileName() {
    if (!UseTaButtons()) {
        return REPLACED(getResFileName)();
    }
    return SP::SaveStatesAllowed() ? "PauseMenuTimeAttackSP" : "PauseMenuTimeAttackSPNoSS";
}

s32 VsRaceMenuPage::getButtonCount() {
    return IsOfflineRace() ? static_cast<s32>(std::size(vsButtons)) : REPLACED(getButtonCount)();
}

const RaceMenuPage::ButtonId *VsRaceMenuPage::getButtons() {
    return IsOfflineRace() ? vsButtons : REPLACED(getButtons)();
}

const char *VsRaceMenuPage::getResFileName() {
    return IsOfflineRace() ? "PauseMenuVSSP" : REPLACED(getResFileName)();
}

s32 BtRaceMenuPage::getButtonCount() {
    return IsOfflineRace() ? static_cast<s32>(std::size(vsButtons)) : REPLACED(getButtonCount)();
}

const RaceMenuPage::ButtonId *BtRaceMenuPage::getButtons() {
    return IsOfflineRace() ? vsButtons : REPLACED(getButtons)();
}

const char *BtRaceMenuPage::getResFileName() {
    return IsOfflineRace() ? "PauseMenuVSSP" : REPLACED(getResFileName)();
}

} // namespace UI
