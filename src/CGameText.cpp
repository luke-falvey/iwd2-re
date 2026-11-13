#include "CGameText.h"

#include "CBaldurChitin.h"
#include "CGameAnimation.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CInfinity.h"
#include "CUtil.h"
#include "CVidMode.h"
#include "CWarp.h"

// 0x8D6570
const CSize CGameText::PADDING(0, 0);

// 0x4CB340
CGameText::CGameText(CGameArea* pArea, const CPoint& pt, BYTE nDuration, BYTE nBeginFade, const CString& sText)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameText.cpp
    // __LINE__: 110
    UTIL_ASSERT(pArea != NULL);

    m_nDuration = 0;
    m_nBeginFade = 0;
    m_nMaxLines = 0;
    m_objectType = TYPE_TEXT;
    m_szLine = NULL;
    m_nLines = 0;

    m_textFont.SetResRef(CResRef("INFOFONT"), FALSE, TRUE);
    m_vidCell.SequenceSet(0);
    m_vidCell.FrameSet(0);

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) == CGameObjectArray::SUCCESS) {
        AddToArea(pArea, pt, 0, LIST_FLIGHT);
        SetText(pt, nDuration, nBeginFade, sText);
        field_1C = 0;
    } else {
        // FIXME: Doesn't look cool.
        delete this;
    }
}

// 0x4CB4B0
void CGameText::SetText(const CPoint& pt, BYTE nDuration, BYTE nBeginFade, const CString& sText)
{
    CString sMutableText(sText);
    sMutableText.TrimRight();

    CSize size;
    if (m_vidCell.GetRes() != NULL) {
        m_vidCell.GetCurrentFrameSize(size, FALSE);
    } else {
        g_pBaldurChitin->GetCurrentVideoMode()->GetFXSize(size);
        size.cx = size.cx / 2 - 1;
        size.cy = size.cy / 2 - 1;
    }

    if (m_textFont.GetRes()->Demand() != NULL) {
        m_textFont.SetColor(RGB(255, 255, 255), RGB(0, 0, 0), TRUE);

        m_nMaxLines = static_cast<BYTE>((size.cy - 2 * PADDING.cy) / m_textFont.GetFontHeight(TRUE));

        if (m_szLine != NULL) {
            delete[] m_szLine;
        }

        m_szLine = new CString[m_nMaxLines];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameText.cpp
        // __LINE__: 222
        UTIL_ASSERT(m_szLine != NULL);

        m_nLines = CUtil::SplitString(&m_textFont,
            sMutableText,
            static_cast<WORD>(size.cx - 2 * PADDING.cx),
            m_nMaxLines,
            m_szLine,
            FALSE,
            TRUE,
            TRUE,
            -1);

        m_textFont.GetRes()->Release();
    }

    m_nDuration = 5 * nDuration * m_nLines;

    if (m_nDuration < 45) {
        m_nDuration = 65;
    }

    if (sText.GetLength() > 255) {
        m_nDuration = 500;
    }

    m_nBeginFade = nBeginFade;
    m_pos = pt;
}

// 0x4CB700
BOOLEAN CGameText::DoAIUpdate(BOOLEAN active, LONG counter)
{
    return (counter & m_AISpeed) == (m_AISpeed & m_id);
}

// 0x4CB720
void CGameText::AIUpdate()
{
    if (m_nDuration == 0) {
        RemoveFromArea();
        return;
    }

    if (m_nDuration > 0) {
        m_nDuration--;
    }

    if (m_vidCell.GetRes() == NULL) {
        if (m_nDuration < m_nBeginFade) {
            LONG nScale = 155 * m_nDuration / m_nBeginFade + 100;
            m_textFont.SetColor(RGB(nScale, nScale, nScale), 0, TRUE);
        }
    }

    m_vidCell.FrameAdvance();
}

// 0x4CB7A0
void CGameText::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    // NOTE: This implementation is slightly different from other `CGameObject`
    // subclasses (looping until success).
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            NULL,
            INFINITE);
    } while (rc != CGameObjectArray::SUCCESS);

    delete this;
}

