#include "CGameAIBase.h"

#include "CAIResponse.h"
#include "CAIScript.h"
#include "CAITrigger.h"
#include "CAIUtil.h"
#include "CBaldurChitin.h"
#include "CBaldurProjector.h"
#include "CGameArea.h"
#include "CGameContainer.h"
#include "CGameDoor.h"
#include "CGameEffect.h"
#include "CGameAnimation.h"
#include "CGameAnimationType.h"
#include "CGameObjectArray.h"
#include "CGameSave.h"
#include "CGameSound.h"
#include "CGameSpawning.h"
#include "CGameSprite.h"
#include "CGameSpriteEquipment.h"
#include "CGameStatic.h"
#include "CGameTiledObject.h"
#include "CGameTimer.h"
#include "CGameTrigger.h"
#include "CInfGame.h"
#include "CItem.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CScreenCharacter.h"
#include "CScreenInventory.h"
#include "CScreenWorld.h"
#include "CSpell.h"
#include "CTimerWorld.h"
#include "CUtil.h"
#include "CVariableHash.h"
#include "CVidCell.h"
#include "Icewind586B70.h"

#include <math.h>
#include <stdlib.h>

// 0x8485C4
const SHORT CGameAIBase::ACTION_DONE = -1;

// 0x8485C6
const SHORT CGameAIBase::ACTION_INTERRUPTABLE = 1;

// 0x8485C8
const SHORT CGameAIBase::ACTION_NORMAL = 0;

// 0x8485CA
const SHORT CGameAIBase::ACTION_ERROR = -2;

// 0x8485CC
const SHORT CGameAIBase::ACTION_NO_ACTION = 2;

// 0x8485CE
const SHORT CGameAIBase::ACTION_STOPPED = -3;

// 0x8485E8
const BYTE CGameAIBase::EFFECT_LIST_TIMED = 1;

// 0x8485E9
const BYTE CGameAIBase::EFFECT_LIST_EQUIPED = 2;

// 0x8D1408
const CString CGameAIBase::DEAD_GLOBAL_PREFIX("_DEAD");

// 0x8D1810
CAIAction CGameAIBase::m_aiAction;

// 0x44C4B0
CGameAIBase::CGameAIBase()
{
    m_nLastActionReturn = 0;
    m_objectType = CGameObject::TYPE_AIBASE;
    m_lAttacker.Set(CAIObjectType::NOONE);
    m_lOrderedBy.Set(CAIObjectType::NOONE);
    field_EA.Set(CAIObjectType::NOONE);
    field_126.Set(CAIObjectType::NOONE);
    field_162.Set(CAIObjectType::NOONE);
    m_lHitter.Set(CAIObjectType::NOONE);
    m_lHelp.Set(CAIObjectType::NOONE);
    m_lTrigger.Set(CAIObjectType::NOONE);
    m_lSeen.Set(CAIObjectType::NOONE);
    m_lTalkedTo.Set(CAIObjectType::NOONE);
    m_lHeard.Set(CAIObjectType::NOONE);
    field_306.Set(CAIObjectType::NOONE);
    field_342.Set(CAIObjectType::NOONE);
    field_37E.Set(CAIObjectType::NOONE);
    field_3BA.Set(CAIObjectType::NOONE);
    m_curResponseNum = -1;
    m_curResponseSetNum = -1;
    m_curScriptNum = -1;
    m_curAction = CAIAction::NULL_ACTION;
    m_interrupt = FALSE;
    m_actionCount = 0;
    field_54C = 0;
    field_550 = 0;
    field_552 = 0;
    field_44A = 0;
    m_overrideScript = NULL;
    m_special1Script = NULL;
    m_teamScript = NULL;
    m_special2Script = NULL;
    m_combatScript = NULL;
    m_special3Script = NULL;
    m_movementScript = NULL;
    m_inCutScene = FALSE;
    m_reactionRoll = 10;
    field_550 = rand() % 120;

    CAITrigger trigger(CAITrigger::ONCREATION, 0);
    m_pendingTriggers.AddTail(new CAITrigger(trigger));

    m_firstCall = TRUE;
    m_forceActionPick = FALSE;
    field_580 = 0;
    m_nLastActionReturn = -1;
    field_588 = 0;
    field_58C = 0;
    field_594 = 0;
    field_595 = 1;
    field_596 = 0;
    m_randValue = rand() & 0x7FFF;
}

// 0x44D160
CGameAIBase::~CGameAIBase()
{
    POSITION pos;

    pos = m_pendingTriggers.GetHeadPosition();
    while (pos != NULL) {
        CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
        if (pTrigger != NULL) {
            delete pTrigger;
        }
    }

    pos = m_queuedActions.GetHeadPosition();
    while (pos != NULL) {
        CAIAction* pAction = m_queuedActions.GetNext(pos);
        if (pAction != NULL) {
            delete pAction;
        }
    }

    pos = m_timers.GetHeadPosition();
    while (pos != NULL) {
        CGameTimer* pTimer = m_timers.GetNext(pos);
        if (pTimer != NULL) {
            delete pTimer;
        }
    }

    if (m_overrideScript != NULL) {
        delete m_overrideScript;
        m_overrideScript = NULL;
    }

    if (m_special1Script != NULL) {
        delete m_special1Script;
        m_special1Script = NULL;
    }

    if (m_teamScript != NULL) {
        delete m_teamScript;
        m_teamScript = NULL;
    }

    if (m_special2Script != NULL) {
        delete m_special2Script;
        m_special2Script = NULL;
    }

    if (m_combatScript != NULL) {
        delete m_combatScript;
        m_combatScript = NULL;
    }

    if (m_special3Script != NULL) {
        delete m_special3Script;
        m_special3Script = NULL;
    }

    if (m_movementScript != NULL) {
        delete m_movementScript;
        m_movementScript = NULL;
    }
}

// 0x44D100
const BYTE* CGameAIBase::GetVisibleTerrainTable()
{
    return CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE;
}

// 0x44D110
const BYTE* CGameAIBase::GetTerrainTable()
{
    return CGameObject::DEFAULT_TERRAIN_TABLE;
}

// 0x44D120
BOOLEAN CGameAIBase::CompressTime(DWORD deltaTime)
{
    CheckTimers(deltaTime / CTimerWorld::TIMESCALE_MSEC_PER_SEC);
    return TRUE;
}

// 0x44D4B0
void CGameAIBase::CheckTimers(LONG cycles)
{
    POSITION pos = m_timers.GetHeadPosition();
    while (pos != NULL) {
        POSITION oldPos = pos;
        CGameTimer* pTimer = m_timers.GetNext(pos);
        pTimer->m_time -= cycles;
        if (pTimer->m_time <= 0) {
            CAITrigger trigger(CAITrigger::TIMEREXPIRED, pTimer->m_id);

            // NOTE: Uninline.
            SetTrigger(trigger);

            m_timers.RemoveAt(pos);
            delete pTimer;
        }
    }
}

// 0x44D640
void CGameAIBase::ClearActions(BOOL a1)
{
    if (a1) {
        POSITION pos = m_queuedActions.GetHeadPosition();
        while (pos != NULL) {
            POSITION curr = pos;
            CAIAction* pAction = m_queuedActions.GetNext(pos);
            if (pAction != NULL) {
                if (pAction->m_actionID != CAIAction::SPELL
                    && pAction->m_actionID != CAIAction::SPELLPOINT
                    && pAction->m_actionID != CAIAction::FORCESPELL
                    && pAction->m_actionID != CAIAction::FORCESPELLPOINT
                    && pAction->m_actionID != CAIAction::APPLYSPELL
                    && pAction->m_actionID != CAIAction::REALLYFORCESPELL
                    && pAction->m_actionID != CAIAction::SPELLNODEC
                    && pAction->m_actionID != CAIAction::SPELLPOINTNODEC) {
                    m_queuedActions.RemoveAt(curr);
                    delete pAction;
                }
            }
        }
    } else {
        POSITION pos = m_queuedActions.GetHeadPosition();
        while (pos != NULL) {
            CAIAction* pAction = m_queuedActions.GetNext(pos);
            if (pAction != NULL) {
                delete pAction;
            }
        }
        m_queuedActions.RemoveAll();
    }

    if (m_queuedActions.GetCount() == 0) {
        m_curResponseNum = -1;
        m_curResponseSetNum = -1;
        m_curScriptNum = -1;
    }
}

// 0x44D730
void CGameAIBase::ClearTriggers()
{
    ApplyTriggers();

    POSITION pos = m_pendingTriggers.GetHeadPosition();
    while (pos != NULL) {
        CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
        if (pTrigger != NULL) {
            delete pTrigger;
        }
    }
    m_pendingTriggers.RemoveAll();
}

// 0x44D780
void CGameAIBase::DoAction()
{
    SHORT actionReturn = ExecuteAction();

    if (actionReturn == ACTION_DONE
        || actionReturn == ACTION_ERROR
        || actionReturn == ACTION_STOPPED) {
        CAIAction action;
        SetCurrAction(GetNextAction(action));
        ResetCurrResponse();
    } else if (m_interrupt && actionReturn == ACTION_INTERRUPTABLE) {
        CAIAction action;
        m_actionCount++;
        SetCurrAction(GetNextAction(action));
        m_interrupt = FALSE;
    } else {
        m_actionCount++;
    }
}

// 0x44DC10
SHORT CGameAIBase::ExecuteAction()
{
    // TODO: Incomplete.

    return ACTION_ERROR;
}

// 0x45BDD0
CGameObject* CGameAIBase::GetTargetShareType(BYTE type)
{
    // TODO: Incomplete.

    return NULL;
}

// 0x45C030
CGameObject* CGameAIBase::GetTargetShareType(const CAIObjectType& type, BYTE objectType)
{
    CGameObject* pObject = type.sub_40CCA0(this, FALSE);

    if (pObject != NULL
        && pObject->GetObjectType() == CGameObject::TYPE_SPRITE
        && static_cast<CGameSprite*>(pObject)->GetDerivedStats()->m_cImmunitiesAIType.OnList(m_typeAI)) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
    }

    UpdateTarget(pObject);

    if (pObject != NULL) {
        if ((pObject->GetObjectType() != objectType
                && objectType != CGameObject::TYPE_AIBASE
                && objectType != CGameObject::TYPE_NONE)
            || (objectType == CGameObject::TYPE_AIBASE
                && (pObject->GetObjectType() & CGameObject::TYPE_AIBASE) == 0)) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            UpdateTarget(NULL);
            return NULL;
        }

        if (GetObjectType() == CGameObject::TYPE_SPRITE
            && g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1
            && (static_cast<CGameSprite*>(this)->GetDerivedStats()->m_generalState & STATE_BLIND) != 0
            && pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            const CPoint& ptTargetPos = pObject->GetPos();
            CPoint ptTarget(ptTargetPos.x / CPathSearch::GRID_SQUARE_SIZEX,
                ptTargetPos.y / CPathSearch::GRID_SQUARE_SIZEY);
            CPoint ptSelf(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);

            LONG nSquares = CAIUtil::CountSquares(ptSelf, ptTarget);
            if (nSquares > (static_cast<CGameSprite*>(this)->GetAnimation()->GetPersonalSpace() >> 1) + 4) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                UpdateTarget(NULL);
                return NULL;
            }
        }
    }

    UpdateTarget(pObject);

    return pObject;
}

// 0x45C300
void CGameAIBase::InsertResponse(CAIResponse& response, BOOL checkCurrentResponse, BOOL clearActions)
{
    CAIAction action;
    CAIObjectType actorType;

    if (checkCurrentResponse
        && m_curResponseSetNum >= 0
        && m_curScriptNum >= 0
        && m_curResponseSetNum == response.m_responseSetNum
        && m_curScriptNum == response.m_scriptNum) {
        return;
    }

    if (clearActions) {
        ClearActions(FALSE);
    }

    m_curResponseNum = response.m_responseNum;
    m_curResponseSetNum = response.m_responseSetNum;
    m_curScriptNum = response.m_scriptNum;
    m_interrupt = TRUE;

    POSITION pos = response.m_actionList.GetHeadPosition();
    while (pos != NULL) {
        CAIAction* node = response.m_actionList.GetNext(pos);
        action = *node;

        CAIAction* newNode = new CAIAction();
        *newNode = action;
        m_queuedActions.AddTail(newNode);
    }
}

// 0x45C730
void CGameAIBase::ProcessAI()
{
    // TODO: Incomplete.
}

// 0x45CA10
void CGameAIBase::ProcessPendingTriggers()
{
    // TODO: Incomplete.
}

// 0x45D130
void CGameAIBase::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBase.cpp
        // __LINE__: 6886
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x44CC70
void CGameAIBase::AddAction(const CAIAction& action)
{
    CAIAction* copy = new CAIAction();
    copy->m_actionID = action.m_actionID;
    copy->m_specificID = action.m_specificID;
    copy->m_actorID.Set(action.m_actorID);
    copy->m_acteeID.Set(action.m_acteeID);
    copy->m_acteeID2.Set(action.m_acteeID2);
    copy->m_dest = action.m_dest;
    copy->m_specificID2 = action.m_specificID2;
    copy->m_specificID3 = action.m_specificID3;
    copy->m_string1 = action.m_string1;
    copy->m_string2 = action.m_string2;
    copy->m_internalFlags = action.m_internalFlags;
    m_queuedActions.AddHead(copy);
}

// 0x44CE20
void CGameAIBase::AddEffect(CGameEffect* pEffect, BYTE list, BOOL noSave, BOOL immediateApply)
{
    if (pEffect != NULL) {
        delete pEffect;
    }
}

// 0x44CE40
void CGameAIBase::ClearAI(BOOLEAN a1)
{
    ClearActions(FALSE);
    ApplyTriggers();

    POSITION pos = m_pendingTriggers.GetHeadPosition();
    while (pos != NULL) {
        CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
        if (pTrigger != NULL) {
            delete pTrigger;
        }
    }
    m_pendingTriggers.RemoveAll();

    SetCurrAction(CAIAction::NULL_ACTION);

    m_curAction.m_actionID = CAIAction::NULL_ACTION.m_actionID;
    m_curAction.m_specificID = CAIAction::NULL_ACTION.m_specificID;
    m_curAction.m_actorID.Set(CAIAction::NULL_ACTION.m_actorID);
    m_curAction.m_acteeID.Set(CAIAction::NULL_ACTION.m_acteeID);
    m_curAction.m_acteeID2.Set(CAIAction::NULL_ACTION.m_acteeID2);
    m_curAction.m_dest = CAIAction::NULL_ACTION.m_dest;
    m_curAction.m_specificID2 = CAIAction::NULL_ACTION.m_specificID2;
    m_curAction.m_specificID3 = CAIAction::NULL_ACTION.m_specificID3;
    m_curAction.m_string1 = CAIAction::NULL_ACTION.m_string1;
    m_curAction.m_string2 = CAIAction::NULL_ACTION.m_string2;
    m_curAction.m_internalFlags = CAIAction::NULL_ACTION.m_internalFlags;
}

// 0x44CF50
void CGameAIBase::InsertAction(const CAIAction& action)
{
    CAIAction* copy = new CAIAction();
    copy->m_actionID = action.m_actionID;
    copy->m_specificID = action.m_specificID;
    copy->m_actorID.Set(action.m_actorID);
    copy->m_acteeID.Set(action.m_acteeID);
    copy->m_acteeID2.Set(action.m_acteeID2);
    copy->m_dest = action.m_dest;
    copy->m_specificID2 = action.m_specificID2;
    copy->m_specificID3 = action.m_specificID3;
    copy->m_string1 = action.m_string1;
    copy->m_string2 = action.m_string2;
    copy->m_internalFlags = action.m_internalFlags;
    m_queuedActions.AddTail(copy);
}

// 0x45D190
void CGameAIBase::SetCurrAction(const CAIAction& action)
{
    m_actionCount = 0;
    m_interrupt = FALSE;
    m_curAction.m_actionID = action.m_actionID;
    m_curAction.m_specificID = action.m_specificID;
    m_curAction.m_actorID.Set(action.m_actorID);
    m_curAction.m_acteeID.Set(action.m_acteeID);
    m_curAction.m_acteeID2.Set(action.m_acteeID2);
    m_curAction.m_dest = action.m_dest;
    m_curAction.m_specificID2 = action.m_specificID2;
    m_curAction.m_specificID3 = action.m_specificID3;
    m_curAction.m_string1 = action.m_string1;
    m_curAction.m_string2 = action.m_string2;
    m_curAction.m_internalFlags = action.m_internalFlags;
    if (action.m_actionID != CAIAction::NO_ACTION
        && g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_lNoDecodeList.Find(action.m_actionID) == NULL) {
        m_curAction.Decode(this);
    }
}

