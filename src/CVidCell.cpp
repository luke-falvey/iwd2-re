#include "CVidCell.h"

#include "CUtil.h"
#include "CVidInf.h"
#include "CVidMode.h"
#include "CVidPoly.h"
#include "CVideo3d.h"

// 0x8BAC88
DWORD CVidCell::dword_8BAC88 = -1;

// 0x9064F4
BOOL CVidCell::TRANSLUCENT_BLTS_ON;

// 0x8BAC8C
BOOL CVidCell::TRANSLUCENT_SHADOWS_ON = TRUE;

// NOTE: The following globals belong to `ChVidImage24.cpp`. They are only
// written to (destination luminance cache) except `dword_8BDDF4`, which is
// used as a divisor.

// 0x8BDDE4
static INT dword_8BDDE4 = 100;

// 0x8BDDE8
static INT dword_8BDDE8 = 100;

// 0x8BDDF0
static INT dword_8BDDF0 = 100;

// 0x8BDDF4
static INT dword_8BDDF4 = 7;

// 0x8BDDF8
static INT dword_8BDDF8 = 100;

// #binary-identical
// 0x7ACD70
CVidCell::CVidCell()
{
    m_nCurrentFrame = 0;
    m_nCurrentSequence = 0;
    m_bPaletteChanged = FALSE;
    m_pFrame = NULL;
    m_bDoubleSize = FALSE;
    field_C8 = 1;
    m_bShadowOn = TRUE;
}

// 0x7ACE50
CVidCell::CVidCell(CResRef cNewResRef, BOOL bDoubleSize)
{
    // NOTE: Uninline.
    SetResRef(cNewResRef, bDoubleSize, TRUE, TRUE);

    field_C8 = 1;
    m_nCurrentFrame = 0;
    m_nCurrentSequence = 0;
    m_bPaletteChanged = FALSE;
    m_pFrame = NULL;
    m_bShadowOn = TRUE;
}

// 0x7AD0F0
CVidCell::~CVidCell()
{
    for (int index = 0; index < CVidPalette::NUM_RANGES; index++) {
        if (m_paletteAffects.pRangeTints[index] != NULL) {
            delete m_paletteAffects.pRangeTints[index];
        }

        if (m_paletteAffects.pRangeAdds[index] != NULL) {
            delete m_paletteAffects.pRangeAdds[index];
        }

        if (m_paletteAffects.pRangeLights[index] != NULL) {
            delete m_paletteAffects.pRangeLights[index];
        }
    }
}

// #binary-identical
// 0x7AD280
void CVidCell::AddRangeAffect(USHORT nAffectType, USHORT nRangeId, COLORREF rgbAffectColor, BYTE pulsePeriod)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 523
    UTIL_ASSERT(nRangeId < CVidPalette::NUM_RANGES);

    switch (nAffectType) {
    case 0:
        if (m_paletteAffects.pRangeTints[nRangeId] == NULL) {
            m_paletteAffects.pRangeTints[nRangeId] = new COLORREF;
        }
        *m_paletteAffects.pRangeTints[nRangeId] = rgbAffectColor;
        m_paletteAffects.aRangeTintPeriods[nRangeId] = pulsePeriod;
        break;
    case 1:
    case 3:
        if (m_paletteAffects.pRangeAdds[nRangeId] == NULL) {
            m_paletteAffects.pRangeAdds[nRangeId] = new COLORREF;
        }
        *m_paletteAffects.pRangeAdds[nRangeId] = rgbAffectColor;
        m_paletteAffects.aRangeAddPeriods[nRangeId] = pulsePeriod;
        break;
    case 2:
        if (m_paletteAffects.pRangeLights[nRangeId] == NULL) {
            m_paletteAffects.pRangeLights[nRangeId] = new COLORREF;
        }
        *m_paletteAffects.pRangeLights[nRangeId] = rgbAffectColor;
        m_paletteAffects.aRangeLightPeriods[nRangeId] = pulsePeriod;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 523
        UTIL_ASSERT(FALSE);
    }
}

// #binary-identical
// 0x7AD3A0
void CVidCell::DeleteResPaletteAffect()
{
    if (m_paletteAffects.pRangeTints[0] != NULL) {
        delete m_paletteAffects.pRangeTints[0];
        m_paletteAffects.pRangeTints[0] = NULL;
        m_paletteAffects.aRangeTintPeriods[0] = 0;
    }

    if (m_paletteAffects.pRangeAdds[0] != NULL) {
        delete m_paletteAffects.pRangeAdds[0];
        m_paletteAffects.pRangeAdds[0] = NULL;
        m_paletteAffects.aRangeAddPeriods[0] = 0;
    }

    if (m_paletteAffects.pRangeLights[0] != NULL) {
        delete m_paletteAffects.pRangeLights[0];
        m_paletteAffects.pRangeLights[0] = NULL;
        m_paletteAffects.aRangeLightPeriods[0] = 0;
    }
}

// #binary-identical
// 0x7AD400
void CVidCell::DeleteRangeAffects(USHORT nRangeId)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 635
    UTIL_ASSERT(nRangeId < CVidPalette::NUM_RANGES);

    if (m_paletteAffects.pRangeTints[nRangeId] != NULL) {
        delete m_paletteAffects.pRangeTints[nRangeId];
        m_paletteAffects.pRangeTints[nRangeId] = NULL;
    }

    if (m_paletteAffects.pRangeAdds[nRangeId] != NULL) {
        delete m_paletteAffects.pRangeAdds[nRangeId];
        m_paletteAffects.pRangeAdds[nRangeId] = NULL;
    }

    if (m_paletteAffects.pRangeLights[nRangeId] != NULL) {
        delete m_paletteAffects.pRangeLights[nRangeId];
        m_paletteAffects.pRangeLights[nRangeId] = NULL;
    }
}

