"""pytest setup for T5 scripted play.

Run from the repo root with the harness venv:
  C:\\iwd2-tools\\venv\\Scripts\\python -m pytest tests/e2e --target original
  C:\\iwd2-tools\\venv\\Scripts\\python -m pytest tests/e2e --target reimpl

These tests take over the real mouse and keyboard. Don't touch them while a
run is going. Run the original first: its screenshots become the reference
for the reimpl run (tests/e2e/golden/, gitignored).
"""

from __future__ import annotations

import os
import shutil
import time
from pathlib import Path

import pytest

from harness.game import Game

E2E = Path(__file__).resolve().parent


def pytest_addoption(parser):
    parser.addoption("--target", default="original", choices=["original", "reimpl"])
    parser.addoption("--play-dir", default=os.environ.get("IWD2_PLAY_DIR", r"C:\iwd2-play"))
    parser.addoption("--exe", default=None, help="override the exe to launch")
    parser.addoption("--virtual", action="store_true", default=os.environ.get("IWD2_VIRTUAL") == "1",
                     help="drive the game through tools/hook (virtual input, window capture) so several "
                          "instances can run at once, each with its own --play-dir")
    parser.addoption("--cpu", type=int, default=None, help="with --virtual: the CPU to pin the game to")


@pytest.fixture(scope="session")
def target(request) -> str:
    return request.config.getoption("--target")


@pytest.fixture(scope="session")
def play_dir(request) -> Path:
    p = Path(request.config.getoption("--play-dir"))
    if not (p / "CHITIN.KEY").exists():
        pytest.skip(f"no game in {p}; run tools/make_play_dir.ps1")
    return p


@pytest.fixture
def out_dir(request, target) -> Path:
    stamp = time.strftime("%Y%m%d-%H%M%S")
    d = E2E / "out" / f"{stamp}-{target}-{request.node.name}"
    d.mkdir(parents=True, exist_ok=True)
    return d


@pytest.fixture
def golden_dir(request) -> Path:
    d = E2E / "golden" / request.node.name
    d.mkdir(parents=True, exist_ok=True)
    return d


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    outcome = yield
    rep = outcome.get_result()
    setattr(item, "rep_" + rep.when, rep)


@pytest.fixture(autouse=True)
def clean_saves(play_dir):
    """Start every test from the same save list: travel between areas leaves
    an "Auto-Save" slot, and a failed run can leave its own saves, either of
    which shifts the slots other tests look at. Autouse, so it runs before a
    test's own save fixtures."""
    keep = {"default", "000000000-Autosave - Prologue"}
    mpsave = play_dir / "mpsave"
    if mpsave.is_dir():
        for p in mpsave.iterdir():
            if p.is_dir() and p.name not in keep:
                shutil.rmtree(p, ignore_errors=True)
    yield


@pytest.fixture
def game(request, target, play_dir, out_dir):
    exe = request.config.getoption("--exe")
    g = Game(target=target, play_dir=play_dir, out_dir=out_dir, exe=Path(exe) if exe else None,
             virtual=request.config.getoption("--virtual"), cpu=request.config.getoption("--cpu"))
    try:
        g.launch()
        yield g
        rep = getattr(request.node, "rep_call", None)
        if rep is not None and rep.failed:
            g.dump_stacks()
            try:
                g.screenshot("failure")
            except Exception:
                g.screenshot_desktop("failure")
    finally:
        g.close()
