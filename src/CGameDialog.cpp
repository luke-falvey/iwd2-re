#include "CGameDialog.h"

#include "CAIScriptFile.h"
#include "CBaldurChitin.h"
#include "CCreatureFile.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CResDLG.h"
#include "CScreenWorld.h"
#include "CUIControlButton.h"
#include "CUIControlTextDisplay.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "CVidPalette.h"
#include "FileFormat.h"

// NOTE: Inlined.
CGameDialogSprite::CGameDialogSprite()
{
    ClearMarshal();

    // NOTE: Stored by the inlined constructor in `CScreenWorld::CScreenWorld`
    // (0x6856CD). `CInfGame::SetProtagonist` reads it before any dialog runs.
    field_54 = 0;
}

// 0x67CB50
CGameDialogSprite::~CGameDialogSprite()
{
    ClearMarshal();
}

// 0x482210
void CGameDialogSprite::sub_482210(CResRef cResDialog, LONG a2, LONG a3)
{
    if (cResDialog == "") {
        ClearMarshal();
        return;
    }

    if (cResDialog == m_file) {
        m_characterIndex = a2;
        g_pBaldurChitin->GetObjectGame()->SetProtagonist(a2);

        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
            && g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
            CMessage* message = new CMessageSetProtagonist(m_characterIndex,
                m_characterIndex,
                m_characterIndex);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        m_talkerIndex = a3;
        return;
    }

    CDialogFile dialogFile;

    // NOTE: Uninline.
    dialogFile.SetResRef(cResDialog, TRUE, TRUE);

    if (dialogFile.GetDataSize() != 0) {
        m_file = cResDialog;
        Unmarshal(dialogFile.GetData(), dialogFile.GetDataSize(), a2, a3);
        dialogFile.ReleaseData();
    } else {
        ClearMarshal();
    }
}

// 0x4824F0
void CGameDialogSprite::Reload(CResRef cResDialog, CGameSprite* pSprite)
{
    LONG nTalker;

    if (m_file != cResDialog) {
        if (pSprite->m_dialog == cResDialog || pSprite->field_56E4 == cResDialog) {
            nTalker = pSprite->m_id;
        } else {
            CGameArea* pArea = pSprite->m_pArea;
            if (pArea->m_cInfinity.nAreaX > pArea->m_cInfinity.nAreaY) {
                nTalker = pArea->GetNearestDialogMatch(pSprite->m_id,
                    cResDialog,
                    pArea->m_cInfinity.nAreaX);
            } else {
                nTalker = pArea->GetNearestDialogMatch(pSprite->m_id,
                    cResDialog,
                    pArea->m_cInfinity.nAreaY);
            }
        }

        if (nTalker == CGameObjectArray::INVALID_INDEX) {
            CString sResRef;
            cResDialog.CopyToString(sResRef);

            // NOTE: Unused.
            CString sError = "No sprite with dialog file: " + sResRef;

            CPoint pt = pSprite->GetPos();
            CPoint ptGrid(pt.x / CPathSearch::GRID_SQUARE_SIZEX,
                pt.y / CPathSearch::GRID_SQUARE_SIZEY);

            BYTE personalSpace = pSprite->m_animation.GetPersonalSpace();
            ptGrid = pSprite->m_pArea->m_search.GetNearestOpenSquare(ptGrid,
                pSprite->GetTerrainTable(),
                personalSpace,
                -1);

            CCreatureFile creatureData;

            // NOTE: Uninline.
            creatureData.SetResRef(CResRef("DIALOGMO"), TRUE, TRUE);

            CGameSprite* pMo = new CGameSprite(creatureData.GetData(),
                creatureData.GetDataSize(),
                0,
                0,
                -1,
                0,
                0,
                0x7FFFFFFF,
                CPoint(-1, -1),
                0);

            // NOTE: See `CGameAIBase::CreateCreature`. The original calls
            // `SetResRef` and `ReleaseData` before checking `pMo`.
            pMo->SetResRef(CResRef(creatureData.GetResRef().GetResRefStr()));
            creatureData.ReleaseData();

            if (pMo == NULL) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
                // __LINE__: 349
                UTIL_ASSERT_MSG(FALSE, "Where's Mo the understudy?");
                return;
            }

            LONG nMo = pMo->m_id;

            CString sMo("DIALOGMO");
            pMo->SetResRef(CResRef(sMo));

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nMo,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pMo),
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
                // __LINE__: 360
                UTIL_ASSERT_MSG(FALSE, "Where's Mo the understudy?");
                return;
            }

            BYTE listType = pMo->m_animation.GetListType();
            pMo->AddToArea(pSprite->m_pArea,
                CPoint(ptGrid.x * CPathSearch::GRID_SQUARE_SIZEX, ptGrid.y * CPathSearch::GRID_SQUARE_SIZEY),
                0,
                listType);

            pMo->m_dialog = CResRef(sResRef);
            pMo->SetSequence(CGameSprite::SEQ_READY);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nMo,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            nTalker = nMo;
        }
    } else {
        nTalker = m_talkerIndex;
    }

    m_talkerIndex = nTalker;
    g_pBaldurChitin->GetObjectGame()->SetProtagonist(m_characterIndex);

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        CMessage* message = new CMessageSetProtagonist(m_characterIndex,
            m_characterIndex,
            m_characterIndex);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }
}

