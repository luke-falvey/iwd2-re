#ifndef WIN32SHIM_WINUSER_WINDOW_INPUT_H_
#define WIN32SHIM_WINUSER_WINDOW_INPUT_H_
#include <win32shim/wintypes.h>
#include <win32shim/wingdi.h>

// Windows, messages, input, cursors, clipboard, display settings, dialogs.

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
} MSG, *PMSG, *LPMSG;

typedef struct tagWNDCLASSEXA {
    UINT cbSize;
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
    HICON hIconSm;
} WNDCLASSEXA, *PWNDCLASSEXA, *LPWNDCLASSEXA;

typedef struct tagPAINTSTRUCT {
    HDC hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *LPPAINTSTRUCT;

#define CCHDEVICENAME 32
#define CCHFORMNAME 32
typedef struct _devicemodeA {
    BYTE dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    union {
        struct {
            short dmOrientation;
            short dmPaperSize;
            short dmPaperLength;
            short dmPaperWidth;
            short dmScale;
            short dmCopies;
            short dmDefaultSource;
            short dmPrintQuality;
        };
        struct {
            POINT dmPosition;
            DWORD dmDisplayOrientation;
            DWORD dmDisplayFixedOutput;
        };
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE dmFormName[CCHFORMNAME];
    WORD dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    union {
        DWORD dmDisplayFlags;
        DWORD dmNup;
    };
    DWORD dmDisplayFrequency;
} DEVMODEA, *PDEVMODEA, *LPDEVMODEA;

// Window messages.
#define WM_DESTROY 0x0002
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_QUERYENDSESSION 0x0011
#define WM_ERASEBKGND 0x0014
#define WM_ACTIVATEAPP 0x001C
#define WM_SETCURSOR 0x0020
#define WM_KEYDOWN 0x0100
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_SYSCHAR 0x0106
#define WM_KILLFOCUS 0x0008
#define WM_INPUTLANGCHANGE 0x0051
#define WM_DISPLAYCHANGE 0x007E
#define WM_SYSCOMMAND 0x0112
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_IME_SETCONTEXT 0x0281
#define WM_IME_NOTIFY 0x0282
#define WM_IME_CONTROL 0x0283
#define WM_IME_COMPOSITIONFULL 0x0284
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION 0x010E
#define WM_IME_COMPOSITION 0x010F
#define WM_IME_KEYDOWN 0x0290

#define SC_SCREENSAVE 0xF140
#define SC_MONITORPOWER 0xF170
#define HTCLIENT 1
#define WHEEL_DELTA 120

// Window styles.
#define WS_POPUP 0x80000000u
#define WS_VISIBLE 0x10000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_CLIPCHILDREN 0x02000000L
#define WS_CAPTION 0x00C00000L
#define WS_SYSMENU 0x00080000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_EX_TOPMOST 0x00000008L
#define GWL_STYLE (-16)
#define GWL_EXSTYLE (-20)
#define HWND_TOP ((HWND)(LONG_PTR)0)
#define HWND_MESSAGE ((HWND)(LONG_PTR)-3)
#define HWND_NOTOPMOST ((HWND)(LONG_PTR)-2)
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_NOACTIVATE 0x0010

// System metrics and parameters.
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define SM_CXFULLSCREEN 16
#define SM_CYFULLSCREEN 17
#define SM_SWAPBUTTON 23
#define SM_CXDOUBLECLK 36
#define SM_CYDOUBLECLK 37
#define SPI_GETKEYBOARDDELAY 22
#define SPI_GETKEYBOARDSPEED 10
#define SPI_GETSCREENSAVEACTIVE 16
#define SPI_SETSCREENSAVEACTIVE 17
#define SPI_GETWORKAREA 48
#define SPI_GETWHEELSCROLLLINES 104

// Display settings.
#define CDS_FULLSCREEN 0x00000004
#define CDS_RESET 0x40000000
#define DISP_CHANGE_SUCCESSFUL 0

// Dialogs.
#define MB_ICONERROR 0x00000010L
#define MB_SYSTEMMODAL 0x00001000L
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

#define IDC_ARROW MAKEINTRESOURCEA(32512)

// Virtual keys.
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_MBUTTON 0x04
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_CAPITAL 0x14
#define VK_ESCAPE 0x1B
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_PRINT 0x2A
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_NUMLOCK 0x90
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE
#define VK_PROCESSKEY 0xE5

// Windows and messages.
HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
ATOM RegisterClassExA(const WNDCLASSEXA* lpWndClass);
BOOL DestroyWindow(HWND hWnd);
BOOL IsWindow(HWND hWnd);
BOOL CloseWindow(HWND hWnd);
BOOL OpenIcon(HWND hWnd);
BOOL UpdateWindow(HWND hWnd);
HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
HWND GetFocus();
HMENU GetMenu(HWND hWnd);
LONG GetWindowLongA(HWND hWnd, int nIndex);
BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
BOOL GetWindowRect(HWND hWnd, LPRECT lpRect);
BOOL GetClientRect(HWND hWnd, LPRECT lpRect);
BOOL GetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase);
BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint);
BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint);
LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
UINT RegisterWindowMessageA(LPCSTR lpString);
BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL TranslateMessage(const MSG* lpMsg);
LRESULT DispatchMessageA(const MSG* lpMsg);
void PostQuitMessage(int nExitCode);
int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

