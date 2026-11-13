"""S10: save the game, quit to the main menu, load it again.

Continues S6 (whole party selected and walked off the ship onto the dock in
AR1000, game running). Then:

  change      pick the fifth party formation in the action bar, and in
              Sheris's inventory move her darts from the second quick weapon
              slot into the first backpack slot
  before      compare the action bar, the portraits and a static part of the
              world view, then Sheris's inventory, record and the journal
              and the area map (dial buttons; Esc back to the world)
  quick save  Q on the world screen; compare the two message lines
              ("Quick-save successful", "Paused for saving game")
  save        Options (O) -> Save Game: compare the slot list, click Save on
              the first empty slot, type the name (each key must show in the
              name box), compare the dialogue, Save; compare the message lines
  quit        Options (O) -> Quit Game -> Quit (don't save) -> main menu
  load        Load Game: compare the slot list (the new save is listed by
              name), Load on the new save, wait for the world screen
  after       the same checkpoints as "before", under loaded-* names, each
              also compared directly with its "before" picture of this run

Not compared: the save thumbnails (a picture of the moving world view) and
the date and time each slot was written (the system clock), the loading
screen (a hint that changes on every load), the rest of the message window
(the hints shown while saving), the paper doll and the gold amount.

The original writes every save, single player included, to
<play dir>\\mpsave\\<number>-<name>\\ ("default" and "000000000-Autosave -
Prologue" there are rewritten by every new game). The save_folders fixture
removes the folders this test makes (the typed name, Quick-Save and
Quick-Save-Backup; no other scenario quick-saves) before the game starts and
after it has been closed, and keeps a copy of them in the output folder
under saves\\. It looks in save\\ too, in case a target writes there.
"""

from __future__ import annotations

import re
import shutil
import time
from pathlib import Path

import pytest
import numpy as np
from PIL import Image

from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s2_new_game import new_game_to_opening_dialog
from test_s5_leave_dialog_inventory import (
    ACTION_BAR_REGION,
    DETAILS as INV_DETAILS,
    INV_AC_HP,
    INV_BACKPACK,
    INV_QUICK_WEAPONS,
    finish_hedron_dialog,
)
from test_s6_walk_off_ship import BOX_FROM, BOX_TO, DOCK_SPOT
from test_s8_screens_tour import DIAL, RECORD_DETAILS

# Typed into the name box one key at a time. Only letters that are not
# shortcut keys (Keymap.ini): the original drops an "s" (Spellbook) typed
# into the box now and then (3 times in 6 tries). No punctuation either:
# type_text has none.
SAVE_NAME = "e2ekeep"
# Save folders this test makes: "<9 digits>-<name>".
TEST_SAVE = re.compile(r"^\d+-(%s|Quick-Save|Quick-Save-Backup)$" % SAVE_NAME)

PARK_SPOT = (300, 560)  # the message window
MSG_PARK = (430, 465)  # empty end of the action bar, off the message lines

FORMATION_5 = (195, 463)
INV_DARTS = (618, 105)  # second quick weapon slot
INV_BACKPACK_1 = (270, 318)

OPTIONS_SAVE = (612, 111)
OPTIONS_QUIT = (612, 141)
QUIT_PROMPT_QUIT = (398, 260)
MAIN_MENU_LOAD = (645, 294)
# Save and Load buttons of the slot rows (102 pixels apart).
SLOT_BUTTON = [(684, 101 + 102 * i) for i in range(5)]
NAME_DIALOG_SAVE = (464, 321)
# On the save screen: Autosave - Prologue, Quick-Save, then the first empty
# slot. On the load screen the new save is listed after them.
SAVE_ROW = 2
LOAD_ROW = 2

SCREEN_REGION = (0, 0, 800, 430)
PORTRAITS_REGION = (500, 435, 300, 55)
MESSAGE_REGION = (8, 500, 535, 90)
# The two message lines a save prints (green "...save successful", red
# "Paused for saving game") are not at a fixed height: the hint shown while
# saving above them has two to four lines. They are cut out from the green
# line down and compared at the top of a blank picture.
MESSAGE_LINES = (8, 500, 535, 92)
LINES_PICTURE = (0, 0, 535, 26)
# Hull and water in the lower right of the view left by the walk; nobody
# walks there. Further right the dithered edge of the fog of war differs a
# little from run to run.
WORLD_STATIC = (430, 280, 240, 150)
AREA_MAP_REGION = (97, 37, 483, 360)
INV_WEIGHT = INV_DETAILS["weight"]
INV_QUICK_WEAPON = INV_DETAILS["selected-quick-weapon"]

# Slot list (save and load screens).
FULL_SCREEN = (0, 0, 800, 600)
SLOT_TEXT = [(150, 90 + 102 * i, 365, 47) for i in range(5)]  # name, chapter and hours
SLOT_PORTRAITS = [(533, 85 + 102 * i, 90, 75) for i in range(5)]
SLOT_MASK = ([(24, 78 + 102 * i, 114, 88) for i in range(5)]  # thumbnails
             + [(150, 138 + 102 * i, 365, 22) for i in range(5)])  # date and time
