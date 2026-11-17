// Modules, processes, memory, strings, environment, OS version, languages,
// IME, version resources, Winsock and COM: the parts of the Win32 subset that
// have no real counterpart here. Every "does not exist" answer below was
// checked against its call sites; see the notes at each function.

#include "kobject.h"
#include "platform.h"

#include <win32shim/com.h>
#include <win32shim/misc.h>
#include <win32shim/winbase_file.h>
#include <win32shim/winbase_process_memory.h>
#include <win32shim/winsock.h>

#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <mutex>
#include <string>

#define ERROR_MOD_NOT_FOUND_PORT 126L
#define ERROR_PROC_NOT_FOUND_PORT 127L
#define ERROR_INSUFFICIENT_BUFFER_PORT 122L
#define ERROR_ENVVAR_NOT_FOUND_PORT 203L
#define ERROR_RESOURCE_DATA_NOT_FOUND_PORT 1812L

namespace platform {

static std::string g_commandLine;

void SetCommandLine(int argc, char** argv)
{
    g_commandLine.clear();
    for (int i = 0; i < argc; i++) {
        if (i > 0) {
            g_commandLine += ' ';
        }
        g_commandLine += argv[i];
    }
}

static std::mutex g_stringsMutex;
static std::map<unsigned int, std::string>& StringTable()
{
    static std::map<unsigned int, std::string> table;
    return table;
}

void RegisterStringResource(unsigned int id, const char* text)
{
    std::lock_guard<std::mutex> lock(g_stringsMutex);
    StringTable()[id] = text;
}

} // namespace platform

using namespace platform;

// ---------------------------------------------------------------------------
// Modules

UINT GetACP()
{
    return 1252;
}

// Switching things off: Bink (movies), the OpenGL driver, dplayx, kernel32's
// GetDiskFreeSpaceExA. Every call site tests for NULL: CBaldurProjector (no
// movies), CChitin3d (message box, return FALSE; 3D is off by default),
// CNetwork::InitDirectPlay (installs the failing fakes), CInfGame and CResCache
// (CResCache keeps its cache size; CInfGame only needs it when the 4 GB check
// of GetDiskFreeSpaceA failed, which it does not here).
namespace platform {
// dplay_fake.cpp
void* CreateFakeDirectPlay();
void* FakeDirectPlayCreateAddress();
void* FakeDirectPlayLobbyCreateAddress();
}

static int s_dplayModule;

// Only "dplayx" exists, and it offers no connections (dplay_fake.cpp).
HMODULE LoadLibraryA(LPCSTR lpLibFileName)
{
    if (lpLibFileName != NULL && strcasecmp(lpLibFileName, "dplayx") == 0) {
        return reinterpret_cast<HMODULE>(&s_dplayModule);
    }
    SetLastError(ERROR_MOD_NOT_FOUND_PORT);
    return NULL;
}

BOOL FreeLibrary(HMODULE)
{
    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
}

FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    if (hModule == reinterpret_cast<HMODULE>(&s_dplayModule) && lpProcName != NULL) {
        if (strcmp(lpProcName, "DirectPlayCreate") == 0) {
            return reinterpret_cast<FARPROC>(platform::FakeDirectPlayCreateAddress());
        }
        if (strcmp(lpProcName, "DirectPlayLobbyCreateA") == 0) {
            return reinterpret_cast<FARPROC>(platform::FakeDirectPlayLobbyCreateAddress());
        }
    }
    SetLastError(ERROR_PROC_NOT_FOUND_PORT);
    return NULL;
}

HMODULE GetModuleHandleA(LPCSTR lpModuleName)
{
    // The executable itself (string resources, window classes) has a handle;
    // no system module does (CChitin looks for Kernel32 and copes with NULL).
    static int mainModule;
    if (lpModuleName == NULL) {
        return reinterpret_cast<HMODULE>(&mainModule);
    }
    return NULL;
}

DWORD GetModuleFileNameA(HMODULE, LPSTR lpFilename, DWORD nSize)
{
    char raw[4096];
    uint32_t size = sizeof(raw);
    if (nSize == 0 || _NSGetExecutablePath(raw, &size) != 0) {
        return 0;
    }
    char resolved[4096];
    const char* path = realpath(raw, resolved) != NULL ? resolved : raw;
    size_t len = strlen(path);
    if (len >= nSize) {
        memcpy(lpFilename, path, nSize - 1);
        lpFilename[nSize - 1] = 0;
        SetLastError(ERROR_INSUFFICIENT_BUFFER_PORT);
        return nSize;
    }
    memcpy(lpFilename, path, len + 1);
    return static_cast<DWORD>(len);
}

