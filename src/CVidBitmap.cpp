#include "CVidBitmap.h"

#include "CChitin.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVideo3d.h"

// 0x7B2A40
CVidBitmap::CVidBitmap()
{
    m_nBitCount = 8;
    m_bDoubleSize = FALSE;
}

// 0x7B2DB0
CVidBitmap::~CVidBitmap()
{
}

// FIXME: Use reference.
//
// 0x7B2AD0
CVidBitmap::CVidBitmap(CResRef cNewResRef, BOOL bDoubleSize)
{
    // NOTE: Uninline.
    SetResRef(cNewResRef, TRUE, TRUE);

    m_nBitCount = 8;
    m_bDoubleSize = bDoubleSize;
}

// FIXME: Use reference.
//
// 0x7B2C40
CVidBitmap::CVidBitmap(CResRef cNewResRef, BOOL bDoubleSize, USHORT nBitCount)
{
    // NOTE: Uninline.
    SetResRef(cNewResRef, TRUE, TRUE);

    m_nBitCount = nBitCount;
    m_bDoubleSize = bDoubleSize;
}

// 0x7B2E90
SHORT CVidBitmap::GetBitCount(BOOL bDemanded)
{
    if (!bDemanded) {
        pRes->Demand();
    }

    SHORT nBitCount = pRes->GetBitDepth();

    if (!bDemanded) {
        pRes->Release();
    }

    return nBitCount;
}

// 0x7B2ED0
BOOL CVidBitmap::GetImageDimensions(CSize& size, BOOLEAN bDemanded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 7277
    UTIL_ASSERT(pRes != NULL);

    if (pRes == NULL) {
        return FALSE;
    }

    if (!bDemanded) {
        if (pRes->Demand() == NULL) {
            return FALSE;
        }
    }

    pRes->GetImageDimensions(size, m_bDoubleSize);

    if (!bDemanded) {
        pRes->Release();
    }

    return TRUE;
}

// 0x7B2F60
BOOL CVidBitmap::GetPixelColor(RGBQUAD& color, INT nX, INT nY, BOOL bLoaded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 7337
    UTIL_ASSERT(pRes != NULL);

    if (pRes == NULL) {
        return FALSE;
    }

    if (bLoaded) {
        pRes->GetPixelColor(nX, nY, color, m_bDoubleSize);
        return TRUE;
    }

    if (pRes->Demand()) {
        pRes->GetPixelColor(nX, nY, color, m_bDoubleSize);
        pRes->Release();
        return TRUE;
    }

    return FALSE;
}

// 0x7B3000
BOOL CVidBitmap::GetPixelValue(BYTE& value, INT x, INT y, BOOLEAN bDemanded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 7400
    UTIL_ASSERT((m_nBitCount == CVIDBITMAP_8BIT || m_nBitCount == CVIDBITMAP_4BIT));

    if (pRes == NULL) {
        return FALSE;
    }

    if (m_nBitCount == CVIDBITMAP_8BIT || m_nBitCount == CVIDBITMAP_4BIT) {
        if (bDemanded) {
            value = pRes->GetPixelValue(x, y, m_bDoubleSize);
            return TRUE;
        }

        if (pRes->Demand() != NULL) {
            value = pRes->GetPixelValue(x, y, m_bDoubleSize);
            pRes->Release();
            return TRUE;
        }
    }

    return FALSE;
}

// 0x7B30C0
BYTE CVidBitmap::GetPixelValue(INT x, INT y, BOOLEAN bDemanded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 7440
    UTIL_ASSERT((m_nBitCount == CVIDBITMAP_8BIT || m_nBitCount == CVIDBITMAP_4BIT));

    if (pRes == NULL) {
        return 0;
    }

    if (m_nBitCount == CVIDBITMAP_8BIT || m_nBitCount == CVIDBITMAP_4BIT) {
        if (bDemanded) {
            return pRes->GetPixelValue(x, y, m_bDoubleSize);
        }

        if (pRes->Demand() != NULL) {
            BYTE value = pRes->GetPixelValue(x, y, m_bDoubleSize);
            pRes->Release();
            return value;
        }
    }

    return 0;
}

// 0x7B3180
void CVidBitmap::SetRes(CResBitmap* pResBmp)
{
    pRes = pResBmp;
}

