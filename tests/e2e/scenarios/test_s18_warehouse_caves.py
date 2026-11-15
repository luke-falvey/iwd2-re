"""S18: the last of the dock goblins. Into the locked warehouse (AR1002) with
Kylie's lock pick, through it into the goblin caves beneath it (AR1007), the
"docks cleared" award, and Brogan's thanks. Starts from S17's banked save and
banks one of its own end state for S19.

What clears the docks (see test_s16_clear_docks): five area flags, the last
of which is AR1007's. At the default difficulty AR1002 holds 3 goblins and 2
archers, AR1007 17 goblins and sappers and Rukwurm, in five groups that come
at the party when it is seen ("Hsssst! Soldiers! SOLDIERS!"). When the last
of them dies the AR1007 area script sets AR1007_GOBLINS_CLEAR, and on its
next pass gives the award (AddXPVar Level_2_Very_Hard with strref 26671):
three lines in a row, "You have cleared the Targos Docks of goblins." (pale
green), "The party has gained experience." and "Your journal has been
updated." (journal entry 27866: "We managed to drive the last of the goblins
off of the Targos Docks. Reig and Lord Ulbrec should be grateful to hear of
our success."). Back on the docks Brogan, at the warehouse, has new lines
(the caves, a journal entry and a second experience award, "seek out Lord
Ulbrec in the town above"); his dialogue ends with Brogan_Leave set, and he
walks off towards the town.

Saves (golden/bank/, git-ignored like all goldens):

  s17-end/  S17's end (see test_s17_dock_houses), loaded here
  s18-end/  this test's end (on the original only): on the warehouse pier
            after Brogan's dialogue, every dock goblin dead
            (Dock_Goblin_Quest = 1), the party rested in AR1002, Brogan
            walking off, whole party selected. For S19 (the town now open: the inn, Deirdre's store).

test_s18_warehouse_caves (both targets) loads the s17-end save the way S17
loads S16's, then, with the whole party selected:

  warehouse   from outside Firtha's house north and east round the bay onto
              the warehouse pier (area-map steps)
  lock pick   Kylie alone (portrait 6): Thieving, then the warehouse door; she
              walks to it and "Kylie- Lock Pick Succeeded" is printed (found
              by the orange of her name; compared at a fixed spot). A "Lock
              Pick Failed" (a shorter line) is retried, up to five times
  AR1002      select all, the door (it opens), the doorway: five goblins come;
              fight them as S16 does; compare the words of the last
              experience line (without the number, which depends on whether
              a goblin or an archer died last); walk into the middle of the
              room and rest (allowed now: "You have rested for N hours." is
              not compared, the hours depend on the damage); compare the
              portraits (every hit point back) and, with the view on the back
              door, the west wall and the covered wagon (no lamps, no party)
  AR1007      the back door (the party walks there and opens it), then the
              doorway with everyone selected: the caves. Fight the goblins at
              the entrance, then walk in steps north, west, down into the
              southern passage (two goblins there never notice the party)
              and back, and west along the northern passage; before each
              fight the view is centred on the party (its marker on the area
              map), since the goblins come at it out of view. A sweep further
              west and round the northern groups follows only if the award
              has not come. The award lines are caught while it happens (the
              grab is watched: they scroll away under the combat lines) and
              compared as one picture; then the journal (J): its text,
              without the yellow day/hour headings
  back        east through the caves, out by To1002, through the warehouse
              and out by To1000
  Brogan      click him on the pier: compare his first line and the reply
              (only the text, each block found by colour: the panel may
              show the lines from before the dialogue above them), replies 1, 1,
              Continue (journal and experience lines: compare), 1, then the
              last reply 1 ends it
  save        Q: compare the save lines; on the original the save is banked

Not compared: the world view on the way, the combat lines and hit points
during fights, where the fights happen, Brogan's and the goblins' overhead
lines, the loading hints, the experience numbers.

Map points: AR1000 (area map at (138,73), 1/8): the warehouse door
2252-2503,455-601 (To1002 entrance FR1002 2291,609); Brogan starts at
2047,532. AR1002 (area map at (250,149)): FR1000 410,775 (entering from the
docks), To1000 174-409,816-1083, the door to the caves 571-645,197-246
(To1007 573-642,199-243, FR1007 541,304). AR1007 (area map at (154,145)):
FR1002 2606,874, To1002 2399-2725,912-1145; Rukwurm 572,564; two cave-in
traps 1069-1403,169-363 and 238-482,648-738 (the route stays clear of them).
"""

