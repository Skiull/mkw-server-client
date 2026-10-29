#pragma once

#include <Common.hh>

namespace SP::TestCheats {

// Applies the built-in test cheat codes. Called once StaticR.rel is loaded and patched.
void Apply();
// Runs per-frame (C0) codes. Called every frame.
void RunFrame();

} // namespace SP::TestCheats
