---
name: audit-old-file
description: Run the old-code audit on one or more source files - split into tranches, verify against the exe, fix, merge, mark audited, play-test. Use when asked to audit old or never-verified code, or to continue the old-code audit. For the orchestrating session, not for subagents.
---

# Audit old code in a file

"Old code" is what `py -I tools/old_code.py` lists: functions written before this project
whose bodies nobody has read against the exe. Roughly one in five has a real difference.

`py -I tools/old_code.py | head -40` lists files by unread bytes. Prefer files the
single-player game uses; the multiplayer screens and `CNetwork` come last.

## 1. Split and brief

```
py -I tools/audit_tranches.py <scratch dir> <tag> src/File.cpp ["hint"]
```

This writes `audit_<tag>_N.txt` and `brief_<tag>_N.txt` per tranche and prints the size of
each tranche and its largest function. Use a short tag per file. Add a hint when the file
has a known trap (for animation classes: which cell, suffix and sequence each line uses).

## 2. Verify

One `iwd2-verifier` per tranche, prompt: "Your full task brief is in the file
`<brief path>` - read it first and follow it exactly. Stay fully read-only and report the
findings in your final message."

- Model: Sonnet by default; Opus when the tranche's largest function is over about 3 KB.
- Verifiers stay read-only and report in their final message. Don't ask one to write its
  report to a file or to repeat a report.
- If the report's count is lower than the tranche's function count, tell the fixer to check
  the gap.

## 3. Write the findings file

Write `<tag>_findings_N.md` in the scratch directory from each report: one section per
differing function with the claim, the asm lines exactly as reported, and the proposed fix.
Head it "claims, confirm each against the asm". Add your own cautions: what the play tests
show of this code, and any claim that looks like an inlined callee or a non-equivalent fix.

## 4. Fix

One `iwd2-translator` per worktree (`C:\Users\Luke\Projects\iwd2-re-wt\batch3|4|5`,
branches `wip/batchN`), with the `apply-audit-findings` skill. Fast-forward the worktree
first: `git -C <worktree> merge -q --ff-only phase-a/stubs-and-layout`.

The prompt names: the worktree and branch, the findings files, the files the agent owns,
what the play tests depend on in this code, and the commit attribution line. Opus for
functions over about 3 KB, for drawing and resource code, and for anything with a header or
signature change.

- Two files that share a change (a virtual signature, a header) go to the same worktree in
  sequence, not to two worktrees.
- If a fixer disagrees with the audit about whether a function matches, send a third agent
  to translate that function afresh and compare. Don't pick a side yourself.
- If an agent dies mid-task, look at the worktree (`git status`, `git log`). A commit with
  no report: review the diff against the findings yourself. Uncommitted edits: give them to
  the next agent as unreviewed.

## 5. Merge, mark, test

On `phase-a/stubs-and-layout`, with no play test running:

```
git merge --no-ff <hash> -m "Merge batch N: <file> old-code audit fixes (<what>)"
py -I tools/audit_tranches.py --mark <scratch dir> <tag> [<tag> ...]
git commit -am "Mark <file> old functions as audited"
tools\play.ps1 > <scratch dir>\play_N.log 2>&1      # background, about 10 minutes
```

- Leave an address out of the mark if its audit is still disputed.
- Test two or three merges together so a failure is cheap to bisect; test alone anything
  that changes drawing, palettes, fonts or the resource manager.
- While the play tests run: don't merge, don't rebuild the main tree, don't start another
  play test. Worktrees and read-only verifiers are fine.
- Read the result with a search for `^test_s|CHECK FAILED|AssertionError|GameCrashed`.
- A failure: bisect over the merges in that group, then debug with the asm. A crash writes
  `iwd2re-crash.txt` in the newest folder under `tests\e2e\out`.

## 6. Record

After every few groups: add an entry to `docs/session-log.md` (table of file, old functions
read, functions with a real difference, batch; then the notable fixes and open notes), run
`py -I tools/inventory.py --write`, and commit.

## Limits

- At most four agents at once.
- Never push. Never touch `C:\GOG Games`.
- The play tests don't check audio and show no monsters, combat, doors, stores or option
  screens, so most fixes in that code rest on the asm comparison alone. Say so in reports.
