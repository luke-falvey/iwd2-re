"""S22: the first level up. Isherwood's arrows errand (Isherwood on AR1200's
west wall, Swift Thomas into town, Deirdre at the depot, Thomas back, Shawford)
and Niles on the Palisade bring every party member past the next level; then
all six are levelled on the record screen and each Level Up panel is
compared. Starts from S21's banked save and banks one of its own end state.

Saves (golden/bank/, git-ignored like all goldens):

  s21-end/  S21's end (see test_s21_lumber), loaded here: in Shawford's house
            (AR1201) after the lumber, Arrow_Quest = 1, 120 gold, experience
            750 / 715 / 715 / 714 / 5713 / 713, whole party selected
  s22-end/  this test's end (on the original only): on the Palisade (AR1200)
            east of Shawford's house where Niles stood, Arrow_Quest = 3,
            Dead_Goblin_Quest = 1 (find Koluhm in town), all six at character
            level 2 (Vraedna: effective level 4), experience 1050 / 1015 /
            1015 / 1014 / 6013 / 1013, 120 gold, whole party selected

Where the experience comes from (quest experience is AddXpVar(EXPTABLE
entry) divided among the six; see S21):

  Deirdre's arrows    Level_2_Average 600 (11DEIRD: the Intimidate reply,
                      Arrow_Quest 3)
  Shawford            Level_2_Average 600 (12SHAWFO S16, Arrow_Quest 3)
  Niles               Level_2_Average 600 (12NILES: tell him off, then the
                      "report to your post" reply)

  1800 = 300 each. S21's plan (Isherwood's Diplomacy reply after Deirdre) does
  not work: that reply (12ISHER S13) needs 300 arrows in the party's packs and
  is the other way to finish the errand (Arrow_Quest 4) instead of Deirdre.
  After Deirdre, Isherwood only thanks the party with burning oil, no
  experience, so he is not visited a second time. Niles (a drunk sell-sword
  east of Shawford's house) makes up the missing 600.

test_s22_level_up (both targets) loads the s21-end save the way S21 loads
S20's, then, with the whole party selected:

  out         leave Shawford's house (the loading screen is not compared)
  Isherwood   west along the road north of the long building (y about 2150)
              to the west wall; click Isherwood (not the archers): his first
              line and replies (text only, as S19), 1 ("Shawford sent us"),
              1 ("Deirdre isn't going to like that number"), each panel; 1
              ends it
  Thomas      back east to Swift Thomas by Shawford's house: "Take you
              somewhere?" (text only), 3 ("into town"): fade, loading, AR1100
              on the bridge
  Deirdre     west along the cliff to the depot door; click the door (the
              party opens it and stands in the doorway), walk back a few
              steps (a doorway click on a party member only selects it), the
              doorway: AR1102. Walk in, click Deirdre: first line and replies
              (text only), 1 (requisition), 1 (Lord of Targos), 1 (Intimidate),
              1 ("You will lose more..."; the panel shows "You have struck a
              deal with Deirdre Gallaway...", "The party has gained
              experience."), each panel; 2 ends it; leave
  Thomas      east to Thomas on the bridge: "Take you somewhere?" (text only),
              2: to the Palisade (lands by Shawford's house)
  Shawford    door, doorway (he may start talking himself); "Were you able to
              get Isherwood the arrows" (text only), 1 (experience), Continue,
              1, Continue, 1 ("Where can I find him?"), each panel; 1 ends it;
              leave the house
  Niles       north-east round the house to Niles: first line (text only), 1,
              1, 1 (each panel; the last shows "You helped maintain
              discipline..." and "The party has gained experience."), 1 ends
              it
  level up    the Record dial button. For each of the six (portrait clicks):
              the record (2%) and its Experience block (0.4%: Current, Next
              Level 1000, Vraedna's 6000); the Level Up button (enabled); Level
              Up: the Class Selection panel; the class (Bard, Fighter,
              Barbarian, Cleric, Sorcerer, Rogue) clicked: its description and
              "2" in its Level box; Next: Skill Points Left; one + per point
              (compare the spent panel); Next. Sheris (bard): Arcane Spell
              Selection: Level 1 (choose 2), the first two icons (Sleep...
              Chromatic Orb shown), Next. Geldin (fighter 2): the Feats panel
              (1 feat), Blind-Fight +, Next. Then the record again (Character
              Level 2, Next Level 3000 / 10000, the new hit points) and its
              Experience block; the Level Up button disabled. Sheris's,
              Geldin's and Vraedna's panels are compared one by one; Khatja's,
              Laeris's and Kylie's only the class and the spent skills panels
              and the record after
  portraits   Esc: the six portraits on the world screen (hit points, no
              level-up marks; Vraedna's Day Blindness icon masked)
  save        Q: compare the save lines; on the original the save is banked

Hit points: the original gives the full hit die plus the Constitution bonus
on this level up (6 / +13 / +14 / +9 / +6 / +7: bard d6, fighter d10 +3,
barbarian d12 +2, cleric d8 +1, sorcerer d4 +2, rogue d6 +1), in every run
seen. So they are not masked; if a run ever differs there, it is a rolled
number after all. The portrait strip is outside every level-up checkpoint
anyway (the selected portrait shows the new current hit points over the old
maximum while the panels are open).

Not compared: the world view (snow, people walking, the clock), the loading
screens and their hints, the overhead lines of Thomas, the drill sergeants,
the weapon instructor and the trainees (they print into the message window at
random times), the first panel of each dialogue except its speaker's paragraph
and the replies.

Map points (area maps at 1/8 scale): AR1100 origin (158,85), AR1200 origin
(99,40), AR1102 origin (278,168). AR1200: arrival outside Shawford's house
FR1201 2535,2540; Isherwood 650,2103 with three archers at 445,2178,
474,1982, 495,1810; Thomas 2565,2390; Niles 3272,2291; the long building
south of the road covers about 1440-2110, 2190-2600 (AR1200SR value 8 and 0:
a click there is not walkable; S22's first try walked into its east wall).
Shawford's house covers 2656-3300, 2240-2650; the way round its north side
to Niles follows the SR map's diagonal edge (2640,2440 - 2760,2340 -
2950,2270 - 3150,2220). AR1100: the depot door DOOR3 1522-1571,1737-1802
(To1102 1523-1570,1739-1800).
"""

