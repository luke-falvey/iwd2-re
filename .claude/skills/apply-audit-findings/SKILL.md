---
name: apply-audit-findings
description: Confirm a list of audit claims about existing C++ against the disassembly and fix what is confirmed. Use when given one or more findings files from an old-code audit and a worktree to fix them in.
---

# Apply audit findings

Input: a worktree and branch, the files you own, and one or more findings files. Each
section of a findings file is one claim: a function address, what the exe is said to do,
what the repo does, asm addresses, and a proposed fix.

Claims are unconfirmed. About one in forty is wrong, and about one in eight proposed fixes
is not quite what the exe does. You decide from the asm.

## For each claim

1. Read the asm the claim cites: `C:\iwd2-tools\decomp\0xADDR.asm` (if the listing stops
   before the function does, continue in the next-address files there or in
   `C:\iwd2-tools\orphans\`). `py -I tools/packet.py 0xADDR` gives callees, constants and
   member offsets. Its stack-slot names can be off after a `SUB ESP`; count ESP by hand
   when a claim rests on a slot.
2. Decide: is the claim true as stated?
   - **Jump and byte tables:** read them from the exe with `py -I tools/peek.py`, entry by
     entry. Never take a case mapping from the claim.
   - **Inlined callees:** if the "different" code is an inlined copy of a helper the repo
     already calls, it is not a difference. Read the helper's own asm to check.
   - **Loop bounds and arithmetic:** mirror the exe's width and sign. A counter in an 8-bit
     register wraps; `IDIV` is signed; `SAR` is not `/ 2` for negative values. If the
     proposed fix is only equivalent under an assumption, write the exact form instead.
   - **Line numbers** in claims are approximate. Find the code by its content.
3. Apply what you confirm, in the repo's style: `// 0xADDR` above functions, `// NOTE:` for
   original quirks you keep, `// __FILE__` / `// __LINE__` above asserts. Faithful means the
   exe's behaviour, including its bugs.
4. If you are already reading a function and see another real difference, fix it and
   report it. Don't go looking in functions no claim touches.
5. If a claim asks you to close a coverage gap (functions the audit did not check), check
   those against the asm yourself.

## Changes that reach other files

- **A signature change** (parameter by reference, an added parameter): read the asm of every
  override and change them all; grep for every caller.
- **A member width change in a header:** confirm the offsets of the members after it from
  the asm, fix the `/* OFFS */` comments, and make sure the layout tests pass. If a class in
  `tests/layout/known_mismatches.txt` is now correct, remove its entry; if any class gets
  worse, stop and report.
- Touch files you don't own only for these, and list them.

## Things the play tests depend on

The play tests compare screenshots with the original and you can't run them. When a claim
changes drawing, text, palettes, sound streaming or the start-up path, work out what the
common case does before and after, and say in the report whether it is the same.

## Finish

1. `tools\check.ps1` from the worktree, in the foreground. It must end with `check OK`.
2. Commit on the given branch, staging only the files you changed, by name. End the message
   with the attribution line you were given.
3. Report, once:
   - commit hash
   - per claim: `APPLIED`, `APPLIED DIFFERENTLY` (say how) or `REJECTED` (say why), with the
     deciding asm address
   - extra differences you fixed
   - files touched and header changes
   - doubts, each tied to an address
