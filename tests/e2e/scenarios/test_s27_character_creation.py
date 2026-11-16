"""S27: character creation. Main menu -> New Game -> "New Party" -> the Party
Formation screen -> one character made from scratch through every Character
Generation step, a second one made quickly and deleted, then the game is
started with the single character and her record, inventory and spellbook
are compared in Targos.

Abilities are point-buy (every score starts at a fixed value with "Ability
Points Left"; nothing is rolled), so every number on the way is compared.

mirel (slot 1), female drow evoker, chaotic neutral:

  gender      Gender -> Female -> Next: the portrait step; right, right, left
              (the second female portrait), Next
  race        Race -> Half-Orc (no subraces: its description) -> Elf (opens
              the Subrace list at once) -> Drow -> Back (the race list
              again) -> Elf -> Drow -> Next
  back        Back on the main menu undoes the race step (Race is the open
              step again, the summary only has the gender); the race is
              chosen again
  class       Class -> Fighter (no kits: its description) -> Cleric (opens
              the deity list at once), one picked, Back (the class list
              again) -> Wizard: the school list -> Evoker -> Next
  alignment   Alignment -> Chaotic Neutral -> Next
  abilities   Abilities: Intelligence + x8, Strength - x2; Next with points
              left asks "You have some unallocated points left": No;
              Dexterity + x4, Constitution + x6 (0 left: 8/16/14/20/10/12);
              the Wisdom label (description); Next
  skills      24 points: Alchemy + x4 and a fifth + (refused: rank cap),
              Concentration x4, Knowledge (Arcana) x4, Bluff x2, Bluff -,
              Bluff +, the list scrolled 6 rows, Spellcraft x4, Search x2
              (0 left); Next
  feats       2 feats: Combat Casting +, the Cleave label (description of a
              greyed feat), Armored Arcana + and -, Blind-Fight + (0 left);
              Next
  spells      Arcane Spell Selection, 5 spells: Burning Hands picked,
              unpicked, picked; Chromatic Orb, Ice Dagger, Shield, Sleep;
              Next: Memorize Arcane Spells, 4: Burning Hands, Chromatic Orb,
              Ice Dagger, Sleep; Next
  appearance  Appearance: Color Choices (the paper doll turns: masked); the
              hair swatch opens the colour grid, a colour; the major
              clothing swatch, a colour; Next: Sound: FEMALE_DROW_1, then
              FEMALE_WIZARD_2; Next
  biography   Biography: the default text; Clear; "ab" typed; Revert; Next
  summary     the summary pane scrolled to its end (skills, feats, spells)
  name        Name: the prompt (Next greyed), "mirel" typed key by key, Next;
              Finish

Party Formation: mirel in slot 1 with her portrait (Done enabled, Modify
Characters greyed). Her portrait opens Character Information (scrolled to
the end, Done). Her name button opens the Character popup (Create greyed,
Delete, Cancel): Cancel.

bor (slot 2), male gray dwarf paladin of Helm, lawful good: default portrait,
Dwarf -> Gray Dwarf, Paladin -> Paladin of Helm, the alignment list (only
Lawful Good enabled), the abilities left alone (Next, the prompt, Yes: 16
points unspent), Diplomacy + x4, one feat (the first row), default colours
and sound, the name. Party Formation shows both; bor's Character
Information; bor's name button -> Delete: slot 2 is empty again.

Done: loading (not compared), the Prologue title, Done: Hedron's opening
line and replies (text only). S5's replies end the dialogue. Then the action
bar of a lone wizard and her portrait's hit points, the record (and S8's
number regions), the inventory (S5's regions: a quarterstaff, the gold) and
the spellbook (the four memorised spells, five known).

On the original, Finish writes the character to the Characters folder
(MIREL.CHR / MIREL.RES, BOR.CHR / BOR.RES; a second MIREL would be MIREL00).
The `tidy` fixture copies them to the run's output folder and removes them,
and puts mpsave's Prologue autosave and "default" folder back as they were.

Not compared: the loading screen and its hint, the scrolling Prologue text,
the paper doll on the colour and inventory screens, the world view.

Coordinates (800x600). Party Formation: name button (530, 100 + 69 * slot),
portrait (668, same), Exit (163,507), Modify Characters (399,507), Done
(634,507); Character popup Delete (400,288), Cancel (400,319); Character
Information scroll down (642,465), Done (396,496). Character Generation:
step and list buttons at x 400, y 129 + 35 * k; Biography (93,562), Import
(262,562), Back (400,562), Next / Finish (535,562), Start Over (706,562);
gender Male (400,287), Female (400,483); portrait arrows (273,281) and
(525,281); ability rows y 170 + 36 * k with + at x 499 and - at x 518; skill
and feat rows y 169 + 36 * k, same x, list scroll down (535,500); spell icons
(293 + 43 * c, 179 + 43 * r); colour swatches x 293, y 335 / 375 / 442 / 482,
grid (330 + 28 * c, 176 + 28 * r); sound rows (340, 125 + 13 * k); biography
Revert (400,468), Clear (400,505); summary scroll down (770,474); name
prompt Back (338,268), Next (459,268); unallocated-points prompt Yes
(338,331), No (458,331)."""

