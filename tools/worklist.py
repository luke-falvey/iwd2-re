#!/usr/bin/env python3
"""Assign every unfinished function to exactly one batch, grouped by source file.

  py -I tools/worklist.py            # summary
  py -I tools/worklist.py --write    # also write docs/worklist.json

An unfinished function is either a stub (annotated in the repo, body still has
`TODO: Incomplete`) or missing (not annotated at all). Stubs belong to the file
that holds them. Missing functions go to the file that holds most of their
translated callers, so the agent that needs a helper is the one that writes it.

Batches that share a file must not run in parallel. Each entry lists its
stubbed and missing callees, and tags that decide who should translate it:

  large    over 3 KB: translate twice and verify the differences
  switch   has a jump table: split into case ranges
  dup      annotated in more than one .cpp: remove the duplicate first
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from inventory import REPO, is_countable, load_exclusions, load_functions, scan_source, tools_dir  # noqa: E402

OUT = REPO / "docs" / "worklist.json"
BATCH_BYTES = 12 * 1024
LARGE = 3 * 1024
JUMP_TABLE = re.compile(r"JMP dword ptr \[[A-Z]{3}\*0x4 \+ 0x")


def to_int(a) -> int:
    return int(a, 16) if isinstance(a, str) else a


def has_jump_table(addr: int) -> bool:
    p = tools_dir() / "decomp" / ("0x%X.asm" % addr)
    return p.exists() and bool(JUMP_TABLE.search(p.read_text(errors="replace")))


def build() -> dict:
    funcs, _ = load_functions(None)
    annotations, _, stubbed = scan_source()
    exclusions = load_exclusions()
    by_addr = {f["addr"]: f for f in funcs}
    countable = {f["addr"] for f in funcs if is_countable(f, exclusions)}

    cpp_files = {a: sorted({f for f in files if f.endswith(".cpp")}) for a, files in annotations.items()}
    missing = {a for a in countable if a not in annotations}
    unfinished = (stubbed & countable) | missing

    def home(addr: int) -> str:
        files = cpp_files.get(addr)
        if files:
            return files[0]
        votes = Counter()
        for c in by_addr[addr].get("callers", []):
            for f in cpp_files.get(to_int(c), [])[:1]:
                votes[f] += 1
        return votes.most_common(1)[0][0] if votes else "(no translated caller)"

    by_file: dict[str, list[dict]] = defaultdict(list)
    for addr in sorted(unfinished):
        f = by_addr[addr]
        callees = {to_int(c) for c in f.get("callees", [])} & countable
        tags = []
        if f.get("size", 0) > LARGE:
            tags.append("large")
        if has_jump_table(addr):
            tags.append("switch")
        if len(cpp_files.get(addr, [])) > 1:
            tags.append("dup")
        by_file[home(addr)].append({
            "addr": "0x%X" % addr,
            "name": f.get("name", ""),
            "size": f.get("size", 0),
            "kind": "missing" if addr in missing else "stub",
            "tags": tags,
            "stubbed_callees": ["0x%X" % c for c in sorted(callees & stubbed)],
            "missing_callees": ["0x%X" % c for c in sorted(callees & missing)],
        })

    batches = []
    for file in sorted(by_file):
        cur: list[dict] = []
        size = 0
        part = 1
        for item in by_file[file]:
            if cur and size + item["size"] > BATCH_BYTES:
                batches.append({"id": "%s#%d" % (file, part), "file": file, "bytes": size, "functions": cur})
                cur, size, part = [], 0, part + 1
            cur.append(item)
            size += item["size"]
        if cur:
            batches.append({"id": "%s#%d" % (file, part), "file": file, "bytes": size, "functions": cur})

    items = [i for b in batches for i in b["functions"]]
    return {
        "unfinished": len(items),
        "stubs": sum(1 for i in items if i["kind"] == "stub"),
        "missing": sum(1 for i in items if i["kind"] == "missing"),
        "bytes": sum(i["size"] for i in items),
        "tagged": dict(Counter(t for i in items for t in i["tags"])),
        "batch_count": len(batches),
        "batches": batches,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--write", action="store_true", help="write docs/worklist.json")
    args = ap.parse_args()
    w = build()
    print("unfinished functions: %d (%d stubs, %d missing), %d bytes, %d batches"
          % (w["unfinished"], w["stubs"], w["missing"], w["bytes"], w["batch_count"]))
    print("tags:", w["tagged"])
    per_file = Counter()
    for b in w["batches"]:
        per_file[b["file"]] += b["bytes"]
    print("largest files by unfinished bytes:")
    for file, n in per_file.most_common(15):
        print("  %7d  %s" % (n, file))
    if args.write:
        OUT.write_text(json.dumps(w, indent=1) + "\n", encoding="utf-8")
        print("wrote", OUT.relative_to(REPO).as_posix())
    return 0


if __name__ == "__main__":
    sys.exit(main())
