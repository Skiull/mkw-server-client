#pragma once

#include <Common.h>

typedef struct {
    void *cb;
    void *param;
} callback;

typedef struct {
    char *buffer;
    int size;
    int len;
    int pos;
} GPBuffer;

typedef struct {
    char errorString[256];             // 0x00
    BOOL infoCaching;                  // 0x100
    BOOL infoCachingBuddyAndBlockOnly; // 0x104
    BOOL simulation;                   // 0x108
    BOOL firewall;                     // 0x10C
    u8 _110[0x1f0 - 0x110];
    u32 cmSocket;
    int connectState;
    GPBuffer socketBuffer;
    char *inputBuffer;
    int inputBufferSize;
    GPBuffer outputBuffer;
    // Replaced by UDP Layer
    // SOCKET peerSocket;
    char mHeader[16];
    unsigned short peerPort;
    int nextOperationID;
    int numSearches;

    u32 lastStatusState;
    unsigned int hostIp;
    unsigned int hostPrivateIp;
    unsigned short queryPort;
    unsigned short hostPort;
    unsigned int sessionFlags;

    char richStatus[256];
    char gameType[33];
    char gameVariant[33];
    char gameMapName[33];

    // New Status Info extended info Keys
    void *extendedInfoKeys;

    // Deprecated
    char lastStatusString[33];
    char lastLocationString[256];

    u32 errorCode;
    u32 fatalError;
    void *diskCache;
    void *operationList;
    void *profileList;
    void *peerList;
    void *callbackList;
    void *lastCallback;
    // The game's struct has more fields here than the PC SDK's (the WFC patcher puts
    // updateproBuffer at 0x5e0, and Open Host sent through it works).
    u8 _4fc[0x5e0 - 0x4fc];
    GPBuffer updateproBuffer;
} GPIConnection;
static_assert(offsetof(GPIConnection, cmSocket) == 0x1f0);
static_assert(offsetof(GPIConnection, outputBuffer) == 0x210);
static_assert(offsetof(GPIConnection, updateproBuffer) == 0x5e0);

typedef GPIConnection *GPConnection;

u32 gpProcess(GPIConnection *connection);
int gpiAppendStringToBuffer(GPConnection *connection, GPBuffer *buffer, const char *string);
