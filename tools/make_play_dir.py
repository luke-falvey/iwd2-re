#!/usr/bin/env python3
"""Build a disposable test copy of the game on macOS/Linux (the port's
counterpart of make_play_dir.ps1).

Copies the game data from a reference install, leaving out Windows
executables and DLLs, and sets icewind2.ini up for testing: windowed, 32 bpp,
cheats on, intro movies off, weather off, feedback levels 5. Paths in the ini
are POSIX paths; the game's file layer accepts either slash.

Usage: python3 -I tools/make_play_dir.py --ref <game folder> --play ~/iwd2-play-port
"""
import argparse
import os
import shutil
import sys

SKIP_EXT = {".exe", ".dll"}
SKIP_NAMES = {"__redist", "__support", "app", "commonappdata", "tmp", "webcache.zip", "manual.pdf"}


def set_ini(lines, section, key, value):
    out = []
    in_section = False
    done = False
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("[") and stripped.endswith("]"):
            if in_section and not done:
                out.append(f"{key}={value}")
                done = True
            in_section = stripped[1:-1] == section
        elif in_section and stripped.startswith(key + "="):
            out.append(f"{key}={value}")
            done = True
            continue
        out.append(line)
    if not done:
        if not in_section:
            out.append(f"[{section}]")
        out.append(f"{key}={value}")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", required=True)
    ap.add_argument("--play", required=True)
    ap.add_argument("--fullscreen", action="store_true")
    args = ap.parse_args()

    ref = os.path.realpath(os.path.expanduser(args.ref))
    play = os.path.realpath(os.path.expanduser(args.play))
    if not any(n.lower() == "chitin.key" for n in os.listdir(ref)):
        sys.exit(f"no CHITIN.KEY in {ref}")
    if play == ref or play.startswith(ref + os.sep):
        sys.exit("play dir must be outside the reference dir")

    if os.path.exists(play):
        shutil.rmtree(play)

    def ignore(folder, names):
        return [n for n in names if n in SKIP_NAMES or os.path.splitext(n)[1].lower() in SKIP_EXT]

    shutil.copytree(ref, play, ignore=ignore)
    for root, dirs, files in os.walk(play):
        for name in dirs + files:
            path = os.path.join(root, name)
            os.chmod(path, os.stat(path).st_mode | 0o200)

    ini = next(os.path.join(play, n) for n in os.listdir(play) if n.lower() == "icewind2.ini")
    with open(ini, "r", encoding="latin-1", newline="") as f:
        lines = f.read().replace("\r\n", "\n").split("\n")
    while lines and lines[-1] == "":
        lines.pop()

    def folder(name):
        for n in os.listdir(play):
            if n.lower() == name.lower():
                return os.path.join(play, n) + "/"
        return os.path.join(play, name) + "/"

    lines = set_ini(lines, "Alias", "HD0:", play + "/")
    lines = set_ini(lines, "Alias", "CD1:", folder("Data"))
    lines = set_ini(lines, "Alias", "CD2:", folder("CD2"))
    lines = set_ini(lines, "Program Options", "Full Screen", "1" if args.fullscreen else "0")
    lines = set_ini(lines, "Program Options", "BitsPerPixel", "32")
    lines = set_ini(lines, "Program Options", "3D Acceleration", "0")
    lines = set_ini(lines, "Program Options", "Screen Position X", "0")
    lines = set_ini(lines, "Program Options", "Screen Position Y", "0")
    lines = set_ini(lines, "Game Options", "Cheats", "1")
    lines = set_ini(lines, "Game Options", "GUI Feedback Level", "5")
    lines = set_ini(lines, "Game Options", "Locator Feedback Level", "5")
    lines = set_ini(lines, "Game Options", "Weather", "0")
    for movie in ("BISLOGO", "INFELOGO", "WOTC", "INTRO"):
        lines = set_ini(lines, "Movies", movie, "1")

    with open(ini, "w", encoding="latin-1", newline="") as f:
        f.write("\r\n".join(lines) + "\r\n")

    print(f"play dir ready: {play}")


if __name__ == "__main__":
    main()
