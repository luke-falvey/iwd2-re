"""S17: the dock buildings S16 left alone: the locked warehouse (AR1002) and
Brogan who guards it, the house west of Reig (AR1005) and Firtha's house
(AR1004). Starts from a save of S16's end state and leaves a save of its
own end state for S18.

Saves (golden/bank/, git-ignored like all goldens):

  s16-end/  the original's quick save at the end of S16 (docks cleared, the
            party rested on Hedron's ship, whole party selected), and
            world.png, a frame of the world screen taken just before it.
            test_s17_make_s16_save writes it: S16 from the start, then Q.
            It runs only on the original and skips once the bank has a save,
            so delete the folder to bank again. The S16 steps are taken with
            their checkpoints recorded in the output folder, not compared.
  s17-end/  the same for the end of this test (on the original only):
            outside AR1004 on the docks, Brogan's and Firtha's dialogues
            done, AR1005 and AR1004 cleared; AR1002 (5 goblins behind the
            locked door, Brogan_Quest = 1) and AR1007 still to clear.

test_s17_dock_houses (both targets) puts the s16-end save into <play
dir>\\mpsave (after conftest has cleared it), loads it from the main menu
(Load Game, second row: after "Autosave - Prologue") and waits for the
world screen. Every save the game writes is removed again after the game
has exited. Then, with the whole party selected:

  warehouse   off the ship and round the bay onto the warehouse pier, in
              steps through the area map (one long walk click from the deck
              is ignored: the path is too long); Brogan stops the party
              himself ("Halt! Identify yourself!"): compare each of his
              lines, replies 1, Continue ("Your journal has been updated."),
              1, Continue, 1 ("Let me go in"), 1 ("Let me in that
              warehouse": "You convinced Brogan..." and an experience line),
              1 ("Don't you have a key?"), 2 ("Stand back"): the dialogue
              ends
  locked      Sheris alone (portrait 1) clicks the warehouse door: she walks
              to it and "LOCKED" is printed (compare that line, found by
              its shortness: Brogan prints his white overhead lines into the
              message window at random times). With the whole party
              selected the click printed nothing for 9 s, so it is Sheris
  AR1005      select all, walk west to the house west of Reig, click its
              door (it opens), click the doorway: two goblins attack at
              once; fight them as S16 does; compare "The party has gained
              experience: 37" (found by colour) and the furnished part of
              the room (table, bed); leave by the exit at the bottom left
  AR1004      round the half-built ship (S16's route) and south to Firtha's
              house; click its door (it opens; Firtha screams), the doorway:
              three goblins and Firtha's nine cats; fight; compare the
              experience line and the back wall of the room (the cats wander
              over the floor); click Firtha (her cyan circle): compare her
              first line, replies 1, 1, 1 (compare each), 3 ("Farewell");
              leave by the exit at the bottom
  outside     AR1004's door seen from the docks, and the action bar
  save        Q: compare the save lines ("Quick-save successful", "Paused
              for saving game"); on the original the save is banked

Not compared: the world view on the way (snow and the party's positions),
Brogan's, Jorun's and Firtha's overhead lines, the combat lines, hit points,
the loading hints, and the dialogue panel's gold amount (outside the region).

AR1000 map points used (see test_s9_docks for the area-map formula): Brogan
2047,532; the warehouse door 2252-2503,455-601 (To1002 2285,468-2484,585;
a hint trap in front of it prints how to open locked doors once); AR1005's
door 823-868,870-930 (arrival back outside 809,1053); AR1004's door
766-815,2192-2253 (arrival outside 680,2162). AR1004: Firtha 502,192,
Goblin_1-3, Cat_1-9 (all difficulties), exit To1000 168,375-312,508; the
view on entering shows map (x, y) at about screen (x+79, y-80).
"""

from __future__ import annotations

import re
import shutil
import time
from pathlib import Path

import numpy as np
import pytest
from PIL import Image

