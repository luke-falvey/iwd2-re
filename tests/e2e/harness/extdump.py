"""Write an iwd2re-stacks-raw.txt style dump of a running 32-bit iwd2-re.exe
from outside the process (for when the game's own dump thread cannot run).
Usage: python extdump.py <pid> <out.txt>"""
import ctypes
import struct
import sys
from ctypes import wintypes

k32 = ctypes.WinDLL("kernel32", use_last_error=True)

TH32CS_SNAPTHREAD = 0x4
TH32CS_SNAPMODULE = 0x8
TH32CS_SNAPMODULE32 = 0x10


class THREADENTRY32(ctypes.Structure):
    _fields_ = [("dwSize", wintypes.DWORD), ("cntUsage", wintypes.DWORD), ("th32ThreadID", wintypes.DWORD),
                ("th32OwnerProcessID", wintypes.DWORD), ("tpBasePri", wintypes.LONG),
                ("tpDeltaPri", wintypes.LONG), ("dwFlags", wintypes.DWORD)]


class MODULEENTRY32(ctypes.Structure):
    _fields_ = [("dwSize", wintypes.DWORD), ("th32ModuleID", wintypes.DWORD), ("th32ProcessID", wintypes.DWORD),
                ("GlblcntUsage", wintypes.DWORD), ("ProccntUsage", wintypes.DWORD),
                ("modBaseAddr", ctypes.c_void_p), ("modBaseSize", wintypes.DWORD), ("hModule", wintypes.HMODULE),
                ("szModule", ctypes.c_char * 256), ("szExePath", ctypes.c_char * 260)]


class FLOATING_SAVE_AREA(ctypes.Structure):
    _fields_ = [("ControlWord", wintypes.DWORD), ("StatusWord", wintypes.DWORD), ("TagWord", wintypes.DWORD),
                ("ErrorOffset", wintypes.DWORD), ("ErrorSelector", wintypes.DWORD), ("DataOffset", wintypes.DWORD),
                ("DataSelector", wintypes.DWORD), ("RegisterArea", ctypes.c_byte * 80),
                ("Cr0NpxState", wintypes.DWORD)]


class WOW64_CONTEXT(ctypes.Structure):
    _fields_ = [("ContextFlags", wintypes.DWORD), ("Dr0", wintypes.DWORD), ("Dr1", wintypes.DWORD),
                ("Dr2", wintypes.DWORD), ("Dr3", wintypes.DWORD), ("Dr6", wintypes.DWORD), ("Dr7", wintypes.DWORD),
                ("FloatSave", FLOATING_SAVE_AREA), ("SegGs", wintypes.DWORD), ("SegFs", wintypes.DWORD),
                ("SegEs", wintypes.DWORD), ("SegDs", wintypes.DWORD), ("Edi", wintypes.DWORD),
                ("Esi", wintypes.DWORD), ("Ebx", wintypes.DWORD), ("Edx", wintypes.DWORD), ("Ecx", wintypes.DWORD),
                ("Eax", wintypes.DWORD), ("Ebp", wintypes.DWORD), ("Eip", wintypes.DWORD),
                ("SegCs", wintypes.DWORD), ("EFlags", wintypes.DWORD), ("Esp", wintypes.DWORD),
                ("SegSs", wintypes.DWORD), ("ExtendedRegisters", ctypes.c_byte * 512)]


k32.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
k32.OpenThread.restype = wintypes.HANDLE
k32.OpenProcess.restype = wintypes.HANDLE
k32.ReadProcessMemory.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t,
                                  ctypes.POINTER(ctypes.c_size_t)]


def main():
    pid, out = int(sys.argv[1]), sys.argv[2]
    proc = k32.OpenProcess(0x0410, False, pid)  # QUERY_INFORMATION | VM_READ
    snap = k32.CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid)
    me = MODULEENTRY32()
    me.dwSize = ctypes.sizeof(me)
    base = size = None
    ok = k32.Module32First(snap, ctypes.byref(me))
    while ok:
        if me.szModule.lower().endswith(b".exe"):
            base, size = me.modBaseAddr, me.modBaseSize
            break
        ok = k32.Module32Next(snap, ctypes.byref(me))
    k32.CloseHandle(snap)
    lines = [f"image {base:08X} {base + size:08X}"]
    snap = k32.CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)
    te = THREADENTRY32()
    te.dwSize = ctypes.sizeof(te)
    ok = k32.Thread32First(snap, ctypes.byref(te))
    while ok:
        if te.th32OwnerProcessID == pid:
            th = k32.OpenThread(0x001A, False, te.th32ThreadID)  # SUSPEND_RESUME | GET_CONTEXT | QUERY
            if th:
                k32.Wow64SuspendThread(th)
                ctx = WOW64_CONTEXT()
                ctx.ContextFlags = 0x10007  # WOW64_CONTEXT_FULL
                if k32.Wow64GetThreadContext(th, ctypes.byref(ctx)):
                    lines.append(f"thread {te.th32ThreadID} eip {ctx.Eip:08X} esp {ctx.Esp:08X} ebp {ctx.Ebp:08X}")
                    buf = ctypes.create_string_buffer(0x10000)
                    got = ctypes.c_size_t(0)
                    k32.ReadProcessMemory(proc, ctx.Esp, buf, 0x10000, ctypes.byref(got))
                    if got.value == 0:
                        for n in (0x8000, 0x4000, 0x2000, 0x1000):
                            if k32.ReadProcessMemory(proc, ctx.Esp, buf, n, ctypes.byref(got)):
                                break
                    for (w,) in struct.iter_unpack("<I", buf.raw[:got.value - got.value % 4]):
                        if base <= w < base + size:
                            lines.append(f"  {w:08X}")
                k32.ResumeThread(th)
                k32.CloseHandle(th)
        ok = k32.Thread32Next(snap, ctypes.byref(te))
    k32.CloseHandle(snap)
    lines.append("== end ==")
    open(out, "w").write("\n".join(lines) + "\n")


main()
