# Session log

Newest first. Each entry covers what was done, what's blocked and what's next.

## 2026-11-12: batches 5–12

Branch `phase-a/stubs-and-layout` (local only). 7,355 of 9,831 functions without a TODO (74.8%); 78.1% of bytes. S1–S4 pass on the rebuild.

**Done** (each batch translated by `iwd2-translator`, checked by `iwd2-verifier`, differences confirmed against the listing before applying)
- `CStore` add/remove/marshal helpers, `DoItemRandomize`, and the rest of `CScreenStore::UpdateStoreItems`.
- Spellbook screen: all main functions, `sub_66A8C0`, and the polygon call in the spell button `Render`.
- Inventory screen: every former stub, including `SwapWithSlot` (4.4 KB), plus `CInfGame::SwapItemPersonal` and `FindEmptyItemPersonalInventory`.
- Character screen: five panel functions.
- Rules: starting hit points (`sub_542CB0`, `sub_542B40`, `sub_542A90`), `GetIntModMaxAllowedSpellLevel`, `sub_546B60`.
- Sprite: `sub_718390`, `CheckQuickLists`, `SelectWeaponAbility`, `sub_723830`.
- Video: `CVidInf::FXRender` for polygons, `CVidPoly::FillPoly` and its four helpers with the edge cache.
- Fixes to older code that verifiers pointed at: `CStore::GetItem` lore-flag test, `CMessageColorChange` constructor NULL check.

**Next**
1. Batches 13 (fourteen `CGameSprite` stubs) and 14 (`CInfGame`, `CGameArea`, `CGameAIBase`, `CInfinity`) are in the `batch3` / `batch4` worktrees on `wip/batch3` / `wip/batch4`. Verify, merge, run S1–S4.
2. Write scenario S5 (walk, open a container or the inventory) to find the next runtime blockers. Translation alone no longer shows what is broken.
3. The 2,117 functions that are not in the repo at all. `tools/worklist.py` can't place about 250 KB of them in a file because nothing translated calls them yet.

**Notes**
- Unverified: the ability-score members in `FileFormat.h` may be mislabelled (the byte called `m_WISBase`, at base stats +0x262, sits where the creature format usually has Constitution). Code is translated by offset, so behaviour isn't affected.
- Unverified: `UpdateLabel` is declared static in the repo, but the original pushes `this` as a stack argument (cdecl member). Callers and callee agree with each other.
- Several callees are declared with `const unsigned int&` where the original reads a byte through the pointer (`sub_725270`, `sub_725110`, `sub_724FD0`, `RemoveDomainSpell`, `RemoveKnownSpell`). Value-equivalent; `const BOOLEAN&` would be faithful.
- `CGameSprite::sub_726F20` ends with an inlined copy of `sub_723830`.

## 2026-11-11 (night): batches 3–4, spells merge, S1–S4 green

Branch `phase-a/stubs-and-layout` (local only). 7,304 of 9,831 functions without a TODO (74.3%).

**Done**
- Merged the old `wip/spells` work (22 functions) and two Sonnet batches: seven `CScreenStore` functions (one partial), six `CScreenInventory` and four `CUIControlEditMultiLine` functions. About 32–34k tokens per function to translate, 11–19k to verify.
- Bugs found by running the game:
  - `CGameObjectArray::Delete` never marked the slot deleted, so removed objects were still handed out (crash once floating text worked).
  - `CGameText` freed its `CString` array with scalar `delete`.
  - The `CGameSprite` constructor set `m_active` from the hour's parity instead of the time-of-day mask. Sprites started inactive on even hours and their scripts didn't run. This was the "hard pause not lifted" flake in S3/S4.
- Bugs found by the verifier, then by scanning the repo for the same pattern: six `GetShare`/`GetDeny` retry loops that tested the wrong results or shouldn't loop.
- S1 failed at night because the main menu scene follows the system clock. Checkpoints now accept golden variants (`name.altN.png`).
- S1–S4 pass on the rebuild.

**Next**
1. `CStore::sub_54CFE0` (0x54CFE0), needed by `CScreenStore::UpdateStoreItems`; `CScreenInventory::SwapWithSlot` (0x62F360), needed by `CancelEngine`.
2. More batches from `tools/worklist.py`. The worktrees `iwd2-re-wt/batch3` and `batch4` are already built; reset them to the work branch and reuse them.
3. Remaining gates (vtable comparison, pitfall scan) and skills (switch chunks, class scaffolding).
4. Scenario S5 and later: walk, fight, save and load.

**Notes**
- Check a verifier's "DIFFERS" against the listing before applying it. So far 7 of 8 were right; the wrong one quoted an instruction that wasn't there.
- A verifier finding is often a pattern. Scan the repo for it (the retry-loop scan found six more).
- `packet.py` can't normalise stack slots for about 8% of functions (28% of those over 2 KB). It says so and prints raw asm.
- Unverified: 0x84CEB4 is read as a global holding 1 in several `Render` functions; the C++ uses a literal.

