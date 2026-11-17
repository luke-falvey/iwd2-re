#include "CGameTrigger.h"

#include "CAIScript.h"
#include "CAITrigger.h"
#include "CBaldurChitin.h"
#include "CCreatureFile.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CProjectile.h"
#include "CScreenWorld.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVidMode.h"
#include "CVidPoly.h"

// 0x8D71DC
const LONG CGameTrigger::RANGE_TRIGGER = 16
    * CPathSearch::GRID_SQUARE_SIZEX
    * CPathSearch::GRID_SQUARE_SIZEX;

// 0x8D71C4
const LONG CGameTrigger::RANGE_EDGE = (CSearchBitmap::TRAVEL_WIDTH + 10)
    * (CSearchBitmap::TRAVEL_WIDTH + 10)
    * CPathSearch::GRID_SQUARE_SIZEX
    * CPathSearch::GRID_SQUARE_SIZEX;

// 0x4CCAB0
CGameTrigger::CGameTrigger(CGameArea* pArea, CAreaFileTriggerObject* pTriggerObject, CAreaPoint* pPoints, WORD maxPts)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 110
    UTIL_ASSERT(pArea != NULL && pTriggerObject != NULL && pPoints != NULL && pTriggerObject->m_pickPointStart + pTriggerObject->m_pickPointCount <= maxPts);

    m_objectType = TYPE_TRIGGER;
    m_triggerType = pTriggerObject->m_triggerType;
    m_rBounding.left = pTriggerObject->m_boundingRectLeft;
    m_rBounding.top = pTriggerObject->m_boundingRectTop;
    m_rBounding.right = pTriggerObject->m_boundingRectRight + 1;
    m_rBounding.bottom = pTriggerObject->m_boundingRectBottom + 1;
    m_cursorType = pTriggerObject->m_cursorType != 30 ? pTriggerObject->m_cursorType : 42;
    memcpy(m_newArea, pTriggerObject->m_newArea, RESREF_SIZE);
    memcpy(m_newEntryPoint, pTriggerObject->m_newEntryPoint, SCRIPTNAME_SIZE);
    m_dwFlags = pTriggerObject->m_dwFlags;
    m_description = pTriggerObject->m_description;
    m_nPolygon = pTriggerObject->m_pickPointCount;
    memcpy(m_scriptRes, pTriggerObject->m_script, RESREF_SIZE);
    strncpy(m_scriptName, pTriggerObject->m_scriptName, SCRIPTNAME_SIZE);

    CAIScript* pScript = new CAIScript(CResRef(pTriggerObject->m_script));
    SetScript(0, pScript);

    m_trapDetectionDifficulty = pTriggerObject->m_trapDetectionDifficulty;
    m_trapDisarmingDifficulty = pTriggerObject->m_trapDisarmingDifficulty;
    m_trapActivated = pTriggerObject->m_trapActivated;
    m_trapDetected = pTriggerObject->m_trapDetected;
    m_posTrapOrigin.x = pTriggerObject->m_posXTrapOrigin;
    m_posTrapOrigin.y = pTriggerObject->m_posYTrapOrigin;
    m_keyType = pTriggerObject->m_keyType;
    m_drawPoly = 0;

    if (m_nPolygon != 0) {
        m_pPolygon = new CPoint[m_nPolygon];

        if (m_pPolygon == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
            // __LINE__: 149
            UTIL_ASSERT(FALSE);
            m_nPolygon = 0;
        } else {
            WORD adjust = 0;
            for (WORD cnt = 0; cnt < m_nPolygon; cnt++) {
                m_pPolygon[cnt - adjust].x = pPoints[cnt + pTriggerObject->m_pickPointStart].m_xPos;
                m_pPolygon[cnt - adjust].y = pPoints[cnt + pTriggerObject->m_pickPointStart].m_yPos;
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
        }
    } else {
        m_pPolygon = NULL;
    }

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id,
        this,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        CPoint pos;
        pos.x = (pTriggerObject->m_boundingRectLeft + pTriggerObject->m_boundingRectRight) / 2;
        pos.y = (pTriggerObject->m_boundingRectTop + pTriggerObject->m_boundingRectBottom) / 2;

        AddToArea(pArea,
            pos,
            0,
            m_triggerType == 0 ? LIST_FRONT : LIST_BACK);

        m_boundingRange = static_cast<WORD>(max((pTriggerObject->m_boundingRectRight - pTriggerObject->m_boundingRectLeft) / 2,
            2 * (pTriggerObject->m_boundingRectBottom - pTriggerObject->m_boundingRectTop) / 3));

        m_typeAI.SetName(CString(m_scriptName));

        CVariable name;
        name.SetName(CString(m_scriptName));
        name.m_intValue = m_id;
        pArea->GetNamedCreatures()->AddKey(name);

        if ((m_dwFlags & 0x200) != 0) {
            field_628.x = pTriggerObject->field_88;
            field_628.y = pTriggerObject->field_8C;
        }

        sub_481890(&m_rBounding, field_630);

        // FIXME: One assignment is usually enough.
        field_644 = NULL;
        field_644 = NULL;

        if (m_nPolygon != 0) {
            field_644 = new CAreaPoint[m_nPolygon];

            for (WORD cnt = 0; cnt < m_nPolygon; cnt++) {
                field_644[cnt].m_xPos = static_cast<WORD>(m_pPolygon[cnt].x);
                field_644[cnt].m_yPos = static_cast<WORD>(m_pPolygon[cnt].y);
            }
        }
    } else {
        delete this;
    }
}

