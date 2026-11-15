"""S23: the two errands before the Palisade battle. Koluhm in his morgue in
town (AR1107), back to Shawford (who sends the party after the Iron Collar
band), and Black Geoffrey's band in the Salty Dog tavern on the docks
(AR1003), talked into reporting for duty. Swift Thomas does the travel between
the Palisade, the town and the docks. Also Sheris's spellbook (her new bard
spell) and the skills page of her record, both from the level up in S22.
Starts from S22's banked save and banks one of its own end state.

Saves (golden/bank/, git-ignored like all goldens):

  s22-end/  S22's end (see test_s22_level_up), loaded here: on the Palisade
            (AR1200) east of Shawford's house, Dead_Goblin_Quest = 1, all six
            at level 2, experience 1050 / 1015 / 1015 / 1014 / 6013 / 1013,
            120 gold, whole party selected
  s23-end/  this test's end (on the original only): inside the Salty Dog
            (AR1003) after Black Geoffrey, Dead_Goblin_Quest = 6,
            Palisade_Iron_Collar_Quest = 2, Goblins_Attack_Palisade = 0,
            experience 1300 / 1265 / 1265 / 1264 / 6263 / 1263, 120 gold,
            whole party selected. Shawford has not been told yet: his next
            dialogue (12SHAWFO S32, "Any luck on bringing the Iron Collar to
            the Palisade?") has the reply "Yes - though they proved to be a
            stubborn bunch" (T65: Level_2_Hard, Palisade_Iron_Collar_Quest 3)
            leading to S3, whose only reply "Did you hear that?" starts the
            battle (Goblins_Attack_Palisade 1, cutscene 12cWar2).

Experience (AddXpVar entries are shared by the six, see S21): Koluhm's
"write down the words" reply (Level_2_Easy 450; Sheris is human with INT 14,
so the goblin-tongue replies are hidden), Shawford's report (Level_2_Easy
450), Black Geoffrey's band (Level_2_Average 600): 1500, 250 each.

Dialogue paths (reply keys; c = Continue):

  Koluhm      11KOLUHM S0 -> 1 S2 -> 2 S4 -> c (11DEADS) -> c S5 -> 1 (11DEADS)
              -> c S6 -> 1 S9 -> c (11DEADG) -> c S10 -> 1 S7 -> 1 S11 -> 1
              S13 -> c S14 -> c S15 -> 1 S16 -> c S17 -> 1 ("Shawford Crale
              has sent us") S41 -> 1 S42 -> 1 (T129, experience, journal,
              Dead_Goblin_Quest 2) S43 -> 1 S51 -> 1 ends it
  Shawford    12SHAWFO S22 "Did you speak to Koluhm?" -> 1 (T37, experience)
              S25 -> 1 ("No, I haven't heard of them") S27 -> c S28 -> 1 S29
              -> 1 (Palisade_Iron_Collar_Quest 1) S30 -> 1 S45 -> c ends it
  Geoffrey    10BLACK S1 (Sheris is female: S0 is for a male protagonist) -> c
              (10KICKSH S5) -> c S3 -> 1 S14 -> c (Kickshaw) -> c S15 -> c S16
              -> c (10BLANC S8) -> 2 (Diplomacy) S17 -> 2 (Diplomacy, Kickshaw
              S14) -> 2 (Diplomacy, Blanchard S9) -> c (Kickshaw S15) -> 2
              (Diplomacy, Kickshaw S16) -> c (Blanchard S10) -> c S19 -> c
              (Blanchard S11) -> c S20 -> c (experience, journal) S21 -> 1
              ends it (Palisade_Iron_Collar_Quest 2). Every other way through
              ends in a fight (Enemy()) or needs Phaen_Loses_It.

Swift Thomas's "Take you somewhere?" (12SWIFT S6) lists only the places the
party is not in, plus "when I last saw you at ..." when Jump_Check names
another area: from the s22-end save (Jump_Check 2) the keys are 2 (town) on
the Palisade, 2 (Palisade) on the bridge in town, 1 (docks) on the Palisade.
Thomas does not stand on the docks after a ride there (AR1000 has only a
hidden copy of him), so the party ends on the docks.

test_s23_iron_collar (both targets) loads the s22-end save the way S22 loads
S21's, then, with the whole party selected:

  spellbook   the Spellbook dial button: Sheris's spellbook, Bard level 1:
              Charm Person and Chromatic Orb (learnt in S22), 2/2 slots; Esc
  record      the Record dial button, the third tab (skills and feats):
              Sheris's skills after S22's ranks (Diplomacy 10 ...); back to
              the first tab (the record remembers its tab); Esc
  Thomas      round the north of Shawford's house to Thomas: "Take you
              somewhere?" (text only), 2: fade, loading, AR1100 on the bridge
  Koluhm      west and north through the town (a search-map path, 15 steps)
              to the morgue door north of the inn; the door, then the doorway
              (AR1107). Click Koluhm: his first line and reply (text only),
              the 20 panels above (each compared; the 19th shows "Your journal
              has been updated.", "You have asked Koluhm to send the goblin's
              words..." and the experience line); 1 ends it; the exit
  Thomas      back through the town to Thomas on the bridge: "Take you
              somewhere?" (text only), 2: to the Palisade
  Shawford    door, doorway (he may start talking himself); "Did you speak to
              Koluhm?" (text only), the 6 panels above (the first shows the
              journal line, "You have carried out Shawford's orders." and the
              experience line); Continue ends it; leave the house
  Thomas      "Take you somewhere?" (text only), 1: to the docks
  Geoffrey    south along x = 792 to the Salty Dog's door; the door, the
              doorway (AR1003). Click Black Geoffrey: his first line (text
              only: no replies, a Continue button), the 17 panels above (the
              last shows the journal line, "You have convinced the Iron Collar
              mercenary band to report to the Palisade." and the experience
              line); 1 ends it
  journal     J: the page's text without the yellow day/hour headings (the
              Koluhm, Shawford and Iron Collar entries)
  record      the Record dial button: Sheris's record (2%) and its Experience
              block (0.4%: 1300, Next Level 3000); Esc
  save        Q: compare the save lines; on the original the save is banked

Not compared: the world view (snow, people walking, the clock), the loading
screens and their hints, overhead lines (Thomas, Gohar, Black Geoffrey and the
Palisade trainees print into the message window at random times), the first
panel of each dialogue except its speaker's paragraph and the replies.

Map points (area maps at 1/8 scale): AR1000 origin (138,73), AR1100 origin
(158,85), AR1200 origin (99,40). AR1100: the morgue door DOOR1 396-445,
906-964 (To1107 397-444,908-962), reached from the north side (420,990 is
walkable; the search-map path from the bridge goes north up x = 2056 and west
along y = 1194 and 1110, about 98 s each way). AR1107 on entering: screen =
map + (15,-143); Koluhm 418,257, the exit To1100 191-311,448-571. AR1000: the
Salty Dog door DOOR3 892-938,1764-1822 (To1003 893-937,1766-1820), walkable
below it at 915,1850. AR1003 on entering: screen = map - (1,104); Black
Geoffrey 717,382, Kickshaw 754,463, Blanchard 821,353, Gohar 356,194.
"""

