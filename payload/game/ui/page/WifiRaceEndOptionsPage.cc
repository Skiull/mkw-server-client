#include "WifiRaceEndOptionsPage.hh"

#include "game/ui/SectionManager.hh"
#include "game/ui/page/WifiRaceEndCountdownPage.hh"

#include <sp/net/mkw_server/RaceManager.hh>

namespace UI {

void WifiRaceEndOptionsPage::transitionToWaitingPage(f32 delay) {
    startReplace(Anim::Next, delay);
    m_replacement = PageId::WifiRaceEndWait;
    m_isTransitioning = true;
    SectionManager::Instance()->currentSection()->page<PageId::WifiRaceEndCountDown>()->stopTimer();
}

void WifiRaceEndOptionsPage::afterCalc() {
    if (MKWServer::RaceManager::Instance()->canStartNextRace()) {
        REPLACED(afterCalc)();
        return;
    }
    // mkw-server isn't ready to start the next race, so transition to the "Please wait..." page.
    transitionToWaitingPage(0.0);
}

void WifiRaceEndOptionsPage::onContinueButtonFront(PushButton *button) {
    if (MKWServer::RaceManager::Instance()->canStartNextRace()) {
        REPLACED(onContinueButtonFront)(button);
        return;
    }

    f32 delay = button->getDelay();
    transitionToWaitingPage(delay);
}

} // namespace UI
