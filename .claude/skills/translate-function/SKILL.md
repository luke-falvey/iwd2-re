---
name: translate-function
description: Translate one function of iwd2.exe to C++ from its context packet. Use when given a function address (0xADDR) to translate or to fill a `TODO: Incomplete` stub.
---

# Translate one function

Input: one function address, and the worktree to work in.

## Step 1: get the packet

```
py -I tools/packet.py 0xADDR
```

It prints the target's repo declaration, every callee (Ghidra name, callee `RET n`, repo
definition, `[STUB]` or `MISSING CALLEE`), data constants, vtables resolved to classes, member
offsets resolved to names, and the asm with stack slots renamed (`[argN]`, `[loc_X]`).

## Step 2: translate from the packet

Don't read `docs/translating.md`, `functions.json` or the Ghidra `.c` file. Look things up
only when the packet leaves a name unresolved:

- Member offset not resolved: grep `/\* 0XXX \*/` (4 upper-case hex digits) in the header of
  the class the register holds. Subtract the sub-object's offset first if it's inside one.
- Virtual call `CALL [reg + 0xNN]`: grep `/\* 00NN \*/ virtual` in the class header (bases:
  `CGameObject.h`, `CGameAIBase.h`).
- A constructor's parameter order, or a small MISSING CALLEE's behaviour: read its asm at
  `C:\iwd2-tools\decomp\0xADDR.asm`.
- Local style: one read of about 60 lines around a translated sibling in the same `.cpp`.

## Facts

- `this` is in ECX.
- `g_pChitin` (`[0x8CF6D8]`) and `g_pBaldurChitin` (`[0x8CF6DC]`) are the same `CBaldurChitin`.
- The repo builds with 2-byte packing (`/Zp2`). Never add pack pragmas.
- `CPtrList` node: `pNext` +0, `pPrev` +4, `data` +8.
- A GetShare/GetDeny call inside a retry loop is written
  `do { rc = ...; } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);`
  but only with the conditions the asm actually tests. `[0x851870]` is SHARED and
  `[0x851871]` is DENIED. Many loops test DENIED alone; some calls have no loop at all.
- `operator new(size)` followed by stores of a vtable and fields is an inlined constructor.
  Write `new ClassName(args)` with the class from the packet's VTABLES section.
- A `CGameEffect` has its vtable at offset 0 and the file structure (`CGameEffectBase`) after
  it, so an asm offset on an effect pointer is the header's `/* OFFS */` comment plus 4:
  `[effect+0xC]` is `m_effectID`, `+0x18` `m_effectAmount`, `+0x1C` `m_dwFlags`,
  `+0x20` `m_durationType`, `+0x24` `m_duration`, `+0x58` `m_flags`.
- A `CMessage` stores its target id at +4 and its source id at +8; the constructors take the
  caller first, then the target.
- MFC `CString`: `[data-8]` is `GetLength()`, `[data-4]` is `GetAllocLength()`.
- Read the asm of a function the tools attribute to a file before translating it. STL, MFC
  template and compiler-generated code (tree or list internals, scalar deleting thunks, catch
  funclets, emitted copies of inline accessors) goes in `tools/inventory_exclusions.txt`.
- Run every command from your worktree, including `tools\check.ps1`.

## Rules

1. **Faithful.** Same control flow, same order of calls, same quirks. Never fix original bugs
   or leaks; mark them `// NOTE:`. Don't restructure: no hand-inlined destructors, no extra
   NULL checks.
2. **Calls.** For every CALL, count the values pushed for it and compare with the callee's RET
   in the packet (`RET 0xN` = N/4 stack arguments for thiscall). Pass exactly that many. Never
   state a callee signature the packet doesn't show.
3. **Width and sign.** Take them from the asm (MOVSX/MOVZX, JL/JG signed, JB/JA unsigned,
   byte/word/dword) and from the declared C++ type. Comparing with all-ones: write `-1` when
   the C++ type is signed (SHORT, char, int, LONG), and `0xFF` / `0xFFFF` / `0xFFFFFFFF` when
   it's unsigned (BYTE, WORD, DWORD). Never `x == 0xFFFF` for a SHORT or `x == -1` for a
   BYTE/WORD. When the asm compares a narrower width than the member's declared type
   (`CMP byte ptr [EAX],0x4` on a WORD field such as `ITEM_ABILITY::type`), cast:
   `static_cast<BYTE>(p->type) == 4`, with a `// NOTE:`.
   String literals: read every pushed string address in the packet's DATA section and copy
   it exactly. `"\n"`, `"\r\n"` and `"\n\r"` are three different constants.
4. **Names.** Use repo names from the packet. No raw pointer arithmetic, no raw vtable stores,
   no invented `field_XXXX` when a header has the member. Take a member from the class the
   register holds, not from another struct that has a field with the same name. If a header
   truly has no member at that offset, use `field_XXXX` and list it under doubts. Name a
   MISSING CALLEE with its Ghidra name without the class prefix.
5. **Helpers.** If the asm is an inlined copy of an existing repo helper (list walking,
   accessors such as `GetObjectArray()`), call the helper. Add `// NOTE: Uninline.` when the
   helper is a real function in the exe.
6. **Asserts.** `// __FILE__: <string>` and `// __LINE__: <decimal>` above
   `UTIL_ASSERT(<expression>)`.
7. **Signature.** If the declaration disagrees with the function's own RET or its argument
   reads, correct the declaration.
8. **Stack slots.** Before writing code, list every `[loc_X]` and `[argN]` slot with what is
   stored into it: a saved register, an out-parameter whose address is passed to a call, a
   running value. Every later read of that slot is that variable. Don't decide what a slot
   holds from what would make sense.

## Step 3: write, build, report

- Keep the `// 0xADDR` line above the function. Remove `// TODO: Incomplete.` only when the
  whole body is translated.
- Edit only the files you were assigned. If a callee is MISSING or a `[STUB]`, don't write it
  unless it was assigned to you; list it in the report.
- Never use `fork` subagents. Never commit `// DBGTRACE` blocks.
- Build from PowerShell with `tools\check.ps1`. Fix what you broke.

Report, and nothing else:

- **A.** Files and functions changed.
- **B.** Header changes.
- **C.** At most 8 doubts, one line each: `0xADDR: what is unsure`.
- **D.** FACTS block:
  - `SLOTS:` one line per stack slot, `loc_X = variable`
  - `CALLS:` one line per call in asm order, `0xADDR(nargs)`; virtual calls `vtbl+0xNN(nargs)`
  - `RETURNS:` one line per return path with its value
  - `CONSTANTS:` every literal compared or stored
- **E.** Stubbed or missing callees this function still depends on.
