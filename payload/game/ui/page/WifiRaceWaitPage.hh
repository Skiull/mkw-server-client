#pragma once

#include <Common.hh>

#include "game/ui/page/Page.hh"

namespace UI {

// This page contains the "Please wait..." text after race results before transitioning to voting.
class WifiRaceWaitPage : public Page {
public:
    REPLACE void afterCalc() override;
    void REPLACED(afterCalc)();

private:
    u8 _044[0x1cc - 0x044];
};
static_assert(sizeof(WifiRaceWaitPage) == 0x1cc);

} // namespace UI