from __future__ import annotations

import shutil
import time

import numpy as np
import pytest

import test_s16_clear_docks as s16
import test_s17_dock_houses as s17
import test_s19_town_errands as s19
import test_s20_palisade as s20
import test_s21_lumber as s21
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s8_screens_tour import PORTRAIT
from test_s9_docks import AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, act
from test_s14_travel import select_all, travel

S21_END = s21.S21_END
S22_END = s17.BANK / "s22-end"

FIGHT_PARK = s17.FIGHT_PARK  # (520, 515): off the world view
SCREEN_PARK = (300, 560)
PORTRAITS = (500, 435, 300, 55)
VRAEDNA_ICON = s16.VRAEDNA_ICON

AR1100_MAP = (158, 85)
AR1102_MAP = (278, 168)
AR1200_MAP = (99, 40)

RECORD = s20.RECORD
NAMES = s20.NAMES
RECORD_SCREEN = s20.RECORD_SCREEN
RECORD_TEXT = s20.RECORD_TEXT
XP_BLOCK = s20.XP_BLOCK
LEVEL_UP = s20.LEVEL_UP

# Level Up panels (all above the portraits).
PANEL = (0, 0, 800, 430)
LEVEL_UP_BUTTON = (714, 392)
NEXT = (721, 394)
CLASS_X = 369
CLASS_Y = {"barbarian": 90, "bard": 120, "cleric": 150, "fighter": 210, "rogue": 330, "sorcerer": 360}
PLUS_X = 477
SKILL_Y = {"alchemy": 94, "bluff": 168, "concentration": 207, "diplomacy": 244,
           "disable": 281, "hide": 321, "intimidate": 358, "arcana": 395}
SKILL_LIST = (250, 20, 280, 395)  # the list and Skill Points Left
SPELL_ICONS = [(278, 94), (321, 94)]
FEAT_BLIND_FIGHT = (PLUS_X, 94)

