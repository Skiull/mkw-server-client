#include "RaceScene.hh"

#include "game/item/PowManager.hh"

#include <sp/SaveStateManager.hh>
#include <sp/TimeTrialCheats.hh>

namespace Scene {

void RaceScene::destroySubsystems() {
    SP::RestoreLicenseIfPracticeUsed();
    REPLACED(destroySubsystems)();
    Item::PowManager::ClearInstance();
    SP::SaveStateManager::DestroyInstance();
}

void RaceScene::createSubsystems() {
    // A new run: ghost saving is allowed until a practice tool is used. Item boxes are placed
    // while the subsystems are created, so their flag must be set first.
    SP::ResetPracticeUsed();
    SP::SnapshotLicense();
    Item::PowManager::ClearInstance();
    TimeTrialCheats_itemBoxes = SP::ItemBoxesAllowed();
    if (TimeTrialCheats_itemBoxes) {
        SP::MarkPracticeUsed();
    }

    REPLACED(createSubsystems)();
    // Only offline Time Trials get save states (the /store and /reload commands need it too).
    if (SP::SaveStatesAllowed()) {
        SP::SaveStateManager::CreateInstance();
    }
}

} // namespace Scene
