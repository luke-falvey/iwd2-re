#include "CVidMosaic.h"

#include <math.h>

#include "CUtil.h"
#include "CVidInf.h"
#include "CVideo3d.h"

// 0x7B0AA0
CVidMosaic::CVidMosaic()
{
    pRes = NULL;
    m_bDoubleSize = FALSE;
}

// 0x7B0B20
CVidMosaic::~CVidMosaic()
{
    // NOTE: The asm is the inlined `CResHelper` and `CVidImage` base
    // destructors.
}

// 0x7B0BF0
void CVidMosaic::GetSize(CSize& frameSize, BOOL bDemanded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 5312
    UTIL_ASSERT(pRes != NULL);

    if (!bDemanded) {
        pRes->Demand();
    }

    frameSize.cx = pRes->GetMosaicWidth(m_bDoubleSize);
    frameSize.cy = pRes->GetMosaicHeight(m_bDoubleSize);

    if (!bDemanded) {
        pRes->Release();
    }
}

// 0x7B0C80
BOOL CVidMosaic::Render(INT nSurface, int x, int y, const CRect& rMosaic, const CRect& rClip, DWORD dwFlags, BOOL bDemanded)
{
    if (rClip.IsRectNull()) {
        return TRUE;
    }

    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return Render3d(x, y, rMosaic, rClip, dwFlags, bDemanded);
    }

    if (m_nFxSurface == -1) {
        if (!g_pChitin->GetCurrentVideoMode()->GetFXSurface(m_nFxSurface, dwFlags)) {
            return FALSE;
        }
    }

    if (pRes == NULL) {
        return FALSE;
    }

    if (!bDemanded) {
        pRes->Demand();
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 5431
    UTIL_ASSERT(nSurface < g_pChitin->GetCurrentVideoMode()->GetNumSurfaces() && g_pChitin->GetCurrentVideoMode()->pSurfaces[nSurface] != NULL);

    BltMosFunc Blt;
    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        Blt = &CVidMosaic::BltMos8To16;
        break;
    case 24:
        Blt = &CVidMosaic::BltMos8To24;
        break;
    case 32:
        Blt = &CVidMosaic::BltMos8To32;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 5449
        UTIL_ASSERT(FALSE);

        // NOTE: `pRes` stays demanded on this path.
        return FALSE;
    }

    INT nTileSize = pRes->GetTileSize(m_bDoubleSize);

    // NOTE: Uninline. There is no clamp: a negative `rMosaic.left` or
    // `rMosaic.top` gives negative tile numbers, as in the original.
    INT nTile = GetTileNumber(rMosaic.left, rMosaic.top);

    // NOTE: The row stride is computed from the mosaic width, not taken from
    // the header's `nXTiles`.
    INT nTilesAcross = static_cast<INT>(ceil(static_cast<double>(pRes->GetMosaicWidth(m_bDoubleSize)) / static_cast<double>(nTileSize)));

    INT nLastTileX = rMosaic.right % nTileSize == 0
        ? rMosaic.right / nTileSize
        : rMosaic.right / nTileSize + 1;
    INT nLastTileY = rMosaic.bottom % nTileSize == 0
        ? rMosaic.bottom / nTileSize
        : rMosaic.bottom / nTileSize + 1;

    CRect rTile;
    rTile.SetRect(0, 0, nTileSize, nTileSize);

    DDSURFACEDESC surfaceDesc;
    surfaceDesc.dwSize = sizeof(surfaceDesc);

    INT nTileY = nTile / nTilesAcross;
    INT nFirstTileX = nTile % nTilesAcross;

    x += nFirstTileX * nTileSize;
    y += nTileY * nTileSize;

    BOOL bResult = TRUE;

    for (; nTileY < nLastTileY; nTileY++) {
        if (y >= rClip.bottom) {
            break;
        }

        int destX = x;
        INT nTileX;
        for (nTileX = nFirstTileX; nTileX < nLastTileX; nTileX++, nTile++, destX += nTileSize) {
            if ((nTileX + 1) * nTileSize > pRes->GetMosaicWidth(m_bDoubleSize)) {
                rTile.right = max(0, pRes->GetMosaicWidth(m_bDoubleSize) - nTileX * nTileSize);
            } else {
                rTile.right = nTileSize;
            }

            if (rTile.right == 0) {
                break;
            }

            if ((nTileY + 1) * nTileSize > pRes->GetMosaicHeight(m_bDoubleSize)) {
                rTile.bottom = max(0, pRes->GetMosaicHeight(m_bDoubleSize) - nTileY * nTileSize);
            } else {
                rTile.bottom = nTileSize;
            }

            if (rTile.bottom == 0) {
                break;
            }

            // NOTE: The pointer is rendered for every tile, including the
            // ones skipped by the clip tests below.
            g_pChitin->GetCurrentVideoMode()->RenderPointer();

            if (destX + nTileSize < rClip.left
                || destX >= rClip.right
                || y + nTileSize < rClip.top) {
                continue;
            }

            // NOTE: The original passes a temporary copy of `rTile`.
            if (!g_pChitin->GetCurrentVideoMode()->LockSurface(m_nFxSurface, &surfaceDesc, CRect(rTile))) {
                if (!bDemanded) {
                    pRes->Release();
                }
                return FALSE;
            }

            bResult = (this->*Blt)(reinterpret_cast<DWORD*>(surfaceDesc.lpSurface),
                surfaceDesc.lPitch,
                nTile,
                rTile,
                dwFlags);

            g_pChitin->GetCurrentVideoMode()->UnLockSurface(m_nFxSurface, surfaceDesc.lpSurface);

            if (!bResult) {
                if (!bDemanded) {
                    pRes->Release();
                }
                return FALSE;
            }

            BltFromFX(g_pChitin->GetCurrentVideoMode()->pSurfaces[nSurface],
                destX,
                y,
                rTile.Size(),
                rClip,
                dwFlags);
        }

        y += nTileSize;
        nTile += nTilesAcross - nTileX + nFirstTileX;
    }

    if (!bDemanded) {
        pRes->Release();
    }

    return bResult;
}

