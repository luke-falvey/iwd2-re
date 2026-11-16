"""S26: a party member dies. Starts from S25's banked save (inside Shawford's
house, AR1201, where no enemy comes) and never leaves the house. Nothing is
banked: there is no `s26-end` save, the story goes on from `s25-end`.

No earlier scenario has a death in the party, and the rebuild crashed on
exactly that path until S25 found it (CGameSprite::AIUpdate reading m_pArea
after RemoveFromArea). Here the death is made on purpose and without the
battle: with Attack armed a click on a party member orders the selected
characters to attack him (S25's note). Geldin, Khatja and Laeris are selected
and sent against Vraedna (sorcerer, 12 hit points, the fifth portrait), who
stands next to them in the save. One order is enough: it stands until she is
dead (3.8 s when explored; how many blows it takes is up to the dice).

Nobody is special on the original: there is no protagonist whose death ends
the game (Sheris, the first portrait, leads dialogues and opens containers;
she is not the victim only so that those steps stay as they are).

test_s26_party_death (both targets) loads the s25-end save, then:

  before      Select All; portraits compared (six alive, all framed)
  kill        Geldin's portrait, Shift+Khatja's, Shift+Laeris's (portraits
              compared: three frames), Attack, a click on Vraedna (her circle
              is where the save puts her: screen 435,409). Polled until her
              portrait is grey; the line "Vraedna- Death" is caught on the
              way (her name is pure grey 161,161,161 and the line ends at
              x=104; found by that, text only, as S18's lock-pick line)
  running     22 s with no input straight after the death: the game must be
              alive, the world view must keep moving (it changes by 0.5-1%
              every second in this room; 0 when paused) and the last red
              line must not be PAUSED
  portraits   Select All: five frames, the fifth portrait grey with no hit
              points (compare); the pointer on the dead portrait: the
              tooltip "Vraedna 0/12" (compare); a click on the dead portrait
              takes every frame away and leaves the group bar (compare both);
              Select All again
  record      the Record button, Vraedna's portrait: the page says DEAD,
              hit points 0 over 12, Export and Customize are greyed (the
              page, and those three as their own regions)
  inventory   the Inventory button, Vraedna's portrait: nothing at all
              (no paper doll, empty slots, "0 / 0 lb."): everything she had
              is on the floor where she died (compare)
  walk        Select All, a click on the middle of the main room: five
              circles arrive there
  pile        a click on the spot where she died opens the ground pile with
              her things (three items; Sheris walks over to it): its slots
              and Sheris's pack compared
  Nolan       the healer says and offers the same as in S25 (text compared);
              1 ("Yes.") runs his healing; afterwards Vraedna is still dead
              (portraits compared)
  save        Q: the green "Quick-save successful" line (as S24); the play
              folder then holds a Quick-Save and a Quick-Save-Backup
  load        Options, Load Game: the prompt "The current game will be
              lost..." (compare), Load; the load screen's third row (the
              new quick save) shows six portraits, Vraedna's in colour
              (compare); Load on it. After the load: the portraits (still
              five and one dead), the record page (DEAD, 0 over 12) and
              the pile on the floor again, and 6 more seconds of running.

Not compared: the world view (lamps, idle poses), the message window except
the two lines found by colour (lines from the battle outside scroll through
it all the time), anything that depends on the dice: how long the kill
takes, the damage lines, who stands where afterwards.

What the original does around a death (explored 2026-11-16):

  - No corpse stays: the body is gone at once and a ground pile with the
    whole inventory lies on the spot (the pointer over it shows the items'
    outline). The inventory screen of the dead member is completely empty.
  - The portrait keeps its place (no reordering), turns grey and loses its
    hit point numbers and its health bar. On the world screen it cannot be
    selected: a click on it deselects everybody.
  - Record, inventory and spellbook still open for the dead member through
    the portrait on those screens. Gold is the party's and stays (120).
  - Nolan's cutscene (12cNolan) heals; it does not raise the dead.
  - The quick save is not refused, and the dead member is dead after a load;
    the pile is still there. The load screen's small portraits are not
    greyed.

Map points: AR1201 as the save shows it (S25): Nolan's circle (496,153);
Vraedna (435,409); Geldin (467,350), Khatja (421,360), Laeris (391,397),
Sheris (496,199), Kylie (167,361). Main room floor (380,250). Load prompt:
Cancel (337,240), Load (459,240); the quick save is the third row.
"""

