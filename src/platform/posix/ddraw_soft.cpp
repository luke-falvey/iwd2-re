// DirectDraw in software, for non-Windows builds.
//
// What the game relies on (from its call sites in CVideo.cpp, CVidInf.cpp,
// CVidMode.cpp, CVidBlitter.cpp, CVidImage.cpp, CVidFont.cpp):
// - Surfaces are plain memory, 32 bits per pixel (X8R8G8B8), never lost.
//   `Lock` returns a pointer to the top-left of the locked rectangle, with
//   the pitch and size of the whole surface.
// - `Blt` copies, optionally with the source colour key (DDBLT_KEYSRC),
//   mirrored (DDBLT_DDFX), stretched, or fills with a colour
//   (DDBLT_COLORFILL). `BltFast` copies with or without the colour key.
// - Windowed mode: the game draws into an offscreen "back" surface and
//   `Blt`s it to the primary surface at the window's client rectangle in
//   screen coordinates. The primary surface here is a memory picture of the
//   virtual screen; whatever lands in the client rectangle is presented.
// - Full screen: the primary surface has one back buffer and `Flip` swaps
//   them.
// - `GetDC` (GDI text for double-byte fonts) is not supported: it fails.

#include <windows.h>

#include <ddraw.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "video_present.h"

#ifndef DDERR_INVALIDPARAMS
#define DDERR_INVALIDPARAMS ((HRESULT)0x80070057)
#endif
#ifndef DDERR_GENERIC
#define DDERR_GENERIC ((HRESULT)0x80004005)
#endif
#ifndef DDERR_NOTFOUND
#define DDERR_NOTFOUND ((HRESULT)0x887600FF)
#endif
#ifndef DDBLT_COLORFILL
#define DDBLT_COLORFILL 0x00000400
#endif
#ifndef DDBLT_DDFX
#define DDBLT_DDFX 0x00000800
#endif
#ifndef DDBLT_KEYSRC
#define DDBLT_KEYSRC 0x00008000
#endif
#ifndef DDBLT_KEYSRCOVERRIDE
#define DDBLT_KEYSRCOVERRIDE 0x00010000
#endif
#ifndef DDBLTFX_MIRRORLEFTRIGHT
#define DDBLTFX_MIRRORLEFTRIGHT 0x00000002
#endif
#ifndef DDBLTFX_MIRRORUPDOWN
#define DDBLTFX_MIRRORUPDOWN 0x00000004
#endif
#ifndef DDBLTFAST_SRCCOLORKEY
#define DDBLTFAST_SRCCOLORKEY 0x00000001
#endif
#ifndef DDSCAPS_PRIMARYSURFACE
#define DDSCAPS_PRIMARYSURFACE 0x00000200
#endif
#ifndef DDSCAPS_BACKBUFFER
#define DDSCAPS_BACKBUFFER 0x00000004
#endif
#ifndef DDSD_CAPS
#define DDSD_CAPS 0x00000001
#endif
#ifndef DDSD_HEIGHT
#define DDSD_HEIGHT 0x00000002
#endif
#ifndef DDSD_WIDTH
#define DDSD_WIDTH 0x00000004
#endif
#ifndef DDSD_PITCH
#define DDSD_PITCH 0x00000008
#endif
#ifndef DDSD_BACKBUFFERCOUNT
#define DDSD_BACKBUFFERCOUNT 0x00000020
#endif
#ifndef DDSD_PIXELFORMAT
#define DDSD_PIXELFORMAT 0x00001000
#endif
#ifndef DDSD_REFRESHRATE
#define DDSD_REFRESHRATE 0x00040000
#endif
#ifndef DDPF_RGB
#define DDPF_RGB 0x00000040
#endif
#ifndef DDCKEY_SRCBLT
#define DDCKEY_SRCBLT 0x00000008
#endif

