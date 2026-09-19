#pragma once

#include <Common.hh>

#include "game/net/records/Event.hh"
#include "game/net/records/RaceData.hh"
#include "game/net/records/Select.hh"

extern "C" {
#include <revolution/os.h>
}

namespace Net {

// RaceHandler runs exclusively in race scene. It handles everything netcode during a race.
class RaceHandler {
public:
    // Added. sets m_readyAcked
    void setAckReady();

    static RaceHandler *Instance() {
        return s_instance;
    }

private:
    // 0x806532d8
    // Initializes members. Called by RaceScene::initSubsystems() once per race.
    // Hooked to reset m_readyRetryCooldown and m_readyAcked at the start of the race.
    REPLACE void init();
    void REPLACED(init)();

    // 0x80653728
    // The main loop. Runs when m_isPrepared is true. This runs in online races
    // but also rival ghost races to update friend status
    void update();

    // Added.
    // Sends a ready packet to mkw-server if we're "ready" to start the race.
    void trySendReady();

    // 0x80654150
    // Runs only when racing. Synchronizes race start, imports, exports, and processes records
    // specifically as a racer.
    // Hooked to call trySendReady() until we receive an ack from mkw-server.
    REPLACE void updateAsRacer();
    void REPLACED(updateAsRacer)();

    // 0x80654d08
    // Exports relevant records as a racer (as opposed to a spectator)
    void exportAsRacer();

    // 0x806554a0
    // Unpacks and processes received Race Info records. Updates m_aidsLoadedIntoRace and time of
    // other players. Also checks if we're out of sync of other players and disconnects us if so.
    void processRecvRaceInfoRecords();

    // 0x80654038
    // Returns true when all players have loaded into the race
    bool isEveryoneInRace() const;

    // 0x80654b00
    // Processes other player's lag frames. Skip frames if someone has skipped more frames than us.
    // Skips one frame if framerate is 60 fps and two if 30 fps.
    void processLagFrames();

    // 0x806544a8
    // Gets the passed in player's time since countdown if its not terribly out of sync
    u32 getPlayerTimeSinceCountdown(s32 playerId) const;

    // Runs update() when true
    bool m_isPrepared;

    // Schecules a disconnect. Set if we're out of sync with other players.
    bool m_scheduleDisconnect;

    // Added, was padding. Cooldown in frames before resending a ready packet to mkw-server.
    u8 m_readyRetryCooldown;

    // Added, was padding. Set when mkw-server has acked our ready packet.
    bool m_readyAcked;

    // Bitfield set by RaceInfo.raceSeed
    u32 m_aidsLoadedIntoRace;

    // Bitfield set if a room or select record was sent
    u32 m_aidLastSentRoomOrSelect;

    // Bitfield that stops non-racers (the dc animation?)
    u32 m_aidShouldStop;

    // Number of frames skipped
    s16 m_localLagFrames;

    // Timer used to syncronize the countdown start with other players, adjusted on the fly.
    // Countdown starts when this is negative.
    s16 m_introCameraCountdown;

    // Outgoing RaceData records. One per local player
    RaceDataRecord m_sendRaceDataRecords[2];

    // Outgoing Event record.
    EventRecord m_sendEventRecords;

    // Padding
    u8 _18c[0x190 - 0x18c];

    // Time since friend status was last retrieved
    OSTime m_TimeSinceFriendStatusUpdated;

    // Set to the countdown time, but set after the countdown ends
    u32 m_aidsTimeSinceCountdown[MAX_PLAYER_COUNT];

    static RaceHandler *s_instance;
};
static_assert(sizeof(RaceHandler) == 0x1c8);

} // namespace Net
