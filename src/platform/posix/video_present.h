#ifndef IWD2RE_PLATFORM_POSIX_VIDEO_PRESENT_H_
#define IWD2RE_PLATFORM_POSIX_VIDEO_PRESENT_H_

// What the window (window_sdl.cpp) and the software DirectDraw
// (ddraw_soft.cpp) need from each other. Not seen by game code.

#include <stdint.h>

#include <vector>

namespace platform {

// Size of the desktop the game is told about (SM_CXSCREEN and friends, and
// the DirectDraw display mode in windowed mode), in pixels of the game.
void GetVirtualScreenSize(int& w, int& h);

// The window's client area, in virtual screen coordinates.
void GetWindowClientRect(int& x, int& y, int& w, int& h);

// Show a finished frame: `w` x `h` pixels of X8R8G8B8, `pitch` bytes per
// row. Callable from any thread; the frame reaches the screen at once when
// called on the main thread and at the next message pump otherwise.
void PresentFrame(const uint32_t* pixels, int w, int h, int pitch);

// The last presented frame and how many frames have been presented (for
// the test driver).
uint32_t GrabFrame(std::vector<uint32_t>& pixels, int& w, int& h);

} // namespace platform

#endif /* IWD2RE_PLATFORM_POSIX_VIDEO_PRESENT_H_ */
