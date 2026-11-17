"""S29: rogue, sorcerer, fighter. The two classes no earlier scenario has
created, plus a fighter, made from scratch (as S28 makes its six); then in
Targos the rogue's skill modes, the sorcerer's spell and the fighter's
weapons.

Creation (S28's quick path; the panels that depend on class or race are
compared):

  nim   female strongheart halfling rogue, chaotic neutral. Abilities left
        alone (8/12/10/10/10/10). 16 skill points, the class skills at cost
        1: the list's end, Open Lock x4, Pick Pocket x4, Search x4, Hide x2,
        Move Silently x2 (Next stays greyed until all are spent). Two feats
        (strongheart): Blind-Fight, Dash.
  zol   male tiefling sorcerer, chaotic neutral. Charisma + x8 (8 -> 16, 8
        points left: the prompt, Yes). 8 skill points: Concentration x4,
        Bluff x4. Combat Casting. Arcane Spell Selection: Level 1 (19 spells,
        2 to choose): Mage Armor, Shield. Next goes straight back to the
        main menu: a sorcerer has no memorisation panel.
  tova  female aasimar fighter (second portrait), lawful good. Strength + x6
        (16). 4 skill points: Intimidate x4. Two feats (the fighter's bonus
        feat; her list starts at Blind-Fight, the armour and shield
        proficiencies are already hers): Bullheaded, Discipline.

In Targos, after Hedron's dialogue (S5's replies), each member alone:

  bars      the action bar and the portrait's hit points; nim's Use Skill
            bar (Stealth, Search, Thieving, Tracking, Animal Empathy); zol's
            Cast Spell bar (Mage Armor 4, Shield 4: one count for the level).
  record    three pages each (S28's regions and text lines)
  inventory S5's regions; the gold read as digits against Hedron's dialogue
  spellbook zol's (Sorcerer page: nothing memorised, two known)

Then:

  search    nim, Search: the button gets a red frame, "nim- Searching..."
            is printed; again: "nim- Stopped Searching", the bar as before.
  tracking  nim, Use Skill -> Tracking: "Your tracking skills reveal nothing
            about this location."
  stealth   nim, Stealth. The Hide roll is random (see STEALTH below): only
            what is the same either way is required, the rest is compared
            when it happens.
  spell     zol casts Mage Armor on himself: the cast bar's counts drop to
            3 / 3, his record shows the armor class and the effect.
  weapons   tova: see WEAPONS below.
  thieving  nim, Thieving on Hedron, last (see THIEVING below).

STEALTH. The Hide roll is random on the original (Hide 3: about half of 20
tries while exploring; 3 failures in 10 rolls over 7 recorded runs). A
success prints "nim- Hide Succeeded" and "nim- Invisible", frames the Stealth
and Use Skill buttons in red and puts a small figure on her portrait. A
failure prints "nim- Hide Failed" and shows no figure; a second after the
click the Stealth button is then either greyed or framed in red like a
success (both seen on the original: two golden variants), and the bar is
back to normal within about 6 s. A second click while hidden prints "nim-
Leaving Shadows", removes the figure and greys the button for about 5.6 s.
The test clicks Stealth up to STEALTH_TRIES times until a roll succeeds, then
compares the bar, the hidden portrait, the lines, the greyed bar and the bar
afterwards. The bar and line of a failure are compared only if the original
has recorded them; the outcomes are written to notes.txt in the output
folder. (Left hidden longer than about 8 s the button loses its frame
while she stays hidden; the test leaves the shadows before that.)

WEAPONS. tova has a quarterstaff in weapon set 1 and nothing else (no armour:
armor class 10 throughout). In the inventory the staff is moved to the first
backpack slot and back; from the action bar (right-click on the quick weapon:
the four sets) set 2 (fists) and then set 1 are chosen. Compared: the
inventory regions, the action bar, and the record's combat page (Main Hand -
Fist 1d2 against Quarterstaff 1d6, Strength +3 against +4).
S29_WITHOUT_WEAPONS=1 leaves this part out.

THIEVING. Thieving frames its button in red; over Hedron the pointer is a hand
with a purse, and a click sends nim to him. The outcome is random: "nim- Pick
Pocket Succeeded" (a dagger in her backpack, or a yellow "The party has
gained gold: N"; Hedron stays neutral and a second attempt is possible), or "nim- Pick Pocket Failed", after which Hedron, Eldgull
and Screed turn hostile and kill a level 1 party within half a minute. So it
is the last step: the armed bar is compared, then the attempt is made, the
line of the outcome that happened is compared if the original has recorded
that outcome, and the game must keep running for 8 s more. The outcome is
written to notes.txt. (A click on Thieving while a dialogue is open arms it
unseen; the next click then disarms it.)

Open Lock and Disable Device are not covered: the docks area (AR1000) has no
locked or trapped container, and its two locked doors (AR1001, AR1002) are a
fight away. S18 and S25 pick locks with the premade rogue.

Not compared: the message window as a whole (the dock people's overhead
lines land in it at random; single lines are found by their left edge), the
pointer over a target, the loading screen, the paper dolls, the world view.

New coordinates (800x600). Creation: sorcerer spells Mage Armor (3,1), Shield
(3,2) in S27's SPELL grid. Action bar of a rogue or fighter: Use Skill k=3,
Stealth 4, Search 5, Thieving 6 (S28's button(k)); a sorcerer has Cast Spell
at 4. Use Skill bar: Stealth 0, Search 1, Thieving 2, Tracking 3, Animal
Empathy 4. Weapon sets bar (right-click on k=1): set n at k = 3 * (n - 1).
Hedron's circle after the opening dialogue (488,218)."""

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
from test_s16_clear_docks import cyan_near
from test_s27_character_creation import (  # noqa: F401  (tidy is a fixture)
    ABILITY_VALUES,
    ABILITY_Y,
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
    SPELL,
    SPELL_GRID,
    STEP,
    SWATCHES,
    WORLD_PARK,
    row,
    tidy,
)
from test_s28_class_variety import (
    BOOK_KNOWN,
    BOOK_MEMORISED,
    LIST_TRACK_END,
    PORTRAIT,
    PORTRAIT_HP,
    RECORD_PAGE,
    RECORD_TEXT,
    SUMMARY_TRACK_END,
    button,
)

