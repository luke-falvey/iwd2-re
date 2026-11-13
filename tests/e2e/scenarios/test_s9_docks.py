"""S9: off the ship and into Targos docks: scroll, talk to Reig, the first
goblin, a ground pile and a door into a house.

Continues S6 (whole party selected and walked off the ship onto the dock in
AR1000, game running). Then:

  scroll       hold the Right arrow key (the view must move; the distance
               depends on the frame rate, so it is not compared), then jump
               the view with a click on the area map and compare the static
               part of the newly shown dock (a boat and posts)
  Reig         walk the party along the dock, jump the view to Reig Redwaters
               and click him; his whole dialogue is compared, one reply at a
               time (1, 1, Continue x3, 1, 2, Continue, 1, 1, 1, 1)
  door         jump the view to the house west of Reig (door to AR1005),
               compare the closed door, click it: the party walks over and
               opens it, and a goblin runs in and attacks Sheris
  fight        click the goblin (found by its red selection circle) until it
               is dead; compare the action bar, the last message line ("The
               party has gained experience: 37") and the open door
  ground pile  click the goblin axe lying on the snow: the ground-item panel
               opens; take the axe and compare the panel and the backpack
               weight; Esc
  house        select all and click the open doorway: the party enters
               AR1005; compare the furnished part of the room

Not compared: hit points and the combat messages (dice rolls differ run to
run on the original: one run the goblin hit Sheris twice, another not at
all), the gold amount (random at party creation, shown in the dialogue and
ground-item panels), where characters stand, and the fight with the two
goblins inside the house, which starts as soon as the party enters.

Clicks in unexplored (black) parts of the map are ignored by the game, so
the party walks in short steps to ground that is already visible.

Map coordinates (AR1000 is 3200x2304; the area map draws it at 1/8 with
its top-left corner at (138, 73), so a click at area-map (x, y) centres the
view on map point ((x-138)*8, (y-73)*8), i.e. the top-left of the 800x430
view is ((x-138)*8-400, (y-73)*8-201)):
  Hedron 2503,900   Reig 1448,901   Jon 1369,959
  door to AR1005 (house) 823-868,870-930; goblin axe on the ground 490,956
  door to AR1001 (Magdar's warehouse, locked) 958-1005,486-540
  doors to AR1002/3/4/6: 2252-2503,455-601; 892-938,1764-1822;
             766-815,2192-2253; 259-306,1649-1713
  exit to AR1100 (palisade) 79-224,1-101
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

AREA_MAP_BUTTON = (619, 562)
SELECT_ALL = (585, 509)
# Area-map clicks that centre the view (see the docstring).
MAP_DOCK = (375, 198)  # the dock west of the ship: view at 1496,799
MAP_REIG = (319, 192)  # Reig: view at 1048,751
MAP_HOUSE_EAST = (276, 192)  # between Reig and the house: view at 704,751
MAP_HOUSE = (230, 190)  # the house and the goblin axe: view at 336,735
AREA_MAP_REGION = (97, 37, 483, 360)

# Screen spots in those views.
DOCK_WALK = (240, 195)  # MAP_DOCK view: the west end of the dock
REIG = (400, 135)  # MAP_REIG view
HOUSE_WALK_1 = (560, 170)  # MAP_HOUSE_EAST view: snow near Reig
HOUSE_WALK_2 = (420, 200)  # MAP_HOUSE_EAST view: in front of the house
HOUSE_DOOR = (509, 165)  # MAP_HOUSE view
GOBLIN_AXE = (154, 221)  # MAP_HOUSE view
PILE_FIRST_SLOT = (170, 515)  # first item in the ground-item panel

PARK_SPOT = (300, 560)  # the message window
DIALOG_PARK = (760, 300)  # world view above the dialogue panel
PANEL_PARK = (600, 330)  # empty snow while the ground-item panel is open
FIGHT_PARK = (700, 380)  # empty snow in the MAP_HOUSE view

WORLD_REGION = (0, 0, 800, 430)
# Static part of the MAP_DOCK view: a rowing boat, posts and water.
DOCK_STATIC = (280, 260, 520, 170)
# Dialogue text with the Continue bar (the gold amount on the right is not
# included: it is random).
DIALOG_REGION = (105, 320, 570, 280)
DOOR_REGION = (465, 95, 95, 115)  # MAP_HOUSE view
LAST_LINE_REGION = (8, 576, 535, 16)
# Ground-item panel without the gold amount; the backpack weight on its own.
PILE_REGION = (0, 478, 640, 122)
WEIGHT_REGION = (418, 500, 75, 72)
# Furnished part of the room in AR1005 (table, bed, chairs).
INSIDE_REGION = (360, 40, 300, 350)


def hostile_circle(img):
    """Centre of the red selection circles of hostile creatures in the world
    view, or None."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[:430]
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    ys, xs = np.nonzero((r > 230) & (g < 30) & (b > 20) & (b < 60))
    if len(xs) < 8:
        return None
    return int(xs.mean()), int(ys.mean())


def act(game, do, what, region, wait=2.0, park=PARK_SPOT, threshold=0.005, timeout=10.0):
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


def jump_view(game, map_spot):
    """Centre the world view on a map point through the area map."""
    act(game, lambda: game.click(*AREA_MAP_BUTTON), "area map button", ACTION_BAR_REGION)
    act(game, lambda: game.click(*map_spot), f"area map click at {map_spot}", AREA_MAP_REGION,
        wait=1.0, threshold=0.001)
    act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION)