// Input, cursor and icons.
SHORT GetAsyncKeyState(int vKey);
BOOL GetCursorPos(LPPOINT lpPoint);
HCURSOR SetCursor(HCURSOR hCursor);
HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);
HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName);
UINT GetDoubleClickTime();

// System information and display.
int GetSystemMetrics(int nIndex);
BOOL SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
LONG ChangeDisplaySettingsA(DEVMODEA* lpDevMode, DWORD dwFlags);
BOOL EnumDisplaySettingsA(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode);

// Rectangles.
BOOL SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom);
BOOL SetRectEmpty(LPRECT lprc);
BOOL CopyRect(LPRECT lprcDst, const RECT* lprcSrc);
BOOL IsRectEmpty(const RECT* lprc);
BOOL EqualRect(const RECT* lprc1, const RECT* lprc2);
BOOL UnionRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2);
BOOL SubtractRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2);
BOOL IntersectRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2);
BOOL PtInRect(const RECT* lprc, POINT pt);

// Clipboard.
#define CF_TEXT 1
BOOL OpenClipboard(HWND hWndNewOwner);
BOOL CloseClipboard();
HANDLE GetClipboardData(UINT uFormat);
#define DM_PELSWIDTH 0x00080000L
#define DM_PELSHEIGHT 0x00100000L
#define DM_BITSPERPEL 0x00040000L
#define HKL_NEXT ((HKL)(ULONG_PTR)1)
#define KLF_REORDER 0x00000008

BOOL PostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
HDC BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint);
BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* lpPaint);
LONG SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong);
BOOL AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
SHORT GetKeyState(int nVirtKey);
HKL GetKeyboardLayout(DWORD idThread);
HKL ActivateKeyboardLayout(HKL hkl, UINT Flags);
HANDLE SetClipboardData(UINT uFormat, HANDLE hMem);

// Device contexts of windows.
HDC GetDC(HWND hWnd);
int ReleaseDC(HWND hWnd, HDC hDC);

// Character helpers.
BOOL IsDBCSLeadByte(BYTE TestChar);
BOOL IsCharAlphaNumericA(CHAR ch);
LPSTR CharNextA(LPCSTR lpsz);

inline LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { return SendMessageA(hWnd, Msg, wParam, lParam); }
inline BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) { return SystemParametersInfoA(uiAction, uiParam, pvParam, fWinIni); }

#endif /* WIN32SHIM_WINUSER_WINDOW_INPUT_H_ */
