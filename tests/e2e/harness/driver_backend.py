"""Driver mode: run the macOS port build with its built-in test driver
(IWD2RE_TEST_DRIVER=<port>, protocol at the top of
src/platform/posix/test_driver.cpp) and do through the socket everything the
real and virtual modes do through harness/win32.py.

Game (harness/game.py) delegates to a DriverBackend when one is set; the real
and virtual paths are untouched. Mouse positions are client pixels of the
800x600 picture, keys are Windows virtual-key codes, and `grab` is the last
frame the game presented. There is no focus, no foreground and no real-input
lock here: the driver ignores the real mouse and keyboard.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO / "tools"))
from driver import Driver  # noqa: E402  (tools/driver.py, shared with the command-line client)

BUTTON = {"left": 1, "right": 2, "middle": 4}
MAC_EXE = REPO / "build-mac" / "iwd2-re"
STACKS = "iwd2re-stacks.txt"
LOG = "iwd2re-run.log"


class DriverBackend:
    def __init__(self, port: int = 47800, no_compare: bool = False):
        self.port = port
        self.no_compare = no_compare
        self.drv: Driver | None = None
        self.proc: subprocess.Popen | None = None
        self.size = (800, 600)
        self.pos = (400, 300)
        self.last_frame = None
        self.log_file = None
        self.play_dir: Path | None = None
        self.out_dir: Path | None = None

    # ---- lifecycle -------------------------------------------------------

    def launch(self, game, timeout: float = 60.0) -> None:
        from .game import GameCrashed
        if game.target != "reimpl":
            raise RuntimeError("--driver drives the port build: use --target reimpl")
        exe = game.exe or MAC_EXE
        self.play_dir, self.out_dir = Path(game.play_dir), Path(game.out_dir)
        env = dict(os.environ)
        env["IWD2RE_TEST_DRIVER"] = str(self.port)
        env.setdefault("IWD2RE_SOUND", "off")
        env.setdefault("IWD2RE_HIDE_WINDOW", "1")
        env["IWD2_GAME_DIR"] = str(self.play_dir)
        self.out_dir.mkdir(parents=True, exist_ok=True)
        import random
        game.rng = random.Random(game.seed)
        game._disable_weather()
        (self.play_dir / STACKS).unlink(missing_ok=True)
        self.log_file = open(self.out_dir / LOG, "wb")
        self.proc = subprocess.Popen([str(exe)], cwd=str(self.play_dir), env=env,
                                     stdout=self.log_file, stderr=subprocess.STDOUT)
        game.proc, game.pid = self.proc, self.proc.pid
        deadline = time.time() + timeout
        # The driver starts listening shortly after the game starts.
        while self.drv is None:
            self.check_alive()
            try:
                self.drv = Driver(self.port)
            except OSError:
                if time.time() > deadline:
                    raise TimeoutError(f"no test driver on port {self.port} after {timeout}s")
                time.sleep(0.2)
        # Up once frames are being presented at a size that has stayed the same.
        last, since = None, time.time()
        while time.time() < deadline:
            self.check_alive()
            count, w, h = self._call(self.drv.status)
            state = (w, h) if count else None
            if state != last:
                last, since = state, time.time()
            elif state and time.time() - since >= 1.5 and w >= 640:
                self.size = (w, h)
                return
            time.sleep(0.25)
        raise TimeoutError(f"no frames from the game after {timeout}s")

    def close(self) -> None:
        try:
            if self.proc and self.proc.poll() is None:
                self.proc.kill()
                self.proc.wait(10)
        finally:
            if self.drv is not None:
                try:
                    self.drv.sock.close()
                except OSError:
                    pass
                self.drv = None
            if self.log_file is not None:
                self.log_file.close()
                self.log_file = None

    # ---- health ----------------------------------------------------------

    def _crashed(self, what: str):
        from .game import GameCrashed
        return GameCrashed(what)

    def check_alive(self) -> None:
        if self.proc is None:
            raise self._crashed("not launched")
        rc = self.proc.poll()
        if rc is not None:
            raise self._crashed(f"reimpl exited with code {rc}" + (f" (signal {-rc})" if rc < 0 else ""))

    def _call(self, fn, *args):
        """A driver call; a dead or silent game becomes GameCrashed."""
        try:
            return fn(*args)
        except (ConnectionError, TimeoutError, OSError) as e:
            self.check_alive()
            raise self._crashed(f"the game's test driver stopped answering: {e!r}")

    # ---- seeing ----------------------------------------------------------

    def client_rect(self):
        return (0, 0) + self.size

    def grab(self):
        from PIL import Image
        self.check_alive()
        w, h, rgb, _ = self._call(self.drv.grab)
        if w == 0:
            raise self._crashed("the game has presented no frame")
        self.size = (w, h)
        img = Image.frombytes("RGB", (w, h), rgb)
        self.last_frame = img
        return img

    def screenshot_desktop(self, name: str) -> None:
        """There is no desktop to capture: save the last frame seen."""
        if self.last_frame is not None:
            self.last_frame.save(self.out_dir / f"desktop-{name}.png")

    # ---- acting ----------------------------------------------------------

    def move_to(self, x: int, y: int) -> None:
        self.pos = (x, y)
        self._call(self.drv.move, x, y)

    def key(self, vk: int, down: bool) -> None:
        self._call(self.drv.cmd, f"{'keydown' if down else 'keyup'} {vk}")

    def button(self, button: str, down: bool) -> None:
        self._call(self.drv.cmd, f"{'down' if down else 'up'} {BUTTON[button]}")

    # ---- failure evidence ------------------------------------------------

    def collect(self) -> Path | None:
        """On a failure or hang: leave the game's log (already in the out dir),
        the crash stacks if the game wrote any, and a last frame there."""
        stacks = None
        src = self.play_dir / STACKS if self.play_dir else None
        if src is not None and src.exists():
            stacks = self.out_dir / STACKS
            shutil.copyfile(src, stacks)
        if self.log_file is not None:
            self.log_file.flush()
        if self.proc is not None and self.proc.poll() is None and self.drv is not None:
            try:
                self.grab().save(self.out_dir / "final-frame.png")
            except Exception:
                if self.last_frame is not None:
                    self.last_frame.save(self.out_dir / "final-frame.png")
        elif self.last_frame is not None:
            self.last_frame.save(self.out_dir / "final-frame.png")
        return stacks
