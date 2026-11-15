---
name: record-scenario
description: Author a new play test (tests/e2e/scenarios/test_sN_*.py) - explore the original game, pick stable checkpoints, record golden screenshots on the original, then try the rebuild once and report what differs. Use when asked to add a play test or to cover a screen or behaviour no scenario shows yet.
---

# Record a play-test scenario

A scenario drives the real game window with mouse and keyboard. Run on the
original (`--target original`) it records golden screenshots; run on the
rebuild (`--target reimpl`) it must match them. A scenario that fails on the
rebuild is the point: it is a finding, not something to tune away.

## Hard rules

- One game process at a time. Runs take over the mouse and keyboard. Check
  the previous game has exited (`tasklist`) before starting the next. Long
  foreground commands get moved to the background after about 3 minutes, so
  don't assume a run has finished because the command returned.
- Only the harness's installs (`C:\iwd2-play`, reference `C:\iwd2-ref`). Never
  touch or launch anything under `C:\GOG Games`.
- Don't rebuild and don't edit `src/` while authoring. Don't fix game code:
  report what differs.
- You may add `tests/e2e/scenarios/test_sN_<name>.py` (and a shared helper in
  that folder if needed). Goldens (`tests/e2e/golden/`) and outputs
  (`tests/e2e/out/`) are git-ignored: never force-add them. Don't commit or
  push; the orchestrator reviews and commits.
- Exploration output goes in the scratchpad, not the repo.

## Read first

`tests/e2e/drive.py` (exploration driver), `tests/e2e/harness/game.py`,
`tests/e2e/scenarios/checkpoints.py`, and the two newest scenarios for style
(`check(...)` collecting failures, region checkpoints, parking the pointer,
`act()` in `test_s8_screens_tour.py`). Python is
`C:\iwd2-tools\venv\Scripts\python.exe -m pytest <file> --target original|reimpl -q`.

## Explore (original only)

- Most scenarios start from S5's end state (new game, premade party, Hedron's
  dialogue finished), which takes about 2.5 minutes to reach. Keep one game
  running after that set-up and feed it commands, rather than restarting for
  every probe.
- Look at the screenshots you take before choosing regions.
- To pick stable regions, grab the screen repeatedly for 5 s and see which
  pixels change.

Known coordinates (800x600):

- Dial buttons, bottom right: Spellbook (620,527) S; Inventory (656,525) I;
  Journal (689,529) J; Area Map (619,562) M; Record (656,569) R; Options
  (690,561) O; Select All (585,509) =; Rest (720,511) Z.
- Portraits: (528+49*i, 462). Premade party left to right: Sheris (bard),
  Geldin, Khatja, Laeris (cleric), Vraedna (sorcerer), Kylie.
- Regions: world view (0,0,800,430); action bar (0,435,500,55); portraits
  (500,435,300,55); message window (8,500,535,90). Park the pointer at
  (300,560).

## Write the scenario

1. **Every input must prove it changed the screen.** Grab before, do the
   input, wait until a still region differs, fail after 10 s. The original
   sometimes drops a key; without this check the dropped input is recorded as
   a golden.
2. **Detect changes in a still region.** The world view always moves (water,
   idle poses). Going to or from the world screen shows in the action bar,
   which is blank on every other screen.
3. **Give every number its own small region with a tight limit (0.4%).** One
   wrong digit is about 0.015% of the screen and passes a 2% whole-screen
   limit.
4. Leave out what varies on the original itself: the clock globe, portrait
   frames outside the world screen, idle poses, anything tied to the system
   clock. Restrict the region or mask it; don't raise the limit.
5. Press shortcut keys only from the world screen. Switching straight from
   one screen to another by key is unreliable on the original.
6. Selecting a portrait inside a screen changes the world selection too: put
   it back.
7. Take every checkpoint even after one fails (collect failures, assert at
   the end), so one rebuild run shows all differences.
8. Timing that has worked: 2 s after each input plus a 1 s settle.
9. Don't save or load, and don't change settings, unless that is what the
   scenario is about; then restore what you changed.

## Record

Run on the original at least 3 times in a row. The first run records; later
runs compare. **On the original a mismatch never fails: `checkpoint()` saves
a `name.altN.png` variant instead.** After every run list the golden folder
for new `.alt*` files and look at each one. A variant is only acceptable if
it is a real, explainable variation of the original (day/night menu art). If
it shows a dropped input or a different screen, delete it and fix the
scenario. If a checkpoint flakes on the original, tighten its region or drop
it, and say so.

## Try the rebuild

