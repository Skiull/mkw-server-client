#pragma once

#include <Common.hh>

extern "C" {
#include <revolution.h>
}

// Note: DWC's source language is C but is written in C++ here to fit in with the codebase.

REPLACE BOOL DWCi_GT2UnrecognizedMessageCallback(GT2Socket socket, u32 ip, u16 port,
        const u8 *message, s32 len);
BOOL REPLACED(DWCi_GT2UnrecognizedMessageCallback)(GT2Socket socket, u32 ip, u16 port,
        const u8 *message, s32 len);
