#include "GameScene.hh"

#include "sp/FastMenus.hh"
#include "sp/TestCheats.hh"
#include "sp/net/mkw_server/GameHash.hh"
#include "sp/settings/Settings.hh"

u32 g_frameCount = 0;

namespace System {

void GameScene::calc() {
    g_frameCount++;
    SP::TestCheats::RunFrame();
    MKWServer::GameHash::Calc();
    SP::FastMenus::Calc();
    REPLACED(calc)();
}

// FPS Mode setting (from mkw-sp).
void GameScene::setFramerate(bool is30FPS) {
    switch (Settings::getSetting<Settings::Setting::FPSMode>()) {
    case Settings::FPSMode::Force60:
        return REPLACED(setFramerate)(false);
    case Settings::FPSMode::Force30:
        return REPLACED(setFramerate)(true);
    default:
        return REPLACED(setFramerate)(is30FPS);
    }
}

} // namespace System
