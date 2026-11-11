#include "CProjectilePart6.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CUtil.h"

// 0x55ACC0
CProjectileHelper84EEA8::CProjectileHelper84EEA8()
{
    field_18 = 10;
    field_20 = 10;
    field_24 = 100000;
    field_28 = 100000;
    field_1C = 0;
    field_2C = 0;
    field_30 = FALSE;
    field_31 = FALSE;
}

// 0x55A610
void CProjectileHelper84EEA8::Virtual0()
{
    // TODO: Incomplete.
}

// 0x55AD90
std::list<LONG> CProjectileHelper84EEA8::Virtual4()
{
    // TODO: Incomplete.

    return std::list<LONG>();
}

// 0x55AF40
CProjectileHelper84EEB0::CProjectileHelper84EEB0()
{
    field_3A = FALSE;
}

// 0x55B1B0
std::list<LONG> CProjectileHelper84EEB0::Virtual4()
{
    // TODO: Incomplete.

    return std::list<LONG>();
}

// 0x579B40
CProjectileSingleTarget850DDC::CProjectileSingleTarget850DDC(const char* a1, const char* a2)
{
    field_2CE = 200;
    field_2D2 = 350;
    field_2DA = 25;
    field_2DE = a2;
    field_2EE = 1;
    field_302 = 20;
    field_306 = 35;
    field_30A = 45;
    field_30E = 15;
    field_312 = 5;
    field_316 = FALSE;
    field_317 = TRUE;
    field_32E = 0;
    field_332 = FALSE;
    field_2D6 = 0;
    field_328 = 0;
    field_32C = 0;
    field_2A0 = FALSE;
    field_2A1 = TRUE;

    // NOTE: Uninline.
    SetVidCell(CResRef(CString(a1)));
    field_1DE = 1;

    // NOTE: Uninline.
    SetVidCell(CResRef("MMissiT"));
    field_1DE = 0;

    m_bHasHeight = TRUE;
}

// 0x579E50
void CProjectileSingleTarget850DDC::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x579EF0
void CProjectileSingleTarget850DDC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x57A530
void CProjectileSingleTarget850DDC::OnArrival()
{
    // TODO: Incomplete.
}

// 0x57A670
void CProjectileSingleTarget850DDC::DeliverEffects()
{
    // TODO: Incomplete.
}

// 0x57A970
void CProjectileSingleTarget850DDC::Virtual88()
{
    // TODO: Incomplete.
}

// 0x57AEB0
CProjectileSingleTarget850E84::CProjectileSingleTarget850E84()
    : CProjectileSingleTarget(CResRef((BYTE*)"AScorcT"))
{
    field_2C6 = 0;
    field_2CA = 3;
    field_2CE = 60;
    field_2B0 = 0;
    field_2B4 = FALSE;
    field_2D2 = 0;
    sub_57BBC0();
    m_pVidCell->FrameSet(0);
    m_fireSoundRef = "TRA_M04";
    field_1C2 = 0;
    field_1D4 = 0;
    field_1D8 = 16;

    CSize frameSize;
    m_pVidCell->GetFrameSize(0, 0, frameSize, FALSE);
    m_speed = static_cast<SHORT>(3 * frameSize.cx / 4);

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    field_2AF = 0;
    field_2AE = 110;
    m_bHasHeight = TRUE;
    field_1C2 = 1;
}

// 0x57B210
void CProjectileSingleTarget850E84::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x57B780
void CProjectileSingleTarget850E84::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x57B1D0
void CProjectileSingleTarget850E84::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x799CA0
void CProjectileSingleTarget850E84::DeliverEffects()
{
}

// 0x57BE80
void CProjectileSingleTarget850E84::Virtual88()
{
    // TODO: Incomplete.
}

// 0x57C1D0
void CProjectileSingleTarget850E84::Virtual8C(CPoint pt)
{
    // TODO: Incomplete.
}

