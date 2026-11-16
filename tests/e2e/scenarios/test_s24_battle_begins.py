"""S24: the report that starts the Palisade battle. From the Salty Dog on the
docks back to Shawford Crale on the Palisade (on foot: Swift Thomas does not
stand on the docks), his dialogue ("Did you hear that?"), the cutscene in his
house (Thomas runs in, Shawford gives his orders and leaves), the journal and
the six experience numbers after it, a quick save, and then out of the door
into the battle: Phaen of the Rags and his goblins come at the party at once.
Starts from S23's banked save and banks one of its own.

Saves (golden/bank/, git-ignored like all goldens):

  s23-end/  S23's end (see test_s23_iron_collar), loaded here: inside the
            Salty Dog (AR1003), Palisade_Iron_Collar_Quest = 2
  s24-end/  this test's end (on the original only): inside Shawford's house
            (AR1201) straight after the cutscene, Goblins_Attack_Palisade = 1,
            Palisade_Iron_Collar_Quest = 3, IC_Good = 1, 1200_BATTLE_SETUP = 2,
            experience 1450 / 1415 / 1415 / 1414 / 6413 / 1413, 120 gold,
            everyone unhurt, whole party selected. The battle has begun but
            the party has not fought yet.

Why it stops there. The battle (AR1200.BCS and the 12cSpn* cutscenes) is
about 60 creatures in four waves: teams 1-3 (14, with Vghotan and three
hounds) are on the training ground when the party comes out, and Phaen's team
8 (6, he casts Sleep and Charm Person) appears beside the party within
seconds of it leaving the house; when teams 1-3 are dead Caballus opens the
north-east wall for team 5 (12, Ghotrag), then team 7 (8), then team 6 at the
gate (11, Caballus) with team 4 (3). The award (Level_1_Very_Hard, journal
27868) comes when teams 4, 6 and 8 are dead. Fought the way S16 and S18 fight
(pause, everyone attacks the lowest red circle) the level-2 party lost three
members in two minutes on the original: Laeris was put to sleep, Khatja died
to Geldin's mace (charmed, or attacked as a charmed "hostile"), then Vghotan
and his hounds arrived. Nothing in that fight is the same twice, so it is
left to S25, which needs real tactics. Here the party only steps out, proves
the goblins are there, gives one attack order and stops; that part is not
banked and nothing in it is compared.

Experience: Shawford's "Yes - though they proved to be a stubborn bunch"
reply is AddXpVar("Level_2_Hard") = 900, 150 each.

Dialogue: 12SHAWFO S32 "Any luck on bringing the Iron Collar to the
Palisade?" -> 1 (T65: experience, journal, Palisade_Iron_Collar_Quest 3,
IC_Good 1) S3 "You surprise me - well done..." -> 1 "Did you hear that?" (T4:
Goblins_Attack_Palisade 1, StartCutScene("12cWar2")). The cutscene hides the
GUI for about 33 s: Swift Thomas "Sir! The goblins are at the Palisade!",
Shawford "Order the gate into town sealed! Sound the alarm!", Thomas "Yes,
sir!", Shawford "I must warn Ulbrec and gather the town forces..." (up for
11 s), "Everyone! To arms! To arms!"; he walks out and the GUI returns with
the party standing where the cutscene put it.

test_s24_battle_begins (both targets) loads the s23-end save, then, with the
whole party selected:

  walk        out of the Salty Dog, north along the docks to the log stairs,
              AR1100, past the crane and north to the Palisade gate (closed
              again: the door, then the gateway), AR1200, Shawford's house
              (door, doorway). Nothing compared.
  autosave    entering AR1201 with Palisade_Iron_Collar_Quest 2 runs
              SaveGame(39843): the folder "Autosave - Visiting Shawford at the
              Palisade" must be in mpsave (checked on disk)
  Shawford    his first line and reply (text only, as S19); 1: the panel with
              "Your journal has been updated.", "You have carried out
              Shawford's orders... and exceeded his expectations." and the
              experience line; 1 ("Did you hear that?")
  cutscene    the GUI goes (the action bar strip turns black); of the frames
              with the GUI hidden the one with the largest block of white
              float text is Shawford's long order: the block is compared
              (0.4%); the GUI must be back within 90 s
  save        Select All, Q at once (people outside start dying within
              seconds, and they are in the save): the green "Quick-save
              successful" line, found by colour because combat lines from
              outside keep scrolling the window; on the original the save is
              banked
  journal     J: the page's text without the yellow headings (ends with "We
              told Shawford that we'd pressured the Iron Collar band...")
  record      the Record dial button: Sheris's record (2%), and for each of
              the six the Experience block at 0.4% (1450, 1415, 1415, 1414,
              6413, 1413); Esc
  battle      the exit: AR1200. A hostile circle must show within 40 s;
              Space (PAUSED), Select All, Attack, a click on the lowest
              goblin (not required to take), Space (UNPAUSED), 10 s of
              fighting, and the game must still be running. Two screenshots
              for the record, not compared.

Not compared: the world view, the loading screens, the message window (the
fight outside prints into it from the moment the cutscene ends, also while the
party is still in the house), the first dialogue panel except its text,
anything of the fight.

Map points: AR1003 as the save shows it (the view on entering: screen = map -
(1,106)): exit To1000 249-363,378-542. AR1000: from the Salty Dog door back
along S23's TO_SALTY_DOG, then (330,760), (266,575) and S19's stairs. AR1100:
S21's TO_CRANE and CRANE_TO_GATE; the gate DOOR6 is closed in this save.
AR1201 after the cutscene: screen = map - (96,8) (Nolan 593,158 at 497,150),
exit To1200 56-210,362-519 at screen (60,380).
"""