// 0x7B1330
BOOL CVidMosaic::BltFromFX(LPDIRECTDRAWSURFACE pSurface, int x, int y, const CSize& size, const CRect& rClip, DWORD dwFlags)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 5597
    UTIL_ASSERT(m_nFxSurface != -1);

    int origX = x;
    int origY = y;

    RECT srcRect;
    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = size.cx;
    srcRect.bottom = size.cy;

    if (x < rClip.left) {
        x = rClip.left;
        srcRect.left = rClip.left - origX;
    }

    if (y < rClip.top) {
        y = rClip.top;
        srcRect.top = rClip.top - origY;
    }

    if (size.cx + origX - 1 >= rClip.right) {
        srcRect.right = rClip.right - origX;
    }

    if (size.cy + origY - 1 >= rClip.bottom) {
        srcRect.bottom = rClip.bottom - origY;
    }

    if (srcRect.bottom >= srcRect.top && srcRect.right >= srcRect.left) {
        if ((dwFlags & 0x1) != 0) {
            do {
                HRESULT hr = g_pChitin->cVideo.cVidBlitter.BltFast(pSurface,
                    x,
                    y,
                    g_pChitin->GetCurrentVideoMode()->pSurfaces[m_nFxSurface],
                    &srcRect,
                    DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
                g_pChitin->GetCurrentVideoMode()->CheckResults(hr);
                if (hr != DDERR_SURFACELOST && hr != DDERR_WASSTILLDRAWING) {
                    break;
                }
            } while (!g_pChitin->field_1932);
        } else {
            do {
                HRESULT hr = g_pChitin->cVideo.cVidBlitter.BltFast(
                    pSurface,
                    x,
                    y,
                    g_pChitin->GetCurrentVideoMode()->pSurfaces[m_nFxSurface],
                    &srcRect,
                    DDBLTFAST_WAIT);
                g_pChitin->GetCurrentVideoMode()->CheckResults(hr);
                if (hr != DDERR_SURFACELOST && hr != DDERR_WASSTILLDRAWING) {
                    break;
                }
            } while (!g_pChitin->field_1932);
        }
    }

    return TRUE;
}

// 0x7B1520
BOOL CVidMosaic::BltMos8To16(DWORD* pSurface, LONG lPitch, UINT nTile, const CRect& rRect, DWORD dwFlags)
{
    int height = rRect.Height();
    int width = rRect.Width();

    if (width != 0) {
        RGBQUAD* pPalette = pRes->GetTilePalette(nTile);
        m_cPalette.SetPalette(pPalette, 256, CVidPalette::TYPE_RESOURCE);
        m_cPalette.Realize(CVidImage::rgbTempPal, 16, dwFlags | 0x400000, &m_paletteAffects, 255);

        BYTE* pTileData = pRes->GetTileData(nTile, m_bDoubleSize);
        LONG nSkip = lPitch - width * 2;

        // TODO: Original code is different (more optimized).
        BYTE* pSurfaceBytes = reinterpret_cast<BYTE*>(pSurface);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                DWORD rgb = CVidImage::rgbTempPal[*pTileData++];
                *(reinterpret_cast<WORD*>(pSurfaceBytes)) = rgb & 0xFFFF;
                pSurfaceBytes += 2;
            }
            pSurfaceBytes += nSkip;
        }
    }

    return TRUE;
}