// 0x7AD490
BOOL CVidCell::Demand(BOOLEAN& bDemanded)
{
    if (!bDemanded) {
        if (m_header.GetResRef() != "") {
            if (m_header.pRes->m_pBamHeaderCopy != NULL) {
                bDemanded = TRUE;
            }

            if (!bDemanded) {
                if (m_header.pRes->Demand() == NULL) {
                    return FALSE;
                }
            }
        } else {
            if (pRes == NULL) {
                return FALSE;
            }

            if (pRes->m_pBamHeaderCopy != NULL) {
                bDemanded = TRUE;
            }

            if (!bDemanded) {
                if (pRes->Demand() == NULL) {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

// 0x7AD520
BOOL CVidCell::GetCurrentCenterPoint(CPoint& ptReference, BOOLEAN bDemanded)
{
    if (pRes == NULL) {
        ptReference.x = 0;
        ptReference.y = 0;
        return FALSE;
    }

    if (!Demand(bDemanded)) {
        ptReference.x = 0;
        ptReference.y = 0;
        return FALSE;
    }

    SHORT nSequence;
    // NOTE: Uninline.
    if (m_nCurrentSequence < GetResBamHeader()->nSequences) {
        nSequence = m_nCurrentSequence;
    } else {
        nSequence = 0;
    }

    SHORT nFrame;
    // NOTE: Uninline.
    if (m_nCurrentFrame < GetResSequences()[nSequence].nFrames) {
        nFrame = m_nCurrentFrame;
    } else {
        if (field_C8) {
            // NOTE: Uninline.
            if (GetResSequences()[nSequence].nFrames > 0) {
                // NOTE: Uninline.
                nFrame = m_nCurrentFrame % GetResSequences()[nSequence].nFrames;
            } else {
                nFrame = 0;
            }
        } else {
            // NOTE: Uninline.
            nFrame = GetResSequences()[nSequence].nFrames - 1;
        }
    }

    if (nFrame < 0) {
        if (field_C8) {
            // FIXME: Repeating calls.
            // NOTE: Lots of inlining.
            if (GetResSequences()[nSequence].nFrames > 0) {
                nFrame = GetResSequences()[nSequence].nFrames + nFrame % GetResSequences()[nSequence].nFrames;
                if (nFrame == GetResSequences()[nSequence].nFrames) {
                    nFrame = 0;
                }
            } else {
                nFrame = 0;
            }
        } else {
            nFrame = 0;
        }
    }

    BOOL bSuccess;

    // NOTE: Uninline.
    FRAMEENTRY* pFrame = GetResFrame(nSequence, nFrame);
    if (pFrame != NULL) {
        ptReference.x = pFrame->nCenterX;
        ptReference.y = pFrame->nCenterY;
        bSuccess = TRUE;
    } else {
        bSuccess = FALSE;
    }

    // NOTE: Uninline.
    Release(bDemanded);

    return bSuccess;
}

// 0x7AD800
BOOL CVidCell::GetCurrentFrameSize(CSize& frameSize, BOOLEAN bDemanded)
{
    if (pRes == NULL) {
        frameSize.cx = 0;
        frameSize.cy = 0;
        return FALSE;
    }

    if (!Demand(bDemanded)) {
        frameSize.cx = 0;
        frameSize.cy = 0;
        return FALSE;
    }

    SHORT nSequence;
    // NOTE: Uninline.
    if (m_nCurrentSequence < GetResBamHeader()->nSequences) {
        nSequence = m_nCurrentSequence;
    } else {
        nSequence = 0;
    }

    SHORT nFrame;
    // NOTE: Uninline.
    if (m_nCurrentFrame < GetResSequences()[nSequence].nFrames) {
        nFrame = m_nCurrentFrame;
    } else {
        if (field_C8) {
            // NOTE: Uninline.
            if (GetResSequences()[nSequence].nFrames > 0) {
                // NOTE: Uninline.
                nFrame = m_nCurrentFrame % GetResSequences()[nSequence].nFrames;
            } else {
                nFrame = 0;
            }
        } else {
            // NOTE: Uninline.
            nFrame = GetResSequences()[nSequence].nFrames - 1;
        }
    }

    if (nFrame < 0) {
        if (field_C8) {
            // FIXME: Repeating calls.
            // NOTE: Lots of inlining.
            if (GetResSequences()[nSequence].nFrames > 0) {
                nFrame = GetResSequences()[nSequence].nFrames + nFrame % GetResSequences()[nSequence].nFrames;
                if (nFrame == GetResSequences()[nSequence].nFrames) {
                    nFrame = 0;
                }
            } else {
                nFrame = 0;
            }
        } else {
            nFrame = 0;
        }
    }

    BOOL bSuccess;

    // NOTE: Uninline.
    FRAMEENTRY* pFrame = GetResFrame(nSequence, nFrame);
    if (pFrame != NULL) {
        frameSize.cx = pFrame->nWidth;
        frameSize.cy = pFrame->nHeight;
        bSuccess = TRUE;
    } else {
        bSuccess = FALSE;
    }

    // NOTE: Uninline.
    Release(bDemanded);

    return bSuccess;
}

// 0x7ADAE0
BYTE CVidCell::GetSequenceLength(WORD nSequence, BOOLEAN bDemanded)
{
    if (!Demand(bDemanded)) {
        return 0;
    }

    BYTE nFrames;
    // NOTE: Uninline.
    if (nSequence < GetResBamHeader()->nSequences) {
        // NOTE: Uninline.
        nFrames = static_cast<BYTE>(GetResSequences()[nSequence].nFrames);
    } else {
        nFrames = 0;
    }

    // NOTE: Uninline.
    Release(bDemanded);

    return nFrames;
}

// 0x7ADBC0
BOOL CVidCell::GetFrameCenterPoint(SHORT nSequence, SHORT nFrame, CSize& ptCtr, BOOLEAN bDemanded)
{
    if (nSequence == -1) {
        nSequence = m_nCurrentSequence;
    }

    if (nFrame == -1) {
        nFrame = m_nCurrentFrame;
        nSequence = m_nCurrentSequence;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 1176
    UTIL_ASSERT(pRes != NULL);

    if (!Demand(bDemanded)) {
        return FALSE;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 1189
    UTIL_ASSERT(nSequence < GetResBamHeader()->nSequences);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 1189
    UTIL_ASSERT(nFrame < GetResSequences()[nSequence].nFrames);

    BOOL bSuccess;

    FRAMEENTRY* pFrame = GetResFrame(nSequence, nFrame);
    if (pFrame != NULL) {
        ptCtr.cx = pFrame->nCenterX;
        ptCtr.cy = pFrame->nCenterY;
        bSuccess = TRUE;
    } else {
        bSuccess = FALSE;
    }

    Release(bDemanded);

    return bSuccess;
}

// 0x7ADD80
BOOL CVidCell::GetFrameSize(SHORT nSequence, SHORT nFrame, CSize& frameSize, BOOLEAN bDemanded)
{
    if (nSequence == -1) {
        nSequence = m_nCurrentSequence;
    }

    if (nFrame == -1) {
        nFrame = m_nCurrentFrame;
        nSequence = m_nCurrentSequence;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 1258
    UTIL_ASSERT(pRes != NULL);

    if (!Demand(bDemanded)) {
        return FALSE;
    }

    // NOTE: Uninline.
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 1271
    UTIL_ASSERT(nSequence < GetResBamHeader()->nSequences);

    // NOTE: Uninline.
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 1272
    UTIL_ASSERT(nFrame < GetResSequences()[nSequence].nFrames);

    BOOL bSuccess;

    // NOTE: Uninline.
    FRAMEENTRY* pFrame = GetResFrame(nSequence, nFrame);
    if (pFrame != NULL) {
        frameSize.cx = pFrame->nWidth;
        frameSize.cy = pFrame->nHeight;
        bSuccess = TRUE;
    } else {
        bSuccess = FALSE;
    }

    // NOTE: Uninline.
    Release(bDemanded);

    return bSuccess;
}

// 0x7ADF40
SHORT CVidCell::GetNumberSequences(BOOLEAN bDemanded)
{
    if (pRes == NULL) {
        return 0;
    }

    if (!Demand(bDemanded)) {
        return 0;
    }

    SHORT nSequences = GetResBamHeader()->nSequences;

    Release(bDemanded);

    return nSequences;
}

// #binary-identical
// 0x7ADFE0
BOOL CVidCell::FrameAdvance()
{
    m_nCurrentFrame++;
    return TRUE;
}

// #binary-identical
// 0x7ADFF0
BOOL CVidCell::FrameSet(SHORT nFrame)
{
    m_nCurrentFrame = nFrame;
    return TRUE;
}

// 0x7AE010
BOOL CVidCell::IsEndOfSequence(BOOLEAN bDemanded)
{
    BOOL bResult = FALSE;

    if (pRes == NULL) {
        return TRUE;
    }

    if (!Demand(bDemanded)) {
        return FALSE;
    }

    // NOTE: Uninline.
    if (m_nCurrentSequence < GetResBamHeader()->nSequences) {
        bResult = TRUE;
        // FIXME: Repeating calls.
        // NOTE: Uninline.
        if (m_nCurrentFrame < GetResSequences()[m_nCurrentSequence].nFrames - 1) {
            // NOTE: Uninline.
            if (GetResSequences()[m_nCurrentSequence].nFrames != 0) {
                bResult = FALSE;
            }
        }
    }

    // NOTE: Uninline.
    Release(bDemanded);

    return bResult;
}

// 0x7AE140
void CVidCell::RestoreBackground(INT nFrom, INT nTo, const CRect& rClip)
{
    RECT srcRect;
    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = rClip.Width();
    srcRect.bottom = rClip.Height();

    do {
        HRESULT hr = g_pChitin->cVideo.cVidBlitter.BltFast(g_pChitin->GetCurrentVideoMode()->pSurfaces[nTo],
            rClip.left,
            rClip.top,
            g_pChitin->GetCurrentVideoMode()->pSurfaces[nFrom],
            &srcRect,
            DDBLTFAST_WAIT);
        g_pChitin->GetCurrentVideoMode()->CheckResults(hr);
        if (hr != DDERR_SURFACELOST && hr != DDERR_WASSTILLDRAWING) {
            break;
        }
    } while (!g_pChitin->field_1932);
}

// 0x7AE210
void CVidCell::RealizePalette(DWORD dwFlags)
{
    if (!m_bPaletteChanged) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 1537
        UTIL_ASSERT(pRes != NULL);

        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, g_pChitin->cVideo.m_nBpp, dwFlags, &m_paletteAffects, 255);
}

// #binary-identical
// 0x7AE290
BOOL CVidCell::SequenceSet(SHORT nSequence)
{
    m_nCurrentSequence = nSequence;
    return TRUE;
}

// 0x7AE2B0
void CVidCell::SetPalette(RGBQUAD* pPalette, int nEntries, USHORT nType)
{
    if (pPalette != NULL) {
        m_cPalette.SetPalette(pPalette, nEntries, nType);
        m_bPaletteChanged = TRUE;
    } else {
        m_cPalette.SetPalette(NULL, 0, CVidPalette::TYPE_RESOURCE);
        m_bPaletteChanged = FALSE;
    }
}

// 0x7AE300
void CVidCell::SetPalette(const CVidPalette& cPalette)
{
    m_cPalette.SetPalette(cPalette);
    m_bPaletteChanged = TRUE;
}

// #binary-identical
// 0x7AE320
void CVidCell::AddResPaletteAffect(USHORT nAffectType, COLORREF rgbAffectColor, BYTE pulsePeriod)
{
    switch (nAffectType) {
    case 0:
        if (m_paletteAffects.pRangeTints[0] == NULL) {
            m_paletteAffects.pRangeTints[0] = new COLORREF;
        }
        *m_paletteAffects.pRangeTints[0] = rgbAffectColor;
        m_paletteAffects.aRangeTintPeriods[0] = pulsePeriod;
        break;
    case 1:
    case 3:
        if (m_paletteAffects.pRangeAdds[0] == NULL) {
            m_paletteAffects.pRangeAdds[0] = new COLORREF;
        }
        *m_paletteAffects.pRangeAdds[0] = rgbAffectColor;
        m_paletteAffects.aRangeAddPeriods[0] = pulsePeriod;
        break;
    case 2:
        if (m_paletteAffects.pRangeLights[0] == NULL) {
            m_paletteAffects.pRangeLights[0] = new COLORREF;
        }
        *m_paletteAffects.pRangeLights[0] = rgbAffectColor;
        m_paletteAffects.aRangeLightPeriods[0] = pulsePeriod;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 1704
        UTIL_ASSERT(FALSE);
    }
}

// 0x7AE3F0
void CVidCell::StoreBackground(INT nFrom, INT nTo, INT x, INT y, const CRect& rClip, CRect& rStorage, BOOLEAN bNumbered)
{
    if (GetFrame(FALSE)) {
        rStorage.left = x - m_pFrame->nCenterX;
        rStorage.top = y - m_pFrame->nCenterY;

        rStorage.left = x - m_pFrame->nCenterX;
        rStorage.top = y - m_pFrame->nCenterY;

        if (bNumbered) {
            rStorage.right = max(rStorage.left + m_pFrame->nWidth, rStorage.left + m_pFrame->nCenterX + 16);
            rStorage.bottom = max(rStorage.top + m_pFrame->nHeight, rStorage.top + m_pFrame->nCenterY + 16);
        } else {
            rStorage.right = rStorage.left + m_pFrame->nWidth;
            rStorage.bottom = rStorage.top + m_pFrame->nHeight;
        }

        // NOTE: Looks like intersection.
        if (rStorage.left < rClip.left) {
            rStorage.left = rClip.left;
        }

        if (rStorage.top < rClip.top) {
            rStorage.top = rClip.top;
        }

        if (rStorage.right >= rClip.right) {
            rStorage.right = rClip.right;
        }

        if (rStorage.bottom >= rClip.bottom) {
            rStorage.bottom = rClip.bottom;
        }

        if (rStorage.left < rStorage.right && rStorage.top < rStorage.bottom) {
            do {
                HRESULT hr = g_pChitin->cVideo.cVidBlitter.BltFast(g_pChitin->GetCurrentVideoMode()->pSurfaces[nTo],
                    0,
                    0,
                    g_pChitin->GetCurrentVideoMode()->pSurfaces[nFrom],
                    &rStorage,
                    DDBLTFAST_WAIT);
                g_pChitin->GetCurrentVideoMode()->CheckResults(hr);
                if (hr != DDERR_SURFACELOST && hr != DDERR_WASSTILLDRAWING) {
                    break;
                }
            } while (!g_pChitin->field_1932);
        }
    }
}

// 0x7AE590
void CVidCell::SuppressTint(SHORT nRange)
{
    m_paletteAffects.suppressTints |= 1 << nRange;
}

// 0x7AE5B0
void CVidCell::UnsuppressTint(SHORT nRange)
{
    m_paletteAffects.suppressTints &= ~(1 << nRange);
}

// 0x7AE5D0
BOOL CVidCell::Render(WORD* pSurface, LONG lPitch, INT nRefPointX, INT nRefPointY, DWORD dwFlags, INT nTransVal)
{
    BOOL bSuccess;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 1902
    UTIL_ASSERT(!((dwFlags & CVIDIMG_TRANSLUCENT) && (dwFlags & CVIDCELL_BRIGHTEST)));

    if (m_pFrame == NULL) {
        if (!GetFrame(FALSE)) {
            // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
            // __LINE__: 1913
            UTIL_ASSERT_MSG(FALSE, GetResRef().GetResRefStr() + ".BAM is corrupted (GetFrame() failed)");
        }
    }

    if ((dwFlags & 0x200) != 0 && TRANSLUCENT_BLTS_ON) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 1902
        UTIL_ASSERT(nTransVal >= 0);

        switch (g_pChitin->cVideo.m_nBpp) {
        case 16:
            bSuccess = Blt8To16Glow(pSurface
                    + lPitch / 2 * (nRefPointY - m_pFrame->nCenterY)
                    + (nRefPointX - m_pFrame->nCenterX),
                lPitch,
                dwFlags);
            break;
        case 24:
            bSuccess = Blt8To24Glow(reinterpret_cast<BYTE*>(pSurface)
                    + lPitch * (nRefPointY - m_pFrame->nCenterY)
                    + 3 * (nRefPointX - m_pFrame->nCenterX),
                lPitch,
                dwFlags);
            break;
        case 32:
            bSuccess = Blt8To32Glow(reinterpret_cast<DWORD*>(pSurface)
                    + lPitch / 4 * (nRefPointY - m_pFrame->nCenterY)
                    + (nRefPointX - m_pFrame->nCenterX),
                lPitch,
                dwFlags);
            break;
        default:
            bSuccess = FALSE;
            break;
        }
    } else {
        if ((dwFlags & CVIDIMG_TRANSLUCENT) != 0) {
            // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
            // __LINE__: 1954
            UTIL_ASSERT(nTransVal >= 0);

            switch (g_pChitin->cVideo.m_nBpp) {
            case 16:
                bSuccess = Blt8To16Translucent(pSurface
                        + lPitch / 2 * (nRefPointY - m_pFrame->nCenterY)
                        + (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags,
                    nTransVal);
                break;
            case 24:
                bSuccess = Blt8To24Translucent(reinterpret_cast<BYTE*>(pSurface)
                        + lPitch * (nRefPointY - m_pFrame->nCenterY)
                        + 3 * (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags,
                    nTransVal);
                break;
            case 32:
                bSuccess = Blt8To32Translucent(reinterpret_cast<DWORD*>(pSurface)
                        + lPitch / 4 * (nRefPointY - m_pFrame->nCenterY)
                        + (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags,
                    nTransVal);
                break;
            default:
                bSuccess = FALSE;
                break;
            }
        } else if ((dwFlags & 0x8) != 0) {
            switch (g_pChitin->cVideo.m_nBpp) {
            case 16:
                bSuccess = Blt8To16Brightest(pSurface
                        + lPitch / 2 * (nRefPointY - m_pFrame->nCenterY)
                        + (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags);
                break;
            case 24:
                bSuccess = Blt8To24Brightest(reinterpret_cast<BYTE*>(pSurface)
                        + lPitch * (nRefPointY - m_pFrame->nCenterY)
                        + 3 * (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags);
                break;
            case 32:
                bSuccess = Blt8To32Brightest(reinterpret_cast<DWORD*>(pSurface)
                        + lPitch / 4 * (nRefPointY - m_pFrame->nCenterY)
                        + (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags);
                break;
            default:
                bSuccess = FALSE;
                break;
            }
        } else if ((dwFlags & 0x4) != 0 && CVidCell::TRANSLUCENT_SHADOWS_ON) {
            switch (g_pChitin->cVideo.m_nBpp) {
            case 16:
                bSuccess = Blt8To16ClearShadow(pSurface
                        + lPitch / 2 * (nRefPointY - m_pFrame->nCenterY)
                        + (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags);
                break;
            case 24:
                bSuccess = Blt8To24ClearShadow(reinterpret_cast<BYTE*>(pSurface)
                        + lPitch * (nRefPointY - m_pFrame->nCenterY)
                        + 3 * (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags);
                break;
            case 32:
                bSuccess = Blt8To32ClearShadow(reinterpret_cast<DWORD*>(pSurface)
                        + lPitch / 4 * (nRefPointY - m_pFrame->nCenterY)
                        + (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags);
                break;
            default:
                bSuccess = FALSE;
                break;
            }
        } else {
            switch (g_pChitin->cVideo.m_nBpp) {
            case 16:
                bSuccess = Blt8To16(pSurface
                        + lPitch / 2 * (nRefPointY - m_pFrame->nCenterY)
                        + (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags);
                break;
            case 24:
                bSuccess = Blt8To24(reinterpret_cast<BYTE*>(pSurface)
                        + lPitch * (nRefPointY - m_pFrame->nCenterY)
                        + 3 * (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags);
                break;
            case 32:
                bSuccess = Blt8To32(reinterpret_cast<DWORD*>(pSurface)
                        + lPitch / 4 * (nRefPointY - m_pFrame->nCenterY)
                        + (nRefPointX - m_pFrame->nCenterX),
                    lPitch,
                    dwFlags,
                    255);
                break;
            default:
                bSuccess = FALSE;
                break;
            }
        }
    }

    pRes->Release();
    m_pFrame = NULL;

    return bSuccess;
}

// 0x7AEAD0
BOOL CVidCell::Render(INT nSurface, int x, int y, const CRect& rClip, CVidPoly* pClipPoly, int nPolys, DWORD dwFlags, int nTransVal)
{
    BOOL bSuccess;

    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
        return Render3d(x, y, rClip, pClipPoly, nPolys, dwFlags | 0x1, nTransVal, 0);
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 2065
    UTIL_ASSERT(nSurface < g_pChitin->GetCurrentVideoMode()->GetNumSurfaces() && g_pChitin->GetCurrentVideoMode()->pSurfaces[nSurface] != NULL);

    if (m_nFxSurface == -1) {
        if (!g_pChitin->GetCurrentVideoMode()->GetFXSurface(m_nFxSurface, dwFlags)) {
            return FALSE;
        }
    }

    DDCOLORKEY ddck;
    g_pChitin->GetCurrentVideoMode()->pSurfaces[m_nFxSurface]->GetColorKey(DDCKEY_SRCBLT, &ddck);

    if (m_pFrame == NULL) {
        if (!GetFrame(FALSE)) {
            return FALSE;
        }
    }

    int left = x - m_pFrame->nCenterX;
    int top = y - m_pFrame->nCenterY;
    CRect rSrc(0, 0, m_pFrame->nWidth, m_pFrame->nHeight);
    CRect rDest(left, top, left + m_pFrame->nWidth, top + m_pFrame->nHeight);

    DDSURFACEDESC ddsd;

    if ((dwFlags & CVIDIMG_TRANSLUCENT) || ((dwFlags & 0x4) != 0 && CVidCell::TRANSLUCENT_SHADOWS_ON)) {
        BltBackGroundToFx(g_pChitin->GetCurrentVideoMode()->pSurfaces[nSurface],
            rDest,
            rClip,
            dwFlags);

        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 2112
        UTIL_ASSERT(m_nFxSurface != -1);

        ddsd.dwSize = sizeof(ddsd);
        if (!g_pChitin->GetCurrentVideoMode()->LockSurface(m_nFxSurface, &ddsd, rSrc)) {
            pRes->Release();
            return FALSE;
        }

        if ((dwFlags & 0x4) != 0) {
            switch (g_pChitin->cVideo.m_nBpp) {
            case 16:
                bSuccess = Blt8To16ClearShadow(reinterpret_cast<WORD*>(ddsd.lpSurface),
                    ddsd.lPitch,
                    dwFlags);
                break;
            case 24:
                bSuccess = Blt8To24ClearShadow(reinterpret_cast<BYTE*>(ddsd.lpSurface),
                    ddsd.lPitch,
                    dwFlags);
                break;
            case 32:
                bSuccess = Blt8To32ClearShadow(reinterpret_cast<DWORD*>(ddsd.lpSurface),
                    ddsd.lPitch,
                    dwFlags);
                break;
            default:
                bSuccess = FALSE;
                break;
            }
        } else {
            // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
            // __LINE__: 2145
            UTIL_ASSERT(nTransVal >= 0);

            switch (g_pChitin->cVideo.m_nBpp) {
            case 16:
                bSuccess = Blt8To16Translucent(reinterpret_cast<WORD*>(ddsd.lpSurface),
                    ddsd.lPitch,
                    dwFlags,
                    nTransVal);
                break;
            case 24:
                bSuccess = Blt8To24Translucent(reinterpret_cast<BYTE*>(ddsd.lpSurface),
                    ddsd.lPitch,
                    dwFlags,
                    nTransVal);
                break;
            case 32:
                bSuccess = Blt8To32Translucent(reinterpret_cast<DWORD*>(ddsd.lpSurface),
                    ddsd.lPitch,
                    dwFlags,
                    nTransVal);
                break;
            default:
                bSuccess = FALSE;
                break;
            }
        }
    } else {
        DDBLTFX fx;
        fx.dwSize = sizeof(fx);
        fx.dwFillColor = ddck.dwColorSpaceLowValue;

        g_pChitin->cVideo.cVidBlitter.Blt(g_pChitin->GetCurrentVideoMode()->pSurfaces[m_nFxSurface],
            rSrc,
            NULL,
            NULL,
            DDBLT_COLORFILL | DDBLT_WAIT,
            &fx);

        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 2180
        UTIL_ASSERT(m_nFxSurface != -1);

        ddsd.dwSize = sizeof(ddsd);
        if (!g_pChitin->GetCurrentVideoMode()->LockSurface(m_nFxSurface, &ddsd, rSrc)) {
            pRes->Release();
            return FALSE;
        }

        switch (g_pChitin->cVideo.m_nBpp) {
        case 16:
            bSuccess = Blt8To16(reinterpret_cast<WORD*>(ddsd.lpSurface),
                ddsd.lPitch,
                dwFlags);
            break;
        case 24:
            bSuccess = Blt8To24(reinterpret_cast<BYTE*>(ddsd.lpSurface),
                ddsd.lPitch,
                dwFlags);
            break;
        case 32:
            bSuccess = Blt8To32(reinterpret_cast<DWORD*>(ddsd.lpSurface),
                ddsd.lPitch,
                dwFlags,
                255);
            break;
        default:
            bSuccess = FALSE;
            break;
        }
    }

    if (pClipPoly != NULL) {
        for (int nIndex = 0; nIndex < nPolys; nIndex++) {
            pClipPoly[nIndex].FillPoly(reinterpret_cast<WORD*>(ddsd.lpSurface),
                ddsd.lPitch,
                rSrc,
                ddck.dwColorSpaceLowValue,
                0,
                CPoint(left, top));
        }
    }

    g_pChitin->GetCurrentVideoMode()->UnLockSurface(m_nFxSurface, ddsd.lpSurface);

    if (bSuccess) {
        BltFromFX(g_pChitin->GetCurrentVideoMode()->pSurfaces[nSurface],
            left,
            top,
            rClip,
            dwFlags);
    }

    pRes->Release();
    m_pFrame = NULL;

    return bSuccess;
}

// 0x7AF060
BOOL CVidCell::Render(WORD* pSurface, LONG lPitch, INT nRefPtX, INT nRefPtY, const CRect& rClip, BOOLEAN bDemanded, DWORD dwFlags, const CPoint& ptSource)
{
    if (m_pFrame == NULL) {
        if (!GetFrame(bDemanded)) {
            return FALSE;
        }
    }

    if (pRes == NULL) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
        // __LINE__: 2289
        UTIL_ASSERT_MSG(FALSE, "CVidCell::Render m_pFrame = GARBAGE");
    }

    CPoint pt(nRefPtX - m_pFrame->nCenterX, nRefPtY - m_pFrame->nCenterY);

    BOOL bResult;
    switch (g_pChitin->cVideo.m_nBpp) {
    case 16:
        bResult = Blt8To16(pSurface,
            lPitch,
            pt,
            rClip,
            dwFlags,
            ptSource);
        break;
    case 24:
        bResult = Blt8To24(reinterpret_cast<BYTE*>(pSurface),
            lPitch,
            pt,
            rClip,
            dwFlags,
            ptSource);
        break;
    case 32:
        bResult = Blt8To32(reinterpret_cast<DWORD*>(pSurface),
            lPitch,
            pt,
            rClip,
            dwFlags,
            ptSource,
            0);
        break;
    default:
        bResult = FALSE;
        break;
    }

    m_pFrame = NULL;

    if (!bDemanded) {
        pRes->Release();
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 2323
    UTIL_ASSERT(bResult);

    return bResult;
}

// NOTE: Inlined in the 16 bpp RLE blitters. Continues a transparent run which
// extends past the end of the current row. Returns `FALSE` when the last row
// is done, otherwise sets `nRemaining` to the pixels left in the row where the
// run ends.
static BOOL SkipTransparentRows16(WORD*& pSurface, LONG nJump, int& nHeight, int nWidth, int nSkip, int& nRemaining)
{
    for (;;) {
        pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nJump);

        nHeight--;
        if (nHeight == 0) {
            return FALSE;
        }

        if (nSkip < nWidth) {
            nRemaining = nWidth - nSkip;
            return TRUE;
        }

        nSkip -= nWidth;
    }
}

// 0x7AF1C0
BOOL CVidCell::Blt8To16(WORD* pSurface, LONG lPitch, DWORD dwFlags)
{
    int nHeight = m_pFrame->nHeight;
    int nWidth = m_pFrame->nWidth;

    if (nHeight == 0 || nWidth == 0) {
        return TRUE;
    }

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 16, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        *reinterpret_cast<WORD*>(&CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY]) = static_cast<WORD>(g_pChitin->GetCurrentVideoMode()->field_24);
    }

    BYTE* pData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 2396
    UTIL_ASSERT(pData != NULL);

    LONG nJump = lPitch - 2 * nWidth;

    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        int nRemaining = nWidth;
        for (;;) {
            BYTE nColor = *pData;
            if (nColor == nTransparentColor) {
                int nSkip = pData[1];
                pData += 2;
                pSurface += nSkip + 1;
                nRemaining--;

                if (nRemaining != 0 && nSkip < nRemaining) {
                    nRemaining -= nSkip;
                } else {
                    if (nRemaining != 0) {
                        nSkip -= nRemaining;
                    }

                    if (!SkipTransparentRows16(pSurface, nJump, nHeight, nWidth, nSkip, nRemaining)) {
                        return TRUE;
                    }
                }
            } else {
                *pSurface++ = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                pData++;
                nRemaining--;

                // NOTE: The original ends the row with `JA` after the
                // comparison against the transparent color, so a color index
                // below the transparent index also ends the row (without
                // consuming the rest of the row data).
                if (nColor < nTransparentColor || nRemaining == 0) {
                    pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nJump);

                    nHeight--;
                    if (nHeight == 0) {
                        return TRUE;
                    }

                    nRemaining = nWidth;
                }
            }
        }
    } else {
        do {
            int nRemaining = nWidth;
            do {
                // NOTE: The original reads one byte ahead of the current
                // pixel (`MOV AL, [ESI + 1]`), unlike the 24/32 bpp versions.
                BYTE nColor = pData[1];
                if (nColor != nTransparentColor) {
                    *pSurface = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                }
                pData++;
                pSurface++;
                nRemaining--;
            } while (nRemaining != 0);

            pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nJump);
            nHeight--;
        } while (nHeight != 0);
    }

    return TRUE;
}

// 0x7AF3C0
BOOL CVidCell::Blt8To16(WORD* pSurface, LONG lPitch, const CPoint& pt, const CRect& rClip, DWORD dwFlags, const CPoint& ptSource)
{
    int nWidth = m_pFrame->nWidth;
    int nBltWidth = nWidth - ptSource.x;
    int nBltHeight = m_pFrame->nHeight - ptSource.y;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 2624
    UTIL_ASSERT(ptSource.x >= 0 && ptSource.y >= 0);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 2625
    UTIL_ASSERT(rClip.top >= 0 && rClip.top <= rClip.bottom && rClip.left >= 0 && rClip.left <= rClip.right);

    if (pt.y > rClip.bottom
        || pt.x > rClip.right
        || pt.y + nBltHeight < rClip.top
        || pt.x + nBltWidth < rClip.left) {
        return TRUE;
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    lPitch >>= 1;

    WORD* pDest = pSurface + lPitch * pt.y + pt.x;

    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        int nRunLength = 0;
        int pos = 0;

        // Skip src top.
        for (int nSkipSrcH = ptSource.y; nSkipSrcH > 0; nSkipSrcH--) {
            int nRemainingWidth = nWidth;
            while (nRemainingWidth > 0) {
                if (pFrameData[pos] == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        nRunLength = 0;
                        pos += 2;
                        break;
                    }

                    if (nRemainingWidth < nRunLength) {
                        nRunLength -= nRemainingWidth;
                        break;
                    }

                    pos += 2;
                    nRemainingWidth -= nRunLength;
                    nRunLength = 0;
                } else {
                    pos++;
                    nRemainingWidth--;
                }
            }
        }

        // Skip dest top.
        int nSkipDestH = pt.y < rClip.top ? rClip.top - pt.y : 0;
        int nDestJump = lPitch - nWidth;
        if (pt.y < rClip.top) {
            for (int nRows = nSkipDestH; nRows > 0; nRows--) {
                int nRemainingWidth = nWidth;
                while (nRemainingWidth > 0) {
                    if (pFrameData[pos] == nTransparentColor) {
                        if (nRunLength == 0) {
                            nRunLength = pFrameData[pos + 1] + 1;
                        }

                        if (nRemainingWidth == nRunLength) {
                            nRunLength = 0;
                            pDest += nRemainingWidth;
                            pos += 2;
                            break;
                        }

                        if (nRemainingWidth < nRunLength) {
                            pDest += nRemainingWidth;
                            nRunLength -= nRemainingWidth;
                            break;
                        }

                        pos += 2;
                        nRemainingWidth -= nRunLength;
                        pDest += nRunLength;
                        nRunLength = 0;
                    } else {
                        pDest++;
                        pos++;
                        nRemainingWidth--;
                    }
                }
                pDest += nDestJump;
            }
        }

        // NOTE: The original computes the left clip with 16-bit arithmetic.
        SHORT nClipLeft = static_cast<SHORT>(rClip.left - pt.x);
        int nSkipDestW = nClipLeft < 0 ? 0 : nClipLeft;

        // Pixels are drawn while the count of pixels left in the row is in
        // `(nBltLeft, nBltRight]`.
        int nBltRight = nBltWidth - nSkipDestW;
        if (nBltRight == 0) {
            return TRUE;
        }

        int nBltLeft = nBltWidth - min(nBltWidth, rClip.right - rClip.left);
        int nBltH = min(rClip.bottom - pt.y, max(nBltHeight - nSkipDestH, 0));
        int nRowJump = lPitch - nBltWidth;

        if (nBltH <= 0) {
            return TRUE;
        }

        do {
            // Skip src left.
            int nSkipSrcW = ptSource.x;
            while (nSkipSrcW > 0) {
                if (pFrameData[pos] == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nSkipSrcW == nRunLength) {
                        nRunLength = 0;
                        pos += 2;
                        break;
                    }

                    if (nSkipSrcW < nRunLength) {
                        nRunLength -= nSkipSrcW;
                        break;
                    }

                    pos += 2;
                    nSkipSrcW -= nRunLength;
                    nRunLength = 0;
                } else {
                    pos++;
                    nSkipSrcW--;
                }
            }

            // Blt.
            int nRemainingWidth = nBltWidth;
            while (nRemainingWidth > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        nRunLength = 0;
                        pDest += nRemainingWidth;
                        pos += 2;
                        break;
                    }

                    if (nRemainingWidth < nRunLength) {
                        pDest += nRemainingWidth;
                        nRunLength -= nRemainingWidth;
                        break;
                    }

                    pos += 2;
                    nRemainingWidth -= nRunLength;
                    pDest += nRunLength;
                    nRunLength = 0;
                } else {
                    if (nRemainingWidth <= nBltRight && nRemainingWidth > nBltLeft) {
                        *pDest = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                    }

                    pDest++;
                    pos++;
                    nRemainingWidth--;
                }
            }

            pDest += nRowJump;
            nBltH--;
        } while (nBltH != 0);
    } else {
        int nSrcJump = ptSource.x;
        int pos = nWidth * ptSource.y + ptSource.x;
        int nBltH = nBltHeight;
        int nBltW = nBltWidth;

        if (pt.y < rClip.top) {
            pDest += lPitch * (rClip.top - pt.y);
            pos += nWidth * (rClip.top - pt.y);
            nBltH = pt.y - rClip.top + nBltHeight;
        }

        if (pt.y + nBltHeight > rClip.bottom) {
            nBltH += rClip.bottom - pt.y - nBltHeight;
        }

        if (pt.x < rClip.left) {
            nSrcJump += rClip.left - pt.x;
            nBltW = pt.x - rClip.left + nBltWidth;
            pos += rClip.left - pt.x;
            pDest += rClip.left - pt.x;
        }

        if (pt.x + nBltWidth > rClip.right) {
            nSrcJump += pt.x + nBltWidth - rClip.right;
            nBltW += rClip.right - pt.x - nBltWidth;
        }

        int nDestJump = lPitch - nBltW;

        if (nBltH <= 0) {
            return TRUE;
        }

        do {
            for (int x = nBltW; x > 0; x--) {
                BYTE nColor = pFrameData[pos];
                if (nColor != nTransparentColor) {
                    *pDest = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                }
                pDest++;
                pos++;
            }

            pDest += nDestJump;
            pos += nSrcJump;
            nBltH--;
        } while (nBltH != 0);
    }

    return TRUE;
}

// 0x7AF8A0
BOOL CVidCell::Blt8To16ClearShadow(WORD* pSurface, LONG lPitch, DWORD dwFlags)
{
    int nWidth = m_pFrame->nWidth;
    int nHeight = m_pFrame->nHeight;

    // FIXME: Only set when `m_bShadowOn` is set, but the compressed branch
    // for `!m_bShadowOn` uses it uninitialized.
    WORD nShadowMask = 0;

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 16, dwFlags, &m_paletteAffects, 255);

    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();
    if (!m_bShadowOn) {
        *reinterpret_cast<WORD*>(&CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY]) = static_cast<WORD>(pVidMode->field_24);
    } else {
        nShadowMask = static_cast<WORD>(((pVidMode->m_dwRBitMask >> 1) & pVidMode->m_dwRBitMask)
            + ((pVidMode->m_dwGBitMask >> 1) & pVidMode->m_dwGBitMask)
            + ((pVidMode->m_dwBBitMask >> 1) & pVidMode->m_dwBBitMask));
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;
    BOOLEAN bShadowOn = m_bShadowOn != FALSE;

    LONG nJump = lPitch - 2 * nWidth;

    // NOTE: There is no check for an empty frame.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        int nRemaining = nWidth;
        for (;;) {
            BYTE nColor = *pFrameData;
            if (nColor == nTransparentColor) {
                int nSkip = pFrameData[1];
                pFrameData += 2;
                pSurface += nSkip + 1;
                nRemaining--;

                if (nRemaining != 0 && nSkip < nRemaining) {
                    nRemaining -= nSkip;
                } else {
                    if (nRemaining != 0) {
                        nSkip -= nRemaining;
                    }

                    if (!SkipTransparentRows16(pSurface, nJump, nHeight, nWidth, nSkip, nRemaining)) {
                        return TRUE;
                    }
                }
            } else {
                if (bShadowOn) {
                    if (nColor == 1) {
                        *pSurface = static_cast<WORD>((*pSurface >> 1) & nShadowMask);
                    } else {
                        *pSurface = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                    }
                } else {
                    // NOTE: Halves every pixel, unlike the uncompressed
                    // branch below.
                    *pSurface = static_cast<WORD>((static_cast<WORD>(CVidImage::rgbTempPal[nColor]) >> 1) & nShadowMask);
                }

                pFrameData++;
                pSurface++;
                nRemaining--;

                if (nRemaining == 0) {
                    pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nJump);

                    nHeight--;
                    if (nHeight == 0) {
                        return TRUE;
                    }

                    nRemaining = nWidth;
                }
            }
        }
    } else {
        do {
            int nRemaining = nWidth;
            do {
                BYTE nColor = *pFrameData;
                if (nColor != nTransparentColor) {
                    if (bShadowOn && nColor <= 1) {
                        *pSurface = static_cast<WORD>((*pSurface >> 1) & nShadowMask);
                    } else {
                        *pSurface = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                    }
                }
                pFrameData++;
                pSurface++;
                nRemaining--;
            } while (nRemaining != 0);

            pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nJump);
            nHeight--;
        } while (nHeight != 0);
    }

    return TRUE;
}

// NOTE: Inlined in 0x7AFB50. Keeps the brightest of each color component.
static WORD BrightestPixel16(WORD nSrc, WORD nDest, DWORD dwRBitMask, DWORD dwGBitMask, DWORD dwBBitMask)
{
    WORD nResult = 0;

    if ((nSrc & dwRBitMask) > (nDest & dwRBitMask)) {
        nResult |= static_cast<WORD>(nSrc & dwRBitMask);
    } else {
        nResult |= static_cast<WORD>(nDest & dwRBitMask);
    }

    if ((nSrc & dwGBitMask) > (nDest & dwGBitMask)) {
        nResult |= static_cast<WORD>(nSrc & dwGBitMask);
    } else {
        nResult |= static_cast<WORD>(nDest & dwGBitMask);
    }

    if ((nSrc & dwBBitMask) > (nDest & dwBBitMask)) {
        nResult |= static_cast<WORD>(nSrc & dwBBitMask);
    } else {
        nResult |= static_cast<WORD>(nDest & dwBBitMask);
    }

    return nResult;
}

// 0x7AFB50
BOOL CVidCell::Blt8To16Brightest(WORD* pSurface, LONG lPitch, DWORD dwFlags)
{
    int nHeight = m_pFrame->nHeight;
    int nWidth = m_pFrame->nWidth;
    DWORD dwRBitMask = g_pChitin->GetCurrentVideoMode()->m_dwRBitMask;
    DWORD dwGBitMask = g_pChitin->GetCurrentVideoMode()->m_dwGBitMask;
    DWORD dwBBitMask = g_pChitin->GetCurrentVideoMode()->m_dwBBitMask;
    LONG nJump = lPitch - 2 * nWidth;

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 16, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        *reinterpret_cast<WORD*>(&CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY]) = static_cast<WORD>(g_pChitin->GetCurrentVideoMode()->field_24);
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    // NOTE: There is no check for an empty frame.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        // NOTE: The original processes opaque pixels in pairs.
        int nRemaining = nWidth;
        for (;;) {
            BYTE nColor = *pFrameData;
            if (nColor == nTransparentColor) {
                int nSkip = pFrameData[1];
                pFrameData += 2;
                pSurface += nSkip + 1;
                nRemaining--;

                if (nRemaining != 0 && nSkip < nRemaining) {
                    nRemaining -= nSkip;
                } else {
                    if (nRemaining != 0) {
                        nSkip -= nRemaining;
                    }

                    if (!SkipTransparentRows16(pSurface, nJump, nHeight, nWidth, nSkip, nRemaining)) {
                        return TRUE;
                    }
                }
            } else {
                WORD nFirst = BrightestPixel16(static_cast<WORD>(CVidImage::rgbTempPal[nColor]),
                    pSurface[0],
                    dwRBitMask,
                    dwGBitMask,
                    dwBBitMask);

                // NOTE: The original only compares the low byte of the
                // remaining count (`CMP CL, 1`).
                if (static_cast<BYTE>(nRemaining) == 1) {
                    *pSurface++ = nFirst;
                    pFrameData++;

                    pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nJump);

                    nHeight--;
                    if (nHeight == 0) {
                        return TRUE;
                    }

                    nRemaining = nWidth;
                    continue;
                }

                BYTE nSecondColor = pFrameData[1];
                if (nSecondColor == nTransparentColor) {
                    *pSurface = nFirst;

                    int nSkip = pFrameData[2];
                    pFrameData += 3;
                    pSurface += nSkip + 2;

                    // NOTE: The original only checks the low byte of the
                    // remaining count (`SUB CL, 2` / `JBE`).
                    if (static_cast<BYTE>(nRemaining) > 2) {
                        nRemaining -= 2;
                        if (nSkip < nRemaining) {
                            nRemaining -= nSkip;
                            continue;
                        }

                        nSkip -= nRemaining;
                    }

                    if (!SkipTransparentRows16(pSurface, nJump, nHeight, nWidth, nSkip, nRemaining)) {
                        return TRUE;
                    }
                } else {
                    WORD nSecond = BrightestPixel16(static_cast<WORD>(CVidImage::rgbTempPal[nSecondColor]),
                        pSurface[1],
                        dwRBitMask,
                        dwGBitMask,
                        dwBBitMask);

                    pFrameData += 2;
                    pSurface[0] = nFirst;
                    pSurface[1] = nSecond;
                    pSurface += 2;
                    nRemaining -= 2;

                    if (nRemaining <= 0) {
                        pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nJump);

                        nHeight--;
                        if (nHeight == 0) {
                            return TRUE;
                        }

                        nRemaining = nWidth;
                    }
                }
            }
        }
    } else {
        do {
            int nRemaining = nWidth;
            do {
                BYTE nColor = *pFrameData;
                if (nColor != nTransparentColor) {
                    *pSurface = BrightestPixel16(static_cast<WORD>(CVidImage::rgbTempPal[nColor]),
                        *pSurface,
                        dwRBitMask,
                        dwGBitMask,
                        dwBBitMask);
                }
                pFrameData++;
                pSurface++;
                nRemaining--;
            } while (nRemaining != 0);

            pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + nJump);
            nHeight--;
        } while (nHeight != 0);
    }

    return TRUE;
}