def docks_to_house(game, golden_dir, check):
    """S9's steps, from the start of the game until the party is inside
    AR1005. `check(name, region, **kw)` takes each checkpoint (S13 passes one
    that only waits for the region to settle)."""
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)
    # S6: select the party on the deck and walk it onto the dock.
    game.drag(*BOX_FROM, *BOX_TO)
    time.sleep(1.0)
    game.click(*DOCK_SPOT)
    time.sleep(0.5)
    game.move(*PARK_SPOT)
    time.sleep(19.5)

    # --- Scrolling ----------------------------------------------------------
    # The arrow key scrolls by a distance that depends on the frame rate, so
    # only check that the view moved (a shift changes most of the picture;
    # water and idle poses alone change a few percent).
    act(game, lambda: game.press("right", hold=1.0), "holding the Right arrow", WORLD_REGION,
        threshold=0.15)

    act(game, lambda: game.click(*AREA_MAP_BUTTON), "area map button", ACTION_BAR_REGION)
    check("area-map", WORLD_REGION)
    act(game, lambda: game.click(*MAP_DOCK), "area map click", AREA_MAP_REGION, wait=1.0, threshold=0.001)
    check("area-map-moved", WORLD_REGION)
    act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION)
    check("scrolled-dock", DOCK_STATIC, max_diff=0.01, timeout=10.0)

    # --- Reig ---------------------------------------------------------------
    game.click(*DOCK_WALK)
    game.move(*PARK_SPOT)
    time.sleep(12.0)
    jump_view(game, MAP_REIG)
    # The party walks up to him first, so allow longer for the dialogue.
    act(game, lambda: game.click(*REIG), "click on Reig", ACTION_BAR_REGION, park=DIALOG_PARK,
        wait=2.0, timeout=20.0)
    check("reig-01", DIALOG_REGION)
    steps = ["1", "1", CONTINUE, CONTINUE, CONTINUE, "1", "2", CONTINUE, "1", "1", "1"]
    for i, step in enumerate(steps, start=2):
        if isinstance(step, str):
            act(game, lambda: game.press(step), f"reply {step} to Reig", DIALOG_REGION, park=DIALOG_PARK)
        else:
            act(game, lambda: game.click(*CONTINUE), "Continue with Reig", DIALOG_REGION, park=DIALOG_PARK)
        check(f"reig-{i:02d}", DIALOG_REGION)
    act(game, lambda: game.press("1"), "last reply to Reig", ACTION_BAR_REGION, wait=3.0)
    check("after-reig-action-bar", ACTION_BAR_REGION)

    # --- The house door and the goblin --------------------------------------
    jump_view(game, MAP_HOUSE_EAST)
    game.click(*HOUSE_WALK_1)
    game.move(*PARK_SPOT)
    time.sleep(6.0)
    game.click(*HOUSE_WALK_2)
    game.move(*PARK_SPOT)
    time.sleep(9.0)
    jump_view(game, MAP_HOUSE)
    check("door-closed", DOOR_REGION, max_diff=0.01)

    game.click(*HOUSE_DOOR)
    game.move(*PARK_SPOT)
    deadline = time.time() + 25.0
    while hostile_circle(game.grab()) is None:
        if time.time() > deadline:
            raise AssertionError("no goblin came when the party went to the door")
        time.sleep(0.3)
    # Attack it; click again if it is still alive (a click on the crowd can
    # select a party member instead).
    for attempt in range(5):
        time.sleep(1.0)
        spot = hostile_circle(game.grab())
        if spot is None:
            break
        game.click(spot[0], spot[1] - 12)
        game.move(*PARK_SPOT)
        deadline = time.time() + 10.0
        while time.time() < deadline and hostile_circle(game.grab()) is not None:
            time.sleep(0.3)
    gone_since = time.time()
    while time.time() - gone_since < 2.0:
        if hostile_circle(game.grab()) is not None:
            raise AssertionError("the goblin is still alive")
        time.sleep(0.3)
    # The last message line runs under PARK_SPOT: move the pointer off it.
    game.move(*FIGHT_PARK)
    time.sleep(2.0)
    check("fight-action-bar", ACTION_BAR_REGION)
    check("fight-last-line", LAST_LINE_REGION, max_diff=0.004)
    check("door-open", DOOR_REGION, max_diff=0.01)

    # --- The goblin axe on the snow -----------------------------------------
    act(game, lambda: game.click(*GOBLIN_AXE), "click on the goblin axe", PILE_REGION,
        park=PANEL_PARK, timeout=20.0)
    check("ground-pile", PILE_REGION)
    check("ground-pile-weight", WEIGHT_REGION, max_diff=0.004)
    act(game, lambda: game.click(*PILE_FIRST_SLOT), "taking the axe", PILE_REGION, park=PANEL_PARK)
    check("picked-up", PILE_REGION)
    check("picked-up-weight", WEIGHT_REGION, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the ground-item panel", ACTION_BAR_REGION)
    check("after-pile-action-bar", ACTION_BAR_REGION)

    # --- Into the house -----------------------------------------------------
    game.click(*SELECT_ALL)
    game.move(*PARK_SPOT)
    time.sleep(0.5)
    act(game, lambda: game.click(*HOUSE_DOOR), "click on the open door", WORLD_REGION,
        threshold=0.5, wait=3.0, timeout=25.0)
    check("inside-house", INSIDE_REGION, max_diff=0.02)



def test_s9_docks(game, golden_dir):
    # Every checkpoint is taken even if an earlier one differs, so that one
    # run on the rebuild shows all the differences. They are reported together.
    failures = []

    def check(name, region, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    docks_to_house(game, golden_dir, check)
    assert not failures, "; ".join(failures)
