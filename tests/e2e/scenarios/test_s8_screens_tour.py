"""S8: a tour of the in-game screens the other scenarios never show.

Continues S5 (Hedron's dialogue finished, party on the deck of the ship, game
running). Selects Sheris (portrait 1), then opens, compares and leaves each
screen, going back to the world screen with Esc in between:

  area map      dial button
  journal       J key
  record        R key, Sheris
  spellbook     dial button: Sheris (bard, empty at level 1), Laeris (cleric
                tab, then the Domain tab) and Vraedna (sorcerer), clicking
                their portraits; Sheris is selected again afterwards
  options       O key; Graphics, Sound, Character Sounds, Game Play, Feedback
                and Auto-Pause, each left with Cancel; Return to the world
  inventory     dial button (S5 compares it through the I key)

and compares the action bar and portraits back on the world screen. No
setting is changed and nothing is saved or loaded.

Only the upper 800x430 of each screen is compared. The portrait row below it
is left out: outside the world screen its frames change while the game runs
(seen on the area map), and the clock globe in the bottom right corner turns
on the area map. The message window is empty on these screens and holds the
parked pointer, so it is left out too. The game is not paused: apart from the
globe, every screen here is still.

Every key press and click must visibly change the screen, or the test stops.
On the original, a dropped input would otherwise be recorded as a new golden
variant (name.alt1.png) and let a rebuild that ignores the input pass. The
original once ignored R pressed on the journal screen, so the shortcuts are
only pressed from the world screen."""

from __future__ import annotations

import time

from checkpoints import checkpoint
from harness.screen import diff_ratio
from test_s2_new_game import new_game_to_opening_dialog
from test_s5_leave_dialog_inventory import (
    ACTION_BAR_REGION,
    INV_NAME_PORTRAIT,
    finish_hedron_dialog,
)

# Buttons in the dial at the bottom right of every screen (800x600).
DIAL = {
    "spellbook": (620, 527),  # S
    "inventory": (656, 525),  # I
    "journal": (689, 529),  # J
    "area_map": (619, 562),  # M
    "record": (656, 569),  # R
    "options": (690, 561),  # O
}
# Party portraits, left to right: Sheris (bard), Geldin, Khatja, Laeris
# (cleric), Vraedna (sorcerer), Kylie.
PORTRAIT = [(528 + 49 * i, 462) for i in range(6)]
SHERIS, LAERIS, VRAEDNA = PORTRAIT[0], PORTRAIT[3], PORTRAIT[4]

SPELLBOOK_DOMAIN_TAB = (430, 30)
OPTIONS = {
    "graphics": (612, 181),
    "sound": (612, 211),
    "game_play": (612, 241),
    "return": (612, 350),
}
SOUND_CHARACTER_SOUNDS = (240, 260)
GAME_PLAY_FEEDBACK = (128, 350)
GAME_PLAY_AUTO_PAUSE = (250, 350)
# Cancel on every options sub-screen.
SUB_CANCEL = (548, 350)

# Somewhere harmless to leave the pointer: the middle of the message window.
PARK_SPOT = (300, 560)

SCREEN_REGION = (0, 0, 800, 430)
PORTRAITS_REGION = (500, 435, 300, 55)
# Numbers, compared on their own with a tight limit: a wrong digit changes
# too few pixels to fail the whole-screen limit (a slot count of 1 instead of
# 0 is 0.015% of the screen).
RECORD_DETAILS = {
    "abilities": (300, 76, 85, 226),
    "armor-class": (458, 382, 44, 30),
    "hit-points": (596, 368, 40, 46),
}
SPELL_SLOTS = (448, 390, 32, 20)


def test_s8_screens_tour(game, golden_dir):
    new_game_to_opening_dialog(game, golden_dir)
    finish_hedron_dialog(game)

    # Every checkpoint is taken even if an earlier one differs, so that one
    # run on the rebuild shows all the differences. They are reported together.
    failures = []

    def check(name, region=SCREEN_REGION, **kw):
        try:
            checkpoint(game, golden_dir, name, region=region, **kw)
        except AssertionError as e:
            failures.append(str(e).splitlines()[0])

    def act(do, what, wait, world):
        """Do one input, then wait until the screen has changed (or fail: the
        input was dropped) and give it `wait` more seconds. Going to or from
        the world screen is seen in the action bar, which is blank on every
        other screen; the world view itself never stops moving (water)."""
        region = ACTION_BAR_REGION if world else SCREEN_REGION
        before = game.grab()
        do()
        game.move(*PARK_SPOT)
        deadline = time.time() + 10.0
        while diff_ratio(before, game.grab(), region=region) <= 0.005:
            if time.time() > deadline:
                raise AssertionError(f"{what} did not change the screen")
            time.sleep(0.2)
        time.sleep(wait)

    def click(xy, wait=2.0, world=False):
        act(lambda: game.click(*xy), f"click at {xy}", wait, world)

    def press(key, wait=2.0, world=False):
        act(lambda: game.press(key), f"key {key!r}", wait, world)

    # Sheris is normally selected already, so this click may change nothing.
    game.click(*SHERIS)
    game.move(*PARK_SPOT)
    time.sleep(1.0)

    click(DIAL["area_map"], world=True)
    check("area-map")
    press("esc", world=True)
    check("world-after-area-map", region=ACTION_BAR_REGION)

    press("j", world=True)
    check("journal")
    press("esc", world=True)

    press("r", world=True)
    check("record-sheris")
    for name, region in RECORD_DETAILS.items():
        check("record-sheris-" + name, region=region, settle=0.5, max_diff=0.004)
    press("esc", world=True)

    def spellbook(name):
        check(name)
        check(name + "-slots", region=SPELL_SLOTS, settle=0.5, max_diff=0.004)

    click(DIAL["spellbook"], world=True)
    spellbook("spellbook-sheris")
    click(LAERIS)
    spellbook("spellbook-laeris-cleric")
    click(SPELLBOOK_DOMAIN_TAB)
    spellbook("spellbook-laeris-domain")
    click(VRAEDNA)
    spellbook("spellbook-vraedna-sorcerer")
    click(SHERIS)
    check("spellbook-sheris-again")
    press("esc", world=True)

    press("o", world=True)
    check("options")
    click(OPTIONS["graphics"])
    check("options-graphics")
    click(SUB_CANCEL)
    click(OPTIONS["sound"])
    check("options-sound")
    click(SOUND_CHARACTER_SOUNDS)
    check("options-character-sounds")
    click(SUB_CANCEL)
    check("options-sound-again")
    click(SUB_CANCEL)
    click(OPTIONS["game_play"])
    check("options-game-play")
    click(GAME_PLAY_FEEDBACK)
    check("options-feedback")
    click(SUB_CANCEL)
    click(GAME_PLAY_AUTO_PAUSE)
    check("options-auto-pause")
    click(SUB_CANCEL)
    click(SUB_CANCEL)
    check("options-again")
    click(OPTIONS["return"], world=True)
    check("world-after-options", region=ACTION_BAR_REGION)

    click(DIAL["inventory"], wait=3.0, world=True)
    check("inventory-name-portrait", region=INV_NAME_PORTRAIT)
    press("esc", world=True)

    check("world-after-tour", region=ACTION_BAR_REGION)
    check("world-after-tour-portraits", region=PORTRAITS_REGION, max_diff=0.004)

    assert not failures, "; ".join(failures)