## 2026-11-11 (evening): cheaper translation pipeline

Branch `phase-a/stubs-and-layout` (local only).

**Why:** `fork` subagents were 56% of usage. `fork` is built in and always runs on the session's model, so translation moved to Sonnet agents defined in the repo.

**Done**
- `tools/packet.py 0xADDR` prints everything a translator used to look up: callees with `RET n`, constants, vtable → class, member offsets → names, and asm with stack slots renamed to `[argN]`/`[loc_X]`.
- `tools/worklist.py` assigns every unfinished function to one batch per file and tags large and switch functions.
- Skills `translate-function` and `verify-against-asm`, agents `iwd2-translator` and `iwd2-verifier` (both Sonnet, no forks) in `.claude/`.
- First batch through the pipeline: `CGameArea::sub_46C9E0`, `sub_46E130`, `Icewind586B70::sub_5872C0`, `CGameSprite::EquipMostDamagingMelee`. About 27k tokens per function to translate and build, 20k to verify. The verifier found one difference (a byte compare on a WORD field).
- S4 play test committed (passes on the original, flaky on the rebuild).

**Measured while tuning the skill** (15 read-only runs on three functions)
- The packet cuts Sonnet from about 49k to 40k tokens and from 14 to 5 tool calls per function.
- Tool-call caps and "don't look things up" produced invented member names.
- Reading `[ESP+n]` after pushes was the main source of wrong-variable bugs. Renaming the slots by script removed them.
- Haiku still costs twice Sonnet's tokens and made behavioural errors in half its runs.

**Next**
1. Second batch for the shapes not yet tried: a function over 3 KB, a switch function in chunks, a class from a vtable.
2. Gates in `tools/check.ps1`: vtable comparison, pitfall scan, no `// DBGTRACE`.
3. Finish or discard the unspliced work in the `wip/spells` worktree.
4. The intermittent hard pause after the chapter screen (S4).

**Notes**
- New agent types in `.claude/agents` only load when a session starts. Until then, run a general-purpose agent with `model: sonnet` and tell it to read the agent and skill files.

## 2026-11-11 (later): scripts, sprites and the chapter screen

Branch `phase-a/stubs-and-layout` (local only).

**Done**
- The seven leftover stubs and the `CCreatureFileHeader` tail (now 0x37C bytes).
- Translated by parallel agents and merged: `CGameAIBase::ExecuteAction`/`ProcessAI`/`ProcessPendingTriggers`, `EvaluateStatusTrigger` (base and sprite), `CMessageHandler::AsynchronousUpdate` plus 26 message classes, `CGameSprite::ExecuteAction`/`ProcessAI`/`ProcessEffectList`/`HandleEffects`/`Unmarshal`, `CGameArea::Unmarshal` gaps and area queries, `CVisibilityMap`, `CVidCell`/`CVidMode`/`CVidBitmap`/`CVidTile` blits and drawing, `CAIObjectType::Decode`, all of `CScreenWorld` and `CGameDialog`.
- Bugs found by running S2 with temporary file logging:
  - Script conditions were parsed from the wrong string, so no script ever fired.
  - Animation `Render` passed the FX rect to `FXUnlock` when not fading, which erased every sprite; FX blits never set `DDBLTFX::dwSize`.
  - `SortLists` never stored the list position of back/flight objects, so area scripts never ran queued actions.
  - `CVariableHash::Marshal` wrote every variable to slot 0; new games didn't start with `CHAPTER = -1`.
  - `StartChapterMultiplayerHost` didn't select the chapter engine; `GetChapterText` compared a BYTE with -1.
- `inventory.py` now reports functions and bytes without `TODO: Incomplete` separately.

**State of S2 on the rebuild: PASSES.** Main menu → premade party → Prologue chapter screen → Targos with Hedron's opening dialogue, all matching the original's screenshots. Known differences: party gold shows 108 (original 114) and the camera position differs slightly.

Later fixes found by running the game: `CMessageHandler::Send`/`Broadcast` were stubs (no message ever ran), `CItem` constructors were stubs (all items blank), `CSearchBitmap::RemoveObject` left its flag set (creatures jumped around), `CDerivedStats::CheckLimits` broke morale (everyone fled), `AddCharacterToParty` didn't fill `m_characters`, named-creature keys weren't uppercased, helmet animation cells, `CopyDisplay`.

**Next**
1. Merge the spells/items and effects agents' work (in progress).
2. Gold difference (108 vs 114) and camera start position.
3. More scenarios (S3+): talk to Hedron, walk, fight, save/load.
4. Remaining stubs: `CInfinity::FXRenderClippingPolys` (sprites behind walls), ~115 projectile methods, the CBaldurMessage multiplayer handlers.

