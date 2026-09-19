#include "RaceManager.hh"

#include "sp/net/mkw_server/MKWServer.hh"

#include <game/kart/KartObjectManager.hh>
#include <game/kart/VehiclePhysics.hh>
#include <game/system/RaceConfig.hh>
#include <game/system/RaceManager.hh>

#include <bit>
#include <utility>

extern "C" {
#include <revolution.h>
}

namespace MKWServer {

RaceFrame::RaceFrame(u16 time) : m_time(time) {
    s8 playerId = System::RaceConfig::Instance()->raceScenario().localPlayerIds[0];
    u16 inputs = System::RaceManager::Instance()
                         ->player(playerId)
                         ->padProxy()
                         ->currentRaceInputState()
                         .inputBitfield();
    auto pos = Kart::KartObjectManager::Instance()->object(0)->getVehiclePhysics()->pos();
    set(inputs, pos);
}

u16 RaceFrame::time() const {
    return m_time;
}

void RaceFrame::set(u16 inputs, const Vec3 *pos) {
    u32 px = std::bit_cast<u32>(pos->x) >> 5;
    u32 py = std::bit_cast<u32>(pos->y) >> 5;
    u32 pz = std::bit_cast<u32>(pos->z) >> 5;

    m_data[0] = (inputs & 0x7fff) | ((px & 0x1fffff) << 15);
    m_data[1] = ((px >> 17) & 0x3ff) | ((py & 0x3fffff) << 10);
    m_data[2] = ((py >> 22) & 0x1f) | ((pz & 0x7ffffff) << 5);
}

RaceFramePacket::RaceFramePacket(u16 time) : frame(time) {}

RaceFramePacket::RaceFramePacket(RaceFrame frame) : frame(frame) {}

RaceManager *RaceManager::s_instance = nullptr;

bool RaceManager::sendFrame(const RaceFramePacket *frame) const {
    return MKWServer::sendRaceFrame(frame);
}

void RaceManager::updateCurrentFrame(u16 time) {
    const RaceFramePacket packet(time);
    if (sendFrame(&packet)) {
        if (!m_raceFrames.push_back(std::move(packet.frame))) {
            SP_LOG("Can't push frame %d back, over capacity", time);
        }
    }
}

void RaceManager::resendOldFrames(u16 time) {
    for (u32 i = 0; i < m_raceFrames.count(); i++) {
        auto raceFrame = m_raceFrames[i];
        u16 timeSinceSentFrame = time - raceFrame->time();
        // Start resending after 2 seconds, every half second.
        if (timeSinceSentFrame > 120 && timeSinceSentFrame % 30 == 0) {
            RaceFramePacket packet(*raceFrame);
            if (!sendFrame(&packet)) {
                SP_LOG("RaceFrame %d failed to resend!", raceFrame->time());
            }
        }
    }
}

void RaceManager::update(u16 time) {
    updateCurrentFrame(time);
    resendOldFrames(time);
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

} // namespace MKWServer
