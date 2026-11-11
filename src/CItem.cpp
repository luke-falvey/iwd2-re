#include "CItem.h"

#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CImmunities.h"
#include "CInfGame.h"
#include "CUtil.h"

// 0x8D828C
const CString CItem::VALUE("VALUE");

// 0x8D86B0
BYTE byte_8D86B0[7];

// 0x4E7840
CItem::CItem()
{
    m_flags = 0;
    m_nAbilities = 0;
    m_numSounds = 0;
    m_wear = 0;
    m_useCount1 = 1;
    m_useCount2 = 1;
    m_useCount3 = 1;
}

// NOTE: `m_nAbilities` is left uninitialized when the resource is missing.
//
// 0x4E7900
CItem::CItem(const CItem& item)
{
    m_useCount1 = item.m_useCount1;
    m_useCount2 = item.m_useCount2;
    m_useCount3 = item.m_useCount3;
    m_numSounds = item.m_numSounds;
    m_wear = item.m_wear;
    m_flags = item.m_flags;

    CResRef resRef;
    resRef = item.cResRef;

    CString sResRef;
    resRef.CopyToString(sResRef);

    sub_4EA4A0(sResRef);
    g_pBaldurChitin->GetObjectGame()->GetRuleTables().sub_5463E0(sResRef);

    if (sResRef.CompareNoCase("no_drop") == 0) {
        sResRef = "";
    }

    if (strcmp(sResRef, "MISC07") == 0) {
        if (m_useCount1 == 0) {
            m_useCount1 = 1;
        } else {
            INT nRange = m_useCount2;
            m_useCount1 = static_cast<WORD>((nRange != 0 ? rand() % nRange : 0) + m_useCount1);
        }
        m_useCount2 = 0;
    }

    CResHelper<CResItem, 1005>::SetResRef(CResRef(sResRef), TRUE, TRUE);

    // NOTE: Uninline.
    GetAbilityCount();

    // NOTE: Uninline.
    if (GetLoreValue() == 0) {
        m_flags |= 0x1;
    }
}

// NOTE: `m_numSounds` is left uninitialized, as is `m_nAbilities` when the
// resource is missing.
//
// 0x4E7B60
CItem::CItem(const CCreatureFileItem& item)
{
    // FIXME: Treats `m_itemId` as a null-terminated string. An 8-character
    // resref runs into `m_wear`.
    CResRef resRef(CString(reinterpret_cast<const char*>(item.m_itemId)));

    CString sResRef;
    resRef.CopyToString(sResRef);
    sResRef.MakeUpper();

    m_useCount1 = item.m_usageCount[0];
    m_useCount2 = item.m_usageCount[1];
    m_useCount3 = item.m_usageCount[2];

    sub_4EA4A0(sResRef);
    g_pBaldurChitin->GetObjectGame()->GetRuleTables().sub_5463E0(sResRef);

    if (sResRef.CompareNoCase("no_drop") == 0) {
        sResRef = "";
    }

    if (strcmp(sResRef, "MISC07") == 0) {
        if (m_useCount1 == 0) {
            m_useCount1 = 1;
        } else {
            INT nRange = m_useCount2;
            m_useCount1 = static_cast<WORD>((nRange != 0 ? rand() % nRange : 0) + m_useCount1);
        }
        m_useCount2 = 0;
    }

    CResHelper<CResItem, 1005>::SetResRef(CResRef(sResRef), TRUE, TRUE);

    // NOTE: Uninline.
    if (GetMaxStackable() > 1) {
        m_useCount1 = max(m_useCount1, 1);
        m_useCount2 = max(m_useCount2, 1);
        m_useCount3 = max(m_useCount3, 1);
    }

    m_wear = item.m_wear;
    m_flags = item.m_dynamicFlags;

    // NOTE: Uninline.
    GetAbilityCount();

    // NOTE: Uninline.
    if (GetLoreValue() == 0) {
        m_flags |= 0x1;
    }

    if ((m_flags & 0x1) == 0) {
        m_flags |= 0x8;
    }
}

// FIXME: `id` should be reference.
//
// NOTE: `m_numSounds` is left uninitialized, as is `m_nAbilities` when the
// resource is missing.
//
// 0x4E7E90
CItem::CItem(CResRef id, WORD useCount1, WORD useCount2, WORD useCount3, int wear, DWORD flags)
{
    CString sResRef;
    id.CopyToString(sResRef);
    sResRef.MakeUpper();

    m_useCount1 = useCount1;
    m_useCount2 = useCount2;
    m_useCount3 = useCount3;

    sub_4EA4A0(sResRef);
    g_pBaldurChitin->GetObjectGame()->GetRuleTables().sub_5463E0(sResRef);

    if (sResRef.CompareNoCase("no_drop") == 0) {
        sResRef = "";
    }

    if (strcmp(sResRef, "MISC07") == 0) {
        if (m_useCount1 == 0) {
            m_useCount1 = 1;
        } else {
            INT nRange = m_useCount2;
            m_useCount1 = static_cast<WORD>((nRange != 0 ? rand() % nRange : 0) + m_useCount1);
        }
        m_useCount2 = 0;
    }

    CResHelper<CResItem, 1005>::SetResRef(CResRef(sResRef), TRUE, TRUE);

    // NOTE: Uninline.
    if (GetMaxStackable() > 1) {
        m_useCount1 = max(m_useCount1, 1);
        m_useCount2 = max(m_useCount2, 1);
        m_useCount3 = max(m_useCount3, 1);
    }

    m_flags = flags;
    m_wear = wear;

    // NOTE: Uninline.
    GetAbilityCount();

    // NOTE: Uninline.
    if (GetLoreValue() == 0) {
        m_flags |= 0x1;
    }

    if ((m_flags & 0x1) == 0) {
        m_flags |= 0x8;
    }
}

