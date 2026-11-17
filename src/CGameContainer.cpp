#include "CGameContainer.h"

#include "CAIScript.h"
#include "CAITrigger.h"
#include "CBaldurChitin.h"
#include "CCreatureFile.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CItem.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CProjectile.h"
#include "CResRef.h"
#include "CScreenWorld.h"
#include "CStore.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVidPoly.h"

// 0x8D3988
const CResRef CGameContainer::RESREF_AR6051("AR6051");

// 0x8D2564
BYTE CGameContainer::STATICBUFFER[STATICBUFFERSIZE_CGAMECONTAINER];

// 0x47BFA0
CGameContainer::CGameContainer(CGameArea* pArea, CAreaFileContainer* pContainerObject, CAreaPoint* pPoints, WORD maxPts, CCreatureFileItem* pItems, DWORD maxItems)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 113
    UTIL_ASSERT(pArea != NULL && pContainerObject != NULL && pPoints != NULL && pContainerObject->m_pickPointStart + pContainerObject->m_pickPointCount <= maxPts);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 114
    UTIL_ASSERT(pItems != NULL && pContainerObject->m_startingItem + pContainerObject->m_itemCount <= maxItems);

    // FIXME: Probably should reuse `RESREF_AR6051`.
    //
    // 0x8D3998
    static CResRef stru_8D3998("AR6051");

    BOOL bIsAR6051 = FALSE;
    if (pArea->m_resRef == stru_8D3998) {
        bIsAR6051 = TRUE;
    }

    m_objectType = TYPE_CONTAINER;

    if (pContainerObject->m_containerType == 4
        && pContainerObject->m_boundingRectLeft == 0
        && pContainerObject->m_boundingRectTop == 0
        && pContainerObject->m_boundingRectRight == 0
        && pContainerObject->m_boundingRectBottom == 0) {
        pContainerObject->m_boundingRectLeft = CPathSearch::GRID_SQUARE_SIZEX * (pContainerObject->m_posX / CPathSearch::GRID_SQUARE_SIZEX);
        pContainerObject->m_boundingRectRight = CPathSearch::GRID_SQUARE_SIZEX * (pContainerObject->m_posX / CPathSearch::GRID_SQUARE_SIZEX + 1) - 1;
        pContainerObject->m_boundingRectTop = CPathSearch::GRID_SQUARE_SIZEY * (pContainerObject->m_posY / CPathSearch::GRID_SQUARE_SIZEY);
        pContainerObject->m_boundingRectBottom = CPathSearch::GRID_SQUARE_SIZEY * (pContainerObject->m_posY / CPathSearch::GRID_SQUARE_SIZEY + 1) - 1;
    }

    m_rBounding.left = pContainerObject->m_boundingRectLeft;
    m_rBounding.top = pContainerObject->m_boundingRectTop;
    m_rBounding.right = pContainerObject->m_boundingRectRight + 1;
    m_rBounding.bottom = pContainerObject->m_boundingRectBottom + 1;
    m_nPolygon = pContainerObject->m_pickPointCount;

    memcpy(m_scriptRes, pContainerObject->m_script, RESREF_SIZE);

    strncpy(m_scriptName, pContainerObject->m_scriptName, SCRIPTNAME_SIZE);

    CAIScript* pScript = new CAIScript(CResRef(pContainerObject->m_script));
    SetScript(0, pScript);

    m_lockDifficulty = pContainerObject->m_lockDifficulty;
    m_dwFlags = pContainerObject->m_dwFlags;
    m_trapDetectionDifficulty = pContainerObject->m_trapDetectionDifficulty;
    m_trapRemovalDifficulty = pContainerObject->m_trapRemovalDifficulty;
    m_trapActivated = pContainerObject->m_trapActivated;
    m_trapDetected = pContainerObject->m_trapDetected;
    m_posTrapOrigin.x = pContainerObject->m_posXTrapOrigin;
    m_posTrapOrigin.y = pContainerObject->m_posYTrapOrigin;
    m_triggerRange = pContainerObject->m_triggerRange;
    strncpy(m_ownedBy, pContainerObject->m_ownedBy, SCRIPTNAME_SIZE);
    m_keyType = pContainerObject->m_keyType;
    m_breakDifficulty = pContainerObject->m_breakDifficulty;
    m_drawPoly = 0;

    if (m_nPolygon != 0) {
        m_pPolygon = new CPoint[m_nPolygon];

        if (m_pPolygon == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
            // __LINE__: 169
            UTIL_ASSERT(FALSE);

            // 0x47C371
            // NOTE: The original jumps past the copy loop; with no points
            // the loop below does nothing.
            m_nPolygon = 0;
        }

        WORD adjust = 0;
        for (WORD cnt = 0; cnt < m_nPolygon; cnt++) {
            m_pPolygon[cnt - adjust].x = pPoints[cnt + pContainerObject->m_pickPointStart].m_xPos;
            m_pPolygon[cnt - adjust].y = pPoints[cnt + pContainerObject->m_pickPointStart].m_yPos;
            if (cnt >= 2) {
                int x2 = m_pPolygon[cnt - adjust - 2].x;
                int y2 = m_pPolygon[cnt - adjust - 2].y;

                int x1 = m_pPolygon[cnt - adjust - 1].x;
                int y1 = m_pPolygon[cnt - adjust - 1].y;

                int x0 = m_pPolygon[cnt - adjust].x;
                int y0 = m_pPolygon[cnt - adjust].y;

                if ((x2 == x1 && x1 == x0)
                    || (y2 == y1 && y1 == y0)
                    || (x2 != x1
                        && x1 != x0
                        && 1000 * (y2 - y1) / (x2 - x1) == 1000 * (y1 - y0) / (x1 - x0))) {
                    m_pPolygon[cnt - adjust - 1] = m_pPolygon[cnt - adjust];
                    adjust++;
                }
            }
        }

        m_nPolygon -= adjust;
    } else {
        m_pPolygon = NULL;
    }

    for (DWORD nItem = pContainerObject->m_startingItem; nItem < pContainerObject->m_startingItem + pContainerObject->m_itemCount; nItem++) {
        CItem* pItem = new CItem(CResRef(pItems[nItem].m_itemId),
            pItems[nItem].m_usageCount[0],
            pItems[nItem].m_usageCount[1],
            pItems[nItem].m_usageCount[2],
            pItems[nItem].m_wear,
            pItems[nItem].m_dynamicFlags);

        if (pItem->cResRef != "") {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
            // __LINE__: 225
            m_lstItems.AddTail(pItem);
        } else {
            delete pItem;
        }
    }

    m_containerType = pContainerObject->m_containerType;

    if (m_containerType == 4) {
        m_ptWalkToUse.x = (pContainerObject->m_boundingRectLeft + pContainerObject->m_boundingRectRight) / 2;
        m_ptWalkToUse.y = (pContainerObject->m_boundingRectTop + pContainerObject->m_boundingRectBottom) / 2;
        RefreshRenderPile();
    } else {
        m_ptWalkToUse.x = pContainerObject->m_posX;
        m_ptWalkToUse.y = pContainerObject->m_posY;
    }

    m_pArea = pArea;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id,
        this,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        m_pArea = NULL;

        m_typeAI.m_nInstance = m_id;
        AddToArea(pArea, m_ptWalkToUse, 0, LIST_BACK);
        m_bDeleteMe = FALSE;

        if (bIsAR6051 == TRUE) {
            SHORT nSlotNum = 0;
            POSITION pos = m_lstItems.GetHeadPosition();
            while (pos != NULL) {
                CItem* pItem = m_lstItems.GetNext(pos);
                if (pItem != NULL) {
                    sub_480480(nSlotNum, pItem);
                }
                nSlotNum++;
            }
        }

        CVariable name;
        CString sScriptName(m_scriptName);

        m_typeAI.SetName(CString(m_scriptName));

        sScriptName.TrimLeft();
        if (sScriptName != "") {
            name.SetName(CString(m_scriptName));
            name.m_intValue = m_id;
            pArea->GetNamedCreatures()->AddKey(name);
        }

        sub_481890(&m_rBounding, field_8D6);

        field_8EA = NULL;

        if (m_nPolygon != 0) {
            field_8EA = new CAreaPoint[m_nPolygon];

            for (WORD cnt = 0; cnt < m_nPolygon; cnt++) {
                field_8EA[cnt].m_xPos = static_cast<WORD>(m_pPolygon[cnt].x);
                field_8EA[cnt].m_yPos = static_cast<WORD>(m_pPolygon[cnt].y);
            }
        } else {
            field_8EA = NULL;
        }

        field_8D2 = 0;
    } else {
        delete this;
    }
}