from __future__ import annotations

import shutil
import time

import pytest

from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s1_boot import MAIN_MENU, boot_to_main_menu
from test_s2_new_game import CHAPTER_DONE, CHAPTER_TITLE_REGION, DIALOG_REGION, SELECT_PARTY, wait_region_changes
from test_s5_leave_dialog_inventory import (
    ACTION_BAR_REGION,
    DETAILS as INV_DETAILS,
    INV_AC_HP,
    INV_BACKPACK,
    INV_NAME_PORTRAIT,
    INV_QUICK_WEAPONS,
    finish_hedron_dialog,
)
from test_s7_hedron_branches import TEXT_REGION
from test_s8_screens_tour import DIAL, RECORD_DETAILS, SPELL_SLOTS
from test_s15_town import read_number

PARK = (796, 596)  # creation screens: the corner, off every control
WORLD_PARK = (300, 560)
FULL = (0, 0, 800, 600)
SCREEN_REGION = (0, 0, 800, 430)

# Party Formation.
SLOT = [(530, 100 + 69 * i) for i in range(6)]
SLOT_PORTRAIT = [(668, 100 + 69 * i) for i in range(6)]
FORMATION_DONE = (634, 507)
SLOTS_REGION = (415, 70, 290, 410)
FORMATION_BUTTONS = (95, 485, 610, 45)
POPUP_DELETE = (400, 288)
POPUP_CANCEL = (400, 319)
INFO_SCROLL = (642, 465)
INFO_DONE = (396, 496)
INFO_TEXT = (148, 120, 500, 352)

# Character Generation.
STEP = {name: (400, 129 + 35 * k) for k, name in enumerate(
    ["gender", "race", "class", "alignment", "abilities", "skills", "appearance", "name"])}


def row(k):
    """List button k of a race, class, kit or alignment list."""
    return (400, 129 + 35 * k)


BIOGRAPHY = (93, 562)
BACK = (400, 562)
NEXT = (535, 562)
MALE, FEMALE = (400, 287), (400, 483)
PORTRAIT_LEFT, PORTRAIT_RIGHT = (273, 281), (525, 281)
PLUS_X, MINUS_X = 499, 518
ABILITY_Y = {"str": 170, "dex": 206, "con": 242, "int": 278, "wis": 314, "cha": 350}
LIST_Y = [169 + 36 * k for k in range(10)]
LIST_SCROLL_DOWN = (535, 500)
SPELL = lambda c, r: (293 + 43 * c, 179 + 43 * r)  # noqa: E731
SWATCH = {"hair": (293, 335), "skin": (293, 375), "major": (293, 442), "minor": (293, 482)}
COLOUR = lambda c, r: (330 + 28 * c, 176 + 28 * r)  # noqa: E731
SOUND = lambda k: (340, 125 + 13 * k)  # noqa: E731
BIO_REVERT, BIO_CLEAR = (400, 468), (400, 505)
SUMMARY_SCROLL = (770, 474)
NAME_BACK, NAME_NEXT = (338, 268), (459, 268)
PROMPT_YES, PROMPT_NO = (338, 331), (458, 331)

