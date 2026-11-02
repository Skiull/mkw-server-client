// See LICENSE-mkw-sp

#include "Page.hh"

#include "game/system/SaveManager.hh"
#include "game/ui/SectionManager.hh"

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

// GlobalContext (SectionParams) fields, from mkw-sp GlobalContext.hh.
static constexpr u32 GC_LOCAL_CHARACTER_IDS = 0x12c; // Last picks, per local player.
static constexpr u32 GC_LOCAL_VEHICLE_IDS = 0x13c;
static constexpr u32 GC_DRIFT_MODES = 0x164;
static constexpr u32 GC_SELECT_PLAYER = 0x220; // {character, vehicle, u8} x 2, 0xc each.

static constexpr u32 CHARACTER_COUNT = 0x30;
static constexpr u32 VEHICLE_COUNT = 0x24;

// Watching a replay: the game makes a player joining a room pick a character, vehicle and
// drift type. Fill those in with the last picks (Mario and a Standard Kart M if there are
// none) and replace the character select with the room search page, where drift select
// hands over in the online menus.
static bool trySkipSelection(Page *page) {
    if (!MKWServer::s_skipCharacterSelect || page->id() != PageId::CharacterSelect ||
            page->state() != Page::State::State4) {
        return false;
    }
    MKWServer::s_skipCharacterSelect = false;
    auto *context = reinterpret_cast<u8 *>(UI::SectionManager::Instance()->sectionParams());
    if (context == nullptr) {
        return false;
    }
    u32 character = *reinterpret_cast<u32 *>(context + GC_LOCAL_CHARACTER_IDS);
    u32 vehicle = *reinterpret_cast<u32 *>(context + GC_LOCAL_VEHICLE_IDS);
    if (character >= CHARACTER_COUNT) {
        character = 0; // Mario
    }
    if (vehicle >= VEHICLE_COUNT) {
        vehicle = 1; // Standard Kart M
    }
    *reinterpret_cast<u32 *>(context + GC_SELECT_PLAYER) = character;
    *reinterpret_cast<u32 *>(context + GC_SELECT_PLAYER + 4) = vehicle;
    u32 &drift = *reinterpret_cast<u32 *>(context + GC_DRIFT_MODES);
    if (drift != 1 && drift != 2) {
        drift = 1;
    }
    SP_LOG("Replay: skipping character select (character %u, vehicle %u)", character, vehicle);
    *reinterpret_cast<PageId *>(reinterpret_cast<u8 *>(page) + MENU_PAGE_REPLACEMENT) =
            PageId::WifiRandomMatching;
    return true;
}

void Page::calc() {
    if (trySkipSelection(this)) {
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
