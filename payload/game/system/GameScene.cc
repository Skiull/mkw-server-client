#include "GameScene.hh"

#include "sp/FastMenus.hh"
#include "sp/TestCheats.hh"
#include "sp/net/mkw_server/GameHash.hh"

u32 g_frameCount = 0;

namespace System {

void GameScene::calc() {
    g_frameCount++;
    SP::TestCheats::RunFrame();
    MKWServer::GameHash::Calc();
    SP::FastMenus::Calc();
    REPLACED(calc)();
}

} // namespace System