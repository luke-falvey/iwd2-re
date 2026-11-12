"""S6: select the whole party and walk it off the ship.

Continues S5 (Hedron's dialogue finished, party on the deck of the ship in
area AR1000, game running). Drags a selection box around the party on the
deck, compares the portraits and the action bar (all six selected, formation
buttons shown), then left-clicks on the dock next to the gangplank, waits for
the walk to finish and compares the portraits, the action bar, the message
window and the world view.

Not compared closely: where exactly the characters stand on the dock and the
deck. Three characters usually stay near the foot of the gangplank on the
deck and their idle poses and spots vary from run to run on the original
itself, so the world view only has a generous limit and only has to look
roughly the same (the party on the dock, the walk marker, the deck emptied).

No area transition was found: AR1000 is only the ship and a few piers, the
upper pier is not reachable on foot (the party walks back onto the ship), and
the left and bottom edges of the map are reached without finding a door. So
there is no second part that changes areas.

Known flake: on one original run in about nine, Hedron started a second
dialogue ("Not right away. We've got cargo to unload...") while the party was
walking. That run fails the message-window checkpoint on the original too."""

from __future__ import annotations

import time

from checkpoints import checkpoint
from test_s2_new_game import new_game_to_opening_dialog
from test_s5_leave_dialog_inventory import finish_hedron_dialog

# Selection box around the six party members standing on the deck.
BOX_FROM = (270, 110)
BOX_TO = (430, 280)
# A spot on the dock next to the foot of the gangplank.
DOCK_SPOT = (150, 340)
# Somewhere harmless to leave the pointer: the middle of the message window.
PARK_SPOT = (300, 560)

ACTION_BAR_REGION = (0, 435, 500, 55)
PORTRAITS_REGION = (500, 435, 300, 55)
MESSAGE_REGION = (8, 500, 535, 90)
WORLD_REGION = (0, 0, 800, 430)
DOCK_REGION = (0, 260, 420, 170)


def unsettled_checkpoint(game, golden_dir, name, **kw):
    """Checkpoint of a region that never stops changing (water and idle
    animations), so it is grabbed without waiting for the picture to settle."""
    stable = game.wait_stable
    game.wait_stable = lambda **_: game.grab()
    try:
        return checkpoint(game, golden_dir, name, **kw)
    finally:
        game.wait_stable = stable


def test_s6_walk_off_ship(game, golden_dir):
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)

    # Every checkpoint is taken even if an earlier one differs, so that one
    # run on the rebuild shows all the differences. They are reported together.
    failures = []

    def check(fn, name, **kw):
        try:
            fn(game, golden_dir, name, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    game.drag(*BOX_FROM, *BOX_TO)
    time.sleep(1.0)
    check(checkpoint, "selected-action-bar", region=ACTION_BAR_REGION, settle=1.0)
    check(checkpoint, "selected-portraits", region=PORTRAITS_REGION, settle=1.0, max_diff=0.004)

    game.click(*DOCK_SPOT)
    # Park the pointer over the message window. Left on the dock it can end up
    # over a character, whose portrait then pulses and never settles.
    time.sleep(0.5)
    game.move(*PARK_SPOT)
    time.sleep(19.5)
    check(checkpoint, "walked-action-bar", region=ACTION_BAR_REGION, settle=1.0)
    check(checkpoint, "walked-portraits", region=PORTRAITS_REGION, settle=1.0, max_diff=0.004)
    check(checkpoint, "walked-messages", region=MESSAGE_REGION, settle=1.0)
    check(unsettled_checkpoint, "walked-dock", region=DOCK_REGION, max_diff=0.03)
    # Characters idle in different poses and spots each run: measured 2.0-3.4% over
    # 30 runs on the rebuild. A wrong weather state differs by 19% or more.
    check(unsettled_checkpoint, "walked-world", region=WORLD_REGION, max_diff=0.045)

    assert not failures, "; ".join(failures)
