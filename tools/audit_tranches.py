#!/usr/bin/env python3
"""Split one file's unread old functions (see old_code.py) into tranches and
write a verifier brief for each.

  py -I tools/audit_tranches.py OUTDIR TAG src/File.cpp ["extra hint for the verifier"]

Writes OUTDIR/audit_TAG_N.txt (one "0xADDR size" per line) and
OUTDIR/brief_TAG_N.txt (the full task brief for an iwd2-verifier), and prints
one line per tranche: tag, number, function count, bytes, largest function.
OUTDIR must be outside the repo (a scratch directory).

  py -I tools/audit_tranches.py --mark OUTDIR TAG [TAG ...]

Appends every address in OUTDIR/audit_TAG_*.txt to tools/audited_old.txt.
Run it only after the fixes for that file are merged.
"""
import glob
import os
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
TRANCHE_BYTES = 11000  # about what one verifier reads with full care
TAIL_MIN = 3000        # a last tranche smaller than this joins the previous one

BRIEF = """Audit of old, never-verified hand-written code. Read-only on the main tree `{repo}` (do not edit, build or commit anything there; a build may be running, ignore it). Run every command in the foreground.

The list of functions is in `{listfile}` (one per line: address, size in bytes). All are in `{src}`; locate each by its `// 0xADDR` comment. Listings are `C:\\iwd2-tools\\decomp\\0xADDR.asm` (if a listing ends before the function does, the body continues in the next-address `.asm` files there or in `C:\\iwd2-tools\\orphans\\`); `py -I tools/packet.py 0xADDR` gives callees, constants and resolved member offsets (its stack-slot names can be off after a `SUB ESP`; count ESP by hand when in doubt); `py -I tools/peek.py` (d/w/b/s ADDR N) reads exe data.

Check every function in the list, in order, against its asm with the verify-against-asm skill, using its "Auditing old code" section. Do not sample or skip; trivially small ones (under ~40 bytes) need only a quick look; every other function must be read in full, with the same care at its end as at its start.{extra}

Output: report only functions that differ, each as `## 0xADDR Name` then BEHAVIOUR or SHAPE, what the exe does, what the repo does, the asm addresses proving it, and the exact corrected C++ line(s). End with a count: functions checked out of {count} in the list, functions differing, and the address of any you could not check and why. Stay fully read-only.
"""


def old_functions(src):
    out = subprocess.run([sys.executable, "-I", str(REPO / "tools" / "old_code.py"), src],
                         capture_output=True, text=True, cwd=REPO).stdout
    fns = []
    for line in out.splitlines():
        p = line.split()
        if len(p) == 3 and p[0].startswith("0x"):
            fns.append((p[0], int(p[1])))
    return fns


def split(fns):
    tranches, cur, size = [], [], 0
    for addr, n in fns:
        if size + n > TRANCHE_BYTES and cur:
            tranches.append(cur)
            cur, size = [], 0
        cur.append((addr, n))
        size += n
    if cur:
        if tranches and size < TAIL_MIN:
            tranches[-1] += cur
        else:
            tranches.append(cur)
    return tranches


def mark(outdir, tags):
    addrs = set()
    for tag in tags:
        for f in glob.glob(os.path.join(outdir, "audit_%s_*.txt" % tag)):
            for line in open(f):
                if line.strip():
                    addrs.add(line.split()[0])
    with open(REPO / "tools" / "audited_old.txt", "a", newline="\n") as fh:
        for a in sorted(addrs):
            fh.write(a + "\n")
    print("marked", len(addrs), "addresses")


def main(argv):
    if len(argv) >= 3 and argv[0] == "--mark":
        mark(argv[1], argv[2:])
        return 0
    if len(argv) < 3:
        print(__doc__)
        return 2
    outdir, tag, src = argv[0], argv[1], argv[2].replace("\\", "/")
    extra = (" " + argv[3]) if len(argv) > 3 else ""
    tranches = split(old_functions(src))
    for i, t in enumerate(tranches, 1):
        listfile = os.path.abspath(os.path.join(outdir, "audit_%s_%d.txt" % (tag, i)))
        with open(listfile, "w") as fh:
            fh.write("".join("%s %d\n" % x for x in t))
        with open(os.path.join(outdir, "brief_%s_%d.txt" % (tag, i)), "w") as fh:
            fh.write(BRIEF.format(repo=REPO, listfile=listfile, src=src, extra=extra, count=len(t)))
        print(tag, i, len(t), sum(n for _, n in t), max(n for _, n in t))
    if not tranches:
        print("no unread old functions in", src)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
