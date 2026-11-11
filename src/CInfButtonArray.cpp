#include "CInfButtonArray.h"

#include "CAIAction.h"
#include "CAIObjectType.h"
#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CButtonData.h"
#include "CDerivedStats.h"
#include "CGameButtonList.h"
#include "CGameEffect.h"
#include "CGameObjectArray.h"
#include "CGameSprite.h"
#include "CGameSpriteEquipment.h"
#include "CIcon.h"
#include "CInfGame.h"
#include "CItem.h"
#include "CMessage.h"
#include "CRuleTables.h"
#include "CScreenWorld.h"
#include "CSound.h"
#include "CSpell.h"
#include "CUIControlButton.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "CVidMode.h"
#include "FileFormat.h"
#include "IcewindCGameEffects.h"

// 0x851700
const BYTE CInfButtonArray::STATE_NONE = 0;

// 0x851701
const BYTE CInfButtonArray::byte_851701 = 110;

// 0x85170E
const BYTE CInfButtonArray::byte_85170E = 114;

// 0x587960
CInfButtonSettings::CInfButtonSettings()
{
    field_1C8 = 0;
    field_1CC = 0;
    field_1D8 = 0;
    field_0 = 0;
    field_4 = 0;
    field_8 = 0;
    field_1D0 = 0;
    m_bGreyOut = FALSE;
    field_C = -1;
    field_10 = -1;
}

// 0x587B80
CInfButtonSettings::~CInfButtonSettings()
{
}

// 0x8E6820
CGameButtonList* CInfButtonArray::m_pButtonList;

// 0x5879E0
CInfButtonArray::CInfButtonArray()
{
    field_1976 = 0;
    field_197A = 0;
    m_nSelectedButton = 100;
    field_16E0 = -1;
    field_16E4 = 0;
    m_nState = 0;
    SetState(STATE_NONE, 0);
    m_customizableButtonTypes[0] = 5;
    m_customizableButtonTypes[1] = 3;
    m_customizableButtonTypes[2] = 70;
    m_customizableButtonTypes[3] = 71;
    m_customizableButtonTypes[4] = 80;
    m_customizableButtonTypes[5] = 81;
    m_customizableButtonTypes[6] = 90;
    m_customizableButtonTypes[7] = 91;
    m_customizableButtonTypes[8] = 10;
    m_nCurrentSelectedSpellClass = 0;
    field_19DC = 0;
    field_19E0 = CResRef();
    m_nSelectedWeaponSet = 0;
}

// 0x587BD0
void CInfButtonArray::ClearList()
{
    if (m_pButtonList != NULL) {
        while (!m_pButtonList->IsEmpty()) {
            delete m_pButtonList->RemoveHead();
        }

        m_pButtonList->RemoveAll();

        if (m_pButtonList != NULL) {
            delete m_pButtonList;
        }

        m_pButtonList = NULL;
    }
}

// 0x587C20
CGameButtonList* CInfButtonArray::GetData(INT nButton, INT nType, BYTE& nClass, DWORD a4, BOOLEAN a5)
{
    CGameButtonList* pList = NULL;

    if (nType != 2
        && nType != 1
        && nType != 3
        && nType != 4
        && nType != 5
        && nType != 6) {
        return NULL;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return NULL;
    }

    LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
    LONG nCharacterId = groupList[0];
    delete groupList;

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return NULL;
    }

    switch (nType) {
    case 1:
        if (a5) {
            pList = pSprite->GetItemUsages(static_cast<SHORT>(nButton), static_cast<WORD>(nType), -1);
        } else {
            pList = pSprite->sub_717250(0);
        }
        break;
    case 2:
        if (nClass == 3 && a4 != 0) {
            pList = pSprite->sub_7155C0(nClass, a4);
        } else {
            pList = pSprite->sub_714F70(nClass);
        }
        break;
    case 3:
        if (g_pBaldurChitin->GetObjectGame()->m_cOptions.m_bQuickItemMapping && !a5) {
            pList = pSprite->GetItemUsages(static_cast<SHORT>(nButton + 15), static_cast<WORD>(nType), -1);
        } else {
            pList = pSprite->sub_716E80(0);
        }
        break;
    case 4:
        pList = pSprite->sub_715BD0();
        break;
    case 5:
        pList = pSprite->GetInternalButtonList();
        break;
    case 6:
        pList = pSprite->GetSongsButtonList();
        break;
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return pList;
}

// 0x587DF0
CGameButtonList* CInfButtonArray::GetFeatData(const CResRef& cResRef)
{
    CGameButtonList* pList = NULL;

    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return NULL;
    }

    LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
    LONG nCharacterId = groupList[0];
    delete groupList;

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return NULL;
    }

    // NOTE: The count returned in the second argument of `sub_546B60` is
    // capped by the constant at 0x85BCB4 (5).
    int nCount;
    int v1;
    int v2;

    if (cResRef == CGameSprite::SPIN275) {
        if (pSprite->sub_763150(47)) {
            g_pBaldurChitin->GetObjectGame()->GetRuleTables().sub_546B60(pSprite,
                nCount,
                v1,
                v2,
                FALSE);

            pList = pSprite->sub_717FC0(cResRef, min(nCount, 5) + 1);
        }
    } else if (cResRef == CGameSprite::SPIN276) {
        if (pSprite->sub_763150(19)) {
            g_pBaldurChitin->GetObjectGame()->GetRuleTables().sub_546B60(pSprite,
                nCount,
                v2,
                v1,
                FALSE);

            pList = pSprite->sub_717FC0(cResRef, min(nCount, 5) + 1);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return pList;
}

// 0x587F80
void CInfButtonArray::GetQuickButtonData(BYTE buttonNum, CButtonData& cButtonData, INT nType)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return;
    }

    LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
    LONG nCharacterId = groupList[0];
    delete groupList;

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    switch (nType) {
    case 1:
        pSprite->GetQuickWeapon(buttonNum, cButtonData);
        break;
    case 2:
        // NOTE: Uninline.
        pSprite->GetQuickSpell(buttonNum, cButtonData);
        break;
    case 3:
        // NOTE: Uninline.
        pSprite->GetQuickItem(buttonNum, cButtonData);
        break;
    case 4:
        // NOTE: Uninline.
        pSprite->GetQuickAbility(buttonNum, cButtonData);
        break;
    case 6:
        // NOTE: Uninline.
        pSprite->GetQuickSong(buttonNum, cButtonData);
        break;
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x588240
void CInfButtonArray::GetSelectedQuickWeaponData(CButtonData& cButtonData)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
        LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
        LONG nCharacterId = groupList[0];
        delete groupList;

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->GetSelectedWeaponButton(cButtonData);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x588300
void CInfButtonArray::sub_588300(BYTE nButtonNum, CButtonData& cButtonData)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
        LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
        LONG nCharacterId = groupList[0];
        delete groupList;

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->GetWeaponButton(nButtonNum, cButtonData);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5883C0
BYTE CInfButtonArray::GetSelectedModalMode()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    BYTE modalState = 0;

    if (pGame->GetGroup()->GetCount() != 0) {
        LONG nCharacterId = pGame->GetGroup()->GetGroupLeader();

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            modalState = pSprite->GetModalState();

            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    return modalState;
}

// 0x588460
BYTE CInfButtonArray::GetButtonId(INT buttonType)
{
    for (BYTE id = 0; id < 12; id++) {
        if (m_buttonTypes[id] == buttonType && !m_buttonArray[id].m_bGreyOut) {
            return id;
        }
    }
    return -1;
}

// 0x5884B0
BOOLEAN CInfButtonArray::sub_5884B0(CButtonData& cButtonData, BOOL a2)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return FALSE;
    }

    // NOTE: Uninline.
    LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    pSprite->ReadyOffInternalList(cButtonData, a2 == 0);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// 0x588570
void CInfButtonArray::sub_588570(INT buttonNum, INT nType)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
        // NOTE: Uninline.
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            switch (nType) {
            case 1:
                pSprite->SetSelectedWeaponButton(static_cast<SHORT>(buttonNum));
                break;
            case 2:
                pSprite->ReadySpell(static_cast<SHORT>(buttonNum), 2, FALSE);
                break;
            case 3:
                pSprite->ReadyItem(static_cast<SHORT>(buttonNum), FALSE);
                break;
            case 4:
                pSprite->ReadySpell(static_cast<SHORT>(buttonNum), 4, FALSE);
                break;
            case 6:
                pSprite->ReadySpell(static_cast<SHORT>(buttonNum), 6, FALSE);
                break;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5886A0
BOOLEAN CInfButtonArray::sub_5886A0(CButtonData& cButtonData, BOOL a2)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return FALSE;
    }

    // NOTE: Uninline.
    LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    pSprite->sub_71A0E0(cButtonData, a2 == 0);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// 0x588760
BOOLEAN CInfButtonArray::sub_588760(CButtonData& cButtonData, BOOL a2)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return FALSE;
    }

    // NOTE: Uninline.
    LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    pSprite->sub_71A550(cButtonData, a2 == 0);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// 0x588820
