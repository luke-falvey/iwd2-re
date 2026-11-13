#include "CVariableHash.h"

#include "CBaldurChitin.h"
#include "CCreatureFile.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CPathSearch.h"
#include "CScreenInventory.h"
#include "CScreenWorld.h"
#include "CSpell.h"
#include "CUtil.h"

// 0x84EC4C
int CVariableHashBase::PRIME[] = {
    0x02,
    0x03,
    0x05,
    0x07,
    0x0B,
    0x0D,
    0x11,
    0x13,
    0x17,
    0x1D,
    0x1F,
    0x25,
    0x29,
    0x2B,
    0x2F,
    0x35,
    0x3B,
    0x3D,
    0x43,
    0x47,
    0x49,
    0x4F,
    0x53,
    0x59,
    0x5B,
    0x5D,
    0x61,
    0x65,
    0x67,
    0x6B,
    0x71,
    0x7F,
    0x83,
    0x89,
};

// -----------------------------------------------------------------------------

// 0x452BD0
CVariable::CVariable()
{
}

// 0x5505A0
CVariable::CVariable(const CVariable& other)
{
    strncpy(m_name, other.m_name, 32);
    m_type = other.m_type;
    m_resRefType = other.m_resRefType;
    m_dwValue = other.m_dwValue;
    m_intValue = other.m_intValue;
    m_floatValue = other.m_floatValue;
    strncpy(m_stringValue, other.m_stringValue, 32);
}

// 0x4EDA30
CString CVariable::GetName()
{
    return CString(m_name);
}

// FIXME: Use reference.
//
// 0x452C60
void CVariable::SetName(CString sName)
{
    strncpy(m_name, sName, 32);
}

// 0x45B600
int CVariable::GetIntValue()
{
    return m_intValue;
}

// 0x427860
void CVariable::SetIntValue(LONG nValue)
{
    m_intValue = nValue;
}

// NOTE: Inlined.
CVariable& CVariable::operator=(const CVariable& other)
{
    if (this != &other) {
        strncpy(m_name, other.m_name, 32);
        m_type = other.m_type;
        m_resRefType = other.m_resRefType;
        m_dwValue = other.m_dwValue;
        m_intValue = other.m_intValue;
        m_floatValue = other.m_floatValue;
        strncpy(m_stringValue, other.m_stringValue, 32);
    }
    return *this;
}

// -----------------------------------------------------------------------------

// FIXME: Use reference.
//
// 0x54FAD0
int CVariableHashBase::Hash(CString sName)
{
    int length = sName.GetLength();
    int v1 = 0;
    int v3 = 1;

    for (int index = 0; index < length && index < 32; index++) {
        int v4 = PRIME[index] * (static_cast<BYTE>(sName[index]) & 0x2F);
        v1 += v4;
        v3 *= v4 & (m_nTableEntries - 1);
    }

    return (m_nTableEntries - 1) & (v1 + v3);
}

// -----------------------------------------------------------------------------

// 0x54FB40
CVariableHash::CVariableHash(LONG nSize)
{
    m_hashEntries = new CVariable[nSize];
    m_nTableEntries = nSize;
}

// 0x54FC10
CVariableHash::~CVariableHash()
{
    delete m_hashEntries;
}

// 0x54FC30
BOOL CVariableHash::AddKey(CVariable& var)
{
    if (var.GetName().GetLength() == 0) {
        return FALSE;
    }

    CString sName = var.GetName();
    CString sTemp;

    for (int index = 0; index < sName.GetLength(); index++) {
        if (sName[index] != ' ') {
            sTemp += sName[index];
        }
    }

    sName = sTemp;
    sName = sName.SpanExcluding(" ");
    sName.MakeUpper();
    var.SetName(sName);

    int end = Hash(sName);
    int curr = end;

    BOOL v1 = g_pBaldurChitin->GetObjectGame()->field_1B84;
    BOOL bDone = FALSE;
    BOOL bResult = FALSE;
    while (!bDone) {
        // NOTE: Compares the low byte of the slot index only.
        if (v1 && (curr & 0xFF) == 0xFF) {
            SleepEx(10, FALSE);
        }

        if (m_hashEntries[curr].m_name[0] == '\0'
            || memcmp(m_hashEntries[curr].m_name, var.m_name, 32) == 0) {
            bResult = TRUE;

            m_hashEntries[curr] = var;

            break;
        }

        curr++;
        if (curr >= m_nTableEntries) {
            curr = 0;
        }

        if (curr == end) {
            bDone = TRUE;
            Resize(2 * m_nTableEntries);
            AddKey(var);
        }
    }

    return bResult;
}

