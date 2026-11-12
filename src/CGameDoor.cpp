#include "CGameDoor.h"

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
#include "CVidMode.h"
#include "CVidPoly.h"

// 0x8D41D8
const LONG CGameDoor::RANGE_DOOR = 16
    * CPathSearch::GRID_SQUARE_SIZEX
    * CPathSearch::GRID_SQUARE_SIZEX;

// 0x485AC0
CGameDoor::CGameDoor(CGameArea* pArea, CAreaFileDoorObject* pDoorObject, CAreaPoint* pPoints, WORD maxPts)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 107
    UTIL_ASSERT(pArea != NULL && pDoorObject != NULL && pPoints != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 108
    UTIL_ASSERT(pDoorObject->m_openSelectionPointStart + pDoorObject->m_openSelectionPointCount <= maxPts);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 109
    UTIL_ASSERT(pDoorObject->m_closedSelectionPointStart + pDoorObject->m_closedSelectionPointCount <= maxPts);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 110
    UTIL_ASSERT(pDoorObject->m_openSearchSquaresStart + pDoorObject->m_openSearchSquaresCount <= maxPts);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 111
    UTIL_ASSERT(pDoorObject->m_closedSearchSquaresStart + pDoorObject->m_closedSearchSquaresCount <= maxPts);

    m_objectType = TYPE_DOOR;
    m_resID = pDoorObject->m_doorID;

    m_rOpenBounding.left = pDoorObject->m_openBoundingRectLeft;
    m_rOpenBounding.top = pDoorObject->m_openBoundingRectTop;
    m_rOpenBounding.right = pDoorObject->m_openBoundingRectRight;
    m_rOpenBounding.bottom = pDoorObject->m_openBoundingRectBottom;

    m_rClosedBounding.left = pDoorObject->m_closedBoundingRectLeft;
    m_rClosedBounding.top = pDoorObject->m_closedBoundingRectTop;
    m_rClosedBounding.right = pDoorObject->m_closedBoundingRectRight;
    m_rClosedBounding.bottom = pDoorObject->m_closedBoundingRectBottom;

    m_cursorType = pDoorObject->m_cursorType;
    m_dwFlags = pDoorObject->m_dwFlags;
    m_nOpenPolygon = pDoorObject->m_openSelectionPointCount;
    m_nClosedPolygon = pDoorObject->m_closedSelectionPointCount;

    m_ptDest1.x = pDoorObject->m_posXWalkTo1;
    m_ptDest1.y = pDoorObject->m_posYWalkTo1;

    m_ptDest2.x = pDoorObject->m_posXWalkTo2;
    m_ptDest2.y = pDoorObject->m_posYWalkTo2;

    m_strNotPickable = pDoorObject->m_strNotPickable;

    POSITION pos = pArea->m_lTiledObjects.AddTail(&m_tiledObject);
    WORD wInitialState = (m_dwFlags & 0x1) != 0
        ? CTiledObject::STATE_PRIMARY_TILE
        : CTiledObject::STATE_SECONDARY_TILE;
    m_tiledObject.Initialize(pArea->m_pResWED,
        m_resID,
        pos,
        wInitialState);

    memcpy(m_scriptRes, pDoorObject->m_script, RESREF_SIZE);

    strncpy(m_scriptName, pDoorObject->m_scriptName, SCRIPTNAME_SIZE);

    CAIScript* pScript = new CAIScript(CResRef(pDoorObject->m_script));
    SetScript(0, pScript);

    m_hitPoints = pDoorObject->m_hitPoints != 0 ? pDoorObject->m_hitPoints : 20;
    m_armourClass = pDoorObject->m_armourClass;
    m_openSound = pDoorObject->m_openSound;
    m_closeSound = pDoorObject->m_closeSound;
    m_trapDetectionDifficulty = pDoorObject->m_trapDetectionDifficulty;
    m_trapDisarmingDifficulty = pDoorObject->m_trapDisarmingDifficulty;
    m_trapActivated = pDoorObject->m_trapActivated;
    m_trapDetected = pDoorObject->m_trapDetected;
    m_posXTrapOrigin = pDoorObject->m_posXTrapOrigin;
    m_posYTrapOrigin = pDoorObject->m_posYTrapOrigin;
    m_keyType = pDoorObject->m_keyType;
    m_detectionDifficulty = pDoorObject->m_detectionDifficulty;
    m_lockDifficulty = pDoorObject->m_lockDifficulty;
    m_drawPoly = 0;

    if (m_nOpenPolygon != 0) {
        m_pOpenPolygon = new CPoint[m_nOpenPolygon];

        if (m_pOpenPolygon == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 170
            UTIL_ASSERT(FALSE);
        }

        WORD adjust = 0;
        for (WORD cnt = 0; cnt < m_nOpenPolygon; cnt++) {
            m_pOpenPolygon[cnt - adjust].x = pPoints[cnt + pDoorObject->m_openSelectionPointStart].m_xPos;
            m_pOpenPolygon[cnt - adjust].y = pPoints[cnt + pDoorObject->m_openSelectionPointStart].m_yPos;
            if (cnt >= 2) {
                int x2 = m_pOpenPolygon[cnt - adjust - 2].x;
                int y2 = m_pOpenPolygon[cnt - adjust - 2].y;

                int x1 = m_pOpenPolygon[cnt - adjust - 1].x;
                int y1 = m_pOpenPolygon[cnt - adjust - 1].y;

                int x0 = m_pOpenPolygon[cnt - adjust].x;
                int y0 = m_pOpenPolygon[cnt - adjust].y;

                if ((x2 == x1 && x1 == x0)
                    || (y2 == y1 && y1 == y0)
                    || (x2 != x1
                        && x1 != x0
                        && 1000 * (y2 - y1) / (x2 - x1) != 1000 * (y1 - y0) / (x1 - x0))) {
                    m_pOpenPolygon[cnt - adjust - 1] = m_pOpenPolygon[cnt - adjust];
                    adjust++;
                }
            }
        }

        m_nOpenPolygon -= adjust;
    } else {
        m_pOpenPolygon = NULL;
    }

    if (m_nClosedPolygon != 0) {
        m_pClosedPolygon = new CPoint[m_nClosedPolygon];

        if (m_pClosedPolygon == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 206
            UTIL_ASSERT(FALSE);
        }

        WORD adjust = 0;
        for (WORD cnt = 0; cnt < m_nClosedPolygon; cnt++) {
            m_pClosedPolygon[cnt - adjust].x = pPoints[cnt + pDoorObject->m_closedSelectionPointStart].m_xPos;
            m_pClosedPolygon[cnt - adjust].y = pPoints[cnt + pDoorObject->m_closedSelectionPointStart].m_yPos;
            if (cnt >= 2) {
                int x2 = m_pClosedPolygon[cnt - adjust - 2].x;
                int y2 = m_pClosedPolygon[cnt - adjust - 2].y;

                int x1 = m_pClosedPolygon[cnt - adjust - 1].x;
                int y1 = m_pClosedPolygon[cnt - adjust - 1].y;

                int x0 = m_pClosedPolygon[cnt - adjust].x;
                int y0 = m_pClosedPolygon[cnt - adjust].y;

                if ((x2 == x1 && x1 == x0)
                    || (y2 == y1 && y1 == y0)
                    || (x2 != x1
                        && x1 != x0
                        && 1000 * (y2 - y1) / (x2 - x1) != 1000 * (y1 - y0) / (x1 - x0))) {
                    m_pClosedPolygon[cnt - adjust - 1] = m_pClosedPolygon[cnt - adjust];
                    adjust++;
                }
            }
        }

        m_nClosedPolygon -= adjust;
    } else {
        m_pClosedPolygon = NULL;
    }

    m_nOpenSearch = pDoorObject->m_openSearchSquaresCount;
    if (m_nOpenSearch != 0) {
        m_pOpenSearch = new CPoint[m_nOpenSearch];

        if (m_pOpenSearch == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 243
            UTIL_ASSERT(FALSE);
        }

        for (WORD cnt = 0; cnt < m_nOpenSearch; cnt++) {
            m_pOpenSearch[cnt].x = pPoints[pDoorObject->m_openSearchSquaresStart + cnt].m_xPos;
            m_pOpenSearch[cnt].y = pPoints[pDoorObject->m_openSearchSquaresStart + cnt].m_yPos;
        }

        if ((m_dwFlags & 0x1) != 0) {
            pArea->m_search.AddDoor(m_pOpenSearch,
                m_nOpenSearch,
                (m_dwFlags & 0x400) != 0);
        }
    } else {
        m_pOpenSearch = NULL;
    }

    m_ptOpenDest.x = (pDoorObject->m_openBoundingRectLeft + pDoorObject->m_openBoundingRectRight) / 2;
    m_ptOpenDest.y = pDoorObject->m_openBoundingRectBottom;

    m_nClosedSearch = pDoorObject->m_openSearchSquaresCount;
    if (m_nClosedSearch != 0) {
        m_pClosedSearch = new CPoint[m_nClosedSearch];

        if (m_pClosedSearch == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 270
            UTIL_ASSERT(FALSE);
        }

        for (WORD cnt = 0; cnt < m_nClosedSearch; cnt++) {
            m_pClosedSearch[cnt].x = pPoints[pDoorObject->m_closedSearchSquaresStart + cnt].m_xPos;
            m_pClosedSearch[cnt].y = pPoints[pDoorObject->m_closedSearchSquaresStart + cnt].m_yPos;
        }

        if ((m_dwFlags & 0x1) == 0) {
            pArea->m_search.AddDoor(m_pClosedSearch,
                m_nClosedSearch,
                (m_dwFlags & 0x400) != 0);
        }
    } else {
        m_pClosedSearch = NULL;
    }

    m_ptClosedDest.x = (pDoorObject->m_closedBoundingRectLeft + pDoorObject->m_closedBoundingRectRight) / 2;
    m_ptClosedDest.y = pDoorObject->m_closedBoundingRectBottom;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id,
        this,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        if ((m_dwFlags & 0x1) != 0) {
            AddToArea(pArea, m_ptOpenDest, 0, LIST_FRONT);
        } else {
            AddToArea(pArea, m_ptClosedDest, 0, LIST_FRONT);
        }

        m_typeAI.SetName(CString(m_scriptName));

        CVariable name;
        name.SetName(CString(m_scriptName));
        name.m_intValue = m_id;
        pArea->GetNamedCreatures()->AddKey(name);

        sub_481890(&m_rClosedBounding, field_6DA);
        sub_481890(&m_rOpenBounding, field_6F2);

        field_6EE = NULL;
        field_706 = NULL;

        if (m_nClosedPolygon != 0) {
            field_6EE = new CAreaPoint[m_nClosedPolygon];

            for (WORD cnt = 0; cnt < m_nClosedPolygon; cnt++) {
                field_6EE[cnt].m_xPos = static_cast<WORD>(m_pClosedPolygon[cnt].x);
                field_6EE[cnt].m_yPos = static_cast<WORD>(m_pClosedPolygon[cnt].y);
            }
        } else {
            field_6EE = NULL;
        }

        if (m_nOpenPolygon != 0) {
            field_706 = new CAreaPoint[m_nOpenPolygon];

            for (WORD cnt = 0; cnt < m_nOpenPolygon; cnt++) {
                field_706[cnt].m_xPos = static_cast<WORD>(m_pOpenPolygon[cnt].x);
                field_706[cnt].m_yPos = static_cast<WORD>(m_pOpenPolygon[cnt].y);
            }
        } else {
            field_706 = NULL;
        }

        m_nAICounter = 0;
    } else {
        delete this;
    }
}

