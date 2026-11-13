#include "CUIControlEdit.h"

#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CScreenWorld.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "CVidInf.h"

// 0x4D6770
CUIControlEdit::CUIControlEdit(CUIPanel* panel, UI_CONTROL_EDIT* controlInfo, int a3)
    : CUIControlBase(panel, &(controlInfo->base), 0)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ChUIControls.cpp
    // __LINE__: 1375
    UTIL_ASSERT(panel != NULL && controlInfo != NULL);

    field_8A1 = 0;

    if (m_pPanel->m_pManager->m_bDoubleSize) {
        m_ptText.x = 2 * controlInfo->x;
        m_ptText.y = 2 * controlInfo->y;
        field_350 = 2 * controlInfo->field_36;
        field_354 = 2 * controlInfo->field_38;
    } else {
        m_ptText.x = controlInfo->x;
        m_ptText.y = controlInfo->y;
        field_350 = controlInfo->field_36;
        field_354 = controlInfo->field_38;
    }

    m_nMaxLength = controlInfo->nMaxLength;
    m_nTextCapitalization = controlInfo->nTextCapitalization;
    m_nRenderCount = 0;
    m_bFocused = 0;
    field_85C = controlInfo->field_68;
    m_sText = controlInfo->initialText;
    m_nCursorIndex = -1;
    m_nVisibleIndex = 0;
    field_872 = 1;
    field_873 = 1;

    m_mosBackground.SetResRef(CResRef(controlInfo->refMosaic), FALSE, TRUE);
    m_mosBackground.m_bDoubleSize = m_pPanel->m_pManager->m_bDoubleSize;
    m_mosTextFocusedBackground.SetResRef(CResRef(controlInfo->refMosaicTextFocused), FALSE, TRUE);
    m_mosTextFocusedBackground.m_bDoubleSize = m_pPanel->m_pManager->m_bDoubleSize;
    m_mosTextUnFocusedBackground.SetResRef(CResRef(controlInfo->refMosaicTextUnFocused), FALSE, TRUE);
    m_mosTextUnFocusedBackground.m_bDoubleSize = m_pPanel->m_pManager->m_bDoubleSize;

    // NOTE: Uninline.
    m_vcCursor.SetResRef(CResRef(controlInfo->refCursor), m_pPanel->m_pManager->m_bDoubleSize, FALSE, TRUE);

    m_vcCursor.SequenceSet(controlInfo->nSequence);
    m_vcCursor.FrameSet(controlInfo->nFrame);

    m_cVidFont.SetResRef(CResRef(controlInfo->refFont), m_pPanel->m_pManager->m_bDoubleSize, FALSE);
    m_cVidFont.SetColor(RGB(200, 200, 200), RGB(60, 60, 60), 0);
    field_89C = a3;
    field_36 = 1;
    field_8A0 = 1;
}

// 0x4D6B80
void CUIControlEdit::OnEditReturn(CString sText)
{
}

// 0x4D6BC0
CString CUIControlEdit::GetText()
{
    return m_sText;
}

// 0x4D6C10
CUIControlEdit::~CUIControlEdit()
{
}

// 0x4D6CE0
void CUIControlEdit::SetActive(BOOLEAN bActive)
{
    m_bActive = bActive;
    if (bActive) {
        m_cVidFont.RegisterFont();
    } else {
        m_cVidFont.Unload();
    }
}

// 0x4D6D10
void CUIControlEdit::KillFocus()
{
    // FIXME: Unused.
    CWnd* pWnd = g_pChitin->GetWnd();
    if (g_pChitin->cImm.field_128) {
        g_pChitin->cImm.sub_7C2E10(g_pChitin->GetWnd()->GetSafeHwnd());
        m_pPanel->InvalidateRect(&(m_pPanel->m_rImeSuggestionsFrame));
        m_pPanel->m_rImeSuggestionsFrame.SetRectEmpty();
        m_pPanel->m_pImeSuggestionsFont = 0;
    }

    static_cast<CBaldurEngine*>(m_pPanel->m_pManager->m_pWarp)->DisableEditKeys();

    m_bFocused = FALSE;
    m_nCursorIndex = -1;
    m_nVisibleIndex = 0;
    InvalidateRect();
}

