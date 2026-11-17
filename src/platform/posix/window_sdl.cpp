// The game's window, message queue and polled input, on SDL3.
//
// What the game relies on (CChitin.cpp, CVidMode.cpp, CVidInf.cpp):
// - One window of class "ChitinClass". Its window procedure gets WM_MOVE and
//   WM_SIZE synchronously from CreateWindowEx and SetWindowPos, and reads
//   the client rectangle in screen coordinates from them.
// - The main thread spins on PeekMessage(PM_NOREMOVE) and draws a frame
//   whenever the AI thread asks for one; GetMessage/DispatchMessage run only
//   when a message is waiting.
// - Input is polled, not message driven: GetAsyncKeyState (bit 15 = down,
//   bit 0 = pressed since the last call, cleared by the call) and
//   GetCursorPos (screen coordinates), possibly from other threads. Key and
//   button messages only feed the "pressed since" notes in WndProc.
//   Key codes are Windows virtual-key codes.
// - The mouse wheel arrives as WM_MOUSEWHEEL with the delta in the high
//   word of wParam.
//
// The screen the game sees is virtual: its size is the display's, the
// window sits where the game put it, and the real SDL window is as large as
// the frames the game presents. SDL is only touched on the main thread;
// other threads read cached input state and leave frames for the next pump.

#include <windows.h>

#include <SDL3/SDL.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "video_present.h"

#ifndef WM_QUIT
#define WM_QUIT 0x0012
#endif
#ifndef WM_KEYUP
#define WM_KEYUP 0x0101
#endif
#ifndef WM_LBUTTONUP
#define WM_LBUTTONUP 0x0202
#endif
#ifndef WM_RBUTTONUP
#define WM_RBUTTONUP 0x0205
#endif
#ifndef WM_MBUTTONUP
#define WM_MBUTTONUP 0x0208
#endif
#ifndef WM_MOUSEMOVE
#define WM_MOUSEMOVE 0x0200
#endif
#ifndef PM_REMOVE
#define PM_REMOVE 0x0001
#endif
#ifndef SWP_NOSIZE
#define SWP_NOSIZE 0x0001
#endif
#ifndef SWP_NOMOVE
#define SWP_NOMOVE 0x0002
#endif
#ifndef SIZE_RESTORED
#define SIZE_RESTORED 0
#endif

namespace {

struct Window {
    WNDPROC proc = NULL;
    bool created = false;
    bool destroyed = false;
    int x = 0;
    int y = 0;
    int w = 800;
    int h = 600;
    LONG style = 0;
    LONG exStyle = 0;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    int textureW = 0;
    int textureH = 0;
    std::string title;
};

Window g_window;
WNDPROC g_classProc = NULL;
std::thread::id g_mainThread;
bool g_videoReady = false;
int g_screenW = 1024;
int g_screenH = 768;

// Messages waiting for GetMessage.
std::mutex g_queueMutex;
std::deque<MSG> g_queue;

// Polled input. Bit 15: down. Bit 0: pressed since the last read.
std::atomic<uint16_t> g_keys[256];
std::atomic<int> g_mouseX { 0 };
std::atomic<int> g_mouseY { 0 };
std::atomic<bool> g_capsLock { false };

// The last frame handed to PresentFrame.
std::mutex g_frameMutex;
std::vector<uint32_t> g_frame;
int g_frameW = 0;
int g_frameH = 0;
uint32_t g_frameCount = 0;
bool g_frameDirty = false;

HWND TheHwnd()
{
    return reinterpret_cast<HWND>(&g_window);
}

bool OnMainThread()
{
    return std::this_thread::get_id() == g_mainThread;
}

void EnsureVideo()
{
    if (g_videoReady) {
        return;
    }
    g_videoReady = true;
    g_mainThread = std::this_thread::get_id();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "[window] SDL_Init failed: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect bounds;
    if (SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &bounds) && bounds.w > 0 && bounds.h > 0) {
        g_screenW = bounds.w;
        g_screenH = bounds.h;
    }

    // The game must always find room for its window on the "desktop".
    if (g_screenW < 2100) {
        g_screenW = 2100;
    }
    if (g_screenH < 1600) {
        g_screenH = 1600;
    }
}

void Post(UINT message, WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    memset(&msg, 0, sizeof(msg));
    msg.hwnd = TheHwnd();
    msg.message = message;
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.time = GetTickCount();
    msg.pt.x = g_mouseX + g_window.x;
    msg.pt.y = g_mouseY + g_window.y;

    std::lock_guard<std::mutex> lock(g_queueMutex);
    g_queue.push_back(msg);
}

