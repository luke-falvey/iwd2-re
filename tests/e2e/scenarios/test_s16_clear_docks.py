"""S16: clearing the goblins from the Targos docks (AR1000), and a rest on
Hedron's ship.

What "the docks are cleared" means in the game data:

  * Each goblin area has a flag that its area script sets once all of its
    named goblins are dead: AR1000_GOBLINS_CLEAR (the docks), AR1002 (the
    warehouse behind the locked door at the east end), AR1004 (Firtha's house
    in the south-west), AR1005 (the house S13 clears) and AR1007 (the goblin
    camp behind AR1002, with Rukwurm). AR1001 (Magdar's warehouse) has its own
    flag but it is not part of the gate.
  * When all five flags are 1, whichever of those area scripts runs next sets
    Dock_Goblin_Quest = 1, awards the party experience (AddXPVar
    Level_2_Very_Hard) and adds journal entry 27866 ("We managed to drive the
    last of the goblins off of the Targos Docks. Reig and Lord Ulbrec should
    be grateful to hear of our success.").
  * Dock_Goblin_Quest = 1 is what opens the town: Cahl-Hyred rents rooms at
    the Weeping Widow (before, "'Ave ye not 'eard? There's goblins at the
    docks!"), Deirdre opens her store, and Reig, Hedron and others get new
    replies (Reig then sends the party to Lord Ulbrec). Nobody has to be told
    for the flag to be set.
  * Resting: AR1000 and AR1100 have the "no rest" area flag and no script
    lifts it, before or after the gate. The houses lift it themselves once
    their own goblins are dead (AR1001, AR1002, AR1005: AllowAreaResting).
    Before the gate, Hedron rests the party on his ship (RestUntilHealed, in
    his dialogue once Reig has been met), and Reig sends the party to him.

Which goblins exist depends on the difficulty: each actor in an area has a
difficulty mask and most AR1000 goblins only appear at the higher settings.
At the default setting AR1000 has 8 goblins (1000_Goblin_01 to _07 and
Archer_01); the area script counts the ones that never appeared as dead (a
quick save made after killing those 8 had AR1000_GOBLINS_CLEAR = 1). The
whole gate at this setting is 8 + 2 (AR1005) + 3 (AR1004) + 5 (AR1002, behind
a locked door) + 18 (AR1007, with Rukwurm) creatures in five areas: more than
one 12-minute test can hold. This test clears AR1000 and rests.

Steps, after the start of the game (new game, Hedron's opening dialogue, the
party walked off the ship and S9's talk with Reig, not compared):

  goblin 1    S9's goblin west of the AR1005 house (fought as below)
  south-west  walk past the half-built ship to Jorun Tamewater: two goblins
              (Goblin_02/03); Jorun fights them too, so whether the party or
              he makes the last kill varies and nothing is compared
  south-east  along the south shore past the Salty Dog and Crandall to three
              goblins (Goblin_06/07, Archer_01); compare the last experience
              line ("The party has gained experience: 37", found by colour)
  east pier   north and east onto the pier by the dead sailor: two goblins
              (Goblin_04/05); compare the last experience line. AR1000 is
              now clear (AR1000_GOBLINS_CLEAR, nothing on screen shows it)
  spell       Laeris: Cast Spell -> Cleric (Cure Light Wounds, Summon Monster
              I), Cure Light Wounds on Sheris: the list then holds only
              Summon Monster I (compare both)
  Hedron      walk onto the ship and click Hedron: compare his first line ("I
              was hoping ye might come back - what's the word from the
              docks?"), reply 1 ("goblins"), compare, reply 1: "Your journal
              has been updated." and his question about his mother (compare);
              reply 2 ("we need someplace safe to rest"): the screen fades,
              the party rests
  rest        compare "You have rested for" (found by colour; the number of
              hours depends on the damage taken, so it is not compared), the
              portraits (every hit point back: 5/5 13/13 14/14 12/12 6/6
              7/7) and Laeris's cleric list (Cure Light Wounds is back; also
              checked against the list before the cast)

Every fight is fought the way S13 does it: pause (Space), Select All, Attack,
click the lowest goblin, unpause, until a pause shows no goblin. The pause is
proved by the new PAUSED line in the message window and a world view that
keeps still (snow still falls a little while paused). A target click that
does not release the Attack button is retried a little lower and higher.

Not compared: hit points before the rest, combat messages and the "Healed N
hit points" lines (dice), the hour count, where characters stand, the world
view (it is night after the rest), Jorun's and Crandall's overhead lines.

AR1000 map points used (see test_s9_docks for the area-map formula): Hedron
on his ship at 2503,900 (the party stands round him when it comes aboard: it
is walked to 2380,830 first), Eldgull 2667,953, Screed on the Wicked Wench
2011,843, Jorun 641,1461, Crandall 1709,1857. The goblins start at
611,1896 and 713,1812 (south-west), 2038,2173 2094,2106 2161,2157
(south-east) and 2172,1457 2186,1509 (east pier).
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
from test_s13_house import ATTACK, CAST_CLERIC, CAST_SPELL, hostile_circles
from test_s14_travel import WEST_OF_HOUSE, select_all, walk

SHERIS, LAERIS = PORTRAIT[0], PORTRAIT[3]
MAP_ORIGIN = (138, 73)  # AR1000's area map: 1/8 scale

# Off the world view (its pause check) and off the message lines it reads.
FIGHT_PARK = (520, 515)

WORLD_REGION = (0, 0, 800, 430)
MESSAGES = (5, 495, 540, 100)
PORTRAITS = (500, 435, 300, 55)
# A status icon at the bottom left of Vraedna's portrait: it was there after
# an 8-hour rest and gone after a 16-hour one, so it is not compared. (A mask
# is relative to the checkpoint's region: screen 698,466.)
VRAEDNA_ICON = (198, 31, 18, 18)
LINE_SPOT = (8, 576, 535, 14)  # where a found message line is compared
REST_WORDS = (8, 576, 125, 14)  # "You have rested for" without the hours

# Walks: (point the view is centred on, or None to keep the view, point
# walked to), map coordinates. Every point walked to is ground the party has
# already seen (clicks on unexplored ground are ignored).
TO_SOUTH_WEST = [
    ((1200, 1296), (1000, 1200)),  # from the house, south-east of the hull
    ((896, 1248), (696, 1377)),  # round the half-built ship
    ((696, 1536), (700, 1470)),
    (None, (700, 1640)),  # Jorun and two goblins
]
TO_SOUTH_EAST = [
    ((1000, 1800), (900, 1919)),  # below the Salty Dog
    (None, (1240, 1899)),
    ((1400, 1896), (1600, 1995)),  # below Crandall's dock
    ((1800, 2048), (1880, 2177)),  # three goblins
]
TO_EAST_PIER = [
    ((1496, 1752), (1426, 1751)),  # back up between the houses
    (None, (1516, 1591)),  # the west end of the pier
    ((2016, 1536), (1996, 1535)),  # two goblins
]
SHIP_VIEW = (2400, 952)
ON_DECK = (2380, 830)  # far enough from the gangplank for the whole party
HEDRON = (2502, 900)


def map_click_for(point):
    """Area-map click that centres the view on a map point, and the map point
    the view is then really centred on."""
    mx = MAP_ORIGIN[0] + point[0] // 8
    my = MAP_ORIGIN[1] + point[1] // 8
    return (mx, my), ((mx - MAP_ORIGIN[0]) * 8, (my - MAP_ORIGIN[1]) * 8)


def to_screen(centre, point):
    return point[0] - centre[0] + 400, point[1] - centre[1] + 201


def last_yellow_line(img, min_right=100):
    """The lowest long line of yellow text (experience, rest) in the message
    window, moved to LINE_SPOT on a black screen, or None. Only the yellow
    text is kept: the window's background texture differs from line to line."""
    from PIL import Image
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y0:y0 + h0, x0:x0 + w0]
    yellow = (a[..., 0] > 200) & (a[..., 1] > 200) & (a[..., 2] < 40)
    rows = [r for r in range(h0) if yellow[r].any() and np.nonzero(yellow[r])[0].max() + x0 > min_right]
    if not rows:
        return None
    top = rows[-1]
    while top - 1 in rows:
        top -= 1
    top = y0 + top - 1
    x, w, h = LINE_SPOT[0], LINE_SPOT[2], LINE_SPOT[3]
    line = np.asarray(img.convert("RGB"), dtype=np.int16)[top:top + h, x:x + w]
    text = ((line[..., 0] > 90) & (np.abs(line[..., 0] - line[..., 1]) < 30)
            & (line[..., 2] < line[..., 0] // 3))
    line = np.where(text[..., None], line, 0).astype(np.uint8)
    out = Image.new("RGB", img.size)
    out.paste(Image.fromarray(line), (LINE_SPOT[0], LINE_SPOT[1]))
    return out


def cyan_near(img, near, dx=35, dy=20):
    """Centre of the cyan (80,255,255) selection circle of a neutral creature
    close to `near` (Eldgull stands 60 px from Hedron), or None."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[:430]
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    ys, xs = np.nonzero((r > 60) & (r < 100) & (g > 240) & (b > 240)
                        & (np.abs(np.arange(a.shape[1])[None, :] - near[0]) < dx)
                        & (np.abs(np.arange(a.shape[0])[:, None] - near[1]) < dy))
    if len(xs) < 8:
        return None
    return int(xs.mean()), int(ys.mean())


def docks_after_reig(game, golden_dir):
    """S14's first steps: from the start of the game until Reig's dialogue is
    over (whole party selected, view on Reig). Returns a frame with the group
    action bar."""
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)
    game.drag(*BOX_FROM, *BOX_TO)
    time.sleep(1.0)
    game.click(*DOCK_SPOT)
    time.sleep(0.5)
    game.move(*PARK_SPOT)
    time.sleep(19.5)
    group_bar = game.grab()
    jump_view(game, MAP_DOCK)
    game.click(*DOCK_WALK)
    game.move(*PARK_SPOT)
    time.sleep(12.0)
    jump_view(game, MAP_REIG)
    act(game, lambda: game.click(*REIG), "click on Reig", ACTION_BAR_REGION, park=DIALOG_PARK,
        wait=2.0, timeout=20.0)
    for step in ["1", "1", CONTINUE, CONTINUE, CONTINUE, "1", "2", CONTINUE, "1", "1", "1"]:
        if isinstance(step, str):
            act(game, lambda: game.press(step), f"reply {step} to Reig", DIALOG_REGION, park=DIALOG_PARK)
        else:
            act(game, lambda: game.click(*CONTINUE), "Continue with Reig", DIALOG_REGION, park=DIALOG_PARK)
    act(game, lambda: game.press("1"), "last reply to Reig", ACTION_BAR_REGION, wait=3.0)
    return group_bar


def still(game):
    """The world view keeps (almost) still for a second: paused. Falling snow
    still changes about 0.1% of it while paused; a running game with water or
    people in view changes more than 0.3%."""
    first = game.grab()
    for _ in range(3):
        time.sleep(0.4)
        if diff_ratio(first, game.grab(), region=WORLD_REGION) > 0.003:
            return False
    return True


def set_paused(game, want, what):
    """Space, proved by the PAUSED or UNPAUSED line it prints; a pause also by
    a still world view (Space is pressed again if the view keeps moving)."""
    for _ in range(3):
        act(game, lambda: game.press("space"), f"Space ({what})", MESSAGES, park=FIGHT_PARK,
            wait=0.3, threshold=0.001)
        if not want:
            time.sleep(0.7)
            return
        if still(game):
            time.sleep(0.5)
            return
    raise AssertionError(f"{what}: Space did not pause the game")


def fight(game, what, shots=True):
    """Wait for a goblin, then fight S13's way until a pause shows none. A
    walk click on the way can leave one character selected: Select All is
    clicked before every order."""
    deadline = time.time() + 25.0
    while not hostile_circles(game.grab()):
        if time.time() > deadline:
            raise AssertionError(f"no goblin came ({what})")
        time.sleep(0.3)
    end = time.time() + 150.0
    while True:
        set_paused(game, True, what)
        targets = hostile_circles(game.grab())
        if not targets:
            set_paused(game, False, what)
            break
        game.click(*SELECT_ALL)
        game.move(*FIGHT_PARK)
        time.sleep(0.5)
        act(game, lambda: game.click(*ATTACK), f"Attack ({what})", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=1.0)
        # The lowest goblin is drawn over the party: aim at its body. If the
        # Attack button stays pressed the click missed (a circle partly hidden
        # behind a party member's has its centre found off to one side): aim
        # again round it, then at the next goblin.
        aims = [(x + dx, y + dy) for x, y in sorted(targets, key=lambda t: -t[1])
                for dx, dy in ((0, -12), (0, -2), (12, -12), (-12, -12), (0, -24))]
        for aim in aims:
            try:
                act(game, lambda: game.click(*aim), f"click on a goblin ({what})",
                    ACTION_BAR_REGION, park=FIGHT_PARK, wait=1.0, timeout=2.0)
                break
            except AssertionError:
                continue
        else:
            raise AssertionError(f"no click on the goblins at {targets} took ({what})")
        set_paused(game, False, what)
        seen = targets
        run_end = time.time() + 8.0
        while time.time() < run_end:
            now = hostile_circles(game.grab())
            if len(now) < len(seen):
                if not now:
                    break
            seen = now if now else seen
            time.sleep(0.25)
        # The experience line can come up to a second after the death.
        time.sleep(1.5)
        if time.time() > end:
            raise AssertionError(f"{len(hostile_circles(game.grab()))} goblin(s) left after 150 s ({what})")
    if shots:
        game.screenshot(f"after-{what}")


def test_s16_clear_docks(game, golden_dir):
    group_bar = docks_after_reig(game, golden_dir)
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
        """A checkpoint of a frame already taken (a line moved to LINE_SPOT)."""
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
        spot, real = map_click_for(centre)
        act(game, lambda: game.click(*AREA_MAP_BUTTON), "area map button", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=0.5)
        act(game, lambda: game.click(*spot), f"area map click at {spot}", AREA_MAP_REGION,
            wait=0.5, threshold=0.001, park=FIGHT_PARK)
        act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=1.0)
        return real

    def walk_route(route, here):
        real = None
        for centre, point in route:
            if centre is not None:
                real = jump(centre)
            game.click(*to_screen(real, point))
            game.move(*FIGHT_PARK)
            # The party walks about 60 px a second.
            time.sleep(2.0 + np.hypot(point[0] - here[0], point[1] - here[1]) / 60)
            here = point
        return here

    def click(xy, region, **kw):
        act(game, lambda: game.click(*xy), f"click at {xy}", region, park=FIGHT_PARK, **kw)

    # --- S9's goblin west of the house ----------------------------------------
    walk(game, MAP_HOUSE_EAST, HOUSE_WALK_1, 6.0)
    game.click(*HOUSE_WALK_2)
    game.move(*PARK_SPOT)
    time.sleep(9.0)
    walk(game, MAP_HOUSE, WEST_OF_HOUSE, 1.0)
    fight(game, "west-of-house")

    # --- South-west: two goblins and Jorun ------------------------------------
    select_all(game, group_bar)
    here = walk_route(TO_SOUTH_WEST, (736, 985))
    fight(game, "south-west")

    # --- South-east: three goblins --------------------------------------------
    select_all(game, group_bar)
    here = walk_route(TO_SOUTH_EAST, here)
    fight(game, "south-east")
    check_frame("south-east-experience", last_yellow_line(game.grab()), LINE_SPOT, max_diff=0.004)

    # --- The east pier: two goblins -------------------------------------------
    select_all(game, group_bar)
    here = walk_route(TO_EAST_PIER, here)
    fight(game, "east-pier")
    check_frame("east-pier-experience", last_yellow_line(game.grab()), LINE_SPOT, max_diff=0.004)

    # --- Cure Light Wounds on Sheris ------------------------------------------
    click(LAERIS, ACTION_BAR_REGION)
    click(CAST_SPELL, ACTION_BAR_REGION)
    click(CAST_CLERIC, ACTION_BAR_REGION)
    check("cure-spells-before", ACTION_BAR_REGION)
    spells_before = game.grab()
    click(CAST_CLERIC, ACTION_BAR_REGION)  # Cure Light Wounds: the bar returns
    game.click(*SHERIS)
    game.move(*FIGHT_PARK)
    time.sleep(8.0)
    click(CAST_SPELL, ACTION_BAR_REGION)
    click(CAST_CLERIC, ACTION_BAR_REGION)
    check("cure-spells-after", ACTION_BAR_REGION)
    act(game, lambda: game.press("esc"), "Esc on the spell list", ACTION_BAR_REGION, park=FIGHT_PARK)
    act(game, lambda: game.press("esc"), "Esc on the class list", ACTION_BAR_REGION, park=FIGHT_PARK)

    # --- Aboard Hedron's ship ---------------------------------------------------
    select_all(game, group_bar)
    real = jump(SHIP_VIEW)
    game.click(*to_screen(real, ON_DECK))
    game.move(*FIGHT_PARK)
    # The party crosses the gangplank one by one (30 s was not always
    # enough). Hedron's cyan circle shows once the party sees him; a click
    # on him before that only walks the party to where he stands.
    time.sleep(25.0)
    expected = to_screen(real, HEDRON)
    deadline = time.time() + 40.0
    while (spot := cyan_near(game.grab(), expected)) is None:
        if time.time() > deadline:
            raise AssertionError(f"Hedron's circle did not show near {expected}")
        time.sleep(1.0)
    time.sleep(15.0)
    # A click while some of the party is still on the gangplank sometimes only
    # walks it: click again.
    for attempt in range(3):
        spot = cyan_near(game.grab(), expected) or spot
        try:
            act(game, lambda: game.click(*spot), "click on Hedron", ACTION_BAR_REGION,
                park=DIALOG_PARK, wait=2.0, timeout=20.0)
            break
        except AssertionError:
            if attempt == 2:
                raise
    check("hedron-01", DIALOG_REGION)
    act(game, lambda: game.press("1"), "reply 1 to Hedron", DIALOG_REGION, park=DIALOG_PARK)
    check("hedron-02", DIALOG_REGION)
    act(game, lambda: game.press("1"), "reply 1 to Hedron", DIALOG_REGION, park=DIALOG_PARK)
    check("hedron-03-journal", DIALOG_REGION)

    # --- The rest ---------------------------------------------------------------
    act(game, lambda: game.press("2"), "reply 2 to Hedron (rest)", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=0.0, timeout=15.0)
    # The fade and the rest take a second or two; then "You have rested for N
    # hours." (yellow, after the yellow journal line) and white "Healed" lines.
    time.sleep(3.0)
    game.wait_stable(settle=2.0, region=MESSAGES, timeout=30.0)
    check_frame("rest-line", last_yellow_line(game.grab()), REST_WORDS, max_diff=0.004)
    check("rest-portraits", PORTRAITS, max_diff=0.004, mask=[VRAEDNA_ICON])
    click(LAERIS, ACTION_BAR_REGION)
    click(CAST_SPELL, ACTION_BAR_REGION)
    click(CAST_CLERIC, ACTION_BAR_REGION)
    check("rest-spells", ACTION_BAR_REGION)
    if diff_ratio(spells_before, game.grab(), region=ACTION_BAR_REGION) > 0.004:
        failures.append("the cleric list after the rest differs from the one before the cast")
    act(game, lambda: game.press("esc"), "Esc on the spell list", ACTION_BAR_REGION, park=FIGHT_PARK)
    act(game, lambda: game.press("esc"), "Esc on the class list", ACTION_BAR_REGION, park=FIGHT_PARK)

    print(f"S16 steps after Reig took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