// 0x47C890
CGameContainer::CGameContainer(CGameArea* pArea, const CRect& rBound)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 322
    UTIL_ASSERT(pArea != NULL);

    m_objectType = TYPE_CONTAINER;
    m_rBounding = rBound;
    m_nPolygon = 0;
    m_pPolygon = NULL;
    m_containerType = 4;
    m_ptWalkToUse.x = (rBound.left + rBound.right) / 2;
    m_ptWalkToUse.y = (rBound.top + rBound.bottom) / 2;
    m_posTrapOrigin.x = 0;
    m_posTrapOrigin.y = 0;
    m_nPileVidCell = 0;
    m_lockDifficulty = 0;
    m_dwFlags = 0;
    m_trapDetectionDifficulty = 0;
    m_trapRemovalDifficulty = 0;
    m_trapActivated = 0;
    m_trapDetected = 0;
    m_posTrapOrigin.y = 0;
    m_triggerRange = 0;
    memset(m_ownedBy, 0, SCRIPTNAME_SIZE);
    memset(m_scriptRes, 0, RESREF_SIZE);
    memset(m_scriptName, 0, SCRIPTNAME_SIZE);
    m_keyType = "";
    m_breakDifficulty = 0;
    m_drawPoly = 0;

    m_pArea = pArea;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id,
        this,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        m_pArea = NULL;

        AddToArea(pArea, m_ptWalkToUse, 0, LIST_BACK);
        m_bDeleteMe = FALSE;
        field_8EA = NULL;
        field_8D2 = 0;
    } else {
        delete this;
    }
}

// 0x47CAC0
CGameContainer::~CGameContainer()
{
    POSITION pos = m_lstItems.GetHeadPosition();
    while (pos != NULL) {
        CItem* pItem = m_lstItems.GetNext(pos);
        if (pItem != NULL) {
            delete pItem;
        }
    }
    m_lstItems.RemoveAll();

    if (m_pPolygon != NULL) {
        delete m_pPolygon;
    }

    if (field_8EA != NULL) {
        delete field_8EA;
        field_8EA = NULL;
    }

    // When there is only one element its an unowned pointer to `m_rBounding`.
    if (field_8D6.GetCount() > 1) {
        for (INT nIndex = 0; nIndex < field_8D6.GetCount(); nIndex++) {
            delete field_8D6[nIndex];
        }
    }
    field_8D6.SetSize(0);
}