namespace {

// The display mode set by the game in full screen, or 0 x 0 in windowed
// mode (then the mode is the virtual screen).
int g_modeW = 0;
int g_modeH = 0;

void CurrentMode(int& w, int& h)
{
    if (g_modeW > 0) {
        w = g_modeW;
        h = g_modeH;
    } else {
        platform::GetVirtualScreenSize(w, h);
    }
}

void FillPixelFormat(DDPIXELFORMAT* pf)
{
    memset(pf, 0, sizeof(*pf));
    pf->dwSize = sizeof(*pf);
    pf->dwFlags = DDPF_RGB;
    pf->dwRGBBitCount = 32;
    pf->dwRBitMask = 0x00FF0000;
    pf->dwGBitMask = 0x0000FF00;
    pf->dwBBitMask = 0x000000FF;
}

struct SoftSurface final : IDirectDrawSurface {
    ULONG m_refs = 1;
    int m_w = 0;
    int m_h = 0;
    uint32_t* m_px = NULL;
    bool m_primary = false;
    bool m_fullscreen = false;
    SoftSurface* m_back = NULL;
    DDCOLORKEY m_key = { 0, 0 };
    DWORD m_caps = 0;

    SoftSurface(int w, int h)
        : m_w(w)
        , m_h(h)
    {
        m_px = static_cast<uint32_t*>(calloc(static_cast<size_t>(w) * h + 16, 4));
    }

    ~SoftSurface()
    {
        free(m_px);
    }

    RECT Full() const
    {
        RECT r = { 0, 0, m_w, m_h };
        return r;
    }

    // The part of the primary surface under the window goes to the screen.
    void Present()
    {
        if (!m_primary) {
            return;
        }

        if (m_fullscreen) {
            platform::PresentFrame(m_px, m_w, m_h, m_w * 4);
            return;
        }

        int x, y, w, h;
        platform::GetWindowClientRect(x, y, w, h);
        if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > m_w || y + h > m_h) {
            return;
        }
        platform::PresentFrame(m_px + static_cast<size_t>(y) * m_w + x, w, h, m_w * 4);
    }

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override
    {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }

    STDMETHOD_(ULONG, AddRef)() override
    {
        return ++m_refs;
    }

    STDMETHOD_(ULONG, Release)() override
    {
        ULONG refs = --m_refs;
        if (refs == 0) {
            if (m_back != NULL) {
                m_back->Release();
            }
            delete this;
        }
        return refs;
    }

