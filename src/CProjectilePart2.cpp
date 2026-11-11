#include "CProjectilePart2.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CUtil.h"

// 0x5309C0
CProjectileBAM84E030::CProjectileBAM84E030(WORD a1, CResRef resRef, SHORT nCount, WORD a4)
    : CProjectileBAM(resRef)
{
    m_pVidCell->SequenceSet(0);
    field_2A0 = nCount;
    field_2BE = a1;
    field_E6 |= 0x8;

    switch (a4) {
    case 0:
        break;
    case 1:
        m_palette.SetRange(0, 33, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_pVidCell->SetPalette(m_palette);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 7083
        UTIL_ASSERT(FALSE);
    }

    for (SHORT index = 0; index < nCount; index++) {
        CProjectile* pProjectile = CProjectile::DecodeProjectile(field_2BE + 1, NULL, FALSE);
        if (index != 0) {
            pProjectile->m_fireSoundRef = CResRef("");
        }
        field_2A2.AddTail(pProjectile);
    }

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x530B60
CProjectileBAM84E030::~CProjectileBAM84E030()
{
    // TODO: Incomplete.
}

// 0x530C90
void CProjectileBAM84E030::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x531120
CProjectileBAM84E0C4::CProjectileBAM84E0C4(SHORT nCount, WORD a2)
    : CProjectileBAM84E030(217, CResRef((BYTE*)"SPMAGMIS"), nCount, a2)
{
    field_1BE = 0;
    field_1C2 = 0;
    field_1D4 = 0;
    field_1CA = 10;
    field_1CE = 7;
    field_1C6 = 1;
    field_1D2 = 128;
    field_1D8 = 1;
    m_fireSoundRef = "";
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531360
CProjectileBAM84E14C::CProjectileBAM84E14C()
    : CProjectileBAM(CResRef((BYTE*)"BOLT"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 5;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531570
CProjectileBAM84E1D4::CProjectileBAM84E1D4()
    : CProjectileBAM(CResRef((BYTE*)"BOULDER"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed = 3 * m_speed / 2;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531790
CProjectileBAM84E25C::CProjectileBAM84E25C()
    : CProjectileBAM(CResRef((BYTE*)"DART"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 3;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x5319A0
CProjectileBAM84E2E4::CProjectileBAM84E2E4()
    : CProjectileBAM(CResRef((BYTE*)"MAGICSTN"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 27, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 1;
    m_fireSoundRef = "";
    m_speed *= 5;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531BE0
CProjectileBAM84E36C::CProjectileBAM84E36C()
    : CProjectileBAM(CResRef((BYTE*)"AXE"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 2;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531DF0
CProjectileBAM84E3F4::CProjectileBAM84E3F4()
    : CProjectileBAM(CResRef((BYTE*)"SPEAR"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 2;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x532000
CProjectileBAM84E47C::CProjectileBAM84E47C()
    : CProjectileBAM(CResRef((BYTE*)"DAGGER"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(5, 27, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(4, 30, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 2;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x532260
CProjectileBAM84E504::CProjectileBAM84E504()
    : CProjectileBAM(CResRef((BYTE*)"ACIDBLOB"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 73, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x5324A0
CProjectileBAM84E58C::CProjectileBAM84E58C()
    : CProjectileBAM(CResRef((BYTE*)"ARARROW"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(5, 27, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(4, 23, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(2, 46, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 5;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x532720
CProjectileBAM84E614::CProjectileBAM84E614()
    : CProjectileBAM(CResRef((BYTE*)"SPFLMARR"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 67, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(1, 46, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 5;
    field_2A0[7] = 0;
    memset(field_2A0, 'A', 7);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x5329A0
void CProjectileBAM84E614::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x532CA0
CProjectileBAM84E69C::CProjectileBAM84E69C()
    : CProjectileBAM(CResRef((BYTE*)"SPBRNHND"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 71, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_E6 |= 0x8;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    field_2A0 = 55;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    field_174 = 23;
    field_178 = 50;
}

// 0x533A50
void CProjectileBAM84E69C::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x532F00
void CProjectileBAM84E69C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x533CB0
CProjectileBAM84E7B0::CProjectileBAM84E7B0(WORD a1, CResRef resRef, WORD a3, WORD a4, LONG a5, CAIObjectType type)
    : CProjectileBAM(resRef)
{
    field_2EE = 0;
    field_2F2 = 0;
    field_2F4 = 0;
    field_2F8 = 0;
    m_pVidCell->SequenceSet(0);
    field_2A8 = a1;
    field_2A0 = a3;
    field_2A2 = a4;
    field_2A4 = a5;
    field_2AE = 0;
    field_2B2.Set(type);
    field_2AA = CProjectile::DecodeProjectile(field_2A8 + 1, NULL, FALSE);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x533E10
CProjectileBAM84E7B0::~CProjectileBAM84E7B0()
{
    // TODO: Incomplete.
}

// 0x534140
void CProjectileBAM84E7B0::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x78E750
void CProjectileBAM84E7B0::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
}

// 0x533F40
void CProjectileBAM84E7B0::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x533DE0
void CProjectileBAM84E7B0::CallBack()
{
    // TODO: Incomplete.
}

// 0x534600
CProjectileBAM84E838::CProjectileBAM84E838(WORD a1, LONG a2)
    : CProjectileBAM84E7B0(22, CResRef((BYTE*)"MAGMISS"), a1, 300, 1, CAIObjectType::ANYONE)
{
    field_1CA = 10;
    field_1CE = 7;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 1;
    field_1D2 = 128;
    field_1D4 = 0;
    field_1D8 = 1;
    m_fireSoundRef = "";
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;

    CAIObjectType type;
    type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
    field_2B2.Set(type);
    field_2F4 = 1;
}

// 0x57E030
CProjectileBAM8510A4::CProjectileBAM8510A4(WORD a1)
    : CProjectileBAM(CResRef((BYTE*)"MMissiT"))
{
    m_pVidCell->SequenceSet(0);
    m_speed *= 2;
    field_1CA = 15;
    field_1CE = 11;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 1;
    field_1D2 = 128;
    field_1D4 = 0;
    m_fireSoundRef = "";
    field_E6 |= 0x8;

    switch (a1) {
    case 0:
        break;
    case 1:
        m_palette.SetRange(0, 33, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_pVidCell->SetPalette(m_palette);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
        // __LINE__: 1481
        UTIL_ASSERT(FALSE);
    }

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}