// 0x45D280
void CGameAIBase::SetScript(SHORT level, CAIScript* script)
{
    switch (level) {
    case 0:
        if (m_overrideScript != NULL) {
            delete m_overrideScript;
        }
        m_overrideScript = script;
        break;
    case 1:
        if (m_special1Script != NULL) {
            delete m_special1Script;
        }
        m_special1Script = script;
        break;
    case 2:
        if (m_teamScript != NULL) {
            delete m_teamScript;
        }
        m_teamScript = script;
        break;
    case 3:
        if (m_special2Script != NULL) {
            delete m_special2Script;
        }
        m_special2Script = script;
        break;
    case 4:
        if (m_combatScript != NULL) {
            delete m_combatScript;
        }
        m_combatScript = script;
        break;
    case 5:
        if (m_special3Script != NULL) {
            delete m_special3Script;
        }
        m_special3Script = script;
        break;
    case 6:
        if (m_movementScript != NULL) {
            delete m_movementScript;
        }
        m_movementScript = script;
        break;
    }
}

// 0x45D3E0
void CGameAIBase::DecodeSpell(INT nID, CString& sSpell)
{
    if (nID == 0) {
        nID = field_588;
        if (nID == 0) {
            sSpell = "";
            return;
        }
    }

    CString sResRef("");
    sResRef += "SP";

    switch (nID / 1000) {
    case 1:
        sResRef += "PR";
        break;
    case 2:
        sResRef += "WI";
        break;
    case 3:
        sResRef += "IN";
        break;
    default:
        sResRef = "MARW";

        CString sError;
        sError.Format("Invalid or missing spell ID (check SPELL.IDS): %d", nID);
        break;
    }

    CString sNumber;
    sNumber.Format("%03d", nID % 1000);
    sResRef += sNumber;

    sSpell = sResRef;
}