// 0x57C510
CProjectileSingleTarget850F14::CProjectileSingleTarget850F14(const std::set<LONG>& a1)
    : CProjectileSingleTarget(CResRef((BYTE*)"CLightT"))
    , field_2AE(a1)
{
    m_pVidCell->SequenceSet(0);
    field_1D8 = 16;
    m_fireSoundRef = "TRA_09";
    m_speed *= 2;
    m_vfx.sub_586A90(TRUE);
    m_bHasHeight = FALSE;
    field_1C2 = 0;
}

// 0x57C7B0
void CProjectileSingleTarget850F14::DeliverEffects()
{
    // TODO: Incomplete.
}

// 0x57D390
CProjectileSingleTarget85101C::CProjectileSingleTarget85101C()
    : CProjectileSingleTarget(CResRef((BYTE*)"LightnT"))
{
    field_2B2 = -1;
    m_pVidCell->SequenceSet(0);
    field_E6 |= 0x8;
    field_1BE = 0;
    field_1C2 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "TRA_09b";
    m_speed *= 2;
    field_2AE = 60;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    m_vfx.sub_586A90(TRUE);
    field_2B6.m_pProjectile = this;
    field_2B6.field_20 = 33;
    field_2B6.field_18 = 2;
    field_2B6.field_32 = 50;
}

// 0x57D9F0
void CProjectileSingleTarget85101C::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x57D700
void CProjectileSingleTarget85101C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x57E370
CProjectileSingleTarget8511AC::CProjectileSingleTarget8511AC(const char* szResRef)
    : CProjectileSingleTarget(CResRef(CString(szResRef)))
{
    m_vfx.sub_586A90(TRUE);
    m_pVidCell->SequenceSet(0);
    field_1BE = 0;
    field_1C2 = 0;
    field_1D4 = 0;
    m_speed *= 2;
    m_fireSoundRef = "";
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    m_bHasHeight = TRUE;
    m_fireSoundRef = "TRA_02";
    m_arrivalSoundRef = "";
}

// 0x57E490
CProjectileSingleTarget851234::CProjectileSingleTarget851234(const char* szResRef, const IcewindCVisualEffect& vfx)
    : CProjectileSingleTarget(CResRef(CString(szResRef)))
{
    field_2AE = FALSE;
    field_2AF = FALSE;
    field_2B0 = 0;
    field_2B4 = 0;
    field_2B8 = TRUE;
    field_2B9 = TRUE;
    field_1D4 = 0;
    field_1D8 = 1;
    m_bHasHeight = FALSE;
    field_1C2 = 0;
    m_vfx = vfx;
}

// 0x57E580
void CProjectileSingleTarget851234::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x57E710
void CProjectileSingleTarget851234::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x57F640
CProjectileSingleTarget851444::CProjectileSingleTarget851444()
    : CProjectileSingleTarget(CResRef((BYTE*)"WhirlwX"))
{
    field_2AE = 1000;
    field_2B2 = 1000;
    field_2B6 = 0;
    field_354 = FALSE;
    field_356 = 0;
    field_1D8 = 1;
    m_speed = 5;
    field_2BA.m_pProjectile = this;
    field_2BA.field_28 = 8;
    field_2BA.field_32 = 70;
    field_2BA.field_20 = 33;
    field_2BA.field_18 = 3;
    field_2BA.field_31 = TRUE;
    m_vfx.sub_586A90(TRUE);
    m_bHasHeight = FALSE;
    field_1C2 = 0;
    m_fireSoundRef = "EFF_P105";
    m_loopFireSound = TRUE;
}

// 0x57F8D0
void CProjectileSingleTarget851444::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x57FF80
void CProjectileSingleTarget851444::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x580270
void CProjectileSingleTarget851444::OnArrival()
{
    // TODO: Incomplete.
}

// 0x5802B0
CProjectileSingleTarget8514CC::CProjectileSingleTarget8514CC()
    : CProjectileSingleTarget(CResRef((BYTE*)"WoMoonX"))
{
    field_2AE = 500;
    field_2B2 = FALSE;
    field_1D8 = 16;
    field_2B4.m_pProjectile = this;
    field_2B4.field_24 = 1;
    field_2B4.field_32 = 70;
    field_2B4.field_36 = 20;
    field_2B4.field_18 = 5;
    m_vfx.sub_586A90(TRUE);
    field_1DE = 0;
    m_speed *= 3;
    m_bHasHeight = FALSE;
    field_1C2 = 0;
    field_2A0 = FALSE;
    m_fireSoundRef = "AFT_P21";
    m_loopFireSound = TRUE;
}