// 0x47CBE0
void CGameContainer::AddEffect(CGameEffect* pEffect, BYTE list, BOOL noSave, BOOL immediateApply)
{
    BOOLEAN bSendStatus = FALSE;

    if (pEffect->m_effectID == CGAMEEFFECT_KNOCK) {
        if (m_lockDifficulty != 100) {
            m_dwFlags &= ~0x1;

            CMessage* message = new CMessageContainerStatus(m_dwFlags,
                m_trapActivated,
                m_trapDetected,
                m_id,
                m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else {
            // NOTE: Original reads marker `-1` from a constant at 0x84C960.
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                23169,
                RGB(0, 0, 0),
                RGB(160, 200, 215),
                -1,
                m_id,
                m_id);
        }
    } else if (pEffect->m_effectID == CGAMEEFFECT_DETECTTRAPS) {
        if (m_trapActivated != 0) {
            if (m_trapDetected == 0
                && m_trapDetectionDifficulty < 100
                && (m_dwFlags & 0x8) != 0) {
                // NOTE: Original reads trigger ID from a constant at
                // 0x847E40.
                CAITrigger trigger(CAITRIGGER_DETECTED, m_typeAI, 0);

                CMessage* message = new CMessageSetTrigger(trigger, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                m_trapDetected = TRUE;

                message = new CMessageTriggerStatus(m_dwFlags,
                    m_trapActivated,
                    m_trapDetected,
                    m_id,
                    m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                bSendStatus = TRUE;

                // NOTE: Original passes `m_id` where the callee expects a
                // `CGameAIBase*` caller.
                CGameObject* pObject = m_typeAI.sub_40CCA0(reinterpret_cast<CGameAIBase*>(static_cast<INT_PTR>(m_id)), FALSE);
                if (pObject != NULL) {
                    // NOTE: Uninline.
                    if (pObject->GetAIType().Equal(CAIObjectType(CGameObject::TYPE_SPRITE))) {
                        static_cast<CGameAIBase*>(pObject)->AutoPause(0x80);
                    }
                }
            }

            if (m_trapDetected != 0) {
                // NOTE: Uninline.
                SetDrawPoly(400);
            }

            if (bSendStatus == TRUE) {
                CMessage* message = new CMessageContainerStatus(m_dwFlags,
                    m_trapActivated,
                    m_trapDetected,
                    m_id,
                    m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    } else if (pEffect->m_effectID == CGAMEEFFECT_CASTSPELL) {
        CAIObjectType type(GetAIType());
        CAIAction action(CAIAction::FORCESPELL, type, 0, 0, 0);

        CString sResRef;
        pEffect->m_res.CopyToString(sResRef);
        action.SetString1(sResRef);
        action.m_specificID = pEffect->m_effectAmount;

        CMessage* message = new CMessageInsertAction(action, m_id, pEffect->m_sourceID);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    } else if (pEffect->m_effectID == CGAMEEFFECT_SUMMON) {
        ITEM_EFFECT* effect = new ITEM_EFFECT;
        CGameEffect::ClearItemEffect(effect, CGAMEEFFECT_SKILLUNSUMMON);

        // NOTE: Original reads duration type 4 from a constant at 0x8491B0.
        effect->durationType = 4;
        effect->duration = (pEffect->m_duration - g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime) / 15;
        effect->effectAmount = 1;

        CGameEffect* pNewEffect = CGameEffect::DecodeEffect(effect,
            CPoint(-1, -1),
            -1,
            CPoint(-1, -1));

        delete effect;

        CPoint ptGrid(pEffect->m_target.x / CPathSearch::GRID_SQUARE_SIZEX,
            pEffect->m_target.y / CPathSearch::GRID_SQUARE_SIZEY);

        CCreatureFile creatureData;

        // NOTE: Uninline.
        creatureData.SetResRef(pEffect->m_res, TRUE, TRUE);

        CGameSprite* pSprite = new CGameSprite(creatureData.GetData(),
            creatureData.GetDataSize(),
            0,
            0,
            -1,
            0,
            0,
            0x7FFFFFFF,
            CPoint(-1, -1),
            0);

        pSprite->SetResRef(creatureData.GetResRef().GetResRefStr());
        creatureData.ReleaseData();

        if (pSprite != NULL) {
            LONG nSpriteId = pSprite->GetId();

            CString sResRef;
            pEffect->m_res.CopyToString(sResRef);
            pSprite->SetResRef(sResRef);

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nSpriteId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                CPoint ptOpen = m_pArea->m_search.GetNearestOpenSquare(ptGrid,
                    GetTerrainTable(),
                    pSprite->m_animation.GetPersonalSpace(),
                    -1);

                pSprite->AddToArea(m_pArea,
                    CPoint(ptOpen.x * CPathSearch::GRID_SQUARE_SIZEX, ptOpen.y * CPathSearch::GRID_SQUARE_SIZEY),
                    0,
                    pSprite->m_animation.GetListType());

                nSpriteId = pSprite->GetId();

                CProjectile* pProjectile = CProjectile::DecodeProjectile(144, pSprite, FALSE);

                CMessage* message = new CMessageFireProjectile(pProjectile->m_projectileType,
                    nSpriteId,
                    CPoint(0, 0),
                    30,
                    nSpriteId,
                    nSpriteId,
                    0);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                pProjectile->Fire(m_pArea, nSpriteId, nSpriteId, CPoint(0, 0), 0, 0);

                // NOTE: Unused.
                CAIObjectType typeAI(pSprite->GetAIType());
                CAIObjectType liveTypeAI(pSprite->m_liveTypeAI);
                CAIObjectType liveTypeAI2(pSprite->m_liveTypeAI);

                // NOTE: Original reads the permanent duration type from a
                // constant at 0x8491AA.
                if (pEffect->m_durationType != 1) {
                    pNewEffect->m_casterLevel = pEffect->m_casterLevel;
                    pSprite->AddEffect(pNewEffect, CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
                }
                // NOTE: Otherwise `pNewEffect` leaks.

                pSprite->SetSequence(CGameSprite::SEQ_AWAKE);
                pSprite->SetFacing(rand() % 16);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nSpriteId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }

    if (pEffect != NULL) {
        delete pEffect;
    }
}

// 0x47C830
BOOLEAN CGameContainer::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x47D7F0
BOOLEAN CGameContainer::DoAIUpdate(BOOLEAN active, LONG counter)
{
    return active && (counter & m_AISpeed) == (m_AISpeed & m_id);
}

// 0x47D820
void CGameContainer::AIUpdate()
{
    if (m_bDeleteMe) {
        RemoveFromArea();
        return;
    }

    if (m_drawPoly > 0) {
        m_drawPoly--;
    }

    ProcessAI();
}

// 0x47D850
BOOLEAN CGameContainer::CompressTime(DWORD deltaTime)
{
    BOOLEAN bEmpty = TRUE;

    // NOTE: Unsigned compare.
    if (deltaTime > static_cast<DWORD>(m_drawPoly)) {
        m_drawPoly = 0;
    } else {
        m_drawPoly = static_cast<SHORT>(m_drawPoly - deltaTime);
    }

    if (deltaTime > CTimerWorld::TIMESCALE_MSEC_PER_DAY && m_containerType == 4) {
        POSITION posCurrent = m_lstItems.GetHeadPosition();
        POSITION pos = posCurrent;
        while (pos != NULL) {
            CItem* pItem = m_lstItems.GetNext(pos);
            if (pItem == NULL) {
                m_lstItems.RemoveAt(posCurrent);
            } else {
                if (pItem->GetItemType() == 58) {
                    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
                    CItem tempItem;

                    if (g_pChitin->cNetwork.GetSessionOpen()) {
                        BOOL bStoreDemanded = FALSE;
                        CStore cStore;

                        if (g_pChitin->cNetwork.GetSessionHosting()) {
                            pGame->DemandServerStore(pItem->GetResRef(), TRUE);
                            cStore.SetResRef(pItem->GetResRef());
                        } else {
                            cStore.SetResRef(pItem->GetResRef());
                            if (!cStore.m_bLocalCopy || memcmp(cStore.m_pVersion, "STORV9.0", 8) != 0) {
                                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(pItem->GetResRef().GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                                    g_pChitin->cNetwork.CloseSession(TRUE);

                                    // NOTE: Skips the `posCurrent = pos`
                                    // update at the end of the loop, so the
                                    // next iteration removes from a stale
                                    // position.
                                    continue;
                                }

                                cStore.SetResRef(pItem->GetResRef());
                                bStoreDemanded = TRUE;
                            }
                        }

                        if (cStore.GetType() == 4) {
                            if (!g_pChitin->cNetwork.GetSessionHosting() && !bStoreDemanded) {
                                bStoreDemanded = TRUE;

                                CMessage* message = new CMessageStoreDemand(pItem->GetResRef(),
                                    CGameObjectArray::INVALID_INDEX,
                                    CGameObjectArray::INVALID_INDEX);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                            }

                            for (INT nIndex = 0; nIndex < cStore.m_lInventory.GetCount(); nIndex++) {
                                cStore.GetItem(nIndex, tempItem);

                                if ((tempItem.GetFlagsFile() & 0x1) != 0) {
                                    CMessage* message = new CMessageContainerAddItem(tempItem,
                                        0x7FFF,
                                        FALSE,
                                        m_id,
                                        m_id);
                                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                                    bEmpty = FALSE;
                                }

                                // NOTE: Original passes the bag's resref as
                                // both the store and the item, and the bag's
                                // flags.
                                CMessage* message = new CMessageStoreRemoveItem(pItem->GetResRef(),
                                    pItem->GetResRef(),
                                    nIndex,
                                    1,
                                    pItem->m_flags,
                                    m_id,
                                    m_id);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                            }

                            if (!g_pChitin->cNetwork.GetSessionHosting()
                                && g_pChitin->cNetwork.GetSessionOpen()) {
                                CStore::InvalidateStore(pItem->GetResRef());
                            }
                        }

                        if (g_pChitin->cNetwork.GetSessionHosting()) {
                            g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(cStore.m_resRef);
                        } else {
                            if (bStoreDemanded) {
                                CMessage* message = new CMessageStoreRelease(cStore.m_resRef,
                                    CGameObjectArray::INVALID_INDEX,
                                    CGameObjectArray::INVALID_INDEX);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                            }
                        }
                    } else {
                        CStore cStore(pItem->GetResRef());

                        if (cStore.GetType() == 4) {
                            for (INT nIndex = 0; nIndex < cStore.m_lInventory.GetCount(); nIndex++) {
                                cStore.GetItem(nIndex, tempItem);

                                if ((tempItem.GetFlagsFile() & 0x1) != 0) {
                                    // NOTE: `CItem::operator=` is inlined
                                    // into this copy of the constructor.
                                    CMessage* message = new CMessageContainerAddItem(tempItem,
                                        0x7FFF,
                                        FALSE,
                                        m_id,
                                        m_id);
                                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                                    bEmpty = FALSE;
                                }

                                cStore.RemoveItemExt(tempItem.GetResRef(), tempItem.m_flags, -1, 1, NULL);
                            }

                            cStore.Marshal(pGame->m_sTempDir);
                        }
                    }
                }

                if ((pItem->GetFlagsFile() & 0x1) == 0) {
                    m_lstItems.RemoveAt(posCurrent);
                    delete pItem;
                } else {
                    bEmpty = FALSE;
                }
            }

            posCurrent = pos;
        }

        RefreshRenderPile();

        if (bEmpty) {
            m_bDeleteMe = TRUE;
            return FALSE;
        }
    }

    return TRUE;
}

// 0x47E040
void CGameContainer::DebugDump(const CString& message, BOOLEAN bEchoToScreen)
{
    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

    CString sTemp;
    CString sType;

    switch (m_containerType) {
    case 1:
        sType = "BAG";
        break;
    case 2:
        sType = "CHEST";
        break;
    case 3:
        sType = "DRAWER";
        break;
    case 4:
        sType = "PILE";
        break;
    case 5:
        sType = "TABLE";
        break;
    case 6:
        sType = "SHELF";
        break;
    case 7:
        sType = "ALTAR";
        break;
    case 8:
        sType = "NONVISIBLE";
        break;
    case 9:
        sType = "SPELLBOOK";
        break;
    case 10:
        sType = "BODY";
        break;
    case 11:
        sType = "BARREL";
        break;
    case 12:
        sType = "CRATE";
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
        // __LINE__: 899
        UTIL_ASSERT(FALSE);
    }

    if (bEchoToScreen) {
        pWorld->DisplayText(CString(""),
            CString("DEBUG DUMP: CGameContainer"),
            -1,
            FALSE);

        pWorld->DisplayText(CString(""),
            message,
            -1,
            FALSE);

        sTemp.Format("Current Area: %.*s", RESREF_SIZE, m_pArea->m_resRef.GetResRef());
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Current Position: x=%d y=%d", m_pos.x, m_pos.y);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Container Type: %s", (LPCSTR)sType);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Locked: %s\n", (m_dwFlags & 0x1) != 0 ? "TRUE" : "FALSE");
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Lock Difficulty: %d%%\n", m_lockDifficulty);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Key Type: %.*s", RESREF_SIZE, m_keyType.GetResRef());
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trapped: %s", m_trapActivated != 0 ? "TRUE" : "FALSE");
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Detected: %s", m_trapDetected != 0 ? "TRUE" : "FALSE");
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Reset Trap: %s", (m_dwFlags & 0x8) != 0 ? "TRUE" : "FALSE");
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Detection Difficulty: %d%%", m_trapDetectionDifficulty);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Removal Difficulty: %d%%", m_trapRemovalDifficulty);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Script: %.*s", RESREF_SIZE, m_scriptRes);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Launching Point: x=%d y=%d", m_posTrapOrigin.x, m_posTrapOrigin.y);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        pWorld->DisplayText(CString(""),
            CString("Contents:"),
            -1,
            FALSE);

        POSITION pos = m_lstItems.GetHeadPosition();
        while (pos != NULL) {
            CItem* pItem = m_lstItems.GetNext(pos);
            if (pItem != NULL) {
                if ((pItem->m_flags & 0x1) != 0) {
                    sTemp.Format("    %.*s, %d, %d, %d, IDENTIFIED\n",
                        RESREF_SIZE,
                        pItem->GetResRef().GetResRef(),
                        pItem->GetUsageCount(0),
                        pItem->GetUsageCount(1),
                        pItem->GetUsageCount(2));
                    pWorld->DisplayText(CString(""),
                        sTemp,
                        -1,
                        FALSE);
                } else {
                    sTemp.Format("    %.*s, %d, %d, %d\n",
                        RESREF_SIZE,
                        pItem->GetResRef().GetResRef(),
                        pItem->GetUsageCount(0),
                        pItem->GetUsageCount(1),
                        pItem->GetUsageCount(2));
                    pWorld->DisplayText(CString(""),
                        sTemp,
                        -1,
                        FALSE);
                }
            }
        }
    }
}

// 0x47E8A0
BOOL CGameContainer::IsOver(const CPoint& pt)
{
    if (!m_rBounding.PtInRect(pt)) {
        return FALSE;
    }

    if (m_containerType == 4) {
        if (m_lstItems.IsEmpty()) {
            return FALSE;
        }

        SHORT cnt = 0;
        POSITION pos = m_lstItems.GetHeadPosition();
        while (cnt == 0 && pos != NULL) {
            CItem* pItem = m_lstItems.GetNext(pos);
            if (pItem != NULL) {
                cnt++;
            }
        }

        if (cnt == 0) {
            return FALSE;
        }
    }

    if (!g_pBaldurChitin->GetObjectGame()->GetGroup()->IsPartyLeader()) {
        return FALSE;
    }

    if (m_pPolygon != NULL) {
        return CVidPoly::IsPtInPoly(m_pPolygon, m_nPolygon, pt);
    }

    return TRUE;
}

// 0x47E970
void CGameContainer::OnActionButton(const CPoint& pt)
{
    if (g_pBaldurChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        CMessage* message = new CMessageContainerAddItem(CItem(), 0x7FFF, FALSE, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CAIGroup* pGroup = pGame->GetGroup();

    if (m_containerType == 4 && m_lstItems.GetCount() == 0) {
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        CGameObject::OnActionButton(pt);
        return;
    }

    switch (pGame->GetState()) {
    case 0:
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        if (pGroup->GetCount() > 0) {
            // NOTE: Original reads action ID 112 from a constant at 0x847854.
            CAIAction action(112, m_id, 0, 0);
            pGroup->GroupAction(CAIAction(), TRUE, &action);

            CAIAction moveAction(CAIAction::MOVETOPOINT, m_ptWalkToUse, 0, -1);
            pGroup->GroupAction(CAIAction(), FALSE, &moveAction);
        }
        break;
    case 2:
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        switch (pGame->GetIconIndex()) {
        case 12:
            if ((m_dwFlags & 0x1) == 0) {
                CGameObject::OnActionButton(pt);
            } else {
                // NOTE: Original reads action ID 148 from a constant at
                // 0x84789C.
                CAIAction action(148, m_typeAI, 0, 0, 0);
                pGroup->GroupAction(CAIAction::NULL_ACTION, TRUE, &action);

                pGame->SetState(0);
                pGame->GetButtonArray()->SetSelectedButton(100);
                pGame->GetButtonArray()->UpdateState();
            }
            break;
        case 18:
        case 40:
        case 255:
            CGameObject::OnActionButton(pt);
            break;
        case 20:
            pGame->UseMagicOnObject(m_id);

            pGame->SetState(0);
            pGame->GetButtonArray()->SetSelectedButton(100);
            pGame->GetButtonArray()->UpdateState();
            break;
        case 36:
            if (m_trapActivated != 0 && m_trapDetected != 0) {
                SHORT nPortrait = g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter();

                // NOTE: Uninline.
                LONG nCharacterId = pGame->GetCharacterId(nPortrait);

                CGameSprite* pSprite;

                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    BYTE nDisableDevice = pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE];

                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    if (nDisableDevice == 0) {
                        break;
                    }
                }

                // NOTE: Original reads action ID 28 from a constant at
                // 0x8477BA.
                CAIAction action(28, m_typeAI, 0, 0, 0);
                pGroup->GroupAction(action, TRUE, NULL);
            } else {
                if ((m_dwFlags & 0x1) == 0) {
                    CGameObject::OnActionButton(pt);
                    break;
                }

                SHORT nPortrait = g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter();

                // NOTE: Uninline.
                LONG nCharacterId = pGame->GetCharacterId(nPortrait);

                CGameSprite* pSprite;

                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    BYTE nOpenLock = pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK];

                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    if (nOpenLock == 0) {
                        break;
                    }
                }

                // NOTE: Original reads action ID 145 from a constant at
                // 0x847896.
                CAIAction action(145, m_typeAI, 0, 0, 0);
                pGroup->GroupAction(action, TRUE, NULL);
            }

            pGame->SetState(0);
            pGame->GetButtonArray()->SetSelectedButton(100);
            pGame->GetButtonArray()->UpdateState();
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
            // __LINE__: 1215
            UTIL_ASSERT(FALSE);
        }
        break;
    case 3:
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        CGameObject::OnActionButton(pt);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
        // __LINE__: 1226
        UTIL_ASSERT(FALSE);
    }
}

// 0x47F580
void CGameContainer::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // NOTE: Original reads the outline colors from globals at 0x8A85F8
    // (RGB(32, 64, 160)), 0x8A85FC (RGB(0, 250, 0)) and 0x8A8608
    // (RGB(255, 0, 0)).
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    SHORT nState = pGame->GetState();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 1267
    UTIL_ASSERT(pVidMode != NULL);

    BOOL bRendered = FALSE;
    BOOLEAN bMenuKey = FALSE;

    if (m_containerType == 4) {
        // NOTE: Uninline.
        if (m_nPileVidCell > 0
            && m_pArea->m_visibility.IsTileExplored(m_pArea->m_visibility.PointToTile(m_pos))) {
            DWORD dwFlags = CInfinity::FXPREP_CLEARFILL | 0x30001;
            CInfinity* pInfinity = m_pArea->GetInfinity();
            CRect rFX(0, 0, 0, 0);
            CPoint ptReference(0, 0);
            BOOLEAN bFogged = FALSE;
            INT nIndex;

            for (nIndex = m_nPileVidCell; nIndex > 0; nIndex--) {
                CPoint ptCenter;
                m_pileVidCell[nIndex - 1].GetCurrentCenterPoint(ptCenter, FALSE);

                CSize frameSize;
                m_pileVidCell[nIndex - 1].GetCurrentFrameSize(frameSize, FALSE);

                CRect rFrame;
                rFrame.SetRect(0, 0, frameSize.cx, frameSize.cy);

                if (ptCenter.x > ptReference.x) {
                    rFX.right += ptCenter.x - ptReference.x;
                    ptReference.x = ptCenter.x;
                }

                if (rFrame.right > rFX.right) {
                    rFX.right = rFrame.right;
                }

                if (ptCenter.y > ptReference.y) {
                    rFX.bottom += ptCenter.y - ptReference.y;
                    ptReference.y = ptCenter.y;
                }

                if (rFrame.bottom > rFX.bottom) {
                    rFX.bottom = rFrame.bottom;
                }
            }

            CPoint ptPos;
            ptPos.x = m_pos.x;
            ptPos.y = m_pArea->GetHeightOffset(m_pos, m_listType) + m_pos.y;

            CRect rGCBounds;
            rGCBounds.left = ptPos.x - ptReference.x;
            rGCBounds.top = m_posZ - ptReference.y + ptPos.y;
            rGCBounds.right = rFX.right - rFX.left + rGCBounds.left;
            rGCBounds.bottom = rFX.bottom - rFX.top + rGCBounds.top;

            CRect rViewPort;
            rViewPort.left = pInfinity->nCurrentX;
            rViewPort.top = pInfinity->nCurrentY;
            rViewPort.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + rViewPort.left;
            rViewPort.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + rViewPort.top;

            if (rViewPort.IntersectRect(rGCBounds, rViewPort)) {
                COLORREF rgbTint = m_pArea->GetTintColor(CPoint(ptPos.x, m_posZ + ptPos.y), m_listType);
                BOOLEAN bDithered = m_id == m_pArea->m_iPicked;

                // NOTE: Uninline.
                if (!m_pArea->m_visibility.IsTileVisible(m_pArea->m_visibility.PointToTile(m_pos))) {
                    if (g_pChitin->cVideo.m_bIs3dAccelerated) {
                        // NOTE: Halves each channel.
                        rgbTint = RGB(GetRValue(rgbTint) >> 1,
                            GetGValue(rgbTint) >> 1,
                            GetBValue(rgbTint) >> 1);
                    } else {
                        bFogged = TRUE;
                    }
                }

                pInfinity->FXPrep(rFX, dwFlags, a3, ptPos, ptReference);

                if (pInfinity->FXLock(rFX, dwFlags)) {
                    for (nIndex = m_nPileVidCell; nIndex > 0; nIndex--) {
                        if (m_id == m_pArea->m_iPicked
                            && pGame->GetGroup()->GetCount() > 0
                            && nState == 0) {
                            m_pileVidCell[nIndex - 1].m_bShadowOn = TRUE;
                        } else {
                            m_pileVidCell[nIndex - 1].m_bShadowOn = FALSE;
                        }

                        if (g_pBaldurChitin->pActiveEngine == g_pBaldurChitin->m_pEngineWorld
                            && g_pBaldurChitin->m_pEngineWorld->GetMenuKey() == TRUE) {
                            m_pileVidCell[nIndex - 1].m_bShadowOn = TRUE;
                            bMenuKey = TRUE;
                        }

                        m_pileVidCell[nIndex - 1].SetTintColor(rgbTint);
                        pInfinity->FXRender(&(m_pileVidCell[nIndex - 1]),
                            ptReference.x,
                            ptReference.y,
                            dwFlags,
                            0);
                    }

                    CRect rClip;
                    rClip.left = rGCBounds.left;
                    rClip.top = rGCBounds.top - m_posZ;
                    rClip.right = rGCBounds.right;
                    rClip.bottom = rGCBounds.bottom - m_posZ;

                    if (!bMenuKey) {
                        pInfinity->FXRenderClippingPolys(ptPos.x,
                            ptPos.y - m_posZ,
                            m_posZ,
                            ptReference,
                            rClip,
                            bDithered,
                            dwFlags);
                    }

                    if (bFogged) {
                        pInfinity->FXUnlock(dwFlags,
                            &rFX,
                            CPoint(ptPos.x + ptReference.x, ptPos.y + ptReference.y));
                    } else {
                        pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
                    }

                    pInfinity->FXBltFrom(a3,
                        rFX,
                        ptPos.x,
                        ptPos.y,
                        ptReference.x,
                        ptReference.y,
                        dwFlags);
                }
            }
        }
    } else {
        if (m_id == m_pArea->m_iPicked) {
            // NOTE: Uninline.
            if (m_pArea->m_visibility.IsTileExplored(m_pArea->m_visibility.PointToTile(m_pos))) {
                if (pGame->GetGroup()->GetCount() > 0 && nState == 0) {
                    if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                        sub_481A10(pArea,
                            pVidMode,
                            a3,
                            g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(32, 64, 160)));
                        bRendered = TRUE;
                    }

                    m_pArea->m_cInfinity.OutlinePoly(m_pPolygon,
                        m_nPolygon,
                        m_rBounding,
                        RGB(32, 64, 160));
                }

                if (pGame->GetState() == 2) {
                    if (pGame->GetIconIndex() == 36) {
                        if ((m_trapActivated != 0 && m_trapDetected != 0)
                            || (m_dwFlags & 0x1) != 0) {
                            if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                                sub_481A10(pArea,
                                    pVidMode,
                                    a3,
                                    g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(0, 250, 0)));
                                bRendered = TRUE;
                            }

                            m_pArea->m_cInfinity.OutlinePoly(m_pPolygon,
                                m_nPolygon,
                                m_rBounding,
                                RGB(0, 250, 0));
                        }
                    } else if (pGame->GetIconIndex() == 12) {
                        if ((m_dwFlags & 0x1) != 0) {
                            if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                                sub_481A10(pArea,
                                    pVidMode,
                                    a3,
                                    g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(0, 250, 0)));
                                bRendered = TRUE;
                            }

                            m_pArea->m_cInfinity.OutlinePoly(m_pPolygon,
                                m_nPolygon,
                                m_rBounding,
                                RGB(0, 250, 0));
                        }
                    }
                }
            }
        } else {
            // NOTE: Uninline.
            if (m_pArea->m_visibility.IsTileExplored(m_pArea->m_visibility.PointToTile(m_pos))
                && g_pBaldurChitin->pActiveEngine == g_pBaldurChitin->m_pEngineWorld
                && g_pBaldurChitin->m_pEngineWorld->GetMenuKey() == TRUE) {
                if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                    sub_481A10(pArea,
                        pVidMode,
                        a3,
                        g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(0, 250, 0)));
                    bRendered = TRUE;
                }

                m_pArea->m_cInfinity.OutlinePoly(m_pPolygon,
                    m_nPolygon,
                    m_rBounding,
                    RGB(0, 250, 0));
            }
        }
    }

    if (m_drawPoly > 0) {
        if (CInfinity::TRANSLUCENT_BLTS_ON
            && !g_pChitin->cVideo.m_bIs3dAccelerated
            && !bRendered) {
            sub_481A10(pArea,
                pVidMode,
                a3,
                g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(255, 0, 0)));
        }

        m_pArea->m_cInfinity.OutlinePoly(m_pPolygon,
            m_nPolygon,
            m_rBounding,
            RGB(255, 0, 0));
    }
}

