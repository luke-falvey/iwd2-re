# Port audit: what ties the code to 32-bit Windows

Read-only count taken on 2026-11-16 at commit 56098996 (437 files and about 411,000 lines in `src/`).
"Uses" are text matches from grep, so they are sizes of the job, not exact call counts.

## Summary

| Area | Uses | Files | How it is used | Size of the port work |
|---|---|---|---|---|
| MFC value types (`CString`, `CPoint`/`CRect`/`CSize`) | 11,557 | about 250 | Everywhere in game logic | Small if replaced by same-named classes in a compatibility header; no call sites change |
| MFC collections (`CPtrList`, `CTypedPtrList`, `CObList`, `CArray`, `CMap`) | 468 | about 100 | Lists of objects, `POSITION` iteration | Small: same-named replacements |
| MFC files (`CFile`, `CFileFind`, `CFileException`) | 124 | 18 | Resource and save I/O | Small |
| MFC sync and `Afx*` (`CCriticalSection`, `CSingleLock`, `CEvent`) | 347 | 76 | Locks around shared game state | Small: wrappers over standard C++ |
| MFC windows (`CWnd`, `CWinApp`, `CDC`) | 9 | 7 | Almost unused | Trivial |
| Win32 threads, events, `Sleep`, waits | 449 | 30 | Five threads from `CChitin` (resource service, messages, music, main AI, path search) and one multimedia timer | Medium: the pacing between the timer, AI and render threads must be kept |
| Win32 time (`GetTickCount`, `timeGetTime`) | 103 | 20 | Timing and timeouts | Small |
| Win32 window, messages, GDI | 65 | 13 | `CChitin` window and message loop | Medium, concentrated in `CChitin.cpp` |
| Win32 input (`GetCursorPos`, `GetAsyncKeyState`, `GetKeyState`) | 19 | 3 | Polled, not message-driven | Small (the test hook already replaces these five imports) |
| Win32 files and directories | 120 | 17 | Save folders, cache, temp | Medium: paths assume backslashes and a case-insensitive file system |
| ini and registry | 290 | 18 | `icewind2.ini` options through `GetPrivateProfile*` | Small: one ini reader |
| IME, clipboard, version, affinity | 118 | 11 | `CImm.cpp` (Asian input), misc | Small; IME can be stubbed |
| DirectDraw | 163 | 21 | `CVidMode.cpp` (4,820 lines), `CVidInf.cpp` (2,661), `CVideo.cpp` | Large: the main video path |
| OpenGL (`gl*`, `wgl*`) | 329 | 10 | `CVideo3d.cpp` (2,626), `CChitin3d.cpp`: the game's optional 3D-accelerated path | Could be dropped at first (software path only), or reused later since it is already GL |
| DirectSound | 21 (+ `src/music/sound.cpp`) | 7 | `CSoundMixer.cpp`, `CSound.cpp`, and the music player's own streaming layer | Medium |
| Music (`src/music/`, about 5,100 lines) | n/a | 5 | Own ACM decoder and streaming, plain C style; only the output goes to DirectSound | Small once sound output is replaced |
| Movies | 39 | 2 | `CBaldurProjector` calls Bink through function pointers loaded from `binkw32.dll` | Medium: that DLL is 32-bit x86 and proprietary; a Mac build needs another decoder (FFmpeg has one) or movies skipped |
| DirectPlay | 26 direct, about 1,950 `cNetwork.` uses in 52 files | 3 direct | `CNetwork.cpp` (4,204 lines) wraps DirectPlay; game code only talks to `CNetwork` | Stub `CNetwork` for single-player first; a rewrite on sockets is a separate project |
| Pointer stored in a 32-bit integer | 301 casts | 38 | `CGameSprite.cpp` 73, `CGameArea.cpp` 60, `CAIGroup.cpp` 31; many are sound-channel or message parameters | Medium: each needs a look; the compiler finds them all in a 64-bit build |
| Integer cast to pointer | about 790 matches | 8 | Mostly list `POSITION` and message parameters | Same as above |
| `#pragma pack` | 19 | 6 | File structures (plus the project-wide 2-byte packing flag) | Keep for on-disk structures only |
| Inline assembly | 0 | 0 | The original's `__asm` blitters are already written as C++ in `CVidBlitter.cpp` and `CVidMode.cpp` | None |
| DBCS string functions (`_mbs*`) | 32 | 6 | Text handling for Asian builds | Small |

