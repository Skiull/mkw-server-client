#include "RaceScene.hh"

#include "game/system/RaceConfig.hh"

#include <sp/SaveStateManager.hh>
#include <sp/net/mkw_server/race_verification/RaceManager.hh>

extern "C" {
#include <revolution.h>
}

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

    // Race has been initialized so we can send mkw-server the config.
    auto raceScenario = System::RaceConfig::Instance()->raceScenario();
    if (!raceScenario.isOnline()) {
        return;
    }

    if (!MKWServer::RaceManager::Instance()->makeAndSendConfig()) {
        SP_LOG("makeAndSendConfig() failed!");
    }
}

} // namespace Scene