// 0x7AFED0
BOOL CVidCell::Blt8To16Translucent(WORD* pSurface, LONG lPitch, DWORD dwFlags, INT nTransVal)
{
    int nWidth = m_pFrame->nWidth;
    int nHeight = m_pFrame->nHeight;
    DWORD dwColorKey = g_pChitin->GetCurrentVideoMode()->field_24;
    DWORD dwRBitMask = g_pChitin->GetCurrentVideoMode()->m_dwRBitMask;
    DWORD dwGBitMask = g_pChitin->GetCurrentVideoMode()->m_dwGBitMask;
    DWORD dwBBitMask = g_pChitin->GetCurrentVideoMode()->m_dwBBitMask;
    BYTE nRBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwRBitShift);
    BYTE nGBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwGBitShift);
    BYTE nBBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwBBitShift);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 4221
    UTIL_ASSERT(nTransVal <= 256);

    if (CVidImage::m_pTranslucentTable == NULL) {
        CVidImage::m_pTranslucentTable = new BYTE[0x20000];
        if (CVidImage::m_pTranslucentTable == NULL) {
            return FALSE;
        }

        // Indexed by `(dest << 11) | (src << 5) | level`, each component
        // being 6 bits and the level 5 bits.
        for (int nLevel = 0; nLevel < 32; nLevel++) {
            for (int nSrc = 0; nSrc < 64; nSrc++) {
                for (int nDest = 0; nDest < 64; nDest++) {
                    CVidImage::m_pTranslucentTable[nLevel + 32 * nSrc + 0x800 * nDest] = static_cast<BYTE>((nSrc * (64 - 2 * nLevel) + nDest * 2 * nLevel) >> 6);
                }
            }
        }
    }

    // Reduce 8-bit `nTransVal` to the 5-bit level of the table.
    int nShift = 8;
    do {
        nShift >>= 1;
        nTransVal >>= 1;
    } while (nShift > 1);

    LONG nJump = (lPitch - 2 * nWidth) >> 1;

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 16, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        *reinterpret_cast<WORD*>(&CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY]) = static_cast<WORD>(dwColorKey);
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    BYTE* pTable = CVidImage::m_pTranslucentTable + nTransVal;

    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        int nRunLength = 0;
        int pos = 0;
        for (int y = 0; y < nHeight; y++) {
            int nRemainingWidth = nWidth;
            while (nRemainingWidth > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        pos += 2;
                        pSurface += nRemainingWidth;
                        nRunLength = 0;
                        break;
                    }

                    if (nRemainingWidth < nRunLength) {
                        nRunLength -= nRemainingWidth;
                        pSurface += nRemainingWidth;
                        break;
                    }

                    nRemainingWidth -= nRunLength;
                    pos += 2;
                    pSurface += nRunLength;
                    nRunLength = 0;
                } else {
                    DWORD nSrc = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                    if (nSrc != (dwColorKey & 0xFFFF)) {
                        DWORD nDest = *pSurface;
                        *pSurface = static_cast<WORD>((pTable[(((((nDest & dwRBitMask) >> nRBitShift) << 6) + ((nSrc & dwRBitMask) >> nRBitShift)) << 5)] << nRBitShift)
                            | (pTable[(((((nDest & dwGBitMask) >> nGBitShift) << 6) + ((nSrc & dwGBitMask) >> nGBitShift)) << 5)] << nGBitShift)
                            | (pTable[(((((nDest & dwBBitMask) >> nBBitShift) << 6) + ((nSrc & dwBBitMask) >> nBBitShift)) << 5)] << nBBitShift));
                    }

                    pSurface++;
                    pos++;
                    nRemainingWidth--;
                }
            }
            pSurface += nJump;
        }
    } else {
        int pos = 0;

        // NOTE: The original renders one row more than the frame height.
        for (int y = 0; y <= nHeight; y++) {
            for (int x = 0; x < nWidth; x++) {
                // NOTE: Only the color key is checked, not the transparent
                // color index.
                DWORD nSrc = static_cast<WORD>(CVidImage::rgbTempPal[pFrameData[pos]]);
                if (nSrc != (dwColorKey & 0xFFFF)) {
                    DWORD nDest = *pSurface;
                    *pSurface = static_cast<WORD>((pTable[(((((nDest & dwRBitMask) >> nRBitShift) << 6) + ((nSrc & dwRBitMask) >> nRBitShift)) << 5)] << nRBitShift)
                        | (pTable[(((((nDest & dwGBitMask) >> nGBitShift) << 6) + ((nSrc & dwGBitMask) >> nGBitShift)) << 5)] << nGBitShift)
                        | (pTable[(((((nDest & dwBBitMask) >> nBBitShift) << 6) + ((nSrc & dwBBitMask) >> nBBitShift)) << 5)] << nBBitShift));
                }

                pSurface++;
                pos++;
            }
            pSurface += nJump;
        }
    }

    return TRUE;
}