// 0x482A40
void CGameDialogSprite::Unmarshal(BYTE* pDialog, LONG dialogSize, LONG a3, LONG a4)
{
    CGameSprite* pSprite;
    BYTE rc;
    DWORD nIndex;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
    // __LINE__: 427
    UTIL_ASSERT(pDialog != NULL);

    // NOTE: The original uses five distinct array types (separate vtables),
    // the three script arrays share one layout.
    CTypedPtrArray<CPtrArray, CDialogFileEntry*> rawEntries;
    CTypedPtrArray<CPtrArray, CDialogFileReply*> rawReplies;
    CTypedPtrArray<CPtrArray, CDialogFileScript*> rawStartConditions;
    CTypedPtrArray<CPtrArray, CDialogFileScript*> rawReplyConditions;
    CTypedPtrArray<CPtrArray, CDialogFileScript*> rawActions;

    if (dialogSize == 0) {
        return;
    }

    ClearMarshal();

    dialogSize -= 0x30;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
    // __LINE__: 448
    UTIL_ASSERT(dialogSize > 0);

    CDialogFileHeader header = *reinterpret_cast<CDialogFileHeader*>(pDialog + 8);

    CDialogFileEntry* pFileEntries = reinterpret_cast<CDialogFileEntry*>(pDialog + header.m_offsetEntries);
    nIndex = 0;
    while (nIndex < header.m_nEntries) {
        CDialogFileEntry* pRawEntry = new CDialogFileEntry();
        if (pRawEntry == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
            // __LINE__: 459
            UTIL_ASSERT(FALSE);
        } else {
            *pRawEntry = *pFileEntries;
            rawEntries.Add(pRawEntry);
            dialogSize -= sizeof(CDialogFileEntry);
            nIndex++;
            pFileEntries++;
        }
    }

    CDialogFileReply* pFileReplies = reinterpret_cast<CDialogFileReply*>(pDialog + header.m_offsetReplies);
    nIndex = 0;
    while (nIndex < header.m_nReplies) {
        CDialogFileReply* pRawReply = new CDialogFileReply();
        if (pRawReply == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
            // __LINE__: 475
            UTIL_ASSERT(FALSE);
        } else {
            *pRawReply = *pFileReplies;
            rawReplies.Add(pRawReply);
            dialogSize -= sizeof(CDialogFileReply);
            nIndex++;
            pFileReplies++;
        }
    }

    CDialogFileScript* pFileScripts = reinterpret_cast<CDialogFileScript*>(pDialog + header.m_offsetStartingConditions);
    nIndex = 0;
    while (nIndex < header.m_nStartingConditions) {
        CDialogFileScript* pRawScript = new CDialogFileScript();
        if (pRawScript == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
            // __LINE__: 491
            UTIL_ASSERT(FALSE);
        } else {
            *pRawScript = *pFileScripts;
            rawStartConditions.Add(pRawScript);
            dialogSize -= sizeof(CDialogFileScript);
            nIndex++;
            pFileScripts++;
        }
    }

    pFileScripts = reinterpret_cast<CDialogFileScript*>(pDialog + header.m_offsetReplyConditions);
    nIndex = 0;
    while (nIndex < header.m_nReplyConditions) {
        CDialogFileScript* pRawScript = new CDialogFileScript();
        if (pRawScript == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
            // __LINE__: 507
            UTIL_ASSERT(FALSE);
        } else {
            *pRawScript = *pFileScripts;
            rawReplyConditions.Add(pRawScript);
            dialogSize -= sizeof(CDialogFileScript);
            nIndex++;
            pFileScripts++;
        }
    }

    pFileScripts = reinterpret_cast<CDialogFileScript*>(pDialog + header.m_offsetActions);
    nIndex = 0;
    while (nIndex < header.m_nActions) {
        CDialogFileScript* pRawScript = new CDialogFileScript();
        if (pRawScript == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
            // __LINE__: 523
            UTIL_ASSERT(FALSE);
        } else {
            *pRawScript = *pFileScripts;
            rawActions.Add(pRawScript);
            nIndex++;
            pFileScripts++;
        }
    }

    m_characterIndex = a3;
    m_talkerIndex = a4;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(a3,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
        // __LINE__: 547
        UTIL_ASSERT(FALSE);
        return;
    }

    m_playerColor = CVidPalette::RANGE_COLORS[pSprite->GetBaseStats()->m_colors[CVIDPALETTE_RANGE_MAIN_CLOTH]];
    m_playerName = pSprite->GetName();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(a3,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    CAIScriptFile scriptFile;

    for (DWORD nEntry = 0; nEntry < header.m_nEntries; nEntry++) {
        CGameDialogEntry* pEntry = new CGameDialogEntry();
        CDialogFileEntry* pFileEntry = rawEntries[nEntry];

        pEntry->m_dialogText = pFileEntry->m_dialogText;

        for (DWORD nReply = 0; nReply < pFileEntry->m_replyCount; nReply++) {
            CDialogFileReply* pFileReply = rawReplies[pFileEntry->m_startingReply + nReply];

            CGameDialogReply* pReply = new CGameDialogReply();
            pReply->m_flags = pFileReply->m_flags;

            if ((pFileReply->m_flags & 0x1) != 0) {
                pReply->m_replyText = pFileReply->m_replyText;
            } else {
                pReply->m_replyText = -1;
            }

            if ((pFileReply->m_flags & 0x10) != 0) {
                pReply->m_journalEntry = pFileReply->m_journalEntry;
            } else {
                pReply->m_journalEntry = -1;
            }

            if ((pFileReply->m_flags & 0x2) != 0) {
                CString sCondition;

                if (pFileReply->m_conditionalScript != -1) {
                    CDialogFileScript* pScript = rawReplyConditions[pFileReply->m_conditionalScript];

                    CString sRaw;
                    sRaw = CString(reinterpret_cast<char*>(pDialog + pScript->m_offset), pScript->m_length);
                    sRaw.TrimLeft();
                    sRaw.TrimRight();

                    while (sRaw.GetLength() > 0) {
                        INT index = sRaw.Find(')');
                        if (index < 0) {
                            CString sError("Error found while parsing dialog conditions:");
                            sError += sCondition + sRaw;

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
                            // __LINE__: 620
                            UTIL_ASSERT_MSG(index >= 0, sError);
                        }

                        sCondition += sRaw.Left(index + 1);
                        sCondition += '\n';
                        sRaw = sRaw.Right(sRaw.GetLength() - index - 1);
                        sRaw.TrimLeft();
                    }
                } else {
                    sCondition = "False()\n";
                }

                scriptFile.ParseConditionalString(sCondition);
                pReply->m_condition.Set(*scriptFile.m_curCondition);
            }

            if ((pFileReply->m_flags & 0x4) != 0) {
                CString sResponse;

                if (pFileReply->m_actionScript != -1) {
                    CDialogFileScript* pScript = rawActions[pFileReply->m_actionScript];

                    CString sRaw;
                    sRaw = CString(reinterpret_cast<char*>(pDialog + pScript->m_offset), pScript->m_length);
                    sRaw.TrimLeft();
                    sRaw.TrimRight();

                    while (sRaw.GetLength() > 0) {
                        INT index = sRaw.Find(')');
                        if (index < 0) {
                            CString sError("Error found while parsing dialog responses:");
                            sError += sResponse + sRaw;

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
                            // __LINE__: 677
                            UTIL_ASSERT_MSG(index >= 0, sError);
                        }

                        sResponse += sRaw.Left(index + 1);
                        sResponse += '\n';
                        sRaw = sRaw.Right(sRaw.GetLength() - index - 1);
                        sRaw.TrimLeft();
                    }
                } else {
                    sResponse = "\n";
                }

                scriptFile.ParseResponseString(sResponse);
                pReply->m_response.Set(*scriptFile.m_curResponse);
            }

            pReply->m_nextDialog = CResRef(pFileReply->m_nextDialog);
            pReply->m_nextEntryIndex = pFileReply->m_nextEntryIndex;

            pEntry->Add(pReply);
        }

        CString sCondition;

        if (pFileEntry->m_StartingConditionalScript != -1) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
            // __LINE__: 719
            UTIL_ASSERT_MSG(pFileEntry->m_StartingConditionalScript < (DWORD)rawStartConditions.GetSize(),
                "Blank Starting condition in Dialog file");

            CDialogFileScript* pScript = rawStartConditions[pFileEntry->m_StartingConditionalScript];

            CString sRaw;
            sRaw = CString(reinterpret_cast<char*>(pDialog + pScript->m_offset), pScript->m_length);
            sRaw.TrimLeft();
            sRaw.TrimRight();

            while (sRaw.GetLength() > 0) {
                INT index = sRaw.Find(')');
                if (index < 0) {
                    CString sError("Error found while parsing dialog conditions:");
                    sError += sCondition + sRaw;

                    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
                    // __LINE__: 740
                    UTIL_ASSERT_MSG(index >= 0, sError);
                }

                sCondition += sRaw.Left(index + 1);
                sCondition += '\n';
                sRaw = sRaw.Right(sRaw.GetLength() - index - 1);
                sRaw.TrimLeft();
            }

            if (strcmp(sCondition, "") == 0) {
                sCondition = "False()";
            }
        } else {
            sCondition = "False()\n";
        }

        scriptFile.ParseConditionalString(sCondition);
        pEntry->m_startCondition.Set(*scriptFile.m_curCondition);

        pEntry->m_dialogIndex = m_dialogEntries.GetSize();
        m_dialogEntries.Add(pEntry);

        pEntry->m_conditionPriority = pFileEntry->m_StartingConditionalScript;

        BOOL bInserted = FALSE;
        for (INT nOrdered = 0; nOrdered < m_dialogEntriesOrdered.GetSize(); nOrdered++) {
            // NOTE: Unsigned compare.
            if (pEntry->m_conditionPriority < m_dialogEntriesOrdered[nOrdered]->m_conditionPriority) {
                m_dialogEntriesOrdered.InsertAt(nOrdered, pEntry);
                bInserted = TRUE;
                break;
            }
        }

        if (!bInserted) {
            m_dialogEntriesOrdered.Add(pEntry);
        }
    }

    INT n;

    for (n = 0; n < rawEntries.GetSize(); n++) {
        if (rawEntries[n] != NULL) {
            delete rawEntries[n];
        }
    }
    rawEntries.RemoveAll();

    for (n = 0; n < rawReplies.GetSize(); n++) {
        if (rawReplies[n] != NULL) {
            delete rawReplies[n];
        }
    }
    rawReplies.RemoveAll();

    for (n = 0; n < rawStartConditions.GetSize(); n++) {
        if (rawStartConditions[n] != NULL) {
            delete rawStartConditions[n];
        }
    }
    rawStartConditions.RemoveAll();

    for (n = 0; n < rawReplyConditions.GetSize(); n++) {
        if (rawReplyConditions[n] != NULL) {
            delete rawReplyConditions[n];
        }
    }
    rawReplyConditions.RemoveAll();

    for (n = 0; n < rawActions.GetSize(); n++) {
        if (rawActions[n] != NULL) {
            delete rawActions[n];
        }
    }
    rawActions.RemoveAll();
}