    STDMETHOD(Blt)(LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) override
    {
        RECT dr = lpDestRect != NULL ? *lpDestRect : Full();
        int dw = dr.right - dr.left;
        int dh = dr.bottom - dr.top;
        if (dw <= 0 || dh <= 0) {
            return DD_OK;
        }

        // The part of the destination rectangle that is on the surface.
        int x0 = dr.left < 0 ? 0 : dr.left;
        int y0 = dr.top < 0 ? 0 : dr.top;
        int x1 = dr.right > m_w ? m_w : dr.right;
        int y1 = dr.bottom > m_h ? m_h : dr.bottom;

        if ((dwFlags & DDBLT_COLORFILL) != 0) {
            if (lpDDBltFx == NULL) {
                return DDERR_INVALIDPARAMS;
            }
            uint32_t color = lpDDBltFx->dwFillColor;
            for (int y = y0; y < y1; y++) {
                uint32_t* row = m_px + static_cast<size_t>(y) * m_w;
                for (int x = x0; x < x1; x++) {
                    row[x] = color;
                }
            }
            Present();
            return DD_OK;
        }

        SoftSurface* src = static_cast<SoftSurface*>(lpDDSrcSurface);
        if (src == NULL) {
            return DDERR_INVALIDPARAMS;
        }

        RECT sr = lpSrcRect != NULL ? *lpSrcRect : src->Full();
        int sw = sr.right - sr.left;
        int sh = sr.bottom - sr.top;
        if (sw <= 0 || sh <= 0) {
            return DD_OK;
        }

        bool mirrorX = false;
        bool mirrorY = false;
        if ((dwFlags & DDBLT_DDFX) != 0 && lpDDBltFx != NULL) {
            mirrorX = (lpDDBltFx->dwDDFX & DDBLTFX_MIRRORLEFTRIGHT) != 0;
            mirrorY = (lpDDBltFx->dwDDFX & DDBLTFX_MIRRORUPDOWN) != 0;
        }

        bool keyed = false;
        uint32_t key = 0;
        if ((dwFlags & DDBLT_KEYSRC) != 0) {
            keyed = true;
            key = src->m_key.dwColorSpaceLowValue;
        } else if ((dwFlags & DDBLT_KEYSRCOVERRIDE) != 0 && lpDDBltFx != NULL) {
            keyed = true;
            key = lpDDBltFx->ddckSrcColorkey.dwColorSpaceLowValue;
        }
        key &= 0x00FFFFFF;

        // A blit within one surface reads from a copy, so that overlapping
        // rectangles behave like a move.
        uint32_t* srcPx = src->m_px;
        uint32_t* copy = NULL;
        if (src == this) {
            size_t n = static_cast<size_t>(m_w) * m_h * 4;
            copy = static_cast<uint32_t*>(malloc(n));
            memcpy(copy, m_px, n);
            srcPx = copy;
        }

        bool stretch = dw != sw || dh != sh;
        for (int y = y0; y < y1; y++) {
            int v = y - dr.top;
            if (mirrorY) {
                v = dh - 1 - v;
            }
            int sy = sr.top + (stretch ? static_cast<int>(static_cast<int64_t>(v) * sh / dh) : v);
            if (sy < 0 || sy >= src->m_h) {
                continue;
            }
            const uint32_t* srow = srcPx + static_cast<size_t>(sy) * src->m_w;
            uint32_t* drow = m_px + static_cast<size_t>(y) * m_w;

            if (!stretch && !mirrorX && !keyed) {
                // The plain case: one row copy, clipped to the source.
                int xs = x0;
                int xe = x1;
                int sx = sr.left + (xs - dr.left);
                if (sx < 0) {
                    xs -= sx;
                    sx = 0;
                }
                if (sr.left + (xe - dr.left) > src->m_w) {
                    xe = src->m_w - sr.left + dr.left;
                }
                if (xe > xs) {
                    memcpy(drow + xs, srow + sx, static_cast<size_t>(xe - xs) * 4);
                }
                continue;
            }

            for (int x = x0; x < x1; x++) {
                int u = x - dr.left;
                if (mirrorX) {
                    u = dw - 1 - u;
                }
                int sx = sr.left + (stretch ? static_cast<int>(static_cast<int64_t>(u) * sw / dw) : u);
                if (sx < 0 || sx >= src->m_w) {
                    continue;
                }
                uint32_t c = srow[sx];
                if (keyed && (c & 0x00FFFFFF) == key) {
                    continue;
                }
                drow[x] = c;
            }
        }

        free(copy);
        Present();
        return DD_OK;
    }

    STDMETHOD(BltFast)(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) override
    {
        SoftSurface* src = static_cast<SoftSurface*>(lpDDSrcSurface);
        if (src == NULL) {
            return DDERR_INVALIDPARAMS;
        }

        RECT sr = lpSrcRect != NULL ? *lpSrcRect : src->Full();
        RECT dr;
        dr.left = static_cast<LONG>(dwX);
        dr.top = static_cast<LONG>(dwY);
        dr.right = dr.left + (sr.right - sr.left);
        dr.bottom = dr.top + (sr.bottom - sr.top);
        return Blt(&dr, src, &sr, (dwTrans & DDBLTFAST_SRCCOLORKEY) != 0 ? DDBLT_KEYSRC : 0, NULL);
    }

    STDMETHOD(Flip)(LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride, DWORD dwFlags) override
    {
        if (m_back == NULL) {
            return DDERR_GENERIC;
        }
        uint32_t* t = m_px;
        m_px = m_back->m_px;
        m_back->m_px = t;
        Present();
        return DD_OK;
    }

