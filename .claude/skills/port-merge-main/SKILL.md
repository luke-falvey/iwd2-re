---
name: port-merge-main
description: Merge the main session's branch (phase-a/stubs-and-layout) into port/macos and resolve conflicts so that both the game fixes and the platform seams survive. Use at the start of every port session, before every milestone gate, and when told a needed fix has landed.
---

# Merge main into the port

The main session keeps fixing game behaviour on `phase-a/stubs-and-layout`. The port session
moves platform calls behind `src/platform/`. Both touch some of the same files, `CChitin.cpp`
most of all. A merge must keep both.

## Steps

1. **In the port worktree, with a clean tree.** Commit or stash first.
2. **Merge, never rebase:**
   `git merge --no-ff phase-a/stubs-and-layout`. The branch is local to this machine; there is
   nothing to fetch. Use the commit message
   `Merge phase-a/stubs-and-layout into port/macos`, plus a short list of what came in
   (`git log --oneline HEAD^1..HEAD^2`) and the attribution line.
3. **For each conflict, apply the rule that fits:**
   - **Their change is a game-behaviour fix inside code you replaced with a platform call**
     (common in `CChitin.cpp`'s input and timer code). Keep the fix's *behaviour*: move it
     into the platform implementation for Win32 and every other implementation. Don't drop it
     because the surrounding code moved.
   - **Their change is game logic next to a call site you changed.** Take theirs, then
     re-apply your one-line call change.
   - **A harness change** (`tests/e2e/harness/`, `tools/play*.py`, `make_play_dir.ps1`). Take
     theirs and re-apply driver mode on top. Keep driver mode in its own functions so this
     stays easy.
   - **Docs.** `docs/session-log.md` is theirs (take theirs); `docs/macos-port-*` is yours.
   - **Unsure.** Stop, take neither, and report the hunk to the orchestrating session.
4. **Build and check** every configured build at below-normal priority:
   - `tools\check.ps1` (default; must pass);
   - `-BuildDir build-nomfc`;
   - `build-sdl` / `build-x64` if configured.
5. **Run the play tests touched by the merged commits** (`port-play-test`, driver or virtual
   mode), or S1–S3 if nothing obvious. Before a gate, run the full suite instead.
6. **Look for platform calls that came in with the merge.** New code from main may call
   Windows directly. Grep for each seamed group's Windows functions outside
   `src/platform/win32/` and move any new ones behind the seam (`port-seam` rule 3).

## Report

- **Commits merged:** count and one-liners.
- **Conflicts:** file, the rule used, and what was done.
- **New platform calls found** and moved.
- **Build and test results.**
