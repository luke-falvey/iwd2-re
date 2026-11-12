#include "CUIControlEditMultiLine.h"

#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CUIControlScrollBar.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "CVidInf.h"

// 0x4D9410
CUIControlEditMultiLine::CUIControlEditMultiLine(CUIPanel* panel, UI_CONTROL_EDIT* controlInfo, int a3)
    : CUIControlBase(panel, &(controlInfo->base), 0)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ChUIControls.cpp
    // __LINE__: 2458
    UTIL_ASSERT(panel != NULL && controlInfo != NULL);

    if (m_pPanel->m_pManager->m_bDoubleSize) {
        m_ptText.x = 2 * controlInfo->x;
        m_ptText.y = 2 * controlInfo->y;
    } else {
        m_ptText.x = controlInfo->x;
        m_ptText.y = controlInfo->y;
    }

    field_350 = 0;
    field_354 = 0;
    m_nTextCapitalization = controlInfo->nTextCapitalization;
    field_85A = controlInfo->field_68;
    m_nRenderCount = 0;
    m_bFocused = FALSE;
    m_sText = controlInfo->initialText;
    field_86A = -1;
    field_86E = 0;
    field_877 = 1;

    CResRef cursorResRef(controlInfo->refCursor);
    m_vcCursor.SetResRef(cursorResRef, m_pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);

    m_vcCursor.m_bDoubleSize = m_pPanel->m_pManager->m_bDoubleSize;

    m_vcCursor.SequenceSet(controlInfo->nSequence);
    m_vcCursor.FrameSet(controlInfo->nFrame);

    m_cVidFont.SetResRef(CResRef(controlInfo->refFont), m_pPanel->m_pManager->m_bDoubleSize, FALSE);
    m_cVidFont.SetColor(RGB(200, 200, 200), RGB(60, 60, 60), 0);

    field_872 = a3;
    field_88A = -1;
    field_88E = -1;
    field_87A = 0;
    field_878 = 0;
    field_888 = 0;
    m_rgbForegroundColor = RGB(200, 200, 200);
    m_rgbBackgroundColor = RGB(60, 60, 60);
    field_36 = 1;
    field_876 = 1;

    // NOTE: Uninline.
    SetNeedMouseMove();

    m_size.cx -= m_ptText.x;
    m_size.cy -= m_ptText.y;
}

// 0x4D96C0
void CUIControlEditMultiLine::SetColor(COLORREF rgbForegroundColor, COLORREF rgbBackgroundColor)
{
    m_rgbForegroundColor = rgbForegroundColor;
    m_rgbBackgroundColor = rgbBackgroundColor;
}

// 0x4D96E0
void CUIControlEditMultiLine::OnEditReturn(CString sText)
{
}

// 0x4D9740
CUIControlEditMultiLine::~CUIControlEditMultiLine()
{
}

// 0x4D9800
void CUIControlEditMultiLine::Remove()
{
    m_sText.Empty();
    field_86A = 0;
    InvalidateRect();
}