import test_s16_clear_docks as s16
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s1_boot import MAIN_MENU, boot_to_main_menu
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s8_screens_tour import PORTRAIT
from test_s9_docks import (
    AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, SELECT_ALL, act)
from test_s10_save_load import (
    LINES_PICTURE, MESSAGE_LINES, SCREEN_REGION, SLOT_BUTTON, save_lines)
from test_s14_travel import select_all

E2E = Path(__file__).resolve().parents[1]
BANK = E2E / "golden" / "bank"
S16_END = BANK / "s16-end"
S17_END = BANK / "s17-end"
QUICK_SAVE = re.compile(r"^\d+-Quick-Save$")
KEEP_SAVES = {"default", "000000000-Autosave - Prologue"}
LOAD_ROW = 1  # the load screen: Autosave - Prologue, then the banked save

SHERIS = PORTRAIT[0]
FIGHT_PARK = (520, 515)
MESSAGES = (5, 495, 540, 100)
LINE_SPOT = (8, 576, 535, 14)  # where a found message line is compared
WORLD_REGION = (0, 0, 800, 430)

# Walks: (point the view is centred on, or None to keep the view, point
# walked to), map coordinates.
SHIP_TO_PIER = [
    ((1700, 900), (1650, 960)),  # down the gangplank, west along the dock
    ((1550, 750), (1520, 760)),  # north along the shore
    ((1700, 650), (1760, 610)),  # the west end of the warehouse pier
    ((2000, 640), (2000, 600)),  # east along the pier towards Brogan
    ((2300, 640), (2300, 620)),  # he starts talking on the way
]
WAREHOUSE_VIEW = (2296, 640)
WAREHOUSE_DOOR = (2378, 545)
PIER_TO_AR1005 = [
    ((1700, 650), (1760, 610)),  # back west along the pier
    ((1550, 750), (1500, 800)),
    ((1250, 850), (1200, 860)),  # past Reig
    ((900, 850), (880, 960)),  # in front of the house
]
AR1005_DOOR = (845, 900)
AR1005_TO_AR1004 = s16.TO_SOUTH_WEST + [
    ((700, 1800), (690, 1880)),
    ((700, 2050), (690, 2110)),  # in front of Firtha's house
]
AR1004_DOOR = (790, 2222)
OUTSIDE_AR1005 = (809, 1053)

# AR1005 as the view shows it on entering (S13).
AR1005_EXIT = (200, 350)
AR1005_ROOM = (405, 40, 255, 320)  # table, chairs, bed (the party is to the left)
# AR1004 as the view shows it on entering.
FIRTHA = (581, 112)
AR1004_EXIT = (319, 360)
AR1004_WALL = (220, 0, 390, 58)  # the back wall: crates and windows, no cats
# AR1000 outside AR1004: the house and its open door, right of the party.
AR1004_OUTSIDE = (455, 100, 270, 300)

BROGAN_STEPS = ["1", CONTINUE, "1", CONTINUE, "1", "1", "1"]
FIRTHA_STEPS = ["1", "1", "1"]


# --- saves ---------------------------------------------------------------------


def rmtree_retry(p: Path) -> None:
    for attempt in range(10):
        try:
            if p.is_dir():
                shutil.rmtree(p)
            return
        except OSError:
            if attempt == 9:
                raise
            time.sleep(1.0)


def new_saves(play_dir: Path):
    d = play_dir / "mpsave"
    return [p for p in d.iterdir() if p.is_dir() and p.name not in KEEP_SAVES] if d.is_dir() else []


def bank_quick_save(play_dir: Path, bank: Path) -> bool:
    """Copy the Quick-Save folder of the play install (not the backup) to the
    bank."""
    saves = sorted(p for p in (play_dir / "mpsave").iterdir() if p.is_dir() and QUICK_SAVE.match(p.name))
    if not saves:
        return False
    if (bank / "save").exists():
        shutil.rmtree(bank / "save")
    shutil.copytree(saves[-1], bank / "save" / saves[-1].name)
    return True


