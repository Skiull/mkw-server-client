#pragma once

#include <Common.hh>

#include "game/ui/UIControl.hh"

namespace UI {

struct CountDownTimer {
    u8 _0[0x4 - 0x0];
    bool isRunning;
    u8 _5[0xc - 0x5];
};
static_assert(sizeof(CountDownTimer) == 0xc);

class CountDownTimerControl : public LayoutUIControl {
public:
    void reset();

private:
    u8 _174[0x17c - 0x174];
};
static_assert(sizeof(CountDownTimerControl) == 0x17c);

} // namespace UI
