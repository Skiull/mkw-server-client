#include "GameHash.hh"
#include "GameHash.h"

#include "sp/net/mkw_server/MKWServer.hh"

extern "C" {
#include "sp/Dol.h"
#include "sp/Payload.h"
#include "sp/Rel.h"
}

// Hashes the game's code and read-only data (main.dol and StaticR.rel, after this client's
// own patches) and this client's own code (so patching this hash shows up too) and reports it to mkw-server, which compares it against known-good values.
// Cheat codes patch code or constants, so they change the hash.
//
// A little is hashed every frame so there's never a hitch: 64 KiB per frame, a full pass in
// about 1.5 seconds. Each finished pass is sent as 'G' 'H' + 8 byte hash (big endian).

namespace MKWServer::GameHash {

static constexpr u32 BYTES_PER_FRAME = 64 * 1024;
static constexpr u64 FNV_OFFSET = 0xcbf29ce484222325ull;
static constexpr u64 FNV_PRIME = 0x100000001b3ull;

struct Region {
    const u32 *start, *end;
};

static Region s_regions[3];

static const u32 *s_maskedWord = nullptr;
static u32 s_maskedOriginal = 0;

void HashAsOriginal(const u32 *word, u32 original) {
    s_maskedWord = word;
    s_maskedOriginal = original;
}

static inline u32 hashedWord(const u32 *p) {
    return p == s_maskedWord ? s_maskedOriginal : *p;
}

// StaticR.rel .data blocks (see SnapshotRelData).
static constexpr u32 DATA_BLOCK_WORDS = 256 / sizeof(u32);
static constexpr u32 MAX_DATA_BLOCKS = 1024;
static const u32 *s_data = nullptr;
static u32 s_dataBlocks = 0;
static u64 s_dataSnapshot[MAX_DATA_BLOCKS];
static u8 s_dataChanged[MAX_DATA_BLOCKS / 8];
static u32 s_dataBlock = 0; // Next block to check; s_dataBlocks when not in a data pass.

static u64 hashBlock(u32 block) {
    u64 hash = FNV_OFFSET;
    const u32 *p = s_data + block * DATA_BLOCK_WORDS;
    for (u32 i = 0; i < DATA_BLOCK_WORDS; i++) {
        hash = (hash ^ p[i]) * FNV_PRIME;
    }
    return hash;
}

void SnapshotRelData(const void *start, u32 size) {
    s_data = reinterpret_cast<const u32 *>(start);
    s_dataBlocks = size / (DATA_BLOCK_WORDS * sizeof(u32));
    if (s_dataBlocks > MAX_DATA_BLOCKS) {
        s_dataBlocks = MAX_DATA_BLOCKS;
    }
    for (u32 b = 0; b < s_dataBlocks; b++) {
        s_dataSnapshot[b] = hashBlock(b);
    }
    s_dataBlock = s_dataBlocks;
}

// Sends 'G' 'D', the block count (u16), then a bit per block (set: changed since load).
static void sendDataChanges() {
    u8 packet[4 + MAX_DATA_BLOCKS / 8] = {'G', 'D', static_cast<u8>(s_dataBlocks >> 8),
            static_cast<u8>(s_dataBlocks)};
    for (u32 i = 0; i < MAX_DATA_BLOCKS / 8; i++) {
        packet[4 + i] = s_dataChanged[i];
    }
    trySendRacePacketToMKWServer(packet, 4 + (s_dataBlocks + 7) / 8);
}

// Checks the next few .data blocks. Returns true while a data pass is under way.
static bool calcData(u32 &budget) {
    while (budget >= DATA_BLOCK_WORDS && s_dataBlock < s_dataBlocks) {
        u32 b = s_dataBlock++;
        u8 bit = 1 << (7 - b % 8);
        if (hashBlock(b) != s_dataSnapshot[b]) {
            s_dataChanged[b / 8] |= bit;
        } else {
            s_dataChanged[b / 8] &= ~bit;
        }
        budget -= DATA_BLOCK_WORDS;
        if (s_dataBlock == s_dataBlocks && hasMKWServerAddress()) {
            sendDataChanges();
        }
    }
    return s_dataBlock < s_dataBlocks;
}
static u32 s_region = 0;
static const u32 *s_cursor = nullptr;
static u64 s_hash = FNV_OFFSET;

static void reset() {
    s_regions[0] = {reinterpret_cast<const u32 *>(Dol_getInitSectionStart()),
            reinterpret_cast<const u32 *>(dol_rodata_end)};
    s_regions[1] = {reinterpret_cast<const u32 *>(Rel_getTextSectionStart()),
            reinterpret_cast<const u32 *>(Rel_getRodataSectionEnd())};
    s_regions[2] = {reinterpret_cast<const u32 *>(payload_text_start),
            reinterpret_cast<const u32 *>(payload_rodata_end)};
    s_region = 0;
    s_cursor = s_regions[0].start;
    s_hash = FNV_OFFSET;
}

static void send(u64 hash) {
#ifdef TEST_FAKE_INROOM_HASH
    hash = TEST_FAKE_INROOM_HASH; // Test build: pretend to be a clean client.
#endif
    u8 packet[10] = {'G', 'H'};
    for (u32 i = 0; i < 8; i++) {
        packet[2 + i] = hash >> (56 - 8 * i);
    }
    trySendRacePacketToMKWServer(packet, sizeof(packet));
}

// The hash sent at login, also reported in rooms ('G' 'L') so the server can learn it from
// trusted players for the login check.
static u64 s_loginHash = 0;

u64 ComputeNow() {
#ifdef TEST_FAKE_LOGIN_HASH
    s_loginHash = TEST_FAKE_LOGIN_HASH; // Test build: pretend to be a clean client.
    return s_loginHash;
#endif
    reset();
    u64 hash = FNV_OFFSET;
    for (const Region &r : s_regions) {
        for (const u32 *p = r.start; p < r.end; p++) {
            hash = (hash ^ hashedWord(p)) * FNV_PRIME;
        }
    }
    s_loginHash = hash;
    return hash;
}

void Calc() {
    if (!s_cursor) {
        reset();
    }
    u32 budget = BYTES_PER_FRAME / sizeof(u32);
    if (calcData(budget)) {
        return;
    }
    while (budget > 0) {
        const Region &r = s_regions[s_region];
        while (budget > 0 && s_cursor < r.end) {
            s_hash = (s_hash ^ hashedWord(s_cursor)) * FNV_PRIME;
            s_cursor++;
            budget--;
        }
        if (s_cursor < r.end) {
            return;
        }
        if (++s_region < sizeof(s_regions) / sizeof(s_regions[0])) {
            s_cursor = s_regions[s_region].start;
            continue;
        }
        // Finished a pass. The .data blocks are checked next.
        if (hasMKWServerAddress()) {
            send(s_hash);
            if (s_loginHash != 0) {
                // 'G' 'L' + login hash (u64) + build number (u32, see build.py).
                static constexpr u32 BUILD_ID =
#include <BuildId.inc>
                        ;
                u8 packet[14] = {'G', 'L'};
                for (u32 i = 0; i < 8; i++) {
                    packet[2 + i] = s_loginHash >> (56 - 8 * i);
                }
                for (u32 i = 0; i < 4; i++) {
                    packet[10 + i] = BUILD_ID >> (24 - 8 * i);
                }
                trySendRacePacketToMKWServer(packet, sizeof(packet));
            }
        }
        reset();
        s_dataBlock = 0;
        return;
    }
}

} // namespace MKWServer::GameHash