// 0x47FE10
void CGameContainer::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
        // __LINE__: 1597
        UTIL_ASSERT(FALSE);
        return;
    }

    delete this;
}

// 0x47FE70
void CGameContainer::SetCursor(LONG nToolTip)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    switch (pGame->GetState()) {
    case 0:
        if (pGame->GetGroup()->GetCount() != 0) {
            g_pBaldurChitin->GetObjectCursor()->SetCursor(2, FALSE);
        } else {
            g_pBaldurChitin->GetObjectCursor()->SetCursor(0, FALSE);
        }
        break;
    case 1:
    case 3:
        CGameObject::SetCursor(nToolTip);
        break;
    case 2:
        switch (pGame->GetIconIndex()) {
        case 12:
            if ((m_dwFlags & 0x1) != 0) {
                g_pBaldurChitin->GetObjectCursor()->SetCursor(12, FALSE);
            } else {
                CGameObject::SetCursor(nToolTip);
            }
            break;
        case 18:
        case 40:
        case 255:
            CGameObject::SetCursor(nToolTip);
            break;
        case 20:
            g_pBaldurChitin->GetObjectCursor()->SetCursor(20, FALSE);
            break;
        case 36:
            if (1) {
                INT nNewCursor;
                if (m_trapActivated != 0 && m_trapDetected != 0) {
                    nNewCursor = 38;
                } else {
                    if ((m_dwFlags & 0x1) == 0) {
                        CGameObject::SetCursor(nToolTip);
                        break;
                    }

                    nNewCursor = 26;
                }

                SHORT nPortrait = g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter();

                // NOTE: Uninline.
                LONG nCharacterId = pGame->GetCharacterId(nPortrait);

                CGameSprite* pSprite;

                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    if (nNewCursor == 26) {
                        if (pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] == 0) {
                            g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                        }
                    } else {
                        if (pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE] == 0) {
                            g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                        }
                    }

                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
            // __LINE__: 1694
            UTIL_ASSERT(FALSE);
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
        // __LINE__: 1704
        UTIL_ASSERT(FALSE);
    }
}

