"""S14: from the Targos docks (AR1000) up the log stairs into the town
(AR1100), the refusal to leave without the whole party, back down to the
docks, and in and out of the house by the cliff (AR1006).

Continues S6 (whole party selected and walked off the ship onto the dock in
AR1000, game running). Then:

  Reig       walk west along the dock as S9 does and click Reig: his
             dialogue starts on its own when the party comes near him, so it
             has to be gone through (S9's replies; not compared again here)
  goblin     walk to the snow west of the house S9 and S13 use: the goblin
             from S9 runs up and attacks; click it until it is dead
  stairs     walk north to the foot of the log stairs at the top-left corner
             of the map, climb them and click the exit at the top (To1100):
             compare the loading screen ("Loading Area..." and its picture,
             a still frame caught while it is up), then
             the town at the top of the stairs (the roof of the big house on
             the right) and the action bar
  area map   the area map of AR1100 (only the patch round the stairs is
             explored); Esc
  gather     Sheris alone (portrait 1) clicks the exit back down the stairs:
             she walks there and the last line reads "You must gather your
             party before venturing forth."
  back       select all, click the exit again: the loading screen, then the
             docks at the top of the stairs (the gate tower) and the action
             bar
  AR1006     walk south along the cliff to the house above the half-built
             ship and click its door: it opens and a goblin attacks (a second
             one fights Jorun Tamewater further off); kill the one at the
             door, select all and go in: compare the room (crates, carpet);
             click the exit at the bottom left: back on the docks by the open
             door (compare the house and the tower below it)

Not compared: Reig's dialogue and the fights (S9 and S13 cover them; dice
rolls differ), hit points, the message window on arrival (each loading
screen prints its random hint there as well), the hint and progress bar of
the loading screen, where the characters stand. The original has no world-map
button on the area-map screen and both exits used here lead straight to the
next area, so no world map is reachable from the start of the game. The
other buildings on the docks are locked (AR1001, AR1002) or behind the
goblin groups in the south (AR1003, AR1004).

Map coordinates (see test_s9_docks for the area-map formula): exit to
AR1100 79-224,1-101 (arrive in AR1100 at 249,1376); AR1100's exit back
86-283,1511-2108 (arrive in AR1000 at 299,513); door to AR1006
259-306,1649-1713 (AR1006's exit 46-177,292-496, arrive in AR1000 at
377,1797). The goblin by the AR1006 door and the one fighting Jorun (641,1461)
come when the party reaches the door.
"""

from __future__ import annotations

import time

import numpy as np

from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s2_new_game import new_game_to_opening_dialog
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION, finish_hedron_dialog
from test_s6_walk_off_ship import BOX_FROM, BOX_TO, DOCK_SPOT
from test_s8_screens_tour import PORTRAIT
from test_s9_docks import (
    AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, DOCK_WALK, HOUSE_WALK_1,
    HOUSE_WALK_2, MAP_DOCK, MAP_HOUSE, MAP_HOUSE_EAST, MAP_REIG, PARK_SPOT, REIG, SELECT_ALL,
    act, jump_view)
from test_s13_house import hostile_circles

SHERIS = PORTRAIT[0]
FIGHT_PARK = (700, 380)

# AR1000: area-map clicks and the screen spots in the views they give.
WEST_OF_HOUSE = (400, 250)  # MAP_HOUSE view: snow west of the house
TRAIL_NORTH = (230, 30)  # MAP_HOUSE view: the trail going north
MAP_GATE = (195, 140)  # the gate tower: view at 16,335
GATE_WEST = (250, 240)  # snow west of the gate tower
MAP_STAIRS = (170, 100)  # the log stairs: view at 0,15
STAIRS_FOOT = (225, 370)
STAIRS_EXIT = (190, 60)  # the top of the stairs: To1100
# AR1100 as the view shows it on arrival.
TOWN_EXIT = (230, 395)  # down the stairs: To1000
# AR1000 after coming back (party at 299,513).
MAP_SNOW_WEST = (205, 175)
SNOW_WEST = (200, 200)
MAP_SHIP_WEST = (200, 200)
TRAIL_SOUTH = (175, 320)
MAP_CLIFF = (175, 235)
CLIFF_SOUTH = (230, 330)
MAP_HOUSE_1006 = (175, 270)  # view at 0,1375
DOOR_1006 = (285, 295)
DOORWAY_1006 = (270, 290)
# AR1006 as the view shows it on entering.
EXIT_1006 = (200, 330)