LRESULT CallProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (g_window.proc == NULL) {
        return 0;
    }
    return g_window.proc(TheHwnd(), message, wParam, lParam);
}

void SetKey(int vk, bool down)
{
    if (vk <= 0 || vk > 255) {
        return;
    }
    if (down) {
        g_keys[vk].fetch_or(0x8001);
    } else {
        g_keys[vk].fetch_and(static_cast<uint16_t>(~0x8000));
    }
}

// SDL key to Windows virtual-key code. `generic` gets the side-less code
// for Shift, Ctrl and Alt (what WM_KEYDOWN carries).
int VirtualKey(SDL_Keycode key, int& generic)
{
    generic = 0;

    if (key >= SDLK_A && key <= SDLK_Z) {
        return 'A' + static_cast<int>(key - SDLK_A);
    }
    if (key >= SDLK_0 && key <= SDLK_9) {
        return '0' + static_cast<int>(key - SDLK_0);
    }
    if (key >= SDLK_F1 && key <= SDLK_F12) {
        return 0x70 + static_cast<int>(key - SDLK_F1);
    }
    if (key >= SDLK_KP_1 && key <= SDLK_KP_9) {
        return 0x61 + static_cast<int>(key - SDLK_KP_1);
    }

    switch (key) {
    case SDLK_BACKSPACE: return 0x08;
    case SDLK_TAB: return 0x09;
    case SDLK_RETURN: return 0x0D;
    case SDLK_KP_ENTER: return 0x0D;
    case SDLK_PAUSE: return 0x13;
    case SDLK_CAPSLOCK: return 0x14;
    case SDLK_ESCAPE: return 0x1B;
    case SDLK_SPACE: return 0x20;
    case SDLK_PAGEUP: return 0x21;
    case SDLK_PAGEDOWN: return 0x22;
    case SDLK_END: return 0x23;
    case SDLK_HOME: return 0x24;
    case SDLK_LEFT: return 0x25;
    case SDLK_UP: return 0x26;
    case SDLK_RIGHT: return 0x27;
    case SDLK_DOWN: return 0x28;
    case SDLK_PRINTSCREEN: return 0x2C;
    case SDLK_INSERT: return 0x2D;
    case SDLK_DELETE: return 0x2E;
    case SDLK_KP_0: return 0x60;
    case SDLK_KP_MULTIPLY: return 0x6A;
    case SDLK_KP_PLUS: return 0x6B;
    case SDLK_KP_MINUS: return 0x6D;
    case SDLK_KP_PERIOD: return 0x6E;
    case SDLK_KP_DIVIDE: return 0x6F;
    case SDLK_NUMLOCKCLEAR: return 0x90;
    case SDLK_SCROLLLOCK: return 0x91;
    case SDLK_LSHIFT: generic = 0x10; return 0xA0;
    case SDLK_RSHIFT: generic = 0x10; return 0xA1;
    case SDLK_LCTRL: generic = 0x11; return 0xA2;
    case SDLK_RCTRL: generic = 0x11; return 0xA3;
    case SDLK_LALT: generic = 0x12; return 0xA4;
    case SDLK_RALT: generic = 0x12; return 0xA5;
    case SDLK_SEMICOLON: return 0xBA;
    case SDLK_EQUALS: return 0xBB;
    case SDLK_COMMA: return 0xBC;
    case SDLK_MINUS: return 0xBD;
    case SDLK_PERIOD: return 0xBE;
    case SDLK_SLASH: return 0xBF;
    case SDLK_GRAVE: return 0xC0;
    case SDLK_LEFTBRACKET: return 0xDB;
    case SDLK_BACKSLASH: return 0xDC;
    case SDLK_RIGHTBRACKET: return 0xDD;
    case SDLK_APOSTROPHE: return 0xDE;
    }

    return 0;
}

