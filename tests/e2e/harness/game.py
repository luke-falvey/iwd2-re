"""Launch and drive one game instance (original exe or the rebuild) the way a
player would: real mouse and keyboard through SendInput, screenshots of what
is on screen, and crash, assert and hang detection."""

from __future__ import annotations

import math
import os
import random
import subprocess
import time
from dataclasses import dataclass, field
from pathlib import Path

from . import win32
from .screen import diff_ratio

REPO = Path(__file__).resolve().parents[3]

VK = {
    "esc": 0x1B, "enter": 0x0D, "space": 0x20, "tab": 0x09, "backspace": 0x08,
    "left": 0x25, "up": 0x26, "right": 0x27, "down": 0x28,
    "ctrl": 0x11, "shift": 0x10, "alt": 0x12,
    **{f"f{i}": 0x6F + i for i in range(1, 13)},
}


def vk_for(k: str) -> int:
    k = k.lower()
    if k in VK:
        return VK[k]
    if len(k) == 1:
        return ord(k.upper())
    raise ValueError(f"unknown key {k!r}")


class GameCrashed(AssertionError):
    pass


@dataclass
class Game:
    target: str  # "original" or "reimpl"
    play_dir: Path
    out_dir: Path
    exe: Path | None = None
    seed: int = 1
    proc: subprocess.Popen | None = None
    hwnd: int = 0
    shots: int = 0
    rng: random.Random = field(default_factory=lambda: random.Random(1))
    # Virtual mode (tools/hook): input goes to this instance only through
    # iwd2hook.dll and frames are read from the window itself, so several
    # games can run at once. Real mode drives the real mouse and keyboard.
    virtual: bool = False
    pid: int = 0
    shared: object = None
    vpos: tuple = (400, 300)
    cpu: int | None = None  # virtual mode: the CPU this instance is pinned to

    # ---- lifecycle -------------------------------------------------------

    def launch(self, timeout: float = 60.0) -> None:
        exe = self.exe or self.default_exe()
        env = dict(os.environ)
        # Treat the old game as DPI aware so its window isn't bitmap-stretched
        # and window pixels are screen pixels.
        env["__COMPAT_LAYER"] = "HighDpiAware"
        env["IWD2_GAME_DIR"] = str(self.play_dir)
        env.setdefault("IWD2RE_CRASH_DIR", str(self.out_dir))
        self.out_dir.mkdir(parents=True, exist_ok=True)
        self.rng = random.Random(self.seed)
        if self.virtual and self.cpu is not None:
            env["IWD2_HOOK_CPU"] = str(self.cpu)
        if self.virtual:
            hook = REPO / "build" / "hook" / "Release"
            self.proc = subprocess.Popen([str(hook / "iwd2launch.exe"), str(hook / "iwd2hook.dll"), str(exe)],
                                         cwd=str(self.play_dir), env=env, stdout=subprocess.PIPE, text=True)
            line = self.proc.stdout.readline().split()
            if len(line) != 2 or line[0] != "pid":
                raise GameCrashed(f"{self.target}: launcher did not start the game")
            self.pid = int(line[1])
            self.shared = win32.open_hook_shared(self.pid)
        else:
            self.proc = subprocess.Popen([str(exe)], cwd=str(self.play_dir), env=env)
            self.pid = self.proc.pid
        # The game can briefly show a desktop-sized window during startup, so
        # wait until the main window's size has stayed the same for a moment.
        deadline = time.time() + timeout
        last, since = None, time.time()
        while time.time() < deadline:
            self.check_alive()
            hwnd = self.main_window()
            if hwnd and self.virtual:
                # The hook makes the window 800x600 from the start, so its
                # size says nothing. The game is up once its update loop has
                # been reading the cursor for a moment.
                if self.shared.reserved[0] <= 30:
                    since = time.time()
                elif time.time() - since >= 1.5:
                    self.hwnd = hwnd
                    self.shared.cursorX, self.shared.cursorY = self.vpos
                    self.shared.cursorValid = 1
                    self._skip_movies()
                    return
            elif hwnd:
                state = (hwnd, win32.client_rect_on_screen(hwnd)[2:])
                if state != last:
                    last, since = state, time.time()
                elif time.time() - since >= 1.5 and state[1][0] >= 640:
                    self.hwnd = hwnd
                    win32.bring_to_front(hwnd)
                    return
            time.sleep(0.25)
        raise TimeoutError(f"{self.target}: no game window after {timeout}s")

    def _skip_movies(self) -> None:
        """The game plays its logo movies at every start; a click skips one.
        (In real mode the ALT tap that takes the foreground does this.) Click
        in the corner, where the main menu has nothing, until the picture
        has stopped changing."""
        self.shared.cursorX, self.shared.cursorY = 3, 3
        prev, still = self.grab(), 0
        deadline = time.time() + 40.0
        while still < 4 and time.time() < deadline:
            self._key(0x01, True)
            time.sleep(0.08)
            self._key(0x01, False)
            time.sleep(0.4)
            cur = self.grab()
            still = still + 1 if diff_ratio(prev, cur) <= 0.002 else 0
            prev = cur
        self.shared.cursorX, self.shared.cursorY = self.vpos

    def default_exe(self) -> Path:
        if self.target == "original":
            return self.play_dir / "iwd2.exe"
        for cfg in ("RelWithDebInfo", "Debug", "Release"):
            p = REPO / "build" / cfg / "iwd2-re.exe"
            if p.exists():
                return p
        raise FileNotFoundError("iwd2-re.exe not built")

    def main_window(self) -> int:
        if not self.proc:
            return 0
        best, best_area = 0, 0
        for h in win32.windows_of_pid(self.pid):
            cls = win32.window_class(h)
            if cls == "ChitinClass":  # the engine's main window (original and rebuild)
                return h
            if cls == "#32770":
                continue
            _, _, w, hh = win32.client_rect_on_screen(h)
            if w * hh > best_area:
                best, best_area = h, w * hh
        return best

    def dump_stacks(self) -> Path | None:
        """Ask the rebuild to write the stacks of all its threads (it listens
        on "iwd2re-dump-<pid>"). Returns the stacks file, if written."""
        if self.target != "reimpl" or not self.proc or self.proc.poll() is not None:
            return None
        path = Path(os.environ.get("IWD2RE_CRASH_DIR", str(self.out_dir))) / "iwd2re-stacks.txt"
        if path.exists():
            path.unlink()
        if not win32.signal_event(f"iwd2re-dump-{self.pid}"):
            return None
        for _ in range(240):
            time.sleep(0.25)
            if path.exists() and b"== end ==" in path.read_bytes():
                return path
        return None

    def close(self) -> None:
        if self.proc and self.proc.poll() is None:
            self.proc.kill()
            self.proc.wait(10)

    def __enter__(self) -> "Game":
        self.launch()
        return self

    def __exit__(self, *exc) -> None:
        if exc[0] is not None:
            try:
                self.screenshot("failure")
            except Exception:
                pass
        self.close()

    # ---- health ----------------------------------------------------------

    def check_alive(self) -> None:
        """Fail on process exit or an assert or error dialog."""
        if self.proc is None:
            raise GameCrashed("not launched")
        rc = self.proc.poll()
        if rc is not None:
            raise GameCrashed(f"{self.target} exited with code {rc:#x}")
        for h in win32.windows_of_pid(self.pid):
            if win32.window_class(h) == "#32770":  # message box: asserts and fatal errors
                text = win32.dialog_text(h)
                self.screenshot_desktop("dialog")
                raise GameCrashed(f"{self.target} showed a dialog: {text!r}")

    # ---- seeing ----------------------------------------------------------

    def client_rect(self) -> tuple[int, int, int, int]:
        return win32.client_rect_on_screen(self.hwnd)

    def grab(self):
        self.check_alive()
        # Virtual mode also reads the screen: capturing from the window
        # (PrintWindow) returns frames a second or two old when several games
        # run. The window is topmost, so it only needs a spot of its own
        # (tools/play_parallel.py tiles them).
        x, y, w, h = self.client_rect()
        return win32.capture_screen_rect(x, y, w, h)

    def screenshot(self, name: str):
        img = self.grab()
        self.shots += 1
        img.save(self.out_dir / f"{self.shots:03d}-{name}.png")
        return img

    def screenshot_desktop(self, name: str) -> None:
        img = win32.capture_screen_rect(0, 0, 1920, 1080)
        img.save(self.out_dir / f"desktop-{name}.png")

    def wait_stable(self, settle: float = 1.0, timeout: float = 30.0, threshold: float = 0.002,
                    region: tuple[int, int, int, int] | None = None):
        """Wait until the picture stops changing for `settle` seconds
        (ignoring cursor-sized changes), then return the frame."""
        deadline = time.time() + timeout
        prev = self.grab()
        stable_since = time.time()
        while time.time() < deadline:
            time.sleep(0.2)
            cur = self.grab()
            if diff_ratio(prev, cur, region=region) > threshold:
                stable_since = time.time()
            elif time.time() - stable_since >= settle:
                return cur
            prev = cur
        raise TimeoutError("screen never settled")

    def wait_for_change(self, before, timeout: float = 15.0, threshold: float = 0.02):
        deadline = time.time() + timeout
        while time.time() < deadline:
            time.sleep(0.2)
            cur = self.grab()
            if diff_ratio(before, cur) > threshold:
                return cur
        raise TimeoutError("screen did not change")

    # ---- acting like a player ------------------------------------------

    def _to_screen(self, x: int, y: int) -> tuple[int, int]:
        cx, cy, _, _ = self.client_rect()
        return cx + x, cy + y

    def _ensure_focus(self) -> None:
        if not self.virtual and not win32.is_foreground(self.hwnd):
            # Something else took the foreground: note what, since input sent
            # around this moment may not have reached the game.
            try:
                with open(Path(self.out_dir) / "focus-lost.txt", "a", encoding="utf-8") as f:
                    print(f"{time.strftime('%H:%M:%S')} foreground was {win32.foreground_title()}", file=f)
            except OSError:
                pass
            win32.bring_to_front(self.hwnd)

    # Input primitives: the real devices, or this instance's virtual ones.
    # Positions are screen pixels in real mode and client pixels in virtual.

    def _cursor(self) -> tuple[int, int]:
        return self.vpos if self.virtual else win32.cursor_pos()

    def _move_to(self, x: int, y: int) -> None:
        if self.virtual:
            self.vpos = (x, y)
            self.shared.cursorX, self.shared.cursorY = x, y
        else:
            win32.mouse_move_abs(x, y)

    def _key(self, vk: int, down: bool) -> None:
        if self.virtual:
            if down:
                self.shared.pressed[vk] = 1
            self.shared.down[vk] = 1 if down else 0
        else:
            win32.key(vk, down)

    def _button(self, button: str, down: bool) -> None:
        if self.virtual:
            self._key({"left": 0x01, "right": 0x02}[button], down)
        else:
            win32.mouse_button(button, down)

    def move(self, x: int, y: int) -> None:
        """Move the cursor to client (x, y) along a slightly curved,
        ease-in-out path the way a hand does. Hover effects see every step."""
        self._ensure_focus()
        tx, ty = (x, y) if self.virtual else self._to_screen(x, y)
        sx, sy = self._cursor()
        dist = math.hypot(tx - sx, ty - sy)
        steps = max(4, min(40, int(dist / 18)))
        # Control point off the straight line gives a gentle arc.
        mx, my = (sx + tx) / 2, (sy + ty) / 2
        off = self.rng.uniform(-0.15, 0.15) * dist
        nx, ny = -(ty - sy) / (dist or 1), (tx - sx) / (dist or 1)
        cx, cy = mx + nx * off, my + ny * off
        for i in range(1, steps + 1):
            t = i / steps
            t = t * t * (3 - 2 * t)  # ease in-out
            px = (1 - t) ** 2 * sx + 2 * (1 - t) * t * cx + t * t * tx
            py = (1 - t) ** 2 * sy + 2 * (1 - t) * t * cy + t * t * ty
            self._move_to(int(round(px)), int(round(py)))
            time.sleep(self.rng.uniform(0.006, 0.014))
        self._move_to(tx, ty)
        time.sleep(self.rng.uniform(0.08, 0.2))  # hover dwell

    def click(self, x: int, y: int, button: str = "left") -> None:
        self.move(x, y)
        self._button(button, True)
        time.sleep(self.rng.uniform(0.05, 0.12))
        self._button(button, False)
        time.sleep(self.rng.uniform(0.15, 0.3))
        self.check_alive()

    def double_click(self, x: int, y: int) -> None:
        self.move(x, y)
        for _ in range(2):
            self._button("left", True)
            time.sleep(0.04)
            self._button("left", False)
            time.sleep(0.06)
        self.check_alive()

    def drag(self, x0: int, y0: int, x1: int, y1: int) -> None:
        self.move(x0, y0)
        self._button("left", True)
        time.sleep(0.15)
        self.move(x1, y1)
        time.sleep(0.15)
        self._button("left", False)
        time.sleep(0.2)
        self.check_alive()

    def press(self, k: str, hold: float | None = None) -> None:
        self._ensure_focus()
        vk = vk_for(k)
        self._key(vk, True)
        time.sleep(hold if hold is not None else self.rng.uniform(0.05, 0.11))
        self._key(vk, False)
        time.sleep(self.rng.uniform(0.1, 0.25))
        self.check_alive()

    def type_text(self, text: str) -> None:
        for ch in text:
            if ch.isupper():
                self._key(VK["shift"], True)
                self.press(ch)
                self._key(VK["shift"], False)
            else:
                self.press(ch)
