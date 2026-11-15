"""S19: the town of Targos (AR1100) once the docks are cleared: a room at the
Weeping Widow inn, Deirdre Gallaway's store, and Lord Ulbrec in the Town
Hall. Starts from S18's banked save and banks one of its own end state.

Saves (golden/bank/, git-ignored like all goldens):

  s18-end/  S18's end (see test_s18_warehouse_caves), loaded here: the party
            on the warehouse pier in AR1000, every dock goblin dead
            (Dock_Goblin_Quest = 1), Brogan gone, whole party selected, 126
            gold
  s19-end/  this test's end (on the original only): in the Town Hall
            (AR1101) after Ulbrec's dialogue (Ulbrec_Gate = 1: report to
            Shawford Crale at the Palisade), 123 gold, whole party selected

The banked save fixes the party's gold (126), so here the gold labels are
compared as pictures as well as read (S15 could only check differences).

test_s19_town_errands (both targets) loads the s18-end save the way S18
loads S17's, then, with the whole party selected:

  docks       west along the pier, past Reig, north past the gate tower to
              the foot of the log stairs (area-map steps; the last ones are
              S14's screen spots), up into the town (not compared: S14 does)
  inn         round the west and south sides of the Weeping Widow to its
              door; click it (the party opens it), then the doorway: AR1104.
              Click Cahl-Hyred behind the bar: his first line and the replies
              (only the text: the panel may show earlier message lines above
              it), replies 2 ("Just a traveler..."), 1, 1, Continue, each
              panel compared; 3 ("Let me see what rooms you have"): the Rooms
              panel (Peasant selected, Cost 3; compare it, gold 126 read and
              compared); Rent: "Are you sure you wish to rest?" (compare);
              Rest: back on the world screen at once (no movie) with "You
              have rested for N hours." and three "Healed N hit points"
              lines (compared as text without the hours, found by colour);
              the portraits (every hit point back); leave by the exit
  store       east along the inn's front, over the log ramp's foot to the
              Gallaway Trade Depot (AR1102); door, doorway; walk in and click
              Deirdre (her cyan circle): her first line and replies
              (compare), 2 ("I've come to purchase supplies.", compare), 1
              ("Very well."): the Buy and Sell panel (compare; gold 123, i.e.
              the room cost 3); select Arrows (Cost 1, compare); Buy: they
              join Sheris's list as "Arrows - 0 GP" (compare the lists, the
              weight, the gold: 121, two less than the Cost shows); select
              her Short Sword (Price 2, compare); Sell: gone from her list,
              weight 34, gold 123 (compare each); the Identify panel
              (compare; no steal panel at this store); Done; leave
  Town Hall   east along the cliff edge, over the bridge across the gully,
              north up the ramp to the Town Hall (AR1101) door; door,
              doorway. Click Ulbrec: his first line and replies (compare),
              replies 1, 1, Continue, Continue, 1, 1, Continue, 1, 2 (each
              panel compared; the last two show "Your journal has been
              updated."; no experience is given here), 2 ends it. The journal
              (J): the page's text without the yellow day/hour headings
  save        Q: compare the save lines; on the original the save is banked

Not compared: the world view (snow, day/night, people wandering), the
loading screens and their hints, Lumbar's, Cahl-Hyred's and Deirdre's
overhead lines, the hours of the rest, the journal's headings.

Map points (area maps at 1/8 scale; a click at (x,y) centres the view on
((x-ox)*8, (y-oy)*8) for the origin (ox,oy)): AR1000 origin (138,73),
AR1100 (158,85). AR1100: arrival from the docks 249,1376; the inn door
(DOOR2) 796-842,1621-1685 (arrival outside FR1104 793,1813); the depot door
(DOOR3) 1522-1571,1737-1802 (FR1102 1475,1916); the Town Hall door (DOOR4)
2290-2312,1351-1441 (FR1101 2180,1354). From the depot the ground east is a
narrow ledge along the cliff; the walkable layout is in AR1100SR.BMP (many
cells right next to the ledge are walls and a click there does nothing).
AR1104: Cahl-Hyred 729,480, three dogs wander (cyan circles), exit To1100
674-801,605-760. AR1102 (area map origin (278,168); the view is clamped at
the map's left edge): Deirdre 630,321, exit To1100 51-247,530-741. AR1101:
Ulbrec 493,430.
"""

from __future__ import annotations

import shutil
import time

import numpy as np
import pytest
from PIL import Image