// 0x7B0490
void CVidCell::BltBackGroundToFx(LPDIRECTDRAWSURFACE pSurface, CRect& rSrc, const CRect& rClip, DWORD dwFlags)
{
    HRESULT hr;

    int destX = 0;
    int destY = 0;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 4908
    UTIL_ASSERT(m_nFxSurface != -1);

    CRect rSrcSurface(rSrc);

    if (rSrcSurface.left < rClip.left) {
        rSrcSurface.left = rClip.left;
        destX = rClip.left - rSrc.left;
    }

    if (rSrcSurface.top < rClip.top) {
        rSrcSurface.top = rClip.top;
        destY = rClip.top - rSrc.top;
    }

    if (rSrcSurface.right >= rClip.right) {
        rSrcSurface.right = rClip.right;
    }

    if (rSrcSurface.bottom >= rClip.bottom) {
        rSrcSurface.bottom = rClip.bottom;
    }

    if ((dwFlags & 0x10) != 0 || (dwFlags & 0x20) != 0) {
        DDBLTFX fx;
        fx.dwSize = sizeof(fx);
        fx.dwDDFX = 0;
        if ((dwFlags & 0x10) != 0) {
            fx.dwDDFX |= DDBLTFX_MIRRORLEFTRIGHT;
        }
        if ((dwFlags & 0x20) != 0) {
            fx.dwDDFX |= DDBLTFX_MIRRORUPDOWN;
        }

        do {
            RECT rDest;
            rDest.left = destX;
            rDest.top = destY;
            rDest.right = destX + rSrc.Width();
            rDest.bottom = destY + rSrc.Height();

            hr = g_pChitin->cVideo.cVidBlitter.Blt(g_pChitin->GetCurrentVideoMode()->pSurfaces[m_nFxSurface],
                &rDest,
                pSurface,
                &rSrc,
                DDBLT_WAIT | DDBLT_DDFX,
                &fx);
        } while (hr == DDERR_SURFACELOST || hr == DDERR_WASSTILLDRAWING);
    } else {
        do {
            hr = g_pChitin->cVideo.cVidBlitter.BltFast(g_pChitin->GetCurrentVideoMode()->pSurfaces[m_nFxSurface],
                destX,
                destY,
                pSurface,
                rSrcSurface,
                DDBLTFAST_WAIT);
            if (hr != DDERR_SURFACELOST && hr != DDERR_WASSTILLDRAWING) {
                break;
            }
        } while (!g_pChitin->field_1932);
    }
}

// 0x7B06E0
BOOL CVidCell::BltFromFX(LPDIRECTDRAWSURFACE pSurface, int x, int y, const CRect& rClip, DWORD dwFlags)
{
    HRESULT hr;

    int destX = x;
    int destY = y;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 5003
    UTIL_ASSERT(m_nFxSurface != -1);

    CRect rSrcSurface;
    rSrcSurface.left = 0;
    rSrcSurface.top = 0;
    rSrcSurface.right = m_pFrame->nWidth;
    rSrcSurface.bottom = m_pFrame->nHeight;

    if (x < rClip.left) {
        destX = rClip.left;
        rSrcSurface.left = rClip.left - x;
    }

    if (y < rClip.top) {
        destY = rClip.top;
        rSrcSurface.top = rClip.top - y;
    }

    if (x + m_pFrame->nWidth - 1 >= rClip.right) {
        rSrcSurface.right -= x + m_pFrame->nWidth - rClip.right;
    }

    if (y + m_pFrame->nHeight - 1 >= rClip.bottom) {
        rSrcSurface.bottom -= y + m_pFrame->nHeight - rClip.bottom;
    }

    if (rSrcSurface.bottom > rSrcSurface.top && rSrcSurface.right > rSrcSurface.left) {
        if ((dwFlags & 0x10) != 0 || (dwFlags & 0x20) != 0) {
            DDBLTFX fx;
            fx.dwSize = sizeof(fx);
            fx.dwDDFX = 0;
            if ((dwFlags & 0x10) != 0) {
                fx.dwDDFX |= DDBLTFX_MIRRORLEFTRIGHT;
            }
            if ((dwFlags & 0x20) != 0) {
                fx.dwDDFX |= DDBLTFX_MIRRORUPDOWN;
            }

            do {
                RECT rDest;
                rDest.left = destX;
                rDest.top = destY;
                rDest.right = rSrcSurface.right - rSrcSurface.left;
                rDest.bottom = rSrcSurface.bottom - rSrcSurface.top;

                hr = g_pChitin->cVideo.cVidBlitter.Blt(pSurface,
                    &rDest,
                    g_pChitin->GetCurrentVideoMode()->pSurfaces[m_nFxSurface],
                    &rSrcSurface,
                    DDBLT_WAIT | DDBLT_DDFX,
                    &fx);
                g_pChitin->GetCurrentVideoMode()->CheckResults(hr);
            } while (hr == DDERR_SURFACELOST || hr == DDERR_WASSTILLDRAWING);
        } else {
            do {
                hr = g_pChitin->cVideo.cVidBlitter.BltFast(pSurface,
                    destX,
                    destY,
                    g_pChitin->GetCurrentVideoMode()->pSurfaces[m_nFxSurface],
                    &rSrcSurface,
                    DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
                g_pChitin->GetCurrentVideoMode()->CheckResults(hr);
                if (hr != DDERR_SURFACELOST && hr != DDERR_WASSTILLDRAWING) {
                    break;
                }
            } while (!g_pChitin->field_1932);
        }
    }

    return TRUE;
}

// 0x7B0980
BOOL CVidCell::GetFrame(BOOLEAN bDemanded)
{
    if (!bDemanded) {
        if (pRes->Demand() == NULL) {
            return FALSE;
        }
    }

    BAMHEADER* pHeader;
    if (pRes->m_bCacheHeader) {
        pHeader = pRes->m_pBamHeaderCopy;
    } else {
        pHeader = pRes->m_pBamHeader;
    }

    if (!(m_nCurrentSequence < pHeader->nSequences)) {
        m_nCurrentSequence = 0;
    }

    if (!(m_nCurrentFrame < pRes->m_pSequences[m_nCurrentSequence].nFrames)) {
        if (field_C8) {
            if (pRes->m_pSequences[m_nCurrentSequence].nFrames > 0) {
                m_nCurrentFrame = m_nCurrentFrame % pRes->m_pSequences[m_nCurrentSequence].nFrames;
            } else {
                m_nCurrentFrame = 0;
            }
        } else {
            m_nCurrentFrame = pRes->m_pSequences[m_nCurrentSequence].nFrames - 1;
        }
    }

    if (m_nCurrentFrame < 0) {
        if (field_C8) {
            if (pRes->m_pSequences[m_nCurrentSequence].nFrames > 0) {
                m_nCurrentFrame = pRes->m_pSequences[m_nCurrentSequence].nFrames + m_nCurrentFrame % pRes->m_pSequences[m_nCurrentSequence].nFrames;
                if (m_nCurrentFrame == pRes->m_pSequences[m_nCurrentSequence].nFrames) {
                    m_nCurrentFrame = 0;
                }
            } else {
                m_nCurrentFrame = 0;
            }
        } else {
            m_nCurrentFrame = 0;
        }
    }

    m_pFrame = pRes->GetFrame(m_nCurrentSequence, m_nCurrentFrame, m_bDoubleSize);

    return m_pFrame != NULL;
}

// 0x7C4120
void CVidCell::StoreBackground(INT x, INT y, const CRect& rClip, CRect& rStorage, BOOLEAN bNumbered)
{
    if (GetFrame(FALSE)) {
        rStorage.left = x - m_pFrame->nCenterX;
        rStorage.top = y - m_pFrame->nCenterY;

        if (bNumbered) {
            rStorage.right = max(rStorage.left + m_pFrame->nWidth, rStorage.left + m_pFrame->nCenterX + 16);
            rStorage.bottom = max(rStorage.top + m_pFrame->nHeight, rStorage.top + m_pFrame->nCenterY + 16);
        } else {
            rStorage.right = rStorage.left + m_pFrame->nWidth;
            rStorage.bottom = rStorage.top + m_pFrame->nHeight;
        }

        // NOTE: Looks like intersection.
        if (rStorage.left < rClip.left) {
            rStorage.left = rClip.left;
        }

        if (rStorage.top < rClip.top) {
            rStorage.top = rClip.top;
        }

        if (rStorage.right >= rClip.right) {
            rStorage.right = rClip.right;
        }

        if (rStorage.bottom >= rClip.bottom) {
            rStorage.bottom = rClip.bottom;
        }
    } else {
        rStorage.SetRect(0, 0, 0, 0);
    }
}

// 0x7CECD0
BOOL CVidCell::Blt8To24(BYTE* pSurface, LONG lPitch, DWORD dwFlags)
{
    UINT nWidth = m_pFrame->nWidth;
    UINT nHeight = m_pFrame->nHeight;

    if (nHeight == 0 || nWidth == 0) {
        return TRUE;
    }

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 24, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;
    LONG nPitchAdjust = lPitch - 3 * nWidth;
    DWORD* pPalette = CVidImage::rgbTempPal;

    // NOTE: Original code is inline assembly.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        UINT nRemainingWidth = nWidth;
        while (TRUE) {
            BYTE nColor = *pFrameData;
            if (nColor == nTransparentColor) {
                UINT nSkip = pFrameData[1];
                pFrameData += 2;
                pSurface += 3 * nSkip + 3;
                nRemainingWidth--;
                if (nSkip < nRemainingWidth) {
                    nRemainingWidth -= nSkip;
                } else {
                    // The run continues on the following row(s).
                    nSkip -= nRemainingWidth;
                    while (TRUE) {
                        pSurface += nPitchAdjust;
                        if (--nHeight == 0) {
                            return TRUE;
                        }

                        if (nSkip < nWidth) {
                            break;
                        }

                        nSkip -= nWidth;
                    }
                    nRemainingWidth = nWidth - nSkip;
                }
            } else {
                DWORD rgb = pPalette[nColor];
                pFrameData++;
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(rgb);
                pSurface[2] = static_cast<BYTE>(rgb >> 16);
                pSurface += 3;
                if (--nRemainingWidth == 0) {
                    pSurface += nPitchAdjust;
                    if (--nHeight == 0) {
                        return TRUE;
                    }
                    nRemainingWidth = nWidth;
                }
            }
        }
    } else {
        do {
            UINT nRemainingWidth = nWidth;
            do {
                BYTE nColor = *pFrameData;
                if (nColor != nTransparentColor) {
                    DWORD rgb = pPalette[nColor];
                    *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(rgb);
                    pSurface[2] = static_cast<BYTE>(rgb >> 16);
                }
                pFrameData++;
                pSurface += 3;
            } while (--nRemainingWidth != 0);
            pSurface += nPitchAdjust;
        } while (--nHeight != 0);
    }

    return TRUE;
}

