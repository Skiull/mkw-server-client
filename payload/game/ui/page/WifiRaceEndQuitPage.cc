#include "WifiRaceEndQuitPage.hh"

#include "game/ui/SectionManager.hh"
#include "game/ui/page/WifiRaceEndCountdownPage.hh"

#include <sp/net/mkw_server/RaceManager.hh>

namespace UI {

void WifiRaceEndQuitPage::afterCalc() {
    if (MKWServer::RaceManager::Instance()->canStartNextRace()) {
        // Just call the base function if mkw-server is good to transition.
        REPLACED(afterCalc)();
        return;
    }

    // mkw-server isn't ready to start the next race, so transition to the "Please wait..." page.
    startReplace(Anim::Next, 0.0);
    m_replacement = PageId::WifiRaceEndWait;
    m_isTransitioning = true;
    SectionManager::Instance()->currentSection()->page<PageId::WifiRaceEndCountDown>()->stopTimer();
}

} // namespace UI