// 0x4D9830
int CUIControlEditMultiLine::ReConstitute()
{
    CString sLine;
    int nLines = 0;

    if (field_877) {
        m_cVidFont.Load(FALSE);
        SHORT nFontHeight = m_cVidFont.GetFontHeight(FALSE);
        field_877 = 0;
        field_878 = static_cast<SHORT>((m_size.cy - m_ptText.y) / nFontHeight);
    }

    m_cVidFont.pRes->Demand();

    int nWidth = m_size.cx - 10;

    int nPos = CUtil::Find(m_sText, '\r', 0);
    while (nPos != -1) {
        m_sText = m_sText.Left(nPos) + m_sText.Mid(nPos + 1);

        if (nPos < field_86A) {
            field_86A--;
        }

        nPos = CUtil::Find(m_sText, '\r', 0);
    }

    int nStart = 0;
    sLine.Empty();

    while (nStart < m_sText.GetLength()) {
        int nEnd = CUtil::Find(m_sText.Mid(nStart), '\n', 0);
        if (nEnd == -1) {
            nEnd = m_sText.GetLength();
        }

        sLine = m_sText.Mid(nStart, nEnd);

        if (m_cVidFont.GetStringLength(sLine, TRUE) < nWidth) {
            nStart = nStart + nEnd + 1;
            if (nStart >= m_sText.GetLength()) {
                nStart = m_sText.GetLength();
            }
            nLines++;
        } else {
            int nLength = sLine.GetLength();
            while (m_cVidFont.GetStringLength(sLine.Left(nLength), TRUE) >= nWidth) {
                CString sTemp = sLine.Left(nLength);

                BOOL bFound = FALSE;
                int nSpace = CUtil::Find(sTemp, ' ', 0);
                if (nSpace >= 0) {
                    int nLastSpace;
                    do {
                        nLastSpace = nSpace;
                        nSpace = CUtil::Find(sTemp, ' ', nLastSpace + 1);
                    } while (nSpace >= 0);

                    if (nLastSpace > 0) {
                        nLength = nLastSpace;
                        bFound = TRUE;
                    }
                }

                if (!bFound) {
                    if (g_pChitin->field_1A0) {
                        int nPrev = nLength - 1;
                        if (CUtil::IsDBCSTrailByte(sLine, nPrev)) {
                            nLength = nPrev;
                        }
                    }

                    nLength--;
                }

                // __FILE__: C:\Projects\Icewind2\src\Baldur\ChUIControls.cpp
                // __LINE__: 2660
                UTIL_ASSERT(nLength > 0);
            }

            nPos = nLength + nStart + 1;

            m_sText = m_sText.Left(nPos) + '\r' + m_sText.Right(m_sText.GetLength() - nLength - nStart - 1);

            if (nPos < field_86A) {
                field_86A++;
            }

            if (nPos >= m_sText.GetLength()) {
                nPos = m_sText.GetLength();
            }

            nStart = nPos;
            nLines++;
        }
    }

    BOOL bExtraLine = m_sText.Right(1) == "\n"
        && (!g_pChitin->field_1A0 || !CUtil::IsDBCSTrailByte(m_sText, m_sText.GetLength() - 1));

    if (bExtraLine) {
        nLines++;
    }

    field_87A = static_cast<short>(nLines);

    AdjustVisibleIndex();

    field_86A = min(field_86A, m_sText.GetLength());

    m_cVidFont.pRes->Release();

    return nLines;
}