// 0x4D6DB0
void CUIControlEdit::SetFocus()
{
    m_bFocused = TRUE;
    field_868 = m_sText;
    m_nCursorIndex = m_sText.GetLength();
    m_nVisibleIndex = 0;
    AdjustVisibleIndex();

    InvalidateRect();

    static_cast<CBaldurEngine*>(m_pPanel->m_pManager->m_pWarp)->EnableEditKeys(field_85C);
}

// 0x4D6E20
BOOL CUIControlEdit::OnLButtonDown(CPoint pt)
{
    if (!m_bActive) {
        return FALSE;
    }

    int x = pt.x - (m_ptText.x + m_ptOrigin.x);
    int y = pt.y - (m_ptText.y + m_ptOrigin.y);

    CSize size;
    if (m_bFocused) {
        m_mosTextFocusedBackground.GetSize(size, FALSE);
    } else {
        m_mosTextUnFocusedBackground.GetSize(size, FALSE);
    }

    if (x < 0 || x >= size.cx || y < 0 || y >= size.cy) {
        if (m_bFocused == TRUE) {
            if (field_89C) {
                m_sText = field_868;
            }

            // NOTE: Uninline.
            m_pPanel->m_pManager->KillCapture();
        }
    } else {
        if (m_bFocused) {
            m_cVidFont.pRes->Demand();

            int index = m_nVisibleIndex;
            while (index < m_sText.GetLength()) {
                LONG nStringLength;
                if (g_pChitin->field_1A0 && IsDBCSLeadByte(m_sText[index])) {
                    nStringLength = m_cVidFont.GetStringLength(m_sText.Mid(index, 2), TRUE);
                } else {
                    nStringLength = m_cVidFont.GetStringLength(CString(m_sText[index], 1), TRUE);
                }

                // NOTE: The exe also adds (SHORT)word_85DD3C (always 0) to
                // the measured length.
                x -= nStringLength;
                if (x < 0) {
                    break;
                }

                if (g_pChitin->field_1A0 && IsDBCSLeadByte(m_sText[index])) {
                    index++;
                }

                index++;
            }

            m_cVidFont.pRes->Release();
            m_nCursorIndex = index;
            InvalidateRect();
        } else {
            m_pPanel->m_pManager->SetCapture(this, CUIManager::KEYBOARD);
        }
    }

    return TRUE;
}

// 0x4D7070
BOOL CUIControlEdit::OnRButtonDown(CPoint pt)
{
    if (!m_bActive) {
        return FALSE;
    }

    int x = pt.x - (m_ptText.x + m_ptOrigin.x);
    int y = pt.y - (m_ptText.y + m_ptOrigin.y);

    CSize size;
    if (m_bFocused) {
        m_mosTextFocusedBackground.GetSize(size, FALSE);
    } else {
        m_mosTextUnFocusedBackground.GetSize(size, FALSE);
    }

    if (x < 0 || x >= size.cx || y < 0 || y >= size.cy) {
        if (m_bFocused == TRUE) {
            if (field_89C) {
                m_sText = field_868;
            }

            // NOTE: Uninline.
            m_pPanel->m_pManager->KillCapture();
        }
    }

    return TRUE;
}

