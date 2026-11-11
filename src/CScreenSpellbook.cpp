#include "CScreenSpellbook.h"

#include "CBaldurChitin.h"
#include "CGameSprite.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CScreenWorld.h"
#include "CSpell.h"
#include "CUIControlFactory.h"
#include "CUIControlTextDisplay.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVidPoly.h"

// 0x667EC0
CScreenSpellbook::CScreenSpellbook()
{
    int index;

    SetVideoMode(0);

    m_pVirtualKeys[0] = CKeyInfo(VK_ESCAPE, -1, 0);
    m_pVirtualKeys[1] = CKeyInfo('1', 0, 0);
    m_pVirtualKeys[2] = CKeyInfo('2', 0, 0);
    m_pVirtualKeys[3] = CKeyInfo('3', 0, 0);
    m_pVirtualKeys[4] = CKeyInfo('4', 0, 0);
    m_pVirtualKeys[5] = CKeyInfo('5', 0, 0);
    m_pVirtualKeys[6] = CKeyInfo('6', 0, 0);
    m_pVirtualKeys[7] = CKeyInfo('7', 0, 0);
    m_pVirtualKeys[8] = CKeyInfo('8', 0, 0);
    m_pVirtualKeys[9] = CKeyInfo('9', 0, 0);
    m_pVirtualKeys[10] = CKeyInfo(VK_SPACE, -1, 0);
    m_pVirtualKeys[11] = CKeyInfo('A', 0, 0);
    m_pVirtualKeys[12] = CKeyInfo('B', 0, 0);
    m_pVirtualKeys[13] = CKeyInfo('C', 0, 0);
    m_pVirtualKeys[14] = CKeyInfo('D', 0, 0);
    m_pVirtualKeys[15] = CKeyInfo('E', 0, 0);
    m_pVirtualKeys[16] = CKeyInfo('F', 0, 0);
    m_pVirtualKeys[17] = CKeyInfo('G', 0, 0);
    m_pVirtualKeys[18] = CKeyInfo('H', 0, 0);
    m_pVirtualKeys[19] = CKeyInfo('I', 0, 0);
    m_pVirtualKeys[20] = CKeyInfo('J', 0, 0);
    m_pVirtualKeys[21] = CKeyInfo('K', 0, 0);
    m_pVirtualKeys[22] = CKeyInfo('L', 0, 0);
    m_pVirtualKeys[23] = CKeyInfo('M', 0, 0);
    m_pVirtualKeys[24] = CKeyInfo('N', 0, 0);
    m_pVirtualKeys[25] = CKeyInfo('O', 0, 0);
    m_pVirtualKeys[26] = CKeyInfo('P', 0, 0);
    m_pVirtualKeys[27] = CKeyInfo('Q', 0, 0);
    m_pVirtualKeys[28] = CKeyInfo('R', 0, 0);
    m_pVirtualKeys[29] = CKeyInfo('S', 0, 0);
    m_pVirtualKeys[30] = CKeyInfo('T', 0, 0);
    m_pVirtualKeys[31] = CKeyInfo('U', 0, 0);
    m_pVirtualKeys[32] = CKeyInfo('V', 0, 0);
    m_pVirtualKeys[33] = CKeyInfo('W', 0, 0);
    m_pVirtualKeys[34] = CKeyInfo('X', 0, 0);
    m_pVirtualKeys[35] = CKeyInfo('Y', 0, 0);
    m_pVirtualKeys[36] = CKeyInfo('Z', 0, 0);
    m_pVirtualKeys[37] = CKeyInfo('0', 0, 0);
    m_pVirtualKeys[38] = CKeyInfo(VK_BACK, 0, 0);
    m_pVirtualKeys[39] = CKeyInfo(VK_END, -1, 0);
    m_pVirtualKeys[40] = CKeyInfo(VK_HOME, -1, 0);
    m_pVirtualKeys[41] = CKeyInfo(VK_LEFT, 0, 0);
    m_pVirtualKeys[42] = CKeyInfo(VK_UP, 0, 0);
    m_pVirtualKeys[43] = CKeyInfo(VK_RIGHT, 0, 0);
    m_pVirtualKeys[44] = CKeyInfo(VK_DOWN, 0, 0);
    m_pVirtualKeys[45] = CKeyInfo(VK_SNAPSHOT, -1, 0);
    m_pVirtualKeys[46] = CKeyInfo(VK_INSERT, -1, 0);
    m_pVirtualKeys[47] = CKeyInfo(VK_DELETE, 0, 0);
    m_pVirtualKeys[48] = CKeyInfo(VK_NUMPAD0, -1, 0);
    m_pVirtualKeys[49] = CKeyInfo(VK_NUMPAD1, -1, 0);
    m_pVirtualKeys[50] = CKeyInfo(VK_NUMPAD2, -1, 0);
    m_pVirtualKeys[51] = CKeyInfo(VK_NUMPAD3, -1, 0);
    m_pVirtualKeys[52] = CKeyInfo(VK_NUMPAD4, -1, 0);
    m_pVirtualKeys[53] = CKeyInfo(VK_NUMPAD5, -1, 0);
    m_pVirtualKeys[54] = CKeyInfo(VK_NUMPAD6, -1, 0);
    m_pVirtualKeys[55] = CKeyInfo(VK_NUMPAD7, -1, 0);
    m_pVirtualKeys[56] = CKeyInfo(VK_NUMPAD8, -1, 0);
    m_pVirtualKeys[57] = CKeyInfo(VK_NUMPAD9, -1, 0);
    m_pVirtualKeys[58] = CKeyInfo(VK_MULTIPLY, -1, 0);
    m_pVirtualKeys[59] = CKeyInfo(VK_ADD, -1, 0);
    m_pVirtualKeys[60] = CKeyInfo(VK_SEPARATOR, -1, 0);
    m_pVirtualKeys[61] = CKeyInfo(VK_SUBTRACT, -1, 0);
    m_pVirtualKeys[62] = CKeyInfo(VK_DECIMAL, -1, 0);
    m_pVirtualKeys[63] = CKeyInfo(VK_DIVIDE, -1, 0);
    m_pVirtualKeys[64] = CKeyInfo(VK_F1, -1, 0);
    m_pVirtualKeys[65] = CKeyInfo(VK_F2, -1, 0);
    m_pVirtualKeys[66] = CKeyInfo(VK_F3, -1, 0);
    m_pVirtualKeys[67] = CKeyInfo(VK_F4, -1, 0);
    m_pVirtualKeys[68] = CKeyInfo(VK_F5, -1, 0);
    m_pVirtualKeys[69] = CKeyInfo(VK_F6, -1, 0);
    m_pVirtualKeys[70] = CKeyInfo(VK_F7, -1, 0);
    m_pVirtualKeys[71] = CKeyInfo(VK_F8, -1, 0);
    m_pVirtualKeys[72] = CKeyInfo(VK_F9, -1, 0);
    m_pVirtualKeys[73] = CKeyInfo(VK_F10, -1, 0);
    m_pVirtualKeys[74] = CKeyInfo(VK_F11, -1, 0);
    m_pVirtualKeys[75] = CKeyInfo(VK_F12, -1, 0);
    m_pVirtualKeys[76] = CKeyInfo(VK_NUMLOCK, -1, 0);
    m_pVirtualKeys[77] = CKeyInfo(VK_RETURN, -1, 0);
    m_pVirtualKeys[78] = CKeyInfo(VK_OEM_3, 0, 0);
    m_pVirtualKeys[79] = CKeyInfo(VK_OEM_MINUS, 0, 0);
    m_pVirtualKeys[80] = CKeyInfo(VK_OEM_PLUS, 0, 0);
    m_pVirtualKeys[81] = CKeyInfo(VK_OEM_4, 0, 0);
    m_pVirtualKeys[82] = CKeyInfo(VK_OEM_6, 0, 0);
    m_pVirtualKeys[83] = CKeyInfo(VK_OEM_5, 0, 0);
    m_pVirtualKeys[84] = CKeyInfo(VK_OEM_1, 0, 0);
    m_pVirtualKeys[85] = CKeyInfo(VK_OEM_7, 0, 0);
    m_pVirtualKeys[86] = CKeyInfo(VK_OEM_COMMA, 0, 0);
    m_pVirtualKeys[87] = CKeyInfo(VK_OEM_PERIOD, 0, 0);
    m_pVirtualKeys[88] = CKeyInfo(VK_OEM_2, 0, 0);
    m_pVirtualKeys[89] = CKeyInfo(VK_TAB, -1, 0);
    m_pVirtualKeys[90] = CKeyInfo(VK_PRIOR, -1, 0);
    m_pVirtualKeys[91] = CKeyInfo(VK_NEXT, -1, 0);

    m_nSpellLevel = 0;
    m_nClassIndex = 11;
    m_nNumKnownSpells = 0;
    m_nTopKnownSpell = 0;
    m_pFlashCurrentSpell = NULL;
    m_bFlash = FALSE;
    m_bFlashMemorize = FALSE;
    m_bFlashUnmemorize = FALSE;
    m_nErrorState = 0;
    m_nNumErrorButtons = 0;
    m_bMultiPlayerViewable = FALSE;
    field_582 = 0;
    m_bCtrlKeyDown = FALSE;
    m_pFlashMemorizeSourceSpell = NULL;
    m_pFlashMemorizeDestSpell = NULL;
    field_1488 = 0;

    for (index = 0; index < 24; index++) {
        field_148C[index] = CResRef();
        field_154C[index] = 0;
        field_15AC[index] = 0;
    }

    field_1670 = 0;

    for (index = 0; index < CSPELLLIST_NUM_CLASSES; index++) {
        field_1654[index] = 0;
    }
}

// 0x668C60
CScreenSpellbook::~CScreenSpellbook()
{
}

// 0x49FC40
BOOL CScreenSpellbook::CheckSystemKeyCtrl()
{
    return TRUE;
}

// 0x668C20
void CScreenSpellbook::SetSystemKeyCtrl(BOOLEAN bValue)
{
    m_bCtrlKeyDown = bValue;
}

// 0x668D10
void CScreenSpellbook::EngineActivated()
{
    if (CChitin::byte_8FB950
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE
        && g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(0, 1, 0);
    }

    m_preLoadFontRealms.SetResRef(CResRef("REALMS"), FALSE, TRUE);
    m_preLoadFontRealms.RegisterFont();

    m_preLoadFontStnSml.SetResRef(CResRef("STONESML"), FALSE, TRUE);
    m_preLoadFontStnSml.RegisterFont();

    m_preLoadFontTool.SetResRef(CResRef("TOOLFONT"), FALSE, TRUE);
    m_preLoadFontTool.RegisterFont();

    UpdateMainPanel();
    UpdateCursorShape(0);
    CheckEnablePortaits(1);
    CheckEnableLeftPanel();
    m_cUIManager.InvalidateRect(NULL);
}

// 0x668E30
void CScreenSpellbook::EngineDeactivated()
{
    if (CChitin::byte_8FB950) {
        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
            && g_pChitin->cNetwork.GetSessionHosting() == TRUE
            && g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            if (!g_pBaldurChitin->m_pEngineWorld->m_bPaused) {
                g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(0, 1, 0);
            }
        }
    }

    m_preLoadFontRealms.Unload();
    m_preLoadFontStnSml.Unload();
    m_preLoadFontTool.Unload();

    // NOTE: Uninline.
    m_cUIManager.KillCapture();
}