// 0x7CEEB0
BOOL CVidCell::Blt8To24(BYTE* pSurface, LONG lPitch, const CPoint& pt, const CRect& rClip, DWORD dwFlags, const CPoint& ptSource)
{
    INT nWidth = m_pFrame->nWidth;
    INT nBltWidth = m_pFrame->nWidth - ptSource.x;
    INT nBltHeight = m_pFrame->nHeight - ptSource.y;
    INT pos = 0;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage24.cpp
    // __LINE__: 3073
    UTIL_ASSERT(ptSource.x >= 0 && ptSource.y >= 0);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage24.cpp
    // __LINE__: 3074
    UTIL_ASSERT(rClip.top >= 0 && rClip.top <= rClip.bottom && rClip.left >= 0 && rClip.left <= rClip.right);

    if (pt.y > rClip.bottom
        || pt.x > rClip.right
        || pt.y + nBltHeight < rClip.top
        || pt.x + nBltWidth < rClip.left) {
        return TRUE;
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    INT nTransparentColor = pBamHeader->nTransparentColor;

    BYTE* pDest = pSurface + 3 * pt.x + lPitch * pt.y;

    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        // NOTE: Run lengths and counters below are in bytes (3 per pixel).
        INT nRunLength = 0;

        // Skip src top.
        for (INT nSkipSrcH = ptSource.y; nSkipSrcH > 0; nSkipSrcH--) {
            INT nRemaining = 3 * nWidth;
            while (nRemaining > 0) {
                if (pFrameData[pos] == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = 3 * (pFrameData[pos + 1] + 1);
                    }

                    if (nRemaining == nRunLength) {
                        nRunLength = 0;
                        pos += 2;
                        break;
                    }

                    if (nRemaining < nRunLength) {
                        nRunLength -= nRemaining;
                        break;
                    }

                    pos += 2;
                    nRemaining -= nRunLength;
                    nRunLength = 0;
                } else {
                    pos++;
                    nRemaining -= 3;
                }
            }
        }

        // Skip dest top.
        INT nSkipDestH = pt.y < rClip.top ? rClip.top - pt.y : 0;
        LONG nRowAdjust = lPitch - 3 * nWidth;
        if (pt.y < rClip.top) {
            for (INT nRows = nSkipDestH; nRows > 0; nRows--) {
                INT nRemaining = 3 * nWidth;
                while (nRemaining > 0) {
                    if (pFrameData[pos] == nTransparentColor) {
                        if (nRunLength == 0) {
                            nRunLength = 3 * (pFrameData[pos + 1] + 1);
                        }

                        if (nRemaining == nRunLength) {
                            pDest += nRemaining;
                            nRunLength = 0;
                            pos += 2;
                            break;
                        }

                        if (nRemaining < nRunLength) {
                            pDest += nRemaining;
                            nRunLength -= nRemaining;
                            break;
                        }

                        pos += 2;
                        nRemaining -= nRunLength;
                        pDest += nRunLength;
                        nRunLength = 0;
                    } else {
                        pDest += 3;
                        pos++;
                        nRemaining -= 3;
                    }
                }
                pDest += nRowAdjust;
            }
        }

        // NOTE: The original computes this difference in 16 bits.
        SHORT nLeftDelta = static_cast<SHORT>(rClip.left - pt.x);
        INT nSkipDestW = nLeftDelta < 0 ? 0 : nLeftDelta;

        INT nVisible = 3 * (nBltWidth - nSkipDestW);
        if (nVisible == 0) {
            return TRUE;
        }

        INT nClipRight = 3 * (nBltWidth - min(nBltWidth, rClip.right - pt.x));
        INT nBltH = min(rClip.bottom - pt.y, max(nBltHeight - nSkipDestH, 0));
        LONG nPitchAdjust = lPitch - 3 * nBltWidth;

        for (; nBltH > 0; nBltH--) {
            // Skip src left.
            INT nRemaining = 3 * ptSource.x;
            while (nRemaining > 0) {
                if (pFrameData[pos] == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = 3 * (pFrameData[pos + 1] + 1);
                    }

                    if (nRemaining == nRunLength) {
                        nRunLength = 0;
                        pos += 2;
                        break;
                    }

                    if (nRemaining < nRunLength) {
                        nRunLength -= nRemaining;
                        break;
                    }

                    pos += 2;
                    nRemaining -= nRunLength;
                    nRunLength = 0;
                } else {
                    pos++;
                    nRemaining -= 3;
                }
            }

            // Blt (left/right clipping is done per pixel).
            nRemaining = 3 * nBltWidth;
            while (nRemaining > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = 3 * (pFrameData[pos + 1] + 1);
                    }

                    if (nRemaining == nRunLength) {
                        pDest += nRemaining;
                        nRunLength = 0;
                        pos += 2;
                        break;
                    }

                    if (nRemaining < nRunLength) {
                        pDest += nRemaining;
                        nRunLength -= nRemaining;
                        break;
                    }

                    pos += 2;
                    nRemaining -= nRunLength;
                    pDest += nRunLength;
                    nRunLength = 0;
                } else {
                    if (nRemaining <= nVisible && nRemaining > nClipRight) {
                        *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                        pDest[2] = static_cast<BYTE>(CVidImage::rgbTempPal[pFrameData[pos]] >> 16);
                    }
                    pDest += 3;
                    pos++;
                    nRemaining -= 3;
                }
            }
            pDest += nPitchAdjust;
        }
    } else {
        INT nSrcJump = ptSource.x;
        INT nBltW = nBltWidth;
        INT nBltH = nBltHeight;

        pos = nWidth * ptSource.y + ptSource.x;

        if (pt.y < rClip.top) {
            pos += nWidth * (rClip.top - pt.y);
            pDest += lPitch * (rClip.top - pt.y);
            nBltH = pt.y - rClip.top + nBltHeight;
        }

        if (pt.y + nBltHeight > rClip.bottom) {
            nBltH += rClip.bottom - pt.y - nBltHeight;
        }

        if (pt.x < rClip.left) {
            nSrcJump += rClip.left - pt.x;
            nBltW = pt.x - rClip.left + nBltWidth;
            pDest += 3 * (rClip.left - pt.x);
            pos += rClip.left - pt.x;
        }

        if (pt.x + nBltWidth > rClip.right) {
            nSrcJump += pt.x - rClip.right + nBltWidth;
            nBltW += rClip.right - pt.x - nBltWidth;
        }

        LONG nPitchAdjust = lPitch - 3 * nBltW;

        for (; nBltH > 0; nBltH--) {
            for (INT x = nBltW; x > 0; x--) {
                BYTE nColor = pFrameData[pos];
                if (nColor != nTransparentColor) {
                    *reinterpret_cast<WORD*>(pDest) = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                    pDest[2] = static_cast<BYTE>(CVidImage::rgbTempPal[pFrameData[pos]] >> 16);
                }
                pDest += 3;
                pos++;
            }
            pDest += nPitchAdjust;
            pos += nSrcJump;
        }
    }

    return TRUE;
}

// 0x7CF3F0
BOOL CVidCell::Blt8To24ClearShadow(BYTE* pSurface, LONG lPitch, DWORD dwFlags)
{
    UINT nWidth = m_pFrame->nWidth;
    UINT nHeight = m_pFrame->nHeight;
    DWORD dwShadowMask;

    if (nHeight == 0 || nWidth == 0) {
        return TRUE;
    }

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 24, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    } else {
        CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();
        dwShadowMask = ((pVidMode->m_dwRBitMask >> 1) & pVidMode->m_dwRBitMask)
            + ((pVidMode->m_dwGBitMask >> 1) & pVidMode->m_dwGBitMask)
            + ((pVidMode->m_dwBBitMask >> 1) & pVidMode->m_dwBBitMask);
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    INT nTransparentColor = pBamHeader->nTransparentColor;
    DWORD* pPalette = CVidImage::rgbTempPal;
    BOOLEAN bShadowOn = m_bShadowOn != 0;
    LONG nPitchAdjust = lPitch - 3 * nWidth;

    // NOTE: Original code is inline assembly.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        UINT nRemainingWidth = nWidth;
        while (TRUE) {
            BYTE nColor = *pFrameData;
            if (nColor == nTransparentColor) {
                UINT nSkip = pFrameData[1];
                pFrameData += 2;
                pSurface += 3 * nSkip + 3;
                nRemainingWidth--;
                if (nSkip < nRemainingWidth) {
                    nRemainingWidth -= nSkip;
                } else {
                    // The run continues on the following row(s).
                    nSkip -= nRemainingWidth;
                    while (TRUE) {
                        pSurface += nPitchAdjust;
                        if (--nHeight == 0) {
                            return TRUE;
                        }

                        if (nSkip < nWidth) {
                            break;
                        }

                        nSkip -= nWidth;
                    }
                    nRemainingWidth = nWidth - nSkip;
                }
            } else {
                DWORD rgb;
                if (bShadowOn && nColor == 1) {
                    rgb = (((*reinterpret_cast<WORD*>(pSurface + 1)) << 8) | pSurface[0]) >> 1;
                    rgb &= dwShadowMask;
                } else {
                    rgb = pPalette[nColor];
                }
                pFrameData++;
                *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(rgb);
                pSurface[2] = static_cast<BYTE>(rgb >> 16);
                pSurface += 3;
                if (--nRemainingWidth == 0) {
                    pSurface += nPitchAdjust;
                    if (--nHeight == 0) {
                        return TRUE;
                    }
                    nRemainingWidth = nWidth;
                }
            }
        }
    } else {
        do {
            UINT nRemainingWidth = nWidth;
            do {
                BYTE nColor = *pFrameData;
                if (bShadowOn && nColor == 1) {
                    DWORD rgb = (((*reinterpret_cast<WORD*>(pSurface + 1)) << 8) | pSurface[0]) >> 1;
                    rgb &= dwShadowMask;
                    *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(rgb);
                    pSurface[2] = static_cast<BYTE>(rgb >> 16);
                } else if (nColor != 0) {
                    // NOTE: Uncompressed frames compare against palette index
                    // 0, not the BAM transparent color.
                    DWORD rgb = pPalette[nColor];
                    *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(rgb);
                    pSurface[2] = static_cast<BYTE>(rgb >> 16);
                }
                pFrameData++;
                pSurface += 3;
            } while (--nRemainingWidth != 0);
            pSurface += nPitchAdjust;
        } while (--nHeight != 0);
    }

    return TRUE;
}

// 0x7CF650
BOOL CVidCell::Blt8To24Brightest(BYTE* pSurface, LONG lPitch, DWORD dwFlags)
{
    UINT nWidth = m_pFrame->nWidth;
    UINT nHeight = m_pFrame->nHeight;

    if (nHeight == 0 || nWidth == 0) {
        return TRUE;
    }

    LONG nPitchAdjust = lPitch - 3 * nWidth;
    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 24, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;
    DWORD* pPalette = CVidImage::rgbTempPal;

    // NOTE: Original code is inline assembly.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        UINT nRemainingWidth = nWidth;
        while (TRUE) {
            BYTE nColor = *pFrameData;
            if (nColor == nTransparentColor) {
                UINT nSkip = pFrameData[1];
                pFrameData += 2;
                pSurface += 3 * nSkip + 3;
                nRemainingWidth--;
                if (nSkip < nRemainingWidth) {
                    nRemainingWidth -= nSkip;
                } else {
                    // The run continues on the following row(s).
                    nSkip -= nRemainingWidth;
                    while (TRUE) {
                        pSurface += nPitchAdjust;
                        if (--nHeight == 0) {
                            return TRUE;
                        }

                        if (nSkip < nWidth) {
                            break;
                        }

                        nSkip -= nWidth;
                    }
                    nRemainingWidth = nWidth - nSkip;
                }
            } else {
                DWORD rgb = pPalette[nColor];
                if (static_cast<BYTE>(rgb) >= pSurface[0]) {
                    pSurface[0] = static_cast<BYTE>(rgb);
                }
                if (static_cast<BYTE>(rgb >> 8) >= pSurface[1]) {
                    pSurface[1] = static_cast<BYTE>(rgb >> 8);
                }
                if (static_cast<BYTE>(rgb >> 16) >= pSurface[2]) {
                    pSurface[2] = static_cast<BYTE>(rgb >> 16);
                }
                pFrameData++;
                pSurface += 3;
                if (--nRemainingWidth == 0) {
                    pSurface += nPitchAdjust;
                    if (--nHeight == 0) {
                        return TRUE;
                    }
                    nRemainingWidth = nWidth;
                }
            }
        }
    } else {
        // NOTE: The original processes all but the last pixel of each row
        // with DWORD reads/writes and the last one with WORD + BYTE accesses
        // (to avoid touching the following pixel). The result is the same.
        INT pos = 0;
        for (INT y = 0; y < static_cast<INT>(nHeight); y++) {
            for (INT x = 0; x < static_cast<INT>(nWidth); x++) {
                BYTE nColor = pFrameData[pos];
                if (nColor != nTransparentColor) {
                    BYTE* pRgb = reinterpret_cast<BYTE*>(&pPalette[nColor]);
                    if (pRgb[0] > pSurface[0]) {
                        pSurface[0] = pRgb[0];
                    }
                    if (pRgb[1] > pSurface[1]) {
                        pSurface[1] = pRgb[1];
                    }
                    if (pRgb[2] > pSurface[2]) {
                        pSurface[2] = pRgb[2];
                    }
                }
                pSurface += 3;
                pos++;
            }
            pSurface += nPitchAdjust;
        }
    }

    return TRUE;
}

// 0x7CF8D0
BOOL CVidCell::Blt8To24Translucent(BYTE* pSurface, LONG lPitch, DWORD dwFlags, INT nTransVal)
{
    UINT nWidth = m_pFrame->nWidth;
    UINT nHeight = m_pFrame->nHeight;
    DWORD rgbShadow = g_pChitin->GetCurrentVideoMode()->field_24;
    INT nInvTransVal = 256 - nTransVal;

    if (nHeight == 0 || nWidth == 0) {
        return TRUE;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage24.cpp
    // __LINE__: 4516
    UTIL_ASSERT(nTransVal <= 256);

    LONG nPitchAdjust = lPitch - 3 * nWidth;
    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 24, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = rgbShadow;
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;
    DWORD* pPalette = CVidImage::rgbTempPal;

    // NOTE: Original code is inline assembly.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        UINT nRemainingWidth = nWidth;
        while (TRUE) {
            BYTE nColor = *pFrameData;
            if (nColor == nTransparentColor) {
                UINT nSkip = pFrameData[1];
                pFrameData += 2;
                pSurface += 3 * nSkip + 3;
                nRemainingWidth--;
                if (nSkip < nRemainingWidth) {
                    nRemainingWidth -= nSkip;
                } else {
                    // The run continues on the following row(s).
                    nSkip -= nRemainingWidth;
                    while (TRUE) {
                        pSurface += nPitchAdjust;
                        if (--nHeight == 0) {
                            return TRUE;
                        }

                        if (nSkip < nWidth) {
                            break;
                        }

                        nSkip -= nWidth;
                    }
                    nRemainingWidth = nWidth - nSkip;
                }
            } else {
                DWORD rgb = pPalette[nColor] & 0xFFFFFF;
                if (rgb != rgbShadow) {
                    BYTE* pRgb = reinterpret_cast<BYTE*>(&pPalette[nColor]);
                    pSurface[2] = static_cast<BYTE>(((rgb >> 16) * nInvTransVal + pSurface[2] * nTransVal) >> 8);
                    pSurface[1] = static_cast<BYTE>((pRgb[1] * nInvTransVal + pSurface[1] * nTransVal) >> 8);
                    pSurface[0] = static_cast<BYTE>((pRgb[0] * nInvTransVal + pSurface[0] * nTransVal) >> 8);
                }
                pFrameData++;
                pSurface += 3;
                if (--nRemainingWidth == 0) {
                    pSurface += nPitchAdjust;
                    if (--nHeight == 0) {
                        return TRUE;
                    }
                    nRemainingWidth = nWidth;
                }
            }
        }
    } else {
        INT pos = 0;
        for (INT y = 0; y < static_cast<INT>(nHeight); y++) {
            for (INT x = 0; x < static_cast<INT>(nWidth); x++) {
                BYTE* pRgb = reinterpret_cast<BYTE*>(&pPalette[pFrameData[pos]]);

                // NOTE: Original bug: compares the low byte of the palette
                // entry (not the whole color / palette index) against both the
                // shadow color and the transparent color index.
                if (pRgb[0] != rgbShadow && pRgb[0] != nTransparentColor) {
                    pSurface[2] = static_cast<BYTE>((pRgb[2] * nInvTransVal + pSurface[2] * nTransVal) >> 8);
                    pSurface[1] = static_cast<BYTE>((pRgb[1] * nInvTransVal + pSurface[1] * nTransVal) >> 8);
                    pSurface[0] = static_cast<BYTE>((pSurface[0] * nTransVal + pRgb[0] * nInvTransVal) >> 8);
                }
                pSurface += 3;
                pos++;
            }
            pSurface += nPitchAdjust;
        }
    }

    return TRUE;
}

// 0x7CFBC0
BOOL CVidCell::Blt8To32(DWORD* pSurface, LONG lPitch, DWORD dwFlags, INT nTransVal)
{
    int nWidth = m_pFrame->nWidth;
    int nHeight = m_pFrame->nHeight;

    if (nWidth == 0 || nHeight == 0) {
        return TRUE;
    }

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 32, dwFlags, &m_paletteAffects, nTransVal);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    }

    if (g_pChitin->field_174) {
        if (m_nCurrentFrame == 0) {
            for (int index = CVidPalette::SHADOW_ENTRY + 1; index < 256; index++) {
                CVidImage::rgbTempPal[index] = g_pChitin->GetCurrentVideoMode()->ConvertToSurfaceRGB(255);
            }
        }
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    // NOTE: Original code is slightly different, but does the same thing.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        int nRunLength = 0;
        int pos = 0;
        for (int y = 0; y < nHeight; y++) {
            int nRemainingWidth = nWidth;
            while (nRemainingWidth != 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        pos += 2;

                        pSurface += nRemainingWidth;
                        nRunLength = 0;
                        nRemainingWidth = 0;
                    } else if (nRemainingWidth > nRunLength) {
                        pos += 2;

                        pSurface += nRunLength;
                        nRemainingWidth -= nRunLength;
                        nRunLength = 0;
                    } else {
                        pSurface += nRemainingWidth;
                        nRunLength -= nRemainingWidth;
                        nRemainingWidth = 0;
                    }
                } else {
                    pos++;

                    *pSurface++ = CVidImage::rgbTempPal[nColor];
                    nRemainingWidth--;
                }
            }
            pSurface += lPitch / 4 - nWidth;
        }
    } else {
        for (int y = 0; y < nHeight; y++) {
            for (int x = 0; x < nWidth; x++) {
                BYTE nColor = *pFrameData;
                if (nColor != nTransparentColor) {
                    *pSurface = CVidImage::rgbTempPal[nColor];
                }
                pFrameData++;
                pSurface++;
            }
            pSurface += lPitch / 4 - nWidth;
        }
    }

    return TRUE;
}

