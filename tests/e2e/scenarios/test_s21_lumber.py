"""S21: Olap's lumber, the first errand Shawford gives on the Palisade, done
the fast way: Olap (AR1200, north-east wall), Swift Thomas (bluffed into
guiding the party), Lumbar in town (AR1100), back to Olap, Thomas to the
docks, Jorun (AR1000) gives the crane wheel, the log stairs up to the town,
the wheel put on the crane (AR1100), back through the Palisade gate to
Shawford (AR1201). Starts from S20's banked save and banks one of its own end
state.

Saves (golden/bank/, git-ignored like all goldens):

  s20-end/  S20's end (see test_s20_palisade), loaded here: on the Palisade
            (AR1200) outside Shawford's house, Lumber_Quest = 1, 120 gold
            plus a 250-gold pile in Sheris's backpack, whole party selected
  s21-end/  this test's end (on the original only): in Shawford's house
            (AR1201) after he has paid for the lumber (Lumber_Quest = 4,
            Crane_Wheel = 2, Arrow_Quest = 1: find Isherwood), 120 gold,
            whole party selected

Why there is no level up here (it was the aim of S21):

  Quest experience is shared: AddXpVar(entry) looks the entry up in
  EXPTABLE.2DA and AddPartyXP divides it among the living party members
  (S18's docks award, Level_2_Very_Hard = 1350, raised everyone by 225).
  The lumber errand pays Level_1_Easy (300) at the crane and Level_2_Average
  (600) from Shawford: 150 each. The party leaves with 750 (Sheris), 715,
  715, 714, 5713 (Vraedna, effective level 3: 6000 for her next level) and
  713 (Kylie): 250-287 short of a level each, so 1722 more party experience
  levels all six. Nothing else is open before the lumber is done (Isherwood's
  arrows, Koluhm and the Iron Collar all start from Shawford's later lines),
  and the lumber alone takes about twelve minutes of play.

  For S22 (the level up), from this save: Shawford has just sent the party to
  Isherwood (west wall of AR1200). Isherwood (Arrow_Quest 1 -> 2), Deirdre
  at the depot (any of her four replies that send the arrows: Level_2_Average
  600, Arrow_Quest 3), Isherwood again (S8 -> S9, the Diplomacy reply: 600,
  Arrow_Quest 4; Sheris has Diplomacy 9), Shawford (600): 1800, 300 each,
  enough for all six (Kylie and Vraedna by 13). Swift Thomas moves the party
  between the Palisade, the town and the docks.

test_s21_lumber (both targets) loads the s20-end save the way S20 loads
S19's, then, with the whole party selected:

  olap        north up the Palisade road to the wall breach; click Olap (found
              by his cyan circle, not the two soldiers'): his first line and
              reply (text only, as S19), replies 1, 1, 2, 1 (each panel; the
              first shows "Your journal has been updated."), 1 ends it
  thomas      back south to Swift Thomas outside Shawford's house: first line
              (text only), replies 1, 1 (Sheris's Bluff reply), 2 (into town):
              a fade, the loading screen (not compared), AR1100 on the bridge
  lumbar      west along the cliff to Lumbar Grundwall (he wanders: found by
              his circle): first line (text only), replies 1, 1, 2, 1 ends it
  thomas      back to Thomas on the bridge: "Take you somewhere?" (text
              only), 2: to the Palisade
  olap again  north to Olap: "Ye find Lumbar yet?" (text only), replies 1,
              2, 2 (each panel; the second shows the journal line), 2 ends it
  thomas      south to Thomas: "I *knew* I'd see you again" (text only),
              replies 1, 1: to the docks (AR1000)
  jorun       south-east to Jorun: first line (text only), replies 1, 1, 1, 1
              (the last panel shows "Your journal has been updated." and "The
              party has gained an item."), 1 ends it
  crane       west and north to the log stairs, up into AR1100 (loading not
              compared), east to the crane: click its end: the container
              panel (Sheris's items with the wheel); click the wheel: it goes
              on the crane (panel compared). Esc: the award lines "You have
              repaired the crane.  Good job." (pale green), "The party has
              gained experience.", "Your journal has been updated." (only
              their pixels, moved to a fixed spot: Lumbar's overhead lines
              come and go around them)
  shawford    north through the town and the open Palisade gate to AR1200,
              Shawford's door and doorway. Click him: "Have you spoken to
              Olap yet?" (text only), 1 (journal line, "You have carried out
              Shawford's orders.", "The party has gained experience."),
              Continue, Continue (each panel), 2 ends it
  record      the Record dial button: each of the six records above the
              portraits (2%) and its Experience block at 0.4% (750 / 715 /
              715 / 714 / 5713 / 713, Next Level 1000 or 6000); Sheris's Level
              Up button (still disabled); Esc
  journal     J: the page's text without the yellow day/hour headings
  save        Q: compare the save lines; on the original the save is banked

Not compared: the world view (snow, people walking, the clock), the loading
screens and their hints, the overhead lines of Thomas, Lumbar, Olap, the drill
sergeant, trainees and soldiers (they print into the message window at random
times), the first panel of each dialogue except its speaker's paragraph and
the replies.

Map points (area maps at 1/8 scale): AR1000 origin (138,73), AR1100 origin
(158,85), AR1200 origin (99,40). AR1200: Olap wanders round 2560-2670,
480-500 by the breach; the soldiers stand at 2540,591 and 2778,568 (cyan
too); Swift Thomas 2565,2390; Thomas's Palisade landing 2500-2630,2340-2475.
AR1100: Thomas's town landing 2035-2176,1837-1886, Thomas himself 2228,1872 on
the bridge; Lumbar wanders round 1060-1220,1655-1670; the crane's container
AR1100_Wheel (script 11DCRAN0) 978-1025,1825-1871, clickable at 986,1853;
arrival from the docks FR1000 249,1376; the gate To1200 1574-1682,170-275
(open since S20). AR1000: Thomas's docks landing 334-464,871-1007; Jorun
641,1461; the stairs exit To1100 79-224,1-101. Thomas goes with the party on
each trip but was not found on the docks after Jorun, so the party walks back
up the stairs.
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
import test_s19_town_errands as s19
import test_s20_palisade as s20
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s8_screens_tour import PORTRAIT
from test_s9_docks import AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, act
from test_s14_travel import select_all, travel

S20_END = s20.S20_END
S21_END = s17.BANK / "s21-end"

FIGHT_PARK = s17.FIGHT_PARK  # (520, 515): off the world view
SCREEN_PARK = (300, 560)
MESSAGES = s17.MESSAGES
JOURNAL_TEXT = s18.JOURNAL_TEXT

AR1000_MAP = (138, 73)
AR1100_MAP = (158, 85)
AR1200_MAP = (99, 40)

RECORD = s20.RECORD
NAMES = s20.NAMES
RECORD_SCREEN = s20.RECORD_SCREEN
RECORD_TEXT = s20.RECORD_TEXT
XP_BLOCK = s20.XP_BLOCK
LEVEL_UP = s20.LEVEL_UP

# Walks: (map point the view is centred on, map point walked to).
ROAD_NORTH = [((2650, y - 40), (2650, y)) for y in range(2350, 500, -230)] + [((2650, 560), (2640, 560))]
ROAD_SOUTH = [((2650, y + 40), (2650, y)) for y in range(780, 2400, 230)] + [((2600, 2440), (2640, 2440))]
OLAP_VIEW = (2650, 560)
THOMAS_VIEW = (2600, 2440)
BRIDGE_TO_LUMBAR = [
    ((1900, 1900), (1850, 1940)),  # west along the cliff edge
    ((1650, 1900), (1650, 1940)),
    ((1500, 1900), (1496, 1930)),  # in front of the depot
    ((1300, 1760), (1260, 1760)),  # the foot of the log ramp
]
LUMBAR_TO_BRIDGE = [
    ((1500, 1900), (1496, 1930)),
    ((1650, 1900), (1650, 1940)),
    ((1850, 1900), (1850, 1940)),
    ((2100, 1840), (2120, 1880)),  # the west end of the bridge
]
TOWN_LANDING = (2100, 1860)
DOCKS_LANDING = (420, 960)
DOCKS_TO_JORUN = [((330, 1150), (290, 1180)), ((500, 1330), (500, 1380))]
JORUN_TO_STAIRS = [((330, 1150), (290, 1180)), ((330, 760), (330, 760)), ((266, 575), (266, 575))]
MAP_STAIRS = s19.MAP_STAIRS  # the area-map click that shows the foot of the stairs
STAIRS_FOOT = s19.STAIRS_FOOT
STAIRS_EXIT = s19.STAIRS_EXIT
TOWN_ARRIVAL = s19.TOWN_ARRIVAL
TO_CRANE = [
    ((344, 1560), (344, 1560)),  # south, west of the inn
    ((400, 1760), (400, 1790)),
    ((650, 1790), (650, 1794)),  # east along the cliff
    ((900, 1794), (900, 1794)),  # beside the crane
]
CRANE_SPOT = (986, 1853)  # the end of the crane (AR1100_Wheel)
CRANE_TO_GATE = [((x, y - 40), (x, y)) for x, y in [
    (1030, 1760), (1200, 1698), (1288, 1560), (1400, 1434), (1480, 1250), (1500, 1030),
    (1560, 930), (1610, 760), (1620, 560), (1620, 380)]]
GATE_VIEW = s20.GATE_VIEW
GATEWAY = s20.GATEWAY
HOUSE_VIEW = s20.HOUSE_VIEW
HOUSE_DOOR = s20.HOUSE_DOOR
HOUSE_DOORWAY = s20.HOUSE_DOORWAY

# People (map points; the ones who wander are looked for round these).
OLAP = (2610, 495)
SOLDIERS = [(2540, 591), (2778, 568)]
THOMAS = (2565, 2390)
THOMAS_TOWN = (2228, 1872)
LUMBAR = (1150, 1662)
JORUN = (643, 1471)
SHAWFORD = s20.SHAWFORD  # screen, AR1201 as on entering

# The container panel (bottom of the screen) and the wheel in it.
CONTAINER_PANEL = (0, 476, 800, 124)
WHEEL_ITEM = (570, 557)
AWARD_SPOT = (8, 540, 535, 42)  # where the three crane lines are compared

OLAP_STEPS = ["1", "1", "2", "1"]
THOMAS_STEPS = ["1", "1"]
LUMBAR_STEPS = ["1", "1", "2"]
OLAP2_STEPS = ["1", "2", "2"]
JORUN_STEPS = ["1", "1", "1", "1"]
SHAWFORD_STEPS = ["1", CONTINUE, CONTINUE]


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s20_save(play_dir, out_dir, target):
    """Put the banked S20 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S20_END / "save").iterdir() if p.is_dir()] if (S20_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S20 save in {S20_END}; run test_s20_palisade --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    if target == "original":
        S21_END.mkdir(parents=True, exist_ok=True)
    yield
    s17.finish_saves(play_dir, out_dir, S21_END if target == "original" else None)


# --- finding things on screen --------------------------------------------------


def cyan_circles(img):
    """Centres of the cyan selection circles of neutral creatures in the
    world view, one per creature (S16's cyan_near averages everything in a
    box, which mixes up people standing close together)."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[:430]
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    ys, xs = np.nonzero((r > 60) & (r < 100) & (g > 240) & (b > 240))
    pts = list(zip(xs.tolist(), ys.tolist()))
    out = []
    while pts:
        sx, sy = pts[0]
        for _ in range(3):
            near = [(x, y) for x, y in pts if abs(x - sx) < 35 and abs(y - sy) < 18]
            sx, sy = np.mean([p[0] for p in near]), np.mean([p[1] for p in near])
        near = set((x, y) for x, y in pts if abs(x - sx) < 35 and abs(y - sy) < 18) | {pts[0]}
        if len(near) >= 8:
            out.append((int(sx), int(sy)))
        pts = [p for p in pts if p not in near]
    return out


def world_mean(img):
    return float(np.asarray(img.convert("L"))[:430].mean())


def award_block(img):
    """The crane's three lines (pale green "You have repaired the crane.",
    then two yellow lines), moved to AWARD_SPOT on a black screen with only
    those colours kept, or None."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y0:y0 + h0, x0:x0 + w0]
    pale = (a[..., 1] > 200) & (a[..., 1] - a[..., 0] > 25) & (a[..., 1] - a[..., 2] > 25) \
        & (a[..., 2] > 100)
    rows = np.nonzero(pale.sum(axis=1) > 3)[0]
    if not len(rows):
        return None
    top = y0 + int(rows[0]) - 2
    x, w, h = AWARD_SPOT[0], AWARD_SPOT[2], AWARD_SPOT[3]
    if top + h > y0 + h0:
        return None
    blk = np.asarray(img.convert("RGB"), dtype=np.int16)[top:top + h, x:x + w]
    # The pale green line, anti-aliased edges included (not white text).
    p = (blk[..., 1] > 120) & (blk[..., 1] - blk[..., 0] > 15) & (blk[..., 1] - blk[..., 2] > 15) \
        & (blk[..., 2] > 60)
    yel = (blk[..., 0] > 200) & (blk[..., 1] > 200) & (blk[..., 2] < 60)
    if yel.sum() < 100:
        return None  # the yellow lines are not printed yet
    blk = np.where((p | yel)[..., None], blk, 0).astype(np.uint8)
    out = Image.new("RGB", img.size)
    out.paste(Image.fromarray(blk), (x, AWARD_SPOT[1]))
    return out