// 0x668ED0
void CScreenSpellbook::EngineGameInit()
{
    m_cUIManager.fInit(this, CResRef("GUISPL"), g_pBaldurChitin->field_4A28);

    CPoint pt;
    if (g_pBaldurChitin->field_4A28) {
        pt.x = CVideo::SCREENWIDTH / 2 - CBaldurChitin::DEFAULT_SCREEN_WIDTH;
        pt.y = CVideo::SCREENHEIGHT / 2 - CBaldurChitin::DEFAULT_SCREEN_HEIGHT;
    } else {
        pt.x = (CVideo::SCREENWIDTH - CBaldurChitin::DEFAULT_SCREEN_WIDTH) / 2;
        pt.y = (CVideo::SCREENHEIGHT - CBaldurChitin::DEFAULT_SCREEN_HEIGHT) / 2;
    }

    m_cUIManager.ShiftPanels(pt);

    for (int side = 0; side < 4; side++) {
        m_cUIManager.AddPanel(&(g_pBaldurChitin->m_aBorderPanels[side]));
    }

    m_nSelectedCharacter = -1;
    m_pCurrentScrollBar = NULL;
    m_nSpellLevel = 0;
    m_bFlash = FALSE;
    m_bFlashMemorize = FALSE;
    m_bFlashUnmemorize = FALSE;
    m_pFlashCurrentSpell = NULL;
    m_spellResRef = "";

    for (int index = 0; index < 24; index++) {
        field_148C[index] = CResRef();
        field_154C[index] = 0;
    }

    m_cUIManager.GetPanel(3)->SetActive(FALSE);
    m_cUIManager.GetPanel(4)->SetActive(FALSE);
    m_cUIManager.GetPanel(5)->SetActive(FALSE);
    m_cUIManager.GetPanel(50)->SetActive(FALSE);

    // NOTE: Uninline.
    m_vcFlash.SetResRef(CResRef("FLASH"), g_pBaldurChitin->field_4A28, TRUE, TRUE);
}

// 0x5D72C0
void CScreenSpellbook::EngineGameUninit()
{
    // NOTE: Uninline.
    m_cUIManager.fUninit();
}

