"""S12: a save written by one game loads in the other.

  direction A   the rebuild loads a save the original wrote
  direction B   the original loads a save the rebuild wrote

Two tests, run top to bottom:

test_s12_make_save (either target)
  Plays to S10's saved state (new game, premade party, Hedron's dialogue
  finished, party walked onto the dock, fifth formation, Sheris's darts moved
  from the second quick weapon slot into the first backpack slot), takes S10's
  "before" checkpoints, saves under SAVE_NAME, quits to the main menu and
  opens Load Game to look at the new slot. These are ordinary checkpoints
  (golden/test_s12_make_save/). After the game has closed, the save folder
  and the pictures taken ("before-*" and "load-screen-slot-*") are put in
  the bank for this target:

    golden/test_s12_cross_save/bank/<target>/save/<number>-<name>/
    golden/test_s12_cross_save/bank/<target>/shots/<checkpoint>.png

  (the golden tree is git-ignored). Each run replaces that target's bank.

test_s12_load_other (either target)
  Copies the OTHER target's banked save into <play dir>\\mpsave\\ (skips if
  the bank has none yet), boots to the main menu, Load Game, compares the
  slot, loads it, waits for the world screen and takes the "loaded"
  checkpoints from the world screen. None of them is recorded as a golden,
  on either target: each loaded picture is compared with
    - what the saving game showed before it saved (the other target's bank),
    - the original's pictures of the same state (golden/test_s12_make_save/,
      all variants), and
    - on the rebuild, what the rebuild itself showed before saving (its own
      bank), which tells a load problem from a drawing difference.
  Any mismatch fails the test, on the original too: on the original this is
  direction B, a real comparison, not a recording.

  Only what does not differ between two separate play-throughs is compared:
  the action bar (formation), portraits, Sheris's inventory slots, weight,
  armour class and hit points, the record screen and its numbers, the
  journal, and the load screen's slot name and "Prologue, N hours". Not the
  world view, the area map (party positions), gold, weather or thumbnails.

The save name has no letter that is a shortcut key (Keymap.ini; the original
drops a typed "s" now and then), so it is not "e2ecross".

Clean-up: make_save removes its save folder before the game starts and after
it has exited (a copy goes to the output folder and the bank).
load_other copies <play dir>\\mpsave as it was before the run into the output
folder, and after the game has exited removes everything in mpsave that was
not there before and puts back any file the game changed.
"""

from __future__ import annotations

import hashlib
import re
import shutil
import time
from pathlib import Path

import pytest
from PIL import Image

from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s1_boot import MAIN_MENU, boot_to_main_menu
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
from test_s10_save_load import (
    AREA_MAP_REGION,
    FORMATION_5,
    FULL_SCREEN,
    INV_BACKPACK_1,
    INV_DARTS,
    MSG_PARK,
    NAME_BOX,
    NAME_DIALOG,
    NAME_DIALOG_SAVE,
    OPTIONS_QUIT,
    OPTIONS_SAVE,
    PARK_SPOT,
    PORTRAITS_REGION,
    QUIT_PROMPT_QUIT,
    SCREEN_REGION,
    SLOT_BUTTON,
    SLOT_MASK,
    SLOT_PORTRAITS,
    SLOT_TEXT,
    WORLD_STATIC,
    LINES_PICTURE,
    MESSAGE_LINES,
    save_lines,
)

E2E = Path(__file__).resolve().parents[1]
BANK = E2E / "golden" / "test_s12_cross_save" / "bank"
MAKE_GOLDEN = E2E / "golden" / "test_s12_make_save"
TARGETS = ("original", "reimpl")

# Letters that are no shortcut key, and a digit (as S10's "e2ekeep").
SAVE_NAME = "e2epeek"
OUR_SAVE = re.compile(r"^\d+-%s$" % SAVE_NAME)

# Save screen: Autosave - Prologue, then the first empty slot. Load screen:
# Autosave - Prologue, then this save.
SAVE_ROW = 1
LOAD_ROW = 1

INV_WEIGHT = INV_DETAILS["weight"]
INV_QUICK_WEAPON = INV_DETAILS["selected-quick-weapon"]
TIGHT = 0.004