// 0x4E8180
CItem::~CItem()
{
    m_useCount1 = 1;
    m_useCount2 = 1;
    m_useCount3 = 1;
    m_nAbilities = 0;
    m_wear = 0;
}

// 0x4E8240
CCreatureFileItem CItem::GetItemFile()
{
    CCreatureFileItem temp;

    cResRef.GetResRef(temp.m_itemId);
    temp.m_dynamicFlags = m_flags;
    temp.m_usageCount[0] = m_useCount1;
    temp.m_usageCount[1] = m_useCount2;
    temp.m_usageCount[2] = m_useCount3;
    temp.m_wear = m_wear;

    return temp;
}

// 0x4E82B0
BOOL CItem::Demand()
{
    if (cResRef == "") {
        return FALSE;
    }

    if (pRes == NULL) {
        return FALSE;
    }

    pRes->Demand();

    return pRes != NULL;
}

// 0x4E82F0
BOOL CItem::Release()
{
    if (pRes == NULL) {
        return FALSE;
    }

    pRes->Release();

    return TRUE;
}

// 0x4E8310
BOOL CItem::ReleaseAll()
{
    if (pRes == NULL) {
        return FALSE;
    }

    while (pRes->GetDemands() > 0) {
        pRes->Release();
    }

    return TRUE;
}

// 0x4E8350
void CItem::SetResRef(const CResRef& cNewResRef, BOOL bSetAutoRequest)
{
    CResHelper<CResItem, 1005>::SetResRef(cNewResRef, bSetAutoRequest, TRUE);
}

// 0x4E8440
INT CItem::GetAbilityCount()
{
    if (cResRef == "") {
        m_nAbilities = 0;
    } else {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 810
            UTIL_ASSERT(pRes->Demand());

            m_nAbilities = pRes->m_pHeader->abilityCount;
            pRes->Release();
        }
    }

    return m_nAbilities;
}

// 0x4E85F0
WORD CItem::GetMaxUsageCount(INT nAbility)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 904
    UTIL_ASSERT(pRes->Demand());

    // NOTE: Uninline.
    GetAbilityCount();

    WORD nMaxUsageCount = 0;
    if (nAbility >= 0 && nAbility < m_nAbilities) {
        // NOTE: Uninline.
        if (GetMaxStackable() <= 1) {
            // NOTE: Uninline.
            ITEM_ABILITY* ability = GetAbility(nAbility);
            if (ability != NULL) {
                nMaxUsageCount = ability->maxUsageCount;
            }
        }
    }

    pRes->Release();
    return nMaxUsageCount;
}

// 0x4E84B0
WORD CItem::GetUsageCount(INT nAbility)
{
    // NOTE: Uninline.
    GetAbilityCount();

    if (nAbility < 0) {
        return 0;
    }

    if (nAbility < m_nAbilities) {
        // NOTE: Uninline.
        // NOTE: Not used.
        GetMaxStackable();

        switch (nAbility) {
        case 1:
            return m_useCount2;
        case 2:
            return m_useCount3;
        default:
            return m_useCount1;
        }
    }

    if (nAbility == 0) {
        // NOTE: Uninline.
        if (GetMaxStackable() > 1) {
            return m_useCount1;
        }
    }

    return 0;
}

// 0x4E8760
void CItem::SetUsageCount(INT nAbility, WORD nUseCount)
{
    // NOTE: Uninline.
    GetAbilityCount();

    if (nAbility < 0) {
        return;
    }

    if (nAbility < m_nAbilities) {
        switch (nAbility) {
        case 1:
            m_useCount2 = nUseCount;
            return;
        case 2:
            m_useCount3 = nUseCount;
            return;
        default:
            m_useCount1 = nUseCount;
            return;
        }
    }

    if (nAbility == 0) {
        // NOTE: Uninline.
        if (GetMaxStackable() > 1) {
            m_useCount1 = nUseCount;
        }
    }
}

