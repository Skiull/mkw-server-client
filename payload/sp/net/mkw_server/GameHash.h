#pragma once

#include <Common.h>

// Exposed to C because UDP packets from mkw-server are dispatched in
// DWCi_GT2UnrecognizedMessageCallback (DWCTransport.c).

EXTERN_C bool GameHash_isChallengePacket(const u8 *message, s32 len);

EXTERN_C void GameHash_handleChallengePacket(const u8 *message, s32 len);