// 0x483970
void CGameDialogSprite::ClearMarshal()
{
    m_characterIndex = 0;
    m_talkerIndex = 0;
    m_dialogFreezeCounter = 6;
    m_dialogFreezeMultiplayer = 0;
    m_file = "";
    m_waitingForResponse = FALSE;
    m_currentEntryIndex = 0;
    m_responseMarker = -1;

    for (INT nIndex = 0; nIndex < m_dialogEntries.GetCount(); nIndex++) {
        CGameDialogEntry* pEntry = m_dialogEntries.GetAt(nIndex);
        if (pEntry != NULL) {
            delete pEntry;
        }
    }

    m_dialogEntries.RemoveAll();
    m_dialogEntriesOrdered.RemoveAll();
}

// 0x4839F0
BOOL CGameDialogSprite::StartDialog(CGameSprite* pSprite)
{
    g_pBaldurChitin->m_pEngineWorld->DisableKeyRepeat();

    m_nMusicThreadPriority = GetThreadPriority(g_pChitin->m_hMusicThread);
    if (GetPrivateProfileIntA("Program Options", "Volume Music", 0, g_pChitin->GetIniFileName())) {
        m_bMusicThreadPriorityChanged = SetThreadPriority(g_pChitin->m_hMusicThread, 15);
    }

    for (INT nIndex = 0; nIndex < m_dialogEntriesOrdered.GetCount(); nIndex++) {
        CGameDialogEntry* pEntry = m_dialogEntriesOrdered.GetAt(nIndex);
        CTypedPtrList<CPtrList, CAITrigger*> triggerList;
        if (pEntry != NULL
            && pEntry->m_startCondition.Hold(triggerList, pSprite)) {
            // FIXME: Unused.
            LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetProtagonist();

            CMessage* pMessage = new CMessageEnterDialog(pEntry->m_dialogIndex,
                TRUE,
                pSprite->GetId(),
                pSprite->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            field_54 = 1;
            field_56 = "";

            return TRUE;
        }
    }

    return FALSE;
}

// 0x483B70
BOOL CGameDialogSprite::FetchRumor(const CResRef& file, CGameSprite* pSprite, LONG& nIndex, STR_RES& strRes)
{
    CTypedPtrArray<CPtrArray, CGameDialogEntry*> validEntries;

    sub_482210(file, pSprite->GetId(), pSprite->GetId());

    for (INT n = 0; n < m_dialogEntriesOrdered.GetSize(); n++) {
        CGameDialogEntry* pEntry = m_dialogEntriesOrdered[n];
        CTypedPtrList<CPtrList, CAITrigger*> triggerList;
        if (pEntry != NULL
            && pEntry->m_startCondition.Hold(triggerList, pSprite)) {
            validEntries.Add(m_dialogEntriesOrdered[n]);
        }
    }

    if (validEntries.GetSize() <= 0) {
        return FALSE;
    }

    if (nIndex < 0) {
        nIndex = rand() % validEntries.GetSize();
    }

    if (nIndex >= validEntries.GetSize()) {
        nIndex %= validEntries.GetSize();
    }

    CGameDialogEntry* pEntry = validEntries[nIndex];
    g_pBaldurChitin->GetTlkTable().Fetch(pEntry->m_dialogText, strRes);

    if (pEntry->GetSize() > 0) {
        CGameDialogReply* pReply = pEntry->GetAt(0);
        if ((pReply->m_flags & 0x10) != 0) {
            g_pBaldurChitin->GetObjectGame()->m_cJournal.AddEntry(pReply->m_journalEntry, 0);
        }
    }

    nIndex++;

    return TRUE;
}

// 0x483CF0
void CGameDialogSprite::EndDialog()
{
    if (m_bMusicThreadPriorityChanged == TRUE) {
        SetThreadPriority(g_pChitin->m_hMusicThread, m_nMusicThreadPriority);
    }

    g_pBaldurChitin->m_pEngineWorld->EnableKeyRepeat();

    m_waitingForResponse = FALSE;
    m_responseMarker = -1;
    field_54 = 0;
    field_56 = "";

    CResRef cResRef("SilentDH");

    CSound cSound;
    cSound.SetResRef(cResRef, TRUE, TRUE);
    if (cSound.m_nLooping == 0) {
        cSound.SetFireForget(TRUE);
    }
    cSound.SetChannel(6, reinterpret_cast<DWORD>(g_pBaldurChitin->GetObjectGame()->GetVisibleArea()));
    cSound.Play(FALSE);
}

// 0x483EB0
BOOL CGameDialogSprite::EnterDialog(DWORD index, CGameSprite* pSprite, int a3)
{
    CGameDialogEntry* pEntry = m_dialogEntries[index];
    if (pEntry == NULL) {
        m_currentEntryIndex = 0;
        return FALSE;
    }

    m_currentEntryIndex = index;
    m_waitingForResponse = TRUE;
    m_responseMarker = -1;
    pEntry->Handle(pSprite, m_playerColor, a3);
    return TRUE;
}

// 0x483F00
void CGameDialogSprite::AsynchronousUpdate()
{
    CGameSprite* pSprite;
    BYTE rc;

    DWORD nMode = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode;

    if (!m_waitingForResponse) {
        if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode != 1282) {
            return;
        }

        if (!g_pChitin->cNetwork.GetSessionOpen()) {
            if (m_dialogFreezeCounter-- > 0) {
                return;
            }

            m_dialogFreezeCounter = 6;
        } else {
            if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode != 1282) {
                return;
            }

            if (m_dialogFreezeMultiplayer++ < 10) {
                return;
            }

            m_dialogFreezeMultiplayer = 0;

            if (m_dialogFreezeCounter-- > 0) {
                return;
            }

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_talkerIndex,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                // NOTE: Unused (debug output stripped).
                CString sError = "Sprite trying to execute dialog with none" + pSprite->GetName();

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_talkerIndex,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            } else {
                // NOTE: Unused (debug output stripped).
                CString sError("Unknown sprite trying to execute dialog with none");
            }

            m_dialogFreezeCounter = 6;
        }

        g_pBaldurChitin->m_pEngineWorld->EndDialog(FALSE, TRUE);
        return;
    }

    if (strcmp(field_56, "") != 0) {
        CString sString;

        INT nIndex = g_pBaldurChitin->m_pEngineWorld->m_pActiveDialogDisplay->m_plstStrings->GetCount() - 1;
        for (; nIndex >= 0; nIndex--) {
            g_pBaldurChitin->m_pEngineWorld->m_pActiveDialogDisplay->GetString(nIndex, sString);
            if (strcmp(sString, field_56) == 0) {
                g_pBaldurChitin->m_pEngineWorld->m_pActiveDialogDisplay->SetTopString(g_pBaldurChitin->m_pEngineWorld->m_pActiveDialogDisplay->m_plstStrings->FindIndex(nIndex));
                break;
            }
        }

        field_56 = "";
    }

    m_dialogFreezeCounter = 6;

    CGameDialogEntry* pEntry = m_dialogEntries.GetAt(m_currentEntryIndex);

    CUIControlBase* pControl = g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->GetControl(0);
    if (pEntry->m_bDisplayButton && !pControl->m_bActive && nMode == 1282) {
        pControl->SetActive(TRUE);
        g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->InvalidateRect(NULL);
    } else if ((!pEntry->m_bDisplayButton && pControl->m_bActive) || nMode != 1282) {
        pControl->SetActive(FALSE);
        g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->InvalidateRect(NULL);
    }

    g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->InvalidateRect(NULL);

    LONG nTalkerIndex = m_talkerIndex;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nTalkerIndex,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->m_pEngineWorld->EndDialog(FALSE, TRUE);
        return;
    }

    CGameDialogTransition* pTransition;
    if (m_responseMarker == -1) {
        pTransition = NULL;
        pEntry->m_picked = FALSE;
    } else {
        pEntry->m_picked = TRUE;
        if (m_responseMarker == -2) {
            pTransition = NULL;
        } else {
            pTransition = pEntry->GetAt(m_responseMarker)->Pick(pSprite);
        }
    }

    if (pEntry->m_picked) {
        if (pTransition != NULL) {
            Reload(pTransition->m_nextDialog, pSprite);

            CString sDialog;
            pTransition->m_nextDialog.CopyToString(sDialog);

            CMessage* pMessage = new CMessagePickedDialogResponse(m_currentEntryIndex,
                m_responseMarker,
                m_playerColor,
                m_playerName,
                pSprite->GetId(),
                sDialog,
                m_talkerIndex,
                m_characterIndex,
                pTransition->m_nextEntryIndex,
                FALSE,
                m_talkerIndex);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            m_responseMarker = -1;

            delete pTransition;
        } else {
            CMessage* pMessage = new CMessageRemoveReplies(m_currentEntryIndex,
                m_responseMarker,
                m_playerColor,
                m_playerName,
                pSprite->GetId(),
                pSprite->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            g_pBaldurChitin->m_pEngineWorld->EndDialog(FALSE, TRUE);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nTalkerIndex,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x4845C0
void CGameDialogSprite::UpdateDialogColors()
{
    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDialog.cpp
    // __LINE__: 1295
    UTIL_ASSERT(pWorld != NULL);

    if (m_waitingForResponse && pWorld->m_pActiveDialogDisplay != NULL) {
        COLORREF rgb = RGB(215, 215, 190);
        if (pWorld->m_bInControlOfDialog) {
            rgb = RGB(255, 46, 33);
        }

        CGameDialogEntry* pEntry = m_dialogEntries[m_currentEntryIndex];

        for (INT nIndex = 0; nIndex < pEntry->GetCount(); nIndex++) {
            CGameDialogReply* pReply = pEntry->GetAt(nIndex);
            if (pReply->m_displayPosition != NULL) {
                pWorld->m_pActiveDialogDisplay->SetItemTextColor(pReply->m_displayPosition, rgb);
            }
        }
    }
}

// 0x484680
CGameDialogEntry::~CGameDialogEntry()
{
    for (INT nIndex = 0; nIndex < GetCount(); nIndex++) {
        CGameDialogReply* pReply = GetAt(nIndex);
        if (pReply != NULL) {
            delete pReply;
        }
    }
    RemoveAll();
}

// 0x484730
void CGameDialogEntry::RemoveReplies(LONG lMarker, COLORREF rgbNameColor, const CString& sName)
{
    STR_RES strRes;
    BOOLEAN bRemoveIfPicked = FALSE;
    STRREF strReplyText;

    for (INT nIndex = 0; nIndex < GetCount(); nIndex++) {
        CGameDialogReply* pReply = GetAt(nIndex);
        if ((pReply->m_flags & 0x20) == 0) {
            if (pReply->m_displayPosition != NULL) {
                if (nIndex == lMarker) {
                    bRemoveIfPicked = pReply->m_removeIfPicked;
                    strReplyText = pReply->m_replyText;
                }

                g_pBaldurChitin->m_pEngineWorld->RemoveText(pReply->m_displayPosition);
                pReply->m_displayPosition = NULL;
            }
        }
    }

    if (!bRemoveIfPicked) {
        if (lMarker >= 0 && lMarker < GetCount()) {
            strReplyText = GetAt(lMarker)->m_replyText;
        }

        g_pBaldurChitin->GetTlkTable().Fetch(strReplyText, strRes);

        g_pBaldurChitin->m_pEngineWorld->DisplayText(sName,
            strRes.szText,
            rgbNameColor,
            RGB(160, 200, 215),
            -1,
            FALSE);

        g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
            CString(""),
            -1,
            FALSE);
    }
}

// NOTE: Inlined in `CGameDialogEntry::Handle`.
static void PlayDialogSound(CSound& cSound)
{
    if (cSound.pRes != NULL) {
        if (cSound.GetLooping() == 0) {
            cSound.SetFireForget(TRUE);
        }

        SleepEx(10, FALSE);
        cSound.Play(FALSE);

        if (cSound.GetLooping() == 0 && cSound.pRes != NULL) {
            cSound.pRes->Dump(TRUE);
        }
    }
}

// 0x484900
void CGameDialogEntry::Handle(CGameSprite* pSprite, COLORREF playerColor, int a3)
{
    INT nIndex;
    CGameDialogReply* pReply = NULL;
    BOOL bValid;
    COLORREF rgbTextColor;

    m_bDisplayButton = FALSE;

    STR_RES strRes;

    DWORD nMode = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode;

    g_pBaldurChitin->GetTlkTable().Fetch(m_dialogText, strRes);
    strRes.cSound.SetChannel(6, reinterpret_cast<DWORD>(pSprite->GetArea()));

    CPoint pos = pSprite->GetPos();

    PlayDialogSound(strRes.cSound);

    CRect rFx;
    CPoint ptReference(0, 0);
    if (pSprite->m_animation.m_animation != NULL) {
        // NOTE: Uninline.
        pSprite->m_animation.CalculateFxRect(rFx, ptReference, 0);
    }

    pos.y -= ptReference.y / 2;

    // NOTE: Unused.
    CRect rViewPort;
    CopyRect(&rViewPort, &(pSprite->GetArea()->m_cInfinity.rViewPort));

    pos.x -= (CInfinity::stru_8E7958.right - CInfinity::stru_8E7958.left) / 2;
    pos.y -= (CInfinity::stru_8E7958.bottom - CInfinity::stru_8E7958.top) / 2;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    BOOLEAN bSetMessageScreen = FALSE;

    if (pSprite->GetArea() != pGame->m_gameAreas[pGame->m_visibleArea]) {
        CGameArea* pVisibleArea = pGame->m_gameAreas[pGame->m_visibleArea];
        pVisibleArea->m_bPicked = FALSE;

        pVisibleArea = g_pBaldurChitin->GetObjectGame()->m_gameAreas[g_pBaldurChitin->GetObjectGame()->m_visibleArea];
        pVisibleArea->m_iPicked = CGameObjectArray::INVALID_INDEX;
        pVisibleArea->m_nToolTip = 0;

        g_pBaldurChitin->GetObjectGame()->m_gameAreas[g_pBaldurChitin->GetObjectGame()->m_visibleArea]->OnDeactivation();

        g_pBaldurChitin->GetObjectGame()->m_visibleArea = pSprite->GetArea()->m_id;

        g_pBaldurChitin->GetObjectGame()->m_gameAreas[g_pBaldurChitin->GetObjectGame()->m_visibleArea]->OnActivation();

        bSetMessageScreen = g_pChitin->cNetwork.GetSessionOpen();
    }

    if (pSprite->GetArea() == g_pBaldurChitin->GetObjectGame()->m_gameAreas[g_pBaldurChitin->GetObjectGame()->m_visibleArea]) {
        INT x;
        INT y;
        g_pBaldurChitin->GetObjectGame()->m_gameAreas[g_pBaldurChitin->GetObjectGame()->m_visibleArea]->m_cInfinity.GetViewPosition(x, y);

        if ((x - pos.x) * (x - pos.x) + (y - pos.y) * (y - pos.y) > 490000) {
            g_pBaldurChitin->m_pEngineWorld->StartScroll(pos, 0);
            bSetMessageScreen = g_pChitin->cNetwork.GetSessionOpen();
        } else {
            g_pBaldurChitin->m_pEngineWorld->StartScroll(pos, 16);
        }
    }

    if (bSetMessageScreen) {
        g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->GetInfinity()->SetMessageScreen(CResRef(""), 10467, 1000);
    }

    COLORREF rgbNameColor = CVidPalette::RANGE_COLORS[pSprite->GetBaseStats()->m_colors[2]];

    g_pBaldurChitin->m_pEngineWorld->field_10B4 = 1;

    if (strcmp(strRes.szText, "") != 0) {
        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
            && !g_pBaldurChitin->m_pEngineWorld->m_bInControlOfDialog) {
            g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                CString(""),
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                FALSE);
        } else {
            g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                CString(""),
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                TRUE);
        }

        g_pBaldurChitin->m_pEngineWorld->DisplayText(pSprite->GetName(),
            strRes.szText,
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            FALSE);

        g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
            CString(""),
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            FALSE);
    }

    strRes.cSound.SetChannel(6, reinterpret_cast<DWORD>(pSprite->GetArea()));
    PlayDialogSound(strRes.cSound);

    CString sNumber;
    SHORT nCount = 0;

    for (nIndex = 0; nIndex < GetCount(); nIndex++) {
        CGameDialogReply* pCurrentReply = GetAt(nIndex);
        if (pCurrentReply != NULL) {
            pReply = pCurrentReply;
            if ((pReply->m_flags & 0x20) != 0) {
                // NOTE: Inlining as trigger list is immediately destroyed.
                if (1) {
                    CTypedPtrList<CPtrList, CAITrigger*> triggers(10);
                    if ((pReply->m_flags & 0x2) == 0
                        || pReply->m_condition.Hold(triggers, pSprite)) {
                        bValid = TRUE;
                    } else {
                        bValid = FALSE;
                    }
                }

                if (bValid) {
                    nCount++;

                    if ((pReply->m_flags & 0x1) != 0) {
                        g_pBaldurChitin->GetTlkTable().Fetch(pReply->m_replyText, strRes);
                        sNumber.Format("    %d:", nCount);
                        pReply->m_removeIfPicked = FALSE;
                        pReply->m_displayListId = static_cast<BYTE>(nCount);
                    }

                    strRes.cSound.SetChannel(6, reinterpret_cast<DWORD>(pSprite->GetArea()));
                    PlayDialogSound(strRes.cSound);

                    rgbTextColor = g_pBaldurChitin->m_pEngineWorld->m_bInControlOfDialog
                        ? RGB(255, 46, 33)
                        : RGB(215, 215, 190);

                    if (strcmp(strRes.szText, "") != 0) {
                        pReply->m_displayPosition = g_pBaldurChitin->m_pEngineWorld->DisplayText(sNumber,
                            strRes.szText,
                            playerColor,
                            rgbTextColor,
                            nIndex,
                            FALSE);
                    } else {
                        pReply->m_displayPosition = g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                            CString(""),
                            playerColor,
                            rgbTextColor,
                            nIndex,
                            FALSE);
                    }
                } else {
                    pReply->m_displayListId = 0xFF;
                }
            }
        }
    }

    for (nIndex = 0; nIndex < GetCount(); nIndex++) {
        CGameDialogReply* pCurrentReply = GetAt(nIndex);
        if (pCurrentReply != NULL) {
            pReply = pCurrentReply;

            // NOTE: Inlining as trigger list is immediately destroyed.
            if (1) {
                CTypedPtrList<CPtrList, CAITrigger*> triggers(10);
                if ((pReply->m_flags & 0x2) == 0
                    || pReply->m_condition.Hold(triggers, pSprite)) {
                    bValid = TRUE;
                } else {
                    bValid = FALSE;
                }
            }

            if (bValid) {
                nCount++;

                if ((pReply->m_flags & 0x1) != 0) {
                    g_pBaldurChitin->GetTlkTable().Fetch(pReply->m_replyText, strRes);
                    sNumber.Format("    %d:", nCount);
                    pReply->m_removeIfPicked = FALSE;
                    pReply->m_displayListId = static_cast<BYTE>(nCount);
                } else {
                    m_bDisplayButton = TRUE;

                    if ((pReply->m_flags & 0x8) != 0) {
                        // "End Dialog"
                        g_pBaldurChitin->GetTlkTable().Fetch(9371, strRes);
                        sNumber = "";
                        pReply->m_removeIfPicked = TRUE;

                        g_pBaldurChitin->m_pEngineWorld->field_10B2 = static_cast<SHORT>(nIndex);
                        g_pBaldurChitin->m_pEngineWorld->field_10B4 = 0;

                        CUIControlButton* pButton = static_cast<CUIControlButton*>(g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->GetControl(0));
                        pButton->SetText(strRes.szText);

                        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                            pButton->SetActive(nMode == 1282);
                            g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->InvalidateRect(NULL);
                        }

                        strRes.szText = "";

                        if (pReply->m_response.InList(CAIAction::STARTCUTSCENEMODE) == TRUE) {
                            g_pBaldurChitin->m_pEngineWorld->field_1204 = 1;
                        }
                    } else {
                        // "Continue"
                        g_pBaldurChitin->GetTlkTable().Fetch(9372, strRes);
                        sNumber = "";
                        pReply->m_removeIfPicked = TRUE;

                        g_pBaldurChitin->m_pEngineWorld->field_10B2 = static_cast<SHORT>(nIndex);
                        g_pBaldurChitin->m_pEngineWorld->field_10B4 = 0;

                        CUIControlButton* pButton = static_cast<CUIControlButton*>(g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->GetControl(0));
                        pButton->SetText(strRes.szText);

                        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                            pButton->SetActive(nMode == 1282);
                            g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->InvalidateRect(NULL);
                        }

                        strRes.szText = "";
                    }
                }

                strRes.cSound.SetChannel(6, reinterpret_cast<DWORD>(pSprite->GetArea()));
                PlayDialogSound(strRes.cSound);

                rgbTextColor = g_pBaldurChitin->m_pEngineWorld->m_bInControlOfDialog
                    ? RGB(255, 46, 33)
                    : RGB(215, 215, 190);

                if (strcmp(strRes.szText, "") != 0) {
                    pReply->m_displayPosition = g_pBaldurChitin->m_pEngineWorld->DisplayText(sNumber,
                        strRes.szText,
                        playerColor,
                        rgbTextColor,
                        nIndex,
                        FALSE);
                } else {
                    pReply->m_displayPosition = g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                        CString(""),
                        playerColor,
                        rgbTextColor,
                        nIndex,
                        FALSE);
                }
            } else {
                pReply->m_displayListId = 0xFF;
            }
        }
    }

    if (nCount == 0) {
        m_bDisplayButton = TRUE;

        // FIXME: The original writes through whatever the last non-NULL reply
        // was, or an uninitialized pointer when there are no replies at all.
        if (pReply != NULL) {
            pReply->m_removeIfPicked = TRUE;
        }

        g_pBaldurChitin->m_pEngineWorld->field_10B2 = -2;
        g_pBaldurChitin->m_pEngineWorld->field_10B4 = 0;

        CUIControlButton* pButton = static_cast<CUIControlButton*>(g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->GetControl(0));
        pButton->SetText(strRes.szText);

        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            pButton->SetActive(nMode == 1282);
            g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(9)->InvalidateRect(NULL);
        }

        g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
            CString("NO VALID REPLIES OR LINKS"),
            playerColor,
            RGB(255, 46, 33),
            -2,
            FALSE);
    }
}