// FIXME: Use reference.
//
// 0x54FE90
CVariable* CVariableHash::FindKey(CString sVarName)
{
    CString sCopy(sVarName);
    CString sTemp;

    for (int index = 0; index < sCopy.GetLength(); index++) {
        if (sCopy[index] != ' ') {
            sTemp += sCopy[index];
        }
    }

    sCopy = sTemp;
    sCopy = sCopy.SpanExcluding(" ");
    sCopy.MakeUpper();

    int end = Hash(sCopy);
    int curr = end;

    BOOL bDone = FALSE;
    CVariable* var = NULL;
    while (!bDone) {
        // FIXME: Unnecessary copying.
        if (m_hashEntries[curr].GetName() == sCopy) {
            var = &(m_hashEntries[curr]);
            break;
        }

        // FIXME: Unnecessary copying.
        if (m_hashEntries[curr].GetName() == "") {
            break;
        }

        curr++;
        if (curr >= m_nTableEntries) {
            curr = 0;
        }

        if (curr == end) {
            bDone = TRUE;
        }
    }

    return var;
}

// FIXME: Use reference.
//
// 0x550070
CVariable* CVariableHash::FindKey(CString sVarName, LONG nValue)
{
    CString sCopy(sVarName);
    CString sTemp;

    for (int index = 0; index < sCopy.GetLength(); index++) {
        if (sCopy[index] != ' ') {
            sTemp += sCopy[index];
        }
    }

    sCopy = sTemp;
    sCopy = sCopy.SpanExcluding(" ");
    sCopy.MakeUpper();

    int end = Hash(sCopy);
    int curr = end;

    BOOL bDone = FALSE;
    CVariable* var = NULL;
    while (!bDone) {
        // FIXME: Unnecessary copying.
        if (m_hashEntries[curr].GetName() == sCopy
            && m_hashEntries[curr].m_intValue == nValue) {
            var = &(m_hashEntries[curr]);
            break;
        }

        // FIXME: Unnecessary copying.
        if (m_hashEntries[curr].GetName() == "") {
            break;
        }

        curr++;
        if (curr >= m_nTableEntries) {
            curr = 0;
        }

        if (curr == end) {
            bDone = TRUE;
        }
    }

    return var;
}

// 0x550280
void CVariableHash::ClearAll()
{
    for (int index = 0; index < m_nTableEntries; index++) {
        // FIXME: Unnecessary copying.
        if (m_hashEntries[index].GetName() != "") {
            CVariable dummy;
            m_hashEntries[index] = dummy;
        }
    }
}

// 0x5503A0
void CVariableHash::Marshal(CVariable** pList, WORD* pCount)
{
    CTypedPtrList<CPtrList, CVariable*> l;

    for (int index = 0; index < m_nTableEntries; index++) {
        // FIXME: Unnecessary copying.
        if (m_hashEntries[index].GetName() != "") {
            l.AddTail(new CVariable(m_hashEntries[index]));
        }
    }

    *pCount = static_cast<WORD>(l.GetCount());

    if (*pCount != 0) {
        *pList = new CVariable[*pCount];

        int index = 0;
        while (l.GetCount() != 0) {
            CVariable* pTemp = l.RemoveHead();
            (*pList)[index++] = *pTemp;
            delete pTemp;
        }
    }
}