# What a save made in one play-through must show again when loaded in
# another: checkpoint name (without the before-/loaded- prefix), region,
# limit.
CROSS = [
    ("action-bar", ACTION_BAR_REGION, 0.02),
    ("portraits", PORTRAITS_REGION, TIGHT),
    ("inventory-quick-weapons", INV_QUICK_WEAPONS, 0.02),
    ("inventory-backpack", INV_BACKPACK, 0.02),
    ("inventory-ac-hp", INV_AC_HP, 0.02),
    ("inventory-weight", INV_WEIGHT, TIGHT),
    ("inventory-selected-quick-weapon", INV_QUICK_WEAPON, TIGHT),
    ("record", SCREEN_REGION, 0.02),
    *[(f"record-{n}", r, TIGHT) for n, r in RECORD_DETAILS.items()],
    ("journal", SCREEN_REGION, 0.02),
]
SLOT_CHECKS = [
    (f"load-screen-slot-{LOAD_ROW}", SLOT_TEXT[LOAD_ROW], TIGHT),
    (f"load-screen-slot-{LOAD_ROW}-portraits", SLOT_PORTRAITS[LOAD_ROW], TIGHT),
]


def other(target: str) -> str:
    return TARGETS[1 - TARGETS.index(target)]


def our_saves(play_dir: Path):
    for sub in ("mpsave", "save"):
        d = play_dir / sub
        if d.is_dir():
            for p in d.iterdir():
                if p.is_dir() and OUR_SAVE.match(p.name):
                    yield p


def rmtree_retry(p: Path) -> None:
    for attempt in range(10):
        try:
            if p.is_dir():
                shutil.rmtree(p)
            elif p.exists():
                p.unlink()
            return
        except OSError:
            if attempt == 9:
                raise
            time.sleep(1.0)


def banked_save(target: str) -> Path | None:
    d = BANK / target / "save"
    if not d.is_dir():
        return None
    found = [p for p in d.iterdir() if p.is_dir() and OUR_SAVE.match(p.name)]
    return found[0] if len(found) == 1 else None


class Driver:
    """S10's input and checkpoint helpers, shared by both tests."""

    def __init__(self, game, golden_dir: Path):
        self.game = game
        self.golden_dir = golden_dir
        self.failures: list[str] = []
        self.seen: dict[str, Image.Image] = {}
        # Checkpoint function used by state(); load_other replaces it.
        self.check = self.golden_check

    def golden_check(self, name, region, **kw):
        """Ordinary checkpoint; a mismatch is collected, not raised."""
        try:
            self.seen[name] = checkpoint(self.game, self.golden_dir, name, region=region, **kw)
        except AssertionError as e:
            self.failures.append(str(e).splitlines()[0])
            self.seen[name] = Image.open(self.game.out_dir / f"{self.game.shots:03d}-{name}.png")

    def check_save_lines(self, name):
        stable = self.game.wait_stable
        self.game.wait_stable = lambda **kw: save_lines(stable(**{**kw, "region": MESSAGE_LINES}))
        try:
            self.golden_check(name, LINES_PICTURE, max_diff=TIGHT)
        finally:
            self.game.wait_stable = stable

    def act(self, do, what, region, wait=2.0, park=PARK_SPOT, threshold=0.005, timeout=10.0):
        """Do one input, then wait until `region` has changed (or fail: the
        input was dropped) and give it `wait` more seconds."""
        game = self.game
        before = game.grab()
        do()
        game.move(*park)
        deadline = time.time() + timeout
        while diff_ratio(before, game.grab(), region=region) <= threshold:
            if time.time() > deadline:
                raise AssertionError(f"{what} did not change the screen")
            time.sleep(0.2)
        time.sleep(wait)

    def press(self, key, region, **kw):
        self.act(lambda: self.game.press(key), f"key {key!r}", region, **kw)

    def click(self, xy, region, **kw):
        self.act(lambda: self.game.click(*xy), f"click at {xy}", region, **kw)

    def state(self, prefix, change=False, cross_only=False):
        """S10's checkpoints of what the save must keep, from the world
        screen, opening the screens with the dial buttons. With `change`,
        move the darts into the backpack first (the change the save keeps).
        With `cross_only`, skip the world view and the area map."""
        check = self.check
        check(prefix + "-action-bar", ACTION_BAR_REGION)
        check(prefix + "-portraits", PORTRAITS_REGION, max_diff=TIGHT)
        if not cross_only:
            check(prefix + "-world", WORLD_STATIC, max_diff=0.01)

        self.click(DIAL["inventory"], ACTION_BAR_REGION, wait=3.0)
        if change:
            self.click(INV_DARTS, (INV_DARTS[0] - 20, INV_DARTS[1] - 20, 40, 40), wait=1.0)
            self.click(INV_BACKPACK_1, (INV_BACKPACK_1[0] - 20, INV_BACKPACK_1[1] - 20, 40, 40))
        check(prefix + "-inventory-quick-weapons", INV_QUICK_WEAPONS)
        check(prefix + "-inventory-backpack", INV_BACKPACK)
        check(prefix + "-inventory-ac-hp", INV_AC_HP)
        check(prefix + "-inventory-weight", INV_WEIGHT, settle=0.5, max_diff=TIGHT)
        check(prefix + "-inventory-selected-quick-weapon", INV_QUICK_WEAPON, settle=0.5,
              max_diff=TIGHT)
        self.press("esc", ACTION_BAR_REGION)

        self.click(DIAL["record"], ACTION_BAR_REGION)
        check(prefix + "-record", SCREEN_REGION)
        for name, region in RECORD_DETAILS.items():
            check(f"{prefix}-record-{name}", region, settle=0.5, max_diff=TIGHT)
        self.press("esc", ACTION_BAR_REGION)

        self.click(DIAL["journal"], ACTION_BAR_REGION)
        check(prefix + "-journal", SCREEN_REGION)
        self.press("esc", ACTION_BAR_REGION)

        if not cross_only:
            self.click(DIAL["area_map"], ACTION_BAR_REGION)
            check(prefix + "-area-map", AREA_MAP_REGION)
            self.press("esc", ACTION_BAR_REGION)