// 0x4E8860
void CItem::Equip(CGameSprite* pSprite, LONG slotNum, BOOL animationOnly)
{
    BOOL bOutermost = FALSE;

    if (cResRef == "") {
        return;
    }

    if (pSprite->field_7544 == 0) {
        pSprite->field_7540 = 0;
        bOutermost = TRUE;
        pSprite->field_7544 = 1;
    }

    if (pRes != NULL) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
        // __LINE__: 1026
        UTIL_ASSERT(pRes->Demand());

        CString sAnimation;

        if (pSprite->m_animation.m_animation != NULL) {
            switch (sub_4E9250(sAnimation, pSprite)) {
            case 1:
                if (sAnimation != "") {
                    // NOTE: Uninline.
                    pSprite->m_animation.EquipArmor(sAnimation[0], pSprite->m_baseStats.m_colors);
                } else {
                    // NOTE: Uninline.
                    pSprite->m_animation.EquipArmor('1', pSprite->m_baseStats.m_colors);
                }
                break;
            case 2:
                // NOTE: Uninline.
                pSprite->m_animation.EquipHelmet(sAnimation, pSprite->m_baseStats.m_colors);
                break;
            case 3:
                // NOTE: Uninline.
                pSprite->m_animation.EquipShield(sAnimation, pSprite->m_baseStats.m_colors);
                break;
            case 4:
                switch (slotNum) {
                case 42:
                case 43:
                case 45:
                case 47:
                case 49: {
                    // NOTE: Uninline.
                    ITEM_ABILITY* ability = GetAbility(pSprite->m_equipment.m_selectedWeaponAbility);
                    if (ability != NULL) {
                        DWORD itemFlags = pRes->m_pHeader->itemFlags;

                        // NOTE: Uninline.
                        pSprite->m_animation.EquipWeapon(sAnimation,
                            pSprite->m_baseStats.m_colors,
                            itemFlags,
                            ability->attackProbability);
                    } else {
                        // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
                        // __LINE__: 1067
                        UTIL_ASSERT(FALSE);
                    }
                    break;
                }
                case 44:
                case 46:
                case 48:
                case 50: {
                    sAnimation += "O";

                    // NOTE: Uninline.
                    GetAbilityCount();

                    ITEM_ABILITY* ability;
                    if (pSprite->m_equipment.m_selectedWeaponAbility < m_nAbilities) {
                        // NOTE: Uninline.
                        ability = GetAbility(pSprite->m_equipment.m_selectedWeaponAbility);
                    } else {
                        // NOTE: Uninline.
                        ability = GetAbility(0);
                    }

                    if (ability != NULL) {
                        DWORD itemFlags = pRes->m_pHeader->itemFlags | 0x400;

                        // NOTE: Uninline.
                        pSprite->m_animation.EquipWeapon(sAnimation,
                            pSprite->m_baseStats.m_colors,
                            itemFlags,
                            ability->attackProbability);
                    } else {
                        // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
                        // __LINE__: 1093
                        UTIL_ASSERT(FALSE);
                    }
                    break;
                }
                }
                break;
            }
        }

        if (!animationOnly) {
            for (INT nEffect = 0; nEffect < pRes->GetEquipedEffectNo(); nEffect++) {
                CGameEffect* pEffect = CGameEffect::DecodeEffect(pRes->GetEquipedEffect(nEffect),
                    CPoint(-1, -1),
                    -1,
                    CPoint(-1, -1));
                pEffect->field_188 = 1;
                pEffect->m_sourceID = pSprite->m_id;
                pEffect->m_flags |= 0x2;
                pEffect->m_casterLevel = 10;
                pEffect->m_sourceRes = cResRef;

                // NOTE: 2 is the "while equipped" duration type (0x8491AC).
                pSprite->AddEffect(pEffect,
                    pEffect->m_durationType == 2
                        ? CGameAIBase::EFFECT_LIST_EQUIPED
                        : CGameAIBase::EFFECT_LIST_TIMED,
                    FALSE,
                    TRUE);
            }
        }

        pRes->Release();
    }

    if (bOutermost) {
        if (pSprite->field_7540 != 0) {
            Equip(pSprite, slotNum, TRUE);
        }
        pSprite->field_7544 = 0;
    }
}

