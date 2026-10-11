#include "RaceManager.hh"

#include "sp/net/mkw_server/MKWServer.hh"
#include "sp/net/mkw_server/race_verification/Config.hh"
#include "sp/net/mkw_server/race_verification/MissingFrames.hh"

#include <game/kart/KartObjectManager.hh>
#include <game/kart/VehiclePhysics.hh>
#include <game/net/NetManager.hh>
#include <game/net/records/User.hh>
#include <game/system/RaceConfig.hh>
#include <game/system/RaceManager.hh>

#include <ranges>

extern "C" {
#include <revolution.h>
}

namespace MKWServer {

RaceManager *RaceManager::s_instance = nullptr;

void RaceManager::update() {
    if (!isActive()) {
        return;
    }

    // save m_time in case it changed.
    u16 currTime = m_time;
    if (updateTime()) {
        // it changed, meaning the last frame is finalized and can be sent.
        if (auto raceFrame = getOrCreateFrame(currTime)) {
            if (!MKWServer::sendRaceFrame(raceFrame, raceFrameSize())) {
                SP_LOG("Failed to send frame %d", currTime);
            }
        } else {
            SP_LOG("failed get frame to send (%d)", currTime);
        }

        // Create a new race frame for the current frame.
        if (!getOrCreateFrame(m_time)) {
            SP_LOG("Creating frame %d failed", m_time);
        }
    }
    resendFrames();
}

bool RaceManager::removeRaceFrame(u16 time) {
    s32 idx = findFrame(time);

    if (idx < 0) {
        return false;
    }

    m_raceFrames.remove(idx);
    return true;
}

void RaceManager::markReceived(u8 aid, u16 time) {
    if (!isActive()) {
        return;
    }

    // Call update() to handle the unlikely event that m_frame changed. This function is called
    // upon receiving a packet and runs on a different thread than System::RaceManager::upate(),
    // which calls MKWServer::RaceManager::update(). So this prevents m_frame from being stale when
    // marking recv timestamps.
    update();
    if (auto raceFrame = getOrCreateFrame(m_time)) {
        raceFrame->recvPacketTimestamps[aid] = time;
    }
}

void RaceManager::updateRaceFinished() {
    if (m_finishTimeAcked) {
        return;
    }

    if (m_raceFinishTime == 0) {
        auto *raceManager = System::RaceManager::Instance();
        if (!raceManager) {
            SP_LOG("raceManager is nullptr in updateRaceState()");
            return;
        }
        // A race frame isn't created on the frame prior to the finished frame. Its unclear why,
        // but this subtraction is needed to not miss a frame.
        // TODO: Figure out the off by one.
        m_raceFinishTime = raceManager->countdownTimer() - 1;
        SP_LOG("finishTime updated to %d", m_raceFinishTime);
    }

    u64 now = OSGetTick();
    if (m_finishTimeSent &&
            OSTicksToMilliseconds(now - m_finishTimeLastSendTick) < FinishTimeResendMs) {
        return;
    }

    if (m_finishTimeSent) {
        SP_LOG("no finishTime ack yet, resending %d", m_raceFinishTime);
    }

    m_finishTimeLastSendTick = now;
    m_finishTimeSent = MKWServer::sendFinishTime(m_raceFinishTime);
}

void RaceManager::onFinishTimeAck() {
    if (!m_finishTimeAcked) {
        SP_LOG("finishTime acked");
    }
    m_finishTimeAcked = true;
}

void RaceManager::handleMissingFrames(const u8 *message, s32 len) {
    if (!MissingFramesRequest::IsWellFormed(message, len)) {
        SP_LOG("Malformed missing frames request (len %d)", len);
        return;
    }

    u16 missingFrameCount = MissingFramesRequest::Count(message);

    for (u32 i = 0; i < missingFrameCount; i++) {
        u16 time = MissingFramesRequest::TimeAt(message, i);

        s32 idx = findFrame(time);
        if (idx < 0) {
            // Uh oh! Can't find it! TODO: Inform server.
            continue;
        }

        if (!MKWServer::sendRaceFrame(m_raceFrames[idx], raceFrameSize())) {
            SP_LOG("Failed to resend missing frame %d", time);
            continue;
        }
    }
}

void RaceManager::onMissingFramesAck() {
    if (!m_missingFramesAcked) {
        SP_LOG("missing frames acked");
    }

    m_missingFramesAcked = true;
}

bool RaceManager::canStartNextRace() {
    // For now, mkw-server just needs to send an ack for the missing race frames to transition
    // to the next race. But more requirements to transition may be added.
    return m_missingFramesAcked;
}

bool RaceManager::makeAndSendConfig() {
    auto &raceScenario = System::RaceConfig::Instance()->raceScenario();
    MKWServer::Race race(raceScenario);

    // outgoingRecord() will create a User record incase it has been cleared.
    auto *userRecord = Net::UserHandler::Instance()->outgoingRecord();
    if (!userRecord) {
        SP_LOG("Failed to make config because my User record is nullptr!");
        return false;
    }

    MKWServer::Aid aid(userRecord);
    MKWServer::ConfigPacket configPacket(race, aid);

    for (auto [i, playerId] : std::views::enumerate(m_playerIds)) {
        if (playerId < 0) {
            continue;
        }
        auto &player = raceScenario.players[playerId];
        configPacket.players[i] = MKWServer::Player(playerId, m_myAid, i, player);
    }

    return MKWServer::sendConfig(configPacket, ConfigPacket::CalcPacketSize(m_localPlayerCount));
}

void RaceManager::CreateInstance() {
    if (s_instance) {
        return;
    }

    s_instance = new RaceManager;

    auto *netManager = Net::NetManager::Instance();
    s_instance->m_myAid = netManager->myAid();
    s_instance->m_localPlayerCount = netManager->localPlayerCount();
    for (auto [i, val] : std::views::enumerate(s_instance->m_playerIds)) {
        s_instance->m_playerIds[i] = netManager->localPlayersPlayerId(i);
    }
}

void RaceManager::DestroyInstance() {
    if (!s_instance) {
        return;
    }

    delete s_instance;
    s_instance = nullptr;
}

RaceFrame *RaceManager::getOrCreateFrame(u16 time) {
    s32 idx = findFrame(time);
    if (idx < 0) {
        if (!m_raceFrames.push_back(makeFrame(time))) {
            return nullptr;
        }
        idx = m_raceFrames.count() - 1;
    }

    return m_raceFrames[idx];
}

bool RaceManager::updateTime() {
    auto *raceManager = System::RaceManager::Instance();
    if (!raceManager) {
        return false;
    }

    if (!raceManager->isRaceOngoing()) {
        return false;
    }

    auto *raceConfig = System::RaceConfig::Instance();
    if (!raceConfig || !raceConfig->raceScenario().isOnline()) {
        return false;
    }

    u16 raceManagerTime = raceManager->countdownTimer();
    if (m_time == raceManagerTime) {
        return false;
    }

    m_time = raceManagerTime;
    return true;
}

bool RaceManager::isActive() {
    auto *raceConfig = System::RaceConfig::Instance();
    auto *raceManager = System::RaceManager::Instance();
    if (!raceConfig || !raceManager) {
        return false;
    }

    return raceConfig->raceScenario().isOnline() && raceManager->isRaceOngoing();
}

void RaceManager::resendFrames() {
    for (u32 i = 0; i < m_raceFrames.count(); i++) {
        auto raceFrame = m_raceFrames[i];

        u16 timeSinceSend = m_time - raceFrame->time;
        // Start resending after 2 seconds, every half second.
        // TODO: Figure out what numbers/logic is good here.
        if (timeSinceSend > 30 && timeSinceSend % 10 == 0) {
            if (!MKWServer::sendRaceFrame(raceFrame, raceFrameSize())) {
                SP_LOG("RaceFrame %d failed to resend!", raceFrame->time);
            }
        }
    }
}

s32 RaceManager::findFrame(u16 time) {
    for (u32 i = 0; i < m_raceFrames.count(); i++) {
        auto raceFrame = m_raceFrames[i];
        if (raceFrame->time == time) {
            return i;
        }
    }
    return -1;
}

RaceFrame RaceManager::makeFrame(u16 time) {
    auto *raceManager = System::RaceManager::Instance();
    auto *kartObjectManager = Kart::KartObjectManager::Instance();

    RaceFrame frame(time);
    for (auto [i, playerId] : std::views::enumerate(m_playerIds)) {
        if (playerId == -1) {
            continue;
        }
        u16 inputs =
                raceManager->player(playerId)->padProxy()->currentRaceInputState().inputBitfield();
        Vec3 pos = *kartObjectManager->object(playerId)->getVehiclePhysics()->pos();
        RaceFramePlayer player(inputs, pos);
        frame.players[i] = player;
    }
    return frame;
}

u32 RaceManager::raceFrameSize() const {
    u32 playerRecordSize = m_localPlayerCount * sizeof(RaceFramePlayer);
    return sizeof(RaceFrame) - (2 * sizeof(RaceFramePlayer)) + playerRecordSize;
}

} // namespace MKWServer
