#pragma once

#include <Common.h>

// Exposed to C because UDP packets from mkw-server are dispatched in
// DWCi_GT2UnrecognizedMessageCallback (DWCTransport.c).

EXTERN_C bool ServerItems_isItemResponsePacket(const u8 *message, s32 len);

EXTERN_C void ServerItems_handleItemResponsePacket(const u8 *message, s32 len);
