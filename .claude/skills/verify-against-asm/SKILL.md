---
name: verify-against-asm
description: Check a translated C++ function against the original disassembly and report every difference. Use after a function has been translated and before it is merged. Read-only.
---

# Verify a translation against the asm

Input: one function address. Don't edit anything.

1. Run `py -I tools/packet.py 0xADDR`. Find the C++ from the `repo:` line.
2. Don't trust the translator's report or comments. Work from the asm.
3. Check each of these, in order, and cite the asm address for anything that differs:
   - **Stack slots.** List every `[loc_X]` / `[argN]` slot and what is stored into it. For
     every read of a slot, confirm the C++ uses that variable. This is the most common error.
   - **Calls.** Same calls in the same order. For each, the number of arguments equals the
     pushes, and equals the callee's `RET n` in the packet. Each argument is the right value.
   - **Branches.** Each conditional jump matches a C++ condition with the same sense, width
     and signedness (JL/JG signed, JB/JA unsigned; byte/word/dword).
   - **All-ones compares.** `-1` only on signed C++ types; `0xFF`/`0xFFFF` on BYTE/WORD.
   - **Members.** Each `[reg + 0xNN]` maps to the member the C++ uses, on the class the
     register actually holds.
   - **Constants** compared or stored, and the value on each return path.
   - **Shape.** Nothing added (NULL checks, fixes) and nothing dropped (leaks, repeated
     calls, dead stores that have side effects).
4. Before reporting a difference, re-read the asm lines it rests on and copy them into the
   report exactly as the packet prints them, address included. Never retype an instruction
   from memory. If the packet says the stack slots are not normalised, show your ESP count at
   each line you cite.
5. Before calling something a difference, check whether the asm is an inlined copy of a
   callee the C++ calls (read the callee's own asm). An inlined helper is not a difference.
6. Never state a callee's signature, or what a jump or byte table holds, without reading it:
   the callee's asm for the first, `py -I tools/peek.py` on the table for the second.

## Auditing old code

Use this section when the brief gives a list of old, never-verified functions in one file.

- Check every address in the list, in order. Keep a tally. The final count must say
  "checked N of N" against the list's length and name any address you did not check.
- These defects recur in old code; look for each one in every function:
  - **Copied from a sibling and not adapted:** wrong member, cell, list, slot index, control
    id, panel id, strref, resref suffix or constant. Compare with the sibling when the
    function has one (base/extend, open/closed, weapon/shield/helmet, G1/G2).
  - **Switches:** a missing `break`, a missing or extra case, two cases swapped, a default
    that asserts the wrong expression (the exe usually pushes the string `FALSE`), a case
    that sets the wrong one of two similar members.
  - **Conditions:** `||` for `&&`, `=` for `==`, an inverted test, `>` for `>=` at a
    rectangle edge, a guard on the wrong object (`pControl->x` for `this->x`).
  - **Constructors:** every member store in the asm needs a counterpart; look for members
    never initialised and for sub-object stores that are an inlined constructor.
  - **Loops:** never incremented, wrong bound, wrong start index, a counter held in a BYTE
    in the exe (it wraps), a count that is returned but never incremented.
  - **Returns:** wrong value on one path, a missing early return after an assert, running
    off the end of a non-void function.
  - **Width and sign:** a byte or word field read, written or compared at full width;
    `~x` or `-x` on a BYTE promoted to int; unsigned divide where the exe has `IDIV` (or
    the reverse); `/ 2` where the exe has `SAR` with no sign fix; float where the exe uses
    double; truncate-then-add where the exe adds then truncates.
  - **Win32 and CRT:** `FAILED(hr)` where the exe tests non-zero; `strcpy` where the exe
    copies a fixed number of bytes; `Find` against `FindOneOf`; a `Demand` with no
    `Release`, or the helper's `Request`/`Release` where the exe calls the resource's own.
  - **Locks:** wrong thread index or lock kind, a missing retry loop, a flag cleared before
    the unlock where the exe clears it after.
  - **Calls:** a dropped call, an extra call, a call on the wrong object, a by-value
    parameter where the exe writes through a pointer.
- Don't report naming, comments or style. Report a wrong assert line number in one line.
- Give the exact corrected C++ for each difference, and say when a proposed form is only
  equivalent under an assumption (for example a loop bound that ignores byte wrap).

## Report

7. Report:
   - `VERDICT: MATCHES` or `VERDICT: DIFFERS`
   - one line per difference: `0xADDR: asm does X; C++ does Y; fix: Z`, followed by the
     copied asm lines
   - `UNSURE:` lines for anything you could not settle, with the address
