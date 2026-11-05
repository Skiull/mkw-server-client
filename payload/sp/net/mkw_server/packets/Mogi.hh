#pragma once

#include <Common.hh>

// Whether this player may create a mogi (their friend code is linked to MKW Lounge), asked by
// the Mogi button (wfc-server qr2/mogi.go). The reply also lists open mogis (unused).
namespace MKWServer {
#define MOGI_LIST 0x4D4F474C // "MOGL"
constexpr u32 MOGI_LIST_SIZE = 4 + 10 * 40;

void processMogiListPacket(const u8 *data);
bool mogiLinked();
u32 mogiListSerial();
bool sendMogiListRequest();
} // namespace MKWServer
