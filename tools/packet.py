#!/usr/bin/env python3
"""Print a context packet for translating one function of iwd2.exe.

  py -I tools/packet.py 0x46E130

The packet holds what a translator otherwise spends most of its effort
looking up:

  TARGET          name, size, repo declaration, own RET operands
  CALLEES         address, Ghidra name, callee RET, repo definition or MISSING
  DATA            referenced data addresses with the repo constant name or string
  VTABLES         vtable addresses stored by the function, resolved to a class
  MEMBER OFFSETS  offsets resolved through sub-objects, arrays and list headers
  ASM             the disassembly with [ESP+n] renamed to [argN] / [loc_X]

Inputs: IWD2_TOOLS_DIR (default C:\\iwd2-tools) for functions.json and
decomp\\0xADDR.asm, IWD2_REF_DIR (default C:\\iwd2-ref) for iwd2.exe.
"""

from __future__ import annotations

import json
import os
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from peek import load_pe, read  # noqa: E402

REPO = Path(__file__).resolve().parent.parent
TOOLS = Path(os.environ.get("IWD2_TOOLS_DIR", r"C:\iwd2-tools"))
REF = Path(os.environ.get("IWD2_REF_DIR", r"C:\iwd2-ref"))
DECOMP = TOOLS / "decomp"

CODE_START, CODE_END = 0x401000, 0x7DA200

# Classes whose members are tried for every offset, after the target's own class.
COMMON_ROOTS = ["CBaldurChitin", "CInfGame", "CGameSprite", "CGameArea", "CAIObjectType"]

SIZES = {
    "BYTE": 1, "BOOLEAN": 1, "char": 1, "unsigned char": 1,
    "SHORT": 2, "WORD": 2, "short": 2,
    "LONG": 4, "DWORD": 4, "INT": 4, "int": 4, "BOOL": 4, "STRREF": 4, "COLORREF": 4,
}
LIST_ACCESSORS = {4: "GetHeadPosition()", 8: "GetTailPosition()", 12: "GetCount()"}

RET_RE = re.compile(r"RET *(0x[0-9a-f]+)?")
ANNOTATION_RE = re.compile(r"// (0x[0-9A-F]+) *$")
CLASS_RE = re.compile(r"(?:class|struct|typedef struct) +([A-Za-z0-9_]+)(?: *: *public +([A-Za-z0-9_]+))?")
MEMBER_RE = re.compile(r" +/\* ([0-9A-F]{4}) \*/ (.*[ *&])([A-Za-z_][A-Za-z0-9_]*)(\[.*)?;")


def key(addr: int) -> str:
    return "0x%X" % addr


def scan_definitions() -> dict[str, list[tuple[str, str, bool]]]:
    """address -> [(file:line, declaration line, is_stub)] from `// 0xADDR` annotations."""
    idx: dict[str, list[tuple[str, str, bool]]] = {}
    for p in sorted((REPO / "src").rglob("*.cpp")):
        lines = p.read_text(errors="replace").split("\n")
        for i, line in enumerate(lines[:-1]):
            m = ANNOTATION_RE.match(line)
            if m:
                stub = "TODO: Incomplete" in "\n".join(lines[i + 1:i + 9])
                idx.setdefault(m.group(1), []).append((f"{p.name}:{i + 2}", lines[i + 1].strip(), stub))
    return idx


def scan_layouts() -> tuple[dict[str, list], dict[str, str]]:
    """class -> [(offset, type, name, array suffix)] and class -> base, from `/* OFFS */` members."""
    members: dict[str, list] = {}
    bases: dict[str, str] = {}
    for p in sorted((REPO / "src").glob("*.h")):
        cur = None
        for line in p.read_text(errors="replace").split("\n"):
            m = CLASS_RE.match(line)
            if m and not line.rstrip().endswith(";"):
                cur = m.group(1)
                members.setdefault(cur, [])
                if m.group(2):
                    bases[cur] = m.group(2)
            if "(" in line or cur is None:
                continue
            m = MEMBER_RE.match(line)
            if m:
                members[cur].append((int(m.group(1), 16), m.group(2).strip(), m.group(3), (m.group(4) or "").strip()))
    return members, bases