// 0x6692A0
void CScreenSpellbook::OnKeyDown(SHORT nKeysFlags)
{
    CUIControlScrollBarSpellbookKnownSpells* pScrollBar;

    if (!m_bFlashUnmemorize && !m_bFlashMemorize) {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        for (SHORT nKeyFlag = 0; nKeyFlag < nKeysFlags; nKeyFlag++) {
            switch (m_pVirtualKeysFlags[nKeyFlag]) {
            case VK_TAB:
                m_cUIManager.ForceToolTip();
                break;
            case VK_RETURN:
                if (GetTopPopup() != NULL) {
                    OnDoneButtonClick();
                }
                break;
            case VK_ESCAPE:
                if (GetTopPopup() != NULL) {
                    OnCancelButtonClick();
                } else {
                    SelectEngine(g_pBaldurChitin->m_pEngineWorld);
                }
                break;
            case VK_PRIOR:
                if (GetTopPopup() == NULL) {
                    m_cUIManager.ClearTooltip();

                    pScrollBar = static_cast<CUIControlScrollBarSpellbookKnownSpells*>(m_cUIManager.GetPanel(2)->GetControl(54));
                    if (pScrollBar != NULL) {
                        pScrollBar->OnPageUp(-1);
                    }
                }
                break;
            case VK_NEXT:
                if (GetTopPopup() == NULL) {
                    m_cUIManager.ClearTooltip();

                    pScrollBar = static_cast<CUIControlScrollBarSpellbookKnownSpells*>(m_cUIManager.GetPanel(2)->GetControl(54));
                    if (pScrollBar != NULL) {
                        pScrollBar->OnPageDown(-1);
                    }
                }
                break;
            case VK_SNAPSHOT:
                g_pBaldurChitin->GetCurrentVideoMode()->PrintScreen();
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
                if (GetTopPopup() == NULL) {
                    OnPortraitLClick(m_pVirtualKeysFlags[nKeyFlag] - '1');
                }
                break;
            case VK_NUMPAD1:
            case VK_NUMPAD2:
            case VK_NUMPAD3:
            case VK_NUMPAD4:
            case VK_NUMPAD5:
            case VK_NUMPAD6:
            case VK_NUMPAD7:
            case VK_NUMPAD8:
            case VK_NUMPAD9:
                if (GetTopPopup() == NULL) {
                    CUIControlButton* pButton = static_cast<CUIControlButton*>(m_cUIManager.GetPanel(2)->GetControl(m_pVirtualKeysFlags[nKeyFlag] - 42));
                    if (pButton != NULL) {
                        pButton->OnLButtonClick(CPoint(0, 0));
                    }
                }
                break;
            default:
                if (GetTopPopup() == NULL) {
                    for (SHORT index = 0; index < CINFGAME_KEYMAP_SIZE; index) {
                        // NOTE: Uninline.
                        if (pGame->GetKeymap(index) == m_pVirtualKeysFlags[nKeyFlag]
                            && pGame->GetKeymapFlag(index) == m_bCtrlKeyDown) {
                            switch (index) {
                            case 0:
                                g_pBaldurChitin->GetActiveEngine()->OnLeftPanelButtonClick(5);
                                break;
                            case 1:
                                g_pBaldurChitin->GetActiveEngine()->OnLeftPanelButtonClick(8);
                                break;
                            case 2:
                                g_pBaldurChitin->GetActiveEngine()->OnLeftPanelButtonClick(0);
                                break;
                            case 3:
                                g_pBaldurChitin->GetActiveEngine()->OnLeftPanelButtonClick(6);
                                break;
                            case 4:
                                g_pBaldurChitin->GetActiveEngine()->OnLeftPanelButtonClick(7);
                                break;
                            case 6:
                                g_pBaldurChitin->GetActiveEngine()->OnLeftPanelButtonClick(9);
                                break;
                            case 7:
                                g_pBaldurChitin->GetActiveEngine()->OnLeftPanelButtonClick(13);
                                break;
                            case 29:
                                OnRestButtonClick();
                                break;
                            default:
                                break;
                            }
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
}

// 0x49FC40
BOOL CScreenSpellbook::CheckMouseLButton()
{
    return TRUE;
}

// 0x6696D0
void CScreenSpellbook::OnLButtonDblClk(CPoint pt)
{
    if (!m_bFlashUnmemorize && !m_bFlashMemorize) {
        m_cUIManager.OnLButtonDblClk(pt);
    }
}

// 0x669700
void CScreenSpellbook::OnLButtonDown(CPoint pt)
{
    g_pBaldurChitin->GetObjectCursor()->m_nState = 1;

    if (!m_bFlashUnmemorize && !m_bFlashMemorize) {
        m_cUIManager.OnLButtonDown(pt);
    }
}

// 0x669740
void CScreenSpellbook::OnLButtonUp(CPoint pt)
{
    g_pBaldurChitin->GetObjectCursor()->m_nState = 0;

    if (!m_bFlashUnmemorize && !m_bFlashMemorize) {
        m_cUIManager.OnLButtonUp(pt);
    }
}

// 0x49FC40
BOOL CScreenSpellbook::CheckMouseMove()
{
    return TRUE;
}

// 0x669780
void CScreenSpellbook::OnMouseMove(CPoint pt)
{
    if (!m_bFlashUnmemorize && !m_bFlashMemorize) {
        m_cUIManager.OnMouseMove(pt);
    }
}

// 0x49FC40
BOOL CScreenSpellbook::CheckMouseRButton()
{
    return TRUE;
}

// 0x6697B0
void CScreenSpellbook::OnRButtonDown(CPoint pt)
{
    g_pBaldurChitin->GetObjectCursor()->m_nState = 1;

    if (!m_bFlashUnmemorize && !m_bFlashMemorize) {
        m_cUIManager.OnRButtonDown(pt);
    }
}

// 0x6697F0
void CScreenSpellbook::OnRButtonUp(CPoint pt)
{
    g_pBaldurChitin->GetObjectCursor()->m_nState = 0;

    if (!m_bFlashUnmemorize && !m_bFlashMemorize) {
        m_cUIManager.OnRButtonUp(pt);
    }
}

// 0x686660
SHORT CScreenSpellbook::GetNumVirtualKeys()
{
    return CSCREENSPELLBOOK_VIRTUAL_KEYS;
}

// 0x699610
CKeyInfo* CScreenSpellbook::GetVirtualKeys()
{
    return m_pVirtualKeys;
}

// 0x668C30
BYTE* CScreenSpellbook::GetVirtualKeysFlags()
{
    return m_pVirtualKeysFlags;
}

// 0x669830
void CScreenSpellbook::sub_669830(DWORD nPortrait)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        BYTE nLastSpellLevel = pSprite->m_nLastSpellbookSpellLevel;

        UINT nLevel = 0;
        if (nLastSpellLevel < 9) {
            nLevel = nLastSpellLevel;
        }

        pSprite->m_nLastSpellbookSpellLevel = static_cast<BYTE>(nLevel);
        m_nSpellLevel = nLevel;

        BOOLEAN bHaveSpells = FALSE;

        m_nNumberOfSpellClasses = 0;
        field_1670 = -1;

        for (UINT nClassIndex = 0; nClassIndex < CSPELLLIST_NUM_CLASSES; nClassIndex++) {
            BYTE nClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nClassIndex);

            if (pSprite->GetAIType().IsClassValid(nClass)
                && pSprite->GetNumSpells() > 0) {
                field_1654[m_nNumberOfSpellClasses] = nClass;
                m_nNumberOfSpellClasses++;

                if (nClassIndex == 1) {
                    field_1670 = m_nNumberOfSpellClasses;
                    field_1654[m_nNumberOfSpellClasses] = 7;
                    m_nNumberOfSpellClasses++;
                }

                bHaveSpells = TRUE;
            }
        }

        if (m_nNumberOfSpellClasses != 0) {
            BYTE nLastClassIndex = pSprite->m_nLastSpellbookClassIndex;
            BYTE nNewClassIndex = nLastClassIndex < m_nNumberOfSpellClasses ? nLastClassIndex : 0;
            pSprite->m_nLastSpellbookClassIndex = nNewClassIndex;
            m_nClassIndex = nNewClassIndex;
        } else {
            m_nClassIndex = 0;
            pSprite->m_nLastSpellbookClassIndex = 0;
        }

        DWORD nNumberOfSpellClasses = m_nNumberOfSpellClasses;

        field_1488 = 0;

        for (INT nIndex = 0; nIndex < 24; nIndex++) {
            field_148C[nIndex] = CResRef();
            field_154C[nIndex] = 0;
            field_15AC[nIndex] = 0;
        }

        if (bHaveSpells == TRUE) {
            UINT nSpellLevel = m_nSpellLevel;

            CGameSpriteSpellList* pSpellList;
            if (field_1670 == m_nClassIndex) {
                // __FILE__: .\Include\ObjCreature.h
                // __LINE__: 1763
                UTIL_ASSERT(nSpellLevel < CSPELLLIST_MAX_LEVELS);

                // NOTE: Uninline.
                pSpellList = pSprite->m_domainSpells.GetSpellsAtLevel(nSpellLevel);
            } else {
                BYTE nClass = static_cast<BYTE>(sub_66A010(m_nClassIndex));
                pSpellList = pSprite->GetSpellsAtLevel(nClass, nSpellLevel);
            }

            for (UINT nIndex = 0; nIndex < 24; nIndex++) {
                if (nIndex < pSpellList->m_List.size()) {
                    // __FILE__: .\Include\FileFormat.h
                    // __LINE__: 2428
                    UTIL_ASSERT(pSpellList->m_List.size() > 0);

                    // __FILE__: .\Include\FileFormat.h
                    // __LINE__: 2429
                    UTIL_ASSERT(nIndex < pSpellList->m_List.size());

                    UINT nField4 = pSpellList->m_List[nIndex].field_4;

                    // __FILE__: .\Include\FileFormat.h
                    // __LINE__: 2428
                    UTIL_ASSERT(pSpellList->m_List.size() > 0);

                    // __FILE__: .\Include\FileFormat.h
                    // __LINE__: 2429
                    UTIL_ASSERT(nIndex < pSpellList->m_List.size());

                    UINT nField8 = pSpellList->m_List[nIndex].field_8;

                    if (nField4 > 0) {
                        // __FILE__: .\Include\FileFormat.h
                        // __LINE__: 2428
                        UTIL_ASSERT(pSpellList->m_List.size() > 0);

                        // __FILE__: .\Include\FileFormat.h
                        // __LINE__: 2429
                        UTIL_ASSERT(nIndex < pSpellList->m_List.size());

                        if ((pSpellList->m_List[nIndex].field_C & 1) == 0) {
                            // __FILE__: .\Include\FileFormat.h
                            // __LINE__: 2428
                            UTIL_ASSERT(pSpellList->m_List.size() > 0);

                            // __FILE__: .\Include\FileFormat.h
                            // __LINE__: 2429
                            UTIL_ASSERT(nIndex < pSpellList->m_List.size());

                            UINT nSpellId = pSpellList->m_List[nIndex].m_nID;

                            // NOTE: Uninline.
                            field_148C[field_1488] = g_pBaldurChitin->GetObjectGame()->m_spells.Get(nSpellId);
                            field_154C[field_1488] = nField8;
                            field_15AC[field_1488] = nField4;
                            field_1488++;
                        }
                    }
                }
            }
        }

        CString sText;

        UINT nSelectedClassIndex = m_nClassIndex;
        UINT nStart = 0;
        if (nSelectedClassIndex >= 4) {
            nStart = 4;
        }

        CUIPanel* pPanel = m_cUIManager.GetPanel(2);

        UINT nDomainClassIndex = field_1670;

        CUIControlButtonSpellbookClassSelection* pButton;

        for (UINT nButton = 0; nButton < 4; nButton++) {
            UINT nClassSlot = nStart + nButton;

            pButton = static_cast<CUIControlButtonSpellbookClassSelection*>(pPanel->GetControl(0x58 + nButton));
            pButton->SetSelected(FALSE);

            if (nClassSlot < nNumberOfSpellClasses) {
                pButton->SetEnabled(TRUE);

                if (nClassSlot == nDomainClassIndex) {
                    STR_RES strRes;
                    g_pBaldurChitin->GetTlkTable().Fetch(0x9B2A, strRes);
                    sText = strRes.szText;
                } else {
                    // __FILE__: .\Include\InfScreenSpellbook.h
                    // __LINE__: 76
                    UTIL_ASSERT(nClassSlot < CSPELLLIST_NUM_CLASSES);

                    pGame->GetRuleTables().GetClassStringMixed(static_cast<BYTE>(field_1654[nClassSlot]),
                        0,
                        0,
                        sText,
                        TRUE);
                }
            } else {
                sText = " ";
                pButton->SetEnabled(FALSE);
            }

            pButton->SetText(sText);
        }

        UINT nSelectedButton = nSelectedClassIndex;
        if (nSelectedButton >= 4) {
            nSelectedButton -= 4;
        }

        if (m_nNumberOfSpellClasses > 0) {
            pButton = static_cast<CUIControlButtonSpellbookClassSelection*>(pPanel->GetControl(0x58 + nSelectedButton));
            pButton->SetSelected(TRUE);
        }

        // NOTE: Unused.
        CString sUnused;

        for (UINT nSpellLevelButton = 0; nSpellLevelButton < 9; nSpellLevelButton++) {
            pButton->SetEnabled(m_nNumberOfSpellClasses > 0);

            static_cast<CUIControlButtonSpellbookLevelSelection*>(pPanel->GetControl(0x37 + nSpellLevelButton))->SetSelected(FALSE);
        }

        static_cast<CUIControlButtonSpellbookLevelSelection*>(pPanel->GetControl(0x37 + nLevel))->SetSelected(m_nNumberOfSpellClasses > 0);

        m_nTopKnownSpell = 0;

        CUIControlScrollBar* pScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(0x36));

        CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 3973
        UTIL_ASSERT(pSpellbook != NULL);

        pScrollBar->AdjustScrollBar(pSpellbook->m_nTopKnownSpell, pSpellbook->m_nNumKnownSpells, 8);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x66A010
int CScreenSpellbook::sub_66A010(DWORD nIndex)
{
    // __FILE__: .\Include\InfScreenSpellbook.h
    // __LINE__: 76
    UTIL_ASSERT(nIndex < CSPELLLIST_NUM_CLASSES);

    return field_1654[nIndex];
}

// 0x66A040
void CScreenSpellbook::OnPortraitLClick(DWORD nPortrait)
{
    if (!m_bFlashUnmemorize && !m_bFlashMemorize) {
        // NOTE: Unused.
        CUIPanel* pPanel = m_cUIManager.GetPanel(2);

        // NOTE: Unsigned compare.
        if (nPortrait < static_cast<DWORD>(g_pBaldurChitin->GetObjectGame()->m_nCharacters)) {
            DWORD nOldPortrait = m_nSelectedCharacter;
            m_nSelectedCharacter = nPortrait;

            sub_669830(nPortrait);
            CheckMultiPlayerViewable();

            UpdateCursorShape(0);
            UpdateMainPanel();

            m_cUIManager.GetPanel(2)->InvalidateRect(NULL);

            if (nOldPortrait != -1) {
                m_cUIManager.GetPanel(1)->GetControl(nOldPortrait)->InvalidateRect();
            }

            m_cUIManager.GetPanel(1)->GetControl(m_nSelectedCharacter)->InvalidateRect();
        }
    }
}

// 0x66A100
void CScreenSpellbook::SetSpellLevel(INT nLevel)
{
    if (nLevel < 0 || nLevel >= 9) {
        return;
    }

    m_nSpellLevel = nLevel;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 1182
    UTIL_ASSERT(pGame != NULL);

    LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        pSprite->m_nLastSpellbookSpellLevel = static_cast<BYTE>(nLevel);

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x66A1C0
void CScreenSpellbook::SetClassIndex(DWORD nNewClassIndex)
{
    DWORD nNumberOfSpellClasses = m_nNumberOfSpellClasses;
    DWORD nIndex = m_nClassIndex;

    // __FILE__: .\Include\InfScreenSpellbook.h
    // __LINE__: 76
    UTIL_ASSERT(nIndex < CSPELLLIST_NUM_CLASSES);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 1236
    UTIL_ASSERT(pGame != NULL);

    LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 1252
        UTIL_ASSERT(nNewClassIndex < nNumberOfSpellClasses);

        pSprite->m_nLastSpellbookClassIndex = static_cast<BYTE>(nNewClassIndex);
        m_nClassIndex = static_cast<BYTE>(nNewClassIndex);

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x66A2D0
void CScreenSpellbook::UpdateFlash()
{
    if (!m_bFlash) {
        return;
    }

    if (m_vcFlash.m_nCurrentFrame + 1 < m_vcFlash.GetSequenceLength(m_vcFlash.m_nCurrentSequence, FALSE)) {
        m_vcFlash.FrameAdvance();
    } else {
        m_bFlash = FALSE;
    }

    if (m_pFlashCurrentSpell != NULL) {
        m_pFlashCurrentSpell->InvalidateRect();
    }

    if (!m_bFlash) {
        if (m_bFlashUnmemorize) {
            // When unmemorizing the flash appears only in that particular slot.
            m_bFlashUnmemorize = FALSE;
            UpdateMainPanel();
            m_cUIManager.GetPanel(2)->InvalidateRect(NULL);
        } else if (m_bFlashMemorize) {
            // When memorizing spell the flash first appears in "Known spells"
            // slot, than in "Memorized spells".
            if (m_pFlashCurrentSpell == m_pFlashMemorizeSourceSpell) {
                UpdateMainPanel();

                if (m_pFlashCurrentSpell != NULL) {
                    m_pFlashCurrentSpell = m_pFlashMemorizeDestSpell;
                }

                m_bFlash = TRUE;

                m_vcFlash.FrameSet(0);

                if (m_pFlashCurrentSpell != NULL) {
                    m_pFlashCurrentSpell->InvalidateRect();
                }
            } else {
                m_bFlashMemorize = FALSE;
            }
        } else {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
            // __LINE__: 1378
            UTIL_ASSERT(FALSE);
        }
    }
}

// 0x66A400
void CScreenSpellbook::DrawFlash()
{
    CVidInf* pVidInf = static_cast<CVidInf*>(g_pBaldurChitin->GetCurrentVideoMode());

    if (m_pFlashCurrentSpell != NULL) {
        CRect rControlFrame(m_pFlashCurrentSpell->m_pPanel->m_ptOrigin + m_pFlashCurrentSpell->m_ptOrigin,
            m_pFlashCurrentSpell->m_size);

        CSize flashSize;
        m_vcFlash.GetCurrentFrameSize(flashSize, FALSE);

        INT x = m_pFlashCurrentSpell->m_size.cx > flashSize.cx
            ? rControlFrame.left + (m_pFlashCurrentSpell->m_size.cx - flashSize.cx) / 2
            : rControlFrame.left;
        INT y = m_pFlashCurrentSpell->m_size.cy > flashSize.cy
            ? rControlFrame.top + (m_pFlashCurrentSpell->m_size.cy - flashSize.cy) / 2
            : rControlFrame.top;

        CRect rFlash(CPoint(x, y), flashSize);

        CRect rClip;
        rClip.IntersectRect(rFlash, m_pFlashCurrentSpell->m_rDirty);

        // FIXME: Result is ignored.
        pVidInf->BKLock(rClip);

        BOOL bResult = pVidInf->BKRender(&m_vcFlash,
            x - rClip.left,
            y - rClip.top,
            0x8,
            -1);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 1539
        UTIL_ASSERT(bResult);

        pVidInf->BKUnlock();
    }
}

// 0x66A540
void CScreenSpellbook::TimerAsynchronousUpdate()
{
    g_pBaldurChitin->m_pEngineWorld->AsynchronousUpdate(FALSE);
    UpdateCursorShape(0);
    m_cUIManager.TimerAsynchronousUpdate();
    g_pBaldurChitin->GetObjectCursor()->CursorUpdate(pVidMode);
}

// 0x66A580
void CScreenSpellbook::TimerSynchronousUpdate()
{
    g_pBaldurChitin->GetObjectGame()->SynchronousUpdate();
    m_cUIManager.Render();

    for (INT nIndex = 0; nIndex < m_lPopupStack.GetCount(); nIndex++) {
        POSITION pos = m_lPopupStack.FindIndex(nIndex);
        if (pos != NULL) {
            CUIPanel* pPanel = m_lPopupStack.GetAt(pos);
            pPanel->SetActive(TRUE);
            pPanel->SetInactiveRender(TRUE);
            pPanel->InvalidateRect(NULL);

            switch (pPanel->m_nID) {
            case 3:
                // NOTE: Uninline.
                UpdateSpellInfoPanel();
                break;
            case 4:
            case 5:
            case 50:
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                // __LINE__: 2145
                UTIL_ASSERT(FALSE);
            }
        }
    }

    pVidMode->Flip(TRUE);
    UpdateFlash();
}

// 0x66A750
void CScreenSpellbook::EnableMainPanel(BOOL bEnable)
{
    CUIPanel* pMainPanel = m_cUIManager.GetPanel(2);
    CUIPanel* pLeftPanel = m_cUIManager.GetPanel(0);
    CUIPanel* pRightPanel = m_cUIManager.GetPanel(1);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4222
    UTIL_ASSERT(pMainPanel != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4223
    UTIL_ASSERT(pLeftPanel != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4224
    UTIL_ASSERT(pRightPanel != NULL);

    if (bEnable) {
        PlayGUISound(RESREF_SOUND_WINDOWCLOSE);
        m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(pMainPanel->GetControl(54));
    }

    pMainPanel->SetEnabled(bEnable);
    pLeftPanel->SetEnabled(bEnable);
    pRightPanel->SetEnabled(bEnable);

    if (CVideo::SCREENWIDTH / (g_pBaldurChitin->field_4A28 ? 2 : 1) != CBaldurChitin::DEFAULT_SCREEN_WIDTH) {
        m_cUIManager.GetPanel(-5)->SetEnabled(bEnable);
        m_cUIManager.GetPanel(-4)->SetEnabled(bEnable);
        m_cUIManager.GetPanel(-3)->SetEnabled(bEnable);
        m_cUIManager.GetPanel(-2)->SetEnabled(bEnable);
    }

    CheckEnablePortaits(1);
    CheckEnableLeftPanel();
}

// 0x66A980
void CScreenSpellbook::UpdateMainPanel()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 1803
    UTIL_ASSERT(pGame != NULL);

    LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        // NOTE: Result is unused.
        pSprite->GetAIType();

        // NOTE: Uninline.
        BOOLEAN bInControl = pSprite->InControl();

        CUIPanel* pPanel = m_cUIManager.GetPanel(2);

        m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(54));

        CUIControlScrollBar* pScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(54));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 1826
        UTIL_ASSERT(pScrollBar != NULL);

        CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 3973
        UTIL_ASSERT(pSpellbook != NULL);

        pScrollBar->AdjustScrollBar(pSpellbook->m_nTopKnownSpell, pSpellbook->m_nNumKnownSpells, 8);

        if (pSprite->m_baseStats.m_name == 0xFFFFFFFF) {
            UpdateLabel(pPanel, 0xFFFFFFF, "%s", (LPCSTR)pSprite->GetName());
        } else {
            UpdateLabel(pPanel, 0xFFFFFFF, "%s", (LPCSTR)FetchString(pSprite->m_baseStats.m_name));
        }

        CUIControlButtonCharacterPortrait* pPortrait = static_cast<CUIControlButtonCharacterPortrait*>(pPanel->GetControl(1));
        pPortrait->SetPortrait(pSprite->m_baseStats.m_portraitLarge);

        if (m_nNumberOfSpellClasses != 0) {
            UINT nLevel = m_nSpellLevel;

            CGameSpriteSpellList* pSpellList;
            if (field_1670 == m_nClassIndex) {
                // __FILE__: .\Include\ObjCreature.h
                // __LINE__: 1763
                UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

                // NOTE: Uninline.
                pSpellList = pSprite->m_domainSpells.GetSpellsAtLevel(nLevel);
            } else {
                DWORD nClassIndex = m_nClassIndex;

                // __FILE__: .\Include\InfScreenSpellbook.h
                // __LINE__: 76
                UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

                BYTE nClass = static_cast<BYTE>(field_1654[nClassIndex]);
                pSpellList = pSprite->GetSpellsAtLevel(nClass, nLevel);
            }

            INT nMaxSpells = pSpellList->field_10;

            DWORD nClassIndex = m_nClassIndex;

            // __FILE__: .\Include\InfScreenSpellbook.h
            // __LINE__: 76
            UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

            INT nClassType = field_1654[nClassIndex];

            INT nMemorizable = sub_66C4B0();
            if (nMemorizable < 0) {
                nMemorizable = 0;
            }

            INT bCanMemorize = 1;
            if (nClassType == 2 || nClassType == 10) {
                bCanMemorize = 0;
                UpdateLabel(pPanel, 0x10000004, "%d/%d", nMemorizable, pSpellList->field_14);
            } else {
                UpdateLabel(pPanel, 0x10000004, "%d", nMemorizable);
            }

            UINT nSpellListSize = pSpellList->m_List.size();

            for (INT nButton = 0; nButton < 24; nButton++) {
                if (static_cast<UINT>(nButton) < nSpellListSize && bCanMemorize == 1) {
                    CUIControlButtonSpellbookSpell* pButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(6 + nButton));

                    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                    // __LINE__: 1886
                    UTIL_ASSERT(pButton != NULL);

                    BOOL bEnable = nButton < nMaxSpells && m_bMultiPlayerViewable && bInControl;
                    if (pButton->field_67A != bEnable) {
                        pButton->field_67A = bEnable;
                        pButton->InvalidateRect();
                    }

                    if (field_148C[nButton] != CResRef()) {
                        pButton->SetSpell(field_148C[nButton]);

                        if (pButton->field_676 != 1) {
                            pButton->field_676 = 1;
                            pButton->InvalidateRect();
                        }

                        pButton->SetEnabled(TRUE);

                        UpdateLabel(pPanel,
                            0x1000003F + nButton,
                            "%d/%d",
                            field_154C[nButton],
                            field_15AC[nButton]);
                    } else {
                        if (pButton->field_676 != 0) {
                            pButton->field_676 = 0;
                            pButton->InvalidateRect();
                        }

                        pButton->SetSpell(CResRef(""));
                        pButton->SetEnabled(FALSE);

                        UpdateLabel(pPanel, 0x1000003F + nButton, " ");
                    }
                } else {
                    CUIControlButtonSpellbookSpell* pButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(6 + nButton));

                    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                    // __LINE__: 1912
                    UTIL_ASSERT(pButton != NULL);

                    if (pButton->field_676 != 0) {
                        pButton->field_676 = 0;
                        pButton->InvalidateRect();
                    }

                    pButton->SetSpell(CResRef(""));
                    pButton->SetEnabled(FALSE);

                    UpdateLabel(pPanel, 0x1000003F + nButton, " ");
                }
            }

            UINT nKnownSpellListSize = pSpellList->m_List.size();

            for (INT nButton = 0; nButton < 8; nButton++) {
                UINT nIndex = m_nTopKnownSpell + nButton;
                if (nIndex < nKnownSpellListSize) {
                    // NOTE: Uninline.
                    UINT nSpellId = pSpellList->Get(nIndex)->m_nID;

                    // NOTE: Uninline.
                    const CResRef& spellResRef = g_pBaldurChitin->GetObjectGame()->m_spells.Get(nSpellId);

                    CSpell cSpell(spellResRef);
                    cSpell.Demand();

                    UpdateLabel(pPanel,
                        0x10000025 + nButton,
                        "%s",
                        (LPCSTR)FetchString(cSpell.GetGenericName()));

                    static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(46 + nButton))->SetEnabled(bCanMemorize);

                    field_160C[nButton] = spellResRef;

                    CUIControlButtonSpellbookSpell* pKnownButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(30 + nButton));
                    pKnownButton->SetSpell(field_160C[nButton]);
                    pKnownButton->SetEnabled(TRUE);

                    cSpell.Release();
                } else {
                    UpdateLabel(pPanel, 0x10000025 + nButton, "");

                    static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(46 + nButton))->SetEnabled(FALSE);

                    CUIControlButtonSpellbookSpell* pKnownButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(30 + nButton));
                    pKnownButton->SetEnabled(FALSE);
                    pKnownButton->SetSpell(CResRef(""));
                }
            }

            m_nNumKnownSpells = nSpellListSize;
        } else {
            UpdateLabel(pPanel, 0x10000004, " ");

            for (INT nButton = 0; nButton < 24; nButton++) {
                CUIControlButtonSpellbookSpell* pButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(6 + nButton));

                if (pButton->field_676 != 0) {
                    pButton->field_676 = 0;
                    pButton->InvalidateRect();
                }

                pButton->SetSpell(CResRef(""));
                pButton->SetEnabled(FALSE);

                UpdateLabel(pPanel, 0x1000003F + nButton, " ");
            }

            for (INT nButton = 0; nButton < 8; nButton++) {
                UpdateLabel(pPanel, 0x10000025 + nButton, "");

                static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(46 + nButton))->SetEnabled(FALSE);

                CUIControlButtonSpellbookSpell* pKnownButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(30 + nButton));
                pKnownButton->SetEnabled(FALSE);
                pKnownButton->SetSpell(CResRef(""));
            }

            m_nNumKnownSpells = 0;
        }

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// NOTE: Inlined.
CUIPanel* CScreenSpellbook::GetTopPopup()
{
    return m_lPopupStack.GetTailPosition() != NULL ? m_lPopupStack.GetTail() : NULL;
}

