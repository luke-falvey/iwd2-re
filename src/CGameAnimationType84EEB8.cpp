#include "CGameAnimationType84EEB8.h"

#include <stdlib.h>

#include "CGameSprite.h"
#include "CInfinity.h"
#include "CProjectileArea.h"
#include "CUtil.h"

// 0x55D3A0
CGameAnimationType84EEB8::CGameAnimationType84EEB8(const CProjectileAreaVisual3* a1, SHORT nDirection)
    : m_palette(CVidPalette::TYPE_RANGE)
{
    field_5DE = 0;
    field_5DF = 0;
    field_5E1 = 0;
    m_currentBamSequence = 0;
    m_currentBamDirection = 0;
    m_extendDirectionTest = 0;
    m_animationID = 0;
    m_visualEffect = a1->field_20;
    field_5E8 = a1->field_36;
    field_5E7 = a1->field_35;
    m_colorChunks = -1;
    m_bRender = TRUE;
    m_pSndDeath = "";

    if (!a1->field_0.empty()) {
        const char* pName = a1->field_0.c_str();

        // NOTE: Uninline.
        m_g1VidCell.SetResRef(CResRef(CString(pName)), FALSE, TRUE, TRUE);

        if (field_5E7 == 1) {
            BYTE nSequences = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
            if (nSequences == 0) {
                m_currentBamSequence = 0;
            } else {
                m_currentBamSequence = rand() % nSequences;
            }
        } else {
            m_currentBamSequence = 0;
        }

        m_g1VidCell.SequenceSet(m_currentBamSequence);
        m_g1VidCell.FrameSet(0);

        field_5F6 = pName;
    } else {
        // NOTE: Uninline.
        m_g1VidCell.SetResRef(CResRef("GreaseA"), FALSE, TRUE, TRUE);
    }

    m_currentVidCell = &m_g1VidCell;
    m_currentVidCellShadow = NULL;
    m_currentBamDirection = nDirection;
    m_extendDirectionTest = CGameSprite::DIR_N;
}

// 0x55CD70
CGameAnimationType84EEB8::CGameAnimationType84EEB8(const CProjectileAreaVisual2* a1, SHORT nDirection)
    : m_palette(CVidPalette::TYPE_RANGE)
{
    field_5DE = 0;
    field_5DF = 0;
    field_5E1 = 0;
    m_currentBamSequence = 0;
    m_currentBamDirection = 0;
    m_extendDirectionTest = 0;
    m_animationID = 0;
    m_visualEffect = a1->field_20;
    field_5E8 = FALSE;
    field_5E7 = a1->field_2C;
    m_colorChunks = -1;
    m_bRender = TRUE;
    m_pSndDeath = "";

    if (!a1->field_0.empty()) {
        const char* pName = a1->field_0.c_str();

        // NOTE: Uninline.
        m_g1VidCell.SetResRef(CResRef(CString(pName)), FALSE, TRUE, TRUE);

        if (field_5E7 == 1) {
            BYTE nSequences = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
            if (nSequences == 0) {
                m_currentBamSequence = 0;
            } else {
                m_currentBamSequence = rand() % nSequences;
            }
        } else {
            m_currentBamSequence = 0;
        }

        m_g1VidCell.SequenceSet(m_currentBamSequence);
        m_g1VidCell.FrameSet(0);

        field_5F6 = pName;
    } else {
        // NOTE: Uninline.
        m_g1VidCell.SetResRef(CResRef("GreaseA"), FALSE, TRUE, TRUE);
    }

    m_currentVidCell = &m_g1VidCell;
    m_currentVidCellShadow = NULL;
    m_currentBamDirection = nDirection;
    m_extendDirectionTest = CGameSprite::DIR_N;
}

// 0x55D2E0
// NOTE: The scalar deleting destructor (0x55D2C0) is excluded from the
// inventory.
CGameAnimationType84EEB8::~CGameAnimationType84EEB8()
{
}

// 0x55DBD0
void CGameAnimationType84EEB8::sub_55DBD0(const char* pName)
{
    // NOTE: Uninline.
    m_g1VidCell.SetResRef(CResRef(CString(pName)), FALSE, TRUE, TRUE);

    if (field_5E7 == 1) {
        BYTE nSequences = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
        if (nSequences == 0) {
            m_currentBamSequence = 0;
        } else {
            m_currentBamSequence = rand() % nSequences;
        }
    } else {
        m_currentBamSequence = 0;
    }

    m_g1VidCell.SequenceSet(m_currentBamSequence);
    m_g1VidCell.FrameSet(0);

    field_5F6 = pName;
}