// 0x4D9D00
int CUIControlEditMultiLine::CalculateNewLines(BYTE a1)
{
    CString sLine;

    if (field_877) {
        m_cVidFont.Load(FALSE);
        SHORT nFontHeight = m_cVidFont.GetFontHeight(FALSE);
        field_877 = 0;
        field_878 = static_cast<SHORT>((m_size.cy - m_ptText.y) / nFontHeight);
    }

    m_cVidFont.pRes->Demand();

    int nWidth = m_size.cx - 10;

    if (g_pChitin->field_1A0) {
        a1 = 3;
    }

    switch (a1) {
    case 1: {
        if (field_86A == m_sText.GetLength()) {
            sLine = m_sText;
            CUtil::MakeReverse(sLine);

            int nLength = CUtil::FindOneOf(sLine, CString("\n\r"), 0);
            if (nLength == -1) {
                nLength = sLine.GetLength();
            }

            if (m_cVidFont.GetStringLength(m_sText.Right(nLength), TRUE) < nWidth) {
                m_cVidFont.pRes->Release();
                return field_87A;
            }

            m_cVidFont.pRes->Release();
            return ReConstitute();
        }

        sLine = m_sText.Right(m_sText.GetLength() - field_86A);

        int nToEnd = CUtil::FindOneOf(sLine, CString("\n\r"), 0);
        if (nToEnd == -1) {
            nToEnd = sLine.GetLength();
        }

        sLine = m_sText.Left(field_86A);
        CUtil::MakeReverse(sLine);

        int nBack = CUtil::FindOneOf(sLine, CString("\n\r"), 0);
        int nBackCopy = nBack;
        if (nBack == -1) {
            nBackCopy = sLine.GetLength();
            nBack = nBackCopy;
        }

        sLine = m_sText.Left(field_86A + nToEnd);
        sLine = sLine.Right(sLine.GetLength() - field_86A + nBack);

        if (m_cVidFont.GetStringLength(sLine, TRUE) < nWidth) {
            m_cVidFont.pRes->Release();
            return field_87A;
        }

        int nLineStart = field_86A - nBackCopy;
        int nEnd = nToEnd + field_86A;
        int nSpaceDist;

        for (;;) {
            sLine = m_sText.Left(nEnd - 1);
            CUtil::MakeReverse(sLine);

            int nSpace = CUtil::Find(sLine, ' ', 0);
            nSpaceDist = nSpace;
            if (nSpace == -1) {
                nSpaceDist = 1;
                nSpace = nSpaceDist;
            }

            if (nEnd - nSpace <= nLineStart) {
                nSpaceDist = 1;
                nSpace = nSpaceDist;
            }

            int nRightCount = m_sText.GetLength() - nEnd - 1;
            nEnd -= nSpace;
            nSpace = nEnd - 1;

            m_sText = m_sText.Left(nLineStart)
                + m_sText.Mid(nLineStart, nEnd - nLineStart - 1)
                + '\r'
                + m_sText.Mid(nSpace, nSpaceDist + 1)
                + m_sText.Right(nRightCount);

            nEnd = nSpace;

            sLine = m_sText.Mid(nLineStart, nEnd - nLineStart);

            if (field_86A > nEnd && field_86A <= nSpaceDist + nEnd + 1) {
                field_86A++;
            }

            if (m_cVidFont.GetStringLength(sLine, TRUE) > nWidth) {
                continue;
            }

            nLineStart = nEnd + 1;

            sLine = m_sText.Right(m_sText.GetLength() - nLineStart);

            int nNext = CUtil::FindOneOf(sLine, CString("\n\r"), 0);
            BOOL bNotFound = nNext == -1;
            if (bNotFound) {
                nNext = sLine.GetLength();
            }

            nEnd = nNext + nLineStart;

            sLine = m_sText.Mid(nLineStart, nEnd - nLineStart);

            if (m_cVidFont.GetStringLength(sLine, TRUE) > nWidth) {
                continue;
            }

            break;
        }

        m_cVidFont.pRes->Release();
        AdjustVisibleIndex();
        return field_87A;
    }
    case 2: {
        sLine = m_sText.Left(field_86A);
        CUtil::MakeReverse(sLine);

        int nBack = CUtil::FindOneOf(sLine, CString("\n\r"), 0);
        int nPrevStart;
        int nLineStart;

        if (nBack == -1) {
            nPrevStart = 0;
            nLineStart = CUtil::FindOneOf(m_sText, CString("\n\r"), 0);
            if (nLineStart == -1) {
                nLineStart = m_sText.GetLength();
            }
        } else {
            sLine = sLine.Right(sLine.GetLength() - nBack - 1);

            nPrevStart = CUtil::FindOneOf(sLine, CString("\n\r"), 0);
            if (nPrevStart == -1) {
                nPrevStart = sLine.GetLength();
            }

            nLineStart = field_86A - nBack;
            nPrevStart = nLineStart - nPrevStart - 1;
        }

        int nWordEnd;
        for (;;) {
            sLine = m_sText.Right(m_sText.GetLength() - nLineStart);

            nWordEnd = CUtil::FindOneOf(sLine, CString(" \n\r"), 0);
            if (nWordEnd == -1) {
                nWordEnd = sLine.GetLength();
            }

            if (nWordEnd == 0) {
                m_cVidFont.pRes->Release();
                return ReConstitute();
            }

            if (m_sText[nLineStart - 1] == '\r') {
                sLine = m_sText.Mid(nPrevStart, nWordEnd - nPrevStart + nLineStart + 1);

                if (m_cVidFont.GetStringLength(sLine, TRUE) > nWidth) {
                    nWordEnd = 0;
                } else {
                    m_sText = m_sText.Left(nLineStart - 1)
                        + m_sText.Mid(nLineStart, nWordEnd + 1)
                        + '\r'
                        + m_sText.Right(m_sText.GetLength() - nWordEnd - nLineStart - 1);

                    nLineStart = nLineStart + nWordEnd + 1;

                    if (m_cVidFont.GetStringLength(sLine, TRUE) <= nWidth) {
                        continue;
                    }
                }
            } else {
                nWordEnd = 0;
            }

            nWordEnd += nLineStart;
            nPrevStart = nWordEnd;

            sLine = m_sText.Right(m_sText.GetLength() - nWordEnd);

            nLineStart = CUtil::FindOneOf(sLine, CString("\n\r"), 0);
            if (nLineStart == -1 || sLine[nLineStart] == '\n') {
                break;
            }

            nLineStart = nLineStart + nWordEnd + 1;

            sLine = m_sText.Right(m_sText.GetLength() - nLineStart);

            nWordEnd = CUtil::FindOneOf(sLine, CString(" \n\r"), 0);
            if (nWordEnd == -1) {
                nWordEnd = sLine.GetLength();
            }

            nWordEnd -= nPrevStart;

            sLine = m_sText.Mid(nPrevStart, nLineStart + nWordEnd + 1);

            if (m_cVidFont.GetStringLength(sLine, TRUE) > nWidth) {
                break;
            }
        }

        m_cVidFont.pRes->Release();
        AdjustVisibleIndex();
        return field_87A;
    }
    case 3:
        m_cVidFont.pRes->Release();
        return ReConstitute();
    default:
        m_cVidFont.pRes->Release();
        return field_87A;
    }
}