// 0x4CB820
void CGameText::sub_4CB820(const CPoint& pt)
{
    CVidMode* pVidMode = g_pBaldurChitin->pActiveEngine != NULL
        ? g_pBaldurChitin->pActiveEngine->pVidMode
        : NULL;

    m_pos = pt;

    CInfinity* pInfinity = &(m_pArea->m_cInfinity);

    CSize size;
    CPoint ptCenter;
    if (m_vidCell.GetRes() != NULL) {
        m_vidCell.GetCurrentCenterPoint(ptCenter, FALSE);
        m_vidCell.GetCurrentFrameSize(size, FALSE);
    } else {
        pVidMode->GetFXSize(size);
        size.cx /= 2;
        size.cy /= 2;

        if (static_cast<BYTE>(m_nLines) == 1) {
            ptCenter.x = m_textFont.GetStringLength(m_szLine[0], FALSE) / 2;
        } else {
            ptCenter.x = size.cx / 2;
        }

        ptCenter.y = static_cast<BYTE>(m_nLines) * size.cy / m_nMaxLines / 2;
    }

    LONG nLeft = m_pos.x - ptCenter.x;
    LONG nRight = m_pos.x + ptCenter.x;
    LONG nTop = m_pos.y - m_posZ - ptCenter.y;
    LONG nBottom = m_pos.y - m_posZ + ptCenter.y;

    LONG nViewLeft = pInfinity->nCurrentX;
    LONG nViewTop = pInfinity->nCurrentY;
    LONG nViewRight = pInfinity->rViewPort.right - pInfinity->rViewPort.left + nViewLeft;
    LONG nViewBottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + nViewTop;

    LONG dx = 0;
    LONG dy = 0;

    if (nLeft < nViewLeft) {
        dx = nViewLeft - nLeft;
    }

    if (nRight > nViewRight) {
        dx = nViewRight - nRight;
    }

    if (nBottom > nViewBottom) {
        dy = nViewBottom - nBottom;
    }

    if (nTop < nViewTop) {
        dy = nViewTop - nTop;
    }

    m_pos.x += dx;
    m_pos.y += dy;
}

// 0x4CB9B0
BOOL CGameText::sub_4CB9B0()
{
    // NOTE: Unlike `sub_4CB820` there is no `NULL` check here.
    CVidMode* pVidMode = g_pBaldurChitin->pActiveEngine->pVidMode;
    CInfinity* pInfinity = &(m_pArea->m_cInfinity);

    if (static_cast<BYTE>(m_nLines) == 0) {
        return FALSE;
    }

    CSize size;
    CPoint ptCenter;
    if (m_vidCell.GetRes() != NULL) {
        m_vidCell.GetCurrentCenterPoint(ptCenter, FALSE);
        m_vidCell.GetCurrentFrameSize(size, FALSE);
    } else {
        pVidMode->GetFXSize(size);
        size.cx /= 2;
        size.cy /= 2;

        if (static_cast<BYTE>(m_nLines) == 1) {
            ptCenter.x = m_textFont.GetStringLength(m_szLine[0], FALSE) / 2;
        } else {
            ptCenter.x = size.cx / 2;
        }

        ptCenter.y = static_cast<BYTE>(m_nLines) * size.cy / m_nMaxLines / 2;
    }

    CRect rText(m_pos.x - ptCenter.x,
        m_pos.y - m_posZ - ptCenter.y,
        m_pos.x + ptCenter.x,
        m_pos.y - m_posZ + ptCenter.y);

    CRect rView;
    rView.left = pInfinity->nCurrentX;
    rView.top = pInfinity->nCurrentY;
    rView.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + rView.left;
    rView.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + rView.top;

    return rView.IntersectRect(rText, rView);
}

