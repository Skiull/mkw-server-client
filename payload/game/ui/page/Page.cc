// See LICENSE-mkw-sp

#include "Page.hh"

#include "game/system/SaveManager.hh"
#include "game/ui/ctrl/Button.hh"

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

// MenuPage's buttons (mkw-sp MenuPage.hh).
static constexpr u32 MENU_PAGE_BUTTONS = 0x48;
static constexpr u32 MENU_PAGE_BUTTON_COUNT = 0x3d8;

// Frames to wait on each selection page before confirming (lets it set up its cursor).
static constexpr s32 AUTO_CONFIRM_FRAME = 10;

static const Page *s_autoConfirmed = nullptr;

// Watching a replay: the game makes a player joining a room pick a character, vehicle and
// drift type. Confirm each page's highlighted button (the last pick) as if A were pressed,
// so the pages go by on their own and the online flow carries on to the race.
static void tryAutoConfirmSelection(Page *page) {
    PageId id = page->id();
    bool selectionPage = id == PageId::CharacterSelect || id == PageId::VehicleSelect ||
            id == PageId::DriftSelect;
    if (!selectionPage || page->state() != Page::State::State4) {
        if (s_autoConfirmed == page) {
            s_autoConfirmed = nullptr;
        }
        return;
    }
    if (!MKWServer::s_skipCharacterSelect || s_autoConfirmed == page ||
            page->frame() < AUTO_CONFIRM_FRAME) {
        return;
    }
    auto *bytes = reinterpret_cast<u8 *>(page);
    auto **buttons = *reinterpret_cast<PushButton ***>(bytes + MENU_PAGE_BUTTONS);
    u32 count = *reinterpret_cast<u32 *>(bytes + MENU_PAGE_BUTTON_COUNT);
    if (buttons == nullptr || count == 0 || count > 64) {
        return;
    }
    PushButton *button = nullptr;
    for (u32 i = 0; i < count && button == nullptr; i++) {
        if (buttons[i] != nullptr && buttons[i]->isSelected()) {
            button = buttons[i];
        }
    }
    if (button == nullptr) {
        button = buttons[0];
    }
    if (button == nullptr) {
        return;
    }
    s_autoConfirmed = page;
    SP_LOG("Replay: confirming page %d, button %d", static_cast<s32>(id), button->m_index);
    if (id == PageId::DriftSelect) {
        MKWServer::s_skipCharacterSelect = false;
    }
    button->onFront(0, 0);
}

void Page::calc() {
    tryAutoConfirmSelection(this);
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