// 0x7B3190
BOOL CVidBitmap::RenderDirect(INT nSurface, INT x, INT y, const CRect& rClip, DWORD dwFlags, BOOLEAN bDemanded)
{
    if (g_pChitin->cVideo.Is3dAccelerated()) {
        return RenderDirect3d(x, y, rClip, dwFlags, bDemanded);
    }

    if (pRes == NULL) {
        return FALSE;
    }

    if (rClip.IsRectEmpty()) {
        return FALSE;
    }

    if (!bDemanded) {
        pRes->Demand();
    }

    BYTE* pData = pRes->GetImageData(m_bDoubleSize);
    m_nBitCount = pRes->GetBitDepth();

    CSize imageSize;
    pRes->GetImageDimensions(imageSize, m_bDoubleSize);

    CSize bmpSize = imageSize;

    INT nBytesPerPixel;
    switch (m_nBitCount) {
    case 8:
        nBytesPerPixel = 1;
        break;
    case 24:
        nBytesPerPixel = 3;
        break;
    default:
        if (!bDemanded) {
            pRes->Release();
        }
        return FALSE;
    }

    LONG nDataJump = (-((imageSize.cx * nBytesPerPixel) & 3)) & 3;

    CRect rSurface(x, y, x + imageSize.cx, y + imageSize.cy);

    if (imageSize.cy > rClip.Height()) {
        SHORT nSkipBottom = static_cast<SHORT>(imageSize.cy - rClip.bottom + y);
        if (nSkipBottom > 0) {
            pData += nSkipBottom * (nDataJump + imageSize.cx * nBytesPerPixel);
            bmpSize.cy -= nSkipBottom;
            rSurface.bottom -= nSkipBottom;
        }

        SHORT nSkipTop = static_cast<SHORT>(rClip.top - y);
        if (nSkipTop > 0) {
            bmpSize.cy -= nSkipTop;
            rSurface.top += nSkipTop;
        }
    }

    if (imageSize.cx > rClip.Width()) {
        SHORT nSkipRight = static_cast<SHORT>(imageSize.cx - rClip.right + x);
        if (nSkipRight > 0) {
            nDataJump += nSkipRight * nBytesPerPixel;
            bmpSize.cx -= nSkipRight;
            rSurface.right -= nSkipRight;
        }

        SHORT nSkipLeft = static_cast<SHORT>(rClip.left - x);
        if (nSkipLeft > 0) {
            bmpSize.cx -= nSkipLeft;
            nDataJump += nSkipLeft * nBytesPerPixel;
            pData += nSkipLeft * nBytesPerPixel;
            rSurface.left += nSkipLeft;
        }
    }

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);

    if (!g_pChitin->GetCurrentVideoMode()->LockSurface(nSurface, &ddsd, rSurface)) {
        if (!bDemanded) {
            pRes->Release();
        }
        return FALSE;
    }

    // NOTE: Bitmaps are stored bottom-up, so blitting starts at the last row.
    BYTE* pSurface = reinterpret_cast<BYTE*>(ddsd.lpSurface) + 4 * (bmpSize.cy - 1) * (ddsd.lPitch >> 2);

    BOOL bResult;
    switch (m_nBitCount) {
    case 8:
        switch (g_pChitin->cVideo.m_nBpp) {
        case 16:
            // NOTE: Uninline.
            bResult = BltBmp8To16(reinterpret_cast<WORD*>(pSurface),
                ddsd.lPitch,
                pData,
                bmpSize,
                nDataJump,
                dwFlags);
            break;
        case 24:
            bResult = BltBmp8To24(pSurface,
                ddsd.lPitch,
                pData,
                bmpSize,
                nDataJump,
                dwFlags);
            break;
        case 32:
            bResult = BltBmp8To32(reinterpret_cast<DWORD*>(pSurface),
                ddsd.lPitch,
                pData,
                bmpSize,
                nDataJump,
                dwFlags);
            break;
        default:
            g_pChitin->GetCurrentVideoMode()->UnLockSurface(nSurface, ddsd.lpSurface);
            if (!bDemanded) {
                pRes->Release();
            }
            return FALSE;
        }
        break;
    case 24:
        switch (g_pChitin->cVideo.m_nBpp) {
        case 16:
            if ((dwFlags & 0xFFFF0000) == 0
                && g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(RGB(128, 128, 128)) == RGB(128, 128, 128)) {
                bResult = BltBmp24To16(reinterpret_cast<WORD*>(pSurface),
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
            } else {
                bResult = BltBmp24To16Tint(reinterpret_cast<WORD*>(pSurface),
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
            }
            break;
        case 24:
            if ((dwFlags & 0xFFFF0000) == 0
                && g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(RGB(128, 128, 128)) == RGB(128, 128, 128)) {
                bResult = BltBmp24To24(pSurface,
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
            } else {
                bResult = BltBmp24To24Tint(pSurface,
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
            }
            break;
        case 32:
            if ((dwFlags & 0xFFFF0000) == 0
                && g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(RGB(128, 128, 128)) == RGB(128, 128, 128)) {
                bResult = BltBmp24To32(reinterpret_cast<DWORD*>(pSurface),
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
            } else {
                bResult = BltBmp24To32Tint(reinterpret_cast<DWORD*>(pSurface),
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
            }
            break;
        default:
            g_pChitin->GetCurrentVideoMode()->UnLockSurface(nSurface, ddsd.lpSurface);
            if (!bDemanded) {
                pRes->Release();
            }
            return FALSE;
        }
        break;
    default:
        g_pChitin->GetCurrentVideoMode()->UnLockSurface(nSurface, ddsd.lpSurface);
        if (!bDemanded) {
            pRes->Release();
        }
        return FALSE;
    }

    g_pChitin->GetCurrentVideoMode()->UnLockSurface(nSurface, ddsd.lpSurface);

    if (!bDemanded) {
        pRes->Release();
    }

    return bResult;
}

// NOTE: Inlined in `CVidBitmap::RenderDirect`. The original is inline
// assembly which writes two pixels at once whenever it can.
BOOL CVidBitmap::BltBmp8To16(WORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    m_cPalette.SetPalette(pRes->GetColorTable(), pRes->GetColorCount(), CVidPalette::TYPE_RESOURCE);
    m_cPalette.Realize(rgbTempPal, 16, dwFlags | 0x800000, &m_paletteAffects, 255);

    BYTE* pDest = reinterpret_cast<BYTE*>(pSurface);
    LONG nSurfaceJump = lPitch + 2 * bmpSize.cx;

    if ((bmpSize.cx & 0xFFFE) == 0) {
        for (LONG y = 0; y < bmpSize.cy; y++) {
            *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(rgbTempPal[*pData++]);
            pDest += 2;

            pDest -= nSurfaceJump;
            pData += nDataJump;
        }
    } else if ((bmpSize.cx & 1) == 0) {
        // FIXME: The original halves only the low byte of the width
        // (`shr byte ptr`), so even widths of 256 or more get too many pairs.
        LONG nPairs = (bmpSize.cx & ~0xFF) | ((bmpSize.cx & 0xFF) >> 1);
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < nPairs; x++) {
                *reinterpret_cast<DWORD*>(pDest) = (static_cast<WORD>(rgbTempPal[pData[1]]) << 16) | static_cast<WORD>(rgbTempPal[pData[0]]);
                pData += 2;
                pDest += 4;
            }

            pDest -= nSurfaceJump;
            pData += nDataJump;
        }
    } else {
        LONG nPairs = bmpSize.cx >> 1;
        for (LONG y = 0; y < bmpSize.cy; y++) {
            *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(rgbTempPal[*pData++]);
            pDest += 2;

            for (LONG x = 0; x < nPairs; x++) {
                *reinterpret_cast<DWORD*>(pDest) = (static_cast<WORD>(rgbTempPal[pData[1]]) << 16) | static_cast<WORD>(rgbTempPal[pData[0]]);
                pData += 2;
                pDest += 4;
            }

            pDest -= nSurfaceJump;
            pData += nDataJump;
        }
    }

    return TRUE;
}

// NOTE: The non-transparent path of the original is inline assembly which
// writes two pixels at once whenever it can.
//
// 0x7B37A0
BOOL CVidBitmap::BltBmp24To16(WORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();

    BYTE nRDown = static_cast<BYTE>(pVidMode->field_C2);
    BYTE nGDown = static_cast<BYTE>(pVidMode->field_C6);
    BYTE nBDown = static_cast<BYTE>(pVidMode->field_CA);
    BYTE nRShift = static_cast<BYTE>(pVidMode->m_dwRBitShift);
    BYTE nGShift = static_cast<BYTE>(pVidMode->m_dwGBitShift);
    BYTE nBShift = static_cast<BYTE>(pVidMode->m_dwBBitShift);

    if ((dwFlags & 0x1) != 0) {
        DWORD dwRBitMask = g_pChitin->GetCurrentVideoMode()->m_dwRBitMask;
        DWORD dwGBitMask = g_pChitin->GetCurrentVideoMode()->m_dwGBitMask;
        DWORD dwBBitMask = g_pChitin->GetCurrentVideoMode()->m_dwBBitMask;
        LONG nSurfaceJump = (lPitch >> 1) + bmpSize.cx;
        INT nDataIndex = 0;

        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                BYTE r = pData[nDataIndex + 2];
                BYTE g = pData[nDataIndex + 1];
                BYTE b = pData[nDataIndex];

                if (r != 0 || g != 255 || b != 0) {
                    if ((dwFlags & 0x4) != 0 && r == 0 && g == 0 && b == 0) {
                        WORD nColor = *pSurface;
                        r = static_cast<BYTE>(static_cast<INT>(((dwRBitMask & 0xFFFF) & nColor) >> nRShift << nRDown) >> 1);
                        g = static_cast<BYTE>(static_cast<INT>(((dwGBitMask & 0xFFFF) & nColor) >> nGShift << nGDown) >> 1);
                        b = static_cast<BYTE>(static_cast<INT>(((dwBBitMask & 0xFFFF) & nColor) >> nBShift << nBDown) >> 1);
                    }

                    *pSurface = static_cast<WORD>(((b >> nBDown) << nBShift)
                        | ((g >> nGDown) << nGShift)
                        | ((r >> nRDown) << nRShift));
                }

                pSurface++;
                nDataIndex += 3;
            }

            pSurface -= nSurfaceJump;
            nDataIndex += nDataJump;
        }
    } else {
        BYTE* pDest = reinterpret_cast<BYTE*>(pSurface);
        LONG nSurfaceJump = lPitch + 2 * bmpSize.cx;

        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(((pData[2] >> nRDown) << nRShift)
                    | ((pData[1] >> nGDown) << nGShift)
                    | ((pData[0] >> nBDown) << nBShift));
                pData += 3;
                pDest += 2;
            }

            pDest -= nSurfaceJump;
            pData += nDataJump;
        }
    }

    return TRUE;
}

