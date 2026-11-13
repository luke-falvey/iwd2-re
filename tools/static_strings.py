#!/usr/bin/env python3
"""Compare the exe's static CString constants with the repo's.

The exe constructs each static `const CString X("...")` in a small
initialiser that belongs to no function (see tools/ghidra/OrphanCode.java):
  PUSH <string>; MOV ECX,<object>; CALL 0x7FCC88; PUSH <dtor thunk>; CALL atexit
The repo marks a static with `// 0xOBJECT` above its definition. This prints
every object whose repo literal differs from the exe's string, and counts the
objects the repo does not annotate.

  py -I tools/static_strings.py [-v]     # -v also lists unannotated objects
"""
import json
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import peek  # noqa: E402

TOOLS = Path(os.environ.get("IWD2_TOOLS_DIR", r"C:\iwd2-tools"))
REF = Path(os.environ.get("IWD2_REF_DIR", r"C:\iwd2-ref"))
SRC = Path(__file__).resolve().parent.parent / "src"
CTOR = "CALL 0x007fcc88"

d, secs = peek.load_pe(REF / "iwd2.exe")


def cstr(addr):
    raw = peek.read(d, secs, addr, 512)
    return raw.split(b"\0")[0].decode("latin-1")


exe = {}
for o in json.loads((TOOLS / "orphan_code.json").read_text()):
    if o["kind"] != "code":
        continue
    p = TOOLS / "orphans" / (o["addr"] + ".asm")
    L = [l[10:].strip() for l in p.read_text().splitlines() if not l.startswith(";")]
    for i in range(len(L) - 2):
        if L[i].startswith("PUSH 0x") and L[i + 1].startswith("MOV ECX,0x") and L[i + 2] == CTOR:
            exe[int(L[i + 1][8:], 16)] = cstr(int(L[i][5:], 16))

ANN = re.compile(r"^// 0x([0-9A-Fa-f]{6,8})[ \t]*\r?\n([^\n]*)", re.M)
LIT = re.compile(r'"((?:[^"\\]|\\.)*)"')
repo = {}
for f in sorted(SRC.rglob("*.cpp")):
    for m in ANN.finditer(f.read_text(encoding="utf-8", errors="replace")):
        repo.setdefault(int(m.group(1), 16), (f.name, m.group(2).strip()))

bad = missing = same = 0
for addr in sorted(exe):
    if addr not in repo:
        missing += 1
        if "-v" in sys.argv:
            print("unannotated 0x%X %r" % (addr, exe[addr]))
        continue
    name, line = repo[addr]
    lits = LIT.findall(line)
    val = "".join(lits).encode().decode("unicode_escape") if lits else ""  # default-constructed
    if val == exe[addr]:
        same += 1
    else:
        bad += 1
        print("DIFF 0x%X %s: exe %r, repo: %s" % (addr, name, exe[addr], line))
print("static CStrings in exe: %d; same: %d; different: %d; not annotated in repo: %d" % (len(exe), same, bad, missing))