from __future__ import annotations

import shutil
import time

import numpy as np
import pytest

import test_s16_clear_docks as s16
import test_s17_dock_houses as s17
import test_s18_warehouse_caves as s18
import test_s19_town_errands as s19
import test_s20_palisade as s20
import test_s21_lumber as s21
import test_s22_level_up as s22
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s9_docks import AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, act
from test_s14_travel import select_all, travel

S22_END = s22.S22_END
S23_END = s17.BANK / "s23-end"

FIGHT_PARK = s17.FIGHT_PARK  # (520, 515): off the world view
SCREEN_PARK = (300, 560)
JOURNAL_TEXT = s18.JOURNAL_TEXT

AR1000_MAP = (138, 73)
AR1100_MAP = (158, 85)
AR1200_MAP = (99, 40)

SPELLBOOK = s20.SPELLBOOK
RECORD = s20.RECORD
RECORD_SCREEN = s20.RECORD_SCREEN
RECORD_TEXT = s20.RECORD_TEXT
XP_BLOCK = s20.XP_BLOCK
PANEL = (0, 0, 800, 430)  # spellbook and record, above the portraits
RECORD_TABS = [(461, 35), (548, 35), (632, 35), (712, 35)]  # the record's four pages

# Walks: (map point the view is centred on, map point walked to).
NILES_SPOT = (3200, 2250)  # where S22 ended
TO_THOMAS = [((x, y), (x, y)) for x, y in [(3150, 2220), (2950, 2270), (2760, 2340), (2640, 2440)]]
THOMAS_VIEW = s21.THOMAS_VIEW
THOMAS = s21.THOMAS
THOMAS_TOWN = s21.THOMAS_TOWN
TOWN_LANDING = s21.TOWN_LANDING
DOCKS_LANDING = s21.DOCKS_LANDING
# AR1100SR path from the bridge to the morgue (every 12th cell).
TO_MORGUE = [((x, y), (x, y)) for x, y in [
    (2056, 1758), (2056, 1614), (2056, 1470), (2008, 1362), (1976, 1242), (1848, 1194),
    (1656, 1194), (1464, 1194), (1272, 1194), (1080, 1194), (952, 1146), (808, 1110),
    (616, 1110), (424, 1110), (424, 990)]]