// 0x4DAA10
CString CUIControlEditMultiLine::GetText()
{
    CString sText;
    sText.Empty();

    if (m_sText.GetLength() > 0) {
        int start = 0;
        int end;
        do {
            end = CUtil::Find(m_sText, '\r', start);
            if (end == -1) {
                end = m_sText.GetLength();
            }

            sText += m_sText.Mid(start, end);
            start = end + 1;
        } while (end < m_sText.GetLength());
    }

    return sText;
}

// 0x4DAAF0
void CUIControlEditMultiLine::KillFocus()
{
    // FIXME: Unused.
    CWnd* pWnd = g_pChitin->GetWnd();
    if (g_pChitin->cImm.field_128) {
        g_pChitin->cImm.sub_7C2E10(g_pChitin->GetWnd()->GetSafeHwnd());
        m_pPanel->InvalidateRect(&(m_pPanel->m_rImeSuggestionsFrame));
        m_pPanel->m_rImeSuggestionsFrame.SetRectEmpty();
        m_pPanel->m_pImeSuggestionsFont = NULL;
    }

    static_cast<CBaldurEngine*>(m_pPanel->m_pManager->m_pWarp)->DisableEditKeys();

    m_bFocused = FALSE;
    InvalidateRect();
}

// 0x4DAB80
void CUIControlEditMultiLine::SetFocus()
{
    m_bFocused = TRUE;
    field_866 = m_sText;

    InvalidateRect();

    static_cast<CBaldurEngine*>(m_pPanel->m_pManager->m_pWarp)->EnableEditKeys(field_85A);
}

// 0x4DABD0
BOOL CUIControlEditMultiLine::OnLButtonDown(CPoint pt)
{
    if (!m_bActive) {
        return FALSE;
    }

    int x = pt.x - (m_ptText.x + m_ptOrigin.x);
    int y = pt.y - (m_ptText.y + m_ptOrigin.y);

    if (x >= 0 && x < m_size.cx && y >= 0 && y < m_size.cy) {
        m_pPanel->m_pManager->SetCapture(this, CUIManager::MOUSELBUTTON);

        m_cVidFont.pRes->Demand();

        int nColumn = 0;

        // NOTE: `min` evaluates its arguments twice.
        int nTargetLine = min(field_86E + min(y / m_cVidFont.GetFontHeight(FALSE), static_cast<INT>(field_878)),
            static_cast<INT>(field_87A));

        int nIndex = 0;
        if (nTargetLine > 0) {
            for (int nLine = 0; nLine < nTargetLine && nIndex < m_sText.GetLength(); nLine++) {
                nIndex += CUtil::FindOneOf(m_sText.Mid(nIndex), CString("\n\r"), 0);
                BOOL bNotFound = nIndex == -1;
                if (bNotFound) {
                    nIndex = m_sText.GetLength();
                } else {
                    nIndex++;
                }
            }
        }

        if (nIndex == m_sText.GetLength()) {
            field_86A = m_sText.GetLength();
            m_cVidFont.pRes->Release();
            field_88E = field_86A;

            if (!m_pPanel->m_pManager->m_pWarp->GetShiftKey()) {
                field_88A = field_88E;
            }

            field_888 = 1;
        } else {
            int nLineEnd = CUtil::FindOneOf(m_sText.Mid(nIndex), CString("\n\r"), 0);
            BOOL bNotFound = nLineEnd == -1;
            if (bNotFound) {
                nLineEnd = m_sText.GetLength();
            }

            if (x > m_cVidFont.GetStringLength(m_sText.Mid(nIndex, nLineEnd), FALSE)) {
                nColumn = nLineEnd + 1;
            } else if (x > m_cVidFont.GetStringLength(m_sText.Mid(nIndex, 0), FALSE)) {
                do {
                    if (g_pChitin->field_1A0) {
                        if (IsDBCSLeadByte(m_sText[nIndex + nColumn - 1])) {
                            nColumn++;
                        }
                    }

                    nColumn++;

                    if (nColumn == m_sText.GetLength()) {
                        break;
                    }
                } while (x > m_cVidFont.GetStringLength(m_sText.Mid(nIndex, nColumn), FALSE));
            }

            m_cVidFont.pRes->Release();

            if (g_pChitin->field_1A0) {
                if (CUtil::IsDBCSTrailByte(m_sText, nIndex + nColumn - 1)) {
                    nColumn--;
                }
            }

            field_86A = min(nIndex + nColumn - 1, m_sText.GetLength());
            field_88E = field_86A;

            if (!m_pPanel->m_pManager->m_pWarp->GetShiftKey()) {
                field_88A = field_88E;
            }

            field_888 = 1;

            InvalidateRect();
        }
    } else if (m_bFocused == TRUE) {
        if (field_872 != 0) {
            m_sText = field_866;
            ReConstitute();
        }

        // NOTE: Uninline.
        m_pPanel->m_pManager->KillCapture();

        field_88A = -1;
        field_88E = -1;
    }

    return TRUE;
}

