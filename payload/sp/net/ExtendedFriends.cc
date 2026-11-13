#include "sp/net/ExtendedFriends.hh"

#include "game/net/FriendManager.hh"
#include "revolution/dwc/DWCFriendsMatch.hh"
#include "sp/storage/Storage.hh"

#include <cstdio>
#include <cstring>

namespace SP::ExtendedFriends {

namespace {

constexpr char MAGIC[4] = {'M', 'K', 'X', 'F'};
constexpr u16 VERSION = 1;

#define FRIENDS_DIR L"/mkw-sp"

struct FileHeader {
    char magic[4];
    u16 version;
    u16 licenseIndex;
    u16 slotCount;
    u16 reserved;
};
static_assert(sizeof(FileHeader) == 12);

bool s_uiSafeMode = false;
bool s_reportRealRosterCount = false;

void pathForLicense(u32 licenseIndex, wchar_t *out, size_t outCount) {
    // friends-L0.bin .. friends-L3.bin
    swprintf(out, outCount, FRIENDS_DIR L"/friends-L%u.bin", licenseIndex);
}

void writeFriendCode(Slot &slot, u64 friendCode) {
    memcpy(slot.main, &friendCode, sizeof(friendCode));
    // FriendData flags at 0x10: big-endian u16, bit0 = occupied (see Slot::occupied).
    slot.main[0x10] = 0;
    slot.main[0x11] = 1;
}

} // namespace

Slot s_slots[EXTRA_COUNT];
DWCFriendData s_dwcList[MAX_FRIEND_COUNT];
u32 s_dwcListLen = 0;

bool Slot::occupied() const {
    // Vanilla main-block flags: low bits of u16 at 0x10 mark a present entry.
    u16 flags = static_cast<u16>((main[0x10] << 8) | main[0x11]);
    return (flags & 0x1) != 0;
}

u32 Slot::profileId() const {
    return (static_cast<u32>(main[0x4]) << 24) | (static_cast<u32>(main[0x5]) << 16) |
            (static_cast<u32>(main[0x6]) << 8) | static_cast<u32>(main[0x7]);
}

u64 Slot::friendCode() const {
    u64 code = 0;
    memcpy(&code, main, sizeof(code));
    return code;
}

DWCFriendData Slot::dwcData() const {
    DWCFriendData data{};
    memcpy(&data, secondary, SECONDARY_SIZE);
    return data;
}

void Clear() {
    memset(s_slots, 0, sizeof(s_slots));
    memset(s_dwcList, 0, sizeof(s_dwcList));
    s_dwcListLen = 0;
}

u32 OccupiedCount() {
    u32 n = 0;
    for (u32 i = 0; i < EXTRA_COUNT; i++) {
        if (s_slots[i].occupied()) {
            n++;
        }
    }
    return n;
}

u32 FirstFreeSlot() {
    for (u32 i = 0; i < EXTRA_COUNT; i++) {
        if (!s_slots[i].occupied()) {
            return i;
        }
    }
    return EXTRA_COUNT;
}

bool AddFriendCode(u32 extraIndex, u64 friendCode) {
    if (extraIndex >= EXTRA_COUNT) {
        return false;
    }
    Slot &slot = s_slots[extraIndex];
    memset(&slot, 0, sizeof(slot));
    writeFriendCode(slot, friendCode);
    DWC_CreateFriendKeyToken(reinterpret_cast<DWCFriendData *>(slot.secondary), 0, friendCode);
    return true;
}

void ClearSlot(u32 extraIndex) {
    if (extraIndex >= EXTRA_COUNT) {
        return;
    }
    memset(&s_slots[extraIndex], 0, sizeof(s_slots[extraIndex]));
}

s32 IndexOfFriendCode(u64 friendCode) {
    for (u32 i = 0; i < EXTRA_COUNT; i++) {
        if (s_slots[i].occupied() && s_slots[i].friendCode() == friendCode) {
            return static_cast<s32>(i);
        }
    }
    return -1;
}

void RebuildDwcList(const DWCFriendData *vanilla, int vanillaLen) {
    memset(s_dwcList, 0, sizeof(s_dwcList));

    // Keep roster index == DWC index: [0..29] vanilla, [30..63] extras (empty slots stay zeroed).
    if (vanilla && vanillaLen > 0) {
        u32 take = static_cast<u32>(vanillaLen);
        if (take > VANILLA_FRIEND_COUNT) {
            take = VANILLA_FRIEND_COUNT;
        }
        memcpy(s_dwcList, vanilla, take * sizeof(DWCFriendData));
    }

    for (u32 i = 0; i < EXTRA_COUNT; i++) {
        if (s_slots[i].occupied()) {
            s_dwcList[VANILLA_FRIEND_COUNT + i] = s_slots[i].dwcData();
        }
    }

    s_dwcListLen = MAX_FRIEND_COUNT;
}

void RebuildDwcList(const Net::FriendRosterManager *mgr) {
    if (!mgr) {
        RebuildDwcList(nullptr, 0);
        return;
    }

    DWCFriendData vanilla[VANILLA_FRIEND_COUNT]{};
    for (u32 i = 0; i < VANILLA_FRIEND_COUNT; i++) {
        if (const DWCFriendData *src = mgr->primaryFriend(i)) {
            vanilla[i] = *src;
        }
    }
    RebuildDwcList(vanilla, VANILLA_FRIEND_COUNT);
}

void SetUiSafeMode(bool enabled) {
    s_uiSafeMode = enabled;
}

bool UiSafeMode() {
    return s_uiSafeMode;
}

void SetReportRealRosterCount(bool enabled) {
    s_reportRealRosterCount = enabled;
}

bool ReportRealRosterCount() {
    return s_reportRealRosterCount;
}

bool Load(u32 licenseIndex) {
    Clear();
    if (licenseIndex >= 4) {
        return false;
    }
    if (!Storage::HasWritable()) {
        return false;
    }

    wchar_t path[64];
    pathForLicense(licenseIndex, path, std::size(path));
    auto file = Storage::Open(path, "r");
    if (!file) {
        return true; // No extras yet: vanilla 30-friend save is fine.
    }

    FileHeader header{};
    if (!file->read(&header, sizeof(header), 0)) {
        SP_LOG("ExtendedFriends: failed to read header for license %u", licenseIndex);
        return false;
    }
    if (memcmp(header.magic, MAGIC, 4) != 0 || header.version != VERSION ||
            header.licenseIndex != licenseIndex || header.slotCount > EXTRA_COUNT) {
        SP_LOG("ExtendedFriends: bad header for license %u", licenseIndex);
        return false;
    }

    const u32 bytes = static_cast<u32>(header.slotCount) * SLOT_SIZE;
    if (file->size() < sizeof(header) + bytes) {
        SP_LOG("ExtendedFriends: truncated file for license %u", licenseIndex);
        return false;
    }
    if (bytes > 0 && !file->read(s_slots, bytes, sizeof(header))) {
        SP_LOG("ExtendedFriends: failed to read slots for license %u", licenseIndex);
        Clear();
        return false;
    }

    SP_LOG("ExtendedFriends: loaded %u extra friends for license %u", OccupiedCount(),
            licenseIndex);
    RebuildDwcList(Net::FriendRosterManager::Instance());
    return true;
}

bool Save(u32 licenseIndex) {
    if (licenseIndex >= 4) {
        return false;
    }
    if (!Storage::HasWritable()) {
        return false;
    }

    Storage::CreateDir(FRIENDS_DIR, true);

    FileHeader header{};
    memcpy(header.magic, MAGIC, 4);
    header.version = VERSION;
    header.licenseIndex = static_cast<u16>(licenseIndex);
    header.slotCount = EXTRA_COUNT;
    header.reserved = 0;

    u8 buffer[sizeof(FileHeader) + sizeof(s_slots)];
    memcpy(buffer, &header, sizeof(header));
    memcpy(buffer + sizeof(header), s_slots, sizeof(s_slots));

    wchar_t path[64];
    pathForLicense(licenseIndex, path, std::size(path));
    if (!Storage::WriteFile(path, buffer, sizeof(buffer), true)) {
        SP_LOG("ExtendedFriends: failed to write %ls", path);
        return false;
    }
    return true;
}

} // namespace SP::ExtendedFriends