# --- make_save ---------------------------------------------------------------


@pytest.fixture
def bank_own_save(play_dir, out_dir, target):
    """Remove this test's save folder before the game starts; after the game
    has exited (set up before `game`, so torn down after it) copy it to the
    output folder, and, with the pictures the test left in <out>/bank-shots,
    make it this target's bank. Then remove it from the play install."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    for p in list(our_saves(play_dir)):
        rmtree_retry(p)
    try:
        yield
    finally:
        saves = list(our_saves(play_dir))
        for p in saves:
            shutil.copytree(p, out_dir / "saves" / p.parent.name / p.name, dirs_exist_ok=True)
        shots = out_dir / "bank-shots"
        if len(saves) == 1 and shots.is_dir():
            bank = BANK / target
            if bank.exists():
                shutil.rmtree(bank)
            shutil.copytree(saves[0], bank / "save" / saves[0].name)
            shutil.copytree(shots, bank / "shots")
        for p in saves:
            rmtree_retry(p)


def test_s12_make_save(bank_own_save, game, golden_dir, out_dir):
    d = Driver(game, golden_dir)
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)
    # S6: select the party on the deck and walk it onto the dock.
    game.drag(*BOX_FROM, *BOX_TO)
    time.sleep(1.0)
    game.click(*DOCK_SPOT)
    time.sleep(0.5)
    game.move(*PARK_SPOT)
    time.sleep(19.5)

    # --- S10's changes, and how they look ------------------------------------
    d.click(FORMATION_5, ACTION_BAR_REGION, wait=1.0)
    d.state("before", change=True)

    # --- Save under a name --------------------------------------------------
    d.press("o", ACTION_BAR_REGION)
    d.click(OPTIONS_SAVE, SCREEN_REGION)
    d.check("save-screen", FULL_SCREEN, mask=SLOT_MASK)
    for i in range(SAVE_ROW + 1):
        d.check(f"save-screen-slot-{i}", SLOT_TEXT[i], settle=0.5, max_diff=TIGHT)
    d.click(SLOT_BUTTON[SAVE_ROW], SCREEN_REGION)
    for ch in SAVE_NAME:
        d.act(lambda: game.press(ch), f"typing {ch!r}", NAME_BOX, wait=0.3)
    d.check("save-name-dialog", NAME_DIALOG)
    d.act(lambda: game.click(*NAME_DIALOG_SAVE), "Save in the name dialogue", ACTION_BAR_REGION,
          park=MSG_PARK, wait=3.0, timeout=30.0)
    d.check_save_lines("saved-message")
    game.move(*PARK_SPOT)

    # --- Quit to the main menu and look at the new slot ---------------------
    d.press("o", ACTION_BAR_REGION)
    d.click(OPTIONS_QUIT, SCREEN_REGION)
    d.click(QUIT_PROMPT_QUIT, SCREEN_REGION, wait=3.0, threshold=0.3, timeout=30.0)
    d.click(MAIN_MENU["load_game"], SCREEN_REGION, threshold=0.3)
    d.check("load-screen", FULL_SCREEN, mask=SLOT_MASK)
    for name, region, limit in SLOT_CHECKS:
        d.check(name, region, settle=0.5, max_diff=limit)

    # The pictures the other target's load is compared with.
    shots = out_dir / "bank-shots"
    shots.mkdir(exist_ok=True)
    for name, _, _ in CROSS:
        d.seen["before-" + name].save(shots / f"before-{name}.png")
    for name, _, _ in SLOT_CHECKS:
        d.seen[name].save(shots / f"{name}.png")

    assert not d.failures, "; ".join(d.failures)


# --- load_other --------------------------------------------------------------


def _digest(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def _snapshot(root: Path) -> dict[str, str]:
    return {str(f.relative_to(root)): _digest(f) for f in root.rglob("*") if f.is_file()}


@pytest.fixture
def foreign_save(play_dir, out_dir, target):
    """Copy the other target's banked save into <play dir>\\mpsave before the
    game starts; after it has exited put mpsave back as it was."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    source = other(target)
    banked = banked_save(source)
    if banked is None:
        pytest.skip(f"no {source} save in the bank {BANK / source / 'save'}; "
                    f"run test_s12_make_save with --target {source} first")
    mpsave = play_dir / "mpsave"
    save_dir = play_dir / "save"
    had_save_dir = save_dir.exists()
    for p in list(our_saves(play_dir)):  # left over from an interrupted run
        rmtree_retry(p)
    before_entries = {p.name for p in mpsave.iterdir()}
    before_files = _snapshot(mpsave)
    shutil.copytree(mpsave, out_dir / "mpsave-before")
    shutil.copytree(banked, mpsave / banked.name)
    try:
        yield source
    finally:
        changed = []
        for p in list(mpsave.iterdir()):
            if p.name not in before_entries:
                if p.is_dir():
                    shutil.copytree(p, out_dir / "mpsave-after" / p.name, dirs_exist_ok=True)
                rmtree_retry(p)
        for rel, digest in before_files.items():
            f = mpsave / rel
            if not f.exists() or _digest(f) != digest:
                changed.append(rel)
                f.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(out_dir / "mpsave-before" / rel, f)
        for f in list(mpsave.rglob("*")):
            rel = str(f.relative_to(mpsave))
            if f.is_file() and rel not in before_files:
                changed.append(rel + " (new, removed)")
                f.unlink()
        if not had_save_dir and save_dir.exists():
            rmtree_retry(save_dir)
        if changed:
            (out_dir / "mpsave-restored.txt").write_text("\n".join(changed) + "\n")


