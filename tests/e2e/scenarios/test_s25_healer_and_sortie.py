"""S25: getting ready in Shawford's house while the goblins attack the Palisade,
and a first sortie against Phaen of the Rags. Starts from S24's banked save
(inside the house, AR1201, straight after the cutscene that starts the battle)
and banks one of its own before going out.

This is not the scenario that was asked for (the fight up to the first
Caballus cutscene). That fight could not be made repeatable with this party;
what was tried and why it fails is in "The battle" below. What is here is the
part that is the same in every run, and then 45 seconds of the fight with
nothing compared.

Saves (golden/bank/, git-ignored like all goldens):

  s24-end/  S24's end (see test_s24_battle_begins), loaded here
  s25-end/  this test's save (on the original only): still inside the house,
            before the sortie. Nolan has been asked for healing once
            (12Heard_Heal_2 = 1: "Yes." now heals at once), the crate is open
            and empty, Kylie carries Flaming Oil x3 in her first quick-item
            slot and a Potion of Healing in the second, everyone unhurt, whole
            party selected, Vraedna 4 casts (Magic Missile / Sleep), Laeris
            one Cure Light Wounds and one domain spell. The battle outside is
            as in s24-end (the Weapon Instructor is dead by then).

test_s25_healer_and_sortie (both targets) loads the s24-end save, then:

  Nolan       the healer in the back room (the only cyan circle): his battle
              line "Damnable goblins!  Are you in need of healing?" and the two
              replies (text only, as S19); 1 ("Yes."): "All right, then..."
              with End Dialog (compare); End Dialog runs the 12cNolan cutscene
              (about 4 s, the GUI stays) and the world screen is back.
              Portraits compared (everyone unhurt, all selected)
  crate       Kylie alone: Thieving, the crate in the store room (locked,
              difficulty 20): "Kylie- Lock Pick Succeeded" (found by the
              orange of her name, as S18; a failure is retried); a second
              click opens the container panel: Flaming Oil (3) and a Potion of
              Healing; each is taken with a click on the first slot (compare
              the crate's slots and the backpack's after each)
  inventory   I: both items dragged from the backpack to the first two quick
              item slots (compare backpack, quick slots, weight); Esc; Kylie's
              action bar with the two items (compare)
  save        Select All, Q: the green "Quick-save successful" line (found by
              colour, as S24); on the original the save is banked
  sortie      the exit: AR1200. Phaen's six are there at once (at least five
              hostile circles must show within 4 s). Space at once (PAUSED),
              and with the game paused: Select All, Attack, Phaen (the circle
              nearest to where he is created, 2385,2717); Kylie, the Flaming
              Oil button, Phaen; Vraedna, Cast Spell (compare the cast bar:
              Magic Missile 4, Sleep 4), Sleep, the middle of his group. Space,
              and then up to 14 rounds of about 3 s (pause, everyone attacks
              the hostile nearest to the party that can be clicked without
              hitting a party member, Kylie throws another flask at Phaen
              while he can be followed, Vraedna a Magic Missile, twice). Nothing
              of it is compared and no outcome is required: only that the game
              is still running at the end. What happened is printed (hostile
              circles and the six health bars per round).

Not compared: the world view anywhere, the message window (lines from the
fight outside scroll through it all the time, also in the house), the first
dialogue panel except its text, anything of the sortie except the cast bar.

The battle (explored on the original, 2026-11-16; scripts 12CABA05, 12PHAEN0,
12cSpn1a/b, AR1200):

  - The first Caballus cutscene (12cSpn1a) is started by 12CABA05 when
    GOBLIN_TEAM_1/2/3_STATE are all 2 (all 14 of those dead). Phaen's team 8
    is not needed for it, but it is created 300 px from the house door and is
    on the party the moment it steps out.
  - Waiting does not help: after a minute in the house the 14 are all alive on
    the training ground (about 2050,2150); they kill the Weapon Instructor and
    stand there. Olap, Black Geoffrey and Blanchard are far north.
  - Phaen (18 hit points) casts at once and keeps casting: Sleep (lands about
    4 s after the party arrives and puts one to three members down for close
    to a minute), Color Spray, Ray of Enfeeblement, Blur, Burning Hands (19
    fire damage: it also kills his own goblins and can kill Sheris or Vraedna
    outright), and he goes invisible. His spells are finite (HaveSpell checks).
  - The house is a refuge: the doorway takes the party in with enemies next
    to it (3.2 s from the arrival spot, an Auto-Save is written) and nothing
    follows. Nolan heals everyone for nothing, any number of times. But a
    sleeping member cannot walk, so there is no way back in once Phaen's
    Sleep has landed, and a retreat order that fails costs 9 s of not
    fighting. A sortie of 1.0 s was already too long (retreat at 1.3 s:
    three asleep at the door); 0.3 s got back in, by about half a second.
  - "You cannot save at this time." with enemies near (Q outside).
  - With Attack armed a click on a party member orders the party to kill
    him: that, not Charm, is what killed Khatja in S24's attempt. Aims here
    are checked against the party's (pure green) circles first.
  - Three full attempts (Sleep on his group in the first frame, everyone on
    Phaen, two Magic Missiles on him): one killed Phaen's six in about 50 s
    of game time with nobody lost, but Vraedna was dead and the rest badly
    hurt a few minutes later with most of teams 1-3 still alive; in one
    Vghotan, his hounds and the rest of teams 1-3 arrived at about 50 s with
    two members asleep, and the party was wiped out; one was lost to the
    attack order on Geldin. The 14 of teams 1-3 come by themselves 40-100 s
    after the party is first seen outside. Nothing here is the same twice,
    and nothing came close to three wins in a row. The flaming oil does 5-6
    fire damage to Phaen and kills a goblin next to him. While its button is
    armed a click on a portrait throws it at that member, and a sleeping
    member's portrait cannot be selected.

  What a later scenario could try: start earlier (from s23-end: rest at the
  inn so Laeris has her spells, spend the 370 gold on potions and Sleep
  scrolls, then make the report), and in the fight keep everyone but two
  fighters by the door so a retreat is always possible.

Map points: AR1201 as the save shows it: screen = map - (96,8); Nolan's
circle (496,153); the crate (container "Crate", box 215-239,334-365, locked,
Flaming Oil x3 and Potion of Healing) at screen (131,342); exit To1200 at
(60,380). Entered from outside the view is S20's (Nolan at (593,22), exit
(129,300)). AR1200 on leaving the house: screen = map - (2138,2345); the
doorway (2707,2475) at (569,130); Phaen's team at 2317-2461,2612-2736.
Container panel: first slot (170,515), second (213,515). Inventory: backpack
(270,318), (270,356); quick item slots (593,246), (631,246), (669,246); on the
action bar of a character with seven buttons before them the quick items are
at (350,463) and (391,463).
"""