# What each one gets: class, skill + clicks, extra panels.
LEVELS = {
    "sheris": ("bard", ["alchemy", "bluff", "concentration", "diplomacy", "hide"], "spells"),
    "geldin": ("fighter", ["intimidate"], "feat"),
    "khatja": ("barbarian", ["alchemy"], None),  # Alchemy costs 2: both points
    "laeris": ("cleric", ["concentration", "diplomacy", "arcana"], None),
    "vraedna": ("sorcerer", ["alchemy", "bluff", "concentration", "arcana"], None),
    "kylie": ("rogue", ["bluff", "diplomacy", "disable", "hide", "intimidate"], None),
}
DETAILED = {"sheris", "geldin", "vraedna"}  # every panel compared

# Walks: (map point the view is centred on, map point walked to).
TO_ISHERWOOD = [((x, y), (x, y)) for x, y in [
    (2400, 2460), (2250, 2320), (2150, 2200),  # round the long building's east end
    (1950, 2150), (1720, 2150), (1490, 2150), (1260, 2150), (1030, 2150), (800, 2150),
    (720, 2130)]]
FROM_ISHERWOOD = [((x, y), (x, y)) for x, y in [
    (950, 2150), (1180, 2150), (1410, 2150), (1640, 2150), (1870, 2150), (2100, 2180),
    (2300, 2300), (2480, 2420)]]
TO_NILES = [((x, y), (x, y)) for x, y in [
    (2640, 2440), (2760, 2340), (2950, 2270), (3150, 2220), (3200, 2250)]]
OUTSIDE_HOUSE = (2535, 2540)
ISHER_VIEW = (680, 2100)
ISHERWOOD = (650, 2103)
ARCHERS = [(445, 2178), (474, 1982), (495, 1810)]
THOMAS_VIEW = s21.THOMAS_VIEW
THOMAS = s21.THOMAS
THOMAS_TOWN = s21.THOMAS_TOWN
TOWN_LANDING = s21.TOWN_LANDING
BRIDGE_TO_DEPOT = s21.BRIDGE_TO_LUMBAR[:3]
DEPOT_TO_BRIDGE = [((1650, 1900), (1650, 1940)), ((1850, 1900), (1850, 1940)),
                   ((2100, 1840), (2120, 1880))]
DEPOT_STEP_BACK = (1430, 1960)  # off the doorway, so a doorway click is not a party member
DEPOT_DOORWAY = (1546, 1765)
NILES = (3272, 2291)
HOUSE_VIEW = s20.HOUSE_VIEW
HOUSE_DOOR = s20.HOUSE_DOOR
HOUSE_DOORWAY = s20.HOUSE_DOORWAY
HOUSE_EXIT = s20.HOUSE_EXIT
SHAWFORD = s20.SHAWFORD

