#include "KartMove.hh"

#include "game/kart/KartObjectManager.hh"
#include "game/system/RaceConfig.hh"
#include "game/system/SaveManager.hh"

#include <sp/TimeTrialCheats.hh>

namespace Kart {

f32 KartMove::hardSpeedLimit() const {
    return m_hardSpeedLimit;
}

const Vec3 *KartMove::internalVelDir() const {
    return &m_internalVelDir;
}

// Longer Thunder Clouds (TALongerTCs), from the henrymkw/mkw-sp fork: offline Time Trials only.
bool KartMove::applyLightningEffect(s32 timer, s32 r5, s32 r6) {
    if (SP::LongerTCsAllowed()) {
        SP::MarkPracticeUsed();
        // About 54 times the normal duration.
        timer = 0x7fff;
    }
    return REPLACED(applyLightningEffect)(timer, r5, r6);
}

} // namespace Kart
