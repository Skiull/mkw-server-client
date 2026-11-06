// See LICENSE-mkw-sp

#include "CtrlRaceTime.hh"

namespace UI {

void CtrlRaceTime::calcSelf() {
    calcLabelVisibility("set_p");

    REPLACED(calcSelf)();
}

} // namespace UI