from __future__ import annotations

import shutil
import time

import numpy as np
import pytest
from PIL import Image

import test_s16_clear_docks as s16
import test_s17_dock_houses as s17
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s8_screens_tour import PORTRAIT
from test_s9_docks import AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, act
from test_s13_house import THIEVING, hostile_circles
from test_s14_travel import select_all

S17_END = s17.S17_END
S18_END = s17.BANK / "s18-end"

KYLIE = PORTRAIT[5]
REST = (720, 511)
FIGHT_PARK = s17.FIGHT_PARK
MESSAGES = s17.MESSAGES
LINE_SPOT = s17.LINE_SPOT
WORLD_REGION = (0, 0, 800, 430)
PORTRAITS = (500, 435, 300, 55)
VRAEDNA_ICON = s16.VRAEDNA_ICON
XP_WORDS = (8, 576, 200, 14)  # "The party has gained experience:" without the number
AWARD_SPOT = (8, 550, 535, 40)  # where the three award lines are compared
JOURNAL_TEXT = (60, 62, 665, 305)

# Area-map origins (1/8 scale, the map centred in the area-map panel).
AR1000_MAP = (138, 73)
AR1002_MAP = (250, 149)
AR1007_MAP = (154, 145)

# Walks: (map point the view is centred on, map point walked to).
AR1004_TO_PIER = [
    ((700, 2050), (690, 1880)),
    ((700, 1700), (700, 1640)),
    ((696, 1536), (696, 1377)),
    ((896, 1248), (1000, 1200)),  # round the half-built ship
    ((1200, 1000), (1200, 860)),
    ((1550, 750), (1500, 800)),
    ((1700, 650), (1760, 610)),  # the west end of the warehouse pier
    ((2000, 640), (2000, 600)),
    ((2300, 640), (2250, 620)),  # by the warehouse door
]
OUTSIDE_AR1004 = (680, 2162)
WAREHOUSE_VIEW = (2296, 640)
WAREHOUSE_DOOR = (2376, 539)
BROGAN = (2047, 532)

AR1002_DOOR_VIEW = (400, 300)
AR1002_DOOR = (600, 220)  # the door to the caves at the back of the warehouse
AR1002_MIDDLE = (500, 500)
AR1002_EXIT_VIEW = (400, 800)
AR1002_EXIT = (280, 929)
# AR1002_DOOR_VIEW: the west wall, the covered wagon and the shelves left of
# the party (no lamp; the fight is at the other end of the room).
AR1002_ROOM = (180, 20, 190, 300)

# West through the caves; every group of goblins comes at the party once it
# is seen. The last steps are a sweep in case a group has not come yet.
AR1007_WEST = [
    ((2550, 650), (2600, 700)),  # north from the entrance (west of it is rock)
    ((2400, 500), (2500, 480)),
    ((2200, 500), (2250, 500)),  # then west
    # Two goblins (Goblin_4/5) stay put in the southern passage, out of
    # sight of the northern one: down to them and back.
    ((2300, 650), (2300, 700)),
    ((2250, 850), (2200, 900)),
    ((1950, 880), (1950, 900)),
    ((2250, 800), (2300, 700)),
    ((2000, 500), (2000, 520)),
    ((1800, 500), (1800, 560)),
    ((1700, 600), (1676, 589)),
    ((1500, 650), (1516, 659)),
    ((1450, 800), (1470, 830)),
    ((1300, 800), (1146, 777)),
]
AR1007_SWEEP = [
    ((1000, 700), (950, 700)),
    ((800, 650), (750, 620)),
    ((600, 600), (550, 580)),
]
# Where the north groups start (sappers by the first cave-in, two goblins
# east of them), if the award has still not come.
AR1007_NORTH = [
    ((1200, 550), (1250, 480)),
    ((1450, 500), (1400, 470)),
    ((1800, 450), (1800, 430)),
]
AR1007_EAST = [
    ((1300, 800), (1450, 800)),
    ((1500, 700), (1450, 700)),
    ((1600, 620), (1650, 620)),
    ((1900, 560), (1880, 560)),
    ((2100, 520), (2150, 500)),
    ((2400, 500), (2450, 500)),
    ((2550, 650), (2600, 700)),
    ((2550, 850), (2600, 850)),
]
AR1007_EXIT = (2582, 977)

