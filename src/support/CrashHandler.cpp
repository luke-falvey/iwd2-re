// NOTE: Not in the original. Writes a symbolized stack trace and a minidump
// when the game crashes, so test runs can report where it happened.
//
// Output goes to the folder in `IWD2RE_CRASH_DIR`, or the current directory:
//   iwd2re-crash.txt  exception code, address and stack with file:line
//   iwd2re-crash.dmp  minidump for a debugger

#include "CrashHandler.h"

#include <windows.h>

#include <dbghelp.h>
#include <tlhelp32.h>
#include <stdio.h>

#pragma comment(lib, "dbghelp.lib")

static char gCrashDir[MAX_PATH];

static void CrashPath(char* buffer, size_t size, const char* fileName)
{
    _snprintf(buffer, size, "%s\\%s", gCrashDir, fileName);
    buffer[size - 1] = '\0';
}

static volatile LONG gSymbolsReady = 0;

// Load symbols once. Called at startup by the dump thread, so a hang dump
// doesn't load modules while other threads may hold the loader lock.
static void EnsureSymbols()
{
    if (InterlockedCompareExchange(&gSymbolsReady, 1, 0) == 0) {
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
        SymInitialize(GetCurrentProcess(), NULL, TRUE);
    }
}

static void WriteStack(FILE* stream, CONTEXT* pContext, HANDLE hThread = GetCurrentThread())
{
    HANDLE hProcess = GetCurrentProcess();

    EnsureSymbols();

    CONTEXT context = *pContext;
    STACKFRAME64 frame = { 0 };
    frame.AddrPC.Offset = context.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode = AddrModeFlat;

    char symbolBuffer[sizeof(SYMBOL_INFO) + 256];
    for (int index = 0; index < 64; index++) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_I386, hProcess, hThread, &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
            break;
        }

        DWORD64 address = frame.AddrPC.Offset;
        if (address == 0) {
            break;
        }

        SYMBOL_INFO* pSymbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
        memset(symbolBuffer, 0, sizeof(symbolBuffer));
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = 255;

        DWORD64 displacement = 0;
        const char* name = SymFromAddr(hProcess, address, &displacement, pSymbol) ? pSymbol->Name : "?";

        IMAGEHLP_LINE64 line = { 0 };
        line.SizeOfStruct = sizeof(line);
        DWORD lineDisplacement = 0;
        if (SymGetLineFromAddr64(hProcess, address, &lineDisplacement, &line)) {
            fprintf(stream, "  #%02d %08llX %s+0x%llX  %s:%lu\n", index, address, name, displacement, line.FileName, line.LineNumber);
        } else {
            fprintf(stream, "  #%02d %08llX %s+0x%llX\n", index, address, name, displacement);
        }
    }

}

static LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo)
{
    char path[MAX_PATH];

    CrashPath(path, sizeof(path), "iwd2re-crash.txt");
    FILE* stream = fopen(path, "w");
    if (stream != NULL) {
        EXCEPTION_RECORD* pRecord = pExceptionInfo->ExceptionRecord;
        fprintf(stream, "exception %08lX at %p", pRecord->ExceptionCode, pRecord->ExceptionAddress);
        if (pRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && pRecord->NumberParameters >= 2) {
            fprintf(stream, " (%s %p)", pRecord->ExceptionInformation[0] ? "write" : "read", reinterpret_cast<void*>(pRecord->ExceptionInformation[1]));
        }
        fprintf(stream, "\nstack:\n");
        WriteStack(stream, pExceptionInfo->ContextRecord);
        fclose(stream);
    }

    CrashPath(path, sizeof(path), "iwd2re-crash.dmp");
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION info;
        info.ThreadId = GetCurrentThreadId();
        info.ExceptionPointers = pExceptionInfo;
        info.ClientPointers = FALSE;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &info, NULL, NULL);
        CloseHandle(hFile);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

