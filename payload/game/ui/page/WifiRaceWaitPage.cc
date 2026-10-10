#include "WifiRaceWaitPage.hh"

#include <sp/net/mkw_server/race_verification/RaceManager.hh>

namespace UI {

void WifiRaceWaitPage::afterCalc() {
    // Don't transition to the voting if mkw-server says we can't.
    if (!MKWServer::RaceManager::Instance()->canStartNextRace()) {
        return;
    }

    REPLACED(afterCalc)();
}

} // namespace UI