// 0x4D7130
void CUIControlEdit::OnKeyDown(SHORT nKey)
{
    // 0x8ABA48
    static BYTE digits[] = {
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
    };

    // 0x8ABA54
    static BYTE shiftedDigits[] = {
        ')',
        '!',
        '@',
        '#',
        '$',
        '%',
        '^',
        '&',
        '*',
        '(',
    };

    // 0x8ABA3C
    static BYTE symbolKeys[] = {
        VK_OEM_1,
        VK_OEM_PLUS,
        VK_OEM_COMMA,
        VK_OEM_MINUS,
        VK_OEM_PERIOD,
        VK_OEM_2,
        VK_OEM_3,
        VK_OEM_4,
        VK_OEM_5,
        VK_OEM_6,
        VK_OEM_7,
        0,
    };

    // 0x8ABA24
    static BYTE symbols[] = {
        ';',
        '=',
        ',',
        '-',
        '.',
        '/',
        '`',
        '[',
        '\\',
        ']',
        '\'',
        0,
    };

    // 0x8ABA30
    static BYTE shiftedSymbols[] = {
        ':',
        '+',
        '<',
        '_',
        '>',
        '?',
        '~',
        '{',
        '|',
        '}',
        '"',
        0,
    };

    if (g_pBaldurChitin->field_1A0) {
        // FIXME: Unused.
        g_pChitin->GetWnd();

        if (g_pBaldurChitin->cImm.field_128) {
            // 0x4D7183 - 0x4D7500 (893 bytes)
            if (nKey == VK_F3 && (GetSystemDefaultLangID() & 0x3FF) == LANG_JAPANESE) {
                g_pChitin->cImm.sub_7C2CC0(g_pChitin->GetWnd()->GetSafeHwnd());
            }

            if (m_sText.GetLength() >= m_nMaxLength) {
                if (nKey == VK_F4 || (nKey == VK_RETURN && (GetSystemDefaultLangID() & 0x3FF) == LANG_KOREAN)) {
                    g_pBaldurChitin->cImm.sub_7C2E10(g_pChitin->GetWnd()->GetSafeHwnd());
                    m_pPanel->InvalidateRect(&(m_pPanel->m_rImeSuggestionsFrame));
                    m_pPanel->m_rImeSuggestionsFrame.SetRectEmpty();
                    m_pPanel->m_pImeSuggestionsFont = NULL;
                }

                m_pPanel->InvalidateRect(NULL);
                return;
            }

            CString sIme = g_pBaldurChitin->cImm.sub_4D89E0();
            if (sIme != "") {
                m_sText = m_sText.Left(m_nCursorIndex)
                    + sIme
                    + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);
            }

            if (m_sText.GetLength() >= m_nMaxLength) {
                m_sText = m_sText.Left(m_nMaxLength);
            }

            m_nCursorIndex += sIme.GetLength();

            AdjustVisibleIndex();

            if (nKey == VK_F4 || (nKey == VK_RETURN && (GetSystemDefaultLangID() & 0x3FF) == LANG_KOREAN)) {
                g_pBaldurChitin->cImm.sub_7C2E10(g_pChitin->GetWnd()->GetSafeHwnd());
                m_pPanel->InvalidateRect(&(m_pPanel->m_rImeSuggestionsFrame));
                m_pPanel->m_rImeSuggestionsFrame.SetRectEmpty();
                m_pPanel->m_pImeSuggestionsFont = NULL;
                m_pPanel->InvalidateRect(NULL);
                static_cast<CBaldurEngine*>(g_pBaldurChitin->pActiveEngine)->GetManager()->InvalidateRect(NULL);
                return;
            }

            if (nKey == VK_RETURN
                && g_pBaldurChitin->pActiveEngine == g_pBaldurChitin->m_pEngineWorld) {
                field_872 = 1;
                if (m_sText != field_874[0]) {
                    for (BYTE index = field_873 - 1; index != 0; index--) {
                        field_874[index] = field_874[index - 1];
                    }

                    field_874[0] = m_sText;
                }

                OnEditReturn(m_sText);

                if (field_8A0) {
                    // NOTE: Uninline.
                    m_pPanel->m_pManager->KillCapture();
                }
            }

            InvalidateRect();
            return;
        }
    }

    if (m_pPanel->m_pManager->m_pWarp->GetMenuKey()) {
        return;
    }

    if (m_pPanel->m_pManager->m_pWarp->GetCtrlKey()) {
        if (m_pPanel->m_pManager->m_pWarp->GetShiftKey()) {
            return;
        }

        if (nKey == VK_INSERT || nKey == 'C') {
            // 0x4D767A - 0x4D771D
            if (OpenClipboard(NULL)) {
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, m_sText.GetLength() + 1);
                if (hMem != NULL) {
                    LPVOID pMem = GlobalLock(hMem);
                    if (pMem != NULL) {
                        memcpy(pMem,
                            m_sText.GetBuffer(m_sText.GetLength() + 1),
                            m_sText.GetLength() + 1);
                        m_sText.ReleaseBuffer();

                        GlobalUnlock(hMem);

                        if (SetClipboardData(CF_TEXT, hMem) == NULL) {
                            // Ownership was not transferred.
                            GlobalFree(hMem);
                        }
                    } else {
                        GlobalFree(hMem);
                    }
                }

                CloseClipboard();
            }

            InvalidateRect();
        } else if (nKey == 'V') {
            // 0x4D7562 - 0x4D767A
            if (OpenClipboard(NULL)) {
                HGLOBAL hMem = GetClipboardData(CF_TEXT);
                if (hMem != NULL) {
                    LPVOID pMem = GlobalLock(hMem);
                    if (pMem != NULL) {
                        m_sText = m_sText.Left(m_nCursorIndex)
                            + reinterpret_cast<LPCSTR>(pMem)
                            + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);

                        GlobalUnlock(hMem);
                    }
                }

                CloseClipboard();
            }

            InvalidateRect();
        }

        return;
    }

    switch (nKey) {
    case VK_BACK:
        // 0x4D7C15 - 0x4D7F41 (812 bytes)
        if (m_nCursorIndex == 0) {
            return;
        }

        m_nCursorIndex--;
        if (m_nCursorIndex < m_nVisibleIndex) {
            m_nVisibleIndex = m_nCursorIndex;
        }

        if (g_pChitin->field_1A0) {
            if (CUtil::IsDBCSTrailByte(m_sText, m_nCursorIndex)) {
                m_nCursorIndex--;
                if (m_nCursorIndex < m_nVisibleIndex) {
                    m_nVisibleIndex = m_nCursorIndex;
                }

                if (g_pChitin->field_1A0 && IsDBCSLeadByte(m_sText[m_nCursorIndex])) {
                    m_sText = m_sText.Left(m_nCursorIndex)
                        + m_sText.Right(m_sText.GetLength() - m_nCursorIndex - 2);
                } else {
                    m_sText = m_sText.Left(m_nCursorIndex)
                        + m_sText.Right(m_sText.GetLength() - m_nCursorIndex - 1);
                }
            } else {
                m_sText = m_sText.Left(m_nCursorIndex)
                    + m_sText.Right(m_sText.GetLength() - m_nCursorIndex - 1);
            }
        } else {
            m_sText = m_sText.Left(m_nCursorIndex)
                + m_sText.Right(m_sText.GetLength() - m_nCursorIndex - 1);
        }

        break;
    case VK_RETURN:
        // 0x4D79C8 - 0x4D7A6D (165 bytes)
        field_872 = 1;
        if (m_sText != field_874[0]) {
            for (BYTE index = field_873 - 1; index != 0; index--) {
                field_874[index] = field_874[index - 1];
            }

            field_874[0] = m_sText;
        }

        OnEditReturn(m_sText);

        if (field_8A0) {
            // NOTE: Uninline.
            m_pPanel->m_pManager->KillCapture();
        }

        break;
    case VK_ESCAPE:
        // 0x4D79AA - 0x4D79C8 (30 bytes), shares the capture release at 0x4D7A4C
        m_sText = field_868;
        field_872 = 1;

        // NOTE: Uninline.
        m_pPanel->m_pManager->KillCapture();

        break;
    case VK_SPACE:
        // 0x4D80D9 - 0x4D81D0 (247 bytes)
        if (field_85C == 2) {
            return;
        }

        if (m_sText.GetLength() == m_nMaxLength) {
            return;
        }

        m_sText = m_sText.Left(m_nCursorIndex)
            + ' '
            + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);

        m_nCursorIndex++;

        AdjustVisibleIndex();
        break;
    case VK_END:
        // 0x4D7B1E - 0x4D7B33 (21 bytes)
        m_nCursorIndex = m_sText.GetLength();

        AdjustVisibleIndex();
        break;
    case VK_HOME:
        // 0x4D7B33 - 0x4D7B47 (20 bytes)
        m_nVisibleIndex = 0;
        m_nCursorIndex = 0;
        break;
    case VK_LEFT:
        // 0x4D7B47 - 0x4D7BC5 (126 bytes)
        if (m_nCursorIndex == 0) {
            return;
        }

        m_nCursorIndex--;
        if (m_nCursorIndex < m_nVisibleIndex) {
            m_nVisibleIndex = m_nCursorIndex;
        }

        if (g_pChitin->field_1A0 && CUtil::IsDBCSTrailByte(m_sText, m_nCursorIndex)) {
            m_nCursorIndex--;
            if (m_nCursorIndex < m_nVisibleIndex) {
                m_nVisibleIndex = m_nCursorIndex;
            }
        }

        break;
    case VK_UP:
        // 0x4D7A6D - 0x4D7ACB (94 bytes)
        if (field_872 == field_873) {
            return;
        }

        if (field_874[field_872].GetLength() == 0) {
            return;
        }

        m_sText = field_874[field_872++];
        m_nCursorIndex = m_sText.GetLength();

        AdjustVisibleIndex();
        break;
    case VK_RIGHT:
        // 0x4D7BC5 - 0x4D7C15 (80 bytes)
        if (m_nCursorIndex == m_sText.GetLength()) {
            return;
        }

        if (g_pChitin->field_1A0 && IsDBCSLeadByte(m_sText[m_nCursorIndex])) {
            m_nCursorIndex++;
        }

        m_nCursorIndex++;

        AdjustVisibleIndex();
        break;
    case VK_DOWN:
        // 0x4D7ACB - 0x4D7B1E (83 bytes)
        if (field_872 < 2) {
            return;
        }

        if (field_874[field_872 - 2].GetLength() == 0) {
            return;
        }

        m_sText = field_874[field_872 - 2];
        field_872--;
        m_nCursorIndex = m_sText.GetLength();

        AdjustVisibleIndex();
        break;
    case VK_INSERT:
        // 0x4D7873 - 0x4D79AA (311 bytes)
        if (m_pPanel->m_pManager->m_pWarp->GetShiftKey()) {
            if (OpenClipboard(NULL)) {
                HGLOBAL hMem = GetClipboardData(CF_TEXT);
                if (hMem != NULL) {
                    LPVOID pMem = GlobalLock(hMem);
                    if (pMem != NULL) {
                        m_sText = m_sText.Left(m_nCursorIndex)
                            + reinterpret_cast<LPCSTR>(pMem)
                            + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);

                        GlobalUnlock(hMem);
                    }
                }

                CloseClipboard();
            }
        }

        break;
    case VK_DELETE:
        // 0x4D7F41 - 0x4D80D9 (408 bytes)
        if (m_nCursorIndex == m_sText.GetLength()) {
            return;
        }

        if (g_pChitin->field_1A0 && IsDBCSLeadByte(m_sText[m_nCursorIndex])) {
            m_sText = m_sText.Left(m_nCursorIndex)
                + m_sText.Right(m_sText.GetLength() - m_nCursorIndex - 2);
        } else {
            m_sText = m_sText.Left(m_nCursorIndex)
                + m_sText.Right(m_sText.GetLength() - m_nCursorIndex - 1);
        }

        break;
    case VK_F3:
    case VK_PROCESSKEY:
        // 0x4D7767 - 0x4D7873 (268 bytes)
        if (g_pChitin->field_1A4) {
            g_pChitin->cImm.sub_7C2CC0(g_pChitin->GetWnd()->GetSafeHwnd());

            // FIXME: Unused.
            g_pChitin->GetWnd();
            if (g_pChitin->cImm.field_128) {
                CRect r = g_pChitin->cImm.sub_7C3020(
                    m_pPanel->m_ptOrigin,
                    m_pPanel->m_size,
                    m_ptOrigin,
                    m_size,
                    m_cVidFont.GetFontHeight(FALSE));
                if (r.IsRectNull()) {
                    m_pPanel->InvalidateRect(&(m_pPanel->m_rImeSuggestionsFrame));
                }

                m_pPanel->m_rImeSuggestionsFrame = r;
                m_pPanel->m_pImeSuggestionsFont = &m_cVidFont;
            }
        }

        break;
    case VK_F4:
        // 0x4D7744 - 0x4D7767 (35 bytes)
        if (g_pBaldurChitin->field_1A0) {
            m_pPanel->InvalidateRect(NULL);
        }

        break;
    default:
        // 0x4D81D0 - 0x4D887D (1709 bytes)
        if (nKey >= 'A' && nKey <= 'Z') {
            if (m_sText.GetLength() >= m_nMaxLength) {
                return;
            }

            if (field_85C == 2) {
                break;
            }

            switch (m_nTextCapitalization) {
            case 0:
                if (m_pPanel->m_pManager->m_pWarp->GetCapsLockKey() == m_pPanel->m_pManager->m_pWarp->GetShiftKey()) {
                    nKey += 0x20;
                }
                break;
            case 2:
                nKey += 0x20;
                break;
            }

            m_sText = m_sText.Left(m_nCursorIndex)
                + static_cast<char>(nKey)
                + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);

            m_nCursorIndex++;

            AdjustVisibleIndex();
        } else if ((nKey >= '0' && nKey <= '9') || (nKey >= VK_NUMPAD0 && nKey <= VK_NUMPAD9)) {
            if (nKey >= VK_NUMPAD0 && nKey <= VK_NUMPAD9) {
                nKey -= '0';
            }

            if (m_sText.GetLength() == m_nMaxLength) {
                return;
            }

            if (m_pPanel->m_pManager->m_pWarp->GetShiftKey()) {
                m_sText = m_sText.Left(m_nCursorIndex)
                    + static_cast<char>(shiftedDigits[nKey - '0'])
                    + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);
            } else {
                m_sText = m_sText.Left(m_nCursorIndex)
                    + static_cast<char>(digits[nKey - '0'])
                    + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);
            }

            m_nCursorIndex++;

            AdjustVisibleIndex();
        } else if (nKey == VK_MULTIPLY
            || nKey == VK_DIVIDE
            || nKey == VK_ADD
            || nKey == VK_SUBTRACT
            || nKey == VK_DECIMAL) {
            int ch;
            switch (nKey) {
            case VK_MULTIPLY:
                ch = '*';
                break;
            case VK_ADD:
                ch = '+';
                break;
            case VK_SUBTRACT:
                ch = '-';
                break;
            case VK_DECIMAL:
                ch = '.';
                break;
            case VK_DIVIDE:
                ch = '/';
                break;
            default:
                // NOTE: Unreachable, `nKey` is one of the keys above.
                ch = 'X';
                break;
            }

            if (m_sText.GetLength() == m_nMaxLength) {
                return;
            }

            m_sText = m_sText.Left(m_nCursorIndex)
                + static_cast<char>(ch)
                + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);

            m_nCursorIndex++;

            AdjustVisibleIndex();
        } else {
            BYTE index = 0;
            do {
                if (symbolKeys[index] == static_cast<BYTE>(nKey)) {
                    break;
                }

                index++;
            } while (index < 12);

            if (index == 12) {
                break;
            }

            if (m_sText.GetLength() == m_nMaxLength) {
                return;
            }

            // NOTE: Entry 11 of the key table is 0, and entry 11 of the
            // symbol tables is the terminator.
            if (m_pPanel->m_pManager->m_pWarp->GetShiftKey()) {
                m_sText = m_sText.Left(m_nCursorIndex)
                    + static_cast<char>(shiftedSymbols[index])
                    + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);
            } else {
                m_sText = m_sText.Left(m_nCursorIndex)
                    + static_cast<char>(symbols[index])
                    + m_sText.Right(m_sText.GetLength() - m_nCursorIndex);
            }

            m_nCursorIndex++;

            AdjustVisibleIndex();
        }

        break;
    }

    InvalidateRect();
}

