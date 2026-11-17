#ifndef WIN32SHIM_MISC_H_
#define WIN32SHIM_MISC_H_
#include <win32shim/wintypes.h>
#include <win32shim/winbase_time.h>
#include <win32shim/winuser_window_input.h>

// Version information, languages, keyboard layouts and IME (imm32), OS version.
// dbghelp.h and tlhelp32.h are only used by src/support/CrashHandler.cpp,
// which the non-Windows build does not compile, so they declare nothing.

#define LANG_CHINESE 0x04
#define LANG_JAPANESE 0x11
#define LANG_KOREAN 0x12
#define SUBLANG_CHINESE_TRADITIONAL 0x01
#define SUBLANG_CHINESE_SIMPLIFIED 0x02
#define PRIMARYLANGID(lgid) ((WORD)(lgid)&0x3ff)
#define SUBLANGID(lgid) ((WORD)(lgid) >> 10)

#define VER_PLATFORM_WIN32s 0
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2

typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR szCSDVersion[128];
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;

typedef struct _OSVERSIONINFOEXA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR szCSDVersion[128];
    WORD wServicePackMajor;
    WORD wServicePackMinor;
    WORD wSuiteMask;
    BYTE wProductType;
    BYTE wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;

BOOL GetVersionExA(LPOSVERSIONINFOA lpVersionInformation);
LANGID GetSystemDefaultLangID();
DWORD GetFileVersionInfoSizeA(LPCSTR lptstrFilename, LPDWORD lpdwHandle);
BOOL GetFileVersionInfoA(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
BOOL VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen);


// IME (imm32).
typedef struct tagCANDIDATELIST {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    DWORD dwOffset[1];
} CANDIDATELIST, *PCANDIDATELIST, *LPCANDIDATELIST;

#define GCS_COMPSTR 0x0008
#define GCS_COMPATTR 0x0010
#define GCS_RESULTCLAUSE 0x1000
#define GCS_RESULTSTR 0x0800
#define IME_CMODE_NATIVE 0x0001
#define IME_CMODE_KATAKANA 0x0002
#define IME_CMODE_FULLSHAPE 0x0008
#define IMN_CLOSECANDIDATE 0x0004
#define IMN_OPENCANDIDATE 0x0005
#define IMN_CHANGECANDIDATE 0x0003
#define IMN_SETOPENSTATUS 0x0008

HIMC ImmGetContext(HWND hWnd);
BOOL ImmReleaseContext(HWND hWnd, HIMC hIMC);
BOOL ImmGetConversionStatus(HIMC hIMC, LPDWORD lpfdwConversion, LPDWORD lpfdwSentence);
BOOL ImmSetConversionStatus(HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence);
LONG ImmGetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen);
UINT ImmGetDescriptionA(HKL hKL, LPSTR lpszDescription, UINT uBufLen);
BOOL ImmGetOpenStatus(HIMC hIMC);
BOOL ImmSetOpenStatus(HIMC hIMC, BOOL fOpen);
DWORD ImmGetCandidateListA(HIMC hIMC, DWORD deIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen);

// String table lookup; `LoadString` is a macro for the A version in windows.h
// and the game relies on that (CString::LoadStringA).
int LoadStringA(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int cchBufferMax);
#define LoadString LoadStringA

#endif /* WIN32SHIM_MISC_H_ */
