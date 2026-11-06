#pragma once

#include "egg/core/eggScene.hh"

namespace EGG {

class SceneManager {
public:
    void changeSiblingScene(int sceneId);

    void REPLACED(createScene)(s32 sceneId, Scene *parent);
    void REPLACE createScene(s32 sceneId, Scene *parent);

private:
    u8 _00[0x04 - 0x00];

protected:
    void *m_creator;

private:
    u8 _08[0x2c - 0x08];
};
static_assert(sizeof(SceneManager) == 0x2c);

} // namespace EGG