BROGAN_STEPS = ["1", "1", CONTINUE, "1"]


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s17_save(play_dir, out_dir, target):
    """Put the banked S17 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S17_END / "save").iterdir() if p.is_dir()] if (S17_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S17 save in {S17_END}; run test_s17_dock_houses --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    if target == "original":
        S18_END.mkdir(parents=True, exist_ok=True)
    yield
    s17.finish_saves(play_dir, out_dir, S18_END if target == "original" else None)


# --- finding things on screen --------------------------------------------------


def _runs(mask_rows):
    runs, start = [], None
    for r, on in enumerate(list(mask_rows) + [False]):
        if on and start is None:
            start = r
        elif not on and start is not None:
            runs.append((start, r))
            start = None
    return runs


def kylie_line(img):
    """The lowest message line that starts with Kylie's name (orange), as
    (frame with only its text at LINE_SPOT, right edge of its text), or
    None."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    m = a[y0:y0 + h0, 8:60]
    orange = (m[..., 0] > 200) & (m[..., 1] > 130) & (m[..., 1] < 190) & (m[..., 2] > 60) & (m[..., 2] < 120)
    runs = [r for r in _runs(orange.any(axis=1)) if r[1] - r[0] >= 6]
    if not runs:
        return None
    top = y0 + runs[-1][0] - 1
    x, w, h = LINE_SPOT[0], LINE_SPOT[2], LINE_SPOT[3]
    line = a[top:top + h, x:x + w]
    keep = line.max(axis=2) > 110
    keep[:, 495 - x:] = False  # the scroll arrow at the right of the window
    cols = np.nonzero(keep[1:11].any(axis=0))[0]
    line = np.where(keep[..., None], line, 0).astype(np.uint8)
    out = Image.new("RGB", img.size)
    out.paste(Image.fromarray(line), (x, LINE_SPOT[1]))
    return out, (int(cols.max()) + x if len(cols) else 0)


def last_xp_line(img):
    """The lowest whole line of yellow text in the message window, moved to
    LINE_SPOT with only its text kept, or None. Unlike S16's
    last_yellow_line, the line is placed by its lowest row (the descenders
    of "party" and "experience"): the rows of two yellow lines in a row
    can touch, and a line cut off at the bottom of the window is skipped."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    m = a[y0:y0 + h0, 8:495]
    yellow = (m[..., 0] > 200) & (m[..., 1] > 200) & (m[..., 2] < 40)
    for start, end in reversed(_runs(yellow.any(axis=1))):
        if end - start < 9 or y0 + end > 594:
            continue
        # A yellow creature name ("Goblin Archer-") ends left of x=150.
        if not yellow[start:end, 150:].any():
            continue
        top = y0 + end - 12
        x, w, h = LINE_SPOT[0], LINE_SPOT[2], LINE_SPOT[3]
        line = a[top:top + h, x:x + w]
        text = ((line[..., 0] > 90) & (np.abs(line[..., 0] - line[..., 1]) < 30)
                & (line[..., 2] < line[..., 0] // 3))
        text[:, 495 - x:] = False
        line = np.where(text[..., None], line, 0).astype(np.uint8)
        out = Image.new("RGB", img.size)
        out.paste(Image.fromarray(line), (x, LINE_SPOT[1]))
        return out
    return None


def award_lines(img):
    """The pale green "You have cleared the Targos Docks of goblins." line
    and the two yellow lines below it, moved to AWARD_SPOT on a black screen
    with only their text kept, or None while the three are not all there."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    m = a[y0:y0 + h0, 8:495]
    r, g, b = m[..., 0], m[..., 1], m[..., 2]
    green = (g > 220) & (g - r >= 6) & (np.abs(r - b) <= 6)
    runs = [run for run in _runs(green.sum(axis=1) >= 3) if run[1] - run[0] >= 6]
    if not runs:
        return None
    top = y0 + runs[-1][0] - 1
    x, w, h = AWARD_SPOT[0], AWARD_SPOT[2], AWARD_SPOT[3]
    block = a[top:top + h, x:x + w]
    if block.shape[0] < h:
        return None
    br, bg, bb = block[..., 0], block[..., 1], block[..., 2]
    yellow = (br > 90) & (np.abs(br - bg) < 30) & (bb < br // 3)
    pale = (bg > 120) & (bg - br >= 6) & (np.abs(br - bb) <= 8)
    # Both yellow lines must be there: the second starts 26 rows down.
    if not yellow[14:26, :200].any() or not yellow[27:39, :200].any():
        return None
    keep = yellow | pale
    keep[:, 495 - x:] = False
    block = np.where(keep[..., None], block, 0).astype(np.uint8)
    out = Image.new("RGB", img.size)
    out.paste(Image.fromarray(block), (x, AWARD_SPOT[1]))
    return out


def journal_text(img):
    """The journal page with only its white text kept (the yellow headings
    carry the day and hour)."""
    x, y, w, h = JOURNAL_TEXT
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y:y + h, x:x + w]
    keep = (a.min(axis=2) > 110) & (a.max(axis=2) - a.min(axis=2) < 60)
    a = np.where(keep[..., None], a, 0).astype(np.uint8)
    out = Image.new("RGB", img.size)
    out.paste(Image.fromarray(a), (x, y))
    return out


