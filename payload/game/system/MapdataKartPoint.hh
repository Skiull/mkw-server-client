#pragma once

#include <Common.hh>
#include <common/TVec3.hh>

namespace System {

// A KMP start point (KTPT).
class MapdataKartPoint {
public:
    // The start position and rotation of grid spot `rank` (1-based) in a race of
    // `playerCount` players. Time Trials ask for spot 1 of 1.
    REPLACE void getTransform(Vec3 *pos, Vec3 *rot, u32 rank, u32 playerCount);
    void REPLACED(getTransform)(Vec3 *pos, Vec3 *rot, u32 rank, u32 playerCount);
};

} // namespace System
