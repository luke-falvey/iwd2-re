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
import sys
import time
from pathlib import Path

import pytest

from harness.game import Game

E2E = Path(__file__).resolve().parent


def _default_play_dir() -> str:
    # Off Windows (the macOS port) the play folder is ~/iwd2-play-port.
    if sys.platform == "win32":
        return os.environ.get("IWD2_PLAY_DIR", r"C:\iwd2-play")
    return os.environ.get("IWD2_PLAY_DIR", str(Path.home() / "iwd2-play-port"))


def pytest_addoption(parser):
    parser.addoption("--target", default="original", choices=["original", "reimpl"])
    parser.addoption("--play-dir", default=_default_play_dir())
    parser.addoption("--exe", default=None, help="override the exe to launch")
    parser.addoption("--virtual", action="store_true", default=os.environ.get("IWD2_VIRTUAL") == "1",
                     help="drive the game through tools/hook (virtual input, window capture) so several "
                          "instances can run at once, each with its own --play-dir")
    parser.addoption("--driver", action="store_true", default=os.environ.get("IWD2_DRIVER") == "1",
                     help="drive the macOS port build through its test driver socket (no real input, "
                          "no screen capture); needs --target reimpl")
    parser.addoption("--port", type=int, default=int(os.environ.get("IWD2_DRIVER_PORT", "47800")),
                     help="with --driver: the test driver's TCP port")
    parser.addoption("--no-compare", action="store_true", default=False,
                     help="with --driver: do every step and save every checkpoint, but skip the "
                          "comparison with the golden images (reported at the end)")
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
    if request.config.getoption("--driver"):
        g.use_driver(request.config.getoption("--port"), request.config.getoption("--no-compare"))
    elif request.config.getoption("--no-compare"):
        pytest.fail("--no-compare needs --driver")
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


_SKIPPED: list = []


@pytest.fixture(autouse=True)
def _note_skipped_compares(request):
    yield
    g = request.node.funcargs.get("game") if hasattr(request.node, "funcargs") else None
    if g is not None and g.compare_skipped:
        _SKIPPED.append((request.node.name, list(g.compare_skipped)))


def pytest_terminal_summary(terminalreporter):
    if _SKIPPED:
        n = sum(len(c) for _, c in _SKIPPED)
        terminalreporter.write_sep("-", f"--no-compare: {n} comparison(s) skipped (no golden), "
                                        "pictures are in each test's out dir")
        for name, checks in _SKIPPED:
            terminalreporter.write_line(f"{name}: {', '.join(checks)}")