// 0x4866E0
CGameDoor::~CGameDoor()
{
    if (m_pOpenPolygon != NULL) {
        delete m_pOpenPolygon;
    }

    if (m_pClosedPolygon != NULL) {
        delete m_pClosedPolygon;
    }

    if (m_pOpenSearch != NULL) {
        delete m_pOpenSearch;
    }

    if (m_pClosedSearch != NULL) {
        delete m_pClosedSearch;
    }

    if (field_6EE != NULL) {
        delete field_6EE;
        field_6EE = NULL;
    }

    // When there is only one element its an unowned pointer to
    // `m_rClosedBounding`.
    if (field_6DA.GetCount() > 1) {
        for (INT nIndex = 0; nIndex < field_6DA.GetCount(); nIndex++) {
            delete field_6DA[nIndex];
        }
    }
    field_6DA.SetSize(0);

    if (field_706 != NULL) {
        delete field_706;
        field_706 = NULL;
    }

    // When there is only one element its an unowned pointer to
    // `m_rOpenBounding`.
    if (field_6F2.GetCount() > 1) {
        for (INT nIndex = 0; nIndex < field_6F2.GetCount(); nIndex++) {
            delete field_6F2[nIndex];
        }
    }
    field_6F2.SetSize(0);
}

// 0x47C830
BOOLEAN CGameDoor::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x486860
void CGameDoor::AddEffect(CGameEffect* pEffect, BYTE list, BOOL noSave, BOOL immediateApply)
{
    BOOLEAN bSendStatus = FALSE;

    if (pEffect->m_effectID == CGAMEEFFECT_KNOCK) {
        if (m_lockDifficulty != 100) {
            m_dwFlags &= ~0x2;

            CMessage* message = new CMessageDoorOpen(TRUE, m_id, m_id);
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

            if (bSendStatus == TRUE) {
                CMessage* message = new CMessageDoorStatus(this, m_id, m_id);
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

// 0x487460
void CGameDoor::AIUpdate()
{
    if (m_drawPoly > 0) {
        m_drawPoly--;
    }

    ProcessAI();

    if (g_pBaldurChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pBaldurChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (m_nAICounter++ == 225) {
            m_nAICounter = 0;

            CMessageDoorStatus* pMessage = new CMessageDoorStatus(this, m_id, m_id);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
    }
}

// 0x487530
BOOLEAN CGameDoor::CompressTime(DWORD deltaTime)
{
    // NOTE: Unsigned compare.
    if (deltaTime > static_cast<DWORD>(m_drawPoly)) {
        m_drawPoly = 0;
    } else {
        m_drawPoly = static_cast<SHORT>(m_drawPoly - deltaTime);
    }

    if (deltaTime > static_cast<DWORD>(CTimerWorld::TIMESCALE_MIN_PER_HOUR
            * CTimerWorld::TIMESCALE_SEC_PER_MIN
            * CTimerWorld::TIMESCALE_MSEC_PER_SEC)) {
        if ((m_dwFlags & 0x1) != 0
            && (m_dwFlags & 0x20) == 0
            && m_pArea->m_search.CanToggleDoor(m_pClosedSearch, m_nClosedSearch)
            && rand() % 5 != 0) {
            m_dwFlags &= ~0x1;
            m_pos = m_ptOpenDest;
            m_tiledObject.m_wAIState = CTiledObject::STATE_SECONDARY_TILE;

            if (m_pOpenSearch != NULL) {
                m_pArea->m_search.RemoveDoor(m_pOpenSearch, m_nOpenSearch);
            }

            if (m_pClosedSearch != NULL) {
                m_pArea->m_search.AddDoor(m_pClosedSearch,
                    m_nClosedSearch,
                    (m_dwFlags >> 10) & 0x1);
            }

            // NOTE: Uninline.
            if (g_pBaldurChitin->cNetwork.GetSessionHosting() == TRUE) {
                CMessageDoorStatus* pDoorStatus = new CMessageDoorStatus(this, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pDoorStatus, FALSE);
            }
        }

        m_dwFlags &= ~0x1000;
    }

    return TRUE;
}

// 0x4876E0
void CGameDoor::DebugDump(const CString& message, BOOLEAN bEchoToScreen)
{
    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

    CString sTemp;

    if (bEchoToScreen) {
        pWorld->DisplayText(CString(""),
            CString("DEBUG DUMP: CGameDoor"),
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

        sTemp.Format("Locked: %s\n", (m_dwFlags & 0x2) != 0 ? "TRUE" : "FALSE");
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

        sTemp.Format("Reset Trap: %s", (m_dwFlags & 0x4) != 0 ? "TRUE" : "FALSE");
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

        sTemp.Format("Trap Launching Point: x=%d y=%d", m_posXTrapOrigin, m_posYTrapOrigin);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);
    }
}

// 0x487C10
BOOL CGameDoor::IsOver(const CPoint& pt)
{
    if ((m_dwFlags & 0x2000) != 0) {
        return FALSE;
    }

    if ((m_dwFlags & 0x1) != 0) {
        if (!m_rOpenBounding.PtInRect(pt)) {
            return FALSE;
        }

        if (!g_pBaldurChitin->GetObjectGame()->GetGroup()->IsPartyLeader()) {
            return FALSE;
        }

        if (m_pOpenPolygon != NULL) {
            return CVidPoly::IsPtInPoly(m_pOpenPolygon, m_nOpenPolygon, pt);
        }
    } else {
        if (!m_rClosedBounding.PtInRect(pt)) {
            return FALSE;
        }

        if (!g_pBaldurChitin->GetObjectGame()->GetGroup()->IsPartyLeader()) {
            return FALSE;
        }

        if (m_pClosedPolygon != NULL) {
            return CVidPoly::IsPtInPoly(m_pClosedPolygon, m_nClosedPolygon, pt);
        }
    }

    return TRUE;
}

// 0x487D10
void CGameDoor::OnActionButton(const CPoint& pt)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CAIGroup* pGroup = pGame->GetGroup();

    if ((m_dwFlags & 0x80) != 0 && (m_dwFlags & 0x100) == 0) {
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
            // NOTE: Original reads action ID 142 from a constant at 0x847890.
            CAIAction action(142, m_id, 0, 0);
            pGroup->GroupAction(action, TRUE, NULL);

            LONG* pList = pGroup->GetGroupList();
            LONG nRange = RANGE_DOOR;

            if (pList == NULL) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
                // __LINE__: 857
                UTIL_ASSERT(FALSE);
            } else {
                CGameObjectArray* pObjectArray = pGame->GetObjectArray();
                CPoint ptDest;
                SHORT nIndex;
                for (nIndex = 0; nIndex < pGroup->GetCount(); nIndex++) {
                    CGameSprite* pSprite;
                    BYTE rc = pObjectArray->GetShare(pList[nIndex],
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                    if (rc != CGameObjectArray::SUCCESS) {
                        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
                        // __LINE__: 865
                        UTIL_ASSERT(FALSE);
                        continue;
                    }

                    CPoint ptSprite = pSprite->GetPos();

                    // NOTE: Uninline.
                    ptDest = GetClosestDest(ptSprite);

                    if (strnicmp(reinterpret_cast<char*>(m_pArea->m_header.m_areaName), "ar5004", 7) == 0) {
                        nRange = RANGE_DOOR / 10;
                    } else if (strnicmp(reinterpret_cast<char*>(m_pArea->m_header.m_areaName), "ar5202", 7) == 0) {
                        nRange = RANGE_DOOR / 5;
                    }

                    LONG dx = ptSprite.x - ptDest.x;
                    LONG dy = ptSprite.y - ptDest.y;

                    // NOTE: Y distance is scaled by 4/3 (isometric).
                    if (dx * dx + (dy * dy * 16) / 9 > nRange) {
                        pObjectArray->ReleaseShare(pList[nIndex],
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    } else {
                        pObjectArray->ReleaseShare(pList[nIndex],
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        break;
                    }
                }

                if (nIndex == pGroup->GetCount()) {
                    // NOTE: If every `GetShare` failed `ptDest` is used
                    // uninitialized.
                    CAIAction moveAction(CAIAction::MOVETOPOINT, ptDest, 0, -1);
                    pGroup->GroupAction(moveAction, FALSE, NULL);
                }

                delete pList;
            }
        }
        break;
    case 2:
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        switch (pGame->GetIconIndex()) {
        case 12:
            if ((m_dwFlags & 0x1) != 0 || (m_dwFlags & 0x2) == 0) {
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
                if ((m_dwFlags & 0x2) == 0) {
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
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 978
            UTIL_ASSERT(FALSE);
        }
        break;
    case 3:
        // NOTE: Uninline.
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        CGameObject::OnActionButton(pt);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 989
        UTIL_ASSERT(FALSE);
    }
}

// 0x488B30
void CGameDoor::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // NOTE: Original reads the outline colors from globals at 0x8A85F8
    // (RGB(32, 64, 160)), 0x8A85FC (RGB(0, 250, 0)), 0x8A8604
    // (RGB(255, 0, 255)) and 0x8A8608 (RGB(255, 0, 0)).
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    SHORT nState = pGame->GetState();
    BOOL bRendered = FALSE;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 1028
    UTIL_ASSERT(pVidMode != NULL);

    if ((m_dwFlags & 0x2000) != 0) {
        return;
    }

    if (m_id == m_pArea->m_iPicked) {
        // NOTE: Uninline.
        if (m_pArea->m_visibility.IsTileExplored(m_pArea->m_visibility.PointToTile(m_pos))
            && (m_dwFlags & 0x80) == 0) {
            if (pGame->GetGroup()->GetCount() > 0 && nState == 0) {
                if ((m_dwFlags & 0x1) != 0) {
                    if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                        // NOTE: Uninline.
                        sub_48B600(pArea,
                            pVidMode,
                            a3,
                            g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(32, 64, 160)));
                        bRendered = TRUE;
                    }

                    m_pArea->m_cInfinity.OutlinePoly(m_pOpenPolygon,
                        m_nOpenPolygon,
                        m_rOpenBounding,
                        RGB(32, 64, 160));
                } else {
                    if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                        // NOTE: Uninline.
                        sub_48B3C0(pArea,
                            pVidMode,
                            a3,
                            g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(32, 64, 160)));
                        bRendered = TRUE;
                    }

                    m_pArea->m_cInfinity.OutlinePoly(m_pClosedPolygon,
                        m_nClosedPolygon,
                        m_rClosedBounding,
                        RGB(32, 64, 160));
                }
            }

            if (pGame->GetState() == 2) {
                if (pGame->GetIconIndex() == 36) {
                    if ((m_trapActivated != 0 && m_trapDetected != 0)
                        || (m_dwFlags & 0x2) != 0) {
                        if ((m_dwFlags & 0x1) != 0) {
                            if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                                // NOTE: Uninline.
                                sub_48B600(pArea,
                                    pVidMode,
                                    a3,
                                    g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(0, 250, 0)));
                                bRendered = TRUE;
                            }

                            m_pArea->m_cInfinity.OutlinePoly(m_pOpenPolygon,
                                m_nOpenPolygon,
                                m_rOpenBounding,
                                RGB(0, 250, 0));
                        } else {
                            if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                                // NOTE: Uninline.
                                sub_48B3C0(pArea,
                                    pVidMode,
                                    a3,
                                    g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(0, 250, 0)));
                                bRendered = TRUE;
                            }

                            m_pArea->m_cInfinity.OutlinePoly(m_pClosedPolygon,
                                m_nClosedPolygon,
                                m_rClosedBounding,
                                RGB(0, 250, 0));
                        }
                    }
                }

                if (pGame->GetIconIndex() == 12
                    && (m_dwFlags & 0x1) == 0
                    && (m_dwFlags & 0x2) != 0) {
                    if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.m_bIs3dAccelerated) {
                        // NOTE: Uninline.
                        sub_48B3C0(pArea,
                            pVidMode,
                            a3,
                            g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(0, 250, 0)));
                        bRendered = TRUE;
                    }

                    m_pArea->m_cInfinity.OutlinePoly(m_pClosedPolygon,
                        m_nClosedPolygon,
                        m_rClosedBounding,
                        RGB(0, 250, 0));
                }
            }
        }
    }

    if (m_drawPoly > 0) {
        if ((m_dwFlags & 0x80) != 0 && (m_dwFlags & 0x100) != 0) {
            if (CInfinity::TRANSLUCENT_BLTS_ON
                && !g_pChitin->cVideo.m_bIs3dAccelerated
                && !bRendered) {
                if ((m_dwFlags & 0x1) != 0) {
                    if (m_nOpenPolygon > 0) {
                        // NOTE: Uninline.
                        sub_48B600(pArea,
                            pVidMode,
                            a3,
                            g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(255, 0, 255)));
                    }
                } else {
                    if (m_nClosedPolygon > 0) {
                        // NOTE: Uninline.
                        sub_48B3C0(pArea,
                            pVidMode,
                            a3,
                            g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(255, 0, 255)));
                    }
                }
            }

            if ((m_dwFlags & 0x1) != 0) {
                if (m_nOpenPolygon > 0) {
                    m_pArea->m_cInfinity.OutlinePoly(m_pOpenPolygon,
                        m_nOpenPolygon,
                        m_rOpenBounding,
                        RGB(255, 0, 255));
                }
            } else {
                if (m_nClosedPolygon > 0) {
                    m_pArea->m_cInfinity.OutlinePoly(m_pClosedPolygon,
                        m_nClosedPolygon,
                        m_rClosedBounding,
                        RGB(255, 0, 255));
                }
            }
        } else {
            if (CInfinity::TRANSLUCENT_BLTS_ON
                && !g_pChitin->cVideo.m_bIs3dAccelerated
                && !bRendered) {
                if ((m_dwFlags & 0x1) != 0) {
                    if (m_nOpenPolygon > 0) {
                        // NOTE: Uninline.
                        sub_48B600(pArea,
                            pVidMode,
                            a3,
                            g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(255, 0, 0)));
                    }
                } else {
                    if (m_nClosedPolygon > 0) {
                        // NOTE: Uninline.
                        sub_48B3C0(pArea,
                            pVidMode,
                            a3,
                            g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(255, 0, 0)));
                    }
                }
            }

            if ((m_dwFlags & 0x1) != 0) {
                if (m_nOpenPolygon > 0) {
                    m_pArea->m_cInfinity.OutlinePoly(m_pOpenPolygon,
                        m_nOpenPolygon,
                        m_rOpenBounding,
                        RGB(255, 0, 0));
                }
            } else {
                if (m_nClosedPolygon > 0) {
                    m_pArea->m_cInfinity.OutlinePoly(m_pClosedPolygon,
                        m_nClosedPolygon,
                        m_rClosedBounding,
                        RGB(255, 0, 0));
                }
            }
        }
    } else {
        if ((m_dwFlags & 0x80) != 0) {
            return;
        }

        // NOTE: Uninline.
        if (!m_pArea->m_visibility.IsTileExplored(m_pArea->m_visibility.PointToTile(m_pos))) {
            return;
        }

        if (g_pBaldurChitin->pActiveEngine != g_pBaldurChitin->m_pEngineWorld) {
            return;
        }

        if (g_pBaldurChitin->m_pEngineWorld->GetMenuKey() != TRUE) {
            return;
        }

        if (CInfinity::TRANSLUCENT_BLTS_ON
            && !g_pChitin->cVideo.m_bIs3dAccelerated
            && !bRendered) {
            if ((m_dwFlags & 0x1) != 0) {
                if (m_nOpenPolygon > 0) {
                    // NOTE: Uninline.
                    sub_48B600(pArea,
                        pVidMode,
                        a3,
                        g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(255, 0, 0)));
                }
            } else {
                if (m_nClosedPolygon > 0) {
                    // NOTE: Uninline.
                    sub_48B3C0(pArea,
                        pVidMode,
                        a3,
                        g_pBaldurChitin->GetCurrentVideoMode()->FUN_0079adf0(RGB(255, 0, 0)));
                }
            }
        }

        if ((m_dwFlags & 0x1) != 0) {
            if (m_nOpenPolygon > 0) {
                m_pArea->m_cInfinity.OutlinePoly(m_pOpenPolygon,
                    m_nOpenPolygon,
                    m_rOpenBounding,
                    RGB(255, 0, 0));
            }
        } else {
            if (m_nClosedPolygon > 0) {
                m_pArea->m_cInfinity.OutlinePoly(m_pClosedPolygon,
                    m_nClosedPolygon,
                    m_rClosedBounding,
                    RGB(255, 0, 0));
            }
        }
    }
}