// 0x4E8DF0
void CItem::Unequip(CGameSprite* pSprite, LONG slotNum, BOOL recalculateEffects, BOOL animationOnly)
{
    WORD attackProb[6] = { 34, 33, 33, 0, 0, 0 };

    if (cResRef == "") {
        return;
    }

    if (pRes == NULL) {
        return;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1164
    UTIL_ASSERT(pRes->Demand());

    if (!animationOnly) {
        for (INT nEffect = 0; nEffect < pRes->GetEquipedEffectNo(); nEffect++) {
            CGameEffect* pEffect = CGameEffect::DecodeEffect(pRes->GetEquipedEffect(nEffect),
                CPoint(-1, -1),
                -1,
                CPoint(-1, -1));
            pEffect->m_sourceID = pSprite->m_id;
            pEffect->m_sourceRes = pRes->GetResRef();

            // NOTE: 2 is the "while equipped" duration type (0x8491AC).
            if (pEffect->m_durationType == 2) {
                pSprite->m_equipedEffectList.RemoveMatchingEffect(pEffect);
            }

            if (pEffect != NULL) {
                delete pEffect;
            }
        }

        if (pRes->GetEquipedEffectNo() > 0) {
            pSprite->field_562C = 1;
            if (recalculateEffects
                && !g_pBaldurChitin->GetObjectGame()->m_worldTime.m_active) {
                pSprite->ProcessEffectList();
            }
        }
    }

    CString sAnimation;

    if (pSprite->m_animation.m_animation != NULL) {
        switch (sub_4E9250(sAnimation, pSprite)) {
        case 1:
            // NOTE: Uninline.
            pSprite->m_animation.EquipArmor('1', pSprite->m_baseStats.m_colors);
            break;
        case 2:
            // NOTE: Uninline.
            pSprite->m_animation.EquipHelmet(CString(""), byte_8D86B0);
            break;
        case 3:
            // NOTE: Uninline.
            pSprite->m_animation.EquipShield(CString(""), byte_8D86B0);
            break;
        case 4:
            if (slotNum != static_cast<BYTE>(pSprite->sub_726800())) {
                // NOTE: Uninline.
                pSprite->m_animation.EquipWeapon(CString(""), byte_8D86B0, 0, attackProb);
            } else {
                // NOTE: Uninline.
                pSprite->m_animation.EquipWeapon(CString(""), byte_8D86B0, 0x400, attackProb);

                // NOTE: Uninline.
                pSprite->m_animation.EquipShield(CString(""), byte_8D86B0);
            }
            break;
        }
    }

    pRes->Release();
}

// 0x4E91F0
WORD CItem::GetAnimationType()
{
    if (cResRef == "") {
        return NULL;
    }

    if (pRes == NULL) {
        return NULL;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1268
    UTIL_ASSERT(pRes->Demand());

    WORD animationType = pRes->m_pHeader->animationType[1] | (pRes->m_pHeader->animationType[0] << 8);
    pRes->Release();

    return animationType;
}

// Returns which part of the paper doll the item changes (0 - none, 1 - armor,
// 2 - helmet, 3 - shield, 4 - weapon) and stores the animation suffix in
// `sAnimation`. The resource must be demanded by the caller.
//
// 0x4E9250
BYTE CItem::sub_4E9250(CString& sAnimation, CGameSprite* pSprite)
{
    BYTE nType = 0;

    sAnimation = "";

    // FIXME: `animationType` is used as a null-terminated string (here and
    // below), but it is only two bytes long and is followed by
    // `minLevelRequired`.
    const char* animationType = reinterpret_cast<const char*>(pRes->m_pHeader->animationType);

    switch (pRes->m_pHeader->itemType) {
    case 0: {
        CString sType("");
        sType = animationType;

        if (sType == "S1"
            || sType == "FS"
            || sType == "S2"
            || sType == "AX"
            || sType == "BW"
            || sType == "CL"
            || sType == "FL"
            || sType == "WH"
            || sType == "HB"
            || sType == "MC"
            || sType == "MS"
            || sType == "SP"
            || sType == "SL"
            || sType == "CB"
            || sType == "DD"
            || sType == "QS"
            || sType == "SS") {
            nType = 4;
            sAnimation = animationType;
        }
        break;
    }
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 40:
    case 42:
    case 43:
    case 45:
    case 46:
    case 48:
    case 50:
    case 51:
    case 52:
    case 54:
    case 55:
    case 56:
    case 58:
    case 59:
    case 70:
    case 71:
    case 73:
        break;
    case 7:
    case 72:
        nType = 2;
        sAnimation = animationType;
        break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 25:
    case 26:
    case 27:
    case 29:
    case 30:
    case 44:
    case 57:
    case 69:
        nType = 4;
        sAnimation = animationType;
        break;
    case 24:
    case 28:
        nType = 4;
        break;
    case 41:
    case 47:
    case 49:
    case 53:
        nType = 3;
        sAnimation = animationType;
        break;
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
        nType = 1;
        if ((animationType[1] == 'W' && (pSprite->m_baseStats.m_animationType & 0xF00) != 0x200)
            || (animationType[1] != 'W' && (pSprite->m_baseStats.m_animationType & 0xF00) == 0x200)) {
            sAnimation = CString("1");
        } else {
            sAnimation = CString(animationType[0], 1);
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
        // __LINE__: 1440
        UTIL_ASSERT(FALSE);
    }

    sAnimation.TrimLeft();
    sAnimation.TrimRight();

    return nType;
}

// NOTE: The resource must be demanded by the caller.
//
// 0x4E9610
ITEM_ABILITY* CItem::GetAbility(INT nAbility)
{
    if (cResRef == "") {
        return NULL;
    }

    if (pRes == NULL) {
        return NULL;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1478
    UTIL_ASSERT(pRes->GetDemands() > 0);

    ITEM_ABILITY* ability = pRes->GetAbility(nAbility);
    if (ability == NULL) {
        return NULL;
    }

    if (ability->type == 0) {
        return NULL;
    }

    return ability;
}

// 0x4E9680
CGameEffect* CItem::GetAbilityEffect(LONG abilityNum, LONG effectNum, CGameObject* pObject)
{
    if (cResRef == "") {
        return NULL;
    }

    if (pRes == NULL) {
        return NULL;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1521
    UTIL_ASSERT(pRes->Demand());

    if (effectNum >= pRes->GetAbilityEffectNo(abilityNum)) {
        pRes->Release();
        return NULL;
    }

    CGameEffect* pEffect = CGameEffect::DecodeEffect(pRes->GetAbilityEffect(abilityNum, effectNum),
        CPoint(-1, -1),
        -1,
        CPoint(-1, -1));
    if (pEffect != NULL) {
        // NOTE: Uninline.
        ITEM_ABILITY* ability = GetAbility(abilityNum);
        if (ability != NULL) {
            pEffect->m_school = ability->school;
            pEffect->m_sourceType = 2;
            // FIXME: Should it be `m_secondaryType`?
            pEffect->field_48 = ability->secondaryType;
            pEffect->m_sourceRes = cResRef;
            pEffect->m_sourceFlags = ability->abilityFlags;
            pEffect->m_casterLevel = 10;
        }
    }

    pRes->Release();
    return pEffect;
}

// 0x4E97E0
WORD CItem::GetItemType()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1595
    UTIL_ASSERT(pRes->Demand());

    WORD nType = pRes->m_pHeader->itemType;
    pRes->Release();

    return nType;
}

// Returns the number by which the item widens the critical threat range: 1
// for some weapon types, plus 1 when the ability has the 0x20000 flag.
//
// 0x4E9840
WORD CItem::sub_4E9840(const ITEM_ABILITY* pAbility)
{
    WORD nRange = 0;

    // NOTE: Uninline.
    switch (GetItemType()) {
    case 16:
    case 19:
    case 20:
    case 23:
    case 27:
    case 31:
    case 57:
    case 69:
        nRange = 1;
        break;
    }

    if (pAbility != NULL && (pAbility->abilityFlags & 0x20000) != 0) {
        nRange++;
    }

    return nRange;
}

// 0x4E9910
DWORD CItem::GetCriticalHitMultiplier()
{
    // NOTE: Uninline.
    WORD nType = GetItemType();
    switch (nType) {
    case 5:
    case 15:
    case 21:
    case 25:
    case 29:
    case 30:
        return 3;
    default:
        return 2;
    }
}

// 0x4E99B0
DWORD CItem::GetWeight()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1688
    UTIL_ASSERT(pRes->Demand());

    DWORD nWeight = pRes->m_pHeader->baseWeight;
    pRes->Release();

    // NOTE: Uninline.
    if (GetMaxStackable() > 1) {
        nWeight *= max(m_useCount1, 1);
    }

    return nWeight;
}

// 0x4E9A80
CResRef CItem::GetUsedUpItemId()
{
    CResRef usedResRef("");

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 1732
            UTIL_ASSERT(pRes->Demand());

            usedResRef = pRes->m_pHeader->usedUpItemID;
            pRes->Release();
        }
    }

    return usedResRef;
}

// 0x4E9B10
STRREF CItem::GetGenericName()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1770
    UTIL_ASSERT(pRes->Demand());

    STRREF strName;
    if ((m_flags & 0x1) != 0) {
        strName = pRes->m_pHeader->identifiedName;
    } else {
        strName = pRes->m_pHeader->genericName;
    }

    pRes->Release();

    return strName;
}

