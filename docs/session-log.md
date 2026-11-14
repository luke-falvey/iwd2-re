# Session log

Newest first. Each entry covers what was done, what's blocked and what's next.

## 2026-11-14 (midday): play tests S12 and S13, one more bug

- **S12, saves across the two games** (`test_s12_cross_save.py`): each game makes a save on the docks and the other loads it. Both directions pass; the 14 compared regions (inventory, record, journal, action bar, portraits, load-screen slot) are identical. Only a start-of-game save is covered.
- **S13, the house** (`test_s13_house.py`, about 7.5 minutes): continues S9 into AR1005. Pausing, ordering attacks while paused, the fight to its end, a goblin's ground pile, the locked footlocker (lock picked, an item taken, weights), Cure Light Wounds, and leaving to the docks. S9's steps moved into `docks_to_house()` as the shared set-up.

| Found by | File | What was wrong |
|---|---|---|
| S13 (assert in `CVidInf::FXPrep`) | `CGameAnimationTypeEffect::Render` | The prep flags were chosen twice; a translucent or shadowed effect got both copy-from-back and clear-fill |

**Experience line timing.** On the rebuild "The party has gained experience" sometimes came after S13's next pause instead of straight after the death line. Traced: the killer handles the kill on its next AI slot (every 16 AI ticks, about a second) once its last order is over 75 ticks old; while the order is fresh it is handled at once. The code for both paths matches the exe (0x72D917-0x72DA89, 0x728B70, 0x45CA10). The original showed no delay in 6 timed kills, but none of them fell in the slow case, so whether it waits too is not measured. S13 now waits 1.5 s after a kill; it then passed once on the original and twice on the rebuild.

**Open.**
- The slow-kill case on the original is unmeasured (above).
- `docks_to_house()` failed twice in about ten uses during timing runs: once on the original ("no goblin came when the party went to the door"), once on the rebuild ("click on Reig did not change the screen"). The Reig failure came again in the full run (S9 itself; the other twelve passed, and S9 passed when rerun): the party stands beside Reig, the last line is "Honest Jon- How fares your arm, Reig?", and no dialogue opens within 20 s. Seen only on the rebuild so far (3 times in about 15 runs); not yet checked whether the original refuses a dialogue during that banter.
- S9 on the original recorded three snow/darker variants (`door-closed`, `door-open`, `scrolled-dock`).

**Next.** Travel to AR1100; a store and a successful rest once reachable; later-game saves loaded across the two games.

## 2026-11-14 (morning): play tests S10 and S11, two more bugs

S1–S11 pass on the rebuild in one full run (about 30 minutes).

- **S10, save and load** (`test_s10_save_load.py`): changes formation and moves an item, quick-saves, saves under a typed name, quits to the menu, loads, and compares inventory, record, journal, area map, action bar and portraits before and after. Cleans up its save folders. Passed on the rebuild as written.
- **S11, items, spells, rest** (`test_s11_items_spells_rest.py`): drops an item on the ground, walks away and back, picks it up; unequips and re-equips armour and shield (AC 14, 12, 11, 14); switches weapon sets; casts Cure Light Wounds; removes a memorised spell and memorises another; rest refused on the docks.

| Found by | File | What was wrong |
|---|---|---|
| S10's save files, unpacked and compared | `CInfGame::GetGroundPile` | Test inverted: a pile was created when one already existed and not when none did |
| S11 | `CScreenSpellbook::ResetErrorPanel` | Buttons fetched as control `nButton + 1` for the exe's `nButton`; clicking a memorised spell asserted |

**Save comparison.** `WORLDMAP.WMP` is byte-identical between the original and the rebuild; `ICEWIND2.GAM` is the same size with time, positions and gold differing; the saved `AR1000.ARE` differed by the missing pile (now fixed) and by goblins' random loot, which differs between any two runs.

**Open.**
- S8's R key was ignored once in eight rebuild runs after the music fix; seven traced runs showed nothing. The original ignored it once too. S8 now uses the dial button.
- Once, on the rebuild, a new game stopped at "Paused for chapter text" and Hedron's dialogue never started; the autosave was not rewritten in that run. Not reproduced since.

**Next.** The fight inside AR1005 and its footlocker; travel to AR1100; a store and a successful rest once reachable; saves loaded across the original and the rebuild.

## 2026-11-14 (early morning): play tests S8 and S9, four bugs the audit missed

S1–S9 pass on the rebuild. The two new scenarios pass on the original and each failed on the rebuild when first run.

- **S8, screens tour** (`test_s8_screens_tour.py`, about 4 minutes): area map, journal, record, spellbook (three casters, domain tab), every options sub-panel, inventory. Every input must visibly change the screen, and each number has its own tight checkpoint.
- **S9, Targos docks** (`test_s9_docks.py`, about 5.5 minutes): view moved through the area map, Reig's whole dialogue, a door, the first goblin fight, a ground item, entering a house (AR1005).