DWORD GetEnvironmentVariableA(LPCSTR lpName, LPSTR lpBuffer, DWORD nSize)
{
    const char* value = lpName != NULL ? getenv(lpName) : NULL;
    if (value == NULL) {
        SetLastError(ERROR_ENVVAR_NOT_FOUND_PORT);
        return 0;
    }
    size_t len = strlen(value);
    if (len >= nSize) {
        return static_cast<DWORD>(len + 1);
    }
    memcpy(lpBuffer, value, len + 1);
    return static_cast<DWORD>(len);
}

// main.cpp runs the registration tool reg32.exe and goes on when this fails.
BOOL CreateProcessA(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    return FALSE;
}

HANDLE OpenProcess(DWORD, BOOL, DWORD)
{
    return new KObject(K_PROCESS);
}

LPSTR GetCommandLineA()
{
    return const_cast<LPSTR>(g_commandLine.c_str());
}

void OutputDebugStringA(LPCSTR lpOutputString)
{
    if (lpOutputString != NULL) {
        fputs(lpOutputString, stderr);
    }
}

// ---------------------------------------------------------------------------
// Memory

HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes)
{
    // Moveable and fixed memory are the same thing here: the handle is the
    // address.
    void* p = (uFlags & GMEM_ZEROINIT) ? calloc(1, dwBytes != 0 ? dwBytes : 1) : malloc(dwBytes != 0 ? dwBytes : 1);
    if (p == NULL) {
        SetLastError(8); // ERROR_NOT_ENOUGH_MEMORY
    }
    return p;
}

LPVOID GlobalLock(HGLOBAL hMem)
{
    return hMem;
}

BOOL GlobalUnlock(HGLOBAL)
{
    // The lock count reaches zero: FALSE with NO_ERROR.
    SetLastError(NO_ERROR);
    return FALSE;
}

HGLOBAL GlobalFree(HGLOBAL hMem)
{
    free(hMem);
    return NULL;
}

void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer)
{
    uint64_t total = 0;
    size_t size = sizeof(total);
    sysctlbyname("hw.memsize", &total, &size, NULL, 0);

    const uint64_t cap = 0x7FFFFFFFull;
    uint64_t clamped = total < cap ? total : cap;

    lpBuffer->dwLength = sizeof(MEMORYSTATUS);
    lpBuffer->dwMemoryLoad = 50;
    lpBuffer->dwTotalPhys = clamped;
    lpBuffer->dwAvailPhys = clamped / 2;
    lpBuffer->dwTotalPageFile = clamped;
    lpBuffer->dwAvailPageFile = clamped / 2;
    lpBuffer->dwTotalVirtual = cap;
    lpBuffer->dwAvailVirtual = cap / 2;
}

BOOL IsBadReadPtr(const void*, UINT_PTR)
{
    return FALSE;
}

// ---------------------------------------------------------------------------
// lstr*

int lstrlenA(LPCSTR lpString)
{
    return lpString != NULL ? static_cast<int>(strlen(lpString)) : 0;
}

LPSTR lstrcpyA(LPSTR lpString1, LPCSTR lpString2)
{
    if (lpString1 == NULL || lpString2 == NULL) {
        return NULL;
    }
    return strcpy(lpString1, lpString2);
}

LPSTR lstrcpynA(LPSTR lpString1, LPCSTR lpString2, int iMaxLength)
{
    if (lpString1 == NULL || lpString2 == NULL || iMaxLength <= 0) {
        return NULL;
    }
    int i = 0;
    for (; i < iMaxLength - 1 && lpString2[i] != 0; i++) {
        lpString1[i] = lpString2[i];
    }
    lpString1[i] = 0;
    return lpString1;
}

LPSTR lstrcatA(LPSTR lpString1, LPCSTR lpString2)
{
    if (lpString1 == NULL || lpString2 == NULL) {
        return NULL;
    }
    return strcat(lpString1, lpString2);
}

// ---------------------------------------------------------------------------
// OS version, languages, version resources, string table

