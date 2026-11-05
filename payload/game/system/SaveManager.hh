#pragma once

#include <Common.hh>

#include "game/system/GhostFile.hh"

namespace System {

class SaveManager {
public:
    struct UnlockFlags {
        u8 _00[0x04 - 0x00];
        u32 flag0;
        u32 flag1;
        u32 flag2;
    };
    static_assert(sizeof(UnlockFlags) == 0x10);

    struct RawLicense {
        u8 _0000[0x0030 - 0x0000];
        u32 unlockFlags[4];
        u8 _0040[0x0080 - 0x0040];
        u16 rules[4];
        u8 _0088[0x8cc0 - 0x0088];
    };
    static_assert(sizeof(RawLicense) == 0x8cc0);

    struct RawSave {
        u8 _00000[0x00008 - 0x00000];
        RawLicense licenses[4];
        u8 _23308[0x28000 - 0x23308];
    };
    static_assert(sizeof(RawSave) == 0x28000);

    static SaveManager *Instance() {
        return s_instance;
    }

    // The current license's VR, which the game copies into the user record it sends
    // (UserHandler, 0x80662d80). Licenses are 0x93f0 bytes from 0x38, the current one's index
    // is at 0x36, and VR is at 0x9018 in it.
    u16 *currentLicenseVR() {
        s16 license = *reinterpret_cast<s16 *>(reinterpret_cast<u8 *>(this) + 0x36);
        if (license < 0 || license >= 4) {
            return nullptr;
        }
        return reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(this) + 0x38 + license * 0x93f0 + 0x9018);
    }

private:
    REPLACE void SaveGhostAsync(u32 /* licenseId */, u32 /* type */, u32 /* slot */,
            GhostFile /* *ghostFile */, u32 /* r8 */);

    u8 _00000[0x00014 - 0x00000];
    RawSave *m_rawSave;
    u8 _00018[0x25008 - 0x00018];

    static SaveManager *s_instance;
};
static_assert(sizeof(SaveManager) == 0x25008);

} // namespace System