from __future__ import annotations

import shutil
import time

import numpy as np
import pytest
from PIL import Image

import test_s13_house as s13
import test_s16_clear_docks as s16
import test_s17_dock_houses as s17
import test_s18_warehouse_caves as s18
import test_s19_town_errands as s19
import test_s20_palisade as s20
import test_s21_lumber as s21
import test_s23_iron_collar as s23
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s8_screens_tour import PORTRAIT
from test_s9_docks import AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, SELECT_ALL, act
from test_s10_save_load import MESSAGE_LINES
from test_s14_travel import select_all, travel

S23_END = s23.S23_END
S24_END = s17.BANK / "s24-end"

FIGHT_PARK = s17.FIGHT_PARK  # (520, 515): off the world view
SCREEN_PARK = (300, 560)
MESSAGES = s17.MESSAGES
JOURNAL_TEXT = s18.JOURNAL_TEXT

AR1000_MAP = (138, 73)
AR1100_MAP = (158, 85)
AR1200_MAP = (99, 40)

RECORD = s20.RECORD
RECORD_SCREEN = s20.RECORD_SCREEN
RECORD_TEXT = s20.RECORD_TEXT
XP_BLOCK = s20.XP_BLOCK
NAMES = s20.NAMES
ATTACK = s13.ATTACK

# Walks: (map point the view is centred on, map point walked to).
DOG_EXIT = (300, 360)  # AR1003, the view the save shows
DOG_OUTSIDE = s23.DOG_OUTSIDE
TO_STAIRS = [((x, y), (x, y)) for x, y in [
    (800, 1700), (792, 1500), (792, 1300), (792, 1100), (600, 966), (420, 960), (330, 760),
    (266, 575)]]
GATE_VIEW = s20.GATE_VIEW
GATE_DOOR = s20.GATE_DOOR
GATEWAY = s20.GATEWAY
HOUSE_VIEW = s20.HOUSE_VIEW
HOUSE_DOOR = s20.HOUSE_DOOR
HOUSE_DOORWAY = s20.HOUSE_DOORWAY
SHAWFORD = s20.SHAWFORD  # screen, AR1201 as on entering
BATTLE_EXIT = (60, 380)  # AR1201's exit in the view the cutscene leaves

