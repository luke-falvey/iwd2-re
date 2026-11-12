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


_RET_CACHE: dict[int, list[int]] = {}


def callee_pops(addr: int) -> list[int]:
    """Stack bytes a direct callee may pop, most likely first."""
    if addr not in _RET_CACHE:
        rets = callee_rets(addr) or ["plain"]
        _RET_CACHE[addr] = sorted({0 if r == "plain" else int(r, 16) for r in rets}, reverse=True)
    return _RET_CACHE[addr]


# CGameObject vtable slots that take no stack arguments: GetObjectType, GetAIType,
# GetPos, GetVertListPos, GetVertListType, RemoveFromArea, OnArrival, RemoveSelf.
# Only a first guess: the search still overrides it when the function disagrees.
ZERO_ARG_SLOTS = {0x04, 0x10, 0x1C, 0x20, 0x24, 0x48, 0x70, 0x74}


def _simulate(lines: list[str], choices: dict[int, int]):
    """One pass over the function. `choices` maps the index of a call whose pop
    count is not known to the option to take. Returns the renamed lines, the
    options available at each such call, the number of join points where two
    paths disagree about ESP, and the index of the first line that proves a
    choice wrong (None if the pass is consistent)."""
    cur = 0          # ESP relative to its value at entry
    known: dict[int, int] = {}
    pending = 0      # pushes since the last call
    saves = 0        # of those, pushes that look like register saves, not arguments
    saved: set[str] = set()
    dead = False     # previous instruction never falls through
    mismatches = 0
    first_bad = None
    restored = {r for r in ("EBX", "ESI", "EDI", "EBP") if any(l[10:].strip() == "POP " + r for l in lines)}
    table_cur = None  # ESP at the last jump-table dispatch
    need = 0         # how much more the calls before first_bad should have popped
    options: dict[int, list[int]] = {}
    out = []
    for n, line in enumerate(lines):
        addr = int(line[:8], 16)
        ins = line[10:].strip()
        if addr in known:
            if not dead and known[addr] != cur:
                mismatches += 1
                if first_bad is None:
                    first_bad, need = n, known[addr] - cur
            cur = known[addr]
        elif dead and table_cur is not None:
            cur = table_cur  # a case block reached only through the jump table
        dead = False

        def slot(m: re.Match) -> str:
            s = cur + int(m.group(1) or "0", 16)
            if s >= 4:
                return "[arg%d]" % (s // 4)
            return "[ret]" if s == 0 else "[loc_%X]" % -s

        out.append(line[:10] + re.sub(r"\[ESP(?: \+ 0x([0-9a-f]+))?\]", slot, ins))

        op = ins.split()[0]
        jump = re.match(r"(J[A-Z]+) 0x([0-9a-f]+)", ins)
        if jump:
            target = int(jump.group(2), 16)
            if known.setdefault(target, cur) != cur:
                mismatches += 1
                if first_bad is None:
                    first_bad, need = n, known[target] - cur
            dead = op == "JMP"
        elif op == "JMP":
            dead = True  # jump table or tail call through a pointer
            if "*0x4" in ins:
                table_cur = cur
        elif op == "PUSH":
            cur -= 4
            pending += 1
            reg = ins[5:]
            # The first push of a callee-saved register that is popped again
            # later is a save. A second push while it is saved is an argument.
            if reg in restored and reg not in saved:
                saved.add(reg)
                saves += 1
        elif op == "POP":
            cur += 4
            saved.discard(ins[4:])
        elif op == "RET":
            if cur != 0:
                mismatches += 1
                if first_bad is None:
                    first_bad, need = n, -cur
            dead = True
        elif re.match(r"SUB ESP,0x[0-9a-f]+$", ins):
            cur -= int(ins.split("0x")[1], 16)
        elif re.match(r"ADD ESP,0x[0-9a-f]+$", ins):
            cur += int(ins.split("0x")[1], 16)
        elif op == "CALL":
            direct = re.match(r"CALL 0x([0-9a-f]+)", ins)
            if direct:
                opts = callee_pops(int(direct.group(1), 16))
            else:
                # Through a pointer: the callee pops some of what was pushed
                # since the last call. All of it is the usual case.
                opts = [4 * k for k in range(pending, -1, -1)]
                likely = 4 * (pending - saves)
                # Getters every game object has take no stack arguments, so
                # pushes made before them belong to a later call. Without
                # this, `PUSH a; PUSH b; CALL [vtbl+0x1c]; PUSH ...; CALL f`
                # is ambiguous and the slots in between come out 8 bytes off.
                virt = re.match(r"CALL dword ptr \[E[A-Z]{2} \+ 0x([0-9a-f]+)\]$", ins)
                if virt and int(virt.group(1), 16) in ZERO_ARG_SLOTS:
                    likely = 0
                opts.remove(likely)
                opts.insert(0, likely)
            if len(opts) > 1:
                options[n] = opts
            popped = opts[min(choices.get(n, 0), len(opts) - 1)]
            cur += popped
            # Pushes the callee left on the stack are arguments of a later call.
            pending = max(0, pending - saves - popped // 4) if not direct else 0
            saves = 0
    return out, options, mismatches, first_bad, need


def normalise_stack(asm: str, budget: int = 400) -> tuple[list[str], int]:
    """Rename [ESP+n] to entry-relative slots. Returns the lines and the number of
    places where the tracking is inconsistent (0 means it is sound).

    MSVC 6 addresses locals from ESP, so the same slot has a different offset after
    every push. Calls through a pointer don't say how much they pop; the search
    below picks the pop counts that make every join point and every RET agree."""
    lines = [l for l in asm.splitlines() if re.match(r"[0-9A-F]{8} ", l)]
    choices: dict[int, int] = {}
    tried: set[tuple[int, int]] = set()
    best = None
    for _ in range(budget):
        out, options, mismatches, first_bad, need = _simulate(lines, choices)
        if best is None or mismatches < best[1]:
            best = (out, mismatches)
        if first_bad is None:
            break
        earlier = sorted((n for n in options if n < first_bad), reverse=True)
        # Prefer the latest call that can absorb the whole difference.
        for n in earlier:
            opts = options[n]
            want = opts[min(choices.get(n, 0), len(opts) - 1)] + need
            if want in opts and (n, want) not in tried:
                tried.add((n, want))
                choices = {k: v for k, v in choices.items() if k < n}
                choices[n] = opts.index(want)
                break
        else:
            # Otherwise step the latest call that still has an untried option.
            for n in earlier:
                if choices.get(n, 0) + 1 < len(options[n]):
                    choices = {k: v for k, v in choices.items() if k < n}
                    choices[n] = choices.get(n, 0) + 1
                    break
            else:
                break
    return best


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
    if mismatches == 0:
        print("=== ASM (ground truth). Stack references are normalised: [argN] is the Nth stack argument, [loc_X] is a"
              " local slot; the same name is always the same slot, whatever ESP was.")
        print("\n".join(lines))
    else:
        # Wrong slot names are worse than none, so fall back to the raw listing.
        print("=== ASM (ground truth). STACK SLOTS NOT NORMALISED: ESP could not be tracked through this function"
              " (%d inconsistencies). [ESP + n] is raw: track every PUSH, POP and callee RET yourself to know which"
              " slot each reference means." % mismatches)
        print("\n".join(l for l in asm.splitlines() if re.match(r"[0-9A-F]{8} ", l)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