from __future__ import annotations

import shutil
import time

import numpy as np
import pytest

import test_s13_house as s13
import test_s16_clear_docks as s16
import test_s17_dock_houses as s17
import test_s18_warehouse_caves as s18
import test_s19_town_errands as s19
import test_s20_palisade as s20
import test_s21_lumber as s21
import test_s24_battle_begins as s24
from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION
from test_s8_screens_tour import PORTRAIT
from test_s9_docks import DIALOG_PARK, DIALOG_REGION, SELECT_ALL, act
from test_s14_travel import select_all

S24_END = s24.S24_END
S25_END = s17.BANK / "s25-end"

FIGHT_PARK = s17.FIGHT_PARK  # (520, 515): off the world view
SCREEN_PARK = (300, 560)
PANEL_PARK = (600, 330)
MESSAGES = s17.MESSAGES
LINE_SPOT = s17.LINE_SPOT
PORTRAITS = (500, 435, 300, 55)
CAST_BAR = s20.CAST_BAR
CAST_SPELL = s20.CAST_SPELL
MM_ICON, SLEEP_ICON = s20.SPELL_ICONS
ATTACK = s13.ATTACK
THIEVING = s13.THIEVING
KYLIE, VRAEDNA = PORTRAIT[5], PORTRAIT[4]