import test_s16_clear_docks as s16
import test_s17_dock_houses as s17
import test_s18_warehouse_caves as s18
from checkpoints import checkpoint
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s9_docks import AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, act
from test_s14_travel import select_all, travel
from test_s15_town import (
    COST, GOLD_LABEL, PRICE, STORE_LISTS, STORE_PARK, STORE_REGION, STORE_WEIGHT, read_number)

S18_END = s18.S18_END
S19_END = s17.BANK / "s19-end"

FIGHT_PARK = s17.FIGHT_PARK  # (520, 515): off the world view
MESSAGES = s17.MESSAGES
PORTRAITS = (500, 435, 300, 55)
VRAEDNA_ICON = s16.VRAEDNA_ICON
JOURNAL_TEXT = s18.JOURNAL_TEXT

AR1000_MAP = (138, 73)
AR1100_MAP = (158, 85)
AR1102_MAP = (278, 168)

# Walks: (map point the view is centred on, map point walked to).
PIER_TO_GATE = [
    ((2000, 640), (2000, 600)),
    ((1700, 650), (1760, 610)),  # the west end of the warehouse pier
    ((1550, 750), (1500, 800)),
    ((1250, 850), (1200, 860)),  # past Reig
    ((900, 850), (864, 760)),
    ((600, 760), (600, 760)),  # west of the house S9 uses
    ((450, 650), (400, 620)),
]
# Then S14's views (raw area-map clicks: the view is clamped at the map's
# edge there, so screen spots are used).
MAP_GATE = (195, 140)
GATE_STEPS = [(220, 360), (250, 240)]  # west of the gate tower, then north
MAP_STAIRS = (170, 100)
STAIRS_FOOT = (225, 370)
STAIRS_EXIT = (190, 60)

TOWN_ARRIVAL = (249, 1376)
TO_INN = [
    ((400, 1500), (312, 1560)),  # south, west of the inn
    ((400, 1700), (340, 1740)),
    ((560, 1750), (560, 1800)),  # east along its south front
    ((760, 1750), (790, 1790)),
]
INN_DOOR = (800, 1668)
INN_DOORWAY = (800, 1638)
OUTSIDE_INN = (793, 1813)
TO_DEPOT = [
    ((1000, 1750), (1080, 1700)),
    ((1250, 1750), (1250, 1800)),  # over the foot of the log ramp
    ((1450, 1850), (1450, 1890)),
]
DEPOT_DOOR = (1546, 1772)
DEPOT_DOORWAY = (1546, 1757)
OUTSIDE_DEPOT = (1475, 1916)
TO_HALL = [
    ((1650, 1900), (1700, 1950)),  # east along the ledge by the cliff
    ((1850, 1900), (1850, 1950)),
    ((2000, 1850), (2020, 1935)),
    ((2050, 1800), (2060, 1840)),
    ((2150, 1800), (2160, 1850)),  # the west end of the bridge
    ((2200, 1800), (2296, 1860)),  # over it
    ((2300, 1700), (2296, 1700)),  # north on the far side
    ((2250, 1550), (2270, 1560)),  # up the ramp
    ((2200, 1400), (2216, 1446)),
    ((2200, 1350), (2184, 1360)),  # in front of the Town Hall door
]
HALL_DOOR = (2300, 1395)
HALL_DOORWAY = (2300, 1383)

# Inside, as the view shows each area on entering.
CAHL = (632, 145)
INN_EXIT = (660, 360)
DEPOT_WALK_IN = (470, 200)
DEIRDRE = (627, 51)
DEPOT_EXIT = (150, 380)  # after centring the clamped view (see AR1102_MAP)
ULBREC = (452, 112)

# Dialogues (numbers are reply keys).
CAHL_STEPS = ["2", "1", "1", CONTINUE]
ULBREC_STEPS = ["1", "1", CONTINUE, CONTINUE, "1", "1", CONTINUE, "1", "2"]

# The Rooms panel and the rest prompt.
RENT = (721, 135)
REST_CONFIRM = (451, 260)
REST_PROMPT = (250, 140, 285, 155)
# Store screen (Buy and Sell panel): as S15.
STORE_FIRST = (155, 130)  # Arrows - 1 GP
SHERIS_FIRST = (420, 130)  # Short Sword - 2 GP
BUY = (721, 203)
SELL = (721, 232)
STORE_DONE = (721, 397)
IDENTIFY_PANEL = (65, 200)

