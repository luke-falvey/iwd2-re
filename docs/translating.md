# How to translate a function

This is the working guide for translating functions from the original `iwd2.exe` into `src/`. Read [translation-plan.md](translation-plan.md) for the overall plan.

## Local setup (outside the repo, never committed)

| Path | What |
|---|---|
| `C:\iwd2-ref` | Pristine GOG 2.01 install. Read-only reference. Hashes are in [reference-hashes.md](reference-hashes.md). |
| `C:\iwd2-play` | Disposable test copy. Rebuild it with `tools\make_play_dir.ps1`. |
| `C:\iwd2-tools\decomp\0xADDR.c` / `.asm` | Ghidra decompilation and disassembly for each function |
| `C:\iwd2-tools\functions.json` | Every function with name, size, callers, callees and signature |
| `C:\iwd2-tools\venv` | Python for the play tests (Pillow, NumPy, pytest) |

## Tools

- `py -I tools/peek.py s 0x8B0B4C`: read a C string from the exe (assert messages, file names). `d`/`w`/`b`/`f` read dwords, words, bytes and floats; `name 0xADDR` looks up a function.
- `grep -rh -A1 "^// 0xADDR$" src`: what the repo already calls that address.
- `py -I tools/inventory.py`: progress, plus the most-needed untranslated functions.
- `powershell -ExecutionPolicy Bypass -File tools\check.ps1`: build, inventory ratchet and unit tests (including layout).
- Build only: `& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config RelWithDebInfo --target iwd2-re -- /m /nologo /v:minimal`. Run it from PowerShell, because Git Bash turns `/m` into a path.
- Play tests: `C:\iwd2-tools\venv\Scripts\python -m pytest tests/e2e/scenarios/test_s2_new_game.py --target reimpl`. They take over the mouse and keyboard.
- Crash reports: the rebuild writes `iwd2re-crash.txt` (symbolized stack) and `.dmp` to `IWD2RE_CRASH_DIR` or the current directory.

## Workflow per function

1. Read **both** `0xADDR.asm` and `0xADDR.c`. The asm is the truth. Ghidra drops arguments to `__thiscall` calls and almost every call shows up as `Foo()`, so get arguments from the `PUSH`es and `ECX` in the asm.
2. Name every callee with `grep -rh -A1 "^// 0x<callee>$" src`. If a callee isn't in `src`, read its asm. Small ones (< ~60 bytes) are often inlined template helpers you should add. Note larger missing callees in your report rather than translating the whole tree.
3. Map every `[reg + offset]` to an annotated member (`/* 05A0 */ CString m_sName;`). Add missing members to headers with offset comments, replacing `field_XXX` padding or `unknown` arrays at the right position.
4. Use the assert strings: `PUSH 0x8b019c` / `PUSH 0x104c` before `CALL 0x780c00` is `UTIL_ASSERT` at `__FILE__` `peek s 0x8b019c` and `__LINE__` 0x104c. Copy the `// __FILE__:` / `// __LINE__:` comment style used everywhere.
5. Write code in the surrounding style. Keep the original's order of side effects, and don't "fix" original bugs (mark them `// NOTE:` or `// FIXME:`).
6. Put `// 0xADDR` on the line above the definition and remove `// TODO: Incomplete.` once the function is complete.
7. Build. Then run the cheapest test that covers the function.

## Pitfalls found so far

