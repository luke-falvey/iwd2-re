#include "CVidImage.h"

#include "CChitin.h"
#include "CResTile.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVideo3d.h"

// 0x85E838
const INT CVidTile::BYTES_PER_TEXEL = 4;

// 0xA09FC8
BYTE CVidTile::m_pPixels[16384];

// 0x9064F8
DWORD CVidImage::rgbTempPal[256];

// 0x9060F0
BYTE* CVidImage::m_pTranslucentTable;

// 0x9060F4
DWORD CVidImage::rgbBlendPal[256];

// 0x9068F8
INT CVidImage::m_nTranslucentTableRefs;

// 0x7ACA60
CVidImage::CVidImage()
{
    m_nFxSurface = -1;
    m_paletteAffects.rgbTintColor = 0xFFFFFF;
    m_paletteAffects.rgbAddColor = 0;
    m_paletteAffects.rgbLightColor = (1 << CVidPalette::LIGHT_SCALE) | ((1 << CVidPalette::LIGHT_SCALE) << 8) | ((1 << CVidPalette::LIGHT_SCALE) << 16);
    memset(m_paletteAffects.pRangeTints, 0, sizeof(m_paletteAffects.pRangeTints[0]) * CVidPalette::NUM_RANGES);
    memset(m_paletteAffects.pRangeAdds, 0, sizeof(m_paletteAffects.pRangeAdds[0]) * CVidPalette::NUM_RANGES);
    memset(m_paletteAffects.pRangeLights, 0, sizeof(m_paletteAffects.pRangeLights[0]) * CVidPalette::NUM_RANGES);
    memset(m_paletteAffects.aRangeTintPeriods, 0, sizeof(m_paletteAffects.aRangeTintPeriods[0]) * CVidPalette::NUM_RANGES);
    memset(m_paletteAffects.aRangeAddPeriods, 0, sizeof(m_paletteAffects.aRangeAddPeriods[0]) * CVidPalette::NUM_RANGES);
    memset(m_paletteAffects.aRangeLightPeriods, 0, sizeof(m_paletteAffects.aRangeLightPeriods[0]) * CVidPalette::NUM_RANGES);
    m_paletteAffects.suppressTints = 0;

    if (m_nTranslucentTableRefs++ == 0) {
        m_pTranslucentTable = new BYTE[0x20000];
        if (m_pTranslucentTable != NULL) {
            // Blend table indexed by [nDest * 0x800 + nSrc * 0x20 + nLevel]
            // for 6-bit components and 32 translucency levels.
            for (INT nLevel = 0; 64 - 2 * nLevel > 0; nLevel++) {
                for (INT nSrc = 0; nSrc < 64; nSrc++) {
                    for (INT nDest = 0; nDest < 64; nDest++) {
                        m_pTranslucentTable[nDest * 0x800 + nSrc * 0x20 + nLevel] = static_cast<BYTE>((nSrc * (64 - 2 * nLevel) + nDest * 2 * nLevel) >> 6);
                    }
                }
            }
        }
    }
}

// 0x7ACC40
CVidImage::~CVidImage()
{
    if (--m_nTranslucentTableRefs == 0) {
        if (m_pTranslucentTable != NULL) {
            delete m_pTranslucentTable;
            m_pTranslucentTable = NULL;
        }
    }
}

// 0x7ACCB0
BOOL CVidImage::InitTranslucentTable()
{
    m_pTranslucentTable = new BYTE[0x20000];
    if (m_pTranslucentTable == NULL) {
        return FALSE;
    }

    // Blend table indexed by [nDest * 0x800 + nSrc * 0x20 + nLevel] for 6-bit
    // components and 32 translucency levels.
    INT nLevel = 0;
    for (INT nWeight = 64; nWeight > 0; nWeight -= 2) {
        for (INT nSrc = 0; nSrc < 64; nSrc++) {
            for (INT nDest = 0; nDest < 64; nDest++) {
                m_pTranslucentTable[nDest * 0x800 + nSrc * 0x20 + nLevel] = static_cast<BYTE>((nSrc * nWeight + nDest * 2 * nLevel) >> 6);
            }
        }
        nLevel++;
    }

    return TRUE;
}

// #binary-identical
// 0x7ACD40
COLORREF CVidImage::GetTintColor()
{
    sizeof(m_cPalette);
    return m_paletteAffects.rgbTintColor;
}

// #binary-identical
// 0x7ACD50
void CVidImage::SetAddColor(COLORREF rgbNewColor)
{
    m_paletteAffects.rgbAddColor = rgbNewColor;
}

// #binary-identical
// 0x7ACD60
void CVidImage::SetTintColor(COLORREF rgbNewColor)
{
    m_paletteAffects.rgbTintColor = rgbNewColor;
}

// 0x7C40D0
void CVidImage::SetPolyColor(DWORD dwFlags)
{
    CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
}