void OnKey(const SDL_KeyboardEvent& key, bool down)
{
    int generic;
    int vk = VirtualKey(key.key, generic);
    if (vk == 0) {
        return;
    }

    g_capsLock = (key.mod & SDL_KMOD_CAPS) != 0;

    SetKey(vk, down);
    if (generic != 0) {
        // The side-less key is down while either side is.
        int other = vk ^ 1;
        bool anyDown = down || (g_keys[other].load() & 0x8000) != 0;
        SetKey(generic, anyDown);
        if (!down && anyDown) {
            g_keys[generic].fetch_or(0x8000);
        }
    }

    bool alt = (key.mod & SDL_KMOD_ALT) != 0;
    int code = generic != 0 ? generic : vk;
    LPARAM lParam = 1 | (static_cast<LPARAM>(key.scancode & 0xFF) << 16);
    if (down) {
        if (key.repeat) {
            lParam |= 0x40000000;
        }
        Post(alt ? WM_SYSKEYDOWN : WM_KEYDOWN, code, lParam);
    } else {
        lParam |= 0xC0000000;
        Post(alt || code == 0x12 ? WM_SYSKEYUP : WM_KEYUP, code, lParam);
    }
}

void SetMouse(float fx, float fy)
{
    float x = fx;
    float y = fy;
    if (g_window.renderer != NULL) {
        SDL_RenderCoordinatesFromWindow(g_window.renderer, fx, fy, &x, &y);
    }
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int w = g_window.textureW > 0 ? g_window.textureW : g_window.w;
    int h = g_window.textureH > 0 ? g_window.textureH : g_window.h;
    if (ix < 0) ix = 0;
    if (iy < 0) iy = 0;
    if (ix > w - 1) ix = w - 1;
    if (iy > h - 1) iy = h - 1;
    g_mouseX = ix;
    g_mouseY = iy;
}

LPARAM MouseLParam()
{
    return static_cast<LPARAM>((g_mouseX.load() & 0xFFFF) | ((g_mouseY.load() & 0xFFFF) << 16));
}

void OnButton(const SDL_MouseButtonEvent& button, bool down)
{
    SetMouse(button.x, button.y);

    int vk;
    UINT downMsg;
    UINT dblMsg;
    UINT upMsg;
    switch (button.button) {
    case SDL_BUTTON_LEFT:
        vk = 0x01;
        downMsg = WM_LBUTTONDOWN;
        dblMsg = WM_LBUTTONDBLCLK;
        upMsg = WM_LBUTTONUP;
        break;
    case SDL_BUTTON_RIGHT:
        vk = 0x02;
        downMsg = WM_RBUTTONDOWN;
        dblMsg = WM_RBUTTONDBLCLK;
        upMsg = WM_RBUTTONUP;
        break;
    case SDL_BUTTON_MIDDLE:
        vk = 0x04;
        downMsg = WM_MBUTTONDOWN;
        dblMsg = WM_MBUTTONDBLCLK;
        upMsg = WM_MBUTTONUP;
        break;
    default:
        return;
    }

    SetKey(vk, down);
    if (down) {
        // The window class has CS_DBLCLKS: every second quick press is a
        // double-click message.
        Post(button.clicks > 0 && button.clicks % 2 == 0 ? dblMsg : downMsg, 0, MouseLParam());
    } else {
        Post(upMsg, 0, MouseLParam());
    }
}

void UploadFrame()
{
    std::lock_guard<std::mutex> lock(g_frameMutex);
    if (!g_frameDirty || g_window.window == NULL || g_frameW <= 0) {
        return;
    }
    g_frameDirty = false;

    if (g_window.texture == NULL || g_window.textureW != g_frameW || g_window.textureH != g_frameH) {
        if (g_window.texture != NULL) {
            SDL_DestroyTexture(g_window.texture);
        }
        g_window.texture = SDL_CreateTexture(g_window.renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, g_frameW, g_frameH);
        g_window.textureW = g_frameW;
        g_window.textureH = g_frameH;
        SDL_SetTextureScaleMode(g_window.texture, SDL_SCALEMODE_NEAREST);
        SDL_SetWindowSize(g_window.window, g_frameW, g_frameH);
        SDL_SetRenderLogicalPresentation(g_window.renderer, g_frameW, g_frameH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
        SDL_ShowWindow(g_window.window);
    }

    SDL_UpdateTexture(g_window.texture, NULL, g_frame.data(), g_frameW * 4);
    SDL_RenderClear(g_window.renderer);
    SDL_RenderTexture(g_window.renderer, g_window.texture, NULL, NULL);
    SDL_RenderPresent(g_window.renderer);
}

// Turn waiting SDL events into messages and input state. Main thread only.
void Pump()
{
    if (!OnMainThread() || !g_window.created) {
        return;
    }

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            Post(WM_CLOSE, 0, 0);
            break;
        case SDL_EVENT_KEY_DOWN:
            OnKey(ev.key, true);
            break;
        case SDL_EVENT_KEY_UP:
            OnKey(ev.key, false);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            SetMouse(ev.motion.x, ev.motion.y);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            OnButton(ev.button, true);
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            OnButton(ev.button, false);
            break;
        case SDL_EVENT_MOUSE_WHEEL: {
            int delta = static_cast<int>(ev.wheel.y * 120.0f);
            if (delta != 0) {
                Post(WM_MOUSEWHEEL, static_cast<WPARAM>(static_cast<uint32_t>(delta & 0xFFFF) << 16), MouseLParam());
            }
            break;
        }
        case SDL_EVENT_WINDOW_EXPOSED: {
            std::lock_guard<std::mutex> lock(g_frameMutex);
            g_frameDirty = g_frameW > 0;
            break;
        }
        }
    }

    UploadFrame();
}

} // namespace

