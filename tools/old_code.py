#!/usr/bin/env python3
"""List the functions that were already written when this project started and
whose body has not changed since, i.e. code that no translator or verifier of
this project has read against the exe, minus the addresses in
tools/audited_old.txt.

  py -I tools/old_code.py                 # totals per file
  py -I tools/old_code.py src/CInfGame.cpp  # the functions of one file
"""
import json
import random
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
BASE = "f04e0db98e771077de4333dd56f4c8413d7c297d"
ANNOTATION = re.compile(r"^// 0x([0-9A-Fa-f]{6,8})\s*$", re.M)
INCOMPLETE = re.compile(r"TODO: Incomplete")


def bodies(text):
    marks = list(ANNOTATION.finditer(text))
    out = {}
    for i, m in enumerate(marks):
        end = marks[i + 1].start() if i + 1 < len(marks) else len(text)
        out.setdefault(int(m.group(1), 16), text[m.end():end])
    return out


def norm(s):
    return re.sub(r"\s+", " ", s).strip()


funcs = {int(f["addr"], 16): f for f in json.loads(Path(r"C:\iwd2-tools\functions.json").read_text(encoding="utf-8"))}
files = subprocess.run(["git", "-C", str(REPO), "ls-tree", "-r", "--name-only", BASE, "src"],
                       capture_output=True, text=True).stdout.split()
unchanged = []
changed = 0
total_old = 0
for rel in files:
    if not rel.endswith(".cpp"):
        continue
    old = subprocess.run(["git", "-C", str(REPO), "show", f"{BASE}:{rel}"], capture_output=True).stdout.decode("utf-8", "replace")
    cur_path = REPO / rel
    if not cur_path.exists():
        continue
    cur = cur_path.read_text(encoding="utf-8", errors="replace")
    ob, cb = bodies(old), bodies(cur)
    for addr, body in ob.items():
        f = funcs.get(addr)
        if f is None or not (0x401000 <= addr < 0x7DA200) or INCOMPLETE.search(body):
            continue
        total_old += f.get("size", 0)
        if addr in cb and norm(cb[addr]) == norm(body):
            unchanged.append((addr, f.get("size", 0), rel))
        else:
            changed += f.get("size", 0)


audited = set()
for line in (REPO / "tools" / "audited_old.txt").read_text().splitlines():
    if line.startswith("0x"):
        audited.add(int(line, 16))
rows = [u for u in unchanged if u[0] not in audited]
if len(sys.argv) > 1:
    for a, s, rel in sorted(rows):
        if rel == sys.argv[1]:
            print("0x%X %5d %s" % (a, s, rel))
else:
    per = {}
    for a, s, rel in rows:
        per[rel] = per.get(rel, 0) + s
    for rel, s in sorted(per.items(), key=lambda kv: -kv[1]):
        print("%7d %s" % (s, rel))
    print("unread old code: %d functions, %d bytes" % (len(rows), sum(s for _, s, _ in rows)))