def finish_saves(play_dir: Path, out_dir: Path, bank: Path | None) -> None:
    """After the game has exited: keep a copy of every save it wrote in the
    output folder, bank the quick save if the test got that far (it leaves
    <out>/bank-ok), and remove the saves from the play install."""
    for p in new_saves(play_dir):
        shutil.copytree(p, out_dir / "saves" / p.name, dirs_exist_ok=True)
    if bank is not None and (out_dir / "bank-ok").exists():
        if bank_quick_save(play_dir, bank):
            shutil.copy2(out_dir / "bank-world.png", bank / "world.png")
    for p in new_saves(play_dir):
        rmtree_retry(p)


def quick_save(game, out_dir: Path, check=None):
    """Select All (a load waits for the group bar), keep a frame of the world
    screen as <out>/bank-world.png, then Q: the message window shows the save
    lines (compared if `check` is given)."""
    game.click(*SELECT_ALL)
    game.move(*FIGHT_PARK)
    time.sleep(1.5)
    game.grab().save(out_dir / "bank-world.png")
    act(game, lambda: game.press("q"), "quick save", MESSAGES, park=FIGHT_PARK, wait=3.0,
        threshold=0.001, timeout=20.0)
    if check is not None:
        stable = game.wait_stable
        game.wait_stable = lambda **kw: save_lines(stable(**{**kw, "region": MESSAGE_LINES}))
        try:
            check("quick-saved", LINES_PICTURE, max_diff=0.004)
        finally:
            game.wait_stable = stable


def load_bank(game, bank: Path = S16_END):
    """Main menu -> Load Game -> the banked quick save -> the world screen
    with the banked action bar. Returns the banked world frame."""
    boot_to_main_menu(game)
    act(game, lambda: game.click(*MAIN_MENU["load_game"]), "Load Game", SCREEN_REGION,
        threshold=0.3)
    act(game, lambda: game.click(*SLOT_BUTTON[LOAD_ROW]), "Load on the banked save", SCREEN_REGION,
        threshold=0.3, wait=0.0, timeout=20.0)
    bar = Image.open(bank / "world.png")
    deadline = time.time() + 90.0
    while diff_ratio(bar, game.grab(), region=ACTION_BAR_REGION) > 0.3:
        if time.time() > deadline:
            raise AssertionError("the banked save never showed the world screen")
        time.sleep(0.5)
    time.sleep(3.0)
    return bar


