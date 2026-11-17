#ifndef IWD2RE_PLATFORM_POSIX_PLATFORM_H_
#define IWD2RE_PLATFORM_POSIX_PLATFORM_H_

// Internal helpers of the POSIX implementation of the Win32 shim. Not for game
// files.

#include <stddef.h>
#include <stdint.h>

#include <string>

namespace platform {

// Remembers the real command line for GetCommandLineA.
void SetCommandLine(int argc, char** argv);

// Adds a string to the table LoadStringA reads (the Windows build has the
// game's .rc string table; the tests register theirs).
void RegisterStringResource(unsigned int id, const char* text);

// Maps errno to a Win32 error code (for the file functions).
uint32_t ErrnoToWin32(int err);

// Sets the thread's last error (GetLastError) from errno.
void SetLastErrorFromErrno();

} // namespace platform

#endif /* IWD2RE_PLATFORM_POSIX_PLATFORM_H_ */