// 0x485750
CGameDialogTransition* CGameDialogReply::Pick(CGameSprite* pSprite)
{
    CMessage* pMessage;

    if ((m_flags & 0x4) != 0) {
        if (m_response.m_actionList.GetCount() != 0) {
            pMessage = new CMessageInsertResponse(m_response,
                FALSE,
                TRUE,
                pSprite->GetId(),
                pSprite->GetId(),
                TRUE);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            pMessage = new CMessageSetCommandPause(75,
                pSprite->GetId(),
                pSprite->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }

        pMessage = new CMessageSetDialogWait(0,
            CGameObjectArray::INVALID_INDEX,
            pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    if ((m_flags & 0x10) != 0) {
        if (g_pBaldurChitin->GetObjectGame()->m_cJournal.AddEntry(m_journalEntry, 0) == TRUE) {
            STR_RES strRes;
            // "Your journal has been updated."
            if (g_pBaldurChitin->GetTlkTable().Fetch(11359, strRes)) {
                g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.field_56 = strRes.szText;
            }
        }
    }

    if (m_displayPosition != NULL) {
        g_pBaldurChitin->m_pEngineWorld->SetItemMarker(m_displayPosition, -1);
    }

    if ((m_flags & 0x8) != 0) {
        return NULL;
    }

    CGameDialogTransition* pTransition = new CGameDialogTransition();
    pTransition->m_nextDialog = m_nextDialog;
    pTransition->m_nextEntryIndex = m_nextEntryIndex;

    return pTransition;
}
