#pragma once

#include <Common.hh>

#include "game/ui/ctrl/Button.hh"
#include "game/ui/page/Page.hh"

namespace UI {

// This page is for the menu after a race (only public rooms) to either play the next race or quit.
class WifiRaceEndOptionsPage : public Page {
private:
    void transitionToWaitingPage(f32 delay);

    REPLACE void afterCalc() override;
    void REPLACED(afterCalc)();

    REPLACE void onContinueButtonFront(PushButton *button);
    void REPLACED(onContinueButtonFront)(PushButton *button);

    u8 _044[0xb14 - 0x044];
    PageId m_replacement;
    bool m_isTransitioning;
    u8 _b19[0xb20 - 0xb19];
};
static_assert(sizeof(WifiRaceEndOptionsPage) == 0xb20);

} // namespace UI