// 0x45D6A0
void CGameAIBase::ApplyTriggers()
{
    CMessage* message;

    if ((g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime / 4) % 900 == 0) {
        message = new CMessageUpdateReaction(11, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    POSITION pos = m_pendingTriggers.GetHeadPosition();
    while (pos != NULL) {
        CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
        if ((pTrigger->m_flags & 0x4) == 0) {
            pTrigger->m_flags |= 0x4;

            switch (pTrigger->m_triggerID) {
            case CAITRIGGER_ATTACKEDBY:
                AutoPause(2);
                if (!m_lAttacker.Equal(pTrigger->GetCause())
                    && (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1
                        || g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pTrigger->GetCause().GetInstance()) != -1)) {
                    m_lAttacker.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_ATTACKEDBY,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_HELP:
                if (!m_lHelp.Equal(pTrigger->GetCause())) {
                    m_lHelp.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_HELP,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_RECEIVEDORDER:
                if (!m_lOrderedBy.Equal(pTrigger->GetCause())) {
                    m_lOrderedBy.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(), 6, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_SAID:
                if (!m_lTalkedTo.Equal(pTrigger->GetCause())) {
                    m_lTalkedTo.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_SAID,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_HITBY:
                m_lAttackStyle = pTrigger->m_specificID;
                if (!m_lHitter.Equal(pTrigger->GetCause())) {
                    m_lHitter.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_HITBY,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_HEARD:
                if (!m_lHeard.Equal(pTrigger->GetCause())) {
                    m_lHeard.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_HEARD,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            }

            if (g_pBaldurChitin->GetObjectGame()->m_saveObjectList.Find(pTrigger->m_triggerID) != NULL
                && !m_lTrigger.Equal(pTrigger->GetCause())) {
                m_lTrigger.Set(pTrigger->GetCause());
                message = new CMessageSetLastObject(pTrigger->GetCause(), 0, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    }
}

// 0x45DED0
BOOL CGameAIBase::CheckAppropriateTarget(CSpell* pSpell, CGameAIBase* pTarget)
{

    pSpell->Demand();
    if (pTarget->GetObjectType() != TYPE_SPRITE) {
        pSpell->Release();
        return FALSE;
    }

    SPELL_ABILITY* ability = pSpell->GetAbility(0);
    if (ability != NULL
        && (ability->actionType != 1
            || (static_cast<CGameSprite*>(pTarget)->GetDerivedStats()->m_generalState & 0x800) == 0
            || pSpell->GetGenericName() == 12117
            || pSpell->GetGenericName() == 25765
            || pSpell->GetGenericName() == 32393)) {
        pSpell->Release();
        return TRUE;
    }

    pSpell->Release();
    return FALSE;
}

// 0x45DF70
BOOL CGameAIBase::PartyHasItem(const CResRef& resRef)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    for (SHORT nPortrait = 0; nPortrait < pGame->GetNumCharacters(); nPortrait++) {
        LONG nCharacterId = pGame->GetCharacterId(nPortrait);

        CGameSprite* pSprite;
        BYTE rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            break;
        }

        CString sName;
        resRef.CopyToString(sName);

        if (pSprite->FindItemPersonal(sName, 0, FALSE) != -1) {
            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return TRUE;
        }

        if (pSprite->FindItemBags(sName, 0, FALSE) != -1) {
            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return TRUE;
        }

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
    return FALSE;
}

// 0x45E100
void CGameAIBase::ApplyEffectToParty(CGameEffect* pEffect)
{
    CMessage* message;
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    for (SHORT nPortrait = 0; nPortrait < pGame->GetNumCharacters(); nPortrait++) {
        LONG nCharacterId = pGame->GetCharacterId(nPortrait);
        if (pGame->GetGameSave()->field_1AC) {
            CGameSprite* pSprite;
            BYTE rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                if (pSprite->InControl()) {
                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    message = new CMessageAddEffect(pEffect, m_id, nCharacterId);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                } else {
                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        } else {
            message = new CMessageAddEffect(pEffect, m_id, nCharacterId);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }
}

// 0x799E60
void CGameAIBase::AutoPause(DWORD type)
{
}

// 0x799E60
void CGameAIBase::UpdateTarget(CGameObject* pTarget)
{
}

// 0x49FC40
BOOL CGameAIBase::GetCanSeeInvisible()
{
    return TRUE;
}

// 0x45F2A0
SHORT CGameAIBase::MoveView(CPoint dest, int speed)
{
    CGameArea* pVisibleArea = g_pBaldurChitin->GetObjectGame()->GetVisibleArea();

    INT x;
    INT y;
    pVisibleArea->GetInfinity()->GetViewPosition(x, y);

    if (dest.x < 0) {
        dest.x = 0;
    }

    if (dest.y < 0) {
        dest.y = 0;
    }

    int maxX = pVisibleArea->GetInfinity()->rViewPort.left
        - pVisibleArea->GetInfinity()->rViewPort.right
        + pVisibleArea->GetInfinity()->nAreaX;
    if (dest.x > maxX) {
        dest.x = maxX;
    }

    int maxY = pVisibleArea->GetInfinity()->rViewPort.top
        - pVisibleArea->GetInfinity()->rViewPort.bottom
        + pVisibleArea->GetInfinity()->nAreaY;
    if (dest.y > maxY) {
        dest.y = maxY;
    }

    if (m_curAction.m_actionID == CAIACTION_MOVEVIEWPOINTUNTILDONE) {
        if (!field_594) {
            CMessageStartScroll* pMessage = new CMessageStartScroll(pVisibleArea,
                CPoint(x, y),
                dest,
                static_cast<BYTE>(speed),
                m_id,
                m_id);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            field_594 = TRUE;
        }

        if (x != dest.x && y != dest.y) {
            return ACTION_NORMAL;
        }

        field_594 = FALSE;
    } else {
        CMessageStartScroll* pMessage = new CMessageStartScroll(pVisibleArea,
            CPoint(x, y),
            dest,
            static_cast<BYTE>(speed),
            m_id,
            m_id);

        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return ACTION_DONE;
}

// 0x45F5D0
SHORT CGameAIBase::MoveViewPoint()
{
    CPoint dest(m_curAction.m_dest);

    CRect viewPort(GetArea()->GetInfinity()->rViewPort);
    dest.x -= viewPort.Width() / 2;
    dest.y -= viewPort.Height() / 2;

    if (dest.x < 0) {
        dest.x = 0;
    }

    if (dest.x > GetArea()->GetInfinity()->nAreaX) {
        dest.x = GetArea()->GetInfinity()->nAreaX;
    }

    if (dest.y < 0) {
        dest.y = 0;
    }

    if (dest.y > GetArea()->GetInfinity()->nAreaY) {
        dest.y = GetArea()->GetInfinity()->nAreaY;
    }

    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    return MoveView(dest, speed);
}

// 0x45F660
SHORT CGameAIBase::MoveViewObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest(target->GetPos());

    CRect viewPort(target->GetArea()->GetInfinity()->rViewPort);
    dest.x -= viewPort.Width() / 2;
    dest.y -= viewPort.Height() / 2;

    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    return MoveView(dest, speed);
}

// 0x45F6D0
SHORT CGameAIBase::MoveCursor(CPoint dest, SHORT speed)
{
    // TODO: Incomplete.

    return ACTION_DONE;
}

// 0x45F900
SHORT CGameAIBase::MoveCursorPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    return MoveCursor(dest, speed);
}

// NOTE: Inlined.
SHORT CGameAIBase::ClickLButton(CPoint pt)
{
    g_pBaldurChitin->m_pEngineWorld->OnLButtonDown(pt);
    g_pBaldurChitin->m_pEngineWorld->OnLButtonUp(pt);
    return ACTION_DONE;
}

// 0x45F920
SHORT CGameAIBase::ClickLButtonPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return ClickLButton(dest);
}

// 0x45F980
SHORT CGameAIBase::ClickLButtonObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest = target->GetPos();
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return ClickLButton(dest);
}

// NOTE: Inlined.
SHORT CGameAIBase::DoubleClickLButton(CPoint pt)
{
    g_pBaldurChitin->m_pEngineWorld->OnLButtonDown(pt);
    g_pBaldurChitin->m_pEngineWorld->OnLButtonUp(pt);
    g_pBaldurChitin->m_pEngineWorld->OnLButtonDblClk(pt);
    g_pBaldurChitin->m_pEngineWorld->OnLButtonUp(pt);
    return ACTION_DONE;
}

// 0x45F9F0
SHORT CGameAIBase::DoubleClickLButtonPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return DoubleClickLButton(dest);
}

// 0x45FA70
SHORT CGameAIBase::DoubleClickLButtonObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest = target->GetPos();
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return DoubleClickLButton(dest);
}

// NOTE: Inlined.
SHORT CGameAIBase::ClickRButton(CPoint pt)
{
    g_pBaldurChitin->m_pEngineWorld->OnRButtonDown(pt);
    g_pBaldurChitin->m_pEngineWorld->OnRButtonUp(pt);
    return ACTION_DONE;
}

// 0x45FB10
SHORT CGameAIBase::ClickRButtonPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return ClickRButton(dest);
}

// 0x45FB70
SHORT CGameAIBase::ClickRButtonObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest = target->GetPos();
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return ClickRButton(dest);
}

// NOTE: Inlined.
SHORT CGameAIBase::DoubleClickRButton(CPoint pt)
{
    g_pBaldurChitin->m_pEngineWorld->OnRButtonDown(pt);
    g_pBaldurChitin->m_pEngineWorld->OnRButtonUp(pt);
    g_pBaldurChitin->m_pEngineWorld->OnRButtonDblClk(pt);
    g_pBaldurChitin->m_pEngineWorld->OnRButtonUp(pt);
    return ACTION_DONE;
}

// 0x45FBF0
SHORT CGameAIBase::DoubleClickRButtonPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return DoubleClickRButton(dest);
}

// 0x45FC80
SHORT CGameAIBase::DoubleClickRButtonObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest = target->GetPos();
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return DoubleClickLButton(dest);
}

// 0x45FD20
SHORT CGameAIBase::ChangeAIScript()
{
    CAIScript* script = new CAIScript(CResRef(m_curAction.GetString1()));
    SetScript(static_cast<SHORT>(m_curAction.m_specificID), script);

    if (GetObjectType() == TYPE_CONTAINER) {
        static_cast<CGameContainer*>(this)->SetScriptRes(m_curAction.GetString1());
    }

    return ACTION_DONE;
}

// 0x45FED0
SHORT CGameAIBase::StartTimer()
{
    BYTE id = static_cast<BYTE>(m_curAction.m_specificID);
    LONG time = m_curAction.m_specificID2;

    POSITION pos = m_timers.GetHeadPosition();
    while (pos != NULL) {
        CGameTimer* pTimer = m_timers.GetNext(pos);
        if (pTimer->m_id == id) {
            pTimer->m_time = time;
            return ACTION_DONE;
        }
    }

    CGameTimer* pTimer = new CGameTimer();
    pTimer->m_time = time;
    pTimer->m_id = id;
    m_timers.AddTail(pTimer);

    return ACTION_DONE;
}

// 0x45FF40
SHORT CGameAIBase::SendTrigger(CGameAIBase* sprite)
{
    if (sprite == NULL) {
        return ACTION_ERROR;
    }

    CAITrigger trigger(CAITrigger::TRIGGER, m_typeAI, m_curAction.m_specificID);

    CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger,
        m_id,
        sprite->GetId());

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x4600B0
SHORT CGameAIBase::Wait()
{
    if (m_actionCount == 0) {
        m_curAction.m_specificID *= 15;
    }

    m_curAction.m_specificID--;

    if (m_curAction.m_specificID > 0) {
        return ACTION_INTERRUPTABLE;
    }

    return ACTION_DONE;
}

// 0x4600F0
SHORT CGameAIBase::SmallWait()
{
    m_curAction.m_specificID--;

    if (m_curAction.m_specificID > 0) {
        return ACTION_INTERRUPTABLE;
    }

    return ACTION_DONE;
}

// 0x460110
SHORT CGameAIBase::Shout()
{
    CTypedPtrList<CPtrList, LONG*> targets;

    SHORT range = m_curAction.m_actionID != CAIACTION_212
        ? GetVisualRange()
        : SHORT_MAX;

    m_pArea->GetAllInRange(m_pos,
        CAIObjectType::ANYONE,
        range,
        GetTerrainTable(),
        targets,
        FALSE,
        FALSE);

    CAITrigger trigger(CAITrigger::HEARD, m_typeAI, m_curAction.m_specificID);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nId = reinterpret_cast<LONG>(targets.GetNext(pos));

        CMessage* message = new CMessageSetTrigger(trigger, m_id, nId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    return ACTION_DONE;
}

// 0x463310
void CGameAIBase::PutItemGround(CItem* pItem)
{
    LONG nContainerId = g_pBaldurChitin->GetObjectGame()->GetGroundPile(m_id);
    if (nContainerId != CGameObjectArray::INVALID_INDEX) {
        CGameContainer* pContainer;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            pContainer->PlaceItemInBlankSlot(pItem, TRUE, SHORT_MAX);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nContainerId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x463740
SHORT CGameAIBase::ClearActions(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CMessage* message = new CMessageClearActions(m_id, target->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    return ACTION_DONE;
}

// 0x465110
SHORT CGameAIBase::TakePartyGold()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    DWORD gold = m_curAction.m_specificID;
    if (gold > pGame->GetGameSave()->m_nPartyGold) {
        gold = pGame->GetGameSave()->m_nPartyGold;
    }

    if (m_objectType == TYPE_SPRITE) {
        static_cast<CGameSprite*>(this)->GetBaseStats()->m_gold += gold;
        static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold += gold;
    }

    CMessagePartyGold* pMessage = new CMessagePartyGold(TRUE,
        TRUE,
        -static_cast<LONG>(gold),
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x4651A0
SHORT CGameAIBase::GivePartyGold()
{
    DWORD gold = m_curAction.m_specificID;

    if (m_objectType == TYPE_SPRITE && m_curAction.m_actionID == CAIAction::GIVEPARTYGOLD) {
        if (gold > static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold) {
            gold = static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold;
        }

        static_cast<CGameSprite*>(this)->GetBaseStats()->m_gold -= gold;
        static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold -= gold;
    }

    CMessagePartyGold* pMessage = new CMessagePartyGold(TRUE,
        TRUE,
        gold,
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x4654C0
SHORT CGameAIBase::GiveOrder(CGameAIBase* sprite)
{
    if (sprite == NULL) {
        return ACTION_ERROR;
    }

    CAITrigger trigger(CAITrigger::RECEIVEDORDER, m_typeAI, m_curAction.m_specificID);

    CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger,
        m_id,
        sprite->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x465630
SHORT CGameAIBase::DisplayString(CGameAIBase* sprite)
{
    STR_RES strRes;
    COLORREF rgbColor = RGB(0, 0, 0);
    STRREF name;

    if (sprite == NULL) {
        return ACTION_ERROR;
    }

    if (sprite->GetObjectType() == TYPE_SPRITE) {
        rgbColor = CVidPalette::RANGE_COLORS[static_cast<CGameSprite*>(sprite)->GetBaseStats()->m_colors[2]];
        name = static_cast<CGameSprite*>(sprite)->GetNameRef();
    } else {
        name = -1;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(m_curAction.m_specificID, strRes);
    strRes.szText.TrimLeft();

    if (sprite->m_typeAI.GetEnemyAlly() == CAIObjectType::EA_PC) {
        if (strRes.cSound.m_nLooping == 0) {
            strRes.cSound.SetFireForget(TRUE);
        }

        if (sprite->GetObjectType() == TYPE_SPRITE) {
            strRes.cSound.SetChannel(static_cast<CGameSprite*>(sprite)->GetChannel(),
                reinterpret_cast<DWORD>(m_pArea));
        } else {
            strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
        }

        strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);

        if (strRes.szText != "") {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(name,
                m_curAction.m_specificID,
                rgbColor,
                RGB(160, 200, 215),
                -1,
                m_id,
                sprite->GetId());
        }
    } else {
        if (strRes.cSound.m_nLooping == 0) {
            strRes.cSound.SetFireForget(TRUE);
        }

        if (sprite->GetObjectType() == TYPE_SPRITE) {
            strRes.cSound.SetChannel(static_cast<CGameSprite*>(sprite)->GetChannel(),
                reinterpret_cast<DWORD>(m_pArea));
        } else {
            strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
        }

        strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);

        if (strRes.szText != "") {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(name,
                m_curAction.m_specificID,
                rgbColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                sprite->GetId());
        }
    }

    return ACTION_DONE;
}

// 0x465F30
SHORT CGameAIBase::StartMovie()
{
    CString sMovieFileName;

    CBaldurProjector* pProjector = g_pBaldurChitin->m_pEngineProjector;
    if (!pProjector->ResolveMovieFileName(CResRef(m_curAction.GetString1()), sMovieFileName)) {
        return ACTION_ERROR;
    }

    g_pBaldurChitin->m_pEngineWorld->ReadyMovie(CResRef(m_curAction.GetString1()), FALSE);

    return ACTION_DONE;
}

// 0x466030
SHORT CGameAIBase::RevealAreaOnMap()
{
    CString sArea = m_curAction.GetString1();

    CWorldMap* pWorldMap = g_pBaldurChitin->GetObjectGame()->GetWorldMap(sArea);
    pWorldMap->EnableArea(pWorldMap->sub_55A3A0(),
        CResRef(sArea),
        TRUE);

    g_pBaldurChitin->GetObjectGame()->FeedBack(CInfGame::FEEDBACK_WORLDMAP_UPDATE, 0, TRUE);

    g_pBaldurChitin->GetBaldurMessage()->SendMapWorldRevealArea(m_curAction.GetString1());

    return ACTION_DONE;
}

// 0x466120
SHORT CGameAIBase::ChangeTileState(CGameTiledObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    if (m_curAction.m_specificID != 0) {
        if ((target->m_dwFlags & 0x1) != 0) {
            target->ToggleState();
        }
    } else {
        if ((target->m_dwFlags & 0x1) == 0) {
            target->ToggleState();
        }
    }

    return ACTION_DONE;
}

// 0x466170
SHORT CGameAIBase::TriggerActivation(CGameTrigger* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    if (m_curAction.m_specificID != 0) {
        target->m_dwFlags &= ~0x100;
    } else {
        target->m_dwFlags |= 0x100;
    }

    CMessageTriggerStatus* pMessage = new CMessageTriggerStatus(target->m_dwFlags,
        target->m_trapActivated,
        target->m_trapDetected,
        m_id,
        target->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x4668B0
SHORT CGameAIBase::StartMusic()
{
    if (m_pArea == NULL) {
        return ACTION_ERROR;
    }

    // TODO: Check cast.
    m_pArea->PlaySong(static_cast<SHORT>(m_curAction.m_specificID), m_curAction.m_specificID2);

    return ACTION_DONE;
}

// 0x4668E0
SHORT CGameAIBase::SetMusic()
{
    if (m_pArea == NULL) {
        return ACTION_ERROR;
    }

    if (!m_pArea->SetSong(static_cast<SHORT>(m_curAction.m_specificID), static_cast<BYTE>(m_curAction.m_specificID2))) {
        return ACTION_ERROR;
    }

    return ACTION_DONE;
}

// 0x466A00
SHORT CGameAIBase::FinalSave()
{
    if (g_pChitin->cNetwork.GetSessionOpen()
        && g_pChitin->cNetwork.GetSessionHosting() != TRUE) {
        return ACTION_DONE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE) != CGameObjectArray::SUCCESS) {
        return ACTION_DONE;
    }

    // NOTE: Looks like inlining.
    if (1) {
        CString sSaveName("000000002-Final-Save");
        g_pBaldurChitin->GetObjectGame()->m_sSaveGame = sSaveName;
        CScreenCharacter::SAVE_NAME = sSaveName;
    }

    g_pBaldurChitin->GetObjectGame()->SaveGame(1, 0, 1);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return ACTION_ERROR;
    }

    return ACTION_DONE;
}

// 0x466B30
SHORT CGameAIBase::Unlock(CGameAIBase* pObject)
{
    CMessage* message;

    if (pObject == NULL) {
        return ACTION_ERROR;
    }

    if (pObject->GetObjectType() != TYPE_DOOR
        && pObject->GetObjectType() != TYPE_CONTAINER) {
        return ACTION_ERROR;
    }

    message = new CMessageSetForceActionPick(TRUE, m_id, pObject->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    CAITrigger trigger(CAITrigger::NO_TRIGGER, 0);

    if (pObject->GetObjectType() == TYPE_DOOR
        && (static_cast<CGameDoor*>(pObject)->m_dwFlags & 0x2) != 0) {
        trigger = CAITrigger(CAITrigger::UNLOCKED, m_typeAI, 0);

        message = new CMessageSetTrigger(trigger, m_id, pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        static_cast<CGameDoor*>(pObject)->m_dwFlags &= ~0x2;

        message = new CMessageUnlock(static_cast<CGameDoor*>(pObject)->m_dwFlags,
            m_id,
            pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    if (pObject->GetObjectType() == TYPE_CONTAINER
        && (static_cast<CGameContainer*>(pObject)->m_dwFlags & 0x1) != 0) {
        trigger = CAITrigger(CAITrigger::UNLOCKED, m_typeAI, 0);

        message = new CMessageSetTrigger(trigger, m_id, pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        message = new CMessageUnlock(static_cast<CGameContainer*>(pObject)->m_dwFlags & ~0x1,
            m_id,
            pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    return ACTION_DONE;
}

// 0x466F90
SHORT CGameAIBase::MoveGlobal(CGameSprite* pSprite)
{
    if (pSprite == NULL) {
        return ACTION_ERROR;
    }

    CString sArea = m_curAction.GetString1();
    sArea.MakeUpper();

    CMessage* message = new CMessageMoveGlobal(sArea,
        m_curAction.m_dest,
        m_id,
        pSprite->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    return ACTION_DONE;
}

// 0x467110
SHORT CGameAIBase::Lock(CGameAIBase* pObject)
{
    if (pObject == NULL) {
        return ACTION_ERROR;
    }

    if (pObject->GetObjectType() != TYPE_DOOR
        && pObject->GetObjectType() != TYPE_CONTAINER) {
        return ACTION_ERROR;
    }

    CMessageSetForceActionPick* pSetForceActionPick = new CMessageSetForceActionPick(TRUE,
        m_id,
        pObject->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pSetForceActionPick, FALSE);

    CAITrigger trigger(CAITrigger::NO_TRIGGER, 0);

    if (pObject->GetObjectType() == TYPE_DOOR) {
        CGameDoor* pDoor = static_cast<CGameDoor*>(pObject);
        if ((pDoor->m_dwFlags & 0x2) == 0) {
            trigger = CAITrigger(CAITrigger::UNLOCKED, m_typeAI, 0);

            CMessageSetTrigger* pSetTrigger = new CMessageSetTrigger(trigger,
                m_id,
                pDoor->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pSetTrigger, FALSE);

            pDoor->m_dwFlags |= 0x2;

            CMessageUnlock* pUnlock = new CMessageUnlock(pDoor->m_dwFlags,
                m_id,
                pDoor->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pUnlock, FALSE);
        }
    }

    if (pObject->GetObjectType() == TYPE_CONTAINER) {
        CGameContainer* pContainer = static_cast<CGameContainer*>(pObject);
        if ((pContainer->m_dwFlags & 0x1) == 0) {
            trigger = CAITrigger(CAITrigger::UNLOCKED, m_typeAI, 0);

            CMessageSetTrigger* pSetTrigger = new CMessageSetTrigger(trigger,
                m_id,
                pContainer->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pSetTrigger, FALSE);

            CMessageUnlock* pUnlock = new CMessageUnlock(pContainer->m_dwFlags | 0x1,
                m_id,
                pContainer->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pUnlock, FALSE);
        }
    }

    return ACTION_DONE;
}

// 0x467550
SHORT CGameAIBase::DestroyItem()
{
    if (GetObjectType() == TYPE_SPRITE) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(this);
        SHORT slotNum = pSprite->FindItemPersonal(m_curAction.GetString1(), 0, FALSE);
        if (slotNum != -1) {
            pSprite->Unequip(slotNum);
            g_pBaldurChitin->GetObjectGame()->AddDisposableItem(pSprite->GetEquipment()->m_items[slotNum]);
            pSprite->GetEquipment()->m_items[slotNum] = NULL;
            return ACTION_DONE;
        }

        if (pSprite->TakeItemBags(m_curAction.GetString1(), 1, -1) > 0) {
            return ACTION_DONE;
        }

        return ACTION_ERROR;
    }

    if (GetObjectType() == TYPE_CONTAINER) {
        CGameContainer* pContainer = static_cast<CGameContainer*>(this);
        SHORT slotNum = pContainer->FindItemSlot(CResRef(m_curAction.GetString1()));
        if (slotNum != -1) {
            pContainer->GetItem(slotNum);
            pContainer->SetItem(slotNum, NULL);
            pContainer->CompressContainer();
        }
        return ACTION_DONE;
    }

    return ACTION_ERROR;
}

// 0x467720
SHORT CGameAIBase::DetectSecretDoor(CGameDoor* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CGameDoor* pDoor;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(target->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pDoor),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return ACTION_ERROR;
    }

    if ((pDoor->m_dwFlags & 0x80) != 0) {
        if ((pDoor->m_dwFlags & 0x100) == 0) {
            pDoor->SetDrawPoly(400);

            pDoor->m_dwFlags |= 0x100;

            CMessageDoorStatus* pDoorStatus = new CMessageDoorStatus(pDoor,
                m_id,
                target->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pDoorStatus, FALSE);
        } else {
            pDoor->SetDrawPoly(400);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(target->GetId(),
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return ACTION_DONE;
}

// 0x467880
SHORT CGameAIBase::FadeToColor()
{
    CMessageFadeColor* pFadeColor = new CMessageFadeColor(TRUE,
        static_cast<BYTE>(m_curAction.m_dest.x),
        static_cast<BYTE>(m_curAction.m_dest.y),
        static_cast<BYTE>(m_curAction.m_specificID),
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pFadeColor, FALSE);

    return ACTION_DONE;
}

// 0x467900
SHORT CGameAIBase::FadeFromColor()
{
    CMessageFadeColor* pFadeColor = new CMessageFadeColor(FALSE,
        static_cast<BYTE>(m_curAction.m_dest.x),
        static_cast<BYTE>(m_curAction.m_dest.y),
        static_cast<BYTE>(m_curAction.m_specificID),
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pFadeColor, FALSE);

    return ACTION_DONE;
}

// 0x467970
SHORT CGameAIBase::sub_467970()
{
    CMessageFadeColor* pFadeColor = new CMessageFadeColor(255,
        static_cast<BYTE>(m_curAction.m_dest.x),
        static_cast<BYTE>(m_curAction.m_dest.y),
        static_cast<BYTE>(m_curAction.m_specificID),
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pFadeColor, FALSE);

    return ACTION_DONE;
}

// 0x4679E0
SHORT CGameAIBase::SpawnPtActivate(CGameSpawning* target)
{
    if (target != NULL) {
        return ACTION_ERROR;
    }

    if (target->GetObjectType() != TYPE_SPAWNING) {
        return ACTION_ERROR;
    }

    if (!target->m_spawningObject.m_activated) {
        CMessageSpawnPtActivate* pMessage = new CMessageSpawnPtActivate(TRUE,
            m_id,
            target->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return ACTION_DONE;
}

// 0x467A80
SHORT CGameAIBase::SpawnPtDeactivate(CGameSpawning* target)
{
    if (target != NULL) {
        return ACTION_ERROR;
    }

    if (target->GetObjectType() != TYPE_SPAWNING) {
        return ACTION_ERROR;
    }

    if (target->m_spawningObject.m_activated) {
        CMessageSpawnPtActivate* pMessage = new CMessageSpawnPtActivate(FALSE,
            m_id,
            target->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return ACTION_DONE;
}

// 0x467B10
SHORT CGameAIBase::SpawnPtSpawn(CGameSpawning* target)
{
    if (target != NULL) {
        return ACTION_ERROR;
    }

    if (target->GetObjectType() != TYPE_SPAWNING) {
        return ACTION_ERROR;
    }

    if (target->m_spawningObject.m_activated) {
        CMessageSpawnPtSpawn* pMessage = new CMessageSpawnPtSpawn(m_pos,
            m_id,
            target->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return ACTION_DONE;
}

// 0x467BB0
SHORT CGameAIBase::StaticStart(CGameStatic* target, BOOL bStart)
{
    if (target != NULL) {
        return ACTION_ERROR;
    }

    if (target->GetObjectType() != TYPE_STATIC) {
        return ACTION_ERROR;
    }

    // TODO: Check, not sure if that's right.
    if (bStart == (target->m_header.m_dwFlags & 0x8)) {
        CMessageStaticStart* pMessage = new CMessageStaticStart(bStart,
            m_id,
            target->GetId());
    }

    return ACTION_DONE;
}

// 0x460D60
SCRIPTNAME& CGameAIBase::GetScriptName()
{
    return m_scriptName;
}

// 0x50A400
void CGameAIBase::SetTrigger(const CAITrigger& trigger)
{
    m_pendingTriggers.AddTail(new CAITrigger(trigger));
}

// NOTE: Inlined.
void CGameAIBase::SetDefaultScript(CAIScript* script)
{
    if (m_movementScript != NULL) {
        delete m_movementScript;
    }

    m_movementScript = script;
}

// 0x6F2C20
SHORT CGameAIBase::GetVisualRange()
{
    return m_pArea->m_visibility.field_E * 32;
}

// 0x6F2C30
SHORT CGameAIBase::GetHelpRange()
{
    return m_pArea->m_visibility.field_E * 48;
}

// NOTE: Inlined.
void CGameAIBase::ResetCurrResponse()
{
    if (m_curAction.m_actionID == CAIAction::NO_ACTION) {
        m_curResponseNum = -1;
        m_curResponseSetNum = -1;
        m_curScriptNum = -1;
    }
}

// 0x45B970
CAIAction& CGameAIBase::GetNextAction(CAIAction& action)
{
    while (!m_queuedActions.IsEmpty()) {
        CAIAction* node = m_queuedActions.RemoveHead();
        if (node->GetActionID() != CAIAction::NO_ACTION) {
            action = *node;

            CAIObjectType actorType(node->m_actorID);
            if (actorType.OfType(CAIObjectType::ANYONE, FALSE, FALSE)
                && actorType.GetName() == ""
                && actorType.m_SpecialCase[0] == 0) {
                delete node;
                return action;
            }

            actorType.Decode(this);
            CGameAIBase* actor = static_cast<CGameAIBase*>(actorType.sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE));
            if (actor != NULL) {
                action.m_actorID = CAIObjectType::ANYONE;
                action.m_internalFlags |= 0x1;

                CMessage* message = new CMessageInsertAction(action, m_id, actor->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(actor->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        delete node;
    }

    action = CAIAction::NULL_ACTION;
    return action;
}

// 0x481890
void CGameAIBase::sub_481890(CRect* r, CArray<CRect*>& ary)
{
    // TODO: Incomplete.
}

// 0x467C50
void CGameAIBase::FireSpell(const CResRef& resRef, CGameObject* target)
{
    // TODO: Incomplete.
}

// 0x4681E0
void CGameAIBase::FireSpellPoint(const CResRef& resRef, const CPoint& ptTarget)
{
    // TODO: Incomplete.
}

// 0x4686C0
CVariable* CGameAIBase::GetGlobalVariable(const CString& sScope, const CString& sName, int a3)
{
    // TODO: Incomplete.

    return NULL;
}

// 0x4530F0
void CGameAIBase::sub_4530F0(const CAIObjectType& type)
{
    field_342.Set(type);
}

// 0x453110
void CGameAIBase::sub_453110(const CAIObjectType& type)
{
    field_37E.Set(type);
}

// 0x453130
void CGameAIBase::SetLastActionReturn(SHORT returnValue)
{
    m_nLastActionReturn = returnValue;
}

// 0x45B2D0
static CPoint sub_45B2D0(CPoint pt)
{
    return CPoint(pt.x / CPathSearch::GRID_SQUARE_SIZEX,
        pt.y / CPathSearch::GRID_SQUARE_SIZEY);
}

// 0x45B300
static LONG sub_45B300(CPoint pt1, CPoint pt2)
{
    return (pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y);
}

// 0x453840
BOOL CGameAIBase::EvaluateStatusTrigger(const CAITrigger& trigger)
{
    CAITrigger statusTrigger(trigger);
    CGameSprite* pSprite = NULL;
    CGameObject* pObject = NULL;
    BOOL bResult = FALSE;
    CString sScope;
    CString sName;
    CString sTemp1;
    CString sTemp2;
    CTypedPtrList<CPtrList, LONG*> targets(10);
    BOOL bSpriteNotShared = FALSE;

    switch (statusTrigger.m_triggerID) {
    case CAITRIGGER_AREARESTDISABLED:
        if (m_pArea != NULL) {
            bResult = (m_pArea->m_header.m_flags & 0x2) != 0;
        }
        break;
    case CAITRIGGER_ISSCRIPTNAME:
        statusTrigger.m_triggerCause.Decode(this);
        pObject = statusTrigger.m_triggerCause.sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject != NULL) {
            if (CString(static_cast<CGameAIBase*>(pObject)->m_scriptName, SCRIPTNAME_SIZE).CompareNoCase(statusTrigger.m_string1) == 0) {
                bResult = TRUE;
            }
        }
        break;
    case CAITRIGGER_ENTIREPARTYONMAP: {
        int nCharacters = g_pBaldurChitin->GetObjectGame()->m_nCharacters;
        if (nCharacters > 0) {
            bResult = TRUE;
            for (SHORT nIndex = 0; nIndex < nCharacters && bResult == TRUE; nIndex++) {
                CGameArea* pArea = m_pArea;
                CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
                LONG nCharacterId = pGame->GetCharacterId(nIndex);

                // NOTE: Shares into `pSprite`, which is released once more at
                // the end of the function.
                pGame->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (pSprite != NULL) {
                    if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0
                        && pSprite->m_pArea != pArea) {
                        bResult = FALSE;
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                } else {
                    bResult = FALSE;
                }
            }
        }
        break;
    }
    case CAITRIGGER_CURRENTAREAIS:
        statusTrigger.m_triggerCause.Decode(this);
        pObject = statusTrigger.m_triggerCause.sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject != NULL && pObject->m_pArea != NULL) {
            CString sArea;
            pObject->m_pArea->m_resRef.CopyToString(sArea);

            int nPos = sArea.FindOneOf("1234567890");
            if (nPos >= 0) {
                if (atoi(static_cast<LPCSTR>(sArea) + nPos) == statusTrigger.m_specificID) {
                    bResult = TRUE;
                }
            }
        }
        break;
    case CAITRIGGER_SUBRACE:
        statusTrigger.m_triggerCause.Decode(this);
        if (statusTrigger.m_triggerCause.m_nRace == (statusTrigger.m_specificID >> 16)
            && statusTrigger.m_triggerCause.m_nSubRace == (statusTrigger.m_specificID & 0xFFFF)) {
            bResult = TRUE;
        }
        break;
    case CAITRIGGER_KIT:
        statusTrigger.m_triggerCause.Decode(this);
        pSprite = static_cast<CGameSprite*>(statusTrigger.m_triggerCause.sub_40CB20(this, CGameObject::TYPE_SPRITE, FALSE));
        if (pSprite != NULL) {
            bResult = (pSprite->GetSpecialization() & statusTrigger.m_specificID) == static_cast<DWORD>(statusTrigger.m_specificID);
        }
        break;
    case CAITRIGGER_ALIGNMENT:
        statusTrigger.m_triggerCause.Decode(this);
        if (statusTrigger.m_specificID == 0x01
            || statusTrigger.m_specificID == 0x02
            || statusTrigger.m_specificID == 0x03
            || statusTrigger.m_specificID == 0x10
            || statusTrigger.m_specificID == 0x20
            || statusTrigger.m_specificID == 0x30) {
            bResult = (statusTrigger.m_triggerCause.m_nAlignment & static_cast<BYTE>(statusTrigger.m_specificID)) != 0;
        } else {
            bResult = statusTrigger.m_triggerCause.m_nAlignment == statusTrigger.m_specificID;
        }
        break;
    case CAITRIGGER_ALLEGIANCE:
        statusTrigger.m_triggerCause.Decode(this);
        bResult = statusTrigger.m_triggerCause.m_nEnemyAlly == statusTrigger.m_specificID;
        break;
    case CAITRIGGER_CLASS:
        statusTrigger.m_triggerCause.Decode(this);
        bResult = statusTrigger.m_triggerCause.IsClassValid(static_cast<BYTE>(statusTrigger.m_specificID));
        break;
    case CAITRIGGER_GENDER:
        statusTrigger.m_triggerCause.Decode(this);
        bResult = statusTrigger.m_triggerCause.m_nGender == statusTrigger.m_specificID;
        break;
    case CAITRIGGER_EXISTS:
        statusTrigger.m_triggerCause.Decode(this);
        pObject = statusTrigger.m_triggerCause.sub_40D0F0(this, TRUE);
        bResult = pObject != NULL;
        break;
    case CAITRIGGER_GENERAL:
        statusTrigger.m_triggerCause.Decode(this);
        bResult = statusTrigger.m_triggerCause.m_nGeneral == statusTrigger.m_specificID;
        break;
    case CAITRIGGER_GLOBAL: {
        bResult = FALSE;
        sScope = statusTrigger.m_string1;
        sScope.MakeUpper();
        sTemp1 = sScope.Left(6);
        sTemp2 = sScope.Right(sScope.GetLength() - 6);

        CVariable* pVariable;
        if (strcmp(sTemp1, "GLOBAL") == 0) {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->m_intValue == statusTrigger.m_specificID;
            } else {
                bResult = statusTrigger.m_specificID == 0;
            }
        } else if (strcmp(sTemp1, "LOCALS") == 0) {
            if (GetObjectType() != CGameObject::TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->m_intValue == statusTrigger.m_specificID;
            } else {
                bResult = statusTrigger.m_specificID == 0;
            }
        } else {
            if (strcmp(sTemp1, "MYAREA") == 0) {
                sTemp1 = reinterpret_cast<LPCSTR>(m_pArea->m_header.m_areaName);
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea != NULL) {
                pVariable = pArea->GetVariables()->FindKey(sTemp2);
                if (pVariable != NULL) {
                    bResult = pVariable->m_intValue == statusTrigger.m_specificID;
                } else {
                    bResult = statusTrigger.m_specificID == 0;
                }
            }
        }
        break;
    }
    case CAITRIGGER_GLOBALGT: {
        bResult = FALSE;
        sScope = statusTrigger.m_string1;
        sScope.MakeUpper();
        sTemp1 = sScope.Left(6);
        sTemp2 = sScope.Right(sScope.GetLength() - 6);

        CVariable* pVariable;
        if (strcmp(sTemp1, "GLOBAL") == 0) {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->m_intValue > statusTrigger.m_specificID;
            } else {
                bResult = statusTrigger.m_specificID < 0;
            }
        } else if (sTemp1.Compare("LOCALS") == 0) {
            if (GetObjectType() != CGameObject::TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->GetIntValue() > statusTrigger.GetSpecifics();
            } else {
                bResult = statusTrigger.GetSpecifics() < 0;
            }
        } else {
            if (sTemp1 == "MYAREA") {
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea != NULL) {
                pVariable = pArea->GetVariables()->FindKey(sTemp2);
                if (pVariable != NULL) {
                    bResult = pVariable->GetIntValue() > statusTrigger.GetSpecifics();
                } else {
                    bResult = statusTrigger.GetSpecifics() < 0;
                }
            }
        }
        break;
    }
    case CAITRIGGER_GLOBALLT: {
        bResult = FALSE;
        sScope = statusTrigger.GetString1();
        sScope.MakeUpper();
        sTemp1 = sScope.Left(6);
        sTemp2 = sScope.Right(sScope.GetLength() - 6);

        CVariable* pVariable;
        if (sTemp1 == "GLOBAL") {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->GetIntValue() < statusTrigger.GetSpecifics();
            } else {
                bResult = statusTrigger.GetSpecifics() > 0;
            }
        } else if (sTemp1 == "LOCALS") {
            if (GetObjectType() != CGameObject::TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->GetIntValue() < statusTrigger.GetSpecifics();
            } else {
                bResult = statusTrigger.GetSpecifics() > 0;
            }
        } else {
            if (sTemp1 == "MYAREA") {
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea != NULL) {
                pVariable = pArea->GetVariables()->FindKey(sTemp2);
                if (pVariable != NULL) {
                    bResult = pVariable->GetIntValue() < statusTrigger.GetSpecifics();
                } else {
                    bResult = statusTrigger.GetSpecifics() > 0;
                }
            }
        }
        break;
    }
    case CAITRIGGER_HP:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetBaseStats()->m_hitPoints == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HPGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetBaseStats()->m_hitPoints > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HPLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetBaseStats()->m_hitPoints < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HPPERCENT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetBaseStats()->m_hitPoints * 100 / pSprite->GetDerivedStats()->m_nMaxHitPoints == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HPPERCENTLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetBaseStats()->m_hitPoints * 100 / pSprite->GetDerivedStats()->m_nMaxHitPoints < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HPPERCENTGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetBaseStats()->m_hitPoints * 100 / pSprite->GetDerivedStats()->m_nMaxHitPoints > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HPLOST:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetDerivedStats()->m_nMaxHitPoints - pSprite->GetBaseStats()->m_hitPoints == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HPLOSTGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetDerivedStats()->m_nMaxHitPoints - pSprite->GetBaseStats()->m_hitPoints > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HPLOSTLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetDerivedStats()->m_nMaxHitPoints - pSprite->GetBaseStats()->m_hitPoints < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_ISCREATUREAREAFLAG:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite != NULL) {
            bResult = (statusTrigger.GetSpecifics() & pSprite->GetBaseStats()->m_flags) != 0;
        } else {
            bResult = FALSE;
        }
        break;
    case CAITRIGGER_LOS: {
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40D050(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        BOOL bOrderable = FALSE;
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            bOrderable = static_cast<CGameSprite*>(pObject)->Orderable(FALSE);
        }

        LONG nDistance = sub_45B300(sub_45B2D0(GetPos()), sub_45B2D0(pObject->GetPos()));
        if (nDistance > (statusTrigger.GetSpecifics() + 1) * (statusTrigger.GetSpecifics() + 1)) {
            bResult = FALSE;
            break;
        }

        if (!m_pArea->CheckLOS(pObject->GetPos(), GetPos(), GetVisibleTerrainTable(), bOrderable)) {
            bResult = FALSE;
            break;
        }

        bResult = TRUE;

        // NOTE: The object type (not the AI type) is used as EA.
        sub_4530F0(CAIObjectType(pObject->GetObjectType()));
        break;
    }
    case CAITRIGGER_MORALE:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = static_cast<BYTE>(statusTrigger.GetSpecifics()) == pSprite->GetBaseStats()->m_morale;
        break;
    case CAITRIGGER_MORALEGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetBaseStats()->m_morale > static_cast<BYTE>(statusTrigger.GetSpecifics());
        break;
    case CAITRIGGER_MORALELT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetBaseStats()->m_morale < static_cast<BYTE>(statusTrigger.GetSpecifics());
        break;
    case CAITRIGGER_RACE:
        statusTrigger.Decode(this);
        bResult = statusTrigger.GetCause().GetRace() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_RANGE: {
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        // NOTE: `ptTarget` is passed to `sub_48B2C0` uninitialized.
        CPoint ptTarget;
        BYTE nObjectType = pObject->GetObjectType();
        if (nObjectType == CGameObject::TYPE_DOOR) {
            ptTarget = static_cast<CGameDoor*>(pObject)->sub_48B2C0(ptTarget);
        } else if (nObjectType == CGameObject::TYPE_SPRITE) {
            ptTarget = pObject->GetPos();
        } else {
            ptTarget = pObject->GetPos();
        }

        LONG nDistance = sub_45B300(sub_45B2D0(GetPos()), sub_45B2D0(ptTarget));
        LONG nRange = (statusTrigger.GetSpecifics() + 1) * (statusTrigger.GetSpecifics() + 1);

        switch (statusTrigger.GetInt1()) {
        case 1:
            bResult = nDistance == nRange;
            break;
        case 2:
            bResult = nDistance < nRange;
            break;
        case 3:
            bResult = nDistance > nRange;
            break;
        default:
            bResult = FALSE;
            break;
        }

        if (bResult == TRUE) {
            sub_4530F0(pObject->GetAIType());
        }
        break;
    }
    case CAITRIGGER_REPUTATION: {
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        LONG nReputation;
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId()) != -1) {
            nReputation = g_pBaldurChitin->GetObjectGame()->GetReputation();
        } else {
            nReputation = pSprite->GetBaseStats()->m_reputation;
        }

        bResult = nReputation == statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_REPUTATIONGT: {
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        LONG nReputation;
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId()) != -1) {
            nReputation = g_pBaldurChitin->GetObjectGame()->GetReputation();
        } else {
            nReputation = pSprite->GetBaseStats()->m_reputation;
        }

        bResult = nReputation > statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_REPUTATIONLT: {
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        LONG nReputation;
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId()) != -1) {
            nReputation = g_pBaldurChitin->GetObjectGame()->GetReputation();
        } else {
            nReputation = pSprite->GetBaseStats()->m_reputation;
        }

        bResult = nReputation < statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_SEE:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        bResult = CanSeeTarget(pObject, statusTrigger.GetSpecifics());
        if (bResult == TRUE) {
            if (!m_lSeen.Equal(pObject->GetAIType())) {
                if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pObject->GetId()) != -1) {
                    m_canBeSeen = 4 * (CGameObject::VISIBLE_DELAY + 1);
                }

                // NOTE: Uninline `CAIObjectType::operator=` (0x45B370).
                m_lSeen.Set(pObject->GetAIType());

                CMessage* message = new CMessageSetLastObject(pObject->GetAIType(),
                    CAITRIGGER_SEE,
                    m_id,
                    m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            sub_4530F0(pObject->GetAIType());
        }
        break;
    case CAITRIGGER_DETECT:
        statusTrigger.Decode(this);
        if (statusTrigger.GetCause().GetInstance() == -1) {
            LONG nTarget;
            if (GetVertListType() == CGameObject::LIST_FRONT) {
                nTarget = m_pArea->GetNearest(m_id,
                    statusTrigger.GetCause(),
                    GetVisualRange(),
                    GetVisibleTerrainTable(),
                    TRUE,
                    TRUE,
                    FALSE,
                    0,
                    FALSE);
            } else {
                nTarget = m_pArea->sub_46DAE0(m_pos.x,
                    m_pos.y,
                    statusTrigger.GetCause(),
                    GetVisualRange(),
                    GetVisibleTerrainTable(),
                    TRUE,
                    TRUE,
                    0,
                    0);
            }

            if (nTarget == CGameObjectArray::INVALID_INDEX) {
                break;
            }

            // NOTE: Sets visibility of the caller, not the target.
            if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(nTarget) != -1) {
                m_canBeSeen = 4 * (CGameObject::VISIBLE_DELAY + 1);
            }

            bResult = TRUE;

            if (m_lSeen.GetInstance() != nTarget) {
                CAIObjectType type;
                type.SetInstance(nTarget);

                // NOTE: Uninline `CAIObjectType::operator=` (0x45B370).
                m_lSeen.Set(type);

                CMessage* message = new CMessageSetLastObject(statusTrigger.GetCause(),
                    CAITRIGGER_SEE,
                    m_id,
                    m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        } else {
            pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
            if (pObject == NULL) {
                break;
            }

            LONG nDistance = sub_45B300(sub_45B2D0(GetPos()), sub_45B2D0(pObject->GetPos()));

            BOOL bOrderable = FALSE;
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                bOrderable = static_cast<CGameSprite*>(pObject)->Orderable(FALSE);
            }

            if (nDistance <= GetVisualRange() * GetVisualRange()
                && m_pArea->CheckLOS(pObject->GetPos(), GetPos(), GetVisibleTerrainTable(), bOrderable)) {
                bResult = TRUE;
            } else {
                bResult = FALSE;
            }

            bResult &= pObject->GetArea() == m_pArea;
            if (bResult) {
                if (!m_lSeen.Equal(pObject->GetAIType())) {
                    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pObject->GetId()) != -1) {
                        m_canBeSeen = 4 * (CGameObject::VISIBLE_DELAY + 1);
                    }

                    // NOTE: Uninline `CAIObjectType::operator=` (0x45B370).
                    m_lSeen.Set(pObject->GetAIType());

                    CMessage* message = new CMessageSetLastObject(pObject->GetAIType(),
                        CAITRIGGER_SEE,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            pObject = NULL;
        }
        break;
    case CAITRIGGER_SPECIFICS:
        statusTrigger.Decode(this);
        bResult = statusTrigger.GetCause().GetSpecific() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_TIME:
        bResult = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->GetCurrentHour() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_TIMEGT:
        bResult = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->GetCurrentHour() > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_TIMELT:
        bResult = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->GetCurrentHour() < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_TIMEOFDAY:
        switch (statusTrigger.GetSpecifics()) {
        case 0:
            bResult = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDay();
            break;
        case 1:
            bResult = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDusk();
            break;
        case 2:
            bResult = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsNight();
            break;
        case 3:
            bResult = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDawn();
            break;
        default:
            bResult = FALSE;
            break;
        }
        break;
    case CAITRIGGER_DELAY:
        bResult = field_54C % statusTrigger.GetSpecifics() <= field_550;
        break;
    case CAITRIGGER_4080: {
        // Not in TRIGGER.IDS. Number of matching creatures in sight not
        // counting the party size.
        m_pArea->GetAllInRange(m_pos,
            statusTrigger.GetCause(),
            GetVisualRange(),
            GetVisibleTerrainTable(),
            targets,
            TRUE,
            FALSE);

        SHORT nCharacters = g_pBaldurChitin->GetObjectGame()->GetNumCharacters();
        bResult = targets.GetCount() - nCharacters == statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_4081: {
        // Not in TRIGGER.IDS.
        m_pArea->GetAllInRange(m_pos,
            statusTrigger.GetCause(),
            GetVisualRange(),
            GetVisibleTerrainTable(),
            targets,
            TRUE,
            FALSE);

        SHORT nCharacters = g_pBaldurChitin->GetObjectGame()->GetNumCharacters();
        bResult = targets.GetCount() - nCharacters < statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_4082: {
        // Not in TRIGGER.IDS.
        m_pArea->GetAllInRange(m_pos,
            statusTrigger.GetCause(),
            GetVisualRange(),
            GetVisibleTerrainTable(),
            targets,
            TRUE,
            FALSE);

        SHORT nCharacters = g_pBaldurChitin->GetObjectGame()->GetNumCharacters();
        bResult = targets.GetCount() - nCharacters > statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_NUMCREATURE:
        m_pArea->GetAllInRange(m_pos,
            statusTrigger.GetCause(),
            GetVisualRange(),
            GetVisibleTerrainTable(),
            targets,
            TRUE,
            FALSE);
        bResult = targets.GetCount() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_NUMCREATUREGT:
        m_pArea->GetAllInRange(m_pos,
            statusTrigger.GetCause(),
            GetVisualRange(),
            GetVisibleTerrainTable(),
            targets,
            TRUE,
            FALSE);
        bResult = targets.GetCount() > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_NUMCREATURELT:
        m_pArea->GetAllInRange(m_pos,
            statusTrigger.GetCause(),
            GetVisualRange(),
            GetVisibleTerrainTable(),
            targets,
            TRUE,
            FALSE);
        bResult = targets.GetCount() < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_ACTIONLISTEMPTY:
        if (m_queuedActions.GetCount() != 0) {
            bResult = FALSE;
        } else if (m_curAction.GetActionID() == CAIAction::NO_ACTION) {
            bResult = TRUE;
        } else {
            bResult = FALSE;
        }
        break;
    case CAITRIGGER_STATECHECK:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = (statusTrigger.GetSpecifics() & pSprite->GetDerivedStats()->m_generalState) != 0;
        break;
    case CAITRIGGER_NOTSTATECHECK:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = (statusTrigger.GetSpecifics() & pSprite->GetDerivedStats()->m_generalState) == 0;
        break;
    case CAITRIGGER_CHECKSTAT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetDerivedStats()->GetAtOffset(static_cast<SHORT>(statusTrigger.GetInt1())) == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_CHECKSTATGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetDerivedStats()->GetAtOffset(static_cast<SHORT>(statusTrigger.GetInt1())) > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_CHECKSTATLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        bResult = pSprite->GetDerivedStats()->GetAtOffset(static_cast<SHORT>(statusTrigger.GetInt1())) < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_REACTION: {
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        BYTE nReaction = GetReaction(pSprite);
        pSprite->FeedBack(CGameSprite::FEEDBACK_29, nReaction, 0, 0, -1, 0, 0);
        bResult = nReaction == statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_REACTIONGT: {
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        BYTE nReaction = GetReaction(pSprite);
        pSprite->FeedBack(CGameSprite::FEEDBACK_29, nReaction, 0, 0, -1, 0, 0);
        bResult = nReaction > statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_REACTIONLT: {
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        BYTE nReaction = GetReaction(pSprite);
        pSprite->FeedBack(CGameSprite::FEEDBACK_29, nReaction, 0, 0, -1, 0, 0);
        bResult = nReaction < statusTrigger.GetSpecifics();
        break;
    }
    case CAITRIGGER_GLOBALTIMEREXACT: {
        bResult = FALSE;

        sScope = statusTrigger.GetString2();
        sScope.MakeUpper();
        sTemp1 = sScope.Left(6);
        sName = statusTrigger.GetString1();

        CVariable* pVariable;
        if (sTemp1 == "GLOBAL") {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        } else if (sTemp1 == "LOCALS") {
            if (GetObjectType() != TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sName);
        } else {
            if (sTemp1 == "MYAREA") {
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea == NULL) {
                break;
            }

            pVariable = pArea->GetVariables()->FindKey(sName);
        }

        if (pVariable == NULL) {
            bResult = FALSE;
            break;
        }

        ULONG nGameTime = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
        bResult = static_cast<ULONG>(pVariable->GetIntValue()) == nGameTime;
        break;
    }
    case CAITRIGGER_GLOBALTIMEREXPIRED: {
        bResult = FALSE;

        sScope = statusTrigger.GetString2();
        sScope.MakeUpper();
        sTemp1 = sScope.Left(6);
        sName = statusTrigger.GetString1();

        CVariable* pVariable;
        if (sTemp1 == "GLOBAL") {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        } else if (sTemp1 == "LOCALS") {
            if (GetObjectType() != TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sName);
        } else {
            if (sTemp1 == "MYAREA") {
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea == NULL) {
                break;
            }

            pVariable = pArea->GetVariables()->FindKey(sName);
        }

        if (pVariable == NULL) {
            bResult = TRUE;
            break;
        }

        ULONG nGameTime = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
        bResult = static_cast<ULONG>(pVariable->GetIntValue()) < nGameTime;
        break;
    }
    case CAITRIGGER_GLOBALTIMERNOTEXPIRED: {
        bResult = FALSE;

        sScope = statusTrigger.GetString2();
        sScope.MakeUpper();
        sTemp1 = sScope.Left(6);
        sName = statusTrigger.GetString1();

        CVariable* pVariable;
        if (sTemp1 == "GLOBAL") {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        } else if (sTemp1 == "LOCALS") {
            if (GetObjectType() != TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sName);
        } else {
            if (sTemp1 == "MYAREA") {
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea == NULL) {
                break;
            }

            pVariable = pArea->GetVariables()->FindKey(sName);
        }

        if (pVariable == NULL) {
            bResult = FALSE;
            break;
        }

        ULONG nGameTime = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
        bResult = nGameTime < static_cast<ULONG>(pVariable->GetIntValue());
        break;
    }
    case CAITRIGGER_PARTYHASITEM:
        for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
            LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                return FALSE;
            }

            if (pSprite->FindItemPersonal(statusTrigger.GetString1(), 0, FALSE) != -1) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return TRUE;
            }

            if (pSprite->FindItemBags(statusTrigger.GetString1(), 0, FALSE) != -1) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return TRUE;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        return FALSE;
    case CAITRIGGER_TRUE:
        return TRUE;
    case CAITRIGGER_GLOBALTIMERSTARTED: {
        sScope = statusTrigger.GetString2();
        sScope.MakeUpper();
        sTemp1 = sScope.Left(6);
        sName = statusTrigger.GetString1();

        CVariable* pVariable;
        if (sTemp1 == "GLOBAL") {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        } else if (sTemp1 == "LOCALS") {
            if (GetObjectType() != TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sName);
        } else {
            if (sTemp1 == "MYAREA") {
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea == NULL) {
                bResult = FALSE;
                break;
            }

            pVariable = pArea->GetVariables()->FindKey(sName);
        }

        if (pVariable != NULL) {
            bResult = TRUE;
            break;
        }

        bResult = FALSE;
        break;
    }
    case CAITRIGGER_ISTEAMBITON:
        bResult = statusTrigger.GetSpecifics() & sub_45B6D0();
        break;
    case CAITRIGGER_ISPLAYERNUMBER:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject == NULL) {
            bResult = FALSE;
            break;
        }

        if (statusTrigger.GetSpecifics() < 1) {
            bResult = FALSE;
            break;
        }

        if (statusTrigger.GetSpecifics() > 6) {
            bResult = FALSE;
            break;
        }

        // NOTE: Uninline `GetFixedOrderCharacterId`.
        bResult = g_pBaldurChitin->GetObjectGame()->GetFixedOrderCharacterId(static_cast<SHORT>(statusTrigger.GetSpecifics() - 1)) == pObject->GetId();
        break;
    case CAITRIGGER_CHECKSKILL:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        // NOTE: Skill values are read as signed chars.
        bResult = static_cast<char>(pSprite->GetDerivedStats()->m_nSkills[statusTrigger.GetInt1()]) == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_CHECKSKILLGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = static_cast<char>(pSprite->GetDerivedStats()->m_nSkills[statusTrigger.GetInt1()]) > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_CHECKSKILLLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = static_cast<char>(pSprite->GetDerivedStats()->m_nSkills[statusTrigger.GetInt1()]) < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_LEVELINCLASS:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = static_cast<LONG>(pSprite->GetDerivedStats()->GetClassLevel(statusTrigger.GetInt1())) == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_LEVELINCLASSGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = static_cast<LONG>(pSprite->GetDerivedStats()->GetClassLevel(statusTrigger.GetInt1())) > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_LEVELINCLASSLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = static_cast<LONG>(pSprite->GetDerivedStats()->GetClassLevel(statusTrigger.GetInt1())) < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_407F:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40CB20(this, TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        if (pObject->GetObjectType() == TYPE_SPRITE) {
            bResult = static_cast<CGameSprite*>(pObject)->FindItemPersonal(statusTrigger.GetString1(), 0, FALSE) != -1;
        } else {
            bResult = FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
        break;
    case CAITRIGGER_HASITEM:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40CB20(this, TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        if (pObject->GetObjectType() == TYPE_SPRITE) {
            if (static_cast<CGameSprite*>(pObject)->FindItemPersonal(statusTrigger.GetString1(), 0, FALSE) != -1) {
                bResult = TRUE;
            } else {
                bResult = static_cast<CGameSprite*>(pObject)->FindItemBags(statusTrigger.GetString1(), 0, FALSE) != -1;
            }
        } else if (pObject->GetObjectType() == TYPE_CONTAINER) {
            CResRef resRef(statusTrigger.GetString1());
            bResult = static_cast<CGameContainer*>(pObject)->FindItemSlot(resRef) != -1;
        } else {
            bResult = FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
        break;
    case CAITRIGGER_INPARTY: {
        statusTrigger.Decode(this);

        if (statusTrigger.GetCause().GetInstance() != -1) {
            if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(statusTrigger.GetCause().GetInstance()) != -1) {
                // NOTE: Result is not checked.
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(statusTrigger.GetCause().GetInstance(),
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);

                if ((pSprite->GetDerivedStats()->m_generalState & 0x800) != 0) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(statusTrigger.GetCause().GetInstance(),
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    return FALSE;
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(statusTrigger.GetCause().GetInstance(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return TRUE;
            }
        } else {
            for (int nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait)),
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS) {
                    if (pSprite->GetAIType().OfType(statusTrigger.GetCause(), FALSE, FALSE)
                        && (pSprite->GetDerivedStats()->m_generalState & 0x800) == 0) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait)),
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        return TRUE;
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait)),
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        }

        CTypedPtrArray<CPtrArray, int*>& overflow = g_pBaldurChitin->GetObjectGame()->GetCharacterOverflow();
        for (int nIndex = 0; nIndex < overflow.GetSize(); nIndex++) {
            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(reinterpret_cast<LONG>(overflow.ElementAt(nIndex)),
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                if (pSprite->GetAIType().OfType(statusTrigger.GetCause(), FALSE, FALSE)
                    && (pSprite->GetDerivedStats()->m_generalState & 0x800) == 0) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(reinterpret_cast<LONG>(overflow.ElementAt(nIndex)),
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    return TRUE;
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(reinterpret_cast<LONG>(overflow.ElementAt(nIndex)),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        // FIXME: `pSprite` is left pointing at the last (already released)
        // sprite, so the cleanup below releases it a second time.
        break;
    }
    case CAITRIGGER_RANDOMNUM:
        bResult = m_randValue % statusTrigger.GetSpecifics() + 1 == statusTrigger.GetInt1();
        break;
    case CAITRIGGER_RANDOMNUMGT:
        bResult = m_randValue % statusTrigger.GetSpecifics() + 1 > statusTrigger.GetInt1();
        break;
    case CAITRIGGER_RANDOMNUMLT:
        bResult = m_randValue % statusTrigger.GetSpecifics() + 1 < statusTrigger.GetInt1();
        break;
    case CAITRIGGER_PARTYGOLD:
        bResult = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold == static_cast<DWORD>(statusTrigger.GetSpecifics());
        break;
    case CAITRIGGER_PARTYGOLDGT:
        bResult = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold > static_cast<DWORD>(statusTrigger.GetSpecifics());
        break;
    case CAITRIGGER_PARTYGOLDLT:
        bResult = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold < static_cast<DWORD>(statusTrigger.GetSpecifics());
        break;
    case CAITRIGGER_DEAD:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40D0F0(this, TRUE);
        if (pObject == NULL) {
            bResult = TRUE;
            break;
        }

        if (pObject->GetObjectType() != TYPE_SPRITE) {
            bResult = TRUE;
            break;
        }

        if ((static_cast<CGameSprite*>(pObject)->GetDerivedStats()->m_generalState & 0x800) != 0) {
            bResult = TRUE;
            break;
        }

        if ((static_cast<CGameSprite*>(pObject)->GetBaseStats()->m_generalState & 0x800) != 0) {
            bResult = TRUE;
            break;
        }

        bResult = FALSE;
        break;
    case CAITRIGGER_HASWEAPONEQUIPED:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = pSprite->GetEquipment()->m_selectedWeapon != CGameSpriteEquipment::SLOT_FIST;
        break;
    case CAITRIGGER_HAPPINESS:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = pSprite->GetHappiness() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HAPPINESSGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = pSprite->GetHappiness() > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_HAPPINESSLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = pSprite->GetHappiness() < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_NUMINPARTY:
        bResult = g_pBaldurChitin->GetObjectGame()->GetNumCharacters() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_NUMINPARTYGT:
        bResult = g_pBaldurChitin->GetObjectGame()->GetNumCharacters() > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_NUMINPARTYLT:
        bResult = g_pBaldurChitin->GetObjectGame()->GetNumCharacters() < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_NUMDEAD:
        sTemp2 = DEAD_GLOBAL_PREFIX + statusTrigger.GetString1();

        // NOTE: Result is unused.
        sTemp2.Left(32);

        {
            CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->GetIntValue() == statusTrigger.GetSpecifics();
            } else {
                bResult = statusTrigger.GetSpecifics() == 0;
            }
        }
        break;
    case CAITRIGGER_NUMDEADGT:
        sTemp2 = DEAD_GLOBAL_PREFIX + statusTrigger.GetString1();

        // NOTE: Result is unused.
        sTemp2.Left(32);

        {
            CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->GetIntValue() > statusTrigger.GetSpecifics();
            } else {
                bResult = statusTrigger.GetSpecifics() < 0;
            }
        }
        break;
    case CAITRIGGER_NUMDEADLT:
        sTemp2 = DEAD_GLOBAL_PREFIX + statusTrigger.GetString1();

        // NOTE: Result is unused.
        sTemp2.Left(32);

        {
            CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                bResult = pVariable->GetIntValue() < statusTrigger.GetSpecifics();
            } else {
                bResult = statusTrigger.GetSpecifics() > 0;
            }
        }
        break;
    case CAITRIGGER_CONTAINS:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
            CGameContainer* pContainer = static_cast<CGameContainer*>(pObject);
            for (SHORT nSlot = 0; nSlot < pContainer->GetNumItems(); nSlot++) {
                CItem* pItem = pContainer->GetItem(nSlot);
                if (pItem != NULL && pItem->GetResRef() == statusTrigger.GetString1()) {
                    bResult = TRUE;
                    break;
                }
            }
        } else {
            bResult = FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
        break;
    case CAITRIGGER_OPENSTATE:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_DOOR) {
            if (statusTrigger.GetSpecifics() != 0) {
                bResult = static_cast<CGameDoor*>(pObject)->sub_45B700();
            } else {
                bResult = !static_cast<CGameDoor*>(pObject)->sub_45B700();
            }
        } else {
            bResult = FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
        break;
    case CAITRIGGER_ISOVERME:
        if (GetObjectType() != CGameObject::TYPE_TRIGGER) {
            return FALSE;
        }

        statusTrigger.Decode(this);

        if (statusTrigger.GetCause().GetInstance() == -1) {
            LONG nIndex;
            if (GetVertListType() == CGameObject::LIST_FRONT) {
                nIndex = m_pArea->GetNearest(m_id,
                    statusTrigger.GetCause(),
                    GetVisualRange(),
                    GetVisibleTerrainTable(),
                    TRUE,
                    TRUE,
                    FALSE,
                    0,
                    FALSE);
            } else {
                nIndex = m_pArea->sub_46DAE0(m_pos.x,
                    m_pos.y,
                    statusTrigger.GetCause(),
                    GetVisualRange(),
                    GetVisibleTerrainTable(),
                    TRUE,
                    TRUE,
                    0,
                    0);
            }

            if (nIndex == CGameObjectArray::INVALID_INDEX) {
                return FALSE;
            }

            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nIndex,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pObject,
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                return FALSE;
            }

            bResult = static_cast<CGameTrigger*>(this)->IsOverActivate(pObject->GetPos());
            if (bResult) {
                // NOTE: Original calls `CAIObjectType::operator=` (0x45B370),
                // which only calls `Set`.
                m_lTrigger.Set(pObject->GetAIType());

                CMessage* message = new CMessageSetLastObject(pObject->GetAIType(), 0, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nIndex,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            pObject = NULL;
        } else {
            pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
            if (pObject != NULL) {
                bResult = static_cast<CGameTrigger*>(this)->IsOverActivate(pObject->GetPos());
                if (bResult) {
                    // NOTE: See above.
                    m_lTrigger.Set(pObject->GetAIType());

                    CMessage* message = new CMessageSetLastObject(pObject->GetAIType(), 0, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                pObject = NULL;
            }
        }
        break;
    case CAITRIGGER_NUMITEMS:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            bResult = static_cast<CGameSprite*>(pObject)->CountItemPersonal(statusTrigger.GetString1()) == statusTrigger.GetSpecifics();
        } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
            bResult = static_cast<CGameContainer*>(pObject)->CountItem(CResRef(statusTrigger.GetString1())) == statusTrigger.GetSpecifics();
        } else {
            bResult = FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
        break;
    case CAITRIGGER_NUMITEMSGT:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            bResult = static_cast<CGameSprite*>(pObject)->CountItemPersonal(statusTrigger.GetString1()) > statusTrigger.GetSpecifics();
        } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
            bResult = static_cast<CGameContainer*>(pObject)->CountItem(CResRef(statusTrigger.GetString1())) > statusTrigger.GetSpecifics();
        } else {
            bResult = FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
        break;
    case CAITRIGGER_NUMITEMSLT:
        statusTrigger.Decode(this);

        pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            bResult = static_cast<CGameSprite*>(pObject)->CountItemPersonal(statusTrigger.GetString1()) < statusTrigger.GetSpecifics();
        } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
            bResult = static_cast<CGameContainer*>(pObject)->CountItem(CResRef(statusTrigger.GetString1())) < statusTrigger.GetSpecifics();
        } else {
            bResult = FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
        break;
    case CAITRIGGER_NUMITEMSPARTY:
        {
            LONG nCount = 0;
            for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                nCount += pSprite->CountItemPersonal(statusTrigger.GetString1());

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            // NOTE: Returns directly, the shares are already released.
            bResult = nCount == statusTrigger.GetSpecifics();
            return bResult;
        }
        break;
    case CAITRIGGER_NUMITEMSPARTYGT:
        {
            LONG nCount = 0;
            for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                nCount += pSprite->CountItemPersonal(statusTrigger.GetString1());

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            // NOTE: Returns directly, the shares are already released.
            bResult = nCount > statusTrigger.GetSpecifics();
            return bResult;
        }
        break;
    case CAITRIGGER_NUMITEMSPARTYLT:
        {
            LONG nCount = 0;
            for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                nCount += pSprite->CountItemPersonal(statusTrigger.GetString1());

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            // NOTE: Returns directly, the shares are already released.
            bResult = nCount < statusTrigger.GetSpecifics();
            return bResult;
        }
        break;
    case CAITRIGGER_407E:
        sScope = statusTrigger.GetString1();
        sName = m_pArea->GetHeader()->m_areaName;

        // NOTE: Original stores the raw compare result, so this is TRUE when
        // the names differ.
        bResult = sScope.Compare(sName);
        break;
    case CAITRIGGER_4083:
        bResult = m_pArea->GetBattleSongCounter() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_4084:
        bResult = m_pArea->GetBattleSongCounter() < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_4085:
        bResult = m_pArea->GetBattleSongCounter() > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_4086:
        bResult = m_pArea->GetHeader()->m_areaType & static_cast<WORD>(statusTrigger.GetSpecifics());
        break;
    case CAITRIGGER_4090:
        statusTrigger.Decode(this);

        if (statusTrigger.GetCause().GetInstance() != -1) {
            if (g_pBaldurChitin->GetObjectGame()->GetFixedOrderCharacterPortraitNum(statusTrigger.GetCause().GetInstance()) != statusTrigger.GetSpecifics()) {
                return FALSE;
            }

            // NOTE: Result is not checked.
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(statusTrigger.GetCause().GetInstance(),
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);

            if ((pSprite->GetDerivedStats()->m_generalState & STATE_DEAD) != 0) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(statusTrigger.GetCause().GetInstance(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return FALSE;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(statusTrigger.GetCause().GetInstance(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return TRUE;
        } else {
            SHORT nSlot = static_cast<SHORT>(statusTrigger.GetSpecifics());

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(g_pBaldurChitin->GetObjectGame()->GetFixedOrderCharacterId(nSlot),
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                return FALSE;
            }

            if (pSprite->GetAIType().OfType(statusTrigger.GetCause(), FALSE, FALSE)
                && (pSprite->GetDerivedStats()->m_generalState & STATE_DEAD) == 0) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetFixedOrderCharacterId(nSlot),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return TRUE;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetFixedOrderCharacterId(nSlot),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return FALSE;
        }
        break;
    case CAITRIGGER_4092:
        {
            CPoint ptObject;
            CPoint ptTarget;
            CAIObjectType typeTarget;

            typeTarget.SetName(statusTrigger.GetString1());

            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            ptObject = pObject->GetPos();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            pObject = NULL;

            typeTarget.Decode(this);

            pObject = typeTarget.sub_40CCA0(this, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            ptTarget = pObject->GetPos();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            pObject = NULL;

            double a = sub_45B300(m_pos, ptObject);
            double b = sub_45B300(m_pos, ptTarget);
            double c = sub_45B300(ptObject, ptTarget);

            if (a > b || c > b) {
                return FALSE;
            }

            a = sqrt(a);
            b = sqrt(b);
            c = sqrt(c);

            // NOTE: Multiplies by pi instead of dividing.
            if (acos((b * b + a * a - c * c) / (b * a + b * a)) * 360.0 / 2.0 * 3.1415926535 >= 30.0) {
                return FALSE;
            }

            return TRUE;
        }
        break;
    case CAITRIGGER_LEVEL:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = static_cast<LONG>(pSprite->GetDerivedStats()->GetClassMaskLevel(0xFFF)) == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_LEVELGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = static_cast<LONG>(pSprite->GetDerivedStats()->GetClassMaskLevel(0xFFF)) > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_LEVELLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = static_cast<LONG>(pSprite->GetDerivedStats()->GetClassMaskLevel(0xFFF)) < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_BITGLOBAL: {
        CVariable* pVariable;

        sScope = statusTrigger.GetString1();
        sScope.MakeUpper();
        int nIndex = sScope.Find(":");
        sTemp1 = sScope.Left(nIndex);
        sTemp2 = sScope.Right(sScope.GetLength() - nIndex - 1);

        if (sTemp1 == "GLOBAL") {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
        } else if (sTemp1 == "LOCALS") {
            if (GetObjectType() != TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sTemp2);
        } else {
            if (sTemp1 == "MYAREA") {
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea == NULL) {
                break;
            }

            pVariable = pArea->GetVariables()->FindKey(sTemp2);
        }

        if (pVariable == NULL) {
            bResult = statusTrigger.GetSpecifics() == 0;
            break;
        }

        // NOTE: The OR and XOR checks are only reached when the first check
        // for 1 succeeded, so they can never pass. This mirrors the original.
        if (statusTrigger.GetInt1() == 1) {
            if ((pVariable->GetIntValue() & statusTrigger.GetSpecifics()) != 0) {
                bResult = TRUE;
            } else if (statusTrigger.GetInt1() == 2) {
                if ((pVariable->GetIntValue() | statusTrigger.GetSpecifics()) != 0) {
                    bResult = TRUE;
                } else if (statusTrigger.GetInt1() == 3) {
                    if ((pVariable->GetIntValue() ^ statusTrigger.GetSpecifics()) != 0) {
                        bResult = TRUE;
                    }
                }
            }
        }
        break;
    }
    case CAITRIGGER_GLOBALBITGLOBAL: {
        CVariable* pVariable;

        sScope = statusTrigger.GetString1();
        sScope.MakeUpper();
        int nIndex = sScope.Find(":");
        sTemp1 = sScope.Left(nIndex);
        sTemp2 = sScope.Right(sScope.GetLength() - nIndex - 1);

        LONG nValue = 0;
        CString sScope2;
        CString sName2;

        sScope = statusTrigger.GetString2();
        sScope.MakeUpper();
        nIndex = sScope.Find(":");
        sScope2 = sScope.Left(nIndex);
        sName2 = sScope.Right(sScope.GetLength() - nIndex - 1);

        if (sScope2 == "GLOBAL") {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName2);
            if (pVariable != NULL) {
                nValue = pVariable->GetIntValue();
            }
        } else if (sScope2 == "LOCALS") {
            if (GetObjectType() == TYPE_SPRITE) {
                pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sName2);
                if (pVariable != NULL) {
                    nValue = pVariable->GetIntValue();
                }
            }
        } else {
            if (sScope2 == "MYAREA") {
                sScope2 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope2);
            if (pArea != NULL) {
                pVariable = pArea->GetVariables()->FindKey(sName2);
                if (pVariable != NULL) {
                    nValue = pVariable->GetIntValue();
                }
            }
        }

        if (sTemp1 == "GLOBAL") {
            pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
        } else if (sTemp1 == "LOCALS") {
            if (GetObjectType() != TYPE_SPRITE) {
                bResult = FALSE;
                break;
            }

            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sTemp2);
        } else {
            if (sTemp1 == "MYAREA") {
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea == NULL) {
                break;
            }

            pVariable = pArea->GetVariables()->FindKey(sTemp2);
        }

        if (pVariable == NULL) {
            bResult = statusTrigger.GetSpecifics() == 0;
            break;
        }

        // NOTE: Same nesting as `BitGlobal`: the OR and XOR checks are only
        // reached when the check for 1 succeeded.
        if (statusTrigger.GetSpecifics() == 1) {
            if ((nValue & pVariable->GetIntValue()) != 0) {
                bResult = TRUE;
            } else if (statusTrigger.GetSpecifics() == 2) {
                if ((pVariable->GetIntValue() | nValue) != 0) {
                    bResult = TRUE;
                } else if (statusTrigger.GetSpecifics() == 3) {
                    if ((pVariable->GetIntValue() ^ nValue) != 0) {
                        bResult = TRUE;
                    }
                }
            }
        }
        break;
    }
    case CAITRIGGER_CREATUREHIDDEN:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = pSprite->GetBaseStats()->field_294;
        break;
    case CAITRIGGER_HELPEX:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = FALSE;
        switch (statusTrigger.GetSpecifics()) {
        case 1:
            bResult = pSprite->GetAIType().GetEnemyAlly() == m_lHelp.GetEnemyAlly();
            m_lHelp.SetEnemyAlly(255);
            break;
        case 2:
            bResult = pSprite->GetAIType().GetGeneral() == m_lHelp.GetGeneral();
            m_lHelp.SetGeneral(255);
            break;
        case 3:
            bResult = pSprite->GetAIType().GetRace() == m_lHelp.GetRace();
            m_lHelp.SetRace(255);
            break;
        case 4:
            bResult = pSprite->GetAIType().GetClass() & m_lHelp.GetClass();
            m_lHelp.SetClass(255);
            break;
        case 5:
            bResult = pSprite->GetAIType().GetSpecific() == m_lHelp.GetSpecific();
            m_lHelp.SetSpecific(255);
            break;
        case 6:
            bResult = pSprite->GetAIType().GetGender() == m_lHelp.GetGender();
            m_lHelp.SetGender(255);
            break;
        case 7:
            bResult = pSprite->GetAIType().GetAlignment() == m_lHelp.GetAlignment();
            m_lHelp.SetAlignment(255);
            break;
        }
        break;
    case CAITRIGGER_NEARLOCATION:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        switch (statusTrigger.GetSpecifics()) {
        case -2:
            statusTrigger.SetSpecifics(static_cast<WORD>(static_cast<CGameSprite*>(this)->GetBaseStats()->field_2E4));
            break;
        case -1:
            statusTrigger.SetSpecifics(GetPos().x);
            break;
        }

        switch (statusTrigger.GetInt1()) {
        case -2:
            statusTrigger.SetInt1(static_cast<WORD>(static_cast<CGameSprite*>(this)->GetBaseStats()->field_2E6));
            break;
        case -1:
            statusTrigger.SetInt1(GetPos().y);
            break;
        }

        if (statusTrigger.GetSpecifics() > 0 && statusTrigger.GetInt1() > 0) {
            LONG nRange = statusTrigger.GetInt2();
            LONG dx = (pSprite->GetPos().x - statusTrigger.GetSpecifics()) / CPathSearch::GRID_SQUARE_SIZEX;
            LONG dy = (pSprite->GetPos().y - statusTrigger.GetInt1()) / CPathSearch::GRID_SQUARE_SIZEY;
            if (dx * dx + dy * dy <= nRange * nRange) {
                bResult = TRUE;
            }
        }
        break;
    case CAITRIGGER_ONISLAND:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            bResult = TRUE;
            break;
        }

        if ((pSprite->GetBaseStats()->m_generalState & 0x800) != 0) {
            bResult = TRUE;
            break;
        }

        if ((pSprite->GetDerivedStats()->m_generalState & 0x800) != 0) {
            bResult = TRUE;
            break;
        }

        {
            LONG x = pSprite->GetPos().x;
            LONG y = pSprite->GetPos().y;
            if (x <= 1027 && y >= 105 && y <= 590) {
                if (x < 692 && y < 590) {
                    bResult = TRUE;
                } else if (x < 1018 && y < 459) {
                    bResult = TRUE;
                } else if (x < 1027 && y < 301) {
                    bResult = TRUE;
                }
            }
        }
        break;
    case CAITRIGGER_TIMERACTIVE: {
        POSITION pos = m_timers.GetHeadPosition();
        bResult = FALSE;
        while (pos != NULL) {
            CGameTimer* pTimer = m_timers.GetNext(pos);
            if (pTimer != NULL) {
                if (pTimer->GetId() == statusTrigger.GetSpecifics()) {
                    bResult = TRUE;
                }
            }
        }
        break;
    }
    case CAITRIGGER_CHECKPARTYLEVEL: {
        BYTE nMinLevel = 255;
        for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
            LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                return FALSE;
            }

            nMinLevel = min(nMinLevel, pSprite->GetDerivedStats()->m_nLevel);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        // FIXME: `pSprite` still points to the last party member, so it is
        // released a second time at the end of the function.
        bResult = statusTrigger.GetSpecifics() <= nMinLevel;
        break;
    }
    case CAITRIGGER_DIFFICULTY:
        switch (statusTrigger.GetInt1()) {
        case 1:
            bResult = g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nDifficultyLevel == statusTrigger.GetSpecifics();
            break;
        case 2:
            bResult = static_cast<LONG>(g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nDifficultyLevel) < statusTrigger.GetSpecifics();
            break;
        case 3:
            bResult = static_cast<LONG>(g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nDifficultyLevel) > statusTrigger.GetSpecifics();
            break;
        default:
            bResult = FALSE;
            break;
        }
        break;
    case CAITRIGGER_40AC:
    case CAITRIGGER_CHECKSPELLSTATE:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = pSprite->GetDerivedStats()->m_spellStates[statusTrigger.GetSpecifics()];
        break;
    case CAITRIGGER_ISHEARTOFFURYMODEON:
        bResult = g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nNightmareMode;
        break;
    case CAITRIGGER_SEQUENCE:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject == NULL) {
            break;
        }

        if (pObject->GetObjectType() == TYPE_STATIC) {
            bResult = static_cast<CGameStatic*>(pObject)->GetVidCell()->GetCurrentSequenceId() == statusTrigger.GetSpecifics();
        } else if (pObject->GetObjectType() == TYPE_SPRITE) {
            bResult = static_cast<CGameSprite*>(pObject)->GetSequence() == statusTrigger.GetSpecifics();
        }
        break;
    case CAITRIGGER_SUMMONINGLIMIT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = Icewind586B70::Instance()->GetCount() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_SUMMONINGLIMITGT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = Icewind586B70::Instance()->GetCount() > statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_SUMMONINGLIMITLT:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = Icewind586B70::Instance()->GetCount() < statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_TOTALITEMCNT:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nCount = 0;
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CGameSprite* pTarget = static_cast<CGameSprite*>(pObject);
                pTarget->GetEquipment();
                for (INT nSlot = CGameSpriteEquipment::NUM_SLOT - 1; nSlot >= 0; nSlot--) {
                    CItem* pItem = pTarget->GetEquipment()->m_items[nSlot];
                    if (pItem != NULL) {
                        CString sResRef = pItem->GetResRef().GetResRefStr();
                        if (pItem->GetMaxStackable() > 1) {
                            nCount += pTarget->GetEquipment()->m_items[nSlot]->GetUsageCount(0);
                        } else if (pItem->GetItemType() == 58) {
                            nCount += g_pBaldurChitin->GetObjectGame()->GetNumberOfItemsInBag(pItem->GetResRef(), CString(""));
                        } else if (sResRef != "00FIST"
                            && sResRef.Left(7) != "00MFIST"
                            && (pItem->GetFlagsFile() & 0x44) != 0x40) {
                            nCount++;
                        }
                    }
                }
            } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
                nCount = static_cast<CGameContainer*>(pObject)->GetTotalItemCount(CString(""));
            }

            bResult = nCount == statusTrigger.GetSpecifics();
        }
        break;
    case CAITRIGGER_TOTALITEMCNTGT:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nCount = 0;
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CGameSprite* pTarget = static_cast<CGameSprite*>(pObject);
                pTarget->GetEquipment();
                for (INT nSlot = CGameSpriteEquipment::NUM_SLOT - 1; nSlot >= 0; nSlot--) {
                    CItem* pItem = pTarget->GetEquipment()->m_items[nSlot];
                    if (pItem != NULL) {
                        CString sResRef = pItem->GetResRef().GetResRefStr();
                        if (pItem->GetMaxStackable() > 1) {
                            nCount += pItem->GetUsageCount(0);
                        } else if (pItem->GetItemType() == 58) {
                            nCount += g_pBaldurChitin->GetObjectGame()->GetNumberOfItemsInBag(pItem->GetResRef(), CString(""));
                        } else if (sResRef != "00FIST"
                            && sResRef.Left(7) != "00MFIST"
                            && (pItem->GetFlagsFile() & 0x44) != 0x40) {
                            nCount++;
                        }
                    }
                }
            } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
                nCount = static_cast<CGameContainer*>(pObject)->GetTotalItemCount(CString(""));
            }

            bResult = nCount > statusTrigger.GetSpecifics();
        }
        break;
    case CAITRIGGER_TOTALITEMCNTLT:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nCount = 0;
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CGameSprite* pTarget = static_cast<CGameSprite*>(pObject);
                pTarget->GetEquipment();
                for (INT nSlot = CGameSpriteEquipment::NUM_SLOT - 1; nSlot >= 0; nSlot--) {
                    CItem* pItem = pTarget->GetEquipment()->m_items[nSlot];
                    if (pItem != NULL) {
                        CString sResRef = pItem->GetResRef().GetResRefStr();
                        if (pItem->GetMaxStackable() > 1) {
                            nCount += pItem->GetUsageCount(0);
                        } else if (pItem->GetItemType() == 58) {
                            nCount += g_pBaldurChitin->GetObjectGame()->GetNumberOfItemsInBag(pItem->GetResRef(), CString(""));
                        } else if (sResRef != "00FIST"
                            && sResRef.Left(7) != "00MFIST"
                            && (pItem->GetFlagsFile() & 0x44) != 0x40) {
                            nCount++;
                        }
                    }
                }
            } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
                nCount = static_cast<CGameContainer*>(pObject)->GetTotalItemCount(CString(""));
            }

            bResult = nCount < statusTrigger.GetSpecifics();
        }
        break;
    case CAITRIGGER_PARTYLEVELVS:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nLevel = 0;
            INT nTotal = 0;
            for (INT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

                // NOTE: Shares into `pSprite`, which is released once more at
                // the end of the function (original bug).
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                nTotal += pSprite->GetDerivedStats()->m_nLevel;

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            INT nAverage = nTotal / g_pBaldurChitin->GetObjectGame()->GetNumCharacters();

            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                nLevel = static_cast<CGameSprite*>(pObject)->GetDerivedStats()->GetAtOffset(STAT_CLASSLEVELSUM);
            }

            bResult = nAverage == nLevel;
        }
        break;
    case CAITRIGGER_PARTYLEVELVSGT:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nLevel = 0;
            INT nTotal = 0;
            for (INT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

                // NOTE: Shares into `pSprite`, which is released once more at
                // the end of the function (original bug).
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                nTotal += pSprite->GetDerivedStats()->m_nLevel;

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            INT nAverage = nTotal / g_pBaldurChitin->GetObjectGame()->GetNumCharacters();

            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                nLevel = static_cast<CGameSprite*>(pObject)->GetDerivedStats()->GetAtOffset(STAT_CLASSLEVELSUM);
            }

            bResult = nAverage > nLevel;
        }
        break;
    case CAITRIGGER_PARTYLEVELVSLT:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nLevel = 0;
            INT nTotal = 0;
            for (INT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

                // NOTE: Shares into `pSprite`, which is released once more at
                // the end of the function (original bug).
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                nTotal += pSprite->GetDerivedStats()->GetClassMaskLevel(0xFFF);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            INT nAverage = nTotal / g_pBaldurChitin->GetObjectGame()->GetNumCharacters();

            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                nLevel = static_cast<CGameSprite*>(pObject)->GetDerivedStats()->GetAtOffset(STAT_CLASSLEVELSUM);
            }

            bResult = nAverage < nLevel;
        }
        break;
    case CAITRIGGER_TOTALITEMCNTEXCLUDE:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nCount = 0;
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CGameSprite* pTarget = static_cast<CGameSprite*>(pObject);
                pTarget->GetEquipment();
                for (INT nSlot = CGameSpriteEquipment::NUM_SLOT - 1; nSlot >= 0; nSlot--) {
                    if (pTarget->GetEquipment()->m_items[nSlot] != NULL) {
                        if (!(pTarget->GetEquipment()->m_items[nSlot]->GetResRef() == statusTrigger.GetString1())) {
                            if (pTarget->GetEquipment()->m_items[nSlot]->GetMaxStackable() > 1) {
                                nCount += pTarget->GetEquipment()->m_items[nSlot]->GetUsageCount(0);
                            } else if (pTarget->GetEquipment()->m_items[nSlot]->GetItemType() == 58) {
                                nCount += g_pBaldurChitin->GetObjectGame()->GetNumberOfItemsInBag(pTarget->GetEquipment()->m_items[nSlot]->GetResRef(), statusTrigger.GetString1());
                            } else {
                                nCount++;
                            }
                        }
                    }
                }
            } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
                // NOTE: Excludes nothing in containers (unlike the GT variant).
                nCount = static_cast<CGameContainer*>(pObject)->GetTotalItemCount(CString(""));
            }

            bResult = nCount == statusTrigger.GetSpecifics();
        }
        break;
    case CAITRIGGER_TOTALITEMCNTEXCLUDEGT:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nCount = 0;
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CGameSprite* pTarget = static_cast<CGameSprite*>(pObject);
                pTarget->GetEquipment();
                for (INT nSlot = CGameSpriteEquipment::NUM_SLOT - 1; nSlot >= 0; nSlot--) {
                    if (pTarget->GetEquipment()->m_items[nSlot] != NULL) {
                        if (!(pTarget->GetEquipment()->m_items[nSlot]->GetResRef() == statusTrigger.GetString1())) {
                            if (pTarget->GetEquipment()->m_items[nSlot]->GetMaxStackable() > 1) {
                                nCount += pTarget->GetEquipment()->m_items[nSlot]->GetUsageCount(0);
                            } else if (pTarget->GetEquipment()->m_items[nSlot]->GetItemType() == 58) {
                                nCount += g_pBaldurChitin->GetObjectGame()->GetNumberOfItemsInBag(pTarget->GetEquipment()->m_items[nSlot]->GetResRef(), statusTrigger.GetString1());
                            } else {
                                nCount++;
                            }
                        }
                    }
                }
            } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
                nCount = static_cast<CGameContainer*>(pObject)->GetTotalItemCount(statusTrigger.GetString1());
            }

            bResult = nCount > statusTrigger.GetSpecifics();
        }
        break;
    case CAITRIGGER_TOTALITEMCNTEXCLUDELT:
        if (1) {
            statusTrigger.Decode(this);

            pObject = statusTrigger.GetCause().sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
            if (pObject == NULL) {
                return FALSE;
            }

            INT nCount = 0;
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CGameSprite* pTarget = static_cast<CGameSprite*>(pObject);
                pTarget->GetEquipment();
                for (INT nSlot = CGameSpriteEquipment::NUM_SLOT - 1; nSlot >= 0; nSlot--) {
                    if (pTarget->GetEquipment()->m_items[nSlot] != NULL) {
                        if (!(pTarget->GetEquipment()->m_items[nSlot]->GetResRef() == statusTrigger.GetString1())) {
                            if (pTarget->GetEquipment()->m_items[nSlot]->GetMaxStackable() > 1) {
                                nCount += pTarget->GetEquipment()->m_items[nSlot]->GetUsageCount(0);
                            } else if (pTarget->GetEquipment()->m_items[nSlot]->GetItemType() == 58) {
                                nCount += g_pBaldurChitin->GetObjectGame()->GetNumberOfItemsInBag(pTarget->GetEquipment()->m_items[nSlot]->GetResRef(), statusTrigger.GetString1());
                            } else {
                                nCount++;
                            }
                        }
                    }
                }
            } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
                // NOTE: Excludes nothing in containers (unlike the GT variant).
                nCount = static_cast<CGameContainer*>(pObject)->GetTotalItemCount(CString(""));
            }

            bResult = nCount < statusTrigger.GetSpecifics();
        }
        break;
    case CAITRIGGER_CHECKPARTYAVERAGELEVEL:
        if (1) {
            INT nTotal = 0;
            for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

                // NOTE: The original shares party members into `pSprite`
                // without clearing it afterwards, so the last one is released
                // a second time at the end of this function.
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                nTotal += pSprite->GetDerivedStats()->GetClassMaskLevel(0xFFF);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            INT nAverage = nTotal / g_pBaldurChitin->GetObjectGame()->GetNumCharacters();

            if (statusTrigger.GetInt1() == 1) {
                bResult = nAverage == statusTrigger.GetSpecifics();
            } else if (statusTrigger.GetInt1() == 2) {
                bResult = nAverage < statusTrigger.GetSpecifics();
            } else if (statusTrigger.GetInt1() == 3) {
                bResult = nAverage > statusTrigger.GetSpecifics();
            } else {
                bResult = FALSE;
            }
        }
        break;
    case CAITRIGGER_NEARBYDIALOG:
        if (1) {
            LONG nRange = statusTrigger.GetSpecifics();
            if (nRange == 0) {
                nRange = max(GetArea()->GetInfinity()->nAreaX, GetArea()->GetInfinity()->nAreaY);
            }

            bResult = m_pArea->GetNearestDialogMatch(m_id, CResRef(statusTrigger.GetString1()), nRange) != CGameObjectArray::INVALID_INDEX;
        }
        break;
    case CAITRIGGER_CHECKAREADIFFLEVEL:
        bResult = FALSE;
        if (m_pArea != NULL) {
            LONG nLevel = statusTrigger.GetSpecifics();
            if (nLevel >= 1 && nLevel <= 3) {
                CAreaFileHeader* pHeader = m_pArea->GetHeader();
                if (pHeader->m_nPartyLevel < pHeader->m_nDifficultyLevel2 && nLevel == 1) {
                    bResult = TRUE;
                }

                if (pHeader->m_nPartyLevel >= pHeader->m_nDifficultyLevel2 && pHeader->m_nPartyLevel < pHeader->m_nDifficultyLevel3 && nLevel == 2) {
                    bResult = TRUE;
                }

                if (pHeader->m_nPartyLevel >= pHeader->m_nDifficultyLevel3 && nLevel == 3) {
                    bResult = TRUE;
                }
            }
        }
        break;
    case CAITRIGGER_NUMCREATURESLTMYLEVEL:
    case CAITRIGGER_NUMCREATURESATMYLEVEL:
    case CAITRIGGER_NUMCREATURESGTMYLEVEL:
        bResult = FALSE;
        if (GetObjectType() == TYPE_SPRITE && m_pArea != NULL) {
            statusTrigger.Decode(this);

            if (GetVertListType() == LIST_FRONT) {
                if (GetVertListPos() != NULL) {
                    m_pArea->GetAllInRange(GetVertListPos(),
                        GetPos(),
                        statusTrigger.GetCause(),
                        GetVisualRange(),
                        GetVisibleTerrainTable(),
                        targets,
                        TRUE,
                        FALSE);
                }
            } else {
                m_pArea->GetAllInRange(GetPos(),
                    statusTrigger.GetCause(),
                    GetVisualRange(),
                    GetVisibleTerrainTable(),
                    targets,
                    TRUE,
                    FALSE);
            }

            INT nCount = targets.GetCount();
            if (nCount > 0) {
                LONG nMyLevel = static_cast<CGameSprite*>(this)->GetLevel();

                if (statusTrigger.GetSpecifics() != 0) {
                    nCount = 0;

                    POSITION pos = targets.GetHeadPosition();
                    while (pos != NULL) {
                        LONG nId = reinterpret_cast<LONG>(targets.GetNext(pos));

                        CGameObject* pTarget;
                        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nId,
                            CGameObjectArray::THREAD_ASYNCH,
                            &pTarget,
                            INFINITE);
                        if (rc == CGameObjectArray::SUCCESS) {
                            if (pTarget->GetObjectType() == TYPE_SPRITE) {
                                nCount += static_cast<CGameSprite*>(pTarget)->GetLevel();
                            }

                            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nId,
                                CGameObjectArray::THREAD_ASYNCH,
                                INFINITE);
                        }
                    }
                }

                if (statusTrigger.m_triggerID == CAITRIGGER_NUMCREATURESLTMYLEVEL && nCount < nMyLevel) {
                    bResult = TRUE;
                } else if (statusTrigger.m_triggerID == CAITRIGGER_NUMCREATURESATMYLEVEL && nCount == nMyLevel) {
                    bResult = TRUE;
                } else if (statusTrigger.m_triggerID == CAITRIGGER_NUMCREATURESGTMYLEVEL && nCount > nMyLevel) {
                    bResult = TRUE;
                }
            }
        }
        break;
    case CAITRIGGER_ITEMISIDENTIFIED:
        bResult = FALSE;
        statusTrigger.Decode(this);
        if (1) {
            CString sItem(statusTrigger.GetString1());

            INT nCharacters = g_pBaldurChitin->GetObjectGame()->GetNumCharacters();
            for (INT nPortrait = 0; nPortrait < nCharacters; nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait));

                // NOTE: The original shares party members into `pSprite`
                // without clearing it afterwards, so the last one is released
                // a second time at the end of this function.
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    SHORT nSlot = pSprite->FindItemPersonal(sItem, 0, FALSE);
                    if (nSlot == -1) {
                        nSlot = pSprite->FindItemBags(sItem, 0, FALSE);
                    }

                    if (nSlot != -1) {
                        CItem* pItem = pSprite->GetEquipment()->m_items[nSlot];
                        if (pItem != NULL && (pItem->GetFlags() & 0x1) != 0) {
                            bResult = TRUE;
                        }
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    if (nSlot != -1) {
                        break;
                    }
                }
            }
        }
        break;
    case CAITRIGGER_TRIGGERSETGLOBAL:
        bResult = TRUE;
        sScope = statusTrigger.GetString1();
        sScope.MakeUpper();
        sTemp1 = sScope.Left(6);
        sTemp2 = sScope.Right(sScope.GetLength() - 6);

        if (sTemp1 == "GLOBAL") {
            CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                pVariable->SetIntValue(statusTrigger.GetSpecifics());
            } else {
                CVariable variable;
                variable.SetName(sTemp2);
                variable.SetIntValue(statusTrigger.GetSpecifics());
                g_pBaldurChitin->GetObjectGame()->GetVariables()->AddKey(variable);
            }
        } else if (sTemp1 == "LOCALS") {
            if (GetObjectType() != TYPE_SPRITE) {
                break;
            }

            CVariable* pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                pVariable->SetIntValue(statusTrigger.GetSpecifics());
            } else {
                CVariable variable;
                variable.SetName(sTemp2);
                variable.SetIntValue(statusTrigger.GetSpecifics());
                static_cast<CGameSprite*>(this)->GetLocalVariables()->AddKey(variable);
            }
        } else {
            if (sTemp1 == "MYAREA") {
                // NOTE: `m_areaName` is not NUL-terminated.
                sTemp1 = m_pArea->GetHeader()->m_areaName;
            }

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sTemp1);
            if (pArea == NULL) {
                break;
            }

            CVariable* pVariable = pArea->GetVariables()->FindKey(sTemp2);
            if (pVariable != NULL) {
                pVariable->SetIntValue(statusTrigger.GetSpecifics());
            } else {
                CVariable variable;
                variable.SetName(sTemp2);
                variable.SetIntValue(statusTrigger.GetSpecifics());
                pArea->GetVariables()->AddKey(variable);
            }
        }

        if (1) {
            CMessage* message = new CMessageSetVariable(sTemp2,
                sTemp1,
                statusTrigger.GetSpecifics(),
                0,
                m_id,
                m_id,
                0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        break;
    case CAITRIGGER_ANYPCONMAP:
        bResult = FALSE;
        if (1) {
            INT nCharacters = g_pBaldurChitin->GetObjectGame()->GetNumCharacters();
            for (INT nPortrait = 0; nPortrait < nCharacters; nPortrait++) {
                CGameSprite* pCharacter;
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait)),
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pCharacter),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    if ((pCharacter->GetDerivedStats()->m_generalState & STATE_DEAD) != 0) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait)),
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    } else {
                        if (pCharacter->GetArea() == GetArea()) {
                            bResult = TRUE;
                        }

                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait)),
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);

                        if (bResult == TRUE) {
                            break;
                        }
                    }
                }
            }
        }
        break;
    case CAITRIGGER_CHECKDOORFLAGS:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CB20(this, TYPE_DOOR, FALSE);
        if (pObject == NULL) {
            bResult = FALSE;
            break;
        }

        bResult = statusTrigger.GetSpecifics() & static_cast<CGameDoor*>(pObject)->GetFlags();
        break;
    case CAITRIGGER_ISCREATUREHIDDENINSHADOWS:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CB20(this, TYPE_SPRITE, FALSE);
        if (pObject == NULL) {
            bResult = FALSE;
            break;
        }

        bResult = static_cast<CGameSprite*>(pObject)->sub_453170();
        break;
    case CAITRIGGER_CHARGECOUNT:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject == NULL) {
            bResult = -2;
            break;
        }

        if (pObject->GetObjectType() == TYPE_CONTAINER) {
            break;
        }

        if (pObject->GetObjectType() != TYPE_SPRITE) {
            bResult = FALSE;
            break;
        }

        if (1) {
            CGameSprite* pTarget = static_cast<CGameSprite*>(pObject);

            SHORT nSlot = pTarget->FindItemPersonal(statusTrigger.GetString1(), 0, FALSE);
            if (nSlot == -1) {
                break;
            }

            CItem* pItem = pTarget->GetEquipment()->m_items[nSlot];
            if (pItem == NULL) {
                break;
            }

            WORD nCharges = pItem->GetUsageCount(statusTrigger.GetSpecifics());
            switch (statusTrigger.GetInt2()) {
            case 1:
                bResult = nCharges == statusTrigger.GetInt1();
                break;
            case 2:
                bResult = nCharges < statusTrigger.GetInt1();
                break;
            case 3:
                bResult = nCharges > statusTrigger.GetInt1();
                break;
            default:
                bResult = FALSE;
                break;
            }
        }
        break;
    case CAITRIGGER_ISACTIVE:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject == NULL) {
            break;
        }

        if (pObject->GetObjectType() == TYPE_SOUND) {
            bResult = static_cast<CGameSound*>(pObject)->IsActive();
        } else if (pObject->GetObjectType() == TYPE_SPRITE) {
            bResult = static_cast<CGameSprite*>(pObject)->GetActive();
        } else if (pObject->GetObjectType() == TYPE_TRIGGER) {
            bResult = static_cast<CGameTrigger*>(pObject)->sub_45B950();
        }
        break;
    case CAITRIGGER_INTRAP:
        statusTrigger.Decode(this);
        if (GetObjectType() == TYPE_TRIGGER) {
            // NOTE: Passes `TYPE_SPRITE` as `checkBackList`.
            pObject = statusTrigger.GetCause().sub_40CCA0(this, TYPE_SPRITE);
        }

        if (pObject != NULL) {
            bResult = static_cast<CGameTrigger*>(this)->IsOverActivate(pObject->GetPos());
        } else {
            bResult = -2;
        }
        break;
    case CAITRIGGER_ISEXTENDEDNIGHT:
        bResult = m_pArea->GetHeader()->m_areaType & 0x40;
        break;
    case CAITRIGGER_INCUTSCENEMODE:
        bResult = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 0x142;
        break;
    case CAITRIGGER_HASITEMINSLOT:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CB20(this, TYPE_SPRITE, FALSE);
        if (pObject == NULL) {
            break;
        }

        if (statusTrigger.GetSpecifics() >= CScreenInventory::PERSONAL_INVENTORY_SIZE) {
            break;
        }

        if (static_cast<CGameSprite*>(pObject)->GetEquipment()->m_items[statusTrigger.GetSpecifics()] != NULL) {
            bResult = static_cast<CGameSprite*>(pObject)->GetEquipment()->m_items[statusTrigger.GetSpecifics()]->GetResRef() == statusTrigger.GetString1();
        } else {
            bResult = statusTrigger.GetString1() == "";
        }
        break;
    case CAITRIGGER_ISROTATION:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CB20(this, TYPE_SPRITE, FALSE);
        if (pObject == NULL) {
            break;
        }

        bResult = static_cast<CGameSprite*>(pObject)->GetDirection() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_ISFACINGSAVEDROTATION:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CB20(this, TYPE_SPRITE, FALSE);
        if (pObject == NULL) {
            break;
        }

        bResult = static_cast<CGameSprite*>(pObject)->GetBaseStats()->field_2E8 == static_cast<CGameSprite*>(pObject)->GetDirection();
        break;
    case CAITRIGGER_FRAME:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject == NULL) {
            break;
        }

        if (1) {
            LONG nMax = max(statusTrigger.GetSpecifics(), statusTrigger.GetInt1());
            LONG nMin = min(statusTrigger.GetSpecifics(), statusTrigger.GetInt1());

            SHORT nFrame;
            if (pObject->GetObjectType() == TYPE_STATIC) {
                nFrame = static_cast<CGameStatic*>(pObject)->GetVidCell()->GetCurrentFrameId();
            } else if (pObject->GetObjectType() == TYPE_SPRITE) {
                nFrame = static_cast<CGameSprite*>(pObject)->GetAnimation()->m_animation->GetCurrentFrame();
            } else {
                break;
            }

            if (nFrame <= nMax && nFrame >= nMin) {
                bResult = TRUE;
            } else {
                bResult = FALSE;
            }
        }
        break;
    case CAITRIGGER_SETLASTMARKEDOBJECT:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject != NULL) {
            sub_4530F0(pObject->GetAIType());
        } else {
            sub_4530F0(CAIObjectType::NOONE);
        }
        bResult = TRUE;
        break;
    case CAITRIGGER_SETSPELLTARGET:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject != NULL) {
            sub_45B6E0(pObject->GetAIType());
        } else {
            sub_45B6E0(CAIObjectType::NOONE);
        }
        bResult = TRUE;
        break;
    case CAITRIGGER_ISPATHCRITICALOBJECT:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CCA0(this, FALSE);
        if (pObject == NULL) {
            bResult = FALSE;
            break;
        }

        bResult = (static_cast<CGameSprite*>(pObject)->GetBaseStats()->m_flags >> 13) & 1;
        break;
    case CAITRIGGER_ANYPCSEESENEMY:
        if (1) {
            CAIObjectType typeEnemy(CAIObjectType::EA_ENEMY, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0);
            bResult = FALSE;

            for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);

                CGameSprite* pCharacter;
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pCharacter),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    typeEnemy.Decode(pCharacter);

                    CGameObject* pEnemy = typeEnemy.sub_40CB20(pCharacter, TYPE_SPRITE, FALSE);
                    if (pEnemy != NULL) {
                        bResult = pCharacter->CanSeeTarget(pEnemy, FALSE);

                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pEnemy->GetId(),
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    if (bResult == TRUE) {
                        break;
                    }
                }
            }
        }
        break;
    case CAITRIGGER_FALLENPALADIN:
    case CAITRIGGER_OUTOFAMMO:
        statusTrigger.Decode(this);
        pObject = statusTrigger.GetCause().sub_40CB20(this, TYPE_SPRITE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        bResult = FALSE;
        if (pObject->GetObjectType() != TYPE_SPRITE) {
            break;
        }

        if ((static_cast<CGameSprite*>(pObject)->GetDerivedStats()->m_classMask & 0x7) == 0) {
            bResult = FALSE;
            break;
        }

        if ((static_cast<CGameSprite*>(pObject)->GetBaseStats()->m_flags & 0x200) != 0) {
            bResult = TRUE;
        }
        break;
    case CAITRIGGER_FALSE:
    case CAITRIGGER_HASINNATEABILITY:
        bResult = FALSE;
        break;
    case CAITRIGGER_ISANIMATIONID:
        bSpriteNotShared = GetTriggerSprite(statusTrigger, &pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }

        bResult = pSprite->GetAnimation()->GetAnimationId() == statusTrigger.GetSpecifics();
        break;
    case CAITRIGGER_PCINSTORE:
        bResult = FALSE;
        if (1) {
            CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
            for (SHORT nPortrait = 0; nPortrait < pGame->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = pGame->GetCharacterId(nPortrait);

                CGameSprite* pCharacter;
                BYTE rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pCharacter),
                    INFINITE);
                if (rc != CGameObjectArray::SUCCESS) {
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBase.cpp
                    // __LINE__: 6035
                    UTIL_ASSERT(FALSE);
                    return -2;
                }

                if (pCharacter->sub_45B710()) {
                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    bResult = TRUE;
                    break;
                }

                pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    default:
        break;
    }

    if (pSprite != NULL && !bSpriteNotShared) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pSprite->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    if (pObject != NULL) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    return bResult;
}