BOOL GetVersionExA(LPOSVERSIONINFOA lpVersionInformation)
{
    if (lpVersionInformation == NULL) {
        return FALSE;
    }
    DWORD size = lpVersionInformation->dwOSVersionInfoSize;
    if (size != sizeof(OSVERSIONINFOA) && size != sizeof(OSVERSIONINFOEXA)) {
        return FALSE;
    }
    // Windows XP, Service Pack 3: the newest system the game knows.
    lpVersionInformation->dwMajorVersion = 5;
    lpVersionInformation->dwMinorVersion = 1;
    lpVersionInformation->dwBuildNumber = 2600;
    lpVersionInformation->dwPlatformId = VER_PLATFORM_WIN32_NT;
    memset(lpVersionInformation->szCSDVersion, 0, sizeof(lpVersionInformation->szCSDVersion));
    strcpy(lpVersionInformation->szCSDVersion, "Service Pack 3");
    if (size == sizeof(OSVERSIONINFOEXA)) {
        OSVERSIONINFOEXA* ex = reinterpret_cast<OSVERSIONINFOEXA*>(lpVersionInformation);
        ex->wServicePackMajor = 3;
        ex->wServicePackMinor = 0;
        ex->wSuiteMask = 0x0100; // VER_SUITE_SINGLEUSERTS
        ex->wProductType = 1; // VER_NT_WORKSTATION
        ex->wReserved = 0;
    }
    return TRUE;
}

LANGID GetSystemDefaultLangID()
{
    return 0x0409; // US English
}

// No version resource: CChitin::GetGameVersionInfo skips everything when the
// size is 0 and leaves the build, version and game name strings as they are.
DWORD GetFileVersionInfoSizeA(LPCSTR, LPDWORD lpdwHandle)
{
    if (lpdwHandle != NULL) {
        *lpdwHandle = 0;
    }
    SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND_PORT);
    return 0;
}

BOOL GetFileVersionInfoA(LPCSTR, DWORD, DWORD, LPVOID)
{
    SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND_PORT);
    return FALSE;
}

BOOL VerQueryValueA(LPCVOID, LPCSTR, LPVOID* lplpBuffer, PUINT puLen)
{
    if (lplpBuffer != NULL) {
        *lplpBuffer = NULL;
    }
    if (puLen != NULL) {
        *puLen = 0;
    }
    return FALSE;
}

int LoadStringA(HINSTANCE, UINT uID, LPSTR lpBuffer, int cchBufferMax)
{
    if (lpBuffer == NULL || cchBufferMax <= 0) {
        return 0;
    }
    std::string text;
    {
        std::lock_guard<std::mutex> lock(g_stringsMutex);
        std::map<unsigned int, std::string>& table = StringTable();
        std::map<unsigned int, std::string>::iterator it = table.find(uID);
        if (it == table.end()) {
            lpBuffer[0] = 0;
            return 0;
        }
        text = it->second;
    }
    int n = static_cast<int>(text.size());
    if (n > cchBufferMax - 1) {
        n = cchBufferMax - 1;
    }
    memcpy(lpBuffer, text.data(), static_cast<size_t>(n));
    lpBuffer[n] = 0;
    return n;
}

// IME: there is none.
HIMC ImmGetContext(HWND)
{
    return NULL;
}
BOOL ImmReleaseContext(HWND, HIMC)
{
    return FALSE;
}
BOOL ImmGetConversionStatus(HIMC, LPDWORD, LPDWORD)
{
    return FALSE;
}
BOOL ImmSetConversionStatus(HIMC, DWORD, DWORD)
{
    return FALSE;
}
LONG ImmGetCompositionStringA(HIMC, DWORD, LPVOID, DWORD)
{
    return 0;
}
UINT ImmGetDescriptionA(HKL, LPSTR, UINT)
{
    return 0;
}
BOOL ImmGetOpenStatus(HIMC)
{
    return FALSE;
}
BOOL ImmSetOpenStatus(HIMC, BOOL)
{
    return FALSE;
}
DWORD ImmGetCandidateListA(HIMC, DWORD, LPCANDIDATELIST, DWORD)
{
    return 0;
}

// ---------------------------------------------------------------------------
// Winsock: the network is down. CGameSpy and CNetwork treat every failure as
// "no network".

