#include "InputManager.hh"

extern "C" {
#include <revolution.h>
}

namespace System {

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
