#include "CVidBlitter.h"

#include "CChitin.h"
#include "CUtil.h"
#include "CWarp.h"

// 0x7B6780
CVidBlitter::CVidBlitter()
{
    m_bSoftSrcKeyBltFast = FALSE;
    m_bSoftBltFast = FALSE;
    m_bSoftSrcKeyBlt = FALSE;
    m_bSoftBlt = FALSE;
    m_bSoftMirrorBlt = FALSE;
    field_E6 = 0;
    m_pSrcSurface = NULL;
    m_pDestSurface = NULL;
    m_pfnBltFast = &CVidBlitter::DxBltFast;
    m_pfnBltNoFX = &CVidBlitter::DxBlt;
    m_pfnSrcKeyBltFast = &CVidBlitter::DxBltFast;
    m_pfnSrcKeyBltNoFX = &CVidBlitter::DxBlt;
    m_pfnBltFX = &CVidBlitter::DxBlt;
    m_pfnSrcKeyBltFX = &CVidBlitter::DxBlt;
}

// #binary-identical
// 0x7B67E0
HRESULT CVidBlitter::DxBlt(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    return lpDDSurface->Blt(lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
}

// #binary-identical
// 0x7B6810
HRESULT CVidBlitter::DxBltFast(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    return lpDDSurface->BltFast(lpDestRect->left, lpDestRect->top, lpDDSrcSurface, lpSrcRect, dwFlags);
}

// #binary-identical
// 0x7B6960
void CVidBlitter::Init()
{
    if (g_pChitin->cVideo.m_nBpp != 16) {
        m_pfnBltNoFX = &CVidBlitter::DxBlt;
        m_pfnSrcKeyBltNoFX = &CVidBlitter::DxBlt;
        m_pfnBltFX = &CVidBlitter::DxBlt;
        m_pfnSrcKeyBltFX = &CVidBlitter::DxBlt;
        m_pfnBltFast = &CVidBlitter::DxBltFast;
        m_pfnSrcKeyBltFast = &CVidBlitter::DxBltFast;
    } else {
        if (m_bSoftMirrorBlt) {
            m_pfnBltFX = &CVidBlitter::SoftBlt;
            m_pfnSrcKeyBltFX = &CVidBlitter::SoftSrcKeyBlt;
        } else {
            if (m_bSoftSrcKeyBlt) {
                m_pfnBltFX = &CVidBlitter::DxBlt;
                m_pfnSrcKeyBltFX = &CVidBlitter::SoftSrcKeyBlt;
            } else {
                m_pfnBltFX = &CVidBlitter::DxBlt;
                m_pfnSrcKeyBltFX = &CVidBlitter::DxBlt;
            }
        }

        if (m_bSoftSrcKeyBltFast) {
            m_pfnSrcKeyBltFast = &CVidBlitter::SoftSrcKeyBlt;
        } else {
            m_pfnSrcKeyBltFast = &CVidBlitter::DxBltFast;
        }

        if (m_bSoftSrcKeyBlt) {
            m_pfnSrcKeyBltNoFX = &CVidBlitter::SoftSrcKeyBlt;
        } else {
            m_pfnSrcKeyBltNoFX = &CVidBlitter::DxBlt;
        }

        if (m_bSoftBltFast) {
            m_pfnBltFast = &CVidBlitter::SoftBlt;
        } else {
            m_pfnBltFast = &CVidBlitter::DxBltFast;
        }

        if (m_bSoftBlt) {
            m_pfnBltNoFX = &CVidBlitter::SoftBlt;
        } else {
            m_pfnBltNoFX = &CVidBlitter::DxBlt;
        }
    }
}

// 0x7B6A40
void CVidBlitter::Reset()
{
    if (g_pChitin->pActiveEngine != NULL) {
        g_pChitin->pActiveEngine->EngineDeactivated();
    }

    g_pChitin->m_bReInitializing = TRUE;

    CVidMode* pVidMode = g_pChitin->cVideo.m_pCurrentVidMode;
    g_pChitin->cVideo.CleanUp();
    g_pChitin->cVideo.m_pCurrentVidMode = pVidMode;

    if (!g_pChitin->cVideo.Initialize(g_pChitin->cWnd.GetSafeHwnd(), g_pChitin->m_bFullscreen)) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidSoftBlt.cpp
        // __LINE__: 342
        UTIL_ASSERT_MSG(FALSE, "Software Blitter re-initialization failed.");
        g_pChitin->ShutDown(-1, NULL, NULL);
    }

    g_pChitin->Resume();

    if (g_pChitin->pActiveEngine != NULL) {
        g_pChitin->pActiveEngine->EngineActivated();
    }

    field_E6 = 0;
}

