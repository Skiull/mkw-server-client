#include "ObjectDirector.hh"

#include <sp/TimeTrialCheats.hh>

// Set when a race's objects are built: Grumble Volcano's road never falls (TANoFallingRoad).
// Also read by the patches in VolcanoPiece.S.
extern "C" {
u8 Field_noFallingRoad = 0;
}

namespace Field {

static bool &g_noFallingRoad = reinterpret_cast<bool &>(Field_noFallingRoad);



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
    // Move and shrink the collision (called from the collision checks). Skipped too.
    REPLACE void update(u32 timeOffset);
    void REPLACED(update)(u32 timeOffset);
    REPLACE void calcScale(u32 timeOffset);
    void REPLACED(calcScale)(u32 timeOffset);

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

void ObjectVolcanoPiece::update(u32 timeOffset) {
    if (g_noFallingRoad) {
        return;
    }
    REPLACED(update)(timeOffset);
}

void ObjectVolcanoPiece::calcScale(u32 timeOffset) {
    if (g_noFallingRoad) {
        return;
    }
    REPLACED(calcScale)(timeOffset);
}

u32 ObjectVolcanoPiece::calcState(u32 frame) const {
    return REPLACED(calcState)(g_noFallingRoad ? 0 : frame);
}

f32 ObjectVolcanoPiece::calcT(u32 frame) const {
    return REPLACED(calcT)(g_noFallingRoad ? 0 : frame);
}

} // namespace Field
