#pragma once

#include "game/ui/ctrl/Button.hh"
#include "game/ui/page/Page.hh"

namespace UI {

// The pause menus (and after-race menus). The game constructs these; we only hook them.
class RaceMenuPage : public Page {
public:
    enum class ButtonId {
        Continue1 = 0x0,
        Quit1 = 0x1,
        Restart1 = 0x2,
        Restart2 = 0x3,
        Replay = 0x4,
        ChangeCourse = 0x5,
        ChangeCharacter = 0x6,
        Next = 0x7,
        Ranking = 0x8,
        ContinueReplay = 0x9,
        RestartReplay = 0xA,
        QuitReplay = 0xB,
        Continue2 = 0xC,
        Quit2 = 0xD,
        BattleGhost = 0xE,
        Restart3 = 0xF,
        Continue3 = 0x10,
        Quit3 = 0x11,
        ChangeMission = 0x12,
        Send1 = 0x13,
        NoSend1 = 0x14,
        GoRanking = 0x15,
        NotGoRanking = 0x16,
        ConfirmContinue = 0x17,
        ConfirmQuit = 0x18,
        SendRecord = 0x19,
        Send2 = 0x1A,
        NoSend2 = 0x1B,
        FriendGhostBattle = 0x1C,
        GoFriendRoom = 0x1D,
        NotGoFriendRoom = 0x1E,
        NextGhost = 0x1F,
        Yes1 = 0x20,
        No1 = 0x21,
        Quit4 = 0x22,
        Yes2 = 0x23,
        No2 = 0x24,

        // Our buttons reuse ids that the pause menus never show, so the game's own id ->
        // variant name table keeps working (no patch to onInit). Their variants
        // ("ButtonGoFriendRoom", "ButtonSend", "ButtonNoSend") are defined with our texts in
        // our PauseMenu*SP.brctr files.
        Settings = GoFriendRoom,
        SaveState = Send1,
        LoadState = NoSend1,
    };

    virtual void setReplacement(PageId id);
    virtual s32 getWindowMessage();
    virtual s32 getButtonCount();
    virtual const ButtonId *getButtons();
    virtual bool getOptionClosable();
    virtual const char *getResFileName();

    // Called by the Settings page when it opens.
    static void OnSettingsActivated();

private:
    bool hasSPButtons();
    void REPLACED(onButtonFront)(PushButton *button, u32 localPlayerId);
    REPLACE void onButtonFront(PushButton *button, u32 localPlayerId);
    void onSettingsButtonFront(PushButton *button);

    u8 _044[0x344 - 0x044];

    static PageId s_settingsOpenedFrom; // The pause menu that opened the Settings page
};
static_assert(sizeof(RaceMenuPage) == 0x344);

// The game's RaceMenuPage subclasses. They derive from Page here, not RaceMenuPage, so these
// hooks of their virtual functions stay plain functions: overriding them would make GCC emit
// vtables for classes that only the game constructs.
class TaRaceMenuPage : public Page {
public:
    using ButtonId = RaceMenuPage::ButtonId;

    s32 REPLACED(getButtonCount)();
    REPLACE s32 getButtonCount();
    const ButtonId *REPLACED(getButtons)();
    REPLACE const ButtonId *getButtons();
    const char *REPLACED(getResFileName)();
    REPLACE const char *getResFileName();

private:
    u8 _044[0x344 - 0x044];
};
static_assert(sizeof(TaRaceMenuPage) == sizeof(RaceMenuPage));

class VsRaceMenuPage : public Page {
public:
    using ButtonId = RaceMenuPage::ButtonId;

    s32 REPLACED(getButtonCount)();
    REPLACE s32 getButtonCount();
    const ButtonId *REPLACED(getButtons)();
    REPLACE const ButtonId *getButtons();
    const char *REPLACED(getResFileName)();
    REPLACE const char *getResFileName();

private:
    u8 _044[0x344 - 0x044];
};
static_assert(sizeof(VsRaceMenuPage) == sizeof(RaceMenuPage));

class BtRaceMenuPage : public Page {
public:
    using ButtonId = RaceMenuPage::ButtonId;

    s32 REPLACED(getButtonCount)();
    REPLACE s32 getButtonCount();
    const ButtonId *REPLACED(getButtons)();
    REPLACE const ButtonId *getButtons();
    const char *REPLACED(getResFileName)();
    REPLACE const char *getResFileName();

private:
    u8 _044[0x344 - 0x044];
};
static_assert(sizeof(BtRaceMenuPage) == sizeof(RaceMenuPage));

} // namespace UI
