"""S4: talk through Hedron's dialogue with the reply keys and press Continue.

Continues S2: press "1" four times (houses, Targos, a place to stay,
Salty Dog), compare the dialogue panel, then click Continue and compare
the bottom of the screen."""

from __future__ import annotations

import time

from checkpoints import checkpoint
from test_s2_new_game import DIALOG_REGION, new_game_to_opening_dialog

CONTINUE = (400, 583)
BOTTOM_REGION = (0, 430, 800, 170)


def test_s4_hedron_continue(game, golden_dir):
    new_game_to_opening_dialog(game, golden_dir)

    for _ in range(4):
        game.press("1")
        time.sleep(2.0)
    checkpoint(game, golden_dir, "hedron-salty-dog", region=DIALOG_REGION, settle=2.0)

    game.click(*CONTINUE)
    time.sleep(3.0)
    checkpoint(game, golden_dir, "hedron-after-continue", region=BOTTOM_REGION, settle=2.0)
