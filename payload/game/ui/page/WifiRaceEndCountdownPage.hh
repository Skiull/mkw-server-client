#pragma once

#include <Common.hh>

#include "game/ui/ctrl/CountDownTimerControl.hh"
#include "game/ui/page/Page.hh"

namespace UI {

// This page is used for the countdown timer after races in public rooms when the player
// has to decide to play the next race or quit.
class WifiRaceEndCountdownPage : public Page {
public:
    void stopTimer();

private:
    u8 _044[0x054 - 0x044];
    CountDownTimerControl m_timerControl;
    CountDownTimer m_countdownTimer;
    u8 _1dc[0x1e0 - 0x1dc];
};
static_assert(sizeof(WifiRaceEndCountdownPage) == 0x1e0);

} // namespace UI
