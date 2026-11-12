#include "CVidMode.h"

#include "CChitin.h"
#include "CUtil.h"
#include "CVidCell.h"
#include "CVideo3d.h"

typedef struct CVIDMODE_CLIP_OUTCODE {
    unsigned int all;
    unsigned int left : 4;
    unsigned int right : 4;
    unsigned int bottom : 4;
    unsigned int top : 4;
} CVIDMODE_CLIP_OUTCODE;

// 0x8FB964
const CString CVidMode::SCREEN_SHOT_EXTENSION("bmp");

// 0x8FB95C
const CString CVidMode::SCREEN_SHOT_DIR_NAME(".\\ScrnShot");

// 0x85DE38
const BYTE CVidMode::byte_85DE38 = 0x40;

// 0x85DE3A
const BYTE CVidMode::byte_85DE3A = 6;

// 0x8BA320
short CVidMode::word_8BA320 = 100;

// 0x8BA322
BYTE CVidMode::NUM_FADE_FRAMES = 20;

// #guess
// 0x8FB968
BOOL CVidMode::bInitialized;

// 0x799C70
BOOLEAN CVidMode::SaveScreen()
{
    return FALSE;
}

// 0x799C80
BOOL CVidMode::RenderPointer()
{
    return TRUE;
}

// #binary-identical
// 0x799CB0
CVidMode::CVidMode()
{
    m_dwRBitMask = 0;
    m_dwGBitMask = 0;
    m_dwBBitMask = 0;
    field_24 = 0;
    field_98 = 0;
    field_C2 = 0;
    field_C6 = 0;
    field_CA = 0;
    field_D2 = 0;
    field_DC = 0;
    m_bPointerEnabled = FALSE;
    m_bPointerInside = TRUE;
    m_nColorDepth = 0;
    field_E0 = 0;
    m_nSurfaces = 0;
    nTickCount = GetTickCount();
    field_94 = 0;
    m_pPointerVidCell = NULL;
    m_bPointerAnimating = FALSE;
    pSurfaces = NULL;
    m_rPointerStorage.left = 0;
    m_rPointerStorage.top = 0;
    m_rPointerStorage.right = 0;
    m_rPointerStorage.bottom = 0;
    rgbGlobalTint = 0xFFFFFF;
    m_nPrintFile = 0;
    m_bPrintScreen = FALSE;
    m_nBrightnessCorrection = 0;
    m_nGammaCorrection = 0;
    m_nPointerNumber = 0;
    m_dwCursorRenderFlags = 0;
    field_10 = 1;
    field_14 = 0;
    m_bFadeTo = 0;
    m_nFade = NUM_FADE_FRAMES;

    bInitialized = TRUE;
}

// 0x799E10
BOOLEAN CVidMode::SetClipper(IDirectDrawClipper* lpDirectDrawClipper)
{
    return FALSE;
}

// #binary-identical
// 0x799EB0
CVidMode::~CVidMode()
{
    bInitialized = FALSE;
}

// #binary-identical
// 0x799E50
COLORREF CVidMode::GetTransparentColor()
{
    return 65280;
}

// #binary-identical
// 0x799E70
void CVidMode::PrintScreen()
{
    m_bPrintScreen = TRUE;
}

// 0x799E80
int CVidMode::GetPointerNumber()
{
    return m_nPointerNumber;
}

// #binary-identical
// 0x799F40
BOOL CVidMode::CheckResults(HRESULT hr)
{
    if (hr != DD_OK) {
        switch (hr) {
        case DDERR_SURFACELOST:
            RestoreSurfaces();
            break;
        }

        return FALSE;
    }

    return TRUE;
}

// 0x799F70
COLORREF CVidMode::GetGlobalTintColor()
{
    return rgbGlobalTint;
}

// 0x795980
BOOL CVidMode::ClipLine(INT& nXFrom, INT& nYFrom, INT& nXTo, INT& nYTo, const CRect& rClip)
{
    CVIDMODE_CLIP_OUTCODE fromCode;
    CVIDMODE_CLIP_OUTCODE toCode;
    CVIDMODE_CLIP_OUTCODE finalCode;
    INT x;
    INT y;

    while (1) {
        memset(&fromCode, 0, sizeof(fromCode));
        memset(&toCode, 0, sizeof(toCode));

        if (nYFrom < rClip.top) {
            fromCode.top |= 1;
            fromCode.all += 8;
        } else if (nYFrom > rClip.bottom) {
            fromCode.bottom |= 1;
            fromCode.all += 4;
        }

        if (nXFrom < rClip.left) {
            fromCode.left |= 1;
            fromCode.all += 1;
        } else if (nXFrom > rClip.right) {
            fromCode.right |= 1;
            fromCode.all += 2;
        }

        if (nYTo < rClip.top) {
            toCode.top |= 1;
            toCode.all += 8;
        } else if (nYTo > rClip.bottom) {
            toCode.bottom |= 1;
            toCode.all += 4;
        }

        if (nXTo < rClip.left) {
            toCode.left |= 1;
            toCode.all += 1;
        } else if (nXTo > rClip.right) {
            toCode.right |= 1;
            toCode.all += 2;
        }

        if (fromCode.all == 0 && toCode.all == 0) {
            return TRUE;
        }

        if ((fromCode.all & toCode.all) != 0) {
            return FALSE;
        }

        finalCode = fromCode.all != 0 ? fromCode : toCode;

        if (finalCode.top) {
            y = rClip.top;
            x = nXFrom + (nXTo - nXFrom) * (y - nYFrom) / (nYTo - nYFrom);
        } else if (finalCode.bottom) {
            y = rClip.bottom;
            x = nXFrom + (nXTo - nXFrom) * (y - nYFrom) / (nYTo - nYFrom);
        } else if (finalCode.right) {
            x = rClip.right;
            y = nYFrom + (nYTo - nYFrom) * (x - nXFrom) / (nXTo - nXFrom);
        } else if (finalCode.left) {
            x = rClip.left;
            y = nYFrom + (nYTo - nYFrom) * (x - nXFrom) / (nXTo - nXFrom);
        }

        if (finalCode.all == fromCode.all) {
            nXFrom = x;
            nYFrom = y;
        } else {
            nXTo = x;
            nYTo = y;
        }
    }
}

// #binary-identical
// 0x795BB0
BOOL CVidMode::CreateSurface(IDirectDrawSurface** lplpDirectDrawSurface, const CSize& cSize)
{
    DDCOLORKEY ddck;
    ddck.dwColorSpaceLowValue = m_dwGBitMask;
    ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;

    DDSURFACEDESC ddsd = { 0 };
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDCAPS_BLT;
    ddsd.dwWidth = cSize.cx;
    ddsd.dwHeight = cSize.cy;

    if (g_pChitin->cVideo.m_pDirectDraw2->CreateSurface(&ddsd, lplpDirectDrawSurface, NULL) != DD_OK) {
        *lplpDirectDrawSurface = NULL;
        return FALSE;
    }

    (*lplpDirectDrawSurface)->SetColorKey(DDCKEY_SRCBLT, &ddck);

    return TRUE;
}

// 0x795C50
void CVidMode::DestroySurface(IDirectDrawSurface** lplpDirectDrawSurface)
{
    if (lplpDirectDrawSurface != NULL) {
        if (*lplpDirectDrawSurface != NULL) {
            (*lplpDirectDrawSurface)->Release();
        }
    }
}

// 0x795C70
void CVidMode::DisplayFrameRate(UINT nSurface)
{
    if (g_pChitin->field_1A2A) {
        CString sFrameRate;
        sFrameRate.Format("%02d %02d %d ",
            g_pChitin->m_nRenderPerSec,
            g_pChitin->m_nAIPerSec,
            GetNumVRamSurfaces());

        CString v3;
        DoTextOut(nSurface, sFrameRate, (CVideo::SCREENWIDTH >> 1) - 240, 4, 255);
    }
}

// 0x795D50
DWORD CVidMode::ConvertToSurfaceRGB(COLORREF rgb)
{
    if (g_pChitin->cVideo.m_nBpp == 16) {
        return ((GetRValue(rgb) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgb) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgb) >> field_CA) << m_dwBBitShift);
    } else {
        return (GetRValue(rgb) << m_dwRBitShift) | (GetGValue(rgb) << m_dwGBitShift) | (GetBValue(rgb) << m_dwBBitShift);
    }
}

// 0x795E00
COLORREF CVidMode::ApplyBrightnessContrast(COLORREF rgb)
{
    if (m_nBrightnessCorrection != 0) {
        BYTE v1 = ~m_nBrightnessCorrection;
        rgb = RGB(~((v1 * ~GetRValue(rgb)) >> 8),
            ~((v1 * ~GetGValue(rgb)) >> 8),
            ~((v1 * ~GetBValue(rgb)) >> 8));
    }

    if (m_nGammaCorrection != 0) {
        BYTE v2 = m_nGammaCorrection + (1 << CVidPalette::LIGHT_SCALE);
        rgb = RGB(min((v2 * GetRValue(rgb)) >> CVidPalette::LIGHT_SCALE, 255),
            min((v2 * GetGValue(rgb)) >> CVidPalette::LIGHT_SCALE, 255),
            min((v2 * GetBValue(rgb)) >> CVidPalette::LIGHT_SCALE, 255));
    }

    return rgb;
}

// 0x795F10
void CVidMode::ApplyFadeAmount(CVIDPALETTE_COLOR* pColor)
{
    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return;
    }

    pColor->rgbRed = pColor->rgbRed * m_nFade / NUM_FADE_FRAMES;

    // NOTE: Strange check for 3D, probably result of some inlining
    pColor->rgbGreen = g_pChitin->cVideo.m_bIs3dAccelerated
        ? pColor->rgbGreen
        : pColor->rgbGreen * m_nFade / NUM_FADE_FRAMES;

    // NOTE: Same as above.
    pColor->rgbBlue = g_pChitin->cVideo.m_bIs3dAccelerated
        ? pColor->rgbBlue
        : pColor->rgbBlue * m_nFade / NUM_FADE_FRAMES;
}

// 0x795FB0
COLORREF CVidMode::ApplyFadeAmount(COLORREF rgb)
{
    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return rgb;
    }

    return RGB(GetRValue(rgb) * m_nFade / NUM_FADE_FRAMES,
        GetGValue(rgb) * m_nFade / NUM_FADE_FRAMES,
        GetBValue(rgb) * m_nFade / NUM_FADE_FRAMES);
}

// 0x796020
INT CVidMode::ApplyFadeAmount(INT nColor)
{
    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return nColor;
    }

    return nColor * m_nFade / NUM_FADE_FRAMES;
}

// #binary-identical
// 0x796050
BOOL CVidMode::DrawEllipse(const CPoint& ptCenter, const CSize& axis, const CRect& rClip, COLORREF rgbColor)
{
    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return DrawEllipse3d(ptCenter, axis, rClip, rgbColor);
    }

    CRect r;
    r.SetRect(ptCenter.x - axis.cx - 1,
        ptCenter.y - axis.cy - 1,
        ptCenter.x + axis.cx + 1,
        ptCenter.y + axis.cy + 1);

    CRect rClipped;
    if (rClipped.IntersectRect(&r, rClip)) {
        DDSURFACEDESC ddsd;
        ddsd.dwSize = sizeof(ddsd);

        if (!LockSurface(0, &ddsd, rClipped)) {
            return FALSE;
        }

        CRect* pTemp;
        CRect rTemp;
        if (rClip.EqualRect(r)) {
            pTemp = NULL;
        } else {
            // NOTE: Looks like inlined OffsetRect.
            rTemp.left = rClipped.left - r.left;
            rTemp.top = rClipped.top - r.top;
            rTemp.right = rClipped.right - r.left;
            rTemp.bottom = rClipped.bottom - r.top;
            pTemp = &rTemp;
        }

        DrawEllipse(CPoint(axis.cx, axis.cy), axis, ddsd, pTemp, rgbColor);

        // NOTE: Uninline.
        UnLockSurface(0, ddsd.lpSurface);
    }

    return TRUE;
}

// #binary-identical
// 0x7961D0
void CVidMode::DrawEllipse(const CPoint& ptCenter, const CSize& axis, const DDSURFACEDESC& ddsd, const CRect* rClip, COLORREF rgbColor)
{
    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        DrawEllipse16(ptCenter, axis, ddsd, rClip, rgbColor);
        break;
    case 24:
        DrawEllipse24(ptCenter, axis, ddsd, rClip, rgbColor);
        break;
    case 32:
        DrawEllipse32(ptCenter, axis, ddsd, rClip, rgbColor);
        break;
    }
}