from __future__ import annotations

import shutil
import time

import numpy as np
import pytest
from PIL import Image

import test_s13_house as s13
import test_s17_dock_houses as s17
import test_s18_warehouse_caves as s18
import test_s19_town_errands as s19
import test_s20_palisade as s20
import test_s21_lumber as s21
import test_s24_battle_begins as s24
import test_s25_healer_and_sortie as s25
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s8_screens_tour import DIAL, PORTRAIT
from test_s9_docks import DIALOG_PARK, DIALOG_REGION, SELECT_ALL, act
from test_s10_save_load import SCREEN_REGION, SLOT_BUTTON, SLOT_PORTRAITS
from test_s14_travel import select_all

S25_END = s25.S25_END

FIGHT_PARK = s17.FIGHT_PARK  # (520, 515): off the world view
SCREEN_PARK = (300, 560)
PANEL_PARK = (600, 330)
MESSAGES = s17.MESSAGES
LINE_SPOT = s17.LINE_SPOT
WORLD = (0, 0, 800, 430)
PORTRAITS = s25.PORTRAITS
ATTACK = s13.ATTACK
GELDIN, KHATJA, LAERIS, VRAEDNA = PORTRAIT[1], PORTRAIT[2], PORTRAIT[3], PORTRAIT[4]
VICTIM = 4  # Vraedna's portrait
VICTIM_CIRCLE = (435, 409)  # where the save puts her
VICTIM_AIM = (435, 399)
PILE = (435, 405)
ROOM_FLOOR = (380, 250)
TOOLTIP = (695, 440, 62, 45)

RECORD_TEXT = s20.RECORD_TEXT
RECORD_DEAD_TEXT = (405, 63, 300, 150)  # levels, DEAD, effective level, experience
RECORD_HP = (585, 370, 55, 50)
RECORD_BUTTONS = (250, 358, 130, 56)  # Export and Customize
INV_DOLL = (250, 20, 310, 255)  # equipment slots round the (empty) paper doll
INV_BACKPACK = s25.INV_BACKPACK
INV_QUICK = s25.INV_QUICK
INV_WEIGHT = s25.INV_WEIGHT
PILE_ITEMS = s25.CRATE_ITEMS
PACK_ITEMS = s25.PACK_ITEMS