// 0x580550
void CProjectileSingleTarget8514CC::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x580600
void CProjectileSingleTarget8514CC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x5820B0
void CProjectileSingleTarget8514CC::OnArrival()
{
    // TODO: Incomplete.
}

// 0x5806C0
CProjectileSingleTarget8513BC::CProjectileSingleTarget8513BC(const char* szResRef)
    : CProjectileSingleTarget(CResRef((BYTE*)szResRef))
{
    field_2AE = 400;
    field_2B2.m_pProjectile = this;
    field_2B2.field_20 = 33;
    field_2B2.field_18 = 2;
}

// 0x5808A0
void CProjectileSingleTarget8513BC::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x580B30
void CProjectileSingleTarget8513BC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x580C00
CProjectileSingleTarget851554::CProjectileSingleTarget851554()
    : CProjectileSingleTarget(CResRef((BYTE*)"MFMissT"))
{
    field_1D8 = 1;
    m_speed *= 2;
    field_2AE.m_pProjectile = this;
    field_2AE.field_32 = 70;
    m_vfx.sub_586A90(TRUE);
    m_bHasHeight = TRUE;
    field_1C2 = 1;
    m_fireSoundRef = "TRA_55";
}

// 0x580E10
void CProjectileSingleTarget851554::OnArrival()
{
    // TODO: Incomplete.
}

// 0x581060
CProjectileSingleTarget8515DC::CProjectileSingleTarget8515DC()
    : CProjectileSingleTarget(CResRef((BYTE*)"AScorcT"))
{
    field_2B0 = 0;
    field_2B4 = FALSE;
    m_pVidCell->FrameSet(0);
    m_fireSoundRef = "TRA_M04";
    field_1C2 = 0;
    field_1D4 = 0;
    field_1D8 = 16;

    CSize frameSize;
    m_pVidCell->GetFrameSize(0, 0, frameSize, FALSE);
    m_speed = static_cast<SHORT>(3 * frameSize.cx / 4);

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    m_bHasHeight = TRUE;
    field_1C2 = 1;
    field_2AF = 0;
    field_2AE = 10;
}

// 0x5811B0
void CProjectileSingleTarget8515DC::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x581520
void CProjectileSingleTarget8515DC::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x581170
void CProjectileSingleTarget8515DC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x581CA0
CProjectileSingleTarget851664::CProjectileSingleTarget851664()
    : CProjectileSingleTarget(CResRef((BYTE*)"WoFireX"))
{
    field_2AE = 500;
    field_2B2 = FALSE;
    field_1D8 = 16;
    field_2B4.m_pProjectile = this;
    field_2B4.field_32 = 70;
    field_2B4.field_36 = 20;
    field_2B4.field_20 = 20;
    field_2B4.field_18 = 5;
    m_vfx.sub_586A90(TRUE);
    field_1DE = 0;
    m_speed *= 3;
    m_bHasHeight = FALSE;
    field_1C2 = 0;
    m_fireSoundRef = "AFT_P21";
    m_loopFireSound = TRUE;
}

// 0x581F40
void CProjectileSingleTarget851664::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x581FF0
void CProjectileSingleTarget851664::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x5820B0
void CProjectileSingleTarget851664::OnArrival()
{
    // TODO: Incomplete.
}

// 0x583500
void CProjectileSingleTarget851664::Virtual88()
{
    // TODO: Incomplete.
}

// 0x8E52D0
std::vector<CPoint> CProjectileSingleTarget850E84::stru_8E52D0;

// 0x8E52E8
BOOLEAN CProjectileSingleTarget850E84::byte_8E52E8;

// 0x57BBC0
void CProjectileSingleTarget850E84::sub_57BBC0()
{
    // TODO: Incomplete.
}