ISHER_STEPS = ["1", "1"]
DEIRDRE_STEPS = ["1", "1", "1", "1"]
SHAWFORD_STEPS = ["1", CONTINUE, "1", CONTINUE, "1"]
NILES_STEPS = ["1", "1", "1"]


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s21_save(play_dir, out_dir, target):
    """Put the banked S21 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S21_END / "save").iterdir() if p.is_dir()] if (S21_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S21 save in {S21_END}; run test_s21_lumber --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    if target == "original":
        S22_END.mkdir(parents=True, exist_ok=True)
    yield
    s17.finish_saves(play_dir, out_dir, S22_END if target == "original" else None)


# --- the test ------------------------------------------------------------------


def test_s22_level_up(from_s21_save, game, golden_dir, out_dir, target):
    group_bar = s17.load_bank(game, S21_END)
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
        the next step is ordered while the party is still on its way (as S21)."""
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
        travel(game, group_bar, spot, what, "loading", lambda name, region, **kw: None, [])

    def wait_replies(who):
        deadline = time.time() + 20.0
        while not s20.replies_shown(game.grab()):
            if time.time() > deadline:
                raise AssertionError(f"{who}'s first line and replies did not show")
            time.sleep(0.3)

    def talk(who, real, near, avoid=(), dx=160, dy=110):
        """Click the cyan circle nearest the map point `near` (as S21)."""
        want = s16.to_screen(real, near)
        shun = [s16.to_screen(real, p) for p in avoid]

        def find():
            best = None
            for c in s21.cyan_circles(game.grab()):
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
        wait_replies(who)

    def talk_at(who, near, dx=30, dy=15):
        """A screen spot (interiors, as S20)."""
        deadline = time.time() + 20.0
        while (spot := s16.cyan_near(game.grab(), near, dx=dx, dy=dy)) is None:
            if time.time() > deadline:
                raise AssertionError(f"{who}'s circle did not show near {near}")
            time.sleep(0.5)
        for attempt in range(3):
            spot = s16.cyan_near(game.grab(), near, dx=dx, dy=dy) or spot
            try:
                act(game, lambda: game.click(*spot), f"click on {who}", ACTION_BAR_REGION,
                    park=DIALOG_PARK, wait=1.0, timeout=20.0)
                break
            except AssertionError:
                if attempt == 2:
                    raise
        wait_replies(who)

    def first_panel(name):
        # Shawford can start talking by himself while the pointer is still
        # parked over the panel's text (FIGHT_PARK): move it off first.
        game.move(*DIALOG_PARK)
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
        """Thomas runs the party to another area (as S21)."""
        act(game, lambda: game.press(key), what, DIALOG_REGION, park=FIGHT_PARK, wait=0.0,
            threshold=0.05)
        deadline = time.time() + 20.0
        while s21.world_mean(game.grab()) > 12:
            if time.time() > deadline:
                raise AssertionError(f"{what}: the world did not fade out")
            time.sleep(0.1)
        deadline = time.time() + 60.0
        while True:
            cur = game.grab()
            if diff_ratio(group_bar, cur, region=ACTION_BAR_REGION) <= 0.05 and s21.world_mean(cur) > 25:
                break
            if time.time() > deadline:
                raise AssertionError(f"{what}: the new area never showed")
            time.sleep(0.3)
        time.sleep(3.0)
        select_all(game, group_bar)

    def click(xy, region, what, park=SCREEN_PARK, **kw):
        act(game, lambda: game.click(*xy), what, region, park=park, **kw)

    # --- Out of Shawford's house, west to Isherwood ---------------------------------
    go(HOUSE_EXIT, "click on the exit of Shawford's house")
    walk(TO_ISHERWOOD, OUTSIDE_HOUSE, AR1200_MAP)
    real = jump(ISHER_VIEW, AR1200_MAP)
    talk("Isherwood", real, ISHERWOOD, avoid=ARCHERS, dx=100, dy=60)
    first_panel("isherwood-01")
    dialogue("Isherwood", ISHER_STEPS, "isherwood")
    end("1", "Isherwood")

    # --- Swift Thomas into town ----------------------------------------------------
    walk(FROM_ISHERWOOD, (720, 2130), AR1200_MAP)
    real = jump(THOMAS_VIEW, AR1200_MAP)
    talk("Thomas", real, THOMAS, dx=80, dy=60)
    first_panel("thomas-01")
    ride("3", "reply 3 to Thomas (into town)")

    # --- Deirdre ---------------------------------------------------------------------
    here, real = walk(BRIDGE_TO_DEPOT, TOWN_LANDING, AR1100_MAP, final=2.0)
    select_all(game, group_bar)
    game.click(*s16.to_screen(real, s19.DEPOT_DOOR))
    game.move(*FIGHT_PARK)
    time.sleep(7.0)
    # The party opened the door and stands in the doorway: a click there
    # would only select one of them.
    game.click(*s16.to_screen(real, DEPOT_STEP_BACK))
    game.move(*FIGHT_PARK)
    time.sleep(8.0)
    go(s16.to_screen(real, DEPOT_DOORWAY), "click on the doorway of the depot")
    game.click(*s19.DEPOT_WALK_IN)
    game.move(*FIGHT_PARK)
    time.sleep(6.0)
    talk_at("Deirdre", s19.DEIRDRE)
    first_panel("deirdre-01")
    dialogue("Deirdre", DEIRDRE_STEPS, "deirdre")
    end("2", "Deirdre")
    jump((300, 500), AR1102_MAP)
    go(s19.DEPOT_EXIT, "click on the exit of the depot")

    # --- Thomas back to the Palisade ----------------------------------------------------
    here, real = walk(DEPOT_TO_BRIDGE, s19.OUTSIDE_DEPOT, AR1100_MAP)
    talk("Thomas", real, THOMAS_TOWN, dx=150, dy=100)
    first_panel("thomas-town-01")
    ride("2", "reply 2 to Thomas (to the Palisade)")

    # --- Shawford ------------------------------------------------------------------------
    real = jump(HOUSE_VIEW, AR1200_MAP)
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
            if diff_ratio(group_bar, cur, region=ACTION_BAR_REGION) <= 0.05 and s21.world_mean(cur) < 40:
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
    wait_replies("Shawford")
    first_panel("shawford-01")
    dialogue("Shawford", SHAWFORD_STEPS, "shawford")
    end("1", "Shawford")
    go(HOUSE_EXIT, "click on the exit of Shawford's house")

    # --- Niles -------------------------------------------------------------------------
    here, real = walk(TO_NILES, OUTSIDE_HOUSE, AR1200_MAP)
    talk("Niles", real, NILES, dx=80, dy=60)
    first_panel("niles-01")
    dialogue("Niles", NILES_STEPS, "niles")
    end("1", "Niles")
    select_all(game, group_bar)
    print(f"S22 errand took {time.time() - t0:.0f} s")

    # --- Level up, all six ---------------------------------------------------------------
    click(RECORD, ACTION_BAR_REGION, "Record dial button", threshold=0.3)
    for i, name in enumerate(NAMES):
        cls, skills, extra = LEVELS[name]
        full = name in DETAILED
        if i:
            click(PORTRAIT[i], RECORD_TEXT, f"{name}'s portrait on the record")
        check(f"record-{name}", RECORD_SCREEN)
        tight(f"record-{name}-xp", XP_BLOCK[name])
        tight(f"record-{name}-level-up", LEVEL_UP)
        click(LEVEL_UP_BUTTON, PANEL, f"Level Up ({name})", threshold=0.05)
        if full:
            check(f"lu-{name}-classes", PANEL)
        click((CLASS_X, CLASS_Y[cls]), PANEL, f"{cls} ({name})", threshold=0.001)
        check(f"lu-{name}-class", PANEL)
        click(NEXT, PANEL, f"Next to the skills ({name})", threshold=0.02)
        if full:
            check(f"lu-{name}-skills", PANEL)
        for sk in skills:
            # The rank and the points left change, and the description.
            click((PLUS_X, SKILL_Y[sk]), SKILL_LIST, f"+ {sk} ({name})", threshold=0.0005, wait=1.0)
        check(f"lu-{name}-skills-spent", PANEL)
        if extra == "spells":
            click(NEXT, PANEL, f"Next to the spells ({name})", threshold=0.02)
            check(f"lu-{name}-spells", PANEL)
            for k, icon in enumerate(SPELL_ICONS):
                click(icon, PANEL, f"spell {k + 1} ({name})", threshold=0.001, wait=1.0)
            check(f"lu-{name}-spells-chosen", PANEL)
        elif extra == "feat":
            click(NEXT, PANEL, f"Next to the feats ({name})", threshold=0.02)
            check(f"lu-{name}-feats", PANEL)
            click(FEAT_BLIND_FIGHT, SKILL_LIST, f"+ Blind-Fight ({name})", threshold=0.0005, wait=1.0)
            check(f"lu-{name}-feats-chosen", PANEL)
        # The last Next closes the panels: the record again.
        click(NEXT, PANEL, f"last Next ({name})", threshold=0.02)
        check(f"record-{name}-after", RECORD_SCREEN)
        tight(f"record-{name}-after-xp", XP_BLOCK[name])
        tight(f"record-{name}-after-level-up", LEVEL_UP)
    act(game, lambda: game.press("esc"), "Esc on the record", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)
    select_all(game, group_bar)
    check("portraits-after", PORTRAITS, max_diff=0.004, mask=[VRAEDNA_ICON])

    # --- Quick save for S23 ------------------------------------------------------------
    s17.quick_save(game, out_dir, check)
    if target == "original":
        (out_dir / "bank-ok").write_text("ok")

    print(f"S22 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