// 0x4892B0
void CGameDoor::RemoveFromArea()
{
    m_pArea->m_lTiledObjects.RemoveAt(m_tiledObject.m_posAreaList);

    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1269
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x489330
void CGameDoor::SetCursor(LONG nToolTip)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if ((m_dwFlags & 0x80) == 0 || (m_dwFlags & 0x100) != 0) {
        switch (pGame->GetState()) {
        case 0:
            if (pGame->GetGroup()->GetCount() != 0) {
                g_pBaldurChitin->GetObjectCursor()->SetCursor(m_cursorType, FALSE);
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
                if ((m_dwFlags & 0x1) == 0 && (m_dwFlags & 0x2) != 0) {
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
            case 36:
                if (1) {
                    INT nNewCursor;
                    if (m_trapActivated != 0 && m_trapDetected != 0) {
                        nNewCursor = 38;
                    } else {
                        if ((m_dwFlags & 0x2) == 0) {
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
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
                // __LINE__: 1370
                UTIL_ASSERT(FALSE);
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 1380
            UTIL_ASSERT(FALSE);
        }
    } else {
        CGameObject::SetCursor(nToolTip);
    }
}

// 0x489680
void CGameDoor::ToggleDoor(const CAIObjectType& cause, BOOL bIgnoreLock)
{
    BOOLEAN bCanToggle = TRUE;

    // NOTE: Unused.
    CString sUnused;
    CSound sndUnused;

    if (!bIgnoreLock && (m_dwFlags & 0x2) != 0) {
        if (m_keyType == "") {
            STRREF strNotPickable;
            if ((m_dwFlags & 0x200) != 0 && m_strNotPickable > -1) {
                strNotPickable = m_strNotPickable;
            } else {
                strNotPickable = 15874;
            }

            if (strNotPickable != -1) {
                CMessageDisplayTextRefSend* pMessage = new CMessageDisplayTextRefSend(-1,
                    strNotPickable,
                    0,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    cause.m_nInstance);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
            return;
        }

        if (!PartyHasItem(m_keyType)) {
            CGameObject* pObject;
            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(cause.m_nInstance,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                STRREF strNotPickable;
                if ((m_dwFlags & 0x200) != 0 && m_strNotPickable > -1) {
                    strNotPickable = m_strNotPickable;
                } else {
                    strNotPickable = 15874;
                }

                if (strNotPickable != -1) {
                    CMessageDisplayTextRefSend* pMessage = new CMessageDisplayTextRefSend(-1,
                        strNotPickable,
                        0,
                        RGB(215, 215, 190),
                        -1,
                        m_id,
                        cause.m_nInstance);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
                }
                return;
            }

            if (pObject->GetObjectType() != CGameObject::TYPE_SPRITE) {
                STRREF strNotPickable;
                if ((m_dwFlags & 0x200) != 0 && m_strNotPickable > -1) {
                    strNotPickable = m_strNotPickable;
                } else {
                    strNotPickable = 15874;
                }

                if (strNotPickable != -1) {
                    CMessageDisplayTextRefSend* pMessage = new CMessageDisplayTextRefSend(-1,
                        strNotPickable,
                        0,
                        RGB(215, 215, 190),
                        -1,
                        m_id,
                        cause.m_nInstance);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(cause.m_nInstance,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return;
            }

            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

            CString sKey;
            m_keyType.CopyToString(sKey);

            if (pSprite->FindItemPersonal(sKey, 0, FALSE) == -1
                && pSprite->FindItemBags(sKey, 0, FALSE) == -1) {
                STRREF strNotPickable;
                if ((m_dwFlags & 0x200) != 0 && m_strNotPickable > -1) {
                    strNotPickable = m_strNotPickable;
                } else {
                    strNotPickable = 15874;
                }

                if (strNotPickable != -1) {
                    CMessageDisplayTextRefSend* pMessage = new CMessageDisplayTextRefSend(-1,
                        strNotPickable,
                        0,
                        RGB(215, 215, 190),
                        -1,
                        m_id,
                        cause.m_nInstance);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(cause.m_nInstance,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(cause.m_nInstance,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if ((m_dwFlags & 0x1) != 0) {
        // Close the door.
        if (_strnicmp(reinterpret_cast<char*>(m_pArea->m_header.m_areaName), "ar5004", 7) == 0) {
            if (!m_pArea->m_search.CanToggleDoor(m_pClosedSearch, m_nClosedSearch)) {
                // NOTE: Unused.
                CAIObjectType typeUnused;
                typeUnused.Set(CAIObjectType::ANYONE);

                CAIAction action;

                CGameObject* pObject = m_pArea->sub_47A1C0(m_id, m_pClosedSearch, m_nClosedSearch);
                if (pObject != NULL) {
                    // ChunkCreature(O:Object*)
                    action.m_actionID = 292;
                    action.m_acteeID.Set(pObject->GetAIType());

                    CMessageAddAction* pAddAction = new CMessageAddAction(action, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pAddAction, FALSE);

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    // CloseDoor(O:Object*)
                    action.m_actionID = 144;
                    action.m_acteeID.Set(m_typeAI);

                    pAddAction = new CMessageAddAction(action, m_id, cause.m_nInstance);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pAddAction, FALSE);
                }
                return;
            }
        } else {
            bCanToggle = m_pArea->m_search.CanToggleDoor(m_pClosedSearch, m_nClosedSearch);
            if (!bCanToggle) {
                return;
            }
        }

        if (!m_sndDoor.IsSoundPlaying()) {
            // NOTE: Uninline.
            if (m_closeSound != "") {
                m_sndDoor.SetResRef(m_closeSound, TRUE, TRUE);
            } else if ((m_dwFlags & 0x80) != 0) {
                m_sndDoor.SetResRef(CResRef("AMB_D04B"), TRUE, TRUE);
            } else {
                m_sndDoor.SetResRef(CResRef("AMB_D03B"), TRUE, TRUE);
            }

            m_sndDoor.SetChannel(2, reinterpret_cast<DWORD>(m_pArea));

            CPoint ear;
            LONG earZ;
            g_pBaldurChitin->cSoundMixer.GetListenPosition(ear, earZ);

            LONG priority = max(99 - 99 * ((ear.x - m_pos.x) * (ear.x - m_pos.x) / 256 + (ear.y - m_pos.y) * (ear.y - m_pos.y) / 144) / 6400, 0);
            m_sndDoor.SetPriority(static_cast<BYTE>(priority));

            m_sndDoor.Play(m_pos.x, m_pos.y, m_posZ, FALSE);

            CMessagePlaySoundRef* pPlaySound = new CMessagePlaySoundRef(m_sndDoor.GetResRef(), m_id, m_id);

            // NOTE: The original writes the channel without a NULL check.
            pPlaySound->m_nChannel = 2;

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pPlaySound, FALSE);
        }

        m_dwFlags &= ~0x1;
        m_pos = m_ptOpenDest;
        m_tiledObject.m_wAIState = CTiledObject::STATE_SECONDARY_TILE;

        if (m_pOpenSearch != NULL) {
            m_pArea->m_search.RemoveDoor(m_pOpenSearch, m_nOpenSearch);
        }

        if (m_pClosedSearch != NULL) {
            m_pArea->m_search.AddDoor(m_pClosedSearch,
                m_nClosedSearch,
                (m_dwFlags & 0x400) != 0);
        }

        if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
            || g_pChitin->cNetwork.m_idLocalPlayer == m_remotePlayerID) {
            CMessageDoorStatus* pDoorStatus = new CMessageDoorStatus(this, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pDoorStatus, FALSE);
        }

        if (m_trapActivated != 0) {
            CAITrigger trigger(CAITrigger::CLOSED, cause, 0);
            m_pendingTriggers.AddTail(new CAITrigger(trigger));

            if ((m_dwFlags & 0x4) == 0) {
                m_trapActivated = 0;

                CMessageDoorStatus* pDoorStatus = new CMessageDoorStatus(this, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pDoorStatus, FALSE);
            }
        }

        if (!bCanToggle) {
            return;
        }
    } else {
        // Open the door.
        if ((m_dwFlags & 0x1800) == 0x800) {
            STRREF strNotPickable = m_strNotPickable;
            if (strNotPickable > -1) {
                if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                    CMessageFloatText* pFloatText = new CMessageFloatText(m_id, m_id, strNotPickable, FALSE);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pFloatText, FALSE);
                } else {
                    FloatText(strNotPickable, 10, 5);
                }

                CMessageDisplayTextRefSend* pMessage = new CMessageDisplayTextRefSend(-1,
                    strNotPickable,
                    0,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    cause.m_nInstance);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }

            m_dwFlags |= 0x1000;
            return;
        }

        if (!m_sndDoor.IsSoundPlaying()) {
            // NOTE: Uninline.
            if (m_openSound != "") {
                m_sndDoor.SetResRef(m_openSound, TRUE, TRUE);
            } else if ((m_dwFlags & 0x80) != 0) {
                m_sndDoor.SetResRef(CResRef("AMB_D04A"), TRUE, TRUE);
            } else {
                m_sndDoor.SetResRef(CResRef("AMB_D03A"), TRUE, TRUE);
            }

            m_sndDoor.SetChannel(2, reinterpret_cast<DWORD>(m_pArea));

            CPoint ear;
            LONG earZ;
            g_pBaldurChitin->cSoundMixer.GetListenPosition(ear, earZ);

            LONG priority = max(99 - 99 * ((ear.x - m_pos.x) * (ear.x - m_pos.x) / 256 + (ear.y - m_pos.y) * (ear.y - m_pos.y) / 144) / 6400, 0);
            m_sndDoor.SetPriority(static_cast<BYTE>(priority));

            m_sndDoor.Play(m_pos.x, m_pos.y, m_posZ, FALSE);

            CMessagePlaySoundRef* pPlaySound = new CMessagePlaySoundRef(m_sndDoor.GetResRef(), m_id, m_id);

            // NOTE: The original writes the channel without a NULL check.
            pPlaySound->m_nChannel = 2;

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pPlaySound, FALSE);
        }

        m_dwFlags |= 0x1;
        m_pos = m_ptClosedDest;
        m_tiledObject.m_wAIState = CTiledObject::STATE_PRIMARY_TILE;

        if (m_pClosedSearch != NULL) {
            m_pArea->m_search.RemoveDoor(m_pClosedSearch, m_nClosedSearch);
        }

        if (m_pOpenSearch != NULL) {
            m_pArea->m_search.AddDoor(m_pOpenSearch,
                m_nOpenSearch,
                (m_dwFlags & 0x400) != 0);
        }

        if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
            || g_pChitin->cNetwork.m_idLocalPlayer == m_remotePlayerID) {
            CMessageDoorStatus* pDoorStatus = new CMessageDoorStatus(this, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pDoorStatus, FALSE);
        }

        if (m_trapActivated != 0) {
            CAITrigger trigger(CAITrigger::OPENED, cause, 0);
            m_pendingTriggers.AddTail(new CAITrigger(trigger));

            if ((m_dwFlags & 0x4) == 0) {
                m_trapActivated = 0;

                CMessageDoorStatus* pDoorStatus = new CMessageDoorStatus(this, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pDoorStatus, FALSE);
            }
        }
    }

    if ((m_dwFlags & 0x2) != 0) {
        m_dwFlags &= ~0x2;

        if ((m_dwFlags & 0x4000) != 0
            && m_keyType != ""
            && PartyHasItem(m_keyType)) {
            // TakePartyItem(S:Item*)
            m_curAction.m_actionID = 116;
            m_curAction.SetString1(m_keyType.GetResRefStr());
            TakePartyItem();
        }
    }
}

// 0x48AC80
void CGameDoor::OnDoorStatusUpdate(BOOLEAN bDoorOpened, DWORD dwFlags, WORD nTrapActivated, WORD nTrapDetected)
{
    if ((m_dwFlags & 0x1) == 0 || bDoorOpened) {
        if ((m_dwFlags & 0x1) == 0 && bDoorOpened == TRUE) {
            m_dwFlags |= 0x1;
            m_pos = m_ptClosedDest;
            m_tiledObject.m_wAIState = CTiledObject::STATE_PRIMARY_TILE;

            if (m_pClosedSearch != NULL) {
                m_pArea->m_search.RemoveDoor(m_pClosedSearch, m_nClosedSearch);
            }

            if (m_pOpenSearch != NULL) {
                m_pArea->m_search.AddDoor(m_pOpenSearch,
                    m_nOpenSearch,
                    (m_dwFlags & 0x400) != 0);
            }
        }
    } else {
        m_dwFlags &= ~0x1;
        m_pos = m_ptOpenDest;
        m_tiledObject.m_wAIState = CTiledObject::STATE_SECONDARY_TILE;

        if (m_pOpenSearch != NULL) {
            m_pArea->m_search.RemoveDoor(m_pOpenSearch, m_nOpenSearch);
        }

        if (m_pClosedSearch != NULL) {
            m_pArea->m_search.AddDoor(m_pClosedSearch,
                m_nClosedSearch,
                (m_dwFlags & 0x400) != 0);
        }
    }

    m_dwFlags = dwFlags;
    m_trapDetected = nTrapDetected;
    m_trapActivated = nTrapActivated;
}

// 0x48ADC0
void CGameDoor::Marshal(CAreaFileDoorObject** pDoorObject)
{
    CAreaPoint* pPoints;
    DWORD cnt;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 1904
    UTIL_ASSERT(pDoorObject != NULL);

    *pDoorObject = new CAreaFileDoorObject();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 1910
    UTIL_ASSERT(*pDoorObject != NULL);

    // FIXME: Redundant, memset is a part of constructor.
    memset(*pDoorObject, 0, sizeof(CAreaFileDoorObject));

    if (m_nClosedSearch > 0) {
        pPoints = new CAreaPoint[m_nClosedSearch];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1915
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nClosedSearch; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pClosedSearch[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pClosedSearch[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pDoorObject)->m_closedSearchSquaresStart = reinterpret_cast<DWORD>(pPoints);
        (*pDoorObject)->m_closedSearchSquaresCount = m_nClosedSearch;
    }

    if (m_nOpenSearch > 0) {
        pPoints = new CAreaPoint[m_nOpenSearch];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1927
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nOpenSearch; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pOpenSearch[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pOpenSearch[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pDoorObject)->m_openSearchSquaresStart = reinterpret_cast<DWORD>(pPoints);
        (*pDoorObject)->m_openSearchSquaresCount = m_nOpenSearch;
    }

    if (m_nClosedPolygon > 0) {
        pPoints = new CAreaPoint[m_nClosedPolygon];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1939
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nClosedPolygon; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pClosedPolygon[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pClosedPolygon[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pDoorObject)->m_closedSelectionPointStart = reinterpret_cast<DWORD>(pPoints);
        (*pDoorObject)->m_closedSelectionPointCount = m_nClosedPolygon;
    }

    if (m_nOpenPolygon > 0) {
        pPoints = new CAreaPoint[m_nOpenPolygon];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1951
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nOpenPolygon; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pOpenPolygon[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pOpenPolygon[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pDoorObject)->m_openSelectionPointStart = reinterpret_cast<DWORD>(pPoints);
        (*pDoorObject)->m_openSelectionPointCount = m_nOpenPolygon;
    }

    m_resID.GetResRef((*pDoorObject)->m_doorID);
    (*pDoorObject)->m_closedBoundingRectLeft = static_cast<WORD>(m_rClosedBounding.left);
    (*pDoorObject)->m_closedBoundingRectTop = static_cast<WORD>(m_rClosedBounding.top);
    (*pDoorObject)->m_closedBoundingRectRight = static_cast<WORD>(m_rClosedBounding.right) - 1;
    (*pDoorObject)->m_closedBoundingRectBottom = static_cast<WORD>(m_rClosedBounding.bottom) - 1;
    (*pDoorObject)->m_openBoundingRectLeft = static_cast<WORD>(m_rOpenBounding.left);
    (*pDoorObject)->m_openBoundingRectTop = static_cast<WORD>(m_rOpenBounding.top);
    (*pDoorObject)->m_openBoundingRectRight = static_cast<WORD>(m_rOpenBounding.right) - 1;
    (*pDoorObject)->m_openBoundingRectBottom = static_cast<WORD>(m_rOpenBounding.bottom) - 1;
    (*pDoorObject)->m_cursorType = m_cursorType;
    (*pDoorObject)->m_dwFlags = m_dwFlags;
    (*pDoorObject)->m_posXWalkTo1 = static_cast<WORD>(m_ptDest1.x);
    (*pDoorObject)->m_posYWalkTo1 = static_cast<WORD>(m_ptDest1.y);
    (*pDoorObject)->m_posXWalkTo2 = static_cast<WORD>(m_ptDest2.x);
    (*pDoorObject)->m_posYWalkTo2 = static_cast<WORD>(m_ptDest2.y);
    memcpy((*pDoorObject)->m_script, m_scriptRes, RESREF_SIZE);
    strncpy((*pDoorObject)->m_scriptName, m_scriptName, SCRIPTNAME_SIZE);
    (*pDoorObject)->m_hitPoints = m_hitPoints;
    (*pDoorObject)->m_armourClass = m_armourClass;
    m_openSound.GetResRef((*pDoorObject)->m_openSound);
    m_closeSound.GetResRef((*pDoorObject)->m_closeSound);
    (*pDoorObject)->m_trapDetectionDifficulty = m_trapDetectionDifficulty;
    (*pDoorObject)->m_trapDisarmingDifficulty = m_trapDisarmingDifficulty;
    (*pDoorObject)->m_trapActivated = m_trapActivated;
    (*pDoorObject)->m_trapDetected = m_trapDetected;
    (*pDoorObject)->m_posXTrapOrigin = m_posXTrapOrigin;
    (*pDoorObject)->m_posYTrapOrigin = m_posYTrapOrigin;
    m_keyType.GetResRef((*pDoorObject)->m_keyType);
    (*pDoorObject)->m_detectionDifficulty = m_detectionDifficulty;
    (*pDoorObject)->m_lockDifficulty = m_lockDifficulty;
    (*pDoorObject)->m_strNotPickable = m_strNotPickable;
}

// 0x48B2C0
const CPoint& CGameDoor::GetClosestDest(const CPoint& pt)
{
    LONG dx1 = pt.x - m_ptDest1.x;
    LONG dy1 = pt.y - m_ptDest1.y;
    LONG dx2 = pt.x - m_ptDest2.x;
    LONG dy2 = pt.y - m_ptDest2.y;

    // NOTE: Y distance is scaled by 4/3 (isometric).
    LONG nDist1 = dx1 * dx1 + (dy1 * dy1 * 16) / 9;
    LONG nDist2 = dx2 * dx2 + (dy2 * dy2 * 16) / 9;

    if (nDist1 < nDist2) {
        return m_ptDest1;
    }

    return m_ptDest2;
}

// 0x48B350
void CGameDoor::SetDrawPoly(SHORT time)
{
    if (m_drawPoly != time) {
        if (!g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly) {
            CMessageSetDrawPoly* pMessage = new CMessageSetDrawPoly(time, GetId(), GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
    }
    m_drawPoly = time;
}

// 0x48B3C0
void CGameDoor::sub_48B3C0(CGameArea* pArea, CVidMode* pVidMode, int a3, DWORD rgbColor)
{
    // TODO: Incomplete.
}

// 0x48B600
void CGameDoor::sub_48B600(CGameArea* pArea, CVidMode* pVidMode, int a3, DWORD rgbColor)
{
    // TODO: Incomplete.
}

// 0x453140
DWORD CGameDoor::GetFlags()
{
    return m_dwFlags;
}

// 0x453150
void CGameDoor::SetFlags(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
}

// 0x45B700
BOOL CGameDoor::sub_45B700()
{
    return m_dwFlags & 0x1;
}
