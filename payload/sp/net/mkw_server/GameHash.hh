#pragma once

#include <Common.hh>

namespace MKWServer::GameHash {

// Hashes the next slice of the game's code. Call every frame.
void Calc();

// Hashes all of it right now (about 25 ms). Used once when connecting.
u64 ComputeNow();

// Fingerprints StaticR.rel's .data in 256 byte blocks, right after loading. Calc() later
// reports which blocks have changed since ('G' 'D' packets), so writes to game data that
// never changes in normal play (cheat codes) show up.
void SnapshotRelData(const void *start, u32 size);

// Hash this word as `original` whatever it holds: for a constant one of our settings
// changes (Fast Menu Transitions), so the hash doesn't depend on the setting.
void HashAsOriginal(const u32 *word, u32 original);

} // namespace MKWServer::GameHash