// NOTE: Locals shared by `BltBmp24To16Tint`, `BltBmp24To24Tint` and
// `BltBmp24To32Tint`.
typedef struct {
    BOOL bTint;
    INT nTintRed;
    INT nTintGreen;
    INT nTintBlue;
    INT nTintShift;
    BOOL bAdd;
    INT nAddRed;
    INT nAddGreen;
    INT nAddBlue;
    INT nAddShift;
    BOOL bLight;
    INT nLightRed;
    INT nLightGreen;
    INT nLightBlue;
} CVidBitmapEffects;

// NOTE: Per-pixel effects of the inline assembly in the non-transparent paths
// of `BltBmp24To16Tint` (a local subroutine at 0x7B3F66), `BltBmp24To24Tint`
// and `BltBmp24To32Tint`. Channels are 32-bit registers whose upper bits are
// not cleared between steps.
static void BltBmpApplyEffects(DWORD& r, DWORD& g, DWORD& b, DWORD dwFlags, const CVidBitmapEffects& effects)
{
    if ((dwFlags & 0x80000) != 0) {
        r = (r + g + b) >> 2;
        g = r;
        b = r;
    }

    if (effects.bTint) {
        r = r * effects.nTintRed >> effects.nTintShift;
        g = g * effects.nTintGreen >> effects.nTintShift;
        b = b * effects.nTintBlue >> effects.nTintShift;
    }

    if (effects.bAdd) {
        r = ((r ^ 0xFF) * effects.nAddRed >> effects.nAddShift) ^ 0xFF;
        g = ((g ^ 0xFF) * effects.nAddGreen >> effects.nAddShift) ^ 0xFF;
        b = ((b ^ 0xFF) * effects.nAddBlue >> effects.nAddShift) ^ 0xFF;
    }

    if (effects.bLight) {
        r = r * effects.nLightRed >> 3;
        g = g * effects.nLightGreen >> 3;
        b = b * effects.nLightBlue >> 3;

        // NOTE: Only the low byte is saturated.
        if ((r & 0xFF00) != 0) {
            r |= 0xFF;
        }
        if ((g & 0xFF00) != 0) {
            g |= 0xFF;
        }
        if ((b & 0xFF00) != 0) {
            b |= 0xFF;
        }
    }
}