// 0x4CD150
CGameTrigger::~CGameTrigger()
{
    if (m_pPolygon != NULL) {
        delete m_pPolygon;
    }

    if (field_644 != NULL) {
        delete field_644;
        field_644 = NULL;
    }

    // When there is only one element its an unowned pointer to `m_rBounding`.
    if (field_630.GetCount() > 1) {
        for (INT nIndex = 0; nIndex < field_630.GetCount(); nIndex++) {
            delete field_630[nIndex];
        }
    }

    field_630.SetSize(0);
}

// 0x47C830
BOOLEAN CGameTrigger::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x4CD220
BOOLEAN CGameTrigger::DoAIUpdate(BOOLEAN active, LONG counter)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CGameObjectArray* pObjectArray = pGame->GetObjectArray();

    if ((m_dwFlags & 0x100) != 0) {
        return FALSE;
    }

    if (m_triggerType == 0) {
        if ((m_dwFlags & 0x20) != 0 && pGame->m_cOptions.m_nTutorialState == 0) {
            return FALSE;
        }

        if (m_trapActivated != 0) {
            CTypedPtrList<CPtrList, LONG*> targets;

            m_pArea->GetAllInRange(m_posVertList,
                m_pos,
                CAIObjectType::ANYONE,
                m_boundingRange,
                m_pArea->m_visibleTerrainTable,
                targets,
                FALSE,
                FALSE);

            POSITION pos = targets.GetHeadPosition();
            while (pos != NULL) {
                LONG nId = static_cast<LONG>(reinterpret_cast<INT_PTR>(targets.GetNext(pos)));

                CGameSprite* pSprite;
                BYTE rc = pObjectArray->GetShare(nId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS) {
                    if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
                        if ((m_dwFlags & 0x40) != 0
                            || pGame->GetCharacterPortraitNum(pSprite->GetId()) != -1
                            || pGame->m_familiars.Find(reinterpret_cast<int*>(pSprite->GetId()), NULL) != NULL) {
                            // NOTE: Uninline.
                            if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
                                && g_pChitin->cNetwork.GetSessionHosting() == TRUE
                                && pSprite->field_580 == 1) {
                                // NOTE: The share is never released.
                                continue;
                            }

                            if (IsOverActivate(pSprite->GetPos())) {
                                CPoint ptOld;
                                ptOld.x = pSprite->field_536A;
                                ptOld.y = pSprite->field_536E;

                                if (!IsOverActivate(ptOld)) {
                                    do {
                                        rc = pObjectArray->GetDeny(nId,
                                            CGameObjectArray::THREAD_ASYNCH,
                                            reinterpret_cast<CGameObject**>(&pSprite),
                                            INFINITE);
                                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                                    if (rc == CGameObjectArray::SUCCESS) {
                                        // NOTE: Original reads trigger ID from a
                                        // constant at 0x847E30.
                                        CAITrigger trigger(CAITRIGGER_ENTERED, pSprite->GetAIType(), 0);

                                        CMessage* message = new CMessageSetTrigger(trigger, m_id, m_id);
                                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                                        pObjectArray->ReleaseDeny(nId,
                                            CGameObjectArray::THREAD_ASYNCH,
                                            INFINITE);
                                    }

                                    if ((m_dwFlags & 0x2) == 0) {
                                        m_trapActivated = 0;
                                    }
                                }
                            }

                            const CPoint& ptPos = pSprite->GetPos();
                            pSprite->field_536A = ptPos.x;
                            pSprite->field_536E = ptPos.y;
                        }
                    }

                    pObjectArray->ReleaseShare(nId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }

            return TRUE;
        }
    }

    return CResRef(m_scriptRes) != "" ? TRUE : FALSE;
}