// 0x7B4230
BOOL CVidImage::GetTint(INT& nRed, INT& nGreen, INT& nBlue, INT& nShiftBack, DWORD dwFlags)
{
    INT nMaxValue = 255;
    BOOL bResult = FALSE;
    INT nLightPercentage = 0;
    INT nAddPercentage = 0;

    COLORREF rgbGlobalTintColor = g_pChitin->GetCurrentVideoMode()->GetGlobalTintColor();
    INT nAUCounter = g_pChitin->nAUCounter + reinterpret_cast<INT>(this);

    if ((dwFlags & 0x20000) != 0) {
        if ((dwFlags & 0x10000) != 0) {
            nMaxValue = 255 * 255;
            nRed = GetRValue(m_paletteAffects.rgbTintColor) * GetRValue(rgbGlobalTintColor);
            nGreen = GetGValue(m_paletteAffects.rgbTintColor) * GetGValue(rgbGlobalTintColor);
            nBlue = GetBValue(m_paletteAffects.rgbTintColor) * GetBValue(rgbGlobalTintColor);
            nShiftBack = 16;
        } else {
            nMaxValue = 255;
            nRed = GetRValue(m_paletteAffects.rgbTintColor);
            nGreen = GetGValue(m_paletteAffects.rgbTintColor);
            nBlue = GetBValue(m_paletteAffects.rgbTintColor);
            nShiftBack = 8;
        }
        bResult = TRUE;
    } else if ((dwFlags & 0x10000) != 0) {
        nMaxValue = 255;
        nRed = GetRValue(rgbGlobalTintColor);
        nGreen = GetGValue(rgbGlobalTintColor);
        nBlue = GetBValue(rgbGlobalTintColor);
        nShiftBack = 8;
        bResult = TRUE;
    }

    if (m_paletteAffects.pRangeTints[0] != NULL) {
        if (m_paletteAffects.aRangeTintPeriods[0] == 1) {
            nRed *= GetRValue(*m_paletteAffects.pRangeTints[0]);
            nGreen *= GetGValue(*m_paletteAffects.pRangeTints[0]);
            nBlue *= GetBValue(*m_paletteAffects.pRangeTints[0]);
        } else {
            INT nPeriod = m_paletteAffects.aRangeTintPeriods[0];
            INT nTintPercentage = nAUCounter % (2 * nPeriod);
            if (nPeriod < nTintPercentage) {
                nTintPercentage = 2 * nPeriod - nTintPercentage;
            }

            nRed *= 255 - (255 - GetRValue(*m_paletteAffects.pRangeTints[0])) * nTintPercentage / nPeriod;
            nGreen *= 255 - (255 - GetGValue(*m_paletteAffects.pRangeTints[0])) * nTintPercentage / nPeriod;
            nBlue *= 255 - (255 - GetBValue(*m_paletteAffects.pRangeTints[0])) * nTintPercentage / nPeriod;
        }

        bResult = TRUE;
        nShiftBack += 8;
        nMaxValue *= 255;
    }

    if (m_paletteAffects.pRangeAdds[0] != NULL) {
        INT nPeriod = m_paletteAffects.aRangeAddPeriods[0];
        if (nPeriod == 1 || nPeriod == 0) {
            nAddPercentage = nPeriod;
        } else {
            nAddPercentage = nAUCounter % (2 * nPeriod);
            if (nPeriod < nAddPercentage) {
                nAddPercentage = 2 * nPeriod - nAddPercentage;
            }
        }
    }

    if (m_paletteAffects.pRangeLights[0] != NULL) {
        if (m_paletteAffects.aRangeLightPeriods[0] == 1) {
            nLightPercentage = 1;
        } else {
            INT nPeriod = m_paletteAffects.aRangeLightPeriods[0];
            nLightPercentage = nAUCounter % (2 * nPeriod);
            if (nPeriod < nLightPercentage) {
                nLightPercentage = 2 * nPeriod - nLightPercentage;
            }
        }
    }

    if ((CVidPalette::m_SuppressTintMasks[0] & m_paletteAffects.suppressTints) != 0
        && (nLightPercentage != 0 || nAddPercentage != 0)) {
        if (nLightPercentage > nAddPercentage) {
            nRed += (nMaxValue - nRed) * nLightPercentage / m_paletteAffects.aRangeLightPeriods[0];
            nGreen += (nMaxValue - nGreen) * nLightPercentage / m_paletteAffects.aRangeLightPeriods[0];
            nBlue += (nMaxValue - nBlue) * nLightPercentage / m_paletteAffects.aRangeLightPeriods[0];
        } else {
            nRed += (nMaxValue - nRed) * nAddPercentage / m_paletteAffects.aRangeAddPeriods[0];
            nGreen += (nMaxValue - nGreen) * nAddPercentage / m_paletteAffects.aRangeAddPeriods[0];
            nBlue += (nMaxValue - nBlue) * nAddPercentage / m_paletteAffects.aRangeAddPeriods[0];
        }
    }

    return bResult;
}

// 0x7B4600
BOOL CVidImage::GetAdd(INT& nRed, INT& nGreen, INT& nBlue, INT& nShiftBack, DWORD dwFlags)
{
    BYTE nBrightnessCorrection = g_pChitin->GetCurrentVideoMode()->m_nBrightnessCorrection;
    INT nAUCounter = g_pChitin->nAUCounter;
    BOOL bResult = FALSE;

    if ((dwFlags & 0x40000) != 0) {
        if (nBrightnessCorrection != 0) {
            BYTE nInvBrightness = ~nBrightnessCorrection;
            nRed = static_cast<BYTE>(~GetRValue(m_paletteAffects.rgbAddColor)) * nInvBrightness;
            nGreen = static_cast<BYTE>(~GetGValue(m_paletteAffects.rgbAddColor)) * nInvBrightness;
            nBlue = static_cast<BYTE>(~GetBValue(m_paletteAffects.rgbAddColor)) * nInvBrightness;
            nShiftBack = 16;
        } else {
            nRed = static_cast<BYTE>(~GetRValue(m_paletteAffects.rgbAddColor));
            nGreen = static_cast<BYTE>(~GetGValue(m_paletteAffects.rgbAddColor));
            nBlue = static_cast<BYTE>(~GetBValue(m_paletteAffects.rgbAddColor));
            nShiftBack = 8;
        }
        bResult = TRUE;
    } else if (nBrightnessCorrection != 0) {
        BYTE nInvBrightness = ~nBrightnessCorrection;
        nRed = nInvBrightness;
        nGreen = nInvBrightness;
        nBlue = nInvBrightness;
        nShiftBack = 8;
        bResult = TRUE;
    }

    if (m_paletteAffects.pRangeAdds[0] != NULL) {
        INT nPeriod = m_paletteAffects.aRangeAddPeriods[0];
        if (nPeriod == 1 || nPeriod == 0) {
            nRed *= static_cast<BYTE>(~GetRValue(*m_paletteAffects.pRangeAdds[0]));
            nGreen *= static_cast<BYTE>(~GetGValue(*m_paletteAffects.pRangeAdds[0]));
            nBlue *= static_cast<BYTE>(~GetBValue(*m_paletteAffects.pRangeAdds[0]));
        } else {
            INT nAddPercentage = (nAUCounter + reinterpret_cast<INT>(this)) % (2 * nPeriod);
            if (nPeriod < nAddPercentage) {
                nAddPercentage = 2 * nPeriod - nAddPercentage;
            }

            nRed *= static_cast<BYTE>(~static_cast<BYTE>(GetRValue(*m_paletteAffects.pRangeAdds[0]) * nAddPercentage / nPeriod));
            nGreen *= static_cast<BYTE>(~static_cast<BYTE>(GetGValue(*m_paletteAffects.pRangeAdds[0]) * nAddPercentage / nPeriod));
            nBlue *= static_cast<BYTE>(~static_cast<BYTE>(GetBValue(*m_paletteAffects.pRangeAdds[0]) * nAddPercentage / nPeriod));
        }

        nShiftBack += 8;
        return TRUE;
    }

    return bResult;
}