# Where found text blocks are compared (on a black screen).
SPEECH_AREA = (108, 325, 560, 218)  # the dialogue panel's text, without the Continue button
SPEECH_SPOT = (108, 330, 560, 215)
REST_SPOT = (8, 530, 535, 50)  # the rest line and the three healed lines
REST_HOURS_X = 133  # "You have rested for" ends left of this


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s18_save(play_dir, out_dir, target):
    """Put the banked S18 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S18_END / "save").iterdir() if p.is_dir()] if (S18_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S18 save in {S18_END}; run test_s18_warehouse_caves --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    if target == "original":
        S19_END.mkdir(parents=True, exist_ok=True)
    yield
    s17.finish_saves(play_dir, out_dir, S19_END if target == "original" else None)


# --- finding things on screen --------------------------------------------------


def speech_block(img):
    """The speaker's last paragraph and the replies under it, moved to
    SPEECH_SPOT on a black screen with only the text kept, or None. The
    first panel of a dialogue can show the message lines from before it
    above the speaker's line (S18), so the panel is not compared as it is."""
    x0, y0, w0, h0 = SPEECH_AREA
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y0:y0 + h0, x0:x0 + w0]
    text = a.max(axis=2) > 120
    red = (a[..., 0] > 200) & (a[..., 1] < 80) & (a[..., 2] < 80)
    red_rows = np.nonzero(red.any(axis=1))[0]
    reply_top = int(red_rows[0]) - 2 if len(red_rows) else h0
    paras = []
    for start, end in s18._runs(text[:reply_top].any(axis=1)):
        if paras and start - paras[-1][1] <= 6:
            paras[-1][1] = end
        else:
            paras.append([start, end])
    if not paras:
        return None
    top, bottom = paras[-1][0] - 1, paras[-1][1] + 1
    out = np.zeros((img.size[1], img.size[0], 3), np.uint8)
    sx, sy, sw, sh = SPEECH_SPOT
    y = sy
    for src0, src1 in ((top, bottom), (reply_top, h0)):
        n = min(src1 - src0, sy + sh - y)
        if n <= 0:
            continue
        block = a[src0:src0 + n]
        out[y:y + n, sx:sx + sw] = np.where(text[src0:src0 + n][..., None], block, 0)
        y += n + 4
    return Image.fromarray(out)


