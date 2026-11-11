#include "CProjectile.h"

#include "CBaldurChitin.h"
#include "CGameContainer.h"
#include "CGameDoor.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CGameTrigger.h"
#include "CInfGame.h"
#include "CMessage.h"
#include "CProjectileArea.h"
#include "CProjectileBAM.h"
#include "CProjectilePart1.h"
#include "CProjectilePart2.h"
#include "CProjectilePart3.h"
#include "CProjectilePart4.h"
#include "CProjectilePart5.h"
#include "CProjectilePart6.h"
#include "CProjectileSingleTarget.h"
#include "CTimerWorld.h"
#include "CUtil.h"
#include "IcewindCVisualEffect.h"
#include "IcewindMisc.h"

// 0x6A3130
BOOLEAN CProjectile::IsProjectile()
{
    return TRUE;
}

// 0x529490
CProjectile::CProjectile()
{
    field_186 = 0;
    m_projectileType = 0;
    m_speed = 0;
    m_sourceId = 0;
    m_targetId = 0;
    field_9A = 0;
    field_E0 = 0;
    field_E6 = 0;
    field_EA = NULL;
    field_16C = 0;
    field_16E = 0;
    field_170 = 0;
    field_17C = 0;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_bHasHeight = FALSE;
    field_C4 = 0;
    field_E2 = 0;
    m_fireSoundRef = "";
    m_loopFireSound = FALSE;
    m_arrivalSoundRef = "";
    m_loopArrivalSound = FALSE;
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x51E9B0
CProjectile::~CProjectile()
{
    // NOTE: Uninline.
    ClearEffects();
}

// 0x536F40
BOOLEAN CProjectile::CompressTime(DWORD deltaTime)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
    // __LINE__: 10139
    UTIL_ASSERT(deltaTime > static_cast<DWORD>(CTimerWorld::TIMESCALE_MSEC_PER_SEC * CTimerWorld::TIMESCALE_SEC_PER_MIN));

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }

    return FALSE;
}

