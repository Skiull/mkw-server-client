#pragma once

#include <Common.hh>

#include "sp/CircularBuffer.hh"
#include "sp/net/mkw_server/race_verification/RaceFrame.hh"

#include <array>

namespace MKWServer {

// TODO: Not sure what a good size is in practice.
static constexpr size_t FrameBufferSize = 300;
static constexpr u32 FinishTimeResendMs = 2000;

class RaceManager {
public:
    void update();

    bool removeRaceFrame(u16 time);

    void markReceived(u8 aid, u16 time);

    void updateRaceFinished();

    void onFinishTimeAck();

    void handleMissingFrames(const u8 *message, s32 len);

    void onMissingFramesAck();

    bool canStartNextRace();

    bool makeAndSendConfig();

    static void CreateInstance();

    static void DestroyInstance();

    static RaceManager *Instance() {
        return s_instance;
    }

private:
    RaceFrame *getOrCreateFrame(u16 time);
    bool updateTime();
    bool isActive();
    void resendFrames();
    s32 findFrame(u16 time);
    RaceFrame makeFrame(u16 time);
    u32 raceFrameSize() const;

    // Race frames sent but waiting to be acked by mkw-server.
    SP::CircularBuffer<RaceFrame, FrameBufferSize> m_raceFrames;

    // Number of frames since the countdown started (RaceManager::m_countdownTimer).
    // Sends mkw-server a frame when updated.
    u32 m_time;

    u16 m_raceFinishTime;
    bool m_finishTimeSent;
    bool m_finishTimeAcked;
    u64 m_finishTimeLastSendTick;
    bool m_missingFramesAcked;

    u8 m_localPlayerCount;
    u8 m_myAid;
    std::array<s8, 2> m_playerIds;

    static RaceManager *s_instance;
};

} // namespace MKWServer