namespace platform {

void GetVirtualScreenSize(int& w, int& h)
{
    EnsureVideo();
    w = g_screenW;
    h = g_screenH;
}

void GetWindowClientRect(int& x, int& y, int& w, int& h)
{
    x = g_window.x;
    y = g_window.y;
    w = g_window.w;
    h = g_window.h;
}

void PresentFrame(const uint32_t* pixels, int w, int h, int pitch)
{
    {
        std::lock_guard<std::mutex> lock(g_frameMutex);
        g_frame.resize(static_cast<size_t>(w) * h);
        for (int y = 0; y < h; y++) {
            memcpy(g_frame.data() + static_cast<size_t>(y) * w,
                reinterpret_cast<const uint8_t*>(pixels) + static_cast<size_t>(y) * pitch,
                static_cast<size_t>(w) * 4);
        }
        g_frameW = w;
        g_frameH = h;
        g_frameCount++;
        g_frameDirty = true;
    }

    if (OnMainThread()) {
        UploadFrame();
    }
}

uint32_t GrabFrame(std::vector<uint32_t>& pixels, int& w, int& h)
{
    std::lock_guard<std::mutex> lock(g_frameMutex);
    pixels = g_frame;
    w = g_frameW;
    h = g_frameH;
    return g_frameCount;
}

} // namespace platform

// Window class and window ------------------------------------------------

ATOM RegisterClassExA(const WNDCLASSEXA* lpWndClass)
{
    g_classProc = lpWndClass->lpfnWndProc;
    return 1;
}

HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    EnsureVideo();
    if (g_window.created || g_classProc == NULL) {
        return NULL;
    }

    g_window.proc = g_classProc;
    g_window.x = X;
    g_window.y = Y;
    g_window.w = nWidth;
    g_window.h = nHeight;
    g_window.style = static_cast<LONG>(dwStyle);
    g_window.exStyle = static_cast<LONG>(dwExStyle);
    g_window.title = lpWindowName != NULL ? lpWindowName : "";

    // The real window appears with the first frame, at the frame's size.
    SDL_WindowFlags flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (!SDL_CreateWindowAndRenderer(g_window.title.c_str(), 800, 600, flags, &g_window.window, &g_window.renderer)) {
        fprintf(stderr, "[window] SDL_CreateWindowAndRenderer failed: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_SetRenderVSync(g_window.renderer, 0);
    SDL_HideCursor();

    g_window.created = true;

    CallProc(WM_SIZE, SIZE_RESTORED, (g_window.w & 0xFFFF) | ((g_window.h & 0xFFFF) << 16));
    CallProc(WM_MOVE, 0, (g_window.x & 0xFFFF) | ((g_window.y & 0xFFFF) << 16));

    return TheHwnd();
}

BOOL DestroyWindow(HWND hWnd)
{
    if (hWnd != TheHwnd() || !g_window.created || g_window.destroyed) {
        return FALSE;
    }
    g_window.destroyed = true;
    CallProc(WM_DESTROY, 0, 0);
    if (OnMainThread() && g_window.window != NULL) {
        SDL_HideWindow(g_window.window);
    }
    return TRUE;
}

BOOL IsWindow(HWND hWnd)
{
    return hWnd == TheHwnd() && g_window.created && !g_window.destroyed;
}

BOOL CloseWindow(HWND hWnd)
{
    return TRUE;
}

BOOL OpenIcon(HWND hWnd)
{
    return TRUE;
}

BOOL UpdateWindow(HWND hWnd)
{
    return TRUE;
}

HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName)
{
    return NULL;
}

