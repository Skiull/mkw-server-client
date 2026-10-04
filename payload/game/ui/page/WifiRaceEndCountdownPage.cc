#include "WifiRaceEndCountdownPage.hh"

namespace UI {

void WifiRaceEndCountdownPage::stopTimer() {
    m_countdownTimer.isRunning = false;
    m_timerControl.reset();
}

} // namespace UI
