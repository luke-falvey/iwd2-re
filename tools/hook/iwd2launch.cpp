// Test-only launcher: starts a game exe suspended, loads iwd2hook.dll into
// it, then lets it run.
//
//   iwd2launch.exe <hook.dll> <game.exe>
//
// The game inherits this process's environment and current directory. The
// launcher prints "pid <n>" once the game is running, waits for it, and exits
// with the game's exit code. The game is in a kill-on-close job, so killing
// the launcher kills the game.

#include <windows.h>

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: iwd2launch <hook.dll> <game.exe>\n");
        return 2;
    }

    char dll[MAX_PATH];
    if (GetFullPathNameA(argv[1], sizeof(dll), dll, NULL) == 0 || GetFileAttributesA(dll) == INVALID_FILE_ATTRIBUTES) {
        fprintf(stderr, "no dll: %s\n", argv[1]);
        return 2;
    }

    HANDLE job = CreateJobObjectA(NULL, NULL);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = { 0 };
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));

    char cmd[MAX_PATH + 4];
    _snprintf(cmd, sizeof(cmd), "\"%s\"", argv[2]);
    cmd[sizeof(cmd) - 1] = '\0';

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(argv[2], cmd, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "CreateProcess failed: %lu\n", GetLastError());
        return 2;
    }
    AssignProcessToJobObject(job, pi.hProcess);

    // Both processes are 32-bit, so kernel32 is at the same address in each.
    SIZE_T len = strlen(dll) + 1;
    void* remote = VirtualAllocEx(pi.hProcess, NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    BOOL ok = remote != NULL && WriteProcessMemory(pi.hProcess, remote, dll, len, NULL);
    HANDLE thread = NULL;
    if (ok) {
        thread = CreateRemoteThread(pi.hProcess, NULL, 0,
            (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
            remote, 0, NULL);
    }
    DWORD loaded = 0;
    if (thread != NULL) {
        WaitForSingleObject(thread, 30000);
        GetExitCodeThread(thread, &loaded);
        CloseHandle(thread);
    }
    if (loaded == 0) {
        fprintf(stderr, "hook dll did not load (error %lu)\n", GetLastError());
        TerminateProcess(pi.hProcess, 2);
        return 2;
    }

    ResumeThread(pi.hThread);
    printf("pid %lu\n", pi.dwProcessId);
    fflush(stdout);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    return (int)code;
}