// 0x5551B0
void CProjectile::RemoveSelf()
{
    RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x51EA90
void CProjectile::AddEffect(CGameEffect* pEffect)
{
    pEffect->m_projectileType = m_projectileType;
    m_effectList.AddTail(pEffect);
}

// 0x51EAF0
CProjectile* CProjectile::DecodeProjectile(WORD projectileType, CGameAIBase* pSource, BOOL a3)
{
    CString sTemp1;
    CString sTemp2;
    CString sTemp3;
    IcewindCVisualEffect vfx;
    CProjectile* pProjectile = NULL;

    if (projectileType > 0x1000) {
        pProjectile = sub_560310(projectileType - 0x1001, NULL, FALSE);
        pProjectile->m_projectileType = projectileType - 1;
        return pProjectile;
    }

    switch (projectileType) {
    case 1: {
        pProjectile = new CProjectile84D8B8();
        break;
    }
    case 2: {
        CProjectileBAM84E58C* pBAM = new CProjectileBAM84E58C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 3: {
        CProjectileBAM84DCF4* pBAM = new CProjectileBAM84DCF4();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 4: {
        CProjectileBAM84E614* pBAM = new CProjectileBAM84E614();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_24";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 5: {
        CProjectileBAM84E58C* pBAM = new CProjectileBAM84E58C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(2, 57, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 6: {
        CProjectileBAM84E58C* pBAM = new CProjectileBAM84E58C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 7: {
        CProjectileBAM84E36C* pBAM = new CProjectileBAM84E36C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 8: {
        CProjectileBAM84DC68* pBAM = new CProjectileBAM84DC68();
        pBAM->SetVidCell(CResRef("AXE"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 9: {
        CProjectileBAM84E614* pBAM = new CProjectileBAM84E614();
        pBAM->SetVidCell(CResRef("AXE"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 10: {
        CProjectileBAM84E36C* pBAM = new CProjectileBAM84E36C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 11: {
        CProjectileBAM84E36C* pBAM = new CProjectileBAM84E36C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 12: {
        CProjectileBAM84E14C* pBAM = new CProjectileBAM84E14C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 13: {
        CProjectileBAM84DC68* pBAM = new CProjectileBAM84DC68();
        pBAM->SetVidCell(CResRef("BOLT"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 14: {
        CProjectileBAM84E614* pBAM = new CProjectileBAM84E614();
        pBAM->SetVidCell(CResRef("BOLT"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 15: {
        CProjectileBAM84E14C* pBAM = new CProjectileBAM84E14C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 16: {
        CProjectileBAM84E14C* pBAM = new CProjectileBAM84E14C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 17: {
        CProjectileBAM84E2E4* pBAM = new CProjectileBAM84E2E4();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 18: {
        CProjectileBAM84DC68* pBAM = new CProjectileBAM84DC68();
        pBAM->SetVidCell(CResRef("MAGICSTN"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 19: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 369
        UTIL_ASSERT(FALSE);
        break;
    }
    case 20: {
        CProjectileBAM84E2E4* pBAM = new CProjectileBAM84E2E4();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 21: {
        CProjectileBAM84E2E4* pBAM = new CProjectileBAM84E2E4();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 22: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "BHandsT");
        pSingleTarget->field_2CE = 200;
        pSingleTarget->field_2D2 = 80;
        pSingleTarget->field_317 = FALSE;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->field_2EE = 16;
        pSingleTarget->field_302 = 50;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_15";
        pProjectile = pSingleTarget;
        break;
    }
    case 23: {
        CProjectile84E8C0* pNew = new CProjectile84E8C0(CString("CallLiH"), CString(""), 255, 1, 0, 20);
        pNew->m_fireSoundRef = "EFF_P19";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 24: {
        CProjectileSingleTarget* pSingleTarget = new CProjectileSingleTarget(CResRef("COrbT"));
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_bHasHeight = TRUE;
        pSingleTarget->m_fireSoundRef = "TRA_23";
        pSingleTarget->m_arrivalSoundRef = "";
        pProjectile = pSingleTarget;
        break;
    }
    case 25: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "CoColdT");
        pSingleTarget->field_2CE = 200;
        pSingleTarget->field_2D2 = 350;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->m_speed *= 3;
        pSingleTarget->field_302 = 50;
        pSingleTarget->field_30E = 8;
        pSingleTarget->field_312 = 4;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_08";
        pProjectile = pSingleTarget;
        break;
    }
    case 26: {
        CProjectileBAM84E69C* pBAM = new CProjectileBAM84E69C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(0, 67, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        pBAM->m_fireSoundRef = "EFF_M20";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 27: {
        CProjectileBAM84E47C* pBAM = new CProjectileBAM84E47C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 28: {
        CProjectileBAM84DC68* pBAM = new CProjectileBAM84DC68();
        pBAM->SetVidCell(CResRef("DAGGER"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 29: {
        CProjectileBAM84E614* pBAM = new CProjectileBAM84E614();
        pBAM->SetVidCell(CResRef("DAGGER"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 30: {
        CProjectileBAM84E47C* pBAM = new CProjectileBAM84E47C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 31: {
        CProjectileBAM84E47C* pBAM = new CProjectileBAM84E47C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 32: {
        CProjectileBAM84E25C* pBAM = new CProjectileBAM84E25C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 33: {
        CProjectileBAM84DC68* pBAM = new CProjectileBAM84DC68();
        pBAM->SetVidCell(CResRef("DART"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 34: {
        CProjectileBAM84E614* pBAM = new CProjectileBAM84E614();
        pBAM->SetVidCell(CResRef("DART"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 35: {
        CProjectileBAM84E25C* pBAM = new CProjectileBAM84E25C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 36: {
        CProjectileBAM84E25C* pBAM = new CProjectileBAM84E25C();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 37: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_02";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 38: {
        pProjectile = new CProjectileArea84F580();
        break;
    }
    case 39: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 545
        UTIL_ASSERT(FALSE);
        break;
    }
    case 40: {
        CProjectileSingleTarget8513BC* pSingleTarget = new CProjectileSingleTarget8513BC("LightnT");
        pSingleTarget->field_2AE = 400;
        pSingleTarget->field_2B2.field_32 = 50;
        pSingleTarget->m_speed = 20;
        pSingleTarget->field_1D8 = 16;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_bHasHeight = TRUE;
        pSingleTarget->field_1C2 = 1;
        pSingleTarget->m_fireSoundRef = CResRef("TRA_09");
        pProjectile = pSingleTarget;
        break;
    }
    case 41: {
        CProjectileBAM84E2E4* pBAM = new CProjectileBAM84E2E4();
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(0, 28, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 42: {
        CProjectileArea84FEA4* pArea = new CProjectileArea84FEA4();
        if (pSource != NULL) {
            if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_NOTEVIL) {
                CAIObjectType type;
                type.m_nEnemyAlly = CAIObjectType::EA_EVILCUTOFF;
                pArea->field_2BE.Set(type);
            } else {
                CAIObjectType type;
                type.m_nEnemyAlly = CAIObjectType::EA_NOTEVIL;
                pArea->field_2BE.Set(type);
            }
        }
        pArea->m_fireSoundRef = "TRA_04C";
        pArea->m_arrivalSoundRef = "EFF_M05";
        pProjectile = pArea;
        break;
    }
    case 43: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 611
        UTIL_ASSERT(FALSE);
        break;
    }
    case 44: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 620
        UTIL_ASSERT(FALSE);
        break;
    }
    case 45: {
        sTemp1 = "Spsmkjet";
        sTemp2 = "";
        sTemp3 = "";
        CProjectile84D838* pNew = new CProjectile84D838(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 46: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 631
        UTIL_ASSERT(FALSE);
        break;
    }
    case 47: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL1"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 2;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_04A";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 48: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL2"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 4;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_04C";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 49: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL5"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 6;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 50: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL7"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 9;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_01";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 51: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 10;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 52: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 1;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 53: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 3;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 54: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL6"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 7;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 55: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL3"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 5;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_04D";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 56: {
        CProjectileBAM84E3F4* pBAM = new CProjectileBAM84E3F4();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 57: {
        CProjectileBAM84DC68* pBAM = new CProjectileBAM84DC68();
        pBAM->SetVidCell(CResRef("SPEAR"));
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 58: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 775
        UTIL_ASSERT(FALSE);
        break;
    }
    case 59: {
        CProjectileBAM84E3F4* pBAM = new CProjectileBAM84E3F4();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 60: {
        CProjectileBAM84E3F4* pBAM = new CProjectileBAM84E3F4();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 61: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 792
        UTIL_ASSERT(FALSE);
        break;
    }
    case 62: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 795
        UTIL_ASSERT(FALSE);
        break;
    }
    case 63: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 798
        UTIL_ASSERT(FALSE);
        break;
    }
    case 64: {
        pProjectile = new CProjectileArea8502E8();
        break;
    }
    case 65: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL2"));
        pBAM->field_1DE = 0;
        pBAM->field_E2 = 1;
        pBAM->field_9A = 10;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_speed *= 2;
        pBAM->m_fireSoundRef = "TRA_04C";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 66: {
        CProjectile84E8C0* pNew = new CProjectile84E8C0(CString("HMightH"), CString(""), 255, 1, 0, 2 * CTimerWorld::TIMESCALE_MSEC_PER_SEC);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 67: {
        pProjectile = new CProjectileArea84F754();
        break;
    }
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78: {
        CProjectileBAM84E0C4* pBAM = new CProjectileBAM84E0C4(projectileType - 67, CVidPalette::TYPE_RANGE);
        pBAM->m_fireSoundRef = "TRA_22";
        pBAM->m_arrivalSoundRef = "";
        pBAM->m_bHasHeight = TRUE;
        pProjectile = pBAM;
        break;
    }
    case 79: {
        pProjectile = new CProjectileBAM84DF1C();
        break;
    }
    case 80: {
        CProjectileBAM84DE0C* pBAM = new CProjectileBAM84DE0C((BYTE*)"SPFIREBL");
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_06";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91: {
        CProjectileBAM84E838* pBAM = new CProjectileBAM84E838(projectileType - 80, CVidPalette::TYPE_RANGE);
        if (pSource != NULL) {
            if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_NOTEVIL) {
                CAIObjectType type;
                type.m_nEnemyAlly = CAIObjectType::EA_EVILCUTOFF;
                pBAM->field_2B2.Set(type);
            } else {
                CAIObjectType type;
                type.m_nEnemyAlly = CAIObjectType::EA_NOTEVIL;
                pBAM->field_2B2.Set(type);
            }
        }
        pProjectile = pBAM;
        break;
    }
    case 92: {
        pProjectile = new CProjectileArea84F6B8();
        break;
    }
    case 93: {
        CProjectileBAM84DBDC* pBAM = new CProjectileBAM84DBDC(CResRef("MAGMISS"));
        pBAM->field_2A4 = 22;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 94: {
        CProjectileBAM84DBDC* pBAM = new CProjectileBAM84DBDC(CResRef("MAGMISS"));
        pBAM->field_2A4 = 78;
        pBAM->field_1DE = 0;
        pProjectile = pBAM;
        break;
    }
    case 95: {
        pProjectile = new CProjectileArea8501B0();
        break;
    }
    case 96: {
        CProjectileBAM84DD80* pBAM = new CProjectileBAM84DD80(CResRef((BYTE*)"sklT"), CResRef("ShSkull"), 1024);
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_2A0 = 100;
        pBAM->field_2A2 = 50;
        pBAM->m_arrivalSoundRef = "ARE_M06";
        pBAM->m_loopArrivalSound = TRUE;
        pBAM->field_4B4 = CString("EFF_M35");
        pProjectile = pBAM;
        break;
    }
    case 97: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "CSprayT");
        pSingleTarget->field_2CE = 200;
        pSingleTarget->field_2D2 = 200;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->field_302 = 30;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_16";
        pProjectile = pSingleTarget;
        break;
    }
    case 98: {
        pProjectile = new CProjectileArea84FAFC();
        break;
    }
    case 99: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 1002
        UTIL_ASSERT(FALSE);
        break;
    }
    case 100: {
        CProjectileBAM84DD80* pBAM = new CProjectileBAM84DD80(CResRef((BYTE*)"glphwrdT"), CResRef("ShGlyph"), 1040);
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_2A0 = 80;
        pBAM->field_2A2 = 50;
        pBAM->m_fireSoundRef = "EFF_P22a";
        pBAM->m_loopFireSound = TRUE;
        pBAM->m_arrivalSoundRef = "";
        pBAM->field_4B4 = CString("EFF_P22b");
        pBAM->field_1C6 = 1;
        pProjectile = pBAM;
        break;
    }
    case 101: {
        pProjectile = new CProjectileArea84F7F0();
        break;
    }
    case 102: {
        CProjectileBAM84E614* pBAM = new CProjectileBAM84E614();
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(0, 69, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        memset(pBAM->field_2A0, 'E', 7);
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 103: {
        CProjectileBAM84E614* pBAM = new CProjectileBAM84E614();
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(0, 68, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        memset(pBAM->field_2A0, 'D', 7);
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 104: {
        CProjectileBAM84DC68* pBAM = new CProjectileBAM84DC68();
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(0, 69, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        memset(pBAM->field_4B2, 'E', 7);
        pBAM->field_4BA = 69;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_06";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 105: {
        CProjectileBAM84DE0C* pBAM = new CProjectileBAM84DE0C((BYTE*)"SPFIREBL");
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(0, 68, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_06";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 108: {
        CProjectileBAM84E504* pBAM = new CProjectileBAM84E504();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 109: {
        CProjectileSingleTarget850E84* pSingleTarget = new CProjectileSingleTarget850E84();
        pSingleTarget->m_fireSoundRef = "TRA_17";
        pProjectile = pSingleTarget;
        break;
    }
    case 110: {
        CProjectile84E940* pNew = new CProjectile84E940(CString("SPDimDr"), CString(""), CString(""), 0, 0);
        pNew->m_fireSoundRef = "EFF_M09";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 111: {
        BYTE nDuration = static_cast<BYTE>(a3) != 0 ? static_cast<BYTE>(a3) : 50;
        pProjectile = new CProjectile850F9C(CString("NecroCG"), CString(""), nDuration, 0, vfx);
        break;
    }
    case 112: {
        BYTE nDuration = static_cast<BYTE>(a3) != 0 ? static_cast<BYTE>(a3) : 50;
        pProjectile = new CProjectile850F9C(CString("AlterCG"), CString(""), nDuration, 0, vfx);
        break;
    }
    case 113: {
        BYTE nDuration = static_cast<BYTE>(a3) != 0 ? static_cast<BYTE>(a3) : 50;
        pProjectile = new CProjectile850F9C(CString("EnchaCG"), CString(""), nDuration, 0, vfx);
        break;
    }
    case 114: {
        BYTE nDuration = static_cast<BYTE>(a3) != 0 ? static_cast<BYTE>(a3) : 50;
        pProjectile = new CProjectile850F9C(CString("AbjurCG"), CString(""), nDuration, 0, vfx);
        break;
    }
    case 115: {
        BYTE nDuration = static_cast<BYTE>(a3) != 0 ? static_cast<BYTE>(a3) : 50;
        pProjectile = new CProjectile850F9C(CString("IllusCG"), CString(""), nDuration, 0, vfx);
        break;
    }
    case 116: {
        BYTE nDuration = static_cast<BYTE>(a3) != 0 ? static_cast<BYTE>(a3) : 50;
        pProjectile = new CProjectile850F9C(CString("ConjuCG"), CString(""), nDuration, 0, vfx);
        break;
    }
    case 117: {
        BYTE nDuration = static_cast<BYTE>(a3) != 0 ? static_cast<BYTE>(a3) : 50;
        pProjectile = new CProjectile850F9C(CString("InvocCG"), CString(""), nDuration, 0, vfx);
        break;
    }
    case 118: {
        BYTE nDuration = static_cast<BYTE>(a3) != 0 ? static_cast<BYTE>(a3) : 50;
        pProjectile = new CProjectile850F9C(CString("DivinCG"), CString(""), nDuration, 0, vfx);
        break;
    }
    case 119: {
        sTemp1 = "SHAir";
        sTemp2 = "";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 120: {
        sTemp1 = "SHEarth";
        sTemp2 = "";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 121: {
        sTemp1 = "SHWater";
        sTemp2 = "";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 122: {
        sTemp1 = "SHAir";
        sTemp2 = "SHAir1";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 123: {
        sTemp1 = "SHEarth";
        sTemp2 = "SHEarth1";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 124: {
        sTemp1 = "SHWater";
        sTemp2 = "SHWater1";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 125: {
        sTemp1 = "SHAir";
        sTemp2 = "SHAir2";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 126: {
        sTemp1 = "SHEarth";
        sTemp2 = "SHEarth2";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 127: {
        sTemp1 = "SHWater";
        sTemp2 = "SHWater2";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 128: {
        sTemp1 = "SHAir";
        sTemp2 = "SHAir3";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 129: {
        sTemp1 = "SHEarth";
        sTemp2 = "SHEarth3";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 130: {
        sTemp1 = "SHWater";
        sTemp2 = "SHWater3";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 131: {
        sTemp1 = "SHAir";
        sTemp2 = "SHAir4";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 132: {
        sTemp1 = "SHEarth";
        sTemp2 = "SHEarth4";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 133: {
        sTemp1 = "SHWater";
        sTemp2 = "SHWater4";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 134: {
        sTemp1 = "SHAir";
        sTemp2 = "SHAir5";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 135: {
        sTemp1 = "SHEarth";
        sTemp2 = "SHEarth5";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 136: {
        sTemp1 = "SHWater";
        sTemp2 = "SHWater5";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 137: {
        sTemp1 = "SHAir";
        sTemp2 = "SHAir6";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 138: {
        sTemp1 = "SHEarth";
        sTemp2 = "SHEarth6";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 139: {
        sTemp1 = "SHWater";
        sTemp2 = "SHWater6";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 140: {
        sTemp1 = "SHAir";
        sTemp2 = "SHAir7";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 141: {
        sTemp1 = "SHEarth";
        sTemp2 = "SHEarth7";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 142: {
        sTemp1 = "SHWater";
        sTemp2 = "SHWater7";
        sTemp3 = "";
        pProjectile = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pProjectile->m_fireSoundRef = "";
        pProjectile->m_arrivalSoundRef = "";
        break;
    }
    case 143: {
        CProjectileBAM84DFA4* pBAM = new CProjectileBAM84DFA4();
        pBAM->field_1D8 = 1;
        pBAM->SetPaletteRange(0, 67);
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 144: {
        CProjectileBAM84DFA4* pBAM = new CProjectileBAM84DFA4();
        pBAM->field_1D8 = 1;
        pBAM->SetPaletteRange(0, 65);
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 145: {
        CProjectileBAM84DFA4* pBAM = new CProjectileBAM84DFA4();
        pBAM->field_1D8 = 1;
        pBAM->SetPaletteRange(0, 69);
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 146: {
        CProjectile84E8C0* pNew = new CProjectile84E8C0(CString("FlmStrk"), CString(""), 255, 0, 0, 2 * CTimerWorld::TIMESCALE_MSEC_PER_SEC);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 147: {
        CProjectile84E8C0* pNew = new CProjectile84E8C0(CString("HLYMITE"), CString(""), 255, 1, 0, 2 * CTimerWorld::TIMESCALE_MSEC_PER_SEC);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 148: {
        sTemp1 = "SHAir";
        sTemp2 = "SHAir7";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 149: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL1"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 2;
        pBAM->field_2A4 = 46;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "TRA_04A";
        pBAM->m_arrivalSoundRef = "CAS_M08";
        pProjectile = pBAM;
        break;
    }
    case 150: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL2"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 4;
        pBAM->field_2A4 = 47;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "TRA_04C";
        pBAM->m_arrivalSoundRef = "EFF_M05";
        pProjectile = pBAM;
        break;
    }
    case 151: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL5"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 6;
        pBAM->field_2A4 = 48;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "TRA_04B";
        pBAM->m_arrivalSoundRef = "CAS_P03";
        pProjectile = pBAM;
        break;
    }
    case 152: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL7"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 9;
        pBAM->field_2A4 = 49;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 153: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 10;
        pBAM->field_2A4 = 50;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 154: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 1;
        pBAM->field_2A4 = 51;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 155: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 3;
        pBAM->field_2A4 = 52;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 156: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL6"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 7;
        pBAM->field_2A4 = 53;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 157: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL3"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 5;
        pBAM->field_2A4 = 54;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "TRA_04A";
        pBAM->m_arrivalSoundRef = "CAS_P02";
        pProjectile = pBAM;
        break;
    }
    case 158: {
        CProjectileBAM84DBDC* pBAM = new CProjectileBAM84DBDC(CResRef("MAGMISS"));
        pBAM->field_2A4 = 78;
        pBAM->field_1DE = 0;
        if (pSource != NULL) {
            if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
                CAIObjectType type;
                type.m_nEnemyAlly = CAIObjectType::EA_GOODCUTOFF;
                pBAM->field_2B4.Set(type);
            } else {
                CAIObjectType type;
                type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
                pBAM->field_2B4.Set(type);
            }
        }
        pProjectile = pBAM;
        break;
    }
    case 159: {
        CProjectileBAM84DBDC* pBAM = new CProjectileBAM84DBDC(CResRef("MAGMISS"));
        pBAM->field_2A4 = 78;
        pBAM->field_1DE = 0;
        if (pSource != NULL) {
            if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
                CAIObjectType type;
                type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
                pBAM->field_2B4.Set(type);
            } else {
                CAIObjectType type;
                type.m_nEnemyAlly = CAIObjectType::EA_GOODCUTOFF;
                pBAM->field_2B4.Set(type);
            }
        }
        pProjectile = pBAM;
        break;
    }
    case 160:
    case 161:
    case 162:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
    case 168: {
        CProjectileBAM84DBDC* pBAM = static_cast<CProjectileBAM84DBDC*>(CProjectile::DecodeProjectile(projectileType - 11, pSource, FALSE));
        if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_GOODCUTOFF;
            pBAM->field_2B4.Set(type);
        } else {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
            pBAM->field_2B4.Set(type);
        }
        pProjectile = pBAM;
        break;
    }
    case 169:
    case 170:
    case 171:
    case 172:
    case 173:
    case 174:
    case 175:
    case 176:
    case 177: {
        CProjectileBAM84DBDC* pBAM = static_cast<CProjectileBAM84DBDC*>(CProjectile::DecodeProjectile(projectileType - 20, pSource, FALSE));
        if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
            pBAM->field_2B4.Set(type);
        } else {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_GOODCUTOFF;
            pBAM->field_2B4.Set(type);
        }
        pProjectile = pBAM;
        break;
    }
    case 178: {
        CProjectileBAM84DBDC* pBAM = static_cast<CProjectileBAM84DBDC*>(CProjectile::DecodeProjectile(182, pSource, FALSE));
        if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
            pBAM->field_2B4.Set(type);
        } else {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_GOODCUTOFF;
            pBAM->field_2B4.Set(type);
        }
        pProjectile = pBAM;
        break;
    }
    case 179: {
        CProjectileBAM84DBDC* pBAM = static_cast<CProjectileBAM84DBDC*>(CProjectile::DecodeProjectile(183, pSource, FALSE));
        if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
            pBAM->field_2B4.Set(type);
        } else {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_GOODCUTOFF;
            pBAM->field_2B4.Set(type);
        }
        pProjectile = pBAM;
        break;
    }
    case 180: {
        CProjectileBAM84DBDC* pBAM = static_cast<CProjectileBAM84DBDC*>(CProjectile::DecodeProjectile(182, pSource, FALSE));
        if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_GOODCUTOFF;
            pBAM->field_2B4.Set(type);
        } else {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
            pBAM->field_2B4.Set(type);
        }
        pProjectile = pBAM;
        break;
    }
    case 181: {
        CProjectileBAM84DBDC* pBAM = static_cast<CProjectileBAM84DBDC*>(CProjectile::DecodeProjectile(183, pSource, FALSE));
        if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_GOODCUTOFF;
            pBAM->field_2B4.Set(type);
        } else {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
            pBAM->field_2B4.Set(type);
        }
        pProjectile = pBAM;
        break;
    }
    case 182: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL4"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 11;
        pBAM->field_2A4 = 54;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 183: {
        CProjectileBAM84D938* pBAM = new CProjectileBAM84D938(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL8"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 12;
        pBAM->field_2A4 = 184;
        pBAM->m_bHasHeight = TRUE;
        pBAM->field_E6 |= 0x8;
        pBAM->m_fireSoundRef = "TRA_03";
        pBAM->m_arrivalSoundRef = "EFF_M04";
        pProjectile = pBAM;
        break;
    }
    case 184: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL4"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 11;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_04A";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 185: {
        CProjectileBAM84DB54* pBAM = new CProjectileBAM84DB54(CVidPalette::TYPE_RESOURCE);
        pBAM->SetVidCell(CResRef("TRAVEL"));
        pBAM->SetBitmap(CString("STTRAVL8"));
        pBAM->field_E2 = 1;
        pBAM->field_9A = 12;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_03";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 186: {
        CProjectileBAM84DBDC* pBAM = static_cast<CProjectileBAM84DBDC*>(CProjectile::DecodeProjectile(94, pSource, FALSE));
        pBAM->field_2F2 = 1;
        pProjectile = pBAM;
        break;
    }
    case 187: {
        pProjectile = new CProjectileArea84F310();
        break;
    }
    case 188: {
        CProjectileBAM84E614* pBAM = new CProjectileBAM84E614();
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(0, 71, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        memset(pBAM->field_2A0, 'G', 7);
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_10";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 189: {
        CProjectileBAM84DA4C* pBAM = new CProjectileBAM84DA4C(CResRef("ACOWG1"));
        pBAM->field_1D8 = 1;
        pBAM->m_fireSoundRef = "AMB_E42A";
        pBAM->m_arrivalSoundRef = "AMB_E42A";
        pBAM->m_speed *= 2;
        pBAM->m_bHasHeight = TRUE;
        pProjectile = pBAM;
        break;
    }
    case 190: {
        CProjectileArea84F9C4* pArea = new CProjectileArea84F9C4();
        pArea->sub_5704E0(pSource);
        pArea->m_fireSoundRef = CResRef("TRA_04C");
        pArea->m_arrivalSoundRef = CResRef("EFF_M05");
        pProjectile = pArea;
        break;
    }
    case 191: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 1684
        UTIL_ASSERT(FALSE);
        break;
    }
    case 192: {
        CProjectileBAM84E504* pBAM = new CProjectileBAM84E504();
        pBAM->m_bHasHeight = TRUE;
        pBAM->SetPaletteRange(0, 86);
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 193: {
        CProjectileBAM84E504* pBAM = new CProjectileBAM84E504();
        pBAM->m_bHasHeight = TRUE;
        pBAM->SetPaletteRange(0, 110);
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 194: {
        CProjectileBAM84E504* pBAM = new CProjectileBAM84E504();
        pBAM->m_bHasHeight = TRUE;
        pBAM->SetPaletteRange(0, 26);
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 195: {
        CProjectile84E8C0* pNew = new CProjectile84E8C0(CString("HLYMITE"), CString(""), 255, 1, 0, 2 * CTimerWorld::TIMESCALE_MSEC_PER_SEC);
        pNew->m_vidCell.SetTintColor(RGB(255, 0, 0));
        pNew->m_fireSoundRef = "EFF_M10";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 196: {
        sTemp1 = "SHAREA";
        sTemp2 = "";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 197: {
        sTemp1 = "SHAREA";
        sTemp2 = "SHAREA1";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 198: {
        sTemp1 = "SHAREA";
        sTemp2 = "";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 199: {
        sTemp1 = "SHAREA";
        sTemp2 = "SHAREA3";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 200: {
        sTemp1 = "SHAREA";
        sTemp2 = "SHAREA4";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 201: {
        sTemp1 = "SHAREA";
        sTemp2 = "SHAREA5";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 202: {
        sTemp1 = "SHAREA";
        sTemp2 = "SHAREA6";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 203: {
        sTemp1 = "SHAREA";
        sTemp2 = "SHAREA2";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 204: {
        CProjectileBAM84DC68* pBAM = new CProjectileBAM84DC68();
        pBAM->m_pVidCell->SetPalette(pBAM->m_palette);
        pBAM->m_palette.SetRange(0, 71, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        memset(pBAM->field_4B2, 'G', 7);
        pBAM->field_4BA = 71;
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_06";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 205: {
        CProjectileBAM84DBDC* pBAM = new CProjectileBAM84DBDC(CResRef("MAGMISS"));
        pBAM->field_2A4 = 78;
        pBAM->field_1DE = 0;
        pBAM->field_2A0 = 80;
        pBAM->field_2A2 = 80;
        pProjectile = pBAM;
        break;
    }
    case 206: {
        CProjectileBAM84DE94* pBAM = new CProjectileBAM84DE94();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_09";
        pBAM->m_arrivalSoundRef = "";
        pBAM->field_2A0 = 1;
        pProjectile = pBAM;
        break;
    }
    case 207: {
        CProjectileSingleTarget85101C* pSingleTarget = new CProjectileSingleTarget85101C();
        pSingleTarget->field_2B2 = 1;
        pProjectile = pSingleTarget;
        break;
    }
    case 208: {
        sTemp1 = "SPFDEATH";
        sTemp2 = "";
        sTemp3 = "";
        CProjectile84E9C0* pNew = new CProjectile84E9C0(sTemp1, sTemp3, sTemp2, 8, 0);
        pNew->m_fireSoundRef = "";
        pNew->m_arrivalSoundRef = "";
        pProjectile = pNew;
        break;
    }
    case 209: {
        CProjectileArea8504BC* pArea = new CProjectileArea8504BC(150);
        pArea->field_4E2.field_0 = "MRageX";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 210: {
        pProjectile = new CProjectileSingleTarget850F14(std::set<LONG>());
        break;
    }
    case 211: {
        pProjectile = new CProjectileArea84F274();
        break;
    }
    case 212: {
        pProjectile = new CProjectileArea84F3AC();
        break;
    }
    case 213: {
        pProjectile = new CProjectileArea84FFDC();
        break;
    }
    case 214: {
        pProjectile = new CProjectileArea84FB98();
        break;
    }
    case 215: {
        pProjectile = new CProjectileArea84FE08();
        break;
    }
    case 216: {
        pProjectile = new CProjectileArea84FC34();
        break;
    }
    case 217: {
        pProjectile = new CProjectileArea84FF40();
        break;
    }
    case 218: {
        CProjectileBAM8510A4* pBAM = new CProjectileBAM8510A4(CVidPalette::TYPE_RESOURCE);
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_02";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 227: {
        pProjectile = new CProjectileSingleTarget8511AC("AbjurT");
        break;
    }
    case 228: {
        pProjectile = new CProjectileSingleTarget8511AC("AlterT");
        break;
    }
    case 229: {
        pProjectile = new CProjectileSingleTarget8511AC("InvocT");
        break;
    }
    case 230: {
        pProjectile = new CProjectileSingleTarget8511AC("NecroT");
        break;
    }
    case 231: {
        pProjectile = new CProjectileSingleTarget8511AC("ConjuT");
        break;
    }
    case 232: {
        pProjectile = new CProjectileSingleTarget8511AC("EnchaT");
        break;
    }
    case 233: {
        pProjectile = new CProjectileSingleTarget8511AC("IllusT");
        break;
    }
    case 234: {
        pProjectile = new CProjectileSingleTarget8511AC("DivinT");
        break;
    }
    case 235: {
        pProjectile = new CProjectileArea84F4E4();
        break;
    }
    case 236: {
        CProjectileArea* pArea = new CProjectileArea(250);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "Area1X";
        pArea->field_4E2.field_10 = "EFF_P99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 237: {
        CProjectileArea* pArea = new CProjectileArea(250);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "Area1X";
        pArea->field_4E2.field_10 = "EFF_P99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 238: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "AbjuraX";
        pArea->field_4E2.field_10 = "ARE_M20";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 239: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->field_4E2.field_0 = "Area2X";
        pArea->field_4E2.field_10 = "EFF_M99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 240: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "Area2X";
        pArea->field_4E2.field_10 = "EFF_M99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 241: {
        CProjectileArea* pArea = new CProjectileArea(150);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "Area4X";
        pArea->field_4E2.field_10 = "EFF_M99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 242: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "AbjuraX";
        pArea->field_4E2.field_10 = "ARE_M20";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 243: {
        CProjectileArea* pArea = new CProjectileArea(350);
        pArea->field_4E2.field_0 = "Area1X";
        pArea->field_4E2.field_10 = "EFF_P99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 244: {
        CProjectileArea* pArea = new CProjectileArea(250);
        pArea->field_300 = 1;
        pProjectile = pArea;
        break;
    }
    case 245: {
        CProjectileArea* pArea = new CProjectileArea(150);
        pArea->field_4E2.field_0 = "AlteraX";
        pArea->field_4E2.field_10 = "ARE_M19";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 246: {
        pProjectile = new CProjectileArea850C0C();
        break;
    }
    case 247: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "AlteraX";
        pArea->field_4E2.field_10 = "ARE_M19";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 248: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "AlteraX";
        pArea->field_4E2.field_10 = "ARE_M19";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 249: {
        CProjectileArea84F88C* pArea = new CProjectileArea84F88C();
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 250: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "AbjuraX";
        pArea->field_4E2.field_10 = "ARE_M20";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 251: {
        CProjectileSingleTarget* pSingleTarget = new CProjectileSingleTarget(CResRef("IcelanT"));
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_1D8 = 16;
        pSingleTarget->m_fireSoundRef = "TRA_19";
        pProjectile = pSingleTarget;
        break;
    }
    case 252: {
        CProjectileArea* pArea = new CProjectileArea(250);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "AlteraX";
        pArea->field_4E2.field_10 = "ARE_M19";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 253: {
        CProjectileArea* pArea = new CProjectileArea(600);
        pArea->field_4E2.field_0 = "Area1X";
        pArea->field_4E2.field_10 = "EFF_P99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 254: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "EnchanX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 255: {
        CProjectileArea* pArea = new CProjectileArea(150);
        pArea->field_4E2.field_0 = "EnchanX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 256: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "EnchanX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 257: {
        CProjectileArea* pArea = new CProjectileArea(100);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "EnchanX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 258: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "AbjuraX";
        pArea->field_4E2.field_10 = "ARE_M20";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 260: {
        CProjectileArea* pArea = new CProjectileArea(600);
        pArea->field_4E2.field_0 = "Area1X";
        pArea->field_4E2.field_10 = "EFF_P99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 261: {
        pProjectile = new CProjectile85112C();
        break;
    }
    case 262: {
        CProjectileArea850078* pArea = new CProjectileArea850078();
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 263: {
        CProjectileArea84F928* pArea = new CProjectileArea84F928();
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 264: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "EnchanX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 266: {
        CProjectileArea* pArea = new CProjectileArea(350);
        pArea->field_4E2.field_0 = "AbjuraX";
        pArea->field_4E2.field_10 = "ARE_M20";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pArea->sub_5703F0(pSource);
        pProjectile = pArea;
        break;
    }
    case 267: {
        pProjectile = new CProjectileArea84F448();
        break;
    }
    case 268: {
        CProjectileSingleTarget* pSingleTarget = new CProjectileSingleTarget(CResRef("DisintT"));
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_1D8 = 16;
        pSingleTarget->m_fireSoundRef = "TRA_20";
        pProjectile = pSingleTarget;
        break;
    }
    case 269: {
        CProjectileSingleTarget* pSingleTarget = new CProjectileSingleTarget(CResRef("OFSpheT"));
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_1D8 = 16;
        pProjectile = pSingleTarget;
        break;
    }
    case 270: {
        pProjectile = new CProjectileArea84F61C();
        break;
    }
    case 271: {
        CProjectileSingleTarget* pSingleTarget = new CProjectileSingleTarget(CResRef("OFSpheT"));
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_1D8 = 16;
        pProjectile = pSingleTarget;
        break;
    }
    case 272: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "PSprayT");
        pSingleTarget->field_2CE = 175;
        pSingleTarget->field_2D2 = 400;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->field_302 = 30;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_21";
        pProjectile = pSingleTarget;
        break;
    }
    case 273: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "Area3X";
        pArea->field_4E2.field_10 = "EFF_M99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 274: {
        pProjectile = new CProjectileArea85024C();
        break;
    }
    case 275: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "EnchanX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 276: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "SoPainX";
        pArea->field_4E2.field_10 = "EFF_P49";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 277: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "SoHopeX";
        pArea->field_4E2.field_10 = "EFF_P50";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 278: {
        CProjectileArea84FD6C* pArea = new CProjectileArea84FD6C();
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 279: {
        pProjectile = new CProjectileArea84FCD0();
        break;
    }
    case 280: {
        // NOTE: The value is read from the global at 0x84F06C.
        pProjectile = new CProjectileArea(100);
        break;
    }
    case 281: {
        pProjectile = new CProjectileArea(250);
        break;
    }
    case 282: {
        IcewindMisc::sub_584880(static_cast<CGameSprite*>(pSource), 4388, 0);
        CProjectileArea* pArea = new CProjectileArea(100);
        pArea->field_50E.field_0 = "SCloudR";
        pArea->field_50E.field_20.sub_586A90(TRUE);
        pArea->field_50E.field_2C = TRUE;
        pArea->field_50E.field_2E = 7;
        pProjectile = pArea;
        break;
    }
    case 283: {
        pProjectile = new CProjectileArea850384();
        break;
    }
    case 284: {
        IcewindMisc::sub_584880(static_cast<CGameSprite*>(pSource), 4387, 0);
        pProjectile = new CProjectileArea84F310();
        break;
    }
    case 285: {
        IcewindMisc::sub_584880(static_cast<CGameSprite*>(pSource), 4386, 0);
        pProjectile = new CProjectileSingleTarget(CResRef("MSporeT"));
        break;
    }
    case 286: {
        CProjectileArea* pArea = new CProjectileArea(150);
        pArea->field_540.field_0 = "ICloudB";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_36 = TRUE;
        pArea->field_540.field_38 = 500;
        pProjectile = pArea;
        break;
    }
    case 287: {
        CProjectileArea* pArea = new CProjectileArea(150);
        pArea->field_540.field_0 = "ICloudA";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_38 = 500;
        pArea->field_4C0 = 200;
        pProjectile = pArea;
        break;
    }
    case 288: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 289: {
        vfx.sub_586A90(TRUE);
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm1X", vfx);
        pSingleTarget->field_2AE = TRUE;
        pSingleTarget->m_arrivalSoundRef = "EFF_M13";
        pProjectile = pSingleTarget;
        break;
    }
    case 290: {
        vfx.sub_586A90(TRUE);
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ASumm1X", vfx);
        pSingleTarget->field_2AE = TRUE;
        pSingleTarget->m_arrivalSoundRef = "EFF_M13";
        pProjectile = pSingleTarget;
        break;
    }
    case 291: {
        vfx.sub_586A90(TRUE);
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CEElemX", vfx);
        pSingleTarget->field_2AE = TRUE;
        pSingleTarget->m_arrivalSoundRef = "EFF_M13";
        pProjectile = pSingleTarget;
        break;
    }
    case 292: {
        vfx.sub_586A90(TRUE);
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CFElemX", vfx);
        pSingleTarget->field_2AE = TRUE;
        pSingleTarget->m_arrivalSoundRef = "EFF_M13";
        pProjectile = pSingleTarget;
        break;
    }
    case 293: {
        vfx.sub_586A90(TRUE);
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CWElemX", vfx);
        pSingleTarget->field_2AE = TRUE;
        pSingleTarget->m_arrivalSoundRef = "EFF_M13";
        pProjectile = pSingleTarget;
        break;
    }
    case 294: {
        CProjectileArea850420* pArea = new CProjectileArea850420();
        pArea->field_4E2.field_10 = "PRTL_OP";
        pProjectile = pArea;
        break;
    }
    case 295: {
        CProjectileArea* pArea = new CProjectileArea(250);
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 296: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "CoColdT");
        pSingleTarget->field_2CE = 200;
        pSingleTarget->field_2D2 = 120;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->m_speed *= 2;
        pSingleTarget->field_302 = 50;
        pSingleTarget->field_30E = 8;
        pSingleTarget->field_312 = 4;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_08";
        pProjectile = pSingleTarget;
        break;
    }
    case 297: {
        CProjectileArea850420* pArea = new CProjectileArea850420();
        pArea->field_4E2.field_10 = "PRTL_CL";
        pProjectile = pArea;
        break;
    }
    case 298: {
        CProjectileSingleTarget* pSingleTarget = new CProjectileSingleTarget(CResRef("ALanceT"));
        pSingleTarget->m_speed *= 2;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_1D8 = 16;
        pSingleTarget->field_2A0 = FALSE;
        pSingleTarget->m_fireSoundRef = "TRA_57";
        pProjectile = pSingleTarget;
        break;
    }
    case 299: {
        CProjectileArea* pArea = new CProjectileArea(120);
        pArea->field_540.field_0 = "SEaterA";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_10 = "ARE_M18";
        pArea->field_540.field_38 = 200;
        pProjectile = pArea;
        break;
    }
    case 300: {
        CProjectileArea* pArea = new CProjectileArea(150);
        pArea->field_4E2.field_0 = "SGrowtX";
        pArea->field_4E2.field_10 = "EFF_P111";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pArea->field_540.field_0 = "SGrowtA";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_10 = "AFT_P24";
        pArea->field_540.field_38 = 250;
        pArea->field_4D4 = 10;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 2000;
        pProjectile = pArea;
        break;
    }
    case 301: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->field_540.field_0 = "CloudBA";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_10 = "ARE_P24";
        pArea->field_540.field_38 = 100;
        pArea->field_4D4 = 10;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 200;
        pProjectile = pArea;
        break;
    }
    case 302: {
        CProjectileSingleTarget8513BC* pSingleTarget = new CProjectileSingleTarget8513BC("SWaveX");
        pSingleTarget->m_speed = 10;
        pSingleTarget->field_1D8 = 16;
        pSingleTarget->field_2A0 = FALSE;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_bHasHeight = FALSE;
        pSingleTarget->field_1C2 = 0;
        pSingleTarget->field_2B2.field_32 = 50;
        pSingleTarget->field_2AE = 400;
        pSingleTarget->m_fireSoundRef = "TRA_56";
        pProjectile = pSingleTarget;
        break;
    }
    case 303: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "TSprayT");
        pSingleTarget->field_2CE = 600;
        pSingleTarget->field_2D2 = 300;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->field_317 = FALSE;
        pSingleTarget->field_2EE = 16;
        pSingleTarget->m_speed *= 3;
        pSingleTarget->field_302 = 35;
        pSingleTarget->field_30E = 8;
        pSingleTarget->field_312 = 4;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 304: {
        pProjectile = new CProjectileSingleTarget8514CC();
        break;
    }
    case 305: {
        pProjectile = new CProjectileSingleTarget851444();
        break;
    }
    case 306: {
        pProjectile = new CProjectileArea850558();
        break;
    }
    case 307: {
        CProjectileArea* pArea = new CProjectileArea(120);
        pArea->field_4E2.field_10 = "EFF_P104";
        pArea->field_540.field_0 = "MoEldaA";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_38 = 1200;
        pProjectile = pArea;
        break;
    }
    case 308: {
        CProjectileArea8504BC* pArea = new CProjectileArea8504BC(70);
        pArea->field_540.field_10 = "EFF_P101";
        pArea->field_4D4 = 5;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 20;
        pArea->field_4C0 = 300;
        pProjectile = pArea;
        break;
    }
    case 309: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->field_540.field_0 = "CoPestA";
        pArea->field_540.field_20.sub_586AC0(TRUE, 128);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_10 = "ARE_P25";
        pArea->field_540.field_38 = 175;
        pArea->field_4D4 = 10;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 400;
        pProjectile = pArea;
        break;
    }
    case 310: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->field_4E2.field_10 = "ARE_P28";
        pArea->field_540.field_10 = "AFT_P27";
        pArea->field_4D4 = 10;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10000;
        pArea->field_4C0 = 1000;
        pProjectile = pArea;
        break;
    }
    case 311: {
        CProjectileArea8504BC* pArea = new CProjectileArea8504BC(70);
        pArea->field_4D4 = 5;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 20;
        pArea->field_540.field_10 = "EFF_P100";
        pArea->field_4C0 = 200;
        pProjectile = pArea;
        break;
    }
    case 312: {
        pProjectile = new CProjectile85133C();
        break;
    }
    case 313: {
        CProjectileSingleTarget8513BC* pSingleTarget = new CProjectileSingleTarget8513BC("LoDisrT");
        pSingleTarget->m_speed = 20;
        pSingleTarget->field_1D8 = 16;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_bHasHeight = TRUE;
        pSingleTarget->field_1C2 = 1;
        pSingleTarget->field_2B2.field_32 = 50;
        pSingleTarget->field_2AE = 600;
        pSingleTarget->m_fireSoundRef = "TRA_59";
        pProjectile = pSingleTarget;
        break;
    }
    case 314: {
        pProjectile = new CProjectileSingleTarget851554();
        break;
    }
    case 315: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "ShoutT");
        pSingleTarget->field_2CE = 100;
        pSingleTarget->field_2D2 = 300;
        pSingleTarget->field_2EE = 16;
        pSingleTarget->m_speed = 3 * pSingleTarget->m_speed / 2;
        pSingleTarget->field_302 = 1000;
        pSingleTarget->field_306 = 75;
        pSingleTarget->field_30A = 0;
        pSingleTarget->field_30E = 12;
        pSingleTarget->field_312 = 2;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_08";
        pProjectile = pSingleTarget;
        break;
    }
    case 316: {
        CProjectileSingleTarget* pSingleTarget = new CProjectileSingleTarget(CResRef("VSpherT"));
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_speed *= 2;
        pSingleTarget->field_1D8 = 1;
        pSingleTarget->m_bHasHeight = TRUE;
        pSingleTarget->field_1C2 = 1;
        pSingleTarget->m_fireSoundRef = "TRA_60";
        pSingleTarget->m_arrivalSoundRef = "";
        pProjectile = pSingleTarget;
        break;
    }
    case 317: {
        CProjectileArea* pArea = new CProjectileArea(120);
        pArea->field_540.field_0 = "SuffocA";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_10 = "AFT_M18";
        pArea->field_540.field_38 = 1000;
        pArea->field_4D4 = 10;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 400;
        pProjectile = pArea;
        break;
    }
    case 318: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->field_4E2.field_0 = "ADHWilX";
        pArea->field_4E2.field_10 = "ARE_M17";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 319: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "GShoutT");
        pSingleTarget->field_2CE = 100;
        pSingleTarget->field_2D2 = 300;
        pSingleTarget->field_2EE = 16;
        pSingleTarget->m_speed = 3 * pSingleTarget->m_speed / 2;
        pSingleTarget->field_302 = 1000;
        pSingleTarget->field_306 = 75;
        pSingleTarget->field_30A = 0;
        pSingleTarget->field_30E = 12;
        pSingleTarget->field_312 = 2;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_08";
        pProjectile = pSingleTarget;
        break;
    }
    case 320: {
        pProjectile = new CProjectileArea(225);
        break;
    }
    case 321: {
        pProjectile = new CProjectileArea(250);
        break;
    }
    case 322: {
        pProjectile = new CProjectileArea(150);
        break;
    }
    case 323: {
        pProjectile = new CProjectileArea(200);
        break;
    }
    case 324:
    case 325:
    case 326:
    case 327:
    case 328:
    case 329:
    case 330:
    case 331:
    case 332:
    case 333:
    case 334: {
        CProjectileBAM84E030* pBAM = new CProjectileBAM84E030(313, CResRef("MFMissT"), projectileType - 323, CVidPalette::TYPE_RANGE);
        pBAM->m_fireSoundRef = "TRA_55";
        pBAM->m_arrivalSoundRef = "";
        pBAM->m_bHasHeight = TRUE;
        pProjectile = pBAM;
        break;
    }
    case 335: {
        CProjectileArea8504BC* pArea = new CProjectileArea8504BC(250);
        pArea->field_4E2.field_0 = "FirebaX";
        pArea->field_4E2.field_10 = "RNG_M03";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pArea->field_50E.field_0 = "FirebaR";
        pArea->field_50E.field_20.sub_586A90(TRUE);
        pArea->field_50E.field_2C = TRUE;
        pArea->field_50E.field_2E = 20;
        pArea->field_540.field_0 = "FirebaA";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_36 = TRUE;
        pArea->field_4D4 = 10000;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 45;
        pProjectile = pArea;
        break;
    }
    case 336: {
        CProjectileArea* pArea = new CProjectileArea(150);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "PWStunH";
        pArea->field_4E2.field_10 = "EFF_M38";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 337: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->field_540.field_0 = "HSmiteA";
        pArea->field_540.field_20.sub_586AC0(TRUE, 64);
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_38 = 300;
        pArea->field_4D4 = 10000;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 45;
        pProjectile = pArea;
        break;
    }
    case 338: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->field_540.field_0 = "UBlighA";
        pArea->field_540.field_20.sub_586AC0(TRUE, 64);
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_38 = 300;
        pArea->field_4D4 = 10000;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 45;
        pProjectile = pArea;
        break;
    }
    case 339: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "EnchanX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 340: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->field_4E2.field_0 = "SoHopeX";
        pArea->field_4E2.field_10 = "EFF_P50";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 341: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->field_4E2.field_0 = "SoHopeX";
        pArea->field_4E2.field_10 = "EFF_P50";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 342: {
        CProjectileArea* pArea = new CProjectileArea(500);
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 343: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "WOWispT");
        pSingleTarget->field_2CE = 200;
        pSingleTarget->field_2D2 = 200;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->field_302 = 30;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 344: {
        CProjectileSingleTarget* pSingleTarget = new CProjectileSingleTarget(CResRef("MFMissT"));
        pSingleTarget->m_speed *= 2;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_bHasHeight = TRUE;
        pSingleTarget->field_1C2 = 1;
        pProjectile = pSingleTarget;
        break;
    }
    case 345: {
        CProjectileBAM84E030* pBAM = new CProjectileBAM84E030(343, CResRef("MFMissT"), 7, CVidPalette::TYPE_RANGE);
        pBAM->m_fireSoundRef = "TRA_55";
        pBAM->m_arrivalSoundRef = "";
        pBAM->m_bHasHeight = TRUE;
        pProjectile = pBAM;
        break;
    }
    case 346: {
        CProjectileSingleTarget8515DC* pSingleTarget = new CProjectileSingleTarget8515DC();
        pSingleTarget->m_fireSoundRef = "TRA_17";
        pProjectile = pSingleTarget;
        break;
    }
    case 347: {
        vfx.sub_586A90(TRUE);
        pProjectile = new CProjectileSingleTarget851234("PortalH", vfx);
        break;
    }
    case 348: {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 2776
        UTIL_ASSERT(FALSE);
        break;
    }
    case 349: {
        pProjectile = new CProjectileArea85072C();
        break;
    }
    case 350: {
        pProjectile = new CProjectileArea8507C8();
        break;
    }
    case 351: {
        IcewindMisc::sub_584880(static_cast<CGameSprite*>(pSource), 27030, 0);
        CProjectileArea* pArea = new CProjectileArea(125);
        pArea->field_50E.field_0 = "CloudKR";
        pArea->field_50E.field_20.sub_586A90(TRUE);
        pArea->field_50E.field_2C = TRUE;
        pArea->field_50E.field_2E = 7;
        pProjectile = pArea;
        break;
    }
    case 352: {
        pProjectile = new CProjectile84DAD4();
        break;
    }
    case 353: {
        vfx.sub_586A90(TRUE);
        pProjectile = new CProjectileSingleTarget851234("IllusH", vfx);
        break;
    }
    case 354: {
        vfx.sub_586A90(TRUE);
        pProjectile = new CProjectileSingleTarget851234("CCDamaH", vfx);
        break;
    }
    case 355: {
        pProjectile = new CProjectileArea850864();
        break;
    }
    case 356: {
        pProjectile = new CProjectileArea850900();
        break;
    }
    case 357: {
        CProjectileArea84FA60* pArea = new CProjectileArea84FA60();
        pArea->sub_5704E0(pSource);
        pArea->m_fireSoundRef = CResRef("TRA_04C");
        pArea->m_arrivalSoundRef = CResRef("EFF_M05");
        pProjectile = pArea;
        break;
    }
    case 358: {
        CProjectileArea* pArea = new CProjectileArea(125);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "Area1X";
        pArea->field_4E2.field_10 = "EFF_M99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 359: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "Area1X";
        pArea->field_4E2.field_10 = "EFF_P99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 360: {
        pProjectile = new CProjectileArea85099C();
        break;
    }
    case 361: {
        pProjectile = new CProjectileSingleTarget851234("GateX", vfx);
        break;
    }
    case 362: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "Area1X";
        pArea->field_4E2.field_10 = "EFF_W99";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 363:
    case 364: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->sub_5704E0(pSource);
        pArea->field_4E2.field_0 = "SoHopeX";
        pArea->field_4E2.field_10 = "EFF_P50";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        // FIXME: There is no `break` in the original, so this falls through to
        // case 365, which replaces (and leaks) this projectile.
    }
    case 365: {
        CProjectileArea850B70* pArea = new CProjectileArea850B70();
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 366: {
        CProjectileArea* pArea = new CProjectileArea(300);
        pArea->field_540.field_0 = "MSwarmA";
        pArea->field_540.field_10 = "ARE_P03";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_34 = FALSE;
        pArea->field_540.field_35 = FALSE;
        pArea->field_540.field_38 = 250;
        pArea->field_4D4 = 10;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10000;
        pArea->field_4C0 = 45;
        pArea->field_1D8 = 1;
        pArea->field_2AE = 300;
        pProjectile = pArea;
        break;
    }
    case 367: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "AlteraX";
        pArea->field_4E2.field_10 = "ARE_M19";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 368: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "FFingeT");
        pSingleTarget->field_2CE = 200;
        pSingleTarget->field_2D2 = 80;
        pSingleTarget->field_317 = FALSE;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->field_2EE = 16;
        pSingleTarget->field_302 = 50;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_15";
        pProjectile = pSingleTarget;
        break;
    }
    case 369: {
        pProjectile = new CProjectileArea8504BC(70);
        break;
    }
    case 370: {
        pProjectile = new CProjectileSingleTarget851664();
        break;
    }
    case 371: {
        CProjectileArea* pArea = new CProjectileArea(70);
        pArea->sub_5703F0(pSource);
        pArea->field_4E2.field_0 = "AlteraX";
        pArea->field_4E2.field_10 = "ARE_M19";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 372: {
        CProjectileArea* pArea = new CProjectileArea(160);
        pArea->field_4E2.field_0 = "AbjuraX";
        pArea->field_4E2.field_10 = "ARE_M20";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 373: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->field_4E2.field_0 = "EnchanX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pProjectile = pArea;
        break;
    }
    case 374: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "ShoutT");
        pSingleTarget->field_2CE = 120;
        pSingleTarget->field_2D2 = 300;
        pSingleTarget->field_2EE = 16;
        pSingleTarget->m_speed = 3 * pSingleTarget->m_speed / 2;
        pSingleTarget->field_302 = 1000;
        pSingleTarget->field_306 = 75;
        pSingleTarget->field_30A = 0;
        pSingleTarget->field_30E = 12;
        pSingleTarget->field_312 = 2;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_08";
        pProjectile = pSingleTarget;
        break;
    }
    case 375: {
        CProjectileBAM84E1D4* pBAM = new CProjectileBAM84E1D4();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "";
        pBAM->m_arrivalSoundRef = "";
        pProjectile = pBAM;
        break;
    }
    case 376: {
        CProjectileArea850A38* pArea = new CProjectileArea850A38();
        pArea->sub_5704E0(pSource);
        pProjectile = pArea;
        break;
    }
    case 377: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->field_540.field_0 = "CoPestA";
        pArea->field_540.field_20.sub_586AC0(TRUE, 128);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_34 = TRUE;
        pArea->field_540.field_10 = "ARE_P25";
        pArea->field_540.field_38 = 175;
        pArea->field_4D4 = 10;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 300;
        pProjectile = pArea;
        break;
    }
    case 378: {
        CProjectileSingleTarget8513BC* pSingleTarget = new CProjectileSingleTarget8513BC("HDABreT");
        pSingleTarget->m_speed = 20;
        pSingleTarget->field_1D8 = 16;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_bHasHeight = TRUE;
        pSingleTarget->field_1C2 = 1;
        pSingleTarget->field_2B2.field_32 = 50;
        pSingleTarget->field_2AE = 600;
        pSingleTarget->m_fireSoundRef = "TRA_59";
        pProjectile = pSingleTarget;
        break;
    }
    case 379: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "HDFBreT");
        pSingleTarget->field_2CE = 300;
        pSingleTarget->field_2D2 = 300;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->m_speed *= 3;
        pSingleTarget->field_302 = 50;
        pSingleTarget->field_30E = 8;
        pSingleTarget->field_312 = 4;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_08";
        pProjectile = pSingleTarget;
        break;
    }
    case 380: {
        CProjectileBAM84DE94* pBAM = new CProjectileBAM84DE94();
        pBAM->m_bHasHeight = TRUE;
        pBAM->m_fireSoundRef = "TRA_09";
        pBAM->m_arrivalSoundRef = "";
        pBAM->field_2A0 = 6;
        pProjectile = pBAM;
        break;
    }
    case 381: {
        CProjectileArea* pArea = new CProjectileArea(200);
        pArea->field_4E2.field_0 = "GABreaX";
        pArea->field_4E2.field_10 = "ARE_M21";
        pArea->field_4E2.field_20.sub_586A90(TRUE);
        pArea->field_540.field_0 = "GABreaA";
        pArea->field_540.field_20.sub_586A90(TRUE);
        pArea->field_540.field_35 = TRUE;
        pArea->field_540.field_36 = TRUE;
        pArea->field_540.field_38 = 500;
        pArea->field_4D4 = 10000;
        pArea->field_4D8 = 0;
        pArea->field_4DC = 10;
        pArea->field_4C0 = 45;
        pArea->field_2AE = 200;
        pProjectile = pArea;
        break;
    }
    case 382: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "GShoutT");
        pSingleTarget->field_2CE = 120;
        pSingleTarget->field_2D2 = 300;
        pSingleTarget->field_2EE = 16;
        pSingleTarget->m_speed = 3 * pSingleTarget->m_speed / 2;
        pSingleTarget->field_302 = 1000;
        pSingleTarget->field_306 = 75;
        pSingleTarget->field_30A = 0;
        pSingleTarget->field_30E = 12;
        pSingleTarget->field_312 = 2;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_08";
        pProjectile = pSingleTarget;
        break;
    }
    case 383: {
        pProjectile = new CProjectileArea850AD4();
        break;
    }
    case 384:
    case 385: {
        pProjectile = new CProjectileArea(65);
        break;
    }
    case 386: {
        CProjectileSingleTarget850DDC* pSingleTarget = new CProjectileSingleTarget850DDC("", "HDFBreT");
        pSingleTarget->field_2CE = 150;
        pSingleTarget->field_2D2 = 150;
        pSingleTarget->field_316 = TRUE;
        pSingleTarget->m_speed *= 3;
        pSingleTarget->field_302 = 50;
        pSingleTarget->field_30E = 8;
        pSingleTarget->field_312 = 4;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = "TRA_08";
        pProjectile = pSingleTarget;
        break;
    }
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 3127
        UTIL_ASSERT(FALSE);
    }

    if (pProjectile == NULL) {
        pProjectile = new CProjectile84D8B8();
    }

    pProjectile->m_projectileType = projectileType - 1;

    return pProjectile;
}

// 0x529F10
void CProjectile::ClearEffects()
{
    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* node = m_effectList.GetNext(pos);
        delete node;
    }
    m_effectList.RemoveAll();
}

// 0x529F40
LONG CProjectile::DetermineHeight(CGameSprite* pSprite)
{
    if (!m_bHasHeight) {
        return 0;
    }

    if (pSprite->GetObjectType() != TYPE_SPRITE) {
        return 32;
    }

    return pSprite->GetAnimation()->GetCastHeight();
}

// 0x78E740
void CProjectile::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
}

// 0x529FB0
void CProjectile::OnArrival()
{
    CProjectile* pProjectile;
    BYTE rc;

    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_callBackProjectile,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        pProjectile->CallBack();

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_callBackProjectile,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    // NOTE: Uninline.
    PlaySound(m_arrivalSoundRef, m_loopArrivalSound, TRUE);

    if (field_182 != CGameObjectArray::INVALID_INDEX) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(field_182,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pProjectile->RemoveSelf();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(field_182,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    DeliverEffects();
    RemoveFromArea();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x52A1A0
void CProjectile::DeliverEffects()
{
    CGameObject* pObject;
    BYTE rc;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    if ((pObject->GetObjectType() & CGameObject::TYPE_AIBASE) == 0) {
        // NOTE: Uninline.
        ClearEffects();

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
        if (pSprite->m_derivedStats.m_cImmunitiesProjectile.OnList(m_projectileType)
            || (static_cast<BYTE>(field_186) < 9
                && pSprite->m_derivedStats.m_cImmunitiesSpellLevel.m_levels[static_cast<BYTE>(field_186)])) {
            // NOTE: Uninline.
            ClearEffects();

            CGameEffect::FeedBackImmuneToResource(pSprite, field_18A);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return;
        }
    }

    std::list<CMessageAddEffect*> effects;

    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* pEffect = m_effectList.GetNext(pos);
        CMessageAddEffect* pAddEffect = new CMessageAddEffect(pEffect, m_sourceId, m_targetId);
        effects.push_back(pAddEffect);
    }
    m_effectList.RemoveAll();

    CMessage105* pMessage = new CMessage105(effects, m_sourceId, m_targetId);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x52A480
SHORT CProjectile::GetDirection(CPoint target)
{
    CPoint ptStart;
    ptStart.x = m_pos.x;
    ptStart.y = 4 * m_pos.y / 3;

    CPoint ptTarget;
    ptTarget.x = target.x;
    ptTarget.y = 4 * target.y / 3;

    return CGameSprite::GetDirection(ptStart, ptTarget);
}

// 0x52A4E0
void CProjectile::PlaySound(CResRef resRef, BOOL loop, BOOL fireAndForget)
{
    m_sound.Stop();
    if (resRef != "") {
        m_sound.SetResRef(resRef, TRUE, TRUE);
        if (loop) {
            m_sound.SetLoopingFlag(TRUE);
        }
        if (fireAndForget) {
            m_sound.SetFireForget(TRUE);
        }
        m_sound.SetChannel(15, reinterpret_cast<DWORD>(field_EA));
        m_sound.Play(m_pos.x, m_pos.y, 0, FALSE);
    }
}

// NOTE: Not in the original. Inlined four times in 0x52A640.
static SHORT GetEllipseDiagonal(CGameSprite* pSprite)
{
    SHORT a = static_cast<SHORT>(static_cast<SHORT>(pSprite->GetAnimation()->GetEllipseRect().right) * static_cast<SHORT>(pSprite->GetAnimation()->GetEllipseRect().right));
    SHORT b = static_cast<SHORT>(static_cast<SHORT>(pSprite->GetAnimation()->GetEllipseRect().bottom) * static_cast<SHORT>(pSprite->GetAnimation()->GetEllipseRect().bottom));
    return static_cast<SHORT>(sqrt(static_cast<double>(a * b / (b + a))));
}

// 0x52A640
void CProjectile::sub_52A640(LONG nObjectId, CPoint& pt, BOOLEAN a3)
{
    CGameObject* pObject;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nObjectId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    switch (pObject->GetObjectType()) {
    case 0: // CGameObject::TYPE_NONE
    case 1: // CGameObject::TYPE_AIBASE
    case 16: // CGameObject::TYPE_SOUND
    case 32: // CGameObject::TYPE_SPAWNING
    case 48: // CGameObject::TYPE_STATIC
    case 49: // CGameObject::TYPE_SPRITE
    case 64: // CGameObject::TYPE_OBJECT_MARKER
    case 80: // CGameObject::TYPE_TEXT
        pt = pObject->GetPos();
        break;
    case 17: // CGameObject::TYPE_CONTAINER
        pt = static_cast<CGameContainer*>(pObject)->m_posTrapOrigin;
        break;
    case 33: // CGameObject::TYPE_DOOR
        pt.x = static_cast<CGameDoor*>(pObject)->m_posXTrapOrigin;
        pt.y = static_cast<CGameDoor*>(pObject)->m_posYTrapOrigin;
        break;
    case 65: // CGameObject::TYPE_TRIGGER
        pt = static_cast<CGameTrigger*>(pObject)->m_posTrapOrigin;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 3590
        UTIL_ASSERT(FALSE);
    }

    if (a3 && pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
        SHORT nDiagonal;

        switch (pSprite->m_nDirection) {
        case 0:
        case 1:
            pt.y += pSprite->GetAnimation()->GetEllipseRect().bottom / 2;
            break;
        case 2:
        case 3:
            nDiagonal = GetEllipseDiagonal(pSprite);
            pt.x += -2 * nDiagonal;
            pt.y += 2 * nDiagonal;
            break;
        case 4:
        case 5:
            pt.x += -2 * pSprite->GetAnimation()->GetEllipseRect().right;
            pt.y += 1;
            break;
        case 6:
        case 7:
            nDiagonal = GetEllipseDiagonal(pSprite);
            pt.x += -2 * nDiagonal;
            pt.y += -2 * nDiagonal;
            break;
        case 8:
        case 9:
            pt.y += -2 * pSprite->GetAnimation()->GetEllipseRect().bottom;
            break;
        case 10:
        case 11:
            nDiagonal = GetEllipseDiagonal(pSprite);
            pt.x += 2 * nDiagonal;
            pt.y += -2 * nDiagonal;
            break;
        case 12:
        case 13:
            pt.x += 2 * pSprite->GetAnimation()->GetEllipseRect().right;
            pt.y += 1;
            break;
        case 14:
        case 15:
            nDiagonal = GetEllipseDiagonal(pSprite);
            pt.x += 2 * nDiagonal;
            pt.y += 2 * nDiagonal;
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
            // __LINE__: 3664
            UTIL_ASSERT(FALSE);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nObjectId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x560310
CProjectile* CProjectile::sub_560310(WORD projectileType, CGameAIBase* pSource, BOOL a3)
{
    CProjectile* pProjectile = NULL;

    switch (projectileType) {
    case 0: {
        pProjectile = new CProjectile84D8B8();
        break;
    }
    case 1: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("AbjurH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(215, 140, 70, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        if (a3 == TRUE) {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_P01");
        } else {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_M02");
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 2: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("AlterH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(75, 210, 160, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        if (a3 == TRUE) {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_P07");
        } else {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_M08");
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 3: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("InvocH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(107, 6, 201, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        if (a3 == TRUE) {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_P05");
        } else {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_M06");
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 4: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("NecroH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(180, 210, 80, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        if (a3 == TRUE) {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_P06");
        } else {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_M07");
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 5: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ConjuH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(105, 215, 70, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        if (a3 == TRUE) {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_P02");
        } else {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_M03");
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 6: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("EnchaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(80, 210, 80, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        if (a3 == TRUE) {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_P04");
        } else {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_M05");
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 7: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("IllusH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(80, 90, 210, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M34");
        pProjectile = pSingleTarget;
        break;
    }
    case 8: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DivinH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(100, 70, 210, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        if (a3 == TRUE) {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_P03");
        } else {
            pSingleTarget->m_fireSoundRef = CResRef("EFF_M04");
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 9: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ArmorH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M48");
        pProjectile = pSingleTarget;
        break;
    }
    case 10: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SArmorH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M36");
        pProjectile = pSingleTarget;
        break;
    }
    case 11: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("GArmorH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M36");
        pProjectile = pSingleTarget;
        break;
    }
    case 12: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("StrengH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M12");
        pProjectile = pSingleTarget;
        break;
    }
    case 13: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ConfusH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M16");
        pProjectile = pSingleTarget;
        break;
    }
    case 14: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SOFlamH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 15: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DSpellH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 16: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DisintH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M43");
        pProjectile = pSingleTarget;
        break;
    }
    case 17: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("PWSileH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M44");
        pProjectile = pSingleTarget;
        break;
    }
    case 18: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("PWStunH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M38");
        pProjectile = pSingleTarget;
        break;
    }
    case 19: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("FODeatH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M07");
        pProjectile = pSingleTarget;
        break;
    }
    case 20: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSwordH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 21: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm1H", IcewindCVisualEffect());
        pSingleTarget->field_2B8 = FALSE;
        pSingleTarget->field_2B9 = TRUE;
        pSingleTarget->field_2AF = FALSE;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 22: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm2H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 23: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm3H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 24: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm4H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 25: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm5H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 26: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm6H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 27: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm7H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 28: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CFElemH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 29: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CEElemH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 30: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CWElemH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 31: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("BlessH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P31");
        pProjectile = pSingleTarget;
        break;
    }
    case 32: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CurseH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P32");
        pProjectile = pSingleTarget;
        break;
    }
    case 33: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("PrayerH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P31");
        pProjectile = pSingleTarget;
        break;
    }
    case 34: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("RecitaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P44");
        pProjectile = pSingleTarget;
        break;
    }
    case 35: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CLWounH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P26");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0, 128, 255, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 36: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CMWounH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P26");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0, 128, 255, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 37: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CSWounH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P34");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0, 128, 255, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 38: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CCWounH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P46");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0, 128, 255, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 39: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("HealH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P37");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0, 128, 255, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 40: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ASumm1H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 41: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ASumm2H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 42: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ASumm3H", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 43: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SPoisoH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P17");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(255, 128, 0, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 44: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("NPoisoH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P41");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(192, 128, 0, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 45: {
        CProjectile84E8C0* pNew = new CProjectile84E8C0(CString("CallLiH"), CString(""), 255, 1, 0, 20);
        pNew->m_fireSoundRef = CResRef("EFF_P19");
        pProjectile = pNew;
        break;
    }
    case 46: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SChargH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P42");
        pProjectile = pSingleTarget;
        break;
    }
    case 47: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("RParalH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P33");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0, 0, 255, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 48: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("FActioH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P40");
        pProjectile = pSingleTarget;
        break;
    }
    case 49: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MMagicH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P13");
        pProjectile = pSingleTarget;
        break;
    }
    case 50: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SOOneH", IcewindCVisualEffect());
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P35");
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(16, 64, 128, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 51: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CStrenH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M41");
        pProjectile = pSingleTarget;
        break;
    }
    case 52: {
        CProjectile84E8C0* pNew = new CProjectile84E8C0(CString("FStrikH"), CString(""), 255, 1, 0, 28);
        pNew->m_fireSoundRef = CResRef("EFF_P16");
        pProjectile = pNew;
        break;
    }
    case 53: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("RDeadH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P18");
        pProjectile = pSingleTarget;
        break;
    }
    case 54: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ResurrH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P18");
        pProjectile = pSingleTarget;
        break;
    }
    case 55: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CCommaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P47");
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(128, 0, 128, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 56: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("RWOTFaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P36");
        pProjectile = pSingleTarget;
        break;
    }
    case 57: {
        pProjectile = new CProjectile8512BC(CString("SunrayX"), CString(""), 255, 1, 0, 325);
        break;
    }
    case 58: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SStoneA", IcewindCVisualEffect());
        pSingleTarget->m_fireSoundRef = CResRef("CRE_P03");
        pProjectile = pSingleTarget;
        break;
    }
    case 59: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DDoorH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M09");
        pProjectile = pSingleTarget;
        break;
    }
    case 60: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DDoorH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M09");
        pProjectile = pSingleTarget;
        break;
    }
    case 61: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CoColdH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 62: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SSOrbH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P38");
        pProjectile = pSingleTarget;
        break;
    }
    case 63: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("FireH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            CMessageAddEffect* message = new CMessageAddEffect(IcewindMisc::CreateEffectColorGlowDissipate(255, 0, 0, 30),
                pSource->GetId(),
                pSource->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 64: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ColdH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0, 0, 255, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 65: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ElectrH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(255, 75, 255, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 66: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("AcidH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0, 255, 0, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 67: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ParalH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        if (pSource != NULL) {
            pSource->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(45, 0, 90, 30), CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        pProjectile = pSingleTarget;
        break;
    }
    case 68: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MRageH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 69: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("RWOTFaG", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_2AE = TRUE;
        pProjectile = pSingleTarget;
        break;
    }
    case 70: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("BDeath", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 71: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("PortalH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 72: {
        CProjectile84E8C0* pNew = new CProjectile84E8C0(CString("SunscoH"), CString(""), 255, 1, 0, 24);
        pNew->m_fireSoundRef = CResRef("EFF_P39");
        pProjectile = pNew;
        break;
    }
    case 73: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("BBarrH1", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A60(TRUE);
        pSingleTarget->field_2AF = TRUE;
        pSingleTarget->field_2B0 = 1;
        pSingleTarget->field_2B4 = 165;
        pSingleTarget->field_2B8 = FALSE;
        pSingleTarget->field_2B9 = FALSE;
        pSingleTarget->m_fireSoundRef = CResRef("ARE_P20");
        pProjectile = pSingleTarget;
        break;
    }
    case 74: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("BBarrH2", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A60(TRUE);
        pSingleTarget->field_2AE = TRUE;
        pSingleTarget->field_2AF = TRUE;
        pSingleTarget->field_2B0 = 1;
        pSingleTarget->field_2B4 = 165;
        pSingleTarget->field_2B8 = FALSE;
        pSingleTarget->field_2B9 = FALSE;
        pProjectile = pSingleTarget;
        break;
    }
    case 75: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CoBonH1", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A60(TRUE);
        pSingleTarget->field_2AF = TRUE;
        pSingleTarget->field_2B0 = 1;
        pSingleTarget->field_2B4 = 265;
        pSingleTarget->field_2B8 = FALSE;
        pSingleTarget->field_2B9 = FALSE;
        pSingleTarget->m_fireSoundRef = CResRef("ARE_P21");
        pProjectile = pSingleTarget;
        break;
    }
    case 76: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CoBonH2", IcewindCVisualEffect());
        pSingleTarget->field_2AE = TRUE;
        pSingleTarget->m_vfx.sub_586A60(TRUE);
        pSingleTarget->field_2AF = TRUE;
        pSingleTarget->field_2B0 = 1;
        pSingleTarget->field_2B4 = 265;
        pSingleTarget->field_2B8 = FALSE;
        pSingleTarget->field_2B9 = FALSE;
        pProjectile = pSingleTarget;
        break;
    }
    case 77: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CLDamaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P103");
        pProjectile = pSingleTarget;
        break;
    }
    case 78: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CMDamaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P103");
        pProjectile = pSingleTarget;
        break;
    }
    case 79: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CSDamaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P103");
        pProjectile = pSingleTarget;
        break;
    }
    case 80: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CCDamaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P103");
        pProjectile = pSingleTarget;
        break;
    }
    case 81: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CDiseaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P108");
        pProjectile = pSingleTarget;
        break;
    }
    case 82: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("PoisonH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P108");
        pProjectile = pSingleTarget;
        break;
    }
    case 83: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SLivinH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P109");
        pProjectile = pSingleTarget;
        break;
    }
    case 84: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("HarmH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P103");
        pProjectile = pSingleTarget;
        break;
    }
    case 85: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DestruH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P113");
        pProjectile = pSingleTarget;
        break;
    }
    case 86: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ExaltaH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P106");
        pProjectile = pSingleTarget;
        break;
    }
    case 87: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CloudbH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 88: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MTouchH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P107");
        pProjectile = pSingleTarget;
        break;
    }
    case 89: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MTouchH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P107");
        pProjectile = pSingleTarget;
        break;
    }
    case 90: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CGraceH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 91: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SEaterH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_2B8 = TRUE;
        pSingleTarget->field_2AF = FALSE;
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M104");
        pProjectile = pSingleTarget;
        break;
    }
    case 92: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SWaveH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_P110");
        pProjectile = pSingleTarget;
        break;
    }
    case 93: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("SuffocA", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 94: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ADHWilH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M105");
        pProjectile = pSingleTarget;
        break;
    }
    case 95: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MFMissX", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("EFF_M103");
        pProjectile = pSingleTarget;
        break;
    }
    case 96: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("VSpherX", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 97: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("WVDeatH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 98: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("UWardX", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_2AE = TRUE;
        pSingleTarget->field_2AF = TRUE;
        pSingleTarget->field_2B0 = 1;
        pSingleTarget->field_2B4 = 965;
        pSingleTarget->field_2B8 = FALSE;
        pSingleTarget->field_2B9 = FALSE;
        pProjectile = pSingleTarget;
        break;
    }
    case 99: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("WVHitH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 100: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("WDeath1", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 101: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("WDeath2", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 102: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DDeath", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 103: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DDeath2", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 104: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("MSumm1X", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_2AE = TRUE;
        pProjectile = pSingleTarget;
        break;
    }
    case 105: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("ASumm1X", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_2AE = TRUE;
        pProjectile = pSingleTarget;
        break;
    }
    case 106: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CEElemX", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_2AE = TRUE;
        pProjectile = pSingleTarget;
        break;
    }
    case 107: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CFElemX", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_2AE = TRUE;
        pProjectile = pSingleTarget;
        break;
    }
    case 108: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("CWElemX", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->field_2AE = TRUE;
        pProjectile = pSingleTarget;
        break;
    }
    case 109: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("GELoopX", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pProjectile = pSingleTarget;
        break;
    }
    case 110: {
        CProjectileSingleTarget851234* pSingleTarget = new CProjectileSingleTarget851234("DAttacH", IcewindCVisualEffect());
        pSingleTarget->m_vfx.sub_586A60(TRUE);
        pSingleTarget->m_fireSoundRef = CResRef("CRE_P01");
        pProjectile = pSingleTarget;
        break;
    }
    case 111: {
        pProjectile = new CProjectileSingleTarget851234("WoMoonX", IcewindCVisualEffect());
        break;
    }
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameEffects.cpp
        // __LINE__: 2981
        UTIL_ASSERT(FALSE);
    }

    return pProjectile;
}

// 0x78E730
void CProjectile::CallBack()
{
}