# --- the test ------------------------------------------------------------------


def test_s21_lumber(from_s20_save, game, golden_dir, out_dir, target):
    group_bar = s17.load_bank(game, S20_END)
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

    def walk(route, here, origin, final=4.0):
        """Centre the view, wait until the spot is out of the black, click it;
        the next step is ordered while the party is still on its way (it
        walks about 45 px a second; a jump takes about 2.5 s)."""
        real = None
        for centre, point in route:
            real = jump(centre, origin)
            spot = s16.to_screen(real, point)
            deadline = time.time() + 10.0
            while s16.unexplored(game.grab(), spot) and time.time() < deadline:
                time.sleep(0.5)
            game.click(*spot)
            game.move(*FIGHT_PARK)
            time.sleep(max(0.5, np.hypot(point[0] - here[0], point[1] - here[1]) / 45 - 2.5))
            here = point
        time.sleep(final)
        return here, real

    def go(spot, what):
        """An exit or a doorway with the whole party selected: the loading
        screen (not compared), then the next area."""
        travel(game, group_bar, spot, what, "loading", lambda name, region, **kw: None, [])

    def talk(who, real, near, avoid=(), dx=160, dy=110):
        """Click the cyan circle nearest the map point `near` (and not one of
        the `avoid` people), wait for the first line and its replies."""
        want = s16.to_screen(real, near)
        shun = [s16.to_screen(real, p) for p in avoid]

        def find():
            best = None
            for c in cyan_circles(game.grab()):
                if any(abs(c[0] - s[0]) < 30 and abs(c[1] - s[1]) < 20 for s in shun):
                    continue
                if abs(c[0] - want[0]) > dx or abs(c[1] - want[1]) > dy:
                    continue
                d = np.hypot(c[0] - want[0], c[1] - want[1])
                if best is None or d < best[0]:
                    best = (d, c)
            return best and best[1]

        deadline = time.time() + 20.0
        while (spot := find()) is None:
            if time.time() > deadline:
                raise AssertionError(f"{who}'s circle did not show near {want}")
            time.sleep(0.5)
        for attempt in range(3):
            spot = find() or spot
            try:
                act(game, lambda: game.click(*spot), f"click on {who}", ACTION_BAR_REGION,
                    park=DIALOG_PARK, wait=1.0, timeout=20.0)
                break
            except AssertionError:
                if attempt == 2:
                    raise
        deadline = time.time() + 20.0
        while not s20.replies_shown(game.grab()):
            if time.time() > deadline:
                raise AssertionError(f"{who}'s first line and replies did not show")
            time.sleep(0.3)

    def first_panel(name):
        check_frame(name, s19.speech_block(game.wait_stable(settle=1.0, region=DIALOG_REGION)),
                    s19.SPEECH_SPOT)

    def dialogue(who, steps, prefix):
        for i, stp in enumerate(steps, start=2):
            if isinstance(stp, str):
                act(game, lambda: game.press(stp), f"reply {stp} to {who}", DIALOG_REGION,
                    park=DIALOG_PARK)
            else:
                act(game, lambda: game.click(*CONTINUE), f"Continue with {who}", DIALOG_REGION,
                    park=DIALOG_PARK)
            check(f"{prefix}-{i:02d}", DIALOG_REGION)

    def end(key, who):
        act(game, lambda: game.press(key), f"last reply to {who}", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=2.0)

    def ride(key, what):
        """The reply that has Thomas run the party to another area: a fade,
        the loading screen, then the new area (the group bar again)."""
        act(game, lambda: game.press(key), what, DIALOG_REGION, park=FIGHT_PARK, wait=0.0,
            threshold=0.05)
        # The world fades to black first (the dialogue panel is gone by
        # then and the old area's action bar is back for a moment).
        deadline = time.time() + 20.0
        while world_mean(game.grab()) > 12:
            if time.time() > deadline:
                raise AssertionError(f"{what}: the world did not fade out")
            time.sleep(0.1)
        deadline = time.time() + 60.0
        while True:
            cur = game.grab()
            if diff_ratio(group_bar, cur, region=ACTION_BAR_REGION) <= 0.05 and world_mean(cur) > 25:
                break
            if time.time() > deadline:
                raise AssertionError(f"{what}: the new area never showed")
            time.sleep(0.3)
        time.sleep(3.0)
        select_all(game, group_bar)

    def click(xy, region, what, park=SCREEN_PARK, **kw):
        act(game, lambda: game.click(*xy), what, region, park=park, **kw)

    # --- Olap ------------------------------------------------------------------------
    walk(ROAD_NORTH, (2632, 2514), AR1200_MAP)
    real = jump(OLAP_VIEW, AR1200_MAP)
    talk("Olap", real, OLAP, avoid=SOLDIERS)
    first_panel("olap-01")
    dialogue("Olap", OLAP_STEPS, "olap")
    end("1", "Olap")

    # --- Swift Thomas into town ----------------------------------------------------
    walk(ROAD_SOUTH, (2640, 560), AR1200_MAP)
    real = jump(THOMAS_VIEW, AR1200_MAP)
    talk("Thomas", real, THOMAS, dx=80, dy=60)
    first_panel("thomas-01")
    dialogue("Thomas", THOMAS_STEPS, "thomas")
    ride("2", "reply 2 to Thomas (into town)")

    # --- Lumbar --------------------------------------------------------------------------
    here, real = walk(BRIDGE_TO_LUMBAR, TOWN_LANDING, AR1100_MAP)
    talk("Lumbar", real, LUMBAR, dx=380, dy=200)
    first_panel("lumbar-01")
    dialogue("Lumbar", LUMBAR_STEPS, "lumbar")
    end("1", "Lumbar")

    # --- Thomas back to the Palisade ----------------------------------------------------
    here, real = walk(LUMBAR_TO_BRIDGE, (1240, 1700), AR1100_MAP)
    talk("Thomas", real, THOMAS_TOWN, dx=150, dy=100)
    first_panel("thomas-town-01")
    ride("2", "reply 2 to Thomas (to the Palisade)")

    # --- Olap again ------------------------------------------------------------------------
    walk(ROAD_NORTH[1:], (2560, 2400), AR1200_MAP)
    real = jump(OLAP_VIEW, AR1200_MAP)
    talk("Olap", real, OLAP, avoid=SOLDIERS)
    first_panel("olap2-01")
    dialogue("Olap", OLAP2_STEPS, "olap2")
    end("2", "Olap")

    # --- Thomas to the docks -------------------------------------------------------------
    walk(ROAD_SOUTH, (2640, 560), AR1200_MAP)
    real = jump(THOMAS_VIEW, AR1200_MAP)
    talk("Thomas", real, THOMAS, dx=80, dy=60)
    first_panel("thomas2-01")
    act(game, lambda: game.press("1"), "reply 1 to Thomas", DIALOG_REGION, park=DIALOG_PARK)
    check("thomas2-02", DIALOG_REGION)
    ride("1", "reply 1 to Thomas (to the docks)")

    # --- Jorun -------------------------------------------------------------------------------
    here, real = walk(DOCKS_TO_JORUN, DOCKS_LANDING, AR1000_MAP)
    talk("Jorun", real, JORUN, dx=120, dy=90)
    first_panel("jorun-01")
    dialogue("Jorun", JORUN_STEPS, "jorun")
    end("1", "Jorun")

    # --- Up the stairs to the crane ------------------------------------------------------
    walk(JORUN_TO_STAIRS, JORUN, AR1000_MAP, final=2.0)
    area_map(MAP_STAIRS)
    game.click(*STAIRS_FOOT)
    game.move(*FIGHT_PARK)
    time.sleep(8.0)
    go(STAIRS_EXIT, "click on the exit at the top of the stairs")
    here, real = walk(TO_CRANE, TOWN_ARRIVAL, AR1100_MAP, final=2.0)
    click(s16.to_screen(real, CRANE_SPOT), ACTION_BAR_REGION, "click on the end of the crane",
          park=(300, 300), threshold=0.3, timeout=20.0)
    check("crane-panel", CONTAINER_PANEL)
    # Only the wheel's icon and the weight change: under 0.5% of the panel.
    click(WHEEL_ITEM, CONTAINER_PANEL, "the crane wheel in Sheris's items", park=(300, 300),
          threshold=0.001)
    check("crane-wheel-placed", CONTAINER_PANEL)
    act(game, lambda: game.press("esc"), "Esc on the container panel", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=0.0, threshold=0.3)
    line = None
    deadline = time.time() + 5.0
    while line is None and time.time() < deadline:
        line = award_block(game.grab())
        time.sleep(0.2)
    check_frame("crane-award", line, AWARD_SPOT, max_diff=0.004)

    # --- Back to Shawford --------------------------------------------------------------
    select_all(game, group_bar)
    walk(CRANE_TO_GATE, (900, 1794), AR1100_MAP, final=2.0)
    real = jump(GATE_VIEW, AR1100_MAP)
    go(s16.to_screen(real, GATEWAY), "click on the Palisade gateway")
    real = jump(HOUSE_VIEW, AR1200_MAP)
    # Shawford sometimes starts his dialogue himself as soon as the party is
    # in (the loading screen is followed by his panel, not the world screen).
    select_all(game, group_bar)
    game.click(*s16.to_screen(real, HOUSE_DOOR))
    game.move(*FIGHT_PARK)
    time.sleep(7.0)
    in_house = None
    for attempt in range(2):
        try:
            act(game, lambda: game.click(*s16.to_screen(real, HOUSE_DOORWAY)),
                "click on the doorway of Shawford's house", ACTION_BAR_REGION, park=FIGHT_PARK,
                threshold=0.5, wait=0.0, timeout=25.0)
        except AssertionError:
            # Once (S20) the party was still on its way and the click did
            # nothing: the door was open a moment later.
            if attempt:
                raise
            time.sleep(3.0)
            continue
        deadline = time.time() + 60.0
        while time.time() < deadline:
            cur = game.grab()
            if s20.replies_shown(cur):
                in_house = "dialogue"
                break
            if diff_ratio(group_bar, cur, region=ACTION_BAR_REGION) <= 0.05 and world_mean(cur) < 40:
                in_house = "world"
                break
            time.sleep(0.3)
        if in_house:
            break
    if in_house is None:
        raise AssertionError("the party did not get into Shawford's house")
    time.sleep(3.0)
    if in_house == "world" and not s20.replies_shown(game.grab()):
        deadline = time.time() + 20.0
        while (spot := s16.cyan_near(game.grab(), SHAWFORD, dx=30, dy=15)) is None:
            if s20.replies_shown(game.grab()):
                break
            if time.time() > deadline:
                raise AssertionError("Shawford's circle did not show")
            time.sleep(0.5)
        if spot is not None and not s20.replies_shown(game.grab()):
            act(game, lambda: game.click(*spot), "click on Shawford", ACTION_BAR_REGION,
                park=DIALOG_PARK, wait=1.0, timeout=20.0)
    deadline = time.time() + 20.0
    while not s20.replies_shown(game.grab()):
        if time.time() > deadline:
            raise AssertionError("Shawford's first line and reply did not show")
        time.sleep(0.3)
    first_panel("shawford-01")
    dialogue("Shawford", SHAWFORD_STEPS, "shawford")
    end("2", "Shawford")
    select_all(game, group_bar)

    # --- The record: experience after the errand ----------------------------------------
    click(RECORD, ACTION_BAR_REGION, "Record dial button", threshold=0.3)
    for i, name in enumerate(NAMES):
        if i:
            click(PORTRAIT[i], RECORD_TEXT, f"{name}'s portrait on the record")
        check(f"record-{name}", RECORD_SCREEN)
        tight(f"record-{name}-xp", XP_BLOCK[name])
        if i == 0:
            tight("record-level-up", LEVEL_UP)
    act(game, lambda: game.press("esc"), "Esc on the record", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)
    select_all(game, group_bar)

    # --- The journal ------------------------------------------------------------------
    act(game, lambda: game.press("j"), "J (journal)", ACTION_BAR_REGION, park=FIGHT_PARK)
    page = game.wait_stable(settle=1.0, region=JOURNAL_TEXT)
    check_frame("journal-lumber", s18.journal_text(page), JOURNAL_TEXT, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the journal", ACTION_BAR_REGION, park=FIGHT_PARK)

    # --- Quick save for S22 ------------------------------------------------------------
    s17.quick_save(game, out_dir, check)
    if target == "original":
        (out_dir / "bank-ok").write_text("ok")

    print(f"S21 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