// 0x55D690
void CGameAnimationType84EEB8::CalculateFxRect(CRect& rFx, CPoint& ptReference, LONG posZ)
{
    CPoint ptCenter;
    CSize size;
    CPoint ptShadowCenter;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameAnimationTypeEffect.cpp
    // __LINE__: 157
    UTIL_ASSERT(m_currentVidCell != NULL);

    m_currentVidCell->GetCurrentCenterPoint(ptCenter, FALSE);

    if (m_currentVidCellShadow != NULL) {
        ptCenter.y += posZ;
    }

    ptReference.x = ptCenter.x;
    ptReference.y = ptCenter.y;

    m_currentVidCell->GetCurrentFrameSize(size, FALSE);

    if (m_currentVidCellShadow == NULL) {
        rFx.SetRect(0, 0, size.cx, size.cy);
        return;
    }

    m_currentVidCellShadow->GetCurrentCenterPoint(ptShadowCenter, FALSE);

    if (ptShadowCenter.x > ptReference.x) {
        ptReference.x = ptShadowCenter.x;
    }

    if (ptShadowCenter.y > ptReference.y) {
        ptReference.y = ptShadowCenter.y;
    }

    size.cx += ptReference.x - ptCenter.x;
    size.cy += ptReference.y - ptCenter.y;
    rFx.SetRect(0, 0, size.cx, size.cy);

    m_currentVidCellShadow->GetCurrentFrameSize(size, FALSE);

    LONG right = size.cx + (ptReference.x - ptShadowCenter.x);
    LONG bottom = size.cy + (ptReference.y - ptShadowCenter.y);

    if (right > rFx.right) {
        rFx.right = right;
    }

    if (bottom > rFx.bottom) {
        rFx.bottom = bottom;
    }

    if (m_currentVidCellShadow != NULL) {
        LONG y1 = ptCenter.y - posZ;
        LONG y2 = ptShadowCenter.y + posZ;
        ptReference.y = y1 > y2 ? y1 : y2;
    }
}

// NOTE: Shares its body with 0x6A33F0 (`CGameAnimationTypeEffect`).
void CGameAnimationType84EEB8::CalculateGCBoundsRect(CRect& rGCBounds, const CPoint& pos, const CPoint& ptReference, LONG posZ, LONG nWidth, LONG nHeight)
{
    if (m_currentBamDirection > m_extendDirectionTest) {
        rGCBounds.left = ptReference.x + pos.x - nWidth;
    } else {
        rGCBounds.left = pos.x - ptReference.x;
    }

    rGCBounds.top = posZ + pos.y - ptReference.y;
    rGCBounds.right = rGCBounds.left + nWidth;
    rGCBounds.bottom = rGCBounds.top + nHeight;
}

// 0x55D800
void CGameAnimationType84EEB8::ChangeDirection(SHORT nDirection)
{
    m_currentBamDirection = nDirection;
}

// NOTE: Shares its body with 0x6A3C60.
BOOLEAN CGameAnimationType84EEB8::DetectedByInfravision()
{
    return FALSE;
}

// NOTE: Body at 0x55D2A0; the address is annotated in CGameAnimationTypeEffect.cpp.
BOOL CGameAnimationType84EEB8::IsMirroring()
{
    return m_currentBamDirection > m_extendDirectionTest;
}

// 0x55D260
BOOL CGameAnimationType84EEB8::IsEndOfSequence()
{
    // __FILE__: .\Include\IcewindCGameAnimationTypeEffect.h
    // __LINE__: 38
    UTIL_ASSERT(m_currentVidCell != NULL);

    return m_currentVidCell->IsEndOfSequence(FALSE);
}