// 0x7B4800
BOOL CVidImage::GetLight(INT& nRed, INT& nGreen, INT& nBlue, DWORD dwFlags)
{
    BYTE nGammaCorrection = g_pChitin->GetCurrentVideoMode()->m_nGammaCorrection;
    INT nAUCounter = g_pChitin->nAUCounter;
    BOOL bResult = FALSE;

    if ((dwFlags & 0x200000) != 0) {
        if (nGammaCorrection != 0) {
            nRed += GetRValue(m_paletteAffects.rgbLightColor) + nGammaCorrection;
            nGreen += GetGValue(m_paletteAffects.rgbLightColor) + nGammaCorrection;
            nBlue += GetBValue(m_paletteAffects.rgbLightColor) + nGammaCorrection;
        } else {
            nRed += GetRValue(m_paletteAffects.rgbLightColor);
            nGreen += GetGValue(m_paletteAffects.rgbLightColor);
            nBlue += GetBValue(m_paletteAffects.rgbLightColor);
        }
        bResult = TRUE;
    } else if (nGammaCorrection != 0) {
        nRed += nGammaCorrection;
        nGreen += nGammaCorrection;
        nBlue += nGammaCorrection;
        bResult = TRUE;
    }

    if (m_paletteAffects.pRangeLights[0] == NULL) {
        return bResult;
    }

    if (m_paletteAffects.aRangeLightPeriods[0] != 1) {
        INT nPeriod = m_paletteAffects.aRangeLightPeriods[0];
        INT nLightPercentage = (nAUCounter + reinterpret_cast<INT>(this)) % (2 * nPeriod);
        if (nPeriod < nLightPercentage) {
            nLightPercentage = 2 * nPeriod - nLightPercentage;
        }

        nRed += GetRValue(*m_paletteAffects.pRangeLights[0]) * nLightPercentage / nPeriod;
        nGreen += GetGValue(*m_paletteAffects.pRangeLights[0]) * nLightPercentage / nPeriod;
        nBlue += GetBValue(*m_paletteAffects.pRangeLights[0]) * nLightPercentage / nPeriod;
        return TRUE;
    }

    nRed += GetRValue(*m_paletteAffects.pRangeLights[0]);
    nGreen += GetGValue(*m_paletteAffects.pRangeLights[0]);
    nBlue += GetBValue(*m_paletteAffects.pRangeLights[0]);
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x7B16E0
CVidTile::CVidTile()
{
    pRes = NULL;
    m_dwFlags = 0;
}

// 0x7B1700
CVidTile::~CVidTile()
{
}

// 0x7B1770
void CVidTile::SetRes(CResTile* pNewResTiles)
{
    pRes = pNewResTiles;
}

// 0x7B1780
BOOL CVidTile::Render(LPDIRECTDRAWSURFACE pSurface, INT x, INT y, const CRect& rClip, DWORD dwFlags)
{
    LONG nDataJump = 0;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 6140
    UTIL_ASSERT(pSurface != NULL);

    if (pRes == NULL) {
        return FALSE;
    }

    if (rClip.IsRectEmpty()) {
        return FALSE;
    }

    pRes->Demand();

    BYTE* pData = pRes->GetTileData();
    if (pData == NULL) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 6159
        UTIL_ASSERT_MSG(FALSE, "Tile data accessed when not loaded");
        return FALSE;
    }

    INT nTileSize = pRes->GetTileSize();
    CSize blitSize(nTileSize, nTileSize);

    CRect rTileRect(x, y, x + nTileSize, y + nTileSize);

    SHORT nSkipTop = static_cast<SHORT>(rClip.top - rTileRect.top);
    if (nSkipTop > 0) {
        rTileRect.top += nSkipTop;
        pData += nTileSize * nSkipTop;
        blitSize.cy -= nSkipTop;
    }

    SHORT nSkipBottom = static_cast<SHORT>(rTileRect.bottom - rClip.bottom);
    if (nSkipBottom > 0) {
        blitSize.cy -= nSkipBottom;
        rTileRect.bottom -= nSkipBottom;
    }

    SHORT nSkipRight = static_cast<SHORT>(rTileRect.right - rClip.right);
    if (nSkipRight > 0) {
        blitSize.cx -= nSkipRight;
        rTileRect.right -= nSkipRight;
    }

    SHORT nSkipLeft = static_cast<SHORT>(rClip.left - rTileRect.left);
    if (nSkipLeft > 0) {
        blitSize.cx -= nSkipLeft;
        pData += nSkipLeft;
        rTileRect.left += nSkipLeft;
        nDataJump = nSkipLeft;
    }

    if (rTileRect.IsRectEmpty()) {
        pRes->Release();
        return FALSE;
    }

    DDSURFACEDESC ddsd;
    HRESULT hr;

    do {
        ddsd.dwSize = sizeof(ddsd);
        hr = pSurface->Lock(&rTileRect, &ddsd, DDLOCK_WAIT, NULL);
        if (hr == DDERR_SURFACELOST) {
            g_pChitin->GetCurrentVideoMode()->RestoreSurfaces();
        }
    } while (hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY);

    if (hr != DD_OK) {
        pRes->Release();
        return FALSE;
    }

    m_cPalette.SetPalette(pRes->m_pPalette,
        256,
        CVidPalette::TYPE_RESOURCE);

    BOOL bSuccess;

    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        m_cPalette.Realize(rgbTempPal,
            16,
            dwFlags | 0x400000,
            &m_paletteAffects,
            255);
        bSuccess = BltTile8To16(reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch,
            pData,
            blitSize,
            nDataJump,
            dwFlags);
        break;
    case 24:
        m_cPalette.Realize(rgbTempPal,
            24,
            dwFlags | 0x400000,
            &m_paletteAffects,
            255);
        bSuccess = BltTile8To24(reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch,
            pData,
            blitSize,
            nDataJump,
            dwFlags);
        break;
    case 32:
        m_cPalette.Realize(rgbTempPal,
            32,
            dwFlags | 0x400000,
            &m_paletteAffects,
            255);
        bSuccess = BltTile8To32(reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch,
            pData,
            blitSize,
            nDataJump,
            dwFlags);
        break;
    default:
        pSurface->Unlock(ddsd.lpSurface);
        pRes->Release();
        return FALSE;
    }

    pSurface->Unlock(ddsd.lpSurface);

    pRes->Release();
    return bSuccess;
}

// 0x7B1B30
BOOL CVidTile::Render(LPDIRECTDRAWSURFACE pSurface, INT x, INT y, CResTile* pResStencil, const CRect& rClip, DWORD dwFlags)
{
    LONG nDataJump = 0;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 6325
    UTIL_ASSERT(pSurface != NULL);

    if (pRes == NULL) {
        return FALSE;
    }

    if (rClip.IsRectEmpty()) {
        return FALSE;
    }

    pRes->Demand();
    pResStencil->Demand();

    BYTE* pData = pRes->GetTileData();
    if (pData == NULL) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 6344
        UTIL_ASSERT_MSG(FALSE, "Tile data accessed when not loaded");
        return FALSE;
    }

    BYTE* pStencilData = pResStencil->GetTileData();
    if (pStencilData == NULL) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 6351
        UTIL_ASSERT_MSG(FALSE, "Tile stencil data accessed when not loaded");
        pRes->Release();
        pResStencil->Release();
        return FALSE;
    }

    INT nTileSize = pRes->GetTileSize();
    CSize blitSize(nTileSize, nTileSize);

    CRect rTileRect(x, y, x + nTileSize, y + nTileSize);

    SHORT nSkipTop = static_cast<SHORT>(rClip.top - rTileRect.top);
    if (nSkipTop > 0) {
        rTileRect.top += nSkipTop;
        pData += nTileSize * nSkipTop;
        blitSize.cy -= nSkipTop;
    }

    SHORT nSkipBottom = static_cast<SHORT>(rTileRect.bottom - rClip.bottom);
    if (nSkipBottom > 0) {
        blitSize.cy -= nSkipBottom;
        rTileRect.bottom -= nSkipBottom;
    }

    SHORT nSkipRight = static_cast<SHORT>(rTileRect.right - rClip.right);
    if (nSkipRight > 0) {
        blitSize.cx -= nSkipRight;
        rTileRect.right -= nSkipRight;
    }

    SHORT nSkipLeft = static_cast<SHORT>(rClip.left - rTileRect.left);
    if (nSkipLeft > 0) {
        blitSize.cx -= nSkipLeft;
        pData += nSkipLeft;
        rTileRect.left += nSkipLeft;
        nDataJump = nSkipLeft;
    }

    if (rTileRect.IsRectEmpty()) {
        pRes->Release();
        pResStencil->Release();
        return FALSE;
    }

    DDSURFACEDESC ddsd;
    HRESULT hr;

    do {
        ddsd.dwSize = sizeof(ddsd);
        hr = pSurface->Lock(&rTileRect, &ddsd, DDLOCK_WAIT, NULL);
        if (hr == DDERR_SURFACELOST) {
            g_pChitin->GetCurrentVideoMode()->RestoreSurfaces();
        }
    } while (hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY);

    if (hr != DD_OK) {
        pRes->Release();
        pResStencil->Release();
        return FALSE;
    }

    m_cPalette.SetPalette(pRes->m_pPalette,
        256,
        CVidPalette::TYPE_RESOURCE);

    BOOL bSuccess;

    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        m_cPalette.Realize(rgbTempPal,
            16,
            dwFlags | 0x400000,
            &m_paletteAffects,
            255);
        bSuccess = BltStencilTile8To16(reinterpret_cast<WORD*>(ddsd.lpSurface),
            ddsd.lPitch,
            pData,
            pStencilData,
            blitSize,
            nDataJump,
            dwFlags);
        break;
    case 24:
        m_cPalette.Realize(rgbTempPal,
            24,
            dwFlags | 0x400000,
            &m_paletteAffects,
            255);
        bSuccess = BltStencilTile8To24(reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch,
            pData,
            pStencilData,
            blitSize,
            nDataJump,
            dwFlags);
        break;
    case 32:
        m_cPalette.Realize(rgbTempPal,
            32,
            dwFlags | 0x400000,
            &m_paletteAffects,
            255);
        bSuccess = BltStencilTile8To32(reinterpret_cast<DWORD*>(ddsd.lpSurface),
            ddsd.lPitch,
            pData,
            pStencilData,
            blitSize,
            nDataJump,
            dwFlags);
        break;
    default:
        pSurface->Unlock(ddsd.lpSurface);
        // NOTE: The stencil is not released on this path.
        pRes->Release();
        return FALSE;
    }

    pSurface->Unlock(ddsd.lpSurface);

    pRes->Release();
    pResStencil->Release();
    return bSuccess;
}

