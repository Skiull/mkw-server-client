#include "GameScene.hh"

#include "game/ui/SectionManager.hh"
#include "sp/FavoriteCombo.hh"
#include "sp/TestCheats.hh"
#include "sp/settings/Settings.hh"
#include "sp/net/mkw_server/GameHash.hh"

u32 g_frameCount = 0;

namespace System {

void GameScene::calc() {
    g_frameCount++;
    SP::TestCheats::RunFrame();
    MKWServer::GameHash::Calc();
    SP::FavoriteCombo::Calc();
    // Fast Menu Transitions: start a requested section change on the next frame. Done on the
    // section manager's timer rather than by patching code, which the game hash would flag.
    if (Settings::getSetting<Settings::Setting::FastMenuTransitions>() ==
            Settings::FastMenuTransitions::Enabled) {
        if (auto *sectionManager = UI::SectionManager::Instance()) {
            if (sectionManager->changeTimer() > 1) {
                sectionManager->changeTimer() = 1;
            }
        }
    }
    REPLACED(calc)();
}

} // namespace System