// 0x4CD630
void CGameTrigger::AIUpdate()
{
    if (m_drawPoly > 0) {
        m_drawPoly--;
    }

    ProcessAI();
}

// 0x4CD650
void CGameTrigger::AddEffect(CGameEffect* pEffect, BYTE list, BOOL noSave, BOOL immediateApply)
{
    if (pEffect->m_effectID == CGAMEEFFECT_DETECTTRAPS) {
        // NOTE: Uninline.
        if (sub_45B950()) {
            if (m_trapDetected == 0
                && m_trapDetectionDifficulty < 100
                && (m_dwFlags & 0x8) != 0) {
                // NOTE: Original reads trigger ID from a constant at
                // 0x847E40.
                CAITrigger trigger(CAITRIGGER_DETECTED, m_typeAI, 0);

                CMessage* message = new CMessageSetTrigger(trigger, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                m_trapDetected = TRUE;

                // NOTE: Uninline.
                message = new CMessageTriggerStatus(m_dwFlags,
                    static_cast<WORD>(sub_45B950()),
                    m_trapDetected,
                    m_id,
                    m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                // NOTE: Original passes `m_id` where the callee expects a
                // `CGameAIBase*` caller.
                CGameObject* pObject = m_typeAI.sub_40CCA0(reinterpret_cast<CGameAIBase*>(m_id), FALSE);
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
        ITEM_EFFECT effect;
        CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_SKILLUNSUMMON);

        // NOTE: Original reads duration type 4 from a constant at 0x8491B0.
        effect.durationType = 4;
        effect.duration = (pEffect->m_duration - g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime) / 15;
        effect.effectAmount = 1;

        CGameEffect* pNewEffect = CGameEffect::DecodeEffect(&effect,
            CPoint(-1, -1),
            -1,
            CPoint(-1, -1));

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

// 0x4CE180
BOOLEAN CGameTrigger::CompressTime(DWORD deltaTime)
{
    if (m_triggerType == 0) {
        // NOTE: Unsigned compare.
        if (deltaTime > static_cast<DWORD>(m_drawPoly)) {
            m_drawPoly = 0;
        } else {
            m_drawPoly = static_cast<SHORT>(m_drawPoly - deltaTime);
        }
    }

    return TRUE;
}

// 0x4CE1C0
void CGameTrigger::DebugDump(const CString& message, BOOLEAN bEchoToScreen)
{
    STR_RES strRes;
    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

    CString sTemp;

    switch (m_triggerType) {
    case 0:
        if (bEchoToScreen) {
            pWorld->DisplayText(CString(""),
                CString("DEBUG DUMP: CGameTrigger - Proximity"),
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

            sTemp.Format("Reset Trap: %s", (m_dwFlags & 0x2) != 0 ? "TRUE" : "FALSE");
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);

            sTemp.Format("Trap Detection Difficulty: %d%%", m_trapDetectionDifficulty);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);

            sTemp.Format("Trap Removal Difficulty: %d%%", m_trapDisarmingDifficulty);
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
        }
        break;
    case 1:
        if (bEchoToScreen) {
            pWorld->DisplayText(CString(""),
                CString("DEBUG DUMP: CGameTrigger - Info"),
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

            sTemp.Format("STRREF: %d", m_description);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);

            // FIXME: `strRes` is never fetched (always empty string)
            sTemp.Format("Description: %s", (LPCSTR)strRes.szText);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);
        }
        break;
    case 2:
        if (bEchoToScreen) {
            pWorld->DisplayText(CString(""),
                CString("DEBUG DUMP: CGameTrigger - Travel"),
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

            sTemp.Format("Target Area: %.*s", RESREF_SIZE, m_newArea);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);

            sTemp.Format("Entry Point Name: %.*s", SCRIPTNAME_SIZE, m_newEntryPoint);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);

            sTemp.Format("Party Required: %s", (m_dwFlags & 0x4) != 0 ? "TRUE" : "FALSE");
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);
        }
        break;
    }
}

// 0x4CEBE0
BOOL CGameTrigger::IsOver(const CPoint& pt)
{
    if ((m_dwFlags & 0x100) != 0) {
        return FALSE;
    }

    if (m_triggerType == 0) {
        if (m_trapActivated == 0) {
            return FALSE;
        }

        if (m_trapDetected == 0) {
            return FALSE;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetState() != 2) {
            return FALSE;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetIconIndex() != 36) {
            return FALSE;
        }
    }

    if (!m_rBounding.PtInRect(pt)) {
        return FALSE;
    }

    if (!g_pBaldurChitin->GetObjectGame()->GetGroup()->IsPartyLeader()) {
        return FALSE;
    }

    if (m_pPolygon != NULL) {
        return CVidPoly::IsPtInPoly(m_pPolygon, m_nPolygon, pt);
    }

    return TRUE;
}

// 0x4CECB0
BOOL CGameTrigger::IsOverActivate(const CPoint& pt)
{
    if (!m_rBounding.PtInRect(pt)) {
        return FALSE;
    }

    if (m_pPolygon != NULL) {
        return CVidPoly::IsPtInPoly(m_pPolygon, m_nPolygon, pt);
    }

    return TRUE;
}

// 0x4CED10
void CGameTrigger::OnActionButton(const CPoint& pt)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CAIGroup* pGroup = pGame->GetGroup();
    CAIAction action;

    if (pGroup->m_memberList.GetCount() != 0) {
        CAIObjectType leaderType;
        CGameObject* pLeader = NULL;

        LONG nLeader = pGroup->GetGroupLeader();
        if (nLeader != CGameObjectArray::INVALID_INDEX) {
            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pLeader,
                    INFINITE);
                // NOTE: The condition can never be true, so the loop runs
                // once.
            } while (rc == CGameObjectArray::SHARED && rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                leaderType.Set(pLeader->GetAIType());

                pGame->GetObjectArray()->ReleaseShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        // NOTE: Original reads trigger ID from a constant at 0x847E78.
        CAITrigger trigger(CAITRIGGER_CLICKED, leaderType, 0);

        CMessage* message = new CMessageSetTrigger(trigger, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    switch (pGame->GetState()) {
    case 0:
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        if (pGroup->GetCount() > 0) {
            if (m_triggerType == 2) {
                CAIAction leaveAction(CAIAction::LEAVEAREANAME, m_id, 0, 0);
                pGroup->GroupAction(leaveAction, TRUE, NULL);

                if ((m_dwFlags & 0x200) != 0) {
                    CAIAction moveAction(CAIAction::MOVETOPOINT, field_628, 0, -1);
                    pGroup->GroupAction(moveAction, FALSE, NULL);
                } else {
                    CAIAction moveAction(CAIAction::MOVETOPOINT, m_pos, 0, -1);
                    pGroup->GroupAction(moveAction, FALSE, NULL);
                }

                pGroup->SetGroupTriggerId(m_id);

                if (m_description != 0xFFFFFFFF && m_description != 0) {
                    STR_RES strRes;
                    g_pBaldurChitin->GetTlkTable().Fetch(m_description, strRes);

                    // NOTE: Original reads marker `-1` from a constant at
                    // 0x84C960.
                    g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                        strRes.szText,
                        RGB(215, 215, 190),
                        RGB(180, 180, 255),
                        -1,
                        FALSE);
                }
            } else if (m_triggerType == 1) {
                if (m_description != 0xFFFFFFFF) {
                    FloatText(m_description, 10, 5);
                }
            } else {
                CGameObject::OnActionButton(pt);
            }
        }
        break;
    case 2:
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        switch (pGame->GetIconIndex()) {
        case 12:
        case 18:
        case 20:
        case 40:
        case 255:
            CGameObject::OnActionButton(pt);
            break;
        case 36:
            if (m_triggerType == 0 && m_trapActivated != 0 && m_trapDetected != 0) {
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
                // 0x8477BA. The assignment operator is inlined.
                action = CAIAction(28, m_typeAI, 0, 0, 0);
                pGroup->GroupAction(action, TRUE, NULL);

                pGame->SetState(0);
                pGame->GetButtonArray()->SetSelectedButton(100);
                pGame->GetButtonArray()->UpdateState();
            } else {
                CGameObject::OnActionButton(pt);
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
            // __LINE__: 973
            UTIL_ASSERT(FALSE);
        }
        break;
    case 3:
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        CGameObject::OnActionButton(pt);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
        // __LINE__: 984
        UTIL_ASSERT(FALSE);
    }
}

// 0x4CFA60
void CGameTrigger::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // NOTE: Original reads the outline colors from globals at 0x8A85FC
    // (RGB(0, 250, 0)), 0x8A8608 (RGB(255, 0, 0)) and 0x8A8600
    // (RGB(0, 0, 255)).

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 1020
    UTIL_ASSERT(pVidMode != NULL);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (m_triggerType == 0) {
        if (m_trapActivated != 0
            && m_trapDetected != 0
            && m_id == m_pArea->m_iPicked
            && pGame->GetState() == 2
            && pGame->GetIconIndex() == 36) {
            if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                // NOTE: Uninline.
                sub_4D02C0(pArea,
                    pVidMode,
                    a3,
                    g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(0, 250, 0)));
            }

            m_pArea->m_cInfinity.OutlinePoly(m_pPolygon,
                m_nPolygon,
                m_rBounding,
                RGB(0, 250, 0));
            return;
        }

        if (m_drawPoly > 0) {
            if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                // NOTE: Uninline.
                sub_4D02C0(pArea,
                    pVidMode,
                    a3,
                    g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(255, 0, 0)));
            }

            m_pArea->m_cInfinity.OutlinePoly(m_pPolygon,
                m_nPolygon,
                m_rBounding,
                RGB(255, 0, 0));
            return;
        }
    }

    if (pGame->field_4AB2) {
        if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
            // NOTE: Uninline.
            sub_4D02C0(pArea,
                pVidMode,
                a3,
                g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(0, 0, 255)));
        }

        m_pArea->m_cInfinity.OutlinePoly(m_pPolygon,
            m_nPolygon,
            m_rBounding,
            RGB(0, 0, 255));
    }
}