NAME_DIALOG = (240, 195, 320, 160)
NAME_BOX = (262, 250, 250, 16)


def save_lines(img):
    """A blank 800x600 picture with the save message lines at its top
    (or the bottom of the message window, if there is no green line)."""
    x, y, w, h = MESSAGE_LINES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y:y + h, x:x + w]
    green = (a[..., 1] > 150) & (a[..., 0] < 120) & (a[..., 2] < 120)
    rows = np.nonzero(green.any(axis=1))[0]
    top = y + int(rows.min()) - 2 if len(rows) else y + h - LINES_PICTURE[3]
    out = Image.new("RGB", img.size)
    out.paste(img.crop((x, top, x + w, top + LINES_PICTURE[3])), (0, 0))
    return out


def created_saves(play_dir: Path):
    for sub in ("mpsave", "save"):
        d = play_dir / sub
        if d.is_dir():
            for p in d.iterdir():
                if p.is_dir() and TEST_SAVE.match(p.name):
                    yield p


@pytest.fixture
def save_folders(play_dir, out_dir):
    """Remove this test's save folders before the game starts and after it is
    closed (this fixture is set up before `game` and torn down after it),
    keeping a copy of what the run wrote in <out>/saves/."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    for p in list(created_saves(play_dir)):
        shutil.rmtree(p)
    try:
        yield
    finally:
        for p in list(created_saves(play_dir)):
            shutil.copytree(p, out_dir / "saves" / p.parent.name / p.name, dirs_exist_ok=True)
            for attempt in range(10):
                try:
                    shutil.rmtree(p)
                    break
                except OSError:
                    if attempt == 9:
                        raise
                    time.sleep(1.0)


def test_s10_save_load(save_folders, game, golden_dir):
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)
    # S6: select the party on the deck and walk it onto the dock.
    game.drag(*BOX_FROM, *BOX_TO)
    time.sleep(1.0)
    game.click(*DOCK_SPOT)
    time.sleep(0.5)
    game.move(*PARK_SPOT)
    time.sleep(19.5)

    # Every checkpoint is taken even if an earlier one differs, so that one
    # run on the rebuild shows all the differences. They are reported together.
    failures = []
    seen = {}

    def check(name, region, **kw):
        try:
            seen[name] = checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])
            seen[name] = Image.open(game.out_dir / f"{game.shots:03d}-{name}.png")

    def check_save_lines(name):
        """Checkpoint of the save message lines wherever they are."""
        stable = game.wait_stable
        game.wait_stable = lambda **kw: save_lines(stable(**{**kw, "region": MESSAGE_LINES}))
        try:
            check(name, LINES_PICTURE, max_diff=0.004)
        finally:
            game.wait_stable = stable

    def same(before, after, region, limit):
        """The picture after loading must match the one before saving."""
        d = diff_ratio(seen[before], seen[after], region=region)
        if d > limit:
            failures.append(f"{after}: {d:.1%} of pixels differ from {before} of this run "
                            f"(limit {limit:.1%})")

    def act(do, what, region, wait=2.0, park=PARK_SPOT, threshold=0.005, timeout=10.0):
        """Do one input, then wait until `region` has changed (or fail: the
        input was dropped) and give it `wait` more seconds."""
        before = game.grab()
        do()
        game.move(*park)
        deadline = time.time() + timeout
        while diff_ratio(before, game.grab(), region=region) <= threshold:
            if time.time() > deadline:
                raise AssertionError(f"{what} did not change the screen")
            time.sleep(0.2)
        time.sleep(wait)

    def press(key, region, **kw):
        act(lambda: game.press(key), f"key {key!r}", region, **kw)

    def click(xy, region, **kw):
        act(lambda: game.click(*xy), f"click at {xy}", region, **kw)

    def state(prefix):
        """Checkpoints of what the save must keep, from the world screen.
        The screens are opened with the dial buttons: the original once
        dropped the R key here."""
        check(prefix + "-action-bar", ACTION_BAR_REGION)
        check(prefix + "-portraits", PORTRAITS_REGION, max_diff=0.004)
        check(prefix + "-world", WORLD_STATIC, max_diff=0.01)

        click(DIAL["inventory"], ACTION_BAR_REGION, wait=3.0)
        if prefix == "before":
            # The change the save has to keep: darts into the backpack.
            # (Watched in the two slots alone: the small icon is too few
            # pixels of the larger regions.)
            click(INV_DARTS, (INV_DARTS[0] - 20, INV_DARTS[1] - 20, 40, 40), wait=1.0)
            click(INV_BACKPACK_1, (INV_BACKPACK_1[0] - 20, INV_BACKPACK_1[1] - 20, 40, 40))
        check(prefix + "-inventory-quick-weapons", INV_QUICK_WEAPONS)
        check(prefix + "-inventory-backpack", INV_BACKPACK)
        check(prefix + "-inventory-ac-hp", INV_AC_HP)
        check(prefix + "-inventory-weight", INV_WEIGHT, settle=0.5, max_diff=0.004)
        check(prefix + "-inventory-selected-quick-weapon", INV_QUICK_WEAPON, settle=0.5,
              max_diff=0.004)
        press("esc", ACTION_BAR_REGION)

        click(DIAL["record"], ACTION_BAR_REGION)
        check(prefix + "-record", SCREEN_REGION)
        for name, region in RECORD_DETAILS.items():
            check(f"{prefix}-record-{name}", region, settle=0.5, max_diff=0.004)
        press("esc", ACTION_BAR_REGION)

        click(DIAL["journal"], ACTION_BAR_REGION)
        check(prefix + "-journal", SCREEN_REGION)
        press("esc", ACTION_BAR_REGION)

        click(DIAL["area_map"], ACTION_BAR_REGION)
        check(prefix + "-area-map", AREA_MAP_REGION)
        press("esc", ACTION_BAR_REGION)

    # --- Change something and look at it ------------------------------------
    click(FORMATION_5, ACTION_BAR_REGION, wait=1.0)
    state("before")

    # --- Quick save ---------------------------------------------------------
    game.move(*MSG_PARK)
    act(lambda: game.press("q"), "quick save", MESSAGE_REGION, park=MSG_PARK, wait=3.0,
        timeout=20.0)
    check_save_lines("quick-saved-message")
    game.move(*PARK_SPOT)

    # --- Save under a name --------------------------------------------------
    press("o", ACTION_BAR_REGION)
    click(OPTIONS_SAVE, SCREEN_REGION)
    check("save-screen", FULL_SCREEN, mask=SLOT_MASK)
    for i in range(SAVE_ROW + 1):
        check(f"save-screen-slot-{i}", SLOT_TEXT[i], settle=0.5, max_diff=0.004)
    click(SLOT_BUTTON[SAVE_ROW], SCREEN_REGION)
    for ch in SAVE_NAME:
        act(lambda: game.press(ch), f"typing {ch!r}", NAME_BOX, wait=0.3)
    check("save-name-dialog", NAME_DIALOG)
    act(lambda: game.click(*NAME_DIALOG_SAVE), "Save in the name dialogue", ACTION_BAR_REGION,
        park=MSG_PARK, wait=3.0, timeout=30.0)
    check_save_lines("saved-message")
    game.move(*PARK_SPOT)

    # --- Quit to the main menu and load -------------------------------------
    press("o", ACTION_BAR_REGION)
    click(OPTIONS_QUIT, SCREEN_REGION)
    # Quit without saving: the main menu (day or night art, not compared).
    click(QUIT_PROMPT_QUIT, SCREEN_REGION, wait=3.0, threshold=0.3, timeout=30.0)
    click(MAIN_MENU_LOAD, SCREEN_REGION, threshold=0.3)
    check("load-screen", FULL_SCREEN, mask=SLOT_MASK)
    for i in range(LOAD_ROW + 1):
        check(f"load-screen-slot-{i}", SLOT_TEXT[i], settle=0.5, max_diff=0.004)
        check(f"load-screen-slot-{i}-portraits", SLOT_PORTRAITS[i], settle=0.5, max_diff=0.004)
    click(SLOT_BUTTON[LOAD_ROW], SCREEN_REGION, threshold=0.3)
    # Loading screen, then the world screen: wait until the action bar looks
    # roughly like it did before saving (the loading screen differs in most
    # of that region; the real comparison follows).
    deadline = time.time() + 90.0
    while diff_ratio(seen["before-action-bar"], game.grab(), region=ACTION_BAR_REGION) > 0.3:
        if time.time() > deadline:
            raise AssertionError("the loaded game never showed the world screen")
        time.sleep(0.5)
    time.sleep(3.0)

    # --- The loaded game must look the same ---------------------------------
    state("loaded")
    tight = 0.004
    for name, region, limit in [
        ("action-bar", ACTION_BAR_REGION, 0.02),
        ("portraits", PORTRAITS_REGION, tight),
        ("world", WORLD_STATIC, 0.01),
        ("inventory-quick-weapons", INV_QUICK_WEAPONS, 0.02),
        ("inventory-backpack", INV_BACKPACK, 0.02),
        ("inventory-ac-hp", INV_AC_HP, 0.02),
        ("inventory-weight", INV_WEIGHT, tight),
        ("inventory-selected-quick-weapon", INV_QUICK_WEAPON, tight),
        ("record", SCREEN_REGION, 0.02),
        *[(f"record-{n}", r, tight) for n, r in RECORD_DETAILS.items()],
        ("journal", SCREEN_REGION, 0.02),
        ("area-map", AREA_MAP_REGION, 0.02),
    ]:
        same("before-" + name, "loaded-" + name, region, limit)

    assert not failures, "; ".join(failures)
