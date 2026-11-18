# macOS port log

## 2026-11-18 (session 1, on the Mac)

- The session was started on the Mac (M1, 8 GB, macOS 27, Apple clang 15.0.0), not the PC.
  Plan revised: Mac-first, Win32-shaped shim instead of `platform::` seams (plan section 0).
- Installed with Homebrew (no admin): cmake, ninja.
- Game data found at `~/Documents/Icewind Dale 2 Complete.app` (GOG, Wine wrapper).

- **P0 passed.** `tools/make_play_dir.py` builds the play folder (POSIX paths in the ini).
- **P1 passed** at 213bb7a9 + fixes: every object compiles with Apple clang 15 (arm64), 0 errors,
  0 pointer-to-int warnings (324 before). Unit tests on the Mac: 147 of 147 (233,469 assertions).
  The layout test returns early when pointers are not 4 bytes.
- **P2 passed:** the game boots to the main menu, starts a new game with a pre-made party,
  shows the Prologue and opens Hedron's dialogue on the Targos docks. Seen through the test
  driver's frame grabs; nobody has listened to the sound yet.
- Shim (`src/platform/posix/`, about 7,500 lines): Win32 headers; files with case-insensitive
  paths; threads, events, critical sections; ini; MSVC `rand`, `_snprintf`; software DirectDraw;
  SDL window, messages and polled input; software DirectSound on an SDL audio stream; a
  DirectPlay that offers no connections (the game needs the object even in single player);
  a signal crash handler (`iwd2re-stacks.txt`); the test driver (`IWD2RE_TEST_DRIVER=<port>`,
  client in `tools/driver.py`, launcher `tools/mac_run.sh`).
- Found on the way: the original parks pointers in 32-bit file-structure fields while saving
  (areas, creatures, the character-slot message). 64-bit builds use tickets (`src/PortPointer.h`).
- `lldb` from the Command Line Tools 15 crashes on this macOS when loading the exe; the crash
  handler's stack file is the way to get a backtrace.
- Agents (Sonnet): 4 runs, about 828,000 tokens in total: shim headers 259k, shim kernel 333k,
  64-bit sites 121k, DirectSound 115k.
- Harness `--driver` mode (merge 1a9a9064): fifth agent, about 104,000 tokens; agent total about
  932,000 across five Sonnet runs.
- First whole-suite run on the Mac, driver mode, `--no-compare`, 2 games at once (3965 s):
  11 scenarios crashed with one bus error in `CVidPoly::FillConvexPoly` (a row offset computed
  in unsigned 32-bit arithmetic, zero-extended when added to a 64-bit pointer). After the fix:
  S1–S16 and S27 run to the end (S12's second half skips), no crashes, no hangs. S17–S26 skip
  because their starting saves are banked from the original on the PC. S28 and S29 fail only
  with "no golden image" (they compare by themselves).
- `kill -USR1 <pid>` appends the main thread's stack to `iwd2re-stacks.txt`; the harness asks
  for it before it kills a game that failed, so a hang leaves a stack too.

**Open:**
- The play suite needs `tests/e2e/golden` (including `golden/bank`) from the PC before any
  scenario counts as passed, and before S17–S26 can run at all.
- Dimmed screens (behind a popup) show a checkerboard: that is the original's software path
  (`CUIPanel::RenderDither`, black on every other pixel). Whether the colours match is for a golden.
- `IDirectDrawSurface::GetDC` (GDI text, only for double-byte fonts) is not supported.
- Focus loss does not pause the game; questions in message boxes are answered "yes".
- `CAITrigger.cpp:107`: `sscanf("%d")` into a `SHORT` (original overrun, left as it is).
- Goldens are only on the PC; `phase-a/stubs-and-layout` is not reachable from this Mac.

### Edits to files the Windows build compiles (to re-check with `tools\check.ps1` on the PC)

- 64-bit sites (merge 62512989): 51 game files, casts through `INT_PTR`/`DWORD_PTR`;
  `CSound::m_nArea` and `CSoundMixer::m_nActiveArea` are `DWORD_PTR`; `%ld` became `%d` for
  `LONG` arguments; `unsigned long` became `DWORD` in `CVidMode`.
- `src/PortPointer.h/.cpp` (new) and their uses in `CGameArea`, `CGameContainer`, `CGameDoor`,
  `CGameTrigger`, `CGameSprite`, `CInfGame`, `CMessage`: identity casts in 32-bit builds.
- `CChitin.cpp` (`TimerFunction` takes `DWORD_PTR`), `CUtil.cpp` (zlib lengths through a local),
  `CBaldurProjector.cpp/.h` (Bink sound-system parameter is `UINT_PTR`), `music/sound.cpp`,
  `compat/cstring.h/.cpp`, `compat/collections.h` (`long` became `LONG`).
- `CMakeLists.txt`, `tests/CMakeLists.txt`: non-Windows branches; `PortPointer.*` added to the
  source list for every build. `tools/gen_layout_asserts.py`: early return on 64-bit.
- `CVidPoly.cpp` (`FillConvexPoly`): the row offset is cast to `LONG` before it is added to the
  surface pointer; the same value in 32-bit builds.
