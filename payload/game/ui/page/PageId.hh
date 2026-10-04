// See LICENSE-mkw-sp

#pragma once

namespace UI {

enum class PageId {
    None = -0x1,
    Empty = 0x0,

    WifiRaceEndCountDown = 0x45,
    WifiRaceEndWait = 0x48,

    WifiShutdown = 0x87,
    WifiMenuTop = 0x8B,
    WifiMenuModeSelect = 0x8C,
    WifiFriendMenu = 0x8D,
    WifiRandomMatching = 0x8F,

    Max = 0xD3,

    ExtensionsBegin = 0xff,

    MenuSettings,

    ExtensionsEnd,
};
static_assert(
        static_cast<size_t>(PageId::ExtensionsEnd) > static_cast<size_t>(PageId::ExtensionsBegin));

constexpr size_t StandardPageCount() {
    return static_cast<size_t>(PageId::Max);
}

constexpr size_t ExtendedPageCount() {
    return static_cast<size_t>(PageId::ExtensionsEnd) -
            static_cast<size_t>(PageId::ExtensionsBegin);
}

} // namespace UI
