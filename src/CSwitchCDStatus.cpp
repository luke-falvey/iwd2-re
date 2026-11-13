#include "CSwitchCDStatus.h"

#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CUIControlLabel.h"
#include "CUIManager.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVideo.h"
#include "CWarp.h"

// NOTE: Layout values for the "insert CD" dialog. Only the ones touched by
// `Init` are named here.

// 0x8AE4A0
static int dword_8AE4A0 = 280;

// 0x8AE4A4
static int dword_8AE4A4 = 228;

// 0x8AE4A8
static int dword_8AE4A8 = 276;

// 0x8AE4AC
static int dword_8AE4AC = 291;

// 0x8AE4B0
static int dword_8AE4B0 = 306;

// 0x8AE4B4
static int dword_8AE4B4 = 321;

// 0x8AE4B8
static int dword_8AE4B8 = 10;

// 0x8E0320
static CPoint dword_8E0320(35, 545);

// 0x8E0330
static CSize dword_8E0330(765, 48);

// 0x54D280
CSwitchCDStatus::CSwitchCDStatus()
{
    field_0 = 0;
    field_4 = 0;
    m_bActivateEngine = FALSE;
}

// 0x54D360
CSwitchCDStatus::~CSwitchCDStatus()
{
    if (field_4 != 0) {
        field_2D8.pRes->Release();
        field_440.pRes->Release();
        field_4F4.pRes->Release();
        field_9F4.pRes->Release();
        field_4 = 0;
    }
}

// 0x54D460
void CSwitchCDStatus::Init()
{
    LONG x;
    LONG y;

    if (g_pBaldurChitin->field_4A28 != 0) {
        dword_8AE4A0 *= 2;
        dword_8AE4AC *= 2;
        dword_8AE4A8 *= 2;
        dword_8AE4A4 *= 2;
        dword_8AE4B8 *= 2;
        dword_8AE4B4 *= 2;
        dword_8AE4B0 *= 2;
        dword_8E0330.cx *= 2;
        dword_8E0320.y *= 2;
        dword_8E0320.x *= 2;
        dword_8E0330.cy *= 2;

        x = CVideo::SCREENWIDTH / 2 - CBaldurChitin::DEFAULT_SCREEN_WIDTH;
        y = CVideo::SCREENHEIGHT / 2 - CBaldurChitin::DEFAULT_SCREEN_HEIGHT;
    } else {
        x = (CVideo::SCREENWIDTH - CBaldurChitin::DEFAULT_SCREEN_WIDTH) / 2;
        y = (CVideo::SCREENHEIGHT - CBaldurChitin::DEFAULT_SCREEN_HEIGHT) / 2;
    }

    dword_8AE4A0 += x;
    dword_8AE4A4 += y;
    dword_8AE4A8 += y;
    dword_8AE4AC += y;
    dword_8AE4B0 += y;
    dword_8AE4B4 += y;
    dword_8E0320.x += x;
    dword_8E0320.y += y;
}


// 0x84C97E
static WORD word_84C97E = 0;