static LONG WINAPI HeapCorruptionHandler(EXCEPTION_POINTERS* pExceptionInfo)
{
    if (pExceptionInfo->ExceptionRecord->ExceptionCode == 0xC0000374) {
        UnhandledExceptionHandler(pExceptionInfo);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

typedef LONG(NTAPI* NtGetNextThreadProc)(HANDLE, HANDLE, ACCESS_MASK, ULONG, ULONG, PHANDLE);

// NOTE: Not declared by the headers at the Windows version this project
// targets.
extern "C" __declspec(dllimport) DWORD WINAPI GetThreadId(HANDLE Thread);

// Looked up at startup: at dump time the loader lock may be held.
static NtGetNextThreadProc gNtGetNextThread = NULL;
static DWORD_PTR gImageBase = 0;
static DWORD_PTR gImageEnd = 0;

// A raw dump of every other thread made without the heap or the loader: if a
// stuck thread holds the heap lock or the loader lock, the toolhelp snapshot,
// fopen and the symbol lookups in DumpAllThreadStacks never return. Each thread gets its registers and the
// stack words that point into the exe (possible return addresses), to be
// resolved against the PDB afterwards. Threads are walked with
// NtGetNextThread, which needs no allocation.
static void DumpAllThreadStacksRaw()
{
    NtGetNextThreadProc pNtGetNextThread = gNtGetNextThread;

    char path[MAX_PATH];
    CrashPath(path, sizeof(path), "iwd2re-stacks-raw.txt");
    HANDLE hRaw = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hRaw == INVALID_HANDLE_VALUE) {
        return;
    }

    char line[128];
    DWORD written;
    DWORD_PTR imageBase = gImageBase;
    DWORD_PTR imageEnd = gImageEnd;

    wsprintfA(line, "image %08lX %08lX\r\n", static_cast<DWORD>(imageBase), static_cast<DWORD>(imageEnd));
    WriteFile(hRaw, line, lstrlenA(line), &written, NULL);

    DWORD selfId = GetCurrentThreadId();
    HANDLE hThread = NULL;
    while (pNtGetNextThread != NULL) {
        HANDLE hNext = NULL;
        LONG status = pNtGetNextThread(GetCurrentProcess(),
            hThread,
            THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
            0,
            0,
            &hNext);
        if (hThread != NULL) {
            CloseHandle(hThread);
        }
        if (status < 0) {
            break;
        }
        hThread = hNext;

        DWORD threadId = GetThreadId(hThread);
        if (threadId == selfId) {
            continue;
        }

        static CONTEXT context;
        static DWORD words[1024];
        SIZE_T bytesRead = 0;
        BOOL bContext = FALSE;

        if (SuspendThread(hThread) != (DWORD)-1) {
            memset(&context, 0, sizeof(context));
            context.ContextFlags = CONTEXT_FULL;
            bContext = GetThreadContext(hThread, &context);
            if (bContext) {
                // Try smaller reads when the stack ends within the buffer.
                for (SIZE_T size = sizeof(words); size >= 64 && bytesRead == 0; size /= 2) {
                    ReadProcessMemory(GetCurrentProcess(),
                        reinterpret_cast<LPCVOID>(static_cast<DWORD_PTR>(context.Esp)),
                        words,
                        size,
                        &bytesRead);
                }
            }
            ResumeThread(hThread);
        }

        if (!bContext) {
            continue;
        }

        wsprintfA(line, "thread %lu eip %08lX esp %08lX ebp %08lX\r\n", threadId, context.Eip, context.Esp, context.Ebp);
        WriteFile(hRaw, line, lstrlenA(line), &written, NULL);

        for (SIZE_T word = 0; word < bytesRead / sizeof(DWORD); word++) {
            if (words[word] >= imageBase && words[word] < imageEnd) {
                wsprintfA(line, "  %08lX\r\n", words[word]);
                WriteFile(hRaw, line, lstrlenA(line), &written, NULL);
            }
        }
    }

    WriteFile(hRaw, "== end ==\r\n", 11, &written, NULL);
    CloseHandle(hRaw);
}

// Writes the stack of every other thread to iwd2re-stacks.txt. Used to find
// where the game hangs. Each thread is suspended only long enough to copy its
// registers, so no thread stays frozen while this one allocates. Stuck threads
// don't move, so their stacks stay accurate.
static void DumpAllThreadStacks()
{
    const int kMaxThreads = 128;
    static DWORD threadIds[kMaxThreads];
    static CONTEXT contexts[kMaxThreads];
    int count = 0;

    DWORD processId = GetCurrentProcessId();
    DWORD selfId = GetCurrentThreadId();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 entry = { sizeof(entry) };
    if (hSnapshot != INVALID_HANDLE_VALUE && Thread32First(hSnapshot, &entry)) {
        do {
            if (entry.th32OwnerProcessID != processId || entry.th32ThreadID == selfId || count >= kMaxThreads) {
                continue;
            }

            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, entry.th32ThreadID);
            if (hThread == NULL) {
                continue;
            }

            if (SuspendThread(hThread) != (DWORD)-1) {
                memset(&contexts[count], 0, sizeof(CONTEXT));
                contexts[count].ContextFlags = CONTEXT_FULL;
                if (GetThreadContext(hThread, &contexts[count])) {
                    threadIds[count] = entry.th32ThreadID;
                    count++;
                }
                ResumeThread(hThread);
            }

            CloseHandle(hThread);
        } while (Thread32Next(hSnapshot, &entry));
    }

    if (hSnapshot != INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
    }

    char path[MAX_PATH];
    CrashPath(path, sizeof(path), "iwd2re-stacks.txt");
    FILE* stream = fopen(path, "w");
    if (stream == NULL) {
        return;
    }

    for (int index = 0; index < count; index++) {
        HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, threadIds[index]);
        fprintf(stream, "thread %lu:\n", threadIds[index]);
        WriteStack(stream, &contexts[index], hThread != NULL ? hThread : GetCurrentThread());
        fprintf(stream, "\n");
        fflush(stream);
        if (hThread != NULL) {
            CloseHandle(hThread);
        }
    }

    // Tells readers the file is complete.
    fprintf(stream, "== end ==\n");
    fclose(stream);
}