// 0x4801A0
void CGameContainer::CompressContainer()
{
    // NOTE: Uninline.
    if (InControl()) {
        POSITION pos = m_lstItems.GetHeadPosition();
        while (pos != NULL) {
            POSITION posOld = pos;
            CItem* pItem = m_lstItems.GetNext(pos);
            if (pItem == NULL) {
                m_lstItems.RemoveAt(posOld);
            }
        }

        RefreshRenderPile();

        g_pBaldurChitin->GetActiveEngine()->UpdateContainerStatus(m_id, -1);

        // NOTE: Uninline.
        if (InControl()) {
            if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
                CMessageContainerItems* pContainerItems = new CMessageContainerItems(this,
                    m_id,
                    m_id);

                g_pBaldurChitin->GetMessageHandler()->AddMessage(pContainerItems, FALSE);
            }
        }
    }
}

// 0x4802B0
CItem* CGameContainer::GetItem(SHORT nSlotNum)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 1791
    UTIL_ASSERT(nSlotNum >= 0);

    POSITION pos = m_lstItems.FindIndex(nSlotNum);
    if (pos != NULL) {
        return m_lstItems.GetAt(pos);
    }

    return NULL;
}

// 0x480300
void CGameContainer::SetItem(SHORT nSlotNum, CItem* pItem)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 1826
    UTIL_ASSERT(nSlotNum >= 0);

    // 0x8D3978
    static const CResRef RESREF_AR6051("AR6051");

    if (m_pArea->m_resRef == RESREF_AR6051) {
        sub_480480(nSlotNum, pItem);
    }

    POSITION pos = m_lstItems.FindIndex(nSlotNum);
    if (pos != NULL) {
        m_lstItems.SetAt(pos, pItem);
    } else {
        if (pItem == NULL) {
            return;
        }

        m_lstItems.AddTail(pItem);
    }

    RefreshRenderPile();

    g_pBaldurChitin->GetActiveEngine()->UpdateContainerStatus(m_id, nSlotNum);

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        // NOTE: Uninline.
        if (InControl()) {
            CMessageContainerItems* pContainerItems = new CMessageContainerItems(this,
                m_id,
                m_id);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pContainerItems, FALSE);
        }
    }
}

