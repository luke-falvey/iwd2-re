"""S3: answer Hedron's opening dialogue.

Continues S2: pick reply 1 ("No, we're here to stay, Hedron.") and compare
the dialogue panel that follows."""

from __future__ import annotations

import time

from checkpoints import checkpoint
from test_s2_new_game import DIALOG_REGION, new_game_to_opening_dialog

REPLY_1 = (350, 398)


def test_s3_hedron_reply(game, golden_dir):
    new_game_to_opening_dialog(game, golden_dir)

    game.click(*REPLY_1)
    time.sleep(1.5)
    checkpoint(game, golden_dir, "hedron-after-reply-1", region=DIALOG_REGION, settle=2.0)
