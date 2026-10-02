#include "RaceManager.hh"

#include "sp/net/mkw_server/MKWServer.hh"

#include <game/kart/KartObjectManager.hh>
#include <game/kart/VehiclePhysics.hh>
#include <game/system/RaceConfig.hh>
#include <game/system/RaceManager.hh>

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
    u32 currTime = m_time;
    if (updateTime()) {
        // it changed, meaning the last frame is finalized and can be sent.
        if (auto raceFrame = getOrCreateFrame(currTime)) {
            if (!MKWServer::sendRaceFrame(raceFrame)) {
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

void RaceManager::markReceived(u8 aid, u32 time) {
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

    u32 raceManagerTime = raceManager->countdownTimer();
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
            if (!MKWServer::sendRaceFrame(raceFrame)) {
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
    u32 inputs = 0;
    Vec3 pos = {};

    auto *raceConfig = System::RaceConfig::Instance();
    auto *raceManager = System::RaceManager::Instance();
    if (raceConfig && raceManager) {
        s8 playerId = raceConfig->raceScenario().localPlayerIds[0];
        if (auto *player = raceManager->player(playerId)) {
            inputs = player->padProxy()->currentRaceInputState().inputBitfield();
        }
    }

    if (auto *kartObjectManager = Kart::KartObjectManager::Instance()) {
        // TODO!!! Replace 0 with playerId!!!
        if (auto *obj = kartObjectManager->object(0)) {
            if (auto *physics = obj->getVehiclePhysics()) {
                pos = *physics->pos();
            }
        }
    }

    return RaceFrame(time, inputs, pos);
}

} // namespace MKWServer