| Found by | File | What was wrong |
|---|---|---|
| S8 | `CScreenSpellbook::sub_66A8C0` | Domain slot cap inverted (`== 0` for the exe's unsigned `> 0`): one free domain slot shown where the original shows none |
| S8 | `CGameArea::AIUpdate` | Day/night song switch lost its `m_nCurrentSong == 1` / `== 0` tests, so the area song was stopped and restarted every 600 ticks with a 500 ms sleep; inputs falling inside the sleep were lost |
| S9 | `CGameSprite` constructor | `m_spriteEffectPalette` and `m_spriteSplashPalette` not constructed as range palettes: assert in `CVidPalette::SetRange` on the first blood splash |
| (same check) | `CGame85BFE8` constructor | `m_palette` likewise; found by listing every `CVidPalette` construction in the exe |

All four are in functions the old-code audit or earlier verification had passed. Zero unread functions does not mean zero differences; scenarios that reach new code are the better detector now.

**Also done.** `record-scenario` skill written from the two authoring runs. 811 stale entries removed from `tests/layout/known_mismatches.txt` (1,359 real mismatches remain).

**How the dropped inputs were found.** A click loop outside S8 lost nothing, so temporary tracing went into a worktree copy of `CChitin::AsynchronousUpdate` (tick gaps, button and key states). It showed a 500 ms gap every 20.3 s; 600 ticks pointed at `m_startedMusicCounter`.

**Next.** More scenarios: save, quit to menu and load; rest and spell memorisation; a store; the fight inside AR1005. Then saves loaded across the original and the rebuild.

## 2026-11-14 (late night): batches 180–183, the old-code audit is complete

Every pre-project function has now been read against the exe: `tools/old_code.py` reports 0 functions, 0 bytes unread. S1–S7 pass on the rebuild after each of the four merges, the last being batch 183.

| Batch | File | What was wrong |
|---|---|---|
| 180 | `CMultiplayerSettings` | Slot assignment status test and character id; permission cascade; gore option on unmarshal; world map control when a player drops |
| 181 | `CNetwork` | Left-game and dropped-game strings swapped in the header; sliding-window queue walks used the wrong node, a stray `+3` and missed offsets; shutdown re-queued to the wrong window; session status counts uninitialised; player lookups returned on an invisible duplicate; compressed sends passed the uncompressed size; uncompress retried on every error; modem check opened the wrong registry key |
| 182 | `CScreenConnection` | Popup enable passed the panel id as the flag; quick load tested a portrait index as a slot; player name setting written from the password; protocol switch fell through; serial settings never fell back to COM1/38400; session index not reset after a join; null provider could be auto-selected; `m_vcTorch` and `field_FB0` declared out of order (10 known mismatches removed) |
| 183 | `CScreenMultiPlayer` | Modify-character skipped the can-save check and had string id 101317 for 10317; portrait click opened popup 7 for 8 and read the neighbouring lock flag; character enable loop never touched one label; word wrap cleared on the wrong member; chat box kept the last line; label redraw rect used the button's size |

**What the tests do not show.** None of batches 180–183 is exercised by S1–S7 beyond start-up and game load, which pass through `CScreenConnection::EngineActivated`, the "finding network devices" popup and `CNetwork` under the null provider. The multiplayer screens themselves have no play test.

**Open notes.**
- `CScreenConnection::EngineActivated` now closes the game if `GetDiskFreeSpaceA` fails, as the exe does. The free-sector product is 32-bit in the exe and can wrap on very large volumes; a false "low space" needs the wrapped value to land under the threshold.
- `CNetwork::CheckSessionStatus` reads its BOOLEAN argument slot as a dword in the exe; the repo uses the BOOLEAN's value. Which result the single-player DirectPlay path returns from `GetSessionDesc(NULL)` was not checked.
- `CUIControlBase.h`: the `/* 0008 */ m_pPanel` offset comment is stale (the asm reads the panel at +6, the id at +0xA).
- `UpdateProtocolPanel` and one protocol button constructor read a reused stack slot in unreachable cases in the exe; the repo leaves the value unset with a `// NOTE:`.

**Next.** Play tests for what nothing covers yet (audio, monsters, combat, doors, stores, options screens), starting with a `record-scenario` skill; prune stale known-mismatch entries.

## 2026-11-14 (night): batches 149–179, every single-player file's old code read against the exe

All pre-project code outside the four multiplayer files has now been read against the exe and fixed. S1–S7 pass on the rebuild after batch 179. Unread old code: 105,776 bytes in 347 functions, all in `CScreenConnection.cpp`, `CScreenMultiPlayer.cpp`, `CNetwork.cpp` and `CMultiplayerSettings.cpp` (audits and fixes for those are in progress).

**How the work was run.** The pipeline is now written down as skills: `audit-old-file` (orchestration), `apply-audit-findings` (the fixer), and an "Auditing old code" section in `verify-against-asm`. The scripts are `tools/audit_tranches.py` (split a file, write verifier briefs, mark addresses audited) and `tools/play.ps1` (build, checks, then every play test). From batch 165 on, small files were audited several to a verifier.

| Batches | Files | What was wrong |
|---|---|---|
| 149–150 | `CVideo3d`, `CVidMode`, `CUIControlTextDisplay`, `CUIControlScrollBar` | Tint and flash colour arithmetic; wrong texture ids freed; speaker label kept on wrapped lines; scroll bar track and down-button geometry |
| 151–153, 156 | `CSound`, `CSoundMixer`, `music/music.cpp`, `music/sound.cpp` | Positioned-sound distance `dy + dy`; destructor leaked its buffer; fade-list loop never advanced; section not updated on a song change; stream read left uninitialised in one branch |
| 154–155 | `CVidPalette`, `CGameDoor`, `CGameTrigger`, `CSpawn` | Realise branch chose on the wrong argument; byte loop counters; door bounding rects one short; both trigger mouse-over tests inverted; `"::"` searched as any colon |
| 157–160 | `CScreenChapter`, `CGameJournal`, `IcewindMisc`, `CScreenStart`, `CUIPanel`, `CUIControlEdit` | Chapter title scrolled as a body paragraph; journal insert returned FALSE; quit confirmation set the wrong state; panel hover tested the control's flag |
| 161–162, 167–168 | `CUIControlButton`, `CVidMosaic`, `CVidBitmap`, `CVidImage`, `CVidPoly`, `CUIManager`, `CUIControlEditMultiLine` | Three member-width fixes (below); mosaic and polygon loops re-translated; dithered lines one pixel short on odd widths |
| 163–166 | `CDerivedStats`, `CItem`, `CGameObjectArray`, `CAIGroup`, `CVariableHash`, `CStore`, `C2DArray`, static animations, `CWorldMap`, `CAIScript`, `CAIObjectType`, `CResCache` | Failed object lookups left the caller's pointer stale; variable hash used the name's length, not its characters; store lists allocated as one element; script target resolution used the wrong search flags and origin |
| 169–173 | `CSearchBitmap`, `CGameAreaNotes`, `CSpell`, `CImmunities`, `CImm`, `music/audio.cpp`, `music/adecode.cpp`, `CResFile`, `CDimmKeyTable`, `CScreenKeymaps`, `CScreenMovies`, `CVideo`, `CTlkTable`, `CGameRemoteObjectArray` | Sorcerer usability bit; projectile immunity list saved as pointers; music decoder branches left empty; key table kept probing from its old slot after growing |
| 174–179 | 84 small files in seven groups | Plain WAV files never parsed (chunk walk skipped the 8-byte header); id-list case branches swapped; game time in single player never checked timed triggers or passed elapsed time to areas and weather on rest and travel; tiled object's script name was 8 bytes for a 32-byte copy |

**Layout fixes.** Four member-width or missing-member fixes corrected many classes at once:
- `CUIControlButton::m_nTextLines` is a byte: 47 button classes, 102 known mismatches removed.
- `CUIManager::field_2E` is four bytes: `CBaldurEngine` and every screen class, 368 removed. This also closed the old note about `CScreenWorld::field_15A` (now a WORD).
- `CVidBitmap` has an unused `CString` at 0xB2. It exposed 13 mismatches in `CProjectileSingleTarget850DDC`, which had only matched because two errors cancelled (its `std::string`/`std::vector` sizes differ from MSVC 6); those are now listed as known.
- `CGameTiledObject::m_scriptRes` is 32 bytes.

**Verifier and fixer accuracy.** Fixers rejected or corrected about a dozen claims in this stretch (a loop bound that ignored byte wrap, an "explicit unlock" that was the lock's destructor, a cited address in the wrong case). One fixer disagreed with an audit about `CSpawn::SelectSpawnPoint`; a third agent re-translated it and showed the fixer had misread inlined callees. One audit claim would have changed a correct call site because two member names in `CNetwork.h` are swapped; the other tranche caught it.

**Incidents.**
- A fixer wrote one edit into the main tree instead of its worktree and my "mark audited" commit swept it in (`git commit -a`). Backed out in 9a1eb43; both skills now guard against it. The fixer reported being denied permission to revert it and asked me to; I had already reverted it independently and did not act on the request.
- Several subagents died on API authentication errors (HTTP 403) twice; no findings were lost.

**Decided, no change.** `CRes::m_pData` is allocated and freed with two allocator pairs in the exe (`operator new`/`delete` in `CRes::Demand`, 0x789710 and `CDimm::Dump`; `malloc`/`free` in `ServiceFromFile` and the save/load screens), and they cross through `Dump`. The exe's `operator delete` is a direct `free`, as is the rebuild's, so the repo's mix is harmless and stays.

**Open notes.**
- The play tests don't check audio and show no monsters, combat, doors, stores or option screens. Fixes to animations, sound and music, doors, stores, spells and immunities rest on the asm comparison only. A `record-scenario` skill and scenarios for those screens are the next thing to build.
- `DSBUFFERDESC` is 36 bytes with the build's DirectSound header; the exe passes 20.
- The corrected variable hash changes the order of variables in saved games.
- `CGameSprite::RemoveKnownSpell` / `RemoveDomainSpell` probably take a `const BOOLEAN&` where the repo declares a full-width reference.
- `CGameAnimationTypeCharacter::m_renderWeapons` / `m_renderHelmet` are BOOLEAN in the repo and read as dwords in the exe.
- About 95 entries in `tests/layout/known_mismatches.txt` are stale (they match now) and can be pruned.

## 2026-11-14 (afternoon): batches 138–148, resource manager, rendering classes and every animation class read against the exe

Twenty-five more files of pre-project code were read against the exe and fixed. S1–S7 pass on the rebuild after batch 147; batch 148 is merged and waits for the next run.

| File | Old functions read | With a real difference | Batch |
|---|---|---|---|
| `CDimm.cpp` | all | 14 | 138 |
| `CChitin.cpp`, `CUIControlFactory.cpp` | all | see commit | 139 |
| `CVidCell.cpp` | 40 | 12 | 140 |
| `CVidInf.cpp` | 44 | 11 | 140 |
| `CVidFont.cpp` | 24 | 13 | 140 |
| `CGameAnimationTypeCharacterOld.cpp` | 32 | 13 | 141 |
| `CGameAnimationTypeCharacter.cpp` | 32 | 12 | 141 |
| `CGameAnimationTypeMonsterIcewindNew.cpp` | 22 | 7 | 142 |
| `CGameAnimationTypeMonsterOld.cpp` | 23 | 2 | 142 |
| `CGameAnimationTypeMonsterMulti.cpp` | 24 | 3 | 143 |
| `CGameAnimationTypeMonsterQuadrant.cpp` | 27 | 3 | 143 |
| `CGameAnimationTypeMonsterLayeredSpell.cpp` | 22 | 6 | 144 |
| `CGameAnimationTypeMonsterLayered.cpp` | 23 | 4 | 144 |
| `CGameAnimationTypeMonsterIcewind.cpp` | 20 | 3 | 145 |
| `CGameAnimationTypeMonster.cpp` | 23 | 1 | 146 |
| `…Effect`, `…MonsterAnkheg`, `…MonsterLarge16`, `…Ambient`, `…MonsterLarge` | 102 | 8 | 147 |
| `CCacheStatus.cpp` | 5 | 1 (15 places) | 148 |
| `CGameSpriteEquipment.cpp` | 6 | 1 | 148 |

- **Control factory (139).** `CUIControlBase::CreateControl` was compared with the exe by emulating the exe's function for every screen, panel and control id: 15,804 mismatches of 406,068 before, 5 after.
- **Rendering (140).** `CVidCell`: sequence member is a WORD, FX blit guards, 3d render demand/release pairs. `CVidInf`: surface creation and pixel-format parsing (24/32 bpp loops stopped at bit 16), the back surface released in the wrong mode, `FXPrep` clear sizes. `CVidFont`: `CreateFontA` arguments in the wrong order, DBCS trail bytes sign-extended, `TextOutEx` takes its clip rectangle by value, 3d text clipping and advance.
- **Animation classes (141–147).** The recurring defects:
  - `GetCurrentSequenceAndFrame` took its two results by value in the whole hierarchy, so callers got nothing back.
  - Equip functions copied from a sibling and not adapted: armour "E" art loaded into the base cells, shield code used for the helmet, weapon flags written to the wrong slot, weapon art names built from the whole item string where the exe uses its first character.
  - `SetSequence` switches: missing `break`s (IcewindNew, Icewind), six missing cases in `CGameAnimationTypeMonster` that fell into the assertion, wrong cells or sequence numbers in single cases of seven classes.
  - Constructors that never set `m_currentVidCellBase` (Large, Large16, Quadrant) or `m_falseColor` (MonsterOld), and two missing `break`s in the character constructor.
  - `CGameAnimationTypeMonsterIcewindNew::EquipWeapon` had a search loop that never incremented.
- **Loading screen (148).** `CCacheStatus::Update`: hint-line positions, hint index saved one too low, two early returns, timeout counter text, minibar player index, two font calls the exe doesn't make.
- **Equipment (148).** `CGameSpriteEquipment::Unmarshal` never stored the sprite pointer that `ClearMarshal` later uses.

All claims in these rounds were confirmed by the fix translators against the asm; none was rejected, and two were corrected in detail (a cited address, a store that the exe has three times).

**Open notes.** `CGameAnimationTypeCharacter::m_renderWeapons` / `m_renderHelmet` are declared BOOLEAN but read as dwords in the exe. `CDimm::Alloc` still uses malloc/free. No play test puts a monster or an equipped weapon swing on screen, so most animation fixes are covered by the asm comparison only.

**State.** 0.45 MB of old code unread (2,112 functions). In progress: `CUIControlTextDisplay.cpp`, `CUIControlScrollBar.cpp`, `CVideo3d.cpp`, `CVidMode.cpp`, `CSound.cpp`, `CSoundMixer.cpp`. Largest left after those: `CScreenConnection.cpp` (39 KB), `CScreenMultiPlayer.cpp` (33 KB), `CNetwork.cpp` (27 KB), `music/sound.cpp`, `music/music.cpp`, `CMultiplayerSettings.cpp`.

## 2026-11-14 (morning): batches 132–137, eight more single-player files read against the exe

`CInfinity.cpp`, `CScreenOptions.cpp`, `CScreenSave.cpp`, `CScreenLoad.cpp`, `CScreenWorldMap.cpp`, `CWeather.cpp`, `CInfCursor.cpp` and `CBaldurChitin.cpp`: 427 pre-project functions, 91 with a real difference. One more wrong verifier claim (the load screen's Tab key entry, a misread push order); the scripted key-table check had been right.

| File | Old functions read | With a real difference | Batch |
|---|---|---|---|
| `CInfinity.cpp` | 62 | 19 | 132 |
| `CScreenOptions.cpp` | 52 | 9 | 133 |
| `CScreenSave.cpp` | 71 | 13 | 134 |
| `CScreenLoad.cpp` | 54 | 7 | 135 |
| `CScreenWorldMap.cpp` | 40 | 4 | 135 |
| `CWeather.cpp` | 29 | 11 | 136 |
| `CInfCursor.cpp` | 19 | 8 | 136 |
| `CBaldurChitin.cpp` | 81 | 12 | 137 |

- **Renderer (`CInfinity`).** Scroll speed terms swapped for vertical and diagonal scrolling; screen shake scaled by 10 instead of shifted by 10, no random flip on Y, wrong end-of-shake restore; the night-to-day WED swap branch was empty; day/night tile caching used raw game time and an inverted test; tile request border loops asked for one edge twice; multi-layer areas looked tiles up in layer 0; viewport bounds exclusive where the exe is inclusive. S5 and S6 still match the original's screenshots after the change.
- **Options.** Saving from the options screen hit `UTIL_ASSERT(FALSE)`; rest set the heal flag the wrong way round; the weather checkbox was never refreshed.
- **Save and load.** Save-as date label used the chapter as the format string; a missing `break` in the overwrite confirmation; no clean-up after saving; the fourth portrait drew the third character's in both screens; the load screen's error popup handled its two types the wrong way round.
- **Weather.** Storm stage end time not updated at full strength; lightning frequency passed on every update; rain sound reset every tick; particle `rand()` calls in a different order from the exe.
- **Cursor and tooltips.** Tooltip clamp and saved-background rectangles used the wrong widths; line count one short in two branches; two `CResCell` sub-objects missing from `CInfCursor` (adding them cleared all 11 of its layout mismatches).
- **Application object.** "First Run" written as 1 on exit; an unknown INI resolution kept its width while using the 800 layout; `GetFirstCDRomDrive` returned the last drive tried when there is no CD drive; one byte of the GameSpy code XORed instead of ANDed.

**State.** 0.81 MB of old code unread (2,854 functions). Largest: `CUIControlFactory.cpp` (54 KB), the character and monster animation files (about 200 KB together), `CScreenConnection.cpp` (39 KB), `CScreenMultiPlayer.cpp` (33 KB), `CNetwork.cpp` (27 KB), `CDimm.cpp` (23 KB), `CVidCell.cpp`, `CVidInf.cpp`, `CChitin.cpp`.

## 2026-11-14 (early): batches 123–131, ten more files read against the exe; S7 added

Same method on `CScreenCreateChar.cpp`, `CGameArea.cpp`, `CRuleTables.cpp`, `CScreenJournal.cpp`, `CGameAIBase.cpp`, `IcewindCGameEffects.cpp`, `CGameContainer.cpp`, `CScreenMap.cpp`, `CScreenSpellbook.cpp` and `CScreenSinglePlayer.cpp`: 1,133 pre-project functions, 138 with a real difference. One verifier claim was wrong (a `CGameContainer::SetCursor` switch table misread) and was caught by the fix translator reading the table from the exe.

| File | Old functions read | With a real difference | Batch |
|---|---|---|---|
| `CScreenCreateChar.cpp` | 239 | 33 | 123 |
| `CGameArea.cpp` | 57 | 18 | 124 |
| `CRuleTables.cpp` | 71 | 13 | 125 |
| `CScreenJournal.cpp` | 33 | 5 | 126 |
| `CGameAIBase.cpp` | 77 | 14 | 127 |
| `IcewindCGameEffects.cpp` | 192 | 14 | 128 |
| `CGameContainer.cpp` | 24 | 7 | 129 |
| `CScreenMap.cpp` | 79 | 8 | 130 |
| `CScreenSpellbook.cpp` | 60 | 10 | 130 |
| `CScreenSinglePlayer.cpp` | 99 | 15 | 131 |

- **Character creation.** Ability rolls were computed and never stored; negative racial adjustments read as 254; a new sorcerer's spell picks tested the class number against the class mask and used the bard's level; feats were cleared through the skill-id table; Evoker and Illusionist were swapped (also in `CRuleTables::GetSpecializationIndex/Mask`, with Helm and Mystra); the appearance preview equipped the wrong animation; export did not replace spaces and compared names case-sensitively.
- **Area.** Ambient loops never played outside low-memory mode (missing "extended" branch); dusk and dawn faded the night volume the wrong way and dawn called the dusk transition; `OnActivation` stopped the timer when paused instead of when not paused; `GetAllInPoly` had the dead test inverted; the travel click queued "leave" twice; `SetSong` stored -1 as 255.
- **Rule tables.** `DSMonst` was loaded as `DSMonste`; `SLizard` was never loaded; intoxication read the reputation table; druid shapeshifts indexed by column; Lay on Hands tested the wrong class mask.
- **AI base.** `ApplyTriggers` ATTACKEDBY party test inverted; `ApplyEffectToParty` shared one effect object across the party; four spawn-point/static actions had inverted NULL tests; `MoveView` finished when one axis arrived.
- **Icewind effects.** Entropy Shield saves +10 for +2; Recitation and Righteous Wrath added damage bonuses the exe does not; Slow Poison switched on its own flags; Bane removed Aid instead of Bless; Rapid Shot's copy built a Stoneskin effect; Arterial Strike used the feat number as a spell-state bit.
- **Containers.** Own script-name buffer at 0x86C added (the exe never uses the base one for containers); cursor for "no party selected" inverted; loot-pile bounding box and polygon clean-up wrong; `SetItem` refreshed every slot.
- **Screens.** Keymap loops that never increment in the journal, map, spellbook, multiplayer and world-map screens (a hang on an unmapped key), all fixed; keys not offered to the UI manager first in the map and spellbook; premade-party import passed the party-list position where the exe passes the slot (all six biographies landed in one string slot).

**S7 (new play test).** `test_s7_hedron_branches.py` takes the branches of the Hedron conversation S3–S5 skip (twelve nodes, including the reply that adds a journal entry), then opens the journal. Recorded twice on the original. On the rebuild every dialogue checkpoint matched at once; the journal checkpoint failed with an empty text area. Cause: `CGameJournal::AddEntry` built entries with the default constructor, so the character mask was garbage and `UpdateTextDisplay` hid them (exe 0x4C6440 stores text, time, type and 0xFF). Fixed in batch 126; `Unmarshal` had the same fault and a mask that was never copied from the save (batch 127). S7 passes since batch 126. A first design (talk to a dock NPC) was dropped: nobody on the docks is talkable in the reachable part of AR1000.

**State.** 0.94 MB of old code unread (3,270 functions). Largest: `CUIControlFactory.cpp` (54 KB), the character animation files (48, 44, 28 KB), `CScreenConnection.cpp` (39 KB), `CScreenMultiPlayer.cpp` (33 KB), `CNetwork.cpp` (27 KB), `CDimm.cpp` (23 KB), `CInfinity.cpp` (22 KB), `CScreenOptions.cpp` (21 KB), `CBaldurChitin.cpp` (20 KB).

**Open items.** `CScreenCreateChar::OnDoneButtonClick` has a second bad-character string no finding covered. `CScreenSinglePlayer::OnCancelButtonClick` unlocks explicitly in three cases where the exe leaves it to the destructor.

## 2026-11-13 (late night): batches 118–121, old code in the four main screens read against the exe

Same method as the entry below, on `CScreenInventory.cpp`, `CScreenStore.cpp`, `CScreenCharacter.cpp` and `CScreenWorld.cpp`: 637 pre-project functions, 117 with a real difference. Every verifier claim held when re-checked; fix translators now read switch tables from the exe with `peek.py` instead of taking them on trust.

| File | Old functions read | With a real difference | Batch |
|---|---|---|---|
| `CScreenInventory.cpp` | 137 | 33 | 118 |
| `CScreenStore.cpp` | 186 | 36 | 119 |
| `CScreenCharacter.cpp` | 204 | 29 | 120 |
| `CScreenWorld.cpp` | 110 | 19 | 121 |

- **Inventory.** Requester plus/minus button ids swapped; right-click on a slot used a portrait number as a character id; Cancel missing for the rest-confirm panel; ground piles never flushed; abilities panel updated two of three buttons; paper doll tested the wrong animation and leaked a share lock; two item types missing from the appearance swap.
- **Store.** Every item-button click handler switched on the control id instead of the panel id, so buying, selling and identifying could not select anything; `StartStore` lacked store type 2; the sell button ran one of three handlers; all four sell handlers reset the list position; identify left paid items selected; the store timer skipped its closing checks in the normal case; `EnableMainPanel` never enabled the main panel.
- **Character.** Negative racial ability adjustments read as 254; Evoker and Illusionist swapped; the ability minus button changed the wrong value; the feat circle renderer was a copy from character creation; the Level Up button's enable logic was commented out; the race label was never written; the script picker sorted backwards and listed every script twice.
- **World.** Numpad direction keys had no repeat, Page Up/Down swapped, two wrong key codes; three viewport fields never set in the constructor; container scroll bar two rows too long and page-up read the wrong position; `CancelEngine` had two wrong cases; `MoveToArea`'s action carried an uninitialised `m_specificID3` (fixed in the `CAIAction` constructor). `AsynchronousUpdate` (6.8 KB) matches.

**Key tables.** Three screens each had a different slip in their `CKeyInfo` table, so a translator decoded every table in the other 16 screens by script (1,019 entries): four more files had wrong entries (`CScreenChapter`, `CScreenCreateChar`, `CScreenKeymaps`, `CScreenSave`), fixed in 5a2f72e. The script is in the session scratchpad (`dec.py`), not in `tools/`.

**Process note.** Asking verifiers to re-emit their reports into scratch files tripped the model's safeguards on two agents (one write interrupted, one agent stopped). Dropped; verifiers stay read-only and report in their final message.

**State.** 1.25 MB of old code unread (4,257 functions). Largest: `CScreenCreateChar.cpp` (93 KB), `CUIControlFactory.cpp` (54 KB), the two character animation files (48 and 44 KB), `CScreenConnection.cpp` (39 KB), `CRuleTables.cpp` (36 KB). No play test opens a store, the character record or a container, so batches 119–121 are checked by the disassembly only.

**Open items.** `CScreenWorld::field_15A` is a WORD in the exe; narrowing it needs the 2-byte base-class shift before `field_106` fixed first. `CUIControlScrollBarCharGenHatedRace::OnPageUp/Down` in `CScreenCreateChar.cpp` probably have the paging defect fixed in the character screen. `CScreenWorld::EngineActivated` calls `TogglePauseGame` on `m_pEngineWorld` where the asm uses `this`.

## 2026-11-13 (night): batches 114–117, old code in four more files read against the exe

The old-code audit went on to `CGameEffect.cpp`, `CMessage.cpp`, `CAIScriptFile.cpp` and `CInfGame.cpp`: 1,165 pre-project functions (about 330 KB), read in tranches of about 10 KB by Sonnet verifiers, fixes confirmed against the asm and applied by a translator per file. About 130 functions had a real difference. Every verifier claim held when re-checked, apart from small adjustments.

| File | Old functions read | With a real difference | Batch |
|---|---|---|---|
| `CGameEffect.cpp` | 433 | about 51 (plus 21 assert stubs missing a return) | 114 |
| `CMessage.cpp` | 554 | 53 | 115 |
| `CInfGame.cpp` | 152 | 17 | 116 |
| `CAIScriptFile.cpp` | 26 | 7, two of them the big parsers | 117 |

- **`CGameEffect.cpp`.** Gold wrote to the XP stats; disease drained INT for WIS and its escalating variant stepped by 7; sanctuary never set its visual bit; web lacked the to-hit penalty; calm removed from the wrong list; seven resistances and several skill and percent modes truncated or mis-signed the amount; `CGameEffectVisible` had no `ApplyEffect` (the exe shares 0x4B2900 with `CGameEffectDispelInvisible`); Banish set effect id 420 instead of 428.
- **`CMessage.cpp`.** Mostly multiplayer: misplaced `else` in four settings handlers, inverted host/client tests in four senders, `ObjectAdd` never sent, GameSpy loops that never incremented, a dozen marshal/unmarshal width, size and lookup-failure differences. Single-player: `CMessageVisualEffect` never stored its property, `CMessageContainerItems::Run` used the wrong loop bound, `CMessageFireProjectile` stores type + 1. `CMessageMoveGlobal::MarshalMessage` now mirrors an original bug (copies from the string object's address), with a note.
- **`CInfGame.cpp`.** `SelectAll` selected one character (stray `break`), `SelectCharacter` used `||` for `&&`, `RemoveCharacterFromParty` did not compact the portrait list, `CompressContainer` never ran (`=` for `==`), `FindItemInStore` searched an empty store, `LoadOptions` had five wrong defaults or range tests, the constructor did not write the default quick formations, `SetProtagonist` ignored its argument. `CSpellList::GetSpellLevel` returns an index one before the match in the exe; mirrored with a note.
- **`CAIScriptFile.cpp`.** `ParseOneLine` parsed only comment lines. `ParseAction` (31 KB) was wrong in 15 action forms (parameter cut from the wrong string or stored in the wrong field), returned an empty action from 77 error paths where the exe returns the partly filled one, and called itself where the exe calls `ParseError`. `ParseTrigger` had five differences of the same kind. The header had the trigger and action lists swapped.

**Regression found by the play tests.** After batch 116, S2, S4 and S6 failed intermittently with an access violation in `SetProtagonist`. The function reads `CGameDialogSprite::field_54` first, and the repo never initialised it (the exe zeroes it in the inlined constructor at 0x6856CD). The old code hid this by ignoring its argument. Fixed in 827e679.

After batch 117, S2–S6 all died with heap corruption (0xC0000374) at the opening dialog. With `ParseOneLine` fixed, dialogue trigger and action strings are compiled for the first time, and `ParseInt` builds a `CAIIdList` from an empty resref on every call; the resref constructor left `m_faster` and `m_pIdArray` uninitialised (the exe zeroes them at 0x40A2F5), so the destructor freed garbage. Fixed in 98604be. The crash handler now also writes `iwd2re-crash.txt` for heap corruption (first-chance handler), which is how the stack was found. Both regressions were old-code defects that the audit fixes exposed; S1–S6 pass on the final state.

**State.** `tools/old_code.py` reports 1.44 MB of old code still unread (5,020 functions). Largest: `CScreenCreateChar.cpp` (93 KB), `CUIControlFactory.cpp` (54 KB), `CScreenCharacter.cpp` (53 KB), `CScreenStore.cpp` (52 KB), the two character animation files (48 and 44 KB), `CScreenInventory.cpp` (43 KB). Cost so far is about 110–160k tokens per 10 KB verified and 120–190k per fix round, all Sonnet except the `CAIScriptFile.cpp` fix round (Opus).

**Open items.** `CSpellList::GetSpellLevel` keeps its name although it returns an index. `CMessageSetProtagonist` and `CMessageStartCombatMusic` constructors take their arguments in a different order from the exe (callers agree; note only). `CAIScriptFile` still has a size problem before `m_actions` (known mismatch). `CMessage.cpp` 0x42F750 keeps some odd creature-offset handling worth a later look.

## 2026-11-13 (evening): batches 109–110, listing holes closed, old code sampled

**Code outside functions.** Batch 109 checked the 11 chunks inside known functions and the five standalone runs against the repo and found five real bugs in older code (monster constructor `animationID & 0xFF`, `CInfGame::StepAnimation` cases '4'–'7', Icewind `ClearColorEffectsAll`, the music `deleteSound` callback). Four functions also had bytes Ghidra left undefined; `tools/ghidra/RangeListing.java` writes a gap-free listing of an address range for those. Three were verified from it (monster constructor: missing `break` after group 0x300 and `m_falseColor = FALSE` for MTAS; world map 0x69D0F0; `EquipHelmet` 0x6D62E0); the fourth, 0x426B20, is the CD check that this exe has patched to return 1. Batch 110 re-derived the `CGameAnimationTypeCharacterOld` constructor.

**Old code, sampled.** 6,454 functions (1.9 MB) were complete when the project started and have not changed since, so no translator or verifier of this project has read them. Three Sonnet verifiers checked a random sample (62 functions of 150–2,500 bytes, 25 KB, seed 20261113): **12 functions (19%) had a behavioural difference, 17 differences in all**. All 17 were confirmed against the disassembly and fixed:
- `CGameSprite::GetLauncher` did not report the slot of a launcher found outside the active weapon set.
- `CScreenConnection::ResetImportPanel` looked for `iwd0.chr`–`iwd5.chr` instead of 1–6 and swapped the two help texts.
- `CScreenOptions::OnKeyDown` did not pass keys to the UI manager first, ran keymap shortcuts with a popup open, and its keymap loop never advanced.
- `CVidMode::ApplyBrightnessContrast` inverted channels as 32-bit values and truncated the gamma factor to a byte.
- `CMessageSpriteUpdate` constructor and `CScreenCreateChar::ImportCharacter` each had a member name as a statement where an assignment belongs (`m_currPath;`, `field_1628;`); `ImportCharacter` also left the step unset when items are imported.
- `CScreenCharacter::EngineGameInit` did not clear the spell lists.
- `CGameEffectList::RemoveAllEffectsIgnoreMoreThenPermanent` compared the dispel level difference unsigned and 32 bits wide.
- `CMemINI::GetFast` never updated the last-section cache; `CResItem::Parse` returned FALSE for a null argument instead of the parsed flag; `CScreenStore::UpdateRentRoomPanel` normalised the inn flags; the clock button used the full 32-bit game time for its frame.

At that rate the unread 1.9 MB would hold on the order of a thousand such differences. Whether to audit all of it (about 40M tokens at the sample's cost) is the user's decision.

**Static data.** Two scripts compare the repo's annotated statics with the exe: `tools/static_strings.py` (523 static `CString`s) and `tools/static_values.py` (1,253 integer constants and arrays; 46 with expression initialisers are skipped). They found `CHARISMA_STORE_MOD` naming the table `CHRMODS` instead of `CHRMODST`, `CGameSprite::READY_COUNT` 75 instead of 60, and nine wrong address comments. A verifier read the other 83 static initialisers: the CD-switch screen's `dword_8E0320` and `dword_8E0330` had no initial values (exe: (35,545) and (765,48)). Four exe objects have no repo definition and no reader in the listings (0x8D86B8, 0x8E17F0, 0x8E52F8, 0xA09F5C). The three table-like undefined runs are switch index tables.

**Old code in `CGameSprite.cpp`, audited in full (batches 111–113).** All 188 functions of this file that predate the project and had never been read against the exe (107 KB) went to Sonnet verifiers in ten lists of about 10 KB: **57 functions (30%) had a behavioural difference, about 125 differences**. Translators then confirmed each claim against the disassembly before applying it; one claim of about 125 was wrong (`GetInternalButtonList`) and one was left as a note (`FeedBack` parameters are 16-bit in the exe; 205 callers). Cost: about 1.9M tokens of verification and 570k of fixing. The ones a player would notice:
- `EquipAll` called `Unequip` on nine slots; `Unequip`/`UnequipAll` passed the wrong slot; quick-spell buttons were always disabled (inverted test) and quick-item buttons had no type.
- `FindTraps` and `Turn` required both classes (rogue and monk, cleric and paladin) instead of either; `PlayDead` and `OneSwing` had inverted sequence tests.
- `StartSpriteEffect` asserted on every successful allocation; `SetDirection` never reversed the turn direction; `GetDirection` was wrong in one quadrant; `ChangeDirection` lacked the slow-mover throttle.
- `FeedBack`: 12 differences (inverted encumbrance tests, a missing `break`, four wrong formats, two missing song messages).
- `OnActionButton` cast on the ground instead of on the object; `IsSpellSchool` swapped Illusionist and Evoker; `GetCasterLevel` queried one class seven times; weapon specialisation tested base attack instead of fighter level.
- `Render`: tint position, invisibility transparency, blur rounding, blood loop skipping entries; `RenderPortrait` clip and health colour; `RenderMirrorImage` moved the sprite itself.
- `AutoPause` was sent to the wrong sprite in `CheckIfVisible` and ignored the trap option; `AddKnownDivineSpells` skipped two cases; the spell-memorisation helpers searched the wrong domain level and list.

The typical causes: `||` for `&&`, an inverted test, a copy-pasted line not adapted (wrong list, wrong index, wrong slot), a statement that is a bare member name, a missing `break` or `else`, and byte-wide fields compared at full width. Ghidra split one function in two (`sub_720B50` continues in the listing `0x720C2F.asm`).

Not audited: the other 1.76 MB of old code (largest: `CGameEffect.cpp` 103 KB, `CScreenCreateChar.cpp` 93 KB, `CMessage.cpp` 91 KB, `CAIScriptFile.cpp` 69 KB, `CInfGame.cpp` 65 KB). `tools/old_code.py` lists it per file (functions complete at f04e0db whose body is unchanged, minus the audited addresses in `tools/audited_old.txt`). Going wider is the user's decision.

**Open**
- `CScreenCharacter.h` declares `field_2B6` at the offset that is also `m_spells.m_nHighestLevel` (0x1BA + 0xFC); they are probably one member, which would shift every later member's comment by four.
- Code outside functions: 773 of the 867 runs match five exact compiler patterns (static constructor and atexit thunks); the rest are covered above or in batch 109.

## 2026-11-13 (afternoon): batches 88–108, every counted function done; code outside functions found

**State.** The inventory counts 8,020 functions and all are translated or excluded; no `TODO: Incomplete` function stub is left (one marker remains on a struct in `BalDataTypes.h`). S1–S5 and S6 pass on each merged build.

**The count is not the whole exe.** The inventory only knows functions Ghidra defined. `tools/ghidra/OrphanCode.java` asks Ghidra for instructions that belong to no function and for bytes that are neither code nor data: about 36 KB of code in 867 runs and 12 KB of undefined bytes. By pattern, about 24 KB of the code is static initialisers and guarded initialisers and most of the rest is short thunks. What matters: 11 chunks that sit inside known functions but were never in their listings (4.9 KB; the largest are 2.5 KB of `CInfGame::StepAnimation` and 1.1 KB of the `CGameAnimationTypeMonster` constructor) and five standalone runs that look like real code. Batch 109 is checking those against the repo. Not yet done: the 700 short runs have only been classified by pattern, the 12 KB of undefined bytes has not been looked at (the largest pieces are in the zlib range), and the static initialisers have not been compared with the repo's constant values.

**Snowflakes.** The missing flakes were not the particle code: `CSnowStorm::Render` (0x558310) multiplied the tint by the base colour without the `>> 8`, so flakes were drawn near-black. Found by forcing the snow roll with a temporary local patch (not committed); the fixed build draws 221 flake pixels of the original's colour against 228 in the golden. Waiting for a 15% roll had cost 26 S6 runs.

**Scripted exclusions.** `tools/exclude_thunks.py` excludes compiler-generated thunks by exact instruction pattern: scalar deleting destructors (498), static-object thunks (238), guarded static initialisers (534), initialisers with atexit (6). A translator had spent 150k tokens excluding 36 of these by hand.

**Opus for large functions.** Trial: the same two functions (6.2 KB) on Opus and Sonnet in separate worktrees: 244k against 252k tokens, 15 against 23 minutes, 0 against 1 behavioural error. Opus batches since then ran at 14k–25k tokens per KB (Sonnet averaged about 25k this morning) and 43 verified functions had one behavioural error (an extra `cnt += 2` in `CMessageSpriteUpdate::MarshalMessage`). Opus also caught what Sonnet runs had missed: a group assigned as "blitter" was the statically linked GLSetup loader (excluded, 17 KB), zlib and STL in the video range, and several bugs in neighbouring old code. Sonnet remains the verifier and does small functions.

**Functions that were missing and ran as base-class versions:** `CGameSprite` `DoAIUpdate`, `ApplyTriggers`, `CompressTime`, `DebugDump`; door, trigger and container `Render`, `AddEffect`, `OnActionButton`, `CompressTime`, `DoAIUpdate` and their polygon highlights; `CGameAnimationTypeMonster`, `MonsterOld` and `MonsterMulti` `Render`, `SetSequence`, `ChangeDirection`, `GetSndWalk`; `CGameSound` and `CGameSpawning` `DoAIUpdate`; `CGameText::Render`; `CVisualEffect` update and destructor; all `MarshalMessage` functions of 17 message classes; the cheat console (17 commands, parser, edit control); save thumbnails; blended tiles; `CVidMode::ShadowRect` (the header was one virtual slot short at 0xAC).

**Bugs fixed in older hand-written code (each confirmed by a verifier first):** red tint channel computed from the blue byte in four monster renderers; `MonsterQuadrant::Render` used the first quadrant's cell and the wrong bounds for every quadrant; `CInfCursor::SetCursor` case 100 lacked two statements; `CResCache::CopyFile` lacked the capacity check, grew the cache by the wrong amount and tested the wrong buffer; twelve functions in the string-table, override-text, cache and file code lacked their try/catch; `CTlkFileOverrideHeader::Unmarshal` always returned FALSE; the session callback copied the password instead of the session name; the inventory history icon ignored the control origin; `CScreenLoad::RefreshGameSlots` path separator; about 25 wrong or missing address annotations.

**Original bugs kept, with a NOTE:** `CMessageSetLastObject` writes one field two bytes away from where its reader expects it; the 24-bit blended tile blit falls one index behind per row; several loops that cannot terminate under DBCS.

**Tooling fixes.** `packet.py` printed nothing for a string that contains a newline or is shorter than three characters (a translator dropped `" does not exist\n"` from five messages because of it); it now prints strings as Python literals. Agent definitions: builds in the foreground only (two translators left a background build running and re-sent their report up to five times), one report, one line per matching function from verifiers.

**Process notes**
- A verifier can be wrong about where something is: one placed a loop in `~CInfGame` that is in `DestroyGame`; applying its "remove the redundant loop" would have changed behaviour. Read the diff before committing a suggested clean-up.
- Give `sed` nothing with `\n` in the replacement on this machine; it wrote real newlines into string literals. Use a script that writes bytes.
- The S6 world checkpoint tolerance is 4.5%; idle characters alone move it between 2.0% and 3.4%.

**Open**
- Batch 109 results (code outside functions), then the short runs, the undefined bytes and a comparison of static initialisers with the repo's constants.
- `CVisualEffectObject` is a placeholder class: the constructor of visual-effect child objects was never found.
- `CVidInf::DestroySurfaces3d` is declared as an override but is not in the original vtable; `CInfCursor` `field_5E2` is a byte in the original; `CUIPanel::m_ptBaseOrigin` and `CStore::m_bLocalCopy` offset comments are wrong; `CCreatureFile::ReleaseData` takes one ignored argument in the original.
- Earlier items below still stand.

## 2026-11-13 (morning): batches 65–87, snow confirmed on the original, 21 markers left

**S6 weather.** Eight S6 runs on the original produced one snow start (dark world with snowflakes), so the "dark world" is the original's own 15% roll. `checkpoints.py` now records a `name.altN.png` variant when the original shows a screen that matches no golden, and S6 has snow variants for the dock and world regions. On the rebuild a snow start matched the snow golden except that no snowflakes were drawn: `CParticle::Render` had only the 32-bit cases. Batch 80 added the 16- and 24-bit cases. Not yet seen in a play test: 20 S6 runs since the merge rolled no snow.

**S6 tolerance.** The world region differs 2.0–3.4% from the golden on every rebuild run (30 runs measured) because idle characters stand in different poses; the limit was 3.0% and is now 4.5%. A wrong weather state differs by 19% or more.

**Batches merged: 65–87.** Character screen complete (skills panel, cancel and done handlers, `LevelUp`, general and equipment information, the 26 KB character-sheet export 0x5EFFD0); `CGameSprite` `Spell`, `SpellPoint`, `UseItem`, `IsSpellTargetValid`, the `SetCurrAction` spell block, item removal in `ExecuteAction`; `CGameAIBase::FireSpell`/`FireSpellPoint`; spawning (`CGameSpawning::Spawn`, `CSpawnList::Execute`, `CSpawn::ExecuteSpawn`); `CAIGroup::GroupDrawMove` and `GroupProtectPoint`; particles, blood and tiled objects; connection screen timer and EM events; save-slot list; map fog of war; projectile `Fire`/`AIUpdate` for the single-target and BAM projectiles, with visual-effect (VEF) creation; `CTimerWorld::GetCurrentTimeString`; `CSwitchCDStatus` complete; GameSpy sockets, heartbeat and query handling; `CUIControlEncumbrance::Render`.

**Verifier findings fixed in old code:** `CInfGame::GetShapeshifts` returned nothing; `CScreenLoad::RefreshGameSlots` built the save path with `/` and skipped the 8-byte signature for creatures referenced by resref (the original does not, on that path only); `CParticle::Render3d` used divisions for shifts and offset one line end; an extra `Unlock` in the map control's `Render`.

**GameSpy.** `sub_4D0980` does `WSAStartup` and an asynchronous DNS lookup of `master.gamespy.com` with a wait of up to 3 s. It only runs when hosting a TCP/IP session with `[GameSpy] Enabled=1` in the ini, never at start-up. The two response senders (0x4D0F40, 0x4D1220) need the key-encoding class `CUnknown1`, in progress.

**Progress:** 7,857 of 9,647 counted functions complete (81.4%), 93.8% of bytes. 21 `TODO: Incomplete` markers remain, in 13 files; the rest of the gap (about 227 KB) is functions that have no body in the repo yet.

**Process notes**
- Five agents at once ran into the 5-hour usage limit; three or four is the ceiling.
- A `CGameEffect` asm offset is the header comment plus 4 (vtable first); a `CMessage` constructor takes the caller, then the target. Both are in the translate-function skill now.
- A translator's doubt is often wrong in the other direction: two "suspected errors" in neighbouring old code were real, one was not (`CScreenLoad` has no `>= 4` slot-number condition; only `CScreenSave` does). Always send such doubts to a verifier before changing anything.
- A stray file with a mangled name (ending `verify_d_packet.txt`) sits untracked in the batch3 worktree root; left for the owner to delete.

**Open**
- Confirm snowflakes on the rebuild in a snow-start S6 run.
- `CVisualEffect` has no translated destructor (components leak); lists at 0x24A/0x266 are typed from one constructor only.
- Flag constants 0x80, 0x40, 0x20, 0x10, 0x8, 0x4 next to `CUIControlLabel::TYPE_WORD_WRAP` are literals.
- Earlier items below still stand.

## 2026-11-13: batches 47–64, S6 stall fixed, old code audited

**S6.** The dock member that stopped short was `CGameArea::SortLists` (0x4794E0), an old hand-written function: it never re-sorted the y-sorted object list after the first render (inverted `m_firstRender` test), its bubble sort advanced the wrong variable, and the merge read the wrong half of the array. `GetAllInRange` depends on that order, so a blocked sprite found no neighbour to bump. A verifier pass over the whole function then found three more differences (an early return that skipped the back/flight/remove lists, a missing count decrement for deleted objects, `InsertAfter` for `InsertBefore`). All fixed. S6 now passes most runs; the remaining failures are the world region at exactly the 3.0% limit.

**The "dark world" is weather, not a bug.** AR1000 has a 15% snow chance, rolled once when the game clock reaches 07:00 a few seconds after the scenario starts; a snow start tints the world with the overcast colour (180/255). `CheckWeather` and `SetCurrentWeather` match the asm, and `rand` is seeded from the clock on the original too. Not yet observed on the original (it would take about ten runs). The S6 goldens need a second variant for a snow start, or the test must pin the weather.

**Other old code fixed by the same verifier pass:** `CWeather::SetCurrentWeather` had its network guard inverted for multiplayer clients; `CInfinity::SetDawn`/`SetDusk` wrote the rain colour into the global-lighting member in the daytime branch.

**Batches merged: 47–64** (store screen buying and selling; `CProjectileArea84F0DC` with its cloud object `CProjectileArea84F14C` and animation `CGameAnimationType84EEB8`; `CProjectileBAM84E7B0::AIUpdate`; multi-line edit control complete, including the 21 KB `OnKeyDown`; single-line edit `OnKeyDown` re-derived, old version was missing the IME block, Esc, symbol keys and DBCS paths; character creation complete; multiplayer screen complete; world-map screen complete with path search; `CInfGame` complete (CRE 2.1→2.2 converter); `CChitin` complete (window procedure, `OnDisplayChange`, `ParseCommandLine`); `CDimm::CDSwitch`; movie player with subtitles). Verifier findings fixed: `IsCharacterAlive` return on a failed share, effect `m_flags` versus `m_effectAmount2`, `CGameAnimationTypeEffect::IsMirroring` comparing the sequence instead of the direction, a link pointer in the world-map click handler, a 16-bit store of the animation type, `WM_INPUTLANGCHANGE` returning 0 unconditionally.

**Tooling.** `tools/ghidra/DefineFunctions.java` defines and exports a function Ghidra's analysis missed (reached only through a vtable); merge its `functions.extra.json` into `functions.json`. Used for 0x534140.

**Progress:** 7,784 of 9,652 counted functions complete (80.6%), 89.9% of bytes, after batch 64. About 50 more addresses went to the exclusions file (STL, MFC templates, emitted copies of inline accessors, a catch funclet).

**Process notes**
- Several functions the worklist attributes to a file are library code attributed by caller; translators now read the asm first and exclude those.
- Translators must be told to run every command from their worktree: one built in the main tree by mistake.
- When two agents share a scratch folder, give each a file-name prefix.

**Open**
- S6: add a snow-start golden or pin the weather; world region limit.
- `CDimm::WriteFile` has no try/catch, so its catch handler (0x7891E1) is excluded rather than translated.
- `sub_428AD0` (subtitle table loader) belongs in `CBaldurChitin.cpp`; it lives in `CBaldurProjector.cpp`.
- Parameter types still declared `const unsigned int&` where the original passes a byte by reference: `RemoveKnownSpell` a4, `sub_725210`/`sub_725270` a3, the grouped list `Remove` third argument.
- Earlier items still open: `m_bHardPaused`/`m_bPaused` width, `SetResRef(const CResRef&)` callers, `CheckRestEncounter` location, `CCreatureFile::ReleaseData`.
- Remaining large functions: `CScreenCharacter` 0x5DC9D0, 0x5E2210, 0x5EFFD0 (26 KB); `CGameSprite` `ExecuteAction`, `Spell`, `SpellPoint`; and about 230 KB of functions with no translated caller.

## 2026-11-12 (late night): S6, the party walks, batches 34–46, packed build

**Play test S6 (walk the party off the ship).** New test: drag-select the party, click on the dock, compare after 20 s. It passes on the original and found, on the rebuild:
- The party did not walk. `CAIGroup::GroupSetTarget` and friends were stubs (batch 39), and then ten separate errors in older path-finding code: `CGameSprite::MoveToPoint` and `AIUpdateWalk`, `SearchThreadMain`, four `CPathSearch` functions, two `CSearchBitmap` snapshot stubs, `CMessageSetPath`. A follow-up audit of the rest of that subsystem by verifiers found seven more differences (`GetLOSCost`, `GetCost`, `AddObject`/`RemoveObject` clamping, `SetPath`, the `CPathSearch` constructor, two swapped members). All fixed.
- Fog of war drew fully fogged tiles as triangles (`CVidInf::LoadFogOWarSurfaces` used frame 2 for the fourth quadrant).
- World-screen portraits lacked the button background (`CUIControlPortraitWorld::Render` did not call the base Render).
- A red ring on an NPC: `CGameSprite::SetCurrAction` (0x7338E0) was missing. Added; its spell set-up block (0x7340BD–0x73452E) is still a TODO.
S6 now passes about one run in three on the rebuild. Remaining: one character sometimes stops short of its formation spot (world view lands at 3.0–3.4% against a 3.0% limit), and occasionally the whole world is drawn about 30% darker.

**Batches merged:** 34–46 (chunk classes and `Shatter`, leave-area-by-name, `CMessageSpriteUpdate::Run`, `CProjectilePart1/6`, group movement, death-effect shatter, `CGameSprite` quick buttons, character screen panels and level-up sequence, world-map travel handler, rest party, record converters). Verifier findings confirmed and fixed along the way: terrain table choice in six projectile/chunk constructors, `RemoveFromArea` returning after its assert, a mirrored formation direction (−360.0 constant), reversed list order in a projectile helper, byte-wide reads of the local player slot.

**Packed build.** The game itself was being built with default struct packing; only the layout test used `/Zp2`. The Windows SDK refuses `/Zp2`, so `src/pack_prefix.h` is now force-included: system headers first, then `#pragma pack(2)`. S1–S6 pass on it (same S6 caveats).

**Progress:** 7,685 of 9,703 counted functions without TODO (79.2%), 86.1% of bytes, before batch 46.

**Process notes**
- Older hand-written translations are a bigger source of runtime bugs than missing code. Auditing a subsystem with verifiers (read-only, about 60–150k tokens per file) before debugging it at runtime is cheap and found real errors every time.
- `tools/inventory_exclusions.txt` uses a union merge so parallel batches can both append.
- Play test goldens of animated world views need limits taken from original-versus-original runs; 3% is tight for S6.

**Open**
- S6 items above; S5 dialogue-step flake not seen again.
- `CScreenWorld::m_bHardPaused` / `m_bPaused` should be `BOOLEAN` (the original reads one byte; about 26 uses).
- `CGameSprite::SetResRef(const CResRef&)` at 0x42FDC0 has the wrong signature (it takes a CString by value); a correct overload was added, callers in `CGameArea.cpp` still use the old one.
- `CGameArea::CheckRestEncounter` (0x46FEA0) sits at the end of `CInfGame.cpp`; move it to `CGameArea.cpp`.
- `CCreatureFile::ReleaseData` takes one stack argument in the original.
- Stubs added: `CBaldurMessage::sub_435C30`, `CScreenCharacter::sub_5E87E0`, `sub_5F7B00`, `sub_5EFFD0` (26 KB), `LevelUp` (0x5E56F0), `CVisualEffect::SetAllDelete` (class missing).

## 2026-11-12 (night): batches 23–33

**Merged.** Every batch went translator → verifier(s) → fixes confirmed against the listing → merge → `tools\check.ps1` → S1–S5.
- 23, 26, 28, 31: `CMessage.cpp` — 13 + 31 + 18 + 10 functions (resource demand, character slot updates, party order, world map, movies, rest, time/weather sync, leave-area, map notes, delete-area vote, two unmarshallers), plus `CGameAreaNotes::Add`/`DeleteANote` and `CGameSprite::sub_6F3B80`.
- 24: `CDerivedStats::Reload` and `operator=` completed.
- 25, 27, 29: `CProjectilePart3`, `Part5`, `Part4`, `Part6` (71 functions).
- 30, 32: new class `CGameSparkle` (constructor 0x51AE40, AddToArea, AIUpdate, Render), the projectile blocks that were waiting on it, `CGameEffectSparkle::ApplyEffect`, eight `CProjectilePart1` functions.
- 33: `CInfGame::sub_5AE020` (party order) and `sub_5C3B20` (leave area, Lua variant).

**Progress:** 7,592 of 9,752 counted functions without TODO (77.9%), 82.7% of bytes, as of batch 31; S1–S5 pass on every merged build.

**What the verifiers caught (all confirmed and fixed).**
- `CMessage51::MarshalMessage` released the wrong id; `CDerivedStats::Reload` sign-extended a byte the original zero-extends.
- `CProjectileArea850420::AIUpdate` ran its range queries on `m_pArea` instead of `field_EA`.
- `CGameEffectSparkle::ApplyEffect` passed the wrong effect fields to the sparkle constructor.
- `CMessageSpriteUpdate` declared twelve fields overlapping its spell-state bitset, shifting every later member by 0x18. The class now passes the layout test.
- `CGameAreaNotes::GetNoteAt` returns the last note when none matches in the original.
- Smaller shape fixes (single `GetPos` call, a shift instead of a divide).

**Library code.** About 75 addresses are now in `tools/inventory_exclusions.txt` (MSVC 6 `std::set`/`map`/`list`/`vector`/`string` internals, `CTypedPtrList` constructors, scalar deleting destructors, template instantiations). Translators are told to exclude these with evidence rather than translate them.

**Tool fix.** `tools/packet.py` named stack slots 8 bytes off after `PUSH a; PUSH b; CALL [vtbl+getter]; PUSH ...; CALL f`. Pushes left by a zero-argument getter (a fixed list of common `CGameObject` slots) are now carried to the next call. Other zero-argument virtual calls (for example vtbl+0xA0, `GetTerrainTable`) can still cause this; translators and verifiers are told to check such spans against the raw listing.

**Open**
- Stubs added by these batches: `CInfGame::sub_5C4AD0` (10 KB, in progress), `CScreenWorldMap::sub_69D0F0` (11.8 KB), `CBaldurMessage::sub_437D70`, `CGameSparkle::sub_51CE50`, and the two object classes created by projectiles (vtables 0x85BFE8 and 0x85BF7C, in progress).
- `CScreenWorld::m_bHardPaused` and `m_bPaused` are declared `BOOL` but the original compares one byte.
- `CCreatureFile::ReleaseData` (0x5C7B00) takes one stack argument in the original; the repo declares none.
- Four `CBaldurMessage` party-order functions read network flags through `g_pChitin` where the original uses `g_pBaldurChitin` (same object).
- `CGameSprite::Animate` (0x728560) looks misnamed: it tests "not dead".
- S5 flake (dialogue keys out of step) has not recurred in about 15 runs since the pauses were lengthened; cause still unknown.

## 2026-11-12 (S5 follow-up): inventory differences fixed, batches 19–22

**Inventory screen.** The three differences S5 left open are fixed:
- Dark wedges beside portraits 1 and 3: `CVidCell::Blt8To32` advanced the wrong pointer at the left clip and used the unclipped width for the row jump.
- Missing status icon on portrait 5: the day blindness effect queued in `CGameSprite::AIUpdate` had no duration type (the original writes 1 from the constant at 0x8491AA), so it expired and was re-queued every tick.
- Paper-doll pose: `CGameSprite::CanSaveGame` (0x75E890) and the 0x47C830 override on eight object classes were missing, so the area always reported "cannot save" and sprites used the READY idle.

**Batches merged.**
- 19: `CInfGame::sub_5BA2E0` and 16 `CProjectilePart6` methods. All verified as matching.
- 20: 16 `CBaldurMessage` handlers. All verified as matching.
- 21: ten `CMessage` functions, including `CMessageSaveGame::Run`. The verifier found one difference (`CMessage51::MarshalMessage` released the wrong id); fixed.
- 22: `CDerivedStats` constructor and `BonusInit`, with the members at 0x300, 0x328, 0x378, 0x4AC–0x538 and 0xCAC now declared. The other 34 functions of that batch are standard-library or compiler-generated code and are listed in the new `tools/inventory_exclusions.txt`.

**Progress:** 7,449 of 9,797 counted functions have no TODO (76.0%); 80.4% of bytes. S1–S5 pass on the rebuild.

**S5 changes.** It now compares the portrait strip. It failed three times in about a dozen runs with the dialogue on a different branch (keys out of step with the dialogue). Pauses between steps are now 3 s and it passed nine runs in a row, but the cause was not established. An attempt to wait on screen changes instead of sleeping did not work: one node shows no change after the reply key, and the bottom of the screen never settles once the dialogue is over.

**Open**
- `CVidInf::BKTextOut` (`src/CVidInf.cpp`, near line 1790) assigns `rNewClip.top` twice and never sets `.right`. Not checked against the listing yet.
- Other callers of `CGameEffect::ClearItemEffect` may also need the duration type the original writes.
- `CBaldurEngine::UpdateLabel` is declared static, but the original pushes the engine as a first argument.
- `CMessage51::MarshalMessage` sizes its buffer one byte short in the original (kept, with a note).

## 2026-11-12 (S5): finish the dialogue, open the inventory

Branch `phase-a/stubs-and-layout` (local only). 7,402 of 9,831 functions without a TODO (75.3%). S1–S5 pass on the rebuild.

**S5** (`tests/e2e/scenarios/test_s5_leave_dialog_inventory.py`) finishes Hedron's dialogue and opens the inventory. It compares small regions around each number with a 0.4% limit, because a wrong digit changes too few pixels to fail the default 2%.

**Bugs it found** (all fixed and checked against the asm)
- `CUIControlTextDisplay::RemoveString` left deleted lines in the list and `Render` never took the display's lock: crash when the dialogue ended.
- `CDerivedStats::Reload`: ability scores copied from the wrong bytes, base AC not reduced by 10, immunity lists not cleared, missing tail (Dodge and other bonuses).
- `CGameSprite::AddEffect` was missing, so item effects were discarded; `CGameEffect`'s `ITEM_EFFECT` constructor left `m_done` uninitialised.
- Inventory: weight colour ratio, missing slot-button update (selected weapon highlight).
- All 24 functions that use the ability-score members were audited by offset after the rename; older code had used several of them by name.

**Still different from the original on the inventory screen** (not covered by S5 yet)
- Dark wedges are drawn beside the first and third portraits. Probably `CUIPanel`'s dimming polygon (0x4D3610, `BKRenderConvexPoly`) for a panel that should not be dimmed, or a bad polygon fill. Varies from run to run.
- The fifth portrait has a small status icon in the original and none in the rebuild.
- The paper doll holds the staff horizontally in the rebuild and diagonally in the original.
- `CInfGame::sub_5BA2E0` ("can this item go in this slot", 2.4 KB) is a stub returning TRUE.
- `CDerivedStats::Reload` and `BonusInit` still don't clear the lists at 0x300, 0x328, 0x378, 0x4AC–0x538 and 0x554.

**Next**
1. The three visible differences above, then extend S5 to cover them.
2. S6: walk the party off the ship (pathfinding, scrolling, the newly translated wall-polygon renderer).
3. More translation batches from `tools/worklist.py`.

**Notes**
- Runtime debugging on Sonnet works when the brief lists the symptoms with exact values, the test command, and suspects. This one cost 413k tokens and changed 12 files, so everything it touched went through verifiers before commit (three verifiers, about 350k tokens).
- After any rename of members that older code might have used by meaning, audit every use against the asm.

## 2026-11-12 (later): batches 13–18

Branch `phase-a/stubs-and-layout` (local only). 7,396 of 9,831 functions without a TODO (75.2%); 79.4% of bytes. S1–S4 pass on the rebuild.

**Done**
- Sprite: quick-slot button lists, feats update, drop/give item, use container, casting sounds and effects (`sub_755A70`, `SpellCastEffect`), `UseItemPoint`, `ChangeAnimation`, `StartStore`, weapon swing sounds, `AddSpecialAbility`, `sub_727720`.
- Game and area: `SwapCharacters`, `ReputationAdjustment`, nearest-object searches, `GetGlobalVariable`, `MoveCursor`.
- Renderer: `FXRenderClippingPolys` (wall polygons over sprites), `ScrollingRequestRect`, `Scroll`.
- Screens: character (`CheckDropSlot`, help lists), store spell button, import-character summary.
- Renamed the mislabelled ability-score members in `CCreatureFileHeader` (real order STR, INT, WIS, DEX, CON, CHR).

**Verifier findings that were real** (all fixed): `ChangeAnimation` swapped source and target in three effect messages; `StartStore` didn't fall through to the local path; `sub_722740` takes a `CString` reference; a 16-bit field needed zero-extending.

**Waiting on classes that don't exist yet**
- `CGameSprite::Shatter` needs `CGameChunk` (ctor 0x70F4F0).
- `CGameEffectSparkle::ApplyEffect` needs the particle-cluster class (ctor 0x51AE40, 5 KB).
- `CScreenCharacter::sub_5E4560` needs 0x5EFFD0 (26 KB).
- `CGameSprite::HandleEffects` needs `CVisualEffect::SetAllDelete` (0x555200).

**Notes**
- One translator run handles about 10 KB of original code. Functions over 2 KB with jump tables go two per batch.
- Translators are told to commit after each group that builds, so a rate-limit interruption loses nothing. Interrupted agents resume with SendMessage.
- `CResHelper::SetResRef` in `CRes.h` is behaviourally right; the original tests `== ""` first with the branches swapped.

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
- The ability-score members in `FileFormat.h` were mislabelled and are now renamed. The real order at base stats +0x25E is STR, INT, WIS, DEX, CON, CHR (confirmed from the labels the import-character summary prints beside each offset). Offsets and behaviour did not change.
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