// 0x7B1670
int CVidMosaic::GetTileNumber(int x, int y)
{
    return pRes->m_pHeader->nXTiles * (y / pRes->GetTileSize(m_bDoubleSize)) + x / pRes->GetTileSize(m_bDoubleSize);
}

// 0x7CC880
BOOL CVidMosaic::BltMos8To24(DWORD* pSurface, LONG lPitch, UINT nTile, const CRect& rRect, DWORD dwFlags)
{
    int height = rRect.Height();
    int width = rRect.Width();

    if (width != 0) {
        m_cPalette.SetPalette(pRes->GetTilePalette(nTile), 256, CVidPalette::TYPE_RESOURCE);
        m_cPalette.Realize(CVidImage::rgbTempPal, 24, dwFlags | 0x400000, &m_paletteAffects, 255);

        BYTE* pTileData = pRes->GetTileData(nTile, m_bDoubleSize);
        LONG nSkip = lPitch - width * 3;

        // NOTE: The original's inline assembly copies `width - 1` pixels and
        // then the last one, and returns early when `width - 1` is zero, so a
        // tile one pixel wide draws nothing. Its row loop is a do-while, so a
        // zero height would not stop it.
        if (width - 1 != 0) {
            // TODO: Original code is different (more optimized).
            BYTE* pSurfaceBytes = reinterpret_cast<BYTE*>(pSurface);
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    DWORD rgb = CVidImage::rgbTempPal[*pTileData++];
                    *pSurfaceBytes++ = rgb & 0xFF;
                    *pSurfaceBytes++ = (rgb >> 8) & 0xFF;
                    *pSurfaceBytes++ = (rgb >> 16) & 0xFF;
                }
                pSurfaceBytes += nSkip;
            }
        }
    }

    return TRUE;
}

// 0x7CC9D0
BOOL CVidMosaic::BltMos8To32(DWORD* pSurface, LONG lPitch, UINT nTile, const CRect& rRect, DWORD dwFlags)
{
    int height = rRect.Height();
    int width = rRect.Width();

    if (width != 0) {
        RGBQUAD* pPalette = pRes->GetTilePalette(nTile);
        if (pPalette == NULL) {
            return FALSE;
        }

        m_cPalette.SetPalette(pPalette, 256, CVidPalette::TYPE_RESOURCE);
        m_cPalette.Realize(CVidImage::rgbTempPal, 32, dwFlags | 0x400000, &m_paletteAffects, 255);

        BYTE* pTileData = pRes->GetTileData(nTile, m_bDoubleSize);
        LONG nSkip = lPitch - width * 4;

        // TODO: Original code is different (more optimized).
        BYTE* pSurfaceBytes = reinterpret_cast<BYTE*>(pSurface);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                *(reinterpret_cast<DWORD*>(pSurfaceBytes)) = CVidImage::rgbTempPal[*pTileData++];
                pSurfaceBytes += 4;
            }
            pSurfaceBytes += nSkip;
        }
    }

    return TRUE;
}

// 0x7C56C0
void CVidMosaic::RenderTexture(INT x, INT y, const CSize& blitSize, const CRect& rClip, DWORD dwFlags)
{
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
    x2 = static_cast<float>(x + blitSize.cx);
    y2 = static_cast<float>(y + blitSize.cy);
    x3 = 0.0f;
    y3 = 0.0f;
    x4 = static_cast<float>(blitSize.cx) / static_cast<float>(CVidInf::FX_WIDTH);
    y4 = static_cast<float>(blitSize.cy) / static_cast<float>(CVidInf::FX_HEIGHT);

    if (x < rClip.left) {
        x1 = static_cast<float>(rClip.left);
        x3 = static_cast<float>(rClip.left - x) / static_cast<float>(CVidInf::FX_WIDTH);
    }

    if (y < rClip.top) {
        y1 = static_cast<float>(rClip.top);
        y3 = static_cast<float>(rClip.top - y) / static_cast<float>(CVidInf::FX_HEIGHT);
    }

    if (x + blitSize.cx > rClip.right) {
        x2 = static_cast<float>(rClip.right);
        x4 = static_cast<float>(rClip.right - x) / static_cast<float>(CVidInf::FX_WIDTH);
    }

    if (y + blitSize.cy > rClip.bottom) {
        y2 = static_cast<float>(rClip.bottom);
        y4 = static_cast<float>(rClip.bottom - y) / static_cast<float>(CVidInf::FX_HEIGHT);
    }

    CVideo3d::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glEnable(GL_BLEND);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

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

    CVideo3d::glDisable(GL_BLEND);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
}