// 0x4DB070
void CUIControlEditMultiLine::OnLButtonUp(CPoint pt)
{
    if (m_bActive) {
        field_888 = 0;
        m_pPanel->m_pManager->SetCapture(this, 2);
        if (field_88E == field_88A) {
            field_86A = field_88E;
            field_88E = -1;
        }

        InvalidateRect();
    }
}

// 0x4DB0C0
BOOL CUIControlEditMultiLine::OnRButtonDown(CPoint pt)
{
    if (!m_bActive) {
        return FALSE;
    }

    int x = pt.x - (m_ptText.x + m_ptOrigin.x);
    int y = pt.y - (m_ptText.y + m_ptOrigin.y);

    if (x < 0 || x >= m_size.cx || y < 0 || y >= m_size.cy) {
        if (m_bFocused == TRUE) {
            if (field_872) {
                m_sText = field_866;
                ReConstitute();
            }

            // NOTE: Uninline.
            m_pPanel->m_pManager->KillCapture();
        }
    }

    return TRUE;
}

// 0x4DB160
void CUIControlEditMultiLine::OnKeyDown(SHORT nKey)
{
    // TODO: Incomplete.
}

// 0x4E05B0
void CUIControlEditMultiLine::OnMouseMove(CPoint pt)
{
    if (!m_bActive || !field_888) {
        return;
    }

    int x = pt.x - (m_ptText.x + m_ptOrigin.x);
    int y = pt.y - (m_ptText.y + m_ptOrigin.y);

    if (min(m_size.cx, x) < 1) {
        x = 1;
    } else if (m_size.cx < x) {
        x = m_size.cx;
    }

    if (y < 0) {
        if (field_86E > 0) {
            field_86E--;
            AdjustVisibleIndex();
        }

        y = 0;
    }

    if (y > m_size.cy) {
        if (field_878 + field_86E < field_87A) {
            field_86E++;
            AdjustVisibleIndex();
        }

        y = m_size.cy - 1;
    }

    m_cVidFont.pRes->Demand();

    int nColumn = 0;

    // NOTE: `min` evaluates its arguments twice.
    int nTargetLine = min(field_86E + min(y / m_cVidFont.GetFontHeight(FALSE), static_cast<INT>(field_878)),
        static_cast<INT>(field_87A));

    int nIndex = 0;
    if (nTargetLine > 0) {
        for (int nLine = 0; nLine < nTargetLine && nIndex < m_sText.GetLength(); nLine++) {
            nIndex += CUtil::FindOneOf(m_sText.Mid(nIndex), CString("\n\r"), 0);
            BOOL bNotFound = nIndex == -1;
            if (bNotFound) {
                nIndex = m_sText.GetLength();
            } else {
                nIndex++;
            }
        }
    }

    if (nIndex == m_sText.GetLength()) {
        field_88E = m_sText.GetLength();
        field_86A = m_sText.GetLength();
        m_cVidFont.pRes->Release();
    } else {
        int nLineEnd = CUtil::FindOneOf(m_sText.Mid(nIndex), CString("\n\r"), 0);
        BOOL bNotFound = nLineEnd == -1;
        if (bNotFound) {
            nLineEnd = m_sText.GetLength();
        }

        if (x > m_cVidFont.GetStringLength(m_sText.Mid(nIndex, nLineEnd), FALSE)) {
            nColumn = nLineEnd + 1;
        } else if (x > m_cVidFont.GetStringLength(m_sText.Mid(nIndex, 0), FALSE)) {
            do {
                if (g_pChitin->field_1A0) {
                    if (IsDBCSLeadByte(m_sText[nIndex + nColumn - 1])) {
                        nColumn++;
                    }
                }

                nColumn++;

                if (nColumn == m_sText.GetLength()) {
                    break;
                }
            } while (x > m_cVidFont.GetStringLength(m_sText.Mid(nIndex, nColumn), FALSE));
        }

        m_cVidFont.pRes->Release();

        if (g_pChitin->field_1A0) {
            if (CUtil::IsDBCSTrailByte(m_sText, nIndex + nColumn - 1)) {
                nColumn--;
            }
        }

        field_86A = min(nIndex + nColumn - 1, m_sText.GetLength());
        field_88E = field_86A;

        InvalidateRect();
    }
}