// NOTE: Per-pixel effects of the transparent paths of `BltBmp24To16Tint`,
// `BltBmp24To24Tint` and `BltBmp24To32Tint`.
static void BltBmpApplyEffects(BYTE& r, BYTE& g, BYTE& b, DWORD dwFlags, const CVidBitmapEffects& effects)
{
    if ((dwFlags & 0x80000) != 0) {
        BYTE v = static_cast<BYTE>((r + g + b) / 4);
        r = v;
        g = v;
        b = v;
    }

    if (effects.bTint) {
        r = static_cast<BYTE>((r * effects.nTintRed) >> effects.nTintShift);
        g = static_cast<BYTE>((g * effects.nTintGreen) >> effects.nTintShift);
        b = static_cast<BYTE>((b * effects.nTintBlue) >> effects.nTintShift);
    }

    if (effects.bAdd) {
        r = ~static_cast<BYTE>((static_cast<DWORD>(static_cast<BYTE>(~r)) * effects.nAddRed) >> effects.nAddShift);
        g = ~static_cast<BYTE>((static_cast<DWORD>(static_cast<BYTE>(~g)) * effects.nAddGreen) >> effects.nAddShift);
        b = ~static_cast<BYTE>((static_cast<DWORD>(static_cast<BYTE>(~b)) * effects.nAddBlue) >> effects.nAddShift);
    }

    if (effects.bLight) {
        INT nRed = (r * effects.nLightRed) >> CVidPalette::LIGHT_SCALE;
        r = nRed > 255 ? 255 : static_cast<BYTE>(nRed);

        INT nGreen = (g * effects.nLightGreen) >> CVidPalette::LIGHT_SCALE;
        g = nGreen > 255 ? 255 : static_cast<BYTE>(nGreen);

        INT nBlue = (b * effects.nLightBlue) >> CVidPalette::LIGHT_SCALE;
        b = nBlue > 255 ? 255 : static_cast<BYTE>(nBlue);

        // Keep the result away from the transparent color.
        if (r == 0 && g == 255 && b == 0) {
            g = 254;
        }
    }
}

// NOTE: Inlined in `BltBmp24To16Tint`, `BltBmp24To24Tint` and
// `BltBmp24To32Tint`.
static void BltBmpGetEffects(CVidImage* pImage, CVidBitmapEffects& effects, DWORD dwFlags)
{
    effects.nTintRed = 255;
    effects.nTintGreen = 255;
    effects.nTintBlue = 255;
    effects.nTintShift = 0;
    effects.nAddRed = 1;
    effects.nAddGreen = 1;
    effects.nAddBlue = 1;
    effects.nAddShift = 0;
    effects.nLightRed = 1 << CVidPalette::LIGHT_SCALE;
    effects.nLightGreen = 1 << CVidPalette::LIGHT_SCALE;
    effects.nLightBlue = 1 << CVidPalette::LIGHT_SCALE;

    effects.bTint = pImage->GetTint(effects.nTintRed, effects.nTintGreen, effects.nTintBlue, effects.nTintShift, dwFlags);
    effects.bAdd = pImage->GetAdd(effects.nAddRed, effects.nAddGreen, effects.nAddBlue, effects.nAddShift, dwFlags);
    effects.bLight = pImage->GetLight(effects.nLightRed, effects.nLightGreen, effects.nLightBlue, dwFlags);
}

// NOTE: One pixel of the inline assembly at 0x7B3FF8 (the non-transparent
// path of `BltBmp24To16Tint`): the effects subroutine at 0x7B3F66, then only
// the low byte of each channel is shifted down but the whole low word is
// shifted up. The low word of the result is the 16-bit pixel; the upper 16
// bits are the upper bits of the red channel register, left as they are.
static DWORD BltBmpPack16(BYTE nRed, BYTE nGreen, BYTE nBlue, DWORD dwFlags, const CVidBitmapEffects& effects, BYTE nRDown, BYTE nGDown, BYTE nBDown, BYTE nRShift, BYTE nGShift, BYTE nBShift)
{
    DWORD r = nRed;
    DWORD g = nGreen;
    DWORD b = nBlue;
    BltBmpApplyEffects(r, g, b, dwFlags, effects);

    return (r & 0xFFFF0000)
        | static_cast<WORD>(static_cast<WORD>(((r & 0xFF00) | (static_cast<BYTE>(r) >> nRDown)) << nRShift)
            | static_cast<WORD>(((g & 0xFF00) | (static_cast<BYTE>(g) >> nGDown)) << nGShift)
            | static_cast<WORD>(((b & 0xFF00) | (static_cast<BYTE>(b) >> nBDown)) << nBShift));
}