// 0x550600
void CVariableHash::MarshalToCharacter(CGameSprite* pSprite)
{
    ITEM_EFFECT* pItemEffect = new ITEM_EFFECT;
    CGameEffect::ClearItemEffect(pItemEffect, 0xBB);

    // NOTE: Original builds this blank variable with an inlined copy of the
    // constructor.
    CVariable blank;

    for (LONG nIndex = 0; nIndex < m_nTableEntries; nIndex++) {
        CVariable* pVariable = &m_hashEntries[nIndex];

        if (pVariable->GetName() != "") {
            CGameEffect* pEffect = CGameEffect::DecodeEffect(pItemEffect,
                pSprite->GetPos(),
                pSprite->GetId(),
                CPoint(-1, -1));

            memcpy(pEffect->m_scriptName, pVariable->m_name, sizeof(SCRIPTNAME));
            pEffect->m_flags = pVariable->m_type;
            pEffect->m_effectAmount2 = pVariable->m_resRefType;
            pEffect->m_dwFlags = pVariable->m_dwValue;
            pEffect->m_effectAmount = pVariable->m_intValue;
            pEffect->m_effectAmount3 = static_cast<LONG>(pVariable->m_floatValue);

            // NOTE: Original reads it from a constant at 0x8491AA.
            pEffect->m_durationType = 1;

            CString sValue(pVariable->m_stringValue);
            pEffect->m_res = sValue.Left(8);

            if (sValue.GetLength() > 8) {
                pEffect->m_res2 = sValue.Mid(8, 8);

                if (sValue.GetLength() > 16) {
                    pEffect->m_res3 = sValue.Mid(16, 8);
                }
            }

            pSprite->AddEffect(pEffect, CGameAIBase::EFFECT_LIST_TIMED, TRUE, FALSE);

            *pVariable = blank;
        }
    }

    delete pItemEffect;
}

// 0x550920
void CVariableHash::Resize(LONG nSize)
{
    CVariable* oldHashEntries = m_hashEntries;
    LONG oldTableEntries = m_nTableEntries;

    m_hashEntries = new CVariable[nSize];
    m_nTableEntries = nSize;

    for (int index = 0; index < oldTableEntries; index++) {
        if (oldHashEntries[index].GetName() != "") {
            AddKey(oldHashEntries[index]);
        }
    }

    delete oldHashEntries;
}

