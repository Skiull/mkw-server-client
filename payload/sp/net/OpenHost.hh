#pragma once

namespace SP::OpenHost {

// Sends the Open Host setting to the server ("wl:oh" in a GameSpy updatepro) whenever it
// differs from what this connection was last sent. Call once per frame.
void Calc();

} // namespace SP::OpenHost
