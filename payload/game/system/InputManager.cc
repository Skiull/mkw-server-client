#include "InputManager.hh"

extern "C" {
#include <revolution.h>
}

namespace System {

bool RaceInputState::isInputValid() const {
    if (trick > Trick::Right) {
        return false;
    }

    // Stick values range from 0-14, anything else is invalid.
    if (rawStick.x > 14 || rawStick.y > 14) {
        return false;
    }

    return true;
}

s16 RaceInputState::inputBitfield() const {
    if (!isInputValid()) {
        return -1;
    }

    s16 inputBitfield = 0;

    if (trick != Trick::Off) {
        // The value of trick the same as the amount to shift by to set the bitfield.
        inputBitfield |= 1 << trick;
    }

    if (accelerate) {
        inputBitfield |= RaceInput::Accel;
    }

    if (drift || brake) {
        inputBitfield |= RaceInput::DriftBrake;
    }

    if (item) {
        inputBitfield |= RaceInput::Item;
    }

    return inputBitfield;
}

const RaceInputState &PadProxy::currentRaceInputState() const {
    return m_currentRaceInputState;
}

const Pad *PadProxy::pad() const {
    return m_pad;
}

PlayerPadProxy *InputManager::playerProxy(u32 i) {
    assert(i <= 3);

    return &m_playerProxies[i];
}

} // namespace System