class WatchAward:
    """While active, every frame the test grabs is looked at for the award
    lines (or what `find` finds); the last frame that showed them is kept."""

    def __init__(self, game, find=None):
        self.game = game
        self.find = find or award_lines
        self.frame = None

    def __enter__(self):
        grab = self.game.grab

        def watching():
            img = grab()
            found = self.find(img)
            if found is not None:
                self.frame = found
            return img

        self.game.grab = watching
        return self

    def __exit__(self, *exc):
        del self.game.grab


BROGAN_BLOCK = (108, 330, 550, 72)  # where his first line and the reply are compared


def speaker_block(img):
    """Brogan's first line (two rows of text from the lowest cyan speaker
    name) and the reply under it (from its first red row), each moved to a
    fixed place in BROGAN_BLOCK on a black screen with only the text kept,
    or None. Above his line the panel shows the message lines from before
    the dialogue (the award lines and a random loading hint) or not,
    depending on their length, and the gap down to the reply differs by a
    few rows with it."""
    x, y, w, h = DIALOG_REGION
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    m = a[y:y + h, x:x + 75]
    cyan = (m[..., 0] < 160) & (m[..., 1] > 170) & (m[..., 2] > 200) & (m[..., 2] - m[..., 0] > 80)
    runs = [r for r in _runs(cyan.any(axis=1)) if r[1] - r[0] >= 6]
    if not runs:
        return None
    top = y + runs[-1][0] - 2
    bx, by, bw, bh = BROGAN_BLOCK
    below = a[top + 26:y + h, bx:bx + bw]
    red = (below[..., 0] > 200) & (below[..., 1] < 80) & (below[..., 2] < 80)
    rows = np.nonzero(red.any(axis=1))[0]
    if not len(rows):
        return None
    reply_top = top + 26 + int(rows[0]) - 2
    out = np.zeros((img.size[1], img.size[0], 3), np.uint8)
    for src, dst, n in ((top, by, 28), (reply_top, by + 32, bh - 32)):
        block = a[src:src + n, bx:bx + bw]
        if block.shape[0] < n:
            return None
        # Only the text: the panel's texture differs with its height.
        out[dst:dst + n, bx:bx + bw] = np.where((block.max(axis=2) > 120)[..., None], block, 0)
    return Image.fromarray(out)


def party_on_map(img):
    """The middle of the party's markers (dark green, 0,160,0) on the area
    map, or None."""
    x, y, w, h = AREA_MAP_REGION
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y:y + h, x:x + w]
    m = (a[..., 0] < 20) & (np.abs(a[..., 1] - 160) < 12) & (a[..., 2] < 20)
    ys, xs = np.nonzero(m)
    if len(xs) < 10:
        return None
    return int(np.median(xs)) + x, int(np.median(ys)) + y


