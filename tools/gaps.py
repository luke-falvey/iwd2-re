"""Find code in the game range that no known function covers.

  py -I tools/gaps.py            # summary and the largest gaps
  py -I tools/gaps.py --all      # every gap

The inventory only counts functions Ghidra defined. A function Ghidra
missed (reached only through a vtable or a jump table, for example) is in
neither the worklist nor the progress figures. This script walks the game
range, subtracts every known function's extent, ignores alignment padding
(0xCC, 0x90 and zero bytes) and jump/switch tables that sit directly after
a function, and reports what is left, with whether the repo annotates an
address inside it."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from inventory import GAME_END, GAME_START, scan_source  # noqa: E402
from peek import load_pe, read  # noqa: E402

TOOLS = Path(r"C:\iwd2-tools")
REF = Path(r"C:\iwd2-ref")
PADDING = {0xCC, 0x90, 0x00}
INSTRUCTION = re.compile(r"^([0-9A-Fa-f]{8})\s")


# Multi-byte no-ops the compiler uses to align a jump table or a loop head.
ALIGN_NOPS = [
    bytes.fromhex("8da42400000000"),  # LEA ESP,[ESP+0x0]
    bytes.fromhex("8d9b00000000"),  # LEA EBX,[EBX+0x0]
    bytes.fromhex("8d642400"),  # LEA ESP,[ESP+0x0]
    bytes.fromhex("8d4900"),  # LEA ECX,[ECX+0x0]
    bytes.fromhex("8bff"),  # MOV EDI,EDI
    bytes.fromhex("8bc0"),  # MOV EAX,EAX
]


def strip_alignment(blob: bytes) -> bytes:
    changed = True
    while changed:
        changed = False
        while blob and blob[0] in PADDING:
            blob = blob[1:]
            changed = True
        for nop in ALIGN_NOPS:
            if blob.startswith(nop):
                blob = blob[len(nop):]
                changed = True
    return blob


def looks_like_table(blob: bytes) -> bool:
    """A run of dwords that all point into the game range (a jump table),
    optionally followed by a byte index table (values below the number of
    dwords) and padding, possibly repeated (several switches in a row)."""
    while True:
        blob = strip_alignment(blob)
        if not blob:
            return True
        n = 0
        while n + 4 <= len(blob):
            v = int.from_bytes(blob[n:n + 4], "little")
            if not (GAME_START <= v < GAME_END):
                break
            n += 4
        if n == 0:
            return False
        entries = n // 4
        blob = blob[n:]
        k = 0
        while k < len(blob) and blob[k] < entries:
            k += 1
        blob = blob[k:]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--all", action="store_true")
    args = ap.parse_args()

    funcs = json.loads((TOOLS / "functions.json").read_text(encoding="utf-8"))
    # A function's size in functions.json counts only its instructions, and a
    # body can have jump tables in the middle, so take each function's extent
    # from its exported listing: first instruction to the end of the last.
    spans = []
    for f in funcs:
        start = int(f["addr"], 16)
        end = start + max(f.get("size", 0), 1)
        listing = TOOLS / "decomp" / ("0x%X.asm" % start)
        if listing.exists():
            last = None
            for line in listing.read_text(errors="replace").splitlines():
                m = INSTRUCTION.match(line)
                if m:
                    last = int(m.group(1), 16)
            if last is not None:
                end = max(end, last + 1)
        spans.append((start, end))
    spans.sort()
    annotations, _, _ = scan_source()
    annotated = sorted(annotations)
    exe, sections = load_pe(REF / "iwd2.exe")

    gaps = []
    cursor = GAME_START
    for start, end in spans:
        if start >= GAME_END:
            break
        if start > cursor:
            gaps.append((cursor, start))
        cursor = max(cursor, end)
    if cursor < GAME_END:
        gaps.append((cursor, GAME_END))

    real = []
    for a, b in gaps:
        blob = read(exe, sections, a, b - a)
        body = blob.rstrip(bytes(PADDING))
        lead = len(body) - len(body.lstrip(bytes(PADDING)))
        body = body.lstrip(bytes(PADDING))
        if not body or looks_like_table(body):
            continue
        inside = [x for x in annotated if a <= x < b]
        real.append((a + lead, len(body), inside))

    total = sum(n for _, n, _ in real)
    with_ann = [g for g in real if g[2]]
    print(f"gaps holding bytes that are not padding or jump tables: {len(real)}, {total} bytes")
    print(f"  of which the repo annotates an address inside: {len(with_ann)}, {sum(n for _, n, _ in with_ann)} bytes")
    print(f"  with no repo annotation: {len(real) - len(with_ann)}, {total - sum(n for _, n, _ in with_ann)} bytes")
    shown = sorted(real, key=lambda g: -g[1])
    if not args.all:
        shown = shown[:40]
    for a, n, inside in shown:
        note = "annotated: " + ", ".join("0x%X" % x for x in inside[:4]) if inside else "NOT ANNOTATED"
        print(f"  0x{a:X}  {n:6d}  {note}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
