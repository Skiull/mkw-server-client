#pragma once

#include <Common.hh>

#include "game/ui/page/Page.hh"

namespace UI {

// This page is for the yes/no menu after pressing the "Quit" button in a public room.
class WifiRaceEndQuitPage : public Page {
private:
    REPLACE void afterCalc() override;
    void REPLACED(afterCalc)();

    u8 _044[0x8b0 - 0x044];
    PageId m_replacement;
    bool m_isTransitioning;
    u8 _8b5[0x8bc - 0x8b5];
};
static_assert(sizeof(WifiRaceEndQuitPage) == 0x8bc);

} // namespace UI