// 0x480480
void CGameContainer::sub_480480(SHORT nSlotNum, CItem* pItem)
{
    // 0x8D3968
    static CResRef stru_8D3968("60HEART");

    // 0x8D3990
    static CString str_8D3990("HCx");

    // 0x8D2558
    static int dword_8D2558 = str_8D3990.GetLength() - 1;

    if (pItem == NULL) {
        POSITION pos = m_lstItems.FindIndex(nSlotNum);
        if (pos != NULL) {
            CItem* pOldItem = m_lstItems.GetAt(pos);
            if (pOldItem != NULL) {
                BYTE resRef[RESREF_SIZE];
                pOldItem->cResRef.GetResRef(resRef);

                // NOTE: The last character is the heart number.
                BYTE nNumber = resRef[7];
                resRef[7] = 0;

                if (stru_8D3968 == CResRef(resRef)) {
                    CVariable var;

                    str_8D3990.SetAt(dword_8D2558, nNumber);

                    CNamedCreatureVariableHash* pNamedCreatures = &m_pArea->m_namedCreatures;

                    CVariable* pVar = pNamedCreatures->FindKey(str_8D3990);
                    if (pVar != NULL) {
                        if (pVar->m_intValue == m_id) {
                            pNamedCreatures->RemoveKey(str_8D3990, FALSE, 0);
                        }
                    }
                }
            }
        }
    } else {
        BYTE resRef[RESREF_SIZE];
        pItem->cResRef.GetResRef(resRef);

        // NOTE: The last character is the heart number.
        BYTE nNumber = resRef[7];
        resRef[7] = 0;

        if (CResRef(resRef) == stru_8D3968) {
            CVariable var;

            str_8D3990.SetAt(dword_8D2558, nNumber);

            CNamedCreatureVariableHash* pNamedCreatures = &m_pArea->m_namedCreatures;

            CVariable* pVar = pNamedCreatures->FindKey(str_8D3990);
            if (pVar != NULL) {
                pVar->m_intValue = m_id;
            } else {
                var.SetName(str_8D3990);
                var.m_intValue = m_id;
                pNamedCreatures->AddKey(var);
            }
        }
    }
}

// 0x480760
BOOLEAN CGameContainer::PlaceItemInBlankSlot(CItem* pItem, BOOLEAN bCompressContainer, SHORT nRecommendedSlotNum)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 1993
    UTIL_ASSERT(pItem != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 1995
    UTIL_ASSERT(nRecommendedSlotNum >= 0);

    CMessageContainerAddItem* pContainerAddItem = new CMessageContainerAddItem(*pItem,
        nRecommendedSlotNum,
        bCompressContainer,
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pContainerAddItem, FALSE);

    g_pBaldurChitin->GetObjectGame()->AddDisposableItem(pItem);

    return TRUE;
}