    STDMETHOD(GetAttachedSurface)(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE* lplpDDAttachedSurface) override
    {
        if (m_back == NULL) {
            *lplpDDAttachedSurface = NULL;
            return DDERR_NOTFOUND;
        }
        m_back->AddRef();
        *lplpDDAttachedSurface = m_back;
        return DD_OK;
    }

    STDMETHOD(GetColorKey)(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) override
    {
        *lpDDColorKey = m_key;
        return DD_OK;
    }

    STDMETHOD(GetDC)(HDC* lphDC) override
    {
        static bool logged = false;
        if (!logged) {
            logged = true;
            fprintf(stderr, "[ddraw] IDirectDrawSurface::GetDC is not supported\n");
        }
        *lphDC = NULL;
        return DDERR_GENERIC;
    }

    STDMETHOD(GetPixelFormat)(LPDDPIXELFORMAT lpDDPixelFormat) override
    {
        FillPixelFormat(lpDDPixelFormat);
        return DD_OK;
    }

    void FillDesc(LPDDSURFACEDESC d)
    {
        d->dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT;
        d->dwHeight = m_h;
        d->dwWidth = m_w;
        d->lPitch = m_w * 4;
        d->ddckCKSrcBlt = m_key;
        d->ddsCaps.dwCaps = m_caps;
        FillPixelFormat(&(d->ddpfPixelFormat));
    }

    STDMETHOD(GetSurfaceDesc)(LPDDSURFACEDESC lpDDSurfaceDesc) override
    {
        FillDesc(lpDDSurfaceDesc);
        lpDDSurfaceDesc->lpSurface = NULL;
        return DD_OK;
    }

    STDMETHOD(Lock)(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) override
    {
        int x = 0;
        int y = 0;
        if (lpDestRect != NULL) {
            x = lpDestRect->left;
            y = lpDestRect->top;
            if (x < 0 || y < 0 || lpDestRect->right > m_w || lpDestRect->bottom > m_h
                || lpDestRect->right < x || lpDestRect->bottom < y) {
                return DDERR_INVALIDPARAMS;
            }
        }
        FillDesc(lpDDSurfaceDesc);
        lpDDSurfaceDesc->lpSurface = m_px + static_cast<size_t>(y) * m_w + x;
        return DD_OK;
    }

    STDMETHOD(ReleaseDC)(HDC hDC) override
    {
        return DDERR_GENERIC;
    }

    STDMETHOD(Restore)() override
    {
        return DD_OK;
    }

    STDMETHOD(SetClipper)(LPDIRECTDRAWCLIPPER lpDDClipper) override
    {
        return DD_OK;
    }

    STDMETHOD(SetColorKey)(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) override
    {
        if (lpDDColorKey != NULL) {
            m_key = *lpDDColorKey;
        }
        return DD_OK;
    }

    STDMETHOD(Unlock)(LPVOID lpSurfaceData) override
    {
        Present();
        return DD_OK;
    }
};

struct SoftClipper final : IDirectDrawClipper {
    ULONG m_refs = 1;

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override
    {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }

    STDMETHOD_(ULONG, AddRef)() override { return ++m_refs; }

    STDMETHOD_(ULONG, Release)() override
    {
        ULONG refs = --m_refs;
        if (refs == 0) {
            delete this;
        }
        return refs;
    }

    STDMETHOD(SetHWnd)(DWORD dwFlags, HWND hWnd) override { return DD_OK; }
};

struct SoftDirectDraw final : IDirectDraw2 {
    ULONG m_refs = 1;

    // IDirectDraw and IDirectDraw2 are one object here.
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override
    {
        AddRef();
        *ppvObj = static_cast<IDirectDraw2*>(this);
        return S_OK;
    }

    STDMETHOD_(ULONG, AddRef)() override { return ++m_refs; }

