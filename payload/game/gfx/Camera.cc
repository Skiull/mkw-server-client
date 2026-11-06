// See LICENSE-mkw-sp

#include <sp/settings/Settings.hh>

// Called from Camera.S.
extern "C" u32 Settings_GetFOV169() {
    return static_cast<u32>(Settings::getSetting<Settings::Setting::FOV169>());
}
