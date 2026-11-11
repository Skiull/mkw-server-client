#include "MapdataKartPoint.hh"

#include <sp/TimeTrialCheats.hh>

namespace System {

// Starting spot practice (TAStartingSpot / TAStartingSpotsLayout, from the henrymkw/mkw-sp fork):
// offline Time Trials start from the chosen spot of a bigger grid. Ghosts start there too.
void MapdataKartPoint::getTransform(Vec3 *pos, Vec3 *rot, u32 rank, u32 playerCount) {
    u32 spot, layout;
    if (SP::StartingSpotChanged(&spot, &layout)) {
        SP::MarkPracticeUsed();
        rank = spot;
        playerCount = layout;
    }
    REPLACED(getTransform)(pos, rot, rank, playerCount);
}

} // namespace System