// 0x4808A0
void CGameContainer::Marshal(CAreaFileContainer** pContainerObject)
{
    CAreaPoint* pPoints;
    DWORD cnt;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 2028
    UTIL_ASSERT(pContainerObject != NULL);

    *pContainerObject = new CAreaFileContainer();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 2037
    UTIL_ASSERT(*pContainerObject != NULL);

    // FIXME: Redundant, memset is a part of constructor.
    memset(*pContainerObject, 0, sizeof(CAreaFileContainer));

    if (m_nPolygon > 0) {
        pPoints = new CAreaPoint[m_nPolygon];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
        // __LINE__: 2042
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nPolygon; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pPolygon[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pPolygon[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pContainerObject)->m_pickPointStart = reinterpret_cast<DWORD>(pPoints);
        (*pContainerObject)->m_pickPointCount = m_nPolygon;
    }

    if (m_lstItems.GetCount() > 0) {
        CCreatureFileItem* pItems;
        POSITION pos;

        cnt = 0;
        pos = m_lstItems.GetHeadPosition();
        while (pos != NULL) {
            if (m_lstItems.GetNext(pos) != NULL) {
                cnt++;
            }
        }

        if (cnt > 0) {
            pItems = new CCreatureFileItem[cnt];

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
            // __LINE__: 2064
            UTIL_ASSERT(pItems != NULL)

            // FIXME: Unsafe x64 conversion.
            (*pContainerObject)->m_startingItem = reinterpret_cast<DWORD>(pItems);
            (*pContainerObject)->m_itemCount = cnt;

            cnt = 0;
            pos = m_lstItems.GetHeadPosition();
            while (pos != NULL) {
                CItem* pItem = m_lstItems.GetNext(pos);
                if (pItem != NULL) {
                    pItem->GetResRef().GetResRef(pItems[cnt].m_itemId);
                    for (INT ability = 0; ability < 3; ability++) {
                        pItems[cnt].m_usageCount[ability] = pItem->GetUsageCount(ability);
                    }
                    pItems[cnt].m_wear = pItem->m_wear;
                    pItems[cnt].m_dynamicFlags = pItem->m_flags;
                    cnt++;
                }
            }
        }
    }

    (*pContainerObject)->m_boundingRectLeft = static_cast<WORD>(m_rBounding.left);
    (*pContainerObject)->m_boundingRectTop = static_cast<WORD>(m_rBounding.top);
    (*pContainerObject)->m_boundingRectRight = static_cast<WORD>(m_rBounding.right) - 1;
    (*pContainerObject)->m_boundingRectBottom = static_cast<WORD>(m_rBounding.bottom) - 1;
    (*pContainerObject)->m_containerType = m_containerType;
    (*pContainerObject)->m_posX = static_cast<WORD>(m_ptWalkToUse.x);
    (*pContainerObject)->m_posY = static_cast<WORD>(m_ptWalkToUse.y);
    memcpy((*pContainerObject)->m_script, m_scriptRes, RESREF_SIZE);
    strncpy((*pContainerObject)->m_scriptName, m_scriptName, SCRIPTNAME_SIZE);
    (*pContainerObject)->m_lockDifficulty = m_lockDifficulty;
    (*pContainerObject)->m_dwFlags = m_dwFlags;
    (*pContainerObject)->m_trapDetectionDifficulty = m_trapDetectionDifficulty;
    (*pContainerObject)->m_trapRemovalDifficulty = m_trapRemovalDifficulty;
    (*pContainerObject)->m_trapActivated = m_trapActivated;
    (*pContainerObject)->m_trapDetected = m_trapDetected;
    (*pContainerObject)->m_posXTrapOrigin = static_cast<WORD>(m_posTrapOrigin.x);
    (*pContainerObject)->m_posYTrapOrigin = static_cast<WORD>(m_posTrapOrigin.y);
    (*pContainerObject)->m_triggerRange = m_triggerRange;
    (*pContainerObject)->m_triggerRange = m_triggerRange;
    (*pContainerObject)->m_breakDifficulty = m_breakDifficulty;
    strncpy((*pContainerObject)->m_ownedBy, m_ownedBy, SCRIPTNAME_SIZE);
    m_keyType.GetResRef((*pContainerObject)->m_keyType);
}

// 0x480C00
void CGameContainer::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    CResRef areaResRef;
    DWORD cnt;
    CAreaFileContainer cContainerObject;
    WORD nItems = 0;
    CItem* tempItem;
    BYTE nAreaStringLength;
    CCreatureFileItem* pItems;
    CAreaPoint* pPoints;
    CString sAreaString;
    POSITION pos;

    if (m_nPolygon > 0) {
        pPoints = new CAreaPoint[m_nPolygon];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
        // __LINE__: 2149
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nPolygon; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pPolygon[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pPolygon[cnt].y);
        }

        cContainerObject.m_pickPointStart = 0;
        cContainerObject.m_pickPointCount = m_nPolygon;
    }

    pItems = NULL;
    if (m_lstItems.GetCount() > 0) {
        nItems = 0;
        pos = m_lstItems.GetHeadPosition();
        while (pos != NULL) {
            tempItem = m_lstItems.GetNext(pos);
            if (tempItem != NULL) {
                nItems++;
            }
        }

        if (nItems > 0) {
            pItems = new CCreatureFileItem[nItems];

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
            // __LINE__: 2173
            UTIL_ASSERT(pItems != NULL)

            // FIXME: Unsafe x64 conversion.
            cContainerObject.m_startingItem = 0;
            cContainerObject.m_itemCount = nItems;

            cnt = 0;
            pos = m_lstItems.GetHeadPosition();
            while (pos != NULL) {
                tempItem = m_lstItems.GetNext(pos);
                if (tempItem != NULL) {
                    tempItem->GetResRef().GetResRef(pItems[cnt].m_itemId);
                    for (INT ability = 0; ability < 3; ability++) {
                        pItems[cnt].m_usageCount[ability] = tempItem->GetUsageCount(ability);
                    }
                    pItems[cnt].m_wear = tempItem->m_wear;
                    pItems[cnt].m_dynamicFlags = tempItem->m_flags;
                    cnt++;
                }
            }

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
            // __LINE__: 2191
            UTIL_ASSERT(cnt == nItems)
        }
    }

    cContainerObject.m_boundingRectLeft = static_cast<WORD>(m_rBounding.left);
    cContainerObject.m_boundingRectTop = static_cast<WORD>(m_rBounding.top);
    cContainerObject.m_boundingRectRight = static_cast<WORD>(m_rBounding.right) - 1;
    cContainerObject.m_boundingRectBottom = static_cast<WORD>(m_rBounding.bottom) - 1;
    cContainerObject.m_containerType = m_containerType;
    cContainerObject.m_posX = static_cast<WORD>(m_ptWalkToUse.x);
    cContainerObject.m_posY = static_cast<WORD>(m_ptWalkToUse.y);
    memcpy(cContainerObject.m_script, m_scriptRes, RESREF_SIZE);
    strncpy(cContainerObject.m_scriptName, m_scriptName, SCRIPTNAME_SIZE);
    cContainerObject.m_lockDifficulty = m_lockDifficulty;
    cContainerObject.m_dwFlags = m_dwFlags;
    cContainerObject.m_trapDetectionDifficulty = m_trapDetectionDifficulty;
    cContainerObject.m_trapRemovalDifficulty = m_trapRemovalDifficulty;
    cContainerObject.m_trapActivated = m_trapActivated;
    cContainerObject.m_trapDetected = m_trapDetected;
    cContainerObject.m_posXTrapOrigin = static_cast<WORD>(m_posTrapOrigin.x);
    cContainerObject.m_posYTrapOrigin = static_cast<WORD>(m_posTrapOrigin.y);
    cContainerObject.m_triggerRange = m_triggerRange;
    cContainerObject.m_triggerRange = m_triggerRange;
    strncpy(cContainerObject.m_ownedBy, m_ownedBy, SCRIPTNAME_SIZE);
    m_keyType.GetResRef(cContainerObject.m_keyType);

    *dwSize = 0;

    areaResRef = m_pArea->m_resRef;
    areaResRef.CopyToString(sAreaString);
    nAreaStringLength = sAreaString.GetLength();

    *dwSize += sizeof(CAreaFileContainer)
        + sizeof(BYTE)
        + sizeof(WORD)
        + sizeof(WORD)
        + nAreaStringLength
        + sizeof(CAreaPoint) * m_nPolygon
        + sizeof(CCreatureFileItem) * nItems;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 2228
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE_CGAMECONTAINER);

    *pData = STATICBUFFER;

    cnt = 0;

    *reinterpret_cast<BYTE*>(*pData + cnt) = nAreaStringLength;
    cnt += sizeof(BYTE);

    memcpy(*pData + cnt, sAreaString.GetBuffer(nAreaStringLength), nAreaStringLength);
    cnt += nAreaStringLength;

    *reinterpret_cast<WORD*>(*pData + cnt) = m_nPolygon;
    cnt += sizeof(WORD);

    *reinterpret_cast<WORD*>(*pData + cnt) = nItems;
    cnt += sizeof(WORD);

    memcpy(*pData + cnt, &cContainerObject, sizeof(CAreaFileContainer));
    cnt += sizeof(CAreaFileContainer);

    if (m_nPolygon > 0) {
        memcpy(*pData + cnt, pPoints, sizeof(CAreaPoint) * m_nPolygon);
        cnt += sizeof(CAreaPoint) * m_nPolygon;
    }

    if (pItems != NULL) {
        memcpy(*pData + cnt, pItems, sizeof(CCreatureFileItem) * nItems);
        cnt += sizeof(CCreatureFileItem) * nItems;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 2261
    UTIL_ASSERT(cnt == *dwSize);

    if (m_nPolygon > 0) {
        delete pPoints;
    }

    if (pItems != NULL) {
        delete pItems;
    }
}

// 0x481160
void CGameContainer::RefreshRenderPile()
{
    if (m_containerType == 4) {
        m_nPileVidCell = 0;

        POSITION pos = m_lstItems.GetHeadPosition();
        while (pos != NULL && m_nPileVidCell < 3) {
            CItem* pItem = m_lstItems.GetNext(pos);
            if (pItem != NULL) {
                m_pileVidCell[m_nPileVidCell].SetResRef(pItem->GetGroundIcon(), FALSE, TRUE, TRUE);

                if (m_pileVidCell[m_nPileVidCell].cResRef != "") {
                    m_pileVidCell[m_nPileVidCell].SequenceSet(0);
                    m_pileVidCell[m_nPileVidCell].FrameSet(0);
                    m_nPileVidCell++;
                }
            }
        }
    }
}