// 0x7CFDF0
BOOL CVidCell::Blt8To32(DWORD* pSurface, LONG lPitch, const CPoint& pt, const CRect& rClip, DWORD dwFlags, const CPoint& ptSource, INT nTransVal)
{
    int nBltWidth = m_pFrame->nWidth - ptSource.x;
    int nBltHeight = m_pFrame->nHeight - ptSource.y;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 5029
    UTIL_ASSERT(ptSource.x >= 0 && ptSource.y >= 0);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 5030
    UTIL_ASSERT(rClip.top >= 0 && rClip.top <= rClip.bottom && rClip.left >= 0 && rClip.left <= rClip.right);

    if (pt.y > rClip.bottom
        || pt.x > rClip.right
        || pt.x + nBltWidth < rClip.left
        || pt.y + nBltHeight < rClip.top) {
        return TRUE;
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    DWORD* pDest = pSurface + lPitch / 4 * pt.y + pt.x;

    // NOTE: Original code is different, but does the same thing.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        int nWidth = m_pFrame->nWidth;
        int nHeight = m_pFrame->nHeight;
        int nRunLength = 0;
        int pos = 0;

        // Skip src top.
        int nSkipSrcH = ptSource.y;
        while (nSkipSrcH > 0) {
            int nRemainingWidth = nWidth;
            while (nRemainingWidth != 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        pos += 2;

                        nRunLength = 0;
                        nRemainingWidth = 0;
                    } else if (nRemainingWidth > nRunLength) {
                        pos += 2;

                        nRemainingWidth -= nRunLength;
                        nRunLength = 0;
                    } else {
                        nRunLength -= nRemainingWidth;
                        nRemainingWidth = 0;
                    }
                } else {
                    pos++;
                    nRemainingWidth--;
                }
            }
            nSkipSrcH--;
        }

        // Skip dest top.
        int nSkipDestH = max(rClip.top - pt.y, 0);
        while (nSkipDestH > 0) {
            int nRemainingWidth = nWidth;
            while (nRemainingWidth != 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        pos += 2;

                        pDest += nRemainingWidth;
                        nRunLength = 0;
                        nRemainingWidth = 0;
                    } else if (nRemainingWidth > nRunLength) {
                        pos += 2;

                        pDest += nRunLength;
                        nRemainingWidth -= nRunLength;
                        nRunLength = 0;
                    } else {
                        pDest += nRemainingWidth;
                        nRunLength -= nRemainingWidth;
                        nRemainingWidth = 0;
                    }
                } else {
                    pos++;

                    pDest++;
                    nRemainingWidth--;
                }
            }
            pDest += lPitch / 4 - nWidth;
            nSkipDestH--;
        }

        int nBltH = min(nBltHeight - max(rClip.top - pt.y, 0), rClip.bottom - pt.y);
        while (nBltH > 0) {
            int nRemainingWidth = nWidth;

            // Skip src left.
            int nSkipSrcW = min(ptSource.x, nRemainingWidth);
            nRemainingWidth -= nSkipSrcW;
            while (nSkipSrcW > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nSkipSrcW == nRunLength) {
                        pos += 2;

                        nRunLength = 0;
                        nSkipSrcW = 0;
                    } else if (nSkipSrcW > nRunLength) {
                        pos += 2;

                        nSkipSrcW -= nRunLength;
                        nRunLength = 0;
                    } else {
                        nRunLength -= nSkipSrcW;
                        nSkipSrcW = 0;
                    }
                } else {
                    pos++;
                    nSkipSrcW--;
                }
            }

            // Skip dest left.
            int nSkipDestW = min(max(rClip.left - pt.x, 0), nRemainingWidth);
            nRemainingWidth -= nSkipDestW;
            while (nSkipDestW > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nSkipDestW == nRunLength) {
                        pos += 2;

                        pDest += nSkipDestW;
                        nRunLength = 0;
                        nSkipDestW = 0;
                    } else if (nSkipDestW > nRunLength) {
                        pos += 2;

                        pDest += nRunLength;
                        nSkipDestW -= nRunLength;
                        nRunLength = 0;
                    } else {
                        pDest += nSkipDestW;
                        nRunLength -= nSkipDestW;
                        nSkipDestW = 0;
                    }
                } else {
                    pos++;
                    pDest++;
                    nSkipDestW--;
                }
            }

            // Blt.
            int nBltW = min(max(rClip.right - rClip.left, 0), nRemainingWidth);
            nRemainingWidth -= nBltW;
            while (nBltW > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nBltW == nRunLength) {
                        pos += 2;

                        pDest += nBltW;
                        nRunLength = 0;
                        nBltW = 0;
                    } else if (nBltW > nRunLength) {
                        pos += 2;

                        pDest += nRunLength;
                        nBltW -= nRunLength;
                        nRunLength = 0;
                    } else {
                        pDest += nBltW;
                        nRunLength -= nBltW;
                        nBltW = 0;
                    }
                } else {
                    pos++;

                    *pDest++ = CVidImage::rgbTempPal[nColor];
                    nBltW--;
                }
            }

            // Skip remainder.
            while (nRemainingWidth > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        pos += 2;

                        pDest += nRemainingWidth;
                        nRunLength = 0;
                        nRemainingWidth = 0;
                    } else if (nRemainingWidth > nRunLength) {
                        pos += 2;

                        pDest += nRunLength;
                        nRemainingWidth -= nRunLength;
                        nRunLength = 0;
                    } else {
                        pDest += nRemainingWidth;
                        nRunLength -= nRemainingWidth;
                        nRemainingWidth = 0;
                    }
                } else {
                    pos++;

                    pDest++;
                    nRemainingWidth--;
                }
            }
            pDest += lPitch / 4 - nBltWidth;
            nBltH--;
        }
    } else {
        INT nSrcJump = ptSource.x;
        INT nBltH = nBltHeight;
        INT nBltW = nBltWidth;

        pFrameData += m_pFrame->nWidth * ptSource.y + ptSource.x;

        if (pt.y < rClip.top) {
            nBltH -= rClip.top - pt.y;
            pDest += lPitch / 4 * (rClip.top - pt.y);
            pFrameData += m_pFrame->nWidth * (rClip.top - pt.y);
        }

        if (pt.y + nBltHeight > rClip.bottom) {
            nBltH -= pt.y + nBltHeight - rClip.bottom;
        }

        if (pt.x < rClip.left) {
            nBltW -= rClip.left - pt.x;
            nSrcJump += rClip.left - pt.x;
            pFrameData += rClip.left - pt.x;
            pDest += rClip.left - pt.x;
        }

        if (pt.x + nBltWidth > rClip.right) {
            nBltW -= pt.x + nBltWidth - rClip.right;
            nSrcJump += pt.x + nBltWidth - rClip.right;
        }

        if (nTransVal == 0) {
            for (int y = 0; y < nBltH; y++) {
                for (int x = 0; x < nBltW; x++) {
                    BYTE nColor = *pFrameData;
                    if (nColor != nTransparentColor) {
                        *pDest = CVidImage::rgbTempPal[nColor];
                    }
                    pFrameData++;
                    pDest++;
                }
                pDest += lPitch / 4 - nBltW;
                pFrameData += nSrcJump;
            }
        } else {
            for (int y = 0; y < nBltH; y++) {
                for (int x = 0; x < nBltW; x++) {
                    BYTE nColor = *pFrameData;
                    if (nColor == nTransparentColor) {
                        *pDest &= 0xFFFFFF;
                    } else {
                        *pDest = (*pDest & 0xFFFFFF) | ((nColor * (*pDest >> 24)) << 16) & 0xFF000000;
                    }
                    pFrameData++;
                    pDest++;
                }
                pDest += lPitch / 4 - nBltW;
                pFrameData += nSrcJump;
            }
        }
    }

    return TRUE;
}

// 0x7D0470
BOOL CVidCell::Blt8To32ClearShadow(DWORD* pSurface, LONG lPitch, DWORD dwFlags)
{
    int nWidth = m_pFrame->nWidth;
    int nHeight = m_pFrame->nHeight;

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 32, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);
    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    if (nWidth == 0 || nHeight == 0) {
        return FALSE;
    }

    // NOTE: Original code is slightly different, but does the same thing.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        if (m_bShadowOn) {
            int nRunLength = 0;
            int pos = 0;
            for (int y = 0; y < nHeight; y++) {
                int nRemainingWidth = nWidth;
                while (nRemainingWidth != 0) {
                    BYTE nColor = pFrameData[pos];
                    if (nColor == nTransparentColor) {
                        if (nRunLength == 0) {
                            nRunLength = pFrameData[pos + 1] + 1;
                        }

                        if (nRemainingWidth == nRunLength) {
                            pos += 2;

                            pSurface += nRemainingWidth;
                            nRunLength = 0;
                            nRemainingWidth = 0;
                        } else if (nRemainingWidth > nRunLength) {
                            pos += 2;

                            pSurface += nRunLength;
                            nRemainingWidth -= nRunLength;
                            nRunLength = 0;
                        } else {
                            pSurface += nRemainingWidth;
                            nRunLength -= nRemainingWidth;
                            nRemainingWidth = 0;
                        }
                    } else if (nColor == 1) {
                        pos++;

                        *pSurface++ = (*pSurface / 2) & 0x7F7F7F;
                        nRemainingWidth--;
                    } else {
                        pos++;

                        *pSurface++ = CVidImage::rgbTempPal[nColor];
                        nRemainingWidth--;
                    }
                }
                pSurface += lPitch / 4 - nWidth;
            }
        } else {
            int nRunLength = 0;
            int pos = 0;
            for (int y = 0; y < nHeight; y++) {
                int nRemainingWidth = nWidth;
                while (nRemainingWidth != 0) {
                    BYTE nColor = pFrameData[pos];
                    if (nColor == nTransparentColor) {
                        if (nRunLength == 0) {
                            nRunLength = pFrameData[pos + 1] + 1;
                        }

                        if (nRemainingWidth == nRunLength) {
                            pos += 2;

                            pSurface += nRemainingWidth;
                            nRunLength = 0;
                            nRemainingWidth = 0;
                        } else if (nRemainingWidth > nRunLength) {
                            pos += 2;

                            pSurface += nRunLength;
                            nRemainingWidth -= nRunLength;
                            nRunLength = 0;
                        } else {
                            pSurface += nRemainingWidth;
                            nRunLength -= nRemainingWidth;
                            nRemainingWidth = 0;
                        }
                    } else {
                        pos++;

                        *pSurface++ = CVidImage::rgbTempPal[nColor];
                        nRemainingWidth--;
                    }
                }
                pSurface += lPitch / 4 - nWidth;
            }
        }
    } else {
        for (int y = 0; y < nHeight; y++) {
            for (int x = 0; x < nWidth; x++) {
                BYTE nColor = *pFrameData;
                if (nColor != nTransparentColor) {
                    if (m_bShadowOn && nColor == 1) {
                        *pSurface = (*pSurface / 2) & 0x7F7F7F;
                    } else {
                        *pSurface = CVidImage::rgbTempPal[nColor];
                    }
                }
                pFrameData++;
                pSurface++;
            }
            pSurface += lPitch / 4 - nWidth;
        }
    }

    return TRUE;
}

// 0x7D0730
BOOL CVidCell::Blt8To32Brightest(DWORD* pSurface, LONG lPitch, DWORD dwFlags)
{
    int nWidth = m_pFrame->nWidth;
    int nHeight = m_pFrame->nHeight;

    if (nWidth == 0 || nHeight == 0) {
        return TRUE;
    }

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 32, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    // NOTE: Original code is different, not sure if the implementation is
    // right.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        int nRunLength = 0;
        int pos = 0;
        for (int y = 0; y < nHeight; y++) {
            int nRemainingWidth = nWidth;
            while (nRemainingWidth != 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        pos += 2;

                        pSurface += nRemainingWidth;
                        nRunLength = 0;
                        nRemainingWidth = 0;
                    } else if (nRemainingWidth > nRunLength) {
                        pos += 2;

                        pSurface += nRunLength;
                        nRemainingWidth -= nRunLength;
                        nRunLength = 0;
                    } else {
                        pSurface += nRemainingWidth;
                        nRunLength -= nRemainingWidth;
                        nRemainingWidth = 0;
                    }
                } else {
                    pos++;

                    // NOTE: Not present in uncompressed data branch.
                    if ((CVidImage::rgbTempPal[nColor] & 0xF0F0F0) != 0) {
                        *pSurface = RGB(max(GetRValue(*pSurface), GetRValue(CVidImage::rgbTempPal[nColor])),
                            max(GetGValue(*pSurface), GetGValue(CVidImage::rgbTempPal[nColor])),
                            max(GetBValue(*pSurface), GetBValue(CVidImage::rgbTempPal[nColor])));
                    }

                    pSurface++;
                    nRemainingWidth--;
                }
            }
            pSurface += lPitch / 4 - nWidth;
        }
    } else {
        for (int y = 0; y < nHeight; y++) {
            for (int x = 0; x < nWidth; x++) {
                BYTE nColor = *pFrameData;
                if (nColor != nTransparentColor) {
                    *pSurface = RGB(max(GetRValue(*pSurface), GetRValue(CVidImage::rgbTempPal[nColor])),
                        max(GetGValue(*pSurface), GetGValue(CVidImage::rgbTempPal[nColor])),
                        max(GetBValue(*pSurface), GetBValue(CVidImage::rgbTempPal[nColor])));
                }
                pFrameData++;
                pSurface++;
            }
            pSurface += lPitch / 4 - nWidth;
        }
    }

    return TRUE;
}

// 0x7D0950
BOOL CVidCell::Blt8To32Translucent(DWORD* pSurface, LONG lPitch, DWORD dwFlags, INT nTransVal)
{
    UINT nWidth = m_pFrame->nWidth;
    UINT nHeight = m_pFrame->nHeight;

    if (nHeight == 0 || nWidth == 0) {
        return TRUE;
    }

    DWORD rgbShadow = g_pChitin->GetCurrentVideoMode()->field_24;
    INT nInvTransVal = 256 - nTransVal;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage24.cpp
    // __LINE__: 6515
    UTIL_ASSERT(nTransVal <= 256);

    // NOTE: In bytes.
    LONG nPitchAdjust = lPitch - 4 * nWidth;
    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 32, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = rgbShadow;
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;
    DWORD* pPalette = CVidImage::rgbTempPal;

    // NOTE: Original code is inline assembly.
    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        BYTE* pDest = reinterpret_cast<BYTE*>(pSurface);
        UINT nRemainingWidth = nWidth;
        while (TRUE) {
            BYTE nColor = *pFrameData;
            if (nColor == nTransparentColor) {
                UINT nSkip = pFrameData[1];
                pFrameData += 2;
                pDest += 4 * nSkip + 4;
                nRemainingWidth--;
                if (nSkip < nRemainingWidth) {
                    nRemainingWidth -= nSkip;
                } else {
                    // The run continues on the following row(s).
                    nSkip -= nRemainingWidth;
                    while (TRUE) {
                        pDest += nPitchAdjust;
                        if (--nHeight == 0) {
                            return TRUE;
                        }

                        if (nSkip < nWidth) {
                            break;
                        }

                        nSkip -= nWidth;
                    }
                    nRemainingWidth = nWidth - nSkip;
                }
            } else {
                DWORD rgb = pPalette[nColor] & 0xFFFFFF;
                if (rgb != rgbShadow) {
                    DWORD rgbDest = *reinterpret_cast<DWORD*>(pDest);
                    DWORD nR = ((rgb >> 16) & 0xFF) * nInvTransVal + ((rgbDest >> 16) & 0xFF) * nTransVal;
                    DWORD nG = ((rgb >> 8) & 0xFF) * nInvTransVal + ((rgbDest >> 8) & 0xFF) * nTransVal;
                    DWORD nB = (rgb & 0xFF) * nInvTransVal + (rgbDest & 0xFF) * nTransVal;
                    rgb = ((nR << 8) & 0xFFFF0000) | (nG & 0xFF00) | ((nB >> 8) & 0xFF);
                }
                *reinterpret_cast<DWORD*>(pDest) = rgb;
                pFrameData++;
                pDest += 4;
                if (--nRemainingWidth == 0) {
                    pDest += nPitchAdjust;
                    if (--nHeight == 0) {
                        return TRUE;
                    }
                    nRemainingWidth = nWidth;
                }
            }
        }
    } else {
        INT pos = 0;
        for (INT y = 0; y < static_cast<INT>(nHeight); y++) {
            for (INT x = 0; x < static_cast<INT>(nWidth); x++) {
                BYTE nColor = pFrameData[pos];
                if (nColor != nTransparentColor) {
                    DWORD rgbDest = *pSurface;
                    BYTE* pDest = reinterpret_cast<BYTE*>(&rgbDest);
                    BYTE* pRgb = reinterpret_cast<BYTE*>(&pPalette[nColor]);
                    pDest[2] = static_cast<BYTE>((pRgb[2] * nInvTransVal + pDest[2] * nTransVal) >> 8);
                    pDest[1] = static_cast<BYTE>((pRgb[1] * nInvTransVal + pDest[1] * nTransVal) >> 8);
                    pDest[0] = static_cast<BYTE>((pRgb[0] * nInvTransVal + pDest[0] * nTransVal) >> 8);
                    *pSurface = rgbDest;
                }
                pSurface++;
                pos++;
            }

            // FIXME: Original bug: `nPitchAdjust` is in bytes, but is added to
            // a `DWORD` pointer (the compressed path above uses it correctly).
            pSurface += nPitchAdjust;
        }
    }

    return TRUE;
}

