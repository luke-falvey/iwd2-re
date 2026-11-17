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


// ---- winuser_window_input.h (50 stubs)
HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) { PENDING_LOG("CreateWindowExA"); return HWND(); }
ATOM RegisterClassExA(const WNDCLASSEXA* lpWndClass) { PENDING_LOG("RegisterClassExA"); return ATOM(); }
BOOL DestroyWindow(HWND hWnd) { PENDING_LOG("DestroyWindow"); return BOOL(); }
BOOL IsWindow(HWND hWnd) { PENDING_LOG("IsWindow"); return BOOL(); }
BOOL CloseWindow(HWND hWnd) { PENDING_LOG("CloseWindow"); return BOOL(); }
BOOL OpenIcon(HWND hWnd) { PENDING_LOG("OpenIcon"); return BOOL(); }
BOOL UpdateWindow(HWND hWnd) { PENDING_LOG("UpdateWindow"); return BOOL(); }
HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName) { PENDING_LOG("FindWindowA"); return HWND(); }
HWND GetFocus() { PENDING_LOG("GetFocus"); return HWND(); }
HMENU GetMenu(HWND hWnd) { PENDING_LOG("GetMenu"); return HMENU(); }
LONG GetWindowLongA(HWND hWnd, int nIndex) { PENDING_LOG("GetWindowLongA"); return LONG(); }
BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) { PENDING_LOG("SetWindowPos"); return BOOL(); }
BOOL GetWindowRect(HWND hWnd, LPRECT lpRect) { PENDING_LOG("GetWindowRect"); return BOOL(); }
BOOL GetClientRect(HWND hWnd, LPRECT lpRect) { PENDING_LOG("GetClientRect"); return BOOL(); }
BOOL GetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase) { PENDING_LOG("GetUpdateRect"); return BOOL(); }
BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint) { PENDING_LOG("ClientToScreen"); return BOOL(); }
BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint) { PENDING_LOG("ScreenToClient"); return BOOL(); }
LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { PENDING_LOG("DefWindowProcA"); return LRESULT(); }
LRESULT SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { PENDING_LOG("SendMessageA"); return LRESULT(); }
UINT RegisterWindowMessageA(LPCSTR lpString) { PENDING_LOG("RegisterWindowMessageA"); return UINT(); }
BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) { PENDING_LOG("PeekMessageA"); return BOOL(); }
BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) { PENDING_LOG("GetMessageA"); return BOOL(); }
BOOL TranslateMessage(const MSG* lpMsg) { PENDING_LOG("TranslateMessage"); return BOOL(); }
LRESULT DispatchMessageA(const MSG* lpMsg) { PENDING_LOG("DispatchMessageA"); return LRESULT(); }
void PostQuitMessage(int nExitCode) { PENDING_LOG("PostQuitMessage"); }
int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) { PENDING_LOG("MessageBoxA"); return int(); }
SHORT GetAsyncKeyState(int vKey) { PENDING_LOG("GetAsyncKeyState"); return SHORT(); }
BOOL GetCursorPos(LPPOINT lpPoint) { PENDING_LOG("GetCursorPos"); return BOOL(); }
HCURSOR SetCursor(HCURSOR hCursor) { PENDING_LOG("SetCursor"); return HCURSOR(); }
HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName) { PENDING_LOG("LoadCursorA"); return HCURSOR(); }
HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName) { PENDING_LOG("LoadIconA"); return HICON(); }
UINT GetDoubleClickTime() { PENDING_LOG("GetDoubleClickTime"); return UINT(); }
int GetSystemMetrics(int nIndex) { PENDING_LOG("GetSystemMetrics"); return int(); }
BOOL SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) { PENDING_LOG("SystemParametersInfoA"); return BOOL(); }
LONG ChangeDisplaySettingsA(DEVMODEA* lpDevMode, DWORD dwFlags) { PENDING_LOG("ChangeDisplaySettingsA"); return LONG(); }
BOOL EnumDisplaySettingsA(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode) { PENDING_LOG("EnumDisplaySettingsA"); return BOOL(); }
BOOL OpenClipboard(HWND hWndNewOwner) { PENDING_LOG("OpenClipboard"); return BOOL(); }
BOOL CloseClipboard() { PENDING_LOG("CloseClipboard"); return BOOL(); }
HANDLE GetClipboardData(UINT uFormat) { PENDING_LOG("GetClipboardData"); return HANDLE(); }
BOOL PostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { PENDING_LOG("PostMessageA"); return BOOL(); }
HDC BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint) { PENDING_LOG("BeginPaint"); return HDC(); }
BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* lpPaint) { PENDING_LOG("EndPaint"); return BOOL(); }
LONG SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong) { PENDING_LOG("SetWindowLongA"); return LONG(); }
BOOL AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle) { PENDING_LOG("AdjustWindowRectEx"); return BOOL(); }
SHORT GetKeyState(int nVirtKey) { PENDING_LOG("GetKeyState"); return SHORT(); }
HKL GetKeyboardLayout(DWORD idThread) { PENDING_LOG("GetKeyboardLayout"); return HKL(); }
HKL ActivateKeyboardLayout(HKL hkl, UINT Flags) { PENDING_LOG("ActivateKeyboardLayout"); return HKL(); }
HANDLE SetClipboardData(UINT uFormat, HANDLE hMem) { PENDING_LOG("SetClipboardData"); return HANDLE(); }
HDC GetDC(HWND hWnd) { PENDING_LOG("GetDC"); return HDC(); }
int ReleaseDC(HWND hWnd, HDC hDC) { PENDING_LOG("ReleaseDC"); return int(); }

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

// ---- ddraw_decl.h (1 stubs)
HRESULT DirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter) { PENDING_LOG("DirectDrawCreate"); return E_FAIL; }

// ---- dsound_decl.h (1 stubs)
HRESULT DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter) { PENDING_LOG("DirectSoundCreate"); return E_FAIL; }

// ---- wgl.h (0 stubs)
