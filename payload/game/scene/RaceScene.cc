#include "RaceScene.hh"

#include <sp/SaveStateManager.hh>
#include <sp/net/mkw_server/race_verification/RaceManager.hh>

namespace Scene {

void RaceScene::destroySubsystems() {
    REPLACED(destroySubsystems)();
    SP::SaveStateManager::DestroyInstance();
    MKWServer::RaceManager::DestroyInstance();
}

void RaceScene::createSubsystems() {
    REPLACED(createSubsystems)();
    SP::SaveStateManager::CreateInstance();
    MKWServer::RaceManager::CreateInstance();
}

} // namespace Scene
