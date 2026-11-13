#pragma once

#include <Common.hh>

#include <game/net/FriendLimits.hh>
#include <revolution/dwc/DWCFriend.h>

namespace Net {
class FriendRosterManager;
}

namespace SP::ExtendedFriends {

// Slots 0..29 stay in rksys.dat (vanilla). Slots 30..63 are stored here so existing
// licenses keep working: an old build never sees this file and still has a valid 30-friend
// roster; a new build merges both.
static constexpr u32 EXTRA_COUNT = EXTRA_FRIEND_COUNT;
static constexpr u32 FIRST_EXTRA_INDEX = VANILLA_FRIEND_COUNT;

// Same sizes as one rksys.dat friend (main + secondary DWC block). See
// docs/design_docs/ExtendedFriendRoster.md and https://mkwiiki.org/wiki/Rksys.dat
static constexpr u32 MAIN_SIZE = 0x1c0;
static constexpr u32 SECONDARY_SIZE = 0xc;
static constexpr u32 SLOT_SIZE = MAIN_SIZE + SECONDARY_SIZE;

struct Slot {
    u8 main[MAIN_SIZE];
    u8 secondary[SECONDARY_SIZE];

    bool occupied() const;
    u32 profileId() const;
    u64 friendCode() const;
    DWCFriendData dwcData() const;
};

// In-memory copy for the current license. Empty until Load() succeeds or clears.
extern Slot s_slots[EXTRA_COUNT];

// Contiguous AccFriendData[64] for DWC_InitFriendsMatch (vanilla 0..29 + extras).
extern DWCFriendData s_dwcList[MAX_FRIEND_COUNT];
extern u32 s_dwcListLen;

// Load /mkw-sp/friends-L{license}.bin for this license. Missing file => all empty
// (compatible with a vanilla 30-friend save).
bool Load(u32 licenseIndex);

// Write the current extra slots. Creates /mkw-sp if needed.
bool Save(u32 licenseIndex);

// Clear in-memory extras (e.g. on license switch before Load).
void Clear();

// How many extra slots currently hold a friend.
u32 OccupiedCount();

// First free extra index in [0, EXTRA_COUNT), or EXTRA_COUNT if full.
u32 FirstFreeSlot();

// Write friend code into an empty extra slot and mark occupied.
bool AddFriendCode(u32 extraIndex, u64 friendCode);

void ClearSlot(u32 extraIndex);

// Index in [0, EXTRA_COUNT) for this friend code, or -1.
s32 IndexOfFriendCode(u64 friendCode);

// Rebuild s_dwcList from the game's vanilla AccFriendData list + occupied extras.
// vanilla/vanillaLen are whatever DWC_InitFriendsMatch would have received (≤30).
void RebuildDwcList(const DWCFriendData *vanilla, int vanillaLen);

// Same, pulling primaries from FriendMgr (friends[i][0], stride 0x18).
void RebuildDwcList(const Net::FriendRosterManager *mgr);

// While true, FriendList::GetFriendCount hides extras (Pages::FriendList sortedFriendsIdx[30]).
void SetUiSafeMode(bool enabled);
bool UiSafeMode();

// While true, GetFriendCount returns the real vanilla+extra total (for page counter).
void SetReportRealRosterCount(bool enabled);
bool ReportRealRosterCount();

} // namespace SP::ExtendedFriends