WORLD_REGION = (0, 0, 800, 430)
LAST_LINE_REGION = (8, 576, 535, 16)
# The loading screen without its random hint and the progress bar: the
# "Loading Area..." title and the picture (the same tower each time).
LOADING_SCREEN = (20, 18, 760, 422)
TOWN_ROOF = (420, 0, 280, 330)  # AR1100 on arrival: roof of the big house
GATE_TOWER = (380, 160, 290, 250)  # AR1000 back at the top of the stairs
# AR1006: the crates and the carpet (the party and the flickering lamps
# kept out: a larger region never settled).
ROOM_1006 = (400, 140, 240, 100)
HOUSE_1006 = (230, 0, 310, 180)  # AR1000 outside AR1006: the house


def select_all(game, group_bar):
    """Select All until the group action bar shows (a click right after a
    fight is sometimes lost, and then the party splits at an exit)."""
    for _ in range(3):
        game.click(*SELECT_ALL)
        game.move(*FIGHT_PARK)
        time.sleep(1.0)
        if diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) <= 0.05:
            return
        # A click that landed on a ground pile leaves its panel over the bar.
        game.press("esc")
        time.sleep(0.5)
    raise AssertionError("Select All did not bring up the group action bar")


def walk(game, map_spot, spot, secs):
    """Jump the view through the area map, then walk the party to a spot."""
    jump_view(game, map_spot)
    game.click(*spot)
    game.move(*FIGHT_PARK)
    time.sleep(secs)


def fight(game, what):
    """Wait for a hostile creature, then click it until none is left."""
    deadline = time.time() + 25.0
    while not hostile_circles(game.grab()):
        if time.time() > deadline:
            raise AssertionError(f"no goblin came ({what})")
        time.sleep(0.3)
    end = time.time() + 60.0
    while time.time() < end:
        targets = hostile_circles(game.grab())
        if not targets:
            time.sleep(1.0)
            if not hostile_circles(game.grab()):
                break
            continue
        # An earlier click that landed on a party member in the crowd leaves
        # only that member selected, and one member alone may not win.
        game.click(*SELECT_ALL)
        game.move(*FIGHT_PARK)
        time.sleep(0.3)
        targets = hostile_circles(game.grab())
        if not targets:
            continue
        x, y = max(targets, key=lambda t: t[1])
        game.click(x, y - 12)
        game.move(*FIGHT_PARK)
        time.sleep(3.0)
    else:
        raise AssertionError(f"the goblin is still alive ({what})")
    # The experience line can lag the death by up to a second.
    time.sleep(1.5)


def travel(game, group_bar, spot, what, prefix, check, failures):
    """Click an exit with the whole party selected: the loading screen,
    then the next area with the same action bar. `check(prefix + "-loading",
    LOADING_SCREEN)` is called with a still frame of the loading screen."""
    # A walk or fight click on the way can land on a party member and leave
    # only that one selected: an exit then refuses ("You must gather your
    # party"). Select everyone again first.
    select_all(game, group_bar)
    act(game, lambda: game.click(*spot), what, ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.5, wait=0.0, timeout=25.0)
    # The loading screen can be up for only a second or two: keep the last
    # frame that equals the one before it while the world bar is gone.
    loading, prev = None, None
    deadline = time.time() + 60.0
    while True:
        cur = game.grab()
        if diff_ratio(group_bar, cur, region=ACTION_BAR_REGION) <= 0.05:
            break
        if time.time() > deadline:
            raise AssertionError(f"no area after the loading screen ({what})")
        if (prev is not None and diff_ratio(prev, cur, region=LOADING_SCREEN) == 0
                and np.asarray(cur.convert("L"))[18:440, 20:780].mean() > 40):
            loading = cur
        prev = cur
        time.sleep(0.15)
    if loading is None:
        failures.append(f"{prefix}: no still loading screen seen")
    else:
        stable = game.wait_stable
        game.wait_stable = lambda **_: loading
        try:
            check(prefix + "-loading", LOADING_SCREEN)
        finally:
            game.wait_stable = stable
    time.sleep(3.0)


