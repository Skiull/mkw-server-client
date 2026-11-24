#include "SaveGhostManager.hh"

#include <sp/TimeTrialCheats.hh>

extern "C" {
#include <revolution.h>
}

namespace System {

// After a run that used a practice tool, nothing from it is written to the save: the license
// save (best times) and ghost save requests are dropped. The license itself is put back to how
// it was before the race when the race ends (Scene::RaceScene::destroySubsystems). Only the
// run's own saves are dropped: a few seconds after the race, license saves work again.
void SaveGhostManager::calc() {
    if (SP::DropRunSaves() && (m_requests[0] || m_requests[1])) {
        SP_LOG("SaveGhostManager: practice tool used, dropping the license/ghost save");
        m_requests[0] = false;
        m_requests[1] = false;
    }
    REPLACED(calc)();
}

} // namespace System
