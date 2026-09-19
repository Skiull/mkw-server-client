#pragma once

#include <Common.h>

void REPLACED(
        DWCi_Auth_SendRequest)(u32 authStage, wchar_t *miiName, char *gameId, u32 r6, u64 userId);

REPLACE void DWCi_Auth_SendRequest(u32 authStage, wchar_t *miiName, char *gameId, u32 r6,
        u64 userId);