// 0x4D8A60
BOOL CUIControlEdit::NeedRender()
{
    return (m_bActive || m_bInactiveRender) && m_nRenderCount > 0;
}

// 0x4D8A90
BOOL CUIControlEdit::Render(BOOL bForce)
{
    CVidInf* pVidInf = static_cast<CVidInf*>(g_pBaldurChitin->GetCurrentVideoMode());
    CString sText;

    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    if (field_8A1) {
        m_cVidFont.Load(FALSE);
        field_8A1 = 0;
    }

    if (m_nRenderCount != 0) {
        CSingleLock lock(&(m_pPanel->m_pManager->field_56), FALSE);
        lock.Lock(INFINITE);
        m_nRenderCount--;
        lock.Unlock();
    }

    CRect rBounds(0, 0, m_size.cx, m_size.cy);

    CRect rFrame(rBounds);
    rFrame.OffsetRect(m_pPanel->m_ptOrigin + m_ptOrigin);

    CRect rDirtyFrame;
    rDirtyFrame.IntersectRect(rFrame, m_rDirty);

    m_mosBackground.Render(CVIDINF_SURFACE_BACK,
        rFrame.left,
        rFrame.top,
        rBounds,
        rDirtyFrame,
        0,
        FALSE);

    int x = m_ptText.x + rFrame.left;
    int y = m_ptText.y + rFrame.top;

    CSize frameSize;

    if (m_bFocused) {
        m_mosTextFocusedBackground.GetSize(frameSize, FALSE);

        frameSize.cx = min(frameSize.cx, m_size.cx - m_ptText.x);
        frameSize.cy = min(frameSize.cy, m_size.cy - m_ptText.y);

        CRect rBounds(0, 0, frameSize.cx, frameSize.cy);

        CRect rFrame(rBounds);
        rFrame.OffsetRect(x, y);

        CRect rFrameDirty;
        rFrameDirty.IntersectRect(rFrame, m_rDirty);

        m_mosTextFocusedBackground.Render(CVIDINF_SURFACE_BACK,
            x,
            y,
            rBounds,
            rFrameDirty,
            0,
            FALSE);
    } else {
        m_mosTextUnFocusedBackground.GetSize(frameSize, FALSE);

        frameSize.cx = min(frameSize.cx, m_size.cx - m_ptText.x);
        frameSize.cy = min(frameSize.cy, m_size.cy - m_ptText.y);

        CRect rBounds(0, 0, frameSize.cx, frameSize.cy);

        CRect rFrame(rBounds);
        rFrame.OffsetRect(x, y);

        CRect rFrameDirty;
        rFrameDirty.IntersectRect(rFrame, m_rDirty);

        m_mosTextUnFocusedBackground.Render(CVIDINF_SURFACE_BACK,
            x,
            y,
            rBounds,
            rFrameDirty,
            0,
            FALSE);
    }

    m_cVidFont.pRes->Demand();
    SHORT nBaseLineHeight = m_cVidFont.GetBaseLineHeight(TRUE);

    frameSize.cx -= 3;
    frameSize.cy -= 3;

    CRect rText(CPoint(x + 1, y + 1), frameSize);

    CRect rTextDirty;
    rTextDirty.IntersectRect(rText, m_rDirty);

    if (pVidInf->BKLock(rTextDirty)) {
        CPoint pt = rText.TopLeft() - rTextDirty.TopLeft();

        CRect rClip = rTextDirty;
        rClip.OffsetRect(-rTextDirty.left, -rTextDirty.top);

        SHORT nFontHeight = m_cVidFont.GetFontHeight(TRUE);

        LONG top = frameSize.cy / 2 - nFontHeight / 2 + pt.y;
        sText = m_sText.Mid(m_nVisibleIndex);
        pVidInf->BKTextOut(&m_cVidFont,
            sText,
            pt.x,
            top + nBaseLineHeight,
            rClip,
            0,
            TRUE);

        m_vcCursor.pRes->Demand();

        CPoint ptCursorCenter;
        m_vcCursor.GetCurrentCenterPoint(ptCursorCenter, TRUE);

        if (m_nCursorIndex != -1) {
            LONG nStringLength = m_cVidFont.GetStringLength(m_sText.Mid(m_nVisibleIndex, m_nCursorIndex - m_nVisibleIndex), FALSE);
            m_vcCursor.RealizePalette(0x1);
            pVidInf->BKRender(&m_vcCursor,
                pt.x + ptCursorCenter.x + static_cast<SHORT>(nStringLength),
                top + ptCursorCenter.y,
                rClip,
                TRUE,
                0);
        }

        m_vcCursor.pRes->Release();

        pVidInf->BKUnlock();
    }

    m_cVidFont.pRes->Release();

    return TRUE;
}