// 0x796260
void CVidMode::DrawEllipse16(const CPoint& ptCenter, const CSize& axis, const DDSURFACEDESC& ddsd, const CRect* rClip, COLORREF rgbColor)
{
    BYTE* pPixelList = new BYTE[max(axis.cx, axis.cy)];
    LONG lPitch = ddsd.lPitch >> 1;
    WORD color = static_cast<WORD>(((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift)
        | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift)
        | ((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift));
    LONG nArcLength = GetEllipseArcPixelList(axis.cx, axis.cy, pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 1506
    UTIL_ASSERT(nArcLength <= max(axis.cx, axis.cy));

    DrawEllipseHOctant16(ptCenter.x,
        ptCenter.y - axis.cy,
        nArcLength,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseHOctant16(ptCenter.x + 1,
        ptCenter.y + pPixelList[0] - axis.cy,
        nArcLength - 1,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawEllipseHOctant16(ptCenter.x,
        ptCenter.y + axis.cy,
        nArcLength,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseHOctant16(ptCenter.x + 1,
        ptCenter.y - pPixelList[0] + axis.cy,
        nArcLength - 1,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    nArcLength = GetEllipseArcPixelList(axis.cy, axis.cx, pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 1529
    UTIL_ASSERT(nArcLength < max(axis.cx, axis.cy));

    DrawEllipseVOctant16(ptCenter.x - axis.cx,
        ptCenter.y,
        nArcLength,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList,
        rClip,
        color);

    DrawEllipseVOctant16(ptCenter.x + pPixelList[0] - axis.cx,
        ptCenter.y + 1,
        nArcLength - 1,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawEllipseVOctant16(ptCenter.x + axis.cx,
        ptCenter.y,
        nArcLength,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseVOctant16(ptCenter.x - pPixelList[0] + axis.cx,
        ptCenter.y + 1,
        nArcLength - 1,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList + 1,
        rClip,
        color);

    delete pPixelList;
}

// 0x796550
LONG CVidMode::DrawEllipseHOctant16(LONG x, LONG y, INT nArcLength, WORD* pSurface, LONG lPitch, BOOLEAN clockwise, BYTE* pPixelList, const CRect* rClip, WORD color)
{
    CHAR nStep;

    if (rClip != NULL) {
        if (lPitch < 0) {
            pSurface += x - (y - rClip->top) * lPitch - rClip->left;
            nStep = -1;
        } else {
            pSurface += (y - rClip->top) * lPitch - rClip->left + x;
            nStep = 1;
        }

        while (nArcLength != 0) {
            if (rClip->PtInRect({ x, y })) {
                *pSurface = color;
            }

            if (*pPixelList++ != 0) {
                pSurface += lPitch;
                y += nStep;
            }

            if (clockwise) {
                pSurface++;
                x++;
            } else {
                pSurface--;
                x--;
            }

            nArcLength--;
        }
    } else {
        if (lPitch < 0) {
            pSurface += x - y * lPitch;
            nStep = -1;
        } else {
            pSurface += y * lPitch + x;
            nStep = 1;
        }

        while (nArcLength != 0) {
            *pSurface = color;

            if (*pPixelList++ != 0) {
                pSurface += lPitch;
                y += nStep;
            }

            if (clockwise) {
                pSurface++;
            } else {
                pSurface--;
            }

            nArcLength--;
        }
    }

    return y;
}

// 0x7966B0
LONG CVidMode::DrawEllipseVOctant16(LONG x, LONG y, INT nArcLength, WORD* pSurface, LONG lPitch, BOOLEAN clockwise, BYTE* pPixelList, const CRect* rClip, WORD color)
{
    CHAR nStep;

    if (rClip != NULL) {
        if (lPitch < 0) {
            pSurface += x - (y - rClip->top) * lPitch - rClip->left;
            nStep = -1;
        } else {
            pSurface += (y - rClip->top) * lPitch - rClip->left + x;
            nStep = 1;
        }

        while (nArcLength != 0) {
            if (rClip->PtInRect({ x, y })) {
                *pSurface = color;
            }

            if (*pPixelList++ != 0) {
                if (clockwise) {
                    pSurface++;
                    x++;
                } else {
                    pSurface--;
                    x--;
                }
            }

            pSurface += lPitch;
            y += nStep;

            nArcLength--;
        }
    } else {
        if (lPitch < 0) {
            pSurface += x - y * lPitch;
        } else {
            pSurface += y * lPitch + x;
        }

        while (nArcLength != 0) {
            *pSurface = color;

            if (*pPixelList++ != 0) {
                if (clockwise) {
                    pSurface++;
                    x++;
                } else {
                    pSurface--;
                    x--;
                }
            }

            pSurface += lPitch;
            nArcLength--;
        }
    }

    return x;
}

// #binary-identical
// 0x7967F0
void CVidMode::DrawLightingEffect(const DDSURFACEDESC& ddsd, const CPoint& a3, const CSize& a4, unsigned long a5, unsigned long a6)
{
    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return;
    }

    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        DrawLightingEffect16(reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch >> 1,
            a3,
            a4,
            a5,
            a6);
        break;
    case 24:
        DrawLightingEffect24(reinterpret_cast<BYTE*>(ddsd.lpSurface),
            ddsd.lPitch,
            a3,
            a4,
            a5,
            a6);
        break;
    case 32:
        DrawLightingEffect32(reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch >> 2,
            a3,
            a4,
            a5,
            a6);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
        // __LINE__: 1826
        UTIL_ASSERT(FALSE);
        break;
    }
}

// NOTE: Inlined in `DrawLightingEffect16`. Scales each channel of `nPixel`
// by `n/64` and clamps it to the channel mask.
static inline WORD LightPixel16(WORD nPixel, BYTE nRed, BYTE nGreen, BYTE nBlue, DWORD dwRMask, DWORD dwGMask, DWORD dwBMask)
{
    DWORD r = ((dwRMask & nPixel) * nRed) >> 6;
    if (r >= dwRMask) {
        r = dwRMask;
    }

    DWORD g = ((dwGMask & nPixel) * nGreen) >> 6;
    if (g >= dwGMask) {
        g = dwGMask;
    }

    DWORD b = ((dwBMask & nPixel) * nBlue) >> 6;
    if (b >= dwBMask) {
        b = dwBMask;
    }

    return static_cast<WORD>((static_cast<WORD>(dwRMask) & r)
        | (static_cast<WORD>(dwBMask) & b)
        | (static_cast<WORD>(dwGMask) & g));
}

// 0x7968C0
void CVidMode::DrawLightingEffect16(WORD* pSurface, LONG lPitch, const CPoint& a4, const CSize& a5, unsigned long a6, unsigned long a7)
{
    BYTE* pPixelList = new BYTE[max(a5.cx, a5.cy)];

    if (a5.cy == 0) {
        // NOTE: Leaks `pPixelList`.
        return;
    }

    if (rgbGlobalTint != RGB(CVidPalette::NO_TINT, CVidPalette::NO_TINT, CVidPalette::NO_TINT)) {
        a7 = RGB((GetRValue(a7) * GetRValue(rgbGlobalTint)) >> 8,
            (GetGValue(a7) * GetGValue(rgbGlobalTint)) >> 8,
            (GetBValue(a7) * GetBValue(rgbGlobalTint)) >> 8);
    }

    INT nValue;

    nValue = GetRValue(a6) + ((255 - GetRValue(a7)) >> 1);
    BYTE nMaxRed = nValue <= 255 ? static_cast<BYTE>(nValue) : 255;

    nValue = GetGValue(a6) + ((255 - GetGValue(a7)) >> 1);
    BYTE nMaxGreen = nValue <= 255 ? static_cast<BYTE>(nValue) : 255;

    nValue = GetBValue(a6) + ((255 - GetBValue(a7)) >> 1);
    BYTE nMaxBlue = nValue <= 255 ? static_cast<BYTE>(nValue) : 255;

    nMaxRed = nMaxRed > 64 ? static_cast<BYTE>((nMaxRed - 64) / a5.cy * a5.cy) : 0;
    nMaxGreen = nMaxGreen > 64 ? static_cast<BYTE>((nMaxGreen - 64) / a5.cy * a5.cy) : 0;
    nMaxBlue = nMaxBlue > 64 ? static_cast<BYTE>((nMaxBlue - 64) / a5.cy * a5.cy) : 0;

    if (nMaxRed == 0 && nMaxGreen == 0 && nMaxBlue == 0) {
        // NOTE: Leaks `pPixelList`.
        return;
    }

    BYTE nRed;
    BYTE nGreen;
    BYTE nBlue;
    BYTE nRedDelta;
    BYTE nGreenDelta;
    BYTE nBlueDelta;
    BYTE nCurRed;
    BYTE nCurGreen;
    BYTE nCurBlue;
    WORD* pDest;
    WORD* p;
    INT nPairs;
    SHORT nPair;
    SHORT nPass;

    // Top and bottom caps, one horizontal span per arc step.
    LONG nArcLength = GetEllipseArcPixelList(a5.cx, a5.cy, pPixelList);
    pDest = pSurface + (a4.y - a5.cy) * lPitch + a4.x;

    BYTE nRedStep = static_cast<BYTE>(nMaxRed / a5.cy);
    BYTE nGreenStep = static_cast<BYTE>(nMaxGreen / a5.cy);
    BYTE nBlueStep = static_cast<BYTE>(nMaxBlue / a5.cy);

    for (nPass = 0; nPass < 2; nPass++) {
        nRed = nRedStep;
        nGreen = nGreenStep;
        nBlue = nBlueStep;

        for (INT nWidth = 1; nWidth - 1 < nArcLength; nWidth++) {
            if (pPixelList[nWidth - 1] != 0) {
                nRedDelta = static_cast<BYTE>(nRed / nWidth);
                nGreenDelta = static_cast<BYTE>(nGreen / nWidth);
                nBlueDelta = static_cast<BYTE>(nBlue / nWidth);

                nCurRed = 64;
                nCurGreen = 64;
                nCurBlue = 64;

                p = pDest;

                // Left half, brightening towards the center.
                if ((nWidth & 1) != 0) {
                    nCurRed = nRedDelta + 64;
                    nCurGreen = nGreenDelta + 64;
                    nCurBlue = nBlueDelta + 64;
                    *p = LightPixel16(*p, nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);
                    p++;
                }

                // NOTE: The original processes pixel pairs as a single DWORD.
                nPairs = nWidth >> 1;
                for (nPair = 0; nPair < nPairs; nPair++) {
                    nCurRed += nRedDelta;
                    nCurGreen += nGreenDelta;
                    nCurBlue += nBlueDelta;
                    p[0] = LightPixel16(p[0], nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);

                    nCurRed += nRedDelta;
                    nCurGreen += nGreenDelta;
                    nCurBlue += nBlueDelta;
                    p[1] = LightPixel16(p[1], nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);

                    p += 2;
                }

                // Right half, darkening away from the center.
                if (((nWidth - 1) & 1) != 0) {
                    nCurRed -= nRedDelta;
                    nCurGreen -= nGreenDelta;
                    nCurBlue -= nBlueDelta;
                    *p = LightPixel16(*p, nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);
                    p++;
                }

                nPairs = (nWidth - 1) >> 1;
                for (nPair = 0; nPair < nPairs; nPair++) {
                    nCurRed -= nRedDelta;
                    nCurGreen -= nGreenDelta;
                    nCurBlue -= nBlueDelta;
                    p[0] = LightPixel16(p[0], nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);

                    nCurRed -= nRedDelta;
                    nCurGreen -= nGreenDelta;
                    nCurBlue -= nBlueDelta;
                    p[1] = LightPixel16(p[1], nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);

                    p += 2;
                }

                nRed += nRedStep;
                nGreen += nGreenStep;
                nBlue += nBlueStep;

                pDest += lPitch;
            }

            pDest--;
        }

        if (nPass == 0) {
            pDest = pSurface + (a4.y + a5.cy) * lPitch + a4.x;
        }

        lPitch = -lPitch;
    }

    // Middle band, one horizontal span per row from the center outwards.
    nArcLength = GetEllipseArcPixelList(a5.cy, a5.cx, pPixelList);

    INT nWidth = a5.cx;
    INT nRight;
    pDest = pSurface + a4.y * lPitch - a5.cx + a4.x;

    for (nPass = 0; nPass < 2; nPass++) {
        nRed = nMaxRed;
        nGreen = nMaxGreen;
        nBlue = nMaxBlue;

        if (nArcLength > 0) {
            nRight = nWidth - 1;

            for (INT nRow = 0; nRow < nArcLength; nRow++) {
                nRedDelta = static_cast<BYTE>(nRed / nWidth);
                nGreenDelta = static_cast<BYTE>(nGreen / nWidth);
                nBlueDelta = static_cast<BYTE>(nBlue / nWidth);

                nCurRed = 64;
                nCurGreen = 64;
                nCurBlue = 64;

                p = pDest;

                if ((nWidth & 1) != 0) {
                    nCurRed = nRedDelta + 64;
                    nCurGreen = nGreenDelta + 64;
                    nCurBlue = nBlueDelta + 64;
                    *p = LightPixel16(*p, nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);
                    p++;
                }

                nPairs = nWidth >> 1;
                for (nPair = 0; nPair < nPairs; nPair++) {
                    nCurRed += nRedDelta;
                    nCurGreen += nGreenDelta;
                    nCurBlue += nBlueDelta;
                    p[0] = LightPixel16(p[0], nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);

                    nCurRed += nRedDelta;
                    nCurGreen += nGreenDelta;
                    nCurBlue += nBlueDelta;
                    p[1] = LightPixel16(p[1], nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);

                    p += 2;
                }

                if ((nRight & 1) != 0) {
                    nCurRed -= nRedDelta;
                    nCurGreen -= nGreenDelta;
                    nCurBlue -= nBlueDelta;
                    *p = LightPixel16(*p, nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);
                    p++;
                }

                nPairs = nRight >> 1;
                for (nPair = 0; nPair < nPairs; nPair++) {
                    nCurRed -= nRedDelta;
                    nCurGreen -= nGreenDelta;
                    nCurBlue -= nBlueDelta;
                    p[0] = LightPixel16(p[0], nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);

                    nCurRed -= nRedDelta;
                    nCurGreen -= nGreenDelta;
                    nCurBlue -= nBlueDelta;
                    p[1] = LightPixel16(p[1], nCurRed, nCurGreen, nCurBlue, m_dwRBitMask, m_dwGBitMask, m_dwBBitMask);

                    p += 2;
                }

                if (pPixelList[nRow] != 0) {
                    pDest++;
                    nWidth--;
                    nRight--;
                }

                pDest -= lPitch;

                nRed -= nRedStep;
                nGreen -= nGreenStep;
                nBlue -= nBlueStep;
            }
        }

        if (nPass == 0) {
            pDest = pSurface + (a4.y + 1) * lPitch - a5.cx + pPixelList[0] + a4.x;
            nWidth = a5.cx - pPixelList[0];
            pPixelList++;
            nArcLength--;
            lPitch = -lPitch;
        }
    }

    delete (pPixelList - 1);
}

// 0x7977D0
BOOL CVidMode::DrawPoints(CPoint* pPoints, INT nPoints, WORD* pSurface, LONG lPitch, const CRect& rSurface, COLORREF rgbColor, BOOL bClipped)
{
    DWORD color = ReduceColor(rgbColor);

    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        if (bClipped) {
            while (nPoints > 0) {
                pSurface[pPoints->y * (lPitch >> 1) + pPoints->x] = static_cast<WORD>(color);
                pPoints++;
                nPoints--;
            }
        } else {
            while (nPoints > 0) {
                if (rSurface.PtInRect(*pPoints)) {
                    pSurface[pPoints->y * (lPitch >> 1) + pPoints->x] = static_cast<WORD>(color);
                }
                pPoints++;
                nPoints--;
            }
        }
        break;
    case 24:
        if (bClipped) {
            while (nPoints > 0) {
                BYTE* pDest = reinterpret_cast<BYTE*>(pSurface) + pPoints->y * lPitch + pPoints->x * 3;
                *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(color);
                pDest[2] = static_cast<BYTE>(color >> 16);
                pPoints++;
                nPoints--;
            }
        } else {
            while (nPoints > 0) {
                if (rSurface.PtInRect(*pPoints)) {
                    BYTE* pDest = reinterpret_cast<BYTE*>(pSurface) + pPoints->y * lPitch + pPoints->x * 3;
                    *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(color);
                    pDest[2] = static_cast<BYTE>(color >> 16);
                }
                pPoints++;
                nPoints--;
            }
        }
        break;
    case 32:
        if (bClipped) {
            while (nPoints > 0) {
                reinterpret_cast<DWORD*>(pSurface)[pPoints->y * (lPitch >> 2) + pPoints->x] = color;
                pPoints++;
                nPoints--;
            }
        } else {
            while (nPoints > 0) {
                if (rSurface.PtInRect(*pPoints)) {
                    reinterpret_cast<DWORD*>(pSurface)[pPoints->y * (lPitch >> 2) + pPoints->x] = color;
                }
                pPoints++;
                nPoints--;
            }
        }
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

// #binary-identical
// 0x797A30
BOOL CVidMode::DrawLine(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, UINT nSurface, const CRect& rSurface, COLORREF rgbColor)
{
    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return DrawLine3d(nXFrom, nYFrom, nXTo, nYTo, rSurface, rgbColor);
    }

    if (!ClipLine(nXFrom, nYFrom, nXTo, nYTo, rSurface)) {
        return FALSE;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 2391
    UTIL_ASSERT(nSurface != -1);

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);

    if (!LockSurface(nSurface, &ddsd, rSurface)) {
        return FALSE;
    }

    BOOL rc = DrawLine(nXFrom, nYFrom, nXTo, nYTo, ddsd, rSurface, rgbColor, TRUE);

    // NOTE: Uninline.
    UnLockSurface(nSurface, ddsd.lpSurface);

    return rc;
}

// #binary-identical
// 0x797B80
BOOL CVidMode::DrawLine(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, const DDSURFACEDESC& ddsd, const CRect& rSurface, COLORREF rgbColor, BOOLEAN bClipped)
{
    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        return DrawLine16(nXFrom,
            nYFrom,
            nXTo,
            nYTo,
            reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch >> 1,
            rSurface,
            ((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift),
            bClipped);
    case 24:
        return DrawLine24(nXFrom,
            nYFrom,
            nXTo,
            nYTo,
            reinterpret_cast<BYTE*>(ddsd.lpSurface),
            ddsd.lPitch,
            rSurface,
            (GetBValue(rgbColor) << m_dwBBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetRValue(rgbColor) << m_dwRBitShift),
            bClipped);
    case 32:
        return DrawLine32(nXFrom,
            nYFrom,
            nXTo,
            nYTo,
            reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch >> 2,
            rSurface,
            (GetBValue(rgbColor) << m_dwBBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetRValue(rgbColor) << m_dwRBitShift),
            bClipped);
    default:
        return FALSE;
    }
}

// 0x797D30
BOOL CVidMode::DrawLine16(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, WORD* pSurface, LONG lPitch, const CRect& rSurface, WORD color, BOOLEAN bClipped)
{
    INT nDeltaX;
    INT nDeltaY;
    INT nStep;

    if (!bClipped) {
        if (!ClipLine(nXFrom, nYFrom, nXTo, nYTo, rSurface)) {
            return FALSE;
        }
    }

    if (nYFrom > nYTo) {
        INT t;

        t = nXTo;
        nXTo = nXFrom;
        nXFrom = t;

        t = nYTo;
        nYTo = nYFrom;
        nYFrom = t;
    }

    if (nXTo - nXFrom < 0) {
        nDeltaX = nXFrom - nXTo;
        nStep = -1;
    } else {
        nDeltaX = nXTo - nXFrom;
        nStep = 1;
    }

    nDeltaY = nYTo - nYFrom;

    pSurface += lPitch * (nYFrom - rSurface.top) - rSurface.left + nXFrom;

    if (nDeltaX == 0) {
        while (nDeltaY >= 0) {
            *pSurface = color;
            pSurface += lPitch;
            nDeltaY--;
        }
        return TRUE;
    }

    if (nDeltaY == 0) {
        while (nDeltaX >= 0) {
            *pSurface = color;
            pSurface += nStep;
            nDeltaX--;
        }
        return TRUE;
    }

    if (nDeltaX == nDeltaY) {
        while (nDeltaX >= 0) {
            *pSurface = color;
            pSurface += lPitch + nStep;
            nDeltaX--;
        }
        return TRUE;
    }

    if (nDeltaX < nDeltaY) {
        ScanLine16(pSurface, nDeltaY, nDeltaX, lPitch, nStep, color);
    } else {
        ScanLine16(pSurface, nDeltaX, nDeltaY, nStep, lPitch, color);
    }

    return TRUE;
}

// 0x79A860
void CVidMode::ScanLine16(WORD* pSurface, UINT nMajorAxis, UINT nMinorAxis, INT nMajorAxisAdjust, INT nMinorAxisAdjust, WORD color)
{
    int v1 = nMajorAxis / nMinorAxis;
    int v2 = 2 * nMinorAxis;
    int v3 = 2 * (nMajorAxis % nMinorAxis);
    int v4 = v3 / 2 - v2;
    int v5 = v1 / 2 + 1;
    int v6 = nMinorAxis - 1;
    int v7 = v1 / 2 + 1;

    if (v3 != 0) {
        if ((v1 & 1) != 0) {
            v4 += nMinorAxis;
        }
    } else {
        if ((v1 & 1) != 0) {
            v4 += nMinorAxis;
        } else {
            v5 -= 1;
        }
    }

    while (v5 > 0) {
        *pSurface = color;
        pSurface += nMajorAxisAdjust;
        v5--;
    }

    pSurface += nMinorAxisAdjust;
    while (v6 > 0) {
        v4 += v3;

        int v8 = v1;
        if (v4 > 0) {
            v8 += 1;
            v4 -= v2;
        }

        while (v8 > 0) {
            *pSurface = color;
            pSurface += nMajorAxisAdjust;
            v8--;
        }

        pSurface += nMinorAxisAdjust;

        v6--;
    }

    while (v7 > 0) {
        *pSurface = color;
        pSurface += nMajorAxisAdjust;
        v7--;
    }
}

// 0x7980E0
BOOL CVidMode::DrawRect(const CRect& r, UINT nSurface, const CRect& rClip, COLORREF rgbColor)
{
    if (g_pChitin->cVideo.Is3dAccelerated()) {
        return DrawRect3d(r, rClip, rgbColor);
    }

    CRect rLock;
    rLock.left = max(r.left, rClip.left);
    rLock.top = max(r.top, rClip.top);
    rLock.right = min(r.right, rClip.right);
    rLock.bottom = min(r.bottom, rClip.bottom);

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);

    WORD color16;
    DWORD color;

    if (!LockSurface(nSurface, &ddsd, rLock)) {
        return FALSE;
    }

    INT x1 = r.left - rLock.left;
    INT y1 = r.top - rLock.top;
    INT x2 = r.right - rLock.left;
    INT y2 = r.bottom - rLock.top;

    switch (g_pChitin->cVideo.GetBitsPerPixels()) {
    case 16:
        color16 = ((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift);
        DrawLine16(x1,
            y1,
            x2,
            y1,
            reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch / 2,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color16,
            FALSE);
        DrawLine16(x1,
            y2,
            x2,
            y2,
            reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch / 2,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color16,
            FALSE);
        DrawLine16(x1,
            y1,
            x1,
            y2,
            reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch / 2,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color16,
            FALSE);
        DrawLine16(x2,
            y1,
            x2,
            y2,
            reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch / 2,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color16,
            FALSE);
        break;
    case 24:
        color = (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift);
        DrawLine24(x1,
            y1,
            x2,
            y1,
            reinterpret_cast<BYTE*>(ddsd.lpSurface),
            ddsd.lPitch,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color,
            FALSE);
        DrawLine24(x1,
            y2,
            x2,
            y2,
            reinterpret_cast<BYTE*>(ddsd.lpSurface),
            ddsd.lPitch,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color,
            FALSE);
        DrawLine24(x1,
            y1,
            x1,
            y2,
            reinterpret_cast<BYTE*>(ddsd.lpSurface),
            ddsd.lPitch,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color,
            FALSE);
        DrawLine24(x2,
            y1,
            x2,
            y2,
            reinterpret_cast<BYTE*>(ddsd.lpSurface),
            ddsd.lPitch,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color,
            FALSE);
        break;
    case 32:
        color = (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift);
        DrawLine32(x1,
            y1,
            x2,
            y1,
            reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch / 4,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color,
            FALSE);
        DrawLine32(x1,
            y2,
            x2,
            y2,
            reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch / 4,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color,
            FALSE);
        DrawLine32(x1,
            y1,
            x1,
            y2,
            reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch / 4,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color,
            FALSE);
        DrawLine32(x2,
            y1,
            x2,
            y2,
            reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch / 4,
            CRect(0, 0, rLock.Width(), rLock.Height()),
            color,
            FALSE);
        break;
    }

    // NOTE: Uninline.
    UnLockSurface(nSurface, ddsd.lpSurface);

    return TRUE;
}

// NOTE: Halves every colour channel of the pixels of `r` (clipped to `rClip`)
// on surface `nSurface`. The three pixel loops are an `__asm` block in the
// original: they count down with `DEC`/`JNZ`, so they are written as
// `do`/`while` loops (a zero width or height runs 2^32 times, as in the
// original).
//
// 0x797E70
BOOL CVidMode::ShadowRect(const CRect& r, UINT nSurface, const CRect& rClip)
{
    CRect rDest(r);
    if (!rDest.IntersectRect(rClip, r)) {
        return FALSE;
    }

    CRect rLock;
    rLock.left = max(rDest.left, rClip.left);
    rLock.top = max(rDest.top, rClip.top);
    rLock.right = min(rDest.right, rClip.right);
    rLock.bottom = min(rDest.bottom, rClip.bottom);

    INT x1 = rDest.left - rLock.left;
    INT y1 = rDest.top - rLock.top;
    INT x2 = x1 + rDest.Width();
    INT y2 = y1 + rDest.Height();

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);

    if (!LockSurface(nSurface, &ddsd, rLock)) {
        return FALSE;
    }

    INT nHeight = y2 - y1;
    DWORD dwMask = ((ddsd.ddpfPixelFormat.dwBBitMask >> 1) & ddsd.ddpfPixelFormat.dwBBitMask)
        + ((ddsd.ddpfPixelFormat.dwGBitMask >> 1) & ddsd.ddpfPixelFormat.dwGBitMask)
        + ((ddsd.ddpfPixelFormat.dwRBitMask >> 1) & ddsd.ddpfPixelFormat.dwRBitMask);
    INT nWidth = x2 - x1;

    LONG nSkip;
    INT nRows;
    INT nColumns;

    switch (g_pChitin->cVideo.GetBitsPerPixels()) {
    case 16:
        nSkip = ddsd.lPitch - nWidth * 2;
        {
            WORD* pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(ddsd.lpSurface) + ddsd.lPitch * y1) + x1;
            nRows = nHeight;
            do {
                nColumns = nWidth;
                do {
                    *pSurface = static_cast<WORD>((*pSurface >> 1) & dwMask);
                    pSurface++;
                } while (--nColumns != 0);
                pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nSkip);
            } while (--nRows != 0);
        }
        break;
    case 24:
        nSkip = ddsd.lPitch - nWidth * 3;
        {
            BYTE* pSurface = reinterpret_cast<BYTE*>(ddsd.lpSurface) + ddsd.lPitch * y1 + x1 * 3;
            nRows = nHeight;
            do {
                nColumns = nWidth;
                do {
                    // NOTE: The original reads the pixel's bytes in the order
                    // 1, 2, 0 into bits 16, 8, 0 and stores a whole dword back,
                    // so it also overwrites the first byte of the next pixel.
                    DWORD dwPixel = (pSurface[1] << 16) | (pSurface[2] << 8) | pSurface[0];
                    *reinterpret_cast<DWORD*>(pSurface) = (dwPixel >> 1) & dwMask;
                    pSurface += 3;
                } while (--nColumns != 0);
                pSurface += nSkip;
            } while (--nRows != 0);
        }
        break;
    case 32:
        nSkip = ddsd.lPitch - nWidth * 4;
        {
            DWORD* pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(ddsd.lpSurface) + ddsd.lPitch * y1) + x1;
            nRows = nHeight;
            do {
                nColumns = nWidth;
                do {
                    *pSurface = (*pSurface >> 1) & dwMask;
                    pSurface++;
                } while (--nColumns != 0);
                pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pSurface) + nSkip);
            } while (--nRows != 0);
        }
        break;
    }

    // NOTE: Uninline. The original locks `nSurface` but unlocks surface 0
    // (`CVIDINF_SURFACE_BACK`).
    UnLockSurface(0, ddsd.lpSurface);

    return TRUE;
}

// #binary-identical
// 0x798780
BOOL CVidMode::DrawRecticle(const CVIDMODE_RECTICLE_DESCRIPTION& rd, const CRect& rClip, COLORREF rgbColor)
{
    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return DrawRecticle3d(rd, rClip, rgbColor);
    }

    CRect r;
    r.SetRect(rd.ptCenter.x - rd.xAxis - rd.piePieceXOffset - 1,
        rd.ptCenter.y - rd.yAxis - rd.piePieceYOffset - 1,
        rd.ptCenter.x + rd.xAxis + rd.piePieceXOffset + 1,
        rd.ptCenter.y + rd.yAxis + rd.piePieceYOffset + 1);

    CRect rClipped;
    if (rClipped.IntersectRect(r, rClip)) {
        DDSURFACEDESC ddsd;
        ddsd.dwSize = sizeof(ddsd);

        if (!LockSurface(0, &ddsd, rClipped)) {
            return FALSE;
        }

        CVIDMODE_RECTICLE_DESCRIPTION copy;
        memcpy(&copy, &rd, sizeof(copy));
        copy.ptCenter.x = rd.xAxis + rd.piePieceXOffset;
        copy.ptCenter.y = rd.yAxis + rd.piePieceYOffset;

        CRect* pTemp;
        CRect rTemp;
        if (rClip.EqualRect(r)) {
            pTemp = NULL;
        } else {
            // NOTE: Looks like inlined OffsetRect.
            rTemp.left = rClipped.left - r.left;
            rTemp.top = rClipped.top - r.top;
            rTemp.right = rClipped.right - r.left;
            rTemp.bottom = rClipped.bottom - r.top;
            pTemp = &rTemp;
        }

        DrawRecticle(copy, ddsd, pTemp, rgbColor);

        // NOTE: Uninline.
        UnLockSurface(0, ddsd.lpSurface);
    }

    return TRUE;
}