// 0x550270
int CVariableHash::RemoveKey(CString sVarName, BOOL a2, LONG nValue)
{
    return 0;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CNamedCreatureVariableHashEntry::CNamedCreatureVariableHashEntry()
{
    m_pNext = NULL;
}

// NOTE: Inlined.
CNamedCreatureVariableHashEntry::CNamedCreatureVariableHashEntry(const CVariable& var)
    : m_variable(var)
{
    m_pNext = NULL;
}

// 0x550B30
void CNamedCreatureVariableHashEntry::sub_550B30()
{
    if (m_pNext != NULL) {
        m_pNext->sub_550B30();

        delete m_pNext;
        m_pNext = NULL;
    }
}

// 0x550CF0
CNamedCreatureVariableHashEntry* CNamedCreatureVariableHashEntry::sub_550CF0()
{
    CNamedCreatureVariableHashEntry* var = this;

    while (var->m_pNext != NULL) {
        var = var->m_pNext;
    }

    return var;
}

// -----------------------------------------------------------------------------

// 0x550A20
CNamedCreatureVariableHash::CNamedCreatureVariableHash(LONG nSize)
{
    m_nTableEntries = nSize;
    m_hashEntries = new CNamedCreatureVariableHashEntry*[nSize];

    for (int index = 0; index < nSize; index++) {
        m_hashEntries[index] = NULL;
    }
}

// 0x550A90
CNamedCreatureVariableHash::~CNamedCreatureVariableHash()
{
    // NOTE: Uninline.
    ClearAll();

    delete m_hashEntries;
    m_hashEntries = NULL;
}

// 0x550B60
void CNamedCreatureVariableHash::ClearAll()
{
    if (m_hashEntries != NULL) {
        for (int index = 0; index < m_nTableEntries; index++) {
            if (m_hashEntries[index] != NULL) {
                // NOTE: Uninline.
                m_hashEntries[index]->sub_550B30();

                delete m_hashEntries[index];
                m_hashEntries[index] = NULL;
            }
        }
    }
}

// 0x550BC0
void CNamedCreatureVariableHash::Resize(LONG nSize)
{
    int index;

    if (nSize < m_nTableEntries && m_hashEntries) {
        for (index = nSize; index < m_nTableEntries; index++) {
            if (m_hashEntries[index] != NULL) {
                // NOTE: Uninline.
                m_hashEntries[index]->sub_550B30();

                delete m_hashEntries[index];
                m_hashEntries[index] = NULL;
            }
        }
    }

    CNamedCreatureVariableHashEntry** oldHashEntries = m_hashEntries;
    LONG oldTableEntries = m_nTableEntries;
    m_nTableEntries = nSize;

    m_hashEntries = new CNamedCreatureVariableHashEntry*[nSize];

    for (index = 0; index < nSize; index++) {
        m_hashEntries[index] = NULL;
    }

    for (index = 0; index < oldTableEntries; index++) {
        if (oldHashEntries[index] != NULL) {
            CNamedCreatureVariableHashEntry* pCurr = oldHashEntries[index];
            while (pCurr != NULL) {
                CNamedCreatureVariableHashEntry* pNext = pCurr->m_pNext;
                pCurr->m_pNext = NULL;

                int hash = Hash(pCurr->m_variable.GetName());
                if (m_hashEntries[hash] != NULL) {
                    m_hashEntries[hash]->sub_550CF0()->m_pNext = pCurr;
                } else {
                    m_hashEntries[hash] = pCurr;
                }

                pCurr = pNext;
            }
        }
    }

    delete oldHashEntries;
}

// 0x550D10
BOOL CNamedCreatureVariableHash::AddKey(CVariable& var)
{
    CString sName = var.GetName();
    CString sTemp;

    // NOTE: The 32-character cap counts the characters kept, not the
    // characters read.
    int nKept = 0;
    for (int index = 0; index < sName.GetLength() && nKept < 32; index++) {
        if (sName[index] != ' ') {
            sTemp += static_cast<char>(toupper(sName[index]));
            nKept++;
        }
    }

    if (sTemp == "") {
        return FALSE;
    }

    var.SetName(sTemp);

    int hash = Hash(sTemp);

    if (m_hashEntries[hash] != NULL) {
        CNamedCreatureVariableHashEntry* pCurr = m_hashEntries[hash];
        while (pCurr != NULL) {
            // FIXME: Unnecessary copying.
            if (sTemp.CompareNoCase(pCurr->m_variable.GetName()) == 0) {
                pCurr->m_variable = var;
                return TRUE;
            }
            pCurr = pCurr->m_pNext;
        }
    }

    CNamedCreatureVariableHashEntry* pVar = new CNamedCreatureVariableHashEntry(var);
    if (m_hashEntries[hash] != NULL) {
        m_hashEntries[hash]->sub_550CF0()->m_pNext = pVar;
    } else {
        m_hashEntries[hash] = pVar;
    }

    // NOTE: A new key returns FALSE; only replacing an existing key returns
    // TRUE.
    return FALSE;
}

// FIXME: Use reference.
//
// 0x550F70
CVariable* CNamedCreatureVariableHash::FindKey(CString sVarName)
{
    CString sTemp;

    // NOTE: The 32-character cap counts the characters kept, not the
    // characters read.
    int nKept = 0;
    for (int index = 0; index < sVarName.GetLength() && nKept < 32; index++) {
        if (sVarName[index] != ' ') {
            sTemp += static_cast<char>(toupper(sVarName[index]));
            nKept++;
        }
    }

    if (sTemp != "") {
        int hash = Hash(sTemp);

        CNamedCreatureVariableHashEntry* pEntry = m_hashEntries[hash];
        while (pEntry != NULL) {
            if (sTemp.CompareNoCase(pEntry->m_variable.GetName()) == 0) {
                return &(pEntry->m_variable);
            }
            pEntry = pEntry->m_pNext;
        }
    }

    return NULL;
}

// 0x551090
CVariable* CNamedCreatureVariableHash::FindKey(CString sVarName, LONG nValue)
{
    CString sTemp;

    // NOTE: The 32-character cap counts the characters kept, not the
    // characters read.
    int nKept = 0;
    for (int index = 0; index < sVarName.GetLength() && nKept < 32; index++) {
        if (sVarName[index] != ' ') {
            sTemp += static_cast<char>(toupper(sVarName[index]));
            nKept++;
        }
    }

    if (sTemp != "") {
        int hash = Hash(sTemp);

        CNamedCreatureVariableHashEntry* pEntry = m_hashEntries[hash];
        while (pEntry != NULL) {
            if (sTemp.CompareNoCase(pEntry->m_variable.GetName()) == 0
                && pEntry->m_variable.m_intValue == nValue) {
                return &(pEntry->m_variable);
            }
            pEntry = pEntry->m_pNext;
        }
    }

    return NULL;
}

// 0x5511C0
BOOL CNamedCreatureVariableHash::RemoveKey(CString sVarName, BOOL a2, LONG nValue)
{
    CString sTemp;

    // NOTE: The 32-character cap counts the characters kept, not the
    // characters read.
    int nKept = 0;
    for (int index = 0; index < sVarName.GetLength() && nKept < 32; index++) {
        if (sVarName[index] != ' ') {
            sTemp += static_cast<char>(toupper(sVarName[index]));
            nKept++;
        }
    }

    if (sTemp != "") {
        int hash = Hash(sTemp);

        CNamedCreatureVariableHashEntry* pEntry = m_hashEntries[hash];
        CNamedCreatureVariableHashEntry* pPrev = pEntry;
        while (pEntry != NULL) {
            if (sTemp.CompareNoCase(pEntry->m_variable.GetName()) == 0
                && (!a2 || pEntry->m_variable.m_intValue == nValue)) {
                if (pPrev == pEntry) {
                    delete pEntry;
                    m_hashEntries[hash] = NULL;
                } else {
                    pPrev->m_pNext = pEntry->m_pNext;
                    delete pEntry;
                }

                return TRUE;
            }
            pPrev = pEntry;
            pEntry = pEntry->m_pNext;
        }
    }

    return FALSE;
}

// 0x551350
void CNamedCreatureVariableHash::Marshal(CVariable** pList, WORD* pCount)
{
    *pList = NULL;
    *pCount = 0;
}

// 0x4A51D0
void CNamedCreatureVariableHash::MarshalToCharacter(CGameSprite* pSprite)
{
}

// -----------------------------------------------------------------------------

// 0x4EC1C0
void CCtrlAltDelete::DisplayText(CString sText)
{
    // NOTE: Original reads the marker from a constant at 0x84C960 (-1).
    g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""), sText, -1, FALSE);
}