// NOTE: Inlined.
void CScreenSpellbook::ShowPopupPanel(DWORD dwPanelId, BOOL bShow)
{
    CUIPanel* pPanel = m_cUIManager.GetPanel(dwPanelId);

    pPanel->SetActive(bShow);
    pPanel->SetInactiveRender(bShow);

    if (bShow) {
        pPanel->InvalidateRect(NULL);
        PlayGUISound(RESREF_SOUND_WINDOWOPEN);
    }
}

// NOTE: Inlined.
void CScreenSpellbook::EnablePopupPanel(DWORD dwPanelId, BOOL bEnable)
{
    CUIPanel* pPanel = m_cUIManager.GetPanel(dwPanelId);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2064
    UTIL_ASSERT(pPanel != NULL);

    pPanel->SetEnabled(bEnable);
}

// NOTE: Inlined.
void CScreenSpellbook::ResetPopupPanel(DWORD dwPanelId)
{
    CUIPanel* pPanel = m_cUIManager.GetPanel(dwPanelId);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2091
    UTIL_ASSERT(pPanel != NULL);

    switch (pPanel->m_nID) {
    case 3:
        ResetSpellInfoPanel(pPanel);
        break;
    case 4:
    case 5:
    case 50:
        ResetErrorPanel(pPanel);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2106
        UTIL_ASSERT(FALSE);
    }
}