// 0x4E9B80
STRREF CItem::GetIdentifiedName()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1799
    UTIL_ASSERT(pRes->Demand());

    STRREF strName = pRes->m_pHeader->identifiedName;
    pRes->Release();

    return strName;
}

// 0x452F30
DWORD CItem::GetFlags()
{
    return m_flags;
}

// 0x4E9BE0
DWORD CItem::GetFlagsFile()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1840
    UTIL_ASSERT(pRes->Demand());

    DWORD dwFlags = pRes->m_pHeader->itemFlags;
    pRes->Release();

    return dwFlags;
}

// 0x4E9C40
DWORD CItem::GetNotUsableBy()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1879
    UTIL_ASSERT(pRes->Demand());

    DWORD dwNotUsableBy = pRes->m_pHeader->notUsableBy;
    pRes->Release();

    return dwNotUsableBy;
}

// 0x4E9CA0
DWORD CItem::GetNotUsableBy2()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1920
    UTIL_ASSERT(pRes->Demand());

    BYTE bNotUsableBy2a = pRes->m_pHeader->notUsableBy2a;
    BYTE bNotUsableBy2b = pRes->m_pHeader->notUsableBy2b;
    BYTE bNotUsableBy2c = pRes->m_pHeader->notUsableBy2c;
    BYTE bNotUsableBy2d = pRes->m_pHeader->notUsableBy2d;
    pRes->Release();

    return bNotUsableBy2d | (bNotUsableBy2c << 8) | (bNotUsableBy2b << 16) | (bNotUsableBy2a << 24);
}

// 0x4E9D40
CResRef CItem::GetGroundIcon()
{
    CResRef iconResRef("");

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 1973
            UTIL_ASSERT(pRes->Demand());

            iconResRef = pRes->m_pHeader->groundIcon;
            pRes->Release();
        }
    }

    if (iconResRef == "") {
        iconResRef = "gsack01";
    }

    return iconResRef;
}

// 0x4E9DF0
CResRef CItem::GetItemIcon()
{
    CResRef iconResRef("");

    if (cResRef != "") {
        if (IsBadReadPtr(pRes, 0x60)) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2019
            UTIL_ASSERT_MSG(FALSE, "Bad item!");
        }

        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2023
            UTIL_ASSERT(pRes->Demand());

            iconResRef = pRes->m_pHeader->itemIcon;
            pRes->Release();
        }
    }

    return iconResRef;
}
// 0x4E9EB0
WORD CItem::GetMaxStackable()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2061
    UTIL_ASSERT(pRes->Demand());

    WORD nMaxStack = pRes->m_pHeader->maxStackable;
    pRes->Release();

    return nMaxStack;
}