// 0x4D9070
void CUIControlEdit::InvalidateRect()
{
    if (m_bActive || m_bInactiveRender) {
        CSingleLock lock(&(m_pPanel->m_pManager->field_56), FALSE);
        lock.Lock(INFINITE);
        m_nRenderCount = CUIManager::RENDER_COUNT;
        lock.Unlock();
    }
}

// 0x4D9100
void CUIControlEdit::AdjustVisibleIndex()
{
    CSize frameSize;
    m_mosTextFocusedBackground.GetSize(frameSize, FALSE);

    frameSize.cx -= 2;

    m_cVidFont.pRes->Demand();

    int index = m_nVisibleIndex;
    while (index < m_sText.GetLength() && index < m_nCursorIndex) {
        if (g_pChitin->field_1A0 && IsDBCSLeadByte(m_sText[index])) {
            frameSize.cx -= m_cVidFont.GetStringLength(m_sText.Mid(index, 2), TRUE);
            index++;
        } else {
            frameSize.cx -= m_cVidFont.GetStringLength(CString(m_sText[index], 1), TRUE);
        }

        while (frameSize.cx < 1) {
            if (g_pChitin->field_1A0 && IsDBCSLeadByte(m_sText[m_nVisibleIndex])) {
                frameSize.cx += m_cVidFont.GetStringLength(m_sText.Mid(m_nVisibleIndex, 2), TRUE);
                m_nVisibleIndex += 2;
            } else {
                frameSize.cx += m_cVidFont.GetStringLength(CString(m_sText[m_nVisibleIndex], 1), TRUE);
                m_nVisibleIndex++;
            }
        }

        index++;
    }

    m_cVidFont.pRes->Release();
}

// 0x4D9360
void CUIControlEdit::SetText(CString sText)
{
    field_868 = sText;
    if (sText != m_sText) {
        m_sText = sText;
        if (m_bFocused) {
            m_nCursorIndex = m_sText.GetLength();
        }

        m_nVisibleIndex = 0;

        AdjustVisibleIndex();

        InvalidateRect();
    }
}