// NOTE: Uninline.
void CScreenSpellbook::UpdatePopupPanel(DWORD dwPanelId)
{
    switch (dwPanelId) {
    case 3:
        // NOTE: Uninline.
        UpdateSpellInfoPanel();
        break;
    case 4:
    case 5:
    case 50:
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2145
        UTIL_ASSERT(FALSE);
    }
}

// 0x66B3C0
void CScreenSpellbook::SummonPopup(DWORD dwPopupId)
{
    // NOTE: Uninline.
    m_cUIManager.KillCapture();

    if (!m_lPopupStack.IsEmpty()) {
        CUIPanel* pPanel = m_lPopupStack.GetTail();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2225
        UTIL_ASSERT(pPanel != NULL);

        // NOTE: Uninline.
        EnablePopupPanel(pPanel->m_nID, FALSE);
    } else {
        // NOTE: Uninline.
        EnableMainPanel(FALSE);
    }

    CUIPanel* pPanel = m_cUIManager.GetPanel(dwPopupId);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2231
    UTIL_ASSERT(pPanel != NULL);

    m_lPopupStack.AddTail(pPanel);

    // NOTE: Uninline.
    ResetPopupPanel(pPanel->m_nID);

    // NOTE: Uninline.
    ShowPopupPanel(pPanel->m_nID, TRUE);

    // NOTE: Uninline.
    EnablePopupPanel(pPanel->m_nID, TRUE);

    // NOTE: Uninline.
    UpdatePopupPanel(pPanel->m_nID);
}

// 0x66B6F0
void CScreenSpellbook::DismissPopup()
{
    // NOTE: Uninline.
    m_cUIManager.KillCapture();

    CUIPanel* pPanel = m_lPopupStack.RemoveTail();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2276
    UTIL_ASSERT(pPanel != NULL);

    // NOTE: Uninline.
    ShowPopupPanel(pPanel->m_nID, FALSE);

    CUIPanel* pMainPanel = m_cUIManager.GetPanel(2);
    pMainPanel->InvalidateRect(NULL);

    if (m_lPopupStack.GetTailPosition() != NULL) {
        CUIPanel* pPanel = m_lPopupStack.GetTail();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2293
        UTIL_ASSERT(pPanel != NULL);

        // NOTE: Uninline.
        ShowPopupPanel(pPanel->m_nID, TRUE);

        // NOTE: Uninline.
        EnablePopupPanel(pPanel->m_nID, TRUE);

        // NOTE: Uninline.
        UpdatePopupPanel(pPanel->m_nID);
    } else {
        EnableMainPanel(TRUE);
        UpdateMainPanel();
    }
}

// NOTE: Inlined.
BOOL CScreenSpellbook::IsDoneButtonClickable()
{
    CUIPanel* pPanel = GetTopPopup();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2323
    UTIL_ASSERT(pPanel != NULL);

    switch (pPanel->m_nID) {
    case 3:
    case 4:
    case 5:
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2335
        UTIL_ASSERT(pPanel != NULL);
    }

    return TRUE;
}

// 0x66B960
void CScreenSpellbook::OnDoneButtonClick()
{
    CUIPanel* pPanel = GetTopPopup();
    if (pPanel != NULL) {
        switch (pPanel->m_nID) {
        case 4:
        case 5:
        case 50:
            OnErrorButtonClick(0);
            return;
        }
    }

    CSingleLock renderLock(&(GetManager()->field_36), FALSE);
    renderLock.Lock(INFINITE);

    // NOTE: Uninline.
    if (IsDoneButtonClickable()) {
        CUIPanel* pPanel = GetTopPopup();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2388
        UTIL_ASSERT(pPanel != NULL);

        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2390
        UTIL_ASSERT(pGame != NULL);

        switch (pPanel->m_nID) {
        case 3:
            DismissPopup();
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
            // __LINE__: 2399
            UTIL_ASSERT(FALSE);
        }
    }

    renderLock.Unlock();
}

// 0x66BB00
void CScreenSpellbook::OnCancelButtonClick()
{
    CSingleLock renderLock(&(GetManager()->field_36), FALSE);
    renderLock.Lock(INFINITE);

    CUIPanel* pPanel = GetTopPopup();

    switch (pPanel->m_nID) {
    case 3:
    case 4:
    case 5:
    case 50:
        DismissPopup();
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2447
        UTIL_ASSERT(FALSE);
    }

    renderLock.Unlock();
}

// 0x66BBE0
void CScreenSpellbook::OnRestButtonClick()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2485
    UTIL_ASSERT(pGame != NULL);

    CSingleLock renderLock(&(m_cUIManager.field_36), FALSE);
    renderLock.Lock(INFINITE);

    STRREF strError;
    if (pGame->CanRestParty(strError, 0, 0, 0)) {
        m_nErrorState = 1;
        m_dwErrorTextId = 15358;
        m_strErrorButtonText[0] = 17199;
        m_strErrorButtonText[1] = 11596;
        m_strErrorButtonText[2] = 13727;
        SummonPopup(50);
    } else {
        m_nErrorState = 0;
        m_dwErrorTextId = strError;
        m_strErrorButtonText[0] = 11973;
        SummonPopup(4);
    }

    renderLock.Unlock();
}

// 0x66BCF0
void CScreenSpellbook::ResetSpellInfoPanel(CUIPanel* pPanel)
{
    CSpell cSpell;
    cSpell.SetResRef(m_spellResRef, TRUE, TRUE);

    cSpell.Demand();

    STRREF strName = cSpell.GetGenericName();
    UpdateLabel(pPanel, 0x10000000, "%s", FetchString(strName));

    CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(3));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2541
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();

    STRREF strDescription = cSpell.GetDescription();
    UpdateText(pText, "%s", FetchString(strDescription));

    pText->SetTopString(pText->m_plstStrings->FindIndex(0));

    CUIControlButtonSpellbookSpellInfoIcon* pIcon = static_cast<CUIControlButtonSpellbookSpellInfoIcon*>(pPanel->GetControl(2));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2608
    UTIL_ASSERT(pIcon != NULL);

    pIcon->SetSpell(m_spellResRef);

    cSpell.Release();
}

// NOTE: Inlined.
void CScreenSpellbook::UpdateSpellInfoPanel()
{
    CUIPanel* pPanel = m_cUIManager.GetPanel(3);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2638
    UTIL_ASSERT(pPanel != NULL);

    m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(4));

    CUIControlButton* pDone = static_cast<CUIControlButton*>(pPanel->GetControl(5));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2641
    UTIL_ASSERT(pDone != NULL);

    // NOTE: Uninline.
    pDone->SetEnabled(IsDoneButtonClickable());
}

// 0x66BEE0
void CScreenSpellbook::ResetErrorPanel(CUIPanel* pPanel)
{
    switch (pPanel->m_nID) {
    case 7:
        m_nNumErrorButtons = 1;
        break;
    case 8:
        m_nNumErrorButtons = 2;
        break;
    case 50:
        m_nNumErrorButtons = 3;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2682
        UTIL_ASSERT(FALSE);
    }

    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(m_dwErrorTextId, strRes);

    strRes.cSound.SetChannel(0, 0);
    strRes.cSound.SetFireForget(TRUE);
    strRes.cSound.Play(FALSE);

    CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(3));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2695
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();
    UpdateText(pText, "%s", strRes.szText);

    for (INT nButton = 0; nButton < m_nNumErrorButtons; nButton++) {
        CUIControlButton* pButton = static_cast<CUIControlButton*>(pPanel->GetControl(nButton + 1));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2704
        UTIL_ASSERT(pButton != NULL);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
        // __LINE__: 2879
        UTIL_ASSERT(0 <= nButton && nButton < CSCREENSPELLBOOK_ERROR_BUTTONS);

        pButton->SetText(FetchString(m_strErrorButtonText[nButton]));
    }
}

// 0x66C0F0
void CScreenSpellbook::OnErrorButtonClick(INT nButton)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2731
    UTIL_ASSERT(0 <= nButton && nButton < GetNumErrorButtons());

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2733
    UTIL_ASSERT(pGame != NULL);

    CSingleLock renderLock(&(GetManager()->field_36), FALSE);
    renderLock.Lock(INFINITE);

    // FIXME: Unncessary access via global var.
    LONG nCharacterId = pGame->GetCharacterId(g_pBaldurChitin->m_pEngineSpellbook->GetSelectedCharacter());

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        switch (m_nErrorState) {
        case 0:
            switch (nButton) {
            case 0:
                DismissPopup();
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                // __LINE__: 2794
                UTIL_ASSERT(FALSE);
            }
            break;
        case 1:
            switch (nButton) {
            case 0:
                DismissPopup();

                CInfGame::m_bHealPartyOnRest = FALSE;
                pGame->RestParty(1, 0);
                UpdateMainPanel();
                break;
            case 1:
                DismissPopup();

                CInfGame::m_bHealPartyOnRest = TRUE;
                pGame->RestParty(1, 0);
                UpdateMainPanel();
                CInfGame::m_bHealPartyOnRest = FALSE;
                break;
            case 2:
                DismissPopup();
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                // __LINE__: 2781
                UTIL_ASSERT(FALSE);
            }
            break;
        case 2:
            switch (nButton) {
            case 0:
                DismissPopup();

                UnmemorizeSpell(pSprite, field_582);
                UpdateMainPanel();
                break;
            case 1:
                DismissPopup();
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                // __LINE__: 2813
                UTIL_ASSERT(FALSE);
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
            // __LINE__: 2819
            UTIL_ASSERT(FALSE);
        }

        pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    renderLock.Unlock();
}

// 0x66C360
void CScreenSpellbook::CancelEngine()
{
    while (GetTopPopup() != NULL) {
        OnCancelButtonClick();
    }
}

// 0x66C380
void CScreenSpellbook::CheckMultiPlayerViewable()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 2942
    UTIL_ASSERT(pGame != NULL);

    m_bMultiPlayerViewable = TRUE;

    LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->Orderable(1)) {
            // NOTE: Uninline.
            if (pSprite->InControl()) {
                m_bMultiPlayerViewable = TRUE;
            } else {
                if (g_pChitin->cNetwork.GetSessionOpen()) {
                    m_bMultiPlayerViewable = g_pChitin->cNetwork.GetSessionHosting()
                        || pGame->m_singlePlayerPermissions.GetSinglePermission(CGamePermission::LEADER);
                }
            }
        } else {
            m_bMultiPlayerViewable = FALSE;
        }

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x66C4B0
int CScreenSpellbook::sub_66C4B0()
{
    CSingleLock renderLock(&(m_cUIManager.GetPanel(2)->m_pManager->field_36), FALSE);
    renderLock.Lock(INFINITE);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // FIXME: Unncessary access via global var.
    LONG nCharacterId = pGame->GetCharacterId(g_pBaldurChitin->m_pEngineSpellbook->GetSelectedCharacter());

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        renderLock.Unlock();
        return 0;
    }

    DWORD nClassIndex = m_nClassIndex;

    // __FILE__: .\Include\InfScreenSpellbook.h
    // __LINE__: 76
    UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

    UINT nLevel = m_nSpellLevel;

    CGameSpriteSpellList* pSpellList;
    if (field_1670 == m_nClassIndex) {
        // __FILE__: .\Include\ObjCreature.h
        // __LINE__: 1763
        UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

        // NOTE: Uninline.
        pSpellList = pSprite->m_domainSpells.GetSpellsAtLevel(nLevel);
    } else {
        BYTE nClass = static_cast<BYTE>(sub_66A010(m_nClassIndex));
        pSpellList = pSprite->GetSpellsAtLevel(nClass, nLevel);
    }

    int nResult;
    if (field_1654[nClassIndex] == 2 || field_1654[nClassIndex] == 10) {
        // NOTE: Uninline.
        nResult = pSpellList->sub_58FE70();
    } else {
        INT nTotal = 0;

        UINT nCount = pSpellList->m_List.size();
        for (UINT nIndex = 0; nIndex < nCount; nIndex++) {
            // __FILE__: .\Include\FileFormat.h
            // __LINE__: 2437
            UTIL_ASSERT(pSpellList->m_List.size() > 0);

            // __FILE__: .\Include\FileFormat.h
            // __LINE__: 2438
            UTIL_ASSERT(nIndex < pSpellList->m_List.size());

            CGameSpriteSpellListEntry entry = pSpellList->m_List[nIndex];

            nTotal += entry.field_4;
        }

        // TODO: Incomplete. The minuend should be `sub_66A8C0(pSprite)`
        // (0x66A8C0), which is not in the repo yet.
        nResult = 0 - nTotal;
    }

    pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    renderLock.Unlock();

    return nResult;
}

