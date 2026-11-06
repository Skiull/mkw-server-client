#pragma once

#include <Common.hh>

#include <vendor/magic_enum/magic_enum.hpp>

namespace Settings {

// Don't assign values when adding new categories to avoid conflicts and diff noise.
enum class Category {
    Online,
    Ranks,
    Menus,
    RaceHUD,
    Sound,
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
    // Race HUD
    FOV169,
    FPSMode,
    FarPlayerTags,
    HUDLabels,
    HUDTeamColors,
    MiniMap,
    InputDisplay,
    Speedometer,
    // Sound
    Volume,
    MusicVolume,
    ItemMusic,
    LastLapJingle,
    LastLapSpeedup,
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

// Start character and vehicle select on the license's most-played combo (like CTGP).
enum class RememberCombo {
    Disabled,
    Enabled,
};

// Skip the course vote: go straight to the race once the host picks the course.
enum class InstantVoting {
    Disabled,
    Enabled,
};

// The settings below are ported from mkw-sp (see LICENSE-mkw-sp).

// Field of view in 16:9 mode.
enum class FOV169 {
    FOV169,
    FOV43,
};

enum class FPSMode {
    Vanilla,
    Force60,
    Force30,
};

enum class HUDLabels {
    Hide,
    Show,
};

enum class HUDTeamColors {
    Disable,
    Enable,
};

enum class MiniMap {
    Enable,
    Disable,
};

// Show name tags of racers at any distance (in vanilla only nearby ones).
enum class FarPlayerTags {
    Disable,
    Enable,
};

enum class InputDisplay {
    Disable,
    Simple,
};

enum class Speedometer {
    InternalPlus,
    Internal,
    XYZ,
    XZ,
    Y,
    Off,
};

enum class ItemMusic {
    None,
    DamageOnly,
    All,
};

enum class LastLapJingle {
    GameplayOnly,
    Always,
};

enum class LastLapSpeedup {
    None,
    Static,
    Dynamic,
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

template <>
struct OptionType<Setting::FOV169> {
    using Type = FOV169;
};

template <>
struct OptionType<Setting::FPSMode> {
    using Type = FPSMode;
};

template <>
struct OptionType<Setting::HUDLabels> {
    using Type = HUDLabels;
};

template <>
struct OptionType<Setting::HUDTeamColors> {
    using Type = HUDTeamColors;
};

template <>
struct OptionType<Setting::MiniMap> {
    using Type = MiniMap;
};

template <>
struct OptionType<Setting::InputDisplay> {
    using Type = InputDisplay;
};

template <>
struct OptionType<Setting::Speedometer> {
    using Type = Speedometer;
};

template <>
struct OptionType<Setting::ItemMusic> {
    using Type = ItemMusic;
};

template <>
struct OptionType<Setting::LastLapJingle> {
    using Type = LastLapJingle;
};

template <>
struct OptionType<Setting::LastLapSpeedup> {
    using Type = LastLapSpeedup;
};

template <>
struct OptionType<Setting::FarPlayerTags> {
    using Type = FarPlayerTags;
};

template <>
struct OptionType<Setting::Volume> {
    using Type = u32;
};

template <>
struct OptionType<Setting::MusicVolume> {
    using Type = u32;
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
        10045,
        10046,
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
        [static_cast<u32>(Setting::FOV169)] =
        {
            .category = Category::RaceHUD,
            .name = magic_enum::enum_name(Setting::FOV169),
            .messageId = 10047,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(FOV169::FOV169),
            .valueCount = magic_enum::enum_count<FOV169>(),
            .valueNames = magic_enum::enum_names<FOV169>().data(),
            .valueMessageIds = (const s32[]){10048, 10049},
            .valueExplanationMessageIds = (const s32[]){10050, 10051},
            .hidden = false,
            .selectedValue = static_cast<u32>(FOV169::FOV169),
        },
        [static_cast<u32>(Setting::FPSMode)] =
        {
            .category = Category::RaceHUD,
            .name = magic_enum::enum_name(Setting::FPSMode),
            .messageId = 10052,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(FPSMode::Vanilla),
            .valueCount = magic_enum::enum_count<FPSMode>(),
            .valueNames = magic_enum::enum_names<FPSMode>().data(),
            .valueMessageIds = (const s32[]){10053, 10054, 10055},
            .valueExplanationMessageIds = (const s32[]){10056, 10057, 10058},
            .hidden = false,
            .selectedValue = static_cast<u32>(FPSMode::Vanilla),
        },
        [static_cast<u32>(Setting::FarPlayerTags)] =
        {
            .category = Category::RaceHUD,
            .name = magic_enum::enum_name(Setting::FarPlayerTags),
            .messageId = 10059,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(FarPlayerTags::Enable),
            .valueCount = magic_enum::enum_count<FarPlayerTags>(),
            .valueNames = magic_enum::enum_names<FarPlayerTags>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10062, 10063},
            .hidden = false,
            .selectedValue = static_cast<u32>(FarPlayerTags::Enable),
        },
        [static_cast<u32>(Setting::HUDLabels)] =
        {
            .category = Category::RaceHUD,
            .name = magic_enum::enum_name(Setting::HUDLabels),
            .messageId = 10064,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(HUDLabels::Show),
            .valueCount = magic_enum::enum_count<HUDLabels>(),
            .valueNames = magic_enum::enum_names<HUDLabels>().data(),
            .valueMessageIds = (const s32[]){10065, 10066},
            .valueExplanationMessageIds = (const s32[]){10067, 10068},
            .hidden = false,
            .selectedValue = static_cast<u32>(HUDLabels::Show),
        },
        [static_cast<u32>(Setting::HUDTeamColors)] =
        {
            .category = Category::RaceHUD,
            .name = magic_enum::enum_name(Setting::HUDTeamColors),
            .messageId = 10069,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(HUDTeamColors::Enable),
            .valueCount = magic_enum::enum_count<HUDTeamColors>(),
            .valueNames = magic_enum::enum_names<HUDTeamColors>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10070, 10071},
            .hidden = false,
            .selectedValue = static_cast<u32>(HUDTeamColors::Enable),
        },
        [static_cast<u32>(Setting::MiniMap)] =
        {
            .category = Category::RaceHUD,
            .name = magic_enum::enum_name(Setting::MiniMap),
            .messageId = 10072,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(MiniMap::Enable),
            .valueCount = magic_enum::enum_count<MiniMap>(),
            .valueNames = magic_enum::enum_names<MiniMap>().data(),
            .valueMessageIds = (const s32[]){10005, 10004},
            .valueExplanationMessageIds = (const s32[]){10073, 10074},
            .hidden = false,
            .selectedValue = static_cast<u32>(MiniMap::Enable),
        },
        [static_cast<u32>(Setting::InputDisplay)] =
        {
            .category = Category::RaceHUD,
            .name = magic_enum::enum_name(Setting::InputDisplay),
            .messageId = 10080,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(InputDisplay::Simple),
            .valueCount = magic_enum::enum_count<InputDisplay>(),
            .valueNames = magic_enum::enum_names<InputDisplay>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10081, 10082},
            .hidden = false,
            .selectedValue = static_cast<u32>(InputDisplay::Simple),
        },
        [static_cast<u32>(Setting::Speedometer)] =
        {
            .category = Category::RaceHUD,
            .name = magic_enum::enum_name(Setting::Speedometer),
            .messageId = 10083,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(Speedometer::InternalPlus),
            .valueCount = magic_enum::enum_count<Speedometer>(),
            .valueNames = magic_enum::enum_names<Speedometer>().data(),
            .valueMessageIds = (const s32[]){10084, 10085, 10086, 10087, 10088, 10089},
            .valueExplanationMessageIds = (const s32[]){10090, 10091, 10092, 10093, 10094, 10095},
            .hidden = false,
            .selectedValue = static_cast<u32>(Speedometer::InternalPlus),
        },
        // A number: valueNames is null, the messages get the number as an argument.
        [static_cast<u32>(Setting::Volume)] =
        {
            .category = Category::Sound,
            .name = magic_enum::enum_name(Setting::Volume),
            .messageId = 10096,
            .valueOffset = 0,
            .defaultValue = 10,
            .valueCount = 11,
            .valueNames = nullptr,
            .valueMessageIds = (const s32[]){10097},
            .valueExplanationMessageIds = (const s32[]){10098},
            .hidden = false,
            .selectedValue = 10,
        },
        // A number: valueNames is null, the messages get the number as an argument.
        [static_cast<u32>(Setting::MusicVolume)] =
        {
            .category = Category::Sound,
            .name = magic_enum::enum_name(Setting::MusicVolume),
            .messageId = 10099,
            .valueOffset = 0,
            .defaultValue = 10,
            .valueCount = 11,
            .valueNames = nullptr,
            .valueMessageIds = (const s32[]){10097},
            .valueExplanationMessageIds = (const s32[]){10100},
            .hidden = false,
            .selectedValue = 10,
        },
        [static_cast<u32>(Setting::ItemMusic)] =
        {
            .category = Category::Sound,
            .name = magic_enum::enum_name(Setting::ItemMusic),
            .messageId = 10101,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(ItemMusic::All),
            .valueCount = magic_enum::enum_count<ItemMusic>(),
            .valueNames = magic_enum::enum_names<ItemMusic>().data(),
            .valueMessageIds = (const s32[]){10102, 10103, 10104},
            .valueExplanationMessageIds = (const s32[]){10105, 10106, 10107},
            .hidden = false,
            .selectedValue = static_cast<u32>(ItemMusic::All),
        },
        [static_cast<u32>(Setting::LastLapJingle)] =
        {
            .category = Category::Sound,
            .name = magic_enum::enum_name(Setting::LastLapJingle),
            .messageId = 10108,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(LastLapJingle::GameplayOnly),
            .valueCount = magic_enum::enum_count<LastLapJingle>(),
            .valueNames = magic_enum::enum_names<LastLapJingle>().data(),
            .valueMessageIds = (const s32[]){10109, 10110},
            .valueExplanationMessageIds = (const s32[]){10111, 10112},
            .hidden = false,
            .selectedValue = static_cast<u32>(LastLapJingle::GameplayOnly),
        },
        [static_cast<u32>(Setting::LastLapSpeedup)] =
        {
            .category = Category::Sound,
            .name = magic_enum::enum_name(Setting::LastLapSpeedup),
            .messageId = 10113,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(LastLapSpeedup::Static),
            .valueCount = magic_enum::enum_count<LastLapSpeedup>(),
            .valueNames = magic_enum::enum_names<LastLapSpeedup>().data(),
            .valueMessageIds = (const s32[]){10102, 10114, 10115},
            .valueExplanationMessageIds = (const s32[]){10116, 10117, 10118},
            .hidden = false,
            .selectedValue = static_cast<u32>(LastLapSpeedup::Static),
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