// 0x481350
void CGameContainer::OpenContainer(const CAIObjectType& user)
{
    CMessage* message;
    if (m_trapActivated != 0) {
        CAITrigger opened(CAITrigger::OPENED, user, 0);
        m_pendingTriggers.AddTail(new CAITrigger(opened));

        message = new CMessageSetTrigger(opened, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        CAIObjectType type;
        type.m_sName = CString(m_ownedBy);

        LONG targetId = g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->sub_46DAE0(m_pos.x,
            m_pos.y,
            type,
            500,
            0,
            0,
            0,
            0,
            0);

        message = new CMessageSetTrigger(CAITrigger(CAITrigger::OPENED, 0), m_id, targetId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        if ((m_dwFlags & 0x8) == 0) {
            m_trapActivated = 0;

            message = new CMessageContainerStatus(m_dwFlags,
                m_trapActivated,
                m_trapDetected,
                m_id,
                m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }
}

// 0x481750
void CGameContainer::SetFlags(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
}

// 0x481760
void CGameContainer::SetTrapActivated(WORD active)
{
    m_trapActivated = active;
}

// 0x481770
void CGameContainer::SetTrapDetected(WORD state)
{
    m_trapDetected = state;
}

// 0x481780
void CGameContainer::SetDrawPoly(SHORT time)
{
    if (m_drawPoly != time) {
        if (!g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly) {
            CMessageSetDrawPoly* pMessage = new CMessageSetDrawPoly(time, GetId(), GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
    }
    m_drawPoly = time;
}

// 0x4817F0
SHORT CGameContainer::FindItemSlot(const CResRef& res)
{
    POSITION pos = m_lstItems.GetHeadPosition();

    SHORT nSlot = 0;
    while (pos != NULL) {
        CItem* pItem = m_lstItems.GetNext(pos);
        if (pItem != NULL && pItem->GetResRef() == res) {
            return nSlot;
        }

        nSlot++;
    }

    return -1;
}

// 0x481830
SHORT CGameContainer::CountItem(const CResRef& res)
{
    POSITION pos = m_lstItems.GetHeadPosition();

    SHORT nCount = 0;
    while (pos != NULL) {
        CItem* pItem = m_lstItems.GetNext(pos);
        if (pItem->GetResRef() == res) {
            if (pItem->GetMaxStackable() > 1) {
                nCount += pItem->GetUsageCount(0);
            } else {
                nCount++;
            }
        }
    }

    return nCount;
}

// 0x481A10
void CGameContainer::sub_481A10(CGameArea* pArea, CVidMode* pVidMode, int a3, DWORD rgbColor)
{
    CInfinity* pInfinity = pArea->GetInfinity();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameContainer.cpp
    // __LINE__: 2691
    UTIL_ASSERT(pInfinity != NULL && pVidMode != NULL);

    CRect rViewPort;
    rViewPort.left = m_pArea->GetInfinity()->nCurrentX;
    rViewPort.top = m_pArea->GetInfinity()->nCurrentY;
    rViewPort.right = m_pArea->GetInfinity()->rViewPort.right - m_pArea->GetInfinity()->rViewPort.left + rViewPort.left;
    rViewPort.bottom = m_pArea->GetInfinity()->rViewPort.bottom - m_pArea->GetInfinity()->rViewPort.top + rViewPort.top;

    INT nRects = field_8D6.GetSize();
    CPoint ptRef(0, 0);

    CVidPoly vidPoly;
    vidPoly.SetPoly(reinterpret_cast<WORD*>(field_8EA), m_nPolygon);

    for (INT nIndex = 0; nIndex < nRects; nIndex++) {
        CRect* pRect = field_8D6[nIndex];
        if (pRect != NULL) {
            CRect rClip = rViewPort & *pRect;
            if (!rClip.IsRectNull()) {
                CPoint ptPos(rClip.left, rClip.top);
                CRect rFX(rClip);
                rFX.OffsetRect(-rClip.left, -rClip.top);

                if (pInfinity->FXPrep(rFX, CInfinity::FXPREP_COPYFROMBACK, a3, ptPos, ptRef)) {
                    if (pInfinity->FXLock(rFX, 0)) {
                        static_cast<CVidInf*>(pVidMode)->FXRender(&vidPoly, rClip, rgbColor, 0x10, ptPos, FALSE);

                        if (pInfinity->FXUnlock(0, NULL, CPoint(0, 0))) {
                            pInfinity->FXBltFrom(a3, rFX, ptPos.x, ptPos.y, ptRef.x, ptRef.y, 0);
                        }
                    }
                }
            }
        }
    }
}

// 0x481C50
SHORT CGameContainer::GetTotalItemCount(CString sExcludeResRef)
{
    INT nCount = 0;

    POSITION pos = m_lstItems.GetHeadPosition();
    while (pos != NULL) {
        CItem* pItem = m_lstItems.GetNext(pos);
        if (pItem == NULL) {
            break;
        }

        WORD nMaxStackable = pItem->GetMaxStackable();
        if (!(pItem->GetResRef() == sExcludeResRef)) {
            if (nMaxStackable > 1) {
                nCount += pItem->GetUsageCount(0);
            } else if (pItem->GetItemType() == 58) {
                nCount += g_pBaldurChitin->GetObjectGame()->GetNumberOfItemsInBag(pItem->GetResRef(), CString(""));
            } else {
                nCount++;
            }
        }
    }

    return static_cast<SHORT>(nCount);
}

// 0x481D30
SHORT CGameContainer::DropInventoryEx(CString sExclude)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    BOOLEAN bHavePile = FALSE;
    LONG iPile;
    CGameContainer* pPile;

    if (m_pArea != NULL) {
        for (SHORT nSlot = 0; nSlot < m_lstItems.GetCount(); nSlot++) {
            if (!bHavePile) {
                iPile = pGame->GetGroundPile(m_id);
                if (iPile == CGameObjectArray::INVALID_INDEX) {
                    return ACTION_ERROR;
                }

                if (pGame->GetObjectArray()->GetDeny(iPile,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pPile),
                        INFINITE)
                    != CGameObjectArray::SUCCESS) {
                    return ACTION_ERROR;
                }

                bHavePile = TRUE;
            }

            // NOTE: Uninline.
            CItem* pItem = GetItem(nSlot);
            if (pItem != NULL) {
                // NOTE: Increments `nSlot` after compressing, so the next
                // item is skipped.
                if (pItem->cResRef != sExclude) {
                    pPile->PlaceItemInBlankSlot(pItem, TRUE, 0x7FFF);
                    SetItem(nSlot, NULL);
                    CompressContainer();
                }
            } else {
                CompressContainer();
            }
        }

        if (bHavePile) {
            pGame->GetObjectArray()->ReleaseDeny(iPile,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    return ACTION_DONE;
}

// 0x481EB0
void CGameContainer::sub_481EB0(INT nSlotNum)
{
    POSITION pos = m_lstItems.FindIndex(nSlotNum);
    if (pos != NULL) {
        CItem* pItem = m_lstItems.GetAt(pos);
        if (pItem != NULL) {
            g_pBaldurChitin->GetObjectGame()->AddDisposableItem(pItem);
        }

        if (m_pArea->m_resRef == RESREF_AR6051) {
            sub_480480(static_cast<SHORT>(nSlotNum), NULL);
        }

        m_lstItems.RemoveAt(pos);
    }

    CMessageContainerAddItem* pMessage = new CMessageContainerAddItem(CItem(),
        0x7FFF,
        TRUE,
        m_id,
        m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
}

// 0x482040
SHORT CGameContainer::SpellHitEffect(CGameAIBase* pSource, const CPoint& ptTarget, LONG targetId, LONG projectileType, LONG nHeight)
{
    CProjectile* pProjectile;
    LONG nMessageType = projectileType;

    if (projectileType != 210) {
        pProjectile = CProjectile::sub_560310(static_cast<WORD>(projectileType), NULL, FALSE);
        nMessageType = projectileType + 0x1000;
    } else {
        pProjectile = CProjectile::DecodeProjectile(211, pSource, FALSE);
    }

    CMessageFireProjectile* pMessage = new CMessageFireProjectile(static_cast<WORD>(nMessageType),
        targetId,
        ptTarget,
        nHeight,
        m_id,
        m_id,
        0);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    pProjectile->Fire(m_pArea,
        m_id,
        targetId,
        ptTarget,
        nHeight,
        static_cast<SHORT>(projectileType));

    return ACTION_DONE;
}

// FIXME: `scriptRes` should be reference.
//
// 0x45FE90
void CGameContainer::SetScriptRes(CString scriptRes)
{
    memset(m_scriptRes, 0, RESREF_SIZE);

    for (int index = 0; index < RESREF_SIZE && index < scriptRes.GetLength(); index++) {
        m_scriptRes[index] = scriptRes[index];
    }
}

// 0x45B930
SHORT CGameContainer::GetNumItems()
{
    return static_cast<SHORT>(m_lstItems.GetCount());
}