@pytest.fixture
def s16_bank(play_dir, out_dir):
    """Set up before `game`, so it tears down after the game has exited."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    yield
    finish_saves(play_dir, out_dir, S16_END)


@pytest.fixture
def from_s16_save(play_dir, out_dir, target):
    """Put the banked S16 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S16_END / "save").iterdir() if p.is_dir()] if (S16_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S16 save in {S16_END}; run test_s17_make_s16_save --target original first")
    for p in new_saves(play_dir):
        rmtree_retry(p)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    yield
    finish_saves(play_dir, out_dir, S17_END if target == "original" else None)


def test_s17_make_s16_save(s16_bank, game, out_dir, target):
    """S16 from the start (its checkpoints recorded in the output folder,
    not compared), then a quick save banked in golden/bank/s16-end/."""
    if target != "original":
        pytest.skip("the S16 save is banked from the original only")
    if (S16_END / "save").is_dir():
        pytest.skip(f"already banked: {S16_END}")
    scratch = out_dir / "s16-golden"
    scratch.mkdir()
    s16.test_s16_clear_docks(game, scratch)
    quick_save(game, out_dir)
    game.screenshot("quick-saved")
    (out_dir / "bank-ok").write_text("ok")


# --- finding things on screen --------------------------------------------------


def short_white_line(img, max_right=95):
    """The lowest message line whose text ends left of `max_right` ("LOCKED";
    Brogan's overhead lines are much longer), moved to LINE_SPOT on a black
    screen with only its text kept, or None."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y0:y0 + h0, x0:x0 + w0]
    text = (a.max(axis=2) > 150) & (np.abs(a[..., 0] - a[..., 1]) < 30)
    rows = [r for r in range(h0) if text[r].any()]
    # Runs of text rows; a line is 9 to 11 rows high.
    runs, start = [], None
    for r in range(h0 + 1):
        on = r in rows
        if on and start is None:
            start = r
        elif not on and start is not None:
            runs.append((start, r))
            start = None
    for top, bottom in reversed(runs):
        cols = np.nonzero(text[top:bottom].any(axis=0))[0]
        if bottom - top >= 7 and cols.max() + x0 < max_right:
            y = y0 + top - 1
            x, w, h = LINE_SPOT[0], LINE_SPOT[2], LINE_SPOT[3]
            line = np.asarray(img.convert("RGB"), dtype=np.int16)[y:y + h, x:x + w]
            keep = (line.max(axis=2) > 150) & (np.abs(line[..., 0] - line[..., 1]) < 30)
            line = np.where(keep[..., None], line, 0).astype(np.uint8)
            out = Image.new("RGB", img.size)
            out.paste(Image.fromarray(line), (x, LINE_SPOT[1]))
            return out
    return None


def dialogue_up(game, bar):
    """The dialogue panel has replaced the action bar."""
    return diff_ratio(bar, game.grab(), region=ACTION_BAR_REGION) > 0.3


# --- the test ------------------------------------------------------------------


def test_s17_dock_houses(from_s16_save, game, golden_dir, out_dir, target):
    group_bar = load_bank(game)
    t0 = time.time()

    # Every checkpoint is taken even if an earlier one differs, so that one
    # run on the rebuild shows all the differences. They are reported together.
    failures = []

    def check(name, region, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    def check_frame(name, frame, region, **kw):
        """A checkpoint of a frame already made (a line moved to LINE_SPOT)."""
        if frame is None:
            failures.append(f"{name}: no such line in the message window")
            return
        stable = game.wait_stable
        game.wait_stable = lambda **_: frame
        try:
            check(name, region, **kw)
        finally:
            game.wait_stable = stable

    def jump(centre):
        spot, real = s16.map_click_for(centre)
        act(game, lambda: game.click(*AREA_MAP_BUTTON), "area map button", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=0.5)
        act(game, lambda: game.click(*spot), f"area map click at {spot}", AREA_MAP_REGION,
            wait=0.5, threshold=0.001, park=FIGHT_PARK)
        act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=1.0)
        return real

    def walk_route(route, here, real=None, wait=True):
        for centre, point in route:
            if centre is not None:
                real = jump(centre)
            game.click(*s16.to_screen(real, point))
            game.move(*FIGHT_PARK)
            if wait:
                # The party walks about 60 px a second.
                time.sleep(2.0 + np.hypot(point[0] - here[0], point[1] - here[1]) / 60)
            here = point
        return here, real

    def dialogue(steps, who, prefix):
        """Replies (keys) and Continue clicks, comparing the panel after each."""
        for i, step in enumerate(steps, start=2):
            if isinstance(step, str):
                act(game, lambda: game.press(step), f"reply {step} to {who}", DIALOG_REGION,
                    park=DIALOG_PARK)
            else:
                act(game, lambda: game.click(*CONTINUE), f"Continue with {who}", DIALOG_REGION,
                    park=DIALOG_PARK)
            check(f"{prefix}-{i:02d}", DIALOG_REGION)

    def enter(door, doorway_what):
        """Click a closed door (the party walks over and opens it), then,
        with everyone selected, the doorway: the next area."""
        game.click(*door)
        game.move(*FIGHT_PARK)
        time.sleep(8.0)
        select_all(game, group_bar)
        act(game, lambda: game.click(*door), doorway_what, WORLD_REGION, park=FIGHT_PARK,
            threshold=0.5, wait=2.0, timeout=25.0)

    def leave(exit_spot, what):
        select_all(game, group_bar)
        act(game, lambda: game.click(*exit_spot), what, WORLD_REGION, park=FIGHT_PARK,
            threshold=0.5, wait=0.0, timeout=25.0)
        deadline = time.time() + 60.0
        while diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) > 0.05:
            if time.time() > deadline:
                raise AssertionError(f"the docks did not appear ({what})")
            time.sleep(0.3)
        time.sleep(3.0)

    # --- Off the ship to the warehouse: Brogan ------------------------------
    here, real = walk_route(SHIP_TO_PIER[:3], s16.ON_DECK)
    # Brogan starts his dialogue when the party comes near him on the pier:
    # stop walking as soon as the panel is up.
    for centre, point in SHIP_TO_PIER[3:]:
        if dialogue_up(game, group_bar):
            break
        real = jump(centre)
        game.click(*s16.to_screen(real, point))
        game.move(*FIGHT_PARK)
        end = time.time() + 2.0 + np.hypot(point[0] - here[0], point[1] - here[1]) / 60
        here = point
        while time.time() < end and not dialogue_up(game, group_bar):
            time.sleep(0.3)
    deadline = time.time() + 30.0
    while not dialogue_up(game, group_bar):
        if time.time() > deadline:
            raise AssertionError("Brogan did not stop the party")
        time.sleep(0.3)
    game.move(*DIALOG_PARK)
    time.sleep(2.0)
    check("brogan-01", DIALOG_REGION)
    dialogue(BROGAN_STEPS, "Brogan", "brogan")
    act(game, lambda: game.press("2"), "last reply to Brogan", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=2.0)

    # --- The warehouse door: locked -----------------------------------------
    real = jump(WAREHOUSE_VIEW)
    act(game, lambda: game.click(*SHERIS), "click on Sheris's portrait", ACTION_BAR_REGION,
        park=FIGHT_PARK)
    game.click(*s16.to_screen(real, WAREHOUSE_DOOR))
    game.move(*FIGHT_PARK)
    deadline = time.time() + 15.0
    while short_white_line(game.grab()) is None and time.time() < deadline:
        time.sleep(0.3)
    time.sleep(0.5)
    check_frame("warehouse-locked-line", short_white_line(game.grab()), LINE_SPOT, max_diff=0.004)

    # --- AR1005 -----------------------------------------------------------------
    select_all(game, group_bar)
    here, real = walk_route(PIER_TO_AR1005, (2000, 560))
    enter(s16.to_screen(real, AR1005_DOOR), "click on the doorway of AR1005")
    s16.fight(game, "AR1005", shots=False)
    check_frame("ar1005-experience", s16.last_yellow_line(game.grab()), LINE_SPOT, max_diff=0.004)
    check("ar1005-room", AR1005_ROOM)
    leave(AR1005_EXIT, "click on the exit of AR1005")

    # --- AR1004: Firtha ---------------------------------------------------------
    here, real = walk_route(AR1005_TO_AR1004, OUTSIDE_AR1005)
    enter(s16.to_screen(real, AR1004_DOOR), "click on the doorway of AR1004")
    s16.fight(game, "AR1004", shots=False)
    check_frame("ar1004-experience", s16.last_yellow_line(game.grab()), LINE_SPOT, max_diff=0.004)
    check("ar1004-wall", AR1004_WALL)
    spot = s16.cyan_near(game.grab(), FIRTHA, dx=18, dy=10) or FIRTHA
    act(game, lambda: game.click(*spot), "click on Firtha", ACTION_BAR_REGION, park=DIALOG_PARK,
        wait=2.0, timeout=20.0)
    check("firtha-01", DIALOG_REGION)
    dialogue(FIRTHA_STEPS, "Firtha", "firtha")
    act(game, lambda: game.press("3"), "last reply to Firtha", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=2.0)
    leave(AR1004_EXIT, "click on the exit of AR1004")
    check("ar1004-outside", AR1004_OUTSIDE)
    check("ar1004-outside-action-bar", ACTION_BAR_REGION)

    # --- Quick save for S18 -----------------------------------------------------
    quick_save(game, out_dir, check)
    if target == "original":
        (out_dir / "bank-ok").write_text("ok")

    print(f"S17 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
