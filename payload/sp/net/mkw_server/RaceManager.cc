#include "RaceManager.hh"

#include "sp/net/mkw_server/MKWServer.hh"

#include <game/kart/KartObjectManager.hh>
#include <game/kart/VehiclePhysics.hh>
#include <game/system/RaceConfig.hh>
#include <game/system/RaceManager.hh>

#include <bit>
#include <utility>

#include <string.h>

extern "C" {
#include <revolution.h>
}

namespace MKWServer {

RaceFrame::RaceFrame() {
    s8 playerId = System::RaceConfig::Instance()->raceScenario().localPlayerIds[0];
    if (auto *raceManager = System::RaceManager::Instance()) {
        m_inputs =
                raceManager->player(playerId)->padProxy()->currentRaceInputState().inputBitfield();
    }

    auto *kartObjectManager = Kart::KartObjectManager::Instance();
    if (!kartObjectManager) {
        return;
    }

    auto *obj = kartObjectManager->object(0);
    if (!obj) {
        return;
    }

    auto *physics = obj->getVehiclePhysics();
    if (!physics) {
        return;
    }

    m_pos = *physics->pos();
}

void RaceFrame::setRecvData(u8 aid, u32 crc) {
    m_receivedCRC[aid] = crc;
}

RaceFramePacket::RaceFramePacket(u16 time) : time(time), frame(RaceFrame()) {}

RaceFramePacket::RaceFramePacket(RaceFrame frame) : frame(frame) {}

void RaceFramePacket::setPacketReceived(u8 aid, u32 crc) {
    frame.setRecvData(aid, crc);
}

bool RaceFramePacket::send() {
    // Temporarily flip the sent flag to false so the server doesn't have to do any bit operations.
    sent = false;
    bool result = MKWServer::sendRaceFrame(this);
    sent = true;
    return result;
}

RaceManager *RaceManager::s_instance = nullptr;

void RaceManager::update() {
    if (!isActive()) {
        return;
    }

    // save m_time incase it changed.
    u32 currTime = m_time;
    if (updateTime()) {
        // it updated, export/send the old frame and initialize the new frame.
        exportFrame(currTime);
        // just create it, no need to set anything.
        getOrCreateFrame(m_time);
    };
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

void RaceManager::pushReceivedPacket(u8 aid, u32 crc) {
    // Calling update
    if (!isActive()) {
        return;
    }
    update();
    if (auto raceFrame = getOrCreateFrame(m_time)) {
        raceFrame->setPacketReceived(aid, crc);
    }
}

bool RaceManager::isRaceFramePacket(const u8 *message, s32 len) {
    return len == static_cast<s32>(sizeof(RaceFrameMagic)) + sizeof(u16) &&
            memcmp(message, RaceFrameMagic, sizeof(RaceFrameMagic)) == 0;
}

void RaceManager::CreateInstance() {
    if (s_instance) {
        return;
    }

    s_instance = new RaceManager;
}

void RaceManager::DestroyInstance() {
    if (!s_instance) {
        return;
    }

    delete s_instance;
    s_instance = nullptr;
}

RaceFramePacket *RaceManager::getOrCreateFrame(u16 time) {
    s32 idx = findFrame(time);
    if (idx < 0) {
        if (!m_raceFrames.push_back(RaceFramePacket(time))) {
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
    auto raceScenario = System::RaceConfig::Instance()->raceScenario();
    if (!raceScenario.isOnline()) {
        return false;
    }

    u32 raceManagerTime = raceManager->countdownTimer();
    if (m_time == raceManagerTime) {
        return false;
    }

    m_time = raceManagerTime;
    return true;
}

bool RaceManager::isActive() {
    auto raceScenario = System::RaceConfig::Instance()->raceScenario();
    auto *raceManager = System::RaceManager::Instance();
    if (!raceScenario.isOnline() || !raceManager->isRaceOngoing()) {
        return false;
    }

    return true;
}

void RaceManager::exportFrame(u32 time) {
    if (auto raceFrame = getOrCreateFrame(time)) {
        if (!raceFrame->send()) {
            SP_LOG("Failed to send frame %d", time);
            return;
        }
        raceFrame->sent = true;
    }
}

void RaceManager::resendFrames() {
    for (u32 i = 0; i < m_raceFrames.count(); i++) {
        auto raceFrame = m_raceFrames[i];
        if (!raceFrame->sent) {
            continue;
        }

        u16 timeSinceSend = m_time - raceFrame->time;
        // Start resending after 2 seconds, every half second.
        if (timeSinceSend > 30 && timeSinceSend % 10 == 0) {
            if (!raceFrame->send()) {
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

} // namespace MKWServer