// 0x66C790
BOOLEAN CScreenSpellbook::sub_66C790(CGameSprite* pSprite, DWORD nIndex)
{
    int nDomainClassIndex = field_1670;
    UINT nSpellLevel = m_nSpellLevel;
    BOOLEAN bResult = FALSE;

    DWORD nClassIndex = m_nClassIndex;

    // __FILE__: .\Include\InfScreenSpellbook.h
    // __LINE__: 76
    UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

    int nClassType = field_1654[nClassIndex];

    int nAvailable = sub_66C4B0();
    if (nAvailable <= 0) {
        return FALSE;
    }

    UINT nLevel = m_nSpellLevel;

    // NOTE: Result is unused.
    CGameSpriteSpellList* pSpellList;
    if (field_1670 == m_nClassIndex) {
        // __FILE__: .\Include\ObjCreature.h
        // __LINE__: 1763
        UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

        // NOTE: Uninline.
        pSpellList = pSprite->m_domainSpells.GetSpellsAtLevel(nLevel);
    } else {
        BYTE nClass = static_cast<BYTE>(sub_66A010(m_nClassIndex));
        pSpellList = pSprite->GetSpellsAtLevel(nClass, nLevel);
    }

    CResRef resRef = field_160C[nIndex];

    if (resRef != CResRef()) {
        if (nDomainClassIndex == nClassIndex) {
            UINT nExtra3 = 1;
            UINT nExtra4 = 0;
            UINT nExtra5 = 0;
            bResult = pSprite->AddDomainSpell(nSpellLevel, resRef, nExtra3, nExtra4, nExtra5);
        } else {
            BYTE nClass = static_cast<BYTE>(nClassType);
            UINT nExtra4 = 1;
            UINT nExtra5 = 0;
            UINT nExtra6 = 0;
            bResult = pSprite->AddKnownSpell(nClass, nSpellLevel, resRef, nExtra4, nExtra5, nExtra6);
        }

        if (bResult) {
            nLevel = m_nSpellLevel;

            if (field_1670 == m_nClassIndex) {
                // __FILE__: .\Include\ObjCreature.h
                // __LINE__: 1763
                UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

                // NOTE: Uninline.
                pSpellList = pSprite->m_domainSpells.GetSpellsAtLevel(nLevel);
            } else {
                BYTE nClass = static_cast<BYTE>(sub_66A010(m_nClassIndex));
                pSpellList = pSprite->GetSpellsAtLevel(nClass, nLevel);
            }

            UINT nSpellId = 0;
            UINT nSpellIndex = 0;
            if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nSpellId) == TRUE) {
                if (pSpellList->Find(nSpellId, nSpellIndex)) {
                    // __FILE__: .\Include\FileFormat.h
                    // __LINE__: 2428
                    UTIL_ASSERT(pSpellList->m_List.size() > 0);

                    // __FILE__: .\Include\FileFormat.h
                    // __LINE__: 2429
                    UTIL_ASSERT(nSpellIndex < pSpellList->m_List.size());

                    CGameSpriteSpellListEntry* pEntry = &(pSpellList->m_List[nSpellIndex]);

                    UINT nSlot;
                    for (nSlot = 0; nSlot < static_cast<UINT>(field_1488); nSlot++) {
                        if (field_148C[nSlot] == resRef) {
                            break;
                        }
                    }

                    if (nSlot < static_cast<UINT>(field_1488)) {
                        field_148C[nSlot] = resRef;
                        field_154C[nSlot] = pEntry->field_8;
                        field_15AC[nSlot] = pEntry->field_4;
                    } else {
                        field_148C[field_1488] = resRef;
                        field_154C[field_1488] = pEntry->field_8;
                        field_15AC[field_1488] = pEntry->field_4;
                        field_1488++;
                    }
                }
            }
        }
    }

    return bResult;
}