// 0x4E09F0
BOOL CUIControlEditMultiLine::NeedRender()
{
    return (m_bActive || m_bInactiveRender) && m_nRenderCount > 0;
}

// 0x4E0A20
BOOL CUIControlEditMultiLine::Render(BOOL bForce)
{
    // NOTE: The original function is split in two at 0x4E0A4C.

    // NOTE: Uninline.
    CVidInf* pVidInf = static_cast<CVidInf*>(g_pBaldurChitin->GetCurrentVideoMode());

    INT nIndex = 0;
    INT nLine = 0;
    CString sText;
    CString sLine;

    if ((!m_bActive && !m_bInactiveRender) || (m_nRenderCount == 0 && !bForce)) {
        return FALSE;
    }

    if (field_877) {
        m_cVidFont.Load(FALSE);
        SHORT nFontHeight = m_cVidFont.GetFontHeight(FALSE);
        field_877 = 0;
        field_878 = static_cast<SHORT>((m_size.cy - m_ptText.y) / nFontHeight);
    }

    if (m_nRenderCount != 0) {
        CSingleLock lock(&(m_pPanel->m_pManager->field_56), FALSE);
        lock.Lock(INFINITE);
        m_nRenderCount--;
        lock.Unlock();
    }

    INT nTextX = m_ptText.x + (m_ptOrigin.x + m_pPanel->m_ptOrigin.x);
    INT nTextY = m_ptText.y + (m_pPanel->m_ptOrigin.y + m_ptOrigin.y);

    m_cVidFont.pRes->Demand();

    SHORT nBaseLine = m_cVidFont.GetBaseLineHeight(TRUE);

    nTextX++;
    nTextY++;

    CRect rText(nTextX, nTextY, m_size.cx + nTextX, m_size.cy + nTextY);

    CRect rIntersect;
    rIntersect.IntersectRect(rText, m_rDirty);

    CRect rClip(rIntersect);

    if (!pVidInf->BKLock(rClip)) {
        m_cVidFont.pRes->Release();
        return TRUE;
    }

    nTextY -= rClip.top;
    nTextX -= rClip.left;
    rClip.OffsetRect(-rClip.left, -rClip.top);

    // NOTE: A one-character string never equals "\r\n", so this branch is dead
    // in the original too.
    if (m_sText.Right(1) == "\r\n"
        && (!g_pChitin->field_1A0 || !CUtil::IsDBCSTrailByte(m_sText, m_sText.GetLength() - 1))) {
        sText = m_sText.Left(m_sText.GetLength() - 1);
    } else {
        sText = m_sText;
    }

    sLine.Empty();

    INT nCursorX = -1;
    INT nCursorY = -1;

    INT nSelStart;
    INT nSelEnd;
    if (field_88A < field_88E) {
        nSelStart = field_88A;
        nSelEnd = field_88E;
    } else {
        nSelStart = field_88E;
        nSelEnd = field_88A;
    }

    if (sText.GetLength() > 0) {
        INT nEnd;
        do {
            if (static_cast<WORD>(nLine) >= field_878 + field_86E) {
                break;
            }

            nEnd = CUtil::FindOneOf(sText.Mid(nIndex), CString("\n\r"), 0);
            if (nEnd == -1) {
                nEnd = sText.GetLength();
            }

            sLine += sText.Mid(nIndex, nEnd);

            if (nIndex <= field_86A && field_86A <= nEnd + nIndex) {
                nCursorY = m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E);
                nCursorX = m_cVidFont.GetStringLength(m_sText.Mid(nIndex, field_86A - nIndex), TRUE);
            }

            if (nLine >= field_86E) {
                INT nLineEnd = nEnd + nIndex;

                if (nSelEnd == -1 || nSelStart == -1) {
                    m_cVidFont.SetColor(m_rgbForegroundColor, m_rgbBackgroundColor, FALSE);

                    pVidInf->BKTextOut(&m_cVidFont,
                        sLine,
                        nTextX,
                        nTextY + nBaseLine + m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E),
                        rClip,
                        0,
                        TRUE);
                } else if (nIndex <= nSelStart && nSelStart <= nLineEnd) {
                    m_cVidFont.SetColor(m_rgbForegroundColor, m_rgbBackgroundColor, FALSE);

                    INT nPreLen = nSelStart - nIndex;

                    pVidInf->BKTextOut(&m_cVidFont,
                        sLine.Left(nPreLen),
                        nTextX,
                        m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E) + nBaseLine + nTextY,
                        rClip,
                        0,
                        TRUE);

                    m_cVidFont.SetColor(0x3C3CC8, m_rgbBackgroundColor, FALSE);

                    INT nPreWidth = m_cVidFont.GetStringLength(sLine.Left(nPreLen), TRUE);

                    if (nSelEnd <= nLineEnd) {
                        INT nSelLen = nSelEnd - nSelStart;

                        pVidInf->BKTextOut(&m_cVidFont,
                            sLine.Mid(nPreLen, nSelLen),
                            nPreWidth + nTextX,
                            m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E) + nBaseLine + nTextY,
                            rClip,
                            0,
                            TRUE);

                        nPreWidth += m_cVidFont.GetStringLength(sLine.Mid(nPreLen, nSelLen), TRUE);

                        m_cVidFont.SetColor(m_rgbForegroundColor, m_rgbBackgroundColor, FALSE);

                        pVidInf->BKTextOut(&m_cVidFont,
                            sLine.Right(sLine.GetLength() - nSelEnd + nIndex),
                            nPreWidth + nTextX,
                            m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E) + nBaseLine + nTextY,
                            rClip,
                            0,
                            TRUE);
                    } else {
                        pVidInf->BKTextOut(&m_cVidFont,
                            sLine.Right(sLine.GetLength() - nSelStart + nIndex),
                            nPreWidth + nTextX,
                            m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E) + nBaseLine + nTextY,
                            rClip,
                            0,
                            TRUE);
                    }
                } else if (nIndex <= nSelEnd && nSelEnd <= nLineEnd) {
                    m_cVidFont.SetColor(0x3C3CC8, m_rgbBackgroundColor, FALSE);

                    INT nSelLen = nSelEnd - nIndex;

                    pVidInf->BKTextOut(&m_cVidFont,
                        sLine.Left(nSelLen),
                        nTextX,
                        m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E) + nBaseLine + nTextY,
                        rClip,
                        0,
                        TRUE);

                    INT nSelWidth = m_cVidFont.GetStringLength(sLine.Left(nSelLen), TRUE);

                    m_cVidFont.SetColor(m_rgbForegroundColor, m_rgbBackgroundColor, FALSE);

                    pVidInf->BKTextOut(&m_cVidFont,
                        sLine.Right(sLine.GetLength() - nSelEnd + nIndex),
                        nSelWidth + nTextX,
                        m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E) + nBaseLine + nTextY,
                        rClip,
                        0,
                        TRUE);
                } else {
                    if (nIndex <= nSelStart || nIndex > nSelEnd) {
                        m_cVidFont.SetColor(m_rgbForegroundColor, m_rgbBackgroundColor, FALSE);
                    } else {
                        m_cVidFont.SetColor(0x3C3CC8, m_rgbBackgroundColor, FALSE);
                    }

                    pVidInf->BKTextOut(&m_cVidFont,
                        sLine,
                        nTextX,
                        nBaseLine + (nTextY + m_cVidFont.GetFontHeight(TRUE) * (nLine - field_86E)),
                        rClip,
                        0,
                        TRUE);
                }
            }

            nIndex = min(nIndex + nEnd + 1, sText.GetLength());
            nLine++;
            sLine.Empty();
        } while (nEnd < sText.GetLength());
    }

    m_vcCursor.pRes->Demand();

    CPoint ptCursor;
    m_vcCursor.GetCurrentCenterPoint(ptCursor, TRUE);

    if (m_bFocused) {
        if (nCursorX == -1 || nCursorY == -1) {
            if (field_86A == 0 && field_86E == 0) {
                nCursorX = 0;
                nCursorY = 0;
            }
        }

        m_vcCursor.RealizePalette(1);

        pVidInf->BKRender(&m_vcCursor,
            nCursorX + ptCursor.x + nTextX,
            ptCursor.y + nCursorY + nTextY,
            rClip,
            TRUE,
            0);
    }

    m_vcCursor.pRes->Release();
    m_cVidFont.pRes->Release();

    pVidInf->BKUnlock();

    return TRUE;
}

