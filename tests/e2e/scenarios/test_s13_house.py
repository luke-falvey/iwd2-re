"""S13: the fight inside the house on the Targos docks (AR1005), the goblin's
ground pile, the locked footlocker, a healing spell, and back out to the docks.

Continues S9 (the party has just walked through the door into AR1005, where
two goblins attack at once). S9's steps run first as the set-up; its
checkpoints are only waited for here, not compared again. Then:

  pause       Space: the game stops ("PAUSED" on the last message line, the
              world view freezes); compare that line and the action bar
  fight       while paused: select all, the Attack button (compare the bar
              with it pressed), click a goblin (found by its red selection
              circle); Space again to let it run. Repeat until no goblin is
              left; compare the action bar and the last three lines ("The
              party has gained experience: 37", PAUSED, UNPAUSED: the loop
              pauses once more to see that no goblin is left)
  pile        click where a goblin died: the ground-item panel opens; compare
              only its picture (the drop is random treasure); Esc
  footlocker  Sheris (portrait 1) clicks it: "LOCKED". Kylie (portrait 6):
              Thieving, click the footlocker (difficulty 10). Sheris clicks it
              again and it opens (else Kylie tries again): compare the
              container panel (dagger, gold, gem; the gold count is random
              and masked) and Sheris's weight; take the
              dagger, compare again; Esc; compare the last line ("Kylie- Lock
              Pick Succeeded")
  heal        Laeris: Cast Spell -> Cleric (Cure Light Wounds, Summon Monster
              I), Cure Light Wounds on Sheris's portrait; the cleric list then
              holds only Summon Monster I
  leave       select all, click the exit at the bottom left of the room: the
              loading screen, then AR1000 by the house door; compare the boats
              and the snow below the party, and the action bar

Not compared: hit points, the healed amount and the combat messages (dice
rolls differ run to run on the original), the gold amounts, where the
characters stand, and the loading screen (its hint is random). On the
original the party does not fight back on its own: without the orders the
goblins keep hitting it.

Area AR1005 is drawn with map point (x, y) at about screen (x+84, y-10):
footlocker 459-489,233-264 (locked, difficulty 10: 00DAGG01, MISC07 gold
(11 and 15 seen), 00GEM01); exit to AR1000 52-175,303-490; goblins start at 285,255 and
425,240. A goblin carries 1 gold (MISC07) and a random-treasure item
(00RTGOB1).
"""

from __future__ import annotations

import time

import numpy as np

from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s8_screens_tour import PORTRAIT
from test_s9_docks import SELECT_ALL, act, docks_to_house

SHERIS, LAERIS, KYLIE = PORTRAIT[0], PORTRAIT[3], PORTRAIT[5]

FIGHT_PARK = (700, 380)  # empty black space right of the room
PANEL_PARK = (600, 330)  # the same, while a panel is open

# Action bar.
ATTACK = (63, 463)  # group bar: Attack
THIEVING = (268, 463)  # Kylie's bar
CAST_SPELL = (174, 463)  # Laeris's bar
CAST_CLERIC = (26, 463)  # class choice, then the first spell of the list

# AR1005 as the view shows it after entering.
FOOTLOCKER = (558, 238)
HOUSE_EXIT = (200, 350)
CARPET = (450, 215)  # middle of the room, away from where the goblins die

WORLD_REGION = (0, 0, 800, 430)
LAST_LINE_REGION = (8, 576, 535, 16)
# The last three message lines: after the fight they are "The party has
# gained experience: 37", then PAUSED and UNPAUSED from the last check that
# no goblin is left.
END_LINES_REGION = (8, 550, 535, 42)
# The gold count in the footlocker is random: mask it (second slot, then
# first slot once the dagger is taken).
GOLD_COUNT = (205, 522, 30, 12)
GOLD_COUNT_TAKEN = (160, 522, 30, 12)
# Ground-item / container panel without the gold amount; the backpack weight.
PILE_REGION = (0, 478, 640, 122)
WEIGHT_REGION = (418, 500, 75, 72)
PILE_PICTURE = (0, 478, 145, 122)  # the panel's left frame and its picture
# AR1000 after leaving the house: the boats and snow below the party.
DOCKS_ARRIVAL = (0, 280, 800, 150)


