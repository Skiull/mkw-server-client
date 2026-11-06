#include "FavoriteCombo.hh"

#include "game/ui/SectionManager.hh"
#include "game/ui/page/PageId.hh"

#include <sp/settings/Settings.hh>

// Remember Combo: character, vehicle and drift select start on the player's last picks (like
// CTGP and Retro Rewind). The picks live in the menu context (mkw-sp's GlobalContext), which
// the game resets between some sections; we keep a copy and put it back while the pickers
// aren't open, so they open on it.
namespace SP::FavoriteCombo {

namespace {

// GlobalContext offsets (mkw-sp GlobalContext.hh).
constexpr u32 VEHICLE_RESTRICTION = 0x74; // 0: karts only, 1: bikes only, 2: all
constexpr u32 CHARACTERS = 0x12c;          // u32[4]
constexpr u32 VEHICLES = 0x13c;            // u32[4]
constexpr u32 DRIFT_MODES = 0x164;         // u32[4]

// Vehicle ids 0-17 are karts, 18-35 bikes. Character ids go up to the Miis (0x2f).
constexpr u32 FIRST_BIKE = 18;
constexpr u32 VEHICLE_COUNT = 36;
constexpr u32 CHARACTER_COUNT = 0x30;

struct Combo {
    bool has = false;
    u32 character, vehicle, drift;
};
Combo s_combos[4];

u32 &field(u8 *context, u32 offset, u32 player) {
    return reinterpret_cast<u32 *>(context + offset)[player];
}

bool vehicleAllowed(u8 *context, u32 vehicle) {
    switch (*reinterpret_cast<u32 *>(context + VEHICLE_RESTRICTION)) {
    case 0:
        return vehicle < FIRST_BIKE;
    case 1:
        return vehicle >= FIRST_BIKE && vehicle < VEHICLE_COUNT;
    default:
        return vehicle < VEHICLE_COUNT;
    }
}

} // namespace

void Calc() {
    if (Settings::getSetting<Settings::Setting::RememberCombo>() !=
            Settings::RememberCombo::Enabled) {
        return;
    }
    auto *sectionManager = UI::SectionManager::Instance();
    if (!sectionManager || !sectionManager->currentSection()) {
        return;
    }
    auto *section = sectionManager->currentSection();
    auto *context = reinterpret_cast<u8 *>(sectionManager->sectionParams());
    if (!context || !section->page(UI::PageId::CharacterSelect)) {
        return; // Not a section with the pickers.
    }

    bool picking = section->isPageActive(UI::PageId::CharacterSelect) ||
            section->isPageActive(UI::PageId::VehicleSelect) ||
            section->isPageActive(UI::PageId::DriftSelect);
    for (u32 i = 0; i < 4; i++) {
        auto &combo = s_combos[i];
        u32 &character = field(context, CHARACTERS, i);
        u32 &vehicle = field(context, VEHICLES, i);
        u32 &drift = field(context, DRIFT_MODES, i);
        if (picking) {
            // The player's picks, as the pickers write them.
            if (character < CHARACTER_COUNT && vehicle < VEHICLE_COUNT && drift < 2) {
                combo = {true, character, vehicle, drift};
            }
        } else if (combo.has) {
            character = combo.character;
            if (vehicleAllowed(context, combo.vehicle)) {
                vehicle = combo.vehicle;
            }
            drift = combo.drift;
        }
    }
}

} // namespace SP::FavoriteCombo
