"""S11: drop an item and pick it up, equip and unequip, weapon sets, a spell
cast from the action bar, spell memorisation, and resting.

Continues S6 (whole party walked off the ship onto the dock in AR1000, game
running). Then, with Sheris (portrait 1) selected:

  drop        inventory: her quarterstaff (first quick weapon) onto the first
              ground slot; compare the quick weapons, the ground slots, the
              weight (19 -> 15) and the selected weapon (now a fist); Esc and
              compare the action bar (fist)
  walk away   click on the dock to the left; in the inventory the ground
              slots are now empty
  ground pile click where she stood (found by her bright green selection
              circle before she walked): she walks back and the ground-item
              panel opens with the staff; compare it and its weight; Esc
  pick up     inventory: the ground slot shows the staff again; put it back
              in the first quick weapon slot; compare as after the drop
  equip       still in the inventory, Laeris (portrait 4): armour (top-left
              equipment slot) into the first backpack slot (AC 14 -> 12),
              shield (first off-hand slot) into the second (AC -> 11), then
              both back (AC 14); compare the AC, the weight and the slots
              after each move; Sheris again, Esc
  weapon set  right-click Sheris's quick weapon in the action bar: the four
              weapon sets are offered; pick the second (darts); compare the
              action bar and, in the inventory, the selected set; back to the
              first set the same way
  cast        Laeris: Cast Spell -> Cleric shows Cure Light Wounds and Summon
              Monster I (1 each); cast Cure Light Wounds on Sheris's portrait;
              the cleric list then holds only Summon Monster I
  memorise    spellbook (Laeris, cleric level 1): click the memorised Summon
              Monster I, compare the "remove this spell" prompt, Remove
              (Spell Slots Available 0 -> 1), click Bless in the known list
              (Bless 0/1 memorised, slots 0); Sheris's page, Esc. Laeris stays
              selected on the world screen: on the original a portrait
              clicked inside the inventory or the spellbook does not change
              the world selection
  rest        Rest dial button: the original refuses on the docks with "You
              may not rest at this time." (no movie, no time passes); compare
              that last message line and the action bar (Laeris's)

Not compared: the world view (snow differs from run to run, and the dropped
staff is a few pixels under Sheris's feet), the paper doll (animated), the
gold amount, and where the party stands. Casting prints no message line on
the original, so the cast is proved by the spell lists before and after.
"""

from __future__ import annotations

import time

import numpy as np

from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s2_new_game import new_game_to_opening_dialog
from test_s5_leave_dialog_inventory import (
    ACTION_BAR_REGION,
    DETAILS as INV_DETAILS,
    INV_AC_HP,
    INV_BACKPACK,
    INV_QUICK_WEAPONS,
    finish_hedron_dialog,
)
from test_s6_walk_off_ship import BOX_FROM, BOX_TO, DOCK_SPOT
from test_s8_screens_tour import DIAL, PORTRAIT, SPELL_SLOTS

SHERIS, LAERIS = PORTRAIT[0], PORTRAIT[3]
REST = (720, 511)

PARK_SPOT = (300, 560)  # the message window
HOLD_PARK = (450, 200)  # paper doll, while an item is on the pointer
WATER_PARK = (700, 400)  # water in the world view, off the message lines
PANEL_PARK = (700, 380)  # water, while the ground-item panel is open

# Inventory screen (800x600).
QUICK_WEAPON_1 = (618, 65)  # main hand of the first weapon set
OFF_HAND_1 = (659, 65)
ARMOUR_SLOT = (272, 40)  # top-left equipment slot
BACKPACK = [(270 + 37.5 * i, 318) for i in range(8)]  # first backpack row
BACKPACK = [(int(x), y) for x, y in BACKPACK]
GROUND = [(590 + 40 * c, 318 + 38 * r) for r in range(3) for c in range(2)]
INV_GROUND = (572, 298, 82, 116)  # the six ground slots
INV_EQUIP_TOP = (252, 22, 296, 38)  # top row of equipment slots
INV_WEIGHT = INV_DETAILS["weight"]
INV_AC = INV_DETAILS["armor-class"]
INV_SELECTED = INV_DETAILS["selected-quick-weapon"]

# Action bar (world screen).
BAR_QUICK_WEAPON = (63, 463)
WEAPON_SET_BAR = [(26, 463), (149, 463)]  # first and second set in the set bar
BAR_CAST_SPELL = (174, 463)  # Laeris (cleric)
CAST_CLERIC = (26, 463)  # class choice, then the first spell of the list

