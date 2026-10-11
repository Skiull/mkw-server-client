#pragma once

#include <Common.h>

typedef struct {
    u8 _00[0x4c - 0x00];
} RFLiStoreData;
static_assert(sizeof(RFLiStoreData) == 0x4c);

// Unofficial name.
typedef struct {
    u8 _00[0x08 - 0x00];
    RFLiStoreData miis[2];
} RFLWifiData;
static_assert(sizeof(RFLWifiData) == 0xa0);
