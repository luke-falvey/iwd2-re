"""S15: the town of Targos (AR1100) at the start of the game: Oswald
Fiddlebender's store, and resting (refused).

S14's steps up the log stairs into the town run first as the set-up
(docks_to_town; its loading-screen checkpoint is only waited for here). Then,
with the whole party selected:

  walk        across the town to Oswald by his airship in the north-east, in
              short steps through the area map (clicks on unexplored ground
              are ignored): north of the Weeping Widow inn, up the log ramp
              to the temple of Tempus, east past the house in front of the
              airship
  Oswald      click him (found by his cyan selection circle): compare his
              first line; then the replies 3, Continue, 1, 1, Continue, 1, 2,
              Continue, Continue, 3 ("Potions?"), 1 (compare his answer), 1:
              his store opens
  store       the Buy and Sell panel (compare it without the gold amount; the
              backpack weight on its own); right-click the Potion of Healing:
              its description (compare; Done); select it: Cost 81 (compare);
              Buy: it moves to Sheris's list as "Potion of Healing - 15 GP",
              the weight goes 37 -> 38 and the gold drops by 81; select her
              Short Sword: Price 5 (compare); Sell: it leaves her list, the
              weight goes to 35 and the gold rises by 5; the Identify panel
              (compare without the gold); Done
  rest        the Rest dial button: "You may not rest at this time." (no movie,
              no time passes; compare that line, found by its colour)
  inventory   Sheris's backpack holds the potion and no sword any more, weight
              35 (compare both) and the same gold as the store showed last

Party gold is random at party creation (130 to 160 seen), so the gold label
is never compared as a picture. Instead it is read (the digits of the NORMAL
font, matched against their shapes) and only the differences are checked:
-81 after the purchase, +5 after the sale, and the same amount again in the
inventory. A rebuild whose gold cannot be read fails that check.

What the story refuses here: Cahl-Hyred, the innkeeper of the Weeping Widow
(AR1104), sends the party back to the docks instead of renting a room ("'Ave
ye not 'eard? There's goblins at the docks!") while Dock_Goblin_Quest is 0,
and resting is refused everywhere (as on the docks, S11). Ragni Bellows in
the temple (AR1103) refuses to heal, and Deirdre Gallaway (AR1102) does not
open her store before the docks are cleared. Oswald's store (no rooms, no
drinks, no stealing) is the only one open. Cahl-Hyred is not visited here:
the inn costs two loading screens and a minute for a refusal, and with no
rest allowed there is nothing for a rest to restore, so no spell is cast.

Not compared: the world view (snow and day/night vary on the original),
where the party and Oswald stand (he wanders a little), Oswald's overhead
lines ("::Whistling::" and others come at random times into the message
window, so the refusal line is found by its green colour), the dialogue
panel's gold amount, and the portraits on the store screen (their frames
change outside the world screen).

AR1100 is 2880x2112; its area map draws it at 1/8 with its top-left corner
at (158, 85), so a click at area-map (x, y) centres the view on map point
((x-158)*8, (y-85)*8), i.e. map point (X, Y) shows at screen
(X - (x-158)*8 + 400, Y - (y-85)*8 + 201). Oswald starts at 2039,667; the
party arrives at 249,1376. Exits: To1104 (Weeping Widow) 797-841,1623-1683;
To1103 (temple) 1209-1306,535-626; To1102 (Gallaway depot) 1523-1570,
1739-1800.
"""

from __future__ import annotations

import time

import numpy as np

from checkpoints import checkpoint
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION, DETAILS as INV_DETAILS, INV_BACKPACK
from test_s9_docks import AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_REGION, act
from test_s14_travel import docks_to_town

FIGHT_PARK = (700, 380)  # world view, off the message lines
DIALOG_PARK = (760, 300)  # world view above the dialogue panel
STORE_PARK = (300, 540)  # bottom of the screen: outside every store region

REST = (720, 511)
INVENTORY = (656, 525)

# Area map of AR1100 (see the docstring).
MAP_ORIGIN = (158, 85)

