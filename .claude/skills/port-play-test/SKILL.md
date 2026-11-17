---
name: port-play-test
description: Run the e2e play tests against a macOS-port build (real input, virtual, or driver mode) without disturbing the main session, and decide whether a failure is a port regression or a known intermittent. Use whenever a port milestone gate or a port change needs the play tests.
---

# Play tests on a port build

The scenarios in `tests/e2e/scenarios/` start the game, drive it and compare screenshots with
goldens recorded on the original game. For the port they answer one question: **does this
build behave like the default build?** It's not whether it is perfect: the default build has
its own known failures.

## Before any run

1. **Your worktree has the goldens junction** (`tests/e2e/golden` → the main tree's golden
   folder; `docs/macos-port-plan.md` section 4.2). Without it every checkpoint fails as
   "no golden".
2. **Your play folder exists:** `C:\iwd2-play-port` for single runs; `play_parallel.py` makes
   `C:\iwd2-play-port-N` itself. Never use the main session's `C:\iwd2-play*` folders.
3. **The exe is in your worktree** (`--exe <worktree>\build-xxx\RelWithDebInfo\iwd2-re.exe`).
   Never the main tree's.
4. **`--target reimpl` always.** Never `--target original` from your worktree: that re-records
   goldens through the junction.
5. **No build is running in the same worktree for that exe.** Builds in other build
   directories are fine at below-normal priority.

Python: `C:\iwd2-tools\venv\Scripts\python.exe`. Run from the worktree root.

## Modes

| Mode | Command | Takes over mouse/keyboard | Parallel | Works for |
|---|---|---|---|---|
| Real input | `python -m pytest tests\e2e\scenarios\test_sN_x.py --target reimpl --exe <exe> --play-dir C:\iwd2-play-port -q` | Yes | No | Any build. Takes the machine-wide real-input lock and waits for the main session |
| Virtual | add `--virtual [--cpu N]`, or `python tools\play_parallel.py --jobs 2 --play-root C:\iwd2-play-port --exe <exe> [names]` | No | Yes | Win32-platform builds only (the hook DLL replaces Win32 input imports) |
| Driver | add `--driver` (from milestone M3) | No | Yes | SDL builds, on Windows and on the Mac |

**Choosing a mode:**
- Prefer driver or virtual mode: they don't block the main session.
- Use real input only when the build can't run the other modes, or to cross-check one.
- The full suite is about 3.75 h in real input, and roughly half that with `--jobs 2`.
- Run long jobs in the background and wait for the notification. Don't poll.

**Rules from experience:**
- Never run S17 or later on the original in virtual mode. You never run the original anyway.
- A few scenarios are slower in parallel. `play_parallel.py` reruns failures alone at the end;
  keep that on (don't pass `--no-retry`) for gate runs.
- Don't raise an elevation prompt (an admin install) while any real-input run, yours or the
  main session's, is going. It fails that run's input.

## Reading results

Each run leaves `tests/e2e/out/<stamp>-reimpl-<test>/`. It holds the numbered screenshots, a
`failure.png`, and on a failure a stack dump: `iwd2re-stacks.txt` from the game itself, or
`iwd2re-stacks-ext-raw.txt` taken from outside when the game hung. Resolve the raw dump with
`py -I tools\symbolize_raw.py <file> <exe>`.

For every failure:

1. **Look at the failure screenshot and the last good one.** Say in words what differs: which
   screen, which control, which number.
2. **Compare against the default build.** Run the same scenario on the default build of the
   same commit, made in your worktree's `build`, 3 times. Also check the known list below.
   - If the default build fails the same way at a similar rate, it is not a port regression.
     Note it and move on.
   - If only the port build fails, or fails much more often, it is a port regression. Debug it
     before the gate can pass.
3. **Rendering differences** (colours, missing sprites, offsets) on a new video path: open the
   golden and the failure frame side by side, and crop both to the checkpoint region. Small
   uniform differences mean a colour/format conversion; shapes in the wrong place mean
   clipping or pitch.
4. **Timing-looking failures** (a dialogue that never opens, a member who stays behind, a
   click with no effect): rerun 3 times before concluding anything.

## Known intermittents on the default build (2026-11-18)

- **New-game stall,** about 1 in 150 new games: Hedron's opening dialogue never opens (S2 and
  everything that starts a new game). This is a locking problem between the AI and render
  threads, under investigation by the main session. Keep any outside stack dump you get and
  tell the user.
- **A straggler at a doorway:** one or two members stay behind and an exit or door does
  nothing (S14, S16).
- **A walk click into unexplored ground is ignored** ("no goblin came", S9/S13/S14 set-up).
- **An attack click lands on a party member** in a crowd ("the goblin is still alive").
- **"click on Reig did not change the screen"** in the S9 set-up.
- **Weather:** turned off by the harness since 2026-11-18. If a world view is uniformly
  darker with snow specks, the play folder's ini has `Weather=1` again.

## Gate runs

- A gate needs the full suite, with retries on.
- Record per scenario: passed / failed with the reason, and for each failure whether the
  default build shares it.
- Write the numbers in `docs/macos-port-log.md`: date, commit, build, mode, passed/total, and
  failures with their classification.