// NOTE: The non-transparent path of the original is inline assembly which
// writes two pixels at once whenever it can.
//
// 0x7B3B70
BOOL CVidBitmap::BltBmp24To16Tint(WORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();

    BYTE nRDown = static_cast<BYTE>(pVidMode->field_C2);
    BYTE nGDown = static_cast<BYTE>(pVidMode->field_C6);
    BYTE nBDown = static_cast<BYTE>(pVidMode->field_CA);
    BYTE nRShift = static_cast<BYTE>(pVidMode->m_dwRBitShift);
    BYTE nGShift = static_cast<BYTE>(pVidMode->m_dwGBitShift);
    BYTE nBShift = static_cast<BYTE>(pVidMode->m_dwBBitShift);

    // NOTE: Uninline.
    CVidBitmapEffects effects;
    BltBmpGetEffects(this, effects, dwFlags);

    if ((dwFlags & 0x1) != 0) {
        COLORREF rgbShadow = pVidMode->ApplyBrightnessContrast(RGB(0, 0, 0));
        DWORD dwRBitMask = g_pChitin->GetCurrentVideoMode()->m_dwRBitMask;
        DWORD dwGBitMask = g_pChitin->GetCurrentVideoMode()->m_dwGBitMask;
        DWORD dwBBitMask = g_pChitin->GetCurrentVideoMode()->m_dwBBitMask;
        LONG nSurfaceJump = (lPitch >> 1) + bmpSize.cx;
        INT nDataIndex = 0;

        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                BYTE r = pData[nDataIndex + 2];
                BYTE g = pData[nDataIndex + 1];
                BYTE b = pData[nDataIndex];

                if (r != 0 || g != 255 || b != 0) {
                    if ((dwFlags & 0x4) != 0 && r == 0 && g == 0 && b == 0) {
                        // Blend the shadow with what's already there.
                        WORD nColor = *pSurface;
                        r = static_cast<BYTE>(static_cast<INT>(((((dwRBitMask & 0xFFFF) & nColor) >> nRShift) << nRDown) + GetRValue(rgbShadow)) * 128 >> 8);
                        g = static_cast<BYTE>(static_cast<INT>(((((dwGBitMask & 0xFFFF) & nColor) >> nGShift) << nGDown) + GetGValue(rgbShadow)) * 128 >> 8);
                        b = static_cast<BYTE>(static_cast<INT>(((((dwBBitMask & 0xFFFF) & nColor) >> nBShift) << nBDown) + GetBValue(rgbShadow)) * 128 >> 8);
                    } else {
                        BltBmpApplyEffects(r, g, b, dwFlags, effects);
                    }

                    *pSurface = static_cast<WORD>(((b >> nBDown) << nBShift)
                        | ((g >> nGDown) << nGShift)
                        | ((r >> nRDown) << nRShift));
                }

                pSurface++;
                nDataIndex += 3;
            }

            pSurface -= nSurfaceJump;
            nDataIndex += nDataJump;
        }
    } else {
        // NOTE: This path is the inline assembly at 0x7B3FF8 (Ghidra split it
        // off). Rows are `do`/`while` loops (no check for an empty bitmap).
        // Pixels are written in pairs as DWORDs: the first pixel of a pair is
        // the low word, and the upper 16 bits of its red channel register
        // are ORed into the high word (the second pixel). An odd row starts
        // with a single WORD pixel; a row of width 1 (only `cx & 0xFFFE` is
        // tested) is written one WORD pixel at a time.
        BYTE* pDest = reinterpret_cast<BYTE*>(pSurface);
        LONG nSurfaceJump = lPitch + 2 * bmpSize.cx;
        DWORD nWidth = bmpSize.cx;
        LONG nHeight = bmpSize.cy;
        DWORD nPairs;
        DWORD nCount;
        DWORD nColor1;
        DWORD nColor2;

        if ((nWidth & 0xFFFE) != 0) {
            if ((nWidth & 0x1) == 0) {
                nPairs = nWidth >> 1;
                do {
                    nCount = nPairs;
                    do {
                        nColor2 = BltBmpPack16(pData[5], pData[4], pData[3], dwFlags, effects, nRDown, nGDown, nBDown, nRShift, nGShift, nBShift);
                        nColor1 = BltBmpPack16(pData[2], pData[1], pData[0], dwFlags, effects, nRDown, nGDown, nBDown, nRShift, nGShift, nBShift);
                        *reinterpret_cast<DWORD*>(pDest) = nColor1 | ((nColor2 << 16) & 0xFFFF0000);

                        pData += 6;
                        pDest += 4;
                    } while (--nCount != 0);

                    pDest -= nSurfaceJump;
                    pData += nDataJump;
                } while (--nHeight != 0);
            } else {
                nPairs = nWidth >> 1;
                do {
                    nCount = nPairs;

                    nColor1 = BltBmpPack16(pData[2], pData[1], pData[0], dwFlags, effects, nRDown, nGDown, nBDown, nRShift, nGShift, nBShift);
                    pData += 3;
                    *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(nColor1);
                    pDest += 2;

                    do {
                        nColor2 = BltBmpPack16(pData[5], pData[4], pData[3], dwFlags, effects, nRDown, nGDown, nBDown, nRShift, nGShift, nBShift);
                        nColor1 = BltBmpPack16(pData[2], pData[1], pData[0], dwFlags, effects, nRDown, nGDown, nBDown, nRShift, nGShift, nBShift);
                        *reinterpret_cast<DWORD*>(pDest) = nColor1 | ((nColor2 << 16) & 0xFFFF0000);

                        pData += 6;
                        pDest += 4;
                    } while (--nCount != 0);

                    pDest -= nSurfaceJump;
                    pData += nDataJump;
                } while (--nHeight != 0);
            }
        } else {
            do {
                nColor1 = BltBmpPack16(pData[2], pData[1], pData[0], dwFlags, effects, nRDown, nGDown, nBDown, nRShift, nGShift, nBShift);
                pData += 3;
                *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(nColor1);
                pDest += 2;

                pDest -= nSurfaceJump;
                pData += nDataJump;
            } while (--nHeight != 0);
        }
    }

    return TRUE;
}

// NOTE: The original is inline assembly. Every pixel but the last in a row is
// written as a DWORD.
//
// 0x7CCB20
BOOL CVidBitmap::BltBmp8To24(BYTE* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    m_cPalette.SetPalette(pRes->GetColorTable(), pRes->GetColorCount(), CVidPalette::TYPE_RESOURCE);
    m_cPalette.Realize(rgbTempPal, 24, dwFlags | 0x800000, &m_paletteAffects, 255);

    LONG nSurfaceJump = lPitch + 3 * bmpSize.cx;

    for (LONG y = 0; y < bmpSize.cy; y++) {
        for (LONG x = 0; x < bmpSize.cx - 1; x++) {
            *reinterpret_cast<DWORD*>(pSurface) = rgbTempPal[*pData++];
            pSurface += 3;
        }

        DWORD color = rgbTempPal[*pData++];
        *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
        pSurface[2] = static_cast<BYTE>(color >> 16);
        pSurface += 3;

        pSurface -= nSurfaceJump;
        pData += nDataJump;
    }

    return TRUE;
}

