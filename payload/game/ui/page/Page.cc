// See LICENSE-mkw-sp

#include "Page.hh"

#include "game/system/SaveManager.hh"

#include <sp/net/mkw_server/MKWServer.hh>

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

// MenuPage::m_replacement (mkw-sp MenuPage.hh): the page that replaces this one.
static constexpr u32 MENU_PAGE_REPLACEMENT = 0x3e8;

void Page::calc() {
    // Watching a replay: close the character select as drift select does at the end in the
    // online menus (replacement None), so character, vehicle and drift select are skipped and
    // the online flow carries on to the race with the last picks.
    if (m_state == State::State4 && MKWServer::s_skipCharacterSelect &&
            m_id == PageId::CharacterSelect) {
        MKWServer::s_skipCharacterSelect = false;
        SP_LOG("Replay: skipping character select");
        *reinterpret_cast<PageId *>(reinterpret_cast<u8 *>(this) + MENU_PAGE_REPLACEMENT) =
                PageId::None;
        startReplace(Anim::Next, 0.0f);
    }
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
