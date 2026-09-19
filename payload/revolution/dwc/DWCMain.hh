#pragma once

#include <Common.hh>

extern "C" {
#include "revolution/dwc/DWCMain.h"
}

// Note: DWC's source language is C but is written in C++ here to fit in with the codebase.

// See MatchMaking.md for bellow replacements

// 0x800d2158
// Called by NetManager::mainNetworkLoop() to get DWC to clean up a few things.
// Patch may not be needed but added to be safe.
REPLACE u32 DWC_GetNumConnectionsHost();

// 0x800d4994
// Called by NetManager::updateStatusData()
REPLACE u32 DWC_GetGroupId();

// 0x800d49b4
// Called by NetManager::mainNetworkLoop() to check match suspension before voting MM suspension
REPLACE u8 DWC_GetSuspendMatch();

// 0x800d41f4
// Also called by NetManager::mainNetworkLoop() to perform some clean up.
// Added to be safe.
REPLACE u8 DWC_IsValidMatchCancel();

// 0x800d49b0
// Patched out to send suspension request to MKW-Server
REPLACE bool DWC_RequestSuspendMatchAsync(bool suspendVote);
