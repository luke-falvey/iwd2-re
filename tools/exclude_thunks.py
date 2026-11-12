"""Find unannotated functions that are compiler-generated thunks, by exact
instruction pattern, so no model has to read them.

  py -I tools/exclude_thunks.py            # print what would be excluded
  py -I tools/exclude_thunks.py --write    # append them to tools/inventory_exclusions.txt

Four patterns, all exact (the two below, plus guarded static initialisers and
static initialisers that register a destructor with atexit):

  scalar deleting destructor   PUSH ESI / MOV ESI,ECX / CALL dtor /
                               TEST byte ptr [ESP + 0x8],0x1 / JZ / PUSH ESI /
                               CALL operator delete / ADD ESP,0x4 / MOV EAX,ESI /
                               POP ESI / RET 0x4
  static-object thunk          MOV ECX,imm / JMP ctor-or-dtor

Anything else is left for a translator to classify."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import worklist  # noqa: E402

DECOMP = Path(r"C:\iwd2-tools\decomp")
EXCLUSIONS = Path(__file__).resolve().parent / "inventory_exclusions.txt"
OPERATOR_DELETE = "0x007fc984"
LINE = re.compile(r"^[0-9A-Fa-f]{8}\s+(.*)$")

SCALAR_DELETING = [
    r"PUSH ESI",
    r"MOV ESI,ECX",
    r"CALL 0x[0-9a-f]{8}",
    r"TEST byte ptr \[ESP \+ 0x8\],0x1",
    r"JZ 0x[0-9a-f]{8}",
    r"PUSH ESI",
    r"CALL " + OPERATOR_DELETE,
    r"ADD ESP,0x4",
    r"MOV EAX,ESI",
    r"POP ESI",
    r"RET 0x4",
]
STATIC_THUNK = [r"MOV ECX,0x[0-9a-f]+", r"JMP 0x[0-9a-f]{8}"]
# Initialiser of a static object defined in a header: test and set a bit in
# a guard byte, then jump to the constructor.
GUARDED_STATIC_INIT = [
    r"MOV CL,byte ptr \[0x[0-9a-f]{8}\]",
    r"MOV AL,0x[0-9a-f]+",
    r"TEST AL,CL",
    r"JNZ 0x[0-9a-f]{8}",
    r"OR CL,AL",
    r"MOV byte ptr \[0x[0-9a-f]{8}\],CL",
    r"MOV ECX,0x[0-9a-f]+",
    r"JMP 0x[0-9a-f]{8}",
    r"RET",
]
# Dynamic initialiser of a global object that takes one constructor argument
# and registers its destructor with atexit.
STATIC_INIT_ATEXIT = [
    r"PUSH 0x[0-9a-f]+",
    r"MOV ECX,0x[0-9a-f]+",
    r"CALL 0x[0-9a-f]{8}",
    r"PUSH 0x[0-9a-f]+",
    r"CALL 0x[0-9a-f]{8}",
    r"POP ECX",
    r"RET",
]


def instructions(addr: int) -> list[str] | None:
    path = DECOMP / ("0x%x.asm" % addr)
    if not path.exists():
        return None
    out = []
    for line in path.read_text(errors="replace").splitlines():
        m = LINE.match(line.strip())
        if m:
            out.append(m.group(1).strip())
    return out


def matches(ops: list[str], pattern: list[str]) -> bool:
    return len(ops) == len(pattern) and all(re.fullmatch(p, o) for p, o in zip(pattern, ops))


def classify() -> dict[str, list[int]]:
    found: dict[str, list[int]] = {
        "scalar deleting destructor": [],
        "static-object thunk": [],
        "guarded static initialiser": [],
        "static initialiser with atexit": [],
    }
    for batch in worklist.build()["batches"]:
        for item in batch["functions"]:
            if item["kind"] != "missing":
                continue
            addr = int(item["addr"], 16)
            ops = instructions(addr)
            if ops is None:
                continue
            if matches(ops, SCALAR_DELETING):
                found["scalar deleting destructor"].append(addr)
            elif matches(ops, STATIC_THUNK):
                found["static-object thunk"].append(addr)
            elif matches(ops, GUARDED_STATIC_INIT):
                found["guarded static initialiser"].append(addr)
            elif matches(ops, STATIC_INIT_ATEXIT):
                found["static initialiser with atexit"].append(addr)
    return found


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--write", action="store_true")
    args = ap.parse_args()

    found = classify()
    for kind, addrs in found.items():
        print("%5d  %s" % (len(addrs), kind))
    if args.write and any(found.values()):
        with EXCLUSIONS.open("a", newline="\n") as f:
            for kind, addrs in found.items():
                if not addrs:
                    continue
                f.write("# Compiler-generated %ss, matched by exact instruction pattern\n" % kind)
                f.write("# (tools/exclude_thunks.py).\n")
                for a in sorted(addrs):
                    f.write("0x%X\n" % a)
        print("appended to", EXCLUSIONS)
    return 0


if __name__ == "__main__":
    sys.exit(main())