# name, gender, portraits to the right, race row, subrace row, class row,
# alignment row, abilities (name, + clicks), skills (row, clicks) in the list
# as it opens and after a click on the end of its scrollbar track, feat rows,
# the class's own panel.
PARTY = [
    dict(name="nim", gender=FEMALE, portrait=0, race=6, subrace=1, cls=8, alignment=5, abilities=[],
         skills=[], skills_end=[(4, 4), (5, 4), (6, 4), (0, 2), (3, 2)], feats=[3, 9], extra=None),
    dict(name="zol", gender=MALE, portrait=0, race=0, subrace=2, cls=9, alignment=5, abilities=[("cha", 8)],
         skills=[(3, 4), (2, 4)], skills_end=[], feats=[6], extra="sorcerer"),
    dict(name="tova", gender=FEMALE, portrait=1, race=0, subrace=1, cls=4, alignment=0, abilities=[("str", 6)],
         skills=[(7, 4)], skills_end=[], feats=[1, 9], extra=None),
]
# S29_WITHOUT=zol (or nim, tova; several with commas) leaves members out, to
# see the rest on a build that cannot get past one of them. The steps in
# Targos that need a missing member are skipped.
_without = [n for n in os.environ.get("S29_WITHOUT", "").split(",") if n]
PARTY = [c for c in PARTY if c["name"] not in _without]
NAMES = [c["name"] for c in PARTY]