Run once with `--target reimpl` (the existing build). For each checkpoint
report the diff percentage; for each failure look at both images and say
what differs in words (which number, which label, which control). Keep the
output folder (`tests/e2e/out/<run>/`, including `iwd2re-stacks.txt` if
present) and name it in the report. If the result looks timing-dependent,
say so; don't rerun until it passes.

## Report

Steps and checkpoints (name, region, limit); original results per run and
any variants; rebuild result per checkpoint with what differs; what was
dropped and why; run time; new coordinates and lessons worth adding to this
skill.

## Targos docks (AR1000) notes

- Move the view with the area map, not the arrow keys (their distance depends
  on the frame rate). The map is drawn at 1/8 scale with its top-left at
  screen (138,73): a click at (x,y) centres the view on map point
  ((x-138)*8, (y-73)*8).
- Clicks on unexplored black map are ignored: walk in steps onto ground
  already shown.
- Hostile creatures can be found by the pure red (255,0,40) of their
  selection circle.
- The parked pointer at (300,560) covers the bottom message line; move it
  before comparing that line.
- Combat messages, hit points and gold amounts vary run to run on the
  original. "The party has gained experience: N" does not.
- `test_s9_docks.py` lists people, doors and exits with map coordinates.
- Reading game files: the S9 author's `res.py` / `are.py` (scratchpad `s9`)
  read a resource from CHITIN.KEY and list an area's actors, regions,
  containers and doors.

## Notes from S10 and S11

- Correction to rule 6 above: on the original, clicking a portrait inside the
  inventory or spellbook does not change the world selection.
