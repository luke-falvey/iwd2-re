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
- **Known problems:**
  - Rarely, a new game stalls before Hedron's opening dialogue.
  - Party members sometimes stay behind at a door, so the area change does not happen.
  - Movies are not played.
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
- intro movies off;
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