// 0x4EC250
void CCtrlAltDelete::MoveToArea(CString sAreaName)
{
    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(sAreaName), 0x3F2, TRUE) == NULL) {
        DisplayText("Error File " + sAreaName + " does not exist\n");
    } else {
        g_pBaldurChitin->GetScreenWorld()->MoveToArea(sAreaName);
    }
}

// 0x4EC350
void CCtrlAltDelete::EnableCheatKeys(int a1)
{
    g_pBaldurChitin->m_pEngineWorld->field_156 = a1;
}

// 0x4EC370
void CCtrlAltDelete::SetCurrentXP(DWORD dwCurrentXP)
{
    g_pBaldurChitin->m_pEngineWorld->SetCurrentXP(dwCurrentXP);
}

// 0x4EC390
void CCtrlAltDelete::CreateCreature(CString sName)
{
    CResRef resRef(sName);

    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(resRef, 0x3F1, TRUE) == NULL) {
        DisplayText("Error File " + sName + " does not exist\n");
        return;
    }

    CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetVisibleArea();

    INT x;
    INT y;
    pArea->GetInfinity()->GetViewPosition(x, y);

    CRect rViewPort;
    rViewPort.CopyRect(pArea->GetInfinity()->rViewPort);

    x += (rViewPort.right - rViewPort.left) >> 1;
    y += (rViewPort.bottom - rViewPort.top) >> 1;
    y /= CPathSearch::GRID_SQUARE_SIZEY;
    x /= CPathSearch::GRID_SQUARE_SIZEX;

    CCreatureFile cre;
    cre.SetResRef(CResRef(sName), TRUE, TRUE);

    CGameSprite* pSprite = new CGameSprite(cre.GetData(),
        cre.GetDataSize(),
        0,
        0,
        -1,
        0,
        0,
        0x7FFFFFFF,
        CPoint(-1, -1),
        0);

    pSprite->SetResRef(cre.GetResRef().GetResRefStr());

    // NOTE: Original passes an unused argument.
    cre.ReleaseData();

    if (pSprite != NULL) {
        pSprite->SetResRef(sName);

        CVariable var;

        CString sScriptName(pSprite->m_scriptName);
        sScriptName.TrimLeft();

        if (strcmp(sScriptName, "") != 0) {
            var.SetName(pSprite->GetScriptName());
            var.m_intValue = pSprite->m_id;

            // NOTE: Uninline.
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_namedCreatures.AddKey(var);
        }

        BYTE personalSpace = pSprite->m_animation.GetPersonalSpace();

        // NOTE: Uninline.
        CPoint pt = g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_search.GetNearestOpenSquare(CPoint(x, y),
            CGameObject::DEFAULT_TERRAIN_TABLE,
            personalSpace,
            -1);
        x = pt.x;
        y = pt.y;

        BYTE listType = pSprite->m_animation.GetListType();

        CPoint pos(CPathSearch::GRID_SQUARE_SIZEX * x, CPathSearch::GRID_SQUARE_SIZEY * y);

        // NOTE: Uninline.
        pSprite->AddToArea(g_pBaldurChitin->GetObjectGame()->GetVisibleArea(), pos, 0, listType);
    }
}

