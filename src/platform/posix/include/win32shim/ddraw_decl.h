#ifndef WIN32SHIM_DDRAW_DECL_H_
#define WIN32SHIM_DDRAW_DECL_H_
#include <win32shim/wintypes.h>
#include <win32shim/com.h>
#include <win32shim/wingdi.h>

// DirectDraw (version 2 era): types, constants, interfaces.

#define MAKE_DDHRESULT(code) MAKE_HRESULT(1, 0x876, code)

#define DD_OK 0
#define DDENUMRET_CANCEL 0
#define DDENUMRET_OK 1
#define DDERR_EXCEPTION MAKE_DDHRESULT(55)
#define DDERR_INVALIDOBJECT MAKE_DDHRESULT(130)
#define DDERR_INVALIDPARAMS E_INVALIDARG
#define DDERR_OUTOFMEMORY E_OUTOFMEMORY
#define DDERR_SURFACEBUSY MAKE_DDHRESULT(430)
#define DDERR_SURFACELOST MAKE_DDHRESULT(450)
#define DDERR_WASSTILLDRAWING MAKE_DDHRESULT(540)
#define DDERR_EXCLUSIVEMODEALREADYSET MAKE_DDHRESULT(581)
#define DDERR_HWNDSUBCLASSED MAKE_DDHRESULT(570)
#define DDERR_HWNDALREADYSET MAKE_DDHRESULT(571)

struct IDirectDraw;
struct IDirectDraw2;
struct IDirectDrawSurface;
struct IDirectDrawClipper;
struct IDirectDrawPalette;
typedef IDirectDraw* LPDIRECTDRAW;
typedef IDirectDraw2* LPDIRECTDRAW2;
typedef IDirectDrawSurface* LPDIRECTDRAWSURFACE;
typedef IDirectDrawClipper* LPDIRECTDRAWCLIPPER;
typedef IDirectDrawPalette* LPDIRECTDRAWPALETTE;

typedef struct _DDCOLORKEY {
    DWORD dwColorSpaceLowValue;
    DWORD dwColorSpaceHighValue;
} DDCOLORKEY, *LPDDCOLORKEY;

typedef struct _DDSCAPS {
    DWORD dwCaps;
} DDSCAPS, *LPDDSCAPS;

typedef struct _DDPIXELFORMAT {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    union {
        DWORD dwRGBBitCount;
        DWORD dwYUVBitCount;
        DWORD dwZBufferBitDepth;
        DWORD dwAlphaBitDepth;
    };
    union {
        DWORD dwRBitMask;
        DWORD dwYBitMask;
    };
    union {
        DWORD dwGBitMask;
        DWORD dwUBitMask;
    };
    union {
        DWORD dwBBitMask;
        DWORD dwVBitMask;
    };
    union {
        DWORD dwRGBAlphaBitMask;
        DWORD dwYUVAlphaBitMask;
        DWORD dwRGBZBitMask;
        DWORD dwYUVZBitMask;
    };
} DDPIXELFORMAT, *LPDDPIXELFORMAT;

typedef struct _DDSURFACEDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    union {
        LONG lPitch;
        DWORD dwLinearSize;
    };
    DWORD dwBackBufferCount;
    union {
        DWORD dwMipMapCount;
        DWORD dwZBufferBitDepth;
        DWORD dwRefreshRate;
    };
    DWORD dwAlphaBitDepth;
    DWORD dwReserved;
    LPVOID lpSurface;
    DDCOLORKEY ddckCKDestOverlay;
    DDCOLORKEY ddckCKDestBlt;
    DDCOLORKEY ddckCKSrcOverlay;
    DDCOLORKEY ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS ddsCaps;
} DDSURFACEDESC, *LPDDSURFACEDESC;

typedef struct _DDBLTFX {
    DWORD dwSize;
    DWORD dwDDFX;
    DWORD dwROP;
    DWORD dwDDROP;
    DWORD dwRotationAngle;
    DWORD dwZBufferOpCode;
    DWORD dwZBufferLow;
    DWORD dwZBufferHigh;
    DWORD dwZBufferBaseDest;
    DWORD dwZDestConstBitDepth;
    union {
        DWORD dwZDestConst;
        LPDIRECTDRAWSURFACE lpDDSZBufferDest;
    };
    DWORD dwZSrcConstBitDepth;
    union {
        DWORD dwZSrcConst;
        LPDIRECTDRAWSURFACE lpDDSZBufferSrc;
    };
    DWORD dwAlphaEdgeBlendBitDepth;
    DWORD dwAlphaEdgeBlend;
    DWORD dwReserved;
    DWORD dwAlphaDestConstBitDepth;
    union {
        DWORD dwAlphaDestConst;
        LPDIRECTDRAWSURFACE lpDDSAlphaDest;
    };
    DWORD dwAlphaSrcConstBitDepth;
    union {
        DWORD dwAlphaSrcConst;
        LPDIRECTDRAWSURFACE lpDDSAlphaSrc;
    };
    union {
        DWORD dwFillColor;
        DWORD dwFillDepth;
        DWORD dwFillPixel;
        LPDIRECTDRAWSURFACE lpDDSPattern;
    };
    DDCOLORKEY ddckDestColorkey;
    DDCOLORKEY ddckSrcColorkey;
} DDBLTFX, *LPDDBLTFX;