def test_s12_load_other(foreign_save, game, out_dir, target):
    source = foreign_save
    d = Driver(game, MAKE_GOLDEN)
    refs = {
        f"what the {source} showed before saving": BANK / source / "shots",
        "the original's pictures of this state": MAKE_GOLDEN,
    }
    if target != "original":
        refs[f"what the {target} itself showed before saving"] = BANK / target / "shots"
    limits = {n: lim for n, _, lim in CROSS} | {n: lim for n, _, lim in SLOT_CHECKS}

    def cross_check(name, region, **kw):
        """Take a picture and compare it with each reference; never records."""
        key = re.sub(r"^loaded-", "", name)
        img = game.wait_stable(settle=kw.get("settle", 1.0), region=region, timeout=30.0)
        game.shots += 1
        img.save(out_dir / f"{game.shots:03d}-{name}.png")
        d.seen[name] = img
        ref_name = ("before-" + key) if name.startswith("loaded-") else key
        limit = limits[key]
        for what, folder in refs.items():
            paths = sorted(folder.glob(f"{ref_name}.png")) + sorted(folder.glob(f"{ref_name}.alt*.png"))
            if not paths:
                d.failures.append(f"{name}: no reference {ref_name}.png in {folder}")
                continue
            best = min(diff_ratio(Image.open(p), img, region=region) for p in paths)
            if best > limit:
                d.failures.append(f"{name}: {best:.1%} of pixels differ from {what} "
                                  f"(limit {limit:.1%})")

    boot_to_main_menu(game)
    d.click(MAIN_MENU["load_game"], SCREEN_REGION, threshold=0.3)
    for name, region, _ in SLOT_CHECKS:
        cross_check(name, region, settle=0.5)
    d.click(SLOT_BUTTON[LOAD_ROW], SCREEN_REGION, threshold=0.3)
    # Loading screen, then the world screen: wait until the action bar looks
    # roughly like the saving game's (the loading screen differs in most of
    # that region; the real comparison follows).
    bar = Image.open(BANK / source / "shots" / "before-action-bar.png")
    deadline = time.time() + 90.0
    while diff_ratio(bar, game.grab(), region=ACTION_BAR_REGION) > 0.3:
        if time.time() > deadline:
            raise AssertionError(f"the {source}'s save never showed the world screen")
        time.sleep(0.5)
    time.sleep(3.0)

    d.check = cross_check
    d.state("loaded", cross_only=True)
    assert not d.failures, "; ".join(d.failures)
