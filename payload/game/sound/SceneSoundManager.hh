// See LICENSE-mkw-sp

#pragma once

#include "game/sound/SoundId.hh"

#include <Common.hh>

namespace Sound {

class SceneSoundManager {
public:
    SceneSoundManager();
    virtual ~SceneSoundManager();
    virtual void vf_08();
    virtual void vf_0c();
    virtual void vf_10();
    virtual void vf_14();
    virtual void vf_18();
    virtual void vf_1c() = 0;
    virtual bool play(SoundId soundId, s32 localPlayerId);
    virtual void vf_24();
    virtual void vf_28();
    virtual void vf_2c();

    static SceneSoundManager *Instance() {
        return s_instance;
    }

private:
    u8 _00[0x18 - 0x00];

    static SceneSoundManager *s_instance;
};

} // namespace Sound
