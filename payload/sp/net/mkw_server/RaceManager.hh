#pragma once

#include <Common.hh>

#include "sp/CircularBuffer.hh"
#include "sp/net/mkw_server/RaceFrame.hh"

namespace MKWServer {

// TODO: Not sure what a good size is in practice.
static constexpr size_t FrameBufferSize = 300;
static constexpr u32 FinishTimeResendMs = 2000;

class RaceManager {
public:
    void update();

    bool removeRaceFrame(u16 time);

    void markReceived(u8 aid, u32 time);

    void updateRaceFinished();

    void onFinishTimeAck();

    void handleMissingFrames(const u8 *message, s32 len);

    void onMissingFramesAck();

    bool canStartNextRace();

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

    // Race frames sent but waiting to be acked by mkw-server.
    SP::CircularBuffer<RaceFrame, FrameBufferSize> m_raceFrames;

    // Number of frames since the countdown started (RaceManager::m_countdownTimer).
    // Sends mkw-server a frame when updated.
    u32 m_time = 0;

    u32 m_raceFinishTime = 0;
    bool m_finishTimeSent = false;
    bool m_finishTimeAcked = false;
    u64 m_finishTimeLastSendTick = 0;
    bool m_missingFramesAcked = false;

    static RaceManager *s_instance;
};

} // namespace MKWServer