// NOTE: The non-transparent path of the original is inline assembly. Every
// pixel but the last in a row is written as a DWORD.
//
// 0x7CCBF0
BOOL CVidBitmap::BltBmp24To24(BYTE* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();

    BYTE nRShift = static_cast<BYTE>(pVidMode->m_dwRBitShift);
    BYTE nGShift = static_cast<BYTE>(pVidMode->m_dwGBitShift);
    BYTE nBShift = static_cast<BYTE>(pVidMode->m_dwBBitShift);

    LONG nSurfaceJump = lPitch + 3 * bmpSize.cx;

    if ((dwFlags & 0x1) != 0) {
        INT nDataIndex = 0;

        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx - 1; x++) {
                BYTE r = pData[nDataIndex + 2];
                BYTE g = pData[nDataIndex + 1];
                BYTE b = pData[nDataIndex];

                if (r != 0 || g != 255 || b != 0) {
                    if ((dwFlags & 0x4) != 0 && r == 0 && g == 0 && b == 0) {
                        DWORD nColor = *reinterpret_cast<DWORD*>(pSurface);
                        r = static_cast<BYTE>((nColor >> nRShift) >> 1) & 0x7F;
                        g = static_cast<BYTE>((nColor >> nGShift) >> 1) & 0x7F;
                        b = static_cast<BYTE>((nColor >> nBShift) >> 1) & 0x7F;
                    }

                    *reinterpret_cast<DWORD*>(pSurface) = (b << nBShift) | (g << nGShift) | (r << nRShift);
                }

                pSurface += 3;
                nDataIndex += 3;
            }

            BYTE r = pData[nDataIndex + 2];
            BYTE g = pData[nDataIndex + 1];
            BYTE b = pData[nDataIndex];

            if (r != 0 || g != 255 || b != 0) {
                if ((dwFlags & 0x4) != 0 && r == 0 && g == 0 && b == 0) {
                    // NOTE: Assumes blue is the high byte.
                    DWORD nColor = *reinterpret_cast<WORD*>(pSurface);
                    r = static_cast<BYTE>((nColor >> nRShift) >> 1) & 0x7F;
                    g = static_cast<BYTE>((nColor >> nGShift) >> 1) & 0x7F;
                    b = pSurface[2] >> 1;
                }

                DWORD color = (b << nBShift) | (g << nGShift) | (r << nRShift);
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                pSurface[2] = static_cast<BYTE>(color >> 16);
            }

            nDataIndex += 3 + nDataJump;
            pSurface += 3 - nSurfaceJump;
        }
    } else {
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx - 1; x++) {
                *reinterpret_cast<DWORD*>(pSurface) = (pData[2] << nRShift) | (pData[1] << nGShift) | (pData[0] << nBShift);
                pData += 3;
                pSurface += 3;
            }

            DWORD color = (pData[2] << nRShift) | (pData[1] << nGShift) | (pData[0] << nBShift);
            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
            pSurface[2] = static_cast<BYTE>(color >> 16);
            pData += 3;
            pSurface += 3;

            pSurface -= nSurfaceJump;
            pData += nDataJump;
        }
    }

    return TRUE;
}

// NOTE: The non-transparent path of the original is inline assembly. Every
// pixel but the last in a row is written as a DWORD.
//
// 0x7CCE80
BOOL CVidBitmap::BltBmp24To24Tint(BYTE* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();

    BYTE nRShift = static_cast<BYTE>(pVidMode->m_dwRBitShift);
    BYTE nGShift = static_cast<BYTE>(pVidMode->m_dwGBitShift);
    BYTE nBShift = static_cast<BYTE>(pVidMode->m_dwBBitShift);

    // NOTE: Uninline.
    CVidBitmapEffects effects;
    BltBmpGetEffects(this, effects, dwFlags);

    LONG nSurfaceJump = lPitch + 3 * bmpSize.cx;

    if ((dwFlags & 0x1) != 0) {
        COLORREF rgbShadow = pVidMode->ApplyBrightnessContrast(RGB(0, 0, 0));
        INT nDataIndex = 0;

        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx - 1; x++) {
                BYTE r = pData[nDataIndex + 2];
                BYTE g = pData[nDataIndex + 1];
                BYTE b = pData[nDataIndex];

                if (r != 0 || g != 255 || b != 0) {
                    if ((dwFlags & 0x4) != 0 && r == 0 && g == 0 && b == 0) {
                        // Blend the shadow with what's already there.
                        DWORD nColor = *reinterpret_cast<DWORD*>(pSurface);
                        r = static_cast<BYTE>((((nColor >> nRShift) & 0xFF) + GetRValue(rgbShadow)) * 128 >> 8);
                        g = static_cast<BYTE>((((nColor >> nGShift) & 0xFF) + GetGValue(rgbShadow)) * 128 >> 8);
                        b = static_cast<BYTE>((((nColor >> nBShift) & 0xFF) + GetBValue(rgbShadow)) * 128 >> 8);
                    } else {
                        BltBmpApplyEffects(r, g, b, dwFlags, effects);
                    }

                    *reinterpret_cast<DWORD*>(pSurface) = (b << nBShift) | (g << nGShift) | (r << nRShift);
                }

                pSurface += 3;
                nDataIndex += 3;
            }

            BYTE r = pData[nDataIndex + 2];
            BYTE g = pData[nDataIndex + 1];
            BYTE b = pData[nDataIndex];

            if (r != 0 || g != 255 || b != 0) {
                if ((dwFlags & 0x4) != 0 && r == 0 && g == 0 && b == 0) {
                    // NOTE: Assumes blue is the high byte.
                    DWORD nColor = *reinterpret_cast<WORD*>(pSurface);
                    r = static_cast<BYTE>((((nColor >> nRShift) & 0xFF) + GetRValue(rgbShadow)) * 128 >> 8);
                    g = static_cast<BYTE>((((nColor >> nGShift) & 0xFF) + GetGValue(rgbShadow)) * 128 >> 8);
                    b = static_cast<BYTE>((pSurface[2] + GetBValue(rgbShadow)) * 128 >> 8);
                } else {
                    BltBmpApplyEffects(r, g, b, dwFlags, effects);
                }

                DWORD color = (b << nBShift) | (g << nGShift) | (r << nRShift);
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                pSurface[2] = static_cast<BYTE>(color >> 16);
            }

            nDataIndex += 3 + nDataJump;
            pSurface += 3 - nSurfaceJump;
        }
    } else {
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx - 1; x++) {
                DWORD r = pData[2];
                DWORD g = pData[1];
                DWORD b = pData[0];
                BltBmpApplyEffects(r, g, b, dwFlags, effects);

                *reinterpret_cast<DWORD*>(pSurface) = (r << nRShift) | (g << nGShift) | (b << nBShift);
                pData += 3;
                pSurface += 3;
            }

            DWORD r = pData[2];
            DWORD g = pData[1];
            DWORD b = pData[0];
            BltBmpApplyEffects(r, g, b, dwFlags, effects);

            DWORD color = (r << nRShift) | (g << nGShift) | (b << nBShift);
            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
            pSurface[2] = static_cast<BYTE>(color >> 16);
            pData += 3;
            pSurface += 3;

            pSurface -= nSurfaceJump;
            pData += nDataJump;
        }
    }

    return TRUE;
}