// 0x4E1580
void CUIControlEditMultiLine::InvalidateRect()
{
    if (m_bActive || m_bInactiveRender) {
        CSingleLock lock(&(m_pPanel->m_pManager->field_56), FALSE);
        lock.Lock(INFINITE);
        m_nRenderCount = CUIManager::RENDER_COUNT;
        lock.Unlock();
    }
}

// 0x4E1610
void CUIControlEditMultiLine::AdjustVisibleIndex()
{
    if (field_87A > field_878) {
        if (field_86A == 0) {
            field_86E = 0;
        }

        if (field_86A != m_sText.GetLength()) {
            int numberOfLines = 0;
            if (m_sText.GetLength() > 0) {
                int start = 0;
                int end;
                do {
                    end = CUtil::FindOneOf(m_sText.Mid(start), CString("\n\r"), 0);
                    if (end == -1) {
                        end = m_sText.GetLength();
                    }

                    if (start <= field_86A && field_86A <= end + start) {
                        break;
                    }

                    start += end + 1;
                    numberOfLines++;

                    if (start >= m_sText.GetLength()) {
                        start = m_sText.GetLength();
                    }
                } while (end < m_sText.GetLength());
            }

            if (field_86E >= numberOfLines || numberOfLines >= field_86E + field_878) {
                if (field_86E < numberOfLines) {
                    field_86E = max(numberOfLines - field_878 + 1, 0);
                } else {
                    field_86E = numberOfLines;
                }
            } else {
                if (field_87A - field_86E < field_878) {
                    field_86E = field_87A - field_878;
                }

                if (field_86E < numberOfLines) {
                    field_86E = max(numberOfLines - field_878 + 1, 0);
                } else {
                    field_86E = numberOfLines;
                }
            }
        } else {
            field_86E = field_87A - field_878;
        }
    } else {
        field_86E = 0;
    }
}