- **Multi-character literals are byte-reversed.** The original compares file signatures as little-endian DWORDs (`CMP [EAX], 0x204D5449` is `"ITM "`). In MSVC `'ITM '` is 0x49544D20, so write `' MTI' /* "ITM " */`. Check the constant in the asm every time.
- **Inlined `CResHelper::SetResRef`.** A block that compares a `CResRef`, calls `CDimm::GetResObject` (0x786DF0) and then `CRes::Request` (0x77E610) is an inlined `SetResRef(resRef, TRUE, TRUE)`. Write it as one call with `// NOTE: Uninline.`
- **Identical code folding.** One address can be the body of many tiny functions (empty virtuals, `CResHelper` getters, destructors). The same `// 0xADDR` in several places is expected. Ghidra names such an address after whichever caller it saw first (e.g. `C2DArray_Destruct`).
- **Struct packing.** The original was built with 2-byte packing, and so is the rebuild: `src/pack_prefix.h` is force-included for every C++ source, includes every system header with the default packing, then switches to `#pragma pack(2)` for the project's own headers (the Windows SDK refuses `/Zp2`). A system header that is first included after the prefix would be packed wrongly, so add new system headers to the prefix. Header offset annotations reflect the packed layout. The unit test `annotated member offsets match the original layout` checks every annotation. Mismatches already known are listed in `tests/layout/known_mismatches.txt`. Don't add new ones, and remove entries you fix.
- **Globals.** `0x8CF6D8` is `g_pChitin` (`cDimm` at +0x542), `0x8CF6DC` is `g_pBaldurChitin` (`m_pObjectGame` at +0x1C54, `m_pEngineSinglePlayer` at +0x1C94). `0x780C00` is the assert handler.
- **Empty CString.** `MOV reg, [0x8C1758]` followed by a store is an inlined default `CString` constructor. `0x8C5CF8` is the empty string `""`.
- **The factory has gaps.** Check `CBaldurChitin::AllocResObject` / `CChitin::AllocResObject` when a resource type silently fails to load.
- **Narrow unsigned values never equal -1.** `BYTE b; if (b == -1)` is always false: `b` is promoted to `int` 255. The original compares the narrow register (`CMP AL,0xff`, `CMP AX,0xffff`), so write `== 0xFF` / `== 0xFFFF`. The same goes for `BYTE`/`WORD`-returning functions (`GetIconIndex() != -1`) and for signed constants (`m_circleFacing == FLY_RIGHT` with `FLY_RIGHT = -1`). Storing -1 in a `BYTE` is fine; only the comparison breaks.
- **Read the signedness from the asm, not from Ghidra's type.** `MOVSX` on load, or `JL`/`JG`/`JLE`/`JGE` after a byte or word `CMP`, means the value is signed (`signed char` / `SHORT`); `MOVZX`, `XOR reg,reg` + `MOV reg8`, `AND reg,0xff` or `JB`/`JA`/`JBE`/`JAE` mean unsigned. A signed member typed `BYTE`/`WORD` breaks range clamps (`x < -40`), `>= 0` checks and every widening read (-1 becomes 255 or 65535). When every access in the exe agrees, fix the member type instead of casting at each use: `grep -h "0x533e\]" C:\iwd2-tools\decomp\*.asm` lists them. The same applies to return values: a callee that only sets `AL` may still be sign-extended by all its callers.
- **Tautologies are transcription errors.** Conditions like `a != 35 || a != 11`, `n < 0 && n >= 9`, `x && 0`, `p.y != -1 && p.y != -1`, a self-assignment (`m_n = m_n` instead of the parameter), `abs(a - b <= 4)` or a `while` whose counter is never incremented have all been found in translated code. Take the branch senses and operands from the asm jump by jump (`JZ`/`JNZ` to the *skip* label means the source condition is inverted).
- **Checking for these mechanically.** `clang-tidy` ships with Visual Studio (`VC\Tools\Llvm\bin`) and parses the tree with `--target=i686-pc-windows-msvc -fms-compatibility -fms-extensions -std=c++17 -D_AFXDLL -D_MBCS -D_DLL -D_MT` plus the MSVC, ATL/MFC and Windows SDK include directories as `-isystem`. Run it per file with `--checks=-*,clang-diagnostic-*,misc-redundant-expression,bugprone-infinite-loop -- -Wall -Wextra -Wtype-limits -Wtautological-unsigned-zero-compare` and look at `tautological-constant-out-of-range-compare`, `tautological-unsigned-zero-compare`, `tautological-overlap-compare`, `self-assign*`, `string-plus-int`, `absolute-value`, `constant-logical-operand` and `sometimes-uninitialized`. The normal MSVC build does not report the out-of-range comparisons. Verify each hit against the asm before changing it; some are faithful (the original really has redundant checks and buffer overruns).