static thread_local int t_wsaError = 0;

int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
    if (lpWSAData != NULL) {
        memset(lpWSAData, 0, sizeof(*lpWSAData));
        lpWSAData->wVersion = wVersionRequested;
        lpWSAData->wHighVersion = 0x0202;
        strcpy(lpWSAData->szDescription, "Winsock shim (no network)");
        strcpy(lpWSAData->szSystemStatus, "Not running");
        lpWSAData->iMaxSockets = 0;
        lpWSAData->iMaxUdpDg = 0;
    }
    return 0;
}

int WSACleanup()
{
    return 0;
}

int WSAGetLastError()
{
    return t_wsaError;
}

static int NetDown()
{
    t_wsaError = WSAENETDOWN;
    return SOCKET_ERROR;
}

HANDLE WSAAsyncGetHostByName(HWND, UINT, const char*, char*, int)
{
    t_wsaError = WSAENETDOWN;
    return NULL;
}
int WSACancelAsyncRequest(HANDLE)
{
    return NetDown();
}
int WSAAsyncSelect(SOCKET, HWND, UINT, LONG)
{
    return NetDown();
}
SOCKET socket(int, int, int)
{
    t_wsaError = WSAENETDOWN;
    return INVALID_SOCKET;
}
int closesocket(SOCKET)
{
    return NetDown();
}
int ioctlsocket(SOCKET, LONG, u_long*)
{
    return NetDown();
}
int connect(SOCKET, const struct sockaddr*, int)
{
    return NetDown();
}
int bind(SOCKET, const struct sockaddr*, int)
{
    return NetDown();
}
int recvfrom(SOCKET, char*, int, int, struct sockaddr*, int*)
{
    return NetDown();
}
int sendto(SOCKET, const char*, int, int, const struct sockaddr*, int)
{
    return NetDown();
}
int gethostname(char* name, int namelen)
{
    if (name == NULL || namelen < 10) {
        t_wsaError = 10014; // WSAEFAULT
        return SOCKET_ERROR;
    }
    strcpy(name, "localhost");
    return 0;
}

uint32_t inet_addr(const char* cp)
{
    if (cp == NULL) {
        return INADDR_NONE;
    }
    uint32_t parts[4];
    int count = 0;
    const char* p = cp;
    for (;;) {
        if (*p < '0' || *p > '9') {
            return INADDR_NONE;
        }
        uint32_t v = 0;
        while (*p >= '0' && *p <= '9') {
            v = v * 10 + static_cast<uint32_t>(*p - '0');
            if (v > 255) {
                return INADDR_NONE;
            }
            p++;
        }
        parts[count++] = v;
        if (*p == '.' && count < 4) {
            p++;
            continue;
        }
        break;
    }
    if (count != 4 || *p != 0) {
        return INADDR_NONE;
    }
    // Network byte order as a little-endian value: first part in the low byte.
    return parts[0] | (parts[1] << 8) | (parts[2] << 16) | (parts[3] << 24);
}

u_short htons(u_short hostshort)
{
    return static_cast<u_short>((hostshort << 8) | (hostshort >> 8));
}

u_short ntohs(u_short netshort)
{
    return htons(netshort);
}

// ---------------------------------------------------------------------------
// COM. CNetwork::CreateDirectPlayInterface reports failure when the class is
// not registered, which switches multiplayer off.

HRESULT CoInitialize(LPVOID)
{
    return S_OK;
}

void CoUninitialize()
{
}

// The only class is DirectPlay, the one that offers no connections.
HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN, DWORD, REFIID, LPVOID* ppv)
{
    static const GUID clsidDirectPlay = { 0xd1eb6d20, 0x8923, 0x11d0, { 0x9d, 0x97, 0x00, 0xa0, 0xc9, 0x0a, 0x43, 0xcb } };
    if (ppv != NULL && memcmp(&rclsid, &clsidDirectPlay, sizeof(GUID)) == 0) {
        *ppv = platform::CreateFakeDirectPlay();
        return S_OK;
    }
    if (ppv != NULL) {
        *ppv = NULL;
    }
    return REGDB_E_CLASSNOTREG;
}

// ---------------------------------------------------------------------------
// Not in the original: CrashHandler.cpp (dbghelp) is not built here.
void InstallCrashHandler();
void InstallCrashHandler()
{
}
