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
