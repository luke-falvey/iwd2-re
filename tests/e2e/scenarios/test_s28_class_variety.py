"""S28: class variety. A party of six made from scratch, one class each that no
earlier scenario has created, then the game is started and every member's
action bar, class sub-bars, record, inventory and spellbook are compared.

Each creation takes the shortest path (default portrait or one or two to the
right, abilities left alone with the "unallocated points" prompt answered
Yes, default colours and sound) and compares the panels that depend on the
class or race:

  ugo   male half-orc barbarian, true neutral. Skills: Intimidate x4 (4
        points: Intelligence 8). Feats: list down 10, Extra Rage.
  ela   female half-elf bard, chaotic neutral. Bluff x4, Diplomacy x4.
        Armored Arcana. (A level 1 bard has no spell panel.)
  dom   male shield dwarf cleric, Morninglord of Lathander, neutral good.
        Alchemy x1 (cross-class: 2 points), Concentration x2. His feat list
        starts at Blind-Fight (no Armor Proficiency, Armored Arcana or
        Arterial Strike rows): Combat Casting. Memorize Divine Spells
        (Armor of Faith), Memorize Domain Spells (Sunscorch).
  wyn   female wild elf druid, true neutral. Animal Empathy x4. Courteous
        Magocracy. Memorize Divine Spells (Cure Light Wounds).
  kato  male human monk: the order list, Dark Moon looked at, Old Order
        taken; lawful neutral. Concentration x4, Hide x4, Move Silently x2
        (10 points). Two feats: Blind-Fight, Bullheaded.
  fin   female rock gnome ranger, neutral good. Animal Empathy x4, Hide x4.
        Dash. Favored Enemy list: Goblins.

Per character: race or subrace list, class description or kit
list, the alignment list (which rows are greyed), abilities (racial
modifiers), skills (costs in brackets, points left, values; the end of the
list after a click on the scrollbar track), feats (which are lit, the dots),
the class's own panels, the colours (paper doll masked), the summary's first
and last page, the name, Party Formation.

In Targos, after Hedron's dialogue (S5's replies):

  bars      each member selected alone: the action bar (as a whole and button
            by button) and the portrait's hit points; then the sub-bars:
            ugo Special Abilities (Barbarian Rage); ela Bard Songs; dom Cast
            Spell -> Cleric / Domain lists, Special Abilities (Lathander's
            Renewal, Turn Undead); wyn Cast Spell; kato Special Abilities
            (Stunning Attack). fin has no sub-bar at level 1.
  record    for each: the first page (S8's number regions and the text line
            by line), the combat page and the skills and feats page
  inventory for each: S5's regions; the gold is read as digits and must
            equal the amount in Hedron's dialogue
  spellbook ela (empty), dom (Cleric and Domain pages), wyn
  abilities ugo uses Barbarian Rage: the record (Str 16, Con 14, 14 hit
            points, Current Effects) and the Special Abilities bar after it;
            kato arms Stunning Attack: the record's Current Effects; ela
            starts and stops the Ballad of Three Heroes: the action bar.

Left out: rogue and sorcerer (a seventh and eighth character would take the
run past 15 minutes). The stealth button (the Hide roll is random), the
message window (dock workers' overhead lines land in it at random), the
loading screen, the paper dolls, the world view.

Finish exports each character to the Characters folder; S27's `tidy` fixture
removes them and restores the saves.

New coordinates (800x600). Creation: a click on a scrollbar's track jumps the
list there: skill list end (535,470), summary end (770,450); Favored Enemy
rows are the list rows (400, 129 + 35 * k), Goblins k=3. Action bar buttons
(21 + 41.3 * k, 463), k = 0..11: Cast Spell 4 (divine casters), Bard Songs 6,
Special Abilities 11; sub-bar entries from k = 0; Esc goes back one bar (on
the main bar Esc opens the options screen instead). Record text (404,64,
354,294), pages (461,35) (548,35) (632,35). Spellbook class pages
(323,30) and (431,30)."""

from __future__ import annotations

import os
import time

