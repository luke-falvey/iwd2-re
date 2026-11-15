// Test-only DLL injected into the original iwd2.exe or the rebuild so that
// several instances can be driven at once on one desktop.
//
// It replaces a few Windows calls in the game's import table:
//   GetCursorPos, GetAsyncKeyState, GetKeyState  -> a virtual mouse and
//       keyboard that the test harness writes into shared memory
//   GetFocus                                     -> always the game window
//   CreateMutexA                                 -> a per-process name, so the
//       single-instance guard passes
//   CreateEventA                                 -> a per-process name: the
//       game's frame timer signals a named event ("BaldurTimerEvent"), and
//       instances sharing it steal each other's ticks
//   CreateWindowExA                              -> the game window starts at
//       800x600 instead of screen size (the game creates it topmost and
//       screen-sized, then shrinks it; meanwhile it would cover the others)
//   DirectSoundCreate                            -> sound buffers get global
//       focus, so they keep playing while another window is in front. (The
//       game waits on sounds, the chapter narration for one; without this a
//       background instance sees them end at once.)
// It moves the process from CPU 0, where the game pins itself, to the CPU
// named by IWD2_HOOK_CPU, so instances don't all share one core.
// And it subclasses the game window so the game never sees itself lose focus.
//
// Nothing here changes game code; the game still reads its input through the
// same calls, they are just answered from the shared block.
//
// Shared block: file mapping "Local\iwd2hook-<pid>", layout `Shared` below.

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Shared {
    LONG magic; // 'IWH1'
    LONG hwnd; // game window, set by the DLL once found
    LONG cursorValid; // harness: 1 once it has set a cursor position
    LONG cursorX; // client coordinates
    LONG cursorY;
    LONG subclassed; // DLL: 1 once the window procedure is replaced
    LONG reserved[10];
    BYTE down[256]; // harness: 1 while the key or button is held
    BYTE pressed[256]; // harness: 1 on press; cleared when the game reads it
};

// IWD2_HOOK_OFF lists features to leave out, for finding out which one
// matters: any of "event", "size", "cpu", "sound", "focus".
static bool Off(const char* feature)
{
    char list[128];
    DWORD n = GetEnvironmentVariableA("IWD2_HOOK_OFF", list, sizeof(list));
    return n > 0 && n < sizeof(list) && strstr(list, feature) != NULL;
}

static Shared* g_shared = NULL;
static HWND g_hwnd = NULL;
static WNDPROC g_prevProc = NULL;

static BOOL(WINAPI* realGetCursorPos)(LPPOINT) = GetCursorPos;
static SHORT(WINAPI* realGetAsyncKeyState)(int) = GetAsyncKeyState;
static SHORT(WINAPI* realGetKeyState)(int) = GetKeyState;
static HWND(WINAPI* realGetFocus)() = GetFocus;
static HANDLE(WINAPI* realCreateMutexA)(LPSECURITY_ATTRIBUTES, BOOL, LPCSTR) = CreateMutexA;
static HANDLE(WINAPI* realCreateEventA)(LPSECURITY_ATTRIBUTES, BOOL, BOOL, LPCSTR) = CreateEventA;

static BOOL WINAPI HookGetCursorPos(LPPOINT pt)
{
    if (g_shared != NULL) {
        // One call per game update: lets the harness measure the update rate.
        InterlockedIncrement(&g_shared->reserved[0]);
    }
    if (g_shared != NULL && g_shared->cursorValid && g_hwnd != NULL && pt != NULL) {
        POINT p = { g_shared->cursorX, g_shared->cursorY };
        ClientToScreen(g_hwnd, &p);
        *pt = p;
        return TRUE;
    }
    return realGetCursorPos(pt);
}

static SHORT WINAPI HookGetAsyncKeyState(int vk)
{
    if (g_shared == NULL || vk < 0 || vk > 255) {
        return 0;
    }
    SHORT state = 0;
    if (g_shared->down[vk]) {
        state |= (SHORT)0x8000;
    }
    if (g_shared->pressed[vk]) {
        g_shared->pressed[vk] = 0;
        state |= 1;
    }
    return state;
}

static SHORT WINAPI HookGetKeyState(int vk)
{
    if (g_shared == NULL || vk < 0 || vk > 255) {
        return 0;
    }
    return g_shared->down[vk] ? (SHORT)0x8000 : 0;
}

static HWND WINAPI HookGetFocus()
{
    return g_hwnd != NULL ? g_hwnd : realGetFocus();
}

static HANDLE WINAPI HookCreateMutexA(LPSECURITY_ATTRIBUTES sa, BOOL owner, LPCSTR name)
{
    if (name != NULL && strstr(name, "Icewind2") != NULL) {
        char unique[128];
        // No "Global\" prefix: a per-process name needs no shared namespace.
        _snprintf(unique, sizeof(unique), "Icewind2-hook-%lu", GetCurrentProcessId());
        unique[sizeof(unique) - 1] = '\0';
        return realCreateMutexA(sa, owner, unique);
    }
    return realCreateMutexA(sa, owner, name);
}