// 0x4EC870
void CCtrlAltDelete::AddItem(CString sName, WORD nUseCount1, WORD nUseCount2, WORD nUseCount3)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(sName), 0x3ED, TRUE) == NULL) {
        DisplayText("Error File " + sName + " does not exist\n");
        return;
    }

    // NOTE: Uninline.
    for (SHORT nIndex = 0; nIndex < pGame->m_nCharacters; nIndex++) {
        LONG nId = pGame->GetCharacterId(nIndex);

        CGameSprite* pSprite;
        BYTE rc;
        do {
            rc = pGame->m_cObjectArray.GetDeny(nId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED);

        if (rc == CGameObjectArray::SUCCESS) {
            INT nSlot;
            for (nSlot = 0; nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE; nSlot++) {
                if (pSprite->m_equipment.m_items[18 + nSlot] == NULL) {
                    pSprite->m_equipment.m_items[18 + nSlot] = new CItem(CResRef(sName),
                        nUseCount1,
                        nUseCount2,
                        nUseCount3,
                        0,
                        0);

                    pGame->m_cObjectArray.ReleaseDeny(nId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    return;
                }
            }

            pGame->m_cObjectArray.ReleaseDeny(nId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x4ECA80
void CCtrlAltDelete::SetAreaExplored()
{
    // NOTE: Uninline.
    g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_visibility.SetAreaExplored();
}

// 0x4ECAB0
void CCtrlAltDelete::AddGold(DWORD dwGold)
{
    g_pBaldurChitin->m_pEngineWorld->AddGold(dwGold);
}

// 0x4ECAD0
void CCtrlAltDelete::SetVariable(CString sName, CString sScope, LONG nValue)
{
    sScope.MakeUpper();
    sName.MakeUpper();

    CVariable var;

    if (strcmp(sScope, "GLOBAL") == 0) {
        CVariable* pVar = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        if (pVar != NULL) {
            pVar->m_intValue = nValue;
        } else {
            var.SetName(sName);
            var.m_intValue = nValue;
            g_pBaldurChitin->GetObjectGame()->m_variables.AddKey(var);
        }
    } else {
        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            CVariable* pVar = pArea->GetVariables()->FindKey(sName);
            if (pVar != NULL) {
                pVar->m_intValue = nValue;
            } else {
                var.SetName(sName);
                var.m_intValue = nValue;
                pArea->m_variables.AddKey(var);
            }
        }
    }
}

// 0x4ED230
void CCtrlAltDelete::AddGold500()
{
    g_pBaldurChitin->m_pEngineWorld->AddGold(500);
}

// 0x4ED250
void CCtrlAltDelete::AddTestItems()
{
    AddItem2("potn08", 5, 0, 0);
    AddItem2("potn20", 5, 0, 0);
    AddItem2("scrlpet", 1, 0, 0);
}

// 0x4ED2C0
void CCtrlAltDelete::JumpPartyToPointer()
{
    // NOTE: Uninline.
    CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetVisibleArea();
    CPoint ptWorld = pArea->GetInfinity()->GetWorldCoordinates(g_pChitin->m_ptPointer);

    CAIAction action(CAIAction::JUMPTOPOINT, ptWorld, 0, -1);

    g_pBaldurChitin->GetObjectGame()->GetGroup()->GroupAction(action, TRUE, NULL);
}

// 0x4ED500
void CCtrlAltDelete::AddItem2(CString sName, WORD nUseCount1, WORD nUseCount2, WORD nUseCount3)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(sName), 0x3ED, TRUE) == NULL) {
        // NOTE: Uninline.
        DisplayText("Error File " + sName + " does not exist\n");
        return;
    }

    // NOTE: Uninline.
    for (SHORT nIndex = 0; nIndex < pGame->m_nCharacters; nIndex++) {
        LONG nId = pGame->GetCharacterId(nIndex);

        CGameSprite* pSprite;
        BYTE rc;
        do {
            rc = pGame->m_cObjectArray.GetDeny(nId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED);

        if (rc == CGameObjectArray::SUCCESS) {
            INT nSlot;
            for (nSlot = 0; nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE; nSlot++) {
                if (pSprite->m_equipment.m_items[18 + nSlot] == NULL) {
                    pSprite->m_equipment.m_items[18 + nSlot] = new CItem(CResRef(sName),
                        nUseCount1,
                        nUseCount2,
                        nUseCount3,
                        0,
                        0);

                    pGame->m_cObjectArray.ReleaseDeny(nId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    return;
                }
            }

            pGame->m_cObjectArray.ReleaseDeny(nId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x4ED730
void CCtrlAltDelete::CheckVar(CString sName, CString sScope)
{
    sScope.MakeUpper();
    sName.MakeUpper();

    CString sResult;

    if (strcmp(sScope, "GLOBAL") == 0) {
        CVariable* pVar = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        if (pVar != NULL) {
            CString sValue;
            sValue.Format("%d", pVar->m_intValue);

            sResult = "GLOBAL::";
            sResult += pVar->GetName();
            sResult += " = ";
            sResult += sValue;
        } else {
            sResult = "No variable called ";
            sResult += "GLOBAL::";
            sResult += sName;
        }
    } else {
        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            CVariable* pVar = pArea->GetVariables()->FindKey(sName);
            if (pVar != NULL) {
                CString sValue;
                sValue.Format("%d", pVar->m_intValue);

                sResult = pArea->m_resRef.GetResRefStr();
                sResult += "::";
                sResult += pVar->GetName();
                sResult += " = ";
                sResult += sValue;
            } else {
                sResult = "No variable called ";
                sResult += sScope;
                sResult += "::";
                sResult += sName;
            }
        } else {
            sResult = "Need to be in area specified when checking local!";
        }
    }

    DisplayText(sResult);
}

// 0x4ECC70
void CCtrlAltDelete::AddSpell(CString sName, UINT nAmount)
{
    if (strcmp(sName, "ALL") == 0) {
        for (UINT nIndex = 0; nIndex < 7; nIndex++) {
            BYTE nClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nIndex);
            AddAllSpellsOfClass(nClass, nAmount);
        }
    } else if (strcmp(sName, "BARD") == 0) {
        BYTE nClass = CAIOBJECTTYPE_C_BARD;
        AddAllSpellsOfClass(nClass, nAmount);
    } else if (strcmp(sName, "CLERIC") == 0) {
        BYTE nClass = CAIOBJECTTYPE_C_CLERIC;
        AddAllSpellsOfClass(nClass, nAmount);
    } else if (strcmp(sName, "DRUID") == 0) {
        BYTE nClass = CAIOBJECTTYPE_C_DRUID;
        AddAllSpellsOfClass(nClass, nAmount);
    } else if (strcmp(sName, "PALADIN") == 0) {
        BYTE nClass = CAIOBJECTTYPE_C_PALADIN;
        AddAllSpellsOfClass(nClass, nAmount);
    } else if (strcmp(sName, "RANGER") == 0) {
        BYTE nClass = CAIOBJECTTYPE_C_RANGER;
        AddAllSpellsOfClass(nClass, nAmount);
    } else if (strcmp(sName, "SORCERER") == 0) {
        BYTE nClass = CAIOBJECTTYPE_C_SORCERER;
        AddAllSpellsOfClass(nClass, nAmount);
    } else if (strcmp(sName, "WIZARD") == 0) {
        BYTE nClass = CAIOBJECTTYPE_C_WIZARD;
        AddAllSpellsOfClass(nClass, nAmount);
    } else {
        if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(sName), 0x3EE, TRUE) == NULL) {
            DisplayText("Error File " + sName + " does not exist\n");
            return;
        }

        CResRef resRef(sName);
        CResRef resSpell = resRef;

        CSpell spell;
        spell.SetResRef(resSpell, TRUE, TRUE);

        CGameSprite* pSprite = NULL;

        // NOTE: Uninline.
        POSITION pos = g_pBaldurChitin->GetObjectGame()->m_group.m_memberList.GetHeadPosition();
        while (pos != NULL) {
            LONG nId = reinterpret_cast<LONG>(g_pBaldurChitin->GetObjectGame()->m_group.m_memberList.GetNext(pos));

            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->m_cObjectArray.GetDeny(nId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                BYTE nClass = 0;
                switch (spell.GetCasterType()) {
                case 1:
                    nClass = CAIOBJECTTYPE_C_WIZARD;
                    break;
                case 2:
                    nClass = CAIOBJECTTYPE_C_CLERIC;
                    break;
                case 4:
                    nClass = pSprite->GetAIType().m_nClass;
                    break;
                }

                switch (g_pBaldurChitin->GetObjectGame()->GetSpellType(resRef)) {
                case 1: {
                    INT nLevel = g_pBaldurChitin->GetObjectGame()->GetSpellLevel(resRef, nClass, 0);
                    if (nLevel < 9) {
                        UINT nSpellLevel = nLevel;
                        UINT a4 = nAmount;
                        UINT a5 = nAmount;
                        UINT a6 = 0;
                        pSprite->AddKnownSpell(nClass, nSpellLevel, resRef, a4, a5, a6);
                    }
                    break;
                }
                case 2: {
                    UINT a2 = nAmount;
                    UINT a3 = nAmount;
                    UINT a4 = 0;
                    pSprite->AddInnateSpell(resRef, a2, a3, a4);
                    break;
                }
                case 3: {
                    UINT a2 = nAmount;
                    UINT a3 = nAmount;
                    UINT a4 = 0;
                    pSprite->AddSong(resRef, a2, a3, a4);
                    break;
                }
                case 4: {
                    UINT a2 = 0;
                    pSprite->AddShapeshift(resRef, a2);

                    UINT a1 = nAmount;
                    pSprite->sub_724C40(a1);

                    UINT a1b = nAmount;
                    BOOLEAN a2b = FALSE;
                    pSprite->m_shapeshifts.sub_725D30(a1b, a2b);
                    break;
                }
                }

                g_pBaldurChitin->GetObjectGame()->m_cObjectArray.ReleaseDeny(nId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }
}

// 0x4EDAB0
void CCtrlAltDelete::AddProtagonistAction161(LONG nValue)
{
    // NOTE: Original reads action ID from a constant at 0x8478B6.
    CAIAction action(161, nValue, 1, 0);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    CGameSprite* pSprite;
    BYTE rc = pGame->m_cObjectArray.GetShare(pGame->GetProtagonist(),
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pSprite),
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        pSprite->AddAction(action);

        pGame = g_pBaldurChitin->GetObjectGame();
        pGame->m_cObjectArray.ReleaseShare(pGame->GetProtagonist(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x4EDCA0
void CCtrlAltDelete::AddAllSpellsOfClass(const BYTE& nClass, UINT nAmount)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CGroupedSpellList* pSpells = &(pGame->m_spellsByClass[pGame->GetSpellcasterIndex(nClass)]);

    CGameSprite* pSprite = NULL;

    // NOTE: Uninline.
    POSITION pos = g_pBaldurChitin->GetObjectGame()->m_group.m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG nId = reinterpret_cast<LONG>(g_pBaldurChitin->GetObjectGame()->m_group.m_memberList.GetNext(pos));

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->m_cObjectArray.GetDeny(nId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            for (UINT nLevel = 0; nLevel < pSpells->m_nHighestLevel; nLevel++) {
                for (UINT nIndex = 0;; nIndex++) {
                    // __FILE__: .\Include\InfGame.h
                    // __LINE__: 772
                    UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

                    if (nIndex >= pSpells->m_lists[nLevel].m_nCount) {
                        break;
                    }

                    // __FILE__: .\Include\InfGame.h
                    // __LINE__: 772
                    UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

                    // NOTE: Uninline.
                    UINT nID = pSpells->m_lists[nLevel].Get(nIndex);

                    pGame = g_pBaldurChitin->GetObjectGame();

                    UINT a4 = nAmount;
                    UINT a5 = nAmount;
                    UINT a6 = 0;
                    pSprite->AddKnownSpell(nClass, nLevel, pGame->m_spells.Get(nID), a4, a5, a6);

                    if (nClass == CAIOBJECTTYPE_C_BARD || nClass == CAIOBJECTTYPE_C_SORCERER) {
                        pSprite->GetSpellsAtLevel(nClass, nLevel)->field_14 = pSprite->GetSpellsAtLevel(nClass, nLevel)->field_14 + nAmount;

                        UINT a1 = nAmount;
                        BOOLEAN a2 = FALSE;
                        pSprite->GetSpellsAtLevel(nClass, nLevel)->sub_725D30(a1, a2);
                    }
                }
            }

            g_pBaldurChitin->GetObjectGame()->m_cObjectArray.ReleaseDeny(nId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x4EDF60
void CCtrlAltDelete::JeffKAttacks()
{
    CreateCreature("JeffK");
}

// 0x4EDF80
void CCtrlAltDelete::JeffKDefends()
{
    CreateCreature("JeffK2");
}