def rest_lines(img):
    """The yellow "You have rested for N hours." line and the three "Healed"
    lines under it, moved to REST_SPOT on a black screen with only the text
    kept and the hours blacked out, or None."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    m = a[y0:y0 + h0, 8:495]
    yellow = (m[..., 0] > 200) & (m[..., 1] > 200) & (m[..., 2] < 40)
    runs = [r for r in s18._runs(yellow.any(axis=1)) if r[1] - r[0] >= 6]
    if not runs:
        return None
    top = y0 + runs[-1][0] - 1
    x, w, h = REST_SPOT[0], REST_SPOT[2], REST_SPOT[3]
    block = a[top:top + h, x:x + w]
    if block.shape[0] < h:
        return None
    keep = block.max(axis=2) > 110
    keep[:, 495 - x:] = False  # the scroll arrow
    keep[:13, REST_HOURS_X - x:] = False  # the hours
    block = np.where(keep[..., None], block, 0).astype(np.uint8)
    out = Image.new("RGB", img.size)
    out.paste(Image.fromarray(block), (x, REST_SPOT[1]))
    return out


# --- the test ------------------------------------------------------------------


def test_s19_town_errands(from_s18_save, game, golden_dir, out_dir, target):
    group_bar = s17.load_bank(game, S18_END)
    t0 = time.time()

    failures = []

    def check(name, region, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    def tight(name, region):
        check(name, region, settle=0.5, max_diff=0.004)

    def check_frame(name, frame, region, **kw):
        if frame is None:
            failures.append(f"{name}: not found on screen")
            return
        stable = game.wait_stable
        game.wait_stable = lambda **_: frame
        try:
            check(name, region, **kw)
        finally:
            game.wait_stable = stable

    def expect_gold(where, want):
        value = read_number(game.grab(), GOLD_LABEL)
        if value is None:
            failures.append(f"{where}: the gold amount could not be read")
        elif value != want:
            failures.append(f"{where}: gold {value}, expected {want}")

    def area_map(spot):
        for attempt in range(3):
            # A click on the button is dropped now and then: click again.
            try:
                act(game, lambda: game.click(*AREA_MAP_BUTTON), "area map button", ACTION_BAR_REGION,
                    park=FIGHT_PARK, wait=0.5, timeout=5.0)
                break
            except AssertionError:
                if attempt == 2:
                    raise
        try:
            act(game, lambda: game.click(*spot), f"area map click at {spot}", AREA_MAP_REGION,
                wait=0.5, threshold=0.001, park=FIGHT_PARK, timeout=3.0)
        except AssertionError:
            pass  # the view was already centred there
        act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=1.0)

    def jump(centre, origin):
        mx, my = origin[0] + centre[0] // 8, origin[1] + centre[1] // 8
        area_map((mx, my))
        return (mx - origin[0]) * 8, (my - origin[1]) * 8

    def walk(route, here, origin):
        """Centre the view, wait until the spot is out of the black, walk the
        whole party there (about 50 px a second)."""
        real = None
        for centre, point in route:
            real = jump(centre, origin)
            spot = s16.to_screen(real, point)
            deadline = time.time() + 10.0
            while s16.unexplored(game.grab(), spot) and time.time() < deadline:
                time.sleep(0.5)
            game.click(*spot)
            game.move(*FIGHT_PARK)
            time.sleep(2.0 + np.hypot(point[0] - here[0], point[1] - here[1]) / 50)
            here = point
        return here, real

    def go(spot, what):
        """An exit or a doorway with the whole party selected: the loading
        screen (not compared: S14 does), then the next area."""
        travel(game, group_bar, spot, what, "loading",
               lambda name, region, **kw: None, [])

    def enter(door, doorway, real, what):
        """Click a door (the party walks over and opens it), then the doorway
        a little above it (a click where the party stands selects a member)."""
        select_all(game, group_bar)
        game.click(*s16.to_screen(real, door))
        game.move(*FIGHT_PARK)
        time.sleep(7.0)
        go(s16.to_screen(real, doorway), what)

    def talk(near, who, dx=30, dy=15):
        deadline = time.time() + 20.0
        while (spot := s16.cyan_near(game.grab(), near, dx=dx, dy=dy)) is None:
            if time.time() > deadline:
                raise AssertionError(f"{who}'s circle did not show near {near}")
            time.sleep(0.5)
        for attempt in range(3):
            spot = s16.cyan_near(game.grab(), near, dx=dx, dy=dy) or spot
            try:
                act(game, lambda: game.click(*spot), f"click on {who}", ACTION_BAR_REGION,
                    park=DIALOG_PARK, wait=2.0, timeout=20.0)
                return
            except AssertionError:
                if attempt == 2:
                    raise

    def dialogue(steps, who, prefix):
        check_frame(f"{prefix}-01", speech_block(game.wait_stable(settle=1.0, region=DIALOG_REGION)),
                    SPEECH_SPOT)
        for i, stp in enumerate(steps, start=2):
            if isinstance(stp, str):
                act(game, lambda: game.press(stp), f"reply {stp} to {who}", DIALOG_REGION,
                    park=DIALOG_PARK)
            else:
                act(game, lambda: game.click(*CONTINUE), f"Continue with {who}", DIALOG_REGION,
                    park=DIALOG_PARK)
            check(f"{prefix}-{i:02d}", DIALOG_REGION)

    def click(xy, region, what, **kw):
        act(game, lambda: game.click(*xy), what, region, park=STORE_PARK, **kw)

    # --- Up into the town ---------------------------------------------------------
    here, _ = walk(PIER_TO_GATE, (2250, 620), AR1000_MAP)
    area_map(MAP_GATE)
    for spot, secs in zip(GATE_STEPS, (7.0, 6.0)):
        game.click(*spot)
        game.move(*FIGHT_PARK)
        time.sleep(secs)
    area_map(MAP_STAIRS)
    game.click(*STAIRS_FOOT)
    game.move(*FIGHT_PARK)
    time.sleep(6.0)
    go(STAIRS_EXIT, "click on the exit at the top of the stairs")

    # --- The Weeping Widow: a room ----------------------------------------------
    here, real = walk(TO_INN, TOWN_ARRIVAL, AR1100_MAP)
    enter(INN_DOOR, INN_DOORWAY, real, "click on the doorway of the Weeping Widow")
    talk(CAHL, "Cahl-Hyred", dx=25, dy=12)
    dialogue(CAHL_STEPS, "Cahl-Hyred", "cahl")
    act(game, lambda: game.press("3"), "reply 3 to Cahl-Hyred (rooms)", DIALOG_REGION,
        park=STORE_PARK, wait=2.0, threshold=0.3)
    check("rooms", STORE_REGION)
    tight("rooms-gold", GOLD_LABEL)
    expect_gold("the Rooms panel", 126)
    click(RENT, REST_PROMPT, "Rent")
    check("rooms-rest-prompt", REST_PROMPT)
    act(game, lambda: game.click(*REST_CONFIRM), "Rest on the prompt", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=0.0, threshold=0.3, timeout=30.0)
    # The rest lines come at once; Cahl-Hyred's overhead lines follow them
    # every few seconds and scroll them up.
    deadline = time.time() + 5.0
    lines = None
    while time.time() < deadline:
        lines = rest_lines(game.grab()) or lines
        if lines is not None:
            break
        time.sleep(0.3)
    time.sleep(1.0)
    lines = rest_lines(game.grab()) or lines
    check_frame("rested-lines", lines, REST_SPOT, max_diff=0.004)
    check("rested-portraits", PORTRAITS, max_diff=0.004, mask=[VRAEDNA_ICON])
    go(INN_EXIT, "click on the exit of the Weeping Widow")

    # --- Deirdre's store -----------------------------------------------------------
    here, real = walk(TO_DEPOT, OUTSIDE_INN, AR1100_MAP)
    enter(DEPOT_DOOR, DEPOT_DOORWAY, real, "click on the doorway of the depot")
    game.click(*DEPOT_WALK_IN)
    game.move(*FIGHT_PARK)
    time.sleep(6.0)
    talk(DEIRDRE, "Deirdre")
    check_frame("deirdre-01", speech_block(game.wait_stable(settle=1.0, region=DIALOG_REGION)),
                SPEECH_SPOT)
    act(game, lambda: game.press("2"), "reply 2 to Deirdre", DIALOG_REGION, park=DIALOG_PARK)
    check("deirdre-02", DIALOG_REGION)
    act(game, lambda: game.press("1"), "reply 1 to Deirdre (store)", DIALOG_REGION,
        park=STORE_PARK, wait=2.0, threshold=0.3)
    check("store-buy-sell", STORE_REGION)
    tight("store-gold", GOLD_LABEL)
    expect_gold("the store at first", 123)
    click(STORE_FIRST, COST, "click on the arrows", threshold=0.001)
    tight("store-cost", COST)
    click(BUY, STORE_LISTS, "Buy", threshold=0.001)
    check("store-bought", STORE_LISTS)
    tight("store-bought-weight", STORE_WEIGHT)
    tight("store-bought-gold", GOLD_LABEL)
    expect_gold("after buying", 121)
    click(SHERIS_FIRST, PRICE, "click on the short sword", threshold=0.001)
    tight("store-price", PRICE)
    click(SELL, STORE_LISTS, "Sell", threshold=0.001)
    check("store-sold", STORE_LISTS)
    tight("store-sold-weight", STORE_WEIGHT)
    tight("store-sold-gold", GOLD_LABEL)
    expect_gold("after selling", 123)
    click(IDENTIFY_PANEL, STORE_REGION, "the Identify panel")
    check("store-identify", STORE_REGION)
    act(game, lambda: game.click(*STORE_DONE), "Done on the store", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=2.0)
    # The dialogue centred the view on Deirdre: back to the view on entering
    # (clamped at the map's left edge).
    jump((300, 500), AR1102_MAP)
    go(DEPOT_EXIT, "click on the exit of the depot")

    # --- Lord Ulbrec ------------------------------------------------------------------
    here, real = walk(TO_HALL, OUTSIDE_DEPOT, AR1100_MAP)
    enter(HALL_DOOR, HALL_DOORWAY, real, "click on the doorway of the Town Hall")
    talk(ULBREC, "Ulbrec")
    dialogue(ULBREC_STEPS, "Ulbrec", "ulbrec")
    act(game, lambda: game.press("2"), "last reply to Ulbrec", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=2.0)
    act(game, lambda: game.press("j"), "J (journal)", ACTION_BAR_REGION, park=FIGHT_PARK)
    page = game.wait_stable(settle=1.0, region=JOURNAL_TEXT)
    check_frame("journal-ulbrec", s18.journal_text(page), JOURNAL_TEXT, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the journal", ACTION_BAR_REGION, park=FIGHT_PARK)

    # --- Quick save for S20 ------------------------------------------------------
    s17.quick_save(game, out_dir, check)
    if target == "original":
        (out_dir / "bank-ok").write_text("ok")

    print(f"S19 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