// 0x45B6D0
int CGameAIBase::sub_45B6D0()
{
    return field_58C;
}

// 0x45B6E0
void CGameAIBase::sub_45B6E0(const CAIObjectType& type)
{
    field_3BA.Set(type);
}

// 0x45D050
BOOL CGameAIBase::GetTriggerSprite(CAITrigger& trigger, CGameSprite** ppSprite)
{
    trigger.m_triggerCause.Decode(this);

    *ppSprite = static_cast<CGameSprite*>(trigger.m_triggerCause.sub_40CB20(this, CGameObject::TYPE_SPRITE, FALSE));
    if (*ppSprite != NULL) {
        if ((*ppSprite)->GetAIType().Equal(CAIObjectType::NOT_SPRITE)) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare((*ppSprite)->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            *ppSprite = NULL;
        }
    }

    return FALSE;
}

// 0x44DAC0
BOOL CGameAIBase::CanSeeTarget(CGameObject* pTarget, BOOL bIgnoreDead)
{
    if (pTarget == NULL || pTarget->m_pArea != m_pArea) {
        return FALSE;
    }

    if (pTarget->GetObjectType() == TYPE_SPRITE) {
        if (static_cast<CGameSprite*>(pTarget)->m_baseStats.field_294) {
            return FALSE;
        }

        if (!bIgnoreDead
            && (static_cast<CGameSprite*>(pTarget)->m_derivedStats.m_generalState & STATE_DEAD) != 0) {
            return FALSE;
        }
    }

    BOOL bOrderable = FALSE;

    LONG nTargetX = pTarget->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX;
    LONG nTargetY = pTarget->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY;
    LONG nX = GetPos().x / CPathSearch::GRID_SQUARE_SIZEX;
    LONG nY = GetPos().y / CPathSearch::GRID_SQUARE_SIZEY;
    LONG nDistance = (nX - nTargetX) * (nX - nTargetX) + (nY - nTargetY) * (nY - nTargetY);

    if (pTarget->GetObjectType() == TYPE_SPRITE) {
        bOrderable = static_cast<CGameSprite*>(pTarget)->Orderable(FALSE);
    }

    if (nDistance > GetVisualRange() * GetVisualRange()) {
        return FALSE;
    }

    if (!m_pArea->CheckLOS(pTarget->GetPos(), GetPos(), GetVisibleTerrainTable(), static_cast<BOOLEAN>(bOrderable))) {
        return FALSE;
    }

    return TRUE;
}