import numpy as np
from PIL import Image

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
from test_s27_character_creation import (  # noqa: F401  (tidy is a fixture)
    ABILITY_VALUES,
    BUTTONS,
    CENTRE,
    DIALOG_GOLD,
    DOLL,
    FEAT_DOTS,
    FEMALE,
    FORMATION_BUTTONS,
    FORMATION_DONE,
    FULL,
    HELP,
    INV_GOLD,
    LIST,
    LIST_SCROLL_DOWN,
    LIST_Y,
    MALE,
    NAME_EDIT,
    NAME_NEXT,
    NAME_POPUP,
    NEXT,
    PARK,
    PLUS_X,
    POINTS,
    PORTRAIT_RIGHT,
    PROMPT_YES,
    RECORD_SCREEN,
    SCREEN_REGION,
    SKILL_VALUES,
    SLOT,
    SLOTS_REGION,
    SOUND_LIST,
    SPELL,
    SPELL_GRID,
    STEP,
    SWATCHES,
    WORLD_PARK,
    row,
    tidy,
)

LIST_TRACK_END = (535, 470)
SUMMARY_TRACK_END = (770, 450)

# In the game.
PORTRAIT = [(528 + 49 * i, 462) for i in range(6)]
PORTRAIT_HP = [(507 + 49 * i, 437, 32, 11) for i in range(6)]


def button(k):
    """Action bar button k (0..11), or entry k of a sub-bar."""
    return (21 + round(41.3 * k), 463)


CAST, SONGS, SPECIAL = 4, 6, 11
RECORD_TEXT = (404, 64, 354, 294)
RECORD_PAGE = [(461, 35), (548, 35), (632, 35)]
BOOK_PAGE = [(323, 30), (431, 30)]
BOOK_MEMORISED = (250, 75, 235, 320)
BOOK_KNOWN = (490, 75, 235, 340)

# name, gender, portraits to the right, race row, subrace row, class row, kit
# row, alignment row, skills (row, clicks), feat list scroll, feat rows,
# the class's own panels.
PARTY = [
    dict(name="ugo", gender=MALE, portrait=0, race=5, subrace=None, cls=0, kit=None, alignment=4,
         skills=[(7, 4)], feat_scroll=10, feats=[7], extra=None),
    dict(name="ela", gender=FEMALE, portrait=0, race=4, subrace=None, cls=1, kit=None, alignment=5,
         skills=[(2, 4), (4, 4)], feat_scroll=0, feats=[1], extra=None),
    dict(name="dom", gender=MALE, portrait=1, race=1, subrace=0, cls=2, kit=1, alignment=1,
         skills=[(0, 1), (3, 2)], feat_scroll=0, feats=[3], extra="cleric"),
    dict(name="wyn", gender=FEMALE, portrait=1, race=2, subrace=2, cls=3, kit=None, alignment=4,
         skills=[(1, 4)], feat_scroll=0, feats=[7], extra="druid"),
    dict(name="kato", gender=MALE, portrait=2, race=0, subrace=None, cls=5, kit=0, alignment=3,
         skills=[(3, 4), (6, 4), (9, 2)], feat_scroll=0, feats=[3, 4], extra=None),
    dict(name="fin", gender=FEMALE, portrait=2, race=3, subrace=0, cls=7, kit=None, alignment=1,
         skills=[(1, 4), (6, 4)], feat_scroll=0, feats=[9], extra="ranger"),
]
# S28_WITHOUT_RANGER=1 leaves fin out (a party of five), to see the rest on a
# build that cannot get past her Favored Enemy panel. Everything compared for
# the other five is the same with or without her.
if os.environ.get("S28_WITHOUT_RANGER") == "1":
    PARTY = PARTY[:5]
NAMES = [c["name"] for c in PARTY]