def pause_line(img):
    """"paused" or "unpaused" by the width of the lowest red line in the
    message window (PAUSED ends near x=62, UNPAUSED near x=81), or None."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y0:y0 + h0, 8:495]
    red = (a[..., 0] > 200) & (a[..., 1] < 70) & (a[..., 2] < 70)
    runs = [r for r in _runs(red.any(axis=1)) if r[1] - r[0] >= 6]
    if not runs:
        return None
    start, end = runs[-1]
    right = int(np.nonzero(red[start:end].any(axis=0))[0].max()) + 8
    return "paused" if right < 72 else "unpaused"


def set_paused(game, want, what):
    """Space, proved by the PAUSED or UNPAUSED line it prints. (S16's check of
    a still world view fails while walk-target markers are on screen: they
    keep turning while paused.)"""
    for _ in range(3):
        act(game, lambda: game.press("space"), f"Space ({what})", MESSAGES, park=FIGHT_PARK,
            wait=0.5, threshold=0.001)
        state = pause_line(game.grab())
        if state is None and not want and not s16.still(game):
            # The UNPAUSED line already scrolled away under a burst of
            # combat lines; the world view moving shows the game runs.
            state = "unpaused"
        if state == ("paused" if want else "unpaused"):
            time.sleep(0.5)
            return
    raise AssertionError(f"{what}: Space did not {'pause' if want else 'unpause'} the game")


def clickable_circles(img):
    """Hostile circles away from the edges of the view (see fight())."""
    return [(x, y) for x, y in hostile_circles(img) if y >= 30 and 15 <= x <= 785]


def fight(game, what):
    """S16's fight (with this test's pause check in place of S16's), retried
    when the only goblin in view is at the very edge of it and no click on it
    takes: unpause, let it come closer, again."""
    saved = s16.set_paused, s16.hostile_circles
    s16.set_paused = set_paused
    # S16 aims up to 24 px above a circle's centre: at the top edge of the
    # view that is the window's title bar, and a click on its close button
    # brings up the quit prompt. Goblins at the edge are left out (the
    # caller centres the view on the party and fights again).
    s16.hostile_circles = clickable_circles
    try:
        for attempt in range(3):
            try:
                s16.fight(game, what, shots=False)
                return
            except AssertionError as e:
                if attempt == 2 or not str(e).startswith("no click on the goblins"):
                    raise
                set_paused(game, False, what)
                time.sleep(3.0)
    finally:
        s16.set_paused, s16.hostile_circles = saved


# --- the test ------------------------------------------------------------------


def test_s18_warehouse_caves(from_s17_save, game, golden_dir, out_dir, target):
    group_bar = s17.load_bank(game, S17_END)
    t0 = time.time()

    failures = []

    def check(name, region, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

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

    def jump(centre, origin=AR1000_MAP):
        mx, my = origin[0] + centre[0] // 8, origin[1] + centre[1] // 8
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
            act(game, lambda: game.click(mx, my), f"area map click at {(mx, my)}", AREA_MAP_REGION,
                wait=0.5, threshold=0.001, park=FIGHT_PARK, timeout=3.0)
        except AssertionError:
            # The view was already centred there (the map shows no change).
            pass
        act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=1.0)
        return (mx - origin[0]) * 8, (my - origin[1]) * 8

    view = {}

    def step(centre, point, here, origin, fights=True):
        """Centre the view, walk the whole party to a map point (waiting for
        it to come out of the black), and fight whatever comes into view."""
        real = view["real"] = jump(centre, origin)
        spot = s16.to_screen(real, point)
        deadline = time.time() + 10.0
        while s16.unexplored(game.grab(), spot) and time.time() < deadline:
            time.sleep(0.5)
        game.click(*s16.to_screen(real, point))
        game.move(*FIGHT_PARK)
        end = time.time() + 2.0 + np.hypot(point[0] - here[0], point[1] - here[1]) / 60
        while time.time() < end:
            if fights and clickable_circles(game.grab()):
                fight(game, f"{origin} {point}")
                break
            time.sleep(0.3)
        return point

    def through(door, what):
        """Click a door (the party walks over and opens it), then, with
        everyone selected, the doorway: the next area."""
        game.click(*door)
        game.move(*FIGHT_PARK)
        time.sleep(8.0)
        select_all(game, group_bar)
        act(game, lambda: game.click(*door), what, WORLD_REGION, park=FIGHT_PARK,
            threshold=0.5, wait=2.0, timeout=25.0)

    def leave(exit_spot, what):
        """Click an exit with the whole party selected and wait for the next
        area's world screen."""
        select_all(game, group_bar)
        act(game, lambda: game.click(*exit_spot), what, WORLD_REGION, park=FIGHT_PARK,
            threshold=0.5, wait=0.0, timeout=30.0)
        deadline = time.time() + 60.0
        while diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) > 0.05:
            if time.time() > deadline:
                raise AssertionError(f"the next area did not appear ({what})")
            time.sleep(0.3)
        time.sleep(3.0)

    def watch_fights(what, origin=None):
        """Fight until no goblin is in view. With an area-map origin the view
        is first centred on the party (its marker on the area map), since
        goblins attack it wherever it is, also out of view."""
        for _ in range(4):
            if origin is not None:
                centre_on_party(origin)
            time.sleep(1.0)
            img = game.grab()
            if not hostile_circles(img):
                return
            if not clickable_circles(img):
                time.sleep(3.0)  # only at the edge: let it come closer
                continue
            fight(game, what)

    def centre_on_party(origin):
        for attempt in range(3):
            try:
                act(game, lambda: game.click(*AREA_MAP_BUTTON), "area map button", ACTION_BAR_REGION,
                    park=FIGHT_PARK, wait=0.5, timeout=5.0)
                break
            except AssertionError:
                if attempt == 2:
                    raise
        spot = party_on_map(game.grab())
        if spot is not None:
            try:
                act(game, lambda: game.click(*spot), "area map click on the party", AREA_MAP_REGION,
                    wait=0.5, threshold=0.001, park=FIGHT_PARK, timeout=3.0)
            except AssertionError:
                pass
        act(game, lambda: game.press("esc"), "Esc on the area map", ACTION_BAR_REGION,
            park=FIGHT_PARK, wait=1.0)

    # --- To the warehouse door ------------------------------------------------
    here = OUTSIDE_AR1004
    for centre, point in AR1004_TO_PIER:
        here = step(centre, point, here, AR1000_MAP, fights=False)

    # --- Kylie picks the lock --------------------------------------------------
    real = jump(WAREHOUSE_VIEW)
    found = None
    for attempt in range(5):
        act(game, lambda: game.click(*KYLIE), "click on Kylie's portrait", ACTION_BAR_REGION,
            park=FIGHT_PARK)
        act(game, lambda: game.click(*THIEVING), "Thieving", ACTION_BAR_REGION, park=FIGHT_PARK,
            wait=0.5)
        before = kylie_line(game.grab())
        game.click(*s16.to_screen(real, WAREHOUSE_DOOR))
        game.move(*FIGHT_PARK)
        deadline = time.time() + 15.0
        while time.time() < deadline:
            now = kylie_line(game.grab())
            if now is not None and (before is None or diff_ratio(before[0], now[0], region=LINE_SPOT) > 0):
                break
            time.sleep(0.3)
        else:
            raise AssertionError("Kylie printed no line at the warehouse door")
        time.sleep(0.5)
        found = kylie_line(game.grab())
        if found[1] > 160:  # "Lock Pick Succeeded"; "Lock Pick Failed" ends near 150
            break
    else:
        raise AssertionError("five lock picks failed")
    check_frame("lock-pick-line", found[0], LINE_SPOT, max_diff=0.004)

    # --- AR1002: the warehouse --------------------------------------------------
    select_all(game, group_bar)
    through(s16.to_screen(real, WAREHOUSE_DOOR), "click on the warehouse doorway")
    # The last experience line can scroll away under the overhead lines
    # that follow: keep the last frame that showed one.
    with WatchAward(game, last_xp_line) as watch:
        fight(game, "AR1002")
        game.grab()
    check_frame("ar1002-experience", watch.frame, XP_WORDS, max_diff=0.004)
    # Into the middle of the room, fighting any goblin still about.
    select_all(game, group_bar)
    here = step(AR1002_MIDDLE, (500, 450), (410, 700), AR1002_MAP)
    watch_fights("AR1002 middle")
    # Rest: allowed now that AR1002 is clear.
    select_all(game, group_bar)
    act(game, lambda: game.click(*REST), "Rest", MESSAGES, park=FIGHT_PARK, wait=0.0,
        threshold=0.001, timeout=20.0)
    time.sleep(3.0)
    game.wait_stable(settle=2.0, region=MESSAGES, timeout=30.0)
    check("ar1002-rested-portraits", PORTRAITS, max_diff=0.004, mask=[VRAEDNA_ICON])
    real = jump(AR1002_DOOR_VIEW, AR1002_MAP)
    check("ar1002-room", AR1002_ROOM)
    game.screenshot("ar1002-room-world")

    # --- AR1007: the caves -------------------------------------------------------
    # The door at the back: the first click walks the party there and opens
    # it, the second (everyone selected) takes it through. A click on the
    # doorway once the party stands in it lands on a party member instead.
    select_all(game, group_bar)
    through(s16.to_screen(real, AR1002_DOOR), "click on the doorway to AR1007")
    deadline = time.time() + 60.0
    while diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) > 0.05:
        if time.time() > deadline:
            raise AssertionError("the caves did not appear")
        time.sleep(0.3)
    time.sleep(2.0)
    with WatchAward(game) as watch:
        deadline = time.time() + 10.0
        while not hostile_circles(game.grab()) and time.time() < deadline:
            time.sleep(0.3)
        watch_fights("AR1007 entrance", AR1007_MAP)
        here = (2606, 874)
        for centre, point in AR1007_WEST:
            select_all(game, group_bar)
            here = step(centre, point, here, AR1007_MAP)
            watch_fights(f"AR1007 {point}", AR1007_MAP)
            game.screenshot(f"ar1007-{point[0]}-{point[1]}")  # not compared
        for centre, point in AR1007_SWEEP:
            if watch.frame is not None:
                break
            select_all(game, group_bar)
            here = step(centre, point, here, AR1007_MAP)
            watch_fights(f"AR1007 {point}", AR1007_MAP)
            game.screenshot(f"ar1007-{point[0]}-{point[1]}")  # not compared
        for centre, point in AR1007_NORTH:
            if watch.frame is not None:
                break
            select_all(game, group_bar)
            here = step(centre, point, here, AR1007_MAP)
            watch_fights(f"AR1007 {point}", AR1007_MAP)
            game.screenshot(f"ar1007-{point[0]}-{point[1]}")  # not compared
        time.sleep(3.0)
        game.grab()
    award = watch.frame
    if award is None:
        game.screenshot("no-award")
        game.press("q")  # a save to look at (copied to <out>/saves)
        time.sleep(4.0)
        raise AssertionError("the docks-cleared award never showed after the AR1007 sweep")
    check_frame("docks-cleared", award, AWARD_SPOT, max_diff=0.004)

    # The journal: the new entry at the bottom of the page.
    act(game, lambda: game.press("j"), "J (journal)", ACTION_BAR_REGION, park=FIGHT_PARK)
    page = game.wait_stable(settle=1.0, region=JOURNAL_TEXT)
    check_frame("journal-docks-cleared", journal_text(page), JOURNAL_TEXT, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the journal", ACTION_BAR_REGION, park=FIGHT_PARK)

    # --- Back to the docks -------------------------------------------------------
    for centre, point in AR1007_EAST:
        select_all(game, group_bar)
        here = step(centre, point, here, AR1007_MAP, fights=False)
    leave(s16.to_screen(view["real"], AR1007_EXIT), "click on the exit of AR1007")
    real = jump(AR1002_EXIT_VIEW, AR1002_MAP)
    leave(s16.to_screen(real, AR1002_EXIT), "click on the exit of AR1002")

    # --- Brogan ------------------------------------------------------------------
    real = jump(WAREHOUSE_VIEW)
    expected = s16.to_screen(real, BROGAN)
    deadline = time.time() + 20.0
    while (spot := s16.cyan_near(game.grab(), expected, dx=60, dy=50)) is None:
        if time.time() > deadline:
            raise AssertionError(f"Brogan's circle did not show near {expected}")
        time.sleep(0.5)
    for attempt in range(3):
        spot = s16.cyan_near(game.grab(), expected, dx=60, dy=50) or spot
        try:
            act(game, lambda: game.click(*spot), "click on Brogan", ACTION_BAR_REGION,
                park=DIALOG_PARK, wait=2.0, timeout=20.0)
            break
        except AssertionError:
            if attempt == 2:
                raise
    check_frame("brogan-01", speaker_block(game.wait_stable(settle=1.0, region=DIALOG_REGION)),
                BROGAN_BLOCK)
    for i, stp in enumerate(BROGAN_STEPS, start=2):
        if isinstance(stp, str):
            act(game, lambda: game.press(stp), f"reply {stp} to Brogan", DIALOG_REGION, park=DIALOG_PARK)
        else:
            act(game, lambda: game.click(*CONTINUE), "Continue with Brogan", DIALOG_REGION, park=DIALOG_PARK)
        check(f"brogan-{i:02d}", DIALOG_REGION)
    act(game, lambda: game.press("1"), "last reply to Brogan", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=2.0)

    # --- Quick save for S19 ------------------------------------------------------
    s17.quick_save(game, out_dir, check)
    if target == "original":
        (out_dir / "bank-ok").write_text("ok")

    print(f"S18 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