# The cutscene: where Shawford's float text is compared.
FLOAT_BOX = (250, 140, 310, 90)
SAVED_SPOT = (0, 0, 535, 14)  # where the "Quick-save successful" line is compared
AUTOSAVE = "Autosave - Visiting Shawford at the Palisade"


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s23_save(play_dir, out_dir, target):
    """Put the banked S23 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S23_END / "save").iterdir() if p.is_dir()] if (S23_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S23 save in {S23_END}; run test_s23_iron_collar --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    if target == "original":
        S24_END.mkdir(parents=True, exist_ok=True)
    yield
    s17.finish_saves(play_dir, out_dir, S24_END if target == "original" else None)


# --- finding things on screen --------------------------------------------------


def gui_hidden(img):
    """A cutscene has just hidden the GUI: the action bar's strip is black
    (until the view moves and the world is drawn there)."""
    return float(np.asarray(img.convert("L"))[440:490, 0:500].mean()) < 12


def float_text(img):
    """The largest block of white float text in the top 430 rows, moved to
    FLOAT_BOX on a black screen with only its white pixels kept, and how many
    those are. The view during the cutscene was not where it was while
    exploring (the party stood elsewhere when the dialogue began), so the
    block is found, not read from a fixed place. Lamp cores are white too: rows with
    fewer than 12 white pixels and columns more than 14 px from the text are
    left out."""
    a = np.asarray(img.convert("RGB"))[:430]
    white = a.min(axis=2) > 235
    blocks = []
    for start, end in s18._runs(white.sum(axis=1) >= 12):
        if blocks and start - blocks[-1][1] <= 10:
            blocks[-1][1] = end
        else:
            blocks.append([start, end])
    best = None
    for top, bottom in blocks:
        top, bottom = max(0, top - 3), bottom + 3  # the thin tops and tails of the letters
        spans = []
        for start, end in s18._runs(white[top:bottom].any(axis=0)):
            if spans and start - spans[-1][1] <= 14:
                spans[-1][1] = end
            else:
                spans.append([start, end])
        for left, right in spans:
            n = int(white[top:bottom, left:right].sum())
            if best is None or n > best[0]:
                best = (n, top, bottom, left, right)
    out = np.zeros((img.size[1], img.size[0], 3), np.uint8)
    if best is None:
        return Image.fromarray(out), 0
    n, top, bottom, left, right = best
    x, y, w, h = FLOAT_BOX
    block = white[top:bottom, left:right][:h, :w]
    out[y:y + block.shape[0], x:x + block.shape[1]] = np.where(block[..., None], 255, 0)
    return Image.fromarray(out), int(block.sum())


def saved_line(img):
    """The green "Quick-save successful" line of the message window, moved to
    SAVED_SPOT with only its text kept, or None. Combat lines keep coming
    (some start with a green name, which ends well left of x=120), so the
    line is the lowest one whose green reaches x=120..150 and no further."""
    x0, y0, w0, h0 = MESSAGE_LINES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y0:y0 + h0, x0:x0 + w0]
    green = (a[..., 1] > 200) & (a[..., 0] < 80) & (a[..., 2] < 80)
    for start, end in reversed(s18._runs(green.any(axis=1))):
        if end - start < 8 or end - start > 14:
            continue
        cols = np.nonzero(green[start:end].any(axis=0))[0]
        if not 120 <= int(cols.max()) <= 150:
            continue
        top = max(0, end - 13)
        line = np.where(green[top:top + 14][..., None], a[top:top + 14], 0).astype(np.uint8)
        out = Image.new("RGB", img.size)
        out.paste(Image.fromarray(line), (0, 0))
        return out
    return None


# --- the test ------------------------------------------------------------------


def test_s24_battle_begins(from_s23_save, game, golden_dir, out_dir, target, play_dir):
    group_bar = s17.load_bank(game, S23_END)
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

    def click(xy, region, what, park=SCREEN_PARK, **kw):
        act(game, lambda: game.click(*xy), what, region, park=park, **kw)

    # --- Back to the Palisade on foot ---------------------------------------------------
    go(DOG_EXIT, "click on the exit of the Salty Dog")
    walk(TO_STAIRS, DOG_OUTSIDE, AR1000_MAP, final=2.0)
    area_map(s21.MAP_STAIRS)
    game.click(*s21.STAIRS_FOOT)
    game.move(*FIGHT_PARK)
    time.sleep(8.0)
    go(s21.STAIRS_EXIT, "click on the exit at the top of the stairs")
    walk(s21.TO_CRANE, s21.TOWN_ARRIVAL, AR1100_MAP, final=0.5)
    walk(s21.CRANE_TO_GATE, (900, 1794), AR1100_MAP, final=2.0)
    real = jump(GATE_VIEW, AR1100_MAP)
    # The gate is closed again. The click on it walks the party up and opens
    # it; on the original that click alone then took the party through (the
    # loading screen came 6-7 s after it, twice in two tries), where S20
    # needed a second click on the gateway. Take either.
    select_all(game, group_bar)
    game.click(*s16.to_screen(real, GATE_DOOR))
    game.move(*FIGHT_PARK)
    deadline, gone = time.time() + 15.0, False
    while time.time() < deadline and not gone:
        gone = diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) > 0.5
        time.sleep(0.2)
    if gone:
        deadline = time.time() + 60.0
        while diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) > 0.05:
            if time.time() > deadline:
                raise AssertionError("no area after the loading screen (the Palisade gate)")
            time.sleep(0.2)
        time.sleep(3.0)
    else:
        for attempt in range(2):
            try:
                go(s16.to_screen(real, GATEWAY), "click on the Palisade gateway")
                break
            except AssertionError:
                if attempt:
                    raise
                time.sleep(3.0)
    print(f"S24 walk took {time.time() - t0:.0f} s")

    # --- Into Shawford's house (as S23: he may start talking himself) ---------------
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
    deadline = time.time() + 30.0
    while not s20.replies_shown(game.grab()):
        if time.time() > deadline:
            raise AssertionError("Shawford's first line did not show")
        time.sleep(0.3)
    if not any(AUTOSAVE in p.name for p in (play_dir / "mpsave").iterdir()):
        failures.append(f'autosave: no "{AUTOSAVE}" folder in mpsave after entering the house')

    # --- Shawford: the report -----------------------------------------------------------
    game.move(*DIALOG_PARK)
    check_frame("shawford-01", s19.speech_block(game.wait_stable(settle=1.0, region=DIALOG_REGION)),
                s19.SPEECH_SPOT)
    act(game, lambda: game.press("1"), "reply 1 to Shawford (the Iron Collar)", DIALOG_REGION,
        park=DIALOG_PARK)
    check("shawford-02", DIALOG_REGION)

    # --- "Did you hear that?": the cutscene ------------------------------------------
    act(game, lambda: game.press("1"), 'reply 1 to Shawford ("Did you hear that?")', DIALOG_REGION,
        park=FIGHT_PARK, wait=0.0, threshold=0.05)
    started = time.time()
    hidden_at, best, next_shot = None, None, 0.0
    while True:
        cur = game.grab()
        if hidden_at is None and gui_hidden(cur):
            hidden_at = time.time()
        if hidden_at and diff_ratio(group_bar, cur, region=ACTION_BAR_REGION) <= 0.05:
            break
        if hidden_at:
            # (The strip does not stay black: with the GUI gone the world
            # view fills the window and the house can be drawn there.)
            if time.time() >= next_shot:  # for the record, not compared
                game.shots += 1
                cur.save(out_dir / f"{game.shots:03d}-cutscene-{time.time() - hidden_at:02.0f}s.png")
                next_shot = time.time() + 3.0
            text, n = float_text(cur)
            if best is None or n > best[1]:
                best = (text, n)
        if time.time() > started + 90.0:
            raise AssertionError("the cutscene did not hide the GUI" if hidden_at is None
                                 else "the GUI did not come back after the cutscene")
        time.sleep(0.25)
    print(f"S24 cutscene: GUI hidden for {time.time() - hidden_at:.0f} s, "
          f"{best[1] if best else 0} white pixels in the largest float text")
    check_frame("cutscene-orders", best and best[0], FLOAT_BOX, max_diff=0.004)

    # --- Quick save for S25, before anything else ------------------------------------
    time.sleep(1.0)
    game.click(*SELECT_ALL)
    game.move(*FIGHT_PARK)
    time.sleep(1.5)
    game.grab().save(out_dir / "bank-world.png")
    game.press("q")
    game.move(*FIGHT_PARK)
    line, deadline = None, time.time() + 20.0
    while line is None and time.time() < deadline:
        time.sleep(0.2)
        line = saved_line(game.grab())
    check_frame("quick-saved", line, SAVED_SPOT, max_diff=0.004)
    time.sleep(3.0)
    if target == "original" and line is not None:
        (out_dir / "bank-ok").write_text("ok")
    select_all(game, group_bar)

    # --- Journal and experience -------------------------------------------------------
    act(game, lambda: game.press("j"), "J (journal)", ACTION_BAR_REGION, park=FIGHT_PARK)
    page = game.wait_stable(settle=1.0, region=JOURNAL_TEXT)
    check_frame("journal-battle", s18.journal_text(page), JOURNAL_TEXT, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the journal", ACTION_BAR_REGION, park=FIGHT_PARK)
    click(RECORD, ACTION_BAR_REGION, "Record dial button", threshold=0.3)
    for i, name in enumerate(NAMES):
        if i:
            click(PORTRAIT[i], RECORD_TEXT, f"{name}'s portrait on the record")
        else:
            check("record-sheris", RECORD_SCREEN)
        tight(f"record-{name}-xp", XP_BLOCK[name])
    act(game, lambda: game.press("esc"), "Esc on the record", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)
    select_all(game, group_bar)
    print(f"S24 steps to the end of the record took {time.time() - t0:.0f} s")

    # --- Out into the battle (not banked, nothing compared) --------------------------
    go(BATTLE_EXIT, "click on the exit of Shawford's house")
    deadline = time.time() + 40.0
    while not s13.hostile_circles(game.grab()):
        if time.time() > deadline:
            raise AssertionError("no hostile creature showed outside Shawford's house")
        time.sleep(0.3)
    s18.set_paused(game, True, "the Palisade battle")
    frame = game.screenshot("battle-paused")
    targets = s18.clickable_circles(frame)
    print(f"S24 battle: {len(s13.hostile_circles(frame))} hostile circle(s) in view")
    game.click(*SELECT_ALL)
    game.move(*FIGHT_PARK)
    time.sleep(0.5)
    order = "not given"
    try:
        act(game, lambda: game.click(*ATTACK), "Attack", ACTION_BAR_REGION, park=FIGHT_PARK, wait=1.0)
        order = "no click on a goblin took"
        for aim in [(x + dx, y + dy) for x, y in sorted(targets, key=lambda t: -t[1])
                    for dx, dy in ((0, -12), (0, -2), (12, -12), (-12, -12))]:
            try:
                act(game, lambda: game.click(*aim), "click on a goblin", ACTION_BAR_REGION,
                    park=FIGHT_PARK, wait=1.0, timeout=2.0)
                order = f"given at {aim}"
                break
            except AssertionError:
                continue
    except AssertionError:
        pass  # someone asleep or charmed can leave the group bar without Attack
    print(f"S24 battle: attack order {order}")
    s18.set_paused(game, False, "the Palisade battle")
    time.sleep(10.0)
    game.screenshot("battle-10s")

    print(f"S24 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
