#include "eggSceneManager.hh"

#include "game/system/ResourceManager.hh"

namespace EGG {

void SceneManager::createScene(s32 sceneId, Scene *parent) {
    System::ResourceManager::OnCreateScene(static_cast<System::SceneId>(sceneId));
    REPLACED(createScene)(sceneId, parent);
}

} // namespace EGG