// 0x7B1F70
BOOL CVidTile::RenderBlended(LPDIRECTDRAWSURFACE pSurface, INT x, INT y, const CRect& rClip, CResTile* pResBlend, BYTE nBlend, DWORD dwFlags)
{
    LONG nDataJump = 0;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 6526
    UTIL_ASSERT(pSurface != NULL);

    if (pRes == NULL) {
        return FALSE;
    }

    if (rClip.IsRectEmpty()) {
        return FALSE;
    }

    pRes->Demand();
    pResBlend->Demand();

    BYTE* pData = pRes->GetTileData();
    BYTE* pBlendData = pResBlend->GetTileData();

    INT nTileSize = pRes->GetTileSize();
    CSize blitSize(nTileSize, nTileSize);

    CRect rTileRect(x, y, x + nTileSize, y + nTileSize);

    // NOTE: The clipping amounts are 16-bit.
    SHORT nSkipTop = static_cast<SHORT>(rClip.top - rTileRect.top);
    if (nSkipTop > 0) {
        rTileRect.top += nSkipTop;
        pData += nSkipTop * nTileSize;
        pBlendData += nSkipTop * nTileSize;
        blitSize.cy -= nSkipTop;
    }

    SHORT nSkipBottom = static_cast<SHORT>(rTileRect.bottom - rClip.bottom);
    if (nSkipBottom > 0) {
        blitSize.cy -= nSkipBottom;
        rTileRect.bottom -= nSkipBottom;
    }

    SHORT nSkipRight = static_cast<SHORT>(rTileRect.right - rClip.right);
    if (nSkipRight > 0) {
        blitSize.cx -= nSkipRight;
        rTileRect.right -= nSkipRight;
    }

    SHORT nSkipLeft = static_cast<SHORT>(rClip.left - rTileRect.left);
    if (nSkipLeft > 0) {
        blitSize.cx -= nSkipLeft;
        pData += nSkipLeft;
        pBlendData += nSkipLeft;
        rTileRect.left += nSkipLeft;
        nDataJump = nSkipLeft;
    }

    BOOL bResult;

    if (!rTileRect.IsRectEmpty()) {
        DDSURFACEDESC ddsd;
        HRESULT hr;

        do {
            ddsd.dwSize = sizeof(ddsd);
            hr = pSurface->Lock(&rTileRect, &ddsd, DDLOCK_WAIT, NULL);
            if (hr == DDERR_SURFACELOST) {
                g_pChitin->GetCurrentVideoMode()->RestoreSurfaces();
            }
        } while (hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY);

        if (hr != DD_OK) {
            // NOTE: `pResBlend` is not released.
            pRes->Release();
            return FALSE;
        }

        switch (g_pChitin->cVideo.m_nBpp) {
        case 16:
            bResult = BltBlendTile8To16(reinterpret_cast<WORD*>(ddsd.lpSurface),
                ddsd.lPitch,
                pData,
                pBlendData,
                pResBlend,
                nBlend,
                blitSize,
                nDataJump,
                dwFlags);
            break;
        case 24:
            bResult = BltBlendTile8To24(reinterpret_cast<BYTE*>(ddsd.lpSurface),
                ddsd.lPitch,
                pData,
                pBlendData,
                pResBlend,
                nBlend,
                blitSize,
                nDataJump,
                dwFlags);
            break;
        case 32:
            bResult = BltBlendTile8To32(reinterpret_cast<DWORD*>(ddsd.lpSurface),
                ddsd.lPitch,
                pData,
                pBlendData,
                pResBlend,
                nBlend,
                blitSize,
                nDataJump,
                dwFlags);
            break;
        default:
            pSurface->Unlock(ddsd.lpSurface);

            // NOTE: `pResBlend` is not released.
            pRes->Release();
            return FALSE;
        }

        pSurface->Unlock(ddsd.lpSurface);
    } else {
        bResult = FALSE;
    }

    pRes->Release();
    pResBlend->Release();

    return bResult;
}

// 0x7B22E0
BOOL CVidTile::BltTile8To16(WORD* pSurface, LONG lPitch, BYTE* pData, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    INT nDataIndex = 0;
    DWORD dwHighColor = g_pChitin->GetCurrentVideoMode()->field_24 << 16;
    DWORD dwLowColor = g_pChitin->GetCurrentVideoMode()->field_24;
    LONG nSurfaceJump = (lPitch >> 1) - blitSize.cx;
    LONG nPairs = blitSize.cx >> 1;

    // NOTE: Pixels are written in pairs.
    DWORD* pDest = reinterpret_cast<DWORD*>(pSurface);

    if ((dwFlags & 0x8) != 0) {
        for (LONG y = 0; y < blitSize.cy; y++) {
            if ((y & 1) == 0) {
                for (LONG x = 0; x < nPairs; x++) {
                    *pDest++ = (rgbTempPal[pData[nDataIndex]] & 0xFFFF) | dwHighColor;
                    nDataIndex += 2;
                }
            } else {
                for (LONG x = 0; x < nPairs; x++) {
                    *pDest++ = (rgbTempPal[pData[nDataIndex + 1]] << 16) | dwLowColor;
                    nDataIndex += 2;
                }
            }

            pDest = reinterpret_cast<DWORD*>(reinterpret_cast<WORD*>(pDest) + nSurfaceJump);
            nDataIndex += nDataJump;
        }
    } else {
        for (LONG y = 0; y < blitSize.cy; y++) {
            for (LONG x = 0; x < nPairs; x++) {
                *pDest++ = (rgbTempPal[pData[nDataIndex + 1]] << 16) | (rgbTempPal[pData[nDataIndex]] & 0xFFFF);
                nDataIndex += 2;
            }

            nDataIndex += nDataJump;
            pDest = reinterpret_cast<DWORD*>(reinterpret_cast<WORD*>(pDest) + nSurfaceJump);
        }
    }

    return TRUE;
}

