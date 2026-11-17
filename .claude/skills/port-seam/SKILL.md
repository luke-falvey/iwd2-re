---
name: port-seam
description: Move one group of Windows-only calls (time, threads, files, ini, window/input, sound, video, movies, network) behind an interface in src/platform/, with a Win32 implementation that keeps today's behaviour exactly. Use for macOS port milestone M1, or when adding a second (software or SDL) implementation of an existing seam.
---

# Put a platform dependency behind a seam

The port (see `docs/macos-port-plan.md`) only works if the default build keeps behaving the same
while Windows calls move behind interfaces. The play tests compare against goldens from the
original game; they are the judge, and they only stay valid if nothing visible changes.

## Inputs you are given

- The group: which calls, or which subsystem.
- The worktree and branch to work in.
- The files you own. Edit only those, plus new files under `src/platform/`.

If any of these is missing, ask once in your report and stop.

## Rules

1. **Behaviour first, API second.** Before writing the interface, write down what the game
   relies on from each call. Read every call site (`Grep`) and note:
   - return values used;
   - error paths;
   - units (milliseconds, hundredths of a decibel, pixels);
   - wrap-around;
   - blocking and recursion;
   - thread affinity: which thread calls it.
   Put these notes in a comment block at the top of the interface header. Examples:
   - `GetTickCount` wraps at 2^32 and callers subtract unsigned.
   - `CRITICAL_SECTION` is recursive.
   - `GetAsyncKeyState`'s low bit means "pressed since the last call" and is consumed by
     reading it (see `PollKey`/`PollButton` in `CChitin.cpp`).
   - `GetPrivateProfileInt` parses a leading number and ignores the rest.
2. **Interfaces are small and C++-plain.** Free functions or small classes in `namespace
   platform`, in `src/platform/<group>.h`.
   - No `<windows.h>` in the header. Use fixed-width types (`uint32_t`, `int32_t`), never `long`.
   - Win32 implementation in `src/platform/win32/<group>.cpp`; other implementations in
     `src/platform/soft/` or `src/platform/sdl/`.
   - Selected by CMake (`IWD2RE_PLATFORM`), never by `#ifdef` inside game files.
3. **Game files change only at the call.** Replace `::GetTickCount()` with
   `platform::TickMs()` and so on.
   - Keep the `// 0xADDR` and `/* OFFS */` annotations and the surrounding code as it is.
   - Don't reformat, rename or tidy anything else: each extra line is a merge conflict with the
     main session.
   - Don't change a class's members (layout asserts guard the 32-bit layout). Where a member
     holds a Windows handle, keep its type and size, and store the platform object's handle or
     index in it.
4. **The Win32 implementation is the old code, moved.** Same calls, same arguments, same order.
   If the old code had a quirk, the implementation keeps it.
5. **Mark intended differences** in non-default implementations with
   `// NOTE: Port: <why>`. The default build has none.
6. **Unit tests** in `tests/unit/test_platform_<group>.cpp` (doctest, as the existing tests):
   - Where the Windows API can be called in a test (time, ini, files, sync), test the
     platform function against the real API on Windows. Then the same test describes the
     behaviour that any later implementation must match.
   - For subsystems that can't be unit tested (window, video, sound), say so in the report and
     rely on the play tests.
7. **Never** rebuild or run anything in `C:\Users\Luke\Projects\iwd2-re`, run the original
   game, touch `C:\GOG Games`, push, or spawn subagents.

## Steps

1. List every call site in the group (`Grep` over `src/`, including `src/compat/` and
   `src/music/`). Count them; if it is over about 150, report and propose a split.
2. Write the behaviour notes (rule 1) and the interface header.
3. Write the Win32 implementation (rule 4) and the unit tests (rule 6).
4. Replace the call sites (rule 3).
5. Build every configuration that exists in the worktree, at below-normal priority, in the
   foreground. Start the PowerShell command with
   `(Get-Process -Id $PID).PriorityClass = 'BelowNormal';`. Then run:
   - `tools\check.ps1` (default build: must pass, layout asserts included);
   - `tools\check.ps1 -BuildDir build-nomfc`;
   - any `build-sdl` / `build-x64` that is configured.
6. Check that nothing outside `src/platform/win32/` still calls the group's Windows functions.
   This is a `Grep` that should return no lines; put it in the report.
7. Commit on your branch if you were told to, ending the message with the attribution line
   from your instructions.

## Report

- **Group and interface:** the header path and its functions.
- **Behaviour notes:** a short list.
- **Files changed,** with the call-site count for each.
- **Tests:** added, and their results.
- **Builds:** each configuration and its result.
- **The leftover-calls grep:** command and output.
- **Doubts:** anything whose behaviour you could not pin down, with the file and line.

Play tests are not your job; the orchestrating session runs them (`port-play-test`).

## Notes for particular groups

- **Threads and timer:**
  - `CChitin` starts four threads with `_beginthread` and drives the game with `timeSetEvent`
    (`TIMER_UPDATES_PER_SECOND`).
  - `music/sound.cpp` has its own `timeSetEvent` for fades.
  - The order in which these start, and the timer's period and resolution, are part of the
    behaviour.
  - Thread priorities, if set, too.
- **Video:** don't start without the orchestrating session's design. Settled so far:
  - DirectDraw surfaces become a surface type with lock/unlock, blits (colour key, mirrored,
    clipped), fills and present.
  - The software implementation must draw exactly what DirectDraw drew. The proof is the full
    play suite passing on it with the goldens unchanged.
- **Sound:** add a call log (env var `IWD2RE_SOUND_LOG=<file>`): one line per seam call with
  its arguments. It is how two implementations are compared, since no test hears sound.