// 0x7B6840
HRESULT CVidBlitter::Blt(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    if ((dwFlags & DDBLT_DDFX) != 0) {
        if ((dwFlags & DDBLT_KEYSRC) != 0) {
            return (this->*m_pfnSrcKeyBltFX)(lpDDSurface, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
        } else {
            return (this->*m_pfnBltFX)(lpDDSurface, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
        }
    } else {
        if ((dwFlags & DDBLT_KEYSRC) != 0) {
            return (this->*m_pfnSrcKeyBltNoFX)(lpDDSurface, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
        } else {
            return (this->*m_pfnBltNoFX)(lpDDSurface, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
        }
    }
}

// 0x7B68B0
HRESULT CVidBlitter::BltFast(IDirectDrawSurface* lpDDSurface, DWORD dwX, DWORD dwY, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans)
{
    if ((dwTrans & DDBLTFAST_SRCCOLORKEY) != 0) {
        RECT dest;
        dest.left = dwX;
        dest.top = dwY;
        dest.right = dwX + lpSrcRect->right - lpSrcRect->left;
        dest.bottom = dwY + lpSrcRect->bottom - lpSrcRect->top;
        return (this->*m_pfnSrcKeyBltFast)(lpDDSurface, &dest, lpDDSrcSurface, lpSrcRect, dwTrans, NULL);
    } else {
        RECT dest;
        dest.left = dwX;
        dest.top = dwY;
        dest.right = dwX + lpSrcRect->right - lpSrcRect->left;
        dest.bottom = dwY + lpSrcRect->bottom - lpSrcRect->top;
        return (this->*m_pfnBltFast)(lpDDSurface, &dest, lpDDSrcSurface, lpSrcRect, dwTrans, NULL);
    }
}

// 0x7B6B20
HRESULT CVidBlitter::FUN_007b6b20(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect)
{
    HRESULT hr;

    while (1) {
        m_ddsdDest.dwSize = sizeof(DDSURFACEDESC);
        hr = lpDDSurface->Lock(lpDestRect, &m_ddsdDest, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
        if (hr == DDERR_SURFACELOST) {
            // NOTE: Uninline.
            g_pChitin->GetCurrentVideoMode()->RestoreSurfaces();
            continue;
        }

        if (hr != DDERR_SURFACEBUSY) {
            break;
        }
    }

    if (hr != DD_OK) {
        switch (hr) {
        case E_OUTOFMEMORY:
            return DDERR_EXCEPTION;
        case E_INVALIDARG:
        case DDERR_INVALIDOBJECT:
        case DDERR_SURFACELOST:
        case DDERR_WASSTILLDRAWING:
            return hr;
        default:
            return E_FAIL;
        }
    }

    while (1) {
        m_ddsdSrc.dwSize = sizeof(DDSURFACEDESC);
        hr = lpDDSrcSurface->Lock(lpSrcRect, &m_ddsdSrc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
        if (hr == DDERR_SURFACELOST) {
            // NOTE: Uninline.
            g_pChitin->GetCurrentVideoMode()->RestoreSurfaces();
            continue;
        }

        if (hr != DDERR_SURFACEBUSY) {
            break;
        }
    }

    if (hr != DD_OK) {
        lpDDSurface->Unlock(m_ddsdDest.lpSurface);

        switch (hr) {
        case E_OUTOFMEMORY:
            return DDERR_EXCEPTION;
        case E_INVALIDARG:
        case DDERR_INVALIDOBJECT:
        case DDERR_SURFACELOST:
        case DDERR_WASSTILLDRAWING:
            return hr;
        default:
            return E_FAIL;
        }
    }

    m_pDestSurface = reinterpret_cast<BYTE*>(m_ddsdDest.lpSurface);
    m_pSrcSurface = reinterpret_cast<BYTE*>(m_ddsdSrc.lpSurface);

    return DD_OK;
}

// NOTE: Copies one row of `nBytes` bytes left-right mirrored: the destination
// is written forwards and the source is read backwards from its end, one
// 16-bit pixel at a time, so `pDest` pixel `i` = `pSrc` pixel `n - 1 - i`.
//
// The original is an `__asm` block: it reads the source a dword (two pixels)
// at a time from the end and swaps the halves with `ROR EAX, 16`, stores
// dwords to a dword-aligned destination, first stores one word when the
// destination is only word-aligned, and finishes with a byte-wise tail for
// the last `count & 3` bytes. The C++ below is a behavioural equivalent that
// keeps the asm's two cursors (`s` = ESI, `d` = EDI) so the odd-byte tails,
// which are not a clean mirror (they only matter for odd byte counts; the
// callers always pass an even count), behave exactly as in the original.
//
// 0x7B6CC0
void CVidBlitter::FUN_007b6cc0(BYTE* pDest, BYTE* pSrc, int nBytes)
{
    DWORD nCount = nBytes;
    DWORD nRemainder;
    BYTE* s;
    BYTE* d;

    if (nCount < 4) {
        s = pSrc - 2;
        d = pDest - 4;
        nRemainder = nCount;
    } else {
        DWORD nAlign = static_cast<DWORD>(reinterpret_cast<UINT_PTR>(pDest)) & 3;
        if (nAlign == 1 || nAlign == 3) {
            // NOTE: The original jumps out of the `__asm` block without
            // popping the two registers it pushed, so the function returns
            // with EBX, ESI and EDI of the caller corrupted.

            // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidSoftBlt.cpp
            // __LINE__: 635
            UTIL_ASSERT_MSG(FALSE, "Error performing software Blt");
            return;
        }

        s = pSrc + nCount - 4;
        d = pDest - 4;

        if (nAlign == 2) {
            nCount -= 2;
            s -= 2;
            d += 2;
            *reinterpret_cast<WORD*>(d + 2) = *reinterpret_cast<WORD*>(s + 4);
        }

        nRemainder = nCount & 3;

        for (DWORD n = nCount >> 2; n != 0; n--) {
            d += 4;
            DWORD dw = *reinterpret_cast<DWORD*>(s);
            s -= 4;
            *reinterpret_cast<DWORD*>(d) = (dw >> 16) | (dw << 16);
        }
    }

    switch (nRemainder) {
    case 3:
        d[6] = s[1];
        // FALLTHROUGH
    case 2:
        d[4] = s[2];
        // FALLTHROUGH
    case 1:
        d[5] = s[3];
        break;
    }
}

// NOTE: Colour-keyed version of `FUN_007b6cc0`: copies one row of `nBytes`
// bytes left-right mirrored (destination forwards, source backwards), and
// leaves a destination pixel untouched when the source pixel equals the
// colour key. In the dword loop the first pixel of each pair (low word) is
// compared with the low word of `dwColorKey` and the second (high word) with
// the high word; single pixels are compared with the low word. The callers
// pass the key replicated into both halves.
//
// The original is an `__asm` block reading the source a dword at a time
// from the end (`ROR EAX, 16`, `XOR EAX, EBX` against the key, then a dword,
// one word or no store), with a word preamble for a word-aligned destination
// and a byte-wise tail. The C++ below is a behavioural equivalent keeping the
// asm's cursors (`s` = ESI, `d` = EDI) so that the odd-byte tails behave as in
// the original; the 3-byte tail is not keyed and writes one byte before the
// current position.
//
// 0x7B6DC0
void CVidBlitter::FUN_007b6dc0(BYTE* pDest, BYTE* pSrc, int nBytes, DWORD dwColorKey)
{
    DWORD nCount = nBytes;
    DWORD nRemainder;
    WORD wKey = static_cast<WORD>(dwColorKey);
    BYTE* s;
    BYTE* d;

    if (nCount < 4) {
        s = pSrc - 2;
        d = pDest - 4;
        nRemainder = nCount;
    } else {
        DWORD nAlign = static_cast<DWORD>(reinterpret_cast<UINT_PTR>(pDest)) & 3;
        if (nAlign == 1 || nAlign == 3) {
            // NOTE: The original jumps out of the `__asm` block without
            // popping the two registers it pushed, so the function returns
            // with EBX, ESI and EDI of the caller corrupted.

            // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidSoftBlt.cpp
            // __LINE__: 862
            UTIL_ASSERT_MSG(FALSE, "Error performing software Blt");
            return;
        }

        s = pSrc + nCount - 4;
        d = pDest - 4;

        if (nAlign == 2) {
            nCount -= 2;
            s -= 2;
            d += 2;
            if (*reinterpret_cast<WORD*>(s + 4) != wKey) {
                d[2] = s[4];
                d[3] = s[5];
            }
        }

        nRemainder = nCount & 3;

        for (DWORD n = nCount >> 2; n != 0; n--) {
            d += 4;
            DWORD dw = *reinterpret_cast<DWORD*>(s);
            s -= 4;
            dw = (dw >> 16) | (dw << 16);

            DWORD dwDiff = dw ^ dwColorKey;
            if (dwDiff != 0) {
                if ((dwDiff & 0xFFFF) == 0) {
                    *reinterpret_cast<WORD*>(d + 2) = static_cast<WORD>(dw >> 16);
                } else if ((dwDiff & 0xFFFF0000) == 0) {
                    *reinterpret_cast<WORD*>(d) = static_cast<WORD>(dw);
                } else {
                    *reinterpret_cast<DWORD*>(d) = dw;
                }
            }
        }
    }

    switch (nRemainder) {
    case 2:
        if (*reinterpret_cast<WORD*>(s + 2) != wKey) {
            d[4] = s[2];
            d[5] = s[3];
        }
        break;
    case 3:
        d[2] = s[3];
        d[4] = s[2];
        d[5] = s[3];
        break;
    case 1:
        d[5] = s[3];
        break;
    }
}

// NOTE: Copies one row of `nBytes` bytes forwards and leaves a destination
// pixel untouched when the source pixel equals the colour key. In the dword
// loop the first pixel of each pair (low word) is compared with the low word
// of `dwColorKey` and the second (high word) with the high word; single
// pixels are compared with the low word. The callers pass the key replicated
// into both halves.
//
// The original is an `__asm` block reading a dword (two pixels) at a time
// (`XOR EAX, EBX` against the key, then a dword, one word or no store), with
// a word preamble for a word-aligned destination and a byte-wise tail. The
// C++ below is a behavioural equivalent keeping the asm's cursors (`s` = ESI,
// `d` = EDI) so that the odd-byte tails behave as in the original; the 1- and
// 3-byte tails are not keyed and read and write past the current position.
//
// 0x7B6F20
void CVidBlitter::FUN_007b6f20(BYTE* pDest, BYTE* pSrc, int nBytes, DWORD dwColorKey)
{
    DWORD nCount = nBytes;
    DWORD nRemainder;
    WORD wKey = static_cast<WORD>(dwColorKey);
    BYTE* s = pSrc;
    BYTE* d = pDest;

    if (nCount < 4) {
        nRemainder = nCount;
    } else {
        DWORD nAlign = static_cast<DWORD>(reinterpret_cast<UINT_PTR>(pDest)) & 3;
        if (nAlign == 1 || nAlign == 3) {
            // NOTE: The original jumps out of the `__asm` block without
            // popping the two registers it pushed, so the function returns
            // with EBX, ESI and EDI of the caller corrupted.

            // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidSoftBlt.cpp
            // __LINE__: 1091
            UTIL_ASSERT_MSG(FALSE, "Error performing software Blt");
            return;
        }

        if (nAlign == 2) {
            nCount -= 2;
            if (*reinterpret_cast<WORD*>(s) != wKey) {
                d[0] = s[0];
                d[1] = s[1];
            }
            s += 2;
            d += 2;
        }

        nRemainder = nCount & 3;

        for (DWORD n = nCount >> 2; n != 0; n--) {
            DWORD dw = *reinterpret_cast<DWORD*>(s);
            s += 4;

            DWORD dwDiff = dw ^ dwColorKey;
            if (dwDiff != 0) {
                if ((dwDiff & 0xFFFF) == 0) {
                    *reinterpret_cast<WORD*>(d + 2) = static_cast<WORD>(dw >> 16);
                } else if ((dwDiff & 0xFFFF0000) == 0) {
                    *reinterpret_cast<WORD*>(d) = static_cast<WORD>(dw);
                } else {
                    *reinterpret_cast<DWORD*>(d) = dw;
                }
            }

            d += 4;
        }
    }

    switch (nRemainder) {
    case 2:
        if (*reinterpret_cast<WORD*>(s) != wKey) {
            d[0] = s[0];
            d[1] = s[1];
        }
        break;
    case 3:
        d[2] = s[3];
        d[4] = s[2];
        d[5] = s[3];
        break;
    case 1:
        d[5] = s[3];
        break;
    }
}

// NOTE: Colour fill of `lpDestRect` (the whole surface when NULL) with the
// low word of `lpDDBltFx->dwFillColor`. The original fills each row with
// `REP STOSD` of the colour replicated into both halves of a dword, after
// one word store when the row start is not dword-aligned and with one word
// store for an odd pixel count. The C++ below is a behavioural equivalent.
//
// 0x7B7070
HRESULT CVidBlitter::FUN_007b7070(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, LPDDBLTFX lpDDBltFx)
{
    DDSURFACEDESC ddsd;
    RECT rect;
    DWORD nWidth;
    int nHeight;
    HRESULT hr;

    if (lpDestRect == NULL) {
        ddsd.dwSize = sizeof(DDSURFACEDESC);
        lpDDSurface->GetSurfaceDesc(&ddsd);
        nWidth = ddsd.dwWidth;
        nHeight = ddsd.dwHeight;
        SetRect(&rect, 0, 0, ddsd.dwWidth, ddsd.dwHeight);
    } else {
        nWidth = lpDestRect->right - lpDestRect->left;
        nHeight = lpDestRect->bottom - lpDestRect->top;
        CopyRect(&rect, lpDestRect);
    }

    while (1) {
        m_ddsdDest.dwSize = sizeof(DDSURFACEDESC);
        hr = lpDDSurface->Lock(&rect, &m_ddsdDest, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
        if (hr == DDERR_SURFACELOST) {
            // NOTE: Uninline.
            g_pChitin->GetCurrentVideoMode()->RestoreSurfaces();
            continue;
        }

        if (hr != DDERR_SURFACEBUSY) {
            break;
        }
    }

    if (hr != DD_OK) {
        switch (hr) {
        case E_OUTOFMEMORY:
            return DDERR_EXCEPTION;
        case E_INVALIDARG:
        case DDERR_INVALIDOBJECT:
        case DDERR_SURFACELOST:
        case DDERR_WASSTILLDRAWING:
            return hr;
        default:
            return E_FAIL;
        }
    }

    int nPitch = m_ddsdDest.lPitch >> 1;
    m_pDestSurface = reinterpret_cast<BYTE*>(m_ddsdDest.lpSurface);

    for (int y = nHeight; y > 0; y--) {
        DWORD dwColor = static_cast<WORD>(lpDDBltFx->dwFillColor);
        dwColor = (dwColor << 16) + dwColor;

        BYTE* pDest = m_pDestSurface;
        DWORD nCount = nWidth;

        if ((static_cast<DWORD>(reinterpret_cast<UINT_PTR>(pDest)) & 3) != 0) {
            // NOTE: With a zero width this makes the count 0xFFFFFFFF and the
            // following `REP STOSD` runs away, as in the original.
            *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(dwColor);
            nCount--;
            pDest += 2;
        }

        for (DWORD n = nCount >> 1; n != 0; n--) {
            *reinterpret_cast<DWORD*>(pDest) = dwColor;
            pDest += 4;
        }

        if ((nCount & 1) != 0) {
            *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(dwColor);
        }

        m_pDestSurface += nPitch * 2;
    }

    hr = lpDDSurface->Unlock(m_ddsdDest.lpSurface);
    switch (hr) {
    case DD_OK:
        return DD_OK;
    case E_OUTOFMEMORY:
        return DDERR_EXCEPTION;
    case E_INVALIDARG:
    case DDERR_INVALIDOBJECT:
    case DDERR_SURFACEBUSY:
    case DDERR_SURFACELOST:
    case DDERR_WASSTILLDRAWING:
        return hr;
    default:
        return E_FAIL;
    }
}

// 0x7B7280
HRESULT CVidBlitter::SoftBlt(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    if ((dwFlags & 0x400) != 0) {
        return FUN_007b7070(lpDDSurface, lpDestRect, lpDDBltFx);
    }

    int width = lpSrcRect->right - lpSrcRect->left;
    int height = lpSrcRect->bottom - lpSrcRect->top;

    RECT rDest;
    SetRect(&rDest,
        lpDestRect->left,
        lpDestRect->top,
        lpDestRect->left + width,
        lpDestRect->top + height);

    HRESULT hr = FUN_007b6b20(lpDDSurface, &rDest, lpDDSrcSurface, lpSrcRect);
    if (hr != S_OK) {
        return hr;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidSoftBlt.cpp
    // __LINE__: 1340
    UTIL_ASSERT(m_pSrcSurface != NULL);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidSoftBlt.cpp
    // __LINE__: 1341
    UTIL_ASSERT(m_pDestSurface != NULL);

    int nBytes = width * 2;
    int destPitch = m_ddsdDest.lPitch >> 1;
    int srcPitch = m_ddsdSrc.lPitch >> 1;

    BOOL bMirrorLeftRight = FALSE;
    if ((dwFlags & 0x800) != 0) {
        DWORD dwDDFX = lpDDBltFx->dwDDFX;
        bMirrorLeftRight = (dwDDFX & 0x2) != 0;
        if ((dwDDFX & 0x4) != 0) {
            m_pSrcSurface += ((height - 1) * srcPitch) << 1;
            srcPitch = -srcPitch;
        }
    }

    if (bMirrorLeftRight) {
        for (int y = 0; y < height; y++) {
            FUN_007b6cc0(m_pDestSurface, m_pSrcSurface, nBytes);
            m_pDestSurface += destPitch * 2;
            m_pSrcSurface += srcPitch * 2;
        }
    } else {
        for (int y = 0; y < height; y++) {
            // NOTE: The original inlines this as a REP MOVSD / REP MOVSB copy.
            memcpy(m_pDestSurface, m_pSrcSurface, nBytes);
            m_pDestSurface += destPitch * 2;
            m_pSrcSurface += srcPitch * 2;
        }
    }

    lpDDSurface->Unlock(m_ddsdDest.lpSurface);
    lpDDSrcSurface->Unlock(m_ddsdSrc.lpSurface);

    return S_OK;
}

// 0x7B74F0
HRESULT CVidBlitter::SoftSrcKeyBlt(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    int width = lpSrcRect->right - lpSrcRect->left;
    int height = lpSrcRect->bottom - lpSrcRect->top;

    RECT rDest;
    rDest.left = lpDestRect->left;
    rDest.top = lpDestRect->top;
    rDest.right = lpDestRect->left + width;
    rDest.bottom = lpDestRect->top + height;

    DDCOLORKEY colorKey;
    lpDDSrcSurface->GetColorKey(DDCKEY_SRCBLT, &colorKey);

    DWORD dwColorKey = colorKey.dwColorSpaceLowValue & 0xFFFF;
    dwColorKey = (dwColorKey << 16) | dwColorKey;

    HRESULT hr = FUN_007b6b20(lpDDSurface, &rDest, lpDDSrcSurface, lpSrcRect);
    if (hr != S_OK) {
        return hr;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidSoftBlt.cpp
    // __LINE__: 1445
    UTIL_ASSERT(m_pSrcSurface != NULL);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidSoftBlt.cpp
    // __LINE__: 1446
    UTIL_ASSERT(m_pDestSurface != NULL);

    int nBytes = width * 2;
    int destPitch = m_ddsdDest.lPitch >> 1;
    int srcPitch = m_ddsdSrc.lPitch >> 1;

    BOOL bMirrorLeftRight = FALSE;
    if ((dwFlags & 0x800) != 0) {
        DWORD dwDDFX = lpDDBltFx->dwDDFX;
        bMirrorLeftRight = (dwDDFX & 0x2) != 0;
        if ((dwDDFX & 0x4) != 0) {
            m_pSrcSurface += ((height - 1) * srcPitch) << 1;
            srcPitch = -srcPitch;
        }
    }

    if (bMirrorLeftRight) {
        for (int y = 0; y < height; y++) {
            FUN_007b6dc0(m_pDestSurface, m_pSrcSurface, nBytes, dwColorKey);
            m_pDestSurface += destPitch * 2;
            m_pSrcSurface += srcPitch * 2;
        }
    } else {
        for (int y = 0; y < height; y++) {
            FUN_007b6f20(m_pDestSurface, m_pSrcSurface, nBytes, dwColorKey);
            m_pDestSurface += destPitch * 2;
            m_pSrcSurface += srcPitch * 2;
        }
    }

    lpDDSurface->Unlock(m_ddsdDest.lpSurface);
    lpDDSrcSurface->Unlock(m_ddsdSrc.lpSurface);

    return S_OK;
}
