#!/usr/bin/env python3
"""Compare the repo's annotated integer constants with the exe's data.

Looks at definitions of the form
  // 0xADDR
  const TYPE Class::NAME = <value>;
  const TYPE Class::NAME[N] = { <value>, ... };
where a value is an integer literal, TRUE/FALSE, RGB(r, g, b), a character
literal or a `#define` from src/*.h that resolves to one of those, and reads
the bytes at ADDR from the exe. Prints every mismatch. Definitions with any
other initialiser (expressions, strings, objects) are counted as skipped;
`-v` lists them.

  py -I tools/static_values.py [-v]
"""
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import peek  # noqa: E402

REF = Path(os.environ.get("IWD2_REF_DIR", r"C:\iwd2-ref"))
SRC = Path(__file__).resolve().parent.parent / "src"
SIZES = {"BYTE": 1, "BOOLEAN": 1, "CHAR": 1, "char": 1, "UCHAR": 1, "bool": 1,
         "WORD": 2, "SHORT": 2, "USHORT": 2, "short": 2,
         "DWORD": 4, "LONG": 4, "ULONG": 4, "INT": 4, "UINT": 4, "int": 4,
         "BOOL": 4, "STRREF": 4, "COLORREF": 4, "long": 4}
DEF = re.compile(
    r"^// 0x([0-9A-Fa-f]{6,8})[ \t]*\r?\n[ \t]*(?:static[ \t]+)?(?:const[ \t]+)?"
    r"(?:unsigned[ \t]+)?([A-Za-z_]+)[ \t]+(?:const[ \t]+)?([A-Za-z_0-9:]+)[ \t]*(\[[^\]]*\])?[ \t]*=[ \t]*"
    r"(\{[^;]*\}|[^;{\r\n]*);", re.M)
NUM = re.compile(r"^(-?)\s*(0[xX][0-9A-Fa-f]+|\d+)[uUlL]*$")
RGBM = re.compile(r"^RGB\(([^,]+),([^,]+),([^)]+)\)$")
DEFINE = re.compile(r"^[ \t]*#define[ \t]+([A-Za-z_][A-Za-z_0-9]*)[ \t]+(.+?)[ \t]*(?://.*)?$", re.M)
WORDS = {"TRUE": 1, "FALSE": 0, "true": 1, "false": 0, "NULL": 0}

defines = {}
for f in SRC.rglob("*.h"):
    for m in DEFINE.finditer(f.read_text(encoding="utf-8", errors="replace")):
        defines.setdefault(m.group(1), m.group(2).strip())


def value(text, depth=0):
    text = text.strip()
    while text.startswith("(") and text.endswith(")"):
        text = text[1:-1].strip()
    n = NUM.match(text)
    if n:
        return int(n.group(2), 0) * (-1 if n.group(1) else 1)
    if text in WORDS:
        return WORDS[text]
    if len(text) == 3 and text[0] == text[2] == "'":
        return ord(text[1])
    c = re.match(r"^'\\x([0-9A-Fa-f]{1,2})'$", text)
    if c:
        return int(c.group(1), 16)
    r = RGBM.match(text)
    if r:
        parts = [value(g, depth + 1) for g in r.groups()]
        return None if None in parts else parts[0] | parts[1] << 8 | parts[2] << 16
    if depth < 8 and text in defines:
        return value(defines[text], depth + 1)
    return None


d, secs = peek.load_pe(REF / "iwd2.exe")
same = bad = skipped = 0
for f in sorted(SRC.rglob("*.cpp")):
    for m in DEF.finditer(f.read_text(encoding="utf-8", errors="replace")):
        addr, typ, name, arr, init = int(m.group(1), 16), m.group(2), m.group(3), m.group(4), m.group(5).strip()
        if typ not in SIZES or addr < 0x7DA200:
            skipped += 1
            continue
        if init.startswith("{"):
            body = re.sub(r"//[^\n]*|/\*.*?\*/", "", init[1:-1], flags=re.S)
            vals = [value(v) for v in re.split(r",(?![^()]*\))", body) if v.strip()]
        else:
            vals = [value(init)] if arr is None else [None]
        if None in vals or not vals:
            skipped += 1
            if "-v" in sys.argv:
                print("skipped 0x%X %s %s = %s" % (addr, typ, name, init[:60].replace("\n", " ")))
            continue
        size = SIZES[typ]
        mask = (1 << (8 * size)) - 1
        raw = peek.read(d, secs, addr, size * len(vals))
        got = [int.from_bytes(raw[i * size:(i + 1) * size], "little") for i in range(len(vals))]
        want = [v & mask for v in vals]
        if got == want:
            same += 1
        else:
            bad += 1
            i = next(k for k in range(len(vals)) if got[k] != want[k])
            print("DIFF 0x%X %s %s %s%s: repo %d, exe %d (0x%X)%s" % (
                addr, f.name, typ, name, "[%d]" % i if len(vals) > 1 else "", vals[i], got[i], got[i],
                "; %d of %d elements differ" % (sum(a != b for a, b in zip(got, want)), len(vals)) if len(vals) > 1 else ""))
print("integer constants compared: %d; different: %d; skipped (other initialisers): %d" % (same + bad, bad, skipped))
