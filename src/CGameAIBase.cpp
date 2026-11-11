#include "CGameAIBase.h"

#include "C2DArray.h"
#include "CAIConditionResponse.h"
#include "CAIResponse.h"
#include "CAIScript.h"
#include "CAITrigger.h"
#include "CAIUtil.h"
#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CBaldurProjector.h"
#include "CButtonData.h"
#include "CCreatureFile.h"
#include "CGameArea.h"
#include "CGameContainer.h"
#include "CGameDoor.h"
#include "CGameEffect.h"
#include "CGameAnimation.h"
#include "CGameAnimationType.h"
#include "CGameEffectList.h"
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
#include "CInfButtonArray.h"
#include "CInfGame.h"
#include "CInfinity.h"
#include "CItem.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CProjectile.h"
#include "CScreenChapter.h"
#include "CScreenCharacter.h"
#include "CScreenInventory.h"
#include "CScreenWorld.h"
#include "CSound.h"
#include "CSpell.h"
#include "CTimerWorld.h"
#include "CUtil.h"
#include "CVariableHash.h"
#include "CVidCell.h"
#include "CWeather.h"
#include "Icewind586B70.h"
#include "IcewindCGameEffects.h"

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

            m_timers.RemoveAt(oldPos);
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
    CGameObject* pObject = NULL;
    SHORT actionReturn = ACTION_ERROR;
    BOOL bShared = FALSE;
    CString sError;

    if (m_curAction.m_actionID == CAIAction::BREAKINSTANTS) {
        SetCurrAction(GetNextAction(m_aiAction));
    }

    switch (m_curAction.m_actionID) {
    case 0: // NoAction()
        actionReturn = ACTION_NO_ACTION;
        break;
    case 286: // AllowAreaResting(I:TrueOrFalse*BOOLEAN)
        if (m_pArea != NULL) {
            if (m_curAction.m_specificID != 0) {
                m_pArea->m_header.m_flags &= ~0x2;
            } else {
                m_pArea->m_header.m_flags |= 0x2;
            }
            actionReturn = ACTION_DONE;
        }
        break;
    case 284: // SetScriptName(O:Object*,S:NewScriptName*)
        pObject = GetTargetShareType(CGameObject::TYPE_AIBASE);
        if (pObject != NULL) {
            CGameAIBase* pTarget = static_cast<CGameAIBase*>(pObject);

            // FIXME: The original never initializes this pointer. It reads
            // the stack slot of `sError` (i.e. the empty string buffer) on
            // the paths where no message was created, then either flags it
            // (`|= 4`) or posts it. Initialized to `NULL` here.
            CMessageSetVariable* pMessage = NULL;

            CString sOldName(pTarget->m_scriptName, SCRIPTNAME_SIZE);
            CString sNewName = m_curAction.GetString1();
            if (sNewName == "") {
                sNewName = "none";
            }

            if (sOldName.CompareNoCase(sNewName) != 0) {
                CGameArea* pArea = pTarget->m_pArea;
                if (pArea != NULL) {
                    if (pArea->GetNamedCreatures()->RemoveKey(sOldName, TRUE, pTarget->m_id) == TRUE) {
                        CString sAreaName;
                        pArea->m_resRef.CopyToString(sAreaName);

                        pMessage = new CMessageSetVariable(sOldName,
                            sAreaName,
                            pTarget->m_id,
                            FALSE,
                            pTarget->m_id,
                            pTarget->m_id,
                            2);

                        CVariable var;
                        var.SetName(sNewName);
                        var.m_intValue = pTarget->m_id;
                        g_pBaldurChitin->GetObjectGame()->m_namedCreatures.AddKey(var);
                    }
                }

                if (g_pBaldurChitin->GetObjectGame()->GetNamedCreatures()->RemoveKey(sOldName, TRUE, pTarget->m_id) == TRUE) {
                    if (pMessage == NULL) {
                        pMessage = new CMessageSetVariable(sOldName,
                            CString("GLOBAL"),
                            pTarget->m_id,
                            FALSE,
                            pTarget->m_id,
                            pTarget->m_id,
                            4);
                    } else {
                        pMessage->m_dwFlags |= 4;
                    }

                    CVariable var;
                    var.SetName(sNewName);
                    var.m_intValue = pTarget->m_id;
                    g_pBaldurChitin->GetObjectGame()->m_namedCreatures.AddKey(var);
                }
            }

            if (pMessage != NULL) {
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }

            // NOTE: The original never releases the share obtained above.
            pObject = NULL;
        }
        actionReturn = ACTION_DONE;
        break;
    case 282: // OverrideAreaDifficulty(S:AreaName*,I:Delta*)
        {
            CString sArea = m_curAction.GetString1();
            if (sArea != "") {
                LONG nDelta = m_curAction.m_specificID;
                if (nDelta != 0) {
                    if (nDelta > 1) {
                        nDelta = 1;
                    } else if (nDelta < -1) {
                        nDelta = -1;
                    }

                    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
                    CInfGameUnknownEntry* pEntry = pGame->FindUnknownEntry(sArea);
                    if (pEntry == NULL) {
                        pEntry = new CInfGameUnknownEntry(sArea);
                        pGame->field_4BDC.AddTail(reinterpret_cast<CString*>(pEntry));
                    }

                    if (pEntry != NULL) {
                        pEntry->field_8 = static_cast<BYTE>(nDelta);
                        pEntry->field_4 = (pEntry->field_4 & ~0x1) | 0x2;
                    }
                }
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 283: // SetCriticalPathObject(O:Object*,I:Critical*Boolean)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            bShared = TRUE;

            if (m_curAction.m_specificID != 0) {
                pSprite->m_baseStats.m_flags |= 0x2000;
            } else {
                pSprite->m_baseStats.m_flags &= ~0x2000;
            }

            if (pSprite->InControl()) {
                CMessage* message = new CMessageSpriteUpdate(pSprite,
                    pSprite->m_id,
                    pSprite->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 63: // Wait(I:Time*)
    case 205: // NOTE: Not in IWD2's ACTION.IDS.
        actionReturn = Wait();
        break;
    case 7: // CreateCreature(S:NewObject*,S:ScriptName*,P:Location*,I:Face*)
    case 228: // NOTE: Not in IWD2's ACTION.IDS (BG2: CreateCreatureImpassable).
        actionReturn = CreateCreature();
        break;
    case 227: // NOTE: Not in IWD2's ACTION.IDS (BG2: CreateCreatureObject).
        pObject = GetTargetShareType(CGameObject::TYPE_AIBASE);
        bShared = TRUE;
        actionReturn = CreateCreatureObject(pObject);
        break;
    case 16: // GiveOrder(O:Object*,I:Order*)
        pObject = GetTargetShareType(CGameObject::TYPE_AIBASE);
        bShared = TRUE;
        actionReturn = GiveOrder(static_cast<CGameAIBase*>(pObject));
        break;
    case 26: // PlaySound(S:Sound*)
        actionReturn = PlaySound(CPoint(0, 0), FALSE);
        break;
    case 320: // PlaySoundPoint(S:Sound*,P:Target*)
        actionReturn = PlaySound(m_curAction.m_dest, TRUE);
        break;
    case 30: // SetGlobal(S:Name*,S:Area*,I:Value*)
    case 306: // SetGlobalRandom(S:Name*,S:Area*,I:Min*,I:Max*)
        actionReturn = SetGlobal();
        break;
    case 49: // MoveViewPoint(P:Target*,I:ScrollSpeed*Scroll)
    case 264: // MoveViewPointUntilDone(P:Target*,I:ScrollSpeed*Scroll)
        actionReturn = MoveViewPoint();
        break;
    case 50: // MoveViewObject(O:Target*,I:ScrollSpeed*Scroll)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
        } else {
            bShared = TRUE;
            actionReturn = MoveViewObject(pObject);
        }
        break;
    case 59: // MoveCursorPoint(P:Target*,I:ScrollSpeed*Scroll)
        actionReturn = MoveCursorPoint();
        break;
    case 51: // ClickLButtonPoint(P:Target*,I:ScrollSpeed*Scroll)
        actionReturn = ClickLButtonPoint();
        break;
    case 52: // ClickLButtonObject(O:Target*,I:ScrollSpeed*Scroll)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        actionReturn = ClickLButtonObject(pObject);
        break;
    case 55: // DoubleClickLButtonPoint(P:Target*,I:ScrollSpeed*Scroll)
        actionReturn = DoubleClickLButtonPoint();
        break;
    case 56: // DoubleClickLButtonObject(O:Target*,I:ScrollSpeed*Scroll)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        actionReturn = DoubleClickLButtonObject(pObject);
        break;
    case 53: // ClickRButtonPoint(P:Target*,I:ScrollSpeed*Scroll)
        actionReturn = ClickRButtonPoint();
        break;
    case 54: // ClickRButtonObject(O:Target*,I:ScrollSpeed*Scroll)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        actionReturn = ClickRButtonObject(pObject);
        break;
    case 57: // DoubleClickRButtonPoint(P:Target*,I:ScrollSpeed*Scroll)
        actionReturn = DoubleClickRButtonPoint();
        break;
    case 58: // DoubleClickRButtonObject(O:Target*,I:ScrollSpeed*Scroll)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        actionReturn = DoubleClickRButtonObject(pObject);
        break;
    case 232: // StartRandomTimer(I:TimerID*,I:MinTime*,I:MaxTime*)
        {
            m_curAction.m_actionID = CAIAction::STARTTIMER;

            LONG nMin = min(m_curAction.m_specificID2, m_curAction.m_specificID3);
            LONG nMax = max(m_curAction.m_specificID2, m_curAction.m_specificID3);
            LONG nRange = nMax - nMin + 1;
            if (nRange != 0) {
                m_curAction.m_specificID2 = rand() % nRange + nMin;
            } else {
                m_curAction.m_specificID2 = nMin;
            }
        }
        actionReturn = StartTimer();
        break;
    case 61: // StartTimer(I:ID*,I:Time*)
        actionReturn = StartTimer();
        break;
    case 62: // SendTrigger(O:Target*,I:TriggerNum*)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        actionReturn = SendTrigger(static_cast<CGameAIBase*>(pObject));
        break;
    case 285: // SmallWaitRandom(I:Min*,I:Max*)
        {
            LONG nMin = m_curAction.m_specificID;
            LONG nMax = m_curAction.m_specificID2;
            if (nMin > nMax) {
                nMin = m_curAction.m_specificID2;
                nMax = m_curAction.m_specificID;
            }

            LONG nRange = nMax - nMin;
            m_curAction.m_specificID = (nRange != 0 ? rand() % nRange : 0) + nMin;
        }
        actionReturn = SmallWait();
        break;
    case 83: // SmallWait(I:Time*)
        actionReturn = SmallWait();
        break;
    case 64: // UndoExplore()
        {
            CMessage* message = new CMessageSetAreaExplored(FALSE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 65: // Explore()
        {
            CMessage* message = new CMessageSetAreaExplored(TRUE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 66: // DayNight(I:TimeOfDay*Time)
        g_pBaldurChitin->GetObjectGame()->m_worldTime.AdvanceCurrentTime(m_curAction.m_specificID
            * CTimerWorld::TIMESCALE_MIN_PER_HOUR
            * CTimerWorld::TIMESCALE_SEC_PER_MIN
            * CTimerWorld::TIMESCALE_MSEC_PER_SEC);
        actionReturn = ACTION_DONE;
        break;
    case 67: // Weather(I:Weather*Weather)
        g_pBaldurChitin->m_pEngineWorld->m_weather.SetCurrentWeather(m_pArea,
            g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime,
            static_cast<BYTE>(m_curAction.m_specificID));
        actionReturn = ACTION_DONE;
        break;
    case 68: // CallLightning(O:Target*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            m_pArea->m_cInfinity.CallLightning(pObject->GetPos().x, pObject->GetPos().y);

            CGameEffectDeath* pEffect = new CGameEffectDeath();
            // NOTE: The inlined constructor here stores 0 (the default
            // constructor stores 1).
            pEffect->field_18C = 0;
            pEffect->m_effectAmount = 0;
            pEffect->m_dwFlags = 0x100;

            CMessage* message = new CMessageAddEffect(pEffect, m_id, pObject->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 113: // ForceSpell(O:Target,I:Spell*Spell)
    case 181: // ReallyForceSpell(O:Target,I:Spell*Spell)
        pObject = GetTargetShareType(CGameObject::TYPE_AIBASE);
        bShared = TRUE;
        actionReturn = ForceSpell(static_cast<CGameAIBase*>(pObject));
        break;
    case 114: // ForceSpellPoint(P:Target,I:Spell*Spell)
        actionReturn = ForceSpellPoint();
        break;
    case 115: // SetGlobalTimer(S:Name*,S:Area*,I:Time*GTimes)
    case 307: // SetGlobalTimerRandom(S:Name*,S:Area*,I:Min*,I:Max*)
    case 308: // SetGlobalTimerOnce(S:Name*,S:Area*,I:Time*GTimes)
        actionReturn = SetGlobalTimer();
        break;
    case 109: // IncrementGlobal(S:Name*,S:Area*,I:Value*)
        actionReturn = IncrementGlobal();
        break;
    case 120: // StartCutScene(S:CutScene*)
        actionReturn = StartCutScene();
        break;
    case 125: // Deactivate(O:Object*)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        if (pObject != NULL) {
            if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pObject->m_id) == -1) {
                CMessage* message = new CMessageSetActive(FALSE, m_id, pObject->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 126: // Activate(O:Object*)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        if (pObject != NULL) {
            CMessage* message = new CMessageSetActive(TRUE, m_id, pObject->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 131: // Kill(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                pSprite->sub_761650();

                CGameEffectDeath* pEffect = new CGameEffectDeath();
                pEffect->m_effectAmount = 0;
                pEffect->m_dwFlags = 4;
                pEffect->m_sourceID = pSprite->m_id;

                CMessage* message = new CMessageAddEffect(pEffect, m_id, pSprite->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 132: // VerbalConstant(O:Object*,I:Constant*soundOff)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            CMessage* message = new CMessageVerbalConstant(m_curAction.m_specificID,
                m_id,
                pObject->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 123: // ClearAllActions()
        actionReturn = ClearAllActions();
        break;
    case 121: // StartCutSceneMode()
        {
            CMessage* message = new CMessageCutSceneModeStatus(TRUE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 122: // EndCutSceneMode()
        {
            CMessage* message = new CMessageCutSceneModeStatus(FALSE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 133: // ClearActions(O:Object*)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
        } else {
            actionReturn = ClearActions(pObject);

            CMessage* message = new CMessageAddAction(CAIAction(CAIAction::NO_ACTION, CPoint(-1, -1), 0, -1),
                m_id,
                pObject->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        break;
    case 136: // UnlockScroll()
        g_pBaldurChitin->m_pEngineWorld->m_scrollLockId = CGameObjectArray::INVALID_INDEX;
        actionReturn = ACTION_DONE;
        break;
    case 82: // CreateItem(S:ResRef*,I:Usage1*,I:Usage2*,I:Usage3*)
        actionReturn = CreateItem();
        break;
    case 140: // GiveItemCreate(S:ResRef*,O:Object*,I:Usage1*,I:Usage2*,I:Usage3*)
        // NOTE: Uninline.
        pObject = GetTargetShare();
        bShared = TRUE;
        actionReturn = GiveItemCreate(pObject);
        break;
    case 116: // TakePartyItem(S:Item*)
    case 188: // TakePartyItemAll(S:Item*)
    case 193: // TakePartyItemRange(S:Item*,I:Range*)
    case 204: // TakePartyItemNum(S:Item*,I:Num)
        actionReturn = TakePartyItem();
        break;
    case 117: // TakePartyGold(I:Amount*)
        actionReturn = TakePartyGold();
        break;
    case 141: // GivePartyGoldGlobal(S:Name*,S:Area*)
        actionReturn = GivePartyGoldGlobal();
        break;
    case 151: // DisplayString(O:Object*,I:StrRef*)
        pObject = GetTargetShareType(CGameObject::TYPE_AIBASE);
        bShared = TRUE;
        actionReturn = DisplayString(static_cast<CGameAIBase*>(pObject));
        break;
    case 153: // ChangeEnemyAlly(O:Object*,I:Value*EA)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            CAIObjectType typeAI;
            CAIObjectType liveTypeAI;
            CAIObjectType startTypeAI;

            typeAI.Set(pSprite->GetAIType());
            typeAI.m_nEnemyAlly = static_cast<BYTE>(m_curAction.m_specificID);

            liveTypeAI.Set(pSprite->m_liveTypeAI);
            liveTypeAI.m_nEnemyAlly = static_cast<BYTE>(m_curAction.m_specificID);

            startTypeAI.Set(pSprite->m_startTypeAI);
            startTypeAI.m_nEnemyAlly = static_cast<BYTE>(m_curAction.m_specificID);

            pSprite->SetAIType(typeAI, TRUE, FALSE);
            pSprite->m_liveTypeAI.Set(liveTypeAI);
            pSprite->m_startTypeAI.Set(startTypeAI);

            if (pSprite->InControl()) {
                CMessageSpriteUpdate* pMessage = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            } else if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
                && g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene == TRUE) {
                CMessageSpriteUpdate* pMessage = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE, CMessage::BROADCAST_FORCED_OTHERS);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 154: // ChangeGeneral(O:Object*,I:Value*General)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            CAIObjectType typeAI;
            CAIObjectType liveTypeAI;
            CAIObjectType startTypeAI;

            typeAI.Set(pSprite->GetAIType());
            typeAI.m_nGeneral = static_cast<BYTE>(m_curAction.m_specificID);

            liveTypeAI.Set(pSprite->m_liveTypeAI);
            liveTypeAI.m_nGeneral = static_cast<BYTE>(m_curAction.m_specificID);

            // NOTE: Start type is copied from live type (unlike in
            // `ChangeEnemyAlly`).
            startTypeAI.Set(pSprite->m_liveTypeAI);
            startTypeAI.m_nGeneral = static_cast<BYTE>(m_curAction.m_specificID);

            pSprite->SetAIType(typeAI, TRUE, FALSE);
            pSprite->m_liveTypeAI.Set(liveTypeAI);
            pSprite->m_startTypeAI.Set(startTypeAI);

            if (pSprite->InControl()) {
                CMessageSpriteUpdate* pMessage = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 155: // ChangeRace(O:Object*,I:Value*Race)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            CAIObjectType typeAI;
            CAIObjectType liveTypeAI;
            CAIObjectType startTypeAI;

            typeAI.Set(pSprite->GetAIType());
            typeAI.m_nRace = static_cast<BYTE>(m_curAction.m_specificID);

            liveTypeAI.Set(pSprite->m_liveTypeAI);
            liveTypeAI.m_nRace = static_cast<BYTE>(m_curAction.m_specificID);

            // NOTE: Start type is copied from live type.
            startTypeAI.Set(pSprite->m_liveTypeAI);
            startTypeAI.m_nRace = static_cast<BYTE>(m_curAction.m_specificID);

            pSprite->SetAIType(typeAI, TRUE, FALSE);
            pSprite->m_liveTypeAI.Set(liveTypeAI);
            pSprite->m_startTypeAI.Set(startTypeAI);

            if (pSprite->InControl()) {
                CMessageSpriteUpdate* pMessage = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 156: // ChangeClass(O:Object*,I:Value*Class)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            // NOTE: The original constructs three `CAIObjectType` locals and
            // does nothing else (the body was probably compiled out).
            CAIObjectType typeAI;
            CAIObjectType liveTypeAI;
            CAIObjectType startTypeAI;
        }
        actionReturn = ACTION_DONE;
        break;
    case 157: // ChangeSpecifics(O:Object*,I:Value*Specific)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            CAIObjectType typeAI;
            CAIObjectType liveTypeAI;
            CAIObjectType startTypeAI;

            typeAI.Set(pSprite->GetAIType());
            typeAI.m_nSpecific = static_cast<BYTE>(m_curAction.m_specificID);

            liveTypeAI.Set(pSprite->m_liveTypeAI);
            liveTypeAI.m_nSpecific = static_cast<BYTE>(m_curAction.m_specificID);

            // NOTE: Start type is copied from live type.
            startTypeAI.Set(pSprite->m_liveTypeAI);
            startTypeAI.m_nSpecific = static_cast<BYTE>(m_curAction.m_specificID);

            pSprite->SetAIType(typeAI, TRUE, FALSE);
            pSprite->m_liveTypeAI.Set(liveTypeAI);
            pSprite->m_startTypeAI.Set(startTypeAI);

            if (pSprite->InControl()) {
                CMessageSpriteUpdate* pMessage = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 158: // ChangeGender(O:Object*,I:Value*Gender)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            CAIObjectType typeAI;
            CAIObjectType liveTypeAI;
            CAIObjectType startTypeAI;

            typeAI.Set(pSprite->GetAIType());
            typeAI.m_nGender = static_cast<BYTE>(m_curAction.m_specificID);

            liveTypeAI.Set(pSprite->m_liveTypeAI);
            liveTypeAI.m_nGender = static_cast<BYTE>(m_curAction.m_specificID);

            // NOTE: Start type is copied from live type.
            startTypeAI.Set(pSprite->m_liveTypeAI);
            startTypeAI.m_nGender = static_cast<BYTE>(m_curAction.m_specificID);

            pSprite->SetAIType(typeAI, TRUE, FALSE);
            pSprite->m_liveTypeAI.Set(liveTypeAI);
            pSprite->m_startTypeAI.Set(startTypeAI);

            if (pSprite->InControl()) {
                CMessageSpriteUpdate* pMessage = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 159: // ChangeAlignment(O:Object*,I:Value*ALIGNMNT)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            CAIObjectType typeAI;
            CAIObjectType liveTypeAI;
            CAIObjectType startTypeAI;

            typeAI.Set(pSprite->GetAIType());
            typeAI.m_nAlignment = static_cast<BYTE>(m_curAction.m_specificID);

            liveTypeAI.Set(pSprite->m_liveTypeAI);
            liveTypeAI.m_nAlignment = static_cast<BYTE>(m_curAction.m_specificID);

            // NOTE: Start type is copied from live type.
            startTypeAI.Set(pSprite->m_liveTypeAI);
            startTypeAI.m_nAlignment = static_cast<BYTE>(m_curAction.m_specificID);

            pSprite->SetAIType(typeAI, TRUE, FALSE);
            pSprite->m_liveTypeAI.Set(liveTypeAI);
            pSprite->m_startTypeAI.Set(startTypeAI);

            if (pSprite->InControl()) {
                CMessageSpriteUpdate* pMessage = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 160: // ApplySpell(O:Target,I:Spell*Spell)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL
            && m_curAction.m_acteeID.m_nInstance != CGameObjectArray::INVALID_INDEX) {
            CAIObjectType type;
            type.m_nInstance = m_curAction.m_acteeID.m_nInstance;
            pObject = GetTargetShareType(type, CGameObject::TYPE_SPRITE);
        }
        bShared = TRUE;
        actionReturn = ApplySpell(static_cast<CGameAIBase*>(pObject));
        break;
    case 161: // IncrementChapter(S:RESREF*)
        if (m_curAction.m_specificID2 == 0) {
            m_curAction.m_specificID = -1;
        }
        actionReturn = IncrementChapter();
        break;
    case 162: // ReputationSet(I:Reputation*)
        {
            CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
            pGame->ReputationAdjustment(static_cast<SHORT>((m_curAction.m_specificID - pGame->m_nReputation / CInfGame::REPUTATION_MULTIPLIER) * CInfGame::REPUTATION_MULTIPLIER),
                FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 163: // ReputationInc(I:Reputation*)
        g_pBaldurChitin->GetObjectGame()->ReputationAdjustment(static_cast<SHORT>(m_curAction.m_specificID * CInfGame::REPUTATION_MULTIPLIER),
            FALSE);
        actionReturn = ACTION_DONE;
        break;
    case 164: // AddexperienceParty(I:XP*)
        g_pBaldurChitin->GetObjectGame()->AddPartyXP(m_curAction.m_specificID, 0, 0);
        actionReturn = ACTION_DONE;
        break;
    case 165: // AddexperiencePartyGlobal(S:Name*,S:Area*)
        actionReturn = AddExperiencePartyGlobal();
        break;
    case 167: // StartMovie(S:ResRef*)
        actionReturn = StartMovie();
        break;
    case 170: // RevealAreaOnMap(S:ResRef*)
        actionReturn = RevealAreaOnMap();
        break;
    case 118: // GivePartyGold(I:Amount*)
    case 171: // GiveGoldForce(I:Amount*)
        actionReturn = GivePartyGold();
        break;
    case 172: // ChangeTileState(O:Tile*, I:State*Boolean)
        pObject = GetTargetShareType(CGameObject::TYPE_TILED_OBJECT);
        bShared = TRUE;
        actionReturn = ChangeTileState(static_cast<CGameTiledObject*>(pObject));
        break;
    case 173: // AddJournalEntry(I:Entry*)
        g_pBaldurChitin->GetObjectGame()->m_cJournal.AddEntry(m_curAction.m_specificID, 0);
        actionReturn = ACTION_DONE;
        break;
    case 177: // TriggerActivation(O:Object*,I:State*Boolean)
        pObject = GetTargetShareType(CGameObject::TYPE_TRIGGER);
        bShared = TRUE;
        actionReturn = TriggerActivation(static_cast<CGameTrigger*>(pObject));
        break;
    case 143: // OpenDoor(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_DOOR);
        bShared = TRUE;
        actionReturn = OpenDoor(static_cast<CGameDoor*>(pObject));
        break;
    case 144: // CloseDoor(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_DOOR);
        bShared = TRUE;
        actionReturn = CloseDoor(static_cast<CGameDoor*>(pObject));
        break;
    case 183: // MultiPlayerSync()
        actionReturn = MultiPlayerSync();
        break;
    case 185: // SetMasterArea(S:Name*)
        {
            actionReturn = ACTION_DONE;

            CPoint ptLocation(-1, -1);
            if (g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_tMasterArea.Find(m_curAction.GetString1(), ptLocation, TRUE)) {
                CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(m_curAction.GetString1());
                if (pArea == NULL) {
                    actionReturn = ACTION_ERROR;
                    break;
                }

                g_pBaldurChitin->GetObjectGame()->m_pGameAreaMaster = pArea;
            } else {
                // NOTE: Built and thrown away (debug leftover).
                CString sMessage = m_curAction.GetString1() + ": That's no master area, that's my wife!";
                actionReturn = ACTION_ERROR;
            }
        }
        break;
    case 265: // ExpansionEndCredits()
        CInfGame::byte_8E7528 = TRUE;
        // FALLTHROUGH
    case 186: // EndCredits()
        g_pBaldurChitin->m_pEngineWorld->ReadyEndCredits(FALSE);
        actionReturn = ACTION_DONE;
        break;
    case 187: // StartMusic(I:Slot*musics,I:Flags*MFLAGS)
        actionReturn = StartMusic();
        break;
    case 324: // SetMusic(I:SetSlot*MUSICS,I:With*MUSIC)
        actionReturn = SetMusic();
        break;
    case 189: // LeaveAreaLUAPanic(S:Area*,S:Parchment*,P:Point*,I:Face*)
        actionReturn = LeaveAreaLUAPanic();
        break;
    case 190: // FinalSave()
        actionReturn = FinalSave();
        break;
    case 195: // Lock(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_AIBASE);
        bShared = TRUE;
        actionReturn = Lock(static_cast<CGameAIBase*>(pObject));
        break;
    case 196: // Unlock(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_AIBASE);
        bShared = TRUE;
        actionReturn = Unlock(static_cast<CGameAIBase*>(pObject));
        break;
    case 197: // MoveGlobal(S:Area*,O:Object*,P:Point*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        bShared = TRUE;
        actionReturn = MoveGlobal(static_cast<CGameSprite*>(pObject));
        break;
    case 169: // DestroyItem(S:ResRef*)
        actionReturn = DestroyItem();
        break;
    case 199: // TextScreen(S:TextList*)
        {
            // NOTE: Unconditional assertion failure in the original, the
            // message below is never reached in practice.
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBase.cpp
            // __LINE__: 1439
            UTIL_ASSERT(FALSE);

            CMessageStartTextScreen* pMessage = new CMessageStartTextScreen(CResRef(m_curAction.GetString1()),
                m_id,
                m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 201: // DetectSecretDoor(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_DOOR);
        bShared = TRUE;
        actionReturn = DetectSecretDoor(static_cast<CGameDoor*>(pObject));
        break;
    case 202: // FadeToColor(P:Point*,I:Blue*)
        actionReturn = FadeToColor();
        break;
    case 203: // FadeFromColor(P:Point*,I:Blue*)
        actionReturn = FadeFromColor();
        break;
    case 253: // FadeToAndFromColor(P:Point*,I:Blue*)
        actionReturn = sub_467970();
        break;
    case 209: // NOTE: Not in IWD2's ACTION.IDS (BG2: SpawnPtActivate(O:Object*)).
        pObject = GetTargetShareType(CGameObject::TYPE_SPAWNING);
        bShared = TRUE;
        actionReturn = SpawnPtActivate(static_cast<CGameSpawning*>(pObject));
        break;
    case 210: // NOTE: Not in IWD2's ACTION.IDS (BG2: SpawnPtDeactivate(O:Object*)).
        pObject = GetTargetShareType(CGameObject::TYPE_SPAWNING);
        bShared = TRUE;
        actionReturn = SpawnPtDeactivate(static_cast<CGameSpawning*>(pObject));
        break;
    case 211: // NOTE: Not in IWD2's ACTION.IDS (BG2: SpawnPtSpawn(O:Object*)).
        pObject = GetTargetShareType(CGameObject::TYPE_SPAWNING);
        bShared = TRUE;
        actionReturn = SpawnPtSpawn(static_cast<CGameSpawning*>(pObject));
        break;
    case 213: // NOTE: Not in IWD2's ACTION.IDS (BG2: StaticStart(O:Object*)).
        pObject = GetTargetShareType(CGameObject::TYPE_STATIC);
        bShared = TRUE;
        actionReturn = StaticStart(static_cast<CGameStatic*>(pObject), TRUE);
        break;
    case 214: // NOTE: Not in IWD2's ACTION.IDS (BG2: StaticStop(O:Object*)).
        pObject = GetTargetShareType(CGameObject::TYPE_STATIC);
        bShared = TRUE;
        actionReturn = StaticStart(static_cast<CGameStatic*>(pObject), FALSE);
        break;
    case 216: // NOTE: Not in IWD2's ACTION.IDS (familiar add).
        {
            CMessageFamiliarAdd* pMessage = new CMessageFamiliarAdd(TRUE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 217: // NOTE: Not in IWD2's ACTION.IDS (familiar remove).
        {
            CMessageFamiliarAdd* pMessage = new CMessageFamiliarAdd(FALSE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 218: // NOTE: Not in IWD2's ACTION.IDS.
        if (GetObjectType() != CGameObject::TYPE_SPRITE) {
            actionReturn = ACTION_ERROR;
            break;
        }
        AutoPause(0x100);
        actionReturn = ACTION_DONE;
        break;
    case 106: // Shout(I:ID*)
    case 212: // NOTE: Not in IWD2's ACTION.IDS.
        actionReturn = Shout();
        break;
    case 220: // NOTE: Not in IWD2's ACTION.IDS.
    case 226: // NOTE: Not in IWD2's ACTION.IDS.
        actionReturn = sub_464200();
        break;
    case 230: // RestParty()
        g_pBaldurChitin->GetObjectGame()->RestParty(1, 1);
        actionReturn = ACTION_DONE;
        break;
    case 243: // IncrementGlobalOnce(S:String1*,S:String2*,S:String3*,S:String4*,I:Value*)
        {
            CString sTemp;
            CString sName1;
            CString sScope1;
            CString sName2;
            CString sScope2;
            int nIndex;

            sTemp = m_curAction.GetString1();
            sTemp.MakeUpper();
            nIndex = sTemp.Find(':');
            sScope1 = sTemp.Left(nIndex);
            sScope1.TrimLeft();
            sScope1.TrimRight();
            sName1 = sTemp.Right(sTemp.GetLength() - nIndex - 1);
            sName1.TrimLeft();
            sName1.TrimRight();

            sTemp = m_curAction.GetString2();
            sTemp.MakeUpper();
            nIndex = sTemp.Find(':');
            sScope2 = sTemp.Left(nIndex);
            sScope2.TrimLeft();
            sScope2.TrimRight();
            sName2 = sTemp.Right(sTemp.GetLength() - nIndex - 1);
            sName2.TrimLeft();
            sName2.TrimRight();

            CVariable* pVariable;
            if (strcmp(sScope1, "GLOBAL") == 0) {
                pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName1);
            } else {
                pVariable = g_pBaldurChitin->GetObjectGame()->GetArea(sScope1)->GetVariables()->FindKey(sName1);
            }

            if (pVariable == NULL || pVariable->m_intValue != 1) {
                LONG nValue = m_curAction.m_specificID;

                m_curAction.SetString1(sScope1 + sName1);
                m_curAction.m_specificID = 1;
                SetGlobal();

                m_curAction.SetString1(sScope2 + sName2);
                m_curAction.m_specificID = nValue;
                IncrementGlobal();
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 244: // DeleteJournalEntry(I:Strref)
        g_pBaldurChitin->GetObjectGame()->m_cJournal.DeleteEntry(m_curAction.m_specificID);
        actionReturn = ACTION_DONE;
        break;
    case 245: // JournalEntryDone(I:Strref)
        g_pBaldurChitin->GetObjectGame()->m_cJournal.SetQuestDone(m_curAction.m_specificID);
        actionReturn = ACTION_DONE;
        break;
    case 247: // BitGlobal(S:String1*,S:String2*,I:Value,I:Mode*BitMode)
        actionReturn = BitGlobal();
        break;
    case 248: // GlobalBitGlobal(S:String1*,S:String2*,S:String3*,S:String4*,I:Mode*BitMode)
        actionReturn = GlobalBitGlobal();
        break;
    case 252: // ChangeCurrentScript(S:Script*)
        m_curAction.m_specificID = m_curScriptNum;
        // FALLTHROUGH
    case 60: // ChangeAIScript(S:ScriptFile*,I:Level*Scrlev)
        actionReturn = ChangeAIScript();
        break;
    case 254: // SetHP(O:Target,I:HP*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            bShared = TRUE;
            if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                if (m_curAction.m_specificID > pSprite->m_derivedStats.GetAtOffset(STAT_MAXHITPOINTS)) {
                    pSprite->m_baseStats.m_hitPoints = static_cast<SHORT>(pSprite->m_derivedStats.GetAtOffset(STAT_MAXHITPOINTS));
                } else {
                    pSprite->m_baseStats.m_hitPoints = static_cast<SHORT>(m_curAction.m_specificID);
                }
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 304: // AddHP(O:Target,I:HP*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            bShared = TRUE;
            if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                LONG nHP = m_curAction.m_specificID;
                SHORT nCurrentHP = pSprite->m_baseStats.m_hitPoints;
                if (nCurrentHP + nHP < pSprite->m_derivedStats.GetAtOffset(STAT_MAXHITPOINTS)) {
                    pSprite->m_baseStats.m_hitPoints = static_cast<SHORT>(m_curAction.m_specificID + nCurrentHP);
                } else {
                    pSprite->m_baseStats.m_hitPoints = static_cast<SHORT>(pSprite->m_derivedStats.GetAtOffset(STAT_MAXHITPOINTS));
                }
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 241: // FloatMessage(O:Object*,I:STRREF*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            pObject = GetTargetShareType(CGameObject::TYPE_DOOR);
        }
        if (pObject != NULL) {
            bShared = TRUE;
            if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                CMessageFloatText* pMessage = new CMessageFloatText(m_id,
                    pObject->GetId(),
                    m_curAction.m_specificID,
                    FALSE);
                if (pObject->InControl()) {
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
                } else {
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE, CMessage::BROADCAST_FORCED);
                }
            } else {
                pObject->FloatText(m_curAction.m_specificID, 10, 5);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 238: // AddXPVar(S:VarTableEntry*,I:Strref*)
        {
            CResRef resRef;
            resRef = "exptable";

            C2DArray tXPTable;
            tXPTable.Load(resRef);

            CPoint ptLocation;
            BOOLEAN bNotFound;
            {
                // NOTE: The original passes the `GetString1` temporary
                // directly.
                CString sName = m_curAction.GetString1();
                bNotFound = !tXPTable.Find(sName, ptLocation, TRUE);
            }

            if (bNotFound) {
                actionReturn = ACTION_ERROR;
                break;
            }

            ptLocation.x = 0;

            if (m_curAction.m_specificID != 0) {
                STR_RES strRes;
                if (g_pBaldurChitin->GetTlkTable().Fetch(m_curAction.m_specificID, strRes)) {
                    g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                        strRes.szText,
                        RGB(200, 255, 200),
                        RGB(200, 255, 200),
                        -1,
                        m_id,
                        m_id);
                }
            }

            g_pBaldurChitin->GetObjectGame()->AddPartyXP(tXPTable.GetAtLong(ptLocation), 0, 0);
            actionReturn = ACTION_DONE;
        }
        break;
    case 235: // PlaySequence(O:Target*,I:Sequence*Sequence)
        pObject = GetTargetShare();
        if (pObject != NULL
            && m_curAction.GetSpecifics() >= CGameSprite::SEQ_ATTACK
            && m_curAction.GetSpecifics() <= CGameSprite::SEQ_SLEEP) {
            bShared = TRUE;

            CMessage* message = new CMessageSetSequence(static_cast<BYTE>(m_curAction.GetSpecifics()),
                m_id,
                pObject->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        // NOTE: When the sequence is out of range, `bShared` is still `FALSE`
        // and the shared target is released with `ReleaseDeny`.
        actionReturn = ACTION_DONE;
        break;
    case 258: // RestUntilHealed()
        {
            STRREF strError;
            if (g_pBaldurChitin->GetObjectGame()->CanRestParty(strError, 0, 1, (m_curAction.GetFlags() & 0x2) == 0x2) == TRUE) {
                CInfGame::m_bHealPartyOnRest = TRUE;
                g_pBaldurChitin->GetObjectGame()->RestParty(TRUE, TRUE);
                CInfGame::m_bHealPartyOnRest = FALSE;
                actionReturn = ACTION_DONE;
                break;
            }

            STR_RES strRes;
            if (g_pBaldurChitin->GetTlkTable().Fetch(strError, strRes)) {
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    strRes.szText,
                    RGB(0, 255, 0),
                    RGB(0, 255, 0),
                    -1,
                    GetId(),
                    GetId());
            }

            actionReturn = ACTION_DONE;
        }
        break;
    case 259: // SetVisualRange(I:Range*)
        {
            CMessage* message = new CMessage92(m_id, m_id, m_curAction.GetSpecifics());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            actionReturn = ACTION_DONE;
        }
        break;
    case 260: // JumpToPointInstant(O:Object*,P:Point*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;
        static_cast<CGameSprite*>(pObject)->JumpToPoint(m_curAction.GetDest(), FALSE);
        actionReturn = ACTION_DONE;
        break;
    case 266: // DisplayMessage(I:StrRef*)
        {
            STR_RES strRes;
            if (g_pBaldurChitin->GetTlkTable().Fetch(m_curAction.GetSpecifics(), strRes)) {
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    strRes.szText,
                    RGB(160, 200, 215),
                    RGB(160, 200, 215),
                    -1,
                    GetId(),
                    GetId());
            }

            actionReturn = ACTION_DONE;
        }
        break;
    case 267: // ExportParty(S:Name*)
        {
            CString sFileName;
            int nCount = 1;

            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                26827,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);

            for (int nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait));

                CGameSprite* pSprite;
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    if ((pSprite->GetBaseStats()->m_flags & 0x800) != 0) {
                        sFileName.Format("%s%d", (LPCSTR)m_curAction.GetString1(), nCount);
                        g_pBaldurChitin->GetObjectGame()->CharacterExport(nCharacterId, sFileName);
                        nCount++;
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }

            actionReturn = ACTION_DONE;
        }
        break;
    case 268: // ClearPartyEffects()
        for (int nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
            LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait));

            CGameSprite* pSprite;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if ((pSprite->GetBaseStats()->m_flags & 0x800) != 0) {
                    pSprite->sub_761990();
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        actionReturn = ACTION_DONE;
        break;
    case 270: // ClearSpriteEffects(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            bShared = TRUE;
            static_cast<CGameSprite*>(pObject)->sub_761990();
        }

        actionReturn = ACTION_DONE;
        break;
    case 271: // StopJoinRequests()
        if (field_596 == 0) {
            field_595 = g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetListenToJoinOption();
            g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetListenToJoinOption(FALSE, TRUE);
            field_596++;
        }

        actionReturn = ACTION_DONE;
        break;
    case 272: // ResetJoinRequests()
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetListenToJoinOption(field_595, TRUE);
        field_596 = 0;

        actionReturn = ACTION_DONE;
        break;
    case 273: // HideGUI()
        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            CMessage* message = new CMessageToggleInterface(TRUE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else {
            // NOTE: `m_bHidden` is read through a tiny getter (0x45B240).
            if (!g_pBaldurChitin->GetScreenWorld()->GetManager()->m_bHidden) {
                g_pBaldurChitin->GetScreenWorld()->HideInterface();
                g_pBaldurChitin->GetScreenWorld()->sub_453790(TRUE);
            }
        }

        actionReturn = ACTION_DONE;
        break;
    case 274: // UnhideGUI()
        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            CMessage* message = new CMessageToggleInterface(FALSE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else {
            if (g_pBaldurChitin->GetScreenWorld()->sub_453780()) {
                g_pBaldurChitin->GetScreenWorld()->UnhideInterface();
                g_pBaldurChitin->GetScreenWorld()->sub_453790(FALSE);
            }
        }

        actionReturn = ACTION_DONE;
        break;
    case 275: // SaveGame(I:STRREF*)
        if (!g_pChitin->cNetwork.GetSessionOpen()
            || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
            CMessage* message = new CMessageSaveGame(m_curAction.GetSpecifics(), m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        actionReturn = ACTION_DONE;
        break;
    case 278: // DropInventoryEXExclude(S:Resref*,O:Object*)
        pObject = GetTargetShare();
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            actionReturn = static_cast<CGameSprite*>(pObject)->DropInventory(FALSE, m_curAction.GetString1());
        } else if (pObject->GetObjectType() == CGameObject::TYPE_CONTAINER) {
            actionReturn = static_cast<CGameContainer*>(pObject)->DropInventoryEx(m_curAction.GetString1());
        } else {
            actionReturn = ACTION_ERROR;
        }
        break;
    case 281: // MarkObject(O:Object*)
        pObject = GetTargetShare();
        if (pObject != NULL) {
            bShared = TRUE;
            sub_4530F0(pObject->GetAIType());
        } else {
            sub_4530F0(CAIObjectType::NOONE);
        }

        actionReturn = ACTION_DONE;
        break;
    case 305: // SetMyTarget(O:Object*)
        pObject = GetTargetShare();
        if (pObject != NULL) {
            bShared = TRUE;
            sub_453110(pObject->GetAIType());
        } else {
            sub_453110(CAIObjectType::NOONE);
        }

        actionReturn = ACTION_DONE;
        break;
    case 279: // AdvanceTime(I:Seconds*)
        g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->AdvanceCurrentTime(m_curAction.GetSpecifics());
        g_pBaldurChitin->GetObjectGame()->FeedBack(CInfGame::FEEDBACK_TIMEPASSEDREST,
            m_curAction.GetSpecifics(),
            TRUE);

        actionReturn = ACTION_DONE;
        break;
    case 280: // NOTE: Not in IWD2's ACTION.IDS. Applies fatigue (effect 93).
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;
        if (m_curAction.GetSpecifics() != 0) {
            actionReturn = ACTION_ERROR;
            break;
        }

        {
            ITEM_EFFECT effect;
            CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FATIGUE);
            effect.dwFlags = 1;
            effect.effectAmount = m_curAction.GetSpecifics2();
            // NOTE: Read from an unnamed `const WORD` at 0x8491AA.
            effect.durationType = 1;

            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                m_pos,
                m_id,
                CPoint(-1, -1));

            CMessage* message = new CMessageAddEffect(pEffect, m_id, pObject->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        actionReturn = ACTION_DONE;
        break;
    case 287: // SetHPPercent(O:Object*,I:Percent*,I:Flags*HPFlags)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            bShared = TRUE;

            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            if ((pSprite->GetDerivedStats()->m_generalState & STATE_DEAD) == 0
                && (pSprite->GetBaseStats()->m_generalState & STATE_DEAD) == 0) {
                float fPercent = static_cast<float>(m_curAction.GetSpecifics());
                if (fPercent > 100.0f) {
                    fPercent = 100.0f;
                } else if (fPercent < 0.0f) {
                    fPercent = 0.0f;
                }
                fPercent /= 100.0f;

                if (m_curAction.GetSpecifics2() == 0) {
                    float fHitPoints = static_cast<float>(pSprite->GetBaseStats()->m_hitPoints);
                    if (static_cast<float>(pSprite->GetDerivedStats()->m_nMaxHitPoints) * fPercent > fHitPoints) {
                        pSprite->GetBaseStats()->m_hitPoints = static_cast<SHORT>(static_cast<float>(pSprite->GetDerivedStats()->m_nMaxHitPoints) * fPercent);
                    }
                } else {
                    pSprite->GetBaseStats()->m_hitPoints = static_cast<SHORT>(static_cast<float>(pSprite->GetBaseStats()->m_hitPoints) * fPercent);
                }
            }
        }

        actionReturn = ACTION_DONE;
        break;
    case 288: // SetDoorFlag(O:Object*,I:Flags*DoorFlag,I:Value*BOOLEAN)
        pObject = GetTargetShareType(CGameObject::TYPE_DOOR);
        if (pObject != NULL) {
            bShared = TRUE;

            CGameDoor* pDoor = static_cast<CGameDoor*>(pObject);
            DWORD dwFlags = pDoor->GetFlags();
            if (m_curAction.GetSpecifics2() != 0) {
                dwFlags |= m_curAction.GetSpecifics();
            } else {
                dwFlags &= ~m_curAction.GetSpecifics();
            }
            pDoor->SetFlags(dwFlags);

            CMessage* message = new CMessageDoorStatus(pDoor, m_id, pDoor->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        actionReturn = ACTION_DONE;
        break;
    case 289: // SpellCastEffect(O:Source*,S:Voice*,S:Sound1*,S:Sound2*,I:Animation*sceffect,I:Speed*,I:Sequence*Sequence)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;
        actionReturn = static_cast<CGameSprite*>(pObject)->SpellCastEffect(m_curAction.GetString1(),
            m_curAction.GetString2(),
            m_curAction.GetSpecifics(),
            m_curAction.GetSpecifics2(),
            m_curAction.GetSpecifics3());
        break;
    case 290: // SpellHitEffectPoint(O:Source*,P:Target*,I:Effect*sheffect,I:Height*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            bShared = TRUE;
            actionReturn = static_cast<CGameSprite*>(pObject)->SpellHitEffect(static_cast<CGameAIBase*>(pObject),
                m_curAction.GetDest(),
                0,
                m_curAction.GetSpecifics(),
                m_curAction.GetSpecifics2());
            break;
        }

        pObject = GetTargetShareType(CGameObject::TYPE_CONTAINER);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;
        actionReturn = static_cast<CGameContainer*>(pObject)->SpellHitEffect(static_cast<CGameAIBase*>(pObject),
            pObject->GetPos(),
            pObject->GetId(),
            m_curAction.GetSpecifics(),
            m_curAction.GetSpecifics2());
        break;
    case 291: // SpellHitEffectSprite(O:Source*,O:Target*,I:Effect*sheffect,I:Height*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;

        {
            CGameObject* pTarget = m_curAction.GetActee2().sub_40CB20(this, CGameObject::TYPE_SPRITE, FALSE);
            if (pTarget == NULL) {
                actionReturn = ACTION_ERROR;
                break;
            }

            // NOTE: `pTarget` is not released here.
            if (pTarget->GetArea() == NULL) {
                actionReturn = ACTION_ERROR;
                break;
            }

            actionReturn = static_cast<CGameSprite*>(pObject)->SpellHitEffect(static_cast<CGameAIBase*>(pObject),
                pTarget->GetPos(),
                pTarget->GetId(),
                m_curAction.GetSpecifics(),
                m_curAction.GetSpecifics2());

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pTarget->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        break;
    case 292: // ChunkCreature(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;
        static_cast<CGameSprite*>(pObject)->sub_761650();

        {
            // NOTE: Leaks `pItemEffect`.
            ITEM_EFFECT* pItemEffect = new ITEM_EFFECT;
            if (pItemEffect == NULL) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBase.cpp
                // __LINE__: 2286
                UTIL_ASSERT_MSG(FALSE, "Memory Allocation problem");
            } else {
                CGameEffect::ClearItemEffect(pItemEffect, CGAMEEFFECT_DEATH);
                pItemEffect->dwFlags = 8;
                pItemEffect->numDice = 0;
                pItemEffect->effectAmount = 49;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(pItemEffect,
                    CPoint(-1, -1),
                    -1,
                    CPoint(-1, -1));

                CMessage* message = new CMessageAddEffect(pEffect, m_id, pObject->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        actionReturn = ACTION_DONE;
        break;
    case 293: // UnHide(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            break;
        }

        bShared = TRUE;

        if (static_cast<CGameSprite*>(pObject)->sub_453170() == 1) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            pSprite->sub_453160(0);
            pSprite->SetModalState(0, TRUE);
            pSprite->SetStealthGreyOut(CGameSprite::STEALTH_GREYOUT_DURATION);

            if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId()) != CGameObjectArray::INVALID_INDEX) {
                g_pBaldurChitin->GetObjectGame()->GetButtonArray()->SetSelectedButton(100);

                CBaldurEngine* pEngine = g_pBaldurChitin->GetActiveEngine();
                if (pEngine->GetSelectedCharacter() == g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId())
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 102
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 101
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 104
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 103
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 105
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 112
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 115
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 116
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 117
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 118
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 119
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 120
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 121
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 122
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 123
                    && g_pBaldurChitin->GetObjectGame()->GetButtonArray()->GetState() != 106) {
                    g_pBaldurChitin->GetObjectGame()->GetButtonArray()->ResetState();
                }
            }

            pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
                CGAMEEFFECT_INVISIBLE,
                pSprite->GetEquipedEffectList()->GetPosCurrent(),
                -1);

            ITEM_EFFECT effect;
            CGameEffect::ClearItemEffect(&effect, ICEWIND_CGAMEEFFECT_REMOVEALLOFTYPE);
            effect.dwFlags = CGAMEEFFECT_INVISIBLE;
            effect.effectAmount = 0;

            // NOTE: The source position is `CPoint(DWORD)` constructed from
            // the sprite id (MFC's inline `CPoint(DWORD)` at 0x452B00), not
            // the sprite position.
            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                CPoint(static_cast<DWORD>(pSprite->GetId())),
                pSprite->GetId(),
                CPoint(-1, -1));

            // FIXME: Target is `m_id` (the caller), not the unhidden sprite.
            CMessageAddEffect* message = new CMessageAddEffect(pEffect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        actionReturn = ACTION_DONE;
        break;
    case 294: // XEquipItem(S:Item*,O:Object*,I:Slot*Slots,I:EquipUnEquip*BOOLEAN)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_DONE;
            break;
        }

        bShared = TRUE;
        actionReturn = static_cast<CGameSprite*>(pObject)->XEquipItem(m_curAction.GetString1(),
            m_curAction.GetSpecifics(),
            m_curAction.GetSpecifics2());
        break;
    case 295: // TransferInventory(O:Source*,O:Target*)
        pObject = GetTargetShare();
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;

        {
            CGameObject* pTarget = m_curAction.GetActee2().sub_40CCA0(this, FALSE);
            if (pTarget != NULL) {
                actionReturn = pObject->TransferInventory(pTarget);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pTarget->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    case 296: // ChangeStoreMarkup(S:Store*,I:BuyMarkup*,I:SellMarkup*,I:AdditionalMarkup*)
        g_pBaldurChitin->GetObjectGame()->ChangeStoreMarkup(CResRef(m_curAction.GetString1()),
            m_curAction.GetSpecifics(),
            m_curAction.GetSpecifics2(),
            m_curAction.GetSpecifics3());
        actionReturn = ACTION_DONE;
        break;
    case 297: // ScreenShake(I:Duration*,I:MagX*,I:MagY*)
        {
            CMessageScreenShake* message = new CMessageScreenShake(static_cast<WORD>(m_curAction.GetSpecifics()),
                static_cast<CHAR>(m_curAction.GetSpecifics2()),
                static_cast<CHAR>(m_curAction.GetSpecifics3()),
                TRUE,
                m_id,
                m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        actionReturn = ACTION_DONE;
        break;
    case 312: // SetApparentNameSTRREF(O:Object*,I:StrRef*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            bShared = TRUE;

            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            if (static_cast<DWORD>(m_curAction.GetSpecifics()) != pSprite->GetBaseStats()->m_apparentName) {
                pSprite->GetBaseStats()->m_apparentName = m_curAction.GetSpecifics();

                CMessageSpriteUpdate* message = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 313: // SetRegularNameSTRREF(O:Object*,I:StrRef*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            bShared = TRUE;

            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            if (static_cast<DWORD>(m_curAction.GetSpecifics()) != pSprite->GetBaseStats()->m_name) {
                pSprite->GetBaseStats()->m_name = m_curAction.GetSpecifics();

                CMessageSpriteUpdate* message = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 298: // FaceSavedLocation(O:Object*)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject != NULL) {
            bShared = TRUE;

            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            pSprite->SetDirection(static_cast<SHORT>(pSprite->GetBaseStats()->field_2E8));

            if (pSprite->GetPos() != CPoint(-1, -1) || pSprite->GetArea() != NULL) {
                CMessageSpriteUpdate* message = new CMessageSpriteUpdate(pSprite,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 233: // HideCreature(O:Object*,I:State*Boolean)
        {
            BYTE bHide = static_cast<BYTE>(m_curAction.GetSpecifics());

            pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
            if (pObject == NULL) {
                break;
            }

            bShared = TRUE;
            actionReturn = static_cast<CGameSprite*>(pObject)->sub_75F3D0(bHide);

            if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                CMessage90* message = new CMessage90(pObject->GetId(),
                    pObject->GetId(),
                    bHide);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        break;
    case 301: // SetExtendedNight(I:TrueOrFalse*BOOLEAN)
        {
            WORD areaType = m_pArea->GetHeader()->m_areaType;
            if (m_curAction.GetSpecifics() != 0) {
                areaType |= 0x40;
            } else {
                areaType &= ~0x40;
            }

            m_pArea->GetHeader()->m_areaType = areaType;
            m_pArea->GetInfinity()->SetAreaType(areaType);

            if (InControl()) {
                CMessageSetAreaType* message = new CMessageSetAreaType(areaType, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 303: // SetRestEncounterChance(I:DayProb*,I:NightProb*)
        {
            CAreaFileRestEncounter* pRestEncounter = m_pArea->GetHeaderRestEncounter();
            if (pRestEncounter != NULL) {
                // NOTE: Uninline `min` helper (0x45EA80).
                LONG nMaxProb = 100;
                LONG nProb = m_curAction.GetSpecifics();
                pRestEncounter->m_probDay = static_cast<WORD>(min(nProb, nMaxProb));

                nMaxProb = 100;
                nProb = m_curAction.GetSpecifics2();
                pRestEncounter->m_probNight = static_cast<WORD>(min(nProb, nMaxProb));
            }

            if (InControl()) {
                // FIXME: `pRestEncounter` can be NULL here.
                CMessageSetAreaRestEncounter* message = new CMessageSetAreaRestEncounter(pRestEncounter->m_probDay,
                    pRestEncounter->m_probNight,
                    m_id,
                    m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        actionReturn = ACTION_DONE;
        break;
    case 314: // EndGame(I:StrRef*)
        {
            g_pBaldurChitin->GetObjectGame()->ReadyCharacterTerminationSequence(3, m_curAction.GetSpecifics());

            // NOTE: Area id is passed as caller and target.
            BYTE nAreaId = g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->GetId();

            CMessageEndGame* message = new CMessageEndGame(3,
                m_curAction.GetSpecifics(),
                nAreaId,
                nAreaId);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE, CMessage::BROADCAST_OTHERS);
        }
        actionReturn = ACTION_DONE;
        break;
    case 315: // AddExperiencePartyCR(I:CR*)
        g_pBaldurChitin->GetObjectGame()->AddPartyXP(m_curAction.GetSpecifics(), 0, 1);
        actionReturn = ACTION_DONE;
        break;
    case 36: // Continue()
    case 178: // BreakInstants()
    case 234: // Debug(S:Message*)
    case 246: // Log(S:String*)
        actionReturn = ACTION_DONE;
        break;
    case 316: // AddFeat(O:Object*,I:Value*Feats)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;

        {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            LONG nFeatNumber = m_curAction.GetSpecifics();
            if (pSprite->sub_763A40(nFeatNumber, 0)) {
                CCreatureFileHeader oldBaseStats;
                oldBaseStats = *pSprite->GetBaseStats();

                pSprite->SetFeatValue(nFeatNumber, pSprite->GetFeatValue(nFeatNumber) + 1);
                pSprite->UpdateFeats(&oldBaseStats, pSprite->GetDerivedStats());
                pSprite->sub_4531B0();
                pSprite->ProcessEffectList();
            }
        }

        actionReturn = ACTION_DONE;
        break;
    case 242: // ChangeStat(O:Object*,I:Stat*Stats,I:Value*,I:Modifier*StatMod)
        {
            // FIXME: The share is never released.
            CGameSprite* pSprite = static_cast<CGameSprite*>(GetTargetShareType(CGameObject::TYPE_SPRITE));
            if (pSprite == NULL) {
                break;
            }

            LONG nStat = m_curAction.GetSpecifics();
            LONG nValue = m_curAction.GetSpecifics2();
            BOOLEAN bModify = static_cast<BOOLEAN>(m_curAction.GetSpecifics3());
            pSprite->SetAtOffset(nStat, nValue, bModify);

            actionReturn = ACTION_DONE;

            if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                CMessageChangeStat* message = new CMessageChangeStat(static_cast<WORD>(nStat),
                    nValue,
                    bModify,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        break;
    case 251: // DropInventoryEX(O:Object)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;
        actionReturn = static_cast<CGameSprite*>(pObject)->DropInventory(FALSE, CString(""));

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
            && !pObject->InControl()
            && g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene == TRUE) {
            CMessageSpriteEquipment* message = new CMessageSpriteEquipment(static_cast<CGameSprite*>(pObject),
                pObject->GetId(),
                pObject->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE, CMessage::BROADCAST_FORCED_OTHERS);
        }
        break;
    case 318: // SetCreatureAreaFlag(O:Object*,I:Flag*CREAREFL,I:Value*BOOLEAN)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            actionReturn = ACTION_ERROR;
            break;
        }

        bShared = TRUE;
        actionReturn = ACTION_DONE;

        if (m_curAction.GetSpecifics2() != 0) {
            static_cast<CGameSprite*>(pObject)->GetBaseStats()->m_flags |= m_curAction.GetSpecifics();
        } else {
            static_cast<CGameSprite*>(pObject)->GetBaseStats()->m_flags &= ~m_curAction.GetSpecifics();
        }
        break;
    case 323: // WaitAnimation(O:Target*,I:Sequence*Sequence)
        pObject = GetTargetShareType(CGameObject::TYPE_SPRITE);
        if (pObject == NULL) {
            break;
        }

        bShared = TRUE;

        if (static_cast<CGameSprite*>(pObject)->GetSequence() == m_curAction.GetSpecifics()) {
            actionReturn = ACTION_INTERRUPTABLE;
        } else {
            actionReturn = ACTION_DONE;
        }
        break;
    case 325: // SetAreaFlags(I:Flags*AREAFLAG,I:BitMode*BITMODE)
        // NOTE: The three valid modes leave `actionReturn` at its initial
        // `ACTION_ERROR`.
        if (m_curAction.GetSpecifics2() == 1) {
            GetArea()->GetHeader()->m_flags &= m_curAction.GetSpecifics();
        } else if (m_curAction.GetSpecifics2() == 2) {
            GetArea()->GetHeader()->m_flags |= m_curAction.GetSpecifics();
        } else if (m_curAction.GetSpecifics2() == 3) {
            GetArea()->GetHeader()->m_flags ^= m_curAction.GetSpecifics();
        } else {
            actionReturn = ACTION_ERROR;
        }
        break;
    case 1: // ActionOverride(O:Actor*,A:Action*)
        break;
    default:
        sError.Format("Error: Action number: %d not implemented", m_curAction.GetActionID());
        actionReturn = ACTION_ERROR;
        break;
    }

    if (pObject != NULL) {
        if (bShared) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pObject->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    SetLastActionReturn(actionReturn);

    return actionReturn;
}

// 0x45BDD0
CGameObject* CGameAIBase::GetTargetShareType(BYTE objectType)
{
    CGameObject* pObject = m_curAction.m_acteeID.sub_40CCA0(this, FALSE);
    if (pObject != NULL
        && pObject->GetObjectType() == CGameObject::TYPE_SPRITE
        && static_cast<CGameSprite*>(pObject)->m_derivedStats.m_cImmunitiesAIType.OnList(m_typeAI)) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->m_id,
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
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            UpdateTarget(NULL);
            return NULL;
        }

        if (GetObjectType() == CGameObject::TYPE_SPRITE
            && g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1
            && (static_cast<CGameSprite*>(this)->m_derivedStats.m_generalState & STATE_BLIND) != 0
            && pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            const CPoint& ptObject = pObject->GetPos();
            CPoint ptEnd(ptObject.x / CPathSearch::GRID_SQUARE_SIZEX,
                ptObject.y / CPathSearch::GRID_SQUARE_SIZEY);
            CPoint ptStart(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);
            LONG nSquares = CAIUtil::CountSquares(ptStart, ptEnd);
            if (nSquares > static_cast<CGameSprite*>(this)->m_animation.GetPersonalSpace() / 2 + 4) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->m_id,
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

// 0x45C290
CGameObject* CGameAIBase::GetTargetShare()
{
    CGameObject* pObject = m_curAction.m_acteeID.sub_40CCA0(this, FALSE);
    if (pObject != NULL
        && pObject->GetObjectType() == CGameObject::TYPE_SPRITE
        && static_cast<CGameSprite*>(pObject)->m_derivedStats.m_cImmunitiesAIType.OnList(m_typeAI)) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->m_id,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObject = NULL;
    }

    UpdateTarget(pObject);

    return pObject;
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
    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.m_idLocalPlayer != m_remotePlayerID) {
        return;
    }

    if (m_curAction.m_actionID == CAIAction::NO_ACTION) {
        if (m_queuedActions.GetCount() != 0) {
            SetCurrAction(GetNextAction(m_aiAction));
            m_interrupt = FALSE;
        }

        if (m_curAction.m_actionID == CAIAction::NO_ACTION) {
            m_inCutScene = FALSE;
        }
    }

    m_randValue = rand() % 0x7FFF;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (!pGame->m_worldTime.m_active && !m_firstCall) {
        if (m_curAction.m_actionID == CAIAction::NO_ACTION) {
            SetCurrAction(GetNextAction(m_aiAction));
            m_interrupt = FALSE;
        }

        BOOL bInstant = g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_lInstantActions.Find(m_curAction.m_actionID) != NULL;
        while (!(g_pChitin->cNetwork.GetSessionOpen() == TRUE && m_curAction.m_actionID == CAIACTION_LEAVEAREALUA)
            && bInstant) {
            DoAction();
            bInstant = g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_lInstantActions.Find(m_curAction.m_actionID) != NULL;
        }
        return;
    }

    field_44A++;
    if (((field_44A ^ m_id) & 0xF) == 0) {
        CheckTimers(1);
    }

    if (((field_44A ^ m_id) & 0xF) == 0 || m_firstCall || m_forceActionPick) {
        field_54C++;
        m_firstCall = FALSE;
        m_forceActionPick = FALSE;

        if (m_curAction.m_actionID != CAIAction::NO_ACTION
            && m_pendingTriggers.GetCount() == 0
            && field_550 < 120
            && field_552 <= 0) {
            field_550++;
        } else {
            if (m_pendingTriggers.GetCount() != 0) {
                field_552 = 5;
            }

            ProcessPendingTriggers();

            if (field_552 > 0) {
                field_552--;
            }

            field_550 = 0;
        }
    }

    if (m_curAction.m_actionID == CAIAction::NO_ACTION) {
        SetCurrAction(GetNextAction(m_aiAction));
        m_interrupt = FALSE;
    }

    if (m_posVertList != NULL) {
        BOOL bInstant;
        do {
            DoAction();
            bInstant = g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_lInstantActions.Find(m_curAction.m_actionID) != NULL;
        } while (!(g_pChitin->cNetwork.GetSessionOpen() == TRUE && m_curAction.m_actionID == CAIACTION_LEAVEAREALUA)
            && bInstant);
    }
}

// 0x45CA10
void CGameAIBase::ProcessPendingTriggers()
{
    CAIResponse* pResponse = NULL;
    SHORT scriptNum;
    BOOL bContinue;

    if (m_inCutScene) {
        return;
    }

    if (m_nLastActionReturn == 0) {
        return;
    }

    CAIResponse response;
    scriptNum = 0;
    CTypedPtrList<CPtrList, CAITrigger*> triggerList(10);
    CAITrigger trigger(CAITrigger::SEE, 0);

    ApplyTriggers();

    if (m_overrideScript != NULL) {
        pResponse = m_overrideScript->Find(m_pendingTriggers, this);
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if ((!pGame->m_bPartyAI && pGame->GetCharacterPortraitNum(m_id) != -1)
        || m_curAction.m_actionID == CAIACTION_PROTECTPOINT
        || m_curAction.m_actionID == CAIACTION_PROTECTOBJECT
        || (m_curAction.m_actionID == CAIACTION_MOVETOPOINT && m_curAction.m_specificID3 == 1)) {
        if (pResponse != NULL) {
            if (pResponse->m_actionList.GetHeadPosition() != NULL) {
                pResponse->m_scriptNum = 0;
                InsertResponse(*pResponse, TRUE, TRUE);

                POSITION pos = m_pendingTriggers.GetHeadPosition();
                while (pos != NULL) {
                    CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
                    if (pTrigger != NULL) {
                        delete pTrigger;
                    }
                }
                m_pendingTriggers.RemoveAll();
            }

            delete pResponse;
        }
        return;
    }

    if (pResponse != NULL) {
        response.Add(*pResponse);
        bContinue = pResponse->InListEnd(CAIAction::CONTINUE);
    } else {
        bContinue = FALSE;
    }

    BOOL bProcessNext = FALSE;
    if (pResponse == NULL
        || pResponse->m_actionList.GetHeadPosition() == NULL
        || bContinue) {
        bProcessNext = TRUE;
    }

    BOOL bScriptsAllowed = TRUE;
    if (GetObjectType() == CGameObject::TYPE_SPRITE
        && (static_cast<CGameSprite*>(this)->m_derivedStats.m_generalState & STATE_CHARMED) != 0
        && static_cast<CGameSprite*>(this)->m_startTypeAI.m_nEnemyAlly != static_cast<CGameSprite*>(this)->m_liveTypeAI.m_nEnemyAlly) {
        bScriptsAllowed = FALSE;
    }

    if (bProcessNext && bScriptsAllowed) {
        CAIScript* scripts[5];
        scripts[0] = m_special1Script;
        scripts[1] = m_teamScript;
        scripts[2] = m_special2Script;
        scripts[3] = m_combatScript;
        scripts[4] = m_special3Script;

        // NOTE: The original unrolls this loop.
        BOOL bDone = FALSE;
        for (int index = 0; index < 5; index++) {
            if (pResponse != NULL) {
                delete pResponse;
            }
            pResponse = NULL;

            scriptNum = index + 1;

            CAIScript* pScript = scripts[index];
            if (pScript != NULL && !pScript->IsEmpty()) {
                pResponse = pScript->Find(m_pendingTriggers, this);

                if (pResponse != NULL) {
                    response.Add(*pResponse);
                    bContinue = pResponse->InListEnd(CAIAction::CONTINUE);
                } else {
                    bContinue = FALSE;
                }

                if (pResponse != NULL
                    && pResponse->m_actionList.GetHeadPosition() != NULL
                    && !bContinue) {
                    bDone = TRUE;
                    break;
                }
            }
        }

        if (!bDone) {
            if (pResponse != NULL) {
                delete pResponse;
            }
            pResponse = NULL;

            scriptNum = 6;

            if (m_movementScript != NULL && !m_movementScript->IsEmpty()) {
                pResponse = m_movementScript->Find(m_pendingTriggers, this);
                if (pResponse != NULL) {
                    response.Add(*pResponse);
                    pResponse->InListEnd(CAIAction::CONTINUE);
                }
            }
        }
    }

    if (response.m_actionList.GetHeadPosition() != NULL) {
        response.m_scriptNum = scriptNum;
        InsertResponse(response, TRUE, TRUE);

        POSITION pos = m_pendingTriggers.GetHeadPosition();
        while (pos != NULL) {
            CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
            if (pTrigger != NULL) {
                delete pTrigger;
            }
        }
        m_pendingTriggers.RemoveAll();
    }

    if (pResponse != NULL) {
        delete pResponse;
    }
}

// 0x45EB60
SHORT CGameAIBase::PlaySound(CPoint pos, BOOL bUsePoint)
{
    CSound sound;

    if (!bUsePoint) {
        pos = m_pos;
    }

    // NOTE: Uninline.
    sound.SetResRef(CResRef(m_curAction.GetString1()), TRUE, TRUE);

    if (sound.m_nLooping != 0) {
        return ACTION_ERROR;
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        // NOTE: Inlined. The channel is not stored.
        CMessage* message = new CMessagePlaySoundPoint(sound.GetResRef(),
            0,
            pos,
            m_id,
            m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    sound.SetFireForget(TRUE);
    sound.SetChannel(2, reinterpret_cast<DWORD>(m_pArea));
    sound.Play(pos.x, pos.y, 0, FALSE);

    return ACTION_DONE;
}

// 0x45EDE0
SHORT CGameAIBase::SetGlobal()
{
    CString sString = m_curAction.GetString1();
    sString.MakeUpper();

    CString sScope = sString.Left(6);
    CString sName = sString.Right(sString.GetLength() - 6);

    CVariable var;

    // SetGlobalRandom
    if (m_curAction.m_actionID == 306) {
        sName = sString.Right(sString.GetLength() - 7);

        if (m_curAction.m_specificID < m_curAction.m_specificID2) {
            m_curAction.m_specificID = rand() % (m_curAction.m_specificID2 - m_curAction.m_specificID + 1) + m_curAction.m_specificID;
        } else if (m_curAction.m_specificID > m_curAction.m_specificID2) {
            m_curAction.m_specificID = rand() % (m_curAction.m_specificID - m_curAction.m_specificID2 + 1) + m_curAction.m_specificID2;
        }
    }

    LONG nValue = m_curAction.m_specificID;

    CMessage* message;
    if (strcmp(sScope, "GLOBAL") == 0) {
        CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        if (pVariable != NULL) {
            pVariable->m_intValue = nValue;
        } else {
            var.SetName(sName);
            var.m_intValue = nValue;
            g_pBaldurChitin->GetObjectGame()->m_variables.AddKey(var);
        }

        message = new CMessageSetVariable(sName, sScope, nValue, FALSE, m_id, m_id, 0);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    } else if (strcmp(sScope, "LOCALS") == 0) {
        if (GetObjectType() == TYPE_SPRITE) {
            CString sVariable(sName);

            CGameSprite* pSprite = static_cast<CGameSprite*>(this);
            CVariable* pVariable = pSprite->GetLocalVariables()->FindKey(sVariable);
            if (pVariable != NULL) {
                pVariable->m_intValue = nValue;
            } else {
                var.SetName(sVariable);
                var.m_intValue = nValue;
                pSprite->m_pLocalVariables->AddKey(var);
            }

            message = new CMessageSetVariable(sVariable, sScope, nValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    } else {
        if (strcmp(sScope, "MYAREA") == 0) {
            // NOTE: `m_areaName` is a `RESREF`, it's not guaranteed to be
            // NULL-terminated.
            sScope = reinterpret_cast<char*>(m_pArea->m_header.m_areaName);
        }

        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            CVariable* pVariable = pArea->GetVariables()->FindKey(sName);
            if (pVariable != NULL) {
                pVariable->m_intValue = nValue;
            } else {
                var.SetName(sName);
                var.m_intValue = nValue;
                pArea->m_variables.AddKey(var);
            }

            message = new CMessageSetVariable(sName, sScope, nValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    return ACTION_DONE;
}

// 0x460300
SHORT CGameAIBase::IncrementGlobal()
{
    CString sString = m_curAction.GetString1();
    sString.MakeUpper();

    CString sScope = sString.Left(6);
    CString sName = sString.Right(sString.GetLength() - 6);

    CMessage* message;
    if (strcmp(sScope, "GLOBAL") == 0) {
        CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        if (pVariable != NULL) {
            pVariable->m_intValue += m_curAction.m_specificID;

            message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
        } else {
            CVariable var;
            var.SetName(sName);
            var.m_intValue += m_curAction.m_specificID;
            g_pBaldurChitin->GetObjectGame()->m_variables.AddKey(var);

            message = new CMessageSetVariable(sName, sScope, var.m_intValue, FALSE, m_id, m_id, 0);
        }
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    } else if (strcmp(sScope, "LOCALS") == 0) {
        if (GetObjectType() == TYPE_SPRITE) {
            CString sVariable(sName);

            CGameSprite* pSprite = static_cast<CGameSprite*>(this);
            CVariable var;
            CVariable* pVariable = pSprite->GetLocalVariables()->FindKey(sVariable);
            if (pVariable != NULL) {
                pVariable->m_intValue += m_curAction.m_specificID;
            } else {
                var.SetName(sVariable);
                var.m_intValue = m_curAction.m_specificID;
                pSprite->m_pLocalVariables->AddKey(var);
                pVariable = &var;
            }

            message = new CMessageSetVariable(sVariable, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    } else {
        if (strcmp(sScope, "MYAREA") == 0) {
            // NOTE: `m_areaName` is a `RESREF`, it's not guaranteed to be
            // NULL-terminated.
            sScope = reinterpret_cast<char*>(m_pArea->m_header.m_areaName);
        }

        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            CVariable* pVariable = pArea->GetVariables()->FindKey(sName);
            if (pVariable != NULL) {
                pVariable->m_intValue += m_curAction.m_specificID;

                message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            } else {
                CVariable var;
                var.SetName(sName);
                var.m_intValue = m_curAction.m_specificID;
                pArea->m_variables.AddKey(var);

                message = new CMessageSetVariable(sName, sScope, var.m_intValue, FALSE, m_id, m_id, 0);
            }
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    return ACTION_DONE;
}

// 0x462910
SHORT CGameAIBase::SetGlobalTimer()
{
    CString sString = m_curAction.GetString1();
    sString.MakeUpper();

    CString sScope = sString.Left(6);
    CString sName = sString.Right(sString.GetLength() - 6);

    // SetGlobalTimerRandom
    if (m_curAction.m_actionID == 307) {
        sName = sString.Right(sString.GetLength() - 7);

        if (m_curAction.m_specificID < m_curAction.m_specificID2) {
            m_curAction.m_specificID = rand() % (m_curAction.m_specificID2 - m_curAction.m_specificID + 1) + m_curAction.m_specificID;
        } else if (m_curAction.m_specificID > m_curAction.m_specificID2) {
            m_curAction.m_specificID = rand() % (m_curAction.m_specificID - m_curAction.m_specificID2 + 1) + m_curAction.m_specificID2;
        }
    }

    LONG nTime = m_curAction.m_specificID;

    CMessage* message;
    if (strcmp(sScope, "GLOBAL") == 0) {
        CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        if (pVariable == NULL) {
            CVariable var;
            var.SetName(sName);
            var.m_intValue = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime + nTime * CTimerWorld::TIMESCALE_MSEC_PER_SEC;
            g_pBaldurChitin->GetObjectGame()->m_variables.AddKey(var);

            message = new CMessageSetVariable(sName, sScope, var.m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else {
            // SetGlobalTimerOnce
            if (m_curAction.m_actionID != 308) {
                pVariable->m_intValue = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime + nTime * CTimerWorld::TIMESCALE_MSEC_PER_SEC;

                message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    } else if (strcmp(sScope, "LOCALS") == 0) {
        if (GetObjectType() == TYPE_SPRITE) {
            CString sVariable(sName);

            CGameSprite* pSprite = static_cast<CGameSprite*>(this);
            CVariable var;
            CVariable* pVariable = pSprite->GetLocalVariables()->FindKey(sVariable);
            if (pVariable == NULL) {
                var.SetName(sVariable);
                var.m_intValue = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime + nTime * CTimerWorld::TIMESCALE_MSEC_PER_SEC;
                pSprite->m_pLocalVariables->AddKey(var);
                pVariable = &var;
            } else {
                // SetGlobalTimerOnce
                // NOTE: Unlike the other scopes, the message is still sent
                // when the timer is already set.
                if (m_curAction.m_actionID != 308) {
                    pVariable->m_intValue = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime + nTime * CTimerWorld::TIMESCALE_MSEC_PER_SEC;
                }
            }

            message = new CMessageSetVariable(sVariable, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    } else {
        if (strcmp(sScope, "MYAREA") == 0) {
            // NOTE: `m_areaName` is a `RESREF`, it's not guaranteed to be
            // NULL-terminated.
            sScope = reinterpret_cast<char*>(m_pArea->m_header.m_areaName);
        }

        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            CVariable* pVariable = pArea->GetVariables()->FindKey(sName);
            if (pVariable == NULL) {
                CVariable var;
                var.SetName(sName);
                var.m_intValue = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime + nTime * CTimerWorld::TIMESCALE_MSEC_PER_SEC;
                pArea->m_variables.AddKey(var);

                message = new CMessageSetVariable(sName, sScope, var.m_intValue, FALSE, m_id, m_id, 0);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            } else {
                // SetGlobalTimerOnce
                if (m_curAction.m_actionID != 308) {
                    pVariable->m_intValue = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime + nTime * CTimerWorld::TIMESCALE_MSEC_PER_SEC;

                    message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            }
        }
    }

    return ACTION_DONE;
}

// 0x4688A0
SHORT CGameAIBase::BitGlobal()
{
    CString sString = m_curAction.GetString1();
    sString.MakeUpper();

    CString sScope;
    CString sName;

    int nIndex = sString.Find(":");
    sScope = sString.Left(nIndex);
    sName = sString.Right(sString.GetLength() - nIndex - 1);

    CMessage* message;
    if (strcmp(sScope, "GLOBAL") == 0) {
        CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        if (pVariable != NULL) {
            switch (m_curAction.m_specificID2) {
            case 1:
                pVariable->m_intValue = m_curAction.m_specificID & pVariable->m_intValue;
                break;
            case 2:
                pVariable->m_intValue = m_curAction.m_specificID | pVariable->m_intValue;
                break;
            case 3:
                pVariable->m_intValue = m_curAction.m_specificID ^ pVariable->m_intValue;
                break;
            default:
                return ACTION_ERROR;
            }

            message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else {
            pVariable = new CVariable();
            pVariable->SetName(sName);

            switch (m_curAction.m_specificID2) {
            case 1:
                pVariable->m_intValue = m_curAction.m_specificID & pVariable->m_intValue;
                break;
            case 2:
                pVariable->m_intValue = pVariable->m_intValue | m_curAction.m_specificID;
                break;
            case 3:
                pVariable->m_intValue = m_curAction.m_specificID ^ pVariable->m_intValue;
                break;
            default:
                // FIXME: Leaks `pVariable`.
                return ACTION_ERROR;
            }

            g_pBaldurChitin->GetObjectGame()->m_variables.AddKey(*pVariable);

            message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            delete pVariable;
        }
    } else if (strcmp(sScope, "LOCALS") == 0) {
        if (GetObjectType() == TYPE_SPRITE) {
            CString sVariable(sName);

            CGameSprite* pSprite = static_cast<CGameSprite*>(this);
            CVariable* pVariable = pSprite->GetLocalVariables()->FindKey(sVariable);
            CVariable var;
            if (pVariable != NULL) {
                switch (m_curAction.m_specificID2) {
                case 1:
                    pVariable->m_intValue = m_curAction.m_specificID & pVariable->m_intValue;
                    break;
                case 2:
                    pVariable->m_intValue = m_curAction.m_specificID | pVariable->m_intValue;
                    break;
                case 3:
                    pVariable->m_intValue = m_curAction.m_specificID ^ pVariable->m_intValue;
                    break;
                default:
                    return ACTION_ERROR;
                }
            } else {
                var.SetName(sVariable);

                switch (m_curAction.m_specificID2) {
                case 1:
                    var.m_intValue &= m_curAction.m_specificID;
                    break;
                case 2:
                    var.m_intValue |= m_curAction.m_specificID;
                    break;
                case 3:
                    var.m_intValue ^= m_curAction.m_specificID;
                    break;
                default:
                    return ACTION_ERROR;
                }

                pSprite->m_pLocalVariables->AddKey(var);
                pVariable = &var;
            }

            message = new CMessageSetVariable(sVariable, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    } else {
        if (strcmp(sScope, "MYAREA") == 0) {
            // NOTE: `m_areaName` is a `RESREF`, it's not guaranteed to be
            // NULL-terminated.
            sScope = reinterpret_cast<char*>(m_pArea->m_header.m_areaName);
        }

        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            CVariable* pVariable = pArea->GetVariables()->FindKey(sName);
            if (pVariable != NULL) {
                switch (m_curAction.m_specificID2) {
                case 1:
                    pVariable->m_intValue = pVariable->m_intValue & m_curAction.m_specificID;
                    break;
                case 2:
                    pVariable->m_intValue = m_curAction.m_specificID | pVariable->m_intValue;
                    break;
                case 3:
                    pVariable->m_intValue = m_curAction.m_specificID ^ pVariable->m_intValue;
                    break;
                default:
                    return ACTION_ERROR;
                }

                message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            } else {
                pVariable = new CVariable();
                pVariable->SetName(sName);

                switch (m_curAction.m_specificID2) {
                case 1:
                    pVariable->m_intValue = m_curAction.m_specificID & pVariable->m_intValue;
                    break;
                case 2:
                    pVariable->m_intValue = pVariable->m_intValue | m_curAction.m_specificID;
                    break;
                case 3:
                    pVariable->m_intValue = m_curAction.m_specificID ^ pVariable->m_intValue;
                    break;
                default:
                    // FIXME: Leaks `pVariable`.
                    return ACTION_ERROR;
                }

                pArea->m_variables.AddKey(*pVariable);

                message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                delete pVariable;
            }
        }
    }

    return ACTION_DONE;
}

// 0x469040
SHORT CGameAIBase::GlobalBitGlobal()
{
    LONG nValue = 0;

    CString sString = m_curAction.GetString1();
    sString.MakeUpper();

    CString sScope;
    CString sName;

    int nIndex = sString.Find(":");
    sScope = sString.Left(nIndex);
    sName = sString.Right(sString.GetLength() - nIndex - 1);

    CString sSourceScope;
    CString sSourceName;

    sString = m_curAction.GetString2();
    sString.MakeUpper();

    nIndex = sString.Find(":");
    sSourceScope = sString.Left(nIndex);
    sSourceName = sString.Right(sString.GetLength() - nIndex - 1);

    // Read source variable.
    if (strcmp(sSourceScope, "GLOBAL") == 0) {
        CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sSourceName);
        if (pVariable != NULL) {
            nValue = pVariable->m_intValue;
        }
    } else if (strcmp(sSourceScope, "LOCALS") == 0) {
        CString sVariable(sSourceName);

        // NOTE: No object type check here (unlike everywhere else).
        CVariable* pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sVariable);
        if (pVariable != NULL) {
            nValue = pVariable->m_intValue;
        }
    } else {
        // FIXME: Checks destination scope instead of source scope.
        if (strcmp(sScope, "MYAREA") == 0) {
            // NOTE: `m_areaName` is a `RESREF`, it's not guaranteed to be
            // NULL-terminated.
            sSourceScope = reinterpret_cast<char*>(m_pArea->m_header.m_areaName);
        }

        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sSourceScope);
        if (pArea != NULL) {
            CVariable* pVariable = pArea->GetVariables()->FindKey(sSourceName);
            if (pVariable != NULL) {
                nValue = pVariable->m_intValue;
            }
        }
    }

    // Update destination variable.
    CMessage* message;
    if (strcmp(sScope, "GLOBAL") == 0) {
        CVariable* pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        if (pVariable != NULL) {
            switch (m_curAction.m_specificID) {
            case 1:
                pVariable->m_intValue = pVariable->m_intValue & nValue;
                break;
            case 2:
                pVariable->m_intValue = pVariable->m_intValue | nValue;
                break;
            case 3:
                pVariable->m_intValue = pVariable->m_intValue ^ nValue;
                break;
            default:
                return ACTION_ERROR;
            }

            message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else {
            pVariable = new CVariable();
            pVariable->SetName(sName);

            switch (m_curAction.m_specificID) {
            case 1:
                pVariable->m_intValue = pVariable->m_intValue & nValue;
                break;
            case 2:
                pVariable->m_intValue = pVariable->m_intValue | nValue;
                break;
            case 3:
                pVariable->m_intValue = pVariable->m_intValue ^ nValue;
                break;
            default:
                // FIXME: Leaks `pVariable`.
                return ACTION_ERROR;
            }

            g_pBaldurChitin->GetObjectGame()->m_variables.AddKey(*pVariable);

            message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            delete pVariable;
        }
    } else if (strcmp(sScope, "LOCALS") == 0) {
        if (GetObjectType() == TYPE_SPRITE) {
            CString sVariable(sName);

            CGameSprite* pSprite = static_cast<CGameSprite*>(this);
            CVariable* pVariable = pSprite->GetLocalVariables()->FindKey(sVariable);
            CVariable var;
            if (pVariable != NULL) {
                switch (m_curAction.m_specificID) {
                case 1:
                    pVariable->m_intValue = pVariable->m_intValue & nValue;
                    break;
                case 2:
                    pVariable->m_intValue = pVariable->m_intValue | nValue;
                    break;
                case 3:
                    pVariable->m_intValue = pVariable->m_intValue ^ nValue;
                    break;
                default:
                    return ACTION_ERROR;
                }
            } else {
                var.SetName(sVariable);

                switch (m_curAction.m_specificID) {
                case 1:
                    var.m_intValue &= nValue;
                    break;
                case 2:
                    var.m_intValue |= nValue;
                    break;
                case 3:
                    var.m_intValue ^= nValue;
                    break;
                default:
                    return ACTION_ERROR;
                }

                pSprite->m_pLocalVariables->AddKey(var);
                pVariable = &var;
            }

            message = new CMessageSetVariable(sVariable, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    } else {
        if (strcmp(sScope, "MYAREA") == 0) {
            // NOTE: `m_areaName` is a `RESREF`, it's not guaranteed to be
            // NULL-terminated.
            sScope = reinterpret_cast<char*>(m_pArea->m_header.m_areaName);
        }

        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            CVariable* pVariable = pArea->GetVariables()->FindKey(sName);
            if (pVariable != NULL) {
                switch (m_curAction.m_specificID) {
                case 1:
                    pVariable->m_intValue = pVariable->m_intValue & nValue;
                    break;
                case 2:
                    pVariable->m_intValue = pVariable->m_intValue | nValue;
                    break;
                case 3:
                    pVariable->m_intValue = pVariable->m_intValue ^ nValue;
                    break;
                default:
                    return ACTION_ERROR;
                }

                message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            } else {
                pVariable = new CVariable();
                pVariable->SetName(sName);

                switch (m_curAction.m_specificID) {
                case 1:
                    pVariable->m_intValue = pVariable->m_intValue & nValue;
                    break;
                case 2:
                    pVariable->m_intValue = pVariable->m_intValue | nValue;
                    break;
                case 3:
                    pVariable->m_intValue = pVariable->m_intValue ^ nValue;
                    break;
                default:
                    // FIXME: Leaks `pVariable`.
                    return ACTION_ERROR;
                }

                pArea->m_variables.AddKey(*pVariable);

                message = new CMessageSetVariable(sName, sScope, pVariable->m_intValue, FALSE, m_id, m_id, 0);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                delete pVariable;
            }
        }
    }

    return ACTION_DONE;
}

// 0x4608A0
SHORT CGameAIBase::CreateCreature()
{
    CVariable creatureName;

    CPoint pt = m_curAction.m_dest;
    if (pt.x == -1 && pt.y == -1) {
        CProjectile::sub_52A640(m_id, pt, TRUE);
    }

    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(m_curAction.GetString1()), 1009, TRUE) == NULL) {
        return ACTION_ERROR;
    }

    CCreatureFile creatureData;

    // NOTE: Uninline.
    creatureData.SetResRef(CResRef(m_curAction.GetString1()), TRUE, TRUE);

    CGameSprite* pSprite = new CGameSprite(creatureData.GetData(),
        creatureData.GetDataSize(),
        0,
        0,
        -1,
        0,
        0,
        0x7FFFFFFF,
        pt,
        static_cast<WORD>(m_curAction.m_specificID));
    if (pSprite == NULL) {
        creatureData.ReleaseData();
        return ACTION_ERROR;
    }

    // NOTE: The original `SetResRef` (0x42FDC0) takes a `CString` by value,
    // here it's converted through `CResRef`.
    pSprite->SetResRef(CResRef(creatureData.GetResRef().GetResRefStr()));
    creatureData.ReleaseData();
    pSprite->SetResRef(CResRef(m_curAction.GetString1()));

    if (m_curAction.m_actionID == 228) {
        // CreateCreatureImpassable (not in IWD2's ACTION.IDS).
        pt.y /= CPathSearch::GRID_SQUARE_SIZEY;
        pt.x /= CPathSearch::GRID_SQUARE_SIZEX;
    } else {
        BYTE personalSpace = pSprite->m_animation.GetPersonalSpace();
        CPoint ptGrid(pt.x / CPathSearch::GRID_SQUARE_SIZEX,
            pt.y / CPathSearch::GRID_SQUARE_SIZEY);
        pt = m_pArea->m_search.GetNearestOpenSquare(ptGrid,
            GetTerrainTable(),
            personalSpace,
            -1);
    }

    BYTE listType = pSprite->m_animation.GetListType();
    pSprite->AddToArea(m_pArea,
        CPoint(pt.x * CPathSearch::GRID_SQUARE_SIZEX, pt.y * CPathSearch::GRID_SQUARE_SIZEY),
        0,
        listType);

    if (m_curAction.GetString2() != "") {
        memcpy(pSprite->m_scriptName,
            m_curAction.GetString2().GetBuffer(SCRIPTNAME_SIZE),
            SCRIPTNAME_SIZE);
    }

    creatureName.SetName(CString(pSprite->GetScriptName()));
    creatureName.m_intValue = pSprite->m_id;
    m_pArea->m_namedCreatures.AddKey(creatureName);

    field_342.Set(pSprite->GetAIType());

    return ACTION_DONE;
}

// 0x460D70
SHORT CGameAIBase::CreateCreatureObject(CGameObject* pObject)
{
    if (pObject == NULL) {
        return ACTION_ERROR;
    }

    CVariable creatureName;

    CPoint pt = pObject->GetPos();
    if (pt.x == -1 && pt.y == -1) {
        CProjectile::sub_52A640(m_id, pt, TRUE);
    }

    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(m_curAction.GetString1()), 1009, TRUE) == NULL) {
        return ACTION_ERROR;
    }

    CCreatureFile creatureData;

    // NOTE: Uninline.
    creatureData.SetResRef(CResRef(m_curAction.GetString1()), TRUE, TRUE);

    CGameSprite* pSprite = new CGameSprite(creatureData.GetData(),
        creatureData.GetDataSize(),
        0,
        0,
        -1,
        0,
        0,
        0x7FFFFFFF,
        pt,
        static_cast<WORD>(m_curAction.m_specificID));
    if (pSprite == NULL) {
        creatureData.ReleaseData();
        return ACTION_ERROR;
    }

    // NOTE: See `CreateCreature`.
    pSprite->SetResRef(CResRef(creatureData.GetResRef().GetResRefStr()));
    creatureData.ReleaseData();
    pSprite->SetResRef(CResRef(m_curAction.GetString1()));

    BYTE personalSpace = pSprite->m_animation.GetPersonalSpace();
    CPoint ptGrid(pt.x / CPathSearch::GRID_SQUARE_SIZEX,
        pt.y / CPathSearch::GRID_SQUARE_SIZEY);
    pt = m_pArea->m_search.GetNearestOpenSquare(ptGrid,
        GetTerrainTable(),
        personalSpace,
        -1);

    BYTE listType = pSprite->m_animation.GetListType();
    pSprite->AddToArea(m_pArea,
        CPoint(pt.x * CPathSearch::GRID_SQUARE_SIZEX, pt.y * CPathSearch::GRID_SQUARE_SIZEY),
        0,
        listType);

    creatureName.SetName(CString(pSprite->GetScriptName()));
    creatureName.m_intValue = pSprite->m_id;
    m_pArea->m_namedCreatures.AddKey(creatureName);

    return ACTION_DONE;
}

// 0x461190
SHORT CGameAIBase::ForceSpell(CGameAIBase* pTarget)
{
    CString sSpell;
    SHORT casterLevel;

    if (m_typeAI.m_nGeneral == CAIObjectType::G_DEAD) {
        return ACTION_DONE;
    }

    if (pTarget == NULL) {
        return ACTION_ERROR;
    }

    if (m_curAction.GetString1() == "") {
        DecodeSpell(m_curAction.m_specificID, sSpell);
        if (sSpell == "") {
            return ACTION_ERROR;
        }

        if (GetObjectType() == TYPE_SPRITE) {
            casterLevel = static_cast<CGameSprite*>(this)->m_derivedStats.m_nLevel;
        } else {
            casterLevel = 1;
        }
    } else {
        sSpell = m_curAction.GetString1();
        casterLevel = static_cast<SHORT>(m_curAction.m_specificID);
    }

    if (casterLevel < 2) {
        casterLevel = 1;
    }

    CSpell* pSpell = new CSpell(CResRef(sSpell));
    if (pSpell == NULL) {
        return ACTION_ERROR;
    }

    pSpell->Demand();

    if (m_curAction.m_actionID != CAIAction::REALLYFORCESPELL
        && !CheckAppropriateTarget(pSpell, pTarget)) {
        pSpell->Release();
        delete pSpell;
        return ACTION_ERROR;
    }

    SHORT nAbility = 0;
    if (pSpell->GetAbilityCount() > 0) {
        do {
            if (pSpell->GetAbility(nAbility)->minCasterLevel > casterLevel) {
                break;
            }

            // NOTE: Result is unused.
            pSpell->GetAbility(nAbility);

            nAbility++;
        } while (nAbility < pSpell->GetAbilityCount());
    }

    LONG abilityNum = static_cast<SHORT>(nAbility - 1);
    SPELL_ABILITY* pAbility = pSpell->GetAbility(abilityNum);
    if (pAbility == NULL) {
        pSpell->Release();
        delete pSpell;
        return ACTION_ERROR;
    }

    CProjectile* pProjectile = CProjectile::DecodeProjectile(pAbility->missileType, this, FALSE);
    if (GetObjectType() == TYPE_SPRITE) {
        BYTE nClass = 0;
        int nType = 0;
        BYTE nLevel = 0;
        if (static_cast<CGameSprite*>(this)->sub_727720(pSpell->cResRef, nClass, nType, nLevel) == 1) {
            pProjectile->field_186 = nLevel;
        }
        pProjectile->field_18A = pSpell->cResRef;
    }

    if (GetObjectType() == TYPE_SPRITE
        && m_curAction.m_actionID != CAIAction::REALLYFORCESPELL) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(this);
        SHORT nSequence = pSprite->m_nSequence;

        if (m_actionCount == 0) {
            pSprite->sub_755A70(pSpell, pAbility, pTarget->GetPos());

            if (!pSprite->IsVisibleToCaller(FALSE) && pTarget->m_id != m_id) {
                ITEM_EFFECT effect;
                CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);

                // NOTE: Original reads it from a constant at 0x8491AA.
                effect.durationType = 1;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    m_pos,
                    m_id,
                    CPoint(-1, -1));

                CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            if (pSprite->m_derivedStats.m_spellStates.test(SPLSTATE_SANCTUARY)) {
                ITEM_EFFECT effect;
                CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_DISPELSANCTUARY);

                // NOTE: Original reads it from a constant at 0x8491AA.
                effect.durationType = 1;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    m_pos,
                    m_id,
                    CPoint(-1, -1));

                CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        INT nCastingTime = pAbility->speedFactor * 100 / 10;

        if (m_actionCount < nCastingTime - 4) {
            if (nSequence != CGameSprite::SEQ_CONJURE) {
                CMessage* message = new CMessageSetSequence(CGameSprite::SEQ_CONJURE, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            pSpell->Release();
            delete pSpell;

            if (pProjectile != NULL) {
                delete pProjectile;
            }

            return ACTION_NORMAL;
        }

        if (m_actionCount < nCastingTime) {
            if (nSequence != CGameSprite::SEQ_CAST) {
                CMessage* message = new CMessageSetSequence(CGameSprite::SEQ_CAST, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                pSprite->ApplyCastingEffectPost(pSpell, pAbility);
            }

            pSpell->Release();
            delete pSpell;

            if (pProjectile != NULL) {
                delete pProjectile;
            }

            return ACTION_NORMAL;
        }

        if (nSequence != CGameSprite::SEQ_CAST) {
            CMessage* message = new CMessageSetSequence(CGameSprite::SEQ_CAST, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            pSprite->ApplyCastingEffectPost(pSpell, pAbility);
        }
    }

    for (LONG nEffect = 0; nEffect < pAbility->effectCount; nEffect++) {
        CGameEffect* pEffect = pSpell->DecodeEffect(abilityNum,
            nEffect,
            this,
            0,
            0,
            static_cast<BYTE>(casterLevel));
        pEffect->m_source = m_pos;
        pEffect->m_sourceID = m_id;
        pEffect->m_target = pTarget->GetPos();
        pEffect->m_casterLevel = casterLevel;

        switch (pEffect->m_targetType) {
        case 1:
            pEffect->m_flags |= 0x2;

            {
                CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
            break;
        case 2:
            pProjectile->AddEffect(pEffect);
            break;
        case 3:
            ApplyEffectToParty(pEffect);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 4:
            m_pArea->sub_46E130(pEffect, FALSE, FALSE, 0, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 5:
            m_pArea->sub_46E130(pEffect, TRUE, FALSE, 0, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 6:
            m_pArea->sub_46E130(pEffect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 7:
            m_pArea->sub_46E130(pEffect, FALSE, TRUE, pTarget->GetAIType().m_nSpecific, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 8:
            m_pArea->sub_46E130(pEffect, FALSE, FALSE, 0, this);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBaseActions.cpp
            // __LINE__: 1594
            UTIL_ASSERT(FALSE);
        }
    }

    if (GetObjectType() == TYPE_SPRITE) {
        static_cast<CGameSprite*>(this)->FeedBack(CGameSprite::FEEDBACK_SPELL,
            0,
            0,
            0,
            pSpell->GetGenericName(),
            0,
            0);
    }

    // NOTE: `DetermineHeight` is called with `this` even when it's not a
    // sprite.
    CMessageFireProjectile* pMessage = new CMessageFireProjectile(pProjectile->m_projectileType,
        pTarget->m_id,
        pTarget->m_pos,
        pProjectile->DetermineHeight(static_cast<CGameSprite*>(this)),
        m_id,
        m_id,
        0);

    if (pProjectile->m_projectileType == 304) {
        int nRandom = rand() % 1000000;
        // FIXME: Member of the specific `CProjectile` subclass for projectile
        // type 304 (offset 0x356), not of `CProjectile`.
        *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pProjectile) + 0x356) = nRandom;
        pMessage->field_20 = nRandom;
    }

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    pProjectile->Fire(m_pArea,
        m_id,
        pTarget->m_id,
        pTarget->m_pos,
        30,
        0);

    pSpell->Release();
    delete pSpell;

    return ACTION_DONE;
}

// 0x461B80
SHORT CGameAIBase::ForceSpellPoint()
{
    CPoint ptTarget = m_curAction.m_dest;
    CString sSpell;
    SHORT casterLevel;

    if (m_typeAI.m_nGeneral == CAIObjectType::G_DEAD) {
        return ACTION_DONE;
    }

    if (m_curAction.GetString1() == "") {
        DecodeSpell(m_curAction.m_specificID, sSpell);
        if (sSpell == "") {
            return ACTION_ERROR;
        }

        if (GetObjectType() == TYPE_SPRITE) {
            casterLevel = static_cast<CGameSprite*>(this)->m_derivedStats.m_nLevel;
        } else {
            casterLevel = 1;
        }
    } else {
        sSpell = m_curAction.GetString1();
        casterLevel = static_cast<SHORT>(m_curAction.m_specificID);
    }

    if (casterLevel < 2) {
        casterLevel = 1;
    }

    CSpell* pSpell = new CSpell(CResRef(sSpell));
    if (pSpell == NULL) {
        return ACTION_ERROR;
    }

    pSpell->Demand();

    SHORT nAbility = 0;
    if (pSpell->GetAbilityCount() > 0) {
        do {
            if (pSpell->GetAbility(nAbility)->minCasterLevel > casterLevel) {
                break;
            }

            nAbility++;
        } while (nAbility < pSpell->GetAbilityCount());
    }

    LONG abilityNum = static_cast<SHORT>(nAbility - 1);
    SPELL_ABILITY* pAbility = pSpell->GetAbility(abilityNum);
    if (pAbility == NULL) {
        pSpell->Release();
        delete pSpell;
        return ACTION_ERROR;
    }

    if (GetObjectType() == TYPE_SPRITE) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(this);
        SHORT nSequence = pSprite->m_nSequence;

        if (m_actionCount == 0) {
            pSprite->sub_755A70(pSpell, pAbility, ptTarget);

            if (!pSprite->IsVisibleToCaller(FALSE)) {
                ITEM_EFFECT effect;
                CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);

                // NOTE: Original reads it from a constant at 0x8491AA.
                effect.durationType = 1;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    m_pos,
                    m_id,
                    CPoint(-1, -1));

                CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            if (pSprite->m_derivedStats.m_spellStates.test(SPLSTATE_SANCTUARY)) {
                ITEM_EFFECT effect;
                CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_DISPELSANCTUARY);

                // NOTE: Original reads it from a constant at 0x8491AA.
                effect.durationType = 1;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    m_pos,
                    m_id,
                    CPoint(-1, -1));

                CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        INT nCastingTime = pAbility->speedFactor * 100 / 10;

        if (m_actionCount < nCastingTime - 4) {
            if (nSequence != CGameSprite::SEQ_CONJURE) {
                CMessage* message = new CMessageSetSequence(CGameSprite::SEQ_CONJURE, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            pSpell->Release();
            delete pSpell;

            return ACTION_NORMAL;
        }

        if (m_actionCount < nCastingTime) {
            if (nSequence != CGameSprite::SEQ_CAST) {
                CMessage* message = new CMessageSetSequence(CGameSprite::SEQ_CAST, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                pSprite->ApplyCastingEffectPost(pSpell, pAbility);
            }

            pSpell->Release();
            delete pSpell;

            return ACTION_NORMAL;
        }

        // NOTE: Unlike `ForceSpell` there is no `ApplyCastingEffectPost` here.
        if (nSequence != CGameSprite::SEQ_CAST) {
            CMessage* message = new CMessageSetSequence(CGameSprite::SEQ_CAST, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    CProjectile* pProjectile = CProjectile::DecodeProjectile(pAbility->missileType, this, FALSE);
    if (GetObjectType() == TYPE_SPRITE) {
        BYTE nClass = 0;
        int nType = 0;
        BYTE nLevel = 0;
        if (static_cast<CGameSprite*>(this)->sub_727720(pSpell->cResRef, nClass, nType, nLevel) == 1) {
            pProjectile->field_186 = nLevel;
        }
        pProjectile->field_18A = pSpell->cResRef;
    }

    for (LONG nEffect = 0; nEffect < pAbility->effectCount; nEffect++) {
        CGameEffect* pEffect = pSpell->DecodeEffect(abilityNum,
            nEffect,
            this,
            0,
            0,
            static_cast<BYTE>(casterLevel));
        pEffect->m_source = m_pos;
        pEffect->m_sourceID = m_id;
        pEffect->m_target = ptTarget;
        pEffect->m_casterLevel = casterLevel;

        switch (pEffect->m_targetType) {
        case 1:
            pEffect->m_flags |= 0x2;

            {
                CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
            break;
        case 2:
            pProjectile->AddEffect(pEffect);
            break;
        case 3:
            ApplyEffectToParty(pEffect);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 4:
            m_pArea->sub_46E130(pEffect, FALSE, FALSE, 0, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 5:
            m_pArea->sub_46E130(pEffect, TRUE, FALSE, 0, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 6:
            m_pArea->sub_46E130(pEffect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 7:
            // NOTE: No target object, the effect is dropped.
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 8:
            m_pArea->sub_46E130(pEffect, FALSE, FALSE, 0, this);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBaseActions.cpp
            // __LINE__: 1845
            UTIL_ASSERT(FALSE);
        }
    }

    if (GetObjectType() == TYPE_SPRITE) {
        static_cast<CGameSprite*>(this)->FeedBack(CGameSprite::FEEDBACK_SPELL,
            0,
            0,
            0,
            pSpell->GetGenericName(),
            0,
            0);
    }

    // NOTE: `DetermineHeight` is called with `this` even when it's not a
    // sprite.
    CMessageFireProjectile* pMessage = new CMessageFireProjectile(pProjectile->m_projectileType,
        CGameObjectArray::INVALID_INDEX,
        ptTarget,
        pProjectile->DetermineHeight(static_cast<CGameSprite*>(this)),
        m_id,
        m_id,
        0);

    if (pProjectile->m_projectileType == 304) {
        int nRandom = rand() % 1000000;
        // FIXME: Member of the specific `CProjectile` subclass for projectile
        // type 304 (offset 0x356), not of `CProjectile`.
        *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pProjectile) + 0x356) = nRandom;
        pMessage->field_20 = nRandom;
    }

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    pProjectile->Fire(m_pArea,
        m_id,
        CGameObjectArray::INVALID_INDEX,
        ptTarget,
        30,
        0);

    pSpell->Release();
    delete pSpell;

    return ACTION_DONE;
}

// 0x462470
SHORT CGameAIBase::ApplySpell(CGameAIBase* pTarget)
{
    CString sSpell;

    if (m_curAction.GetString1() == "") {
        DecodeSpell(m_curAction.m_specificID, sSpell);
        if (sSpell == "") {
            return ACTION_ERROR;
        }
    } else {
        sSpell = m_curAction.GetString1();
    }

    CSpell* pSpell = new CSpell(CResRef(sSpell));
    if (pSpell == NULL) {
        return ACTION_ERROR;
    }

    if (pTarget == NULL) {
        delete pSpell;
        return ACTION_ERROR;
    }

    pSpell->Demand();

    if (!CheckAppropriateTarget(pSpell, pTarget)) {
        pSpell->Release();
        delete pSpell;
        return ACTION_ERROR;
    }

    SPELL_ABILITY* pAbility = pSpell->GetAbility(0);
    if (pAbility == NULL) {
        pSpell->Release();
        delete pSpell;
        return ACTION_ERROR;
    }

    BYTE casterLevel = 0;
    if ((m_curAction.m_internalFlags & 0x20000000) != 0) {
        casterLevel = static_cast<BYTE>(m_curAction.m_specificID3);
    }

    for (LONG nEffect = 0; nEffect < pAbility->effectCount; nEffect++) {
        CGameEffect* pEffect = pSpell->DecodeEffect(0,
            nEffect,
            this,
            0,
            0,
            casterLevel);
        pEffect->m_source = m_pos;
        pEffect->m_sourceID = m_id;
        pEffect->m_target = pTarget->GetPos();

        switch (pEffect->m_targetType) {
        case 1:
            pEffect->m_flags |= 0x2;

            {
                CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
            break;
        case 2:
            {
                CMessage* message = new CMessageAddEffect(pEffect, m_id, pTarget->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
            break;
        case 3:
            ApplyEffectToParty(pEffect);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 4:
            m_pArea->sub_46E130(pEffect, FALSE, FALSE, 0, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 5:
            m_pArea->sub_46E130(pEffect, TRUE, FALSE, 0, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 6:
            m_pArea->sub_46E130(pEffect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 7:
            m_pArea->sub_46E130(pEffect, FALSE, TRUE, pTarget->GetAIType().m_nSpecific, NULL);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        case 8:
            m_pArea->sub_46E130(pEffect, FALSE, FALSE, 0, this);
            if (pEffect != NULL) {
                delete pEffect;
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBaseActions.cpp
            // __LINE__: 2001
            UTIL_ASSERT(FALSE);
        }
    }

    pSpell->Release();
    delete pSpell;

    return ACTION_DONE;
}

// 0x462F90
SHORT CGameAIBase::StartCutScene()
{
    if (m_curAction.GetString1() == "") {
        return ACTION_ERROR;
    }

    CAIScript* pScript = new CAIScript(CResRef(m_curAction.GetString1()));

    POSITION pos = pScript->m_caList.GetHeadPosition();
    while (pos != NULL) {
        CAIConditionResponse* pConditionResponse = pScript->m_caList.GetNext(pos);
        if (pConditionResponse->m_responseSet.m_responseList.GetCount() != 0) {
            CAIResponse* pResponse = pConditionResponse->m_responseSet.m_responseList.GetHead();
            if (pResponse->m_actionList.GetCount() != 0) {
                CAIAction* pAction = pResponse->m_actionList.RemoveHead();
                pAction->Decode(this);

                CGameObject* pObject = pAction->m_acteeID.sub_40CB20(this, CGameObject::TYPE_AIBASE, FALSE);
                if (pObject != NULL) {
                    CMessageInsertResponse* pInsertResponse = new CMessageInsertResponse(*pResponse,
                        FALSE,
                        FALSE,
                        m_id,
                        pObject->GetId(),
                        FALSE);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pInsertResponse, FALSE);

                    if ((pObject->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
                        CMessageSetInCutScene* pSetInCutScene = new CMessageSetInCutScene(TRUE,
                            m_id,
                            pObject->GetId());
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(pSetInCutScene, FALSE);
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }

                if (pAction != NULL) {
                    delete pAction;
                }
            }
        }
    }

    if (pScript != NULL) {
        delete pScript;
    }

    return ACTION_DONE;
}

// 0x4637B0
SHORT CGameAIBase::ClearAllActions()
{
    // NOTE: Walks the area's vertical sort list backwards.
    POSITION pos = m_pArea->m_lVertSort.GetTailPosition();
    while (pos != NULL) {
        LONG id = reinterpret_cast<LONG>(m_pArea->m_lVertSort.GetPrev(pos));
        if (id != m_id) {
            CGameAIBase* pObject;
            if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(id,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pObject),
                    INFINITE)
                == CGameObjectArray::SUCCESS) {
                // NOTE: Original is a `switch` on the object type.
                BYTE objectType = pObject->GetObjectType();
                if (objectType == CGameObject::TYPE_AIBASE || objectType == CGameObject::TYPE_SPRITE) {
                    if (pObject != NULL) {
                        CMessageClearActions* pClearActions = new CMessageClearActions(m_id,
                            pObject->GetId());
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(pClearActions, FALSE);
                    }

                    CMessageAddAction* pAddAction = new CMessageAddAction(CAIAction(CAIAction::NO_ACTION, CPoint(-1, -1), 0, -1),
                        m_id,
                        pObject->GetId());
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pAddAction, FALSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(id,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }

    return ACTION_DONE;
}

// 0x4658A0
SHORT CGameAIBase::IncrementChapter()
{
    CScreenChapter* pChapter = g_pBaldurChitin->m_pEngineChapter;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBaseActions.cpp
    // __LINE__: 3423
    UTIL_ASSERT(pChapter != NULL);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBaseActions.cpp
    // __LINE__: 3425
    UTIL_ASSERT(pGame != NULL);

    CResRef cResText("Chapters");

    BYTE nChapter;
    if (m_curAction.m_specificID == -1) {
        CVariable* pVar = pGame->GetVariables()->FindKey(CInfGame::CHAPTER_GLOBAL);
        nChapter = static_cast<BYTE>((pVar != NULL ? pVar->m_intValue : 0) + 1);
    } else {
        nChapter = static_cast<BYTE>(m_curAction.m_specificID);
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == FALSE) {
        g_pBaldurChitin->m_cBaldurMessage.SendChapterAnnounceToServer(nChapter, cResText);
        return ACTION_DONE;
    }

    CVariable* pVar = pGame->GetVariables()->FindKey(CInfGame::CHAPTER_GLOBAL);
    if (pVar != NULL) {
        pVar->m_intValue = nChapter;
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        CString sResText;
        cResText.CopyToString(sResText);

        CVariable* pChapterVar = pGame->GetVariables()->FindKey(CInfGame::CHAPTER_GLOBAL);
        LONG nCurrentChapter = pChapterVar != NULL ? pChapterVar->m_intValue : 0;

        BYTE resRef[RESREF_SIZE] = { 0 };
        int nLength = sResText.GetLength();
        memcpy(resRef, sResText.GetBuffer(nLength), nLength);

        g_pBaldurChitin->m_pEngineChapter->StartChapterMultiplayerHost(static_cast<BYTE>(nCurrentChapter), resRef);

        C2DArray tChapters;
        CResRef cResChapters;
        cResChapters = "chapters";
        tChapters.Load(cResChapters);

        CPoint pt;
        pt.x = 1;
        pt.y = nChapter + 1;
        g_pBaldurChitin->GetObjectGame()->m_cJournal.AddEntry(tChapters.GetAtLong(pt), 0);

        return ACTION_DONE;
    }

    pChapter->StartChapter(cResText);
    g_pBaldurChitin->pActiveEngine->SelectEngine(pChapter);

    C2DArray tChapters;
    CResRef cResChapters;
    cResChapters = "chapters";
    tChapters.Load(cResChapters);

    CPoint pt;
    pt.x = 1;
    pt.y = nChapter + 1;
    g_pBaldurChitin->GetObjectGame()->m_cJournal.AddEntry(tChapters.GetAtLong(pt), 0);

    return ACTION_DONE;
}

// 0x465D50
SHORT CGameAIBase::AddExperiencePartyGlobal()
{
    CString sName = m_curAction.GetString1();
    sName.MakeUpper();

    CString sScope = m_curAction.GetString2();
    sScope.MakeUpper();

    // FIXME: In the original `nValue` is left uninitialized (a stale stack
    // slot) when the named area is not loaded.
    LONG nValue = 0;

    if (strcmp(sScope, "GLOBAL") == 0) {
        CVariable* pVar = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        nValue = pVar != NULL ? pVar->m_intValue : 0;
    } else if (strcmp(sScope, "LOCALS") == 0) {
        if (GetObjectType() == CGameObject::TYPE_SPRITE) {
            CString sVarName(sName);
            CVariable* pVar = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sVarName);
            nValue = pVar != NULL ? pVar->m_intValue : 0;
        } else {
            nValue = 0;
        }
    } else {
        if (strcmp(sScope, "MYAREA") == 0) {
            sScope = (LPCSTR)m_pArea->m_header.m_areaName;
        }

        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            CVariable* pVar = pArea->GetVariables()->FindKey(sName);
            nValue = pVar != NULL ? pVar->m_intValue : 0;
        }
    }

    g_pBaldurChitin->GetObjectGame()->AddPartyXP(nValue, 0, 0);

    return ACTION_DONE;
}

// 0x466220
SHORT CGameAIBase::OpenDoor(CGameDoor* pDoor)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pDoor == NULL) {
        return ACTION_ERROR;
    }

    if ((pDoor->m_dwFlags & 0x1) != 0) {
        return ACTION_DONE;
    }

    // NOTE: Result is unused.
    pDoor->GetClosestDest(m_pos);

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.m_idLocalPlayer != pDoor->m_remotePlayerID) {
        CMessageDoorOpen* pMessage = new CMessageDoorOpen(TRUE, m_id, pDoor->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        return ACTION_DONE;
    }

    // NOTE: Only retries on `SHARED`, not on `DENIED`.
    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetDeny(pDoor->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pDoor),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED);

    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBaseActions.cpp
        // __LINE__: 3796
        UTIL_ASSERT(FALSE);

        return ACTION_ERROR;
    }

    CAITrigger trigger(CAITrigger::OPENED, m_typeAI, 0);

    CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger, m_id, pDoor->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    pDoor->ToggleDoor(m_typeAI, GetObjectType() != CGameObject::TYPE_SPRITE);

    pGame->GetObjectArray()->ReleaseDeny(pDoor->GetId(),
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return ACTION_DONE;
}

// 0x4664E0
SHORT CGameAIBase::CloseDoor(CGameDoor* pDoor)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pDoor == NULL) {
        return ACTION_ERROR;
    }

    if ((pDoor->m_dwFlags & 0x1) == 0) {
        return ACTION_DONE;
    }

    // NOTE: Result is unused.
    pDoor->GetClosestDest(m_pos);

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.m_idLocalPlayer != pDoor->m_remotePlayerID) {
        CMessageDoorOpen* pMessage = new CMessageDoorOpen(FALSE, m_id, pDoor->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        return ACTION_DONE;
    }

    // NOTE: Only retries on `SHARED`, not on `DENIED`.
    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetDeny(pDoor->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pDoor),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED);

    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBaseActions.cpp
        // __LINE__: 3876
        UTIL_ASSERT(FALSE);

        return ACTION_ERROR;
    }

    CAITrigger trigger(CAITrigger::CLOSED, m_typeAI, 0);

    CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger, m_id, pDoor->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    pDoor->ToggleDoor(m_typeAI, GetObjectType() != CGameObject::TYPE_SPRITE);

    pGame->GetObjectArray()->ReleaseDeny(pDoor->GetId(),
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return ACTION_DONE;
}

// 0x466750
SHORT CGameAIBase::MultiPlayerSync()
{
    if (g_pBaldurChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL) {
        return ACTION_DONE;
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == FALSE) {
        return ACTION_DONE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchServerPending == FALSE) {
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientPending = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchServerPending = TRUE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchServerFinished = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientFinished = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.RequestClientSignal(CBaldurMessage::SIGNAL_MPSYNCH);
        }

        if (g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchServerFinished == TRUE) {
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientPending = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchServerPending = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchServerFinished = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientFinished = FALSE;
            return ACTION_DONE;
        }

        return ACTION_NORMAL;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == FALSE) {
        if (g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientPending == FALSE) {
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientPending = TRUE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientFinished = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.MultiplayerSynchRequest();
        }

        if (g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientFinished == TRUE) {
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientPending = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchServerPending = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchServerFinished = FALSE;
            g_pBaldurChitin->m_cBaldurMessage.m_bMultiplayerSynchClientFinished = FALSE;
            return ACTION_DONE;
        }

        return ACTION_NORMAL;
    }

    return ACTION_ERROR;
}

// 0x466910
SHORT CGameAIBase::LeaveAreaLUAPanic()
{
    CPoint ptStart = m_curAction.m_dest;
    CString sArea = m_curAction.GetString1();
    LONG nFace = m_curAction.m_specificID;
    CString sParchment = m_curAction.GetString2();

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        g_pBaldurChitin->GetBaldurMessage()->SendLeaveAreaLUAPanic(ptStart, nFace, sArea, sParchment);
    }

    return ACTION_DONE;
}

// 0x4633A0
SHORT CGameAIBase::CreateItem()
{
    CGameContainer* pContainer;

    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(m_curAction.GetString1()), 1005, TRUE) == NULL) {
        return ACTION_ERROR;
    }

    CItem* pItem = new CItem(CResRef(m_curAction.GetString1()),
        static_cast<WORD>(m_curAction.m_specificID),
        static_cast<WORD>(m_curAction.m_specificID2),
        static_cast<WORD>(m_curAction.m_specificID3),
        0,
        0);

    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1) {
        for (int index = 0; index < CScreenInventory::PERSONAL_INVENTORY_SIZE; index++) {
            if (static_cast<CGameSprite*>(this)->m_equipment.m_items[18 + index] == NULL) {
                static_cast<CGameSprite*>(this)->m_equipment.m_items[18 + index] = pItem;
                return ACTION_DONE;
            }
        }

        LONG iPile = g_pBaldurChitin->GetObjectGame()->GetGroundPile(m_id);
        if (iPile != CGameObjectArray::INVALID_INDEX) {
            if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(iPile,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pContainer),
                    INFINITE)
                == CGameObjectArray::SUCCESS) {
                pContainer->PlaceItemInBlankSlot(pItem, TRUE, 0x7FFF);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(iPile,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        // NOTE: Dropping to the ground pile still returns `ACTION_ERROR`.
        return ACTION_ERROR;
    } else if (m_objectType == TYPE_SPRITE) {
        // NOTE: Starts from `SLOT_MISC` (quick items + personal inventory).
        for (int index = 0; index < 28; index++) {
            if (static_cast<CGameSprite*>(this)->m_equipment.m_items[CGameSpriteEquipment::SLOT_MISC + index] == NULL) {
                static_cast<CGameSprite*>(this)->m_equipment.m_items[CGameSpriteEquipment::SLOT_MISC + index] = pItem;
                return ACTION_DONE;
            }
        }

        LONG iPile = g_pBaldurChitin->GetObjectGame()->GetGroundPile(m_id);
        if (iPile != CGameObjectArray::INVALID_INDEX) {
            if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(iPile,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pContainer),
                    INFINITE)
                == CGameObjectArray::SUCCESS) {
                pContainer->PlaceItemInBlankSlot(pItem, TRUE, 0x7FFF);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(iPile,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        // NOTE: Dropping to the ground pile still returns `ACTION_ERROR`.
        return ACTION_ERROR;
    } else if (m_objectType == TYPE_CONTAINER) {
        static_cast<CGameContainer*>(this)->PlaceItemInBlankSlot(pItem, TRUE, 0x7FFF);
    } else {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        LONG iPile = pGame->GetGroundPile(m_id);
        if (iPile == CGameObjectArray::INVALID_INDEX) {
            // NOTE: Leaks `pItem`.
            return ACTION_ERROR;
        }

        if (pGame->GetObjectArray()->GetDeny(iPile,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pContainer),
                INFINITE)
            != CGameObjectArray::SUCCESS) {
            // NOTE: Leaks `pItem`.
            return ACTION_ERROR;
        }

        pContainer->PlaceItemInBlankSlot(pItem, TRUE, 0x7FFF);

        pGame->GetObjectArray()->ReleaseDeny(iPile,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    return ACTION_DONE;
}

// 0x463B30
SHORT CGameAIBase::TakePartyItem()
{
    CGameSprite* pSprite;
    BYTE rc;

    CString sName = m_curAction.GetString1();
    SHORT actionID = m_curAction.m_actionID;
    CItem* pItem = NULL;
    LONG nNumber = 1;
    BOOL bRange = FALSE;
    BOOL bNumber = FALSE;
    LONG nRange = 0;

    // TakePartyItemNum
    if (actionID == 204) {
        nNumber = m_curAction.m_specificID;
        bNumber = TRUE;
    }

    // TakePartyItemRange
    if (actionID == 193) {
        nRange = m_curAction.m_specificID;
        bRange = TRUE;
    }

    for (SHORT nIndex = 0; nIndex < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nIndex++) {
        int nCount = 0;

        // NOTE: Uninline.
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nIndex);

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            return 0;
        }

        if (bRange) {
            if (pSprite->m_pArea != m_pArea) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                continue;
            }

            const CPoint& pos = pSprite->GetPos();
            LONG dy = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY - pos.y / CPathSearch::GRID_SQUARE_SIZEY;
            LONG dx = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX - pos.x / CPathSearch::GRID_SQUARE_SIZEX;
            if (dx * dx + dy * dy > nRange) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                continue;
            }
        }

        SHORT nSlot = pSprite->FindItemPersonal(sName, 0, FALSE);
        while (nSlot != -1) {
            pItem = new CItem(*pSprite->m_equipment.m_items[nSlot]);

            CMessageRemoveInventoryItem* pRemove = new CMessageRemoveInventoryItem(nSlot,
                m_id,
                pSprite->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pRemove, FALSE);

            CAbilityId abilityId;
            abilityId.m_itemType = 2;
            abilityId.m_itemNum = nSlot;
            abilityId.m_abilityNum = 0;
            pSprite->CheckQuickLists(&abilityId, 0, TRUE, FALSE);

            abilityId.m_abilityNum = 1;
            pSprite->CheckQuickLists(&abilityId, 0, TRUE, FALSE);

            abilityId.m_abilityNum = 2;
            pSprite->CheckQuickLists(&abilityId, 0, TRUE, FALSE);

            if (bNumber) {
                if (pItem->GetMaxStackable() > 1) {
                    if (pItem->GetUsageCount(0) <= nNumber) {
                        nNumber -= pItem->GetUsageCount(0);
                    } else {
                        pItem->SetUsageCount(0, pItem->GetUsageCount(0) - static_cast<WORD>(nNumber));
                        nNumber = 0;

                        // NOTE: The original inlines a variant of the
                        // constructor which uses `CItem::operator=`.
                        CMessageAddInventoryItem* pAdd = new CMessageAddInventoryItem(*pItem,
                            m_id,
                            pSprite->m_id);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(pAdd, FALSE);
                    }
                } else {
                    nNumber--;
                }
            }

            sub_464950(pItem, TRUE, TRUE, 1, TRUE);

            if (m_curAction.m_actionID != 188 && m_curAction.m_actionID != 204) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return ACTION_DONE;
            }

            nCount++;

            if (m_curAction.m_actionID == 204 && nNumber <= 0) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return ACTION_DONE;
            }

            nSlot = pSprite->FindItemPersonal(sName, nCount, FALSE);
        }

        if (pItem == NULL
            || m_curAction.m_actionID == 188
            || (m_curAction.m_actionID == 204 && nNumber > 0)) {
            SHORT nTaken = pSprite->TakeItemBags(sName, nNumber, -1);
            if (nTaken > 0) {
                nNumber -= nTaken;

                CItem* pBagItem = new CItem(CResRef(sName), 1, 0, 0, 0, 0);
                sub_464950(pBagItem, TRUE, TRUE, nTaken, TRUE);
            }

            if (m_curAction.m_actionID == 204 && nNumber <= 0) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return ACTION_DONE;
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    if (pItem != NULL) {
        return ACTION_DONE;
    }

    return ACTION_ERROR;
}

// 0x464200
SHORT CGameAIBase::sub_464200()
{
    C2DArray tItems;
    CString sItem;

    tItems.Load(CResRef(m_curAction.GetString1()));

    INT nCount = -1;
    if (m_curAction.m_actionID == 226) {
        nCount = m_curAction.m_specificID;
    }

    for (INT nRow = 0; nRow < tItems.GetHeight(); nRow++) {
        // NOTE: Uninline.
        sItem = tItems.GetAt(CPoint(0, nRow));

        BOOL bFound = FALSE;

        for (INT nIndex = 0; nIndex < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nIndex++) {
            INT nSkip = 0;

            // NOTE: Uninline.
            LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nIndex));

            CGameSprite* pSprite;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                return ACTION_NORMAL;
            }

            SHORT nSlot = pSprite->FindItemPersonal(sItem, 0, FALSE);
            if (nSlot != -1) {
                bFound = TRUE;

                do {
                    CItem* pItem = new CItem(*pSprite->m_equipment.m_items[nSlot]);

                    CMessageRemoveInventoryItem* pMessage = new CMessageRemoveInventoryItem(nSlot,
                        m_id,
                        pSprite->GetId());
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

                    CAbilityId abilityId;
                    abilityId.m_itemType = 2;
                    abilityId.m_itemNum = nSlot;
                    abilityId.m_abilityNum = 0;
                    pSprite->CheckQuickLists(&abilityId, 0, TRUE, FALSE);

                    abilityId.m_abilityNum = 1;
                    pSprite->CheckQuickLists(&abilityId, 0, TRUE, FALSE);

                    abilityId.m_abilityNum = 2;
                    pSprite->CheckQuickLists(&abilityId, 0, TRUE, FALSE);

                    sub_464950(pItem, TRUE, TRUE, 1, TRUE);

                    nSkip++;
                    nSlot = pSprite->FindItemPersonal(sItem, nSkip, FALSE);
                } while (nSlot != -1);
            }

            SHORT nTaken = pSprite->TakeItemBags(sItem, 0x7FFF, -1);
            if (nTaken > 0) {
                bFound = TRUE;

                CItem* pItem = new CItem(CResRef(sItem), 1, 0, 0, 0, 0);
                sub_464950(pItem, TRUE, TRUE, nTaken, FALSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        if (bFound && m_curAction.m_actionID == 226) {
            nCount--;
            if (nCount <= 0) {
                return ACTION_DONE;
            }
        }
    }

    if (nCount == m_curAction.m_specificID && m_curAction.m_actionID == 226) {
        CString sDefault = tItems.GetDefault();
        sDefault.MakeUpper();

        if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(sDefault), 1005, TRUE) != NULL) {
            CItem* pItem = new CItem(CResRef(sDefault), 0, 0, 0, 0, 0);
            sub_464950(pItem, TRUE, TRUE, 1, TRUE);
        }
    }

    return ACTION_DONE;
}

// NOTE: Uninlined from `CGameAIBase::sub_464950` (the original repeats this
// block for both the party and the non-party branch).
static void sub_464950_DropOnGround(LONG nSpriteId, CItem* pItem)
{
    LONG nContainerId = g_pBaldurChitin->GetObjectGame()->GetGroundPile(nSpriteId);
    if (nContainerId != CGameObjectArray::INVALID_INDEX) {
        CGameContainer* pContainer;
        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            pContainer->PlaceItemInBlankSlot(pItem, TRUE, 0x7FFF);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nContainerId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x464950
BOOL CGameAIBase::sub_464950(CItem* pItem, BOOL a2, BOOL a3, INT nCount, BOOL bFeedback)
{
    // NOTE: The inventory is accessed through `this` once the object is known
    // to be a party member or `TYPE_SPRITE`.
    CGameSprite* pSprite = static_cast<CGameSprite*>(this);

    if (pItem == NULL) {
        return TRUE;
    }

    // NOTE: Unsigned compare.
    if (static_cast<UINT>(nCount) > 1) {
        CItem* pCopy = new CItem(*pItem);
        sub_464950(pCopy, a2, a3, nCount - 1, TRUE);
    }

    pItem->m_flags |= 0x2;

    if (g_pBaldurChitin->GetObjectGame()->m_familiars.Find(reinterpret_cast<int*>(m_id), NULL) != NULL) {
        CMessageAddInventoryItem* pMessage = new CMessageAddInventoryItem(*pItem,
            m_id,
            g_pBaldurChitin->GetObjectGame()->GetProtagonist());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        return TRUE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1) {
        INT nSlot;
        for (nSlot = 0; nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE; nSlot++) {
            if (pSprite->m_equipment.m_items[18 + nSlot] == NULL) {
                break;
            }
        }

        if (nSlot >= CScreenInventory::PERSONAL_INVENTORY_SIZE) {
            if (a3) {
                sub_464950_DropOnGround(m_id, pItem);
            }
            return FALSE;
        }

        if (a2) {
            pSprite->m_equipment.m_items[18 + nSlot] = pItem;
            return TRUE;
        }

        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->m_equipment.m_items[18 + nSlot] = pItem;
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
            && g_pChitin->cNetwork.m_idLocalPlayer != m_remotePlayerID) {
            CMessageAddInventoryItem* pMessage = new CMessageAddInventoryItem(*pItem,
                m_id,
                m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }

        return TRUE;
    }

    if (m_objectType == TYPE_SPRITE) {
        if (bFeedback) {
            g_pBaldurChitin->GetObjectGame()->FeedBack(CInfGame::FEEDBACK_ITEMLOST, 0, TRUE);
        }

        INT nSlot;
        for (nSlot = 27; nSlot >= 0; nSlot--) {
            if (pSprite->m_equipment.m_items[15 + nSlot] == NULL) {
                break;
            }
        }

        if (nSlot < 0) {
            if (a3) {
                sub_464950_DropOnGround(m_id, pItem);
            }
            return FALSE;
        }

        if (a2) {
            pSprite->m_equipment.m_items[15 + nSlot] = pItem;
            return TRUE;
        }

        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            // FIXME: Original bug - the free slot was searched as `15 + nSlot`
            // (see above), but the item is stored at `18 + nSlot`.
            pSprite->m_equipment.m_items[18 + nSlot] = pItem;
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
            && g_pChitin->cNetwork.m_idLocalPlayer != m_remotePlayerID) {
            // NOTE: The original inlines `CItem::operator=` here (the other
            // two messages call it out of line).
            CMessageAddInventoryItem* pMessage = new CMessageAddInventoryItem(*pItem,
                m_id,
                m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }

        return TRUE;
    }

    if (m_objectType == TYPE_CONTAINER) {
        static_cast<CGameContainer*>(static_cast<CGameObject*>(this))->PlaceItemInBlankSlot(pItem, TRUE, 0x7FFF);
        return TRUE;
    }

    if (a3) {
        delete pItem;
    }

    return FALSE;
}

// 0x464EB0
SHORT CGameAIBase::GiveItemCreate(CGameObject* pObject)
{
    if (pObject == NULL) {
        return ACTION_ERROR;
    }

    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(m_curAction.GetString1()), 1005, TRUE) == NULL) {
        return ACTION_ERROR;
    }

    CItem* pItem = new CItem(CResRef(m_curAction.GetString1()),
        static_cast<WORD>(m_curAction.m_specificID),
        static_cast<WORD>(m_curAction.m_specificID2),
        static_cast<WORD>(m_curAction.m_specificID3),
        0,
        0);

    if (pObject->m_objectType == TYPE_SPRITE) {
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pObject->m_id) != -1) {
            g_pBaldurChitin->GetObjectGame()->FeedBack(CInfGame::FEEDBACK_ITEMGAINED, 0, TRUE);
        }

        CMessageAddInventoryItem* pMessage = new CMessageAddInventoryItem(*pItem,
            m_id,
            pObject->m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

        g_pBaldurChitin->GetObjectGame()->AddDisposableItem(pItem);

        return ACTION_DONE;
    }

    if (pObject->m_objectType == TYPE_CONTAINER) {
        static_cast<CGameContainer*>(pObject)->PlaceItemInBlankSlot(pItem, TRUE, 0x7FFF);
        return ACTION_DONE;
    }

    // NOTE: Leaks `pItem`.
    return ACTION_ERROR;
}

// 0x465220
SHORT CGameAIBase::GivePartyGoldGlobal()
{
    CVariable* pVariable;
    // FIXME: Uninitialized in the original when the MYAREA area is not
    // loaded.
    DWORD nGold = 0;

    CString sName = m_curAction.GetString1();
    sName.MakeUpper();

    CString sScope = m_curAction.GetString2();
    sScope.MakeUpper();

    if (strcmp(sScope, "GLOBAL") == 0) {
        pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        nGold = pVariable != NULL ? pVariable->m_intValue : 0;
    } else if (strcmp(sScope, "LOCALS") == 0) {
        if (GetObjectType() == TYPE_SPRITE) {
            CString sTemp(sName);
            pVariable = static_cast<CGameSprite*>(this)->GetLocalVariables()->FindKey(sTemp);
            nGold = pVariable != NULL ? pVariable->m_intValue : 0;
        } else {
            nGold = 0;
        }
    } else if (strcmp(sScope, "LOCALS") == 0) {
        // NOTE: Dead code, `LOCALS` is checked twice.
        CString sTemp(sName);
        pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sTemp);
        nGold = pVariable != NULL ? pVariable->m_intValue : 0;
    } else {
        if (strcmp(sScope, "MYAREA") == 0) {
            sScope = reinterpret_cast<const char*>(m_pArea->m_header.m_areaName);
        }

        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            pVariable = pArea->GetVariables()->FindKey(sName);
            nGold = pVariable != NULL ? pVariable->m_intValue : 0;
        }
    }

    if (m_objectType == TYPE_SPRITE) {
        if (static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold < nGold) {
            nGold = static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold;
        }

        static_cast<CGameSprite*>(this)->GetBaseStats()->m_gold -= nGold;
        static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold -= nGold;
    }

    CMessagePartyGold* pMessage = new CMessagePartyGold(TRUE,
        TRUE,
        nGold,
        m_id,
        m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
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

        // NOTE: `ptTarget` is passed to `GetClosestDest` uninitialized.
        CPoint ptTarget;
        BYTE nObjectType = pObject->GetObjectType();
        if (nObjectType == CGameObject::TYPE_DOOR) {
            ptTarget = static_cast<CGameDoor*>(pObject)->GetClosestDest(ptTarget);
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