// #binary-identical
// 0x798930
void CVidMode::DrawRecticle(const CVIDMODE_RECTICLE_DESCRIPTION& rd, const DDSURFACEDESC& ddsd, const CRect* rClip, COLORREF rgbColor)
{
    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        DrawRecticle16(rd, ddsd, rClip, rgbColor);
        break;
    case 24:
        DrawRecticle24(rd, ddsd, rClip, rgbColor);
        break;
    case 32:
        DrawRecticle32(rd, ddsd, rClip, rgbColor);
        break;
    }
}

// 0x7989B0
void CVidMode::DrawRecticle16(const CVIDMODE_RECTICLE_DESCRIPTION& rd, const DDSURFACEDESC& ddsd, const CRect* rClip, COLORREF rgbColor)
{
    BOOLEAN bHaveClip = TRUE;
    LONG lPitch = ddsd.lPitch >> 1;
    BYTE* pPixelList = new BYTE[max(rd.xAxis, rd.yAxis)];

    INT v1 = rd.xAxis + rd.piePieceXOffset;
    INT v2 = rd.yAxis + rd.piePieceYOffset;
    INT v3;

    WORD color = static_cast<WORD>(((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift)
        | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift)
        | ((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift));

    CRect rLineClip;
    if (rClip != NULL) {
        rLineClip.SetRect(rClip->left, rClip->top, rClip->right - 1, rClip->bottom - 1);
    } else {
        rLineClip.SetRect(0, 0, 0, 0);
        bHaveClip = FALSE;
    }

    // NOTE: Without a clip rect the lines are already within the surface.
    BOOLEAN bLineClipped = !bHaveClip;

    LONG nArcLength = GetEllipseArcPixelList(rd.xAxis, rd.yAxis, pPixelList);

    nArcLength -= 1;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 3073
    UTIL_ASSERT(nArcLength < max(rd.xAxis, rd.yAxis));

    v3 = DrawEllipseHOctant16(rd.ptCenter.x,
        rd.ptCenter.y - v2,
        nArcLength - rd.xGap,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine16(rd.ptCenter.x + rd.xGap - nArcLength,
        v3,
        rd.ptCenter.x,
        rd.ptCenter.y + rd.piePiecePtYOffset - v2,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant16(rd.ptCenter.x + 1,
        rd.ptCenter.y + pPixelList[0] - v2,
        nArcLength - rd.xGap - 1,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine16(rd.ptCenter.x - rd.xGap + nArcLength,
        v3,
        rd.ptCenter.x,
        rd.ptCenter.y + rd.piePiecePtYOffset - v2,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant16(rd.ptCenter.x,
        rd.ptCenter.y + v2,
        nArcLength - rd.xGap,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine16(rd.ptCenter.x + rd.xGap - nArcLength,
        v3,
        rd.ptCenter.x,
        rd.ptCenter.y - rd.piePiecePtYOffset + v2,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant16(rd.ptCenter.x + 1,
        rd.ptCenter.y - pPixelList[0] + v2,
        nArcLength - rd.xGap - 1,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine16(rd.ptCenter.x - rd.xGap + nArcLength,
        v3,
        rd.ptCenter.x,
        rd.ptCenter.y - rd.piePiecePtYOffset + v2,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    nArcLength = GetEllipseArcPixelList(rd.yAxis, rd.xAxis, pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 3105
    UTIL_ASSERT(nArcLength < max(rd.xAxis, rd.yAxis));

    v3 = DrawEllipseVOctant16(rd.ptCenter.x - v1,
        rd.ptCenter.y,
        nArcLength - rd.yGap,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList,
        rClip,
        color);

    DrawLine16(v3,
        rd.ptCenter.y + rd.yGap - nArcLength,
        rd.ptCenter.x + rd.piePiecePtXOffset - v1,
        rd.ptCenter.y,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant16(rd.ptCenter.x + pPixelList[0] - v1,
        rd.ptCenter.y + 1,
        nArcLength - rd.yGap - 1,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine16(v3,
        rd.ptCenter.y - rd.yGap + nArcLength,
        rd.ptCenter.x + rd.piePiecePtXOffset - v1,
        rd.ptCenter.y,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant16(rd.ptCenter.x + v1,
        rd.ptCenter.y,
        nArcLength - rd.yGap,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine16(v3,
        rd.ptCenter.y + rd.yGap - nArcLength,
        rd.ptCenter.x - rd.piePiecePtXOffset + v1,
        rd.ptCenter.y,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant16(rd.ptCenter.x - pPixelList[0] + v1,
        rd.ptCenter.y + 1,
        nArcLength - rd.yGap - 1,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine16(v3,
        rd.ptCenter.y - rd.yGap + nArcLength,
        rd.ptCenter.x - rd.piePiecePtXOffset + v1,
        rd.ptCenter.y,
        reinterpret_cast<WORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    delete pPixelList;
}

// #binary-identical
// 0x798F90
BOOL CVidMode::DrawThickLine(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, UINT nSurface, CRect& rSurface, int a8, COLORREF rgbColor)
{
    SHORT v1 = (a8 + 1) >> 1;

    CRect rClip;
    rClip.left = rSurface.left + v1;
    rClip.top = rSurface.top + v1;
    rClip.right = rSurface.right - v1;
    rClip.bottom = rSurface.bottom - v1;

    if (!ClipLine(nXFrom, nYFrom, nXTo, nYTo, rClip)) {
        return FALSE;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 3191
    UTIL_ASSERT(nSurface != -1);

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);

    if (!LockSurface(nSurface, &ddsd, rSurface)) {
        return FALSE;
    }

    BOOL rc = DrawThickLine(nXFrom, nYFrom, nXTo, nYTo, ddsd, rSurface, a8, rgbColor, TRUE);

    // NOTE: Uninline.
    UnLockSurface(nSurface, ddsd.lpSurface);

    return rc;
}

// #binary-identical
// 0x7990D0
BOOL CVidMode::DrawThickLine(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, const DDSURFACEDESC& ddsd, CRect& rSurface, int a8, COLORREF rgbColor, BOOLEAN bClipped)
{
    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        return DrawThickLine16(nXFrom,
            nYFrom,
            nXTo,
            nYTo,
            reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch >> 1,
            rSurface,
            a8,
            ((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift),
            bClipped);
    case 24:
        return DrawThickLine24(nXFrom,
            nYFrom,
            nXTo,
            nYTo,
            reinterpret_cast<BYTE*>(ddsd.lpSurface),
            ddsd.lPitch,
            rSurface,
            a8,
            (GetBValue(rgbColor) << m_dwBBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetRValue(rgbColor) << m_dwRBitShift),
            bClipped);
    case 32:
        return DrawThickLine32(nXFrom,
            nYFrom,
            nXTo,
            nYTo,
            reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch >> 2,
            rSurface,
            a8,
            (GetBValue(rgbColor) << m_dwBBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetRValue(rgbColor) << m_dwRBitShift),
            bClipped);
    default:
        return FALSE;
    }
}

// 0x799280
BOOL CVidMode::DrawThickLine16(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, WORD* pSurface, LONG lPitch, CRect& rSurface, int a8, WORD color, BOOLEAN bClipped)
{
    INT nThickness = a8 + 1;
    SHORT nHalf = static_cast<SHORT>(nThickness >> 1);
    INT nDeltaX;
    INT nDeltaY;
    INT nStep = 1;
    INT n;

    if (!bClipped) {
        CRect rClip;
        rClip.left = rSurface.left + nHalf;
        rClip.top = rSurface.top + nHalf;
        rClip.right = rSurface.right - nHalf;
        rClip.bottom = rSurface.bottom - nHalf;

        if (!ClipLine(nXFrom, nYFrom, nXTo, nYTo, rClip)) {
            return FALSE;
        }
    }

    if (nYFrom > nYTo) {
        INT t;

        t = nXTo;
        nXTo = nXFrom;
        nXFrom = t;

        t = nYTo;
        nYTo = nYFrom;
        nYFrom = t;
    }

    nDeltaX = nXTo - nXFrom;
    if (nDeltaX < 0) {
        nDeltaX = nXFrom - nXTo;
        nStep = -1;
    }

    nDeltaY = nYTo - nYFrom;

    pSurface += lPitch * (nYFrom - rSurface.top) - rSurface.left + nXFrom;

    if (nStep > 0) {
        pSurface -= nHalf;
    } else {
        pSurface += nHalf;
    }

    pSurface -= nHalf * lPitch;

    if (nDeltaX == 0) {
        if (a8 < 0) {
            return TRUE;
        }

        // NOTE: Every column ends up one row lower than the previous one.
        for (INT nColumn = nThickness; nColumn != 0; nColumn--) {
            if (nDeltaY >= 0) {
                for (n = nDeltaY + 1; n != 0; n--) {
                    *pSurface = color;
                    pSurface += lPitch;
                }
            }
            pSurface += 1 - nDeltaY * lPitch;
        }

        return TRUE;
    }

    if (nDeltaY == 0) {
        if (a8 < 0) {
            return TRUE;
        }

        // NOTE: Every row ends up one column further than the previous one.
        for (INT nRow = nThickness; nRow != 0; nRow--) {
            if (nDeltaX >= 0) {
                for (n = nDeltaX + 1; n != 0; n--) {
                    *pSurface = color;
                    pSurface += nStep;
                }
            }
            pSurface += lPitch - nDeltaX * nStep;
        }

        return TRUE;
    }

    if (nDeltaX == nDeltaY) {
        INT nCount = a8 - 1;
        INT nIndex;

        for (nIndex = 0; nIndex < a8; nIndex++) {
            nCount += nIndex < nDeltaX ? 1 : 0;

            for (n = nCount; n > 0; n--) {
                *pSurface = color;
                pSurface += nStep;
            }

            pSurface += lPitch - nCount * nStep;
        }

        pSurface += nStep;

        for (INT nRows = nDeltaY - ((a8 & 1) != 0 ? a8 : a8 - 1); nRows > 0; nRows--) {
            for (n = nCount; n > 0; n--) {
                *pSurface = color;
                pSurface += nStep;
            }

            pSurface += lPitch - (nCount - 1) * nStep;
        }

        if (a8 > 0) {
            INT nAdjust = (1 - nCount) * nStep + lPitch;

            for (nIndex = a8; nIndex != 0; nIndex--) {
                for (n = nCount; n > 0; n--) {
                    *pSurface = color;
                    pSurface += nStep;
                }

                pSurface += nAdjust;
                nCount--;
                nAdjust += nStep;
            }
        }

        return TRUE;
    }

    if (nDeltaX < nDeltaY) {
        ThickScanLine16(pSurface, nDeltaY, nDeltaX, lPitch, nStep, a8, color);
    } else {
        ThickScanLine16(pSurface, nDeltaX, nDeltaY, nStep, lPitch, a8, color);
    }

    return TRUE;
}

// 0x79A950
void CVidMode::ThickScanLine16(WORD* pSurface, UINT nMajorAxis, UINT nMinorAxis, INT nMajorAxisAdjust, INT nMinorAxisAdjust, UINT nThickness, WORD color)
{
    UINT v1 = nMajorAxis / nMinorAxis;
    INT v2 = 2 * nMinorAxis;
    INT v3 = 2 * (nMajorAxis % nMinorAxis);
    INT v4 = v3 / 2 - v2;
    UINT v5 = v1 / 2 + 1;
    UINT v7 = v1 / 2 + 1;
    UINT n;

    if (v3 == 0 && (v1 & 1) == 0) {
        v5--;
    }

    if ((v1 & 1) != 0) {
        v4 += nMinorAxis;
    }

    INT v8 = v4;

    // Initial run, long enough to cover the leading edge of the thick line.
    UINT nEnd = (nThickness & ~1) + v5;
    for (n = nEnd; n != 0; n--) {
        *pSurface = color;
        pSurface += nMajorAxisAdjust;
    }

    pSurface += nMinorAxisAdjust - static_cast<INT>(nEnd) * nMajorAxisAdjust;

    UINT nDelay = nThickness - 2;
    UINT nStart = 0;
    UINT nAdvance = v5;
    UINT nRun;

    for (UINT nRow = nMinorAxis - 1; nRow != 0; nRow--) {
        nRun = v1;
        v4 += v3;
        if (v4 > 0) {
            nRun = v1 + 1;
            v4 -= v2;
        }

        nEnd += nRun;

        for (n = nStart; n < nEnd; n++) {
            *pSurface = color;
            pSurface += nMajorAxisAdjust;
        }

        pSurface -= static_cast<INT>(nEnd - nStart) * nMajorAxisAdjust;

        if (nDelay == 0) {
            pSurface += static_cast<INT>(nAdvance) * nMajorAxisAdjust;
            nStart += nAdvance;

            nAdvance = v1;
            v8 += v3;
            if (v8 > 0) {
                nAdvance = v1 + 1;
                v8 -= v2;
            }
        } else {
            nDelay--;
        }

        pSurface += nMinorAxisAdjust;
    }

    // Trailing edge.
    nEnd += v7;

    for (UINT nIndex = 0; nIndex < nThickness; nIndex++) {
        for (n = nStart; n < nEnd; n++) {
            *pSurface = color;
            pSurface += nMajorAxisAdjust;
        }

        pSurface -= static_cast<INT>(nEnd - nStart) * nMajorAxisAdjust;

        if (nIndex == nThickness - 1) {
            nAdvance = v7;
        }

        pSurface += static_cast<INT>(nAdvance) * nMajorAxisAdjust;
        nStart += nAdvance;

        nAdvance = v1;
        v8 += v3;
        if (v8 > 0) {
            nAdvance = v1 + 1;
            v8 -= v2;
        }

        pSurface += nMinorAxisAdjust;
    }
}

// 0x799540
BOOL CVidMode::OutlinePoly(const CPoint* pPoly, SHORT nVertices, const CRect& rSurface, COLORREF rgbColor, const CPoint& ptOffset)
{
    CRect rClip(0, 0, rSurface.Width(), rSurface.Height());
    INT nOffsetX = rSurface.left + ptOffset.x;
    INT nOffsetY = rSurface.top + ptOffset.y;
    SHORT nIndex;
    DWORD color;

    if (nVertices <= 0) {
        return FALSE;
    }

    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return OutlinePoly3d(pPoly, nVertices, rSurface, rgbColor, ptOffset);
    }

    if (rSurface.IsRectEmpty()) {
        return FALSE;
    }

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);

    BOOLEAN bLocked = LockSurface(0, &ddsd, rSurface);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 3466
    UTIL_ASSERT(ddsd.lpSurface != NULL);

    if (!bLocked) {
        return FALSE;
    }

    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        color = ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift)
            | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift)
            | ((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift);

        for (nIndex = 0; nIndex < nVertices - 1; nIndex++) {
            DrawLine16(pPoly[nIndex].x - nOffsetX,
                pPoly[nIndex].y - nOffsetY,
                pPoly[nIndex + 1].x - nOffsetX,
                pPoly[nIndex + 1].y - nOffsetY,
                reinterpret_cast<WORD*>(ddsd.lpSurface),
                ddsd.lPitch >> 1,
                rClip,
                static_cast<WORD>(color),
                FALSE);
        }

        DrawLine16(pPoly[nVertices - 1].x - nOffsetX,
            pPoly[nVertices - 1].y - nOffsetY,
            pPoly[0].x - nOffsetX,
            pPoly[0].y - nOffsetY,
            reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch >> 1,
            rClip,
            static_cast<WORD>(color),
            FALSE);
        break;
    case 24:
        color = (GetGValue(rgbColor) << m_dwGBitShift)
            | (GetBValue(rgbColor) << m_dwBBitShift)
            | (GetRValue(rgbColor) << m_dwRBitShift);

        for (nIndex = 0; nIndex < nVertices - 1; nIndex++) {
            DrawLine24(pPoly[nIndex].x - nOffsetX,
                pPoly[nIndex].y - nOffsetY,
                pPoly[nIndex + 1].x - nOffsetX,
                pPoly[nIndex + 1].y - nOffsetY,
                reinterpret_cast<BYTE*>(ddsd.lpSurface),
                ddsd.lPitch,
                rClip,
                color,
                FALSE);
        }

        DrawLine24(pPoly[nVertices - 1].x - nOffsetX,
            pPoly[nVertices - 1].y - nOffsetY,
            pPoly[0].x - nOffsetX,
            pPoly[0].y - nOffsetY,
            reinterpret_cast<BYTE*>(ddsd.lpSurface),
            ddsd.lPitch,
            rClip,
            color,
            FALSE);
        break;
    case 32:
        color = (GetGValue(rgbColor) << m_dwGBitShift)
            | (GetBValue(rgbColor) << m_dwBBitShift)
            | (GetRValue(rgbColor) << m_dwRBitShift);

        for (nIndex = 0; nIndex < nVertices - 1; nIndex++) {
            DrawLine32(pPoly[nIndex].x - nOffsetX,
                pPoly[nIndex].y - nOffsetY,
                pPoly[nIndex + 1].x - nOffsetX,
                pPoly[nIndex + 1].y - nOffsetY,
                reinterpret_cast<DWORD*>(ddsd.lpSurface),
                ddsd.lPitch >> 2,
                rClip,
                color,
                FALSE);
        }

        DrawLine32(pPoly[nVertices - 1].x - nOffsetX,
            pPoly[nVertices - 1].y - nOffsetY,
            pPoly[0].x - nOffsetX,
            pPoly[0].y - nOffsetY,
            reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch >> 2,
            rClip,
            color,
            FALSE);
        break;
    }

    // NOTE: Uninline.
    UnLockSurface(0, ddsd.lpSurface);

    return TRUE;
}

// #binary-identical
// 0x7999B0
BOOL CVidMode::EraseScreen(UINT nSurface, COLORREF rgbColor)
{
    if (nSurface >= m_nSurfaces) {
        return FALSE;
    }

    IDirectDrawSurface* pSurface = pSurfaces[nSurface];
    if (pSurface == NULL) {
        return FALSE;
    }

    DDBLTFX fx;
    fx.dwSize = sizeof(fx);
    fx.dwFillColor = ((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift);

    HRESULT hr;
    do {
        hr = g_pChitin->cVideo.cVidBlitter.Blt(pSurface, NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
        CheckResults(hr);
    } while (hr == DDERR_SURFACELOST || hr == DDERR_WASSTILLDRAWING);

    return TRUE;
}

// 0x799A90
int CVidMode::GetColorDepth()
{
    return m_nColorDepth;
}

// 0x799AA0
LONG CVidMode::GetEllipseArcPixelList(LONG nMajorAxis, LONG nMinorAxis, BYTE* pPixelList)
{
    INT nMajorAxisSqrd = nMajorAxis * nMajorAxis;
    INT nMinorAxisSqrd = nMinorAxis * nMinorAxis;
    INT nMajorAxisAdjust = 0;
    INT nMinorAxisAdjust = nMinorAxis * nMajorAxisSqrd * 2;
    INT nThreshold = nMajorAxisSqrd / 4 - nMinorAxis * nMajorAxisSqrd;
    LONG nArcLength = 0;

    do {
        nThreshold += nMajorAxisAdjust + nMinorAxisSqrd;
        if (nThreshold < 0) {
            pPixelList[nArcLength] = 0;
        } else {
            pPixelList[nArcLength] = 1;
            nMinorAxisAdjust -= 2 * nMajorAxisSqrd;
            nThreshold -= nMinorAxisAdjust;
        }

        nMajorAxisAdjust += 2 * nMinorAxisSqrd;

        nArcLength++;
    } while (nMajorAxisAdjust < nMinorAxisAdjust);

    return nArcLength;
}

// #binary-identical
// 0x799B00
BOOL CVidMode::GetCursorSurfaceSize(CSize& size)
{
    size.cx = 0;
    size.cy = 0;
    return FALSE;
}

// #binary-identical
// 0x799B00
BOOL CVidMode::GetFXSize(CSize& size)
{
    size.cx = 0;
    size.cy = 0;
    return FALSE;
}

// #binary-identical
// 0x799B20
BOOL CVidMode::GetFXSurface(INT& nSurface, DWORD dwFlags)
{
    nSurface = 0;
    return FALSE;
}

// #binary-identical
// 0x799B30
int CVidMode::GetType()
{
    return 255;
}

// #binary-identical
// 0x799B40
BOOL CVidMode::RenderPoly(UINT nSurface, CRect& a3, POINT* apt, int cpt, COLORREF rgbColor)
{
    if (nSurface >= m_nSurfaces) {
        return FALSE;
    }

    IDirectDrawSurface* pSurface = pSurfaces[nSurface];
    if (pSurface == NULL) {
        return FALSE;
    }

    HRESULT hr;
    HDC hdc;

    do {
        hr = pSurface->GetDC(&hdc);
        CheckResults(hr);
    } while (hr == DDERR_SURFACELOST || hr == DDERR_WASSTILLDRAWING);

    if (hr != DD_OK) {
        return FALSE;
    }

    HPEN hPen = CreatePen(PS_SOLID, 1, rgbColor);
    HBRUSH hBrush = CreateSolidBrush(rgbColor);
    HGDIOBJ hPrevPen = SelectObject(hdc, hPen);
    HGDIOBJ hPrevBrush = SelectObject(hdc, hBrush);
    Polygon(hdc, apt, cpt);
    SelectObject(hdc, hPrevPen);
    SelectObject(hdc, hPrevBrush);
    DeleteObject(hPen);
    DeleteObject(hBrush);

    do {
        hr = pSurface->ReleaseDC(hdc);
        CheckResults(hr);
    } while (hr == DDERR_SURFACELOST || hr == DDERR_WASSTILLDRAWING);

    return TRUE;
}

// 0x799C60
BOOLEAN CVidMode::PrintSurfaceToBmp(LPBYTE& data, int a3, const CRect& r, LONG& size, short a6)
{
    return FALSE;
}

// 0x799CA0
void CVidMode::DestroySurfaces()
{
}

// 0x799E30
void CVidMode::DoTextOut(UINT nSurface, const CString& sText, int x, int y, COLORREF color)
{
}

// 0x799E40
void CVidMode::DoTextOut(UINT nSurface, const CString& sText, int x, int y, COLORREF color, int height)
{
}

// 0x799E60
void CVidMode::LoadFogOWarSurfaces(const CString& a2)
{
}

// 0x799CA0
void CVidMode::RestoreSurfaces()
{
}

// 0x799F90
CVidCell* CVidMode::GetPointer()
{
    return m_pPointerVidCell;
}

// #binary-identical
// 0x799FA0
BOOL CVidMode::LockSurface(UINT nIndex, LPDDSURFACEDESC pSurfaceDesc, const CRect& rect)
{
    RECT r = static_cast<RECT>(rect);

    if (nIndex >= m_nSurfaces) {
        return FALSE;
    }

    IDirectDrawSurface* pSurface = pSurfaces[nIndex];
    if (pSurface == NULL) {
        return FALSE;
    }

    HRESULT hr;
    do {
        while (TRUE) {
            pSurfaceDesc->dwSize = sizeof(*pSurfaceDesc);
            hr = pSurface->Lock(&r, pSurfaceDesc, DDLOCK_WAIT, NULL);
            if (hr != DDERR_SURFACELOST) {
                break;
            }
            RestoreSurfaces();
        }

        CheckResults(hr);
    } while (hr == DDERR_SURFACEBUSY);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 5071
    UTIL_ASSERT(g_pChitin->cVideo.GetBitsPerPixels() == 24 || !((LONG)(pSurfaceDesc->lpSurface) & 0x00000001));

    return hr == DD_OK;
}

// 0x79A080
BOOL CVidMode::PolyLine(INT nSurface, CRect& rSurface, LPPOINT lpPoints, int nCount, COLORREF rgbColor, int nThickness)
{
    BOOL bResult;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo.cpp
    // __LINE__: 5104
    UTIL_ASSERT(nSurface != -1 && (nThickness > 0));

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);

    bResult = LockSurface(nSurface, &ddsd, rSurface);
    if (!bResult) {
        return FALSE;
    }

    switch (g_pChitin->cVideo.GetBitsPerPixels()) {
    case 16:
        if (nThickness == 1) {
            while (nCount - 1 > 0) {
                bResult = DrawLine16(lpPoints[0].x,
                    lpPoints[0].y,
                    lpPoints[1].x,
                    lpPoints[1].y,
                    reinterpret_cast<WORD*>(ddsd.lpSurface),
                    ddsd.lPitch >> 1,
                    rSurface,
                    static_cast<WORD>(((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift)),
                    FALSE);
                lpPoints++;
                nCount--;
            }
        } else {
            while (nCount - 1 > 0) {
                bResult = DrawThickLine16(lpPoints[0].x,
                    lpPoints[0].y,
                    lpPoints[1].x,
                    lpPoints[1].y,
                    reinterpret_cast<WORD*>(ddsd.lpSurface),
                    ddsd.lPitch >> 1,
                    rSurface,
                    nThickness,
                    static_cast<WORD>(((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift)),
                    FALSE);
                lpPoints++;
                nCount--;
            }
        }
        break;
    case 24:
        if (nThickness == 1) {
            while (nCount - 1 > 0) {
                bResult = DrawLine24(lpPoints[0].x,
                    lpPoints[0].y,
                    lpPoints[1].x,
                    lpPoints[1].y,
                    reinterpret_cast<BYTE*>(ddsd.lpSurface),
                    ddsd.lPitch,
                    rSurface,
                    (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift),
                    FALSE);
                lpPoints++;
                nCount--;
            }
        } else {
            while (nCount - 1 > 0) {
                bResult = DrawThickLine24(lpPoints[0].x,
                    lpPoints[0].y,
                    lpPoints[1].x,
                    lpPoints[1].y,
                    reinterpret_cast<BYTE*>(ddsd.lpSurface),
                    ddsd.lPitch,
                    rSurface,
                    nThickness,
                    (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift),
                    FALSE);
                lpPoints++;
                nCount--;
            }
        }
        break;
    case 32:
        if (nThickness == 1) {
            while (nCount - 1 > 0) {
                bResult = DrawLine32(lpPoints[0].x,
                    lpPoints[0].y,
                    lpPoints[1].x,
                    lpPoints[1].y,
                    reinterpret_cast<DWORD*>(ddsd.lpSurface),
                    ddsd.lPitch / 4,
                    rSurface,
                    (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift),
                    FALSE);
                lpPoints++;
                nCount--;
            }
        } else {
            while (nCount - 1 > 0) {
                bResult = DrawThickLine32(lpPoints[0].x,
                    lpPoints[0].y,
                    lpPoints[1].x,
                    lpPoints[1].y,
                    reinterpret_cast<DWORD*>(ddsd.lpSurface),
                    ddsd.lPitch / 4,
                    rSurface,
                    nThickness,
                    (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift),
                    FALSE);
                lpPoints++;
                nCount--;
            }
        }
        break;
    }

    // NOTE: Uninline.
    UnLockSurface(nSurface, ddsd.lpSurface);

    return bResult;
}

// #binary-identical
// 0x79A670
BOOL CVidMode::SetPointer(CVidCell* pVidCell, BOOLEAN bAnimating, INT nPointerNumber)
{
    CSingleLock lock(&m_csRenderPointer, FALSE);
    lock.Lock(INFINITE);

    m_nPointerNumber = nPointerNumber;
    m_pPointerVidCell = pVidCell;
    m_bPointerAnimating = bAnimating;

    lock.Unlock();

    return TRUE;
}

// 0x79A700
BOOL CVidMode::SetPointer(CVidCell* pVidCell, CResRef cResRef, BOOLEAN bAnimating, INT nPointerNumber)
{
    CSingleLock lock(&m_csRenderPointer, FALSE);
    lock.Lock(INFINITE);

    // NOTE: Uninline.
    pVidCell->SetResRef(cResRef, FALSE, TRUE, TRUE);

    if (pVidCell->pRes == NULL) {
        lock.Unlock();
        return NULL;
    }

    m_bPointerAnimating = bAnimating;
    m_nPointerNumber = nPointerNumber;
    m_pPointerVidCell = pVidCell;

    lock.Unlock();

    return TRUE;
}

// #binary-identical
// 0x79A820
BOOL CVidMode::UnLockSurface(UINT nIndex, LPVOID lpSurfaceData)
{
    if (nIndex >= m_nSurfaces) {
        return FALSE;
    }

    IDirectDrawSurface* pSurface = pSurfaces[nIndex];
    if (pSurface == NULL) {
        return FALSE;
    }

    return pSurface->Unlock(lpSurfaceData) == DD_OK;
}

// #binary-identical
// 0x79AB50
DWORD CVidMode::ReduceColor(COLORREF rgbColor)
{
    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return rgbColor | 0xFF000000;
    }

    if (g_pChitin->cVideo.m_nBpp == 16) {
        return ((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift);
    } else {
        return (GetBValue(rgbColor) << m_dwBBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetRValue(rgbColor) << m_dwRBitShift);
    }
}

// 0x79AC20
void CVidMode::SetWindowedMode(HWND hWnd)
{
    LONG windowStyle = GetWindowLongA(hWnd, GWL_STYLE);
    windowStyle &= ~(WS_POPUP | WS_CAPTION | WS_MINIMIZEBOX);
    windowStyle |= (WS_CAPTION | WS_MINIMIZEBOX);
    SetWindowLongA(hWnd, GWL_STYLE, windowStyle);

    RECT windowRect;
    SetRect(&windowRect, 0, 0, CVideo::SCREENWIDTH, CVideo::SCREENHEIGHT);

    AdjustWindowRectEx(&windowRect,
        GetWindowLongA(hWnd, GWL_STYLE),
        GetMenu(hWnd) != NULL,
        GetWindowLongA(hWnd, GWL_EXSTYLE));

    SetWindowPos(hWnd,
        HWND_TOP,
        0,
        0,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);

    SetWindowPos(hWnd,
        HWND_NOTOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

    RECT workRect;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &workRect, 0);

    GetWindowRect(hWnd, &windowRect);

    if (windowRect.left < workRect.left) {
        windowRect.left = workRect.left;
    }

    if (windowRect.top < workRect.top) {
        windowRect.top = workRect.top;
    }

    SetWindowPos(hWnd,
        HWND_TOP,
        windowRect.left,
        windowRect.top,
        0,
        0,
        SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

// 0x79AD20
DWORD CVidMode::FUN_0079ad20(DWORD dwPixel, DWORD dwScale)
{
    DWORD pixel = dwPixel & 0xFFFF;
    DWORD r = (((m_dwRBitMask & pixel) >> m_dwRBitShift) * dwScale >> 8) << m_dwRBitShift;
    DWORD g = (((m_dwGBitMask & pixel) >> m_dwGBitShift) * dwScale >> 8) << m_dwGBitShift;
    DWORD b = (((m_dwBBitMask & pixel) >> m_dwBBitShift) * dwScale >> 8) << m_dwBBitShift;
    return r | g | b;
}

// 0x79AD90
DWORD CVidMode::FUN_0079ad90(DWORD dwPixel)
{
    DWORD pixel = dwPixel & 0xFFFF;
    return ((((m_dwBBitMask & pixel) >> m_dwBBitShift) << field_CA) + (((m_dwGBitMask & pixel) >> m_dwGBitShift) << field_C6) * 2) + (((m_dwRBitMask & pixel) >> m_dwRBitShift) << field_C2);
}

// 0x79ADF0
DWORD CVidMode::FUN_0079adf0(COLORREF rgbColor)
{
    if (g_pChitin->cVideo.m_nBpp > 16) {
        return (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift);
    } else {
        return (((GetRValue(rgbColor) >> field_C2) << m_dwRBitShift) | ((GetGValue(rgbColor) >> field_C6) << m_dwGBitShift) | ((GetBValue(rgbColor) >> field_CA) << m_dwBBitShift)) & 0xFFFF;
    }
}

// 0x7BC100
BOOL CVidMode::ActivateVideoMode(CVidMode* pPrevVidMode, HWND hWnd, BOOLEAN bFullscreen)
{
    return TRUE;
}

// 0x7C8B40
LONG CVidMode::DrawEllipse24(const CPoint& ptCenter, const CSize& axis, const DDSURFACEDESC& ddsd, const CRect* rClip, COLORREF rgbColor)
{
    BYTE* pPixelList = new BYTE[max(axis.cx, axis.cy)];
    DWORD color = (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift);
    LONG nArcLength = GetEllipseArcPixelList(axis.cx, axis.cy, pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo24.cpp
    // __LINE__: 68
    UTIL_ASSERT(nArcLength <= max(axis.cx, axis.cy));

    DrawEllipseHOctant24(ptCenter.x,
        ptCenter.y - axis.cy,
        nArcLength,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        ddsd.lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseHOctant24(ptCenter.x + 1,
        ptCenter.y + pPixelList[0] - axis.cy,
        nArcLength - 1,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        ddsd.lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawEllipseHOctant24(ptCenter.x,
        ptCenter.y + axis.cy,
        nArcLength,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        -ddsd.lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseHOctant24(ptCenter.x + 1,
        ptCenter.y - pPixelList[0] + axis.cy,
        nArcLength - 1,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        -ddsd.lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    nArcLength = GetEllipseArcPixelList(axis.cy, axis.cx, pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo24.cpp
    // __LINE__: 91
    UTIL_ASSERT(nArcLength < max(axis.cx, axis.cy));

    DrawEllipseVOctant24(ptCenter.x - axis.cx,
        ptCenter.y,
        nArcLength,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        -ddsd.lPitch,
        TRUE,
        pPixelList,
        rClip,
        color);

    DrawEllipseVOctant24(ptCenter.x + pPixelList[0] - axis.cx,
        ptCenter.y + 1,
        nArcLength - 1,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        ddsd.lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawEllipseVOctant24(ptCenter.x + axis.cx,
        ptCenter.y,
        nArcLength,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        -ddsd.lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseVOctant24(ptCenter.x - pPixelList[0] + axis.cx,
        ptCenter.y + 1,
        nArcLength - 1,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        ddsd.lPitch,
        FALSE,
        pPixelList + 1,
        rClip,
        color);

    delete pPixelList;

    return 0;
}

// 0x7C8E10
LONG CVidMode::DrawEllipseVOctant24(LONG x, LONG y, INT nArcLength, BYTE* pSurface, LONG lPitch, BOOLEAN clockwise, unsigned char* pArcData, const CRect* rClip, DWORD color)
{
    INT nStep;

    if (rClip != NULL) {
        if (lPitch < 0) {
            pSurface += 3 * (x - rClip->left) - lPitch * (y - rClip->top);
            nStep = -1;
        } else {
            pSurface += 3 * (x - rClip->left) + lPitch * (y - rClip->top);
            nStep = 1;
        }

        while (nArcLength != 0) {
            if (rClip->PtInRect({ x, y })) {
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                pSurface[2] = static_cast<BYTE>(color >> 16);
            }

            if (*pArcData++ != 0) {
                if (clockwise) {
                    pSurface += 3;
                    x++;
                } else {
                    pSurface -= 3;
                    x--;
                }
            }

            pSurface += lPitch;
            y += nStep;

            nArcLength--;
        }
    } else {
        if (lPitch < 0) {
            pSurface += 3 * x - lPitch * y;
        } else {
            pSurface += 3 * x + lPitch * y;
        }

        while (nArcLength != 0) {
            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
            pSurface[2] = static_cast<BYTE>(color >> 16);

            if (*pArcData++ != 0) {
                if (clockwise) {
                    pSurface += 3;
                    x++;
                } else {
                    pSurface -= 3;
                    x--;
                }
            }

            pSurface += lPitch;
            nArcLength--;
        }
    }

    return x;
}

// 0x7C8F70
void CVidMode::DrawLightingEffect24(BYTE* pSurface, LONG lPitch, const CPoint& ptCenter, const CSize& axis, unsigned long a6, unsigned long a7)
{
    BYTE* pPixelList = new BYTE[max(axis.cx, axis.cy)];

    if (axis.cy == 0) {
        // NOTE: Leaks `pPixelList`.
        return;
    }

    if (rgbGlobalTint != RGB(CVidPalette::NO_TINT, CVidPalette::NO_TINT, CVidPalette::NO_TINT)) {
        a7 = RGB((GetRValue(a7) * GetRValue(rgbGlobalTint)) >> 8,
            (GetGValue(a7) * GetGValue(rgbGlobalTint)) >> 8,
            (GetBValue(a7) * GetBValue(rgbGlobalTint)) >> 8);
    }

    BYTE nRed = static_cast<BYTE>(min(GetRValue(a6) + ((255 - GetRValue(a7)) >> 1), 255));
    BYTE nGreen = static_cast<BYTE>(min(GetGValue(a6) + ((255 - GetGValue(a7)) >> 1), 255));
    BYTE nBlue = static_cast<BYTE>(min(GetBValue(a6) + ((255 - GetBValue(a7)) >> 1), 255));

    if (nRed > byte_85DE38) {
        nRed = static_cast<BYTE>((nRed - byte_85DE38) / axis.cy * axis.cy);
    } else {
        nRed = 0;
    }

    if (nGreen > byte_85DE38) {
        nGreen = static_cast<BYTE>((nGreen - byte_85DE38) / axis.cy * axis.cy);
    } else {
        nGreen = 0;
    }

    if (nBlue > byte_85DE38) {
        nBlue = static_cast<BYTE>((nBlue - byte_85DE38) / axis.cy * axis.cy);
    } else {
        nBlue = 0;
    }

    if (nRed == 0 && nGreen == 0 && nBlue == 0) {
        // NOTE: Leaks `pPixelList`.
        return;
    }

    LONG nArcLength = GetEllipseArcPixelList(axis.cx, axis.cy, pPixelList);
    BYTE* pDest = pSurface + (ptCenter.y - axis.cy) * lPitch + 3 * ptCenter.x;

    BYTE nRedStep = static_cast<BYTE>(nRed / axis.cy);
    BYTE nGreenStep = static_cast<BYTE>(nGreen / axis.cy);
    BYTE nBlueStep = static_cast<BYTE>(nBlue / axis.cy);

    BYTE* pPixel;
    INT nR;
    INT nG;
    INT nB;
    WORD nRedLight;
    WORD nGreenLight;
    WORD nBlueLight;
    BYTE nRedInc;
    BYTE nGreenInc;
    BYTE nBlueInc;
    BYTE nRedCur;
    BYTE nGreenCur;
    BYTE nBlueCur;
    SHORT nPass;
    SHORT j;

    // Top and bottom halves, drawn as horizontal spans.
    for (nPass = 0; nPass < 2; nPass++) {
        nRedCur = nRedStep;
        nGreenCur = nGreenStep;
        nBlueCur = nBlueStep;

        for (INT i = 1; i - 1 < nArcLength; i++) {
            if (pPixelList[i - 1] != 0) {
                pPixel = pDest;

                nRedLight = byte_85DE38;
                nGreenLight = byte_85DE38;
                nBlueLight = byte_85DE38;

                nRedInc = static_cast<BYTE>(nRedCur / i);
                nGreenInc = static_cast<BYTE>(nGreenCur / i);
                nBlueInc = static_cast<BYTE>(nBlueCur / i);

                for (j = 0; j < i; j++) {
                    nRedLight += nRedInc;
                    nGreenLight += nGreenInc;
                    nBlueLight += nBlueInc;

                    nR = min((pPixel[0] * nRedLight) >> byte_85DE3A, 255);
                    nG = min((pPixel[1] * nGreenLight) >> byte_85DE3A, 255);
                    nB = min((pPixel[2] * nBlueLight) >> byte_85DE3A, 255);
                    pPixel[0] = static_cast<BYTE>(nR);
                    pPixel[1] = static_cast<BYTE>(nG);
                    pPixel[2] = static_cast<BYTE>(nB);
                    pPixel += 3;
                }

                for (j = 0; j < i - 2; j++) {
                    nRedLight -= nRedInc;
                    nGreenLight -= nGreenInc;
                    nBlueLight -= nBlueInc;

                    nR = min((pPixel[0] * nRedLight) >> byte_85DE3A, 255);
                    nG = min((pPixel[1] * nGreenLight) >> byte_85DE3A, 255);
                    nB = min((pPixel[2] * nBlueLight) >> byte_85DE3A, 255);
                    pPixel[0] = static_cast<BYTE>(nR);
                    pPixel[1] = static_cast<BYTE>(nG);
                    pPixel[2] = static_cast<BYTE>(nB);
                    pPixel += 3;
                }

                // NOTE: The last pixel is unrolled and written even when the
                // loop above does not run (i.e. 2 pixels for a span of 1).
                nRedLight -= nRedInc;
                nGreenLight -= nGreenInc;
                nBlueLight -= nBlueInc;

                nR = min((pPixel[0] * nRedLight) >> byte_85DE3A, 255);
                nG = min((pPixel[1] * nGreenLight) >> byte_85DE3A, 255);
                nB = min((pPixel[2] * nBlueLight) >> byte_85DE3A, 255);
                pPixel[0] = static_cast<BYTE>(nR);
                pPixel[1] = static_cast<BYTE>(nG);
                pPixel[2] = static_cast<BYTE>(nB);

                pDest += lPitch;

                nRedCur += nRedStep;
                nGreenCur += nGreenStep;
                nBlueCur += nBlueStep;
            }

            pDest -= 3;
        }

        if (nPass == 0) {
            pDest = pSurface + (ptCenter.y + axis.cy) * lPitch + 3 * ptCenter.x;
        }

        lPitch = -lPitch;
    }

    // Left and right halves, drawn as horizontal spans from the edge.
    nArcLength = GetEllipseArcPixelList(axis.cy, axis.cx, pPixelList);

    LONG nWidth = axis.cx;
    LONG nWidthMinusOne;
    pDest = pSurface + ptCenter.y * lPitch + 3 * (ptCenter.x - nWidth);

    for (nPass = 0; nPass < 2; nPass++) {
        nRedCur = nRed;
        nGreenCur = nGreen;
        nBlueCur = nBlue;

        if (nArcLength > 0) {
            nWidthMinusOne = nWidth - 1;

            for (INT k = 0; k < nArcLength; k++) {
                pPixel = pDest;

                nRedLight = byte_85DE38;
                nGreenLight = byte_85DE38;
                nBlueLight = byte_85DE38;

                nRedInc = static_cast<BYTE>(nRedCur / nWidth);
                nGreenInc = static_cast<BYTE>(nGreenCur / nWidth);
                nBlueInc = static_cast<BYTE>(nBlueCur / nWidth);

                for (j = 0; j < nWidth; j++) {
                    nRedLight += nRedInc;
                    nGreenLight += nGreenInc;
                    nBlueLight += nBlueInc;

                    nR = min((pPixel[0] * nRedLight) >> byte_85DE3A, 255);
                    nG = min((pPixel[1] * nGreenLight) >> byte_85DE3A, 255);
                    nB = min((pPixel[2] * nBlueLight) >> byte_85DE3A, 255);
                    pPixel[0] = static_cast<BYTE>(nR);
                    pPixel[1] = static_cast<BYTE>(nG);
                    pPixel[2] = static_cast<BYTE>(nB);
                    pPixel += 3;
                }

                for (j = 0; j < nWidthMinusOne; j++) {
                    nRedLight -= nRedInc;
                    nGreenLight -= nGreenInc;
                    nBlueLight -= nBlueInc;

                    nR = min((pPixel[0] * nRedLight) >> byte_85DE3A, 255);
                    nG = min((pPixel[1] * nGreenLight) >> byte_85DE3A, 255);
                    nB = min((pPixel[2] * nBlueLight) >> byte_85DE3A, 255);
                    pPixel[0] = static_cast<BYTE>(nR);
                    pPixel[1] = static_cast<BYTE>(nG);
                    pPixel[2] = static_cast<BYTE>(nB);
                    pPixel += 3;
                }

                // NOTE: The last pixel is unrolled and written even when the
                // loop above does not run.
                nRedLight -= nRedInc;
                nGreenLight -= nGreenInc;
                nBlueLight -= nBlueInc;

                nR = min((pPixel[0] * nRedLight) >> byte_85DE3A, 255);
                nG = min((pPixel[1] * nGreenLight) >> byte_85DE3A, 255);
                nB = min((pPixel[2] * nBlueLight) >> byte_85DE3A, 255);
                pPixel[0] = static_cast<BYTE>(nR);
                pPixel[1] = static_cast<BYTE>(nG);
                pPixel[2] = static_cast<BYTE>(nB);

                if (pPixelList[k] != 0) {
                    pDest += 3;
                    nWidth--;
                    nWidthMinusOne--;
                }

                pDest -= lPitch;

                nRedCur -= nRedStep;
                nGreenCur -= nGreenStep;
                nBlueCur -= nBlueStep;
            }
        }

        if (nPass == 0) {
            pDest = pSurface + (ptCenter.y + 1) * lPitch + 3 * (ptCenter.x - axis.cx + pPixelList[0]);
            nWidth = axis.cx - pPixelList[0];
            nArcLength--;
            pPixelList++;
            lPitch = -lPitch;
        }
    }

    delete (pPixelList - 1);
}

// 0x7C9990
LONG CVidMode::DrawEllipseHOctant24(LONG x, LONG y, INT nArcLength, BYTE* pSurface, LONG lPitch, BOOLEAN clockwise, unsigned char* pArcData, const CRect* rClip, DWORD color)
{
    INT nStep;

    if (rClip != NULL) {
        if (lPitch < 0) {
            pSurface += 3 * (x - rClip->left) - lPitch * (y - rClip->top);
            nStep = -1;
        } else {
            pSurface += 3 * (x - rClip->left) + lPitch * (y - rClip->top);
            nStep = 1;
        }

        while (nArcLength != 0) {
            if (rClip->PtInRect({ x, y })) {
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                pSurface[2] = static_cast<BYTE>(color >> 16);
            }

            if (*pArcData++ != 0) {
                pSurface += lPitch;
                y += nStep;
            }

            if (clockwise) {
                pSurface += 3;
                x++;
            } else {
                pSurface -= 3;
                x--;
            }

            nArcLength--;
        }
    } else {
        if (lPitch < 0) {
            pSurface += 3 * x - lPitch * y;
            nStep = -1;
        } else {
            pSurface += lPitch * y + 3 * x;
            nStep = 1;
        }

        while (nArcLength != 0) {
            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
            pSurface[2] = static_cast<BYTE>(color >> 16);

            if (*pArcData != 0) {
                pSurface += lPitch;
                y += nStep;
            }

            if (clockwise) {
                pSurface += 3;
            } else {
                pSurface -= 3;
            }

            nArcLength--;
            pArcData++;
        }
    }

    return y;
}

// 0x7C9B10
void CVidMode::DrawRecticle24(const CVIDMODE_RECTICLE_DESCRIPTION& recticleDesc, const DDSURFACEDESC& ddsd, const CRect* rClip, COLORREF rgbColor)
{
    BYTE* pPixelList = new BYTE[max(recticleDesc.xAxis, recticleDesc.yAxis)];
    LONG lPitch = ddsd.lPitch;
    DWORD color = (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift);

    INT v1 = recticleDesc.xAxis + recticleDesc.piePieceXOffset;
    INT v2 = recticleDesc.yAxis + recticleDesc.piePieceYOffset;
    INT v3;

    CRect rLineClip;
    BOOLEAN bLineClipped;

    if (rClip != NULL) {
        rLineClip.SetRect(rClip->left, rClip->top, rClip->right - 1, rClip->bottom - 1);
        bLineClipped = FALSE;
    } else {
        rLineClip.SetRect(0, 0, 0, 0);
        bLineClipped = TRUE;
    }

    LONG nArcLength = GetEllipseArcPixelList(recticleDesc.xAxis,
        recticleDesc.yAxis,
        pPixelList);

    nArcLength -= 1;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo24.cpp
    // __LINE__: 721
    UTIL_ASSERT(nArcLength < max(recticleDesc.xAxis, recticleDesc.yAxis));

    v3 = DrawEllipseHOctant24(recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y - v2,
        nArcLength - recticleDesc.xGap,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine24(recticleDesc.ptCenter.x + recticleDesc.xGap - nArcLength,
        v3,
        recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y + recticleDesc.piePiecePtYOffset - v2,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant24(recticleDesc.ptCenter.x + 1,
        recticleDesc.ptCenter.y + pPixelList[0] - v2,
        nArcLength - recticleDesc.xGap - 1,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine24(recticleDesc.ptCenter.x - recticleDesc.xGap + nArcLength,
        v3,
        recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y + recticleDesc.piePiecePtYOffset - v2,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant24(recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y + v2,
        nArcLength - recticleDesc.xGap,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine24(recticleDesc.ptCenter.x + recticleDesc.xGap - nArcLength,
        v3,
        recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y - recticleDesc.piePiecePtYOffset + v2,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant24(recticleDesc.ptCenter.x + 1,
        recticleDesc.ptCenter.y - pPixelList[0] + v2,
        nArcLength - recticleDesc.xGap - 1,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine24(recticleDesc.ptCenter.x - recticleDesc.xGap + nArcLength,
        v3,
        recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y - recticleDesc.piePiecePtYOffset + v2,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    nArcLength = GetEllipseArcPixelList(recticleDesc.yAxis,
        recticleDesc.xAxis,
        pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo24.cpp
    // __LINE__: 753
    UTIL_ASSERT(nArcLength < max(recticleDesc.xAxis, recticleDesc.yAxis));

    v3 = DrawEllipseVOctant24(recticleDesc.ptCenter.x - v1,
        recticleDesc.ptCenter.y,
        nArcLength - recticleDesc.yGap,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList,
        rClip,
        color);

    DrawLine24(v3,
        recticleDesc.ptCenter.y + recticleDesc.yGap - nArcLength,
        recticleDesc.ptCenter.x + recticleDesc.piePiecePtXOffset - v1,
        recticleDesc.ptCenter.y,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant24(recticleDesc.ptCenter.x + pPixelList[0] - v1,
        recticleDesc.ptCenter.y + 1,
        nArcLength - recticleDesc.yGap - 1,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine24(v3,
        recticleDesc.ptCenter.y - recticleDesc.yGap + nArcLength,
        recticleDesc.ptCenter.x + recticleDesc.piePiecePtXOffset - v1,
        recticleDesc.ptCenter.y,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant24(recticleDesc.ptCenter.x + v1,
        recticleDesc.ptCenter.y,
        nArcLength - recticleDesc.yGap,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine24(v3,
        recticleDesc.ptCenter.y + recticleDesc.yGap - nArcLength,
        recticleDesc.ptCenter.x - recticleDesc.piePiecePtXOffset + v1,
        recticleDesc.ptCenter.y,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant24(recticleDesc.ptCenter.x - pPixelList[0] + v1,
        recticleDesc.ptCenter.y + 1,
        nArcLength - recticleDesc.yGap - 1,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine24(v3,
        recticleDesc.ptCenter.y - recticleDesc.yGap + nArcLength,
        recticleDesc.ptCenter.x - recticleDesc.piePiecePtXOffset + v1,
        recticleDesc.ptCenter.y,
        reinterpret_cast<BYTE*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    delete pPixelList;
}

// 0x7CA0D0
BOOL CVidMode::DrawLine24(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, BYTE* pSurface, LONG lPitch, const CRect& rSurface, DWORD color, BOOLEAN bClipped)
{
    INT nDeltaX;
    INT nDeltaY;
    INT nStep = 3;

    if (!bClipped) {
        if (!ClipLine(nXFrom, nYFrom, nXTo, nYTo, rSurface)) {
            return FALSE;
        }
    }

    if (nYFrom > nYTo) {
        INT t;

        t = nXTo;
        nXTo = nXFrom;
        nXFrom = t;

        t = nYTo;
        nYTo = nYFrom;
        nYFrom = t;
    }

    if (nXTo - nXFrom < 0) {
        nDeltaX = nXFrom - nXTo;
        nStep = -3;
    } else {
        nDeltaX = nXTo - nXFrom;
    }

    nDeltaY = nYTo - nYFrom;

    pSurface += lPitch * (nYFrom - rSurface.top) + 3 * (nXFrom - rSurface.left);

    if (nDeltaX == 0) {
        while (nDeltaY >= 0) {
            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
            pSurface[2] = static_cast<BYTE>(color >> 16);
            pSurface += lPitch;
            nDeltaY--;
        }
        return TRUE;
    }

    if (nDeltaY == 0) {
        while (nDeltaX >= 0) {
            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
            pSurface[2] = static_cast<BYTE>(color >> 16);
            pSurface += nStep;
            nDeltaX--;
        }
        return TRUE;
    }

    if (nDeltaX == nDeltaY) {
        while (nDeltaX >= 0) {
            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
            pSurface[2] = static_cast<BYTE>(color >> 16);
            pSurface += lPitch + nStep;
            nDeltaX--;
        }
        return TRUE;
    }

    if (nDeltaX < nDeltaY) {
        ScanLine24(pSurface, nDeltaY, nDeltaX, lPitch, nStep, color);
    } else {
        ScanLine24(pSurface, nDeltaX, nDeltaY, nStep, lPitch, color);
    }

    return TRUE;
}

// 0x7CA240
void CVidMode::ScanLine24(BYTE* pSurface, UINT nMajorAxis, UINT nMinorAxis, INT nMajorAxisAdjust, INT nMinorAxisAdjust, DWORD color)
{
    int v1 = nMajorAxis / nMinorAxis;
    int v2 = 2 * nMinorAxis;
    int v3 = 2 * (nMajorAxis % nMinorAxis);
    int v4 = v3 / 2 - v2;
    int v5 = v1 / 2 + 1;
    int v6 = nMinorAxis - 1;
    int v7 = v1 / 2 + 1;

    if (v3 != 0) {
        if ((v1 & 1) != 0) {
            v4 += nMinorAxis;
        }
    } else {
        if ((v1 & 1) != 0) {
            v4 += nMinorAxis;
        } else {
            v5 -= 1;
        }
    }

    while (v5 > 0) {
        *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
        pSurface[2] = static_cast<BYTE>(color >> 16);
        pSurface += nMajorAxisAdjust;
        v5--;
    }

    pSurface += nMinorAxisAdjust;
    while (v6 > 0) {
        v4 += v3;

        int v8 = v1;
        if (v4 > 0) {
            v8 += 1;
            v4 -= v2;
        }

        while (v8 > 0) {
            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
            pSurface[2] = static_cast<BYTE>(color >> 16);
            pSurface += nMajorAxisAdjust;
            v8--;
        }

        pSurface += nMinorAxisAdjust;

        v6--;
    }

    while (v7 > 0) {
        *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
        pSurface[2] = static_cast<BYTE>(color >> 16);
        pSurface += nMajorAxisAdjust;
        v7--;
    }
}

// 0x7CA330
BOOL CVidMode::DrawThickLine24(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, BYTE* pSurface, LONG lPitch, CRect& rSurface, int a8, DWORD color, BOOLEAN bClipped)
{
    INT nDeltaX;
    INT nDeltaY;
    INT nStep = 3;
    INT nCount = a8 + 1;
    INT nHalf = (a8 + 1) >> 1;
    INT n;

    if (!bClipped) {
        CRect rClip(rSurface.left + nHalf,
            rSurface.top + nHalf,
            rSurface.right - nHalf,
            rSurface.bottom - nHalf);
        if (!ClipLine(nXFrom, nYFrom, nXTo, nYTo, rClip)) {
            return FALSE;
        }
    }

    if (nYFrom > nYTo) {
        INT t;

        t = nXTo;
        nXTo = nXFrom;
        nXFrom = t;

        t = nYTo;
        nYTo = nYFrom;
        nYFrom = t;
    }

    if (nXTo - nXFrom < 0) {
        nDeltaX = nXFrom - nXTo;
        nStep = -3;
    } else {
        nDeltaX = nXTo - nXFrom;
    }

    nDeltaY = nYTo - nYFrom;

    pSurface += lPitch * (nYFrom - rSurface.top) + 3 * (nXFrom - rSurface.left);

    if (nStep > 0) {
        pSurface -= 3 * nHalf;
    } else {
        pSurface += 3 * nHalf;
    }

    pSurface -= nHalf * lPitch;

    if (nDeltaX == 0) {
        if (a8 >= 0) {
            while (nCount > 0) {
                for (n = nDeltaY; n >= 0; n--) {
                    *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                    pSurface[2] = static_cast<BYTE>(color >> 16);
                    pSurface += lPitch;
                }
                pSurface += 3 - nDeltaY * lPitch;
                nCount--;
            }
        }
        return TRUE;
    }

    if (nDeltaY == 0) {
        if (a8 >= 0) {
            while (nCount > 0) {
                for (n = nDeltaX; n >= 0; n--) {
                    *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                    pSurface[2] = static_cast<BYTE>(color >> 16);
                    pSurface += nStep;
                }
                pSurface += lPitch - nDeltaX * nStep;
                nCount--;
            }
        }
        return TRUE;
    }

    if (nDeltaX == nDeltaY) {
        INT nSpan = a8 - 1;
        INT nIndex;

        for (nIndex = 0; nIndex < a8; nIndex++) {
            nSpan += nIndex < nDeltaX ? 1 : 0;

            for (n = nSpan; n > 0; n--) {
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                pSurface[2] = static_cast<BYTE>(color >> 16);
                pSurface += nStep;
            }

            pSurface += lPitch - nSpan * nStep;
        }

        pSurface += nStep;

        for (n = nDeltaY - ((a8 & 1) != 0 ? a8 : a8 - 1); n > 0; n--) {
            for (INT m = nSpan; m > 0; m--) {
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                pSurface[2] = static_cast<BYTE>(color >> 16);
                pSurface += nStep;
            }

            pSurface += lPitch - (nSpan - 1) * nStep;
        }

        if (a8 > 0) {
            INT nAdjust = lPitch + (1 - nSpan) * nStep;

            for (n = a8; n > 0; n--) {
                for (INT m = nSpan; m > 0; m--) {
                    *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                    pSurface[2] = static_cast<BYTE>(color >> 16);
                    pSurface += nStep;
                }

                pSurface += nAdjust;
                nSpan--;
                nAdjust += nStep;
            }
        }

        return TRUE;
    }

    if (nDeltaX < nDeltaY) {
        ThickScanLine24(pSurface, nDeltaY, nDeltaX, lPitch, nStep, a8, color);
    } else {
        ThickScanLine24(pSurface, nDeltaX, nDeltaY, nStep, lPitch, a8, color);
    }

    return TRUE;
}

// 0x7CA630
void CVidMode::ThickScanLine24(BYTE* pSurface, UINT nMajorAxis, UINT nMinorAxis, INT nMajorAxisAdjust, INT nMinorAxisAdjust, UINT nThickness, DWORD color)
{
    UINT nRun = nMajorAxis / nMinorAxis;
    INT nErrorAdjust = 2 * (nMajorAxis % nMinorAxis);
    INT nErrorReset = 2 * nMinorAxis;
    INT nError = nErrorAdjust / 2 - nErrorReset;
    UINT nFinalRun = nRun / 2 + 1;
    UINT nFirstRun = nRun / 2 + 1;
    UINT nDrawn = 0;
    UINT nEnd;
    UINT nTailRun;
    INT nTailError;
    INT nDelay;
    UINT n;

    if (nErrorAdjust != 0) {
        if ((nRun & 1) != 0) {
            nError += nMinorAxis;
        }
    } else {
        if ((nRun & 1) != 0) {
            nError += nMinorAxis;
        } else {
            nFirstRun -= 1;
        }
    }

    nTailError = nError;
    nTailRun = nFirstRun;

    nEnd = (nThickness & ~1) + nFirstRun;
    for (n = nEnd; n != 0; n--) {
        *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
        pSurface[2] = static_cast<BYTE>(color >> 16);
        pSurface += nMajorAxisAdjust;
    }

    pSurface += nMinorAxisAdjust - static_cast<INT>(nEnd) * nMajorAxisAdjust;

    nDelay = nThickness - 2;

    for (UINT nLine = nMinorAxis - 1; nLine != 0; nLine--) {
        UINT nRunLength = nRun;
        nError += nErrorAdjust;
        if (nError > 0) {
            nRunLength++;
            nError -= nErrorReset;
        }

        nEnd += nRunLength;

        if (nDrawn < nEnd) {
            for (n = nEnd - nDrawn; n != 0; n--) {
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                pSurface[2] = static_cast<BYTE>(color >> 16);
                pSurface += nMajorAxisAdjust;
            }
        }

        pSurface -= static_cast<INT>(nEnd - nDrawn) * nMajorAxisAdjust;

        if (nDelay == 0) {
            pSurface += static_cast<INT>(nTailRun) * nMajorAxisAdjust;
            nDrawn += nTailRun;

            nTailRun = nRun;
            nTailError += nErrorAdjust;
            if (nTailError > 0) {
                nTailRun++;
                nTailError -= nErrorReset;
            }
        } else {
            nDelay--;
        }

        pSurface += nMinorAxisAdjust;
    }

    nEnd += nFinalRun;

    for (UINT nIndex = 0; nIndex < nThickness; nIndex++) {
        if (nDrawn < nEnd) {
            for (n = nEnd - nDrawn; n != 0; n--) {
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(color);
                pSurface[2] = static_cast<BYTE>(color >> 16);
                pSurface += nMajorAxisAdjust;
            }
        }

        pSurface -= static_cast<INT>(nEnd - nDrawn) * nMajorAxisAdjust;

        if (nIndex == nThickness - 1) {
            nTailRun = nFinalRun;
        }

        pSurface += static_cast<INT>(nTailRun) * nMajorAxisAdjust;
        nDrawn += nTailRun;

        nTailRun = nRun;
        nTailError += nErrorAdjust;
        if (nTailError > 0) {
            nTailRun++;
            nTailError -= nErrorReset;
        }

        pSurface += nMinorAxisAdjust;
    }
}

// 0x7CA840
LONG CVidMode::DrawEllipse32(const CPoint& ptCenter, const CSize& axis, const DDSURFACEDESC& ddsd, const CRect* rClip, COLORREF rgbColor)
{
    BYTE* pPixelList = new BYTE[max(axis.cx, axis.cy)];
    LONG lPitch = ddsd.lPitch / 4;
    DWORD color = (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift);
    LONG nArcLength = GetEllipseArcPixelList(axis.cx, axis.cy, pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo24.cpp
    // __LINE__: 1474
    UTIL_ASSERT(nArcLength <= max(axis.cx, axis.cy));

    DrawEllipseHOctant32(ptCenter.x,
        ptCenter.y - axis.cy,
        nArcLength,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseHOctant32(ptCenter.x + 1,
        ptCenter.y + pPixelList[0] - axis.cy,
        nArcLength - 1,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawEllipseHOctant32(ptCenter.x,
        ptCenter.y + axis.cy,
        nArcLength,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseHOctant32(ptCenter.x + 1,
        ptCenter.y - pPixelList[0] + axis.cy,
        nArcLength - 1,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    nArcLength = GetEllipseArcPixelList(axis.cy, axis.cx, pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo24.cpp
    // __LINE__: 1497
    UTIL_ASSERT(nArcLength < max(axis.cx, axis.cy));

    DrawEllipseVOctant32(ptCenter.x - axis.cx,
        ptCenter.y,
        nArcLength,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList,
        rClip,
        color);

    DrawEllipseVOctant32(ptCenter.x + pPixelList[0] - axis.cx,
        ptCenter.y + 1,
        nArcLength - 1,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawEllipseVOctant32(ptCenter.x + axis.cx,
        ptCenter.y,
        nArcLength,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawEllipseVOctant32(ptCenter.x - pPixelList[0] + axis.cx,
        ptCenter.y + 1,
        nArcLength - 1,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList + 1,
        rClip,
        color);

    delete pPixelList;

    return 0;
}

// 0x7CAB20
LONG CVidMode::DrawEllipseVOctant32(LONG x, LONG y, INT nArcLength, DWORD* pSurface, LONG lPitch, BOOLEAN clockwise, BYTE* pPixelList, const CRect* rClip, DWORD color)
{
    INT nStep;

    if (rClip != NULL) {
        if (lPitch < 0) {
            pSurface += -lPitch * (y - rClip->top) - rClip->left + x;
            nStep = -1;
        } else {
            pSurface += lPitch * (y - rClip->top) - rClip->left + x;
            nStep = 1;
        }

        while (nArcLength != 0) {
            if (rClip->PtInRect({ x, y })) {
                *pSurface = color;
            }

            if (*pPixelList++ != 0) {
                if (clockwise) {
                    pSurface++;
                    x++;
                } else {
                    pSurface--;
                    x--;
                }
            }

            pSurface += lPitch;
            y += nStep;

            nArcLength--;
        }
    } else {
        if (lPitch < 0) {
            pSurface += -lPitch * y + x;
        } else {
            pSurface += lPitch * y + x;
        }

        while (nArcLength != 0) {
            *pSurface = color;

            if (*pPixelList++ != 0) {
                if (clockwise) {
                    pSurface++;
                    x++;
                } else {
                    pSurface--;
                    x--;
                }
            }

            pSurface += lPitch;
            nArcLength--;
        }
    }

    return x;
}

// 0x7CAC60
void CVidMode::DrawLightingEffect32(DWORD* pSurface, LONG lPitch, const CPoint& ptCenter, const CSize& axis, unsigned long a6, unsigned long a7)
{
    BYTE* pPixelList = new BYTE[max(axis.cx, axis.cy)];

    if (axis.cy == 0) {
        // NOTE: Leaks `pPixelList`.
        return;
    }

    if (rgbGlobalTint != RGB(CVidPalette::NO_TINT, CVidPalette::NO_TINT, CVidPalette::NO_TINT)) {
        a7 = RGB((GetRValue(a7) * GetRValue(rgbGlobalTint)) >> 8,
            (GetGValue(a7) * GetGValue(rgbGlobalTint)) >> 8,
            (GetBValue(a7) * GetBValue(rgbGlobalTint)) >> 8);
    }

    BYTE nRed = static_cast<BYTE>(min(GetRValue(a6) + ((255 - GetRValue(a7)) >> 1), 255));
    BYTE nGreen = static_cast<BYTE>(min(GetGValue(a6) + ((255 - GetGValue(a7)) >> 1), 255));
    BYTE nBlue = static_cast<BYTE>(min(GetBValue(a6) + ((255 - GetBValue(a7)) >> 1), 255));

    if (nRed > byte_85DE38) {
        nRed = static_cast<BYTE>((nRed - byte_85DE38) / axis.cy * axis.cy);
    } else {
        nRed = 0;
    }

    if (nGreen > byte_85DE38) {
        nGreen = static_cast<BYTE>((nGreen - byte_85DE38) / axis.cy * axis.cy);
    } else {
        nGreen = 0;
    }

    if (nBlue > byte_85DE38) {
        nBlue = static_cast<BYTE>((nBlue - byte_85DE38) / axis.cy * axis.cy);
    } else {
        nBlue = 0;
    }

    if (nRed == 0 && nGreen == 0 && nBlue == 0) {
        // NOTE: Leaks `pPixelList`.
        return;
    }

    LONG nArcLength = GetEllipseArcPixelList(axis.cx, axis.cy, pPixelList);
    DWORD* pDest = pSurface + (ptCenter.y - axis.cy) * lPitch + ptCenter.x;

    BYTE nRedStep = static_cast<BYTE>(nRed / axis.cy);
    BYTE nGreenStep = static_cast<BYTE>(nGreen / axis.cy);
    BYTE nBlueStep = static_cast<BYTE>(nBlue / axis.cy);

    DWORD* pPixel;
    DWORD nPixel;
    INT nR;
    INT nG;
    INT nB;
    BYTE nRedLight;
    BYTE nGreenLight;
    BYTE nBlueLight;
    BYTE nRedInc;
    BYTE nGreenInc;
    BYTE nBlueInc;
    BYTE nRedCur;
    BYTE nGreenCur;
    BYTE nBlueCur;
    SHORT nPass;
    SHORT j;

    // Top and bottom halves, drawn as horizontal spans.
    for (nPass = 0; nPass < 2; nPass++) {
        nRedCur = nRedStep;
        nGreenCur = nGreenStep;
        nBlueCur = nBlueStep;

        for (INT i = 1; i - 1 < nArcLength; i++) {
            if (pPixelList[i - 1] != 0) {
                pPixel = pDest;

                nRedLight = byte_85DE38;
                nGreenLight = byte_85DE38;
                nBlueLight = byte_85DE38;

                nRedInc = static_cast<BYTE>(nRedCur / i);
                nGreenInc = static_cast<BYTE>(nGreenCur / i);
                nBlueInc = static_cast<BYTE>(nBlueCur / i);

                for (j = 0; j < i; j++) {
                    nRedLight += nRedInc;
                    nGreenLight += nGreenInc;
                    nBlueLight += nBlueInc;

                    nPixel = *pPixel;
                    nR = min((static_cast<INT>(nPixel & 0xFF) * nRedLight) >> byte_85DE3A, 255);
                    nG = min((static_cast<INT>((nPixel >> 8) & 0xFF) * nGreenLight) >> byte_85DE3A, 255);
                    nB = min((static_cast<INT>((nPixel >> 16) & 0xFF) * nBlueLight) >> byte_85DE3A, 255);
                    *pPixel++ = (((nB << 8) | nG) << 8) | nR;
                }

                for (j = 0; j < i - 1; j++) {
                    nRedLight -= nRedInc;
                    nGreenLight -= nGreenInc;
                    nBlueLight -= nBlueInc;

                    nPixel = *pPixel;
                    nR = min((static_cast<INT>(nPixel & 0xFF) * nRedLight) >> byte_85DE3A, 255);
                    nG = min((static_cast<INT>((nPixel >> 8) & 0xFF) * nGreenLight) >> byte_85DE3A, 255);
                    nB = min((static_cast<INT>((nPixel >> 16) & 0xFF) * nBlueLight) >> byte_85DE3A, 255);
                    *pPixel++ = (((nB << 8) | nG) << 8) | nR;
                }

                pDest += lPitch;

                nRedCur += nRedStep;
                nGreenCur += nGreenStep;
                nBlueCur += nBlueStep;
            }

            pDest--;
        }

        if (nPass == 0) {
            pDest = pSurface + (ptCenter.y + axis.cy) * lPitch + ptCenter.x;
        }

        lPitch = -lPitch;
    }

    // Left and right halves, drawn as horizontal spans from the edge.
    nArcLength = GetEllipseArcPixelList(axis.cy, axis.cx, pPixelList);

    LONG nWidth = axis.cx;
    LONG nWidthMinusOne;
    pDest = pSurface + ptCenter.y * lPitch - nWidth + ptCenter.x;

    for (nPass = 0; nPass < 2; nPass++) {
        nRedCur = nRed;
        nGreenCur = nGreen;
        nBlueCur = nBlue;

        if (nArcLength > 0) {
            nWidthMinusOne = nWidth - 1;

            for (INT k = 0; k < nArcLength; k++) {
                pPixel = pDest;

                nRedLight = byte_85DE38;
                nGreenLight = byte_85DE38;
                nBlueLight = byte_85DE38;

                nRedInc = static_cast<BYTE>(nRedCur / nWidth);
                nGreenInc = static_cast<BYTE>(nGreenCur / nWidth);
                nBlueInc = static_cast<BYTE>(nBlueCur / nWidth);

                for (j = 0; j < nWidth; j++) {
                    nRedLight += nRedInc;
                    nGreenLight += nGreenInc;
                    nBlueLight += nBlueInc;

                    nPixel = *pPixel;
                    nR = min((static_cast<INT>(nPixel & 0xFF) * nRedLight) >> byte_85DE3A, 255);
                    nG = min((static_cast<INT>((nPixel >> 8) & 0xFF) * nGreenLight) >> byte_85DE3A, 255);
                    nB = min((static_cast<INT>((nPixel >> 16) & 0xFF) * nBlueLight) >> byte_85DE3A, 255);
                    *pPixel++ = (((nB << 8) | nG) << 8) | nR;
                }

                // NOTE: Only half of the remaining span is lit, unlike the
                // 24 bpp version.
                for (j = 0; j < (nWidthMinusOne >> 1); j++) {
                    nRedLight -= nRedInc;
                    nGreenLight -= nGreenInc;
                    nBlueLight -= nBlueInc;

                    nPixel = *pPixel;
                    nR = min((static_cast<INT>(nPixel & 0xFF) * nRedLight) >> byte_85DE3A, 255);
                    nG = min((static_cast<INT>((nPixel >> 8) & 0xFF) * nGreenLight) >> byte_85DE3A, 255);
                    nB = min((static_cast<INT>((nPixel >> 16) & 0xFF) * nBlueLight) >> byte_85DE3A, 255);
                    *pPixel++ = (((nB << 8) | nG) << 8) | nR;
                }

                if (pPixelList[k] != 0) {
                    pDest++;
                    nWidth--;
                    nWidthMinusOne--;
                }

                pDest -= lPitch;

                nRedCur -= nRedStep;
                nGreenCur -= nGreenStep;
                nBlueCur -= nBlueStep;
            }
        }

        if (nPass == 0) {
            pDest = pSurface + (ptCenter.y + 1) * lPitch - axis.cx + ptCenter.x + pPixelList[0];
            nWidth = axis.cx - pPixelList[0];
            nArcLength--;
            pPixelList++;
            lPitch = -lPitch;
        }
    }

    delete (pPixelList - 1);
}

// 0x7CB510
LONG CVidMode::DrawEllipseHOctant32(LONG x, LONG y, INT nArcLength, DWORD* pSurface, LONG lPitch, BOOLEAN clockwise, BYTE* pPixelList, const CRect* rClip, DWORD color)
{
    INT nStep;

    if (rClip != NULL) {
        if (lPitch < 0) {
            pSurface += -lPitch * (y - rClip->top) - rClip->left + x;
            nStep = -1;
        } else {
            pSurface += lPitch * (y - rClip->top) - rClip->left + x;
            nStep = 1;
        }

        while (nArcLength != 0) {
            if (rClip->PtInRect({ x, y })) {
                *pSurface = color;
            }

            if (*pPixelList++ != 0) {
                pSurface += lPitch;
                y += nStep;
            }

            if (clockwise) {
                pSurface++;
                x++;
            } else {
                pSurface--;
                x--;
            }

            nArcLength--;
        }
    } else {
        if (lPitch < 0) {
            pSurface += -lPitch * y + x;
            nStep = -1;
        } else {
            pSurface += lPitch * y + x;
            nStep = 1;
        }

        while (nArcLength != 0) {
            *pSurface = color;

            if (*pPixelList++ != 0) {
                pSurface += lPitch;
                y += nStep;
            }

            if (clockwise) {
                pSurface++;
            } else {
                pSurface--;
            }

            nArcLength--;
        }
    }

    return y;
}

// 0x7CB670
void CVidMode::DrawRecticle32(const CVIDMODE_RECTICLE_DESCRIPTION& recticleDesc, const DDSURFACEDESC& ddsd, const CRect* rClip, COLORREF rgbColor)
{
    BYTE* pPixelList = new BYTE[max(recticleDesc.xAxis, recticleDesc.yAxis)];
    LONG lPitch = ddsd.lPitch / 4;
    DWORD color = (GetRValue(rgbColor) << m_dwRBitShift) | (GetGValue(rgbColor) << m_dwGBitShift) | (GetBValue(rgbColor) << m_dwBBitShift);

    INT v1 = recticleDesc.xAxis + recticleDesc.piePieceXOffset;
    INT v2 = recticleDesc.yAxis + recticleDesc.piePieceYOffset;
    INT v3;

    CRect rLineClip;
    BOOLEAN bLineClipped;

    if (rClip != NULL) {
        rLineClip.SetRect(rClip->left, rClip->top, rClip->right - 1, rClip->bottom - 1);
        bLineClipped = FALSE;
    } else {
        rLineClip.SetRect(0, 0, 0, 0);
        bLineClipped = TRUE;
    }

    LONG nArcLength = GetEllipseArcPixelList(recticleDesc.xAxis,
        recticleDesc.yAxis,
        pPixelList);

    nArcLength -= 1;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo24.cpp
    // __LINE__: 2097
    UTIL_ASSERT(nArcLength < max(recticleDesc.xAxis, recticleDesc.yAxis));

    v3 = DrawEllipseHOctant32(recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y - v2,
        nArcLength - recticleDesc.xGap,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine32(recticleDesc.ptCenter.x + recticleDesc.xGap - nArcLength,
        v3,
        recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y + recticleDesc.piePiecePtYOffset - v2,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant32(recticleDesc.ptCenter.x + 1,
        recticleDesc.ptCenter.y + pPixelList[0] - v2,
        nArcLength - recticleDesc.xGap - 1,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine32(recticleDesc.ptCenter.x - recticleDesc.xGap + nArcLength,
        v3,
        recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y + recticleDesc.piePiecePtYOffset - v2,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant32(recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y + v2,
        nArcLength - recticleDesc.xGap,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine32(recticleDesc.ptCenter.x + recticleDesc.xGap - nArcLength,
        v3,
        recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y - recticleDesc.piePiecePtYOffset + v2,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseHOctant32(recticleDesc.ptCenter.x + 1,
        recticleDesc.ptCenter.y - pPixelList[0] + v2,
        nArcLength - recticleDesc.xGap - 1,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine32(recticleDesc.ptCenter.x - recticleDesc.xGap + nArcLength,
        v3,
        recticleDesc.ptCenter.x,
        recticleDesc.ptCenter.y - recticleDesc.piePiecePtYOffset + v2,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    nArcLength = GetEllipseArcPixelList(recticleDesc.yAxis,
        recticleDesc.xAxis,
        pPixelList);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVideo24.cpp
    // __LINE__: 2129
    UTIL_ASSERT(nArcLength < max(recticleDesc.xAxis, recticleDesc.yAxis));

    v3 = DrawEllipseVOctant32(recticleDesc.ptCenter.x - v1,
        recticleDesc.ptCenter.y,
        nArcLength - recticleDesc.yGap,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        -lPitch,
        TRUE,
        pPixelList,
        rClip,
        color);

    DrawLine32(v3,
        recticleDesc.ptCenter.y + recticleDesc.yGap - nArcLength,
        recticleDesc.ptCenter.x + recticleDesc.piePiecePtXOffset - v1,
        recticleDesc.ptCenter.y,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant32(recticleDesc.ptCenter.x + pPixelList[0] - v1,
        recticleDesc.ptCenter.y + 1,
        nArcLength - recticleDesc.yGap - 1,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        TRUE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine32(v3,
        recticleDesc.ptCenter.y - recticleDesc.yGap + nArcLength,
        recticleDesc.ptCenter.x + recticleDesc.piePiecePtXOffset - v1,
        recticleDesc.ptCenter.y,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant32(recticleDesc.ptCenter.x + v1,
        recticleDesc.ptCenter.y,
        nArcLength - recticleDesc.yGap,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        -lPitch,
        FALSE,
        pPixelList,
        rClip,
        color);

    DrawLine32(v3,
        recticleDesc.ptCenter.y + recticleDesc.yGap - nArcLength,
        recticleDesc.ptCenter.x - recticleDesc.piePiecePtXOffset + v1,
        recticleDesc.ptCenter.y,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    v3 = DrawEllipseVOctant32(recticleDesc.ptCenter.x - pPixelList[0] + v1,
        recticleDesc.ptCenter.y + 1,
        nArcLength - recticleDesc.yGap - 1,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        FALSE,
        pPixelList + 1,
        rClip,
        color);

    DrawLine32(v3,
        recticleDesc.ptCenter.y - recticleDesc.yGap + nArcLength,
        recticleDesc.ptCenter.x - recticleDesc.piePiecePtXOffset + v1,
        recticleDesc.ptCenter.y,
        reinterpret_cast<DWORD*>(ddsd.lpSurface),
        lPitch,
        rLineClip,
        color,
        bLineClipped);

    delete pPixelList;
}

// 0x7CBC40
BOOL CVidMode::DrawLine32(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, DWORD* pSurface, LONG lPitch, const CRect& rSurface, DWORD color, BOOLEAN bClipped)
{
    INT nDeltaX;
    INT nDeltaY;
    INT nStep;

    if (!bClipped) {
        if (!ClipLine(nXFrom, nYFrom, nXTo, nYTo, rSurface)) {
            return FALSE;
        }
    }

    if (nYFrom > nYTo) {
        INT t;

        t = nXTo;
        nXTo = nXFrom;
        nXFrom = t;

        t = nYTo;
        nYTo = nYFrom;
        nYFrom = t;
    }

    if (nXTo - nXFrom < 0) {
        nDeltaX = nXFrom - nXTo;
        nStep = -1;
    } else {
        nDeltaX = nXTo - nXFrom;
        nStep = 1;
    }

    nDeltaY = nYTo - nYFrom;

    pSurface += lPitch * (nYFrom - rSurface.top) - rSurface.left + nXFrom;

    if (nDeltaX == 0) {
        while (nDeltaY >= 0) {
            *pSurface = color;
            pSurface += lPitch;
            nDeltaY--;
        }
        return TRUE;
    }

    if (nDeltaY == 0) {
        while (nDeltaX >= 0) {
            *pSurface = color;
            pSurface += nStep;
            nDeltaX--;
        }
        return TRUE;
    }

    if (nDeltaX == nDeltaY) {
        while (nDeltaX >= 0) {
            *pSurface = color;
            pSurface += lPitch + nStep;
            nDeltaX--;
        }
        return TRUE;
    }

    if (nDeltaX < nDeltaY) {
        ScanLine32(pSurface, nDeltaY, nDeltaX, lPitch, nStep, color);
    } else {
        ScanLine32(pSurface, nDeltaX, nDeltaY, nStep, lPitch, color);
    }

    return TRUE;
}

// 0x7CBD80
void CVidMode::ScanLine32(DWORD* pSurface, UINT nMajorAxis, UINT nMinorAxis, INT nMajorAxisAdjust, INT nMinorAxisAdjust, DWORD color)
{
    int v1 = nMajorAxis / nMinorAxis;
    int v2 = 2 * nMinorAxis;
    int v3 = 2 * (nMajorAxis % nMinorAxis);
    int v4 = v3 / 2 - v2;
    int v5 = v1 / 2 + 1;
    int v6 = nMinorAxis - 1;
    int v7 = v1 / 2 + 1;

    if (v3 != 0) {
        if ((v1 & 1) != 0) {
            v4 += nMinorAxis;
        }
    } else {
        if ((v1 & 1) != 0) {
            v4 += nMinorAxis;
        } else {
            v5 -= 1;
        }
    }

    while (v5 > 0) {
        *pSurface = color;
        pSurface += nMajorAxisAdjust;
        v5--;
    }

    pSurface += nMinorAxisAdjust;
    while (v6 > 0) {
        v4 += v3;

        int v8 = v1;
        if (v4 > 0) {
            v8 += 1;
            v4 -= v2;
        }

        while (v8 > 0) {
            *pSurface = color;
            pSurface += nMajorAxisAdjust;
            v8--;
        }

        pSurface += nMinorAxisAdjust;

        v6--;
    }

    while (v7 > 0) {
        *pSurface = color;
        pSurface += nMajorAxisAdjust;
        v7--;
    }
}

// 0x7CBE70
BOOL CVidMode::DrawThickLine32(INT nXFrom, INT nYFrom, INT nXTo, INT nYTo, DWORD* pSurface, LONG lPitch, CRect& rSurface, int a8, DWORD color, BOOLEAN bClipped)
{
    INT nDeltaX;
    INT nDeltaY;
    INT nStep = 1;
    INT nCount = a8 + 1;
    INT nHalf = (a8 + 1) >> 1;
    INT n;

    if (!bClipped) {
        CRect rClip(rSurface.left + nHalf,
            rSurface.top + nHalf,
            rSurface.right - nHalf,
            rSurface.bottom - nHalf);
        if (!ClipLine(nXFrom, nYFrom, nXTo, nYTo, rClip)) {
            return FALSE;
        }
    }

    if (nYFrom > nYTo) {
        INT t;

        t = nXTo;
        nXTo = nXFrom;
        nXFrom = t;

        t = nYTo;
        nYTo = nYFrom;
        nYFrom = t;
    }

    if (nXTo - nXFrom < 0) {
        nDeltaX = nXFrom - nXTo;
        nStep = -1;
    } else {
        nDeltaX = nXTo - nXFrom;
    }

    nDeltaY = nYTo - nYFrom;

    pSurface += lPitch * (nYFrom - rSurface.top) - rSurface.left + nXFrom;

    if (nStep > 0) {
        pSurface -= nHalf;
    } else {
        pSurface += nHalf;
    }

    pSurface -= nHalf * lPitch;

    if (nDeltaX == 0) {
        if (a8 >= 0) {
            while (nCount > 0) {
                for (n = nDeltaY; n >= 0; n--) {
                    *pSurface = color;
                    pSurface += lPitch;
                }
                pSurface += 1 - nDeltaY * lPitch;
                nCount--;
            }
        }
        return TRUE;
    }

    if (nDeltaY == 0) {
        if (a8 >= 0) {
            while (nCount > 0) {
                for (n = nDeltaX; n >= 0; n--) {
                    *pSurface = color;
                    pSurface += nStep;
                }
                pSurface += lPitch - nDeltaX * nStep;
                nCount--;
            }
        }
        return TRUE;
    }

    if (nDeltaX == nDeltaY) {
        INT nSpan = a8 - 1;
        INT nIndex;

        for (nIndex = 0; nIndex < a8; nIndex++) {
            nSpan += nIndex < nDeltaX ? 1 : 0;

            for (n = nSpan; n > 0; n--) {
                *pSurface = color;
                pSurface += nStep;
            }

            pSurface += lPitch - nSpan * nStep;
        }

        pSurface += nStep;

        for (n = nDeltaY - ((a8 & 1) != 0 ? a8 : a8 - 1); n > 0; n--) {
            for (INT m = nSpan; m > 0; m--) {
                *pSurface = color;
                pSurface += nStep;
            }

            pSurface += lPitch - (nSpan - 1) * nStep;
        }

        if (a8 > 0) {
            INT nAdjust = lPitch + (1 - nSpan) * nStep;

            for (n = a8; n > 0; n--) {
                for (INT m = nSpan; m > 0; m--) {
                    *pSurface = color;
                    pSurface += nStep;
                }

                pSurface += nAdjust;
                nSpan--;
                nAdjust += nStep;
            }
        }

        return TRUE;
    }

    if (nDeltaX < nDeltaY) {
        ThickScanLine32(pSurface, nDeltaY, nDeltaX, lPitch, nStep, a8, color);
    } else {
        ThickScanLine32(pSurface, nDeltaX, nDeltaY, nStep, lPitch, a8, color);
    }

    return TRUE;
}

// 0x7CC130
void CVidMode::ThickScanLine32(DWORD* pSurface, UINT nMajorAxis, UINT nMinorAxis, INT nMajorAxisAdjust, INT nMinorAxisAdjust, UINT nThickness, DWORD color)
{
    UINT nRun = nMajorAxis / nMinorAxis;
    INT nErrorAdjust = 2 * (nMajorAxis % nMinorAxis);
    INT nErrorReset = 2 * nMinorAxis;
    INT nError = nErrorAdjust / 2 - nErrorReset;
    UINT nFinalRun = nRun / 2 + 1;
    UINT nFirstRun = nRun / 2 + 1;
    UINT nDrawn = 0;
    UINT nEnd;
    UINT nTailRun;
    INT nTailError;
    INT nDelay;
    UINT n;

    if (nErrorAdjust != 0) {
        if ((nRun & 1) != 0) {
            nError += nMinorAxis;
        }
    } else {
        if ((nRun & 1) != 0) {
            nError += nMinorAxis;
        } else {
            nFirstRun -= 1;
        }
    }

    nTailError = nError;
    nTailRun = nFirstRun;

    nEnd = (nThickness & ~1) + nFirstRun;
    for (n = nEnd; n != 0; n--) {
        *pSurface = color;
        pSurface += nMajorAxisAdjust;
    }

    pSurface += nMinorAxisAdjust - static_cast<INT>(nEnd) * nMajorAxisAdjust;

    nDelay = nThickness - 2;

    for (UINT nLine = nMinorAxis - 1; nLine != 0; nLine--) {
        UINT nRunLength = nRun;
        nError += nErrorAdjust;
        if (nError > 0) {
            nRunLength++;
            nError -= nErrorReset;
        }

        nEnd += nRunLength;

        if (nDrawn < nEnd) {
            for (n = nEnd - nDrawn; n != 0; n--) {
                *pSurface = color;
                pSurface += nMajorAxisAdjust;
            }
        }

        pSurface -= static_cast<INT>(nEnd - nDrawn) * nMajorAxisAdjust;

        if (nDelay == 0) {
            pSurface += static_cast<INT>(nTailRun) * nMajorAxisAdjust;
            nDrawn += nTailRun;

            nTailRun = nRun;
            nTailError += nErrorAdjust;
            if (nTailError > 0) {
                nTailRun++;
                nTailError -= nErrorReset;
            }
        } else {
            nDelay--;
        }

        pSurface += nMinorAxisAdjust;
    }

    nEnd += nFinalRun;

    for (UINT nIndex = 0; nIndex < nThickness; nIndex++) {
        if (nDrawn < nEnd) {
            for (n = nEnd - nDrawn; n != 0; n--) {
                *pSurface = color;
                pSurface += nMajorAxisAdjust;
            }
        }

        pSurface -= static_cast<INT>(nEnd - nDrawn) * nMajorAxisAdjust;

        if (nIndex == nThickness - 1) {
            nTailRun = nFinalRun;
        }

        pSurface += static_cast<INT>(nTailRun) * nMajorAxisAdjust;
        nDrawn += nTailRun;

        nTailRun = nRun;
        nTailError += nErrorAdjust;
        if (nTailError > 0) {
            nTailRun++;
            nTailError -= nErrorReset;
        }

        pSurface += nMinorAxisAdjust;
    }
}

// 0x799E20
BOOL CVidMode::CreateSurfaces(BOOLEAN bFullscreen)
{
    return FALSE;
}

// 0x799C90
BOOL CVidMode::DeactivateVideoMode(CVidMode* pNextVidMode)
{
    return TRUE;
}

// 0x799E20
IDirectDrawSurface* CVidMode::GetFXSurfacePtr(DWORD dwFlags)
{
    return NULL;
}

// 0x7ABB90
RGBQUAD* CVidMode::GetPalette()
{
    return NULL;
}

// 0x78E6E0
int CVidMode::GetNumVRamSurfaces()
{
    return 0;
}

// 0x7B5450
void CVidMode::RenderFlash(UINT nSurface, COLORREF rgbColor, unsigned char a4, const CRect& rClip)
{
}

// 0x799C90
BOOL CVidMode::RenderPointer(UINT nSurface)
{
    return TRUE;
}

// 0x799C90
BOOL CVidMode::Flip(BOOL bRenderCursor)
{
    return TRUE;
}

// 0x799E60
void CVidMode::ParsePixelFormat(const DDPIXELFORMAT& ddpf)
{
}

// NOTE: Inlined.
BOOL CVidMode::GetFullyFaded()
{
    return m_nFade == 0;
}

// NOTE: Inlined.
void CVidMode::UpdateFade()
{
    if (m_bFadeTo) {
        if (m_nFade > 0) {
            m_nFade--;
        }
    } else {
        if (m_nFade < NUM_FADE_FRAMES) {
            m_nFade++;
        }
    }
}

// FIXME: `rgbFade` is misleading (both type and meaning). This function is
// usually called via inlined non-static `CInfinity::SetFade`, but I'm not sure
// how `CInfinity` instance is obtained in the first place.
//
// NOTE: Inlined.
void CVidMode::SetFade(BOOL bFadeTo, COLORREF rgbFade)
{
    if (rgbFade != 0) {
        NUM_FADE_FRAMES = static_cast<BYTE>(rgbFade);
    }

    m_bFadeTo = bFadeTo;

    if (m_bFadeTo) {
        m_nFade = NUM_FADE_FRAMES;
    } else {
        m_nFade = 0;
    }
}
