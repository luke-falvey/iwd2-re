#ifndef IWD2RE_COMPAT_AFXCOMPAT_H_
#define IWD2RE_COMPAT_AFXCOMPAT_H_

// The MFC names the game uses, without MFC (build option IWD2RE_NO_MFC).
//
// Each class has the same size and member order as the MFC class it stands
// in for, on 32-bit Windows, so the game's own classes keep their layouts.
// The classes are written from MFC's documented behaviour; tests/unit/
// test_compat_*.cpp run against both builds to check that they agree.

#include <windows.h>

#include <intsafe.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>

// windows.h turns this name into a call of GetTickCount.
#undef GetCurrentTime

// MFC lays its classes out with 4-byte packing.
#pragma pack(push, 4)

#include "compat/cstring.h"
#include "compat/geometry.h"
#include "compat/object.h"

#include "compat/collections.h"

#include "compat/cfile.h"
#include "compat/sync.h"

// A window handle holder. The game only attaches the handle of the window
// it created itself and reads it back; the padding keeps MFC's size and the
// position of `m_hWnd`.
class CWnd : public CObject {
    BYTE m_reserved1[0x1C];

public:
    HWND m_hWnd;

private:
    BYTE m_reserved2[0x5C];

public:
    CWnd()
    {
        memset(m_reserved1, 0, sizeof(m_reserved1));
        m_hWnd = NULL;
        memset(m_reserved2, 0, sizeof(m_reserved2));
    }

    // A window that is still attached is destroyed with the object.
    ~CWnd() override
    {
        if (m_hWnd != NULL) {
            ::DestroyWindow(m_hWnd);
        }
    }

    operator HWND() const { return m_hWnd; }
    HWND GetSafeHwnd() const { return m_hWnd; }

    BOOL Attach(HWND hWndNew)
    {
        if (hWndNew == NULL) {
            return FALSE;
        }
        m_hWnd = hWndNew;
        return TRUE;
    }

    HWND Detach()
    {
        HWND hWnd = m_hWnd;
        m_hWnd = NULL;
        return hWnd;
    }

    // Convert between window (client area) and screen coordinates.
    void ClientToScreen(POINT* lpPoint) const { ::ClientToScreen(m_hWnd, lpPoint); }
    void ScreenToClient(POINT* lpPoint) const { ::ScreenToClient(m_hWnd, lpPoint); }
    void ClientToScreen(RECT* lpRect) const
    {
        ::ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(lpRect));
        ::ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(lpRect) + 1);
    }
    void ScreenToClient(RECT* lpRect) const
    {
        ::ScreenToClient(m_hWnd, reinterpret_cast<POINT*>(lpRect));
        ::ScreenToClient(m_hWnd, reinterpret_cast<POINT*>(lpRect) + 1);
    }
};

#pragma pack(pop)

// The sizes of the MFC classes on 32-bit Windows.
#if defined(_M_IX86)
static_assert(sizeof(CString) == 0x4, "CString must be a single pointer");
static_assert(sizeof(CPoint) == 0x8 && sizeof(CSize) == 0x8 && sizeof(CRect) == 0x10, "geometry types add no data");
static_assert(sizeof(CObject) == 0x4, "CObject is a virtual table pointer");
static_assert(sizeof(CPtrList) == 0x1C && sizeof(CObList) == 0x1C && sizeof(CStringList) == 0x1C, "list layout");
static_assert(sizeof(CPtrArray) == 0x14 && sizeof(CObArray) == 0x14 && sizeof(CStringArray) == 0x14 && sizeof(CDWordArray) == 0x14, "array layout");
static_assert(sizeof(CMapWordToPtr) == 0x1C && sizeof(CMapStringToString) == 0x1C, "map layout");
static_assert(sizeof(CTime) == 0x8, "CTime is a 64-bit time");
static_assert(sizeof(CException) == 0x8 && sizeof(CFileException) == 0x14, "exception layout");
static_assert(sizeof(CFile) == 0x14 && sizeof(CFileStatus) == 0x128 && sizeof(CFileFind) == 0x1C, "file class layout");
static_assert(sizeof(CSyncObject) == 0x8 && sizeof(CCriticalSection) == 0x20 && sizeof(CSingleLock) == 0xC, "lock layout");
static_assert(sizeof(CWnd) == 0x80 && offsetof(CWnd, m_hWnd) == 0x20, "CWnd layout");
#endif

// Module whose resources `CString::LoadString` reads. Defaults to the exe.
HINSTANCE AfxGetResourceHandle();
void AfxSetResourceHandle(HINSTANCE hInstResource);

// Current MFC only checks for a null pointer here.
inline BOOL AfxIsValidAddress(const void* lp, UINT_PTR nBytes, BOOL bReadWrite = TRUE)
{
    (void)nBytes;
    (void)bReadWrite;
    return lp != NULL;
}

#endif /* IWD2RE_COMPAT_AFXCOMPAT_H_ */