// 0x55D810
void CGameAnimationType84EEB8::IncrementFrame()
{
    if (m_currentVidCell->IsEndOfSequence(FALSE)) {
        if (field_5E8 == 1) {
            m_bRender = FALSE;
            return;
        }

        if (field_5E7 == 1) {
            BYTE nSequences = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
            if (nSequences == 0) {
                m_currentBamSequence = 0;
            } else {
                m_currentBamSequence = rand() % nSequences;
            }

            m_g1VidCell.SequenceSet(m_currentBamSequence);
            m_g1VidCell.FrameSet(0);

            if (m_currentVidCellShadow != NULL) {
                m_g1VidCellShadow.SequenceSet(m_currentBamSequence);
                m_g1VidCellShadow.FrameSet(0);
            }
        } else {
            m_currentBamSequence++;

            BYTE nSequences = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
            if (m_currentBamSequence >= nSequences) {
                nSequences = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
                m_currentBamSequence = nSequences - 1;
            }

            m_g1VidCell.SequenceSet(m_currentBamSequence);
            m_g1VidCell.FrameSet(0);

            if (m_currentVidCellShadow != NULL) {
                m_g1VidCellShadow.SequenceSet(m_currentBamSequence);
                m_g1VidCellShadow.FrameSet(0);
            }
        }
    } else {
        m_currentVidCell->FrameAdvance();

        if (m_currentVidCellShadow != NULL) {
            m_currentVidCellShadow->FrameAdvance();
        }
    }
}

// 0x55D950
void CGameAnimationType84EEB8::Render(CInfinity* pInfinity, CVidMode* pVidMode, INT nSurface, const CRect& rectFX, const CPoint& ptNewPos, const CPoint& ptReference, DWORD dwRenderFlags, COLORREF rgbTintColor, const CRect& rGCBounds, BOOL bDithered, BOOL bFadeOut, LONG posZ, BYTE transparency)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameAnimationTypeEffect.cpp
    // __LINE__: 262
    UTIL_ASSERT(pInfinity != NULL && pVidMode != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameAnimationTypeEffect.cpp
    // __LINE__: 263
    UTIL_ASSERT(m_currentVidCell != NULL);

    if (m_bRender) {
        CPoint ptPos(ptNewPos.x, ptNewPos.y + posZ);
        CRect rFXRect(rectFX);

        if (m_currentVidCellShadow == NULL
            && !m_visualEffect.field_3
            && !m_visualEffect.field_2
            && !m_visualEffect.field_1) {
            dwRenderFlags |= CInfinity::FXPREP_CLEARFILL;
        } else {
            dwRenderFlags |= CInfinity::FXPREP_COPYFROMBACK;
        }

        pInfinity->FXPrep(rFXRect, dwRenderFlags, nSurface, ptPos, ptReference);

        if (pInfinity->FXLock(rFXRect, dwRenderFlags)) {
            if (m_visualEffect.field_0 == TRUE) {
                m_currentVidCell->SetTintColor(rgbTintColor);
            }

            if (m_currentVidCellShadow != NULL) {
                pInfinity->FXRender(m_currentVidCellShadow,
                    ptReference.x,
                    ptReference.y - posZ,
                    m_visualEffect.m_dwFlags | 0x4,
                    0);
            }

            if (m_visualEffect.field_3 == TRUE) {
                pInfinity->FXRender(m_currentVidCell,
                    ptReference.x,
                    ptReference.y,
                    m_visualEffect.m_dwFlags,
                    m_visualEffect.m_nTransValue);
            } else {
                pInfinity->FXRender(m_currentVidCell,
                    ptReference.x,
                    ptReference.y,
                    m_visualEffect.m_dwFlags,
                    0);
            }

            // NOTE: Both branches make the same call.
            if (m_currentVidCellShadow != NULL) {
                pInfinity->FXRenderClippingPolys(ptPos.x,
                    ptPos.y,
                    0,
                    ptReference,
                    rGCBounds,
                    bDithered,
                    m_visualEffect.m_dwFlags);
            } else {
                pInfinity->FXRenderClippingPolys(ptPos.x,
                    ptPos.y,
                    0,
                    ptReference,
                    rGCBounds,
                    bDithered,
                    m_visualEffect.m_dwFlags);
            }

            if (bFadeOut) {
                pInfinity->FXUnlock(dwRenderFlags, &rFXRect, ptPos + ptReference);
            } else {
                pInfinity->FXUnlock(dwRenderFlags, NULL, CPoint(0, 0));
            }

            pInfinity->FXBltFrom(nSurface,
                rFXRect,
                ptPos.x,
                ptPos.y,
                ptReference.x,
                ptReference.y,
                m_visualEffect.m_dwFlags | 0x1);
        }
    }
}

// NOTE: Body at 0x55DBC0; the address is annotated in CGameAnimationTypeEffect.cpp.
SHORT CGameAnimationType84EEB8::SetSequence(SHORT nSequence)
{
    return CGameSprite::SEQ_READY;
}