// 0x4E9F10
DWORD CItem::GetBaseValue()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2099
    UTIL_ASSERT(pRes->Demand());

    DWORD nBaseValue = pRes->m_pHeader->baseValue;
    pRes->Release();

    return nBaseValue;
}

// 0x4E9F70
WORD CItem::GetLoreValue()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2140
    UTIL_ASSERT(pRes->Demand());

    WORD nLoreValue = pRes->m_pHeader->loreValue;
    pRes->Release();

    return nLoreValue;
}

// 0x4E9FD0
STRREF CItem::GetDescription()
{
    if (cResRef == "") {
        return -1;
    }

    if (pRes == NULL) {
        return -1;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2176
    UTIL_ASSERT(pRes->Demand());

    STRREF strDescription;
    if ((m_flags & 0x1) != 0 && pRes->m_pHeader->identifiedDescription != -1) {
        strDescription = pRes->m_pHeader->identifiedDescription;
    } else {
        strDescription = pRes->m_pHeader->genericDescription;
    }

    pRes->Release();
    return strDescription;
}

// 0x4EA050
CResRef CItem::GetDescriptionPicture()
{
    CResRef iconResRef("");

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2218
            UTIL_ASSERT(pRes->Demand());

            iconResRef = pRes->m_pHeader->descriptionPicture;
            pRes->Release();
        }
    }

    return iconResRef;
}

// 0x4EA0E0
void CItem::LoadWeaponIdentification(CWeaponIdentification& weaponId)
{
    if (pRes == NULL) {
        return;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2425
    UTIL_ASSERT(pRes->Demand());

    weaponId.m_itemType = pRes->m_pHeader->itemType;
    weaponId.m_itemFlags = pRes->m_pHeader->itemFlags;
    weaponId.m_itemFlagMask = 0;
    weaponId.m_attributes = pRes->m_pHeader->attributes;

    pRes->Release();
}

// 0x4EA150
BYTE CItem::GetMinLevelRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2464
            UTIL_ASSERT(pRes->Demand());

            nValue = static_cast<BYTE>(pRes->m_pHeader->minLevelRequired);
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA1B0
BYTE CItem::GetMinSTRRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2483
            UTIL_ASSERT(pRes->Demand());

            nValue = static_cast<BYTE>(pRes->m_pHeader->minSTRRequired);
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA210
BYTE CItem::GetMinINTRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2521
            UTIL_ASSERT(pRes->Demand());

            nValue = pRes->m_pHeader->minINTRequired;
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA270
BYTE CItem::GetMinDEXRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2540
            UTIL_ASSERT(pRes->Demand());

            nValue = pRes->m_pHeader->minDEXRequired;
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA2D0
BYTE CItem::GetMinWISRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2559
            UTIL_ASSERT(pRes->Demand());

            nValue = pRes->m_pHeader->minWISRequired;
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA330
BYTE CItem::GetMinCONRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2578
            UTIL_ASSERT(pRes->Demand());

            nValue = pRes->m_pHeader->minCONRequired;
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA390
BYTE CItem::GetMinCHRRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2597
            UTIL_ASSERT(pRes->Demand());

            nValue = static_cast<BYTE>(pRes->m_pHeader->minCHRRequired);
            pRes->Release();
        }
    }

    return nValue;
}

// Returns the highest spell level among the effects of all abilities.
//
// 0x4EA3F0
DWORD CItem::sub_4EA3F0()
{
    DWORD nMaxLevel = 0;

    for (DWORD nAbility = 0; nAbility < static_cast<DWORD>(m_nAbilities); nAbility++) {
        // NOTE: Uninline.
        if (GetAbility(nAbility) != NULL) {
            // FIXME: Leaks every effect.
            LONG nEffect = 0;
            CGameEffect* pEffect = GetAbilityEffect(nAbility, nEffect, NULL);
            while (pEffect != NULL) {
                if (nMaxLevel <= pEffect->m_spellLevel) {
                    nMaxLevel = pEffect->m_spellLevel;
                }

                nEffect++;
                pEffect = GetAbilityEffect(nAbility, nEffect, NULL);
            }
        }
    }

    return nMaxLevel;
}

// Rolls the stack size of the random gold placeholders ("00RIITM?" and
// "00RTAVE1"): `m_useCount1` is the minimum, `m_useCount2` the random range.
//
// 0x4EA4A0
void CItem::sub_4EA4A0(CString& sResRef)
{
    if (sResRef.GetLength() == 7
        && _strnicmp(sResRef.GetBuffer(1), "00RIITM", 7) == 0) {
        // FIXME: The string is 7 characters long, so this looks at the null
        // terminator and always returns.
        char ch = static_cast<LPCSTR>(sResRef)[7];
        if (ch < '1' || ch > '9') {
            return;
        }
    } else if (sResRef.GetLength() != 8
        || _strnicmp(sResRef.GetBuffer(1), "00RTAVE1", 8) != 0) {
        return;
    }

    if (m_useCount1 == 0) {
        m_useCount2 = 0;
        m_useCount1 = 1;
    } else {
        INT nRange = m_useCount2;
        INT nRoll = nRange != 0 ? rand() % nRange : 0;
        m_useCount1 = static_cast<WORD>(nRoll + m_useCount1);
        m_useCount2 = 0;
    }
}

