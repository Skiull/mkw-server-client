#include "RacePage.hh"

#include "game/system/RaceConfig.hh"
#include "game/ui/ctrl/CtrlRaceInputDisplay.hh"
#include "game/ui/ctrl/CtrlRaceLap.hh"
#include "game/ui/ctrl/CtrlRacePing.hh"
#include "game/ui/ctrl/CtrlRaceSpeed.hh"

#include <sp/net/mkw_server/MKWServer.hh>
#include <sp/settings/Settings.hh>

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

    // Player Tags setting: RacePage.S makes vanilla load 12 name tags.
    if (getNameBalloonCount() != 0) {
        count += 12 - getNameBalloonCount();
    }

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

} // namespace UI