# The walk to Oswald: (point the view is centred on, or None to keep the
# view, point walked to), map coordinates. Every point walked to must be
# ground the party has already seen (clicks on unexplored ground are
# ignored; area-map clicks are not).
WALK = [
    ((448, 1296), (280, 1130)),  # from the top of the stairs, north
    ((440, 1100), (600, 1070)),  # east, north of the Weeping Widow
    ((765, 1100), (930, 1125)),
    ((1055, 1137), (1180, 1150)),  # the foot of the log ramp
    ((1255, 1100), (1330, 1060)),  # onto the ramp
    ((1385, 930), (1440, 800)),  # north, west of the log pile
    ((1530, 820), (1650, 845)),  # east, past a rain barrel (a container)
    ((1800, 830), (1800, 870)),  # between two houses
    (None, (1960, 800)),  # in front of the airship
]
OSWALD_VIEW = (1880, 740)
OSWALD_START = (2039, 667)
PARTY_ARRIVAL = (249, 1376)

# Oswald's dialogue up to his store (numbers are reply keys).
OSWALD_STEPS = ["3", CONTINUE, "1", "1", CONTINUE, "1", "2", CONTINUE, CONTINUE, "3", "1"]

# Store screen (Buy and Sell panel).
STORE_FIRST = (155, 130)  # the store's first item: Potion of Healing
SHERIS_FIRST = (420, 130)  # Sheris's first item: Short Sword
BUY = (721, 203)
SELL = (721, 232)
STORE_DONE = (721, 397)
IDENTIFY_PANEL = (65, 200)
DESCRIPTION_DONE = (543, 456)

STORE_REGION = (0, 0, 800, 430)
GOLD_LABEL = (692, 88, 85, 24)  # masked: random
GOLD_MASK = [GOLD_LABEL]
STORE_LISTS = (130, 80, 510, 300)  # the store's list and Sheris's list
STORE_WEIGHT = (664, 118, 116, 68)  # the backpack with "37 lbs" and "90 lbs"
COST = (278, 385, 95, 24)
PRICE = (543, 385, 95, 24)
DESCRIPTION = (130, 15, 515, 475)
INV_GOLD = (692, 135, 90, 25)
INV_WEIGHT = INV_DETAILS["weight"]

WORLD_REGION = (0, 0, 800, 430)
MESSAGES = (5, 495, 540, 100)
LINE_SPOT = (8, 576, 535, 14)  # where a found message line is compared

# Digits of the NORMAL font (frames of NORMAL.BAM), used to read the gold.
DIGITS = {
    "0": ".####./##.###/##..##/##..##/##..##/##..##/##..##/##..##/#####./..##..",
    "1": "..###./#####./#.###./..###./..###./..###./..###./..###./######",
    "2": "#####./######/#..###/#..###/...##./..###./.###../######/######",
    "3": "#####./######/#..###/..###./.#####/...###/#...##/#...##/#####./.##...",
    "4": "..###./..###./.####./.####./##.##./######/######/...##./.#####",
    "5": "######/######/##..../#####./######/##..##/#...##/##..##/#####./.###..",
    "6": "..####/.###../###.../#####./######/##..##/##..##/##..##/######/..##..",
    "7": "######/######/#...##/#..##./..###./..##../.###../.##.../###...",
    "8": ".####./##.###/##..##/#####./######/######/##..##/##..##/#####./.###..",
    "9": "#####./##.###/##..##/##..##/######/.#####/...###/..###./####../##....",
}
TEMPLATES = {d: np.array([[c == "#" for c in row] for row in s.split("/")], float)
             for d, s in DIGITS.items()}