// 0x4CBB20
void CGameText::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    CInfinity* pInfinity = &(m_pArea->m_cInfinity);
    BOOLEAN bFade = FALSE;
    DWORD dwFlags = CInfinity::FXPREP_CLEARFILL | 0x20001;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameText.cpp
    // __LINE__: 550
    UTIL_ASSERT(pVidMode != NULL);

    if (static_cast<BYTE>(m_nLines) == 0) {
        return;
    }

    CSize size;
    CPoint ptCenter;
    CRect rFx;
    if (m_vidCell.GetRes() != NULL) {
        if (m_nDuration < m_nBeginFade) {
            bFade = TRUE;
        }

        m_vidCell.GetCurrentCenterPoint(ptCenter, FALSE);
        m_vidCell.GetCurrentFrameSize(size, FALSE);
        rFx.SetRect(0, 0, size.cx, size.cy);
    } else {
        pVidMode->GetFXSize(size);
        size.cx /= 2;
        size.cy /= 2;
        rFx.SetRect(0, 0, size.cx - 1, size.cy - 1);

        if (static_cast<BYTE>(m_nLines) == 1) {
            ptCenter.x = m_textFont.GetStringLength(m_szLine[0], FALSE) / 2;
        } else {
            ptCenter.x = size.cx / 2;
        }

        ptCenter.y = static_cast<BYTE>(m_nLines) * size.cy / m_nMaxLines / 2;
    }

    CRect rClip(PADDING.cx,
        PADDING.cy,
        rFx.right - PADDING.cx,
        rFx.bottom - PADDING.cy);

    CRect rText(m_pos.x - ptCenter.x,
        m_pos.y - m_posZ - ptCenter.y,
        m_pos.x + ptCenter.x,
        m_pos.y - m_posZ + ptCenter.y);

    CRect rView;
    rView.left = pInfinity->nCurrentX;
    rView.top = pInfinity->nCurrentY;
    rView.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + rView.left;
    rView.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + rView.top;

    if (!rView.IntersectRect(rText, rView)) {
        return;
    }

    if (field_1C != 0) {
        // NOTE: Unused.
        CString sUnused;

        CGameObjectArray* pObjectArray = g_pBaldurChitin->GetObjectGame()->GetObjectArray();
        CGameObject* pObject;

        BYTE rc;
        do {
            rc = pObjectArray->GetShare(field_1C,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        CPoint& ptPos = pObject->GetPos();
        LONG nX = ptPos.x;
        LONG nY = ptPos.y;

        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameAnimation* pAnimation = &(static_cast<CGameSprite*>(pObject)->m_animation);
            if (pAnimation->GetAnimationId() == 0xE048) {
                m_pos.x = nX;
                m_pos.y = nY + 5 * (static_cast<BYTE>(m_nLines) - 20);
            } else {
                BYTE nPersonalSpace = pAnimation->GetPersonalSpace();
                m_pos.x = nX;
                m_pos.y = nY + 5 * (static_cast<BYTE>(m_nLines) - 7 * nPersonalSpace);
            }
        } else if (pObject->GetObjectType() != CGameObject::TYPE_TRIGGER) {
            m_pos.x = nX;
            m_pos.y = nY + 5 * (-3 - static_cast<BYTE>(m_nLines));
        }

        pObjectArray->ReleaseShare(field_1C,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    CRect rShadow;
    rShadow.left = m_pos.x - ptCenter.x - 4;
    rShadow.top = m_pos.y - m_posZ - ptCenter.y - 2;
    rShadow.right = m_pos.x + ptCenter.x + 4;
    rShadow.bottom = m_pos.y - m_posZ + ptCenter.y + 2;
    pInfinity->ShadowRect(rShadow);

    CPoint ptPos(m_pos.x, m_pos.y - m_posZ);
    pInfinity->FXPrep(rFx, dwFlags, nSurface, ptPos, ptCenter);

    if (!pInfinity->FXLock(rFx, dwFlags)) {
        return;
    }

    if (m_vidCell.GetRes() != NULL) {
        pInfinity->FXRender(&m_vidCell, ptCenter.x, ptCenter.y, dwFlags, 0);
    }

    if (m_textFont.GetRes()->Demand() != NULL) {
        for (BYTE nLine = 0; nLine < static_cast<BYTE>(m_nLines); nLine++) {
            pInfinity->FXTextOut(&m_textFont,
                m_szLine[nLine],
                PADDING.cx,
                PADDING.cy + m_textFont.GetFontHeight(TRUE) * nLine + m_textFont.GetBaseLineHeight(TRUE),
                rClip,
                dwFlags,
                TRUE);
        }

        m_textFont.GetRes()->Release();
    }

    if (bFade) {
        pInfinity->FXUnlock(dwFlags, &rFx, CPoint(m_pos.x + ptCenter.x, m_pos.y + ptCenter.y));
    } else {
        pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
    }

    pInfinity->FXBltFrom(nSurface,
        rFx,
        m_pos.x,
        m_pos.y - m_posZ,
        ptCenter.x,
        ptCenter.y,
        dwFlags | 1);
}

// 0x4CC0D0
CGameText::~CGameText()
{
    if (m_szLine != NULL) {
        delete[] m_szLine;
    }

    if (field_1C != 0) {
        CGameObject* pObject;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(field_1C,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pObject->field_1C = 0;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(field_1C,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x47C830
BOOLEAN CGameText::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}
