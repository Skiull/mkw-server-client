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
        // Same as the game's own "saving disabled" path: not busy, result OK. Returning
        // without this left an old result behind, which the game reported as a corrupt save.
        m_isBusy = false;
        m_result = 0;
        return;
    }
    REPLACED(SaveGhostAsync)(licenseId, type, slot, ghostFile, r8);
}

// Every character, vehicle, cup (mirror included) and other unlockable is available on every
// license, new or existing. The game reads a license's unlock flags through here when it loads
// the license, so nothing is written to the save itself.
void SaveManager::RawSave::ReadUnlockFlags(UnlockFlags *flags, u32 licenseId) {
    REPLACED(ReadUnlockFlags)(flags, licenseId);
    flags->flag0 = 0xffffffff;
    flags->flag1 = 0xffffffff;
    flags->flag2 = 0xffffffff;
}

} // namespace System
