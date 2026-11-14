#include <Common.S>

PATCH_REPLACE_START(NetManager_CreateInstance, 0x28) 
    // Allocate the full NetManager: the game's 0x29c8 plus m_outgoingUniquePackets and the
    // extended friend slots. Must equal sizeof(Net::NetManager) (static_assert in NetManager.hh).
    li r3, 0x2bf8
PATCH_REPLACE_END(NetManager_CreateInstance, 0x28)