HOUSE_EXIT = s24.BATTLE_EXIT  # (60, 380) in the view the save shows
CRATE = (131, 342)
PANEL = (0, 478, 800, 122)
PANEL_SLOT = (170, 515)
CRATE_ITEMS = (145, 492, 225, 90)
PACK_ITEMS = (415, 492, 180, 90)  # the backpack picture (its weight) and its slots
BACKPACK = [(270, 318), (270, 356)]
QUICK_SLOT = [(593, 246), (631, 246)]
INV_BACKPACK = (250, 298, 305, 116)
INV_QUICK = (572, 228, 117, 36)
INV_WEIGHT = s20.INV_WEIGHT
OIL_BUTTON = (350, 463)  # Kylie's first quick item on the action bar
OIL_ON_BAR = (334, 447, 34, 34)

ARRIVAL = (2538, 2560)  # AR1200 on leaving the house: screen = map - (2138, 2345)
PHAEN = (2385, 2717)
ROUNDS = 14


# --- saves ---------------------------------------------------------------------


@pytest.fixture
def from_s24_save(play_dir, out_dir, target):
    """Put the banked S24 save into mpsave before the game starts; after the
    game has exited bank this test's quick save (original only) and remove
    every save it wrote."""
    assert "gog games" not in str(play_dir).lower(), play_dir
    banked = [p for p in (S24_END / "save").iterdir() if p.is_dir()] if (S24_END / "save").is_dir() else []
    if len(banked) != 1:
        pytest.skip(f"no S24 save in {S24_END}; run test_s24_battle_begins --target original first")
    for p in s17.new_saves(play_dir):
        s17.rmtree_retry(p)
    (play_dir / "mpsave").mkdir(exist_ok=True)
    shutil.copytree(banked[0], play_dir / "mpsave" / banked[0].name)
    if target == "original":
        S25_END.mkdir(parents=True, exist_ok=True)
    yield
    s17.finish_saves(play_dir, out_dir, S25_END if target == "original" else None)


# --- finding things on screen --------------------------------------------------