def docks_to_town(game, golden_dir, check, failures):
    """S14's steps from the start of the game until the party has arrived in
    AR1100 at the top of the log stairs (whole party selected, the view as on
    arrival). `check` takes the loading-screen checkpoint ("town-loading");
    problems with the loading screen are added to `failures`. Returns a frame
    with the group action bar (for select_all and travel)."""
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)
    # S6: select the party on the deck and walk it onto the dock.
    game.drag(*BOX_FROM, *BOX_TO)
    time.sleep(1.0)
    game.click(*DOCK_SPOT)
    time.sleep(0.5)
    game.move(*PARK_SPOT)
    time.sleep(19.5)
    # The whole party is selected: the action bar with the formation buttons.
    group_bar = game.grab()

    # --- West along the dock; Reig stops the party --------------------------
    jump_view(game, MAP_DOCK)
    game.click(*DOCK_WALK)
    game.move(*PARK_SPOT)
    time.sleep(12.0)
    jump_view(game, MAP_REIG)
    act(game, lambda: game.click(*REIG), "click on Reig", ACTION_BAR_REGION, park=DIALOG_PARK,
        wait=2.0, timeout=20.0)
    steps = ["1", "1", CONTINUE, CONTINUE, CONTINUE, "1", "2", CONTINUE, "1", "1", "1"]
    for step in steps:
        if isinstance(step, str):
            act(game, lambda: game.press(step), f"reply {step} to Reig", DIALOG_REGION, park=DIALOG_PARK)
        else:
            act(game, lambda: game.click(*CONTINUE), "Continue with Reig", DIALOG_REGION, park=DIALOG_PARK)
    act(game, lambda: game.press("1"), "last reply to Reig", ACTION_BAR_REGION, wait=3.0)

    # --- Past the house: S9's goblin ------------------------------------------
    walk(game, MAP_HOUSE_EAST, HOUSE_WALK_1, 6.0)
    game.click(*HOUSE_WALK_2)
    game.move(*PARK_SPOT)
    time.sleep(9.0)
    walk(game, MAP_HOUSE, WEST_OF_HOUSE, 1.0)
    fight(game, "west of the house")

    # --- North to the log stairs and up into the town -------------------------
    select_all(game, group_bar)
    # The view is still at MAP_HOUSE (a click on the area map where the view
    # already is changes nothing).
    game.click(*TRAIL_NORTH)
    game.move(*FIGHT_PARK)
    time.sleep(8.0)
    walk(game, MAP_GATE, GATE_WEST, 7.0)
    walk(game, MAP_STAIRS, STAIRS_FOOT, 6.0)
    travel(game, group_bar, STAIRS_EXIT, "click on the exit at the top of the stairs", "town",
           check, failures)
    return group_bar


def test_s14_travel(game, golden_dir):
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

    group_bar = docks_to_town(game, golden_dir, check, failures)
    t0 = time.time()
    check("town-arrival", TOWN_ROOF)
    check("town-arrival-action-bar", ACTION_BAR_REGION)

    act(game, lambda: game.click(*AREA_MAP_BUTTON), "area map button", ACTION_BAR_REGION)
    check("town-area-map", AREA_MAP_REGION)
    act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION)

    # --- Not without the whole party ------------------------------------------
    act(game, lambda: game.click(*SHERIS), "click on Sheris's portrait", ACTION_BAR_REGION,
        park=FIGHT_PARK)
    act(game, lambda: game.click(*TOWN_EXIT), "Sheris alone to the exit", LAST_LINE_REGION,
        park=FIGHT_PARK, threshold=0.001, timeout=15.0)
    tight("gather-party-line", LAST_LINE_REGION)

    # --- Back down to the docks -----------------------------------------------
    select_all(game, group_bar)
    travel(game, group_bar, TOWN_EXIT, "click on the exit down the stairs", "docks", check, failures)
    check("docks-arrival", GATE_TOWER)
    check("docks-arrival-action-bar", ACTION_BAR_REGION)

    # --- The house by the cliff (AR1006) --------------------------------------
    walk(game, MAP_SNOW_WEST, SNOW_WEST, 7.0)
    walk(game, MAP_SHIP_WEST, TRAIL_SOUTH, 8.0)
    walk(game, MAP_CLIFF, CLIFF_SOUTH, 6.0)
    walk(game, MAP_HOUSE_1006, DOOR_1006, 0.5)
    fight(game, "at the door of AR1006")
    select_all(game, group_bar)
    act(game, lambda: game.click(*DOORWAY_1006), "click on the open door", WORLD_REGION,
        park=FIGHT_PARK, threshold=0.5, wait=3.0, timeout=25.0)
    check("inside-1006", ROOM_1006)
    act(game, lambda: game.click(*EXIT_1006), "click on the exit of AR1006", WORLD_REGION,
        park=FIGHT_PARK, threshold=0.5, wait=3.0, timeout=25.0)
    check("outside-1006", HOUSE_1006)
    check("outside-1006-action-bar", ACTION_BAR_REGION)

    print(f"S14 steps in the town and after took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