// Surface description flags.
#define DDSD_CAPS 0x00000001
#define DDSD_HEIGHT 0x00000002
#define DDSD_WIDTH 0x00000004
#define DDSD_PITCH 0x00000008
#define DDSD_BACKBUFFERCOUNT 0x00000020
#define DDSD_PIXELFORMAT 0x00001000

// Surface capabilities.
#define DDSCAPS_BACKBUFFER 0x00000004
#define DDSCAPS_COMPLEX 0x00000008
#define DDSCAPS_FLIP 0x00000010
#define DDSCAPS_OFFSCREENPLAIN 0x00000040
#define DDSCAPS_PRIMARYSURFACE 0x00000200
#define DDSCAPS_SYSTEMMEMORY 0x00000800
#define DDSCAPS_VIDEOMEMORY 0x00004000

// Driver capabilities.
#define DDCAPS_BLT 0x00000040

// Blit flags.
#define DDBLT_COLORFILL 0x00000400
#define DDBLT_DDFX 0x00000800
#define DDBLT_KEYSRC 0x00008000
#define DDBLT_WAIT 0x01000000
#define DDBLTFX_MIRRORLEFTRIGHT 0x00000002
#define DDBLTFX_MIRRORUPDOWN 0x00000004
#define DDBLTFAST_SRCCOLORKEY 0x00000001
#define DDBLTFAST_WAIT 0x00000010

#define DDLOCK_WAIT 0x00000001
#define DDLOCK_READONLY 0x00000010
#define DDLOCK_WRITEONLY 0x00000020

#define DDCKEY_SRCBLT 0x00000008

#define DDSCL_FULLSCREEN 0x00000001
#define DDSCL_NORMAL 0x00000008
#define DDSCL_EXCLUSIVE 0x00000010

#define DDFLIP_WAIT 0x00000001

#define DDEDM_REFRESHRATES 0x00000001

typedef HRESULT(WINAPI* LPDDENUMMODESCALLBACK)(LPDDSURFACEDESC, LPVOID);

DEFINE_GUID(IID_IDirectDraw, 0x6C14DB80, 0xA733, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IDirectDraw2, 0xB3A6F3E0, 0x2B43, 0x11CF, 0xA2, 0xDE, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56);

interface IDirectDrawClipper : public IUnknown {
    STDMETHOD(SetHWnd)(DWORD dwFlags, HWND hWnd) PURE;
};

interface IDirectDrawSurface : public IUnknown {
    STDMETHOD(Blt)(LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) PURE;
    STDMETHOD(BltFast)(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) PURE;
    STDMETHOD(Flip)(LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride, DWORD dwFlags) PURE;
    STDMETHOD(GetAttachedSurface)(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE* lplpDDAttachedSurface) PURE;
    STDMETHOD(GetColorKey)(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) PURE;
    STDMETHOD(GetDC)(HDC* lphDC) PURE;
    STDMETHOD(GetPixelFormat)(LPDDPIXELFORMAT lpDDPixelFormat) PURE;
    STDMETHOD(GetSurfaceDesc)(LPDDSURFACEDESC lpDDSurfaceDesc) PURE;
    STDMETHOD(Lock)(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) PURE;
    STDMETHOD(ReleaseDC)(HDC hDC) PURE;
    STDMETHOD(Restore)() PURE;
    STDMETHOD(SetClipper)(LPDIRECTDRAWCLIPPER lpDDClipper) PURE;
    STDMETHOD(SetColorKey)(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) PURE;
    STDMETHOD(Unlock)(LPVOID lpSurfaceData) PURE;
};

interface IDirectDraw : public IUnknown {
    STDMETHOD(CreateSurface)(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter) PURE;
    STDMETHOD(RestoreDisplayMode)() PURE;
    STDMETHOD(SetCooperativeLevel)(HWND hWnd, DWORD dwFlags) PURE;
};

interface IDirectDraw2 : public IDirectDraw {
    STDMETHOD(CreateClipper)(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter) PURE;
    STDMETHOD(GetDisplayMode)(LPDDSURFACEDESC lpDDSurfaceDesc) PURE;
    STDMETHOD(SetDisplayMode)(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags) PURE;
    STDMETHOD(EnumDisplayModes)(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback) PURE;
};

HRESULT WINAPI DirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter);

#endif /* WIN32SHIM_DDRAW_DECL_H_ */