# In the game.
SKILLS, STEALTH, SEARCH, THIEVING = 3, 4, 5, 6  # nim's and tova's action bar
CAST = 4  # zol's
SUB_TRACKING = 3  # entry of the Use Skill bar
MESSAGES = (5, 495, 540, 100)
LINE_SPOT = (8, 576, 535, 14)  # where a found message line is compared
BAR_PARK = (300, 420)  # world view just above the action bar: off the message lines
PORTRAIT_ICONS = [(503 + 49 * i, 447, 49, 43) for i in range(6)]  # a portrait without its hit points
STEALTH_TRIES = 8
WEAPON_1 = (618, 65)  # inventory: the weapon of set 1
BACKPACK_0 = (270, 318)  # inventory: the first backpack slot
HEDRON = (488, 218)  # his circle in the view the game starts with


def name_line(img, orange=False):
    """The lowest message line that starts at the window's left edge (a
    character's "name- ..." line; the dock people's overhead lines, which
    land in the window at random, are centred), alone on black at LINE_SPOT
    (the window's texture differs with the height), or None. With `orange`
    only a line whose first letters have the orange of nim's name counts
    (not the yellow "The party has gained gold" under it). The pointer must
    not be in the window."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    m = a[y0:y0 + h0, 8:21]
    if orange:
        # nim's name is (224..239, 160..175, 80..95); Hedron's (240, 128, 112).
        edge = ((m[..., 0] > 200) & (m[..., 1] > 150) & (m[..., 1] < 190) & (m[..., 2] > 60)
                & (m[..., 2] < 110)).any(axis=1)
    else:
        edge = (m.max(axis=2) > 110).any(axis=1)
    rows = np.nonzero(edge)[0]
    if not len(rows):
        return None
    # The name has no descenders: its lowest row is the baseline. 9 rows
    # above it and 3 below hold the whole line and nothing of its neighbours.
    top = y0 + int(rows.max()) - 9
    line = a[top:top + 13, 8:495]
    keep = line.max(axis=2) > 110
    out = Image.new("RGB", img.size)
    out.paste(Image.fromarray(np.where(keep[..., None], line, 0).astype(np.uint8)), (LINE_SPOT[0], LINE_SPOT[1]))
    return out


def test_s29_rogue_sorcerer_fighter(tidy, game, golden_dir, out_dir, play_dir):  # noqa: F811
    t0 = time.time()

    class Failures(list):
        """Printed and written to the output folder as they come, so that a
        crash later in the run does not lose them."""

        def append(self, line):
            super().append(line)
            print(f"S29 DIFFERENCE: {line}")
            with open(out_dir / "differences.txt", "a") as f:
                f.write(line + "\n")

    failures = Failures()
    dropped = []  # keys that did not register and were pressed again

    def note(line):
        """What happened where the original itself varies."""
        print(f"S29 NOTE: {line}")
        with open(out_dir / "notes.txt", "a") as f:
            f.write(line + "\n")

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

    def check_if_recorded(name, frame, region, **kw):
        """A checkpoint for something that only happens on some runs (a
        failed Hide roll): recorded when the original shows it, compared on
        the rebuild only if the original ever did."""
        if game.target == "original" or (golden_dir / f"{name}.png").exists():
            check_frame(name, frame, region, **kw)
        else:
            note(f"{name}: not compared (the original has not recorded it)")

    def cells(name, frame, region, cell, allow=4):
        """`region` of `frame` cut into cells of `cell` (w, h): every cell
        must match the original's within `allow` pixels, so one wrong digit
        or icon fails however large the region. Recorded and given variants
        on the original like any checkpoint. (S28's.)"""
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
        not show within 3 s is pressed again and reported (as S28)."""
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

    # --- Three characters -------------------------------------------------------------
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
        step(row(c["race"]), f"the race ({n})")  # opens the subrace list, the first one chosen
        cg(f"{n}-subrace")
        click(row(c["subrace"]), f"the subrace ({n})", HELP)
        cg(f"{n}-subrace-picked")
        step(NEXT, f"Next after the race ({n})")

        step(STEP["class"], f"Class ({n})")
        click(row(c["cls"]), f"the class ({n})", HELP)
        cg(f"{n}-class")
        step(NEXT, f"Next after the class ({n})")

        step(STEP["alignment"], f"Alignment ({n})")
        cg(f"{n}-alignment")
        click(row(c["alignment"]), f"the alignment ({n})", HELP)
        step(NEXT, f"Next after the alignment ({n})")

        def abilities(name):
            cg(name, ("points", POINTS), ("values", ABILITY_VALUES, 0.001))

        step(STEP["abilities"], f"Abilities ({n})")
        abilities(f"{n}-abilities")
        for ability, times in c["abilities"]:
            for i in range(times):
                point((PLUS_X, ABILITY_Y[ability]), f"{ability} + ({i + 1}) ({n})")
            abilities(f"{n}-abilities-{ability}")
        step(NEXT, f"Next with ability points left ({n})")
        step(PROMPT_YES, f"Yes on the unallocated points prompt ({n})")

        def skills(name):
            frame = cg(name, ("points", POINTS), ("values", SKILL_VALUES, 0.001))
            cells(name + "-rows", frame, LIST, (280, 36))

        step(STEP["skills"], f"Skills ({n})")
        skills(f"{n}-skills")
        for k, times in c["skills"]:
            for i in range(times):
                point((PLUS_X, LIST_Y[k]), f"skill row {k} + ({i + 1}) ({n})")
        if c["skills"]:
            skills(f"{n}-skills-spent")
        click(LIST_TRACK_END, f"the skill list's scrollbar track ({n})", LIST, threshold=0.001, wait=0.3)
        skills(f"{n}-skills-end")
        for k, times in c["skills_end"]:
            for i in range(times):
                point((PLUS_X, LIST_Y[k]), f"skill row {k} of the list's end + ({i + 1}) ({n})")
        if c["skills_end"]:
            skills(f"{n}-skills-end-spent")
        step(NEXT, f"Next after the skills ({n})")

        def feats(name):
            frame = cg(name, ("points", POINTS), ("dots", FEAT_DOTS, 0.001))
            cells(name + "-rows", frame, LIST, (280, 36))

        feats(f"{n}-feats")
        for k in c["feats"]:
            point((PLUS_X, LIST_Y[k]), f"feat row {k} + ({n})")
        feats(f"{n}-feats-spent")
        step(NEXT, f"Next after the feats ({n})")

        if c["extra"] == "sorcerer":
            def spells(name):
                cg(name, ("points", POINTS), ("grid", SPELL_GRID, 0.001))

            spells(f"{n}-spells")
            point(SPELL(3, 1), "Mage Armor")
            spells(f"{n}-spells-mage-armor")
            point(SPELL(3, 2), "Shield")
            spells(f"{n}-spells-chosen")
            step(NEXT, f"Next after the spells ({n})")
        # Back on the main menu (for zol: straight from the spell selection,
        # there is no memorisation panel), Appearance the open step.
        cg(f"{n}-main-skills-done")

        step(STEP["appearance"], f"Appearance ({n})")
        panel(f"{n}-colours", ("help", HELP), ("buttons", BUTTONS), ("swatches", SWATCHES, 0.001), mask=[DOLL])
        step(NEXT, f"Next after the colours ({n})")
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
        panel(f"formation-{slot + 1}-{n}", ("slots", SLOTS_REGION), ("buttons", FORMATION_BUTTONS))
        print(f"S29 {n} took {time.time() - t1:.0f} s")

    for slot, c in enumerate(PARTY):
        create(slot, c)
    print(f"S29 creation took {time.time() - t0:.0f} s")

    made = sorted(p.name.upper() for p in (play_dir / "characters").iterdir()
                  if p.name.upper().rsplit(".", 1)[0] in [n.upper() for n in NAMES])
    print(f"S29 exported characters: {made}")
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
    print(f"S29 in Targos {t2 - t0:.0f} s after the main menu")

    at = {n: i for i, n in enumerate(NAMES)}  # portrait of each member

    # --- helpers in the game ---------------------------------------------------------------------
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
        return frame

    def sub(do, what, park=WORLD_PARK):
        """An action bar button or Esc that must change the bar."""
        act(do, what, ACTION_BAR_REGION, threshold=0.003, wait=0.6, park=park)

    def press(k, what, **kw):
        sub(lambda: game.click(*button(k)), what, **kw)

    def back(what):
        sub(lambda: game.press("esc"), f"Esc on {what}")

    def world(do, what):
        """To or from a screen: the action bar goes blank or comes back."""
        act(do, what, ACTION_BAR_REGION, threshold=0.1, wait=1.5, park=WORLD_PARK)

    def member(i, region, what):
        """Portrait i inside a screen. The first one may be showing already."""
        if i == 0:
            game.click(*PORTRAIT[0])
            game.move(*WORLD_PARK)
            time.sleep(1.0)
        else:
            act(lambda: game.click(*PORTRAIT[i]), f"portrait {i + 1} on the {what}", region,
                threshold=0.005, wait=0.5, park=WORLD_PARK)

    def record_page(name):
        frame = game.wait_stable(settle=0.6, region=RECORD_SCREEN)
        check_frame(name, frame, RECORD_SCREEN)
        for part, region in RECORD_DETAILS.items():
            check_frame(f"{name}-{part}", frame, region, max_diff=0.004)
        cells(name + "-lines", frame, RECORD_TEXT, (354, 13))

    def record_of(i, name, pages=(0, 1)):
        """Open the record on member i, compare the pages, leave."""
        world(lambda: game.click(*DIAL["record"]), f"Record dial button ({name})")
        game.click(*PORTRAIT[i])  # the record may still show the last member looked at
        game.move(*WORLD_PARK)
        time.sleep(0.8)
        for k in pages:
            if k:
                act(lambda: game.click(*RECORD_PAGE[k]), f"record page {k + 1} ({name})", RECORD_TEXT,
                    threshold=0.01, wait=0.4, park=WORLD_PARK)
            record_page(f"{name}-{('record', 'combat', 'skills')[k]}")
        if pages[-1]:
            act(lambda: game.click(*RECORD_PAGE[0]), f"record page 1 ({name})", RECORD_TEXT,
                threshold=0.01, wait=0.4, park=WORLD_PARK)
        world(lambda: game.press("esc"), f"Esc on the record ({name})")

    def inventory_parts(name, frame):
        check_frame(f"{name}-name-portrait", frame, INV_NAME_PORTRAIT)
        check_frame(f"{name}-quick-weapons", frame, INV_QUICK_WEAPONS)
        check_frame(f"{name}-ac-hp", frame, INV_AC_HP)
        check_frame(f"{name}-backpack", frame, INV_BACKPACK)
        for part in ("armor-class", "hit-points", "weight", "selected-quick-weapon"):
            check_frame(f"{name}-{part}", frame, INV_DETAILS[part], max_diff=0.004)

    def line_check(name, before, optional=False, timeout=12.0):
        """Wait for a new left-aligned message line (a different lowest one
        than in `before`; with `before` None the line is there already) and
        compare it."""
        deadline = time.time() + timeout
        was = name_line(before) if before is not None else None
        while before is not None:
            frame = game.grab()
            now = name_line(frame)
            if now is not None and (was is None or diff_ratio(was, now, region=LINE_SPOT) > 0.001):
                break
            if time.time() > deadline:
                failures.append(f"{name}: no new message line within {timeout:.0f} s")
                return frame
            time.sleep(0.2)
        time.sleep(0.4)  # a second line printed in the same moment ("Hide Succeeded", "Invisible")
        frame = game.grab()
        line = name_line(frame)
        if line is None:
            failures.append(f"{name}: no message line")
        else:
            (check_if_recorded if optional else check_frame)(name, line, LINE_SPOT, max_diff=0.001)
        return frame

    def wait_bar(ref, what, timeout=30.0):
        """Until the action bar is as in `ref` again."""
        deadline = time.time() + timeout
        while diff_ratio(ref, game.grab(), region=ACTION_BAR_REGION) > 0.002:
            if time.time() > deadline:
                failures.append(f"{what}: the action bar did not come back within {timeout:.0f} s")
                return False
            time.sleep(0.3)
        return True

    # --- action bars and sub-bars -------------------------------------------------------------
    for i, n in enumerate(NAMES):
        select(i)
        bar(f"{n}-bar", hp=i)
        if n == "nim":
            press(SKILLS, "nim's Use Skill")
            bar("nim-skills-bar")
            back("nim's Use Skill")
        elif n == "zol":
            press(CAST, "zol's Cast Spell")
            bar("zol-cast")
            back("zol's Cast Spell")
        elif n == "tova":
            sub(lambda: game.click(*button(1), button="right"), "right-click on tova's quick weapon")
            bar("tova-weapon-sets")
            back("tova's weapon sets")
    print(f"S29 bars took {time.time() - t2:.0f} s")

    # --- records -------------------------------------------------------------------------------
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
        inventory_parts(f"{n}-inventory", frame)
        if i == 0:
            gold_inventory = read_number(frame, INV_GOLD)
    print(f"S29 gold: {gold_dialog} in the dialogue, {gold_inventory} in the inventory")
    if gold_dialog is None or gold_dialog != gold_inventory:
        failures.append(f"gold reads {gold_dialog} in the dialogue and {gold_inventory} in the inventory")
    world(lambda: game.press("esc"), "Esc on the inventory")

    # --- the sorcerer's spellbook ---------------------------------------------------------------
    if "zol" in at:
        select(at["zol"])
        world(lambda: game.click(*DIAL["spellbook"]), "Spellbook dial button")
        game.click(*PORTRAIT[at["zol"]])
        game.move(*WORLD_PARK)
        frame = game.wait_stable(settle=0.8, region=SCREEN_REGION)
        check_frame("zol-spellbook", frame, SCREEN_REGION)
        check_frame("zol-spellbook-slots", frame, SPELL_SLOTS, max_diff=0.004)
        cells("zol-spellbook-memorised", frame, BOOK_MEMORISED, (40, 40))
        cells("zol-spellbook-known", frame, BOOK_KNOWN, (235, 42))
        world(lambda: game.press("esc"), "Esc on the spellbook")
    print(f"S29 screens done {time.time() - t2:.0f} s after the dialogue")

    # --- the rogue's modes -----------------------------------------------------------------------
    if "nim" in at:
        i = at["nim"]
        select(i)
        game.move(*BAR_PARK)
        normal = bar("nim-bar-before-modes")

        # Search: on, off.
        before = game.grab()
        press(SEARCH, "nim's Search", park=BAR_PARK)
        frame = bar("nim-searching")
        check_frame("nim-searching-portrait", frame, PORTRAIT_ICONS[i], max_diff=0.004)
        line_check("nim-searching-line", before)
        before = game.grab()
        press(SEARCH, "nim's Search again", park=BAR_PARK)
        bar("nim-search-stopped")
        line_check("nim-search-stopped-line", before)

        # Use Skill -> Tracking.
        press(SKILLS, "nim's Use Skill (to track)", park=BAR_PARK)
        before = game.grab()
        press(SUB_TRACKING, "Tracking", park=BAR_PARK)
        line_check("nim-tracking-line", before)
        bar("nim-bar-after-tracking")

        # Stealth (see STEALTH in the docstring).
        outcomes = []
        for attempt in range(STEALTH_TRIES):
            before = game.grab()
            press(STEALTH, f"nim's Stealth (try {attempt + 1})", park=BAR_PARK)
            time.sleep(0.8)
            frame = game.grab()
            hidden = diff_ratio(normal, frame, region=PORTRAIT_ICONS[i]) > 0.01
            outcomes.append("succeeded" if hidden else "failed")
            if hidden:
                check_frame("nim-stealth-on", frame, ACTION_BAR_REGION)
                cells("nim-stealth-on-buttons", frame, ACTION_BAR_REGION, (42, 55))
                check_frame("nim-hidden-portrait", frame, PORTRAIT_ICONS[i], max_diff=0.004)
                line_check("nim-hidden-line", before)
                before = game.grab()
                press(STEALTH, "nim's Stealth while hidden", park=BAR_PARK)
                frame = game.grab()
                check_frame("nim-leaving-shadows", frame, ACTION_BAR_REGION)
                cells("nim-leaving-shadows-buttons", frame, ACTION_BAR_REGION, (42, 55))
                check_frame("nim-leaving-shadows-portrait", frame, PORTRAIT_ICONS[i], max_diff=0.004)
                line_check("nim-leaving-shadows-line", before)
                wait_bar(normal, "after Leaving Shadows")
                break
            if game.target == "original" or (golden_dir / "nim-hide-failed.png").exists():
                check_frame("nim-hide-failed", frame, ACTION_BAR_REGION)
                cells("nim-hide-failed-buttons", frame, ACTION_BAR_REGION, (42, 55))
            line_check("nim-hide-failed-line", None, optional=True)
            wait_bar(normal, "after a failed Hide")
            time.sleep(0.5)
        note(f"stealth: Hide {', '.join(outcomes)}")
        if "succeeded" not in outcomes:
            failures.append(f"Hide failed {STEALTH_TRIES} times in a row: the hidden state was not compared")
        bar("nim-bar-after-stealth")
        print(f"S29 rogue modes done {time.time() - t2:.0f} s after the dialogue")

    # --- the sorcerer casts Mage Armor on himself --------------------------------------------
    if "zol" in at:
        i = at["zol"]
        select(i)
        plain = bar("zol-bar-before-cast")
        press(CAST, "zol's Cast Spell (to cast)")
        press(0, "Mage Armor")  # back to the main bar, the pointer a target cursor
        game.click(*PORTRAIT[i])
        game.move(*WORLD_PARK)
        deadline = time.time() + 20.0
        while diff_ratio(plain, game.grab(), region=PORTRAIT_ICONS[i]) <= 0.01:
            if time.time() > deadline:
                failures.append("Mage Armor: no effect icon on zol's portrait within 20 s")
                break
            time.sleep(0.3)
        time.sleep(1.5)
        frame = game.wait_stable(settle=0.5, region=ACTION_BAR_REGION)
        check_frame("zol-armor-portrait", frame, PORTRAIT_ICONS[i], max_diff=0.004)
        press(CAST, "zol's Cast Spell after the cast")
        bar("zol-cast-after")
        back("zol's Cast Spell after the cast")
        record_of(i, "zol-armor")
        print(f"S29 spell done {time.time() - t2:.0f} s after the dialogue")

    # --- the fighter's weapons (see WEAPONS in the docstring) ---------------------------------
    if "tova" in at and os.environ.get("S29_WITHOUT_WEAPONS") != "1":
        i = at["tova"]
        select(i)
        bar("tova-bar-before-weapons")

        def to_inventory(what):
            world(lambda: game.click(*DIAL["inventory"]), f"Inventory dial button ({what})")
            game.click(*PORTRAIT[i])
            game.move(*WORLD_PARK)
            time.sleep(0.8)

        def move_item(a, b, what):
            act(lambda: game.click(*a), f"picking up {what}", SCREEN_REGION, threshold=0.0005, wait=0.4,
                park=WORLD_PARK)
            act(lambda: game.click(*b), f"putting down {what}", SCREEN_REGION, threshold=0.0005, wait=0.4,
                park=WORLD_PARK)
            return game.wait_stable(settle=0.6, region=INV_NAME_PORTRAIT)

        to_inventory("to unequip")
        inventory_parts("tova-unarmed-inventory", move_item(WEAPON_1, BACKPACK_0, "the quarterstaff"))
        world(lambda: game.press("esc"), "Esc on the inventory (unarmed)")
        bar("tova-bar-unarmed")
        record_of(i, "tova-unarmed", pages=(1,))
        to_inventory("to equip")
        inventory_parts("tova-rearmed-inventory", move_item(BACKPACK_0, WEAPON_1, "the quarterstaff again"))
        world(lambda: game.press("esc"), "Esc on the inventory (armed again)")
        bar("tova-bar-rearmed")

        sub(lambda: game.click(*button(1), button="right"), "right-click on tova's quick weapon (to swap)")
        press(3, "weapon set 2")  # back to the main bar
        bar("tova-bar-set-2")
        record_of(i, "tova-set-2", pages=(1,))
        sub(lambda: game.click(*button(1), button="right"), "right-click on tova's quick weapon (set 2)")
        bar("tova-weapon-sets-2")
        press(0, "weapon set 1")
        bar("tova-bar-set-1")
        record_of(i, "tova-set-1", pages=(1,))
        print(f"S29 weapons done {time.time() - t2:.0f} s after the dialogue")

    # --- Thieving on Hedron, last (see THIEVING in the docstring) --------------------------------
    if "nim" in at:
        i = at["nim"]
        select(i)
        game.move(*BAR_PARK)
        normal = bar("nim-bar-before-thieving")
        press(THIEVING, "nim's Thieving", park=BAR_PARK)
        frame = bar("nim-thieving-armed")
        hedron = cyan_near(frame, HEDRON, dx=40, dy=25)
        if hedron is None:
            failures.append("Hedron's circle was not found: no Pick Pocket attempt")
        else:
            before = frame
            game.click(*hedron)
            game.move(*BAR_PARK)
            # The line depends on the roll: wait for it, then tell the outcome
            # by Hedron's circle (cyan still, or the red of a hostile).
            deadline = time.time() + 20.0
            was = name_line(before)
            while time.time() < deadline:
                frame = game.grab()
                now = name_line(frame)
                if now is not None and (was is None or diff_ratio(was, now, region=LINE_SPOT) > 0.001):
                    break
                time.sleep(0.2)
            else:
                failures.append("Pick Pocket: no message line within 20 s")
            # The window scrolls and a success can print a second line (gold):
            # take nim's line once two grabs show it the same.
            line = None
            for _ in range(6):
                time.sleep(0.2)
                frame = game.grab()
                again = name_line(frame, orange=True)
                if line is not None and again is not None and diff_ratio(line, again, region=LINE_SPOT) == 0:
                    break
                line = again
            time.sleep(0.4)
            frame = game.grab()
            a = np.asarray(frame.convert("RGB"), dtype=np.int16)
            m = a[MESSAGES[1]:MESSAGES[1] + MESSAGES[3], 8:495]
            gold = int(((m[..., 0] > 200) & (m[..., 1] > 200) & (m[..., 2] < 40)).sum()) > 60
            time.sleep(2.0)
            a = np.asarray(game.grab().convert("RGB"), dtype=np.int16)[:430]
            hostile = int(((a[..., 0] > 240) & (a[..., 1] < 30) & (a[..., 2] > 20) & (a[..., 2] < 60)).sum())
            outcome = "failed" if hostile > 30 else "succeeded"
            note(f"pick pocket: {outcome} ({hostile} hostile-red pixels 3 s later"
                 + (", a yellow line: gold gained)" if gold else ")"))
            if line is not None:
                check_if_recorded(f"nim-pick-pocket-{outcome}-line", line, LINE_SPOT, max_diff=0.001)
            # The game must keep running either way (a failure makes Hedron,
            # Eldgull and Screed attack the party).
            for _ in range(16):
                game.grab()
                time.sleep(0.5)
            if outcome == "succeeded":
                wait_bar(normal, "after the Pick Pocket", timeout=10.0)
                frame = game.wait_stable(settle=0.5, region=ACTION_BAR_REGION)
                check_if_recorded("nim-bar-after-pick-pocket", frame, ACTION_BAR_REGION)

    if dropped:
        (out_dir / "dropped-keys.txt").write_text("\n".join(dropped) + "\n")
    print(f"S29 dropped keys: {len(dropped)} {dropped}")
    print(f"S29 took {time.time() - t0:.0f} s after the main menu")
    assert not failures, f"{len(failures)} difference(s): " + "; ".join(failures)
