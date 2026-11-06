#pragma once

#include <Common.hh>

#include <vendor/magic_enum/magic_enum.hpp>

namespace Settings {

// Don't assign values when adding new categories to avoid conflicts and diff noise.
enum class Category {
    Online,
    Ranks,
    Menus,
};

enum class Setting {
    // Online
    Ping,
    // Ranks
    LoungeRank,
    // Menus
    FastMenuTransitions,
    RememberCombo,
    InstantVoting,
};

enum class Ping {
    Disabled,
    Enabled,
};

// Show my MKW Lounge rank (instead of the star rank) on my name tag in online races.
enum class LoungeRank {
    Disabled,
    Enabled,
};

// Skip the wait before the game changes to another section of menus.
enum class FastMenuTransitions {
    Disabled,
    Enabled,
};

// Start character, vehicle and drift select on the player's last combo.
enum class RememberCombo {
    Disabled,
    Enabled,
};

// Skip the course vote: go straight to the race once the host picks the course.
enum class InstantVoting {
    Disabled,
    Enabled,
};

template <Setting S>
struct OptionType;

template <>
struct OptionType<Setting::InstantVoting> {
    using Type = InstantVoting;
};

template <>
struct OptionType<Setting::RememberCombo> {
    using Type = RememberCombo;
};

template <>
struct OptionType<Setting::FastMenuTransitions> {
    using Type = FastMenuTransitions;
};

template <>
struct OptionType<Setting::LoungeRank> {
    using Type = LoungeRank;
};

template <>
struct OptionType<Setting::Ping> {
    using Type = Ping;
};

// For each new setting, make a new OptionType for the new setting
template <Setting S>
using Option = typename OptionType<S>::Type;

constexpr u32 settingCount = magic_enum::enum_count<Setting>();

constexpr u32 categoryMessageIds[]{
        // clang-format off
        10002,
        10030,
        10035,
        // clang-format on
};

struct SettingEntry {
    const Category category;
    const std::string_view name;
    const s32 messageId;
    const u32 valueOffset;
    const u32 defaultValue;
    const u32 valueCount;
    const std::string_view *valueNames;
    const s32 *valueMessageIds;
    const s32 *valueExplanationMessageIds;
    const bool hidden = false;
    u32 selectedValue;
};

// inline to prevent multiple definition errors at link time
inline SettingEntry settings[]{
        // clang-format off
        [static_cast<u32>(Setting::Ping)] =
        {
            .category = Category::Online,
            .name = magic_enum::enum_name(Setting::Ping),
            .messageId = 10003,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(Ping::Disabled),
            .valueCount = magic_enum::enum_count<Ping>(),
            .valueNames = magic_enum::enum_names<Ping>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10006, 10007},
            .hidden = false,
            .selectedValue = static_cast<u32>(Ping::Disabled),
        },
        [static_cast<u32>(Setting::LoungeRank)] =
        {
            .category = Category::Ranks,
            .name = magic_enum::enum_name(Setting::LoungeRank),
            .messageId = 10031,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(LoungeRank::Enabled),
            .valueCount = magic_enum::enum_count<LoungeRank>(),
            .valueNames = magic_enum::enum_names<LoungeRank>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10032, 10033},
            .hidden = false,
            .selectedValue = static_cast<u32>(LoungeRank::Enabled),
        },
        [static_cast<u32>(Setting::FastMenuTransitions)] =
        {
            .category = Category::Menus,
            .name = magic_enum::enum_name(Setting::FastMenuTransitions),
            .messageId = 10036,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(FastMenuTransitions::Enabled),
            .valueCount = magic_enum::enum_count<FastMenuTransitions>(),
            .valueNames = magic_enum::enum_names<FastMenuTransitions>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10037, 10038},
            .hidden = false,
            .selectedValue = static_cast<u32>(FastMenuTransitions::Enabled),
        },
        [static_cast<u32>(Setting::RememberCombo)] =
        {
            .category = Category::Menus,
            .name = magic_enum::enum_name(Setting::RememberCombo),
            .messageId = 10039,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(RememberCombo::Enabled),
            .valueCount = magic_enum::enum_count<RememberCombo>(),
            .valueNames = magic_enum::enum_names<RememberCombo>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10040, 10041},
            .hidden = false,
            .selectedValue = static_cast<u32>(RememberCombo::Enabled),
        },
        [static_cast<u32>(Setting::InstantVoting)] =
        {
            .category = Category::Menus,
            .name = magic_enum::enum_name(Setting::InstantVoting),
            .messageId = 10042,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(InstantVoting::Enabled),
            .valueCount = magic_enum::enum_count<InstantVoting>(),
            .valueNames = magic_enum::enum_names<InstantVoting>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10043, 10044},
            .hidden = false,
            .selectedValue = static_cast<u32>(InstantVoting::Enabled),
        },
        // clang-format on
};

template <Setting S>
inline void setSetting(Option<S> option) {
    settings[static_cast<size_t>(S)].selectedValue = static_cast<u32>(option);
}

template <Setting S>
inline Option<S> getSetting() {
    return static_cast<Option<S>>(settings[static_cast<size_t>(S)].selectedValue);
}

inline void setSetting(u32 setting, u32 option) {
    settings[setting].selectedValue = option;
}

inline u32 getSetting(u32 index) {
    return settings[index].selectedValue;
}

} // namespace Settings