// 0x7B2440
BOOL CVidTile::BltStencilTile8To16(WORD* pSurface, LONG lPitch, BYTE* pData, BYTE* pStencilData, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    INT nDataIndex = 0;
    DWORD dwHighColor = g_pChitin->GetCurrentVideoMode()->field_24 << 16;
    DWORD dwLowColor = g_pChitin->GetCurrentVideoMode()->field_24;

    if ((blitSize.cx & 1) != 0) {
        return FALSE;
    }

    LONG nSurfaceJump = (lPitch >> 1) - blitSize.cx;
    LONG nPairs = blitSize.cx >> 1;

    // NOTE: Pixels are written in pairs.
    DWORD* pDest = reinterpret_cast<DWORD*>(pSurface);

    for (LONG y = 0; y < blitSize.cy; y++) {
        if ((y & 1) == 0) {
            for (LONG x = 0; x < nPairs; x++) {
                if (pStencilData[nDataIndex + 1] == CVidPalette::CLEAR_ENTRY) {
                    *pDest = (rgbTempPal[pData[nDataIndex]] & 0xFFFF) | dwHighColor;
                } else {
                    *pDest = (rgbTempPal[pData[nDataIndex + 1]] << 16) | (rgbTempPal[pData[nDataIndex]] & 0xFFFF);
                }
                pDest++;
                nDataIndex += 2;
            }
        } else {
            for (LONG x = 0; x < nPairs; x++) {
                if (pStencilData[nDataIndex] == CVidPalette::CLEAR_ENTRY) {
                    *pDest = (rgbTempPal[pData[nDataIndex + 1]] << 16) | dwLowColor;
                } else {
                    *pDest = (rgbTempPal[pData[nDataIndex + 1]] << 16) | (rgbTempPal[pData[nDataIndex]] & 0xFFFF);
                }
                pDest++;
                nDataIndex += 2;
            }
        }

        nDataIndex += nDataJump;
        pDest = reinterpret_cast<DWORD*>(reinterpret_cast<WORD*>(pDest) + nSurfaceJump);
    }

    return TRUE;
}

// 0x7B25E0
BOOL CVidTile::BltBlendTile8To16(WORD* pSurface, LONG lPitch, BYTE* pData, BYTE* pBlendData, CResTile* pResBlend, BYTE nBlend, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    LONG cx = blitSize.cx;
    LONG cy = blitSize.cy;
    INT nDataIndex = 0;
    CVidPalette cBlendPalette(CVidPalette::TYPE_RESOURCE);

    DWORD dwRBitMask = g_pChitin->GetCurrentVideoMode()->m_dwRBitMask;
    DWORD dwGBitMask = g_pChitin->GetCurrentVideoMode()->m_dwGBitMask;
    DWORD dwBBitMask = g_pChitin->GetCurrentVideoMode()->m_dwBBitMask;
    BYTE nRBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwRBitShift);
    BYTE nGBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwGBitShift);
    BYTE nBBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwBBitShift);

    // NOTE: Uninline.
    if (m_pTranslucentTable == NULL && !InitTranslucentTable()) {
        return FALSE;
    }

    // Reduce 8-bit `nBlend` to the 5-bit level of the table.
    int nShift = 8;
    do {
        nShift >>= 1;
        nBlend >>= 1;
    } while (nShift > 1);

    m_cPalette.SetPalette(pRes->m_pPalette,
        256,
        CVidPalette::TYPE_RESOURCE);
    cBlendPalette.SetPalette(pResBlend->m_pPalette,
        256,
        CVidPalette::TYPE_RESOURCE);

    m_cPalette.Realize(rgbTempPal,
        16,
        dwFlags | 0x400000,
        &m_paletteAffects,
        255);
    cBlendPalette.Realize(rgbBlendPal,
        16,
        dwFlags | 0x400000,
        &m_paletteAffects,
        255);

    LONG nSurfaceJump = (lPitch >> 1) - cx;

    for (LONG y = 0; y < cy; y++) {
        for (LONG x = 0; x < cx; x++) {
            WORD nColor = static_cast<WORD>(rgbTempPal[pData[nDataIndex]]);
            WORD nBlendColor = static_cast<WORD>(rgbBlendPal[pBlendData[nDataIndex]]);

            WORD r = m_pTranslucentTable[(((((((dwRBitMask & 0xFFFF) & nColor) >> nRBitShift) << 6) + (((dwRBitMask & 0xFFFF) & nBlendColor) >> nRBitShift)) << 5) + nBlend)];
            WORD g = m_pTranslucentTable[(((((((dwGBitMask & 0xFFFF) & nColor) >> nGBitShift) << 6) + (((dwGBitMask & 0xFFFF) & nBlendColor) >> nGBitShift)) << 5) + nBlend)];
            WORD b = m_pTranslucentTable[(((((((dwBBitMask & 0xFFFF) & nColor) >> nBBitShift) << 6) + (((dwBBitMask & 0xFFFF) & nBlendColor) >> nBBitShift)) << 5) + nBlend)];

            *pSurface++ = static_cast<WORD>((b << nBBitShift) | (g << nGBitShift) | (r << nRBitShift));
            nDataIndex++;
        }

        pSurface += nSurfaceJump;
        nDataIndex += nDataJump;
    }

    return TRUE;
}