// Waits on the named event "iwd2re-dump-<pid>". Test tools set it when the
// game looks stuck.
static DWORD WINAPI DumpRequestThread(LPVOID lpParameter)
{
    HANDLE hEvent = reinterpret_cast<HANDLE>(lpParameter);
    EnsureSymbols();
    while (WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0) {
        DumpAllThreadStacksRaw();
        DumpAllThreadStacks();
    }
    return 0;
}

void InstallCrashHandler()
{
    DWORD length = GetEnvironmentVariableA("IWD2RE_CRASH_DIR", gCrashDir, sizeof(gCrashDir));
    if (length == 0 || length >= sizeof(gCrashDir)) {
        GetCurrentDirectoryA(sizeof(gCrashDir), gCrashDir);
    }

    SetUnhandledExceptionFilter(UnhandledExceptionHandler);

    gNtGetNextThread = reinterpret_cast<NtGetNextThreadProc>(
        GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtGetNextThread"));
    gImageBase = reinterpret_cast<DWORD_PTR>(GetModuleHandleA(NULL));
    IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(gImageBase);
    IMAGE_NT_HEADERS* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(gImageBase + dosHeader->e_lfanew);
    gImageEnd = gImageBase + ntHeaders->OptionalHeader.SizeOfImage;

    // Heap corruption (0xC0000374) is raised fail-fast and never reaches the
    // unhandled exception filter, so catch it first-chance.
    AddVectoredExceptionHandler(1, HeapCorruptionHandler);

    char eventName[64];
    _snprintf(eventName, sizeof(eventName), "iwd2re-dump-%lu", GetCurrentProcessId());
    eventName[sizeof(eventName) - 1] = '\0';
    HANDLE hEvent = CreateEventA(NULL, FALSE, FALSE, eventName);
    if (hEvent != NULL) {
        CreateThread(NULL, 0, DumpRequestThread, hEvent, 0, NULL);
    }
}