// 0x7B49D0
BOOL CVidCell::Blt8To16Glow(WORD* pSurface, LONG lPitch, DWORD dwFlags)
{
    int nHeight = m_pFrame->nHeight;
    int nWidth = m_pFrame->nWidth;
    DWORD dwRBitMask = g_pChitin->GetCurrentVideoMode()->m_dwRBitMask;
    DWORD dwGBitMask = g_pChitin->GetCurrentVideoMode()->m_dwGBitMask;
    DWORD dwBBitMask = g_pChitin->GetCurrentVideoMode()->m_dwBBitMask;
    BYTE nRBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwRBitShift);
    BYTE nGBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwGBitShift);
    BYTE nBBitShift = static_cast<BYTE>(g_pChitin->GetCurrentVideoMode()->m_dwBBitShift);
    LONG nJump = (lPitch - 2 * nWidth) >> 1;

    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 16, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        // NOTE: The original indexes `rgbTempPal` as a `WORD` array here
        // (`[EDX*2 + 0x9064f8]`), so this overwrites the high word of entry 0
        // rather than the shadow entry.
        reinterpret_cast<WORD*>(CVidImage::rgbTempPal)[CVidPalette::SHADOW_ENTRY] = static_cast<WORD>(g_pChitin->GetCurrentVideoMode()->field_24);
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    BOOL b555 = (dwRBitMask & 0x7C00) == dwRBitMask
        && (dwGBitMask & 0x3E0) == dwGBitMask
        && (dwBBitMask & 0x1F) == dwBBitMask;
    BOOL b565 = (dwRBitMask & 0xF800) == dwRBitMask
        && (dwGBitMask & 0x7E0) == dwGBitMask
        && (dwBBitMask & 0x1F) == dwBBitMask;

    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        // NOTE: The original has three versions of this loop: inline
        // assembly with hardcoded 5-5-5 and 5-6-5 masks, and C code using the
        // video mode masks. They produce the same result.
        if (b555) {
            dwRBitMask = 0x7C00;
            dwGBitMask = 0x3E0;
            dwBBitMask = 0x1F;
            nRBitShift = 10;
            nGBitShift = 5;
            nBBitShift = 0;
        } else if (b565) {
            dwRBitMask = 0xF800;
            dwGBitMask = 0x7E0;
            dwBBitMask = 0x1F;
            nRBitShift = 11;
            nGBitShift = 5;
            nBBitShift = 0;
        }

        if (nHeight <= 0) {
            return TRUE;
        }

        int nRunLength = 0;
        int pos = 0;
        for (int y = 0; y < nHeight; y++) {
            int nRemainingWidth = nWidth;
            while (nRemainingWidth > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = pFrameData[pos + 1] + 1;
                    }

                    if (nRemainingWidth == nRunLength) {
                        pos += 2;
                        pSurface += nRemainingWidth;
                        nRunLength = 0;
                        break;
                    }

                    if (nRemainingWidth < nRunLength) {
                        nRunLength -= nRemainingWidth;
                        pSurface += nRemainingWidth;
                        break;
                    }

                    pos += 2;
                    nRemainingWidth -= nRunLength;
                    pSurface += nRunLength;
                    nRunLength = 0;
                } else {
                    DWORD nSrc = static_cast<WORD>(CVidImage::rgbTempPal[nColor]);
                    DWORD nDest = *pSurface;
                    DWORD nSrcR = (nSrc & dwRBitMask) >> nRBitShift;
                    DWORD nSrcG = (nSrc & dwGBitMask) >> nGBitShift;
                    DWORD nSrcB = (nSrc & dwBBitMask) >> nBBitShift;

                    // Brighter source pixels are more opaque.
                    DWORD nAlpha = (nSrcG + nSrcB + nSrcR) / 3;
                    DWORD nInvAlpha = 41 - nAlpha;

                    *pSurface = static_cast<WORD>(((((nDest & dwBBitMask) >> nBBitShift) * nInvAlpha + nAlpha * nSrcB) / 41 << nBBitShift)
                        | ((((nDest & dwGBitMask) >> nGBitShift) * nInvAlpha + nAlpha * nSrcG) / 41 << nGBitShift)
                        | ((((nDest & dwRBitMask) >> nRBitShift) * nInvAlpha + nAlpha * nSrcR) / 41 << nRBitShift));

                    pSurface++;
                    pos++;
                    nRemainingWidth--;
                }
            }
            pSurface += nJump;
        }
    } else {
        int pos = 0;

        // NOTE: The original renders one row more than the frame height and
        // one pixel more than the frame width per row, does not check the
        // transparent color, and uses 31 instead of 41 as the opacity scale.
        for (int y = 0; y <= nHeight; y++) {
            for (int x = 0; x <= nWidth; x++) {
                DWORD nDest = *pSurface;
                DWORD nSrc = static_cast<WORD>(CVidImage::rgbTempPal[pFrameData[pos]]);
                DWORD nSrcR = (nSrc & dwRBitMask) >> nRBitShift;
                DWORD nSrcG = (nSrc & dwGBitMask) >> nGBitShift;
                DWORD nSrcB = (nSrc & dwBBitMask) >> nBBitShift;
                DWORD nAlpha = (nSrcB + nSrcG + nSrcR) / 3;
                DWORD nInvAlpha = 31 - nAlpha;

                *pSurface = static_cast<WORD>(((((nDest & dwBBitMask) >> nBBitShift) * nInvAlpha + nAlpha * nSrcB) / 31 << nBBitShift)
                    | ((((nDest & dwGBitMask) >> nGBitShift) * nInvAlpha + nAlpha * nSrcG) / 31 << nGBitShift)
                    | ((((nDest & dwRBitMask) >> nRBitShift) * nInvAlpha + nAlpha * nSrcR) / 31 << nRBitShift));

                pSurface++;
                pos++;
            }
            pSurface += nJump;
        }
    }

    return TRUE;
}

// 0x7D0C60
BOOL CVidCell::Blt8To24Glow(BYTE* pSurface, LONG lPitch, DWORD dwFlags)
{
    INT nWidth = m_pFrame->nWidth;
    INT nHeight = m_pFrame->nHeight;

    if (nHeight == 0 || nWidth == 0) {
        return TRUE;
    }

    LONG nPitchAdjust = lPitch - 3 * nWidth;
    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 24, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    BYTE nTransparentColor = pBamHeader->nTransparentColor;

    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        // NOTE: Run lengths and counters below are in bytes (3 per pixel).
        INT nRunLength = 0;
        INT pos = 0;
        for (INT y = nHeight; y > 0; y--) {
            INT nRemaining = 3 * nWidth;
            while (nRemaining > 0) {
                BYTE nColor = pFrameData[pos];
                if (nColor == nTransparentColor) {
                    if (nRunLength == 0) {
                        nRunLength = 3 * (pFrameData[pos + 1] + 1);
                    }

                    if (nRemaining == nRunLength) {
                        pSurface += nRemaining;
                        pos += 2;
                        nRunLength = 0;
                        break;
                    }

                    if (nRemaining < nRunLength) {
                        pSurface += nRemaining;
                        nRunLength -= nRemaining;
                        break;
                    }

                    pos += 2;
                    nRemaining -= nRunLength;
                    pSurface += nRunLength;
                    nRunLength = 0;
                } else {
                    BYTE* pRgb = reinterpret_cast<BYTE*>(&CVidImage::rgbTempPal[nColor]);
                    dword_8BDDE4 = (pSurface[2] + pSurface[1] + pSurface[0]) >> 3;

                    INT nLum = (pRgb[0] + pRgb[2] + pRgb[1]) / 3;
                    if (nLum >= dword_8BDDE4) {
                        INT nInvLum = 255 - nLum;
                        pSurface[0] = static_cast<BYTE>((pSurface[0] * nInvLum + pRgb[0] * nLum) >> 8);
                        pSurface[1] = static_cast<BYTE>((pRgb[1] * nLum + pSurface[1] * nInvLum) >> 8);
                        pSurface[2] = static_cast<BYTE>((pRgb[2] * nLum + pSurface[2] * nInvLum) >> 8);
                    }

                    pSurface += 3;
                    pos++;
                    nRemaining -= 3;
                }
            }
            pSurface += nPitchAdjust;
        }
    } else {
        // NOTE: Original bugs: the transparent color is ignored, each row
        // processes `nWidth + 1` pixels (the extra one being handled
        // separately below) so both the source and destination drift by one
        // pixel per row.
        //
        // FIXME: The original stores an uninitialized stack variable over
        // every blended pixel (a DWORD in the main loop, a WORD in the
        // trailing pixel), so this path produced garbage in the original. It
        // is replaced with zero here.
        DWORD dwUninitialized = 0;
        INT pos = 0;
        for (INT y = nHeight; y > 0; y--) {
            BYTE* pLast = pSurface;
            for (INT x = nWidth; x > 0; x--) {
                pLast = pSurface;

                BYTE* pRgb = reinterpret_cast<BYTE*>(&CVidImage::rgbTempPal[pFrameData[pos]]);
                dword_8BDDE8 = (pSurface[2] + pSurface[1] + pSurface[0]) >> 3;

                INT nLum = (pRgb[0] + pRgb[2] + pRgb[1]) / 3;
                if (nLum < dword_8BDDE8) {
                    nLum = 0;
                }

                INT nInvLum = 255 - nLum;
                pSurface[0] = static_cast<BYTE>(((pSurface[0] * nInvLum) >> 8) + ((pRgb[0] * nLum) >> 8));
                pSurface[1] = static_cast<BYTE>(((pRgb[1] * nLum) >> 8) + ((pSurface[1] * nInvLum) >> 8));
                pSurface[2] = static_cast<BYTE>(((pRgb[2] * nLum) >> 8) + ((pSurface[2] * nInvLum) >> 8));
                *reinterpret_cast<DWORD*>(pSurface) = dwUninitialized;

                pSurface += 3;
                pos++;
            }

            BYTE* pRgb = reinterpret_cast<BYTE*>(&CVidImage::rgbTempPal[pFrameData[pos]]);

            *reinterpret_cast<WORD*>(pLast) = *reinterpret_cast<WORD*>(pSurface);
            pLast[2] = pSurface[2];

            dword_8BDDF0 = (pLast[2] + pLast[0] + pLast[1]) / dword_8BDDF4;

            INT nLum = (pRgb[0] + pRgb[2] + pRgb[1]) / 3;
            if (nLum < dword_8BDDF0) {
                nLum = 0;
            }

            INT nInvLum = 255 - nLum;
            pLast[0] = static_cast<BYTE>(pLast[0] * nInvLum / 255 + pRgb[0] * nLum / 255);
            pLast[1] = static_cast<BYTE>(pRgb[1] * nLum / 255 + pLast[1] * nInvLum / 255);
            pLast[2] = static_cast<BYTE>(pRgb[2] * nLum / 255 + pLast[2] * nInvLum / 255);

            *reinterpret_cast<WORD*>(pSurface) = static_cast<WORD>(dwUninitialized);
            pSurface[2] = pLast[2];

            pos++;
            pSurface += nPitchAdjust + 3;
        }
    }

    return TRUE;
}

