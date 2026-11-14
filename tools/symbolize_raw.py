"""Resolve the addresses in an iwd2re-stacks-raw.txt against the rebuild's PDB.

  py -I tools/symbolize_raw.py <iwd2re-stacks-raw.txt> [<iwd2-re.exe>]

The raw file is written by the rebuild's dump thread without using the heap
(see src/support/CrashHandler.cpp). Each thread has its registers and the
stack words that point into the exe; most of them are return addresses, some
are stale. Uses dbghelp.dll through ctypes.
"""

from __future__ import annotations

import ctypes
import sys
from ctypes import wintypes
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO / "build" / "RelWithDebInfo" / "iwd2-re.exe"

MAX_NAME = 512


class SYMBOL_INFO(ctypes.Structure):
    _fields_ = [
        ("SizeOfStruct", wintypes.ULONG),
        ("TypeIndex", wintypes.ULONG),
        ("Reserved", ctypes.c_uint64 * 2),
        ("Index", wintypes.ULONG),
        ("Size", wintypes.ULONG),
        ("ModBase", ctypes.c_uint64),
        ("Flags", wintypes.ULONG),
        ("Value", ctypes.c_uint64),
        ("Address", ctypes.c_uint64),
        ("Register", wintypes.ULONG),
        ("Scope", wintypes.ULONG),
        ("Tag", wintypes.ULONG),
        ("NameLen", wintypes.ULONG),
        ("MaxNameLen", wintypes.ULONG),
        ("Name", ctypes.c_char * MAX_NAME),
    ]


class IMAGEHLP_LINE64(ctypes.Structure):
    _fields_ = [
        ("SizeOfStruct", wintypes.DWORD),
        ("Key", ctypes.c_void_p),
        ("LineNumber", wintypes.DWORD),
        ("FileName", ctypes.c_char_p),
        ("Address", ctypes.c_uint64),
    ]


def main() -> int:
    raw = Path(sys.argv[1])
    exe = Path(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_EXE

    dbghelp = ctypes.WinDLL("dbghelp.dll")
    dbghelp.SymSetOptions.argtypes = [wintypes.DWORD]
    dbghelp.SymInitialize.argtypes = [wintypes.HANDLE, ctypes.c_char_p, wintypes.BOOL]
    dbghelp.SymLoadModuleEx.argtypes = [wintypes.HANDLE, wintypes.HANDLE, ctypes.c_char_p, ctypes.c_char_p,
                                        ctypes.c_uint64, wintypes.DWORD, ctypes.c_void_p, wintypes.DWORD]
    dbghelp.SymLoadModuleEx.restype = ctypes.c_uint64
    dbghelp.SymFromAddr.argtypes = [wintypes.HANDLE, ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint64),
                                    ctypes.POINTER(SYMBOL_INFO)]
    dbghelp.SymGetLineFromAddr64.argtypes = [wintypes.HANDLE, ctypes.c_uint64, ctypes.POINTER(wintypes.DWORD),
                                             ctypes.POINTER(IMAGEHLP_LINE64)]

    lines = raw.read_text(errors="replace").splitlines()
    base = None
    for line in lines:
        if line.startswith("image "):
            base = int(line.split()[1], 16)
    if base is None:
        print("no image line in", raw)
        return 1

    handle = wintypes.HANDLE(0x1234)  # any unique value; no live process is needed
    dbghelp.SymSetOptions(0x2 | 0x10)  # undecorate names, load lines
    if not dbghelp.SymInitialize(handle, str(exe.parent).encode(), False):
        print("SymInitialize failed")
        return 1
    if not dbghelp.SymLoadModuleEx(handle, None, str(exe).encode(), None, base, 0, None, 0):
        print("SymLoadModuleEx failed for", exe)
        return 1

    def resolve(addr: int) -> str:
        info = SYMBOL_INFO()
        info.SizeOfStruct = ctypes.sizeof(SYMBOL_INFO) - MAX_NAME
        info.MaxNameLen = MAX_NAME - 1
        disp = ctypes.c_uint64(0)
        if not dbghelp.SymFromAddr(handle, addr, ctypes.byref(disp), ctypes.byref(info)):
            return "?"
        text = f"{info.Name.decode(errors='replace')}+0x{disp.value:X}"
        line = IMAGEHLP_LINE64()
        line.SizeOfStruct = ctypes.sizeof(IMAGEHLP_LINE64)
        ldisp = wintypes.DWORD(0)
        if dbghelp.SymGetLineFromAddr64(handle, addr, ctypes.byref(ldisp), ctypes.byref(line)):
            text += f"  ({Path(line.FileName.decode(errors='replace')).name}:{line.LineNumber})"
        return text

    for line in lines:
        parts = line.split()
        if line.startswith("thread "):
            eip = int(parts[3], 16)
            print(f"{line}   eip: {resolve(eip)}")
        elif len(parts) == 1 and len(parts[0]) == 8:
            print(f"  {parts[0]}  {resolve(int(parts[0], 16))}")
        else:
            print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
