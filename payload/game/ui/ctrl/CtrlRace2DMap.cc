// See LICENSE-mkw-sp

#include "CtrlRace2DMap.hh"

#include <sp/settings/Settings.hh>

namespace UI {

// Minimap setting.
void CtrlRace2DMap::calcSelf() {
    auto setting = Settings::getSetting<Settings::Setting::MiniMap>();
    setVisible(setting == Settings::MiniMap::Enable);
    REPLACED(calcSelf)();
}

} // namespace UI

// Called from CtrlRace2DMap.S.
extern "C" u32 Settings_GetMapIcons() {
    return static_cast<u32>(Settings::getSetting<Settings::Setting::MapIcons>());
}