- Saves go to `C:\iwd2-play\mpsave\<number>-<name>\` for single player too.
  A test that saves must clean up in a fixture requested before `game` (so it
  tears down after the game exits); see `test_s10_save_load.py`.
- Type names in lowercase letters that are not shortcuts, one key at a time,
  each proved by a change in the edit box. The original drops "s" in edit
  boxes and R on the world screen now and then: prefer buttons to keys.
- Message lines are not at a fixed height when a multi-line hint is above
  them; find the line by colour.
- Weather (snow) varies between runs on the original: a world-view variant
  that differs only by snow is acceptable.
- Options panel in game: Load (612,81), Save (612,111), Quit (612,141). Quit
  prompt: Save (398,230), Quit (398,260), Cancel (398,290). Save/load slot
  rows are 102 px apart with their button at (684,101+102*i).
- Inventory: ground slots at x 590/630, y 318/356/394; backpack row at y 318,
  x = 270 + 37.5*i; armour slot (272,40); weapon set 1 (618,65)/(659,65),
  set 2 (618,105). Ground slots show only items under the character.
- Action bar: quick weapon (63,463), right-click for weapon sets; Cast Spell
  for a cleric (174,463), then class, then spell icons from (26,463).
- Spellbook: memorised grid at (305+79*k, 95); known rows at x 615,
  y = 95 + 42.3*row; remove prompt Cancel (334,258), Remove (455,258).
- Resting on the docks at the start is refused ("You may not rest at this
  time."). AR1000 has no merchant.
- Parks to avoid: (430,465) shows a tooltip when a caster is selected.
- Beyond screenshots: compare what the two targets write to disk (S10's
  saves were unpacked and diffed, which found a bug no checkpoint showed).
  Random loot and gold differ between any two runs.

## Notes from S12 and S13

- Cross-target tests (S12): one target makes a save, the other loads it; keep
  what the first target showed in a bank folder under the golden directory.
- The party does not fight back on its own on the original. Order it: Select
  All, Attack (63,463), click the creature. Do it paused (Space): the world
  view freezes completely (`diff_ratio` 0) and the last line reads PAUSED.
- With several hostile circles, cluster the red pixels (`hostile_circles()`
  in `test_s13_house.py`) rather than averaging them.
- "The party has gained experience" can come up to a second after the death
  line on both targets. Wait 1.5 s after a kill before the next input that
  prints a line, or the order of lines varies.
- Message lines are 13 px apart; descenders of the line above reach into a
  one-line region.
- A panel open inside a dark interior turns the action-bar strip black
  (mean < 30).
- Locked container: a non-thief clicks it first ("LOCKED") and walks there,
  then Kylie (portrait 6) uses Thieving (268,463) on it.
- AR1005: screen = map point (x+84, y-10). Footlocker (558,238), exit
  (200,350), open floor (450,215). Goblin loot is random.
- `docks_to_house()` in `test_s9_docks.py` is the shared set-up for anything
  starting in the house. It fails now and then in its own fight ("no goblin
  came", "click on Reig did not change the screen"): rerun.

## Notes from S14

- AR1000 to AR1100 (the town): area-map jump (170,100), walk to the foot of
  the log stairs (225,370), click the exit (190,60). Travel is direct; no
  world map is reachable at the start of the game. The way back is (230,395)
  in the arrival view. With part of the party selected an exit prints "You
  must gather your party before venturing forth."
- AR1006 (house by the cliff): door at map 259-306,1649-1713, a goblin
  attacks when it opens; inside, the exit is at screen (200,330). AR1001 and
  AR1002 are locked.
- Reig starts his dialogue himself when the party passes near him. S9's
  goblin comes when the party reaches the snow west of the AR1005 house.
- A click on the area map where the view already is changes nothing: skip
  that jump or `act()` fails.
- Select All straight after a fight can be lost: check the group action bar
  and retry.
- The loading screen can be up for under a second: poll for two equal,
  non-dark frames while the action bar is gone instead of `wait_stable`.
  Its hint is random and is also printed in the message window on arrival.
- Interior lamps flicker: keep them and the party out of interior regions.
- `server.py` in scratchpad `s14` keeps one game open at S6's end state and
  runs command files against it.

## Notes from S15

- Talking to a neutral character: click the centre of their cyan circle (the
  feet). Characters standing among the party can't be clicked.
- Story limits at the start of the game: no inn room and no rest anywhere
  until the docks are cleared; Oswald Fiddlebender (AR1100, by the airship)
  is the only open store (buy, sell, identify).
- AR1100 area map: 1/8 scale, top-left at (158,85). Area-map clicks into
  black are accepted; it is world clicks on unexplored ground that are
  ignored. Keep each walk step under about 250 px (about 60 px/s).
- Store screen: Buy/Sell panel (65,117), Identify panel (65,200), first store
  item (155,130), first party item (420,130), Buy (721,203), Sell (721,232),
  Done (721,397); right-click an item for its description. Park at (300,540).
- Gold is random at party creation: read the digits (`read_number` in
  `test_s15_town.py`) and check differences, never a picture of the amount.
- Travel between areas writes an "Auto-Save" slot. `conftest.py` clears
  leftover saves before every test; a test that needs a save in place must
  put it there in its own fixture (as S12 does).
- A walk or fight click can land on a party member and leave only that one
  selected. `travel()` in `test_s14_travel.py` selects all again before an
  exit; do the same before any step that needs the whole party.
- `docks_to_town()` in `test_s14_travel.py` is the shared set-up for the town.

## Notes from S16

- **Difficulty mask.** Byte +0x2F of each ARE actor says at which difficulty
  settings it exists; bit 0 is the default. Read it before planning fights:
  at the default the docks have 8 goblins, not 29.
- **Snow keeps falling while paused** (about 0.1% of the world view changes).
  A "nothing moved" check needs a threshold of about 0.3% outdoors. Prove the
  Space key by the new PAUSED/UNPAUSED line instead.
- **A checkpoint `mask` is relative to its region**, not to the screen.
- **A message line copied to a fixed spot** needs its background blacked out:
  the window texture differs by height. Find yellow lines by colour (r and g
  above 200, b below 40) and compare the last one, since the experience line
  can come after PAUSED/UNPAUSED.
- **Target clicks.** A goblin's red circle can be half hidden behind a party
  member's, so its centre is found off to one side. Retry a few aims round it.
- **Hedron after the opening** stands at 2503,900 on his ship. Walk the whole
  party to 2380,830 first, wait for his cyan circle, then click its centre.
  His rest (replies 1, 1, 2) is refused if anyone is still on the dock.
- **Reading globals.** Quick save (Q) during exploration, then read
  ICEWIND2.GAM: the value is at the name + 0x28. `conftest` clears the save.
- **Long walks.** From the east pier to Hedron's ship the path goes round
  the bay (west through the gap by the half-built ship, about a minute);
  on both games some characters first walk to the pier's east dead end.
  Before an order that needs the whole party together, wait until no green
  arrow at the edge of the world view points at a member out of sight
  (`party_arrows()` in `test_s16_clear_docks.py`).
- **A walk click into black can be dropped** when the party is slow to come
  into sight of the spot: wait until the target is no longer pure black
  (`unexplored()`).
- **Reading positions while exploring:** a quick save's ICEWIND2.GAM holds
  each party member's area and x,y at +0x18/+0x20 of its 0x340-byte entry
  (table offset at file offset 0x20).
- **Keep a scenario under about 10 minutes.** Longer story stretches should
  start from a save banked by an earlier scenario.

## Notes from S17

- **Banked saves.** `golden/bank/<state>/save/<n>-Quick-Save/` plus
  `world.png` (a frame with the group action bar, taken just before Q).
  `s16-end` is S16's end state (made by `test_s17_make_s16_save`, original
  only, skips while the bank exists); `s17-end` is S17's end, for S18.
  `load_bank()` and the `from_s16_save` fixture in `test_s17_dock_houses.py`
  show how to load one (Load Game, row 1 after "Autosave - Prologue") and
  clean up. Entering the AR1000 houses writes no Auto-Save.
- **Long walk clicks are ignored** even on explored ground (from Hedron's
  deck to the warehouse pier the party did not move): walk in steps.
- **Brogan** (2047,532, guarding AR1002) starts his dialogue himself when
  the party comes along the warehouse pier, and from then on prints white
  overhead lines into the message window every few seconds, anywhere on
  the docks. Find a short line like "LOCKED" by its width.
- **A locked door clicked with the whole party selected** printed nothing
  for 9 s (once, later, "LOCKED" came). With one character selected the
  line comes in about 2 s.
- AR1004 (Firtha's house): view on entering shows map (x, y) at screen
  (x+79, y-80); Firtha's circle (581,112); nine cats wander the floor (only
  the back wall is still); exit (319,360).
- S16's `party_arrows()` wait once failed with all six on deck (an arrow at
  the bottom edge pointing at nobody visible); a rerun passed.

## Notes from S18

- **The docks gate needs AR1007** (the goblin caves behind AR1002's back
  door), not only the five houses: at the default difficulty 17 goblins and
  sappers and Rukwurm in five groups. The award ("You have cleared the
  Targos Docks of goblins." pale green, "The party has gained experience.",
  "Your journal has been updated.", entry 27866) comes from AR1007's script
  the moment the last one dies; nobody has to be told. Brogan (on the pier
  afterwards) has new lines, a second journal entry and experience, and
  leaves for the town. There is no gate soldier.
- **AR1002's lock**: Kylie, Thieving (268,463), the door: "Kylie- Lock Pick
  Succeeded" in about 2 s (her name is orange: find the line by that). It
  has not failed yet; "Lock Pick Failed" is retried.
- **Search maps** (`<AREA>SR.BMP`, one pixel per 16x12 map pixels; 0 is
  rock) show the walkable layout of a dark cave much better than the area
  map. Two AR1007 goblins in the southern passage never notice a party in the
  northern one and must be walked to.
- **Fights out of view.** Goblins attack the party wherever it is; a fight
  loop that only looks at the current view misses them. Centre the view on
  the party first: its markers on the area map are dark green (0,160,0).
- **Which creatures are still alive:** quick save, then unpack the area
  from ICEWIND2.SAV (`SAV V1.0`: name, sizes, zlib data per file) and read
  each ARE actor's embedded CRE (offset/size at actor +0x88): hit points at
  CRE +0x24, state at +0x20 (0x800 dead). `sav.py` in scratchpad `s18`.
- **Clicking a doorway the party stands in** selects a party member instead.
  Click the door from a distance (the party walks there and opens it), then
  the doorway with everyone selected (`enter()` in S17).
- **A yellow creature name** ("Goblin Archer-") passes S16's
  `last_yellow_line` colour test: require yellow beyond x=150 for an
  experience line (`last_xp_line` in `test_s18_warehouse_caves.py`).
- **Lines that scroll away**: the award lines are printed mid-fight and
  scroll out under the combat lines. Wrap `game.grab` while fighting and keep
  the last frame that shows them (`WatchAward`).
- The area-map button click is dropped now and then: retry it.
- **S16's pause check fails while walk-target markers are on screen** (the
  green crosshairs keep turning while paused, so the view never looks
  still). Prove Space by the width of the last red line instead: PAUSED
  ends near x=62, UNPAUSED near x=81 (`pause_line()` in S18).
- **A dialogue's first panel can show the message lines from before it**
  above the speaker's line (here the award lines and a random loading
  hint), and the gap to the replies shifts by a few rows with them. Compare
  the speaker's line and the reply as separate text-only blocks found by
  colour (`speaker_block()` in S18).
- **Aiming above a goblin at the top edge of the view clicks the window's
  title bar**; S16's aims (up to 24 px above the circle) once hit the close
  button and brought up the quit prompt. Leave circles within 30 px of the
  top edge out and re-centre instead.
- S18 runs about 14 minutes after the load (the AR1007 clear alone is about
  six): over the 10-minute guideline, but the caves cannot be split at a
  save without leaving the award out.