// 0x7D1190
BOOL CVidCell::Blt8To32Glow(DWORD* pSurface, LONG lPitch, DWORD dwFlags)
{
    DWORD rgbPalette[256];
    INT nWidth = m_pFrame->nWidth;
    INT nHeight = m_pFrame->nHeight;

    if (nHeight == 0 || nWidth == 0) {
        return TRUE;
    }

    // NOTE: In pixels.
    LONG nPitchAdjust = (lPitch >> 2) - nWidth;
    BYTE* pFrameData = pRes->GetFrameData(m_pFrame, m_bDoubleSize);

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(rgbPalette, 32, dwFlags, &m_paletteAffects, 255);

    if (!m_bShadowOn) {
        rgbPalette[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    }

    BAMHEADER* pBamHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;
    INT nTransparentColor = pBamHeader->nTransparentColor;

    if (pRes->GetCompressed(m_pFrame, m_bDoubleSize)) {
        // NOTE: Original code is inline assembly.

        // NOTE: In bytes.
        nPitchAdjust = lPitch - 4 * nWidth;

        BYTE* pDest = reinterpret_cast<BYTE*>(pSurface);
        UINT nRemainingWidth = nWidth;
        while (TRUE) {
            BYTE nColor = *pFrameData;
            if (nColor == nTransparentColor) {
                UINT nSkip = pFrameData[1];
                pFrameData += 2;
                pDest += 4 * nSkip + 4;
                nRemainingWidth--;
                if (nSkip < nRemainingWidth) {
                    nRemainingWidth -= nSkip;
                } else {
                    // The run continues on the following row(s).
                    nSkip -= nRemainingWidth;
                    while (TRUE) {
                        pDest += nPitchAdjust;
                        if (--nHeight == 0) {
                            return TRUE;
                        }

                        if (nSkip < static_cast<UINT>(nWidth)) {
                            break;
                        }

                        nSkip -= nWidth;
                    }
                    nRemainingWidth = nWidth - nSkip;
                }
            } else {
                DWORD rgb = rgbPalette[nColor];
                INT nSum = (rgb & 0xFF) + ((rgb >> 8) & 0xFF) + ((rgb >> 16) & 0xFF);
                if (nSum > 0x81) {
                    if (nSum >= 0x28C) {
                        *reinterpret_cast<DWORD*>(pDest) = rgb;
                    } else {
                        DWORD nAlpha = (nSum * 42) >> 7;
                        DWORD rgbDest = *reinterpret_cast<DWORD*>(pDest);
                        DWORD nDestLum = (rgbDest >> 3) & 0x1F1F1F;
                        nDestLum = static_cast<BYTE>((nDestLum >> 16) + static_cast<BYTE>(nDestLum + (nDestLum >> 8)));
                        if (nAlpha >= nDestLum) {
                            DWORD nInvAlpha = nAlpha ^ 0xFF;
                            rgbDest &= 0xFFFFFF;

                            DWORD rgbNew = ((((rgb & 0xFFFFFF) >> 16) * nAlpha + (rgbDest >> 16) * nInvAlpha) << 8) & 0xFF0000;
                            rgbNew = ((((rgb >> 8) & 0xFF) * nAlpha | rgbNew) + ((rgbDest >> 8) & 0xFF) * nInvAlpha) & 0xFFFF00;
                            rgbNew |= (((rgb & 0xFF) * nAlpha + (rgbDest & 0xFF) * nInvAlpha) >> 8) & 0xFF;
                            *reinterpret_cast<DWORD*>(pDest) = rgbNew;
                        }
                    }
                }
                pFrameData++;
                pDest += 4;
                if (--nRemainingWidth == 0) {
                    pDest += nPitchAdjust;
                    if (--nHeight == 0) {
                        return TRUE;
                    }
                    nRemainingWidth = nWidth;
                }
            }
        }
    } else {
        // NOTE: Original bugs: the transparent color is ignored, the loops
        // process `nHeight + 1` rows of `nWidth + 1` pixels (so source and
        // destination drift by one pixel per row and the last row overruns).
        INT pos = 0;
        for (INT y = 0; y <= nHeight; y++) {
            for (INT x = 0; x <= nWidth; x++) {
                BYTE* pRgb = reinterpret_cast<BYTE*>(&rgbPalette[pFrameData[pos]]);
                BYTE* pDest = reinterpret_cast<BYTE*>(pSurface);
                dword_8BDDF8 = (pDest[1] + pDest[0] + pDest[2]) >> 3;

                INT nLum = (pRgb[0] + pRgb[2] + pRgb[1]) / 3;
                if (nLum >= dword_8BDDF8) {
                    INT nInvLum = 255 - nLum;
                    pDest[0] = static_cast<BYTE>((pDest[0] * nInvLum + pRgb[0] * nLum) >> 8);
                    pDest[1] = static_cast<BYTE>((pDest[1] * nInvLum + pRgb[1] * nLum) >> 8);
                    pDest[2] = static_cast<BYTE>((pRgb[2] * nLum + pDest[2] * nInvLum) >> 8);
                }

                pSurface++;
                pos++;
            }
            pSurface += nPitchAdjust;
        }
    }

    return TRUE;
}

// 0x7C4240
void CVidCell::RenderTexture(INT x, INT y, const CRect& rFxRect, CSize dataPitch, const CRect& rClip, DWORD dwFlags)
{
    CSize texSize(dataPitch);
    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    float x4;
    float y4;

    if (x > rClip.right
        || y > rClip.bottom
        || x + rFxRect.Width() < rClip.left
        || y + rFxRect.Height() < rClip.top) {
        return;
    }

    x1 = static_cast<float>(x);
    y1 = static_cast<float>(y);
    x2 = static_cast<float>(x + rFxRect.Width());
    y2 = static_cast<float>(y + rFxRect.Height());

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
            texSize.cx,
            texSize.cy,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            CVideo3d::texImageData);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }

    if ((dwFlags & 0x20) != 0) {
        y3 = static_cast<float>(rFxRect.bottom) / static_cast<float>(CVidInf::FX_HEIGHT);
        y4 = static_cast<float>(rFxRect.top) / static_cast<float>(CVidInf::FX_HEIGHT);

        if (y < rClip.top) {
            y1 = static_cast<float>(rClip.top);
            y3 -= static_cast<float>(rClip.top - y) / static_cast<float>(CVidInf::FX_HEIGHT);
        }

        if (y + rFxRect.Height() > rClip.bottom) {
            y2 = static_cast<float>(rClip.bottom);
            y4 += static_cast<float>(y + rFxRect.Height() - rClip.bottom) / static_cast<float>(CVidInf::FX_HEIGHT);
        }
    } else {
        y3 = static_cast<float>(rFxRect.top) / static_cast<float>(CVidInf::FX_HEIGHT);
        y4 = static_cast<float>(rFxRect.bottom) / static_cast<float>(CVidInf::FX_HEIGHT);

        if (y < rClip.top) {
            y1 = static_cast<float>(rClip.top);
            y3 = static_cast<float>(rClip.top - y) / static_cast<float>(CVidInf::FX_HEIGHT);
        }

        if (y + rFxRect.Height() > rClip.bottom) {
            y2 = static_cast<float>(rClip.bottom);
            y4 = static_cast<float>(rClip.bottom - y) / static_cast<float>(CVidInf::FX_HEIGHT);
        }
    }

    if ((dwFlags & 0x10) != 0) {
        x3 = static_cast<float>(rFxRect.right) / static_cast<float>(CVidInf::FX_WIDTH);
        x4 = static_cast<float>(rFxRect.left) / static_cast<float>(CVidInf::FX_WIDTH);

        if (x < rClip.left) {
            x1 = static_cast<float>(rClip.left);
            x3 -= static_cast<float>(rClip.left - x) / static_cast<float>(CVidInf::FX_WIDTH);
        }

        if (x + rFxRect.Width() > rClip.right) {
            x2 = static_cast<float>(rClip.right);
            x4 += static_cast<float>(x + rFxRect.Width() - rClip.right) / static_cast<float>(CVidInf::FX_WIDTH);
        }
    } else {
        x3 = static_cast<float>(rFxRect.left) / static_cast<float>(CVidInf::FX_WIDTH);
        x4 = static_cast<float>(rFxRect.right) / static_cast<float>(CVidInf::FX_WIDTH);

        if (x < rClip.left) {
            x1 = static_cast<float>(rClip.left);
            x3 = static_cast<float>(rClip.left - x) / static_cast<float>(CVidInf::FX_WIDTH);
        }

        if (x + rFxRect.Width() > rClip.right) {
            x2 = static_cast<float>(rClip.right);
            x4 = static_cast<float>(rClip.right - x) / static_cast<float>(CVidInf::FX_WIDTH);
        }
    }

    CVideo3d::glEnable(GL_BLEND);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x200) != 0) {
        CVideo3d::glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    } else if ((dwFlags & 0x8) != 0) {
        CVideo3d::glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    } else {
        CVideo3d::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }

    if ((dwFlags & 0x80000000) != 0) {
        BYTE r = GetRValue(dword_8BAC88);
        BYTE g = GetGValue(dword_8BAC88);
        BYTE b = GetBValue(dword_8BAC88);

        BYTE nBrightnessCorrection = g_pChitin->GetCurrentVideoMode()->m_nBrightnessCorrection;
        if (nBrightnessCorrection != 0) {
            r = ~((~nBrightnessCorrection * ~r) >> 8);
            g = ~((~nBrightnessCorrection * ~g) >> 8);
            b = ~((~nBrightnessCorrection * ~b) >> 8);
        }

        BYTE nGammaCorrection = g_pChitin->GetCurrentVideoMode()->m_nGammaCorrection;
        if (nGammaCorrection != 0) {
            DWORD nLight = (1 << CVidPalette::LIGHT_SCALE) + nGammaCorrection;
            r = static_cast<BYTE>(min((nLight * r) / 8, 255));
            g = static_cast<BYTE>(min((nLight * g) / 8, 255));
            b = static_cast<BYTE>(min((nLight * b) / 8, 255));
        }

        CVideo3d::glColor4f(static_cast<float>(r) / 255.0f,
            static_cast<float>(g) / 255.0f,
            static_cast<float>(b) / 255.0f,
            1.0f);
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

    if ((dwFlags & 0x200) != 0) {
        CVideo3d::glBlendFunc(GL_SRC_COLOR, GL_ONE);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

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

    CVideo3d::glDisable(GL_BLEND);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
}

// 0x7C4960
BOOL CVidCell::Render3d(INT nRefPtX, INT nRefPtY, DWORD dwFlags, INT nTransVal)
{
    INT nNewTransVal = 255;
    if (nTransVal != -1) {
        nNewTransVal = 255 - nTransVal;
    }

    if (m_pFrame == NULL) {
        if (!GetFrame(FALSE)) {
            // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
            // __LINE__: 627
            UTIL_ASSERT(FALSE);
        }
    }

    if (!m_bPaletteChanged) {
        m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
    }

    m_cPalette.Realize(CVidImage::rgbTempPal, 32, dwFlags, &m_paletteAffects, nNewTransVal);

    if (!m_bShadowOn) {
        CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = g_pChitin->GetCurrentVideoMode()->field_24;
    }

    CRect rClip(0, 0, CVideo::SCREENWIDTH, CVideo::SCREENHEIGHT);

    BOOLEAN bResult = Render3d(nRefPtX, nRefPtY, rClip, FALSE, dwFlags);

    pRes->Release();
    m_pFrame = NULL;

    return bResult;
}

// 0x7C4A90
BOOL CVidCell::Render3d(INT nRefPtX, INT nRefPtY, const CRect& rClip, CVidPoly* pClipPoly, int nPolys, DWORD dwFlags, INT nTransVal, BOOL bDemanded)
{
    INT nNewTransVal = 255;
    if (nTransVal != -1) {
        nNewTransVal = 255 - nTransVal;
    }

    if (m_pFrame == NULL) {
        if (!GetFrame(FALSE)) {
            return FALSE;
        }
    }

    // NOTE: No reference to `CVidInf` constants.
    if (m_pFrame->nWidth > CVIDINF_FX_WIDTH || m_pFrame->nHeight > CVIDINF_FX_HEIGHT) {
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
        // __LINE__: 690
        UTIL_ASSERT_MSG(FALSE, "CVidcell larger then fx");
    }

    INT x = nRefPtX - m_pFrame->nCenterX;
    INT y = nRefPtY - m_pFrame->nCenterY;

    CRect rFxRect(0, 0, m_pFrame->nWidth, m_pFrame->nHeight);

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

    CVideo3d::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    LONG lPitch = g_pChitin->cVideo.field_13A
        ? CVidTile::BYTES_PER_TEXEL * CVIDINF_FX_WIDTH
        : CVidTile::BYTES_PER_TEXEL * m_pFrame->nWidth;

    CVidInf::FXClear(CVideo3d::texImageData, lPitch * m_pFrame->nHeight / CVidTile::BYTES_PER_TEXEL);

    BOOL bResult = Blt8To32(reinterpret_cast<DWORD*>(CVideo3d::texImageData),
        lPitch,
        dwFlags,
        nNewTransVal);
    if (bResult) {
        if (pClipPoly != NULL) {
            for (int nPoly = 0; nPoly < nPolys; nPoly++) {
                pClipPoly[nPoly].FillPoly(reinterpret_cast<WORD*>(CVideo3d::texImageData),
                    lPitch,
                    rFxRect,
                    RGB(0, 255, 0),
                    0,
                    CPoint(x, y));
            }
        }

        CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        if ((dwFlags & 0x40) == 0) {
            RenderTexture(x,
                y,
                rFxRect,
                CSize(m_pFrame->nWidth, m_pFrame->nHeight),
                rClip,
                dwFlags);
        }
    }

    if (!bDemanded) {
        pRes->Release();
    }
    m_pFrame = NULL;

    return bResult;
}

// 0x7C4E20
BOOL CVidCell::Render3d(INT nRefPtX, INT nRefPtY, const CRect& rClip, BOOLEAN bDemanded, DWORD dwFlags)
{
    CPoint ptStart;
    CPoint ptSource;
    CRect rFrameClip;
    CRect rFxRect;
    BOOLEAN bResult;
    INT x;
    INT y;
    LONG lPitch;

    ptStart.x = 0;
    ptStart.y = 0;

    if (m_pFrame == NULL) {
        if (!GetFrame(bDemanded)) {
            return FALSE;
        }
    }

    x = nRefPtX - m_pFrame->nCenterX;
    y = nRefPtY - m_pFrame->nCenterY;

    if ((dwFlags & 0x40) != 0) {
        ptStart.x = x;
        ptStart.y = y;
    }

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

    CVideo3d::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    for (INT yOffset = 0; yOffset < static_cast<LONG>(m_pFrame->nHeight); yOffset += CVidInf::FX_HEIGHT) {
        for (INT xOffset = 0; xOffset < static_cast<LONG>(m_pFrame->nWidth); xOffset += CVidInf::FX_WIDTH) {
            if (yOffset != 0 || xOffset != 0) {
                CVidInf::FXClear(CVideo3d::texImageData, CVidInf::FX_WIDTH * CVidInf::FX_HEIGHT);
            }

            rFrameClip.SetRect(0,
                0,
                min(m_pFrame->nWidth - xOffset, CVidInf::FX_WIDTH),
                min(m_pFrame->nHeight - yOffset, CVidInf::FX_HEIGHT));
            lPitch = g_pChitin->cVideo.field_13A
                ? CVidTile::BYTES_PER_TEXEL * CVIDINF_FX_WIDTH
                : CVidTile::BYTES_PER_TEXEL * rFrameClip.Width();
            ptSource.x = xOffset;
            ptSource.y = yOffset;

            bResult = Blt8To32(reinterpret_cast<DWORD*>(CVideo3d::texImageData),
                lPitch,
                ptStart,
                rFrameClip,
                dwFlags,
                ptSource,
                0);
            if (!bResult) {
                // FIXME: Does not check `bDemanded` before releasing.
                pRes->Release();
                m_pFrame = NULL;
                return FALSE;
            }

            if ((dwFlags & 0x40) == 0) {
                rFxRect.SetRect(0,
                    0,
                    min(m_pFrame->nWidth - xOffset, CVidInf::FX_WIDTH),
                    min(m_pFrame->nHeight - yOffset, CVidInf::FX_HEIGHT));

                CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

                RenderTexture(x + xOffset,
                    y + yOffset,
                    rFxRect,
                    rFrameClip.Size(),
                    rClip,
                    dwFlags);
            }
        }
    }

    if ((dwFlags & 0x40) != 0) {
        CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }

    m_pFrame = NULL;

    if (!bDemanded) {
        pRes->Release();
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
    // __LINE__: 918
    UTIL_ASSERT(bResult);

    return bResult;
}

// 0x7C5210
BOOL CVidCell::FXRender3d(INT nRefPtX, INT nRefPtY, const CRect& rFXRect, DWORD dwFlags, INT nTransVal, BOOL bAlphaValues)
{
    BOOL bResult;

    INT nNewTransVal = 255;
    if (nTransVal != -1) {
        nNewTransVal = 255 - nTransVal;
    }

    if (m_pFrame == NULL) {
        if (!GetFrame(FALSE)) {
            // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
            // __LINE__: 960
            UTIL_ASSERT(FALSE);
        }
    }

    if (!bAlphaValues) {
        if (!m_bPaletteChanged) {
            m_cPalette.SetPalette(pRes->m_pPalette, 256, CVidPalette::TYPE_RESOURCE);
        }

        m_cPalette.Realize(CVidImage::rgbTempPal,
            32,
            dwFlags,
            &m_paletteAffects,
            nNewTransVal);

        if (!m_bShadowOn) {
            CVidImage::rgbTempPal[CVidPalette::SHADOW_ENTRY] = RGB(0, 0, 0);
        }
    }

    CRect rClip(0, 0, CVideo::SCREENWIDTH, CVideo::SCREENHEIGHT);
    bResult = FXRender3d(nRefPtX, nRefPtY, rFXRect, rClip, TRUE, dwFlags, bAlphaValues);
    m_pFrame = NULL;

    return bResult;
}

// 0x7C5330
BOOL CVidCell::FXRender3d(INT nRefPtX, INT nRefPtY, const CRect& rFXRect, const CRect& rClip, BOOLEAN bDemanded, DWORD dwFlags, BOOL bAlphaValues)
{
    BOOL bResult;
    CPoint pt(0, 0);
    CSize size;
    CRect r;

    if (m_pFrame == NULL) {
        if (!GetFrame(bDemanded)) {
            return FALSE;
        }
    }

    INT x = nRefPtX - m_pFrame->nCenterX;
    INT y = nRefPtY - m_pFrame->nCenterY;

    size.cx = min(m_pFrame->nWidth, CVidInf::FX_WIDTH);
    size.cy = min(m_pFrame->nHeight, CVidInf::FX_HEIGHT);

    if ((dwFlags & 0x40) != 0) {
        pt.x = x;
        pt.y = y;
        r.SetRect(max(x, rClip.left),
            max(y, rClip.top),
            min(x + size.cx, rClip.right),
            min(y + size.cy, rClip.bottom));
    } else {
        r.SetRect(pt, pt + size);
    }

    r.NormalizeRect();

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

    CVideo3d::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    LONG lPitch = g_pChitin->cVideo.field_13A
        ? CVidTile::BYTES_PER_TEXEL * CVIDINF_FX_WIDTH
        : CVidTile::BYTES_PER_TEXEL * rFXRect.Width();

    bResult = Blt8To32(CVideo3d::texImageData,
        lPitch,
        pt,
        r,
        dwFlags,
        CPoint(0, 0),
        bAlphaValues);

    if (bResult) {
        CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        if ((dwFlags & 0x40) == 0) {
            RenderTexture(x,
                y,
                CRect(0, 0, size.cx, size.cy),
                rFXRect.Size(),
                rClip,
                dwFlags);
        }
    }

    if (!bDemanded) {
        pRes->Release();
    }

    m_pFrame = NULL;

    return bResult;
}

// 0x58FC70
void CVidCell::SetResRef(const CResRef& cNewResRef, BOOL bDoubleSize, BOOL bSetAutoRequest, BOOL bWarningIfMissing)
{
    CResHelper<CResCell, 1000>::SetResRef(cNewResRef, bSetAutoRequest, bWarningIfMissing);
    m_header.SetResRef(cNewResRef, bSetAutoRequest, FALSE);

    if (pRes != NULL) {
        pRes->m_bCacheHeader = m_header.GetResRef() == "";
    }

    m_bDoubleSize = bDoubleSize;
}

// 0x45B2B0
WORD CVidCell::GetCurrentFrameId()
{
    return m_nCurrentFrame;
}

// 0x45B2C0
WORD CVidCell::GetCurrentSequenceId()
{
    return m_nCurrentSequence;
}

// NOTE: Inlined in many places. Seen in `CVidCell::GetFrameCenterPoint`
// assertion.
BAMHEADER* CVidCell::GetResBamHeader()
{
    if (m_header.GetResRef() != "") {
        return m_header.pRes->m_pBamHeaderCopy;
    } else {
        if (pRes->m_bCacheHeader) {
            return pRes->m_pBamHeaderCopy;
        } else {
            return pRes->m_pBamHeader;
        }
    }
}

// NOTE: Inlined in many places.
FRAMEENTRY* CVidCell::GetResFrame(WORD nSequence, WORD nFrame)
{
    if (m_header.GetResRef() != "") {
        return m_header.pRes->GetFrame(nSequence, nFrame, m_bDoubleSize);
    } else {
        return pRes->GetFrame(nSequence, nFrame, m_bDoubleSize);
    }
}

// NOTE: Inlined in many places. Seen in `CVidCell::GetFrameCenterPoint`
// assertion.
SEQUENCEENTRY* CVidCell::GetResSequences()
{
    if (m_header.GetResRef() != "") {
        return m_header.pRes->m_pSequences;
    } else {
        return pRes->m_pSequences;
    }
}

// NOTE: Inlined in many places.
void CVidCell::Release(BOOLEAN& bDemanded)
{
    if (!bDemanded) {
        if (m_header.GetResRef() != "") {
            m_header.Release();
        } else {
            pRes->Release();
        }
    }
}

// NOTE: Inlined.
void CVidCell::UnsuppressTintAllRanges()
{
    m_paletteAffects.suppressTints = 0;
}