static HWND(WINAPI* realCreateWindowExA)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID) = CreateWindowExA;

static HWND WINAPI HookCreateWindowExA(DWORD exStyle, LPCSTR cls, LPCSTR title, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE inst, LPVOID param)
{
    // Class names can be atoms (small integers), so check the pointer first.
    if (((ULONG_PTR)cls >> 16) != 0 && strcmp(cls, "ChitinClass") == 0 && !Off("size")) {
        w = 800;
        h = 600;
    }
    return realCreateWindowExA(exStyle, cls, title, style, x, y, w, h, parent, menu, inst, param);
}

// ---- DirectSound: global focus for every secondary buffer ----------------

struct DSBufferDesc { // DSBUFFERDESC, first two fields
    DWORD dwSize;
    DWORD dwFlags;
};

static const DWORD kDSBCAPS_PRIMARYBUFFER = 0x00000001;
static const DWORD kDSBCAPS_STICKYFOCUS = 0x00004000;
static const DWORD kDSBCAPS_GLOBALFOCUS = 0x00008000;

typedef HRESULT(WINAPI* CreateSoundBufferFn)(void* self, const void* desc, void** buffer, void* outer);
static CreateSoundBufferFn realCreateSoundBuffer = NULL;

static HRESULT WINAPI HookCreateSoundBuffer(void* self, const void* desc, void** buffer, void* outer)
{
    BYTE copy[64];
    if (desc != NULL) {
        DWORD size = ((const DSBufferDesc*)desc)->dwSize;
        if (size >= sizeof(DSBufferDesc) && size <= sizeof(copy)) {
            memcpy(copy, desc, size);
            DSBufferDesc* d = (DSBufferDesc*)copy;
            if ((d->dwFlags & kDSBCAPS_PRIMARYBUFFER) == 0) {
                d->dwFlags = (d->dwFlags & ~kDSBCAPS_STICKYFOCUS) | kDSBCAPS_GLOBALFOCUS;
            }
            desc = copy;
        }
    }
    return realCreateSoundBuffer(self, desc, buffer, outer);
}

typedef HRESULT(WINAPI* DirectSoundCreateFn)(const GUID*, void**, void*);
static DirectSoundCreateFn realDirectSoundCreate = NULL;

static HRESULT WINAPI HookDirectSoundCreate(const GUID* guid, void** ds, void* outer)
{
    HRESULT hr = realDirectSoundCreate(guid, ds, outer);
    if (hr == 0 && ds != NULL && *ds != NULL) {
        // IDirectSound vtable: QueryInterface, AddRef, Release,
        // CreateSoundBuffer. The table is shared by every device object.
        void** vtable = *(void***)*ds;
        if (vtable[3] != (void*)HookCreateSoundBuffer) {
            realCreateSoundBuffer = (CreateSoundBufferFn)vtable[3];
            DWORD old;
            VirtualProtect(&vtable[3], sizeof(void*), PAGE_READWRITE, &old);
            vtable[3] = (void*)HookCreateSoundBuffer;
            VirtualProtect(&vtable[3], sizeof(void*), old, &old);
        }
    }
    return hr;
}

static HANDLE WINAPI HookCreateEventA(LPSECURITY_ATTRIBUTES sa, BOOL manual, BOOL initial, LPCSTR name)
{
    if (name != NULL && strcmp(name, "BaldurTimerEvent") == 0 && !Off("event")) {
        char unique[128];
        _snprintf(unique, sizeof(unique), "BaldurTimerEvent-hook-%lu", GetCurrentProcessId());
        unique[sizeof(unique) - 1] = '\0';
        return realCreateEventA(sa, manual, initial, unique);
    }
    return realCreateEventA(sa, manual, initial, name);
}

static LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_shared != NULL) {
        // Counters for the harness: what reaches this window from outside.
        switch (msg) {
        case WM_DISPLAYCHANGE: InterlockedIncrement(&g_shared->reserved[1]); break;
        case WM_ACTIVATEAPP: InterlockedIncrement(&g_shared->reserved[2]); break;
        case WM_SETTINGCHANGE: InterlockedIncrement(&g_shared->reserved[3]); break;
        case WM_WINDOWPOSCHANGED: InterlockedIncrement(&g_shared->reserved[4]); break;
        case WM_PALETTECHANGED: InterlockedIncrement(&g_shared->reserved[5]); break;
        case WM_SYSCOLORCHANGE: InterlockedIncrement(&g_shared->reserved[6]); break;
        case WM_KILLFOCUS: InterlockedIncrement(&g_shared->reserved[7]); break;
        case WM_PAINT: InterlockedIncrement(&g_shared->reserved[8]); break;
        case WM_NCACTIVATE: InterlockedIncrement(&g_shared->reserved[9]); break;
        }
    }
    switch (msg) {
    case WM_ACTIVATEAPP:
        if (wParam == FALSE) {
            return 0;
        }
        break;
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) {
            return 0;
        }
        break;
    case WM_KILLFOCUS:
        return 0;
    }
    return CallWindowProcA(g_prevProc, hWnd, msg, wParam, lParam);
}

struct Patch {
    const char* dll;
    const char* name;
    void* hook;
};