def hostile_circles(img):
    """Centres of the red (255,0,40) selection circles of hostile creatures in
    the world view, one per creature."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[:430]
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    ys, xs = np.nonzero((r > 230) & (g < 30) & (b > 20) & (b < 60))
    pts = np.stack([xs, ys], axis=1) if len(xs) else np.zeros((0, 2), int)
    found = []
    while len(pts):
        near = (np.abs(pts[:, 0] - pts[0, 0]) < 40) & (np.abs(pts[:, 1] - pts[0, 1]) < 30)
        cx, cy = pts[near].mean(axis=0)
        near = (np.abs(pts[:, 0] - cx) < 30) & (np.abs(pts[:, 1] - cy) < 20)
        if not near.any():
            near = np.zeros(len(pts), bool)
            near[0] = True
        group = pts[near]
        if len(group) >= 8:
            found.append((int(group[:, 0].mean()), int(group[:, 1].mean())))
        pts = pts[~near]
    return found


def panel_open(img):
    """A ground-item or container panel is open: inside the house it replaces
    the action bar with the (black) world view."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    return a[440:475, 20:480].mean() < 30


def test_s13_house(game, golden_dir):
    # S9's checkpoints are not compared again: only wait as they would.
    docks_to_house(game, golden_dir,
                   lambda name, region, settle=1.0, timeout=30.0, **kw:
                   game.wait_stable(settle=settle, region=region, timeout=timeout))

    # Every checkpoint is taken even if an earlier one differs, so that one
    # run on the rebuild shows all the differences. They are reported together.
    failures = []

    def check(name, region, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    def tight(name, region):
        check(name, region, settle=0.5, max_diff=0.004)

    def frozen():
        """The world view does not change at all for a second (paused)."""
        first = game.grab()
        for _ in range(3):
            time.sleep(0.4)
            if diff_ratio(first, game.grab(), region=WORLD_REGION) > 0:
                return False
        return True

    def set_paused(want, what):
        game.move(*FIGHT_PARK)
        game.press("space")
        deadline = time.time() + 10.0
        while frozen() != want:
            if time.time() > deadline:
                raise AssertionError(f"{what}: Space did not {'pause' if want else 'unpause'} the game")
        time.sleep(1.0)

    def click(xy, region, **kw):
        act(game, lambda: game.click(*xy), f"click at {xy}", region, **kw)

    # --- Pause, order an attack while paused, unpause; until no goblin ------
    time.sleep(1.0)
    if not hostile_circles(game.grab()):
        raise AssertionError("no goblin in the house")
    deaths = []  # where goblins were last seen before they vanished
    first = True
    fight_end = time.time() + 90.0
    while True:
        set_paused(True, "pause")
        targets = hostile_circles(game.grab())
        if not targets:
            set_paused(False, "unpause")
            break
        if first:
            tight("paused-last-line", LAST_LINE_REGION)
            check("paused-action-bar", ACTION_BAR_REGION)
        game.click(*SELECT_ALL)
        game.move(*FIGHT_PARK)
        time.sleep(0.5)
        click(ATTACK, ACTION_BAR_REGION, park=FIGHT_PARK, wait=1.0)
        if first:
            check("attack-pressed-bar", ACTION_BAR_REGION)
        # The lowest goblin is drawn over the party: aim at its body.
        x, y = max(targets, key=lambda t: t[1])
        click((x, y - 12), ACTION_BAR_REGION, park=FIGHT_PARK, wait=1.0)
        set_paused(False, "unpause")
        first = False
        # Let it run until one more goblin is gone (or 8 s pass).
        seen = targets
        run_end = time.time() + 8.0
        while time.time() < run_end:
            now = hostile_circles(game.grab())
            gone = [p for p in seen
                    if not any(abs(p[0] - q[0]) < 40 and abs(p[1] - q[1]) < 30 for q in now)]
            if len(now) < len(seen):
                deaths += gone
                if not now:
                    break
            seen = now if now else seen
            time.sleep(0.25)
        # The experience line can come up to a second after the death (the
        # killer handles the kill on its next AI slot, every 16 AI ticks, once
        # its last order is more than 75 ticks old): let it arrive before the
        # next pause, so the lines are always experience, PAUSED, UNPAUSED.
        time.sleep(1.5)
        if time.time() > fight_end:
            raise AssertionError(f"{len(hostile_circles(game.grab()))} goblin(s) still alive after 90 s")
    # No hostile circle for 3 s.
    gone_since = time.time()
    while time.time() - gone_since < 3.0:
        if hostile_circles(game.grab()):
            raise AssertionError("a goblin is still alive")
        time.sleep(0.3)
    game.move(*FIGHT_PARK)
    time.sleep(2.0)
    check("fight-action-bar", ACTION_BAR_REGION)
    check("fight-end-lines", END_LINES_REGION, settle=0.5, max_diff=0.002)

    # --- A goblin's ground pile ---------------------------------------------
    # The party often stands on the piles (a click there selects whoever stands
    # on it): walk it to the middle of the carpet first.
    act(game, lambda: game.click(*CARPET), "click on the carpet", WORLD_REGION,
        park=FIGHT_PARK, wait=5.0)
    opened = False
    for spot in deaths:
        game.click(*spot)
        game.move(*PANEL_PARK)
        deadline = time.time() + 15.0
        while time.time() < deadline and not panel_open(game.grab()):
            time.sleep(0.3)
        if panel_open(game.grab()):
            opened = True
            break
    if opened:
        time.sleep(2.0)
        # Only the pile picture: the items are random treasure, and the
        # backpack is that of whoever walked over.
        check("goblin-pile", PILE_PICTURE)
        act(game, lambda: game.press("esc"), "Esc on the ground-item panel", ACTION_BAR_REGION,
            park=FIGHT_PARK)
    else:
        failures.append(f"no ground pile opened where the goblins died ({deaths})")

    # --- The locked footlocker ----------------------------------------------
    # Sheris first: she walks over and finds it locked. (When she comes later
    # Kylie stands in her way and her click does nothing.)
    click(SHERIS, ACTION_BAR_REGION, park=FIGHT_PARK)
    click(FOOTLOCKER, LAST_LINE_REGION, park=FIGHT_PARK, timeout=15.0, threshold=0.001)
    tight("locked-line", LAST_LINE_REGION)
    for attempt in range(5):
        click(KYLIE, ACTION_BAR_REGION, park=FIGHT_PARK)
        click(THIEVING, ACTION_BAR_REGION, park=FIGHT_PARK, wait=1.0)
        # She walks over and tries: a new message line.
        click(FOOTLOCKER, LAST_LINE_REGION, park=FIGHT_PARK, timeout=15.0, threshold=0.001)
        click(SHERIS, ACTION_BAR_REGION, park=FIGHT_PARK)
        game.click(*FOOTLOCKER)
        game.move(*PANEL_PARK)
        deadline = time.time() + 8.0
        while time.time() < deadline and not panel_open(game.grab()):
            time.sleep(0.3)
        if panel_open(game.grab()):
            break
    else:
        raise AssertionError("the footlocker never opened")
    time.sleep(2.0)
    check("footlocker", PILE_REGION, mask=[GOLD_COUNT])
    tight("footlocker-weight", WEIGHT_REGION)
    click((170, 515), PILE_REGION, park=PANEL_PARK)
    check("footlocker-taken", PILE_REGION, mask=[GOLD_COUNT_TAKEN])
    tight("footlocker-taken-weight", WEIGHT_REGION)
    act(game, lambda: game.press("esc"), "Esc on the container panel", ACTION_BAR_REGION,
        park=FIGHT_PARK)
    # Opening the footlocker and taking from it print nothing: the lock pick
    # line is still the last one.
    tight("lock-picked-line", LAST_LINE_REGION)

    # --- Cure Light Wounds on Sheris ----------------------------------------
    click(LAERIS, ACTION_BAR_REGION, park=FIGHT_PARK)
    click(CAST_SPELL, ACTION_BAR_REGION, park=FIGHT_PARK)
    click(CAST_CLERIC, ACTION_BAR_REGION, park=FIGHT_PARK)
    check("cure-spells-before", ACTION_BAR_REGION)
    click(CAST_CLERIC, ACTION_BAR_REGION, park=FIGHT_PARK)  # Cure Light Wounds: the bar returns
    game.click(*SHERIS)
    game.move(*FIGHT_PARK)
    time.sleep(8.0)
    click(CAST_SPELL, ACTION_BAR_REGION, park=FIGHT_PARK)
    click(CAST_CLERIC, ACTION_BAR_REGION, park=FIGHT_PARK)
    check("cure-spells-after", ACTION_BAR_REGION)
    act(game, lambda: game.press("esc"), "Esc on the spell list", ACTION_BAR_REGION, park=FIGHT_PARK)
    act(game, lambda: game.press("esc"), "Esc on the class list", ACTION_BAR_REGION, park=FIGHT_PARK)

    # --- Back out to the docks ----------------------------------------------
    click(SELECT_ALL, ACTION_BAR_REGION, park=FIGHT_PARK)
    group_bar = game.grab()
    act(game, lambda: game.click(*HOUSE_EXIT), "click on the exit", WORLD_REGION,
        threshold=0.5, wait=0.0, timeout=25.0)
    # The loading screen, then the docks: wait for the same action bar.
    deadline = time.time() + 60.0
    while diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) > 0.05:
        if time.time() > deadline:
            raise AssertionError("the docks did not appear after leaving the house")
        time.sleep(0.3)
    time.sleep(3.0)
    check("docks-arrival", DOCKS_ARRIVAL, max_diff=0.01)
    check("docks-arrival-action-bar", ACTION_BAR_REGION)

    assert not failures, "; ".join(failures)