BOOLEAN CInfButtonArray::sub_588820(CButtonData& cButtonData, BOOL a2)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return FALSE;
    }

    // NOTE: Uninline.
    LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    UINT nSongIndex = 0;
    if (g_pBaldurChitin->GetObjectGame()->m_songs.Find(cButtonData.m_abilityId.m_res, nSongIndex) == true
        && a2) {
        if ((pSprite->m_derivedStats.m_generalState & STATE_SILENCED) != 0
            || (pSprite->m_baseStats.m_generalState & STATE_SILENCED) != 0) {
            pSprite->FeedBack(CGameSprite::FEEDBACK_SILENCED,
                0,
                0,
                0,
                -1,
                0,
                0);
            pSprite->SetModalState(0, FALSE);
            m_nSelectedButton = 100;
            g_pBaldurChitin->GetObjectGame()->m_nState = 0;
            UpdateButtons();
        } else if (pSprite->m_nModalState == 1) {
            pSprite->SetModalState(0, FALSE);
            m_nSelectedButton = 100;
        } else {
            pSprite->m_nLastSong = static_cast<BYTE>(nSongIndex);
            pSprite->SetModalState(1, FALSE);
            m_nSelectedButton = 2;

            CSound cSound;

            // NOTE: Uninline.
            cSound.SetResRef(CResRef("ACT_01"), TRUE, TRUE);

            cSound.SetFireForget(TRUE);
            cSound.SetChannel(5, 0);
            cSound.Play(FALSE);

            CAIAction action(CAIAction::SMALLWAIT, CPoint(-1, -1), 1, -1);

            CMessage* message = new CMessageAddAction(action,
                pSprite->m_id,
                pSprite->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// 0x588CB0
void CInfButtonArray::SetQuickButtonData(CButtonData& cButtonData, BYTE buttonNum, INT nType)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return;
    }

    LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
    LONG nCharacterId = groupList[0];
    delete groupList;

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    switch (nType) {
    case 1:
        // NOTE: Uninline.
        pSprite->SetQuickWeapon(buttonNum, cButtonData);

        if (cButtonData.m_abilityId.m_itemNum >= 11 && cButtonData.m_abilityId.m_itemNum <= 14) {
            // NOTE: Uninline.
            pSprite->SetQuickWeapon(buttonNum, static_cast<BYTE>(cButtonData.m_abilityId.m_itemNum));
        } else {
            // NOTE: Uninline.
            pSprite->SetQuickWeapon(buttonNum, static_cast<BYTE>(0));
        }

        // NOTE: Unconditionally overwrites the value set above.
        //
        // NOTE: Uninline.
        pSprite->SetQuickWeapon(buttonNum, static_cast<BYTE>(cButtonData.m_abilityId.m_itemNum));
        break;
    case 2:
        // NOTE: Uninline.
        pSprite->SetQuickSpell(buttonNum, cButtonData);
        break;
    case 3:
        // NOTE: Uninline.
        pSprite->SetQuickItem(buttonNum, cButtonData);
        break;
    case 4:
        // NOTE: Uninline.
        pSprite->SetQuickAbility(buttonNum, cButtonData);
        break;
    case 6:
        // NOTE: Uninline.
        pSprite->SetQuickSong(buttonNum, cButtonData);
        break;
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x588FF0
BOOL CInfButtonArray::ResetState()
{
    if (m_stateStack.size() != 0) {
        INT nState = m_stateStack.front();
        m_stateStack.pop_front();
        SetState(nState, 0);
    }

    return TRUE;
}

// 0x589100
void CInfButtonArray::UpdateState()
{
    SetState(m_nState, 0);
}

// 0x589110
BOOL CInfButtonArray::SetState(INT nState, BOOLEAN bPush)
{
    INT nIndex;

    // NOTE: Uninline.
    field_16E8.SetResRef(CResRef("GUIBTBUT"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);

    // NOTE: Uninline.
    field_17C2.SetResRef(CResRef("GUIBTACT"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);

    if (g_pBaldurChitin->pActiveEngine != NULL
        && g_pBaldurChitin->pActiveEngine->pVidMode != NULL) {
        g_pBaldurChitin->m_pEngineWorld->GetManager()->ClearTooltip();
    }

    // NOTE: Uninline.
    ClearList();

    if (bPush == TRUE && nState != m_nState && m_nState != 0) {
        m_stateStack.push_front(m_nState);
    }

    switch (nState) {
    case 101: {
        BYTE nClass = 0;
        m_pButtonList = GetData(field_1976 + 43, 1, nClass, 0, TRUE);

        for (nIndex = 0; nIndex < 12; nIndex++) {
            m_buttonTypes[nIndex] = 21 + nIndex;
        }

        m_nState = 101;
        return UpdateButtons();
    }
    case 102:
    case 103:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
        // __LINE__: 1370
        // NOTE: `CAIOBJECT_CLASS_NONE` (0) in the original.
        UTIL_ASSERT(m_nCurrentSelectedSpellClass != 0);

        m_pButtonList = GetData(field_1976, 2, m_nCurrentSelectedSpellClass, field_19DC, FALSE);
        break;
    case 104:
    case 105: {
        BYTE nClass = 0;
        m_pButtonList = GetData(field_1976, 3, nClass, 0, nState == 105);
        break;
    }
    case 106:
    case 107: {
        BYTE nClass = 0;
        m_pButtonList = GetData(field_1976, 4, nClass, 0, FALSE);
        break;
    }
    case 108:
        for (nIndex = 0; nIndex < 12; nIndex++) {
            m_buttonTypes[nIndex] = 21 + nIndex;
        }

        m_nState = 108;
        return UpdateButtons();
    case 109:
        for (nIndex = 0; nIndex < 12; nIndex++) {
            m_buttonTypes[nIndex] = 21 + nIndex;
        }

        m_nState = 109;
        return UpdateButtons();
    case 110:
        m_buttonTypes[0] = 7;
        m_buttonTypes[1] = 8;
        m_buttonTypes[2] = 15;
        m_buttonTypes[3] = 16;
        m_buttonTypes[4] = 17;
        m_buttonTypes[5] = 18;
        m_buttonTypes[6] = 19;
        m_buttonTypes[7] = 20;
        m_buttonTypes[8] = 100;
        m_buttonTypes[9] = 100;
        m_buttonTypes[10] = 100;
        m_buttonTypes[11] = 100;
        m_nState = nState;
        return UpdateButtons();
    case 111:
        for (nIndex = 0; nIndex < 12; nIndex++) {
            m_buttonTypes[nIndex] = 100;
        }

        m_nState = 111;
        return UpdateButtons();
    case 112: {
        BYTE nClass = 0;
        m_pButtonList = GetData(field_1976, 5, nClass, 0, FALSE);
        break;
    }
    case 113:
    case 122: {
        BYTE nClass = 0;
        m_pButtonList = GetData(field_1976, 6, nClass, 0, FALSE);
        break;
    }
    case 114:
        UpdateTempSelectedWeaponSet();

        m_buttonTypes[0] = 7;
        m_nState = nState;
        m_buttonTypes[1] = 2 * m_nSelectedWeaponSet + 60;
        m_buttonTypes[2] = 2 * m_nSelectedWeaponSet + 61;
        m_buttonTypes[3] = m_customizableButtonTypes[0];
        m_buttonTypes[4] = m_customizableButtonTypes[1];
        m_buttonTypes[5] = m_customizableButtonTypes[2];
        m_buttonTypes[6] = m_customizableButtonTypes[3];
        m_buttonTypes[7] = m_customizableButtonTypes[4];
        m_buttonTypes[8] = m_customizableButtonTypes[5];
        m_buttonTypes[9] = m_customizableButtonTypes[6];
        m_buttonTypes[10] = m_customizableButtonTypes[7];
        m_buttonTypes[11] = m_customizableButtonTypes[8];
        return UpdateButtons();
    case 115:
    case 116:
        m_buttonTypes[0] = 11;
        m_buttonTypes[1] = 4;
        m_buttonTypes[2] = 12;
        m_buttonTypes[3] = 119;
        m_buttonTypes[4] = 13;
        m_buttonTypes[5] = 100;
        m_buttonTypes[6] = 100;
        m_buttonTypes[7] = 100;
        m_buttonTypes[8] = 100;
        m_buttonTypes[9] = 100;
        m_buttonTypes[10] = 100;
        m_buttonTypes[11] = 100;

        if (m_nState != nState) {
            field_197A = 0;
        }

        m_nState = nState;
        return UpdateButtons();
    case 117: {
        CGameSprite* pSprite = NULL;
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return FALSE;
        }

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
        // __LINE__: 1631
        UTIL_ASSERT(pSprite != NULL);

        BOOLEAN bCaster = pSprite->sub_724900();

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        m_buttonTypes[0] = 35;
        if (bCaster == TRUE) {
            m_buttonTypes[1] = 36;
            m_buttonTypes[2] = 37;
            m_buttonTypes[3] = 38;
            m_buttonTypes[4] = 39;
            if (sub_595C80() == TRUE) {
                m_buttonTypes[5] = 40;
            } else {
                m_buttonTypes[5] = 100;
            }
        } else {
            m_buttonTypes[1] = 37;
            m_buttonTypes[2] = 38;
            m_buttonTypes[3] = 39;
            if (sub_595C80() == TRUE) {
                m_buttonTypes[4] = 40;
            } else {
                m_buttonTypes[4] = 100;
            }
            m_buttonTypes[5] = 100;
        }

        m_nState = nState;
        m_buttonTypes[6] = 100;
        m_buttonTypes[7] = 100;
        m_buttonTypes[8] = 100;
        m_buttonTypes[9] = 100;
        m_buttonTypes[10] = 41;
        m_buttonTypes[11] = 42;
        return UpdateButtons();
    }
    case 118:
    case 119: {
        UINT nCount = 0;
        BYTE nClass;

        nClass = CAIOBJECTTYPE_C_BARD;
        if (sub_595D60(nClass) == TRUE) {
            m_buttonTypes[nCount++] = 50;
        }

        nClass = CAIOBJECTTYPE_C_CLERIC;
        if (sub_595D60(nClass) == TRUE) {
            m_buttonTypes[nCount++] = 51;
            m_buttonTypes[nCount++] = 57;
        }

        nClass = CAIOBJECTTYPE_C_DRUID;
        if (sub_595D60(nClass) == TRUE) {
            m_buttonTypes[nCount++] = 52;
        }

        nClass = CAIOBJECTTYPE_C_PALADIN;
        if (sub_595D60(nClass) == TRUE) {
            m_buttonTypes[nCount++] = 53;
        }

        nClass = CAIOBJECTTYPE_C_RANGER;
        if (sub_595D60(nClass) == TRUE) {
            m_buttonTypes[nCount++] = 54;
        }

        nClass = CAIOBJECTTYPE_C_SORCERER;
        if (sub_595D60(nClass) == TRUE) {
            m_buttonTypes[nCount++] = 55;
        }

        nClass = CAIOBJECTTYPE_C_WIZARD;
        if (sub_595D60(nClass) == TRUE) {
            m_buttonTypes[nCount++] = 56;
        }

        while (nCount < 12) {
            m_buttonTypes[nCount++] = 100;
        }

        if (m_nState != nState) {
            field_197A = 0;
        }

        m_nState = nState;
        return UpdateButtons();
    }
    case 120:
        m_buttonTypes[0] = 80;
        m_buttonTypes[1] = 81;
        m_buttonTypes[2] = 82;
        m_buttonTypes[3] = 100;
        m_buttonTypes[4] = 100;
        m_buttonTypes[5] = 100;
        m_buttonTypes[6] = 100;
        m_buttonTypes[7] = 100;
        m_buttonTypes[8] = 100;
        m_buttonTypes[9] = 100;
        m_buttonTypes[10] = 100;
        m_buttonTypes[11] = 100;

        if (m_nState != nState) {
            field_197A = 0;
        }

        m_nState = nState;
        return UpdateButtons();
    case 121:
        m_buttonTypes[0] = 60;
        m_buttonTypes[1] = 61;
        m_buttonTypes[2] = 100;
        m_buttonTypes[3] = 62;
        m_buttonTypes[4] = 63;
        m_buttonTypes[5] = 100;
        m_buttonTypes[6] = 64;
        m_buttonTypes[7] = 65;
        m_buttonTypes[8] = 100;
        m_buttonTypes[9] = 66;
        m_buttonTypes[10] = 67;
        m_buttonTypes[11] = 100;
        m_nState = 121;
        return UpdateButtons();
    case 123:
        m_pButtonList = GetFeatData(field_19E0);

        for (nIndex = 0; nIndex < 12; nIndex++) {
            m_buttonTypes[nIndex] = 21 + nIndex;
        }

        if (m_nState != nState) {
            field_197A = 0;
        }

        m_nState = nState;
        return UpdateButtons();
    default:
        for (nIndex = 0; nIndex < 12; nIndex++) {
            m_buttonTypes[nIndex] = 100;
        }

        m_nState = 0;
        return UpdateButtons();
    }

    if (m_pButtonList != NULL && m_pButtonList->GetCount() > 12) {
        m_buttonTypes[0] = 33;
        for (nIndex = 1; nIndex < 11; nIndex++) {
            m_buttonTypes[nIndex] = 20 + nIndex;
        }
        m_buttonTypes[11] = 34;
    } else {
        for (nIndex = 0; nIndex < 12; nIndex++) {
            m_buttonTypes[nIndex] = 21 + nIndex;
        }
    }

    if (m_nState != nState) {
        field_197A = 0;
    }

    m_nState = nState;
    return UpdateButtons();
}

// 0x589FF0
void CInfButtonArray::sub_589FF0(BOOLEAN bPush, BOOLEAN bAll)
{
    if (m_stateStack.size() != 0) {
        if (bAll == TRUE) {
            INT nState = m_stateStack.back();
            m_stateStack.clear();
            SetState(nState, 0);
        } else {
            INT nState = m_stateStack.front();
            m_stateStack.pop_front();
            SetState(nState, bPush);
        }
    }
}

// 0x58A340
BOOL CInfButtonArray::UpdateButtons()
{
    CButtonData buttonData;
    CButtonData buttonData2;
    CString sTemp;
    CGameSprite* pSprite = NULL;
    CString sKey;
    CUIPanel* pPanel;
    LONG nCharacterId;
    BYTE rc;
    BYTE bClassHasSpells[8];
    BOOLEAN bHasSpells;
    BOOLEAN bHasSongs;
    BOOLEAN bHasAbilities;
    INT nButton;
    CUIControlButton* pControl;
    CInfButtonSettings* pSettings;

    pPanel = g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(1);
    if (pPanel == NULL) {
        return TRUE;
    }

    nCharacterId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pSprite),
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
    // __LINE__: 1800
    UTIL_ASSERT(pSprite != NULL);

    memset(bClassHasSpells, 0, sizeof(bClassHasSpells));
    bHasSpells = FALSE;
    bHasSongs = FALSE;

    if (pSprite->sub_724900() == TRUE) {
        UINT nClassIndex;
        UINT nLevel;

        for (nClassIndex = 0; nClassIndex < CSPELLLIST_NUM_CLASSES; nClassIndex++) {
            // __FILE__: .\Include\ObjCreature.h
            // __LINE__: 1741
            UTIL_ASSERT(nClassIndex <= CSPELLLIST_NUM_CLASSES);

            for (nLevel = 0; nLevel < pSprite->m_spells.Get(nClassIndex)->m_nHighestLevel; nLevel++) {
                // NOTE: Uninline.
                bClassHasSpells[nClassIndex] |= pSprite->m_spells.GetSpellsAtLevel(nClassIndex, nLevel)->field_10 > 0;
            }
        }

        for (nLevel = 0; nLevel < pSprite->m_domainSpells.m_nHighestLevel; nLevel++) {
            // __FILE__: .\Include\FileFormat.h
            // __LINE__: 2521
            UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

            bClassHasSpells[7] |= pSprite->m_domainSpells.m_lists[nLevel].field_10 > 0;
        }

        // NOTE: The two loops below check `field_18` (the loops above check
        // `field_10`).
        for (nLevel = 0; nLevel < pSprite->GetSpells(2)->m_nHighestLevel; nLevel++) {
            bClassHasSpells[0] |= pSprite->GetSpellsAtLevel(2, nLevel)->field_18 > 0;
        }

        for (nLevel = 0; nLevel < pSprite->GetSpells(10)->m_nHighestLevel; nLevel++) {
            bClassHasSpells[5] |= pSprite->GetSpellsAtLevel(10, nLevel)->field_18 > 0;
        }

        for (UINT nIndex = 0; nIndex < 8; nIndex++) {
            if (bHasSpells) {
                break;
            }
            bHasSpells = bClassHasSpells[nIndex] != 0;
        }
    }

    if (pSprite->m_innateSpells.field_10 > 0
        || pSprite->m_shapeshifts.field_18 > 0
        || pSprite->m_shapeshifts.field_10 > 0) {
        bHasAbilities = TRUE;
    } else {
        bHasAbilities = FALSE;
    }

    if (pSprite->sub_724920() == TRUE) {
        bHasSongs = pSprite->m_songs.m_List.size() > 0;
    }

    for (nButton = 0; nButton < 12; nButton++) {
        pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nButton + 6));
        if (pControl == NULL) {
            continue;
        }

        sKey.Format("F%d", nButton + 1);

        pSettings = &(m_buttonArray[nButton]);
        pSettings->field_1CC = m_nSelectedButton == m_buttonTypes[nButton];
        pSettings->m_bGreyOut = FALSE;

        switch (m_buttonTypes[nButton]) {
        case 0: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 48;
            pSettings->field_10 = 48;
            pSettings->field_1C8 = 0;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            pControl->SetToolTipStrRef(-1, -1, -1);
            break;
        }
        case 1: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 52;
            pSettings->field_10 = 52;
            pSettings->field_1C8 = 0;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            pControl->SetToolTipStrRef(-1, -1, -1);
            break;
        }
        case 2: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef("STONSONG"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 20;
            pSettings->field_10 = 22;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;
            pSettings->field_1CC = 0;

            if (!bHasSongs) {
                pSettings->m_bGreyOut = TRUE;
            }

            STRREF strRef = 4918;
            STRREF secondaryStrRef = -1;

            if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
                LONG nLeaderId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

                CGameSprite* pLeader;
                if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nLeaderId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pLeader),
                        INFINITE)
                    == CGameObjectArray::SUCCESS) {
                    BYTE nModalState = pLeader->m_nModalState;

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nLeaderId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    if (nModalState == 1) {
                        pSettings->field_1CC = 1;

                        UINT nSongIndex = pSprite->m_nLastSong;

                        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
                        // __LINE__: 2184
                        UTIL_ASSERT_MSG(nSongIndex < g_pBaldurChitin->GetObjectGame()->GetMasterSongLookup().m_nCount, "Invalid Song Index.");

                        // NOTE: Uninline.
                        const CResRef& cSongResRef = g_pBaldurChitin->GetObjectGame()->GetMasterSongLookup().Get(nSongIndex);

                        CGameButtonList* pSongs = pSprite->GetSongsButtonList();
                        if (pSongs != NULL) {
                            if (cSongResRef != "") {
                                CButtonData* pButtonData = NULL;
                                POSITION pos = pSongs->GetHeadPosition();
                                while (pos != NULL) {
                                    pButtonData = pSongs->GetNext(pos);
                                    if (pButtonData != NULL) {
                                        if (pButtonData->m_abilityId.m_res == cSongResRef) {
                                            break;
                                        }
                                        pButtonData = NULL;
                                    }
                                }

                                if (pButtonData != NULL) {
                                    strRef = pButtonData->m_name;
                                    secondaryStrRef = 39824;
                                }
                            }

                            while (!pSongs->IsEmpty()) {
                                delete pSongs->RemoveHead();
                            }
                            delete pSongs;
                        }
                    }
                }
            }

            pControl->SetToolTipStrRef(strRef, secondaryStrRef, -1);
            pControl->SetToolTipHotKey(10, 0xFFFF, sKey);
            break;
        }
        case 3: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            STRREF strRef = 4688;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 8;
            pSettings->field_10 = 10;
            pSettings->field_1C8 = 0;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            if (!bHasSpells) {
                pSettings->m_bGreyOut = TRUE;
                strRef = 37450;
            }

            pControl->SetToolTipStrRef(strRef, -1, -1);
            pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            break;
        }
        case 4: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 36;
            pSettings->field_10 = 38;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            BOOL bActive = FALSE;
            if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
                LONG nLeaderId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

                CGameSprite* pLeader;
                if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nLeaderId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pLeader),
                        INFINITE)
                    == CGameObjectArray::SUCCESS) {
                    BYTE nModalState = pLeader->m_nModalState;

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nLeaderId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    if (nModalState == 2) {
                        bActive = TRUE;
                    }
                }
            }

            if (bActive) {
                pSettings->field_1CC = 1;

                // NOTE: Original code really stores 5 here (not 4).
                m_nSelectedButton = 5;
            } else {
                pSettings->field_1CC = 0;
            }

            pControl->SetToolTipStrRef(4927, -1, -1);
            pControl->SetToolTipHotKey(16, 0xFFFF, sKey);
            break;
        }
        case 5: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 96;
            pSettings->field_10 = 98;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            STRREF strRef = 4933;

            // NOTE: Looks like inlined `GetSelectedModalMode` (0x5883C0), but
            // with a single `GetShare` attempt.
            BYTE nModalState = 0;
            if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
                LONG nLeaderId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

                CGameSprite* pLeader;
                if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nLeaderId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pLeader),
                        INFINITE)
                    == CGameObjectArray::SUCCESS) {
                    nModalState = pLeader->m_nModalState;

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nLeaderId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }

            if (nModalState == 2) {
                strRef = 39826;
                m_nSelectedButton = 5;
            } else if (nModalState == 3) {
                strRef = 39825;
                m_nSelectedButton = 5;
            }

            pControl->SetToolTipStrRef(strRef, -1, -1);
            pControl->SetToolTipHotKey(13, 0xFFFF, sKey);
            break;
        }
        case 6:
        default: {
            pSettings->field_0 = 0;
            pSettings->field_4 = 0;
            pSettings->field_8 = 0;
            pSettings->field_C = -1;
            pSettings->field_10 = -1;
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
            pSettings->field_1C8 = 0;
            pSettings->field_1CC = 0;
            pSettings->field_1D0 = 0;
            pSettings->field_1D8 = 0;

            pControl->SetToolTipStrRef(-1, -1, -1);
            break;
        }
        case 7: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 0;
            pSettings->field_10 = 2;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            pControl->SetToolTipStrRef(15925, -1, -1);
            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
            break;
        }
        case 8: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 12;
            pSettings->field_10 = 14;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            pControl->SetToolTipStrRef(4666, -1, -1);
            pControl->SetToolTipHotKey(18, 0xFFFF, sKey);
            break;
        }
        case 9: {
            if (pSprite->GetShapeshifts()->sub_58FE70() == 0) {
                pSettings->m_bGreyOut = TRUE;
            }

            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
            pSettings->field_C = 40;
            pSettings->field_10 = 40;
            pSettings->field_1C8 = 0;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            pControl->SetToolTipStrRef(4958, -1, -1);
            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
            break;
        }
        case 10: {
            STRREF strRef = 4954;

            if (!bHasAbilities || pSprite->m_derivedStats.m_disabledSpellTypes[2] == TRUE) {
                pSettings->m_bGreyOut = TRUE;
                strRef = 37443;
            }

            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 40;
            pSettings->field_10 = 42;
            pSettings->field_1C8 = 0;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            if (static_cast<BYTE>(sub_595EB0(pSprite)) == 1) {
                strRef = 39827;
                pSettings->field_1CC = 1;
            }

            pControl->SetToolTipStrRef(strRef, -1, -1);
            pControl->SetToolTipHotKey(19, 0xFFFF, sKey);
            break;
        }
        case 11: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 28;
            pSettings->field_10 = 30;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            if (pSprite->m_derivedStats.m_disabledButtons[0] == TRUE
                || pSprite->m_nStealthGreyOut > 0) {
                pSettings->m_bGreyOut = TRUE;
            }

            BOOL bActive = FALSE;
            if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
                LONG nLeaderId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

                CGameSprite* pLeader;
                if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nLeaderId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pLeader),
                        INFINITE)
                    == CGameObjectArray::SUCCESS) {
                    BYTE nModalState = pLeader->m_nModalState;

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nLeaderId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    if (nModalState == 3) {
                        bActive = TRUE;
                    }
                }
            }

            if (bActive) {
                pSettings->field_1CC = 1;

                // NOTE: Original code really stores 5 here (not 11).
                m_nSelectedButton = 5;
            } else {
                pSettings->field_1CC = 0;
            }

            pControl->SetToolTipStrRef(4968, -1, -1);
            pControl->SetToolTipHotKey(15, 0xFFFF, sKey);
            break;
        }
        case 12: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 24;
            pSettings->field_10 = 26;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            pControl->SetToolTipStrRef(4971, -1, -1);
            pControl->SetToolTipHotKey(14, 0xFFFF, sKey);
            break;
        }
        case 13: {
            // NOTE: Signed compare.
            if (static_cast<signed char>(pSprite->m_derivedStats.m_nSkills[1]) < 1) {
                pSettings->m_bGreyOut = TRUE;
            }

            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 124;
            pSettings->field_10 = 126;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            pControl->SetToolTipStrRef(4974, -1, -1);
            pControl->SetToolTipHotKey(9, 0xFFFF, sKey);
            break;
        }
        case 14: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;

            STRREF strRef = 4978;

            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 16;
            pSettings->field_10 = 18;
            pSettings->field_1C8 = 0;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            CGameButtonList* pItems = pSprite->sub_716E80(1);
            if (pItems == NULL
                || pItems->GetCount() == 0
                || pSprite->m_derivedStats.m_bPolymorphed == TRUE) {
                pSettings->m_bGreyOut = TRUE;
                strRef = 37449;
            }

            if (pItems != NULL) {
                while (!pItems->IsEmpty()) {
                    delete pItems->RemoveHead();
                }
                delete pItems;
            }

            pControl->SetToolTipStrRef(strRef, -1, -1);
            pControl->SetToolTipHotKey(12, 0xFFFF, sKey);
            break;
        }
        case 15: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 0;
            pSettings->field_8 = 1;

            // NOTE: Uninline.
            pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_C = 44;
            pSettings->field_10 = 44;
            pSettings->field_1C8 = 1;
            pSettings->field_1D8 = 0;
            pSettings->field_1D0 = 0;

            pControl->SetToolTipStrRef(15924, -1, -1);
            pControl->SetToolTipHotKey(17, 0xFFFF, sKey);
            break;
        }
        case 16:
        case 17:
        case 18:
        case 19:
        case 20: {
            pSettings->field_0 = 1;
            pSettings->field_4 = 1;
            pSettings->field_8 = 0;
            pSettings->field_C = -1;
            pSettings->field_10 = -1;
            pSettings->field_1C8 = 1;

            // NOTE: Uninline.
            pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

            pSettings->field_1D0 = 0;

            CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
            SHORT buttonNum = static_cast<SHORT>(m_buttonTypes[nButton]) - 16;

            // NOTE: Uninline (some `CInfGame` quick formation getter).

            // __FILE__: .\Include\InfGame.h
            // __LINE__: 1498
            UTIL_ASSERT(buttonNum < CGAMESAVE_NUM_QUICK_FORMATIONS);

            INT nFormation = pGame->m_gameSave.m_quickFormations[buttonNum];
            INT nCurFormation = g_pBaldurChitin->GetObjectGame()->m_gameSave.m_curFormation;

            if (nFormation == -1) {
                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef("STONFORM"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);

                nFormation = -1;
            } else if (nFormation < 10) {
                sTemp.Format("FORM%d", nFormation);

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(sTemp), g_pBaldurChitin->field_4A2C, TRUE, TRUE);
            } else {
                sTemp.Format("FORM%c", static_cast<char>(nFormation + 55));

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(sTemp), g_pBaldurChitin->field_4A2C, TRUE, TRUE);
            }

            pSettings->field_1D8 = 0;
            pSettings->field_1CC = nFormation == nCurFormation;

            pControl->SetToolTipStrRef(4935, -1, -1);
            pControl->SetToolTipHotKey(static_cast<WORD>(m_buttonTypes[nButton] + 32), 0xFFFF, sKey);
            break;
        }
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 1;
                pSettings->field_8 = 0;
                pSettings->field_C = -1;
                pSettings->field_10 = -1;
                pSettings->field_1C8 = 0;
                pSettings->field_1CC = 0;
                pSettings->field_1D0 = 0;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_1D8 = 0;

                pControl->SetToolTipStrRef(-1, -1, -1);
            } break;
            case 33: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 48;
                pSettings->field_10 = 48;
                pSettings->field_1C8 = 0;
                pSettings->field_1D0 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1CC = 0;

                pControl->SetToolTipStrRef(-1, -1, -1);
            } break;
            case 34: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 52;
                pSettings->field_10 = 52;
                pSettings->field_1C8 = 0;
                pSettings->field_1D0 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1CC = 0;

                pControl->SetToolTipStrRef(-1, -1, -1);
            } break;
            case 35: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 96;
                pSettings->field_10 = 98;
                pSettings->field_1C8 = 1;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                pControl->SetToolTipStrRef(4933, -1, -1);
            } break;
            case 36: {
                STRREF strRef = 4688;

                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 8;
                pSettings->field_10 = 10;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                if (!pSprite->sub_724900()) {
                    pSettings->m_bGreyOut = TRUE;
                    strRef = 37450;
                }

                pControl->SetToolTipStrRef(strRef, -1, -1);
            } break;
            case 37: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 16;
                pSettings->field_10 = 18;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                pControl->SetToolTipStrRef(4978, -1, -1);
            } break;
            case 38: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 112;
                pSettings->field_10 = 114;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                pControl->SetToolTipStrRef(4937, -1, -1);
            } break;
            case 39: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 40;
                pSettings->field_10 = 42;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                pControl->SetToolTipStrRef(4954, -1, -1);
            } break;
            case 40: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 1;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 20;
                pSettings->field_10 = 22;
                pSettings->field_1C8 = 1;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                pControl->SetToolTipStrRef(4918, -1, -1);
            } break;
            case 41: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 116;
                pSettings->field_10 = 118;
                pSettings->field_1C8 = 1;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                pControl->SetToolTipStrRef(39723, -1, -1);
            } break;
            case 42: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_C = 120;
                pSettings->field_10 = 122;
                pSettings->field_1C8 = 1;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                pControl->SetToolTipStrRef(39724, -1, -1);
            } break;
            case 50: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 56;
                pSettings->field_10 = 58;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                BYTE nClass = 2;
                if (!sub_595D60(nClass) || !bClassHasSpells[0]) {
                    pSettings->m_bGreyOut = TRUE;
                }

                pControl->SetToolTipStrRef(39709, -1, -1);
                pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            } break;
            case 51: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 60;
                pSettings->field_10 = 62;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                BYTE nClass = 3;
                if (!sub_595D60(nClass) || !bClassHasSpells[1]) {
                    pSettings->m_bGreyOut = TRUE;
                }

                pControl->SetToolTipStrRef(39710, -1, -1);
                pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            } break;
            case 52: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 64;
                pSettings->field_10 = 66;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                BYTE nClass = 4;
                if (!sub_595D60(nClass) || !bClassHasSpells[2]) {
                    pSettings->m_bGreyOut = TRUE;
                }

                pControl->SetToolTipStrRef(39711, -1, -1);
                pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            } break;
            case 53: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 68;
                pSettings->field_10 = 70;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                BYTE nClass = 7;
                if (!sub_595D60(nClass) || !bClassHasSpells[3]) {
                    pSettings->m_bGreyOut = TRUE;
                }

                pControl->SetToolTipStrRef(39712, -1, -1);
                pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            } break;
            case 54: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 72;
                pSettings->field_10 = 74;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                BYTE nClass = 8;
                if (!sub_595D60(nClass) || !bClassHasSpells[4]) {
                    pSettings->m_bGreyOut = TRUE;
                }

                pControl->SetToolTipStrRef(39713, -1, -1);
                pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            } break;
            case 55: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 76;
                pSettings->field_10 = 78;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                BYTE nClass = 10;
                if (!sub_595D60(nClass) || !bClassHasSpells[5]) {
                    pSettings->m_bGreyOut = TRUE;
                }

                pControl->SetToolTipStrRef(39714, -1, -1);
                pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            } break;
            case 56: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 80;
                pSettings->field_10 = 82;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                BYTE nClass = 11;
                if (!sub_595D60(nClass) || !bClassHasSpells[6]) {
                    pSettings->m_bGreyOut = TRUE;
                }

                pControl->SetToolTipStrRef(39715, -1, -1);
                pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            } break;
            case 57: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 84;
                pSettings->field_10 = 86;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                BYTE nClass = 3;
                if (!sub_595D60(nClass) || !bClassHasSpells[7]) {
                    pSettings->m_bGreyOut = TRUE;
                }

                pControl->SetToolTipStrRef(39940, -1, -1);
                pControl->SetToolTipHotKey(11, 0xFFFF, sKey);
            } break;
            case 60:
            case 61:
            case 62:
            case 63:
            case 64:
            case 65:
            case 66:
            case 67: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 1;
                pSettings->field_8 = 0;
                pSettings->field_C = 104;
                pSettings->field_10 = 106;
                pSettings->field_1C8 = 1;

                GetSelectedQuickWeaponData(buttonData);
                GetQuickButtonData(static_cast<BYTE>(m_buttonTypes[nButton] - 60), buttonData2, 1);

                if (m_nState == 121 && buttonData2.m_icon == "FIST") {
                    CButtonData launcherData;
                    sub_588300(m_buttonTypes[nButton] - 60, launcherData);

                    if (launcherData.m_icon != "") {
                        // NOTE: Uninline.
                        buttonData2 = launcherData;
                    }
                }

                if (buttonData2.m_icon != "") {
                    // NOTE: Uninline.
                    pSettings->field_14.SetResRef(buttonData2.m_icon, g_pBaldurChitin->field_4A2C, TRUE, TRUE);
                } else {
                    if (m_buttonTypes[nButton] == 61
                        || m_buttonTypes[nButton] == 63
                        || m_buttonTypes[nButton] == 65
                        || m_buttonTypes[nButton] == 67) {
                        // NOTE: Uninline.
                        pSettings->field_14.SetResRef(CResRef("STONSHIL"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);
                    } else {
                        // NOTE: Uninline.
                        pSettings->field_14.SetResRef(CResRef("STONWEAP"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);
                    }
                }

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(buttonData2.m_launcherIcon, g_pBaldurChitin->field_4A2C, TRUE, TRUE);

                if (buttonData2.m_bDisplayCount) {
                    pSettings->field_1D8 = buttonData2.m_count;
                }

                if (pSprite->m_nWeaponSet == (m_buttonTypes[nButton] - 60) / 2) {
                    pSettings->field_1D0 = 1;
                } else {
                    pSettings->field_1D0 = 0;
                }

                if (buttonData2.m_abilityId.m_abilityNum == buttonData.m_abilityId.m_abilityNum
                    && buttonData2.m_abilityId.m_itemNum == buttonData.m_abilityId.m_itemNum) {
                    if (m_nSelectedButton == 60
                        || m_nSelectedButton == 61
                        || m_nSelectedButton == 62
                        || m_nSelectedButton == 63
                        || m_nSelectedButton == 64
                        || m_nSelectedButton == 65
                        || m_nSelectedButton == 66
                        || m_nSelectedButton == 67) {
                        pSettings->field_1CC = 1;
                    }
                } else {
                    pSettings->field_1CC = 0;
                }

                if (buttonData2.m_name == -1) {
                    pControl->SetToolTipStrRef(4950, -1, -1);
                } else {
                    pControl->SetToolTipStrRef(buttonData2.m_name, -1, -1);
                }

                if (buttonData2.m_abilityId.m_abilityNum != -1) {
                    pControl->SetToolTipHotKey(static_cast<USHORT>(m_buttonTypes[nButton]) - 19, 0xFFFF, sKey);
                }
            } break;
            case 70:
            case 71:
            case 72:
            case 73:
            case 74:
            case 75:
            case 76:
            case 77:
            case 78: {
                // Quick spell slots.
                pSettings->field_0 = 1;
                pSettings->field_4 = 1;
                pSettings->field_8 = 0;
                pSettings->field_C = -1;
                pSettings->field_10 = -1;

                STRREF strToolTip = 4688;

                pSettings->field_1C8 = 0;

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_1D0 = 0;

                GetQuickButtonData(static_cast<BYTE>(m_buttonTypes[nButton] - 70), buttonData2, 2);

                if (buttonData2.m_icon != "") {
                    // NOTE: Uninline.
                    pSettings->field_14.SetResRef(buttonData2.m_icon, g_pBaldurChitin->field_4A2C, TRUE, TRUE);
                } else {
                    // NOTE: Uninline.
                    pSettings->field_14.SetResRef(CResRef("STONSPEL"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);
                }

                if (buttonData2.m_bDisplayCount) {
                    pSettings->field_1D8 = buttonData2.m_count;
                } else {
                    pSettings->field_1D8 = 0;
                }

                BOOL bSpellTypeDisabled = FALSE;
                INT nCanCast = 0;

                CSpell* curSpell = new CSpell(buttonData2.m_abilityId.m_res);

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
                // __LINE__: 2729
                UTIL_ASSERT(curSpell != NULL);

                if (curSpell->Demand()) {
                    if (curSpell->pRes != NULL) {
                        SHORT nCasterType = curSpell->GetCasterType();
                        bSpellTypeDisabled = pSprite->GetDerivedStats()->m_disabledSpellTypes[nCasterType];
                    }
                    curSpell->Release();

                    DWORD nSpecialization = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetSpecializationMask(buttonData2.m_abilityId.m_nClass,
                        static_cast<BYTE>(buttonData2.m_abilityId.field_1E));

                    nCanCast = pSprite->CanCast(buttonData2.m_abilityId.m_nClass,
                        nSpecialization,
                        curSpell);
                }

                if (curSpell != NULL) {
                    delete curSpell;
                }

                if (!bHasSpells || bSpellTypeDisabled == 1) {
                    pSettings->m_bGreyOut = TRUE;
                    strToolTip = 37450;
                } else {
                    pSettings->m_bGreyOut = nCanCast != 1;
                }

                if (pSettings->field_1D8 == 0) {
                    pSettings->m_bGreyOut = TRUE;
                }

                if (buttonData2.m_name == -1) {
                    pControl->SetToolTipStrRef(strToolTip, -1, -1);
                } else {
                    if (nCanCast != 0) {
                        buttonData2.m_abilityId.field_18 = -1;
                    }
                    pControl->SetToolTipStrRef(buttonData2.m_name, -1, buttonData2.m_abilityId.field_18);
                }

                // Find the keymap entry bound to this spell.
                WORD nKeymapIndex;
                for (nKeymapIndex = 53; nKeymapIndex < CINFGAME_KEYMAP_SIZE; nKeymapIndex++) {
                    if (buttonData2.m_abilityId.m_res == CString(off_8B4F84[nKeymapIndex - 53])) {
                        break;
                    }
                }

                // Fallback hot key is derived from the position among usable
                // quick spell buttons.
                INT nHotKeyIndex2 = 38;
                for (INT nIndex = nButton; nIndex != 0; nIndex--) {
                    if (m_buttonTypes[nIndex - 1] >= 70
                        && m_buttonTypes[nIndex - 1] <= 78
                        && !m_buttonArray[nIndex - 1].m_bGreyOut) {
                        nHotKeyIndex2++;
                    }
                }

                pControl->SetToolTipHotKey(nKeymapIndex < CINFGAME_KEYMAP_SIZE ? nKeymapIndex : 0xFFFF,
                    static_cast<USHORT>(nHotKeyIndex2),
                    sKey);
                break;
            }
            case 80:
            case 81:
            case 82: {
                // Quick item slots.
                pSettings->field_0 = 1;
                pSettings->field_4 = 1;
                pSettings->field_8 = 0;
                pSettings->field_C = -1;
                pSettings->field_10 = -1;

                STRREF strToolTip = 4978;

                pSettings->field_1C8 = 1;

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_1D0 = 0;

                GetQuickButtonData(static_cast<BYTE>(m_buttonTypes[nButton] - 80), buttonData2, 3);

                if (buttonData2.m_icon != "") {
                    // NOTE: Uninline.
                    pSettings->field_14.SetResRef(buttonData2.m_icon, g_pBaldurChitin->field_4A2C, TRUE, TRUE);
                } else {
                    // NOTE: Uninline.
                    pSettings->field_14.SetResRef(CResRef("STONITEM"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);
                }

                if (buttonData2.m_bDisplayCount) {
                    pSettings->field_1D8 = buttonData2.m_count;
                } else {
                    pSettings->field_1D8 = 0;
                }

                if (pSettings->field_1D8 > 0) {
                    pSettings->m_bGreyOut = FALSE;
                } else {
                    // NOTE: `field_1D4` is not assigned anywhere in this
                    // function.
                    if (pSettings->field_1D4 == 24727) {
                        pSettings->m_bGreyOut = FALSE;
                    } else {
                        pSettings->m_bGreyOut = TRUE;
                        strToolTip = 37449;
                    }
                }

                pControl->SetToolTipStrRef(buttonData2.m_name != -1 ? buttonData2.m_name : strToolTip, -1, -1);
                pControl->SetToolTipHotKey(static_cast<USHORT>(m_buttonTypes[nButton] - 35), 0xFFFF, sKey);

                if (pSprite->GetDerivedStats()->m_bPolymorphed == TRUE) {
                    pSettings->m_bGreyOut = TRUE;
                }
                break;
            }
            case 90:
            case 91:
            case 92:
            case 93:
            case 94:
            case 95:
            case 96:
            case 97:
            case 98: {
                // Quick ability slots.
                pSettings->field_0 = 1;
                pSettings->field_4 = 1;
                pSettings->field_8 = 0;
                pSettings->field_C = -1;
                pSettings->field_10 = -1;

                STRREF strToolTip = 4954;

                pSettings->field_1C8 = 0;
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_1D0 = 0;

                GetQuickButtonData(static_cast<BYTE>(m_buttonTypes[nButton] - 90), buttonData2, 4);

                if (buttonData2.m_icon != "") {
                    pSettings->field_14.SetResRef(buttonData2.m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                } else {
                    pSettings->field_14.SetResRef(CResRef("STONSPEC"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                }

                if (buttonData2.m_bDisplayCount) {
                    if (buttonData2.m_abilityId.m_res == CGameSprite::SPIN275) {
                        if (pSprite->sub_763150(CGAMESPRITE_FEAT_POWER_ATTACK)) {
                            buttonData2.m_count = static_cast<SHORT>(pSprite->sub_726270(CGAMESPRITE_FEAT_POWER_ATTACK));
                            pSettings->field_1CC = buttonData2.m_count > 0;
                        }
                    } else if (buttonData2.m_abilityId.m_res == CGameSprite::SPIN276) {
                        if (pSprite->sub_763150(CGAMESPRITE_FEAT_EXPERTISE)) {
                            buttonData2.m_count = static_cast<SHORT>(pSprite->sub_726270(CGAMESPRITE_FEAT_EXPERTISE));
                            pSettings->field_1CC = buttonData2.m_count > 0;
                        }
                    }
                    pSettings->field_1D8 = buttonData2.m_count;
                } else {
                    if (buttonData2.m_abilityId.m_res == CGameSprite::SPIN277) {
                        if (pSprite->sub_763150(CGAMESPRITE_FEAT_ARTERIAL_STRIKE)) {
                            buttonData2.m_count = static_cast<SHORT>(pSprite->sub_726270(CGAMESPRITE_FEAT_ARTERIAL_STRIKE));
                            pSettings->field_1CC = pSprite->sub_726270(CGAMESPRITE_FEAT_ARTERIAL_STRIKE) > 0;
                        }
                    } else if (buttonData2.m_abilityId.m_res == CGameSprite::SPIN278) {
                        if (pSprite->sub_763150(CGAMESPRITE_FEAT_HAMSTRING)) {
                            buttonData2.m_count = static_cast<SHORT>(pSprite->sub_726270(CGAMESPRITE_FEAT_HAMSTRING));
                            pSettings->field_1CC = pSprite->sub_726270(CGAMESPRITE_FEAT_HAMSTRING) > 0;
                        }
                    } else if (buttonData2.m_abilityId.m_res == CGameSprite::SPIN279) {
                        if (pSprite->sub_763150(CGAMESPRITE_FEAT_RAPID_SHOT)) {
                            buttonData2.m_count = static_cast<SHORT>(pSprite->sub_726270(CGAMESPRITE_FEAT_RAPID_SHOT));
                            pSettings->field_1CC = pSprite->sub_726270(CGAMESPRITE_FEAT_RAPID_SHOT) > 0;
                        }
                    }
                    pSettings->field_1D8 = 0;
                }

                if (!bHasAbilities || buttonData2.m_bDisabled) {
                    pSettings->m_bGreyOut = TRUE;
                    strToolTip = 37443;
                }

                if (buttonData2.m_name != -1) {
                    pControl->SetToolTipStrRef(buttonData2.m_name, -1, -1);
                } else {
                    pControl->SetToolTipStrRef(strToolTip, -1, -1);
                }
                break;
            }
            case 100: {
                // Empty slot.
                pSettings->field_0 = 0;
                pSettings->field_4 = 0;
                pSettings->field_8 = 0;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = -1;
                pSettings->field_10 = -1;
                pSettings->field_1C8 = 0;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;
                pSettings->field_1CC = 0;
                pSettings->m_bGreyOut = TRUE;

                if (m_nState == 114) {
                    pControl->SetToolTipStrRef(40976, -1, -1);
                } else {
                    pControl->SetToolTipStrRef(-1, -1, -1);
                }
                break;
            }
            case 110:
            case 111:
            case 112:
            case 113:
            case 114:
            case 115:
            case 116:
            case 117:
            case 118: {
                // Quick song slots.
                pSettings->field_0 = 1;
                pSettings->field_4 = 1;
                pSettings->field_8 = 0;
                pSettings->field_C = 20;
                pSettings->field_10 = 22;
                pSettings->field_1C8 = 0;
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);
                pSettings->field_1D0 = 0;

                GetQuickButtonData(static_cast<BYTE>(m_buttonTypes[nButton] - 110), buttonData2, 6);

                if (buttonData2.m_icon != "") {
                    pSettings->field_14.SetResRef(buttonData2.m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                } else {
                    pSettings->field_14.SetResRef(CResRef("STONSONG"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                }

                if (!bHasSongs) {
                    pSettings->m_bGreyOut = TRUE;
                }

                if (GetSelectedModalMode() == 1) {
                    UINT nSongIndex = 0;
                    if (g_pBaldurChitin->GetObjectGame()->GetMasterSongLookup().Find(buttonData2.m_abilityId.m_res, nSongIndex) == true) {
                        if (nSongIndex == pSprite->GetLastSong()) {
                            pSettings->field_1CC = 1;
                        }
                    }
                }

                if (buttonData2.m_name != -1) {
                    pControl->SetToolTipStrRef(buttonData2.m_name, -1, -1);
                } else {
                    pControl->SetToolTipStrRef(37436, -1, -1);
                }
                break;
            }
            case 119: {
                pSettings->field_0 = 1;
                pSettings->field_4 = 0;
                pSettings->field_8 = 1;

                // NOTE: Uninline.
                pSettings->field_14.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                // NOTE: Uninline.
                pSettings->field_EE.SetResRef(CResRef(""), FALSE, TRUE, TRUE);

                pSettings->field_C = 92;
                pSettings->field_10 = 94;
                pSettings->field_1C8 = 1;
                pSettings->field_1D8 = 0;
                pSettings->field_1D0 = 0;

                pControl->SetToolTipStrRef(32186, -1, -1);
                pControl->SetToolTipHotKey(54, 0xFFFF, sKey);
                break;
            }
            }

            if (pSettings->field_0 != 0 && !pSettings->m_bGreyOut) {
                pSettings->m_bGreyOut = sub_5959A0(m_buttonTypes[nButton]) == 0;
            }
        }
    switch (m_nState) {
    case 101: {
        // 0x58EA6A
        if (m_pButtonList != NULL) {
            POSITION pos = m_pButtonList->GetHeadPosition();
            for (INT nIndex = 0; nIndex < 12; nIndex++) {
                pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nIndex + 6));
                if (pControl != NULL) {
                    pSettings = &(m_buttonArray[nIndex]);
                    sKey.Format("F%d", nIndex + 1);

                    if (pos != NULL) {
                        CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                        if (pButtonData != NULL) {
                            if (pButtonData->m_icon != "") {
                                pSettings->field_14.SetResRef(pButtonData->m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            } else {
                                pSettings->field_14.SetResRef(CResRef("STONWEAP"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            }

                            if (pButtonData->m_bDisplayCount) {
                                pSettings->field_1D8 = pButtonData->m_count;
                            }

                            if (pButtonData->m_abilityId.m_itemNum == pSprite->GetEquipment()->m_selectedWeapon
                                && pButtonData->m_abilityId.m_abilityNum == pSprite->GetEquipment()->m_selectedWeaponAbility) {
                                pSettings->field_1CC = 1;
                            } else {
                                pSettings->field_1CC = 0;
                            }

                            if (pButtonData->m_abilityId.field_10 == -1) {
                                pControl->SetToolTipStrRef(4950, -1, -1);
                            } else {
                                pControl->SetToolTipStrRef(pButtonData->m_abilityId.field_10, -1, -1);
                            }
                            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
                        }
                        pSettings->field_0 = 1;
                    } else {
                        pSettings->field_0 = 0;
                    }
                }
            }
        }
        break;
    }
    case 102:
    case 103: {
        // 0x58ED52
        if (m_pButtonList != NULL) {
            INT nFirst;
            INT nLast;
            if (m_pButtonList->GetCount() > 12) {
                nFirst = 1;
                nLast = 10;
            } else {
                nFirst = 0;
                nLast = 11;
            }

            POSITION pos = m_pButtonList->FindIndex(field_197A);
            for (INT nIndex = nFirst; nIndex <= nLast; nIndex++) {
                pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nIndex + 6));
                if (pControl != NULL) {
                    pSettings = &(m_buttonArray[nIndex]);
                    sKey.Format("F%d", nIndex + 1);

                    if (pos != NULL) {
                        CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                        if (pButtonData != NULL) {
                            if (pButtonData->m_icon != "") {
                                pSettings->field_14.SetResRef(pButtonData->m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            } else {
                                pSettings->field_14.SetResRef(CResRef("STONSPEL"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            }

                            if (pButtonData->m_bDisplayCount) {
                                pSettings->field_1D8 = pButtonData->m_count;
                            }

                            pSettings->m_bGreyOut = pButtonData->m_bDisabled;

                            if (pButtonData->m_name == -1) {
                                pControl->SetToolTipStrRef(4938, -1, -1);
                            } else {
                                pControl->SetToolTipStrRef(pButtonData->m_name, -1, pButtonData->m_abilityId.field_18);
                            }
                            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
                        }
                        pSettings->field_0 = 1;
                    } else {
                        pSettings->field_0 = 0;
                    }
                }
            }
        }
        break;
    }
    case 104:
    case 105: {
        // 0x58EED8
        if (m_pButtonList != NULL) {
            INT nFirst;
            INT nLast;
            if (m_pButtonList->GetCount() > 12) {
                nFirst = 1;
                nLast = 10;
            } else {
                nFirst = 0;
                nLast = 11;
            }

            POSITION pos = m_pButtonList->FindIndex(field_197A);
            for (INT nIndex = nFirst; nIndex <= nLast; nIndex++) {
                pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nIndex + 6));
                if (pControl != NULL) {
                    pSettings = &(m_buttonArray[nIndex]);
                    sKey.Format("F%d", nIndex + 1);

                    if (pos != NULL) {
                        CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                        if (pButtonData != NULL) {
                            if (pButtonData->m_icon != "") {
                                pSettings->field_14.SetResRef(pButtonData->m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            } else {
                                pSettings->field_14.SetResRef(CResRef("STONITEM"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            }

                            if (pButtonData->m_bDisplayCount) {
                                pSettings->field_1D8 = pButtonData->m_count;
                            }

                            if (pButtonData->m_abilityId.field_10 == -1) {
                                pControl->SetToolTipStrRef(4950, -1, -1);
                            } else {
                                pControl->SetToolTipStrRef(pButtonData->m_abilityId.field_10, -1, -1);
                            }
                            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
                        }
                        pSettings->field_0 = 1;
                    } else {
                        pSettings->field_0 = 0;
                    }
                }
            }
        }
        break;
    }
    case 106:
    case 107: {
        // 0x58F056
        if (m_pButtonList != NULL) {
            INT nFirst;
            INT nLast;
            if (m_pButtonList->GetCount() > 12) {
                nFirst = 1;
                nLast = 10;
            } else {
                nFirst = 0;
                nLast = 11;
            }

            POSITION pos = m_pButtonList->FindIndex(field_197A);
            for (INT nIndex = nFirst; nIndex <= nLast; nIndex++) {
                pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nIndex + 6));
                if (pControl != NULL) {
                    pSettings = &(m_buttonArray[nIndex]);
                    sKey.Format("F%d", nIndex + 1);

                    if (pos != NULL) {
                        CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                        if (pButtonData != NULL) {
                            if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN275) {
                                if (pSprite->sub_763150(CGAMESPRITE_FEAT_POWER_ATTACK)) {
                                    if (pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_1]
                                        || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_2]
                                        || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_3]
                                        || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_4]
                                        || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_5]) {
                                        pSettings->field_1CC = 1;
                                    }
                                    pSettings->m_bGreyOut = FALSE;
                                } else {
                                    pSettings->field_1CC = 0;
                                    pSettings->m_bGreyOut = TRUE;
                                }
                            } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN276) {
                                if (pSprite->sub_763150(CGAMESPRITE_FEAT_EXPERTISE)) {
                                    if (pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_1]
                                        || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_2]
                                        || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_3]
                                        || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_4]
                                        || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_5]) {
                                        pSettings->field_1CC = 1;
                                    }
                                    pSettings->m_bGreyOut = FALSE;
                                } else {
                                    pSettings->field_1CC = 0;
                                    pSettings->m_bGreyOut = TRUE;
                                }
                            } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN277) {
                                if (pSprite->sub_763150(CGAMESPRITE_FEAT_ARTERIAL_STRIKE)) {
                                    if (pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_ARTERIAL_STRIKE]) {
                                        pSettings->field_1CC = 1;
                                    }
                                    pSettings->m_bGreyOut = FALSE;
                                } else {
                                    pSettings->field_1CC = 0;
                                    pSettings->m_bGreyOut = TRUE;
                                }
                            } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN278) {
                                if (pSprite->sub_763150(CGAMESPRITE_FEAT_HAMSTRING)) {
                                    if (pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_HAMSTRING]) {
                                        pSettings->field_1CC = 1;
                                    }
                                    pSettings->m_bGreyOut = FALSE;
                                } else {
                                    pSettings->field_1CC = 0;
                                    pSettings->m_bGreyOut = TRUE;
                                }
                            } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN279) {
                                if (pSprite->sub_763150(CGAMESPRITE_FEAT_RAPID_SHOT)) {
                                    if (pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_RAPID_SHOT]) {
                                        pSettings->field_1CC = 1;
                                    }
                                    pSettings->m_bGreyOut = FALSE;
                                } else {
                                    pSettings->field_1CC = 0;
                                    pSettings->m_bGreyOut = TRUE;
                                }
                            }

                            if (pButtonData->m_icon != "") {
                                pSettings->field_14.SetResRef(pButtonData->m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            } else {
                                pSettings->field_14.SetResRef(CResRef("STONSPEC"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            }

                            if (pButtonData->m_bDisplayCount) {
                                pSettings->field_1D8 = pButtonData->m_count;
                            }

                            if (pButtonData->m_abilityId.field_10 == -1) {
                                pControl->SetToolTipStrRef(37435, -1, -1);
                            } else {
                                pControl->SetToolTipStrRef(pButtonData->m_abilityId.field_10, -1, -1);
                            }
                            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
                        }
                        pSettings->field_0 = 1;
                    } else {
                        pSettings->field_0 = 0;
                    }
                }
            }
        }
        break;
    }
    case 108: {
        // 0x58E7F8
        m_buttonArray[0].field_14.SetResRef(CResRef("FORM0"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[1].field_14.SetResRef(CResRef("FORM1"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[2].field_14.SetResRef(CResRef("FORM2"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[3].field_14.SetResRef(CResRef("FORM3"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[4].field_14.SetResRef(CResRef("FORM4"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[5].field_14.SetResRef(CResRef("FORM5"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[6].field_14.SetResRef(CResRef("FORM6"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[7].field_14.SetResRef(CResRef("FORM7"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[8].field_14.SetResRef(CResRef("FORM8"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[9].field_14.SetResRef(CResRef("FORM9"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[10].field_14.SetResRef(CResRef("FORMA"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[11].field_14.SetResRef(CResRef("FORMB"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        break;
    }
    case 109: {
        // 0x58E592
        m_buttonArray[0].field_14.SetResRef(CResRef("FORM0"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[1].field_14.SetResRef(CResRef("FORM1"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[2].field_14.SetResRef(CResRef("FORM2"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[3].field_14.SetResRef(CResRef("FORM3"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[4].field_14.SetResRef(CResRef("FORM4"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[5].field_14.SetResRef(CResRef("FORM5"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[6].field_14.SetResRef(CResRef("FORM6"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[7].field_14.SetResRef(CResRef("FORM7"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[8].field_14.SetResRef(CResRef("FORM8"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[9].field_14.SetResRef(CResRef("FORM9"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[10].field_14.SetResRef(CResRef("FORMA"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        m_buttonArray[11].field_14.SetResRef(CResRef("FORMB"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
        break;
    }
    case 112: {
        // 0x58F8E3
        if (m_pButtonList != NULL) {
            INT nFirst;
            INT nLast;
            if (m_pButtonList->GetCount() > 12) {
                nFirst = 1;
                nLast = 10;
            } else {
                nFirst = 0;
                nLast = 11;
            }

            POSITION pos = m_pButtonList->FindIndex(field_197A);
            for (INT nIndex = nFirst; nIndex <= nLast; nIndex++) {
                pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nIndex + 6));
                if (pControl != NULL) {
                    pSettings = &(m_buttonArray[nIndex]);
                    sKey.Format("F%d", nIndex + 1);

                    if (pos != NULL) {
                        CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                        if (pButtonData != NULL) {
                            if (pButtonData->m_icon != "") {
                                pSettings->field_14.SetResRef(pButtonData->m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            } else {
                                pSettings->field_14.SetResRef(CResRef("STONSPEL"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            }

                            if (pButtonData->m_bDisplayCount) {
                                pSettings->field_1D8 = pButtonData->m_count;
                            }

                            pSettings->m_bGreyOut = pButtonData->m_bDisabled;

                            if (pButtonData->m_abilityId.field_10 == -1) {
                                pControl->SetToolTipStrRef(4938, -1, -1);
                            } else {
                                pControl->SetToolTipStrRef(pButtonData->m_abilityId.field_10, -1, -1);
                            }
                            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
                        }
                        pSettings->field_0 = 1;
                    } else {
                        pSettings->field_0 = 0;
                    }
                }
            }
        }
        break;
    }
    case 113:
    case 122: {
        // 0x58F758
        if (m_pButtonList != NULL) {
            INT nFirst;
            INT nLast;
            if (m_pButtonList->GetCount() > 12) {
                nFirst = 1;
                nLast = 10;
            } else {
                nFirst = 0;
                nLast = 11;
            }

            POSITION pos = m_pButtonList->FindIndex(field_197A);
            for (INT nIndex = nFirst; nIndex <= nLast; nIndex++) {
                pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nIndex + 6));
                if (pControl != NULL) {
                    pSettings = &(m_buttonArray[nIndex]);
                    sKey.Format("F%d", nIndex + 1);

                    if (pos != NULL) {
                        CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                        if (pButtonData != NULL) {
                            if (pButtonData->m_icon != "") {
                                pSettings->field_14.SetResRef(pButtonData->m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            } else {
                                pSettings->field_14.SetResRef(CResRef("STONSONG"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            }

                            if (pButtonData->m_bDisplayCount) {
                                pSettings->field_1D8 = pButtonData->m_count;
                            }

                            pSettings->m_bGreyOut = pButtonData->m_bDisabled;

                            if (pButtonData->m_name == -1) {
                                pControl->SetToolTipStrRef(37436, -1, -1);
                            } else {
                                pControl->SetToolTipStrRef(pButtonData->m_name, -1, -1);
                            }
                            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
                        }
                        pSettings->field_0 = 1;
                    } else {
                        pSettings->field_0 = 0;
                    }
                }
            }
        }
        break;
    }
    case 115: {
        // 0x58EBEB
        if (m_pButtonList != NULL) {
            if (m_pButtonList->GetCount() <= 9) {
                POSITION pos = m_pButtonList->FindIndex(field_197A);
                for (INT nIndex = 5; nIndex < 12; nIndex++) {
                    pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nIndex + 6));
                    if (pControl != NULL) {
                        pSettings = &(m_buttonArray[nIndex]);
                        sKey.Format("F%d", nIndex + 1);

                        if (pos != NULL) {
                            CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                            if (pButtonData != NULL) {
                                if (pButtonData->m_icon != "") {
                                    pSettings->field_14.SetResRef(pButtonData->m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                                } else {
                                    pSettings->field_14.SetResRef(CResRef("STONSPEL"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                                }

                                if (pButtonData->m_bDisplayCount) {
                                    pSettings->field_1D8 = pButtonData->m_count;
                                }

                                pSettings->m_bGreyOut = pButtonData->m_bDisabled;

                                if (pButtonData->m_name == -1) {
                                    pControl->SetToolTipStrRef(37434, -1, -1);
                                } else {
                                    pControl->SetToolTipStrRef(pButtonData->m_name, -1, -1);
                                }
                                pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
                            }
                            pSettings->field_0 = 1;
                        } else {
                            pSettings->field_0 = 0;
                        }
                    }
                }
            }
        }
        break;
    }
    case 123: {
        // 0x58F4AF
        if (m_pButtonList != NULL) {
            INT nFirst;
            INT nLast;
            if (m_pButtonList->GetCount() > 12) {
                nFirst = 1;
                nLast = 10;
            } else {
                nFirst = 0;
                nLast = 11;
            }

            POSITION pos = m_pButtonList->FindIndex(field_197A);
            for (INT nIndex = nFirst; nIndex <= nLast; nIndex++) {
                pControl = static_cast<CUIControlButton*>(pPanel->GetControl(nIndex + 6));
                if (pControl != NULL) {
                    pSettings = &(m_buttonArray[nIndex]);
                    sKey.Format("F%d", nIndex + 1);

                    if (pos != NULL) {
                        CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                        if (pButtonData != NULL) {
                            if ((pButtonData->m_abilityId.m_res == CGameSprite::SPIN275
                                    && pSprite->sub_763150(CGAMESPRITE_FEAT_POWER_ATTACK)
                                    && pSprite->sub_726270(CGAMESPRITE_FEAT_POWER_ATTACK) > 0
                                    && pButtonData->m_count == pSprite->sub_726270(CGAMESPRITE_FEAT_POWER_ATTACK))
                                || (pButtonData->m_abilityId.m_res == CGameSprite::SPIN276
                                    && pSprite->sub_763150(CGAMESPRITE_FEAT_EXPERTISE)
                                    && pSprite->sub_726270(CGAMESPRITE_FEAT_EXPERTISE) > 0
                                    && pButtonData->m_count == pSprite->sub_726270(CGAMESPRITE_FEAT_EXPERTISE))
                                || (pButtonData->m_abilityId.m_res == CGameSprite::SPIN277
                                    && pSprite->sub_763150(CGAMESPRITE_FEAT_ARTERIAL_STRIKE)
                                    && pSprite->sub_726270(CGAMESPRITE_FEAT_ARTERIAL_STRIKE) > 0)
                                || (pButtonData->m_abilityId.m_res == CGameSprite::SPIN278
                                    && pSprite->sub_763150(CGAMESPRITE_FEAT_HAMSTRING)
                                    && pSprite->sub_726270(CGAMESPRITE_FEAT_HAMSTRING) > 0)
                                || (pButtonData->m_abilityId.m_res == CGameSprite::SPIN279
                                    && pSprite->sub_763150(CGAMESPRITE_FEAT_RAPID_SHOT)
                                    && pSprite->sub_726270(CGAMESPRITE_FEAT_RAPID_SHOT) > 0)) {
                                pSettings->field_1CC = 1;
                            }

                            if (pButtonData->m_icon != "") {
                                pSettings->field_14.SetResRef(pButtonData->m_icon, g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            } else {
                                pSettings->field_14.SetResRef(CResRef("STONSPEC"), g_pBaldurChitin->GetDoubleSize(), TRUE, TRUE);
                            }

                            if (pButtonData->m_bDisplayCount) {
                                pSettings->field_1D8 = pButtonData->m_count;
                            }

                            if (pButtonData->m_abilityId.field_10 == -1) {
                                pControl->SetToolTipStrRef(37435, -1, -1);
                            } else {
                                pControl->SetToolTipStrRef(pButtonData->m_abilityId.field_10, -1, pButtonData->m_abilityId.field_18);
                            }
                            pControl->SetToolTipHotKey(0xFFFF, 0xFFFF, sKey);
                        }
                        pSettings->field_0 = 1;
                    } else {
                        pSettings->field_0 = 0;
                    }
                }
            }
        }
        break;
    }
    }

    // 0x58FA67
    if (rc == CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    if (g_pBaldurChitin->GetScreenWorld()->GetActivePanelID() == -1) {
        pPanel->InvalidateRect(NULL);
    }

    return TRUE;
}

// 0x452C40
INT CInfButtonArray::GetState()
{
    return m_nState;
}

// 0x452C50
void CInfButtonArray::SetSelectedButton(INT nSelectedButton)
{
    m_nSelectedButton = nSelectedButton;
}

// 0x58FF20
BOOL CInfButtonArray::OnLButtonPressed(int buttonID)
{
    // NOTE: All of these are declared up front (without initializers) because
    // the function is full of labels that are jumped to across cases.
    CInfGame* pGame;
    CInfButtonSettings* pSettings;
    CGameSprite* pSprite;
    LONG nCharacterId;
    BYTE rc;
    BOOLEAN bAll;
    INT nNewStart;

    pGame = g_pBaldurChitin->GetObjectGame();

    // NOTE: Original checks `> 12`, not `>= 12`.
    if (buttonID < 0 || buttonID > 12) {
        return FALSE;
    }

    pSettings = &(m_buttonArray[buttonID]);

    if (pSettings->m_bGreyOut) {
        if (m_nState == 114) {
            return TRUE;
        }

        if (m_nState == 102) {
            if (m_buttonTypes[buttonID] < 21 || m_buttonTypes[buttonID] > 32) {
                return TRUE;
            }
        } else {
            if (!(m_buttonTypes[buttonID] >= 50 && m_buttonTypes[buttonID] <= 56)
                && !(m_buttonTypes[buttonID] >= 80 && m_buttonTypes[buttonID] <= 82)
                && m_buttonTypes[buttonID] != 100) {
                return TRUE;
            }
        }
    }

    // NOTE: Uninline.
    pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

    // NOTE: Uninline.
    pGame->SetLastClick(CPoint(-1, -1));

    // NOTE: Uninline.
    nCharacterId = pGame->m_group.GetGroupLeader();

    do {
        rc = pGame->m_cObjectArray.GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    switch (m_nState) {
    case 101: {
        BOOLEAN bFound = FALSE;

        if (m_pButtonList != NULL) {
            POSITION pos = m_pButtonList->GetHeadPosition();
            for (UINT nIndex = 0; nIndex < static_cast<UINT>(m_pButtonList->GetCount()); nIndex++) {
                if (pos != NULL) {
                    CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                    if (nIndex == static_cast<UINT>(buttonID) && pButtonData != NULL) {
                        bFound = TRUE;
                        SetQuickButtonData(*pButtonData, static_cast<BYTE>(field_1976), 1);
                        sub_588570(static_cast<BYTE>(field_1976), 1);
                        break;
                    }
                }
            }
        }

        UpdateButtons();

        // NOTE: Uninline.
        ClearList();

        if (m_stateStack.size() != 0) {
            INT nPrevState = m_stateStack.front();
            m_stateStack.pop_front();
            SetState(nPrevState, 0);
        }

        if (!bFound) {
            m_nSelectedButton = 100;
        }

        goto L_EXIT;
    }
    case 102:
    case 103:
    case 104:
    case 105:
    case 112:
    case 113:
    case 122: {
        BOOL bFlag = FALSE;
        if (m_nState == 103
            || m_nState == 105
            || m_nState == 122
            || m_nState == 112) {
            bFlag = TRUE;
        }

        BOOLEAN bResult = FALSE;

        if (m_pButtonList != NULL) {
            switch (m_buttonTypes[buttonID]) {
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32: {
                INT nIndex = m_pButtonList->GetCount() > 12 ? 1 : 0;

                POSITION pos = m_pButtonList->FindIndex(field_197A);
                while (pos != NULL) {
                    CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                    if (nIndex == buttonID && pButtonData != NULL) {
                        BOOL bDone = FALSE;

                        if (m_nState == 103 && !pSettings->m_bGreyOut) {
                            // NOTE: Uninline.
                            CSpell cSpell(pButtonData->m_abilityId.m_res);
                            cSpell.Demand();

                            BYTE nClass = pButtonData->m_abilityId.m_nClass;
                            DWORD nSpecialization = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetSpecializationMask(nClass,
                                static_cast<BYTE>(pButtonData->m_abilityId.field_1E));

                            cSpell.Release();

                            if (!pSprite->CanCast(nClass, nSpecialization, &cSpell)) {
                                pSprite->FeedBack(CGameSprite::FEEDBACK_89,
                                    0,
                                    0,
                                    0,
                                    g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetClassBeyondCastingAbilityStringRef(nClass),
                                    0,
                                    0);
                                bDone = TRUE;
                            } else if (g_pBaldurChitin->pActiveEngine->GetShiftKey() == TRUE
                                && bFlag == TRUE
                                && pButtonData->m_abilityId.m_nClass == 3
                                && pButtonData->m_abilityId.field_1E == 0
                                && pButtonData->m_bDisabled == FALSE) {
                                pSprite->sub_716770(*pButtonData);
                                bDone = TRUE;
                            }
                        }

                        if (!bDone) {
                            // 0x59163F
                            if (m_nState == 102) {
                                SetQuickButtonData(*pButtonData, static_cast<BYTE>(field_1976), 2);
                                m_customizableButtonTypes[field_1976] = field_1976 + 70;

                                // NOTE: Uninline.
                                pSprite->sub_594120(static_cast<BYTE>(field_1976), field_1976 + 70);

                                bResult = TRUE;
                            } else if (m_nState == 104) {
                                SetQuickButtonData(*pButtonData, static_cast<BYTE>(field_1976), 3);
                                m_customizableButtonTypes[field_1976] = field_1976 + 80;

                                // NOTE: Uninline.
                                pSprite->sub_594120(static_cast<BYTE>(field_1976), field_1976 + 80);

                                bResult = TRUE;
                            } else if (m_nState == 113) {
                                SetQuickButtonData(*pButtonData, static_cast<BYTE>(field_1976), 6);
                                m_customizableButtonTypes[field_1976] = field_1976 + 110;

                                // NOTE: Uninline.
                                pSprite->sub_594120(static_cast<BYTE>(field_1976), field_1976 + 110);

                                bResult = TRUE;
                            }

                            if (!pSettings->m_bGreyOut) {
                                if (m_nState == 102 || m_nState == 103) {
                                    bResult = static_cast<BOOLEAN>(sub_5886A0(*pButtonData, bFlag));
                                } else if (m_nState == 104 || m_nState == 105) {
                                    bResult = static_cast<BOOLEAN>(sub_5884B0(*pButtonData, bFlag));
                                } else if (m_nState == 113 || m_nState == 122) {
                                    bResult = static_cast<BOOLEAN>(sub_588820(*pButtonData, bFlag));
                                } else {
                                    bResult = static_cast<BOOLEAN>(sub_588760(*pButtonData, bFlag));
                                }
                            }
                        }

                        break;
                    }
                    nIndex++;
                }

                // NOTE: Uninline.
                ClearList();

                if (bResult) {
                    if (m_stateStack.size() != 0) {
                        // 0x592368
                        INT nPrevState = m_stateStack.back();
                        m_stateStack.clear();
                        SetState(nPrevState, 0);
                    }
                    goto L_EXIT;
                }

                if (m_stateStack.size() != 0) {
                    INT nPrevState = m_stateStack.front();
                    m_stateStack.pop_front();
                    SetState(nPrevState, 0);
                    m_nSelectedButton = 100;
                    UpdateButtons();
                    goto L_EXIT;
                }
                break;
            }
            case 33:
                if (field_197A <= 0) {
                    goto L_EXIT;
                }

                if ((m_nState == 103 || m_nState == 102)
                    && g_pBaldurChitin->pActiveEngine->GetShiftKey() == TRUE) {
                    field_197A = ScrollLeft(m_pButtonList);
                    UpdateButtons();
                    goto L_EXIT;
                }

                // 0x592896
                field_197A = max(field_197A - 10, 0);
                UpdateButtons();
                goto L_EXIT;
            case 34:
                if (field_197A >= m_pButtonList->GetCount() - 10) {
                    goto L_EXIT;
                }

                if ((m_nState == 103 || m_nState == 102)
                    && g_pBaldurChitin->pActiveEngine->GetShiftKey() == TRUE) {
                    field_197A = ScrollRight(m_pButtonList);
                    UpdateButtons();
                    goto L_EXIT;
                }

                if (m_pButtonList->GetCount() - 10 >= field_197A + 10) {
                    // 0x5928A3
                    field_197A = field_197A + 10;
                } else {
                    field_197A = m_pButtonList->GetCount() - 10;
                }
                UpdateButtons();
                goto L_EXIT;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
                // __LINE__: 4454
                UTIL_ASSERT(FALSE);
            }
        }

        // 0x5918DD
        m_nSelectedButton = 100;
        UpdateButtons();
        goto L_EXIT;
    }
    case 106:
    case 107: {
        // NOTE: No initializers - `L_592881_CASE_21` below is jumped to from
        // another case.
        BOOL bFlag;
        BOOLEAN bResult;

        bFlag = m_nState == 106;
        bResult = FALSE;

        if (m_pButtonList == NULL) {
            goto L_592E65;
        }

        switch (m_buttonTypes[buttonID]) {
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32: {
            BOOLEAN bFeat = FALSE;
            INT nIndex = m_pButtonList->GetCount() > 12 ? 1 : 0;

            POSITION pos = m_pButtonList->FindIndex(field_197A);
            while (pos != NULL) {
                CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                if (nIndex == buttonID && pButtonData != NULL) {
                    if (m_nState == 107) {
                        SetQuickButtonData(*pButtonData, static_cast<BYTE>(field_1976), 4);
                        m_customizableButtonTypes[field_1976] = field_1976 + 90;
                        pSprite->sub_594120(static_cast<BYTE>(field_1976), field_1976 + 90);
                    }

                    BOOL bHandled = FALSE;
                    if (m_nState == 106) {
                        UINT nFeatNumber = 0;
                        WORD nEffectId = 0;

                        if ((pButtonData->m_abilityId.m_res == CGameSprite::SPIN275
                                && pSprite->sub_763150(47))
                            || (pButtonData->m_abilityId.m_res == CGameSprite::SPIN276
                                && pSprite->sub_763150(19))) {
                            field_19E0 = pButtonData->m_abilityId.m_res;
                            bFeat = TRUE;
                            bResult = TRUE;
                            bHandled = TRUE;
                        } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN277) {
                            nFeatNumber = 5;
                            nEffectId = 455;
                        } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN278) {
                            nFeatNumber = 27;
                            nEffectId = 456;
                        } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN279) {
                            nFeatNumber = 49;
                            nEffectId = 457;
                        }

                        if (nEffectId != 0) {
                            pSprite->sub_726330(nFeatNumber, pSprite->sub_726270(nFeatNumber) > 0 ? 0 : 1);

                            ITEM_EFFECT effect;
                            CGameEffect::ClearItemEffect(&effect, nEffectId);

                            // NOTE: Original reads it from a constant at 0x8491AA.
                            effect.durationType = 1;

                            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                                pSprite->GetPos(),
                                pSprite->GetId(),
                                CPoint(-1, -1));

                            CMessageAddEffect* pMessage = new CMessageAddEffect(pEffect,
                                pSprite->GetId(),
                                pSprite->GetId());

                            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

                            bResult = TRUE;
                            bHandled = TRUE;
                        }
                    }

                    if (!bHandled) {
                        // 0x592801
                        if (m_nState == 107 || m_nState == 106) {
                            bResult = static_cast<BOOLEAN>(sub_5886A0(*pButtonData, bFlag));
                        }
                    }

                    break;
                }
                nIndex++;
            }

            ClearList();

            if (!bResult) {
                // 0x592AB3
                sub_589FF0(FALSE, FALSE);
                SetSelectedButton(100);
                UpdateButtons();
                goto L_EXIT;
            }

            if (bFeat == TRUE) {
                SetState(123, TRUE);
            } else {
                // 0x5939DD
                sub_589FF0(FALSE, TRUE);
            }
            goto L_EXIT;
        }
        case 33:
            if (field_197A <= 0) {
                goto L_EXIT;
            }

            // 0x592896
            field_197A = max(field_197A - 10, 0);
            UpdateButtons();
            goto L_EXIT;
        case 34:
            if (field_197A >= m_pButtonList->GetCount() - 10) {
                goto L_EXIT;
            }

            if (m_pButtonList->GetCount() - 10 < field_197A + 10) {
                // 0x5928A3
                field_197A = m_pButtonList->GetCount() - 10;
            } else {
                field_197A = field_197A + 10;
            }
            UpdateButtons();
            goto L_EXIT;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
            // __LINE__: 4827
            UTIL_ASSERT(FALSE);

            SetSelectedButton(100);
            UpdateButtons();
            goto L_EXIT;
        }
    }
    case 108:
        if (buttonID < 12) {
            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetQuickFormation(static_cast<SHORT>(field_1976), static_cast<SHORT>(buttonID));

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetFormationToQuickFormation(static_cast<SHORT>(field_1976));

            if (m_stateStack.size() != 0) {
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, 0);
            }
        }
        goto L_EXIT;
    case 109:
        if (buttonID < 12) {
            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetFormation(static_cast<SHORT>(buttonID));

            if (m_stateStack.size() != 0) {
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, 0);
            }
        }
        goto L_EXIT;
    case 110:
        switch (m_buttonTypes[buttonID]) {
        case 6:
            SetState(109, 0);
            m_nSelectedButton = m_buttonTypes[buttonID];
            UpdateButtons();
            break;
        case 7:
            if (g_pBaldurChitin->GetObjectGame()->m_nState != 3) {
                g_pBaldurChitin->GetObjectGame()->m_nState = 3;
                m_nSelectedButton = 7;
                UpdateButtons();
            } else {
                // 0x590110
                g_pBaldurChitin->GetObjectGame()->m_nState = 0;
                m_nSelectedButton = 100;
                UpdateButtons();
            }
            break;
        case 8:
            if (g_pBaldurChitin->GetObjectGame()->m_nState != 2
                || g_pBaldurChitin->GetObjectGame()->m_iconIndex != 12) {
                g_pBaldurChitin->GetObjectGame()->m_nState = 2;
                g_pBaldurChitin->GetObjectGame()->m_iconIndex = 12;
                g_pBaldurChitin->GetObjectGame()->m_iconResRef = "";
                m_nSelectedButton = 8;
                UpdateButtons();
            } else {
                // 0x590110
                g_pBaldurChitin->GetObjectGame()->m_nState = 0;
                m_nSelectedButton = 100;
                UpdateButtons();
            }
            break;
        case 15:
            g_pBaldurChitin->GetObjectGame()->m_nState = 0;
            m_nSelectedButton = 100;
            g_pBaldurChitin->GetObjectGame()->m_group.ClearActions();
            UpdateButtons();
            break;
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetFormationToQuickFormation(static_cast<SHORT>(m_buttonTypes[buttonID] - 16));

            SetState(110, 0);
            UpdateButtons();
            break;
        default:
            // 0x5928A9
            UpdateButtons();
            break;
        }
        goto L_EXIT;
    default:
        // 0x592C6F
        switch (m_buttonTypes[buttonID]) {
        case 2: {
            // 0x59336B
            if (pSprite->GetModalState() == 1) {
                pSprite->SetModalState(0, 0);
                SetSelectedButton(100);
                UpdateButtons();
            } else {
                g_pBaldurChitin->GetObjectGame()->SetState(0);
                UpdateButtons();
                SetState(122, TRUE);
            }
            break;
        }
        case 3: {
            // 0x5932A5
            sub_594280(3, pSprite);
            break;
        }
        case 4: {
            // 0x59309E
            if (pSprite->GetModalState() == 2) {
                pSprite->SetModalState(0, 0);
                goto L_5939B2;
            }

            pSprite->FeedBack(CGameSprite::FEEDBACK_SEARCHSTART, 0, 0, 0, -1, 0, 0);
            pSprite->SetModalState(2, 0);
            SetSelectedButton(5);
            goto L_5939BB;
        }
        case 5: {
            // 0x592ED9
            sub_594280(5, pSprite);
            break;
        }
        case 7: {
            // 0x592C9C
            if (g_pBaldurChitin->GetObjectGame()->GetState() == 3) {
                g_pBaldurChitin->GetObjectGame()->SetState(0);
                SetSelectedButton(100);
            } else {
                g_pBaldurChitin->GetObjectGame()->SetState(3);
                SetSelectedButton(7);
            }
            pSprite->SetModalState(0, 0);
            UpdateButtons();
            break;
        }
        case 8: {
            // 0x592D02
            if (g_pBaldurChitin->GetObjectGame()->GetState() == 2
                && g_pBaldurChitin->GetObjectGame()->GetIconIndex() == 12) {
                g_pBaldurChitin->GetObjectGame()->SetState(0);
                SetSelectedButton(100);
            } else {
                g_pBaldurChitin->GetObjectGame()->SetState(2);
                g_pBaldurChitin->GetObjectGame()->SetIconIndex(12);
                SetSelectedButton(8);
            }
            pSprite->SetModalState(0, 0);
            UpdateButtons();
            break;
        }
        case 10: {
            // 0x593358
            sub_594280(10, pSprite);
            break;
        }
        case 11: {
            // 0x592EEC
            if (pSprite->GetModalState() == 3) {
                pSprite->SetModalState(0, 0);
                pSprite->SetStealthGreyOut(CGameSprite::STEALTH_GREYOUT_DURATION);
                SetSelectedButton(100);

                ITEM_EFFECT effect;
                CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);

                // NOTE: Original reads it from a constant at 0x8491AA.
                effect.durationType = 1;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    pSprite->GetPos(),
                    pSprite->GetId(),
                    CPoint(-1, -1));

                CMessageAddEffect* pMessage = new CMessageAddEffect(pEffect,
                    pSprite->GetId(),
                    pSprite->GetId());

                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            } else {
                pSprite->SetModalState(3, 0);
                SetSelectedButton(5);

                // NOTE: Action id is read from an unnamed `const SHORT` at
                // 0x8477A6 (18).
                CAIAction action(18, CPoint(-1, -1), 0, -1);

                CMessageAddAction* pMessage = new CMessageAddAction(action,
                    pSprite->GetId(),
                    pSprite->GetId());

                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
            goto L_5939BB;
        }
        case 12: {
            // 0x5930E2
            if (g_pBaldurChitin->GetObjectGame()->GetState() == 2
                && (g_pBaldurChitin->GetObjectGame()->GetIconIndex() == 36
                    || g_pBaldurChitin->GetObjectGame()->GetIconIndex() == 40)) {
                g_pBaldurChitin->GetObjectGame()->SetState(0);
                SetSelectedButton(100);
            } else {
                g_pBaldurChitin->GetObjectGame()->SetState(2);
                g_pBaldurChitin->GetObjectGame()->SetIconIndex(36);
                SetSelectedButton(12);
            }
            pSprite->SetModalState(0, 0);
            goto L_592B14_DEFAULT;
        }
        case 13: {
            // 0x59325A
            if (static_cast<signed char>(pSprite->GetDerivedStats()->m_nSkills[1]) > 0) {
                CButtonData buttonData;
                pSprite->sub_718390(CGameSprite::SPIN108, 0, 0, buttonData);
                sub_5886A0(buttonData, TRUE);
            }
            goto L_592B14_DEFAULT;
        }
        case 14: {
            // 0x593345
            sub_594280(14, pSprite);
            break;
        }
        case 50:
            // 0x5932B8
            m_nCurrentSelectedSpellClass = 2;
            goto L_5932BF;
        case 51:
            // 0x5932F4
            m_nCurrentSelectedSpellClass = 3;
            goto L_5932BF;
        case 52:
            // 0x593315
            m_nCurrentSelectedSpellClass = 4;
            goto L_5932BF;
        case 53:
            // 0x59331E
            m_nCurrentSelectedSpellClass = 7;
            goto L_5932BF;
        case 54:
            // 0x593327
            m_nCurrentSelectedSpellClass = 8;
            goto L_5932BF;
        case 55:
            // 0x593330
            m_nCurrentSelectedSpellClass = 10;
            goto L_5932BF;
        case 56:
            // 0x593339
            m_nCurrentSelectedSpellClass = 11;
        L_5932BF:
            field_19DC = 0;
        L_5932C9:
            g_pBaldurChitin->GetObjectGame()->SetState(0);
            UpdateButtons();
            SetState(103, TRUE);
            break;
        case 57:
            // 0x5932FD
            m_nCurrentSelectedSpellClass = 3;
            field_19DC = pSprite->sub_5940D0();
            goto L_5932C9;
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67: {
            // 0x592D92
            if (g_pBaldurChitin->GetActiveEngine()->GetShiftKey() == TRUE) {
                g_pBaldurChitin->GetObjectGame()->SetState(0);
                SetSelectedButton(100);
                field_1976 = m_buttonTypes[buttonID] - 60;
                UpdateButtons();
                SetState(101, TRUE);
                break;
            }

            INT nType = m_buttonTypes[buttonID];
            if (nType == 61 || nType == 63 || nType == 65 || nType == 67) {
                // NOTE: Second slot of a weapon set (shield / off-hand). When
                // there is nothing usable in it, the click goes to the first
                // slot of the set.
                CItem* pItem = pSprite->GetEquipment()->m_items[m_buttonTypes[buttonID] - 17];
                if (pItem == NULL
                    || pItem->GetItemType() == 47
                    || pItem->GetItemType() == 53
                    || pItem->GetItemType() == 49
                    || pItem->GetItemType() == 41) {
                    nType--;
                }
            }

            if (m_nSelectedButton == nType) {
                goto L_592E59;
            }

            g_pBaldurChitin->GetObjectGame()->SetState(0);
            pSprite->SetModalState(0, 0);
            SetSelectedButton(nType);
            sub_588570(static_cast<BYTE>(nType - 60), 1);

            if (g_pBaldurChitin->GetObjectGame()->GetState() != 0) {
                // NOTE: Jumps to shared code at 0x5928A9.
                UpdateButtons();
                break;
            }

            SetSelectedButton(100);
            UpdateButtons();
            break;
        }
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78: {
            // 0x5933C3
            if (m_nSelectedButton == m_buttonTypes[buttonID]) {
                goto L_592E59;
            }

            CButtonData* pButtonData = new CButtonData();
            GetQuickButtonData(static_cast<BYTE>(m_buttonTypes[buttonID] - 70), *pButtonData, 2);

            CSpell cSpell(pButtonData->m_abilityId.m_res);
            cSpell.Demand();

            BYTE nClass = pButtonData->m_abilityId.m_nClass;
            DWORD nSpecialization = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetSpecializationMask(nClass,
                static_cast<BYTE>(pButtonData->m_abilityId.field_1E));

            cSpell.Release();

            if (!pSprite->CanCast(nClass, nSpecialization, &cSpell)) {
                // FIXME: Leaks `pButtonData`.
                pSprite->FeedBack(CGameSprite::FEEDBACK_89,
                    0,
                    0,
                    0,
                    g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetClassBeyondCastingAbilityStringRef(nClass),
                    0,
                    0);
            } else {
                if (m_nState == 114
                    && pButtonData->m_abilityId.m_nClass == 3
                    && pButtonData->m_abilityId.field_1E == 0
                    && g_pBaldurChitin->GetActiveEngine()->GetShiftKey() == TRUE
                    && !pButtonData->m_bDisabled) {
                    pSprite->sub_716770(*pButtonData);
                } else {
                    g_pBaldurChitin->GetObjectGame()->SetState(0);
                    SetSelectedButton(m_buttonTypes[buttonID]);
                    pSprite->SetModalState(0, 0);
                    UpdateButtons();

                    if (pSettings->field_1D8 > 0) {
                        sub_588570(static_cast<BYTE>(m_buttonTypes[buttonID] - 70), 2);
                    }

                    if (g_pBaldurChitin->GetObjectGame()->GetState() == 0) {
                        SetSelectedButton(100);
                    }
                }

                delete pButtonData;
            }
            break;
        }
        case 80:
        case 81:
        case 82: {
            // 0x5935A3
            sub_594280(m_buttonTypes[buttonID], pSprite);
            break;
        }
        case 90:
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
        case 98: {
            // 0x5935B5
            if (m_nSelectedButton == m_buttonTypes[buttonID]) {
                goto L_592E59;
            }

            g_pBaldurChitin->GetObjectGame()->SetState(0);
            SetSelectedButton(m_buttonTypes[buttonID]);
            pSprite->SetModalState(0, 0);

            CButtonData* pButtonData = new CButtonData();
            GetQuickButtonData(static_cast<BYTE>(m_buttonTypes[buttonID] - 90), *pButtonData, 4);

            if ((pButtonData->m_abilityId.m_res == CGameSprite::SPIN275 && pSprite->sub_763150(47))
                || (pButtonData->m_abilityId.m_res == CGameSprite::SPIN276 && pSprite->sub_763150(19))) {
                field_19E0 = pButtonData->m_abilityId.m_res;
                SetState(123, TRUE);
                delete pButtonData;
                break;
            }

            BOOL bUse = pSettings->field_1D8 > 0;

            UINT nFeatNumber;
            WORD nEffectId;
            BOOL bToggle = TRUE;
            if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN277) {
                nFeatNumber = 5;
                nEffectId = ICEWIND_CGAMEEFFECT_FEATARTERIALSTRIKE;
            } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN278) {
                nFeatNumber = 27;
                nEffectId = ICEWIND_CGAMEEFFECT_FEATHAMSTRING;
            } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN279) {
                nFeatNumber = 49;
                nEffectId = ICEWIND_CGAMEEFFECT_FEATRAPIDSHOT;
            } else {
                bToggle = FALSE;
            }

            // NOTE: Three identical inlined copies in the original, one per
            // feat (0x5936CF, 0x593772, 0x593838).
            if (bToggle) {
                pSprite->sub_726330(nFeatNumber, pSprite->sub_726270(nFeatNumber) > 0 ? 0 : 1);

                ITEM_EFFECT effect;
                CGameEffect::ClearItemEffect(&effect, nEffectId);

                // NOTE: Original reads it from a constant at 0x8491AA.
                effect.durationType = 1;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    pSprite->GetPos(),
                    pSprite->GetId(),
                    CPoint(-1, -1));

                CMessageAddEffect* pMessage = new CMessageAddEffect(pEffect,
                    pSprite->GetId(),
                    pSprite->GetId());

                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }

            if (bUse == TRUE) {
                sub_588570(static_cast<BYTE>(m_buttonTypes[buttonID] - 90), 4);
            }

            if (g_pBaldurChitin->GetObjectGame()->GetState() == 0) {
                SetSelectedButton(100);
            }

            delete pButtonData;
            break;
        }
        case 110:
        case 111:
        case 112:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118: {
            // 0x59393C
            if (pSprite->GetModalState() == 1) {
                pSprite->SetModalState(0, 0);
                goto L_5939B2;
            }

            CButtonData* pButtonData = new CButtonData();
            GetQuickButtonData(static_cast<BYTE>(m_buttonTypes[buttonID] - 110), *pButtonData, 6);
            sub_588820(*pButtonData, TRUE);
            delete pButtonData;
            goto L_5939BB;
        }
        case 119: {
            // 0x593181
            ITEM_EFFECT effect;
            CGameEffect::ClearItemEffect(&effect, ICEWIND_CGAMEEFFECT_RANGERTRACKING);
            effect.targetType = 1;

            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                CPoint(-1, -1),
                -1,
                CPoint(-1, -1));
            pEffect->SetSource(pSprite->GetPos());
            pEffect->SetSourceId(pSprite->GetId());
            pEffect->sub_594020(FALSE);

            CMessageAddEffect* pMessage = new CMessageAddEffect(pEffect,
                pSprite->GetId(),
                pSprite->GetId());

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            goto L_5939B2;
        }
        }
        break;

    // NOTE: Shared tails of the `default` case above. Also jumped to from other
    // states (`L_592B14_DEFAULT`, `L_5939DD`, `L_592E65`). They sit directly in
    // the outer switch so no initialization is bypassed.
    L_5939B2:
        // 0x5939B2
        SetSelectedButton(100);
    L_5939BB:
        // 0x5939BB
        g_pBaldurChitin->GetObjectGame()->SetState(0);
    L_592B14_DEFAULT:
        // 0x5939CF
        UpdateButtons();
        ClearList();
        bAll = FALSE;
        // 0x5939DD
        sub_589FF0(FALSE, bAll);
        break;

    L_592E59:
        // 0x592E59
        g_pBaldurChitin->GetObjectGame()->SetState(0);
    L_592E65:
        // 0x592E65
        SetSelectedButton(100);
        UpdateButtons();
        break;
    case 0x73: {
        switch (m_buttonTypes[buttonID]) {
        case 4: {
            if (pSprite->m_nModalState == 2) {
                pSprite->SetModalState(0, FALSE);
                m_nSelectedButton = 100;
            } else {
                pSprite->FeedBack(CGameSprite::FEEDBACK_SEARCHSTART, 0, 0, 0, -1, 0, 0);
                pSprite->SetModalState(2, FALSE);
                m_nSelectedButton = 5;
            }

            g_pBaldurChitin->GetObjectGame()->m_nState = 0;
            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                // NOTE: Inlined `std::deque` `front()` and `pop_front()`
                // (0x596250).
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, FALSE);
            }
            break;
        }
        case 11: {
            if (pSprite->m_nModalState == 3) {
                pSprite->SetModalState(0, FALSE);
                pSprite->m_nStealthGreyOut = CGameSprite::STEALTH_GREYOUT_DURATION;
                m_nSelectedButton = 100;

                ITEM_EFFECT effect;
                CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);

                // NOTE: Original reads it from a constant at 0x8491AA.
                effect.durationType = 1;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    pSprite->GetPos(),
                    pSprite->m_id,
                    CPoint(-1, -1));

                CMessage* message = new CMessageAddEffect(pEffect, pSprite->m_id, pSprite->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            } else {
                pSprite->SetModalState(3, FALSE);
                m_nSelectedButton = 5;

                // NOTE: Action id is read from an unnamed `const SHORT` at
                // 0x8477A6 (18). The action is built inline (specificID = 0,
                // dest = (-1, -1), specificID2 = -1, specificID3 = 0).
                CAIAction action(18, CPoint(-1, -1), 0, -1);

                CMessage* message = new CMessageAddAction(action, pSprite->m_id, pSprite->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            g_pBaldurChitin->GetObjectGame()->m_nState = 0;
            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                // NOTE: Inlined `std::deque` `front()` and `pop_front()`
                // (0x596250).
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, FALSE);
            }
            break;
        }
        case 12: {
            CInfGame* pObjectGame = g_pBaldurChitin->GetObjectGame();
            if (pObjectGame->m_nState == 2
                && (pObjectGame->m_iconIndex == 36 || pObjectGame->m_iconIndex == 40)) {
                pObjectGame->m_nState = 0;
                m_nSelectedButton = 100;
            } else {
                // NOTE: Inlined `CInfGame::SetState` and
                // `CInfGame::SetIconIndex`.
                pObjectGame->m_nState = 2;
                g_pBaldurChitin->GetObjectGame()->m_iconIndex = 36;
                g_pBaldurChitin->GetObjectGame()->m_iconResRef = "";
                m_nSelectedButton = 12;
            }

            pSprite->SetModalState(0, FALSE);
            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                // NOTE: Inlined `std::deque` `front()` and `pop_front()`
                // (0x596250).
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, FALSE);
            }
            break;
        }
        case 13: {
            if (static_cast<char>(pSprite->m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_ANIMAL_EMPATHY]) > 0) {
                CButtonData buttonData;
                pSprite->sub_718390(CGameSprite::SPIN108, 0, 0, buttonData);
                sub_5886A0(buttonData, TRUE);
            }

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                // NOTE: Inlined `std::deque` `front()` and `pop_front()`
                // (0x596250).
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, FALSE);
            }
            break;
        }
        case 119: {
            ITEM_EFFECT effect;
            CGameEffect::ClearItemEffect(&effect, 286);
            effect.targetType = 1;

            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                CPoint(-1, -1),
                -1,
                CPoint(-1, -1));

            // NOTE: Inlined `CGameEffect::SetSource` and
            // `CGameEffect::SetSourceId`.
            pEffect->m_source = pSprite->GetPos();
            pEffect->m_sourceID = pSprite->m_id;
            pEffect->m_flags &= ~0x1;

            CMessage* message = new CMessageAddEffect(pEffect, pSprite->m_id, pSprite->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            m_nSelectedButton = 100;
            g_pBaldurChitin->GetObjectGame()->m_nState = 0;
            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                // NOTE: Inlined `std::deque` `front()` and `pop_front()`
                // (0x596250).
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, FALSE);
            }
            break;
        }
        default: {
            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                // NOTE: Inlined `std::deque` `front()` and `pop_front()`
                // (0x596250).
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, FALSE);
            }
            break;
        }
        }
        goto L_EXIT;
    }
    case 0x74: {
        // NOTE: The five non-default cases are separate identical copies in
        // the original.
        switch (m_buttonTypes[buttonID]) {
        case 4:
        case 11:
        case 12:
        case 13:
        case 119: {
            m_customizableButtonTypes[field_1976] = m_buttonTypes[buttonID];

            BYTE buttonNum = static_cast<BYTE>(field_1976);

            // NOTE: Inlined `CGameSprite` custom button setter.
            // __FILE__: .\Include\ObjCreature.h
            // __LINE__: 2036
            UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_CUSTOM_BUTTONS22);

            pSprite->field_3D14[buttonNum] = m_buttonTypes[buttonID];

            g_pBaldurChitin->GetObjectGame()->m_nState = 0;
            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        }
        default: {
            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                // NOTE: Inlined `std::deque` `front()` and `pop_front()`
                // (0x596250).
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, FALSE);
            }
            goto L_EXIT;
        }
        }

        {
            // NOTE: Inlined `std::deque` code: reads `*(end() - 1)`
            // (0x596600), then `clear()` (0x5962E0).
            INT nPrevState = m_stateStack.back();
            m_stateStack.clear();
            SetState(nPrevState, FALSE);
        }
        goto L_EXIT;
    }
    case 0x75: {
        switch (m_buttonTypes[buttonID]) {
        case 35:
            m_customizableButtonTypes[field_1976] = 5;
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 5);
            break;
        case 36:
            m_customizableButtonTypes[field_1976] = 3;
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 3);
            break;
        case 37:
            m_customizableButtonTypes[field_1976] = 14;
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 14);
            break;
        case 38:
            g_pBaldurChitin->GetObjectGame()->SetState(0);
            UpdateButtons();
            SetState(0x78, TRUE);
            goto L_EXIT;
        case 39:
            m_customizableButtonTypes[field_1976] = 10;
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 10);
            break;
        case 40:
            m_customizableButtonTypes[field_1976] = 2;
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 2);
            break;
        case 41:
            m_customizableButtonTypes[field_1976] = 100;
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 100);
            break;
        case 42: {
            UINT nIndex;
            for (nIndex = 0; nIndex < 9; nIndex++) {
                pSprite->sub_594120(static_cast<BYTE>(nIndex), 0);
            }

            pSprite->ResetQuickSlots();

            for (nIndex = 0; nIndex < 9; nIndex++) {
                m_customizableButtonTypes[nIndex] = pSprite->sub_5940E0(static_cast<BYTE>(nIndex));
            }
            break;
        }
        }

        // NOTE: Same code as the shared tail at 0x5939CF (`L_592B14_DEFAULT`
        // in the default state handler), duplicated here to avoid a `goto`.
        UpdateButtons();
        ClearList();
        sub_589FF0(FALSE, FALSE);
        goto L_EXIT;
    }
    case 0x76: {
        switch (m_buttonTypes[buttonID]) {
        case 50:
            m_nCurrentSelectedSpellClass = 2;
            field_19DC = 0;
            break;
        case 51:
            field_19DC = 0;
            m_nCurrentSelectedSpellClass = 3;
            break;
        case 52:
            m_nCurrentSelectedSpellClass = 4;
            field_19DC = 0;
            break;
        case 53:
            m_nCurrentSelectedSpellClass = 7;
            field_19DC = 0;
            break;
        case 54:
            m_nCurrentSelectedSpellClass = 8;
            field_19DC = 0;
            break;
        case 55:
            m_nCurrentSelectedSpellClass = 10;
            field_19DC = 0;
            break;
        case 56:
            m_nCurrentSelectedSpellClass = 11;
            field_19DC = 0;
            break;
        case 57:
            m_nCurrentSelectedSpellClass = 3;
            field_19DC = pSprite->m_baseStats.m_specialization;
            break;
        default: {
            m_nCurrentSelectedSpellClass = 0;
            field_19DC = 0;

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                // NOTE: Inlined `std::deque` `front()` and `pop_front()`
                // (0x596250).
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, FALSE);
            }
            goto L_EXIT;
        }
        }

        g_pBaldurChitin->GetObjectGame()->m_nState = 0;
        UpdateButtons();
        SetState(0x67, TRUE);
        goto L_EXIT;
    }
    case 0x77:
        switch (m_buttonTypes[buttonID]) {
        case 0x32:
            m_customizableButtonTypes[field_1976] = 0x32;

            // NOTE: Uninline.
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 0x32);

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetState(0);

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        case 0x33:
            m_customizableButtonTypes[field_1976] = 0x33;

            // NOTE: Uninline.
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 0x33);

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetState(0);

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        case 0x34:
            m_customizableButtonTypes[field_1976] = 0x34;

            // NOTE: Uninline.
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 0x34);

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetState(0);

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        case 0x35:
            m_customizableButtonTypes[field_1976] = 0x35;

            // NOTE: Uninline.
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 0x35);

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetState(0);

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        case 0x36:
            m_customizableButtonTypes[field_1976] = 0x36;

            // NOTE: Uninline.
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 0x36);

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetState(0);

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        case 0x37:
            m_customizableButtonTypes[field_1976] = 0x37;

            // NOTE: Uninline.
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 0x37);

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetState(0);

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        case 0x38:
            m_customizableButtonTypes[field_1976] = 0x38;

            // NOTE: Uninline.
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 0x38);

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetState(0);

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        case 0x39:
            m_customizableButtonTypes[field_1976] = 0x39;

            // NOTE: Uninline.
            pSprite->sub_594120(static_cast<BYTE>(field_1976), 0x39);

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->SetState(0);

            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() == 0) {
                goto L_EXIT;
            }
            break;
        default:
            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, 0);
            }
            goto L_EXIT;
        }

        // NOTE: Return to the oldest state, dropping everything in between.
        {
            INT nPrevState = m_stateStack.back();
            m_stateStack.clear();
            SetState(nPrevState, 0);
        }
        break;
    case 0x78:
        if (m_buttonTypes[buttonID] < 0x50 || m_buttonTypes[buttonID] > 0x52) {
            UpdateButtons();

            // NOTE: Uninline.
            ClearList();

            if (m_stateStack.size() != 0) {
                INT nPrevState = m_stateStack.front();
                m_stateStack.pop_front();
                SetState(nPrevState, 0);
            }
            break;
        }

        m_customizableButtonTypes[field_1976] = m_buttonTypes[buttonID];

        // NOTE: Uninline.
        pSprite->sub_594120(static_cast<BYTE>(field_1976), m_buttonTypes[buttonID]);

        UpdateButtons();

        // NOTE: Uninline.
        ClearList();

        if (m_stateStack.size() == 0) {
            break;
        }

    // NOTE: Also the target of a jump from the 0x73-0x76 states (0x591871).
        {
            INT nPrevState = m_stateStack.back();
            m_stateStack.clear();
            SetState(nPrevState, 0);
        }
        break;
    case 0x79:
        if (m_buttonTypes[buttonID] >= 0x3C && m_buttonTypes[buttonID] <= 0x43) {
            pSprite->sub_726810(static_cast<BYTE>((m_buttonTypes[buttonID] - 0x3C) / 2));

            // NOTE: Uninline.
            UpdateSelectedWeaponSet(pSprite->m_nWeaponSet);
        }

        UpdateButtons();

        // NOTE: Uninline.
        ClearList();

        if (m_stateStack.size() != 0) {
            INT nPrevState = m_stateStack.front();
            m_stateStack.pop_front();
            SetState(nPrevState, 0);
        }
        break;
    case 0x7B:
        if (m_pButtonList == NULL) {
            goto L_592E65;
        }

        switch (m_buttonTypes[buttonID]) {
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20:
            {
                INT nIndex = m_pButtonList->GetCount() > 12 ? 1 : 0;
                POSITION pos = m_pButtonList->FindIndex(field_197A);
                while (pos != NULL) {
                    CButtonData* pButtonData = m_pButtonList->GetNext(pos);
                    if (nIndex == buttonID && pButtonData != NULL) {
                        if (m_nState == 0x7B) {
                            WORD effectId = 0;

                            if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN275
                                && pSprite->sub_763150(0x2F)) {
                                effectId = 0x1C5;
                                pSprite->sub_726330(0x2F, pButtonData->m_count);
                            } else if (pButtonData->m_abilityId.m_res == CGameSprite::SPIN276
                                && pSprite->sub_763150(0x13)) {
                                effectId = 0x1C6;
                                pSprite->sub_726330(0x13, pButtonData->m_count);
                            }

                            if (pButtonData->m_count == 0) {
                                field_19E0 = CResRef();
                            }

                            ITEM_EFFECT effect;
                            CGameEffect::ClearItemEffect(&effect, effectId);
                            // NOTE: Read from an unnamed `const WORD` at 0x8491AA.
                            effect.durationType = 1;

                            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                                pSprite->GetPos(),
                                pSprite->GetId(),
                                CPoint(-1, -1));

                            CMessage* message = new CMessageAddEffect(pEffect,
                                pSprite->GetId(),
                                pSprite->GetId());
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                        }
                        break;
                    }
                    nIndex++;
                }
            }

            ClearList();

        // NOTE: Also the target of a jump from the 0x6A / 0x6B states (0x5924F7).
            sub_589FF0(0, 0);
            SetSelectedButton(100);
            UpdateButtons();
            break;
        case 0x21:
        // NOTE: The 0x6A / 0x6B states share this case (their jump table at
        // 0x593C34 points here).
            if (field_197A <= 0) {
                goto L_EXIT;
            }

        // NOTE: Jumped to from 0x59143A (with `field_197A` in EAX).
            nNewStart = field_197A - 10;
            if (nNewStart < 0) {
                nNewStart = 0;
            }

        // NOTE: Jumped to from 0x5914A8 and 0x59248B with the new value in
        // `nNewStart` (EAX).
            field_197A = nNewStart;
            UpdateButtons();
            break;
        case 0x22:
            if (field_197A < m_pButtonList->GetCount() - 10) {
                // NOTE: Not the hoisted `nNewStart`, the original keeps this
                // one in EDI.
                INT nStart = field_197A + 10;
                if (m_pButtonList->GetCount() - 10 < nStart) {
                    field_197A = m_pButtonList->GetCount() - 10;
                    UpdateButtons();
                } else {
                    field_197A = nStart;
                    UpdateButtons();
                }
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
            // __LINE__: 4952
            UTIL_ASSERT(FALSE);

            SetSelectedButton(100);
            UpdateButtons();
            break;
        }
        break;
    }

L_EXIT:
    if (rc == CGameObjectArray::SUCCESS) {
        pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    return TRUE;
}

// 0x594170
BOOL CInfButtonArray::sub_594170(INT buttonType)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (m_nState != 114) {
        return FALSE;
    }

    // NOTE: Uninline.
    LONG nLeaderId = pGame->m_group.GetGroupLeader();

    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = pGame->m_cObjectArray.GetDeny(nLeaderId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    switch (buttonType) {
    case 3:
    case 5:
    case 10:
    case 14:
    case 80:
    case 81:
    case 82:
        sub_594280(buttonType, pSprite);
        break;
    }

    pGame->m_cObjectArray.ReleaseDeny(nLeaderId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// 0x594280
void CInfButtonArray::sub_594280(INT buttonType, CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
    // __LINE__: 5711
    UTIL_ASSERT(pSprite != NULL);

    switch (buttonType) {
    case 3: {
        g_pBaldurChitin->GetObjectGame()->m_nState = 0;
        UpdateButtons();

        BYTE nClass = 0;
        UINT nCount = 0;
        UINT nClassIndex;
        UINT nLevel;

        for (nClassIndex = 0; nClassIndex < CSPELLLIST_NUM_CLASSES; nClassIndex++) {
            if (nCount > 1) {
                break;
            }

            nLevel = 0;
            while (1) {
                // __FILE__: .\Include\ObjCreature.h
                // __LINE__: 1741
                UTIL_ASSERT(nClassIndex <= CSPELLLIST_NUM_CLASSES);

                if (nLevel >= pSprite->m_spells.Get(nClassIndex)->m_nHighestLevel) {
                    break;
                }

                // NOTE: Uninline.
                if (pSprite->m_spells.GetSpellsAtLevel(nClassIndex, nLevel)->field_10 > 0) {
                    nClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nClassIndex);
                    nCount++;
                    break;
                }

                nLevel++;
            }
        }

        for (nLevel = 0; nLevel < pSprite->m_domainSpells.m_nHighestLevel; nLevel++) {
            // __FILE__: .\Include\ObjCreature.h
            // __LINE__: 1763
            UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

            // NOTE: Uninline.
            if (pSprite->m_domainSpells.GetSpellsAtLevel(nLevel)->field_10 > 0) {
                if (nCount == 0) {
                    nClass = CAIOBJECTTYPE_C_CLERIC;
                    field_19DC = pSprite->m_baseStats.m_specialization;
                }
                nCount++;
                break;
            }

            field_19DC = 0;
        }

        if (nCount > 1) {
            SetState(118, TRUE);
        } else if (nCount == 1) {
            m_nCurrentSelectedSpellClass = nClass;
            SetState(103, TRUE);
        }
        break;
    }
    case 5:
        if (pSprite->m_nModalState == 0) {
            g_pBaldurChitin->GetObjectGame()->m_nState = 0;
            UpdateButtons();
            SetState(115, TRUE);
        } else {
            if (pSprite->m_nModalState == 3) {
                pSprite->SetModalState(0, FALSE);
                pSprite->m_nStealthGreyOut = CGameSprite::STEALTH_GREYOUT_DURATION;
            } else {
                pSprite->SetModalState(0, FALSE);
            }

            m_nSelectedButton = 100;
            UpdateButtons();
        }
        break;
    case 10:
        g_pBaldurChitin->GetObjectGame()->m_nState = 0;
        UpdateButtons();
        SetState(106, TRUE);
        break;
    case 14:
        g_pBaldurChitin->GetObjectGame()->m_nState = 0;
        m_nSelectedButton = 14;
        pSprite->SetModalState(0, FALSE);
        UpdateButtons();
        SetState(105, TRUE);
        break;
    case 80:
    case 81:
    case 82: {
        CGameButtonList* pButtonList = pSprite->GetItemUsages(static_cast<SHORT>(buttonType - 65), 3, -1);

        if (pButtonList != NULL && pButtonList->GetCount() > 1) {
            m_nSelectedButton = buttonType;
            field_1976 = buttonType - 80;
            SetState(104, TRUE);
        } else {
            g_pBaldurChitin->GetObjectGame()->m_nState = 0;

            if (m_nSelectedButton == buttonType) {
                m_nSelectedButton = 100;
                UpdateButtons();
            } else {
                m_nSelectedButton = buttonType;
                pSprite->SetModalState(0, FALSE);
                sub_588570(static_cast<BYTE>(buttonType - 80), 3);

                if (g_pBaldurChitin->GetObjectGame()->m_nState == 0) {
                    m_nSelectedButton = 100;
                }
            }
        }

        if (pButtonList != NULL) {
            while (pButtonList->GetCount() != 0) {
                delete pButtonList->RemoveHead();
            }
            pButtonList->RemoveAll();
            delete pButtonList;
        }
        break;
    }
    }
}

// 0x594720
BOOL CInfButtonArray::OnRButtonPressed(int buttonID)
{
    CGameSprite* pSprite;

    // NOTE: Upper bound is inclusive in the original.
    if (buttonID < 0 || buttonID > 12) {
        return FALSE;
    }

    // NOTE: Original code is slightly different (range checks are inverted
    // and can never be true), so greyed out buttons are never rejected here.
    if (m_buttonArray[buttonID].m_bGreyOut) {
        INT buttonType = m_buttonTypes[buttonID];
        if ((buttonType < 70 && buttonType > 78)
            || (buttonType < 80 && buttonType > 82)
            || (buttonType < 90 && buttonType > 98)
            || (buttonType < 110 && buttonType > 118)) {
            return TRUE;
        }
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // NOTE: Uninline.
    LONG nCharacterId = pGame->GetGroup()->GetGroupLeader();

    // FIXME: Result is not checked, `pSprite` is used unconditionally.
    pGame->GetObjectArray()->GetShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pSprite),
        INFINITE);

    switch (m_nState) {
    case 110:
        if (m_buttonTypes[buttonID] >= 16 && m_buttonTypes[buttonID] <= 20) {
            field_1976 = m_buttonTypes[buttonID] - 16;
            SetState(108, TRUE);
        }
        break;
    case 114:
        switch (m_buttonTypes[buttonID]) {
        case 2:
        case 3:
        case 4:
        case 5:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 80:
        case 81:
        case 82:
        case 90:
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
        case 98:
        case 100:
        case 110:
        case 111:
        case 112:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 119:
            field_1976 = buttonID - 3;
            SetState(117, TRUE);
            break;
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
            if (pSprite->sub_737910(FALSE) == TRUE) {
                if (pSprite->m_equipment.m_items[42] != NULL) {
                    pSprite->FeedBack(CGameSprite::FEEDBACK_MAGICALWEAPONINUSE, 0, 0, 0, -1, 0, 0);
                } else {
                    field_1976 = m_buttonTypes[buttonID] - 60;
                    SetState(121, TRUE);
                }
            } else {
                pSprite->FeedBack(CGameSprite::FEEDBACK_ITEMCURSED, 0, 0, 0, -1, 0, 0);
            }
            break;
        }
        break;
    case 117:
        switch (m_buttonTypes[buttonID]) {
        case 35:
            SetState(116, TRUE);
            break;
        case 36: {
            BYTE nClass = 0;
            UINT nCount = 0;
            UINT nClassIndex;
            UINT nLevel;

            for (nClassIndex = 0; nClassIndex < CSPELLLIST_NUM_CLASSES; nClassIndex++) {
                if (nCount > 1) {
                    break;
                }

                nLevel = 0;
                while (1) {
                    // __FILE__: .\Include\ObjCreature.h
                    // __LINE__: 1741
                    UTIL_ASSERT(nClassIndex <= CSPELLLIST_NUM_CLASSES);

                    if (nLevel >= pSprite->m_spells.Get(nClassIndex)->m_nHighestLevel) {
                        break;
                    }

                    // NOTE: Uninline.
                    if (pSprite->m_spells.GetSpellsAtLevel(nClassIndex, nLevel)->m_List.size() != 0) {
                        nClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nClassIndex);
                        nCount++;
                        break;
                    }

                    nLevel++;
                }
            }

            for (nLevel = 0; nLevel < pSprite->m_domainSpells.m_nHighestLevel; nLevel++) {
                // __FILE__: .\Include\ObjCreature.h
                // __LINE__: 1763
                UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

                // NOTE: Uninline.
                if (pSprite->m_domainSpells.GetSpellsAtLevel(nLevel)->m_List.size() != 0) {
                    if (nCount == 0) {
                        nClass = CAIOBJECTTYPE_C_CLERIC;
                        field_19DC = pSprite->m_baseStats.m_specialization;
                    }
                    nCount++;
                    break;
                }

                field_19DC = 0;
            }

            if (nCount > 1) {
                SetState(119, TRUE);
            } else if (nCount == 1) {
                if (pSprite->GetSpells(nClass)->sub_725F00() > 0) {
                    m_nCurrentSelectedSpellClass = nClass;
                    SetState(102, TRUE);
                }
            }
            break;
        }
        case 38:
            SetState(120, TRUE);
            break;
        case 39:
            if (pSprite->m_innateSpells.field_10 > 0) {
                SetState(107, TRUE);
            }
            break;
        case 40:
            if (pSprite->m_songs.m_List.size() != 0) {
                SetState(113, TRUE);
            }
            break;
        }
        break;
    case 119: {
        BOOLEAN bFound = FALSE;
        UINT nLevel;

        // NOTE: Every case in the original leaves without changing state
        // when there are no spell levels at all, which is the same as falling
        // out of the loop with `bFound` unset.
        switch (m_buttonTypes[buttonID]) {
        case 50:
            m_nCurrentSelectedSpellClass = CAIOBJECTTYPE_C_BARD;
            field_19DC = 0;
            for (nLevel = 0; nLevel < pSprite->GetSpells(CAIOBJECTTYPE_C_BARD)->m_nHighestLevel && !bFound; nLevel++) {
                bFound = pSprite->GetSpellsAtLevel(CAIOBJECTTYPE_C_BARD, nLevel)->field_18 > 0;
            }
            break;
        case 51:
            m_nCurrentSelectedSpellClass = CAIOBJECTTYPE_C_CLERIC;
            field_19DC = 0;
            for (nLevel = 0; nLevel < pSprite->GetSpells(CAIOBJECTTYPE_C_CLERIC)->m_nHighestLevel && !bFound; nLevel++) {
                bFound = pSprite->GetSpellsAtLevel(CAIOBJECTTYPE_C_CLERIC, nLevel)->field_10 > 0;
            }
            break;
        case 52:
            m_nCurrentSelectedSpellClass = CAIOBJECTTYPE_C_DRUID;
            field_19DC = 0;
            for (nLevel = 0; nLevel < pSprite->GetSpells(CAIOBJECTTYPE_C_DRUID)->m_nHighestLevel && !bFound; nLevel++) {
                bFound = pSprite->GetSpellsAtLevel(CAIOBJECTTYPE_C_DRUID, nLevel)->field_10 > 0;
            }
            break;
        case 53:
            m_nCurrentSelectedSpellClass = CAIOBJECTTYPE_C_PALADIN;
            field_19DC = 0;
            for (nLevel = 0; nLevel < pSprite->GetSpells(CAIOBJECTTYPE_C_PALADIN)->m_nHighestLevel && !bFound; nLevel++) {
                bFound = pSprite->GetSpellsAtLevel(CAIOBJECTTYPE_C_PALADIN, nLevel)->field_10 > 0;
            }
            break;
        case 54:
            m_nCurrentSelectedSpellClass = CAIOBJECTTYPE_C_RANGER;
            field_19DC = 0;
            for (nLevel = 0; nLevel < pSprite->GetSpells(CAIOBJECTTYPE_C_RANGER)->m_nHighestLevel && !bFound; nLevel++) {
                bFound = pSprite->GetSpellsAtLevel(CAIOBJECTTYPE_C_RANGER, nLevel)->field_10 > 0;
            }
            break;
        case 55:
            m_nCurrentSelectedSpellClass = CAIOBJECTTYPE_C_SORCERER;
            field_19DC = 0;
            for (nLevel = 0; nLevel < pSprite->GetSpells(CAIOBJECTTYPE_C_SORCERER)->m_nHighestLevel && !bFound; nLevel++) {
                bFound = pSprite->GetSpellsAtLevel(CAIOBJECTTYPE_C_SORCERER, nLevel)->field_18 > 0;
            }
            break;
        case 56:
            m_nCurrentSelectedSpellClass = CAIOBJECTTYPE_C_WIZARD;
            field_19DC = 0;
            for (nLevel = 0; nLevel < pSprite->GetSpells(CAIOBJECTTYPE_C_WIZARD)->m_nHighestLevel && !bFound; nLevel++) {
                bFound = pSprite->GetSpellsAtLevel(CAIOBJECTTYPE_C_WIZARD, nLevel)->field_10 > 0;
            }
            break;
        case 57:
            m_nCurrentSelectedSpellClass = CAIOBJECTTYPE_C_CLERIC;
            field_19DC = pSprite->m_baseStats.m_specialization;
            for (nLevel = 0; nLevel < pSprite->m_domainSpells.m_nHighestLevel && !bFound; nLevel++) {
                // __FILE__: .\Include\FileFormat.h
                // __LINE__: 2521
                UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

                bFound = pSprite->m_domainSpells.m_lists[nLevel].field_10 > 0;
            }
            break;
        }

        if (bFound == TRUE) {
            SetState(102, TRUE);
        }
        break;
    }
    }

    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// 0x5950F0
BOOL CInfButtonArray::PostRender(const CPoint& pos, const CRect& rClip, BOOL bPressed, INT nButton)
{
    CVidCell vcHighlight;
    CResRef cResRef;
    DWORD dwFlags = 0;

    CSize size(CIcon::ICON_SIZE_SM.cx * (g_pBaldurChitin->field_4A2C ? 2 : 1),
        CIcon::ICON_SIZE_SM.cy * (g_pBaldurChitin->field_4A2C ? 2 : 1));

    if (nButton >= 0 && nButton < 12) {
        CInfButtonSettings* pSettings = &(m_buttonArray[nButton]);
        DWORD dwHighlightFlags = pSettings->m_bGreyOut ? 0x80000 : 0;

        if (g_pBaldurChitin->pActiveEngine == NULL
            || g_pBaldurChitin->pActiveEngine->pVidMode == NULL) {
            return FALSE;
        }

        CPoint pt((g_pBaldurChitin->field_4A2C ? 2 : 1) * 3 + pos.x,
            (g_pBaldurChitin->field_4A2C ? 2 : 1) * 3 + pos.y);

        if (bPressed) {
            pt.x += (g_pBaldurChitin->field_4A2C ? 2 : 1) * 2;
            pt.y += (g_pBaldurChitin->field_4A2C ? 2 : 1) * 2;
        }

        CRect rIcon(pt.x, pt.y, pt.x + size.cx, pt.y + size.cy);

        CRect rIconClip;
        rIconClip.IntersectRect(rIcon, rClip);

        if (pSettings->field_0) {
            if (!pSettings->field_4) {
                return FALSE;
            }

            if (pSettings->field_1D0 && !pSettings->field_1CC) {
                // NOTE: Uninline.
                vcHighlight.SetResRef(CResRef("HIGHLGHT"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);

                vcHighlight.Render(0, pt.x, pt.y, rIconClip, NULL, 0, dwHighlightFlags, -1);
            }

            if (pSettings->m_bGreyOut) {
                // 0x84CEB8
                dwFlags = 0x2;
            }

            CIcon::RenderIcon(0,
                pt,
                size,
                rIconClip,
                pSettings->field_EE.cResRef,
                g_pBaldurChitin->field_4A2C,
                dwFlags,
                0,
                FALSE,
                0,
                FALSE,
                0);

            switch (m_buttonTypes[nButton]) {
            case 70:
            case 71:
            case 72:
            case 73:
            case 74:
            case 75:
            case 76:
            case 77:
            case 78:
            case 90:
            case 91:
            case 92:
            case 93:
            case 94:
            case 95:
            case 96:
            case 97:
            case 98:
                CIcon::RenderIcon(0,
                    pt,
                    size,
                    rIconClip,
                    pSettings->field_14.cResRef,
                    g_pBaldurChitin->field_4A2C,
                    dwFlags,
                    static_cast<WORD>(pSettings->field_1D8),
                    pSettings->field_14.cResRef == "STONSPEL" || pSettings->field_14.cResRef == "STONSPEC",
                    0,
                    FALSE,
                    0);
                break;
            case 80:
            case 81:
            case 82: {
                CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
                BOOL bUnusable = FALSE;

                // NOTE: Uninline.
                LONG nCharacterId = pGame->GetCharacterId(static_cast<SHORT>(g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter()));

                CGameSprite* pSprite = NULL;

                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                INT nUsable = 1;
                if (rc == CGameObjectArray::SUCCESS) {
                    if (pSprite != NULL) {
                        CItem* pItem = pSprite->GetQuickItem(static_cast<BYTE>(m_buttonTypes[nButton] - 80));
                        if (pItem != NULL) {
                            nUsable = g_pBaldurChitin->GetObjectGame()->CheckItemUsable(pSprite, pItem);
                            bUnusable = nUsable == 2;
                        }
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }

                COLORREF rgbTint = 0;
                if (bUnusable == TRUE) {
                    // 0x84EBB0
                    rgbTint = RGB(255, 255, 0);
                }

                CIcon::RenderIcon(0,
                    pt,
                    size,
                    rIconClip,
                    pSettings->field_14.cResRef,
                    g_pBaldurChitin->field_4A2C,
                    dwFlags,
                    static_cast<WORD>(pSettings->field_1D8),
                    FALSE,
                    0,
                    FALSE,
                    rgbTint);

                if (nUsable == 2) {
                    CVidCell vcTint(CResRef("STORTIN4"), g_pBaldurChitin->m_pEngineWorld->GetManager()->m_bDoubleSize);
                    vcTint.Render(0, pt.x, pt.y, rIconClip, NULL, 0, 0x2, 192);
                } else if (nUsable == 0) {
                    CVidCell vcTint(CResRef("STORTINT"), g_pBaldurChitin->m_pEngineWorld->GetManager()->m_bDoubleSize);
                    vcTint.Render(0, pt.x, pt.y, rIconClip, NULL, 0, 0x2, 192);
                }
                break;
            }
            default:
                CIcon::RenderIcon(0,
                    pt,
                    size,
                    rIconClip,
                    pSettings->field_14.cResRef,
                    g_pBaldurChitin->field_4A2C,
                    dwFlags,
                    static_cast<WORD>(pSettings->field_1D8),
                    FALSE,
                    0,
                    FALSE,
                    0);
                break;
            }
        }
    }

    return TRUE;
}

// 0x5957C0
BOOL CInfButtonArray::PreRender(const CPoint& pos, const CRect& rClip, BOOL bPressed, INT nButton)
{
    if (nButton < 0 || nButton >= 12) {
        return TRUE;
    }

    CInfButtonSettings* pSettings = &(m_buttonArray[nButton]);

    DWORD dwFlags;
    if (pSettings->m_bGreyOut) {
        // 0x84C924
        field_17C2.SetTintColor(RGB(180, 180, 180));
        field_16E8.SetTintColor(RGB(180, 180, 180));
        dwFlags = 0xA0000;
    } else {
        dwFlags = 0;
    }

    if (g_pBaldurChitin->pActiveEngine == NULL
        || g_pBaldurChitin->pActiveEngine->pVidMode == NULL) {
        return FALSE;
    }

    INT nSize = (g_pBaldurChitin->field_4A2C ? 2 : 1) * 38;
    CRect rButton(pos.x, pos.y, pos.x + nSize, pos.y + nSize);

    CRect rButtonClip;
    rButtonClip.IntersectRect(rButton, rClip);

    if (!pSettings->field_0) {
        return TRUE;
    }

    if (pSettings->field_8 && pSettings->field_C >= 0) {
        field_17C2.SequenceSet(0);

        INT nFrame = pSettings->field_1CC ? pSettings->field_10 : pSettings->field_C;
        if (bPressed) {
            nFrame++;
        }
        field_17C2.FrameSet(static_cast<SHORT>(nFrame));

        field_17C2.Render(0, pos.x, pos.y, rButtonClip, NULL, 0, dwFlags, -1);
        return TRUE;
    }

    if (!pSettings->field_1CC) {
        return FALSE;
    }

    field_16E8.SequenceSet(0);
    field_16E8.FrameSet(static_cast<SHORT>(bPressed ? 2 * nButton + 25 : 2 * nButton + 24));
    field_16E8.Render(0, pos.x, pos.y, rButtonClip, NULL, 0, dwFlags, -1);

    return TRUE;
}

// 0x5959A0
BOOL CInfButtonArray::sub_5959A0(INT buttonType)
{
    SHORT nIndex;

    switch (buttonType) {
    case 2:
        nIndex = 12;
        break;
    case 3:
        nIndex = 2;
        break;
    case 11:
        nIndex = 0;
        break;
    case 12:
        nIndex = 1;
        break;
    case 70:
        nIndex = 3;
        break;
    case 71:
        nIndex = 4;
        break;
    case 72:
        nIndex = 5;
        break;
    case 73:
        nIndex = 6;
        break;
    case 74:
        nIndex = 7;
        break;
    case 75:
        nIndex = 8;
        break;
    case 76:
        nIndex = 9;
        break;
    case 77:
        nIndex = 10;
        break;
    case 78:
        nIndex = 11;
        break;
    case 90:
        nIndex = 31;
        break;
    case 91:
        nIndex = 32;
        break;
    case 92:
        nIndex = 33;
        break;
    case 93:
        nIndex = 34;
        break;
    case 94:
        nIndex = 35;
        break;
    case 95:
        nIndex = 36;
        break;
    case 96:
        nIndex = 37;
        break;
    case 97:
        nIndex = 38;
        break;
    case 98:
        nIndex = 39;
        break;
    case 110:
        nIndex = 13;
        break;
    case 111:
        nIndex = 14;
        break;
    case 112:
        nIndex = 15;
        break;
    case 113:
        nIndex = 16;
        break;
    case 114:
        nIndex = 17;
        break;
    case 115:
        nIndex = 18;
        break;
    case 116:
        nIndex = 19;
        break;
    case 117:
        nIndex = 20;
        break;
    case 118:
        nIndex = 21;
        break;
    default:
        return TRUE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
        // NOTE: Uninline.
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupLeader();

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            // NOTE: Uninline.
            BOOL bDisabled = pSprite->GetActiveStats()->m_disabledButtons[nIndex];

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            return bDisabled == FALSE;
        }
    }

    return TRUE;
}

// 0x595C80
BOOL CInfButtonArray::sub_595C80()
{
    BOOL bResult = FALSE;

    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return FALSE;
    }

    LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
    LONG nCharacterId = groupList[0];
    delete groupList;

    CGameSprite* pSprite;

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

    if (pSprite->GetAIType().IsClassValid(CAIObjectType::C_BARD)) {
        bResult = TRUE;
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return bResult;
}

// FIXME: `nClass` should not be reference.
//
// 0x595D60
BOOL CInfButtonArray::sub_595D60(const BYTE& nClass)
{
    BOOL bResult = FALSE;

    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return FALSE;
    }

    LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
    LONG nCharacterId = groupList[0];
    delete groupList;

    CGameSprite* pSprite;

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

    switch (nClass) {
    case CAIOBJECTTYPE_C_BARD:
    case CAIOBJECTTYPE_C_CLERIC:
    case CAIOBJECTTYPE_C_DRUID:
    case CAIOBJECTTYPE_C_PALADIN:
    case CAIOBJECTTYPE_C_RANGER:
    case CAIOBJECTTYPE_C_SORCERER:
    case CAIOBJECTTYPE_C_WIZARD:
        bResult = pSprite->GetSpells(nClass)->m_nHighestLevel > 0;
        break;
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return bResult;
}

// 0x595E70
void CInfButtonArray::UpdateCustomizableButtonTypes(INT* pButtonList)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
    // __LINE__: 7128
    UTIL_ASSERT(pButtonList != NULL);

    memcpy(m_customizableButtonTypes, pButtonList, sizeof(m_customizableButtonTypes));
}

// 0x595EB0
BOOLEAN CInfButtonArray::sub_595EB0(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
    // __LINE__: 7136
    UTIL_ASSERT(pSprite != NULL);

    if (pSprite->sub_726270(CGAMESPRITE_FEAT_POWER_ATTACK) > 0
        && pSprite->sub_763150(CGAMESPRITE_FEAT_POWER_ATTACK)) {
        return TRUE;
    }

    if (pSprite->sub_726270(CGAMESPRITE_FEAT_EXPERTISE) > 0
        && pSprite->sub_763150(CGAMESPRITE_FEAT_EXPERTISE)) {
        return TRUE;
    }

    if (pSprite->sub_726270(CGAMESPRITE_FEAT_ARTERIAL_STRIKE) > 0
        && pSprite->sub_763150(CGAMESPRITE_FEAT_ARTERIAL_STRIKE)) {
        return TRUE;
    }

    if (pSprite->sub_726270(CGAMESPRITE_FEAT_HAMSTRING) > 0
        && pSprite->sub_763150(CGAMESPRITE_FEAT_HAMSTRING)) {
        return TRUE;
    }

    if (pSprite->sub_726270(CGAMESPRITE_FEAT_RAPID_SHOT) > 0
        && pSprite->sub_763150(CGAMESPRITE_FEAT_RAPID_SHOT)) {
        return TRUE;
    }

    return FALSE;
}

// 0x595F70
void CInfButtonArray::UpdateSelectedWeaponSet(BYTE nIndex)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
    // __LINE__: 7154
    UTIL_ASSERT(nIndex < CCREATUREFILEEQUIPMENT21_WEAPON_NUM_SETS);

    m_nSelectedWeaponSet = nIndex;
}

// 0x595FB0
INT CInfButtonArray::ScrollRight(CGameButtonList* pButtonList)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
    // __LINE__: 7162
    UTIL_ASSERT(pButtonList != NULL);

    INT nCount = 0;
    INT nGroup = 0;

    // NOTE: Uses global list instead of `pButtonList`.
    POSITION pos = m_pButtonList->FindIndex(field_197A);
    while (pos != NULL) {
        CButtonData* pButtonData = m_pButtonList->GetNext(pos);
        if (pButtonData != NULL) {
            if (nCount == 0) {
                nGroup = pButtonData->m_abilityId.field_1D;
            } else if (pButtonData->m_abilityId.field_1D != nGroup) {
                return field_197A + nCount;
            }
            nCount++;
        }
    }

    return 0;
}

// 0x596040
INT CInfButtonArray::ScrollLeft(CGameButtonList* pButtonList)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfButtonArray.cpp
    // __LINE__: 7203
    UTIL_ASSERT(pButtonList != NULL);

    INT nCount = 0;
    INT nGroup = 0;
    INT nPrevGroup = -1;

    // NOTE: Uses global list instead of `pButtonList`.
    POSITION pos = m_pButtonList->FindIndex(field_197A);
    while (pos != NULL) {
        CButtonData* pButtonData = m_pButtonList->GetPrev(pos);
        if (pButtonData != NULL) {
            if (nCount == 0) {
                nGroup = pButtonData->m_abilityId.field_1D;
            } else if (nPrevGroup == -1 && pButtonData->m_abilityId.field_1D != nGroup) {
                nPrevGroup = pButtonData->m_abilityId.field_1D;
            } else if (pButtonData->m_abilityId.field_1D != nPrevGroup) {
                INT nIndex = field_197A - nCount + 1;
                return nIndex >= 0 ? nIndex : 0;
            }
            nCount++;
        }
    }

    return 0;
}

// 0x5960E0
void CInfButtonArray::UpdateTempSelectedWeaponSet()
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
        LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
        LONG nCharacterId = groupList[0];
        delete groupList;

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            m_nSelectedWeaponSet = pSprite->m_nWeaponSet;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}
