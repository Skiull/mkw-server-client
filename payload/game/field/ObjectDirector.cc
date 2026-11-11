#include "ObjectDirector.hh"

#include <sp/TimeTrialCheats.hh>

namespace Field {

// Set when a race's objects are built: Grumble Volcano's road never falls (TANoFallingRoad).
bool g_noFallingRoad = false;

void ObjectDirector::createObjects() {
    g_noFallingRoad = SP::NoFallingRoadAllowed();
    if (g_noFallingRoad) {
        SP::MarkPracticeUsed();
    }
    REPLACED(createObjects)();
}

// Grumble Volcano's shaking and falling road pieces (VolcanoPiece, Kinoko's ObjectVolcanoPiece).
// Their state comes from the race frame: rest, shake, quake, fall, gone. With the setting on,
// they always see frame 0, so they stay at rest.
class ObjectVolcanoPiece {
private:
    // Moves the piece every frame (shake, then fall). Skipped while the setting is on, so the
    // piece stays where it was built.
    REPLACE void calc();
    void REPLACED(calc)();

    REPLACE u32 calcState(u32 frame) const;
    u32 REPLACED(calcState)(u32 frame) const;
    REPLACE f32 calcT(u32 frame) const;
    f32 REPLACED(calcT)(u32 frame) const;
};

void ObjectVolcanoPiece::calc() {
    if (g_noFallingRoad) {
        return;
    }
    REPLACED(calc)();
}

u32 ObjectVolcanoPiece::calcState(u32 frame) const {
    return REPLACED(calcState)(g_noFallingRoad ? 0 : frame);
}

f32 ObjectVolcanoPiece::calcT(u32 frame) const {
    return REPLACED(calcT)(g_noFallingRoad ? 0 : frame);
}

} // namespace Field