// 0x7CDD00
BOOL CVidTile::BltTile8To24(DWORD* pSurface, LONG lPitch, BYTE* pData, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    INT nDataIndex = 0;
    DWORD color = g_pChitin->GetCurrentVideoMode()->field_24 & 0xFFFFFF;

    if ((blitSize.cx & 3) != 0) {
        return FALSE;
    }

    // NOTE: Four 24-bit pixels are written as three DWORDs.
    LONG nSurfaceJump = (lPitch - 3 * blitSize.cx) / 4;
    LONG nQuads = blitSize.cx >> 2;

    if ((dwFlags & 0x8) != 0) {
        for (LONG y = 0; y < blitSize.cy; y++) {
            if ((y & 1) == 0) {
                for (LONG x = 0; x < nQuads; x++) {
                    pSurface[0] = (rgbTempPal[pData[nDataIndex]] & 0xFFFFFF) | (color << 24);
                    pSurface[1] = (rgbTempPal[pData[nDataIndex + 2]] << 16) | ((color >> 8) & 0xFFFF);
                    pSurface[2] = ((rgbTempPal[pData[nDataIndex + 2]] >> 16) & 0xFF) | (color << 8);
                    pSurface += 3;
                    nDataIndex += 4;
                }
            } else {
                for (LONG x = 0; x < nQuads; x++) {
                    pSurface[0] = (rgbTempPal[pData[nDataIndex + 1]] << 24) | color;
                    pSurface[1] = ((rgbTempPal[pData[nDataIndex + 1]] >> 8) & 0xFFFF) | (color << 16);
                    pSurface[2] = (rgbTempPal[pData[nDataIndex + 3]] << 8) | ((color >> 16) & 0xFF);
                    pSurface += 3;
                    nDataIndex += 4;
                }
            }

            // FIXME: The original never applies `nSurfaceJump` and `nDataJump`
            // between dithered rows.
        }
    } else {
        for (LONG y = 0; y < blitSize.cy; y++) {
            for (LONG x = 0; x < nQuads; x++) {
                pSurface[0] = (rgbTempPal[pData[nDataIndex]] & 0xFFFFFF) | (rgbTempPal[pData[nDataIndex + 1]] << 24);
                pSurface[1] = ((rgbTempPal[pData[nDataIndex + 1]] >> 8) & 0xFFFF) | (rgbTempPal[pData[nDataIndex + 2]] << 16);
                pSurface[2] = ((rgbTempPal[pData[nDataIndex + 2]] >> 16) & 0xFF) | (rgbTempPal[pData[nDataIndex + 3]] << 8);
                pSurface += 3;
                nDataIndex += 4;
            }

            nDataIndex += nDataJump;
            pSurface += nSurfaceJump;
        }
    }

    return TRUE;
}

// 0x7CDF90
BOOL CVidTile::BltStencilTile8To24(DWORD* pSurface, LONG lPitch, BYTE* pData, BYTE* pStencilData, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    INT nDataIndex = 0;
    DWORD color = g_pChitin->GetCurrentVideoMode()->field_24 & 0xFFFFFF;

    if ((blitSize.cx & 3) != 0) {
        return FALSE;
    }

    // NOTE: Four 24-bit pixels are written as three DWORDs.
    LONG nSurfaceJump = (lPitch - 3 * blitSize.cx) / 4;
    LONG nQuads = blitSize.cx >> 2;

    for (LONG y = 0; y < blitSize.cy; y++) {
        if ((y & 1) == 0) {
            for (LONG x = 0; x < nQuads; x++) {
                if (pStencilData[nDataIndex + 1] == CVidPalette::CLEAR_ENTRY) {
                    pSurface[0] = (rgbTempPal[pData[nDataIndex]] & 0xFFFFFF) | (color << 24);
                    pSurface[1] = (rgbTempPal[pData[nDataIndex + 2]] << 16) | ((color >> 8) & 0xFFFF);
                } else {
                    pSurface[0] = (rgbTempPal[pData[nDataIndex]] & 0xFFFFFF) | (rgbTempPal[pData[nDataIndex + 1]] << 24);
                    pSurface[1] = ((rgbTempPal[pData[nDataIndex + 1]] >> 8) & 0xFFFF) | (rgbTempPal[pData[nDataIndex + 2]] << 16);
                }

                if (pStencilData[nDataIndex + 3] == CVidPalette::CLEAR_ENTRY) {
                    pSurface[2] = ((rgbTempPal[pData[nDataIndex + 2]] >> 16) & 0xFF) | (color << 8);
                } else {
                    pSurface[2] = (rgbTempPal[pData[nDataIndex + 3]] << 8) | ((rgbTempPal[pData[nDataIndex + 2]] >> 16) & 0xFF);
                }

                pSurface += 3;
                nDataIndex += 4;
            }
        } else {
            for (LONG x = 0; x < nQuads; x++) {
                if (pStencilData[nDataIndex] == CVidPalette::CLEAR_ENTRY) {
                    pSurface[0] = (rgbTempPal[pData[nDataIndex + 1]] << 24) | color;
                } else {
                    pSurface[0] = (rgbTempPal[pData[nDataIndex]] & 0xFFFFFF) | (rgbTempPal[pData[nDataIndex + 1]] << 24);
                }

                if (pStencilData[nDataIndex + 2] == CVidPalette::CLEAR_ENTRY) {
                    pSurface[1] = ((rgbTempPal[pData[nDataIndex + 1]] >> 8) & 0xFFFF) | (color << 16);
                    pSurface[2] = (rgbTempPal[pData[nDataIndex + 3]] << 8) | ((color >> 16) & 0xFF);
                } else {
                    pSurface[1] = ((rgbTempPal[pData[nDataIndex + 1]] >> 8) & 0xFFFF) | (rgbTempPal[pData[nDataIndex + 2]] << 16);
                    pSurface[2] = (rgbTempPal[pData[nDataIndex + 3]] << 8) | ((rgbTempPal[pData[nDataIndex + 2]] >> 16) & 0xFF);
                }

                pSurface += 3;
                nDataIndex += 4;
            }
        }

        nDataIndex += nDataJump;
        pSurface += nSurfaceJump;
    }

    return TRUE;
}

// 0x7CE830
BOOL CVidTile::BltTile8To32(DWORD* pSurface, LONG lPitch, BYTE* pData, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    INT nSurfaceJump = lPitch - 4 * blitSize.cx;
    DWORD color = g_pChitin->GetCurrentVideoMode()->field_24;

    if ((blitSize.cx & 1) != 0) {
        return FALSE;
    }

    if ((dwFlags & 0x8) != 0) {
        for (int y = 0; y < blitSize.cy; y++) {
            for (int x = 0; x < blitSize.cx; x++) {
                pSurface[0] = rgbTempPal[*pData];

                x++;
                if (x == blitSize.cx) {
                    break;
                }

                pSurface[1] = color;

                pData += 2;
                pSurface += 2;
            }

            pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pSurface) + nSurfaceJump);
            pData += nDataJump;

            y++;
            if (y == blitSize.cy) {
                break;
            }

            for (int x = 0; x < blitSize.cx; x++) {
                pSurface[0] = color;

                x++;
                if (x == blitSize.cx) {
                    break;
                }

                pSurface[1] = rgbTempPal[pData[1]];

                pData += 2;
                pSurface += 2;
            }

            pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pSurface) + nSurfaceJump);
            pData += nDataJump;
        }
    } else {
        for (int y = 0; y < blitSize.cy; y++) {
            for (int x = 0; x < blitSize.cx; x++) {
                *pSurface++ = rgbTempPal[*pData++];
            }

            pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pSurface) + nSurfaceJump);
            pData += nDataJump;
        }
    }

    return TRUE;
}

// 0x7CE290
BOOL CVidTile::BltStencilTile8To32(DWORD* pSurface, LONG lPitch, BYTE* pData, BYTE* pStencilData, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    INT nDataIndex = 0;
    INT nSurfaceJump = lPitch - 4 * blitSize.cx;
    DWORD color = g_pChitin->GetCurrentVideoMode()->field_24;

    if ((blitSize.cx & 1) != 0) {
        return FALSE;
    }

    for (int y = 0; y < blitSize.cy; y++) {
        if ((y & 1) != 0) {
            for (int x = 0; x < blitSize.cx / 2; x++) {
                if (pStencilData[nDataIndex] == CVidPalette::CLEAR_ENTRY) {
                    pSurface[0] = color;
                } else {
                    pSurface[0] = rgbTempPal[pData[nDataIndex]];
                }

                pSurface[1] = rgbTempPal[pData[nDataIndex + 1]];

                pSurface += 2;
                nDataIndex += 2;
            }
        } else {
            for (int x = 0; x < blitSize.cx / 2; x++) {
                pSurface[0] = rgbTempPal[pData[nDataIndex]];

                if (pStencilData[nDataIndex + 1] == CVidPalette::CLEAR_ENTRY) {
                    pSurface[1] = color;
                } else {
                    pSurface[1] = rgbTempPal[pData[nDataIndex + 1]];
                }

                pSurface += 2;
                nDataIndex += 2;
            }
        }

        pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pSurface) + nSurfaceJump);
        nDataIndex += nDataJump;
    }

    return TRUE;
}