// NOTE: `rgbColor` is not used.
//
// 0x4EA580
void CItem::sub_4EA580(CUIControlTextDisplay* pText, COLORREF rgbColor)
{
    // NOTE: Not used.
    CString v1;
    CString v2;

    // NOTE: Uninline.
    GetItemType();

    // NOTE: Uninline.
    GetAbilityCount();

    CBaldurEngine* pEngine = static_cast<CBaldurEngine*>(g_pBaldurChitin->pActiveEngine);
    if (pEngine != NULL) {
        // NOTE: Uninline.
        STRREF strDescription = GetDescription();

        pEngine->UpdateText(pText,
            "%s",
            static_cast<LPCSTR>(CBaldurEngine::FetchString(strDescription)));
        pEngine->UpdateText(pText, "");

        sub_4EA750(pText, RGB(200, 200, 0));
    }
}

// Appends the "Usable by" / "Not usable by" class list of the item to `pText`.
//
// 0x4EA750
void CItem::sub_4EA750(CUIControlTextDisplay* pText, COLORREF rgbColor)
{
    BOOL bUsable = FALSE;

    // NOTE: Uninline.
    DWORD dwNotUsableBy = GetNotUsableBy();
    DWORD dwNotUsableBy2 = GetNotUsableBy2();

    CString sUnusable;
    CString sUsable;
    CString sTemp;
    CString v1; // NOTE: Not used.
    CString sKit1;
    CString sKit2;
    CString sKit3;
    CString sKit4;
    CString sKit5;
    CString sKit6;
    CString sKit7;
    CString sKit8;

    if (dwNotUsableBy == 0 && dwNotUsableBy2 == 0) {
        return;
    }

    CBaldurEngine* pEngine = static_cast<CBaldurEngine*>(g_pBaldurChitin->pActiveEngine);
    if (pEngine == NULL) {
        return;
    }

    // NOTE: The original counts the bits of `dwNotUsableBy` here and discards
    // the result.

    // NOTE: Uninline.
    dwNotUsableBy = GetNotUsableBy();

    if ((dwNotUsableBy & 0xE000) != 0) {
        CString s1 = "  " + CBaldurEngine::FetchString(31532) + "\n";
        CString s2 = "  " + CBaldurEngine::FetchString(31534) + "\n";
        CString s3 = "  " + CBaldurEngine::FetchString(31536) + "\n";

        if ((dwNotUsableBy & 0x4000) != 0) {
            sUnusable += s1;
        } else {
            sUsable += s1;
        }

        if ((dwNotUsableBy & 0x8000) != 0) {
            sUnusable += s2;
        } else {
            sUsable += s2;
        }

        if ((dwNotUsableBy & 0x2000) != 0) {
            sUnusable += s3;
        } else {
            sUsable += s3;
        }
    }

    if ((dwNotUsableBy & 0x31000) != 0) {
        CString s1 = "  " + CBaldurEngine::FetchString(31533) + "\n";
        CString s2 = "  " + CBaldurEngine::FetchString(31534) + "\n";
        CString s3 = "  " + CBaldurEngine::FetchString(31535) + "\n";

        if ((dwNotUsableBy & 0x10000) != 0) {
            sUnusable += s1;
        } else {
            sUsable += s1;
        }

        if ((dwNotUsableBy & 0x20000) != 0) {
            sUnusable += s2;
        } else {
            sUsable += s2;
        }

        if ((dwNotUsableBy & 0x1000) != 0) {
            sUnusable += s3;
        } else {
            sUsable += s3;
        }
    }

    if ((dwNotUsableBy & 0x3F800000) != 0) {
        CString s1 = "  " + CBaldurEngine::FetchString(31570) + "\n";
        CString s2 = "  " + CBaldurEngine::FetchString(31574) + "\n";
        CString s3 = "  " + CBaldurEngine::FetchString(31573) + "\n";
        CString s4 = "  " + CBaldurEngine::FetchString(31571) + "\n";
        CString s5 = "  " + CBaldurEngine::FetchString(31576) + "\n";
        CString s6 = "  " + CBaldurEngine::FetchString(31569) + "\n";
        CString s7 = "  " + CBaldurEngine::FetchString(31572) + "\n";

        if ((dwNotUsableBy & 0x800000) != 0) {
            sUnusable += s1;
        } else {
            sUsable += s1;
        }

        if ((dwNotUsableBy & 0x1000000) != 0) {
            sUnusable += s2;
        } else {
            sUsable += s2;
            bUsable = TRUE;
        }

        if ((dwNotUsableBy & 0x2000000) != 0) {
            sUnusable += s3;
        } else {
            sUsable += s3;
        }

        if ((dwNotUsableBy & 0x4000000) != 0) {
            sUnusable += s4;
        } else {
            sUsable += s4;
        }

        if ((dwNotUsableBy & 0x8000000) != 0) {
            sUnusable += s6;
        } else {
            sUsable += s6;
        }

        if ((dwNotUsableBy & 0x10000000) != 0) {
            sUnusable += s7;
        } else {
            sUsable += s7;
        }

        if ((dwNotUsableBy & 0x20000000) != 0) {
            sUnusable += s5;
        } else {
            sUsable += s5;
        }
    }

    sTemp = "  " + CBaldurEngine::FetchString(34) + "\n";
    if ((dwNotUsableBy & 0x1) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(1083) + "\n";
    if ((dwNotUsableBy & 0x2) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(1079) + "\n";
    if ((dwNotUsableBy & 0x4) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(1080) + "\n";
    if ((dwNotUsableBy & 0x8) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(16799) + "\n";
    if ((dwNotUsableBy & 0x10) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(33) + "\n";
    if ((dwNotUsableBy & 0x20) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(1078) + "\n";
    if ((dwNotUsableBy & 0x40) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(1077) + "\n";
    if ((dwNotUsableBy & 0x80) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(1082) + "\n";
    if ((dwNotUsableBy & 0x100) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
    }

    sTemp = "  " + CBaldurEngine::FetchString(32) + "\n";
    if ((dwNotUsableBy & 0x200) != 0) {
        if (!bUsable) {
            sUnusable += sTemp;
        }
    } else {
        sUsable += sTemp;
        bUsable = TRUE;
    }

    sTemp = "  " + CBaldurEngine::FetchString(9987) + "\n";
    sKit1 = "  " + CBaldurEngine::FetchString(502) + "\n";
    sKit2 = "  " + CBaldurEngine::FetchString(504) + "\n";
    sKit3 = "  " + CBaldurEngine::FetchString(2012) + "\n";
    sKit4 = "  " + CBaldurEngine::FetchString(2022) + "\n";
    sKit5 = "  " + CBaldurEngine::FetchString(12786) + "\n";
    sKit6 = "  " + CBaldurEngine::FetchString(12785) + "\n";
    sKit7 = "  " + CBaldurEngine::FetchString(12787) + "\n";
    sKit8 = "  " + CBaldurEngine::FetchString(12788) + "\n";

    if ((dwNotUsableBy & 0x400) != 0) {
        if ((dwNotUsableBy2 & 0x40) == 0) {
            sUsable += sKit1;
            bUsable = TRUE;
        }

        if ((dwNotUsableBy2 & 0x80) == 0) {
            sUsable += sKit2;
            bUsable = TRUE;
        }

        if ((dwNotUsableBy2 & 0x100) == 0) {
            sUsable += sKit3;
            bUsable = TRUE;
        }

        if ((dwNotUsableBy2 & 0x200) == 0) {
            sUsable += sKit4;
            bUsable = TRUE;
        }

        if ((dwNotUsableBy2 & 0x400) == 0) {
            sUsable += sKit5;
            bUsable = TRUE;
        }

        if ((dwNotUsableBy2 & 0x800) == 0) {
            sUsable += sKit6;
            bUsable = TRUE;
        }

        if ((dwNotUsableBy2 & 0x1000) == 0) {
            sUsable += sKit7;
            bUsable = TRUE;
        }

        if ((dwNotUsableBy2 & 0x2000) == 0) {
            sUsable += sKit8;
            bUsable = TRUE;
        } else {
            if (!bUsable) {
                sUnusable += sTemp;
            }
        }
    } else {
        if ((dwNotUsableBy2 & 0x40) != 0) {
            sUnusable += sKit1;
        }

        if ((dwNotUsableBy2 & 0x80) != 0) {
            sUnusable += sKit2;
        }

        if ((dwNotUsableBy2 & 0x100) != 0) {
            sUnusable += sKit3;
        }

        if ((dwNotUsableBy2 & 0x200) != 0) {
            sUnusable += sKit4;
        }

        if ((dwNotUsableBy2 & 0x400) != 0) {
            sUnusable += sKit5;
        }

        if ((dwNotUsableBy2 & 0x800) != 0) {
            sUnusable += sKit6;
        }

        if ((dwNotUsableBy2 & 0x1000) != 0) {
            sUnusable += sKit7;
        }

        if ((dwNotUsableBy2 & 0x2000) != 0) {
            sUnusable += sKit8;
        }

        sUsable += sTemp;
    }

    if (bUsable) {
        pEngine->UpdateTextForceColor(pText,
            rgbColor,
            "%s",
            static_cast<LPCSTR>(CBaldurEngine::FetchString(31531)));
        pEngine->UpdateTextNoTrim(pText, "%s", static_cast<LPCSTR>(sUsable));
    } else {
        pEngine->UpdateTextForceColor(pText,
            rgbColor,
            "%s",
            static_cast<LPCSTR>(CBaldurEngine::FetchString(31530)));
        pEngine->UpdateTextNoTrim(pText, "%s", static_cast<LPCSTR>(sUnusable));
    }
}

// 0x464130
CItem& CItem::operator=(const CItem& other)
{
    SetResRef(other.GetResRef(), TRUE);
    m_useCount1 = other.m_useCount1;
    m_useCount2 = other.m_useCount2;
    m_useCount3 = other.m_useCount3;
    m_wear = other.m_wear;
    m_flags = other.m_flags;
    return *this;
}

// 0x675320
bool CItem::operator==(const CItem& other)
{
    return m_useCount1 == other.m_useCount1
        && m_useCount2 == other.m_useCount2
        && m_useCount3 == other.m_useCount3
        && m_wear == other.m_wear
        && m_flags == other.m_flags
        && cResRef == other.cResRef;
}