// 0x45D560
BYTE CGameAIBase::GetReaction(CGameSprite* pSprite)
{
    BYTE nReactionRoll = m_reactionRoll;
    SHORT nCHR = pSprite->m_derivedStats.m_nCHR;

    SHORT nReputation;
    if (pSprite->GetAIType().m_nEnemyAlly == CAIObjectType::EA_PC) {
        nReputation = g_pBaldurChitin->GetObjectGame()->m_nReputation / CInfGame::REPUTATION_MULTIPLIER;
    } else {
        nReputation = pSprite->m_derivedStats.m_nReputation;
    }

    BYTE nReaction = static_cast<BYTE>(atol(g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_tReactionModCharisma.GetAt(CPoint(static_cast<WORD>(nCHR) - 1, 0))));
    nReaction += static_cast<BYTE>(atol(g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_tReactionModReputation.GetAt(CPoint(static_cast<WORD>(nReputation) - 1, 0))));
    nReaction += nReactionRoll;

    if (GetObjectType() == CGameObject::TYPE_SPRITE) {
        if ((pSprite->GetAIType().m_nClassMask & CLASSMASK_RANGER) != 0) {
            if (g_pBaldurChitin->GetObjectGame()->GetRuleTables().IsHatedRace(pSprite->GetAIType(), static_cast<CGameSprite*>(this)->m_baseStats)) {
                if (nReaction < 4) {
                    return 0;
                }
                nReaction -= 4;
            }
        }
    }

    return nReaction;
}

// -----------------------------------------------------------------------------

// 0x45E250
CGameAIArea::CGameAIArea()
{
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x45E2D0
CGameAIArea::~CGameAIArea()
{
}

// 0x47C830
BOOLEAN CGameAIArea::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x766660
BOOLEAN CGameAIArea::CompressTime(DWORD deltaTime)
{
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x45E2E0
CGameAIGame::CGameAIGame()
{
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x45E2D0
CGameAIGame::~CGameAIGame()
{
}

// 0x47C830
BOOLEAN CGameAIGame::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x766660
BOOLEAN CGameAIGame::CompressTime(DWORD deltaTime)
{
    return TRUE;
}