// 0x4E17B0
void CUIControlEditMultiLine::SetText(CString sText)
{
    field_866 = sText;
    if (sText != m_sText) {
        m_sText = sText;
        field_86A = 0;
        if (sText.GetLength() != 0) {
            CalculateNewLines(3);
            InvalidateRect();
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4E1860
CUIControlEditMultiLineScroller::CUIControlEditMultiLineScroller(CUIPanel* panel, UI_CONTROL_EDIT* controlInfo)
    : CUIControlEditMultiLine(panel, controlInfo, 0)
{
    field_896 = 0;
    field_897 = 0;
    field_898 = 0;
    field_899 = 0;
    field_892 = field_36;
}

// 0x4E18D0
CUIControlEditMultiLineScroller::~CUIControlEditMultiLineScroller()
{
}

// 0x4E1990
void CUIControlEditMultiLineScroller::AdjustVisibleIndex()
{
    CUIControlEditMultiLine::AdjustVisibleIndex();

    if (field_892 != -1) {
        CUIControlScrollBar* pScrollBar = static_cast<CUIControlScrollBar*>(m_pPanel->GetControl(field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(field_878, field_878, field_87A);
    }
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnScroll(SHORT a1, SHORT a2)
{
    if (field_87A < field_878) {
        return;
    }

    // TODO: Check casts.
    int v1 = static_cast<int>(static_cast<float>(a1) / static_cast<float>(a2) * (field_87A - field_878));

    if (v1 == field_86E || v1 < 0 || v1 > field_87A - field_878) {
        return;
    }

    field_86E = v1;

    if (field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(field_86E, field_87A, field_878);
    }

    InvalidateRect();
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnScrollUp()
{
    field_86E--;
    if (field_86E < 0) {
        field_86E = 0;
    }

    if (field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(field_86E, field_87A, field_878);
    }

    InvalidateRect();
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnScrollDown()
{
    field_86E++;
    if (field_86E > field_87A - field_878) {
        field_86E = field_87A - field_878;
    }

    if (field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(field_86E, field_87A, field_878);
    }

    InvalidateRect();
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnPageUp(DWORD nLines)
{
    SHORT v1 = static_cast<SHORT>(nLines);
    if (v1 > field_878) {
        v1 = field_878;
    }

    field_86E -= v1;
    if (field_86E < 0) {
        field_86E = 0;
    }

    if (field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(field_86E, field_87A, field_878);
    }

    InvalidateRect();
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnPageDown(DWORD nLines)
{
    if (field_87A < field_878) {
        field_86E = 0;
    } else {
        SHORT v1 = static_cast<SHORT>(nLines);
        if (v1 > field_878) {
            v1 = field_878;
        }

        field_86E += v1;
        if (field_86E > field_87A - field_878) {
            field_86E = field_87A - field_878;
        }
    }

    if (field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(field_86E, field_87A, field_878);
    }

    InvalidateRect();
}