// 0x7CD5E0
BOOL CVidBitmap::BltBmp8To32(DWORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    m_cPalette.SetPalette(pRes->GetColorTable(), pRes->GetColorCount(), CVidPalette::TYPE_RESOURCE);
    m_cPalette.Realize(rgbTempPal, 32, dwFlags | 0x800000, &m_paletteAffects, 255);

    for (LONG y = 0; y < bmpSize.cy; y++) {
        for (LONG x = 0; x < bmpSize.cx; x++) {
            *pSurface++ = rgbTempPal[*pData++];
        }

        pSurface += -bmpSize.cx - lPitch / 4;
        pData += nDataJump;
    }

    return TRUE;
}

// 0x7CD690
BOOL CVidBitmap::BltBmp24To32(DWORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();

    DWORD dwRBitShift = pVidMode->m_dwRBitShift;
    DWORD dwGBitShift = pVidMode->m_dwGBitShift;
    DWORD dwBBitShift = pVidMode->m_dwBBitShift;

    if ((dwFlags & 0x1) != 0) {
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                BYTE r = pData[2];
                BYTE g = pData[1];
                BYTE b = pData[0];
                BYTE a;

                if (r != 0 || g != 255 || b != 0) {
                    if ((dwFlags & 0x4) == 0 || r != 0 || g != 0 || b != 0) {
                        a = 255;
                    } else {
                        a = 128;
                        r = ((*pSurface >> dwRBitShift) >> 1) & 0x7F;
                        g = ((*pSurface >> dwGBitShift) >> 1) & 0x7F;
                        b = ((*pSurface >> dwBBitShift) >> 1) & 0x7F;
                    }

                    *pSurface = (a << 24) | (r << dwRBitShift) | (g << dwGBitShift) | (b << dwBBitShift);
                }

                pSurface++;
                pData += 3;
            }

            pSurface += -bmpSize.cx - lPitch / 4;
            pData += nDataJump;
        }
    } else {
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                *pSurface++ = (pData[0] << dwBBitShift) | (pData[1] << dwGBitShift) | (pData[2] << dwRBitShift);
                pData += 3;
            }

            pSurface += -bmpSize.cx - lPitch / 4;
            pData += nDataJump;
        }
    }

    return TRUE;
}

/// NOTE: The non-transparent path of the original is inline assembly.
//
// 0x7CD870
BOOL CVidBitmap::BltBmp24To32Tint(DWORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();

    BYTE nRShift = static_cast<BYTE>(pVidMode->m_dwRBitShift);
    BYTE nGShift = static_cast<BYTE>(pVidMode->m_dwGBitShift);
    BYTE nBShift = static_cast<BYTE>(pVidMode->m_dwBBitShift);

    // NOTE: Uninline.
    CVidBitmapEffects effects;
    BltBmpGetEffects(this, effects, dwFlags);

    if ((dwFlags & 0x1) != 0) {
        COLORREF rgbShadow = pVidMode->ApplyBrightnessContrast(RGB(0, 0, 0));
        LONG nSurfaceJump = (lPitch >> 2) + bmpSize.cx;
        INT nDataIndex = 0;

        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                BYTE r = pData[nDataIndex + 2];
                BYTE g = pData[nDataIndex + 1];
                BYTE b = pData[nDataIndex];

                if (r != 0 || g != 255 || b != 0) {
                    BYTE a;
                    if ((dwFlags & 0x4) != 0 && r == 0 && g == 0 && b == 0) {
                        // Blend the shadow with what's already there.
                        a = 128;

                        DWORD nColor = *pSurface;
                        r = static_cast<BYTE>((((nColor >> nRShift) & 0xFF) + GetRValue(rgbShadow)) * 128 >> 8);
                        g = static_cast<BYTE>((((nColor >> nGShift) & 0xFF) + GetGValue(rgbShadow)) * 128 >> 8);
                        b = static_cast<BYTE>((((nColor >> nBShift) & 0xFF) + GetBValue(rgbShadow)) * 128 >> 8);
                    } else {
                        a = 255;
                        BltBmpApplyEffects(r, g, b, dwFlags, effects);
                    }

                    *pSurface = (b << nBShift) | (g << nGShift) | (r << nRShift) | (a << 24);
                }

                pSurface++;
                nDataIndex += 3;
            }

            pSurface -= nSurfaceJump;
            nDataIndex += nDataJump;
        }
    } else {
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                DWORD r = pData[2];
                DWORD g = pData[1];
                DWORD b = pData[0];
                BltBmpApplyEffects(r, g, b, dwFlags, effects);

                *pSurface++ = (r << nRShift) | (g << nGShift) | (b << nBShift);
                pData += 3;
            }

            pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pSurface) - (lPitch + 4 * bmpSize.cx));
            pData += nDataJump;
        }
    }

    return TRUE;
}

// 0x7C6800
void CVidBitmap::RenderTexture(INT x, INT y, const CSize& bmpSize, const CRect& rClip, DWORD dwFlags)
{
    if (x + bmpSize.cx < rClip.left
        || x > rClip.right
        || y + bmpSize.cy < rClip.top
        || y > rClip.bottom) {
        return;
    }

    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    float x4;
    float y4;

    x1 = static_cast<float>(x);
    y1 = static_cast<float>(y);
    x2 = static_cast<float>(x + bmpSize.cx);
    y2 = static_cast<float>(y + bmpSize.cy);
    x3 = 0.0f;
    y3 = 0.0f;
    x4 = static_cast<float>(bmpSize.cx) / static_cast<float>(CVidInf::FX_WIDTH);
    y4 = static_cast<float>(bmpSize.cy) / static_cast<float>(CVidInf::FX_HEIGHT);

    if (x < rClip.left) {
        x1 = static_cast<float>(rClip.left);
        x3 = static_cast<float>(rClip.left - x) / static_cast<float>(CVidInf::FX_WIDTH);
    }

    if (y < rClip.top) {
        y1 = static_cast<float>(rClip.top);
        y3 = static_cast<float>(rClip.top - y) / static_cast<float>(CVidInf::FX_HEIGHT);
    }

    if (x + bmpSize.cx > rClip.right) {
        x2 = static_cast<float>(rClip.right);
        x4 = static_cast<float>(rClip.right - x) / static_cast<float>(CVidInf::FX_WIDTH);
    }

    if (y + bmpSize.cy > rClip.bottom) {
        y2 = static_cast<float>(rClip.bottom);
        y4 = static_cast<float>(rClip.bottom - y) / static_cast<float>(CVidInf::FX_HEIGHT);
    }

    CVideo3d::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x1) != 0) {
        CVideo3d::glEnable(GL_BLEND);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }

    x1 += CVideo3d::SUB_PIXEL_SHIFT;
    y1 += CVideo3d::SUB_PIXEL_SHIFT;
    x2 += CVideo3d::SUB_PIXEL_SHIFT;
    y2 += CVideo3d::SUB_PIXEL_SHIFT;

    CVideo3d::glBegin(GL_TRIANGLE_STRIP);
    CVideo3d::glTexCoord2f(x3, y3);
    CVideo3d::glVertex3f(x1, y1, 0.0f);
    CVideo3d::glTexCoord2f(x3, y4);
    CVideo3d::glVertex3f(x1, y2, 0.0f);
    CVideo3d::glTexCoord2f(x4, y3);
    CVideo3d::glVertex3f(x2, y1, 0.0f);
    CVideo3d::glTexCoord2f(x4, y4);
    CVideo3d::glVertex3f(x2, y2, 0.0f);
    CVideo3d::glEnd();
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x1) != 0) {
        CVideo3d::glDisable(GL_BLEND);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }
}

