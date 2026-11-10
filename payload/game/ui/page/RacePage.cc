#include "RacePage.hh"

#include "game/item/PowManager.hh"
#include "game/kart/KartObjectManager.hh"
#include "game/kart/KartState.hh"
#include "game/system/RaceConfig.hh"
#include "game/system/RaceManager.hh"
#include "game/ui/ctrl/CtrlRaceInputDisplay.hh"
#include "game/ui/ctrl/CtrlRaceLap.hh"
#include "game/ui/ctrl/CtrlRacePing.hh"
#include "game/ui/ctrl/CtrlRaceSpeed.hh"

#include <sp/net/mkw_server/MKWServer.hh>
#include <sp/TimeTrialCheats.hh>
#include <sp/settings/Settings.hh>

extern "C" {
#include <revolution/kpad.h>
}

namespace UI {

namespace Control {

enum {
    Time = 1 << 1,
    Count = 1 << 2,
    Map = 1 << 3,
    RankNum = 1 << 4,
    Lap = 1 << 5,
    ItemWindow = 1 << 6,
    Point = 1 << 7,
    GhostTimeDiff = 1 << 8,
    Live = 1 << 9,
    Score = 1 << 10,
    ItemBalloon = 1 << 11,
    GhostMessage = 1 << 12,
};

} // namespace Control

bool RacePage::pingEnabled() const {
    return Settings::getSetting<Settings::Setting::Ping>() == Settings::Ping::Enabled &&
            MKWServer::hasMKWServerAddress();
}

static u32 localPlayerCount() {
    u32 count = System::RaceConfig::Instance()->raceScenario().localPlayerCount;
    return count < 1 ? 1 : count;
}

// The speedometer is left out in 3-4 player battles (no room next to the score).
static bool hasSpeedometer(u32 controls) {
    return localPlayerCount() <= 2 || !(controls & Control::Point);
}

// Vanilla passes the return into initChildren().
u8 RacePage::getControlCount(u32 controls) const {
    u8 count = REPLACED(getControlCount)(controls);

    if (pingEnabled()) {
        count++;
    }

    // From mkw-sp: the lap counters are ours (they move for the speedometer) but vanilla
    // already counts them.
    if (hasSpeedometer(controls)) {
        count += localPlayerCount(); // CtrlRaceSpeed
    }
    count += localPlayerCount(); // CtrlRaceInputDisplay

    return count;
}

// This gets called right after initChildren(). We can hook it to load our controls. Vanilla
// fills the children from the start, ours go at the end.
void RacePage::initControls(u32 controls) {
    REPLACED(initControls)(controls & ~Control::Lap);

    u32 index = getControlCount(controls) - 1;
    u32 localCount = localPlayerCount();

    if (pingEnabled()) {
        auto *control = new CtrlRacePing;
        insertChild(index--, control, 0);
        control->load();
    }

    if (controls & Control::Lap) {
        for (u32 i = 0; i < localCount; i++) {
            auto *control = new CtrlRaceLap;
            insertChild(index--, control, 0);
            control->load(localCount, i);
        }
    }

    if (hasSpeedometer(controls)) {
        for (u32 i = 0; i < localCount; i++) {
            auto *control = new CtrlRaceSpeed;
            insertChild(index--, control, 0);
            control->load(localCount, i);
        }
    }

    for (u32 i = 0; i < localCount; i++) {
        auto *control = new CtrlRaceInputDisplay;
        insertChild(index--, control, 0);
        control->load(localCount, i);
    }
}

// Hop dodge practice feedback, after the henrymkw/mkw-sp fork: a POW Block hits when its timer
// reaches 45, and hopping on that frame dodges it. The first hop between 55 and 35 is timed.
namespace HopDodge {

enum {
    WindowStart = 55,
    HitFrame = 45,
    WindowEnd = 35,
    ShowFrames = 150, // How long the result stays on screen
};

static bool s_inWindow = false;
static bool s_timed = false;
static bool s_hopButtonHeld = false;
static u32 s_showTimer = 0;

// The drift/hop button of player 0's controller.
static bool HopButtonPressed() {
    auto *raceManager = System::RaceManager::Instance();
    if (!raceManager) {
        return false;
    }
    auto *padProxy = raceManager->player(0)->padProxy();
    u16 buttons = padProxy->currentRaceInputState().rawButtons;
    switch (padProxy->pad()->getControllerId()) {
    case Registry::Controller::WiiWheel:
    case Registry::Controller::WiiRemoteAndNunchuck:
        return buttons & 0x0400; // Wii Remote B (WPAD_BUTTON_B)
    case Registry::Controller::Classic:
        return buttons & (KPAD_CL_BUTTON_B | KPAD_CL_TRIGGER_R);
    case Registry::Controller::GameCube:
        return buttons & (PAD_BUTTON_B | PAD_TRIGGER_R);
    default:
        return false;
    }
}

} // namespace HopDodge

void RacePage::afterCalc() {
    REPLACED(afterCalc)();
    calcHopDodgeFeedback();
}

void RacePage::calcHopDodgeFeedback() {
    using namespace HopDodge;

    auto *powManager = Item::PowManager::Instance();
    auto *kartObjectManager = Kart::KartObjectManager::Instance();
    if (!m_ghostMessage || !powManager || !kartObjectManager || !SP::HopDodgePracticeAllowed()) {
        return;
    }

    bool buttonHeld = HopButtonPressed();
    bool buttonPressed = buttonHeld && !s_hopButtonHeld;
    s_hopButtonHeld = buttonHeld;

    u32 timer = powManager->powTimer();
    bool inWindow = timer <= WindowStart && timer >= WindowEnd;
    if (inWindow && !s_inWindow) {
        s_timed = false;
    }

    s32 result = 0;
    bool hasResult = false;
    if (inWindow && !s_timed) {
        // Early hops show up as a hop start; late ones may not (the POW already hit), so the
        // button counts too.
        auto *state = kartObjectManager->object(0)->m_accessor.state;
        if (state->hopStart() || buttonPressed) {
            s_timed = true;
            hasResult = true;
            result = static_cast<s32>(timer) - HitFrame; // > 0: early, < 0: late
        }
    } else if (!inWindow && s_inWindow && !s_timed) {
        s_timed = true;
        hasResult = true;
        result = INT32_MIN; // No hop
    }
    s_inWindow = inWindow;

    if (hasResult) {
        MessageInfo info{};
        u32 messageId;
        if (result == INT32_MIN) {
            messageId = 10161;
        } else if (result == 0) {
            messageId = 10158;
        } else if (result > 0) {
            messageId = 10159;
            info.intVals[0] = result;
        } else {
            messageId = 10160;
            info.intVals[0] = -result;
        }
        m_ghostMessage->setMessageAll(messageId, &info);
        m_ghostMessage->setVisible(true);
        s_showTimer = ShowFrames;
    } else if (s_showTimer > 0 && --s_showTimer == 0) {
        m_ghostMessage->setVisible(false);
    }
}

} // namespace UI
