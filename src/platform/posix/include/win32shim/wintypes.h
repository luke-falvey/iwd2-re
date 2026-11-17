#ifndef WIN32SHIM_WINTYPES_H_
#define WIN32SHIM_WINTYPES_H_

// Basic Windows types, calling conventions and trivial macros for non-Windows
// builds. `long` is never used: it is 64 bits wide on macOS, 32 on Windows.

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#ifndef _WIN32
// Some game code and headers test for these.
#define _M_IX86_PORT_NOT_DEFINED 1
#endif

#define __stdcall
#define __cdecl
#define __fastcall
#define __thiscall
#define __forceinline inline __attribute__((always_inline))
#define __int8 int8_t
#define __int16 int16_t
#define __int32 int32_t
#define __int64 int64_t
#define WINAPI
#define WINAPIV
#define CALLBACK
#define APIENTRY
#define PASCAL
#define CDECL
#define FAR
#define NEAR
#define CONST const
#define WINAPI_INLINE inline
#define STDMETHODCALLTYPE
#define STDMETHODVCALLTYPE
#define STDAPICALLTYPE
#define STDAPIVCALLTYPE
#define DECLSPEC_NOVTABLE
#define DECLSPEC_IMPORT
#define DECLSPEC_SELECTANY
#define DLLIMPORT
#define DLLEXPORT
#define UNALIGNED
#define __RPC_FAR
#define _declspec __declspec

// `__declspec(...)` is accepted by -fdeclspec; the Windows-only attributes are
// ignored with a warning that CMake silences.

typedef void VOID;
typedef void* PVOID;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef char CHAR;
typedef char* PCHAR;
typedef char* PSTR;
typedef char* LPSTR;
typedef const char* PCSTR;
typedef const char* LPCSTR;
typedef char TCHAR;
typedef char* LPTSTR;
typedef const char* LPCTSTR;
typedef char* PTSTR;
typedef const char* PCTSTR;
typedef uint16_t WCHAR;
typedef WCHAR* PWSTR;
typedef WCHAR* LPWSTR;
typedef const WCHAR* PCWSTR;
typedef const WCHAR* LPCWSTR;
typedef WCHAR OLECHAR;
typedef OLECHAR* LPOLESTR;
typedef const OLECHAR* LPCOLESTR;
typedef int8_t INT8;
typedef int16_t SHORT;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;
typedef uint8_t UINT8;
typedef uint16_t USHORT;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uint8_t BYTE;
typedef uint8_t UCHAR;
typedef uint8_t* PUCHAR;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t ULONG;
typedef int32_t LONG;
typedef uint32_t UINT;
typedef int32_t INT;
typedef int32_t BOOL;
typedef uint8_t BOOLEAN;
typedef float FLOAT;
typedef double DOUBLE;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef int64_t LONG64;
typedef uint64_t ULONG64;
typedef uint64_t DWORDLONG;
typedef uint64_t DWORD64;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef uintptr_t DWORD_PTR;
typedef intptr_t SSIZE_T;
typedef size_t SIZE_T;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef int32_t HRESULT;
typedef WORD ATOM;
typedef WORD LANGID;
typedef DWORD LCID;
typedef DWORD COLORREF;
typedef COLORREF* LPCOLORREF;
typedef DWORD* LPDWORD;
typedef DWORD* PDWORD;
typedef WORD* LPWORD;
typedef WORD* PWORD;
typedef BYTE* LPBYTE;
typedef BYTE* PBYTE;
typedef BOOL* LPBOOL;
typedef BOOL* PBOOL;
typedef INT* LPINT;
typedef INT* PINT;
typedef UINT* PUINT;
typedef LONG* LPLONG;
typedef LONG* PLONG;
typedef ULONG* PULONG;
typedef float* PFLOAT;
typedef LONGLONG* PLONGLONG;
typedef ULONGLONG* PULONGLONG;
typedef ULONG_PTR* PULONG_PTR;
typedef const BYTE* LPCBYTE;
typedef BYTE BOOLEAN_T;

typedef void* HANDLE;
typedef HANDLE* PHANDLE;
typedef HANDLE* LPHANDLE;

#define DECLARE_HANDLE(name) \
    struct name##__ { int unused; }; \
    typedef struct name##__* name

DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HPEN);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HPALETTE);
DECLARE_HANDLE(HRGN);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HCURSOR);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HKEY);
DECLARE_HANDLE(HGLRC);
DECLARE_HANDLE(HIMC);
DECLARE_HANDLE(HKL);
DECLARE_HANDLE(HMONITOR);
DECLARE_HANDLE(HHOOK);
DECLARE_HANDLE(HACCEL);
DECLARE_HANDLE(HMIDIOUT);
DECLARE_HANDLE(HWAVEOUT);
DECLARE_HANDLE(HMIXEROBJ);
DECLARE_HANDLE(HMIXER);
typedef HKEY* PHKEY;
typedef HICON HICON_T;
typedef void* HGDIOBJ;
typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef HANDLE HRSRC;

