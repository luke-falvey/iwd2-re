#ifndef WIN32SHIM_WINBASE_PROCESS_MEMORY_H_
#define WIN32SHIM_WINBASE_PROCESS_MEMORY_H_
#include <win32shim/wintypes.h>

// Modules, processes, global memory, lstr* string helpers, environment.

typedef INT_PTR(WINAPI* FARPROC)();
typedef INT_PTR(WINAPI* PROC)();

typedef struct _STARTUPINFOA {
    DWORD cb;
    LPSTR lpReserved;
    LPSTR lpDesktop;
    LPSTR lpTitle;
    DWORD dwX;
    DWORD dwY;
    DWORD dwXSize;
    DWORD dwYSize;
    DWORD dwXCountChars;
    DWORD dwYCountChars;
    DWORD dwFillAttribute;
    DWORD dwFlags;
    WORD wShowWindow;
    WORD cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;
typedef STARTUPINFOA STARTUPINFO;

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

#define NORMAL_PRIORITY_CLASS 0x00000020
#define PROCESS_ALL_ACCESS 0x001F0FFF
#define GMEM_MOVEABLE 0x0002
#define GMEM_SHARE 0x2000

UINT GetACP();
HMODULE LoadLibraryA(LPCSTR lpLibFileName);
BOOL FreeLibrary(HMODULE hLibModule);
FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
HMODULE GetModuleHandleA(LPCSTR lpModuleName);
DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
DWORD GetEnvironmentVariableA(LPCSTR lpName, LPSTR lpBuffer, DWORD nSize);
BOOL CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
HANDLE OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);

HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes);
LPVOID GlobalLock(HGLOBAL hMem);
BOOL GlobalUnlock(HGLOBAL hMem);
HGLOBAL GlobalFree(HGLOBAL hMem);
void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer);
BOOL IsBadReadPtr(const void* lp, UINT_PTR ucb);

int lstrlenA(LPCSTR lpString);
LPSTR lstrcpyA(LPSTR lpString1, LPCSTR lpString2);
LPSTR lstrcpynA(LPSTR lpString1, LPCSTR lpString2, int iMaxLength);
LPSTR lstrcatA(LPSTR lpString1, LPCSTR lpString2);

#endif /* WIN32SHIM_WINBASE_PROCESS_MEMORY_H_ */
