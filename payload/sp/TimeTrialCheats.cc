#include "TimeTrialCheats.hh"

#include "game/system/SaveManager.hh"

#include <cstring>

extern "C" {
#include <revolution.h>
}

extern "C" {
u8 TimeTrialCheats_itemBoxes = 0;
}

namespace SP {

static bool s_practiceUsed = false;

void ResetPracticeUsed() {
    if (s_practiceUsed) {
        SP_LOG("TimeTrialCheats: new race, ghost saving allowed again");
    }
    s_practiceUsed = false;
}

void MarkPracticeUsed() {
    if (!s_practiceUsed) {
        SP_LOG("TimeTrialCheats: practice tool used, this run's ghost won't be saved");
    }
    s_practiceUsed = true;
}

bool PracticeUsed() {
    return s_practiceUsed;
}

static u8 s_license[System::SaveManager::LicenseSize];
static u8 *s_licenseSource = nullptr;

void SnapshotLicense() {
    s_licenseSource = nullptr;
    auto *saveManager = System::SaveManager::Instance();
    if (!saveManager || !IsOfflineTimeTrial()) {
        return;
    }
    u8 *license = saveManager->currentLicense();
    if (!license) {
        return;
    }
    memcpy(s_license, license, sizeof(s_license));
    s_licenseSource = license;
}

void RestoreLicenseIfPracticeUsed() {
    if (s_practiceUsed && s_licenseSource) {
        auto *saveManager = System::SaveManager::Instance();
        if (saveManager && saveManager->currentLicense() == s_licenseSource) {
            memcpy(s_licenseSource, s_license, sizeof(s_license));
            SP_LOG("TimeTrialCheats: practice run, license put back as it was before the race");
        }
    }
    s_licenseSource = nullptr;
}

} // namespace SP
