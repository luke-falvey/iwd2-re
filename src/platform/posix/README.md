# src/platform/posix

Implementations of the Win32, DirectDraw, DirectSound and MSVC CRT subset declared in
`include/win32shim/`, for non-Windows builds (see `docs/macos-port-plan.md`, section 0).
Every `.cpp` here is compiled into `iwd2-platform`. These files are built with natural
packing; the game's 2-byte packing applies only to the game's own headers, so don't include
game headers here.
