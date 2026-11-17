---
name: port-64bit
description: Fix the pointer-size problems of one or more source files for the 64-bit (x64 and arm64) build without changing the 32-bit layout, or find and mark the structures that are read from or written to files. Use for macOS port milestone M5 when given a file list.
---

# 64-bit fixes, file by file

The game was a 32-bit program. The 32-bit build must keep its exact class layouts: the layout
asserts check about 5,900 annotated member offsets (`/* 0xOFFS */`) against the original.
The 64-bit build must still be correct. Every fix here satisfies both.

## Inputs

- The worktree and branch.
- The files you own.
- The task: **A** (pointer-size sites), **B** (file structures), or both.

Never edit other files, push, spawn subagents, or run anything in
`C:\Users\Luke\Projects\iwd2-re`.

## Task A: pointer-size sites

Get the list from the compiler. In the worktree's x64 build directory (`build-x64`, already
configured), build the one project at below-normal priority:

```
(Get-Process -Id $PID).PriorityClass = 'BelowNormal'; cmake --build build-x64 --config RelWithDebInfo --target iwd2-core -- /m:4 /nologo /v:minimal | Select-String "<your file names>"
```

The warnings that matter are:
- C4311 / C4302: pointer truncation;
- C4312: conversion to a larger pointer;
- C4244 / C4267, only where a pointer difference or `size_t` that can exceed 32 bits ends up
  in an `int`.

The known idioms, and their fixes:

| Idiom | Example | Fix |
|---|---|---|
| Object ids kept in a pointer list | `CTypedPtrList<CPtrList, LONG>`, `(LONG)list.GetNext(pos)` | The values were integers all along. Cast through `INT_PTR`: `static_cast<LONG>(reinterpret_cast<INT_PTR>(list.GetNext(pos)))`, and `reinterpret_cast<void*>(static_cast<INT_PTR>(id))` on the way in. Same 32-bit code |
| A pointer passed as a `DWORD` tag | `CGameArea*` as the sound channel's owner (`(DWORD)pArea`) | Change the parameter or member type to `UINT_PTR` / `DWORD_PTR`. Same size in 32-bit, so layout asserts still pass. Change both sides (the setter and the comparison) |
| A pointer stored in an `int` member or local | `CVidCell*` / `CVidImage*` kept in `int` | `INT_PTR` for the member (same 32-bit size), with casts at use. If the member is in a class with layout asserts, check the class's assert still passes |
| A pointer in a message parameter or `LPARAM`-like slot | message `DWORD` fields | `DWORD_PTR` where the field is in memory only. If the field is **serialized** (network message, save file), stop and report: that needs a design decision |
| Pointer difference into `int` | `int n = p - q;` | Fine when the arrays are small (game buffers); add `static_cast<int>` with `// NOTE: Port: fits` only if you checked the bound |

**Rules:**
1. Never change a type's 32-bit size. `INT_PTR`, `UINT_PTR`, `DWORD_PTR` and `LONG_PTR` are
   4 bytes in 32-bit and 8 in 64-bit; that is the point of them.
2. Never change behaviour: no added null checks, no "fixes".
3. Keep the annotations. Add `// NOTE: Port: <one line>` only where the reason isn't obvious
   from the type.
4. **A value that crosses into a file or network buffer is a design decision, not a site
   fix.** This includes save games, `.ARE`/`.CRE` structures and `CBaldurMessage` payloads.
   List it in the report and leave it alone.
5. After each file:
   - build x64 again and confirm its pointer warnings are gone;
   - build the default 32-bit configuration (`tools\check.ps1`, which runs the layout
     asserts). It must pass.

## Task B: structures read from or written to files

In 64-bit builds the 2-byte packing goes away for in-memory classes. Only structures whose
bytes are a file or network format keep `#pragma pack(2)` (or their exact format packing).

For each file you own:
1. Find structures whose memory is read from or written to a file or buffer as raw bytes:
   - `CFile::Read` / `Write` into a struct;
   - `memcpy` to or from a struct and a resource buffer;
   - pointers into resource data cast to a struct type (`(ARE_HEADER*)pData`);
   - structures inside save or network messages.
2. Mark each one, at its definition, with the project macro `IWD2_FILE_STRUCT_BEGIN(name,
   size)` / `IWD2_FILE_STRUCT_END(name, size)`. If the macro doesn't exist yet, ask the
   orchestrating session for it in your report; don't invent one. `size` is the format size
   in bytes as it is today in the 32-bit build. Get it from an existing `static_assert`, or
   compute it and say how.
3. A structure that holds a pointer **and** is file data is a finding: report it with the
   file and line.

## Report

- **Per file:**
  - warnings before → after;
  - idioms fixed and how many of each;
  - sites left alone, with the reason (rule 4);
  - file structures marked, with name, size and where the size came from.
- **Builds:** x64 `iwd2-core` result, and the default `tools\check.ps1` result.
- **Findings** that need a decision.
