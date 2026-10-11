#pragma once

#include <Common.hh>

#include "game/util/Registry.hh"

extern "C" {
#include <revolution.h>
}

namespace Net {

struct UserRecord {
    RFLWifiData wifiData;
    u8 _a0[0xb8 - 0xa0];
    u16 vr;
    u16 br;
    u8 _bc[0xbd - 0xbc];
    Registry::RegionLine regionLine : 8;
    u8 _be[0xc0 - 0xbe];
};
static_assert(sizeof(UserRecord) == 0xc0);

class UserHandler {
public:
    // 0x8066300c
    // Creates the outgoing user record.
    REPLACE void createUser() const;
    void REPLACED(createUser)() const;

    // Will create a record incase it doesn't exist.
    const UserRecord *outgoingRecord() const;

    bool prepared();

    // Returns our vr or br depending on the current game mode.
    u16 getRating() const;

    u16 myVR() const;

    u16 myBR() const;

    static UserHandler *Instance() {
        return s_instance;
    }

private:
    bool m_prepared;
    u8 _001[0x008 - 0x001];
    UserRecord m_recordToSend;
    u8 _0c8[0x9f0 - 0x0c8];

    static UserHandler *s_instance;
};
static_assert(sizeof(UserHandler) == 0x9f0);

} // namespace Net
