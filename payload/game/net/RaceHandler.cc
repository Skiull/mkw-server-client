#include "RaceHandler.hh"

#include "game/net/NetManager.hh"
#include "game/system/RaceManager.hh"

#include <sp/net/mkw_server/MKWServer.hh>

// Time the intro camera rotation will finish by. It actually finishes after 170 frames,
// but 30 frames are added to be safe.
static constexpr u16 INTRO_CAMERA_ROTATION_FINISH_TIME = 200;

static constexpr u8 READY_RETRY_COOLDOWN = 5;

namespace Net {

void RaceHandler::setAckReady() {
    m_readyAcked = true;
}

void RaceHandler::init() {
    REPLACED(init)();

    m_readyRetryCooldown = 0;
    m_readyAcked = false;
}

void RaceHandler::trySendReady() {
    if (m_readyRetryCooldown > 0) {
        m_readyRetryCooldown--;
        return;
    }

    // Send a ready packet when the intro camera has finished rotating.
    if (System::RaceManager::Instance()->introTimer() > INTRO_CAMERA_ROTATION_FINISH_TIME) {
        SP_LOG("Sending Ready Packet!");
        MKWServer::sendReadyPacket();
        m_readyRetryCooldown = READY_RETRY_COOLDOWN;
    }
}

void RaceHandler::updateAsRacer() {
    if (!m_readyAcked) {
        trySendReady();
    }

    REPLACED(updateAsRacer)();
}

void RaceHandler::exportAsRacer() {
    REPLACED(exportAsRacer)();

    auto *netManager = NetManager::Instance();
    auto *raceManager = System::RaceManager::Instance();

    // Set if we've started the countdown or not in the outgoing records.
    for (u8 aid = 0; aid < MAX_PLAYER_COUNT; aid++) {
        if (!netManager->canSendToAid(aid)) {
            continue;
        }
        netManager->lastSentRaceBuffer(aid)->raceInfo()->record()->startedCountdown =
                raceManager->isRaceOngoing();
    }
}

} // namespace Net