FROM_MORGUE = [((x, y), (x, y)) for x, y in [
    (568, 1038), (760, 1038), (920, 1062), (1048, 1110), (1208, 1134), (1400, 1134),
    (1576, 1146), (1736, 1170), (1928, 1170), (2072, 1206), (2072, 1350), (2088, 1482),
    (2088, 1626), (2088, 1770), (2120, 1878)]]
MORGUE_OUTSIDE = (424, 990)
MORGUE_DOOR = (420, 945)
MORGUE_DOORWAY = (424, 933)
TO_SALTY_DOG = [((x, y), (x, y)) for x, y in [
    (600, 966), (792, 1100), (792, 1300), (792, 1500), (800, 1700), (915, 1860)]]
DOG_OUTSIDE = (915, 1860)
DOG_DOOR = (915, 1790)
DOG_DOORWAY = (915, 1795)

# Inside, as the view shows each area on entering.
KOLUHM = (433, 114)
MORGUE_EXIT = (266, 385)
GEOFFREY = (716, 278)

HOUSE_VIEW = s20.HOUSE_VIEW
HOUSE_DOOR = s20.HOUSE_DOOR
HOUSE_DOORWAY = s20.HOUSE_DOORWAY
HOUSE_EXIT = s20.HOUSE_EXIT
SHAWFORD = s20.SHAWFORD

KOLUHM_STEPS = ["1", "2", CONTINUE, CONTINUE, "1", CONTINUE, "1", CONTINUE, CONTINUE, "1", "1",
                "1", CONTINUE, CONTINUE, "1", CONTINUE, "1", "1", "1", "1"]
