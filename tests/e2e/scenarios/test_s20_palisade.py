"""S20: the character record after the docks, a new spell memorised and
rested in at the Weeping Widow, the walk to the Palisade (AR1200) and Shawford
Crale's first dialogue (AR1201), and two spells cast there. Starts from S19's
banked save and banks one of its own end state.

Saves (golden/bank/, git-ignored like all goldens):

  s19-end/  S19's end (see test_s19_town_errands), loaded here: the party in
            the Town Hall (AR1101) after Ulbrec (Ulbrec_Gate = 1), 123 gold,
            whole party selected
  s20-end/  this test's end (on the original only): on the Palisade (AR1200)
            outside Shawford's house after his dialogue (Lumber_Quest = 1:
            find Olap), 120 gold plus a 250-gold pile in Sheris's backpack,
            Laeris's Armor of Faith and one of Vraedna's level-1 casts used,
            whole party selected

What the Prologue offers here, and what is left for S21:

  - Nobody can level up: the docks left everyone at 563-600 experience (1000
    for level 2; Vraedna, a drow with an effective character level of 3, at
    5563 of 6000). So the record pages and their experience numbers are
    compared instead; the Level Up panels wait for a later scenario (S21:
    Olap's lumber; Shawford pays it with AddXpVar("Level_2_Average")).
  - No rest is allowed in the Town Hall, in the town streets or on the
    Palisade ("You may not rest at this time."); the inn is the only place.
  - Shawford's first dialogue does not start the Palisade battle: that comes
    from his dialogue only after Olap's lumber, Isherwood's arrows, Koluhm and
    the Iron Collar errands. There is no hostile creature in reach, so the
    offensive spell is Vraedna's Sleep cast on empty ground (a targeted area
    spell; Magic Missile needs a creature). Casting prints no message line on
    the original (S11 found the same), so the casts are proved by the spell
    counts on the action bar before and after.
  - Shawford's GiveItemCreate("Misc07", 250) puts a 250-gold pile in the
    protagonist's (Sheris's) backpack; the party's gold stays at 120.

test_s20_palisade (both targets) loads the s19-end save the way S19 loads
S18's, then, with the whole party selected:

  record      the Record dial button: Sheris's record; then the portraits of
              Geldin, Khatja, Laeris, Vraedna and Kylie. For each, the record
              screen above the portraits (2%) and the Experience block
              (heading, Current, Next Level) at 0.4%; Sheris's Level Up
              button (disabled for everyone); Esc
  memorise    the Spellbook dial button (Sheris's page), Laeris's portrait:
              her page (Cure Light Wounds 1/1, Summon Monster I 1/1); click
              Summon Monster I: the remove prompt; Remove (slots 0 -> 1);
              Armor of Faith in the known list: memorised 0/1 (slots 0); Esc
  inn         out of the Town Hall (the loading screen is not compared), south
              down the gully path past the depot to the Weeping Widow; door,
              doorway (AR1104). Click Cahl-Hyred: "All hail, the sell-sword
              of Targos 'as returned!" and four replies (text only, as S19);
              3 ("Let me see what rooms you have available."): the Rooms
              panel (gold 123 read and compared); Rent: the rest prompt;
              Rest: "You have rested for N hours." (the words without the
              hours; the party had every hit point, so no Healed lines). The
              spellbook (Sheris's page), Laeris: Armor of Faith now 1/1; Esc;
              leave by the exit
  palisade    north through the town to the Palisade gate (AR1100's DOOR6,
              closed: the party opens it), the gateway: AR1200. Centre the
              view on Shawford's house (AR1201), click its door (it opens),
              then the doorway
  Shawford    click him: his first line and replies (text only), replies 1,
              Continue x4, 1, Continue, Continue (each panel compared; the
              last shows "Your journal has been updated."), 1 ends it
  inventory   the Inventory dial button (Sheris): the backpack with the
              250-gold pile, the weight (34 / 90 lb.: the pile weighs
              nothing), the gold label (120, read); Esc
  spells      leave AR1201 (back in AR1200 by the door). Laeris: Cast Spell,
              Cleric: Armor of Faith 1 and Cure Light Wounds 1 (after the rest
              Armor of Faith comes first; compare the bar); Armor of Faith on
              her own portrait; the cleric list again: only Cure Light Wounds
              (compare). Vraedna: Cast Spell: Magic
              Missile 4, Sleep 4 (compare); Sleep on empty ground west of the
              party; Cast Spell again: 3 and 3 (sorcerer casts share the
              level's slots; compare)
  journal     J: the page's text without the yellow day/hour headings
  save        Q: compare the save lines; on the original the save is banked

Not compared: the world view (snow, day/night: the rest moves the clock to
night, people wandering), the loading screens and their hints, the overhead
lines of Cahl-Hyred, Lumbar, Oswald, Ghille-Laider, the soldiers and trainees
(they print into the message window at random times), the hours of the rest,
the journal's headings, the dialogue panel's gold label.

Map points (area maps at 1/8 scale, see S19): AR1100 origin (158,85), AR1200
origin (99,40). AR1101 (the view on entering, as S19 leaves it): Ulbrec 493,430
at screen (452,112); exit To1100 296-422,561-746 (screen (319,382)); arrival
outside FR1101 2180,1354. AR1100: inn door DOOR2 (closed again in the bank),
the gate DOOR6 1573-1683,168-277 (To1200 1574-1682,170-275; a rain barrel at
1620-1641,766-797 on the way north opens its container panel if clicked).
AR1200: arrival FR1100 2373,2682; Shawford's house door HOUSE2 2702-2728,
2456-2547 (To1201 2703-2727,2460-2544); arrival back FR1201 2535,2540; the
training keg and trainees at 2200-2400,2130-2250 (neutral). AR1201 (view on
entering: screen = map - (1,140)): Shawford 496,268, Nolan 593,158, exit
To1200 56-210,362-519. AR1200SR/AR1100SR give the walkable cells (sr.py in
scratchpad s19).
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
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION, INV_BACKPACK
from test_s8_screens_tour import PORTRAIT, SPELL_SLOTS
from test_s9_docks import AREA_MAP_BUTTON, AREA_MAP_REGION, DIALOG_PARK, DIALOG_REGION, SELECT_ALL, act
from test_s14_travel import select_all, travel
from test_s15_town import GOLD_LABEL, STORE_PARK, STORE_REGION, read_number

S19_END = s19.S19_END
S20_END = s17.BANK / "s20-end"

FIGHT_PARK = s17.FIGHT_PARK  # (520, 515): off the world view
SCREEN_PARK = (300, 560)  # the message window, on full screens
MESSAGES = s17.MESSAGES
JOURNAL_TEXT = s18.JOURNAL_TEXT
WORLD_REGION = (0, 0, 800, 430)

AR1100_MAP = (158, 85)
AR1200_MAP = (99, 40)

# Dial buttons.
SPELLBOOK = (620, 527)
INVENTORY = (656, 525)
RECORD = (656, 569)

SHERIS, LAERIS, VRAEDNA = PORTRAIT[0], PORTRAIT[3], PORTRAIT[4]
NAMES = ["sheris", "geldin", "khatja", "laeris", "vraedna", "kylie"]

# Record screen.
RECORD_SCREEN = (0, 0, 800, 430)  # everything above the portraits
RECORD_TEXT = (403, 63, 358, 292)  # the text pane (proves a portrait click)
# "Experience / Current: N / Next Level: N": Vraedna's has one line more
# above it (Effective Character Level).
XP_BLOCK = {name: (405, 144 if name == "vraedna" else 131, 150, 40) for name in NAMES}
LEVEL_UP = (655, 378, 118, 28)

# Spellbook (S11).
MEMORISED_2 = (384, 95)  # Summon Monster I
REMOVE_PROMPT = (255, 135, 285, 155)
REMOVE_BUTTON = (455, 258)
KNOWN_FIRST = (615, 95)  # Armor of Faith
MEMORISED_REGION = (250, 75, 232, 320)
BOOK_PAGE = (250, 20, 475, 395)  # memorised grid, known list, class tabs

# Action bar casting.
CAST_SPELL = (186, 463)  # Laeris and Vraedna: the fifth button
CLASS_CLERIC = (26, 463)
SPELL_ICONS = [(26, 463), (67, 463)]
CAST_BAR = (0, 440, 500, 48)

# Inventory: the gold label (number right of the coins) and the weight.
INV_GOLD = (700, 138, 85, 24)
INV_WEIGHT = (340, 280, 120, 16)

# Walks: (map point the view is centred on, map point walked to).
HALL_TO_INN = [
    ((2100, 1300), (2088, 1302)),  # west of the Town Hall door
    ((2072, 1450), (2072, 1450)),  # south down the gully path
    ((2072, 1650), (2072, 1650)),
    ((2072, 1800), (2072, 1850)),
    ((1900, 1900), (1850, 1940)),  # west along the cliff edge
    ((1650, 1900), (1650, 1940)),
    ((1500, 1900), (1496, 1930)),  # in front of the depot
    ((1300, 1800), (1250, 1800)),  # S19's route the other way
    ((1080, 1750), (1080, 1700)),
    ((800, 1750), (790, 1790)),  # in front of the inn
]
OUTSIDE_HALL = (2180, 1354)
INN_TO_GATE = [((x, y - 40), (x, y)) for x, y in [
    (1030, 1760), (1190, 1698), (1288, 1610), (1288, 1480), (1400, 1434), (1470, 1320),
    (1480, 1180), (1496, 1030), (1560, 930), (1600, 840), (1612, 720), (1620, 610),
    (1620, 500), (1620, 380)]]
GATE_VIEW = (1620, 340)
GATE_DOOR = (1630, 230)
GATEWAY = (1638, 209)
HOUSE_VIEW = (2600, 2550)
HOUSE_DOOR = (2712, 2483)
HOUSE_DOORWAY = (2707, 2475)
CAST_VIEW = (2480, 2500)
SLEEP_SPOT = (2230, 2495)  # empty snow west of the party, away from everyone

# Inside, as the view shows each area on entering.
HALL_EXIT = (319, 382)
SHAWFORD = (495, 128)
HOUSE_EXIT = (129, 300)

SHAWFORD_STEPS = ["1", CONTINUE, CONTINUE, CONTINUE, CONTINUE, "1", CONTINUE, CONTINUE]

REST_WORDS = s16.REST_WORDS  # "You have rested for" without the hours


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s19_save(play_dir, out_dir, target):
    """Put the banked S19 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S19_END / "save").iterdir() if p.is_dir()] if (S19_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S19 save in {S19_END}; run test_s19_town_errands --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    if target == "original":
        S20_END.mkdir(parents=True, exist_ok=True)
    yield
    s17.finish_saves(play_dir, out_dir, S20_END if target == "original" else None)


# --- finding things on screen --------------------------------------------------


def replies_shown(img):
    """The dialogue panel shows red reply lines (the first panel can be
    empty for a few seconds while the speaker walks over)."""
    x, y, w, h = s19.SPEECH_AREA
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[y:y + h, x:x + w]
    red = (a[..., 0] > 200) & (a[..., 1] < 80) & (a[..., 2] < 80)
    return int(red.sum()) > 200


# --- the test ------------------------------------------------------------------


def test_s20_palisade(from_s19_save, game, golden_dir, out_dir, target):
    group_bar = s17.load_bank(game, S19_END)
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

    def expect_gold(where, region, want):
        value = read_number(game.grab(), region)
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
        (a click where the party stands selects a member)."""
        select_all(game, group_bar)
        game.click(*s16.to_screen(real, door))
        game.move(*FIGHT_PARK)
        time.sleep(7.0)
        try:
            go(s16.to_screen(real, doorway), what)
        except AssertionError:
            # Once on the original the party was still on its way and the
            # doorway click did nothing: the door was open a moment later.
            time.sleep(3.0)
            go(s16.to_screen(real, doorway), what + " (again)")

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
                    park=DIALOG_PARK, wait=1.0, timeout=20.0)
                break
            except AssertionError:
                if attempt == 2:
                    raise
        deadline = time.time() + 20.0
        while not replies_shown(game.grab()):
            if time.time() > deadline:
                raise AssertionError(f"{who}'s first line and replies did not show")
            time.sleep(0.3)

    def first_panel(name):
        check_frame(name, s19.speech_block(game.wait_stable(settle=1.0, region=DIALOG_REGION)),
                    s19.SPEECH_SPOT)

    def click(xy, region, what, park=SCREEN_PARK, **kw):
        act(game, lambda: game.click(*xy), what, region, park=park, **kw)

    # --- The record ---------------------------------------------------------------
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

    # --- A new spell for Laeris ------------------------------------------------
    click(SPELLBOOK, ACTION_BAR_REGION, "Spellbook dial button", threshold=0.3)
    click(LAERIS, BOOK_PAGE, "Laeris's portrait in the spellbook")
    check("book-laeris", BOOK_PAGE)
    click(MEMORISED_2, WORLD_REGION, "the memorised Summon Monster I")
    check("book-remove-prompt", REMOVE_PROMPT)
    click(REMOVE_BUTTON, WORLD_REGION, "Remove")
    check("book-removed", MEMORISED_REGION)
    tight("book-removed-slots", SPELL_SLOTS)
    click(KNOWN_FIRST, MEMORISED_REGION, "Armor of Faith in the known list")
    check("book-armor-of-faith", MEMORISED_REGION)
    tight("book-armor-of-faith-slots", SPELL_SLOTS)
    act(game, lambda: game.press("esc"), "Esc on the spellbook", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)

    # --- A room at the Weeping Widow -----------------------------------------
    go(HALL_EXIT, "click on the exit of the Town Hall")
    here, real = walk(HALL_TO_INN, OUTSIDE_HALL, AR1100_MAP)
    enter(s19.INN_DOOR, s19.INN_DOORWAY, real, "click on the doorway of the Weeping Widow")
    talk(s19.CAHL, "Cahl-Hyred", dx=25, dy=12)
    first_panel("cahl-01")
    act(game, lambda: game.press("3"), "reply 3 to Cahl-Hyred (rooms)", DIALOG_REGION,
        park=STORE_PARK, wait=2.0, threshold=0.3)
    check("rooms", STORE_REGION)
    tight("rooms-gold", GOLD_LABEL)
    expect_gold("the Rooms panel", GOLD_LABEL, 123)
    click(s19.RENT, s19.REST_PROMPT, "Rent", park=STORE_PARK)
    check("rooms-rest-prompt", s19.REST_PROMPT)
    act(game, lambda: game.click(*s19.REST_CONFIRM), "Rest on the prompt", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=0.0, threshold=0.3, timeout=30.0)
    # The rest line comes at once; Cahl-Hyred's overhead lines follow every
    # few seconds and scroll it up.
    deadline = time.time() + 5.0
    line = None
    while line is None and time.time() < deadline:
        line = s16.last_yellow_line(game.grab())
        time.sleep(0.3)
    check_frame("rested-line", line, REST_WORDS, max_diff=0.004)
    click(SPELLBOOK, ACTION_BAR_REGION, "Spellbook dial button", threshold=0.3)
    try:
        click(LAERIS, BOOK_PAGE, "Laeris's portrait in the spellbook", timeout=4.0)
    except AssertionError:
        pass  # the spellbook opened on her page
    check("book-rested", MEMORISED_REGION)
    tight("book-rested-slots", SPELL_SLOTS)
    act(game, lambda: game.press("esc"), "Esc on the spellbook", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)
    go(s19.INN_EXIT, "click on the exit of the Weeping Widow")

    # --- To the Palisade ---------------------------------------------------------
    here, real = walk(INN_TO_GATE, s19.OUTSIDE_INN, AR1100_MAP)
    real = jump(GATE_VIEW, AR1100_MAP)
    enter(GATE_DOOR, GATEWAY, real, "click on the Palisade gateway")
    real = jump(HOUSE_VIEW, AR1200_MAP)
    enter(HOUSE_DOOR, HOUSE_DOORWAY, real, "click on the doorway of Shawford's house")

    # --- Shawford Crale -------------------------------------------------------------
    talk(SHAWFORD, "Shawford")
    first_panel("shawford-01")
    for i, stp in enumerate(SHAWFORD_STEPS, start=2):
        if isinstance(stp, str):
            act(game, lambda: game.press(stp), f"reply {stp} to Shawford", DIALOG_REGION,
                park=DIALOG_PARK)
        else:
            act(game, lambda: game.click(*CONTINUE), "Continue with Shawford", DIALOG_REGION,
                park=DIALOG_PARK)
        check(f"shawford-{i:02d}", DIALOG_REGION)
    act(game, lambda: game.press("1"), "last reply to Shawford", ACTION_BAR_REGION,
        park=FIGHT_PARK, wait=2.0)

    # --- The pay: a gold pile in Sheris's backpack -------------------------------
    select_all(game, group_bar)
    click(INVENTORY, ACTION_BAR_REGION, "Inventory dial button", threshold=0.3)
    check("inventory-backpack", INV_BACKPACK)
    tight("inventory-weight", INV_WEIGHT)
    tight("inventory-gold", INV_GOLD)
    expect_gold("the inventory", INV_GOLD, 120)
    act(game, lambda: game.press("esc"), "Esc on the inventory", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)

    # --- Two spells on the Palisade ---------------------------------------------
    go(HOUSE_EXIT, "click on the exit of Shawford's house")
    real = jump(CAST_VIEW, AR1200_MAP)
    # Laeris: Armor of Faith on herself.
    click(LAERIS, CAST_BAR, "Laeris's portrait", park=FIGHT_PARK, threshold=0.05)
    click(CAST_SPELL, CAST_BAR, "Cast Spell (Laeris)", park=FIGHT_PARK)
    click(CLASS_CLERIC, CAST_BAR, "Cleric", park=FIGHT_PARK)
    check("cast-laeris-before", CAST_BAR, max_diff=0.004)
    click(SPELL_ICONS[0], CAST_BAR, "Armor of Faith", park=FIGHT_PARK)
    game.click(*LAERIS)
    game.move(*FIGHT_PARK)
    time.sleep(4.0)
    click(CAST_SPELL, CAST_BAR, "Cast Spell (Laeris again)", park=FIGHT_PARK)
    click(CLASS_CLERIC, CAST_BAR, "Cleric", park=FIGHT_PARK)
    check("cast-laeris-after", CAST_BAR, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the cleric list", CAST_BAR, park=FIGHT_PARK)
    # Vraedna: Sleep on empty ground.
    click(VRAEDNA, CAST_BAR, "Vraedna's portrait", park=FIGHT_PARK, threshold=0.05)
    click(CAST_SPELL, CAST_BAR, "Cast Spell (Vraedna)", park=FIGHT_PARK)
    check("cast-vraedna-before", CAST_BAR, max_diff=0.004)
    click(SPELL_ICONS[1], CAST_BAR, "Sleep", park=FIGHT_PARK)
    game.click(*s16.to_screen(real, SLEEP_SPOT))
    game.move(*FIGHT_PARK)
    time.sleep(4.0)
    click(CAST_SPELL, CAST_BAR, "Cast Spell (Vraedna again)", park=FIGHT_PARK)
    check("cast-vraedna-after", CAST_BAR, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the spell list", CAST_BAR, park=FIGHT_PARK)
    select_all(game, group_bar)

    # --- The journal ------------------------------------------------------------------
    act(game, lambda: game.press("j"), "J (journal)", ACTION_BAR_REGION, park=FIGHT_PARK)
    page = game.wait_stable(settle=1.0, region=JOURNAL_TEXT)
    check_frame("journal-shawford", s18.journal_text(page), JOURNAL_TEXT, max_diff=0.004)
    act(game, lambda: game.press("esc"), "Esc on the journal", ACTION_BAR_REGION, park=FIGHT_PARK)

    # --- Quick save for S21 ------------------------------------------------------
    s17.quick_save(game, out_dir, check)
    if target == "original":
        (out_dir / "bank-ok").write_text("ok")

    print(f"S20 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
