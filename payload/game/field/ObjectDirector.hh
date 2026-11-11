#pragma once

#include <Common.hh>

namespace Field {

class ObjectDirector {
private:
    // Builds the course objects from the KMP GOBJ section (PAL 0x80826E8C, see Kinoko).
    REPLACE void createObjects();
    void REPLACED(createObjects)();
};

} // namespace Field