static void PatchImports(HMODULE module, const Patch* patches, int count)
{
    BYTE* base = (BYTE*)module;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    IMAGE_DATA_DIRECTORY dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (dir.VirtualAddress == 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        HMODULE target = GetModuleHandleA(patches[i].dll);
        if (target == NULL) {
            continue;
        }
        void* real = (void*)GetProcAddress(target, patches[i].name);
        if (real == NULL) {
            continue;
        }

        // Match by resolved address, so it works whether the import is by
        // name or ordinal and whatever the DLL name's case.
        IMAGE_IMPORT_DESCRIPTOR* desc = (IMAGE_IMPORT_DESCRIPTOR*)(base + dir.VirtualAddress);
        for (; desc->Name != 0; desc++) {
            IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)(base + desc->FirstThunk);
            for (; thunk->u1.Function != 0; thunk++) {
                if ((void*)thunk->u1.Function == real) {
                    DWORD old;
                    VirtualProtect(&thunk->u1.Function, sizeof(void*), PAGE_READWRITE, &old);
                    thunk->u1.Function = (ULONG_PTR)patches[i].hook;
                    VirtualProtect(&thunk->u1.Function, sizeof(void*), old, &old);
                }
            }
        }
    }
}

static BOOL CALLBACK FindGameWindow(HWND hWnd, LPARAM lParam)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid == GetCurrentProcessId()) {
        char cls[64];
        GetClassNameA(hWnd, cls, sizeof(cls));
        if (strcmp(cls, "ChitinClass") == 0) {
            *(HWND*)lParam = hWnd;
            return FALSE;
        }
    }
    return TRUE;
}

static DWORD WINAPI WatchThread(LPVOID)
{
    char cpuText[16];
    DWORD cpuMask = 0;
    if (GetEnvironmentVariableA("IWD2_HOOK_CPU", cpuText, sizeof(cpuText)) > 0) {
        cpuMask = 1u << (atoi(cpuText) & 31);
    }
    if (Off("cpu")) {
        cpuMask = 0;
    }
    int affinityPolls = 0;

    for (;;) {
        // The game pins itself to CPU 0 right after creating its window.
        // Keep the one-CPU pinning, but on this instance's own CPU. Repeated
        // for a while because the game's call can come after ours.
        if (cpuMask != 0 && g_hwnd != NULL && affinityPolls < 200) {
            SetProcessAffinityMask(GetCurrentProcess(), cpuMask);
            affinityPolls++;
        }

        HWND found = NULL;
        EnumWindows(FindGameWindow, (LPARAM)&found);
        if (found != NULL && found != g_hwnd && Off("focus")) {
            g_hwnd = found;
            if (g_shared != NULL) {
                g_shared->hwnd = (LONG)found;
            }
        } else if (found != NULL && found != g_hwnd) {
            g_prevProc = (WNDPROC)SetWindowLongA(found, GWL_WNDPROC, (LONG)HookWndProc);
            g_hwnd = found;
            if (g_shared != NULL) {
                g_shared->hwnd = (LONG)found;
                g_shared->subclassed = 1;
            }
        }
        // Screenshots are read from the screen, so keep the window above
        // whatever else is on the desktop (the game drops its topmost style
        // once it is windowed).
        if (g_hwnd != NULL && !Off("top") && (GetWindowLongA(g_hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == 0) {
            SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        Sleep(50);
    }
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);

        char name[64];
        _snprintf(name, sizeof(name), "Local\\iwd2hook-%lu", GetCurrentProcessId());
        HANDLE mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Shared), name);
        if (mapping != NULL) {
            g_shared = (Shared*)MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Shared));
            if (g_shared != NULL) {
                g_shared->magic = 0x31485749; // "IWH1"
            }
        }

        static const Patch patches[] = {
            { "user32.dll", "GetCursorPos", (void*)HookGetCursorPos },
            { "user32.dll", "GetAsyncKeyState", (void*)HookGetAsyncKeyState },
            { "user32.dll", "GetKeyState", (void*)HookGetKeyState },
            { "user32.dll", "GetFocus", (void*)HookGetFocus },
            { "kernel32.dll", "CreateMutexA", (void*)HookCreateMutexA },
            { "kernel32.dll", "CreateEventA", (void*)HookCreateEventA },
            { "user32.dll", "CreateWindowExA", (void*)HookCreateWindowExA },
        };
        PatchImports(GetModuleHandleA(NULL), patches, sizeof(patches) / sizeof(patches[0]));

        HMODULE dsound = LoadLibraryA("dsound.dll");
        if (dsound != NULL && !Off("sound")) {
            realDirectSoundCreate = (DirectSoundCreateFn)GetProcAddress(dsound, "DirectSoundCreate");
            static const Patch soundPatch[] = {
                { "dsound.dll", "DirectSoundCreate", (void*)HookDirectSoundCreate },
            };
            PatchImports(GetModuleHandleA(NULL), soundPatch, 1);
        }

        CloseHandle(CreateThread(NULL, 0, WatchThread, NULL, 0, NULL));
    }
    return TRUE;
}