**Notes**
- Debugging recipe that worked: add `fopen("C:\\iwd2-play\\trace.txt", "a")` logging in a `// DBGTRACE` block, run `tests/e2e/drive.py`, read the file, never commit the block.
- A recurring bug class: BYTE/WORD compared with -1 (always false). Check the asm for 0xFF/0xFFFF.
- Parallel agents that share helper functions produce duplicate definitions. Give each agent its own files and name the helpers other agents own.
- Never resolve a conflict with `git checkout --theirs` on a whole file; it drops the other side's non-conflicting changes.

## 2026-11-11: Phase 0 setup, first Phase A fixes

**Done**
- Tooling: VS 2022 MFC + CMake components, innoextract, JDK 21, Ghidra 12.1.4, Python venv, all in `C:\iwd2-tools`.
- Pristine reference extracted to `C:\iwd2-ref` and hashes recorded. The installed `C:\GOG Games` copy is modded (IWD2EE/IEex), and its `iwd2.exe` differs in 80 bytes of `.data`.
- Ghidra export: `C:\iwd2-tools\functions.json` and `decomp\0xADDR.{c,asm}` for 32k functions.
- Repo: `iwd2-core` static library, doctest unit tests, generated layout test (`/Zp2`, 5,145 members, 2,609 known mismatches), inventory and ratchet, check script, CI on windows-2022, crash handler, play harness (S1 boot, S2 new game), exploration driver, `tools/peek.py`, [translating.md](translating.md).
- T5: S1 passes on the original and the rebuild (main menu and Options match). S2 golden run recorded on the original (Targos opening dialogue).
- Fixes found by S2 on the rebuild:
  - Premade party Done checked the wrong member.
  - Six resource parsers compared byte-reversed signatures (CHR, ITM, BAM, DLG, EFF, STO).
  - `CInfGame::ImportCharacter` translated, plus `CResBIO`, `CCharacterFile` and missing factory cases.

- Phase A, translated by parallel agents (one worktree each, then merged):
  - `CInfGame`: `UpdateCharacterSlots`, `SetupCharacters`, `GetProtagonist`, `SelectToolbar`, `SynchronousUpdate`, `CacheResFileWithResource`, `SaveGame`, `Marshal`, `Unmarshal`.
  - Marshal functions: `CGameArea::Marshal`, `CGameTrigger::Marshal`, `CWorldMap::Marshal`, `CGameSprite::Marshal`, `CGameSpriteEquipment::Marshal`.
  - `CDimm::CompressDirectory`, `CGameEffectList::RemoveAllOfType`, plus small helpers.
- Fixes I made along the way:
  - `NonBlockingWaitForSignal` was a stub, which hung the single-player save.
  - The 8-byte resref used as a file name had no terminator, so `ICEWIND2.GAM` was written under a garbage name.
  - `WriteSetUp` created one-character folder paths, so compressed CD2 BIFs couldn't be cached and the area's WED failed to load.
  - Test tooling: an on-demand hang stack dump (`iwd2re-dump-<pid>` event).

**Current state of S2 on the rebuild:** a new game with a premade party **reaches the world screen** at Targos (AR1000). The map, action bar and HP counters render. Still different from the original:
- The Prologue chapter screen is skipped.
- Party sprites and portraits aren't drawn.
- Hedron's opening dialogue doesn't start.
- Starting gold isn't checked yet.

**Next (Phase A continues)**
1. Make S2 wait for the chapter screen, and find why it's skipped (`CScreenChapter`, the chapter text resources and the call after `LoadGame`).
2. Find why sprites and portraits don't draw (`CGameSprite` render/AIUpdate TODOs, `CScreenWorld` portrait panel). Use the hang/crash tools and compare screenshots with the original.
3. Opening dialogue: the area script and dialogue start (`CMessage` TODOs, `CScreenWorld` dialogue).
4. Stubs left behind:
   - `CSearchBitmap::GetNearestOpenSquare` (0x548B80), which only matters for multiplayer late joins.
   - `CResFile::sub_789A20`/`sub_789CE0` (BIF cache helpers).
   - `CInfGame::CachingRequirements` (0x5A04B0).
   - `CBaldurMessage::WaitForSignal`/`KickOutWaitingForSignal`.
   - `sub_5A79E0`/`sub_5A7BF0`, which only convert old save versions.
   - `CBaldurMessage::sub_43D8D0`, multiplayer.
5. The `CCreatureFileHeader` tail layout is wrong (about 0x2AA bytes instead of 0x37C), so CRE saves aren't byte-compatible after offset 0x294. Fix this before checking save compatibility (S10).

**Notes for next session**
- `inventory.py` counts a function as done once annotated, even with `TODO: Incomplete` inside, so bytes done (69%) overstates progress. Consider subtracting the sizes of functions that contain TODOs.
- The T4 hook-DLL spike hasn't started. `mfc.h` has VC6 support (`_MSC_VER == 1200`), so a VC6 + MFC 4.2 build would match the original's `CString`/heap ABI. That's worth checking before writing shims.
- Builds must run from PowerShell, because Git Bash mangles MSBuild `/m`.
- Play tests take over the mouse. Never run them in parallel.