# Parts of a creation screen, each compared on its own with a tight limit.
PORTRAIT_PANE = (22, 150, 212, 332)
CENTRE = (270, 110, 280, 400)
HELP = (567, 150, 195, 332)
BUTTONS = (30, 545, 740, 36)  # Biography, Import, Back, Next / Finish, Start Over
POINTS = (492, 115, 34, 32)  # Ability Points / Skill Points / Feats / Spells Left
ABILITY_VALUES = (412, 152, 76, 216)
SKILL_VALUES = (452, 152, 38, 360)
FEAT_DOTS = (440, 152, 50, 360)
LIST = (270, 152, 280, 360)
SPELL_GRID = (272, 158, 256, 180)
SWATCHES = (275, 315, 255, 185)
DOLL = (370, 160, 50, 70)  # the turning paper doll on Color Choices
SOUND_LIST = (277, 117, 232, 330)
NAME_EDIT = (283, 228, 235, 24)
BIO_FIRST_LINE = (277, 117, 232, 26)
NAME_POPUP = (262, 172, 276, 132)

# In the game.
RECORD_SCREEN = (0, 0, 800, 430)
PORTRAIT_HP = (507, 437, 32, 11)
DIALOG_GOLD = (745, 350, 45, 24)
INV_GOLD = (745, 140, 40, 22)


