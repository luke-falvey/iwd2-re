"""S7: Hedron's opening dialogue down the branches S3-S5 skip, then the journal.

Starts at Hedron's opening dialogue (S2). Replies: 2 ("Skeleton of a town?"),
Continue, 1 (anything about Targos?), 2 ("Mostly decent folk?"), Continue,
Continue after reply 1 of the next node (his mother), Continue, 2 (sign on with
the Targos guard), 1 (where to find Lord Ulbrec: the line "Your journal has
been updated." appears), 1 ("All right. Thanks for your help, Hedron."), his farewell, Continue and the
last reply 1 ("We'll be careful. Farewell, Hedron."), which ends the dialogue. Then press J, compare the journal, press Escape.

Not compared: the gold amount in the dialogue frame (random at party
creation) and the portrait to its left; only the text area is compared."""

from __future__ import annotations

import time

from checkpoints import checkpoint
from test_s2_new_game import new_game_to_opening_dialog
from test_s4_hedron_continue import CONTINUE
from test_s5_leave_dialog_inventory import ACTION_BAR_REGION

# The dialogue text area, without the gold amount and the portrait.
TEXT_REGION = (105, 322, 570, 235)
JOURNAL_REGION = (50, 55, 690, 315)


def test_s7_hedron_branches(game, golden_dir):
    new_game_to_opening_dialog(game, golden_dir)

    def reply(key, name, wait=3.0):
        game.press(key)
        time.sleep(wait)
        checkpoint(game, golden_dir, name, region=TEXT_REGION, settle=2.0)

    def cont(name):
        game.click(*CONTINUE)
        time.sleep(3.0)
        checkpoint(game, golden_dir, name, region=TEXT_REGION, settle=2.0)

    reply("2", "skeleton-of-a-town")
    cont("targos-lumber-wall")
    reply("1", "targos-not-the-town")
    reply("2", "mostly-decent-folk")
    cont("town-color-replies")
    reply("1", "mother-sharp-tongue")
    cont("umberlee-daughter-replies")
    reply("2", "targos-guard-ulbrec")
    reply("1", "journal-updated-thanks")
    reply("1", "hedron-farewell")
    cont("magdar-last-reply")
    game.press("1")
    time.sleep(4.0)
    checkpoint(game, golden_dir, "dialog-ended-action-bar", region=ACTION_BAR_REGION, settle=2.0)

    game.press("j")
    time.sleep(3.0)
    checkpoint(game, golden_dir, "journal", region=JOURNAL_REGION, settle=1.5)
    game.press("esc")
    time.sleep(2.0)
    checkpoint(game, golden_dir, "world-after-journal", region=ACTION_BAR_REGION, settle=1.0)
