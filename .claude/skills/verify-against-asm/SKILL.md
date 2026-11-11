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
5. Report:
   - `VERDICT: MATCHES` or `VERDICT: DIFFERS`
   - one line per difference: `0xADDR: asm does X; C++ does Y; fix: Z`, followed by the
     copied asm lines
   - `UNSURE:` lines for anything you could not settle, with the address