// NOTE: Port: the game always has the focus; switching away does not
// pause it.
HWND GetFocus()
{
    return g_window.created ? TheHwnd() : NULL;
}

HMENU GetMenu(HWND hWnd)
{
    return NULL;
}

LONG GetWindowLongA(HWND hWnd, int nIndex)
{
    switch (nIndex) {
    case -16: // GWL_STYLE
        return g_window.style;
    case -20: // GWL_EXSTYLE
        return g_window.exStyle;
    }
    return 0;
}

LONG SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong)
{
    LONG old = GetWindowLongA(hWnd, nIndex);
    switch (nIndex) {
    case -16:
        g_window.style = dwNewLong;
        break;
    case -20:
        g_window.exStyle = dwNewLong;
        break;
    }
    return old;
}

BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    bool moved = false;
    bool sized = false;
    if ((uFlags & SWP_NOMOVE) == 0 && (X != g_window.x || Y != g_window.y)) {
        g_window.x = X;
        g_window.y = Y;
        moved = true;
    }
    if ((uFlags & SWP_NOSIZE) == 0 && (cx != g_window.w || cy != g_window.h)) {
        g_window.w = cx;
        g_window.h = cy;
        sized = true;
    }

    if (sized) {
        CallProc(WM_SIZE, SIZE_RESTORED, (g_window.w & 0xFFFF) | ((g_window.h & 0xFFFF) << 16));
    }
    if (moved) {
        CallProc(WM_MOVE, 0, (g_window.x & 0xFFFF) | ((g_window.y & 0xFFFF) << 16));
    }
    return TRUE;
}

// The window has no frame as far as the game can tell: the window
// rectangle is the client rectangle.
BOOL GetWindowRect(HWND hWnd, LPRECT lpRect)
{
    lpRect->left = g_window.x;
    lpRect->top = g_window.y;
    lpRect->right = g_window.x + g_window.w;
    lpRect->bottom = g_window.y + g_window.h;
    return TRUE;
}

BOOL GetClientRect(HWND hWnd, LPRECT lpRect)
{
    lpRect->left = 0;
    lpRect->top = 0;
    lpRect->right = g_window.w;
    lpRect->bottom = g_window.h;
    return TRUE;
}

BOOL AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle)
{
    return TRUE;
}

BOOL GetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase)
{
    return FALSE;
}

BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint)
{
    lpPoint->x += g_window.x;
    lpPoint->y += g_window.y;
    return TRUE;
}

BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint)
{
    lpPoint->x -= g_window.x;
    lpPoint->y -= g_window.y;
    return TRUE;
}

HDC BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint)
{
    memset(lpPaint, 0, sizeof(*lpPaint));
    return NULL;
}

BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* lpPaint)
{
    return TRUE;
}

HDC GetDC(HWND hWnd)
{
    return NULL;
}

int ReleaseDC(HWND hWnd, HDC hDC)
{
    return 1;
}

// Messages -----------------------------------------------------------------

LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg == WM_CLOSE) {
        DestroyWindow(hWnd);
    }
    return 0;
}

// NOTE: Port: runs the window procedure on the calling thread, whichever
// it is.
LRESULT SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return CallProc(Msg, wParam, lParam);
}

BOOL PostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    Post(Msg, wParam, lParam);
    return TRUE;
}

void PostQuitMessage(int nExitCode)
{
    Post(WM_QUIT, static_cast<WPARAM>(nExitCode), 0);
}

UINT RegisterWindowMessageA(LPCSTR lpString)
{
    static UINT next = 0xC000;
    return next++;
}

BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
    Pump();

    {
        std::lock_guard<std::mutex> lock(g_queueMutex);
        if (!g_queue.empty()) {
            *lpMsg = g_queue.front();
            if ((wRemoveMsg & PM_REMOVE) != 0) {
                g_queue.pop_front();
            }
            return TRUE;
        }
    }

    // NOTE: Port: the game's main loop spins on this call. Windows lets it
    // burn a core; here an idle spin naps for a moment.
    SDL_DelayNS(500000);
    return FALSE;
}

BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
    while (1) {
        Pump();

        {
            std::lock_guard<std::mutex> lock(g_queueMutex);
            if (!g_queue.empty()) {
                *lpMsg = g_queue.front();
                g_queue.pop_front();
                return lpMsg->message != WM_QUIT;
            }
        }

        SDL_Delay(1);
    }
}

