// Credits: CLF78: Midnight-Variety-Pack, licensed under the Mozilla Public License 2.0.
// See license in LICENSE-Midnight-Variety-Pack.

#pragma once

#include <Common.hh>

extern "C" {
#include <revolution.h>
}

// Note: DWC's source language is C but is written in C++ here to fit in with the codebase.

// This gets called by NetManager::mainNetworkLoop() upon pressing the create room button
// Patch sets up server browser communication, allowing us to send custom create room requests.
// Patch done here since I don't want to rewrite mainNetworkLoop()
REPLACE bool DWC_SetupGameServer(void *r3, void *r4, void *r5, void *r6, void *r7, void *r8,
        void *r9, void *r10);

REPLACE bool DWC_ConnectToGameServerAsync(s32 friendId, void *r4, void *r5, void *r6, void *r7,
        void *r8, void *r9, void *r10);

REPLACE void DWC_ProcessFriendsMatch();
void REPLACED(DWC_ProcessFriendsMatch)();
