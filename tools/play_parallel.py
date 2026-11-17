"""Run every play test against the rebuild, several at once.

Each worker has its own play folder (C:\\iwd2-play-1, -2, ...) and drives its
game through tools/hook (virtual input), so the runs don't
share the mouse, the keyboard or the foreground.

  python tools/play_parallel.py [--jobs 4] [--target reimpl] [test_s2 test_s9 ...]

Prints one line per scenario as it finishes and the tail of any that fails.
A scenario that fails is run once more alone at the end.
Exit code: number of failed scenarios.
"""

from __future__ import annotations

import argparse
import json
import os
import queue
import subprocess
import sys
import threading
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SCENARIOS = REPO / "tests" / "e2e" / "scenarios"
TIMES = REPO / "build" / "play_times.json"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--target", default="reimpl", choices=["original", "reimpl"])
    ap.add_argument("--play-root", default=r"C:\iwd2-play" if sys.platform == "win32"
                    else str(Path.home() / "iwd2-play-port"))
    ap.add_argument("--driver", action="store_true",
                    help="macOS port: drive each game through its test driver (port 47800+N for job N) "
                         "instead of tools/hook; one play folder per job (<play-root>-N)")
    ap.add_argument("--port-base", type=int, default=47800, help="with --driver: the first job's port")
    ap.add_argument("--no-compare", action="store_true", help="with --driver: skip golden comparisons")
    ap.add_argument("--ref", default=os.environ.get("IWD2_REF_DIR"),
                    help="off Windows: the reference game folder tools/make_play_dir.py copies from")
    ap.add_argument("--exe", default=None, help="override the exe to launch")
    ap.add_argument("--repeat", type=int, default=1, help="run each chosen scenario this many times")
    ap.add_argument("--no-retry", action="store_true", help="don't rerun failed scenarios alone")
    ap.add_argument("names", nargs="*", help="scenario file stems (default: all test_s*.py)")
    args = ap.parse_args()

    files = sorted(SCENARIOS.glob("test_s*.py"))
    if args.names:
        files = [f for f in files if any(f.stem == n or f.stem.startswith(n + "_") for n in args.names)]
    files = files * args.repeat
    try:
        times = json.loads(TIMES.read_text())
    except Exception:
        times = {}
    # Longest first, so the long ones don't end up queued behind each other.
    files.sort(key=lambda f: -times.get(f.stem, 600.0))

    play_dirs = []
    for i in range(1, args.jobs + 1):
        d = Path(f"{args.play_root}-{i}")
        if not (d / "CHITIN.KEY").exists() and sys.platform != "win32":
            subprocess.run([sys.executable, "-I", str(REPO / "tools" / "make_play_dir.py"), "--ref", args.ref,
                            "--play", str(d)], check=True)
        elif not (d / "CHITIN.KEY").exists():
            subprocess.run(["powershell", "-ExecutionPolicy", "Bypass", "-File",
                            str(REPO / "tools" / "make_play_dir.ps1"), "-Play", str(d)], check=True)
        # Tile the windows (three across): screenshots are read from the
        # screen, so no game window may cover another.
        ini = d / "icewind2.ini"
        x, y = ((i - 1) % 3) * 830, ((i - 1) // 3) * 660
        lines = ini.read_text().splitlines()
        lines = [f"Screen Position X={x}" if ln.startswith("Screen Position X=") else
                 f"Screen Position Y={y}" if ln.startswith("Screen Position Y=") else
                 # The tests look for selection circles and markers (see
                 # make_play_dir.ps1); the game rewrites these on exit.
                 "GUI Feedback Level=5" if ln.startswith("GUI Feedback Level=") else
                 "Locator Feedback Level=5" if ln.startswith("Locator Feedback Level=") else ln for ln in lines]
        ini.write_text("\n".join(lines) + "\n")
        play_dirs.append(d)

    work: queue.Queue = queue.Queue()
    for f in files:
        work.put(f)
    lock = threading.Lock()
    failed = []

    def worker(play_dir: Path, cpu: int) -> None:
        while True:
            try:
                f = work.get_nowait()
            except queue.Empty:
                return
            start = time.time()
            p = subprocess.run([sys.executable, "-m", "pytest", str(f), "--target", args.target]
                               + (["--driver", "--port", str(args.port_base + play_dirs.index(play_dir))]
                                  + (["--no-compare"] if args.no_compare else []) if args.driver else
                                  ["--virtual", "--cpu", str(cpu)])
                               + ["--play-dir", str(play_dir), "-q"]
                               + (["--exe", args.exe] if args.exe else []),
                               cwd=str(REPO), capture_output=True, text=True)
            lines = (p.stdout + p.stderr).splitlines()
            with lock:
                print(f"{f.stem}: {lines[-1] if lines else '(no output)'}", flush=True)
                if p.returncode != 0:
                    failed.append(f.stem)
                    print("\n".join(lines[-40:]), flush=True)
                else:
                    times[f.stem] = round(time.time() - start, 1)

    start = time.time()
    # The game pins itself to one CPU; give each instance its own, on
    # separate physical cores (even logical CPUs), leaving CPU 0 alone.
    ncpu = os.cpu_count() or 2
    threads = [threading.Thread(target=worker, args=(d, (2 * (i + 1)) % ncpu)) for i, d in enumerate(play_dirs)]
    for t in threads:
        t.start()
        time.sleep(3.0)  # stagger the launches
    for t in threads:
        t.join()

    # A scenario that fails beside others gets one more go alone: some steps
    # are slower with several games running, and a late click fails a test.
    first = sorted(failed)
    if first and args.jobs > 1 and not args.no_retry:
        print(f"retrying alone: {', '.join(first)}", flush=True)
        failed.clear()
        for stem in first:
            work.put(SCENARIOS / f"{stem}.py")
        worker(play_dirs[0], 2 % ncpu)

    TIMES.parent.mkdir(exist_ok=True)
    TIMES.write_text(json.dumps(times, indent=1, sort_keys=True))
    print(f"{len(files) - len(failed)} of {len(files)} scenarios passed in {time.time() - start:.0f}s "
          f"with {args.jobs} at once" + (f"; failed: {', '.join(sorted(failed))}" if failed else "")
          + (f"; passed only when rerun alone: {', '.join(x for x in first if x not in failed)}"
             if any(x not in failed for x in first) else ""), flush=True)
    return len(failed)


if __name__ == "__main__":
    sys.exit(main())