OPTIONS_LOAD = (612, 81)
LOAD_PROMPT = (263, 125, 275, 131)
PROMPT_LOAD = (459, 240)
QUICK_ROW = 2  # Autosave - Prologue, Quick-Save-Backup, Quick-Save


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s25_save(play_dir, out_dir):
    """Put the banked S25 save into mpsave before the game starts; after the
    game has exited remove every save it wrote (nothing is banked)."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S25_END / "save").iterdir() if p.is_dir()] if (S25_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S25 save in {S25_END}; run test_s25_healer_and_sortie --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    yield
    s17.finish_saves(play_dir, out_dir, None)


# --- finding things on screen --------------------------------------------------


def death_line(img):
    """The lowest "Vraedna- Death" line of the message window, moved to
    LINE_SPOT with only its text kept, or None. Her name is pure grey
    (no other text is); "Death" ends at x=104, her damage lines go on to
    x=300 and more."""
    x0, y0, w0, h0 = MESSAGES
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    m = a[y0:y0 + h0, 8:60]
    grey = (np.abs(m[..., 0] - 161) <= 3) & (np.abs(m[..., 1] - 161) <= 3) & (np.abs(m[..., 2] - 161) <= 3)
    x, w, h = LINE_SPOT[0], LINE_SPOT[2], LINE_SPOT[3]
    for start, end in reversed([r for r in s18._runs(grey.any(axis=1)) if r[1] - r[0] >= 6]):
        top = y0 + start - 1
        line = a[top:top + h, x:x + w]
        if line.shape[0] < h:
            continue
        keep = line.max(axis=2) > 110
        keep[:, 495 - x:] = False  # the scroll arrow at the right of the window
        cols = np.nonzero(keep[1:11].any(axis=0))[0]
        if not len(cols) or not 95 <= int(cols.max()) + x <= 112:
            continue
        out = Image.new("RGB", img.size)
        out.paste(Image.fromarray(np.where(keep[..., None], line, 0).astype(np.uint8)), (x, LINE_SPOT[1]))
        return out
    return None


# --- the test ------------------------------------------------------------------


def test_s26_party_death(from_s25_save, game, golden_dir, out_dir, target, play_dir):
    group_bar = s17.load_bank(game, S25_END)
    t0 = time.time()

    failures = []

    def check(name, region, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    def tight(name, region, **kw):
        check(name, region, settle=0.5, max_diff=0.004, **kw)

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

    def click(xy, region, what, park=FIGHT_PARK, **kw):
        act(game, lambda: game.click(*xy), what, region, park=park, **kw)

    def shift_click(xy, region, what, **kw):
        def do():
            game._key(0x10, True)
            time.sleep(0.15)
            game.click(*xy)
            time.sleep(0.15)
            game._key(0x10, False)
        try:
            act(game, do, what, region, park=FIGHT_PARK, **kw)
        finally:
            game._key(0x10, False)

    def world_back(what, timeout=30.0):
        deadline = time.time() + timeout
        while diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) > 0.05:
            if time.time() > deadline:
                raise AssertionError(f"the world screen did not come back ({what})")
            time.sleep(0.2)

    def running(seconds, what):
        """No input for `seconds`: the game is alive (grab checks), the world
        view moves in nearly every second and the game is not paused."""
        game.move(*FIGHT_PARK)
        prev, moved = game.grab(), 0
        for _ in range(seconds):
            time.sleep(1.0)
            cur = game.grab()
            moved += diff_ratio(prev, cur, region=WORLD) > 0.002
            prev = cur
        state = s18.pause_line(prev)
        print(f"S26 {what}: the world view moved in {moved} of {seconds} s, pause line {state}")
        if moved < seconds - 3 or state == "paused":
            failures.append(f"{what}: the game is not running (moved in {moved} of {seconds} s, {state})")

    def dead_record(prefix):
        click(DIAL["record"], ACTION_BAR_REGION, "Record dial button", park=SCREEN_PARK, threshold=0.3)
        click(VRAEDNA, RECORD_TEXT, "Vraedna's portrait on the record", park=SCREEN_PARK)
        check(f"{prefix}record-dead", s20.RECORD_SCREEN)
        tight(f"{prefix}record-dead-text", RECORD_DEAD_TEXT)
        tight(f"{prefix}record-dead-hp", RECORD_HP)
        tight(f"{prefix}record-dead-buttons", RECORD_BUTTONS)
        act(game, lambda: game.press("esc"), "Esc on the record", ACTION_BAR_REGION, park=FIGHT_PARK,
            threshold=0.3)
        select_all(game, group_bar)

    def dead_pile(prefix):
        """The pile where she died: Sheris walks over and its panel opens."""
        select_all(game, group_bar)
        for attempt in range(3):
            game.click(*PILE)
            game.move(*PANEL_PARK)
            deadline = time.time() + 12.0
            while time.time() < deadline and not s13.panel_open(game.grab()):
                time.sleep(0.3)
            if s13.panel_open(game.grab()):
                break
            select_all(game, group_bar)
        else:
            failures.append(f"{prefix}pile: no ground pile opened where Vraedna died")
            return
        time.sleep(1.5)
        tight(f"{prefix}pile-items", PILE_ITEMS)
        tight(f"{prefix}pile-pack", PACK_ITEMS)
        act(game, lambda: game.press("esc"), "Esc on the pile's panel", ACTION_BAR_REGION, park=FIGHT_PARK)
        select_all(game, group_bar)

    # --- Before ---------------------------------------------------------------------------
    select_all(game, group_bar)
    tight("before-portraits", PORTRAITS)
    frame = game.grab()
    assert s25.health(frame)[VICTIM] >= 0, "Vraedna is already dead in the save"
    greens = s25.party_circles(frame)
    assert any(abs(x - VICTIM_CIRCLE[0]) < 15 and abs(y - VICTIM_CIRCLE[1]) < 12 for x, y in greens), (
        f"Vraedna is not where the save puts her: party circles {greens}")

    # --- The kill: Geldin, Khatja and Laeris against Vraedna -------------------------------
    click(GELDIN, ACTION_BAR_REGION, "click on Geldin's portrait")
    shift_click(KHATJA, ACTION_BAR_REGION, "Shift-click on Khatja's portrait", wait=1.0)
    shift_click(LAERIS, PORTRAITS, "Shift-click on Laeris's portrait", wait=1.0, threshold=0.003)
    assert diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) <= 0.05, "no group bar for the attackers"
    tight("attackers-portraits", PORTRAITS)

    line, died, t_kill = None, None, time.time()
    for order in range(4):
        # Later orders: only if her circle still stands clear of the others
        # (an armed click on an attacker would turn them on him).
        frame = game.grab()
        others = [g for g in s25.party_circles(frame)
                  if not (abs(g[0] - VICTIM_CIRCLE[0]) < 15 and abs(g[1] - VICTIM_CIRCLE[1]) < 12)]
        if order and s25.on_member(VICTIM_AIM, others):
            print(f"S26 kill: no new order, an attacker stands over her ({others})")
        else:
            click(ATTACK, ACTION_BAR_REGION, "Attack", wait=0.4, timeout=4.0)
            try:
                click(VICTIM_AIM, ACTION_BAR_REGION, "click on Vraedna with Attack armed", wait=0.0, timeout=3.0)
            except AssertionError:
                game.click(*ATTACK)  # still armed: put it back
                game.move(*FIGHT_PARK)
                time.sleep(0.5)
                continue
        deadline = time.time() + 45.0
        while time.time() < deadline:
            frame = game.grab()
            line = death_line(frame) or line
            if died is None and s25.health(frame)[VICTIM] < 0:
                died = time.time()
                deadline = died + 2.0  # the line can come a moment after the portrait
            if died is not None and line is not None:
                break
            time.sleep(0.1)
        if died is not None:
            break
    assert died is not None, "Vraedna did not die"
    print(f"S26 kill: Vraedna dead {died - t_kill:.1f} s after the first order ({order + 1} order(s))")
    game.screenshot("dead")
    check_frame("death-line", line, LINE_SPOT[:3] + (12,), max_diff=0.004)

    # --- The game goes on -------------------------------------------------------------------
    running(22, "after the death")

    # --- Portraits --------------------------------------------------------------------------
    select_all(game, group_bar)
    tight("dead-portraits", PORTRAITS)
    game.move(*VRAEDNA)
    time.sleep(3.0)
    tight("dead-tooltip", TOOLTIP)
    game.move(*FIGHT_PARK)
    time.sleep(1.0)
    try:
        click(VRAEDNA, PORTRAITS, "click on the dead portrait", threshold=0.003, wait=1.0)
    except AssertionError as e:
        failures.append(str(e))
    tight("dead-portrait-clicked", PORTRAITS)
    tight("dead-portrait-clicked-bar", ACTION_BAR_REGION)
    select_all(game, group_bar)

    # --- Record and inventory of the dead member ----------------------------------------------
    dead_record("")
    click(DIAL["inventory"], ACTION_BAR_REGION, "Inventory dial button", park=SCREEN_PARK, threshold=0.3)
    click(VRAEDNA, SCREEN_REGION, "Vraedna's portrait on the inventory", park=SCREEN_PARK, threshold=0.01)
    check("inventory-dead", SCREEN_REGION)
    tight("inventory-dead-doll", INV_DOLL)
    tight("inventory-dead-backpack", INV_BACKPACK)
    tight("inventory-dead-quick-items", INV_QUICK)
    tight("inventory-dead-weight", INV_WEIGHT)
    act(game, lambda: game.press("esc"), "Esc on the inventory", ACTION_BAR_REGION, park=FIGHT_PARK,
        threshold=0.3)

    # --- The five walk ----------------------------------------------------------------------
    select_all(game, group_bar)
    game.click(*ROOM_FLOOR)
    game.move(*FIGHT_PARK)
    time.sleep(7.0)
    greens = s25.party_circles(game.screenshot("walked"))
    # (Circles that overlap are counted twice or as one, so no exact count:
    # everyone is in the main room, nobody is left by the crate or by Nolan.)
    there = [g for g in greens if 290 <= g[0] <= 500 and 180 <= g[1] <= 365
             and not (abs(g[0] - 496) < 12 and abs(g[1] - 199) < 10)]
    print(f"S26 walk: party circles {greens}")
    if len(there) < 4 or len(there) != len(greens):
        failures.append(f"walk: expected the five in the main room, found circles at {greens}")

    # --- Her things on the floor ---------------------------------------------------------------
    dead_pile("")

    # --- Nolan: the same offer, and no raising ----------------------------------------------
    deadline = time.time() + 15.0
    while not (cyan := s21.cyan_circles(game.grab())):
        if time.time() > deadline:
            raise AssertionError("Nolan's circle did not show")
        time.sleep(0.3)
    act(game, lambda: game.click(*cyan[0]), "click on Nolan", ACTION_BAR_REGION, park=DIALOG_PARK,
        wait=0.0, timeout=20.0)
    deadline = time.time() + 30.0
    while not s20.replies_shown(game.grab()):
        if time.time() > deadline:
            raise AssertionError("Nolan's first line did not show")
        time.sleep(0.3)
    game.move(*DIALOG_PARK)
    check_frame("nolan-first", s19.speech_block(game.wait_stable(settle=1.0, region=DIALOG_REGION)),
                s19.SPEECH_SPOT)
    act(game, lambda: game.press("1"), 'reply 1 to Nolan ("Yes.")', DIALOG_REGION, park=FIGHT_PARK,
        wait=0.0, threshold=0.05)
    time.sleep(1.0)
    world_back("Nolan's healing")
    time.sleep(3.0)
    select_all(game, group_bar)
    tight("nolan-healed-portraits", PORTRAITS)

    # --- Quick save -------------------------------------------------------------------------
    game.press("q")
    game.move(*FIGHT_PARK)
    line, deadline = None, time.time() + 20.0
    while line is None and time.time() < deadline:
        time.sleep(0.2)
        line = s24.saved_line(game.grab())
    check_frame("quick-saved", line, s24.SAVED_SPOT, max_diff=0.004)
    time.sleep(4.0)
    saves = sorted(p.name.split("-", 1)[1] for p in s17.new_saves(play_dir))
    print(f"S26 saves after Q: {saves}")
    if saves != ["Quick-Save", "Quick-Save-Backup"]:
        failures.append(f"quick save: the play folder holds {saves}")

    # --- Load it again ------------------------------------------------------------------------
    select_all(game, group_bar)
    click(DIAL["options"], ACTION_BAR_REGION, "Options dial button", park=SCREEN_PARK, threshold=0.3)
    click(OPTIONS_LOAD, SCREEN_REGION, "Load Game on the options", park=SCREEN_PARK, threshold=0.05)
    tight("load-prompt", LOAD_PROMPT)
    click(PROMPT_LOAD, SCREEN_REGION, "Load on the prompt", park=SCREEN_PARK, threshold=0.3)
    tight("load-slot-portraits", SLOT_PORTRAITS[QUICK_ROW])
    click(SLOT_BUTTON[QUICK_ROW], SCREEN_REGION, "Load on the quick save", threshold=0.3, wait=0.0,
          timeout=20.0)
    world_back("the load", timeout=90.0)
    time.sleep(4.0)
    select_all(game, group_bar)
    tight("loaded-portraits", PORTRAITS)
    dead_record("loaded-")
    dead_pile("loaded-")
    running(6, "after the load")

    print(f"S26 steps after the first load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