// 0x66CAF0
void CScreenSpellbook::UnmemorizeSpell(CGameSprite* pSprite, int a2)
{
    DWORD nClassIndex = m_nClassIndex;
    int nDomainClassIndex = field_1670;
    UINT nSpellLevel = m_nSpellLevel;

    // __FILE__: .\Include\InfScreenSpellbook.h
    // __LINE__: 76
    UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

    int nClassType = field_1654[nClassIndex];

    CUIPanel* pPanel = m_cUIManager.GetPanel(2);

    // NOTE: Uninline.
    BOOLEAN bInControl = pSprite->InControl();

    CResRef resRef = field_148C[a2];

    if (resRef != CResRef()) {
        BOOLEAN bRemoved;
        if (nDomainClassIndex == nClassIndex) {
            // NOTE: Third param is `BOOLEAN` in the original (passed by
            // address of a byte).
            UINT nExtra3 = 0;
            UINT nExtra4 = 1;
            UINT nExtra5 = 0;
            bRemoved = pSprite->RemoveDomainSpell(nSpellLevel, resRef, nExtra3, nExtra4, nExtra5);
        } else {
            BYTE nClass = static_cast<BYTE>(nClassType);

            // NOTE: Fourth param is `BOOLEAN` in the original (passed by
            // address of a byte).
            UINT nExtra4 = 0;
            UINT nExtra5 = 1;
            UINT nExtra6 = 0;
            bRemoved = pSprite->RemoveKnownSpell(nClass, nSpellLevel, resRef, nExtra4, nExtra5, nExtra6);
        }

        if (bRemoved) {
            UINT nLevel = m_nSpellLevel;

            CGameSpriteSpellList* pSpellList;
            if (field_1670 == m_nClassIndex) {
                // __FILE__: .\Include\ObjCreature.h
                // __LINE__: 1763
                UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

                // NOTE: Uninline.
                pSpellList = pSprite->m_domainSpells.GetSpellsAtLevel(nLevel);
            } else {
                BYTE nClass = static_cast<BYTE>(sub_66A010(m_nClassIndex));
                pSpellList = pSprite->GetSpellsAtLevel(nClass, nLevel);
            }

            UINT nSpellListSize = pSpellList->m_List.size();

            BOOL bFound = FALSE;
            UINT nListIndex;
            for (nListIndex = 0; nListIndex < nSpellListSize; nListIndex++) {
                // __FILE__: .\Include\FileFormat.h
                // __LINE__: 2428
                UTIL_ASSERT(pSpellList->m_List.size() > 0);

                // __FILE__: .\Include\FileFormat.h
                // __LINE__: 2429
                UTIL_ASSERT(nListIndex < pSpellList->m_List.size());

                UINT nSpellId = pSpellList->m_List[nListIndex].m_nID;

                // NOTE: Uninline.
                if (resRef == g_pBaldurChitin->GetObjectGame()->m_spells.Get(nSpellId)) {
                    bFound = TRUE;
                    break;
                }
            }

            if (!bFound) {
                BOOL bShifting = FALSE;

                for (UINT nSlot = 0; nSlot < 24; nSlot++) {
                    if (bShifting == TRUE) {
                        field_148C[nSlot - 1] = field_148C[nSlot];
                        field_154C[nSlot - 1] = field_154C[nSlot];
                        field_15AC[nSlot - 1] = field_15AC[nSlot];

                        field_148C[nSlot] = CResRef();
                        field_154C[nSlot] = 0;
                        field_15AC[nSlot] = 0;

                        CUIControlButtonSpellbookSpell* pButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(6 + nSlot));

                        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                        // __LINE__: 3249
                        UTIL_ASSERT(pButton != NULL);

                        CUIControlButtonSpellbookSpell* pLastButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(5 + nSlot));

                        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                        // __LINE__: 3253
                        UTIL_ASSERT(pLastButton != NULL);

                        BOOL bEnable = nSlot < static_cast<UINT>(field_1488) && m_bMultiPlayerViewable && bInControl;
                        if (pLastButton->field_67A != bEnable) {
                            pLastButton->field_67A = bEnable;
                            pLastButton->InvalidateRect();
                        }

                        pLastButton->SetSpell(field_148C[nSlot]);

                        if (pLastButton->field_676 != 1) {
                            pLastButton->field_676 = 1;
                            pLastButton->InvalidateRect();
                        }

                        UpdateLabel(pPanel,
                            0x1000003E + nSlot,
                            "%d/%d",
                            field_154C[nSlot],
                            field_15AC[nSlot]);

                        if (pButton->field_676 != 0) {
                            pButton->field_676 = 0;
                            pButton->InvalidateRect();
                        }

                        pButton->SetSpell(CResRef(""));

                        UpdateLabel(pPanel, 0x1000003F + nSlot, " ");
                    }

                    if (field_148C[nSlot] == resRef) {
                        field_148C[nSlot] = CResRef();
                        field_154C[nSlot] = 0;
                        field_15AC[nSlot] = 0;
                        bShifting = TRUE;
                    }
                }

                if (bShifting == TRUE) {
                    field_1488--;

                    CUIControlButtonSpellbookSpell* pFlashButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(a2 + 6));

                    m_bFlashUnmemorize = TRUE;
                    m_pFlashMemorizeDestSpell = pFlashButton;
                    m_pFlashCurrentSpell = pFlashButton;
                    m_bFlash = TRUE;

                    m_vcFlash.FrameSet(0);

                    pFlashButton->InvalidateRect();

                    PlayGUISound(CResRef("GAM_24"));
                }
            } else {
                // NOTE: Result of inlined accessor is unused.
                //
                // __FILE__: .\Include\FileFormat.h
                // __LINE__: 2428
                UTIL_ASSERT(pSpellList->m_List.size() > 0);

                // __FILE__: .\Include\FileFormat.h
                // __LINE__: 2429
                UTIL_ASSERT(nListIndex < pSpellList->m_List.size());

                BOOL bShifted = FALSE;
                BOOL bFlash = FALSE;

                for (UINT nSlot = 0; nSlot < 24; nSlot++) {
                    if (bShifted == TRUE) {
                        field_148C[nSlot - 1] = field_148C[nSlot];
                        field_154C[nSlot - 1] = field_154C[nSlot];
                        field_15AC[nSlot - 1] = field_15AC[nSlot];

                        field_148C[nSlot] = CResRef();
                        field_154C[nSlot] = 0;
                        field_15AC[nSlot] = 0;
                    }

                    if (field_148C[nSlot] == resRef) {
                        // __FILE__: .\Include\FileFormat.h
                        // __LINE__: 2428
                        UTIL_ASSERT(pSpellList->m_List.size() > 0);

                        // __FILE__: .\Include\FileFormat.h
                        // __LINE__: 2429
                        UTIL_ASSERT(nListIndex < pSpellList->m_List.size());

                        CGameSpriteSpellListEntry* pEntry = &(pSpellList->m_List[nListIndex]);

                        UINT nField4 = pEntry->field_4;
                        UINT nField8 = pEntry->field_8;

                        CAbilityId ab;
                        ab.m_itemType = 1;
                        ab.m_res = resRef;

                        if (nDomainClassIndex == nClassIndex) {
                            ab.m_nClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nClassIndex);
                        } else {
                            // __FILE__: .\Include\InfScreenSpellbook.h
                            // __LINE__: 76
                            UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

                            ab.m_nClass = static_cast<BYTE>(field_1654[nClassIndex]);
                        }

                        ab.field_1D = static_cast<BYTE>(nSpellLevel + 1);

                        if (ab.m_nClass == 3 && nDomainClassIndex == nClassIndex) {
                            ab.field_1E = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetSpecializationIndex(3, pSprite->m_baseStats.m_specialization);
                        } else {
                            ab.field_1E = 0;
                        }

                        pSprite->CheckQuickLists(&ab, -1, FALSE, TRUE);

                        g_pBaldurChitin->GetObjectGame()->m_cButtonArray.UpdateButtons();

                        if (nField8 == 0 && nField4 == 0) {
                            field_148C[nSlot] = CResRef();
                            field_154C[nSlot] = 0;
                            field_15AC[nSlot] = 0;

                            field_1488--;

                            bShifted = TRUE;
                        } else {
                            field_148C[nSlot] = resRef;
                            field_154C[nSlot] = pEntry->field_8;
                            field_15AC[nSlot] = pEntry->field_4;

                            bFlash = TRUE;
                            break;
                        }
                    }
                }

                if (bFlash || bShifted == TRUE) {
                    CUIControlButtonSpellbookSpell* pFlashButton = static_cast<CUIControlButtonSpellbookSpell*>(pPanel->GetControl(a2 + 6));

                    m_bFlashUnmemorize = TRUE;
                    m_pFlashMemorizeDestSpell = pFlashButton;
                    m_pFlashCurrentSpell = pFlashButton;
                    m_bFlash = TRUE;

                    m_vcFlash.FrameSet(0);

                    pFlashButton->InvalidateRect();

                    PlayGUISound(CResRef("GAM_24"));
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

// 0x66D420
CUIControlButtonSpellbookSpell::CUIControlButtonSpellbookSpell(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON | RBUTTON, 1)
{
    m_spellResRef = "";
    m_iconResRef = "";
    field_676 = 1;
    field_67A = 0;
}

// 0x66D500
CUIControlButtonSpellbookSpell::~CUIControlButtonSpellbookSpell()
{
}

// 0x66D5A0
void CUIControlButtonSpellbookSpell::SetSpell(const CResRef& cNewResRef)
{
    CString sIconResRef;

    m_spellResRef = cNewResRef;
    m_iconResRef = "";

    if (m_spellResRef != "") {
        CSpell cSpell;
        cSpell.SetResRef(m_spellResRef, TRUE, TRUE);
        cSpell.Demand();

        if (cSpell.pRes != NULL) {
            RESREF iconResRef;
            cSpell.GetIcon(iconResRef);
            m_iconResRef = iconResRef;

            m_iconResRef.CopyToString(sIconResRef);
            sIconResRef.SetAt(sIconResRef.GetLength() - 1, 'C');
            m_iconResRef = sIconResRef;

            SetToolTipStrRef(cSpell.GetGenericName(), -1, -1);
        }

        cSpell.Release();
    } else {
        SetToolTipStrRef(-1, -1, -1);
    }
}

// 0x66D760
BOOL CUIControlButtonSpellbookSpell::OnLButtonDown(CPoint pt)
{
    if (!m_bActive) {
        return FALSE;
    }

    if ((m_nMouseButtons & LBUTTON) == 0) {
        return FALSE;
    }

    m_cVidCell.FrameSet(m_nPressedFrame);
    m_bPressed = TRUE;

    m_pPanel->m_pManager->SetCapture(this, CUIManager::MOUSELBUTTON);
    InvalidateRect();

    m_pPanel->m_pManager->field_32 = m_nID;
    m_pPanel->m_pManager->field_2D = 0;
    m_pPanel->m_pManager->field_1C = 0;

    return TRUE;
}

// 0x66D7E0
void CUIControlButtonSpellbookSpell::OnLButtonClick(CPoint pt)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    CSingleLock renderLock(&(m_pPanel->m_pManager->field_36), FALSE);
    renderLock.Lock(INFINITE);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // FIXME: Unncessary access via global var.
    LONG nCharacterId = pGame->GetCharacterId(g_pBaldurChitin->m_pEngineSpellbook->GetSelectedCharacter());

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        // NOTE: Uninline.
        if (pSprite->InControl()) {
            DWORD nClassIndex = pSpellbook->m_nClassIndex;
            DWORD nID = m_nID;

            if (nID >= 46 && nID <= 53) {
                nID -= 16;
            }

            if (nID >= 30 && nID <= 37) {
                DWORD nSlot = nID - 30;

                if (pSpellbook->sub_66C790(pSprite, nSlot)) {
                    CResRef resRef = pSpellbook->field_160C[nSlot];

                    CUIControlButtonSpellbookSpell* pFoundButton = NULL;

                    if (resRef != CResRef()) {
                        for (DWORD nIndex = 0; nIndex < static_cast<DWORD>(pSpellbook->field_1488); nIndex++) {
                            if (pSpellbook->field_148C[nIndex] == resRef && 6 + nIndex <= 29) {
                                pFoundButton = static_cast<CUIControlButtonSpellbookSpell*>(m_pPanel->GetControl(6 + nIndex));
                                break;
                            }
                        }
                    }

                    // __FILE__: .\Include\InfScreenSpellbook.h
                    // __LINE__: 76
                    UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

                    int nClassType = pSpellbook->field_1654[nClassIndex];
                    if (nClassType != 2 && nClassType != 10) {
                        if (pFoundButton == NULL) {
                            pFoundButton = static_cast<CUIControlButtonSpellbookSpell*>(m_pPanel->GetControl(nID));
                        }

                        CUIControlButtonSpellbookSpell* pCurrentButton = static_cast<CUIControlButtonSpellbookSpell*>(m_pPanel->GetControl(nID));

                        pSpellbook->m_bFlashMemorize = TRUE;
                        pSpellbook->m_bFlash = TRUE;
                        pSpellbook->m_pFlashMemorizeDestSpell = pFoundButton;
                        pSpellbook->m_pFlashMemorizeSourceSpell = pCurrentButton;
                        pSpellbook->m_pFlashCurrentSpell = pCurrentButton;

                        pSpellbook->m_vcFlash.FrameSet(0);

                        InvalidateRect();

                        pSpellbook->PlayGUISound(CResRef("GAM_24"));
                    }
                }
            } else if (nID >= 6 && nID <= 29) {
                DWORD nSlot = nID - 6;

                CResRef resRef = pSpellbook->field_148C[nSlot];

                if (resRef != CResRef()) {
                    if (pSpellbook->field_154C[nSlot] != 0
                        && pSpellbook->field_154C[nSlot] == pSpellbook->field_15AC[nSlot]) {
                        pSpellbook->field_582 = nSlot;
                        pSpellbook->m_nErrorState = 2;
                        pSpellbook->m_dwErrorTextId = 11824;
                        pSpellbook->m_strErrorButtonText[0] = 17507;
                        pSpellbook->m_strErrorButtonText[1] = 13727;
                        pSpellbook->SummonPopup(5);
                    } else {
                        pSpellbook->UnmemorizeSpell(pSprite, nSlot);
                    }
                }
            } else {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
                // __LINE__: 3694
                UTIL_ASSERT(FALSE);
            }
        }

        pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        pSpellbook->UpdateMainPanel();
    }

    renderLock.Unlock();
}

// 0x66DB60
void CUIControlButtonSpellbookSpell::OnRButtonClick(CPoint pt)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 3730
    UTIL_ASSERT(pSpellbook != NULL);

    CSingleLock renderLock(&(pSpellbook->GetManager()->field_36), FALSE);
    renderLock.Lock(INFINITE);

    if (m_spellResRef != "") {
        pSpellbook->m_spellResRef = m_spellResRef;
        pSpellbook->SummonPopup(3);
    }

    renderLock.Unlock();
}