// The game takes its text from key codes, not from WM_CHAR.
BOOL TranslateMessage(const MSG* lpMsg)
{
    return FALSE;
}

LRESULT DispatchMessageA(const MSG* lpMsg)
{
    return CallProc(lpMsg->message, lpMsg->wParam, lpMsg->lParam);
}

int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    fprintf(stderr, "[MessageBox] %s: %s\n", lpCaption != NULL ? lpCaption : "", lpText != NULL ? lpText : "");

    UINT buttons = uType & 0xF;
    if (g_videoReady && OnMainThread() && getenv("IWD2RE_TEST_DRIVER") == NULL && buttons == 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, lpCaption, lpText, g_window.window);
    }

    // NOTE: Port: questions are answered "yes" / "OK" for now.
    switch (buttons) {
    case 3: // MB_YESNOCANCEL
    case 4: // MB_YESNO
        return 6; // IDYES
    }
    return 1; // IDOK
}

// Input ----------------------------------------------------------------------

SHORT GetAsyncKeyState(int vKey)
{
    if (vKey < 0 || vKey > 255) {
        return 0;
    }
    return static_cast<SHORT>(g_keys[vKey].fetch_and(0x8000));
}

SHORT GetKeyState(int nVirtKey)
{
    if (nVirtKey < 0 || nVirtKey > 255) {
        return 0;
    }
    SHORT state = static_cast<SHORT>(g_keys[nVirtKey].load() & 0x8000);
    if (nVirtKey == 0x14 && g_capsLock) {
        state |= 1;
    }
    return state;
}

BOOL GetCursorPos(LPPOINT lpPoint)
{
    lpPoint->x = g_mouseX + g_window.x;
    lpPoint->y = g_mouseY + g_window.y;
    return TRUE;
}

HCURSOR SetCursor(HCURSOR hCursor)
{
    return NULL;
}

HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName)
{
    static int cursor;
    return reinterpret_cast<HCURSOR>(&cursor);
}

HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName)
{
    static int icon;
    return reinterpret_cast<HICON>(&icon);
}

UINT GetDoubleClickTime()
{
    return 500;
}

int GetSystemMetrics(int nIndex)
{
    EnsureVideo();
    switch (nIndex) {
    case 0: // SM_CXSCREEN
    case 16: // SM_CXFULLSCREEN
        return g_screenW;
    case 1: // SM_CYSCREEN
    case 17: // SM_CYFULLSCREEN
        return g_screenH;
    case 23: // SM_SWAPBUTTON
        return 0;
    case 36: // SM_CXDOUBLECLK
    case 37: // SM_CYDOUBLECLK
        return 4;
    }
    return 0;
}

BOOL SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
{
    switch (uiAction) {
    case 0x0030: // SPI_GETWORKAREA
        if (pvParam != NULL) {
            RECT* rect = static_cast<RECT*>(pvParam);
            rect->left = 0;
            rect->top = 0;
            rect->right = g_screenW;
            rect->bottom = g_screenH;
        }
        return TRUE;
    case 0x0068: // SPI_GETWHEELSCROLLLINES
        if (pvParam != NULL) {
            *static_cast<UINT*>(pvParam) = 3;
        }
        return TRUE;
    }
    // The screen saver settings and anything else: not available.
    return FALSE;
}

LONG ChangeDisplaySettingsA(DEVMODEA* lpDevMode, DWORD dwFlags)
{
    return 0; // DISP_CHANGE_SUCCESSFUL
}

BOOL EnumDisplaySettingsA(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode)
{
    return FALSE;
}

HKL GetKeyboardLayout(DWORD idThread)
{
    return reinterpret_cast<HKL>(static_cast<uintptr_t>(0x04090409));
}

HKL ActivateKeyboardLayout(HKL hkl, UINT Flags)
{
    return GetKeyboardLayout(0);
}

// Clipboard: not available. ------------------------------------------------

BOOL OpenClipboard(HWND hWndNewOwner)
{
    return FALSE;
}

BOOL CloseClipboard()
{
    return FALSE;
}

HANDLE GetClipboardData(UINT uFormat)
{
    return NULL;
}

HANDLE SetClipboardData(UINT uFormat, HANDLE hMem)
{
    return NULL;
}

// The rectangle and character functions are in user_pure.cpp.
