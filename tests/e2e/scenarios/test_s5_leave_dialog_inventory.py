"""S5: finish Hedron's dialogue, then open the inventory.

Continues S4: two Continues, reply 3 ("So you're off, then?"), then reply 1
and Continue alternately until the dialogue ends with "We'll be careful.
Farewell, Hedron." Then press I and compare the static parts of the
inventory screen.

Not compared: the gold amount (random at party creation), the paper doll
(animated), and the pause notices under the last line of dialogue (their
order depends on timing)."""

from __future__ import annotations

import time

from checkpoints import checkpoint
from test_s2_new_game import new_game_to_opening_dialog
from test_s4_hedron_continue import CONTINUE

# First line of the message window on the world screen.
LAST_LINE_REGION = (8, 505, 535, 20)
# Action bar and portraits, which only appear once the dialogue is over.
ACTION_BAR_REGION = (0, 435, 500, 55)

# Inventory screen (800x600).
INV_NAME_PORTRAIT = (20, 20, 215, 395)
INV_QUICK_WEAPONS = (570, 20, 215, 110)
INV_AC_HP = (700, 170, 85, 230)
INV_BACKPACK = (250, 300, 310, 115)
# The six party portraits with their frames and status icons.
INV_PORTRAITS = (500, 435, 300, 55)
DETAILS = {
    "armor-class": (715, 248, 50, 24),
    "hit-points": (715, 352, 50, 42),
    "weight": (340, 280, 120, 16),
    "selected-quick-weapon": (598, 46, 42, 42),
}
# The hit point labels in the top-left corner of each of the six portraits.
# Compared one portrait at a time so that a wrong number is not averaged away.
for _i in range(6):
    DETAILS["portrait-%d-hit-points" % (_i + 1)] = (507 + 49 * _i, 437, 32, 11)


def finish_hedron_dialog(game):
    for _ in range(4):
        game.press("1")
        time.sleep(2.0)
    for _ in range(2):
        game.click(*CONTINUE)
        time.sleep(3.0)
    game.press("3")
    time.sleep(3.0)
    for _ in range(9):
        game.press("1")
        time.sleep(3.0)
        game.click(*CONTINUE)
        time.sleep(3.0)


def test_s5_leave_dialog_inventory(game, golden_dir):
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)

    checkpoint(game, golden_dir, "world-last-line", region=LAST_LINE_REGION, settle=2.0)
    checkpoint(game, golden_dir, "world-action-bar", region=ACTION_BAR_REGION, settle=1.0)

    game.press("i")
    time.sleep(3.0)
    checkpoint(game, golden_dir, "inventory-name-portrait", region=INV_NAME_PORTRAIT, settle=1.0)
    checkpoint(game, golden_dir, "inventory-quick-weapons", region=INV_QUICK_WEAPONS, settle=1.0)
    checkpoint(game, golden_dir, "inventory-ac-hp", region=INV_AC_HP, settle=1.0)
    checkpoint(game, golden_dir, "inventory-backpack", region=INV_BACKPACK, settle=1.0)
    checkpoint(game, golden_dir, "inventory-portraits", region=INV_PORTRAITS, settle=1.0, max_diff=0.004)
    # Small regions holding numbers or a single highlight: a wrong value only
    # changes a few pixels, so these use a much tighter limit.
    for name, region in DETAILS.items():
        checkpoint(game, golden_dir, "inventory-" + name, region=region, settle=0.5, max_diff=0.004)

    game.press("esc")
    time.sleep(2.0)
    checkpoint(game, golden_dir, "world-after-inventory", region=ACTION_BAR_REGION, settle=1.0)