// 0x7C5A00
BOOL CVidMosaic::Render3d(int x, int y, const CRect& rMosaic, const CRect& rClip, DWORD dwFlags, BOOL bDemanded)
{
    if (pRes == NULL) {
        return FALSE;
    }

    if (!bDemanded) {
        pRes->Demand();
    }

    INT nTileSize = pRes->GetTileSize(m_bDoubleSize);

    // NOTE: There is no clamp: a negative `rMosaic.left` or `rMosaic.top`
    // gives negative tile numbers, as in the original.
    INT nTile = GetTileNumber(rMosaic.left, rMosaic.top);

    // NOTE: The row stride is computed from the mosaic width, not taken from
    // the header's `nXTiles`.
    INT nTilesAcross = static_cast<INT>(ceil(static_cast<double>(pRes->GetMosaicWidth(m_bDoubleSize)) / static_cast<double>(nTileSize)));

    INT nLastTileX = rMosaic.right % nTileSize == 0
        ? rMosaic.right / nTileSize
        : rMosaic.right / nTileSize + 1;
    INT nLastTileY = rMosaic.bottom % nTileSize == 0
        ? rMosaic.bottom / nTileSize
        : rMosaic.bottom / nTileSize + 1;

    CRect rTile;
    rTile.SetRect(0, 0, nTileSize, nTileSize);

    INT nTileY = nTile / nTilesAcross;
    INT nFirstTileX = nTile % nTilesAcross;

    y += nTileY * nTileSize;
    x += nFirstTileX * nTileSize;

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

    BOOL bResult = TRUE;

    for (; nTileY < nLastTileY; nTileY++) {
        if (y >= rClip.bottom) {
            break;
        }

        int destX = x;
        INT nTileX;
        for (nTileX = nFirstTileX; nTileX < nLastTileX; nTileX++, nTile++, destX += nTileSize) {
            // NOTE: The pointer is rendered for every tile, before the size
            // and clip tests.
            g_pChitin->GetCurrentVideoMode()->RenderPointer();

            if ((nTileX + 1) * nTileSize > pRes->GetMosaicWidth(m_bDoubleSize)) {
                rTile.right = max(0, pRes->GetMosaicWidth(m_bDoubleSize) - nTileX * nTileSize);
            } else {
                rTile.right = nTileSize;
            }

            if (rTile.right == 0) {
                break;
            }

            if ((nTileY + 1) * nTileSize > pRes->GetMosaicHeight(m_bDoubleSize)) {
                rTile.bottom = max(0, pRes->GetMosaicHeight(m_bDoubleSize) - nTileY * nTileSize);
            } else {
                rTile.bottom = nTileSize;
            }

            if (rTile.bottom == 0) {
                break;
            }

            CSize blitSize(rTile.right, rTile.bottom);

            if (destX + nTileSize < rClip.left
                || destX >= rClip.right
                || y + nTileSize < rClip.top) {
                continue;
            }

            LONG lPitch = g_pChitin->cVideo.field_13A
                ? CVidTile::BYTES_PER_TEXEL * CVIDINF_FX_WIDTH
                : CVidTile::BYTES_PER_TEXEL * rTile.right;

            bResult = BltMos8To32(reinterpret_cast<DWORD*>(CVideo3d::texImageData),
                lPitch,
                nTile,
                rTile,
                dwFlags);
            if (!bResult) {
                if (!bDemanded) {
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
                    blitSize.cx,
                    blitSize.cy,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    CVideo3d::texImageData);
                g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
            }

            RenderTexture(destX, y, blitSize, rClip, dwFlags);
        }

        y += nTileSize;
        nTile += nTilesAcross - nTileX + nFirstTileX;
    }

    if (!bDemanded) {
        pRes->Release();
    }

    return bResult;
}