// 0x7CE3D0
BOOL CVidTile::BltBlendTile8To24(BYTE* pSurface, LONG lPitch, BYTE* pData, BYTE* pBlendData, CResTile* pResBlend, BYTE nBlend, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    LONG cx = blitSize.cx;
    LONG cy = blitSize.cy;
    INT nDataIndex = 0;
    CVidPalette cBlendPalette(CVidPalette::TYPE_RESOURCE);

    DWORD dwRBitMask = g_pChitin->GetCurrentVideoMode()->m_dwRBitMask;
    DWORD dwGBitMask = g_pChitin->GetCurrentVideoMode()->m_dwGBitMask;
    DWORD dwBBitMask = g_pChitin->GetCurrentVideoMode()->m_dwBBitMask;
    BYTE nRBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwRBitShift);
    BYTE nGBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwGBitShift);
    BYTE nBBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwBBitShift);

    if (m_pTranslucentTable == NULL && !InitTranslucentTable()) {
        return FALSE;
    }

    // Reduce 8-bit `nBlend` to the 5-bit level of the table.
    int nShift = 8;
    do {
        nShift >>= 1;
        nBlend >>= 1;
    } while (nShift > 1);

    m_cPalette.SetPalette(pRes->m_pPalette,
        256,
        CVidPalette::TYPE_RESOURCE);
    cBlendPalette.SetPalette(pResBlend->m_pPalette,
        256,
        CVidPalette::TYPE_RESOURCE);

    m_cPalette.Realize(rgbTempPal,
        24,
        dwFlags | 0x400000,
        &m_paletteAffects,
        255);
    cBlendPalette.Realize(rgbBlendPal,
        24,
        dwFlags | 0x400000,
        &m_paletteAffects,
        255);

    LONG nSurfaceJump = lPitch - cx * 3;

    DWORD nColor;
    DWORD nBlendColor;
    DWORD r;
    DWORD g;
    DWORD b;

    for (LONG y = 0; y < cy; y++) {
        // NOTE: All pixels but the last are written as DWORDs.
        for (LONG x = 0; x < cx - 1; x++) {
            nColor = rgbTempPal[pData[nDataIndex]];
            nBlendColor = rgbBlendPal[pBlendData[nDataIndex]];

            r = m_pTranslucentTable[(((((nColor & dwRBitMask) >> nRBitShift) << 6) + ((nBlendColor & dwRBitMask) >> nRBitShift)) << 5) + nBlend];
            g = m_pTranslucentTable[(((((nColor & dwGBitMask) >> nGBitShift) << 6) + ((nBlendColor & dwGBitMask) >> nGBitShift)) << 5) + nBlend];
            b = m_pTranslucentTable[(((((nColor & dwBBitMask) >> nBBitShift) << 6) + ((nBlendColor & dwBBitMask) >> nBBitShift)) << 5) + nBlend];

            *reinterpret_cast<DWORD*>(pSurface) = (r << nRBitShift) | (g << nGBitShift) | (b << nBBitShift);
            pSurface += 3;
            nDataIndex++;
        }

        nColor = rgbTempPal[pData[nDataIndex]];
        nBlendColor = rgbBlendPal[pBlendData[nDataIndex]];

        r = m_pTranslucentTable[(((((nColor & dwRBitMask) >> nRBitShift) << 6) + ((nBlendColor & dwRBitMask) >> nRBitShift)) << 5) + nBlend];
        g = m_pTranslucentTable[(((((nColor & dwGBitMask) >> nGBitShift) << 6) + ((nBlendColor & dwGBitMask) >> nGBitShift)) << 5) + nBlend];
        b = m_pTranslucentTable[(((((nColor & dwBBitMask) >> nBBitShift) << 6) + ((nBlendColor & dwBBitMask) >> nBBitShift)) << 5) + nBlend];

        DWORD dwPixel = (b << nBBitShift) | (r << nRBitShift) | (g << nGBitShift);
        *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(dwPixel);
        pSurface[2] = static_cast<BYTE>(dwPixel >> 16);

        // NOTE: The last pixel of the row does not advance `nDataIndex`.
        pSurface += nSurfaceJump + 3;
        nDataIndex += nDataJump;
    }

    return TRUE;
}

// 0x7CE930
BOOL CVidTile::BltBlendTile8To32(DWORD* pSurface, LONG lPitch, BYTE* pData, BYTE* pBlendData, CResTile* pResBlend, BYTE nBlend, const CSize& blitSize, LONG nDataJump, DWORD dwFlags)
{
    LONG cx = blitSize.cx;
    LONG cy = blitSize.cy;
    INT nDataIndex = 0;
    CVidPalette cBlendPalette(CVidPalette::TYPE_RESOURCE);

    DWORD dwRBitMask = g_pChitin->GetCurrentVideoMode()->m_dwRBitMask;
    DWORD dwGBitMask = g_pChitin->GetCurrentVideoMode()->m_dwGBitMask;
    DWORD dwBBitMask = g_pChitin->GetCurrentVideoMode()->m_dwBBitMask;
    BYTE nRBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwRBitShift);
    BYTE nGBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwGBitShift);
    BYTE nBBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwBBitShift);

    if (m_pTranslucentTable == NULL && !InitTranslucentTable()) {
        return FALSE;
    }

    // Reduce 8-bit `nBlend` to the 5-bit level of the table.
    int nShift = 8;
    do {
        nShift >>= 1;
        nBlend >>= 1;
    } while (nShift > 1);

    m_cPalette.SetPalette(pRes->m_pPalette,
        256,
        CVidPalette::TYPE_RESOURCE);
    cBlendPalette.SetPalette(pResBlend->m_pPalette,
        256,
        CVidPalette::TYPE_RESOURCE);

    m_cPalette.Realize(rgbTempPal,
        32,
        dwFlags | 0x400000,
        &m_paletteAffects,
        255);
    cBlendPalette.Realize(rgbBlendPal,
        32,
        dwFlags | 0x400000,
        &m_paletteAffects,
        255);

    LONG nSurfaceJump = (lPitch >> 2) - cx;

    for (LONG y = 0; y < cy; y++) {
        for (LONG x = 0; x < cx; x++) {
            DWORD nColor = rgbTempPal[pData[nDataIndex]];
            DWORD nBlendColor = rgbBlendPal[pBlendData[nDataIndex]];

            DWORD r = m_pTranslucentTable[(((((nColor & dwRBitMask) >> nRBitShift) << 6) + ((nBlendColor & dwRBitMask) >> nRBitShift)) << 5) + nBlend];
            DWORD g = m_pTranslucentTable[(((((nColor & dwGBitMask) >> nGBitShift) << 6) + ((nBlendColor & dwGBitMask) >> nGBitShift)) << 5) + nBlend];
            DWORD b = m_pTranslucentTable[(((((nColor & dwBBitMask) >> nBBitShift) << 6) + ((nBlendColor & dwBBitMask) >> nBBitShift)) << 5) + nBlend];

            *pSurface++ = (b << nBBitShift) | (g << nGBitShift) | (r << nRBitShift);
            nDataIndex++;
        }

        nDataIndex += nDataJump;
        pSurface += nSurfaceJump;
    }

    return TRUE;
}