typedef struct _GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} GUID;
typedef GUID IID;
typedef GUID CLSID;
typedef const GUID& REFGUID;
typedef const IID& REFIID;
typedef const CLSID& REFCLSID;
typedef GUID* LPGUID;
typedef const GUID* LPCGUID;
typedef IID* LPIID;
typedef CLSID* LPCLSID;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#define MAX_PATH 260
#define MAXDWORD 0xFFFFFFFFu
#define MAXLONG 0x7FFFFFFF
#define MAXWORD 0xFFFF
#define MAXBYTE 0xFF
#define MAXSHORT 0x7FFF
#define MININT (-2147483647 - 1)
#define MAXINT 2147483647
#define INFINITE 0xFFFFFFFFu
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define INVALID_FILE_SIZE 0xFFFFFFFFu
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

#define MAKEWORD(a, b) ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#define MAKELPARAM(l, h) ((LPARAM)(DWORD)MAKELONG(l, h))
#define MAKEWPARAM(l, h) ((WPARAM)(DWORD)MAKELONG(l, h))
#define MAKELRESULT(l, h) ((LRESULT)(DWORD)MAKELONG(l, h))

#define RGB(r, g, b) ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define PALETTERGB(r, g, b) (0x02000000 | RGB(r, g, b))
#define GetRValue(rgb) (LOBYTE(rgb))
#define GetGValue(rgb) (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) (LOBYTE((rgb) >> 16))

#define MAKEINTRESOURCEA(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#define MAKEINTRESOURCE MAKEINTRESOURCEA
#define IS_INTRESOURCE(p) ((((ULONG_PTR)(p)) >> 16) == 0)

#define UNREFERENCED_PARAMETER(P) ((void)(P))
#define UNUSED_ALWAYS(x) ((void)(x))
#define ZeroMemory(d, l) memset((d), 0, (l))
#define FillMemory(d, l, f) memset((d), (f), (l))
#define CopyMemory(d, s, l) memcpy((d), (s), (l))
#define MoveMemory(d, s, l) memmove((d), (s), (l))
#define SecureZeroMemory(d, l) memset((d), 0, (l))
#define RtlZeroMemory ZeroMemory
#define RtlCopyMemory CopyMemory
#define OUT
#define IN
#define OPTIONAL
#define IN_OPT
#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address) - offsetof(type, field)))
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
#define _countof(a) (sizeof(a) / sizeof((a)[0]))
#define __max(a, b) (((a) > (b)) ? (a) : (b))
#define __min(a, b) (((a) < (b)) ? (a) : (b))

// HRESULT helpers.
#define S_OK ((HRESULT)0)
#define S_FALSE ((HRESULT)1)
#define E_FAIL ((HRESULT)0x80004005)
#define E_NOTIMPL ((HRESULT)0x80004001)
#define E_NOINTERFACE ((HRESULT)0x80004002)
#define E_POINTER ((HRESULT)0x80004003)
#define E_ABORT ((HRESULT)0x80004004)
#define E_UNEXPECTED ((HRESULT)0x8000FFFF)
#define E_OUTOFMEMORY ((HRESULT)0x8007000E)
#define E_INVALIDARG ((HRESULT)0x80070057)
#define E_ACCESSDENIED ((HRESULT)0x80070005)
#define NOERROR 0
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define MAKE_HRESULT(sev, fac, code) ((HRESULT)(((uint32_t)(sev) << 31) | ((uint32_t)(fac) << 16) | ((uint32_t)(code))))
#define HRESULT_CODE(hr) ((hr)&0xFFFF)
#define FACILITY_WIN32 7
#define HRESULT_FROM_WIN32(x) ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT)(((x)&0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)))

// GUID helpers. DEFINE_GUID declares only, unless INITGUID is set.
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#define EXTERN_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    extern const GUID name
#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    extern const GUID name; \
    const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    extern const GUID name
#endif
#define __uuidof(x) IID_##x

inline bool IsEqualGUID(REFGUID a, REFGUID b) { return memcmp(&a, &b, sizeof(GUID)) == 0; }
inline bool operator==(const GUID& a, const GUID& b) { return IsEqualGUID(a, b); }
inline bool operator!=(const GUID& a, const GUID& b) { return !IsEqualGUID(a, b); }
#define IsEqualIID(a, b) IsEqualGUID(a, b)
#define IsEqualCLSID(a, b) IsEqualGUID(a, b)

// min and max are macros in windows.h unless NOMINMAX is defined. Function
// templates stand in for them (macros would break libc++), mixed types allowed.
#ifndef NOMINMAX
#include <type_traits>
template <class A, class B>
inline constexpr typename std::common_type<A, B>::type max(A a, B b)
{
    return (a > b) ? a : b;
}
template <class A, class B>
inline constexpr typename std::common_type<A, B>::type min(A a, B b)
{
    return (a < b) ? a : b;
}
#endif

// intsafe.h limits.
#define SHORT_MAX 32767
#define WORD_MAX 0xffff
#define DWORD_MAX 0xffffffffUL


#endif /* WIN32SHIM_WINTYPES_H_ */
