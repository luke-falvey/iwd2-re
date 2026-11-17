// Software mixer behind the DirectSound shim (see dsound_soft.cpp).
#pragma once
#include <stdint.h>

namespace platform {

// Mixes `frames` frames of every playing buffer into `out` (interleaved stereo S16, 44100 Hz)
// and advances the buffers. No SDL in here; the SDL audio callback and the unit tests call it.
void MixSound(int16_t* out, int frames);

// Test hook: when true, buffers advance only through MixSound, never from the clock of the
// device-less mode.
void SoundSetManualClock(bool manual);

} // namespace platform