class Layouts:
    def __init__(self) -> None:
        self.members, self.bases = scan_layouts()

    def all_members(self, cls: str) -> list:
        out = list(self.members.get(cls, []))
        base = self.bases.get(cls)
        while base:
            out += self.members.get(base, [])
            base = self.bases.get(base)
        return sorted(out)

    def resolve(self, cls: str, off: int, depth: int = 0) -> str | None:
        """Name the member of `cls` at `off`, descending into sub-objects and arrays."""
        candidates = [m for m in self.all_members(cls) if m[0] <= off]
        if not candidates:
            return None
        start, mtype, name, array = candidates[-1]
        delta = off - start
        mtype = mtype.replace("const", "").strip()
        if array:
            size = 4 if mtype.endswith("*") else SIZES.get(mtype)
            if size and delta % size == 0 and delta // size < 512:
                return "%s[%d]" % (name, delta // size)
            return None
        if delta == 0:
            return name
        if "PtrList" in mtype or mtype.startswith("CList"):
            return name + "." + LIST_ACCESSORS[delta] if delta in LIST_ACCESSORS else None
        if mtype in self.members and depth < 4:
            inner = self.resolve(mtype, delta, depth + 1)
            return name + "." + inner if inner else None
        return None


def callee_rets(addr: int) -> list[str] | None:
    p = DECOMP / (key(addr) + ".asm")
    if not p.exists():
        return None
    return sorted({x or "plain" for x in RET_RE.findall(p.read_text(errors="replace"))})


def normalise_stack(asm: str) -> tuple[list[str], int]:
    """Rename [ESP+n] to entry-relative slots. Returns the lines and the number of
    join points where two paths disagreed about ESP (0 means the tracking is sound)."""
    lines = [l for l in asm.split("\n") if re.match(r"[0-9A-F]{8} ", l)]
    cur = 0          # ESP relative to its value at entry
    known: dict[int, int] = {}
    pending = 0      # pushes since the last call or branch, for indirect calls
    dead = False     # previous instruction never falls through
    mismatches = 0
    out = []
    for line in lines:
        addr = int(line[:8], 16)
        ins = line[10:].strip()
        if addr in known:
            if not dead and known[addr] != cur:
                mismatches += 1
            cur = known[addr]
            pending = 0
        dead = False

        def slot(m: re.Match) -> str:
            s = cur + int(m.group(1), 16)
            if s >= 4:
                return "[arg%d]" % (s // 4)
            return "[ret]" if s == 0 else "[loc_%X]" % -s

        out.append(line[:10] + re.sub(r"\[ESP \+ 0x([0-9a-f]+)\]", slot, ins))

        op = ins.split()[0]
        jump = re.match(r"(J[A-Z]+) 0x([0-9a-f]+)", ins)
        if jump:
            known.setdefault(int(jump.group(2), 16), cur)
            pending = 0
            dead = op == "JMP"
        elif op == "PUSH":
            cur -= 4
            pending += 1
        elif op == "POP":
            cur += 4
        elif op == "RET":
            dead = True
        elif re.match(r"SUB ESP,0x[0-9a-f]+$", ins):
            cur -= int(ins.split("0x")[1], 16)
        elif re.match(r"ADD ESP,0x[0-9a-f]+$", ins):
            cur += int(ins.split("0x")[1], 16)
        elif op == "CALL":
            direct = re.match(r"CALL 0x([0-9a-f]+)", ins)
            if direct:
                rets = callee_rets(int(direct.group(1), 16))
                if rets and len(rets) == 1 and rets[0] != "plain":
                    cur += int(rets[0], 16)
            else:
                cur += 4 * pending  # virtual thiscall: the callee pops what was pushed
            pending = 0
    return out, mismatches


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__)
        return 1
    addr = int(sys.argv[1], 16)
    asm_path = DECOMP / (key(addr) + ".asm")
    if not asm_path.exists():
        print(f"no disassembly at {asm_path}")
        return 1
    asm = asm_path.read_text(errors="replace")
    funcs = {int(f["addr"], 16): f for f in json.loads((TOOLS / "functions.json").read_text(encoding="utf-8"))}
    defs = scan_definitions()
    layouts = Layouts()
    exe, sections = load_pe(REF / "iwd2.exe")

    f = funcs.get(addr, {})
    print("=== TARGET", key(addr), f.get("name"), "size", f.get("size"))
    for where, decl, stub in defs.get(key(addr), []):
        print("repo:", where, decl, "[STUB]" if stub else "")
    print("own RET operands:", sorted(set(RET_RE.findall(asm))), "(RET 0xN pops N/4 stack args)")

    print("=== CALLEES: address | Ghidra name | callee RET | repo definition(s)")
    for t in sorted({int(x, 16) for x in re.findall(r"CALL 0x([0-9a-f]{8})", asm)}):
        rets = callee_rets(t)
        found = [w + "  " + d + ("  [STUB]" if s else "") for w, d, s in defs.get(key(t), [])]
        if not found:
            found = ["NOT IN REPO " + ("(MISSING CALLEE)" if CODE_START <= t < CODE_END else "(library)")]
        print(key(t), "|", funcs.get(t, {}).get("name", "?"), "| RET", ",".join(rets) if rets else "?", "|", " ;; ".join(found))

    print("=== DATA: address | repo constant | string")
    vtables = []
    for c in sorted({int(x, 16) for x in re.findall(r"0x(?:00)?(8[4-9a-c][0-9a-f]{4})", asm)}):
        if ("CALL 0x%08x" % c) in asm:
            continue
        try:
            raw = read(exe, sections, c, 32)
        except SystemExit:
            continue
        slots = []
        owners: list[str] = []
        for i, v in enumerate(struct.unpack("<8I", raw)):
            if CODE_START <= v < CODE_END and key(v) in defs:
                found = defs[key(v)]
                slots.append("+0x%X %s" % (i * 4, " / ".join(d[1] for d in found)[:80]))
                if len(found) == 1:  # not folded with another class's identical function
                    owners += re.findall(r"([A-Za-z0-9_]+)::", found[0][1])[:1]
        if len(slots) >= 2:
            cls = max(set(owners), key=owners.count) if owners else "(unknown class)"
            vtables.append("%s is the VTABLE of %s: %s" % (key(c), cls, " | ".join(slots[:5])))
            continue
        text = raw.split(b"\0")[0]
        string = text.decode("latin-1") if len(text) >= 3 and all(32 <= b < 127 for b in text) else ""
        print(key(c), "|", " ;; ".join(d[1] for d in defs.get(key(c), [])) or "-", "|", string[:60])

    print("=== VTABLES stored by this function")
    for v in vtables:
        print(v)

    roots = []
    own = defs.get(key(addr))
    m = re.search(r"([A-Za-z0-9_]+)::", own[0][1]) if own else None
    for r in ([m.group(1)] if m else []) + COMMON_ROOTS:
        if r not in roots:
            roots.append(r)
    print("=== MEMBER OFFSETS (resolved through sub-objects and arrays; pick the row whose class the register holds;"
          " g_pChitin and g_pBaldurChitin are both CBaldurChitin)")
    for off in sorted({int(x, 16) for x in re.findall(r"E(?:AX|BX|CX|DX|SI|DI|BP) \+ 0x([0-9a-f]+)\]", asm)}):
        if off < 0x10:
            continue
        row = [r + "::" + name for r in roots for name in [layouts.resolve(r, off)] if name]
        print("0x%X:" % off, " ;; ".join(row) if row else "(not resolved: grep the class header)")

    lines, mismatches = normalise_stack(asm)
    print("=== ASM (ground truth). Stack references are normalised: [argN] is the Nth stack argument, [loc_X] is a"
          " local slot; the same name is always the same slot, whatever ESP was. Slot tracking mismatches:", mismatches)
    print("\n".join(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
