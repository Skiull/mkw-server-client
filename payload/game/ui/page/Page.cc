// See LICENSE-mkw-sp

#include "Page.hh"

#include "game/system/SaveManager.hh"

#include <sp/net/mkw_server/MKWServer.hh>
#include <sp/net/mkw_server/packets/LoungeRanks.hh>

namespace UI {

Page::~Page() = default;

void Page::dt(s32 type) {
    if (type > 0) {
        delete this;
    } else {
        this->~Page();
    }
}

Page::State Page::state() const {
    return m_state;
}

Page::Anim Page::anim() const {
    return m_anim;
}

const MenuInputManager *Page::inputManager() const {
    return m_inputManager;
}

MenuInputManager *Page::inputManager() {
    return m_inputManager;
}

// CharacterSelectPage::m_wifiCountDownTimer (mkw-sp CharacterSelectPage.hh): the time left to
// pick a character, vehicle and drift type in the online menus. Its value (seconds, f32) is
// first; at 0 the game picks for the player and carries on.
static constexpr u32 CHARACTER_SELECT_COUNTDOWN = 0x90c;

// Watching a replay: run the selection countdown out at once, so the game picks by itself
// and carries on to the race being watched.
static void tryEndSelectionCountdown(Page *page) {
    if (!MKWServer::s_skipCharacterSelect || page->id() != PageId::CharacterSelect ||
            page->state() != Page::State::State4) {
        return;
    }
    auto *timer = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(page) + CHARACTER_SELECT_COUNTDOWN);
    if (timer == nullptr || !timer[4] /* running */) {
        return;
    }
    *reinterpret_cast<f32 *>(timer) = 0.0f;
    MKWServer::s_skipCharacterSelect = false;
    SP_LOG("Replay: ended the selection countdown");
}

void Page::calc() {
    MKWServer::mogiShowLR();
    tryEndSelectionCountdown(this);
    switch (m_state) {
    case State::State3:
    case State::State4:
    case State::State5:
        beforeCalc();
        m_controlGroup.calc();
        afterCalc();
        calcAnim();
        break;
    default:
        break;
    }
}

} // namespace UI
