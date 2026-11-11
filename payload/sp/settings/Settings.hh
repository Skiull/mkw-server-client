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
    TimeTrials,
};

enum class Setting {
    // Online
    Ping,
    // Ranks
    LoungeRank,
    // Menus
    FastMenuTransitions,
    RememberCombo,
    // Race HUD
    FOV169,
    FPSMode,
    FarPlayerTags,
    HUDLabels,
    InputDisplay,
    Speedometer,
    // Sound
    Volume,
    MusicVolume,
    ItemMusic,
    LastLapJingle,
    LastLapSpeedup,
    // Time Trials (offline only)
    YButton,
    SaveStates,
    TAGhostTagVisibility,
    TAGhostTagContent,
    TAItemBoxes,
    TALongerTCs,
    TAHopDodgePractice,
    TAStartingSpot,
    TAStartingSpotsLayout,
    TANoVolcanoRocks,
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

// Y / ZL / D-pad down (see RaceManager.cc) in offline Time Trials.
enum class YButton {
    None,
    ItemWheel,
};

// Save and reload the kart's state in offline Time Trials.
enum class SaveStates {
    Disabled,
    Enabled,
};

// Which ghost name tags to show in offline Time Trials.
enum class TAGhostTagVisibility {
    None,
    Watched,
    All,
};

// What a ghost's name tag shows in offline Time Trials.
enum class TAGhostTagContent {
    Name,
    Time,
    TimeNoLeading,
    Date,
};

// Practice tools ported from the henrymkw/mkw-sp fork (ui-rework-with-practice-features).
// Offline Time Trials only (see sp/TimeTrialCheats.hh); a run that uses one can't save its ghost.

// Item boxes on the course in offline Time Trials.
enum class TAItemBoxes {
    Disabled,
    Enabled,
};

// A Thunder Cloud strike keeps you shrunk for much longer in offline Time Trials.
enum class TALongerTCs {
    Disabled,
    Enabled,
};

// Press X (GameCube / Classic) or D-pad left/right (Wii Remote) to set off a POW Block on yourself.
enum class TAHopDodgePractice {
    Disabled,
    Enabled,
};

template <Setting S>
struct OptionType;

// Start a Time Trial from grid spot N (1 = the normal Time Trial spot) of a grid laid out like an
// M-player room (from the henrymkw/mkw-sp fork). Numbers, 1 to 12.
template <>
struct OptionType<Setting::TAStartingSpot> {
    using Type = u32;
};

template <>
struct OptionType<Setting::TAStartingSpotsLayout> {
    using Type = u32;
};

// No falling rocks on Grumble Volcano in offline Time Trials.
enum class TANoVolcanoRocks {
    Disabled,
    Enabled,
};

template <>
struct OptionType<Setting::TANoVolcanoRocks> {
    using Type = TANoVolcanoRocks;
};

template <>
struct OptionType<Setting::TAItemBoxes> {
    using Type = TAItemBoxes;
};

template <>
struct OptionType<Setting::TALongerTCs> {
    using Type = TALongerTCs;
};

template <>
struct OptionType<Setting::TAHopDodgePractice> {
    using Type = TAHopDodgePractice;
};

template <>
struct OptionType<Setting::TAGhostTagVisibility> {
    using Type = TAGhostTagVisibility;
};

template <>
struct OptionType<Setting::TAGhostTagContent> {
    using Type = TAGhostTagContent;
};

template <>
struct OptionType<Setting::YButton> {
    using Type = YButton;
};

template <>
struct OptionType<Setting::SaveStates> {
    using Type = SaveStates;
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
        10119,
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
        [static_cast<u32>(Setting::YButton)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::YButton),
            .messageId = 10120,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(YButton::ItemWheel),
            .valueCount = magic_enum::enum_count<YButton>(),
            .valueNames = magic_enum::enum_names<YButton>().data(),
            .valueMessageIds = (const s32[]){10121, 10122},
            .valueExplanationMessageIds = (const s32[]){10123, 10124},
            .hidden = false,
            .selectedValue = static_cast<u32>(YButton::ItemWheel),
        },
        [static_cast<u32>(Setting::SaveStates)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::SaveStates),
            .messageId = 10125,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(SaveStates::Enabled),
            .valueCount = magic_enum::enum_count<SaveStates>(),
            .valueNames = magic_enum::enum_names<SaveStates>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10126, 10127},
            .hidden = false,
            .selectedValue = static_cast<u32>(SaveStates::Enabled),
        },
        [static_cast<u32>(Setting::TAGhostTagVisibility)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::TAGhostTagVisibility),
            .messageId = 10128,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(TAGhostTagVisibility::All),
            .valueCount = magic_enum::enum_count<TAGhostTagVisibility>(),
            .valueNames = magic_enum::enum_names<TAGhostTagVisibility>().data(),
            .valueMessageIds = (const s32[]){10129, 10130, 10131},
            .valueExplanationMessageIds = (const s32[]){10132, 10133, 10134},
            .hidden = false,
            .selectedValue = static_cast<u32>(TAGhostTagVisibility::All),
        },
        [static_cast<u32>(Setting::TAGhostTagContent)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::TAGhostTagContent),
            .messageId = 10135,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(TAGhostTagContent::Name),
            .valueCount = magic_enum::enum_count<TAGhostTagContent>(),
            .valueNames = magic_enum::enum_names<TAGhostTagContent>().data(),
            .valueMessageIds = (const s32[]){10136, 10137, 10138, 10139},
            .valueExplanationMessageIds = (const s32[]){10140, 10141, 10142, 10143},
            .hidden = false,
            .selectedValue = static_cast<u32>(TAGhostTagContent::Name),
        },
        [static_cast<u32>(Setting::TAItemBoxes)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::TAItemBoxes),
            .messageId = 10149,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(TAItemBoxes::Disabled),
            .valueCount = magic_enum::enum_count<TAItemBoxes>(),
            .valueNames = magic_enum::enum_names<TAItemBoxes>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10150, 10151},
            .hidden = false,
            .selectedValue = static_cast<u32>(TAItemBoxes::Disabled),
        },
        [static_cast<u32>(Setting::TALongerTCs)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::TALongerTCs),
            .messageId = 10152,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(TALongerTCs::Disabled),
            .valueCount = magic_enum::enum_count<TALongerTCs>(),
            .valueNames = magic_enum::enum_names<TALongerTCs>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10153, 10154},
            .hidden = false,
            .selectedValue = static_cast<u32>(TALongerTCs::Disabled),
        },
        [static_cast<u32>(Setting::TAHopDodgePractice)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::TAHopDodgePractice),
            .messageId = 10155,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(TAHopDodgePractice::Disabled),
            .valueCount = magic_enum::enum_count<TAHopDodgePractice>(),
            .valueNames = magic_enum::enum_names<TAHopDodgePractice>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10156, 10157},
            .hidden = false,
            .selectedValue = static_cast<u32>(TAHopDodgePractice::Disabled),
        },
        // A number (1-12): valueNames is null, the messages get the number as an argument.
        [static_cast<u32>(Setting::TAStartingSpot)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::TAStartingSpot),
            .messageId = 10162,
            .valueOffset = 1,
            .defaultValue = 1,
            .valueCount = 12,
            .valueNames = nullptr,
            .valueMessageIds = (const s32[]){10166},
            .valueExplanationMessageIds = (const s32[]){10163},
            .hidden = false,
            .selectedValue = 1,
        },
        // A number (1-12): valueNames is null, the messages get the number as an argument.
        [static_cast<u32>(Setting::TAStartingSpotsLayout)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::TAStartingSpotsLayout),
            .messageId = 10164,
            .valueOffset = 1,
            .defaultValue = 1,
            .valueCount = 12,
            .valueNames = nullptr,
            .valueMessageIds = (const s32[]){10166},
            .valueExplanationMessageIds = (const s32[]){10165},
            .hidden = false,
            .selectedValue = 1,
        },
        [static_cast<u32>(Setting::TANoVolcanoRocks)] =
        {
            .category = Category::TimeTrials,
            .name = magic_enum::enum_name(Setting::TANoVolcanoRocks),
            .messageId = 10167,
            .valueOffset = 0,
            .defaultValue = static_cast<u32>(TANoVolcanoRocks::Disabled),
            .valueCount = magic_enum::enum_count<TANoVolcanoRocks>(),
            .valueNames = magic_enum::enum_names<TANoVolcanoRocks>().data(),
            .valueMessageIds = (const s32[]){10004, 10005},
            .valueExplanationMessageIds = (const s32[]){10168, 10169},
            .hidden = false,
            .selectedValue = static_cast<u32>(TANoVolcanoRocks::Disabled),
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

// Persistence in /mkw-selfhost/settings.ini on the SD card or USB drive (see Settings.cc).
void Load();
void Save();

} // namespace Settings