def read_number(img, region):
    """Read a number drawn in the NORMAL font: the best-correlating digit
    shapes that do not overlap, left to right. None if there are none."""
    x0, y0, w0, h0 = region
    a = np.asarray(img.convert("L").crop((x0, y0, x0 + w0, y0 + h0)), dtype=float)
    a = np.clip((a - 50) / 150, 0, 1)
    found = []
    for d, t in TEMPLATES.items():
        h, w = t.shape
        tz = t - t.mean()
        tn = np.sqrt((tz * tz).sum())
        for y in range(a.shape[0] - h + 1):
            for x in range(a.shape[1] - w + 1):
                p = a[y:y + h, x:x + w]
                pz = p - p.mean()
                den = np.sqrt((pz * pz).sum()) * tn
                if den > 0:
                    found.append(((pz * tz).sum() / den, x, d))
    found.sort(reverse=True)
    taken = []
    for score, x, d in found:
        if score < 0.6:
            break
        if all(abs(x - u) >= 5 for u, _ in taken):
            taken.append((x, d))
    if not taken:
        return None
    return int("".join(d for _, d in sorted(taken)))


def cyan_circle(img, near):
    """Centre of the cyan (80,255,255) selection circle of a neutral creature
    nearest to `near` in the world view, or None."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[:430]
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    ys, xs = np.nonzero((r > 60) & (r < 100) & (g > 240) & (b > 240))
    if len(xs) < 8:
        return None
    near_pts = (np.abs(xs - near[0]) < 150) & (np.abs(ys - near[1]) < 120)
    if near_pts.sum() < 8:
        return None
    return int(xs[near_pts].mean()), int(ys[near_pts].mean())


def map_click_for(point):
    """Area-map click that centres the view on a map point, and the map point
    the view is then really centred on (the area map has 1/8 resolution)."""
    mx = MAP_ORIGIN[0] + point[0] // 8
    my = MAP_ORIGIN[1] + point[1] // 8
    return (mx, my), ((mx - MAP_ORIGIN[0]) * 8, (my - MAP_ORIGIN[1]) * 8)


def to_screen(centre, point):
    return point[0] - centre[0] + 400, point[1] - centre[1] + 201


def last_green_line(img):
    """The lowest line of green (feedback) text in the message window, moved
    to LINE_SPOT on a black screen, or None."""
    from PIL import Image
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y0:y0 + h0, x0:x0 + w0]
    rows = np.nonzero(((a[..., 1] > 200) & (a[..., 0] < 120) & (a[..., 2] < 120)).any(axis=1))[0]
    if not len(rows):
        return None
    bottom = y0 + int(rows.max())
    top = bottom - 11
    out = Image.new("RGB", img.size)
    out.paste(img.crop((LINE_SPOT[0], top, LINE_SPOT[0] + LINE_SPOT[2], top + LINE_SPOT[3])),
              (LINE_SPOT[0], LINE_SPOT[1]))
    return out


def test_s15_town(game, golden_dir):
    failures = []
    docks_to_town(game, golden_dir,
                  lambda name, region, settle=1.0, timeout=30.0, **kw:
                  game.wait_stable(settle=settle, region=region, timeout=timeout),
                  failures)
    t0 = time.time()

    # Every checkpoint is taken even if an earlier one differs, so that one
    # run on the rebuild shows all the differences. They are reported together.
    def check(name, region, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    def tight(name, region):
        check(name, region, settle=0.5, max_diff=0.004)

    def jump(centre):
        spot, real = map_click_for(centre)
        act(game, lambda: game.click(*AREA_MAP_BUTTON), "area map button", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=0.5)
        act(game, lambda: game.click(*spot), f"area map click at {spot}", AREA_MAP_REGION,
            wait=0.5, threshold=0.001, park=FIGHT_PARK)
        act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=1.0)
        return real

    # --- Across the town to Oswald ------------------------------------------
    here = PARTY_ARRIVAL
    for centre, point in WALK:
        if centre is not None:
            real = jump(centre)
        game.click(*to_screen(real, point))
        game.move(*FIGHT_PARK)
        # The party walks about 60 px a second and must be close enough to
        # have seen the next point before it is clicked.
        time.sleep(2.0 + np.hypot(point[0] - here[0], point[1] - here[1]) / 60)
        here = point
    real = jump(OSWALD_VIEW)
    at_oswald(game, golden_dir, check, tight, failures, real)
    print(f"S15 steps took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)


def at_oswald(game, golden_dir, check, tight, failures, real):
    """From the view of Oswald (centred on `real`) to the end of the test."""
    expected = to_screen(real, OSWALD_START)
    spot = cyan_circle(game.grab(), expected)
    if spot is None:
        raise AssertionError(f"Oswald not found near {expected}")
    # Click the middle of the circle (his feet): a click higher up, on his
    # body, often missed him and only walked the party over.
    act(game, lambda: game.click(*spot), "click on Oswald", ACTION_BAR_REGION,
        park=DIALOG_PARK, wait=2.0, timeout=20.0)
    in_dialogue(game, golden_dir, check, tight, failures)


def in_dialogue(game, golden_dir, check, tight, failures):
    """From Oswald's first line to the end of the test."""
    def click(xy, region, park=STORE_PARK, **kw):
        act(game, lambda: game.click(*xy), f"click at {xy}", region, park=park, **kw)

    def gold(where, region):
        value = read_number(game.grab(), region)
        if value is None:
            failures.append(f"{where}: the gold amount could not be read")
        return value

    def expect_gold(where, region, want):
        value = gold(where, region)
        if value is not None and want is not None and value != want:
            failures.append(f"{where}: gold {value}, expected {want}")
        return value if value is not None else want

    check("oswald-first", DIALOG_REGION)
    for i, step in enumerate(OSWALD_STEPS):
        if isinstance(step, str):
            act(game, lambda: game.press(step), f"reply {step} to Oswald", DIALOG_REGION,
                park=DIALOG_PARK)
        else:
            act(game, lambda: game.click(*CONTINUE), "Continue with Oswald", DIALOG_REGION,
                park=DIALOG_PARK)
    check("oswald-potions", DIALOG_REGION)
    act(game, lambda: game.press("1"), "last reply to Oswald", DIALOG_REGION, park=STORE_PARK,
        wait=2.0, threshold=0.3)

    # --- Buy and Sell -------------------------------------------------------
    check("store-buy-sell", STORE_REGION, mask=GOLD_MASK)
    tight("store-weight", STORE_WEIGHT)
    start = gold("store at first", GOLD_LABEL)
    act(game, lambda: game.click(*STORE_FIRST, button="right"), "right-click on the potion",
        STORE_REGION, park=STORE_PARK)
    check("store-description", DESCRIPTION)
    click(DESCRIPTION_DONE, STORE_REGION)
    click(STORE_FIRST, COST, threshold=0.001)
    tight("store-cost", COST)
    click(BUY, STORE_LISTS)
    check("store-bought", STORE_LISTS)
    tight("store-bought-weight", STORE_WEIGHT)
    now = expect_gold("after buying", GOLD_LABEL, start - 81 if start is not None else None)
    click(SHERIS_FIRST, PRICE, threshold=0.001)
    tight("store-price", PRICE)
    click(SELL, STORE_LISTS)
    check("store-sold", STORE_LISTS)
    tight("store-sold-weight", STORE_WEIGHT)
    now = expect_gold("after selling", GOLD_LABEL, now + 5 if now is not None else None)

    # --- Identify -----------------------------------------------------------
    click(IDENTIFY_PANEL, STORE_REGION)
    check("store-identify", STORE_REGION, mask=GOLD_MASK)
    act(game, lambda: game.click(*STORE_DONE), "Done on the store", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=2.0)

    # --- Rest: refused ------------------------------------------------------
    act(game, lambda: game.click(*REST), "Rest button", MESSAGES, park=FIGHT_PARK,
        threshold=0.001, wait=1.0)
    line = last_green_line(game.grab())
    if line is None:
        failures.append("no green line after the Rest button")
    else:
        stable = game.wait_stable
        game.wait_stable = lambda **_: line
        try:
            check("rest-refused-line", LINE_SPOT, max_diff=0.004)
        finally:
            game.wait_stable = stable

    # --- Inventory ----------------------------------------------------------
    act(game, lambda: game.click(*INVENTORY), "inventory button", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=3.0)
    check("inventory-backpack", INV_BACKPACK)
    tight("inventory-weight", INV_WEIGHT)
    expect_gold("inventory", INV_GOLD, now)
    act(game, lambda: game.press("esc"), "Esc on the inventory", ACTION_BAR_REGION,
        park=FIGHT_PARK)