    STDMETHOD_(ULONG, Release)() override
    {
        ULONG refs = --m_refs;
        if (refs == 0) {
            delete this;
        }
        return refs;
    }

    STDMETHOD(CreateSurface)(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter) override
    {
        const DDSURFACEDESC& d = *lpDDSurfaceDesc;
        DWORD caps = (d.dwFlags & DDSD_CAPS) != 0 ? d.ddsCaps.dwCaps : 0;

        if ((caps & DDSCAPS_PRIMARYSURFACE) != 0) {
            int w, h;
            CurrentMode(w, h);
            SoftSurface* front = new SoftSurface(w, h);
            front->m_primary = true;
            front->m_caps = caps;
            if ((d.dwFlags & DDSD_BACKBUFFERCOUNT) != 0 && d.dwBackBufferCount > 0) {
                front->m_fullscreen = true;
                front->m_back = new SoftSurface(w, h);
                front->m_back->m_caps = DDSCAPS_BACKBUFFER;
            }
            *lplpDDSurface = front;
            return DD_OK;
        }

        if ((d.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) != (DDSD_WIDTH | DDSD_HEIGHT)
            || d.dwWidth == 0 || d.dwHeight == 0 || d.dwWidth > 16384 || d.dwHeight > 16384) {
            *lplpDDSurface = NULL;
            return DDERR_INVALIDPARAMS;
        }

        SoftSurface* surface = new SoftSurface(static_cast<int>(d.dwWidth), static_cast<int>(d.dwHeight));
        surface->m_caps = caps;
        *lplpDDSurface = surface;
        return DD_OK;
    }

    STDMETHOD(RestoreDisplayMode)() override
    {
        g_modeW = 0;
        g_modeH = 0;
        return DD_OK;
    }

    STDMETHOD(SetCooperativeLevel)(HWND hWnd, DWORD dwFlags) override
    {
        return DD_OK;
    }

    STDMETHOD(CreateClipper)(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter) override
    {
        *lplpDDClipper = new SoftClipper();
        return DD_OK;
    }

    STDMETHOD(GetDisplayMode)(LPDDSURFACEDESC lpDDSurfaceDesc) override
    {
        int w, h;
        CurrentMode(w, h);
        lpDDSurfaceDesc->dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_REFRESHRATE;
        lpDDSurfaceDesc->dwWidth = w;
        lpDDSurfaceDesc->dwHeight = h;
        lpDDSurfaceDesc->lPitch = w * 4;
        lpDDSurfaceDesc->dwRefreshRate = 60;
        FillPixelFormat(&(lpDDSurfaceDesc->ddpfPixelFormat));
        return DD_OK;
    }

    // NOTE: Port: the pixels are 32 bits whatever depth is asked for.
    STDMETHOD(SetDisplayMode)(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags) override
    {
        g_modeW = static_cast<int>(dwWidth);
        g_modeH = static_cast<int>(dwHeight);
        return DD_OK;
    }

    // Every size the game can ask for, at 32 bits and 60 Hz.
    STDMETHOD(EnumDisplayModes)(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback) override
    {
        static const int sizes[][2] = {
            { 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1280, 960 }, { 1280, 1024 }, { 1600, 1200 }, { 2048, 1536 }
        };
        for (size_t k = 0; k < sizeof(sizes) / sizeof(sizes[0]); k++) {
            DDSURFACEDESC d;
            memset(&d, 0, sizeof(d));
            d.dwSize = sizeof(d);
            d.dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_REFRESHRATE;
            d.dwWidth = sizes[k][0];
            d.dwHeight = sizes[k][1];
            d.lPitch = sizes[k][0] * 4;
            d.dwRefreshRate = 60;
            FillPixelFormat(&(d.ddpfPixelFormat));
            if (lpEnumModesCallback(&d, lpContext) == 0) {
                break;
            }
        }
        return DD_OK;
    }
};

} // namespace

HRESULT WINAPI DirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter)
{
    *lplpDD = new SoftDirectDraw();
    return DD_OK;
}