// 0x54E220
void CSwitchCDStatus::FUN_0054e220(CVidInf* pVidInf, int x, int y, const CSize* pSize, const CRect* pClip, const CString* sText, CVidFont* pFont, DWORD dwFlags)
{
    CRect rText(x, y, x + pSize->cx, y + pSize->cy);

    WORD nMargin = (g_pBaldurChitin->field_4A28 != 0 ? 2 : 1) * word_84C97E;
    nMargin <<= 1;
    if (g_pBaldurChitin->field_4A28 != 0) {
        nMargin <<= 1;
    }

    CRect rClip;
    IntersectRect(&rClip, &rText, pClip);
    rText = rClip;

    SHORT nLineHeight = pFont->GetFontHeight(TRUE);

    // NOTE: The flag values are read from word globals: 0x84C97C is
    // `CUIControlLabel::TYPE_WORD_WRAP`, the others are unnamed.
    if (static_cast<WORD>(CUIControlLabel::TYPE_WORD_WRAP & dwFlags) == 0
        || nLineHeight > pSize->cy - nMargin) {
        nLineHeight = static_cast<SHORT>(pSize->cy - nMargin);
    }

    int nMaxLines = (pSize->cy - nMargin) / nLineHeight;
    CString* pText = new CString[nMaxLines];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CSwitchCDStatus.cpp
    // __LINE__: 712
    UTIL_ASSERT(pText);

    BYTE nLines = CUtil::SplitString(pFont,
        *sText,
        static_cast<WORD>(pSize->cx - nMargin),
        static_cast<BYTE>((pSize->cy - nMargin) / nLineHeight),
        pText,
        FALSE,
        TRUE,
        TRUE,
        0xFFFF);

    int nOffset;
    if (static_cast<WORD>(0x20 & dwFlags) != 0) {
        // 0x84C976
        nOffset = nMargin >> 1;
    } else if (static_cast<WORD>(0x80 & dwFlags) != 0) {
        // 0x84C97A
        nOffset = pSize->cy - nLines * nLineHeight - (nMargin >> 1);
    } else {
        nOffset = (pSize->cy >> 1) - ((nLines * nLineHeight) >> 1);
    }
    y += nOffset;

    SHORT nBaseLineHeight = pFont->GetBaseLineHeight(TRUE);

    // NOTE: When none of the three horizontal alignment flags (0x8, 0x10,
    // 0x4) is set, the original leaves the stack slot that held `sText` as
    // the x position.
    int nTextX = reinterpret_cast<int>(sText);

    if (static_cast<WORD>(0x8 & dwFlags) != 0) {
        nTextX = (nMargin >> 1) + x;
    }

    if (nLines > 0) {
        int nLine = 0;
        do {
            if (static_cast<WORD>(0x10 & dwFlags) != 0) {
                if ((nMargin >> 1) > pSize->cx - (nMargin >> 1) - pFont->GetStringLength(pText[nLine], TRUE)) {
                    nTextX = (nMargin >> 1) + x;
                } else {
                    nTextX = pSize->cx - (nMargin >> 1) - pFont->GetStringLength(pText[nLine], TRUE) + x;
                }
            } else if (static_cast<WORD>(0x4 & dwFlags) != 0) {
                if ((nMargin >> 1) > (pSize->cx >> 1) - (pFont->GetStringLength(pText[nLine], TRUE) >> 1)) {
                    nTextX = (nMargin >> 1) + x;
                } else {
                    nTextX = (pSize->cx >> 1) - (pFont->GetStringLength(pText[nLine], TRUE) >> 1) + x;
                }
            }

            pVidInf->BKTextOut(pFont,
                pText[nLine],
                nTextX,
                nBaseLineHeight + y,
                rText,
                0,
                TRUE);

            y += nLineHeight;
            nLine++;
        } while (nLine != nLines);
    }

    delete[] pText;
}
// 0x54D5A0
void CSwitchCDStatus::Update(BOOLEAN a1, BOOLEAN a2, BYTE a3, const CString& a4, BOOLEAN a5, BOOLEAN a6, BOOLEAN a7)
{
    STR_RES strRes;
    CString sText;
    CRect rMosaic;
    CRect rClip;
    CSize mosSize;
    INT x;
    INT y;
    WORD nScreenWidth;
    WORD nScreenHeight;
    CVidInf* pVidInf;

    CSingleLock lock(&field_EF8, FALSE);
    lock.Lock(INFINITE);

    if (!g_pChitin->cDimm.m_bCDSwitchActivated && a2 == TRUE) {
        lock.Unlock();
        return;
    }

    CWarp* pEngine = g_pBaldurChitin->pActiveEngine;
    if (pEngine == NULL || (pVidInf = static_cast<CVidInf*>(pEngine->pVidMode)) == NULL) {
        lock.Unlock();
        return;
    }

    if ((!a1 || a2 == TRUE) && field_4 == 0) {
        lock.Unlock();
        return;
    }

    nScreenWidth = CVideo::SCREENWIDTH;
    nScreenHeight = CVideo::SCREENHEIGHT;

    if (g_pBaldurChitin->field_4A28 != 0) {
        x = CVideo::SCREENWIDTH / 2 - CBaldurChitin::DEFAULT_SCREEN_WIDTH;
        y = CVideo::SCREENHEIGHT / 2 - CBaldurChitin::DEFAULT_SCREEN_HEIGHT;
    } else {
        x = (CVideo::SCREENWIDTH - CBaldurChitin::DEFAULT_SCREEN_WIDTH) / 2;
        y = (CVideo::SCREENHEIGHT - CBaldurChitin::DEFAULT_SCREEN_HEIGHT) / 2;
    }

    rClip.left = 0;
    rClip.top = 0;
    rClip.right = nScreenWidth;
    rClip.bottom = nScreenHeight;

    if (!a2) {
        while (g_pBaldurChitin->field_F8 == TRUE) {
            SleepEx(50, FALSE);
        }

        if (a1 == TRUE) {
            for (BYTE side = 0; side < 4; side++) {
                // NOTE: Inlining.
                UI_PANELHEADER* panelInfo = side < 4
                    ? &(g_pBaldurChitin->m_aBorderPanels[side])
                    : NULL;

                field_8[side].SetResRef(CResRef(panelInfo->refMosaic), FALSE, TRUE);
                field_8[side].m_bDoubleSize = g_pBaldurChitin->field_4A28;
            }

            field_2D8.SetResRef(CResRef("GTRCDBG"), FALSE, TRUE);
            field_2D8.m_bDoubleSize = g_pBaldurChitin->field_4A28;

            field_440.SetResRef(CResRef("GTRCDREQ"), FALSE, TRUE);
            field_440.m_bDoubleSize = g_pBaldurChitin->field_4A28;

            field_4F4.SetResRef(CResRef("NORMAL"), g_pBaldurChitin->field_4A28, FALSE);
            field_4F4.RegisterFont();
            field_4F4.SetColor(RGB(230, 230, 230), RGB(30, 30, 30), FALSE);

            field_9F4.SetResRef(CResRef("NORMAL"), g_pBaldurChitin->field_4A28, FALSE);
            field_9F4.RegisterFont();
            field_9F4.SetColor(RGB(230, 230, 230), RGB(30, 30, 30), FALSE);

            for (int side = 0; side < 4; side++) {
                if (field_8[side].pRes != NULL) {
                    field_8[side].pRes->Demand();
                }
            }

            field_2D8.pRes->Demand();
            field_440.pRes->Demand();
            field_4F4.pRes->Demand();
            field_9F4.pRes->Demand();

            g_pBaldurChitin->GetObjectCursor()->SetCursor(100, FALSE);
            g_pBaldurChitin->GetObjectCursor()->m_bVisible = FALSE;
            g_pBaldurChitin->GetObjectCursor()->CursorUpdate(g_pBaldurChitin->GetActiveEngine()->pVidMode);

            field_4 = 1;
            field_0 = 0;
        } else {
            for (int side = 0; side < 4; side++) {
                if (field_8[side].pRes != NULL) {
                    field_8[side].pRes->Release();
                }
            }

            field_2D8.pRes->Release();
            field_440.pRes->Release();
            field_4F4.pRes->Release();
            field_4F4.Unload();
            field_9F4.pRes->Release();
            field_9F4.Unload();

            field_4 = 0;

            if (m_bActivateEngine) {
                m_bActivateEngine = FALSE;

                CBaldurEngine* pActiveEngine = g_pBaldurChitin->GetActiveEngine();
                if (pActiveEngine != NULL) {
                    pActiveEngine->EngineActivated();
                }

                g_pChitin->Resume();
            }

            while (g_pBaldurChitin->m_bReInitializing) {
                SleepEx(25, FALSE);
            }

            if (!g_pBaldurChitin->cProgressBar.m_bProgressBarActivated
                && !g_pBaldurChitin->GetObjectGame()->m_bGameLoaded) {
                g_pBaldurChitin->GetObjectCursor()->SetCursor(0, TRUE);
                g_pBaldurChitin->GetObjectCursor()->m_bVisible = TRUE;
                g_pBaldurChitin->GetObjectCursor()->CursorUpdate(g_pBaldurChitin->GetActiveEngine()->pVidMode);
            }

            g_pBaldurChitin->GetActiveEngine()->GetManager()->InvalidateRect(NULL);

            field_0 = 0;
        }

        lock.Unlock();
        return;
    }

    if (field_4 == 1 && field_0 < 2 && !g_pBaldurChitin->m_bReInitializing) {
        for (BYTE side = 0; side < 4; side++) {
            // NOTE: Inlining.
            UI_PANELHEADER* panelInfo = side < 4
                ? &(g_pBaldurChitin->m_aBorderPanels[side])
                : NULL;

            if (g_pBaldurChitin->field_4A28 != 0) {
                rMosaic.left = 0;
                rMosaic.top = 0;
                rMosaic.right = 2 * panelInfo->nWidth;
                rMosaic.bottom = 2 * panelInfo->nHeight;
                if (panelInfo->nWidth > 0 && panelInfo->nHeight > 0) {
                    field_8[side].Render(CVIDINF_SURFACE_BACK,
                        2 * panelInfo->x,
                        2 * panelInfo->y,
                        rMosaic,
                        rClip,
                        0x1,
                        TRUE);
                }
            } else {
                rMosaic.left = 0;
                rMosaic.top = 0;
                rMosaic.right = panelInfo->nWidth;
                rMosaic.bottom = panelInfo->nHeight;
                if (panelInfo->nWidth > 0 && panelInfo->nHeight > 0) {
                    field_8[side].Render(CVIDINF_SURFACE_BACK,
                        panelInfo->x,
                        panelInfo->y,
                        rMosaic,
                        rClip,
                        0x1,
                        TRUE);
                }
            }
        }

        rMosaic.left = 0;
        rMosaic.top = 0;
        rMosaic.right = nScreenWidth;
        rMosaic.bottom = nScreenHeight;

        field_2D8.Render(CVIDINF_SURFACE_BACK,
            x,
            y,
            rMosaic,
            rClip,
            0,
            TRUE);

        field_440.GetSize(mosSize, TRUE);

        rMosaic.left = 0;
        rMosaic.top = 0;
        rMosaic.right = mosSize.cx;
        rMosaic.bottom = mosSize.cy;

        field_440.Render(CVIDINF_SURFACE_BACK,
            dword_8AE4A0,
            dword_8AE4A4,
            rMosaic,
            rClip,
            0x1,
            TRUE);

        if (pVidInf->BKLock(rClip)) {
            g_pBaldurChitin->GetTlkTable().Fetch(0x407D, strRes);
            sText.Format(strRes.szText);
            pVidInf->BKTextOut(&field_4F4,
                sText,
                (nScreenWidth - field_4F4.GetStringLength(sText, TRUE)) >> 1,
                dword_8AE4A8,
                rClip,
                0,
                TRUE);

            if (!a5) {
                if (a7 == TRUE) {
                    g_pBaldurChitin->GetTlkTable().Fetch(0x40EB, strRes);
                } else {
                    g_pBaldurChitin->GetTlkTable().Fetch(0x40EC, strRes);
                }

                sText.Format("%s %s", (LPCSTR)strRes.szText, (LPCSTR)a4);
                pVidInf->BKTextOut(&field_4F4,
                    sText,
                    (nScreenWidth - field_4F4.GetStringLength(sText, TRUE)) >> 1,
                    dword_8AE4AC,
                    rClip,
                    0,
                    TRUE);

                if (a6) {
                    g_pBaldurChitin->GetTlkTable().Fetch(0x407B, strRes);
                    sText.Format(strRes.szText);
                } else {
                    g_pBaldurChitin->GetTlkTable().Fetch(0x407A, strRes);
                    sText.Format("%s %01d", (LPCSTR)strRes.szText, a3);
                }

                pVidInf->BKTextOut(&field_4F4,
                    sText,
                    (nScreenWidth - field_4F4.GetStringLength(sText, TRUE)) >> 1,
                    dword_8AE4B0,
                    rClip,
                    0,
                    TRUE);

                g_pBaldurChitin->GetTlkTable().Fetch(0x407C, strRes);
                sText.Format("%s %s", (LPCSTR)strRes.szText, (LPCSTR)a4);
                pVidInf->BKTextOut(&field_4F4,
                    sText,
                    (nScreenWidth - field_4F4.GetStringLength(sText, TRUE)) >> 1,
                    dword_8AE4B4,
                    rClip,
                    0,
                    TRUE);

                g_pBaldurChitin->GetTlkTable().Fetch(0x50E0, strRes);
                sText.Format(strRes.szText);

                // NOTE: The flags are read from three word globals at
                // 0x84C97C (TYPE_WORD_WRAP), 0x84C978 (0x40) and 0x84C972
                // (0x8); only the first is named in the repo.
                FUN_0054e220(pVidInf,
                    dword_8E0320.x,
                    dword_8E0320.y,
                    &dword_8E0330,
                    &rClip,
                    &sText,
                    &field_9F4,
                    CUIControlLabel::TYPE_WORD_WRAP | 0x40 | 0x8);
            } else {
                if (a6) {
                    g_pBaldurChitin->GetTlkTable().Fetch(0x407B, strRes);
                    sText.Format(strRes.szText);
                } else {
                    g_pBaldurChitin->GetTlkTable().Fetch(0x407A, strRes);
                    sText.Format("%s %01d", (LPCSTR)strRes.szText, a3);
                }

                pVidInf->BKTextOut(&field_4F4,
                    sText,
                    (nScreenWidth - field_4F4.GetStringLength(sText, TRUE)) >> 1,
                    dword_8AE4AC,
                    rClip,
                    0,
                    TRUE);

                g_pBaldurChitin->GetTlkTable().Fetch(0x407C, strRes);
                sText.Format("%s %s", (LPCSTR)strRes.szText, (LPCSTR)a4);
                pVidInf->BKTextOut(&field_4F4,
                    sText,
                    (nScreenWidth - field_4F4.GetStringLength(sText, TRUE)) >> 1,
                    dword_8AE4B0,
                    rClip,
                    0,
                    TRUE);

                g_pBaldurChitin->GetTlkTable().Fetch(0xAF2, strRes);
                sText.Format("%s", (LPCSTR)strRes.szText);
                pVidInf->BKTextOut(&field_4F4,
                    sText,
                    (nScreenWidth - field_4F4.GetStringLength(sText, TRUE)) >> 1,
                    dword_8AE4B8 + dword_8AE4B4,
                    rClip,
                    0,
                    TRUE);
            }
        }

        pVidInf->BKUnlock();

        field_0 = 1;

        if (!g_pBaldurChitin->field_F8 && !g_pBaldurChitin->m_bReInitializing) {
            pVidInf->Flip(FALSE);
        } else {
            field_0 = 0;
        }
    }

    lock.Unlock();
}