def party_circles(img):
    """Centres of the party's selection circles (pure green) in the world view."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[:430]
    ys, xs = np.nonzero((a[..., 0] < 10) & (a[..., 1] > 245) & (a[..., 2] < 10))
    pts = np.stack([xs, ys], axis=1) if len(xs) else np.zeros((0, 2), int)
    found = []
    while len(pts):
        near = (np.abs(pts[:, 0] - pts[0, 0]) < 40) & (np.abs(pts[:, 1] - pts[0, 1]) < 30)
        cx, cy = pts[near].mean(axis=0)
        near = (np.abs(pts[:, 0] - cx) < 22) & (np.abs(pts[:, 1] - cy) < 15)
        if not near.any():
            near = np.zeros(len(pts), bool)
            near[0] = True
        group = pts[near]
        if len(group) >= 8:
            found.append((int(group[:, 0].mean()), int(group[:, 1].mean())))
        pts = pts[~near]
    return found


def hostiles(img):
    """Hostile circles away from the edges of the view (an aim above one at
    the top edge is the window's title bar, see S18)."""
    return [(x, y) for x, y in s13.hostile_circles(img) if 40 <= y <= 420 and 20 <= x <= 780]


def on_member(aim, greens):
    """The aim is on, or too near, a party member's body: with Attack armed
    a click there orders the party to kill him."""
    return any(abs(aim[0] - gx) <= 20 and -58 <= aim[1] - gy <= 10 for gx, gy in greens)


def safe_aims(target, greens):
    return [aim for aim in ((target[0] + dx, target[1] + dy)
                            for dx, dy in ((0, -10), (0, -2), (8, -6), (-8, -6), (0, -18), (0, 5)))
            if not on_member(aim, greens)]


def health(img):
    """Per portrait: how much of the health bar under it is filled (0..1),
    or -1 for a dead member (grey portrait)."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)
    out = []
    for i in range(6):
        p = a[446:482, 508 + 49 * i:548 + 49 * i]
        if float((p.max(axis=2) - p.min(axis=2)).mean()) < 12:
            out.append(-1.0)
            continue
        row = a[486, 507 + 49 * i:551 + 49 * i]
        out.append(round(float((row.max(axis=1) > 120).mean()), 2))
    return out


# --- the test ------------------------------------------------------------------


def test_s25_healer_and_sortie(from_s24_save, game, golden_dir, out_dir, target):
    group_bar = s17.load_bank(game, S24_END)
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

    def world_back(what, timeout=30.0):
        deadline = time.time() + timeout
        while diff_ratio(group_bar, game.grab(), region=ACTION_BAR_REGION) > 0.05:
            if time.time() > deadline:
                raise AssertionError(f"the world screen did not come back ({what})")
            time.sleep(0.2)

    # --- Nolan the Healer ---------------------------------------------------------------
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
    check_frame("nolan-01", s19.speech_block(game.wait_stable(settle=1.0, region=DIALOG_REGION)),
                s19.SPEECH_SPOT)
    act(game, lambda: game.press("1"), 'reply 1 to Nolan ("Yes.")', DIALOG_REGION, park=DIALOG_PARK)
    check("nolan-02", DIALOG_REGION)
    act(game, lambda: game.click(*CONTINUE), "End Dialog with Nolan", DIALOG_REGION, park=FIGHT_PARK,
        wait=0.0, threshold=0.05)
    world_back("Nolan's healing")
    time.sleep(3.0)
    select_all(game, group_bar)
    tight("healed-portraits", PORTRAITS)

    # --- The crate: Kylie picks the lock ----------------------------------------------
    found = None
    click(KYLIE, ACTION_BAR_REGION, "click on Kylie's portrait")
    for attempt in range(5):
        click(THIEVING, ACTION_BAR_REGION, "Thieving", wait=0.5)
        before = s18.kylie_line(game.grab())
        game.click(*CRATE)
        game.move(*FIGHT_PARK)
        deadline, now = time.time() + 20.0, None
        while time.time() < deadline:
            now = s18.kylie_line(game.grab())
            if now is not None and (before is None or diff_ratio(before[0], now[0], region=LINE_SPOT) > 0):
                break
            time.sleep(0.2)
        else:
            raise AssertionError("Kylie printed no line at the crate")
        found = now
        if found[1] > 160:  # "Lock Pick Succeeded"; "Lock Pick Failed" ends near 150
            break
        time.sleep(1.0)
    else:
        raise AssertionError("five lock picks failed")
    # (Without the two bottom rows: the tops of whatever line came next.)
    check_frame("crate-lock-pick-line", found[0], LINE_SPOT[:3] + (12,), max_diff=0.004)

    for attempt in range(3):
        game.click(*CRATE)
        game.move(*PANEL_PARK)
        deadline = time.time() + 8.0
        while time.time() < deadline and not s13.panel_open(game.grab()):
            time.sleep(0.3)
        if s13.panel_open(game.grab()):
            break
    else:
        raise AssertionError("the crate never opened")
    time.sleep(1.5)
    tight("crate-items", CRATE_ITEMS)
    tight("crate-pack", PACK_ITEMS)
    click(PANEL_SLOT, PANEL, "click on the Flaming Oil in the crate", park=PANEL_PARK, threshold=0.001,
          wait=1.0)
    tight("crate-items-oil-taken", CRATE_ITEMS)
    tight("crate-pack-oil-taken", PACK_ITEMS)
    click(PANEL_SLOT, PANEL, "click on the potion in the crate", park=PANEL_PARK, threshold=0.001,
          wait=1.0)
    tight("crate-items-empty", CRATE_ITEMS)
    tight("crate-pack-both", PACK_ITEMS)
    act(game, lambda: game.press("esc"), "Esc on the container panel", ACTION_BAR_REGION,
        park=FIGHT_PARK)

    # --- Inventory: both to the quick item slots ----------------------------------------
    act(game, lambda: game.press("i"), "I (inventory)", ACTION_BAR_REGION, park=SCREEN_PARK)
    for (src, dst, what) in ((BACKPACK[0], QUICK_SLOT[0], "the Flaming Oil"),
                             (BACKPACK[1], QUICK_SLOT[1], "the potion")):
        act(game, lambda: game.drag(*src, *dst), f"drag of {what} to a quick item slot", INV_QUICK,
            park=SCREEN_PARK, threshold=0.01, wait=1.0)
    tight("inventory-backpack", INV_BACKPACK)
    tight("inventory-quick-items", INV_QUICK)
    tight("inventory-weight", INV_WEIGHT)
    act(game, lambda: game.press("esc"), "Esc on the inventory", ACTION_BAR_REGION, park=FIGHT_PARK)
    tight("kylie-bar", CAST_BAR)
    kylie_bar = game.grab()

    # --- Quick save for a later scenario (as S24: the line found by colour) ---------------
    game.click(*SELECT_ALL)
    game.move(*FIGHT_PARK)
    time.sleep(1.5)
    select_all(game, group_bar)
    game.grab().save(out_dir / "bank-world.png")
    game.press("q")
    game.move(*FIGHT_PARK)
    line, deadline = None, time.time() + 20.0
    while line is None and time.time() < deadline:
        time.sleep(0.2)
        line = s24.saved_line(game.grab())
    check_frame("quick-saved", line, s24.SAVED_SPOT, max_diff=0.004)
    time.sleep(3.0)
    if target == "original" and line is not None and not failures:
        (out_dir / "bank-ok").write_text("ok")
    print(f"S25 steps in the house took {time.time() - t0:.0f} s")

    # --- The sortie -----------------------------------------------------------------------
    select_all(game, group_bar)
    act(game, lambda: game.click(*HOUSE_EXIT), "click on the exit of Shawford's house", ACTION_BAR_REGION,
        park=FIGHT_PARK, threshold=0.5, wait=0.0, timeout=25.0)
    world_back("AR1200", timeout=60.0)
    deadline = time.time() + 4.0
    while len(hostiles(game.grab())) < 5 and time.time() < deadline:
        time.sleep(0.05)
    s18.set_paused(game, True, "the sortie")
    frame = game.screenshot("sortie-paused")
    reds, greens = hostiles(frame), party_circles(frame)
    print(f"S25 sortie: {len(reds)} hostile circle(s) {reds}, party circles {greens}")
    if len(reds) < 5:
        failures.append(f"sortie: {len(reds)} hostile circle(s) outside the house, expected Phaen's six")

    def paused(want):
        try:
            s18.set_paused(game, want, "the sortie")
            return True
        except AssertionError as e:
            print(f"S25 sortie: {str(e)[:90]}")
            return False

    def attack(reds, greens, prefer):
        """Paused: Select All, Attack, a hostile (the preferred one, else the
        nearest to the party) at a point that is on no party member."""
        if not reds:
            return "nobody to attack"
        cx, cy = (np.median([g[0] for g in greens]), np.median([g[1] for g in greens])) if greens else (400, 215)
        order = sorted(reds, key=lambda t: (t[0] - cx) ** 2 + ((t[1] - cy) * 1.4) ** 2)
        if prefer is not None:
            order.sort(key=lambda t: t != prefer)
        aims = [aim for t in order for aim in safe_aims(t, greens)][:8]
        if not aims:
            return "no aim clear of the party"
        game.click(*SELECT_ALL)
        game.move(*FIGHT_PARK)
        time.sleep(0.4)
        try:
            click(ATTACK, ACTION_BAR_REGION, "Attack", wait=0.4, timeout=4.0)
        except AssertionError:
            return "Attack did not arm"
        for aim in aims:
            try:
                click(aim, ACTION_BAR_REGION, "click on a hostile", wait=0.3, timeout=1.5)
                return f"attack at {aim}"
            except AssertionError:
                continue
        game.click(*ATTACK)  # still armed: put it back
        game.move(*FIGHT_PARK)
        time.sleep(0.4)
        return "no click took"

    def throw_oil(aim):
        # The quick item button shows no change when pressed, so it cannot be
        # proved; and while it is armed a click on a portrait throws the
        # flask at that member (this killed Vraedna once). The aim follows
        # at once, and the bar is checked to be Kylie's first.
        try:
            click(KYLIE, ACTION_BAR_REGION, "Kylie's portrait", wait=0.3, timeout=4.0, threshold=0.05)
        except AssertionError as e:
            return str(e)[:60]
        if diff_ratio(kylie_bar, game.grab(), region=OIL_ON_BAR) > 0.05:
            return "no Flaming Oil on Kylie's bar"
        game.click(*OIL_BUTTON)
        game.move(*FIGHT_PARK)
        time.sleep(0.5)
        game.click(*aim)
        game.move(*FIGHT_PARK)
        time.sleep(0.4)
        return f"oil at {aim}"

    def vraedna_casts(icon, spot, bar_check=None):
        try:
            click(VRAEDNA, ACTION_BAR_REGION, "Vraedna's portrait", wait=0.3, timeout=4.0, threshold=0.05)
            click(CAST_SPELL, ACTION_BAR_REGION, "Cast Spell (Vraedna)", wait=0.4, timeout=4.0)
            if bar_check:
                tight(bar_check, CAST_BAR)
            click(icon, ACTION_BAR_REGION, "spell icon", wait=0.4, timeout=4.0)
        except AssertionError as e:
            if bar_check:
                failures.append(f"{bar_check}: {str(e).splitlines()[0]}")
            return str(e)[:60]
        game.click(*spot)
        game.move(*FIGHT_PARK)
        time.sleep(0.4)
        return f"cast at {spot}"

    def nearest(reds, spot, limit):
        near = [r for r in reds if (r[0] - spot[0]) ** 2 + (r[1] - spot[1]) ** 2 < limit ** 2]
        return min(near, key=lambda r: (r[0] - spot[0]) ** 2 + (r[1] - spot[1]) ** 2) if near else None

    # The first orders, with the game paused in its first second outside.
    phaen = nearest(reds, s16.to_screen(ARRIVAL, PHAEN), 80)
    oil, casts = 3, 4
    print(f"S25 sortie: Phaen at {phaen}; {attack(reds, greens, phaen)}")
    if phaen is not None and (aims := safe_aims(phaen, greens)):
        print(f"S25 sortie: {throw_oil(aims[0])}")
        oil -= 1
    away = [r for r in reds if not any(abs(r[0] - g[0]) < 60 and abs(r[1] - g[1]) < 50 for g in greens)] or reds
    spot = ((int(np.mean([r[0] for r in away])), int(np.mean([r[1] for r in away])) - 15)
            if away else s16.to_screen(ARRIVAL, PHAEN))
    print(f"S25 sortie: Sleep, {vraedna_casts(SLEEP_ICON, spot, bar_check='battle-cast-bar')}")
    casts -= 1
    running = paused(False)

    # The fight: not compared, no outcome required.
    hp, strikes = health(frame), 0
    for rnd in range(1, ROUNDS + 1):
        time.sleep(3.0)
        game.check_alive()
        if not paused(True):
            strikes += 1
            if strikes >= 3:
                break
            continue
        frame = game.grab()
        game.shots += 1
        frame.save(out_dir / f"{game.shots:03d}-sortie-round-{rnd:02d}.png")
        reds, greens, hp = hostiles(frame), party_circles(frame), health(frame)
        phaen = nearest(reds, phaen, 60) if phaen is not None else None
        did = [attack(reds, greens, phaen)]
        if phaen is not None and (aims := safe_aims(phaen, greens)):
            if oil and hp[5] >= 0:
                did.append(throw_oil(aims[0]))
                oil -= 1
            if casts > 1 and hp[4] >= 0:
                did.append("Magic Missile, " + vraedna_casts(MM_ICON, aims[0]))
                casts -= 1
        print(f"S25 sortie round {rnd}: {len(reds)} hostile, health {hp}, Phaen {phaen}: {'; '.join(did)}")
        running = paused(False)
        if all(h < 0 for h in hp):
            break
    time.sleep(2.0)
    game.check_alive()
    game.screenshot("sortie-end")
    print(f"S25 sortie ended: health {hp} ({sum(h < 0 for h in hp)} dead), game "
          f"{'running' if running else 'left paused'}")

    print(f"S25 steps after the load took {time.time() - t0:.0f} s")
    assert not failures, "; ".join(failures)