// NOTE: Inlined.
BOOL CVidTile::BltStencilTile8to32_3d(DWORD* pSurface, LONG lPitch, BYTE* pData, BYTE* pStencilData, const CSize& blitSize, LONG nDataJump, DWORD dwAlpha, DWORD dwFlags)
{
    // NOTE: The row jump is counted in pixels (`lPitch / 4 - cx`) but added
    // to the surface pointer in bytes (0x7C6391). The tiles are 64 pixels
    // wide, so the jump is 0 in practice.
    INT nSurfaceJump = lPitch / 4 - blitSize.cx;

    for (int y = 0; y < blitSize.cy; y++) {
        for (int x = 0; x < blitSize.cx; x++) {
            DWORD dwPixel = rgbTempPal[*pData++];
            if (*pStencilData == CVidPalette::CLEAR_ENTRY) {
                dwPixel &= (dwAlpha << 24) | 0xFFFFFF;
            }
            *pSurface++ = dwPixel;
            pStencilData++;
        }

        pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pSurface) + nSurfaceJump);
        // NOTE: Only the tile data skips nDataJump, the stencil data does not.
        pData += nDataJump;
    }

    return TRUE;
}

// 0x7C5F60
BOOL CVidTile::ReadyTexture(INT nTextureId, DWORD dwFlags, DWORD dwAlpha)
{
    if (nTextureId < 9 && nTextureId != 2) {
        return FALSE;
    }

    if (pRes == NULL) {
        return FALSE;
    }

    pRes->Demand();

    BYTE* pTileData = pRes->GetTileData();
    if (pTileData == NULL) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
        // __LINE__: 1465
        UTIL_ASSERT_MSG(FALSE, "Tile data accessed when not loaded");
    }

    m_dwFlags = dwFlags;

    GLuint textures[1];
    textures[0] = static_cast<GLuint>(nTextureId);

    CVideo3d::glDeleteTextures(1, textures);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    INT nTileSize = pRes->GetTileSize();
    CSize blitSize(nTileSize, nTileSize);

    m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    m_cPalette.Realize(CVidImage::rgbTempPal, 32, dwFlags | 0x400000, &m_paletteAffects, dwAlpha);

    BltTile8To32(reinterpret_cast<DWORD*>(m_pPixels), 256, pTileData, blitSize, 0, dwFlags);

    g_pChitin->cVideo.field_13E = nTextureId;
    CVideo3d::glBindTexture(GL_TEXTURE_2D, nTextureId);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexImage2D(GL_TEXTURE_2D,
        0,
        g_pChitin->cVideo.field_136,
        nTileSize,
        nTileSize,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        m_pPixels);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    pRes->Release();

    return TRUE;
}

// 0x7C61D0
BOOL CVidTile::ReadyTexture(INT nTextureId, CResTile* pResStencil, DWORD dwFlags, DWORD dwAlpha)
{
    if (nTextureId < 9 && nTextureId != 2) {
        return FALSE;
    }

    if (pRes == NULL) {
        return FALSE;
    }

    pRes->Demand();
    pResStencil->Demand();

    BYTE* pTileData = pRes->GetTileData();
    if (pTileData == NULL) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
        // __LINE__: 1564
        UTIL_ASSERT_MSG(FALSE, "Tile data accessed when not loaded");
        // NOTE: The stencil is not released on this path.
        pRes->Release();
        return FALSE;
    }

    BYTE* pStencilData = pResStencil->GetTileData();
    if (pStencilData == NULL) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
        // __LINE__: 1573
        UTIL_ASSERT_MSG(FALSE, "Tile stencil data accessed when not loaded");
        pRes->Release();
        pResStencil->Release();
        return FALSE;
    }

    m_dwFlags = dwFlags;

    GLuint textures[1];
    textures[0] = static_cast<GLuint>(nTextureId);

    CVideo3d::glDeleteTextures(1, textures);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    INT nTileSize = pRes->GetTileSize();
    CSize blitSize(nTileSize, nTileSize);

    m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    m_cPalette.Realize(CVidImage::rgbTempPal, 32, dwFlags | 0x400000, &m_paletteAffects, dwAlpha);

    // NOTE: Uninline.
    BltStencilTile8to32_3d(reinterpret_cast<DWORD*>(m_pPixels),
        256,
        pTileData,
        pStencilData,
        blitSize,
        0,
        dwAlpha,
        dwFlags);

    g_pChitin->cVideo.field_13E = nTextureId;
    CVideo3d::glBindTexture(GL_TEXTURE_2D, nTextureId);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexImage2D(GL_TEXTURE_2D,
        0,
        g_pChitin->cVideo.field_136,
        nTileSize,
        nTileSize,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        m_pPixels);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    pRes->Release();
    pResStencil->Release();

    return TRUE;
}

// 0x7C64F0
void CVidTile::RenderTexture(INT nTextureId, const CRect& rDest, INT x, INT y, DWORD dwFlags)
{
    float x1;
    float y1;
    float x2;
    float y2;

    if (nTextureId < 9 && nTextureId != 2) {
        return;
    }

    x1 = static_cast<float>(x);
    y1 = static_cast<float>(y);
    x2 = static_cast<float>(x) + 64.0f;
    y2 = static_cast<float>(y) + 64.0f;

    CVideo3d::glEnable(GL_TEXTURE_2D);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    g_pChitin->cVideo.field_13E = nTextureId;
    CVideo3d::glBindTexture(GL_TEXTURE_2D, nTextureId);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x3) != 0) {
        CVideo3d::glEnable(GL_BLEND);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }

    CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    x1 += CVideo3d::SUB_PIXEL_SHIFT;
    y1 += CVideo3d::SUB_PIXEL_SHIFT;
    x2 += CVideo3d::SUB_PIXEL_SHIFT;
    y2 += CVideo3d::SUB_PIXEL_SHIFT;

    CVideo3d::glBegin(GL_TRIANGLE_STRIP);
    CVideo3d::glTexCoord2f(0.0f, 0.0f);
    CVideo3d::glVertex3f(x1, y1, 0.0f);
    CVideo3d::glTexCoord2f(0.0f, 1.0f);
    CVideo3d::glVertex3f(x1, y2, 0.0f);
    CVideo3d::glTexCoord2f(1.0f, 0.0f);
    CVideo3d::glVertex3f(x2, y1, 0.0f);
    CVideo3d::glTexCoord2f(1.0f, 1.0f);
    CVideo3d::glVertex3f(x2, y2, 0.0f);
    CVideo3d::glEnd();
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x3) != 0) {
        CVideo3d::glDisable(GL_BLEND);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }
}