@pytest.fixture
def tidy(play_dir, out_dir):
    """Finish exports each new character to the Characters folder, and a new
    game overwrites mpsave's Prologue autosave and "default" folder. After
    the game has exited (this fixture is requested before `game`), keep the
    exported files with the run's output, remove them and put the saves
    back."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    chars = play_dir / "characters"
    before = {p.name for p in chars.iterdir()}
    keep = out_dir / "mpsave-before"
    saved = []
    for name in ("default", "000000000-Autosave - Prologue"):
        src = play_dir / "mpsave" / name
        if src.is_dir():
            shutil.copytree(src, keep / name)
            saved.append(name)
    yield
    made = out_dir / "characters"
    made.mkdir(exist_ok=True)
    for p in chars.iterdir():
        if p.name not in before:
            shutil.copy2(p, made / p.name)
            p.unlink()
    for name in saved:
        dst = play_dir / "mpsave" / name
        for _ in range(10):
            shutil.rmtree(dst, ignore_errors=True)
            if not dst.exists():
                break
            time.sleep(0.5)
        shutil.copytree(keep / name, dst)
    shutil.rmtree(keep, ignore_errors=True)


def test_s27_character_creation(tidy, game, golden_dir, out_dir, play_dir):
    t0 = time.time()
    failures = []

    def check(name, region=None, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    def check_frame(name, frame, region, **kw):
        stable = game.wait_stable
        game.wait_stable = lambda **_: frame
        try:
            check(name, region, **kw)
        finally:
            game.wait_stable = stable

    def panel(name, *parts, mask=None, whole=FULL):
        """One still frame: the whole screen at 2%, and each named part
        (suffix, region) at 0.4% (numbers: 0.1% or less, given per part)."""
        game.move(*PARK)
        frame = game.wait_stable(settle=0.4, region=whole)
        check_frame(name, frame, whole, mask=mask)
        for part in parts:
            suffix, region = part[0], part[1]
            limit = part[2] if len(part) > 2 else 0.004
            check_frame(f"{name}-{suffix}", frame, region, max_diff=limit)
        return frame

    def cg(name, *parts, **kw):
        """A Character Generation screen: the list or step buttons, the help
        box and the bottom buttons as parts."""
        return panel(name, ("centre", CENTRE), ("help", HELP), ("buttons", BUTTONS), *parts, **kw)

    def act(do, what, region=FULL, threshold=0.002, wait=0.2, park=PARK, timeout=10.0):
        """One input; it must change `region` (or the input was dropped)."""
        before = game.grab()
        do()
        if park:
            game.move(*park)
        deadline = time.time() + timeout
        while diff_ratio(before, game.grab(), region=region) <= threshold:
            if time.time() > deadline:
                raise AssertionError(f"{what} did not change the screen")
            time.sleep(0.15)
        time.sleep(wait)

    def click(xy, what, region=FULL, **kw):
        act(lambda: game.click(*xy), what, region, **kw)

    def step(xy, what):
        """A button that brings up another screen or list."""
        click(xy, what, FULL, threshold=0.005, wait=0.5)

    def tap(xy):
        """A click that may change nothing (the entry is already selected);
        the step after it proves the state."""
        game.click(*xy)
        game.move(*PARK)
        time.sleep(0.8)

    def point(xy, what):
        """A + or - that must change the points left."""
        click(xy, what, POINTS, threshold=0.004, wait=0.15, park=None)

    def type_name(text, region):
        for ch in text:
            act(lambda: game.press(ch), f"typing {ch!r}", region, threshold=0.002, wait=0.2)

    # --- Main menu -> Party Formation ----------------------------------------------
    boot_to_main_menu(game)
    step(MAIN_MENU["new_game"], "New Game")
    tap(SELECT_PARTY["new_party"])
    panel("select-new-party")
    step(SELECT_PARTY["done"], "Done on Select Party")
    panel("formation-empty", ("slots", SLOTS_REGION), ("buttons", FORMATION_BUTTONS))

    # --- mirel: gender and portrait ---------------------------------------------------
    step(SLOT[0], "Create Character in slot 1")
    cg("cg-start")
    step(STEP["gender"], "Gender")
    cg("gender")
    click(FEMALE, "Female", threshold=0.0005)
    cg("gender-female")
    step(NEXT, "Next after the gender")
    cg("portrait-1", ("portrait", PORTRAIT_PANE))
    click(PORTRAIT_RIGHT, "next portrait", threshold=0.01)
    click(PORTRAIT_RIGHT, "next portrait again", threshold=0.01)
    cg("portrait-3")
    click(PORTRAIT_LEFT, "previous portrait", threshold=0.01)
    cg("portrait-2")
    step(NEXT, "Next after the portrait")
    cg("main-gender-done", ("portrait", PORTRAIT_PANE))

    # --- race and subrace --------------------------------------------------------------
    step(STEP["race"], "Race")
    cg("race")
    click(row(5), "Half-Orc", HELP)  # a race without subraces: only its description
    cg("race-half-orc")
    step(row(2), "Elf")  # a race with subraces opens their list at once
    cg("subrace-elf")
    click(row(1), "Drow", HELP)
    cg("subrace-drow")
    step(BACK, "Back on the subrace list")
    cg("race-after-back")
    step(row(2), "Elf again")
    click(row(1), "Drow again", HELP)
    step(NEXT, "Next after Drow")
    cg("main-race-done")
    step(BACK, "Back on the main menu")
    cg("main-back-undoes-race")
    step(STEP["race"], "Race again")
    step(row(2), "Elf a third time")
    click(row(1), "Drow a third time", HELP)
    step(NEXT, "Next after Drow a third time")
    cg("main-race-done-again")

    # --- class: the cleric's deities, then wizard and school ----------------------
    step(STEP["class"], "Class")
    cg("class")
    click(row(4), "Fighter", HELP)  # a class without kits: only its description
    cg("class-fighter")
    step(row(2), "Cleric")  # a class with kits opens their list at once
    cg("cleric-deities")
    click(row(5), "a deity", HELP)
    cg("cleric-deity-picked")
    step(BACK, "Back on the deity list")
    cg("class-after-back")
    step(row(10), "Wizard")
    cg("wizard-schools")
    click(row(5), "Evoker", HELP)
    cg("wizard-evoker")
    step(NEXT, "Next after Evoker")
    cg("main-class-done")

    # --- alignment -------------------------------------------------------------------------
    step(STEP["alignment"], "Alignment")
    cg("alignment")
    click(row(5), "Chaotic Neutral", HELP)
    cg("alignment-chaotic-neutral")
    step(NEXT, "Next after the alignment")
    cg("main-alignment-done")

    # --- abilities (point-buy) -----------------------------------------------------------
    def ability(name, x=PLUS_X, times=1):
        for i in range(times):
            point((x, ABILITY_Y[name]), f"{name} {'+' if x == PLUS_X else '-'} ({i + 1})")

    def abilities(name):
        cg(name, ("points", POINTS), ("values", ABILITY_VALUES, 0.001))

    step(STEP["abilities"], "Abilities")
    abilities("abilities")
    ability("int", times=8)
    abilities("abilities-int-20")
    ability("str", MINUS_X, times=2)
    abilities("abilities-str-8")
    step(NEXT, "Next with ability points left")
    panel("abilities-unallocated-prompt")
    step(PROMPT_NO, "No on the unallocated points prompt")
    abilities("abilities-after-no")
    ability("dex", times=4)
    ability("con", times=6)
    abilities("abilities-spent")
    click((340, ABILITY_Y["wis"]), "the Wisdom label", HELP)
    abilities("abilities-wisdom-help")
    step(NEXT, "Next after the abilities")
    cg("main-abilities-done")

    # --- skills ------------------------------------------------------------------------------
    def skill(k, what, x=PLUS_X, times=1):
        for i in range(times):
            point((x, LIST_Y[k]), f"{what} {'+' if x == PLUS_X else '-'} ({i + 1})")

    def skills(name):
        cg(name, ("points", POINTS), ("values", SKILL_VALUES, 0.001))

    step(STEP["skills"], "Skills")
    skills("skills")
    skill(0, "Alchemy", times=4)
    skills("skills-alchemy-4")
    # A fifth rank is refused (the cap at level 1): nothing may change.
    game.click(PLUS_X, LIST_Y[0])
    game.move(*PARK)
    time.sleep(1.0)
    skills("skills-alchemy-capped")
    skill(3, "Concentration", times=4)
    skill(8, "Knowledge (Arcana)", times=4)
    skill(2, "Bluff", times=2)
    skills("skills-bluff-2")
    skill(2, "Bluff", MINUS_X)
    skills("skills-bluff-minus")
    skill(2, "Bluff")
    for i in range(6):
        click(LIST_SCROLL_DOWN, f"skill list down ({i + 1})", LIST, threshold=0.001, wait=0.15)
    skills("skills-scrolled")
    skill(7, "Spellcraft", times=4)
    skill(6, "Search", times=2)
    skills("skills-spent")
    step(NEXT, "Next after the skills")

    # --- feats ---------------------------------------------------------------------------------
    def feats(name):
        cg(name, ("points", POINTS), ("dots", FEAT_DOTS, 0.001))

    feats("feats")
    point((PLUS_X, LIST_Y[8]), "Combat Casting +")
    feats("feats-combat-casting")
    click((360, LIST_Y[7]), "the Cleave label", HELP)
    feats("feats-cleave-help")
    point((PLUS_X, LIST_Y[3]), "Armored Arcana +")
    feats("feats-armored-arcana")
    point((MINUS_X, LIST_Y[3]), "Armored Arcana -")
    feats("feats-armored-arcana-minus")
    point((PLUS_X, LIST_Y[5]), "Blind-Fight +")
    feats("feats-spent")
    step(NEXT, "Next after the feats")

    # --- spells ----------------------------------------------------------------------------------
    def spells(name):
        cg(name, ("points", POINTS), ("grid", SPELL_GRID, 0.001))

    spells("spells")
    point(SPELL(0, 0), "Burning Hands")
    spells("spells-burning-hands")
    point(SPELL(0, 0), "Burning Hands off")
    spells("spells-burning-hands-off")
    point(SPELL(0, 0), "Burning Hands on")
    for c, r, what in [(3, 0, "Chromatic Orb"), (5, 0, "Ice Dagger"), (0, 2, "Shield"), (2, 2, "Sleep")]:
        point(SPELL(c, r), what)
    spells("spells-chosen")
    step(NEXT, "Next after the spells")
    spells("memorize")
    for c, what in [(0, "Burning Hands"), (1, "Chromatic Orb"), (2, "Ice Dagger"), (4, "Sleep")]:
        point(SPELL(c, 0), f"memorize {what}")
    spells("memorize-chosen")
    step(NEXT, "Next after memorizing")
    cg("main-skills-done")

    # --- appearance: colours and sound ---------------------------------------------------
    def colours(name):
        # Not cg(): its centre part would include the turning doll.
        panel(name, ("help", HELP), ("buttons", BUTTONS), ("swatches", SWATCHES, 0.001), mask=[DOLL])

    step(STEP["appearance"], "Appearance")
    colours("colours")
    step(SWATCH["hair"], "the hair swatch")
    panel("colours-hair-grid")
    step(COLOUR(3, 0), "a hair colour")
    colours("colours-hair-picked")
    step(SWATCH["major"], "the major clothing swatch")
    panel("colours-major-grid")
    step(COLOUR(4, 1), "a clothing colour")
    colours("colours-major-picked")
    step(NEXT, "Next after the colours")
    cg("sound", ("list", SOUND_LIST))
    click(SOUND(2), "FEMALE_DROW_1", SOUND_LIST, threshold=0.001)
    cg("sound-drow", ("list", SOUND_LIST))
    click(SOUND(14), "FEMALE_WIZARD_2", SOUND_LIST, threshold=0.001)
    cg("sound-wizard-2", ("list", SOUND_LIST))
    step(NEXT, "Next after the sound")
    cg("main-appearance-done")

    # --- biography ---------------------------------------------------------------------------------
    step(BIOGRAPHY, "Biography")
    cg("biography")
    click(BIO_CLEAR, "Clear", CENTRE, threshold=0.01)
    cg("biography-cleared")
    type_name("ab", BIO_FIRST_LINE)
    cg("biography-typed")
    click(BIO_REVERT, "Revert", CENTRE, threshold=0.01)
    cg("biography-reverted")
    step(NEXT, "Next after the biography")

    # --- the summary, to its end ---------------------------------------------------------------
    before = game.grab()
    for _ in range(12):
        game.click(*SUMMARY_SCROLL)
    game.move(*PARK)
    time.sleep(0.5)
    if diff_ratio(before, game.grab(), region=HELP) <= 0.01:
        raise AssertionError("the summary did not scroll")
    cg("summary-middle")
    for _ in range(20):
        game.click(*SUMMARY_SCROLL)
    game.move(*PARK)
    cg("summary-end")

    # --- name, Finish ------------------------------------------------------------------------------
    step(STEP["name"], "Name")
    panel("name-prompt", ("popup", NAME_POPUP))
    type_name("mirel", NAME_EDIT)
    panel("name-typed", ("popup", NAME_POPUP), ("edit", NAME_EDIT))
    step(NAME_NEXT, "Next on the name prompt")
    cg("main-named")
    step(NEXT, "Finish")
    panel("formation-mirel", ("slots", SLOTS_REGION), ("buttons", FORMATION_BUTTONS))
    print(f"S27 first character took {time.time() - t0:.0f} s")

    # --- Party Formation: information, the Character popup ------------------------------
    step(SLOT_PORTRAIT[0], "mirel's portrait")
    panel("info-mirel", ("text", INFO_TEXT))
    before = game.grab()
    for _ in range(12):
        game.click(*INFO_SCROLL)
    game.move(*PARK)
    time.sleep(0.5)
    if diff_ratio(before, game.grab(), region=INFO_TEXT) <= 0.01:
        raise AssertionError("Character Information did not scroll")
    panel("info-mirel-end", ("text", INFO_TEXT))
    step(INFO_DONE, "Done on Character Information")
    step(SLOT[0], "mirel's name button")
    panel("popup-mirel")
    step(POPUP_CANCEL, "Cancel on the Character popup")
    panel("formation-after-cancel", ("slots", SLOTS_REGION))

    # --- bor: a gray dwarf paladin of Helm in slot 2 -----------------------------------------
    step(SLOT[1], "Create Character in slot 2")
    step(STEP["gender"], "Gender (bor)")
    click(MALE, "Male", threshold=0.0005)
    step(NEXT, "Next after the gender (bor)")
    cg("bor-portrait", ("portrait", PORTRAIT_PANE))
    step(NEXT, "Next after the portrait (bor)")
    step(STEP["race"], "Race (bor)")
    step(row(1), "Dwarf")
    cg("bor-subrace")
    click(row(2), "Gray Dwarf", HELP)
    cg("bor-subrace-gray")
    step(NEXT, "Next after Gray Dwarf")
    step(STEP["class"], "Class (bor)")
    step(row(6), "Paladin")
    cg("bor-orders")
    click(row(1), "Paladin of Helm", HELP)
    cg("bor-order-helm")
    step(NEXT, "Next after Paladin of Helm")
    step(STEP["alignment"], "Alignment (bor)")
    cg("bor-alignment")
    click(row(0), "Lawful Good", HELP)
    cg("bor-alignment-lawful-good")
    step(NEXT, "Next after the alignment (bor)")
    step(STEP["abilities"], "Abilities (bor)")
    abilities("bor-abilities")
    step(NEXT, "Next with all ability points left")
    step(PROMPT_YES, "Yes on the unallocated points prompt")
    cg("bor-main-abilities-done")
    step(STEP["skills"], "Skills (bor)")
    skills("bor-skills")
    skill(4, "Diplomacy (bor)", times=4)
    skills("bor-skills-spent")
    step(NEXT, "Next after the skills (bor)")
    feats("bor-feats")
    point((PLUS_X, LIST_Y[0]), "the first feat (bor)")
    feats("bor-feats-spent")
    step(NEXT, "Next after the feats (bor)")
    cg("bor-main-skills-done")
    step(STEP["appearance"], "Appearance (bor)")
    colours("bor-colours")
    step(NEXT, "Next after the colours (bor)")
    cg("bor-sound", ("list", SOUND_LIST))
    step(NEXT, "Next after the sound (bor)")
    step(STEP["name"], "Name (bor)")
    type_name("bor", NAME_EDIT)
    step(NAME_NEXT, "Next on the name prompt (bor)")
    cg("bor-main-named")
    step(NEXT, "Finish (bor)")
    panel("formation-two", ("slots", SLOTS_REGION), ("buttons", FORMATION_BUTTONS))
    step(SLOT_PORTRAIT[1], "bor's portrait")
    panel("info-bor", ("text", INFO_TEXT))
    step(INFO_DONE, "Done on Character Information (bor)")
    step(SLOT[1], "bor's name button")
    step(POPUP_DELETE, "Delete on the Character popup")
    panel("formation-bor-deleted", ("slots", SLOTS_REGION), ("buttons", FORMATION_BUTTONS))
    print(f"S27 creation took {time.time() - t0:.0f} s")

    # What Finish wrote to the Characters folder.
    made = sorted(p.name.upper() for p in (play_dir / "characters").iterdir()
                  if p.name.upper().startswith(("MIREL", "BOR")))
    print(f"S27 exported characters: {made}")
    if made != ["BOR.CHR", "BOR.RES", "MIREL.CHR", "MIREL.RES"]:
        failures.append(f"Characters folder after two Finishes: {made}")

    # --- start the game ------------------------------------------------------------------------
    game.click(*FORMATION_DONE)
    game.move(*WORLD_PARK)
    time.sleep(1.5)
    wait_region_changes(game, CHAPTER_TITLE_REGION, timeout=120)
    check("prologue-title", CHAPTER_TITLE_REGION, settle=2.0)
    game.click(*CHAPTER_DONE)
    game.move(*WORLD_PARK)
    wait_region_changes(game, DIALOG_REGION, timeout=120)
    frame = game.wait_stable(settle=2.0, region=TEXT_REGION, timeout=90)
    check_frame("targos-opening-dialog", frame, TEXT_REGION)
    gold_dialog = read_number(frame, DIALOG_GOLD)

    finish_hedron_dialog(game)
    game.move(*WORLD_PARK)
    check("world-action-bar", ACTION_BAR_REGION)
    check("world-portrait-hit-points", PORTRAIT_HP, settle=0.5, max_diff=0.004)

    def world(do, what):
        act(do, what, ACTION_BAR_REGION, threshold=0.1, wait=2.0, park=WORLD_PARK)

    world(lambda: game.click(*DIAL["record"]), "Record dial button")
    frame = game.wait_stable(settle=1.0, region=RECORD_SCREEN)
    check_frame("record", frame, RECORD_SCREEN)
    for name, region in RECORD_DETAILS.items():
        check_frame("record-" + name, frame, region, max_diff=0.004)
    world(lambda: game.press("esc"), "Esc on the record")

    world(lambda: game.click(*DIAL["inventory"]), "Inventory dial button")
    frame = game.wait_stable(settle=1.0, region=INV_NAME_PORTRAIT)
    check_frame("inventory-name-portrait", frame, INV_NAME_PORTRAIT)
    check_frame("inventory-quick-weapons", frame, INV_QUICK_WEAPONS)
    check_frame("inventory-ac-hp", frame, INV_AC_HP)
    check_frame("inventory-backpack", frame, INV_BACKPACK)
    for name in ("armor-class", "hit-points", "weight", "selected-quick-weapon"):
        check_frame("inventory-" + name, frame, INV_DETAILS[name], max_diff=0.004)
    gold_inventory = read_number(frame, INV_GOLD)
    print(f"S27 gold: {gold_dialog} in the dialogue, {gold_inventory} in the inventory")
    if gold_dialog is None or gold_dialog != gold_inventory:
        failures.append(f"gold reads {gold_dialog} in the dialogue and {gold_inventory} in the inventory")
    world(lambda: game.press("esc"), "Esc on the inventory")

    world(lambda: game.click(*DIAL["spellbook"]), "Spellbook dial button")
    frame = game.wait_stable(settle=1.0, region=SCREEN_REGION)
    check_frame("spellbook", frame, SCREEN_REGION)
    check_frame("spellbook-slots", frame, SPELL_SLOTS, max_diff=0.004)
    world(lambda: game.press("esc"), "Esc on the spellbook")
    check("world-after-screens", ACTION_BAR_REGION)

    print(f"S27 took {time.time() - t0:.0f} s after the main menu")
    assert not failures, "; ".join(failures)