namespace MKWServer::GameHash {

// Challenges: mkw-server sends 'H' 'C' + seed (u32) and gets back 'H' 'R' + seed + a hash of
// 64 pseudo-random 256 byte pieces of the hashed regions, picked by the seed. A client that
// fakes its game hash can't answer without a clean copy of the game in memory.
static constexpr u32 CHALLENGE_PIECES = 64;
static constexpr u32 CHALLENGE_PIECE_WORDS = 256 / sizeof(u32);

static u64 answerChallenge(u32 seed) {
    if (!s_cursor) {
        reset();
    }
    u32 totalWords = 0;
    for (const Region &r : s_regions) {
        totalWords += r.end - r.start;
    }
    u64 hash = (FNV_OFFSET ^ seed) * FNV_PRIME;
    u32 x = seed * 2654435761u + 1;
    for (u32 i = 0; i < CHALLENGE_PIECES; i++) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        u32 word = x % (totalWords - CHALLENGE_PIECE_WORDS);
        for (const Region &r : s_regions) {
            u32 size = r.end - r.start;
            if (word < size) {
                const u32 *p = r.start + word;
                for (u32 k = 0; k < CHALLENGE_PIECE_WORDS && p + k < r.end; k++) {
                    hash = (hash ^ p[k]) * FNV_PRIME;
                }
                break;
            }
            word -= size;
        }
    }
    return hash;
}

static void handleChallenge(const u8 *seedBytes) {
    u32 seed = seedBytes[0] << 24 | seedBytes[1] << 16 | seedBytes[2] << 8 | seedBytes[3];
    u64 hash = answerChallenge(seed);
    u8 packet[14] = {'H', 'R', seedBytes[0], seedBytes[1], seedBytes[2], seedBytes[3]};
    for (u32 i = 0; i < 8; i++) {
        packet[6 + i] = hash >> (56 - 8 * i);
    }
    trySendRacePacketToMKWServer(packet, sizeof(packet));
}

} // namespace MKWServer::GameHash

bool GameHash_isChallengePacket(const u8 *message, s32 len) {
    return len == 6 && message[0] == 'H' && message[1] == 'C';
}

void GameHash_handleChallengePacket(const u8 *message, s32 len) {
    if (GameHash_isChallengePacket(message, len)) {
        MKWServer::GameHash::handleChallenge(message + 2);
    }
}