// 0x66DC30
BOOL CUIControlButtonSpellbookSpell::Render(BOOL bForce)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 3770
    UTIL_ASSERT(pSpellbook != NULL);

    CVidPoly vidPoly;
    CVidCell vcIcon;

    // NOTE: Unused.
    CVidCell vcUnused;

    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    if (m_nRenderCount != 0) {
        CSingleLock lock(&(m_pPanel->m_pManager->field_56), FALSE);
        lock.Lock(INFINITE);
        m_nRenderCount--;
        lock.Unlock();
    }

    CVidInf* pVidInf = static_cast<CVidInf*>(g_pBaldurChitin->GetCurrentVideoMode());

    if (field_67A != 0 || m_iconResRef != "") {
        CPoint pt = m_pPanel->m_ptOrigin + m_ptOrigin;
        CRect rControlFrame(pt, m_size);

        CRect rClip;
        rClip.IntersectRect(rControlFrame, m_rDirty);

        CRect rFXClip(rClip);
        CRect rFXRect(0, 0, m_size.cx, m_size.cy);

        CPoint ptZero(0, 0);
        pVidInf->FXPrep(rFXRect, CVIDCELL_FXPREP_COPYFROMBACK, pt, ptZero, rFXClip);

        if (!pVidInf->FXLock(rFXRect, CVIDCELL_FXPREP_COPYFROMBACK)) {
            return FALSE;
        }

        INT nOffsetX = 0;
        INT nOffsetY = 0;

        if (m_iconResRef != "") {
            // NOTE: Uninline.
            vcIcon.SetResRef(m_iconResRef, m_pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);

            CSize frameSize;
            vcIcon.GetCurrentFrameSize(frameSize, FALSE);

            CPoint ptReference;
            vcIcon.GetCurrentCenterPoint(ptReference, FALSE);

            if (m_size.cx > frameSize.cx) {
                nOffsetX = (m_size.cx - frameSize.cx) / 2;
            }

            if (m_size.cy > frameSize.cy) {
                nOffsetY = (m_size.cy - frameSize.cy) / 2;
            }

            if (vcIcon.pRes != NULL) {
                vcIcon.pRes->Demand();
                vcIcon.RealizePalette(1);

                pVidInf->FXRender(&vcIcon,
                    ptReference.x + nOffsetX,
                    ptReference.y + nOffsetY,
                    rFXRect,
                    TRUE,
                    CVIDCELL_FXPREP_COPYFROMBACK);

                vcIcon.pRes->Release();
            }

            if (field_676 == 0) {
                CVIDPOLY_VERTEX vertices[4];

                vertices[0].x = static_cast<WORD>(nOffsetX);
                vertices[0].y = static_cast<WORD>(nOffsetY);
                vertices[1].x = static_cast<WORD>(nOffsetX + frameSize.cx);
                vertices[1].y = static_cast<WORD>(nOffsetY);
                vertices[2].x = static_cast<WORD>(nOffsetX + frameSize.cx);
                vertices[2].y = static_cast<WORD>(nOffsetY + frameSize.cy);
                vertices[3].x = static_cast<WORD>(nOffsetX);
                vertices[3].y = static_cast<WORD>(nOffsetY + frameSize.cy);

                vidPoly.SetPoly(vertices, 4);

                // TODO: Incomplete. Missing call to 0x79D680 on `pVidInf`
                // (RET 0x18, six arguments: `&vidPoly`, `&rFXRect`,
                // `pVidInf->field_24`, 2, `&ptZero` (0, 0), 0).
            }

        }

        ptZero = CPoint(0, 0);
        pVidInf->FXUnlock(CVIDCELL_FXPREP_COPYFROMBACK, NULL, ptZero);

        pVidInf->FXBltToBack(rFXRect, pt.x, pt.y, 0, 0, rFXClip, CVIDCELL_FXPREP_COPYFROMBACK);
    }

    if (pSpellbook->m_bFlash != 0 && this == pSpellbook->m_pFlashCurrentSpell) {
        pSpellbook->DrawFlash();
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x66E0B0
CUIControlScrollBarSpellbookKnownSpells::CUIControlScrollBarSpellbookKnownSpells(CUIPanel* panel, UI_CONTROL_SCROLLBAR* controlInfo)
    : CUIControlScrollBar(panel, controlInfo)
{
}

// 0x67F640
CUIControlScrollBarSpellbookKnownSpells::~CUIControlScrollBarSpellbookKnownSpells()
{
}

// 0x66E0D0
void CUIControlScrollBarSpellbookKnownSpells::OnScrollUp()
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 3998
    UTIL_ASSERT(pSpellbook != NULL);

    pSpellbook->m_nTopKnownSpell = max(pSpellbook->m_nTopKnownSpell - 1, 0);

    InvalidateItems();
}

// 0x66E120
void CUIControlScrollBarSpellbookKnownSpells::OnScrollDown()
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4021
    UTIL_ASSERT(pSpellbook != NULL);

    pSpellbook->m_nTopKnownSpell = max(min(pSpellbook->m_nTopKnownSpell + 1, pSpellbook->m_nNumKnownSpells - 8), 0);

    InvalidateItems();
}

// 0x66E190
void CUIControlScrollBarSpellbookKnownSpells::OnPageUp(DWORD nLines)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4044
    UTIL_ASSERT(pSpellbook != NULL);

    INT nStep = min(nLines, 7);
    pSpellbook->m_nTopKnownSpell = max(pSpellbook->m_nTopKnownSpell - nStep, 0);

    InvalidateItems();
}

// 0x66E200
void CUIControlScrollBarSpellbookKnownSpells::OnPageDown(DWORD nLines)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4073
    UTIL_ASSERT(pSpellbook != NULL);

    INT nStep = min(nLines, 7);
    pSpellbook->m_nTopKnownSpell = max(min(pSpellbook->m_nTopKnownSpell + nStep, pSpellbook->m_nNumKnownSpells - 8), 0);

    InvalidateItems();
}

// 0x66E280
void CUIControlScrollBarSpellbookKnownSpells::OnScroll()
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4098
    UTIL_ASSERT(pSpellbook != NULL);

    pSpellbook->m_nTopKnownSpell = max(field_144 * (pSpellbook->m_nNumKnownSpells - 8), 0) / field_142;

    if (pSpellbook->m_nNumKnownSpells < 8) {
        field_144 = 0;
    }

    InvalidateItems();
}

// 0x66E300
void CUIControlScrollBarSpellbookKnownSpells::InvalidateItems()
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4156
    UTIL_ASSERT(pSpellbook != NULL);

    CSingleLock renderLock(&(pSpellbook->GetManager()->field_36), FALSE);
    renderLock.Lock(INFINITE);

    pSpellbook->UpdateMainPanel();

    renderLock.Unlock();
}

// -----------------------------------------------------------------------------

// 0x66E3A0
CUIControlButtonSpellbookSpellInfoIcon::CUIControlButtonSpellbookSpellInfoIcon(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, 0, 1)
{
    m_spellResRef = "";
    m_iconResRef = "";
}

// 0x66E460
CUIControlButtonSpellbookSpellInfoIcon::~CUIControlButtonSpellbookSpellInfoIcon()
{
}

// 0x66E500
void CUIControlButtonSpellbookSpellInfoIcon::SetSpell(const CResRef& cNewResRef)
{
    CString sIconResRef;

    m_spellResRef = cNewResRef;
    m_iconResRef = "";

    if (m_spellResRef != "") {
        CSpell cSpell;
        cSpell.SetResRef(m_spellResRef, TRUE, TRUE);
        cSpell.Demand();

        if (cSpell.pRes != NULL) {
            RESREF iconResRef;
            cSpell.GetIcon(iconResRef);
            m_iconResRef = iconResRef;

            m_iconResRef.CopyToString(sIconResRef);
            sIconResRef.SetAt(sIconResRef.GetLength() - 1, 'B');
            m_iconResRef = sIconResRef;
        }

        cSpell.Release();
    }
}

// 0x66E6A0
BOOL CUIControlButtonSpellbookSpellInfoIcon::Render(BOOL bForce)
{
    CVidCell vcIcon;

    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    if (m_nRenderCount != 0) {
        CSingleLock lock(&(m_pPanel->m_pManager->field_56), FALSE);
        lock.Lock(INFINITE);
        m_nRenderCount--;
        lock.Unlock();
    }

    if (m_iconResRef == "") {
        return FALSE;
    }

    CRect rControlFrame(m_pPanel->m_ptOrigin + m_ptOrigin, m_size);

    CRect rClip;
    rClip.IntersectRect(rControlFrame, m_rDirty);

    // NOTE: Uninline.
    vcIcon.SetResRef(m_iconResRef, m_pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);

    if (vcIcon.pRes == NULL) {
        return FALSE;
    }

    CSize iconSize;
    vcIcon.GetCurrentFrameSize(iconSize, FALSE);

    INT x = m_size.cx > iconSize.cx
        ? rControlFrame.left + (m_size.cx - iconSize.cx) / 2
        : rControlFrame.left;
    INT y = m_size.cy > iconSize.cy
        ? rControlFrame.top + (m_size.cy - iconSize.cy) / 2
        : rControlFrame.top;

    BOOL bResult = vcIcon.Render(0, x, y, rClip, NULL, 0, 0, -1);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4341
    UTIL_ASSERT(bResult);

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x66E9E0
CUIControlButtonSpellbookPopupDone::CUIControlButtonSpellbookPopupDone(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 1)
{
    SetText(CBaldurEngine::FetchString(11973));
}

// 0x66EA90
CUIControlButtonSpellbookPopupDone::~CUIControlButtonSpellbookPopupDone()
{
}

// 0x66EB30
void CUIControlButtonSpellbookPopupDone::OnLButtonClick(CPoint pt)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4398
    UTIL_ASSERT(pSpellbook != NULL);

    pSpellbook->OnDoneButtonClick();
}

// -----------------------------------------------------------------------------

// 0x66EB70
CUIControlButtonSpellbookError::CUIControlButtonSpellbookError(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 1)
{
}

// 0x66EBC0
CUIControlButtonSpellbookError::~CUIControlButtonSpellbookError()
{
}

// 0x66EC60
void CUIControlButtonSpellbookError::OnLButtonClick(CPoint pt)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenSpellbook.cpp
    // __LINE__: 4450
    UTIL_ASSERT(pSpellbook != NULL);

    pSpellbook->OnErrorButtonClick(m_nID);
}

// -----------------------------------------------------------------------------

// 0x66ECA0
CUIControlButtonSpellbookLevelSelection::CUIControlButtonSpellbookLevelSelection(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton3State(panel, controlInfo, LBUTTON, 1)
{
    m_nSelectedFrame = 0;
}

// 0x66ED00
CUIControlButtonSpellbookLevelSelection::~CUIControlButtonSpellbookLevelSelection()
{
}

// 0x66EDA0
void CUIControlButtonSpellbookLevelSelection::OnLButtonClick(CPoint pt)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    pSpellbook->SetSpellLevel(m_nID - 55);
    pSpellbook->sub_669830(pSpellbook->GetSelectedCharacter());
    pSpellbook->UpdateMainPanel();
}

// 0x66EE20
void CUIControlButtonSpellbookLevelSelection::OnLButtonDoubleClick(CPoint pt)
{
    OnLButtonClick(pt);
}

// -----------------------------------------------------------------------------

// 0x66EDE0
CUIControlButtonSpellbookClassSelection::CUIControlButtonSpellbookClassSelection(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton3State(panel, controlInfo, LBUTTON, 1)
{
    m_nSelectedFrame = 0;
}

// 0x66EE60
CUIControlButtonSpellbookClassSelection::~CUIControlButtonSpellbookClassSelection()
{
}

// 0x66EF00
void CUIControlButtonSpellbookClassSelection::OnLButtonClick(CPoint pt)
{
    CScreenSpellbook* pSpellbook = g_pBaldurChitin->m_pEngineSpellbook;

    DWORD nClassIndex = pSpellbook->m_nClassIndex;
    DWORD nNewClassIndex = m_nID - 88;

    switch (m_nID) {
    case 88:
    case 89:
    case 90:
    case 91:
        if (nClassIndex >= 4) {
            nNewClassIndex += 4;
        }

        if (nNewClassIndex < pSpellbook->m_nNumberOfSpellClasses) {
            pSpellbook->SetClassIndex(nNewClassIndex);
        }

        pSpellbook->sub_669830(pSpellbook->GetSelectedCharacter());
        pSpellbook->UpdateMainPanel();
        break;
    case 92:
        // Scroll class tabs left.
        if (nClassIndex >= 4) {
            pSpellbook->SetClassIndex(nClassIndex - 4);
            pSpellbook->sub_669830(pSpellbook->GetSelectedCharacter());
            pSpellbook->UpdateMainPanel();
        }
        break;
    case 93:
        // Scroll class tabs right.
        if (nClassIndex < 4 && pSpellbook->m_nNumberOfSpellClasses > 4) {
            nNewClassIndex = min(nClassIndex + 4, pSpellbook->m_nNumberOfSpellClasses - 1);
            pSpellbook->SetClassIndex(nNewClassIndex);
            pSpellbook->sub_669830(pSpellbook->GetSelectedCharacter());
            pSpellbook->UpdateMainPanel();
        }
        break;
    }
}

// 0x66EE20
void CUIControlButtonSpellbookClassSelection::OnLButtonDoubleClick(CPoint pt)
{
    OnLButtonClick(pt);
}