// 0x4CFC20
void CGameTrigger::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
        // __LINE__: 1111
        UTIL_ASSERT(FALSE);
        return;
    }

    delete this;
}

// 0x4CFC80
void CGameTrigger::SetCursor(LONG nToolTip)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if ((m_dwFlags & 0x100) == 0) {
        switch (pGame->GetState()) {
        case 0:
            if (pGame->GetGroup()->GetCount() != 0) {
                if (m_triggerType != 0) {
                    g_pBaldurChitin->GetObjectCursor()->SetCursor(m_cursorType, FALSE);
                } else {
                    CGameObject::SetCursor(nToolTip);
                }
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
            case 18:
            case 20:
            case 40:
            case 255:
                CGameObject::SetCursor(nToolTip);
                break;
            case 36:
                if (m_triggerType == 0
                    && m_trapActivated != 0
                    && m_trapDetected != 0) {
                    g_pBaldurChitin->GetObjectCursor()->SetCursor(38, FALSE);

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
                        if (pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE] == 0) {
                            g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                        }

                        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                } else {
                    CGameObject::SetCursor(nToolTip);
                }
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
                // __LINE__: 1200
                UTIL_ASSERT(FALSE);
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
            // __LINE__: 1210
            UTIL_ASSERT(FALSE);
        }
    } else {
        CGameObject::SetCursor(nToolTip);
    }
}