# Ground-item panel (S9) without the gold amount; its backpack weight.
PILE_REGION = (0, 478, 640, 122)
PILE_WEIGHT = (418, 500, 75, 72)
WALK_AWAY = (60, 380)  # the dock left of where Sheris stands after S6

# Spellbook (cleric, level 1).
MEMORISED_2 = (384, 95)  # second memorised spell (Summon Monster I)
REMOVE_PROMPT = (255, 135, 285, 155)
REMOVE_BUTTON = (455, 258)
KNOWN_BLESS = (615, 137)
MEMORISED_REGION = (250, 75, 232, 320)

SCREEN_REGION = (0, 0, 800, 430)
LAST_LINE_REGION = (8, 576, 535, 16)


def selected_circle(img):
    """Centre of the bright green selection circle (the one selected party
    member) in the world view, or None."""
    a = np.asarray(img.convert("RGB"), dtype=np.int16)[:430]
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    ys, xs = np.nonzero((g > 230) & (r < 40) & (b < 40))
    if len(xs) < 20:
        return None
    return int(round(xs.mean())), int(round(ys.mean()))


def test_s11_items_spells_rest(game, golden_dir):
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)
    # S6: select the party on the deck and walk it onto the dock.
    game.drag(*BOX_FROM, *BOX_TO)
    time.sleep(1.0)
    game.click(*DOCK_SPOT)
    time.sleep(0.5)
    game.move(*PARK_SPOT)
    time.sleep(19.5)

    # Every checkpoint is taken even if an earlier one differs, so that one
    # run on the rebuild shows all the differences. They are reported together.
    failures = []

    def check(name, region, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    def tight(name, region):
        check(name, region, settle=0.5, max_diff=0.004)

    def act(do, what, region, wait=2.0, park=PARK_SPOT, threshold=0.005, timeout=10.0):
        """Do one input, then wait until `region` has changed (or fail: the
        input was dropped) and give it `wait` more seconds."""
        before = game.grab()
        do()
        game.move(*park)
        deadline = time.time() + timeout
        while diff_ratio(before, game.grab(), region=region) <= threshold:
            if time.time() > deadline:
                raise AssertionError(f"{what} did not change the screen")
            time.sleep(0.2)
        time.sleep(wait)

    def click(xy, region, **kw):
        act(lambda: game.click(*xy), f"click at {xy}", region, **kw)

    def rclick(xy, region, **kw):
        act(lambda: game.click(*xy, button="right"), f"right-click at {xy}", region, **kw)

    def press(key, region, **kw):
        act(lambda: game.press(key), f"key {key!r}", region, **kw)

    def slot(xy):
        return (xy[0] - 18, xy[1] - 18, 36, 36)

    def move_item(src, dst):
        """Pick an item up from one inventory slot and put it in another."""
        click(src, slot(src), park=HOLD_PARK, wait=1.0)
        click(dst, slot(dst), wait=1.5)

    def inventory(prefix, ground=True):
        check(prefix + "-quick-weapons", INV_QUICK_WEAPONS)
        check(prefix + "-ac-hp", INV_AC_HP)
        tight(prefix + "-armor-class", INV_AC)
        tight(prefix + "-weight", INV_WEIGHT)
        tight(prefix + "-selected-quick-weapon", INV_SELECTED)
        if ground:
            check(prefix + "-ground", INV_GROUND, max_diff=0.01)

    # Sheris alone (normally she is selected already: this may change nothing).
    game.click(*SHERIS)
    game.move(*PARK_SPOT)
    time.sleep(1.5)
    pile_spot = selected_circle(game.grab())
    assert pile_spot is not None, "no selected party member found in the world view"

    # --- Drop the quarterstaff on the ground --------------------------------
    click(DIAL["inventory"], ACTION_BAR_REGION, wait=3.0)
    inventory("start")
    move_item(QUICK_WEAPON_1, GROUND[0])
    inventory("dropped")
    press("esc", ACTION_BAR_REGION)
    check("dropped-action-bar", ACTION_BAR_REGION)

    # --- Walk away: the ground slots follow the character -------------------
    game.click(*WALK_AWAY)
    game.move(*PARK_SPOT)
    time.sleep(5.0)
    click(DIAL["inventory"], ACTION_BAR_REGION, wait=3.0)
    check("away-ground", INV_GROUND, max_diff=0.01)
    press("esc", ACTION_BAR_REGION)

    # --- The pile in the world ----------------------------------------------
    click(pile_spot, PILE_REGION, park=PANEL_PARK, timeout=20.0)
    check("pile-panel", PILE_REGION)
    tight("pile-panel-weight", PILE_WEIGHT)
    press("esc", ACTION_BAR_REGION)

    # --- Pick it up from the inventory ground slot --------------------------
    click(DIAL["inventory"], ACTION_BAR_REGION, wait=3.0)
    check("back-ground", INV_GROUND, max_diff=0.01)
    move_item(GROUND[0], QUICK_WEAPON_1)
    inventory("picked-up")

    # --- Laeris: armour and shield off and on -------------------------------
    click(LAERIS, SCREEN_REGION)
    inventory("laeris", ground=False)
    check("laeris-equipment", INV_EQUIP_TOP)
    move_item(ARMOUR_SLOT, BACKPACK[0])
    inventory("laeris-no-armour", ground=False)
    check("laeris-no-armour-equipment", INV_EQUIP_TOP)
    check("laeris-no-armour-backpack", INV_BACKPACK)
    move_item(OFF_HAND_1, BACKPACK[1])
    inventory("laeris-no-shield", ground=False)
    check("laeris-no-shield-backpack", INV_BACKPACK)
    move_item(BACKPACK[0], ARMOUR_SLOT)
    move_item(BACKPACK[1], OFF_HAND_1)
    inventory("laeris-again", ground=False)
    check("laeris-again-equipment", INV_EQUIP_TOP)
    check("laeris-again-backpack", INV_BACKPACK)
    click(SHERIS, SCREEN_REGION)
    press("esc", ACTION_BAR_REGION)

    # --- Weapon sets from the action bar ------------------------------------
    rclick(BAR_QUICK_WEAPON, ACTION_BAR_REGION)
    check("weapon-sets-bar", ACTION_BAR_REGION)
    click(WEAPON_SET_BAR[1], ACTION_BAR_REGION)
    check("set-2-action-bar", ACTION_BAR_REGION)
    click(DIAL["inventory"], ACTION_BAR_REGION, wait=3.0)
    check("set-2-quick-weapons", INV_QUICK_WEAPONS)
    press("esc", ACTION_BAR_REGION)
    rclick(BAR_QUICK_WEAPON, ACTION_BAR_REGION)
    click(WEAPON_SET_BAR[0], ACTION_BAR_REGION)
    check("set-1-action-bar", ACTION_BAR_REGION)

    # --- Cast Cure Light Wounds on Sheris -----------------------------------
    click(LAERIS, ACTION_BAR_REGION)
    check("laeris-action-bar", ACTION_BAR_REGION)
    click(BAR_CAST_SPELL, ACTION_BAR_REGION)
    check("cast-classes", ACTION_BAR_REGION)
    click(CAST_CLERIC, ACTION_BAR_REGION)
    check("cast-cleric-spells", ACTION_BAR_REGION)
    click(CAST_CLERIC, ACTION_BAR_REGION)  # Cure Light Wounds: the bar returns
    # Target Sheris's portrait. Laeris walks over and casts; nothing still
    # changes on the screen, so wait for it.
    game.click(*SHERIS)
    game.move(*PARK_SPOT)
    time.sleep(8.0)
    click(BAR_CAST_SPELL, ACTION_BAR_REGION)
    click(CAST_CLERIC, ACTION_BAR_REGION)
    check("cast-cleric-spells-after", ACTION_BAR_REGION)
    press("esc", ACTION_BAR_REGION)
    press("esc", ACTION_BAR_REGION)

    # --- Spellbook: Summon Monster I out, Bless in --------------------------
    click(DIAL["spellbook"], ACTION_BAR_REGION, wait=3.0)
    check("spellbook", SCREEN_REGION)
    check("spellbook-memorised", MEMORISED_REGION)
    tight("spellbook-slots", SPELL_SLOTS)
    click(MEMORISED_2, SCREEN_REGION)
    check("remove-prompt", REMOVE_PROMPT)
    click(REMOVE_BUTTON, SCREEN_REGION)
    check("removed-memorised", MEMORISED_REGION)
    tight("removed-slots", SPELL_SLOTS)
    click(KNOWN_BLESS, MEMORISED_REGION)
    check("bless-memorised", MEMORISED_REGION)
    tight("bless-slots", SPELL_SLOTS)
    click(SHERIS, SCREEN_REGION)
    press("esc", ACTION_BAR_REGION)
    check("after-spellbook-action-bar", ACTION_BAR_REGION)

    # --- Rest ---------------------------------------------------------------
    game.move(*WATER_PARK)
    time.sleep(0.5)
    click(REST, LAST_LINE_REGION, park=WATER_PARK, threshold=0.001)
    tight("rest-refused-line", LAST_LINE_REGION)
    check("rest-action-bar", ACTION_BAR_REGION)

    assert not failures, "; ".join(failures)
