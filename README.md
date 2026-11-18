# Icewind Dale II

Reverse-engineered source code for Icewind Dale II (`iwd2.exe`, GOG version 2.01), rebuilt as
C++ that compiles into a working game.

The project was started by Alexander Batalov, who translated about 80% of the functions and
brought the game up to its menu screens. This repository continues that work.

## Status

- **Translation is complete.** All 8,019 functions of the original executable (3.7 MB of
  code) are translated and annotated with their original addresses.
- **The game is playable.** Character creation, the world, combat, dialogue, stores, spells,
  resting, level-up, saving and loading all work.
- **It is tested against the original.** 29 play-test scenarios drive both the original game
  and the rebuild with the same inputs and compare thousands of screenshots. Together they cover
  the game from the main menu to the opening of the Palisade battle in Targos. The latest full
  run passed 28 of 29.
- **Movies** play through `binkw32.dll`, as in the original.
- **Known problems:**
  - **A new game sometimes stalls** after the party is chosen: roughly once in 100 to 150
    new games in testing.
    - What happens: the screen stays on the world view with "Paused for saving game" where
      the Prologue screen should be. The game is still running; the Prologue screen draws
      nothing.
    - Ruled out: a deadlock between threads, lost input, the pause state, and an unfinished
      save.
    - Likely cause: the new game's autosave draws the world for its screenshot on the AI
      thread while the main thread switches to the Prologue screen. The original's code has
      the same race. A milder form leaves "PAUSED / UNPAUSED / Paused for chapter text" lines
      in Hedron's opening dialogue.
    - Workaround: start the new game again.
  - **Party members sometimes stay behind at an exit or door,** and the party does not
    change area. The members at the exit wait for everyone, as in the original; a straggler is
    a member whose own walk ended early.
    - Ruled out: lost clicks and the exit code itself (it matches the original). A bug in the
      path search's start point is fixed and a walk's retries no longer give up on a target
      square nobody can stand on; stragglers still happen, less often.
    - Not known: why a member loses both its path and its path request after bumping into
      another. The original does not show this.
    - Workaround: click the straggler and send it to the exit.
  - Multiplayer is untested.
- **Graphics:** only the windowed 32-bit software renderer is used (`3D Acceleration=0`).
- **A macOS (Apple Silicon) port** is in progress on the `port/macos` branch.

## You need

- **The game:** Icewind Dale II: Complete from [GOG](https://www.gog.com/game/icewind_dale_2).
  No game data is included here.
- **To build:**
  - Windows with Visual Studio 2022 (C++ desktop workload, including its CMake).
  - The default build is 32-bit and uses MFC. The `IWD2RE_NO_MFC` option builds without MFC.

## Building

```
cmake -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config RelWithDebInfo
```

`tools\check.ps1` builds the game and runs the unit tests, including the layout tests that
check class member offsets against the original.

## Playing

The game must run with a copy of the game folder as its working directory. Don't use your
real install: the game writes its ini file and saves there.

```
powershell -ExecutionPolicy Bypass -File tools\make_play_dir.ps1 -Ref "<GOG install folder>" -Play C:\iwd2-play-me
copy build\RelWithDebInfo\iwd2-re.exe C:\iwd2-play-me\
cd C:\iwd2-play-me
iwd2-re.exe
```

`make_play_dir.ps1` sets the copy up for testing:
- windowed;
- cheats on;
- the opening movies marked as already seen (they still play at start-up);
- weather off.

Edit `icewind2.ini` in the copy to change these. Running the script again deletes the copy,
saves included.

## Play tests

The scenarios are in `tests/e2e/scenarios/`; `tools\play.ps1` runs them all. They need the
original game and screenshots ("goldens") recorded from it. The goldens are game content, so
they are not in this repository.

## Documentation

| Document | What it covers |
|---|---|
| `docs/translating.md` | Code conventions and how functions are translated |
| `docs/translation-plan.md` | The plan this work followed |
| `docs/session-log.md` | What was done when, and what is still open |

## License

The source code in this repository is available under the [Sustainable Use License](LICENSE.md).