SHAWFORD_STEPS = ["1", "1", CONTINUE, "1", "1", "1"]
GEOFFREY_STEPS = [CONTINUE, CONTINUE, "1", CONTINUE, CONTINUE, CONTINUE, CONTINUE, "2", "2", "2",
                  CONTINUE, "2", CONTINUE, CONTINUE, CONTINUE, CONTINUE, CONTINUE]


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s22_save(play_dir, out_dir, target):
    """Put the banked S22 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S22_END / "save").iterdir() if p.is_dir()] if (S22_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S22 save in {S22_END}; run test_s22_level_up --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    if target == "original":
        S23_END.mkdir(parents=True, exist_ok=True)
    yield
    s17.finish_saves(play_dir, out_dir, S23_END if target == "original" else None)


# --- the test ------------------------------------------------------------------


def test_s23_iron_collar(from_s22_save, game, golden_dir, out_dir, target):
    group_bar = s17.load_bank(game, S22_END)
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

    def wait_speech(who, replies=True):
        """Until the first panel shows its replies (or, for a first line with
        only a Continue button, the speaker's text)."""
        deadline = time.time() + 30.0
        while True:
            cur = game.grab()
            if s20.replies_shown(cur) if replies else s19.speech_block(cur) is not None:
                return
            if time.time() > deadline:
                raise AssertionError(f"{who}'s first line did not show")
            time.sleep(0.3)

    def click_talk(who, find, replies=True):
        deadline = time.time() + 20.0
        while (spot := find()) is None:
            if time.time() > deadline:
                raise AssertionError(f"{who}'s circle did not show")
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
        wait_speech(who, replies)

    def talk(who, real, near, dx=80, dy=60):
        """Click the cyan circle nearest the map point `near` (as S21)."""
        want = s16.to_screen(real, near)

        def find():
            best = None
            for c in s21.cyan_circles(game.grab()):
                if abs(c[0] - want[0]) > dx or abs(c[1] - want[1]) > dy:
                    continue
                d = np.hypot(c[0] - want[0], c[1] - want[1])
                if best is None or d < best[0]:
                    best = (d, c)
            return best and best[1]

        click_talk(who, find)

    def talk_at(who, near, replies=True, dx=30, dy=15):
        """A screen spot (interiors, as S20)."""
        click_talk(who, lambda: s16.cyan_near(game.grab(), near, dx=dx, dy=dy), replies)

    def first_panel(name):
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
        if isinstance(key, str):
            act(game, lambda: game.press(key), f"last reply to {who}", ACTION_BAR_REGION,
                park=FIGHT_PARK, wait=2.0)
        else:
            act(game, lambda: game.click(*key), f"last Continue with {who}", ACTION_BAR_REGION,
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

    def enter(real, door, doorway, what):
        """Click a closed door (the party walks there and opens it), then the
        doorway; a second try if the first doorway click does nothing (S20)."""
        select_all(game, group_bar)
        game.click(*s16.to_screen(real, door))
        game.move(*FIGHT_PARK)
        time.sleep(6.0)
        for attempt in range(2):
            try:
                go(s16.to_screen(real, doorway), what)
                return
            except AssertionError:
                if attempt:
                    raise
                time.sleep(3.0)

    # --- Sheris's spellbook and record (S22's level up) --------------------------------
    click(SPELLBOOK, ACTION_BAR_REGION, "Spellbook dial button", threshold=0.3)
    check("spellbook-sheris", PANEL)
    act(game, lambda: game.press("esc"), "Esc on the spellbook", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)
    click(RECORD, ACTION_BAR_REGION, "Record dial button", threshold=0.3)
    click(RECORD_TABS[2], RECORD_TEXT, "the record's skills page", threshold=0.01)
    check("record-sheris-skills", RECORD_SCREEN)
    click(RECORD_TABS[0], RECORD_TEXT, "the record's first page", threshold=0.01)
    act(game, lambda: game.press("esc"), "Esc on the record", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)

    # --- Thomas into town ----------------------------------------------------------------
    walk(TO_THOMAS, NILES_SPOT, AR1200_MAP)
    real = jump(THOMAS_VIEW, AR1200_MAP)
    talk("Thomas", real, THOMAS, dx=80, dy=60)
    first_panel("thomas-01")
    ride("2", "reply 2 to Thomas (into town)")

    # --- Koluhm ------------------------------------------------------------------------------
    here, real = walk(TO_MORGUE, TOWN_LANDING, AR1100_MAP, final=2.0)
    enter(real, MORGUE_DOOR, MORGUE_DOORWAY, "click on the doorway of the morgue")
    time.sleep(1.0)
    talk_at("Koluhm", KOLUHM)
    first_panel("koluhm-01")
    dialogue("Koluhm", KOLUHM_STEPS, "koluhm")
    end("1", "Koluhm")
    go(MORGUE_EXIT, "click on the exit of the morgue")

    # --- Thomas to the Palisade -------------------------------------------------------------
    here, real = walk(FROM_MORGUE, MORGUE_OUTSIDE, AR1100_MAP)
    talk("Thomas", real, THOMAS_TOWN, dx=150, dy=100)
    first_panel("thomas-town-01")
    ride("2", "reply 2 to Thomas (to the Palisade)")

    # --- Shawford ----------------------------------------------------------------------------
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
    wait_speech("Shawford")
    first_panel("shawford-01")
    dialogue("Shawford", SHAWFORD_STEPS, "shawford")
    end(CONTINUE, "Shawford")
    go(HOUSE_EXIT, "click on the exit of Shawford's house")

    # --- Thomas to the docks -------------------------------------------------------------
    real = jump(THOMAS_VIEW, AR1200_MAP)
    talk("Thomas", real, THOMAS, dx=80, dy=60)
    first_panel("thomas-02")
    ride("1", "reply 1 to Thomas (to the docks)")

    # --- Black Geoffrey ------------------------------------------------------------------
    here, real = walk(TO_SALTY_DOG, DOCKS_LANDING, AR1000_MAP, final=6.0)
    enter(real, DOG_DOOR, DOG_DOORWAY, "click on the doorway of the Salty Dog")
    time.sleep(1.0)
    talk_at("Black Geoffrey", GEOFFREY, replies=False)
    first_panel("geoffrey-01")
    dialogue("Black Geoffrey", GEOFFREY_STEPS, "geoffrey")
    end("1", "Black Geoffrey")
    select_all(game, group_bar)
    print(f"S23 errands took {time.time() - t0:.0f} s")

    # --- Journal and record ----------------------------------------------------------------
    act(game, lambda: game.press("j"), "J (journal)", ACTION_BAR_REGION, park=FIGHT_PARK)
    page = game.wait_stable(settle=1.0, region=JOURNAL_TEXT)
    check_frame("journal-iron-collar", s18.journal_text(page), JOURNAL_TEXT, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the journal", ACTION_BAR_REGION, park=FIGHT_PARK)
    click(RECORD, ACTION_BAR_REGION, "Record dial button", threshold=0.3)
    check("record-sheris-after", RECORD_SCREEN)
    tight("record-sheris-after-xp", XP_BLOCK["sheris"])
    act(game, lambda: game.press("esc"), "Esc on the record", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)

    # --- Quick save for S24 ------------------------------------------------------------
    s17.quick_save(game, out_dir, check)
    if target == "original":
        (out_dir / "bank-ok").write_text("ok")

    print(f"S23 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
