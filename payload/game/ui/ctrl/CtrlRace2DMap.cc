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
