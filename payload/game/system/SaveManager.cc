#include "SaveManager.hh"

extern "C" {
#include <revolution.h>
}

#include <sp/TimeTrialCheats.hh>

namespace System {

// Like mkw-sp: a run that used the item wheel, save states or another practice tool can't save
// its ghost. Skipping the request leaves the save manager idle, so the menus carry on as if the
// save finished.
void SaveManager::SaveGhostAsync(u32 licenseId, u32 type, u32 slot, GhostFile *ghostFile, u32 r8) {
    if (SP::PracticeUsed()) {
        SP_LOG("SaveManager: not saving the ghost, a practice tool was used in this run");
        return;
    }
    REPLACED(SaveGhostAsync)(licenseId, type, slot, ghostFile, r8);
}

} // namespace System