def test_s28_class_variety(tidy, game, golden_dir, out_dir, play_dir):  # noqa: F811
    t0 = time.time()

    class Failures(list):
        """Printed and written to the output folder as they come, so that a
        crash later in the run does not lose them."""

        def append(self, line):
            super().append(line)
            print(f"S28 DIFFERENCE: {line}")
            with open(out_dir / "differences.txt", "a") as f:
                f.write(line + "\n")

    failures = Failures()
    dropped = []  # keys that did not register and were pressed again

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

    def cells(name, frame, region, cell, allow=4):
        """`region` of `frame` cut into cells of `cell` (w, h): every cell
        must match the original's within `allow` pixels, so one wrong digit
        or icon fails however large the region. Recorded and given variants
        on the original like any checkpoint."""
        x0, y0, w, h = region
        cw, ch = cell
        crop = frame.convert("RGB").crop((x0, y0, x0 + w, y0 + h))
        game.shots += 1
        crop.save(out_dir / f"{game.shots:03d}-{name}.png")
        golden = golden_dir / f"{name}.png"
        variants = ([golden] if golden.exists() else []) + sorted(golden_dir.glob(f"{name}.alt*.png"))
        a = np.asarray(crop, dtype=np.int16)
        best = None
        for p in variants:
            b = np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)
            if b.shape != a.shape:
                bad = [(0, 0, -1)]
            else:
                d = np.abs(a - b).max(axis=2) > 24
                bad = [(cx, cy, int(d[cy:cy + ch, cx:cx + cw].sum()))
                       for cy in range(0, h, ch) for cx in range(0, w, cw)
                       if d[cy:cy + ch, cx:cx + cw].sum() > allow]
            if not bad:
                return
            if best is None or len(bad) < len(best):
                best = bad
        if game.target == "original" and not getattr(game, "virtual", False):
            crop.save(golden if not variants else golden_dir / f"{name}.alt{len(variants)}.png")
        elif not variants:
            failures.append(f"checkpoint {name}: no golden image")
        else:
            where = ", ".join(f"({x0 + cx},{y0 + cy}) {n} px" for cx, cy, n in best[:6])
            failures.append(f"checkpoint {name}: {len(best)} cell(s) of {cw}x{ch} differ from the original: {where}")

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
        click(xy, what, FULL, threshold=0.005, wait=0.35)

    def point(xy, what):
        """A +, - or spell icon that must change the points left."""
        click(xy, what, POINTS, threshold=0.004, wait=0.15, park=None)

    def type_name(text, who):
        """One key at a time, each proved by the edit box. A key that does
        not show within 3 s is pressed again (the box is still as it was:
        that is what the wait checked) and reported."""
        for ch in text:
            for attempt in range(3):
                try:
                    act(lambda: game.press(ch), f"typing {ch!r}", NAME_EDIT, threshold=0.002, wait=0.2, timeout=3.0)
                    break
                except AssertionError:
                    dropped.append(f"{who}: {ch!r} of {text!r} (attempt {attempt + 1}, "
                                   f"{time.time() - t0:.0f} s after the main menu)")
                    if attempt == 2:
                        raise

    # --- Main menu -> Party Formation ----------------------------------------------
    boot_to_main_menu(game)
    step(MAIN_MENU["new_game"], "New Game")
    game.click(*SELECT_PARTY["new_party"])
    game.move(*PARK)
    time.sleep(0.8)
    step(SELECT_PARTY["done"], "Done on Select Party")

    # --- Six characters ---------------------------------------------------------------
    def create(slot, c):
        n = c["name"]
        t1 = time.time()
        step(SLOT[slot], f"Create Character in slot {slot + 1}")
        step(STEP["gender"], f"Gender ({n})")
        click(c["gender"], f"the gender ({n})", threshold=0.0005)
        step(NEXT, f"Next after the gender ({n})")
        for k in range(c["portrait"]):
            click(PORTRAIT_RIGHT, f"next portrait {k + 1} ({n})", threshold=0.01)
        step(NEXT, f"Next after the portrait ({n})")

        step(STEP["race"], f"Race ({n})")
        if c["subrace"] is None:
            click(row(c["race"]), f"the race ({n})", HELP)
            cg(f"{n}-race")
        else:
            step(row(c["race"]), f"the race ({n})")  # opens the subrace list, the first one chosen
            cg(f"{n}-subrace")
            if c["subrace"]:
                click(row(c["subrace"]), f"the subrace ({n})", HELP)
                cg(f"{n}-subrace-picked")
        step(NEXT, f"Next after the race ({n})")

        step(STEP["class"], f"Class ({n})")
        if c["kit"] is None:
            click(row(c["cls"]), f"the class ({n})", HELP)
            cg(f"{n}-class")
        else:
            step(row(c["cls"]), f"the class ({n})")  # opens the deity or order list
            cg(f"{n}-kits")
            if n == "kato":
                click(row(2), "Monk of the Dark Moon", HELP)
                cg("kato-order-dark-moon")
                click(row(0), "Monk of the Old Order", HELP)
                cg("kato-order-old")
            else:
                click(row(c["kit"]), f"the kit ({n})", HELP)
                cg(f"{n}-kit-picked")
        step(NEXT, f"Next after the class ({n})")

        step(STEP["alignment"], f"Alignment ({n})")
        cg(f"{n}-alignment")
        click(row(c["alignment"]), f"the alignment ({n})", HELP)
        step(NEXT, f"Next after the alignment ({n})")

        step(STEP["abilities"], f"Abilities ({n})")
        cg(f"{n}-abilities", ("points", POINTS), ("values", ABILITY_VALUES, 0.001))
        step(NEXT, f"Next with all ability points left ({n})")
        step(PROMPT_YES, f"Yes on the unallocated points prompt ({n})")

        def skills(name):
            frame = cg(name, ("points", POINTS), ("values", SKILL_VALUES, 0.001))
            cells(name + "-rows", frame, LIST, (280, 36))

        step(STEP["skills"], f"Skills ({n})")
        skills(f"{n}-skills")
        for k, times in c["skills"]:
            for i in range(times):
                point((PLUS_X, LIST_Y[k]), f"skill row {k} + ({i + 1}) ({n})")
        skills(f"{n}-skills-spent")
        click(LIST_TRACK_END, f"the skill list's scrollbar track ({n})", LIST, threshold=0.001, wait=0.3)
        skills(f"{n}-skills-end")
        step(NEXT, f"Next after the skills ({n})")

        def feats(name):
            frame = cg(name, ("points", POINTS), ("dots", FEAT_DOTS, 0.001))
            cells(name + "-rows", frame, LIST, (280, 36))

        feats(f"{n}-feats")
        for i in range(c["feat_scroll"]):
            click(LIST_SCROLL_DOWN, f"feat list down ({i + 1}) ({n})", LIST, threshold=0.001, wait=0.15)
        if c["feat_scroll"]:
            feats(f"{n}-feats-scrolled")
        for k in c["feats"]:
            point((PLUS_X, LIST_Y[k]), f"feat row {k} + ({n})")
        feats(f"{n}-feats-spent")
        step(NEXT, f"Next after the feats ({n})")

        def spells(name):
            cg(name, ("points", POINTS), ("grid", SPELL_GRID, 0.001))

        if c["extra"] in ("cleric", "druid"):
            spells(f"{n}-memorize")
            point(SPELL(0, 0), f"the first divine spell ({n})")
            spells(f"{n}-memorize-chosen")
            step(NEXT, f"Next after memorizing ({n})")
        if c["extra"] == "cleric":
            spells(f"{n}-domain")
            point(SPELL(1, 0), f"the second domain spell ({n})")
            spells(f"{n}-domain-chosen")
            step(NEXT, f"Next after the domain spell ({n})")
        if c["extra"] == "ranger":
            cg(f"{n}-enemy")
            click(row(3), "Goblins", HELP)
            cg(f"{n}-enemy-goblins")
            step(NEXT, f"Next after the favored enemy ({n})")

        step(STEP["appearance"], f"Appearance ({n})")
        panel(f"{n}-colours", ("help", HELP), ("buttons", BUTTONS), ("swatches", SWATCHES, 0.001), mask=[DOLL])
        step(NEXT, f"Next after the colours ({n})")
        if slot < 2:  # the list for a man and for a woman
            cg(f"{n}-sound", ("list", SOUND_LIST))
        step(NEXT, f"Next after the sound ({n})")

        frame = cg(f"{n}-summary")
        cells(f"{n}-summary-lines", frame, HELP, (195, 13))
        click(SUMMARY_TRACK_END, f"the summary's scrollbar track ({n})", HELP, threshold=0.01, wait=0.3)
        frame = cg(f"{n}-summary-end")
        cells(f"{n}-summary-end-lines", frame, HELP, (195, 13))

        step(STEP["name"], f"Name ({n})")
        type_name(n, n)
        panel(f"{n}-name-typed", ("popup", NAME_POPUP), ("edit", NAME_EDIT))
        step(NAME_NEXT, f"Next on the name prompt ({n})")
        step(NEXT, f"Finish ({n})")
        panel(f"formation-{slot + 1}", ("slots", SLOTS_REGION), ("buttons", FORMATION_BUTTONS))
        print(f"S28 {n} took {time.time() - t1:.0f} s")

    for slot, c in enumerate(PARTY):
        create(slot, c)
    print(f"S28 creation took {time.time() - t0:.0f} s")

    made = sorted(p.name.upper() for p in (play_dir / "characters").iterdir()
                  if p.name.upper().rsplit(".", 1)[0] in [n.upper() for n in NAMES])
    print(f"S28 exported characters: {made}")
    want = sorted(f"{n.upper()}.{ext}" for n in NAMES for ext in ("CHR", "RES"))
    if made != want:
        failures.append(f"Characters folder after {len(NAMES)} Finishes: {made}")

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
    t2 = time.time()
    print(f"S28 in Targos {t2 - t0:.0f} s after the main menu")

    # --- action bars and sub-bars -------------------------------------------------------------
    def select(i):
        """Select member i alone. Not proved by itself (he may be selected
        already): the action bar compared next shows whose it is."""
        game.click(*PORTRAIT[i])
        game.move(*WORLD_PARK)
        time.sleep(1.0)

    def bar(name, hp=None):
        frame = game.wait_stable(settle=0.5, region=ACTION_BAR_REGION)
        check_frame(name, frame, ACTION_BAR_REGION)
        cells(name + "-buttons", frame, ACTION_BAR_REGION, (42, 55))
        if hp is not None:
            check_frame(f"{name}-hit-points", frame, PORTRAIT_HP[hp], max_diff=0.004)

    def sub(do, what):
        """An action bar button or Esc that must change the bar."""
        act(do, what, ACTION_BAR_REGION, threshold=0.003, wait=0.6, park=WORLD_PARK)

    def press(k, what):
        sub(lambda: game.click(*button(k)), what)

    def back(what):
        sub(lambda: game.press("esc"), f"Esc on {what}")

    def world(do, what):
        """To or from a screen: the action bar goes blank or comes back."""
        act(do, what, ACTION_BAR_REGION, threshold=0.1, wait=1.5, park=WORLD_PARK)

    for i, n in enumerate(NAMES):
        select(i)
        bar(f"{n}-bar", hp=i)
        if n == "ugo":
            press(SPECIAL, "ugo's Special Abilities")
            bar("ugo-special")
            back("ugo's Special Abilities")
        elif n == "ela":
            press(SONGS, "ela's Bard Songs")
            bar("ela-songs")
            back("ela's Bard Songs")
        elif n == "dom":
            press(CAST, "dom's Cast Spell")
            bar("dom-cast-classes")
            press(0, "dom's Cleric spells")
            bar("dom-cast-cleric")
            back("dom's Cleric spells")
            press(1, "dom's Domain spells")
            bar("dom-cast-domain")
            back("dom's Domain spells")
            back("dom's spell classes")
            press(SPECIAL, "dom's Special Abilities")
            bar("dom-special")
            back("dom's Special Abilities")
        elif n == "wyn":
            press(CAST, "wyn's Cast Spell")
            bar("wyn-cast")
            back("wyn's Cast Spell")
        elif n == "kato":
            press(SPECIAL, "kato's Special Abilities")
            bar("kato-special")
            back("kato's Special Abilities")
    print(f"S28 bars took {time.time() - t2:.0f} s")

    # --- records -------------------------------------------------------------------------------
    def record_page(name):
        frame = game.wait_stable(settle=0.6, region=RECORD_SCREEN)
        check_frame(name, frame, RECORD_SCREEN)
        for part, region in RECORD_DETAILS.items():
            check_frame(f"{name}-{part}", frame, region, max_diff=0.004)
        cells(name + "-lines", frame, RECORD_TEXT, (354, 13))

    def member(i, region, what):
        """Portrait i inside a screen. The first one may be showing already."""
        if i == 0:
            game.click(*PORTRAIT[0])
            game.move(*WORLD_PARK)
            time.sleep(1.0)
        else:
            act(lambda: game.click(*PORTRAIT[i]), f"portrait {i + 1} on the {what}", region,
                threshold=0.005, wait=0.5, park=WORLD_PARK)

    select(0)
    world(lambda: game.click(*DIAL["record"]), "Record dial button")
    for i, n in enumerate(NAMES):
        member(i, RECORD_SCREEN, "record")
        record_page(f"{n}-record")
        for k, page in ((1, "combat"), (2, "skills")):
            act(lambda: game.click(*RECORD_PAGE[k]), f"record page {k + 1} ({n})", RECORD_TEXT,
                threshold=0.01, wait=0.4, park=WORLD_PARK)
            record_page(f"{n}-record-{page}")
        act(lambda: game.click(*RECORD_PAGE[0]), f"record page 1 ({n})", RECORD_TEXT,
            threshold=0.01, wait=0.4, park=WORLD_PARK)
    world(lambda: game.press("esc"), "Esc on the record")

    # --- inventories ------------------------------------------------------------------------------
    gold_inventory = None
    world(lambda: game.click(*DIAL["inventory"]), "Inventory dial button")
    for i, n in enumerate(NAMES):
        member(i, INV_NAME_PORTRAIT, "inventory")
        frame = game.wait_stable(settle=0.6, region=INV_NAME_PORTRAIT)
        check_frame(f"{n}-inventory-name-portrait", frame, INV_NAME_PORTRAIT)
        check_frame(f"{n}-inventory-quick-weapons", frame, INV_QUICK_WEAPONS)
        check_frame(f"{n}-inventory-ac-hp", frame, INV_AC_HP)
        check_frame(f"{n}-inventory-backpack", frame, INV_BACKPACK)
        for part in ("armor-class", "hit-points", "weight", "selected-quick-weapon"):
            check_frame(f"{n}-inventory-{part}", frame, INV_DETAILS[part], max_diff=0.004)
        if i == 0:
            gold_inventory = read_number(frame, INV_GOLD)
    print(f"S28 gold: {gold_dialog} in the dialogue, {gold_inventory} in the inventory")
    if gold_dialog is None or gold_dialog != gold_inventory:
        failures.append(f"gold reads {gold_dialog} in the dialogue and {gold_inventory} in the inventory")
    world(lambda: game.press("esc"), "Esc on the inventory")

    # --- spellbooks -------------------------------------------------------------------------------
    def book(name):
        frame = game.wait_stable(settle=0.6, region=SCREEN_REGION)
        check_frame(name, frame, SCREEN_REGION)
        check_frame(name + "-slots", frame, SPELL_SLOTS, max_diff=0.004)
        cells(name + "-memorised", frame, BOOK_MEMORISED, (40, 40))
        cells(name + "-known", frame, BOOK_KNOWN, (235, 42))

    world(lambda: game.click(*DIAL["spellbook"]), "Spellbook dial button")
    for i in (1, 2, 3):
        n = NAMES[i]
        act(lambda: game.click(*PORTRAIT[i]), f"portrait {i + 1} on the spellbook", SCREEN_REGION,
            threshold=0.005, wait=0.5, park=WORLD_PARK)
        book(f"{n}-spellbook")
        if n == "dom":
            act(lambda: game.click(*BOOK_PAGE[1]), "the Domain page of dom's spellbook", SCREEN_REGION,
                threshold=0.003, wait=0.5, park=WORLD_PARK)
            book("dom-spellbook-domain")
    world(lambda: game.press("esc"), "Esc on the spellbook")
    print(f"S28 screens done {time.time() - t2:.0f} s after the dialogue")

    # --- abilities with a lasting, visible effect ------------------------------------------
    def effect_record(i, name):
        world(lambda: game.click(*DIAL["record"]), f"Record dial button ({name})")
        game.click(*PORTRAIT[i])  # the record may still show the last member looked at
        game.move(*WORLD_PARK)
        time.sleep(0.8)
        record_page(name)
        world(lambda: game.press("esc"), f"Esc on the record ({name})")

    select(0)
    bar("ugo-bar-before-rage")
    press(SPECIAL, "ugo's Special Abilities (to rage)")
    press(0, "Barbarian Rage")
    effect_record(0, "ugo-rage-record")
    press(SPECIAL, "ugo's Special Abilities after the rage")
    bar("ugo-special-after-rage")
    back("ugo's Special Abilities after the rage")

    select(4)
    bar("kato-bar-before-stunning")
    press(SPECIAL, "kato's Special Abilities (to stun)")
    press(0, "Stunning Attack")
    bar("kato-bar-stunning")
    effect_record(4, "kato-stunning-record")

    select(1)
    bar("ela-bar-before-song")
    press(SONGS, "ela's Bard Songs (to sing)")
    press(0, "The Ballad of Three Heroes")
    bar("ela-singing")
    press(SONGS, "Bard Songs while singing (stops the song)")
    bar("ela-song-stopped")

    if dropped:
        (out_dir / "dropped-keys.txt").write_text("\n".join(dropped) + "\n")
    print(f"S28 dropped keys: {len(dropped)} {dropped}")
    print(f"S28 took {time.time() - t0:.0f} s after the main menu")
    assert not failures, f"{len(failures)} difference(s): " + "; ".join(failures)
