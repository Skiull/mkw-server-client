#include "FavoriteCombo.hh"

#include "game/system/SaveManager.hh"

#include <sp/settings/Settings.hh>

// Favorite Combo (hooks in sp/FavoriteCombo.S): the most-played character and vehicle of the
// first license in the save (rksys.dat layout: https://wiki.tockdom.com/wiki/Rksys.dat),
// as in Bully, CLF78 and Kazuki's code.
namespace SP::FavoriteCombo {

namespace {

constexpr u32 LICENSE_SIZE = 0x8CC0;
constexpr u32 RKPD = 0x524B5044;
constexpr u32 CHARACTER_COUNTS = 0xF4; // u16 per character (24, no Miis), from the save start
constexpr u32 VEHICLE_COUNTS = CHARACTER_COUNTS + 0x32; // u16 per vehicle, 3 per row
constexpr u32 CHARACTER_COUNT = 24;
constexpr u32 VEHICLES_PER_WEIGHT = 12;

// Weight class (0 light, 1 medium, 2 heavy) of each character, by id.
constexpr u8 WEIGHTS[CHARACTER_COUNT] = {1, 0, 2, 2, 0, 0, 0, 1, 0, 2, 1, 2, 0, 0, 0, 1, 1, 1,
        1, 2, 1, 2, 2, 2};

bool enabled() {
    return Settings::getSetting<Settings::Setting::RememberCombo>() ==
            Settings::RememberCombo::Enabled;
}

// The first license in the save, or nullptr.
const u8 *license() {
    auto *save = System::SaveManager::Instance();
    if (!save) {
        return nullptr;
    }
    auto *rksys = *reinterpret_cast<const u8 *const *>(reinterpret_cast<const u8 *>(save) + 0x14);
    if (!rksys) {
        return nullptr;
    }
    for (u32 i = 0; i < 4; i++) {
        const u8 *l = rksys + 8 + i * LICENSE_SIZE;
        if (*reinterpret_cast<const u32 *>(l) == RKPD) {
            return rksys + i * LICENSE_SIZE;
        }
    }
    return nullptr;
}

u16 count(const u8 *base, u32 offset, u32 index) {
    return reinterpret_cast<const u16 *>(base + offset)[index];
}

// Most-played character id, or -1 if none (or the save has no plays).
s32 favoriteCharacter(const u8 *l) {
    s32 best = -1;
    u16 bestCount = 0;
    for (u32 c = 0; c < CHARACTER_COUNT; c++) {
        if (count(l, CHARACTER_COUNTS, c) > bestCount) {
            bestCount = count(l, CHARACTER_COUNTS, c);
            best = c;
        }
    }
    return best;
}

} // namespace

} // namespace SP::FavoriteCombo

extern "C" s32 FavoriteCombo_character() {
    using namespace SP::FavoriteCombo;
    const u8 *l = enabled() ? license() : nullptr;
    s32 c = l ? favoriteCharacter(l) : -1;
    return c < 0 ? 0 : c; // Original: Mario
}

extern "C" s32 FavoriteCombo_vehicle(s32 original) {
    using namespace SP::FavoriteCombo;
    const u8 *l = enabled() ? license() : nullptr;
    s32 c = l ? favoriteCharacter(l) : -1;
    if (c < 0) {
        return original;
    }
    // The most-played vehicle of that character's weight class.
    u32 weight = WEIGHTS[c];
    s32 best = -1;
    u16 bestCount = 0;
    for (u32 v = 0; v < VEHICLES_PER_WEIGHT; v++) {
        u16 n = count(l, VEHICLE_COUNTS, v * 3 + weight);
        if (n > bestCount) {
            bestCount = n;
            best = v * 3 + weight;
        }
    }
    return best < 0 ? original : best;
}