## What this says about the plan

- **Game logic is tied to MFC by name only.** Over 12,000 uses, but of a narrow set of classes
  (`CString`, three geometry types, a few collections, files, locks). A compatibility header that
  defines the same names would let nearly all 250 files compile unchanged. This is the lowest-risk
  first step and can be checked on Windows by building against the replacement instead of MFC.
- **The platform code is concentrated.** Video, sound, window, input, threads and network live in
  about a dozen files: `CChitin.cpp`, `CVidMode.cpp`, `CVidInf.cpp`, `CVideo.cpp`, `CVideo3d.cpp`,
  `CChitin3d.cpp`, `CSoundMixer.cpp`, `CSound.cpp`, `music/sound.cpp`, `CNetwork.cpp`,
  `CBaldurProjector.cpp`, `CImm.cpp`. Together roughly 20,000 lines of 411,000.
- **Two external pieces have no Mac equivalent:** Bink (movies) and DirectPlay (multiplayer). Both
  sit behind a single class, so they can be stubbed without touching callers.
- **The 64-bit work is bounded and compiler-driven:** about 300 pointer-to-integer casts in 38
  files, plus the layout change. This is the part that breaks the link to the original exe and
  should wait.
- **Not measured:** how many structures that are written to disk contain pointers or rely on
  member order shared with in-memory classes; how the software blitters depend on the DirectDraw
  surface format; byte-order assumptions (not a concern for Apple Silicon, which is little-endian).

## Low-risk steps available now (behaviour unchanged, play tests still apply)

1. Build with clang on Windows (`clang-cl`, 32-bit, still with MFC) and fix what it rejects. Needs
   the Visual Studio "C++ Clang tools" component, which is not installed.
2. Add a compatibility header with the MFC names used by game logic, selectable at build time, and
   get the Windows build passing the play tests against it.
3. Put window, input, timer and thread creation behind one small interface each, implemented with
   the existing Win32 calls.
4. Do the same for sound output and the video surface calls.
5. Compile-only x64 build to list every pointer-size problem, without changing the code yet.

## Step 2: building without MFC (2026-11-16)

`cmake -DIWD2RE_NO_MFC=ON` builds the game and the tests without MFC. `src/mfc.h` then includes
`src/compat/afxcompat.h`, which defines the MFC names the game uses: `CString`, `CPoint`, `CSize`,
`CRect`, the lists, arrays and maps, `CFile`, `CFileFind`, `CFileStatus`, `CFileException`,
`CException`, `CTime`, `CCriticalSection`, `CSingleLock`, `CObject`, `CWnd` (handle holder only),
`AfxIsValidAddress` and `AfxSetResourceHandle`. The default build is unchanged.

- Every class has the size and member order of the installed MFC class, so all 5,868 annotated
  member offsets come out the same in both builds.
- `tests/unit/test_compat_*.cpp` run in both builds. They pass against MFC, which shows they
  describe MFC, and against the replacement, which shows it behaves the same. They cover details
  the game depends on without saying so: the capacity `CString::GetAllocLength` reports (game
  code copies that many bytes), `+=` stopping at a terminator inside the appended string, the
  order in which maps are iterated, list nodes being reused, and which wrong arguments throw.
- Win32 calls of the replacement are confined to `src/compat/cstring.cpp`, `cfile.cpp` and the
  inline bodies of the lock and window classes.
- Not covered by unit tests: the game itself running on the replacement. That needs the play
  tests against `build-nomfc\RelWithDebInfo\iwd2-re.exe`.
