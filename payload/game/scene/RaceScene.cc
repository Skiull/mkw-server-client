#include "RaceScene.hh"

#include <sp/SaveStateManager.hh>
#include <sp/TimeTrialCheats.hh>

namespace Scene {

void RaceScene::destroySubsystems() {
    REPLACED(destroySubsystems)();
    SP::SaveStateManager::DestroyInstance();
}

void RaceScene::createSubsystems() {
    REPLACED(createSubsystems)();
    // Only offline Time Trials get save states (the /store and /reload commands need it too).
    if (SP::SaveStatesAllowed()) {
        SP::SaveStateManager::CreateInstance();
    }
}

} // namespace Scene