// 0x4CFF70
void CGameTrigger::Marshal(CAreaFileTriggerObject** pTriggerObject)
{
    CAreaPoint* pPoints;
    DWORD cnt;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 1235
    UTIL_ASSERT(pTriggerObject != NULL);

    *pTriggerObject = new CAreaFileTriggerObject();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 1241
    UTIL_ASSERT(*pTriggerObject != NULL);

    // FIXME: Redundant, memset is a part of constructor.
    memset(*pTriggerObject, 0, sizeof(CAreaFileTriggerObject));

    if (m_nPolygon > 0) {
        pPoints = new CAreaPoint[m_nPolygon];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
        // __LINE__: 1246
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nPolygon; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pPolygon[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pPolygon[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pTriggerObject)->m_pickPointStart = reinterpret_cast<DWORD>(pPoints);
        (*pTriggerObject)->m_pickPointCount = m_nPolygon;
    }

    (*pTriggerObject)->m_boundingRectLeft = static_cast<WORD>(m_rBounding.left);
    (*pTriggerObject)->m_boundingRectTop = static_cast<WORD>(m_rBounding.top);
    (*pTriggerObject)->m_boundingRectRight = static_cast<WORD>(m_rBounding.right) - 1;
    (*pTriggerObject)->m_boundingRectBottom = static_cast<WORD>(m_rBounding.bottom) - 1;

    if ((m_dwFlags & 0x200) != 0) {
        (*pTriggerObject)->m_transitionWalkToX = static_cast<WORD>(field_628.x);
        (*pTriggerObject)->m_transitionWalkToY = static_cast<WORD>(field_628.y);
    } else {
        (*pTriggerObject)->m_transitionWalkToX = static_cast<WORD>(m_pos.x);
        (*pTriggerObject)->m_transitionWalkToY = static_cast<WORD>(m_pos.y);
    }

    (*pTriggerObject)->m_triggerType = m_triggerType;
    (*pTriggerObject)->m_cursorType = m_cursorType;
    memcpy((*pTriggerObject)->m_newArea, m_newArea, RESREF_SIZE);
    memcpy((*pTriggerObject)->m_newEntryPoint, m_newEntryPoint, SCRIPTNAME_SIZE);
    (*pTriggerObject)->m_dwFlags = m_dwFlags;
    (*pTriggerObject)->m_description = m_description;
    memcpy((*pTriggerObject)->m_script, m_scriptRes, RESREF_SIZE);
    strncpy((*pTriggerObject)->m_scriptName, m_scriptName, SCRIPTNAME_SIZE);
    (*pTriggerObject)->m_trapDetectionDifficulty = m_trapDetectionDifficulty;
    (*pTriggerObject)->m_trapDisarmingDifficulty = m_trapDisarmingDifficulty;
    (*pTriggerObject)->m_trapActivated = m_trapActivated;
    (*pTriggerObject)->m_trapDetected = m_trapDetected;
    (*pTriggerObject)->m_posXTrapOrigin = static_cast<WORD>(m_posTrapOrigin.x);
    (*pTriggerObject)->m_posYTrapOrigin = static_cast<WORD>(m_posTrapOrigin.y);
    m_keyType.GetResRef((*pTriggerObject)->m_keyType);
    (*pTriggerObject)->field_88 = field_628.x;
    (*pTriggerObject)->field_8C = field_628.y;
}

// 0x4D0230
void CGameTrigger::SetDrawPoly(SHORT time)
{
    if (m_drawPoly != time) {
        if (!g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly) {
            CMessageSetDrawPoly* pMessage = new CMessageSetDrawPoly(time, GetId(), GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
    }
    m_drawPoly = time;
}

// 0x4D02A0
CPoint& CGameTrigger::GetPos()
{
    return (m_dwFlags & 0x200) != 0 ? field_628 : m_pos;
}

// 0x4D02C0
void CGameTrigger::sub_4D02C0(CGameArea* pArea, CVidMode* pVidMode, int a3, DWORD rgbColor)
{
    CInfinity* pInfinity = pArea->GetInfinity();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 1432
    UTIL_ASSERT(pInfinity != NULL && pVidMode != NULL);

    CRect rViewPort;
    rViewPort.left = m_pArea->GetInfinity()->nCurrentX;
    rViewPort.top = m_pArea->GetInfinity()->nCurrentY;
    rViewPort.right = m_pArea->GetInfinity()->rViewPort.right - m_pArea->GetInfinity()->rViewPort.left + rViewPort.left;
    rViewPort.bottom = m_pArea->GetInfinity()->rViewPort.bottom - m_pArea->GetInfinity()->rViewPort.top + rViewPort.top;

    INT nRects = field_630.GetSize();
    CPoint ptRef(0, 0);

    CVidPoly vidPoly;
    vidPoly.SetPoly(reinterpret_cast<WORD*>(field_644), m_nPolygon);

    for (INT nIndex = 0; nIndex < nRects; nIndex++) {
        CRect* pRect = field_630[nIndex];
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

// 0x45B950
BOOL CGameTrigger::sub_45B950()
{
    return m_trapActivated && (m_dwFlags & 0x100) == 0;
}
