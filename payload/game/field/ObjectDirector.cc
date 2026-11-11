#include "ObjectDirector.hh"

#include <sp/TimeTrialCheats.hh>

namespace System {

// Just enough of the course map to reach the KMP GOBJ entries (layout from mkw-sp / Kinoko).
struct RawGeoObj {
    u16 id;
    u8 _02[0x38 - 0x02];
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

// Grumble Volcano's falling rocks (VolcanoBall1, the fireball launchers).
static constexpr u16 VolcanoBallId = 0x0d6;

// No falling rocks (TANoVolcanoRocks), offline Time Trials only: hide the rock launchers from
// the object builder, then put the KMP data back as it was (the course file stays cached).
void ObjectDirector::createObjects() {
    auto *courseMap = System::CourseMap::Instance();
    auto *geoObj = courseMap ? courseMap->geoObj() : nullptr;
    if (!geoObj || !SP::NoVolcanoRocksAllowed()) {
        REPLACED(createObjects)();
        return;
    }

    constexpr u32 MaxHidden = 64;
    System::RawGeoObj *hidden[MaxHidden];
    u16 flags[MaxHidden];
    u32 hiddenCount = 0;
    for (u32 i = 0; i < geoObj->m_numEntries && hiddenCount < MaxHidden; i++) {
        System::RawGeoObj *data = geoObj->m_entryAccessors[i]->m_data;
        if (data->id != VolcanoBallId || data->presenceFlag == 0) {
            continue;
        }
        hidden[hiddenCount] = data;
        flags[hiddenCount] = data->presenceFlag;
        hiddenCount++;
        data->presenceFlag = 0;
    }
    if (hiddenCount > 0) {
        SP::MarkPracticeUsed();
    }

    REPLACED(createObjects)();

    for (u32 i = 0; i < hiddenCount; i++) {
        hidden[i]->presenceFlag = flags[i];
    }
}

} // namespace Field