// 0x7C6B80
BOOL CVidBitmap::Render3d(INT x, INT y, const CRect& rClip, DWORD dwFlags, BOOLEAN bDemanded)
{
    if (pRes == NULL || rClip.IsRectEmpty()) {
        return FALSE;
    }

    if (!bDemanded) {
        pRes->Demand();
    }

    BYTE* pData = pRes->GetImageData(m_bDoubleSize);
    m_nBitCount = pRes->GetBitDepth();

    CSize bmpSize;
    GetImageDimensions(bmpSize, m_bDoubleSize);

    INT nBytesPerPixel;
    switch (m_nBitCount) {
    case 8:
        nBytesPerPixel = 1;
        break;
    case 24:
        nBytesPerPixel = 3;
        break;
    default:
        if (!bDemanded) {
            pRes->Release();
        }
        return FALSE;
    }

    LONG nDataJump = (-((bmpSize.cx * nBytesPerPixel) & 3)) & 3;

    CVideo3d::glEnable(GL_TEXTURE_2D);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    g_pChitin->cVideo.field_13E = 2;
    CVideo3d::glBindTexture(GL_TEXTURE_2D, 2);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CSize tileSize;
    LONG lTilePitch;
    LONG nTileDataJump = 0;

    BYTE* pTileRowData = pData;
    for (INT yOffset = 0; yOffset < bmpSize.cy; yOffset += CVidInf::FX_HEIGHT) {
        BYTE* pTileData = pTileRowData;
        for (INT xOffset = 0; xOffset < bmpSize.cx; xOffset += CVidInf::FX_WIDTH) {
            if ((dwFlags & 0x1) != 0) {
                CVidInf::FXClear(CVideo3d::texImageData, CVidInf::FX_WIDTH * CVidInf::FX_HEIGHT);
            }

            tileSize.cx = min(bmpSize.cx - xOffset, CVidInf::FX_WIDTH);
            tileSize.cy = min(bmpSize.cy - yOffset, CVidInf::FX_HEIGHT);

            if (bmpSize.cx > tileSize.cx) {
                nTileDataJump = nBytesPerPixel * (bmpSize.cx - tileSize.cx);
            }

            lTilePitch = g_pChitin->cVideo.field_13A
                ? CVidTile::BYTES_PER_TEXEL * CVIDINF_FX_WIDTH
                : CVidTile::BYTES_PER_TEXEL * tileSize.cx;

            BOOLEAN bResult = TRUE;
            switch (m_nBitCount) {
            case 8:
                bResult = BltBmp8To32(&(CVideo3d::texImageData[lTilePitch * (tileSize.cy - 1) / CVidTile::BYTES_PER_TEXEL]),
                    lTilePitch,
                    pTileData,
                    tileSize,
                    nDataJump + nTileDataJump,
                    dwFlags);
                break;
            case 24:
                if ((dwFlags & 0xFFFF0000) != 0
                    && g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(RGB(128, 128, 128)) != RGB(128, 128, 128)) {
                    bResult = BltBmp24To32Tint(&(CVideo3d::texImageData[lTilePitch * (tileSize.cy - 1) / CVidTile::BYTES_PER_TEXEL]),
                        lTilePitch,
                        pTileData,
                        tileSize,
                        nDataJump + nTileDataJump,
                        dwFlags);
                } else {
                    bResult = BltBmp24To32(&(CVideo3d::texImageData[(lTilePitch * (tileSize.cy - 1) / CVidTile::BYTES_PER_TEXEL)]),
                        lTilePitch,
                        pTileData,
                        tileSize,
                        nDataJump + nTileDataJump,
                        dwFlags);
                }
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
                // __LINE__: 2120
                UTIL_ASSERT(FALSE);
            }

            if (!bResult) {
                if (bDemanded) {
                    pRes->Release();
                }
                return FALSE;
            }

            if (g_pChitin->cVideo.field_13A) {
                CVideo3d::glTexImage2D(GL_TEXTURE_2D,
                    0,
                    GL_RGBA,
                    CVidInf::FX_WIDTH,
                    CVidInf::FX_HEIGHT,
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    CVideo3d::texImageData);
                g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
            } else {
                CVideo3d::glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    tileSize.cx,
                    tileSize.cy,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    CVideo3d::texImageData);
                g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
            }

            RenderTexture(x + xOffset,
                y + bmpSize.cy - tileSize.cy - yOffset,
                tileSize,
                rClip,
                dwFlags);

            pTileData += CVidInf::FX_WIDTH * nBytesPerPixel;
        }
        pTileRowData += CVidInf::FX_HEIGHT * (nDataJump + bmpSize.cx * nBytesPerPixel);
    }

    if (!bDemanded) {
        pRes->Release();
    }

    return TRUE;
}

// 0x7C7070
BOOL CVidBitmap::RenderDirect3d(INT x, INT y, const CRect& rClip, DWORD dwFlags, BOOLEAN bDemanded)
{
    return Render3d(x, y, rClip, dwFlags, bDemanded);
}
