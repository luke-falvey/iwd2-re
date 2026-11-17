// Stubs for the parts of the Win32 subset that the window/input, video and
// sound tasks implement. Each stub logs its name to stderr the first time it
// is called ("[pending] Name") and returns a failure or zero value. When a
// group is implemented, delete its group (heading to the next heading).

#include <win32shim/ddraw_decl.h>
#include <win32shim/dsound_decl.h>
#include <win32shim/wgl.h>
#include <win32shim/wingdi.h>
#include <win32shim/winuser_window_input.h>

#include <atomic>
#include <stdio.h>

#define PENDING_LOG(name)                                            \
    do {                                                             \
        static std::atomic<bool> once(false);                        \
        if (!once.exchange(true)) fprintf(stderr, "[pending] %s\n", name); \
    } while (0)


// ---- wingdi.h (17 stubs)
HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h) { PENDING_LOG("SelectObject"); return HGDIOBJ(); }
BOOL DeleteObject(HGDIOBJ ho) { PENDING_LOG("DeleteObject"); return BOOL(); }
HGDIOBJ GetStockObject(int i) { PENDING_LOG("GetStockObject"); return HGDIOBJ(); }
HBRUSH CreateSolidBrush(COLORREF color) { PENDING_LOG("CreateSolidBrush"); return HBRUSH(); }
HFONT CreateFontA(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName) { PENDING_LOG("CreateFontA"); return HFONT(); }
HPEN CreatePen(int iStyle, int cWidth, COLORREF color) { PENDING_LOG("CreatePen"); return HPEN(); }
BOOL GetTextMetricsA(HDC hdc, LPTEXTMETRICA lptm) { PENDING_LOG("GetTextMetricsA"); return BOOL(); }
int SetBkMode(HDC hdc, int mode) { PENDING_LOG("SetBkMode"); return int(); }
COLORREF SetTextColor(HDC hdc, COLORREF color) { PENDING_LOG("SetTextColor"); return COLORREF(); }
BOOL ExtTextOutA(HDC hdc, int x, int y, UINT options, const RECT* lprect, LPCSTR lpString, UINT c, const INT* lpDx) { PENDING_LOG("ExtTextOutA"); return BOOL(); }
BOOL TextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c) { PENDING_LOG("TextOutA"); return BOOL(); }
BOOL GetTextExtentPoint32A(HDC hdc, LPCSTR lpString, int c, LPSIZE lpsz) { PENDING_LOG("GetTextExtentPoint32A"); return BOOL(); }
BOOL Polygon(HDC hdc, const POINT* apt, int cpt) { PENDING_LOG("Polygon"); return BOOL(); }
int GetDeviceCaps(HDC hdc, int index) { PENDING_LOG("GetDeviceCaps"); return int(); }
int ChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd) { PENDING_LOG("ChoosePixelFormat"); return int(); }
BOOL SetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR* ppfd) { PENDING_LOG("SetPixelFormat"); return BOOL(); }
BOOL SwapBuffers(HDC hdc) { PENDING_LOG("SwapBuffers"); return BOOL(); }

// ---- wgl.h (0 stubs)
