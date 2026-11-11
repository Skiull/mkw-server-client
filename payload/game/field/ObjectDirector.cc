#include "ObjectDirector.hh"

#include <sp/TimeTrialCheats.hh>

namespace System {

// Just enough of the course map to reach the KMP GOBJ entries (layout from mkw-sp / Kinoko).
struct RawGeoObj {
    u16 id;
    u8 _02[0x28 - 0x02];
    u16 settings[8];
    u16 presenceFlag; // Bit 0: present in single player races
    u8 _3a[0x3c - 0x3a];
};
static_assert(sizeof(RawGeoObj) == 0x3c);

struct MapdataGeoObj {
    RawGeoObj *m_data;
};

struct MapdataGeoObjAccessor {
    MapdataGeoObj **m_entryAccessors;
    u16 m_numEntries;
};

class CourseMap {
public:
    static CourseMap *Instance() {
        return s_instance;
    }

    MapdataGeoObjAccessor *geoObj() const {
        return m_geoObj;
    }

private:
    void *m_vtable;
    u8 _04[0x28 - 0x04];
    MapdataGeoObjAccessor *m_geoObj;
    u8 _2c[0x54 - 0x2c];

    static CourseMap *s_instance;
};
static_assert(sizeof(CourseMap) == 0x54);

} // namespace System

namespace Field {

// Grumble Volcano's road pieces that shake and fall (VolcanoPiece). settings[1] is how many
// seconds a piece rests before it starts shaking, counted from the race start (see Kinoko's
// ObjectVolcanoPiece); it never repeats.
static constexpr u16 VolcanoPieceId = 0x1fc;
static constexpr u16 NeverFallSeconds = 0x7fff; // About 9 hours

// No falling road (TANoFallingRoad), offline Time Trials only: the pieces read their timing
// when they are built, so give them a very long rest just for that, then put the KMP data back
// (the course file stays cached between races).
void ObjectDirector::createObjects() {
    auto *courseMap = System::CourseMap::Instance();
    auto *geoObj = courseMap ? courseMap->geoObj() : nullptr;
    if (!geoObj || !SP::NoFallingRoadAllowed()) {
        REPLACED(createObjects)();
        return;
    }

    constexpr u32 MaxChanged = 64;
    System::RawGeoObj *changed[MaxChanged];
    u16 restSeconds[MaxChanged];
    u32 changedCount = 0;
    for (u32 i = 0; i < geoObj->m_numEntries && changedCount < MaxChanged; i++) {
        System::RawGeoObj *data = geoObj->m_entryAccessors[i]->m_data;
        if (data->id != VolcanoPieceId) {
            continue;
        }
        changed[changedCount] = data;
        restSeconds[changedCount] = data->settings[1];
        changedCount++;
        data->settings[1] = NeverFallSeconds;
    }
    if (changedCount > 0) {
        SP::MarkPracticeUsed();
    }

    REPLACED(createObjects)();

    for (u32 i = 0; i < changedCount; i++) {
        changed[i]->settings[1] = restSeconds[i];
    }
}

} // namespace Field
