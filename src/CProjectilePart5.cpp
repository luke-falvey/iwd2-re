#include "CProjectilePart5.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CUtil.h"

// 0x574140
CProjectileArea84FE08::CProjectileArea84FE08()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "PFireX";
    field_4E2.field_10 = "EFF_P45";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "PFireA";
    field_540.field_10 = "ARE_P03";
    field_540.field_20.sub_586A90(TRUE);
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 750;
    field_4D4 = 10;
    field_4D8 = 0;
    field_4DC = 10000;
    field_4C0 = 100;
    field_1D8 = 1;
    field_2AE = 60;
}

// 0x574300
CProjectileArea84FEA4::CProjectileArea84FEA4()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "EnchanX";
    field_4E2.field_10 = "ARE_M21";
    field_4E2.field_20.sub_586A90(TRUE);
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 150;
}

// 0x574430
void CProjectileArea84FEA4::Virtual94(std::list<LONG>& targets)
{
    // TODO: Incomplete.
}

// 0x5745E0
CProjectileArea84FF40::CProjectileArea84FF40()
    : CProjectileArea(256)
{
    const char* szTravel = "SSSwarT";
    if (strlen(szTravel) != 0) {
        field_1DE = 1;
        SetVidCell(CResRef(szTravel));
        field_4E0 = TRUE;
    } else {
        field_1DE = 0;
        field_4E0 = FALSE;
    }

    m_fireSoundRef = "TRA_18";
    m_vfx.sub_586A90(TRUE);
    field_4E2.field_0 = "SSSwarX";
    field_4E2.field_10 = "RNG_M02";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "SSSwarR";
    field_50E.field_20.sub_586A90(TRUE);
    field_4D8 = 0;
    m_speed *= 2;
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 26;
    field_4D4 = 10000;
    field_4DC = 10;
    field_4C0 = 45;
    field_1D8 = 16;
    field_2AE = 250;
}

// 0x5747A0
CProjectileArea84FFDC::CProjectileArea84FFDC()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "SStoneA";
    field_4E2.field_10 = "EFF_P48";
    field_540.field_0 = "SStoneA";
    field_540.field_10 = "ARE_P04";
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 500;
    field_4D8 = 0;
    field_4C0 = 1200;
    field_2AE = 150;
}

// 0x574970
CProjectileArea850078::CProjectileArea850078()
    : CProjectileArea(256)
{
    field_2AE = 275;
}

// 0x5749A0
void CProjectileArea850078::Virtual94(std::list<LONG>& targets)
{
    // TODO: Incomplete.
}

// 0x574B80
CProjectileArea8501B0::CProjectileArea8501B0()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "SCloudX";
    field_4E2.field_10 = "RNG_M01";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "SCloudR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 13;
    field_540.field_0 = "SCloudA";
    field_540.field_10 = "ARE_M02";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_38 = 1000;
    field_4C0 = 1000;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_4D8 = 0;
    field_1D8 = 1;
    field_2AE = 100;
}

// 0x574D50
CProjectileArea85024C::CProjectileArea85024C()
    : CProjectileArea(256)
{
    field_4D4 = 10;
    field_4DC = 10;
    field_4D8 = 0;
    field_4C0 = 325;
    field_2AE = 150;
}

// 0x574DA0
void CProjectileArea85024C::OnArrival()
{
    // TODO: Incomplete.
}

// 0x574EB0
CProjectileArea8502E8::CProjectileArea8502E8()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "WebX";
    field_4E2.field_10 = "EFF_M19";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "WebA";
    field_540.field_10 = "ARE_M03";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 450;
    field_4D8 = 0;
    field_4C0 = 1500;
    field_2AE = 150;
}

// 0x575090
CProjectileArea850384::CProjectileArea850384()
    : CProjectileArea(256)
{
    field_66E = 0;
    field_2AE = 250;
}

// 0x5752E0
void CProjectileArea850384::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x5756C0
CProjectileArea850420::CProjectileArea850420()
    : CProjectileArea(2000)
{
    field_2BE.Set(CAIObjectType::NOT_SPRITE);
}

// 0x575720
void CProjectileArea850420::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x575B00
CProjectileArea8504BC::CProjectileArea8504BC(SHORT a1)
    : CProjectileArea(a1)
{
}

// 0x575B30
void CProjectileArea8504BC::Virtual94(std::list<LONG>& targets)
{
    // TODO: Incomplete.
}

// 0x575B90
CProjectileArea850558::CProjectileArea850558()
    : CProjectileArea(256)
{
    field_4E2.field_10 = "ARE_P27";
    field_2AE = 350;
}

// 0x575C90
void CProjectileArea850558::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x575EA0
CProjectileArea85072C::CProjectileArea85072C()
    : CProjectileArea(256)
{
    field_65E = 0;
    field_4E2.field_0 = "DBreatH";
    field_4E2.field_10 = "ARE_M21";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "DBreatA";
    field_540.field_20.sub_586A90(TRUE);
    field_4D8 = 0;
    field_1DE = 0;
    field_540.field_35 = TRUE;
    field_540.field_36 = TRUE;
    field_540.field_38 = 500;
    field_4D4 = 10000;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 200;
}

// 0x576040
void CProjectileArea85072C::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x799CA0
void CProjectileArea85072C::OnArrival()
{
}

// 0x576080
CProjectileArea8507C8::CProjectileArea8507C8()
    : CProjectileArea(256)
{
    field_2AE = 450;
}

// 0x5760B0
void CProjectileArea8507C8::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x576150
void CProjectileArea8507C8::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x576310
std::list<LONG> CProjectileArea8507C8::Virtual90()
{
    // TODO: Incomplete.

    return std::list<LONG>();
}

// 0x5765F0
void CProjectileArea8507C8::Virtual94(std::list<LONG>& targets)
{
    // TODO: Incomplete.
}

// NOTE: Inlines `CProjectileArea8507C8` constructor.
//
// 0x576770
CProjectileArea850864::CProjectileArea850864()
{
}

// 0x5767A0
void CProjectileArea850864::Virtual94(std::list<LONG>& targets)
{
    // TODO: Incomplete.
}

// NOTE: Inlines `CProjectileArea8507C8` constructor.
//
// 0x576870
CProjectileArea850900::CProjectileArea850900()
{
}

// 0x5768C0
void CProjectileArea850900::Virtual94(std::list<LONG>& targets)
{
    // TODO: Incomplete.
}

// 0x576990
CProjectileArea85099C::CProjectileArea85099C()
    : CProjectileArea(256)
{
    field_65E = FALSE;
    field_660 = 0;

    const char* szTravel = "FirebaT";
    if (strlen(szTravel) != 0) {
        field_1DE = 1;
        SetVidCell(CResRef(szTravel));
        field_4E0 = TRUE;
    } else {
        field_1DE = 0;
        field_4E0 = FALSE;
    }

    m_fireSoundRef = "TRA_06";
    m_vfx.sub_586A90(TRUE);
    field_4E2.field_0 = "FirebaX";
    field_4E2.field_10 = "RNG_M03";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "FirebaR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 20;
    field_540.field_0 = "FirebaA";
    field_540.field_20.sub_586A90(TRUE);
    field_4D8 = 0;
    m_speed *= 2;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_36 = TRUE;
    field_4D4 = 10000;
    field_4DC = 10;
    field_4C0 = 45;
    field_1D8 = 16;
    field_2AE = 300;
}

// 0x576BA0
void CProjectileArea85099C::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x576CE0
void CProjectileArea85099C::OnArrival()
{
    field_2B6 = 1;
}

// 0x576CF0
CProjectileArea850A38::CProjectileArea850A38()
    : CProjectileArea(256)
{
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 300;
}

// 0x576DC0
void CProjectileArea850A38::Virtual94(std::list<LONG>& targets)
{
    // TODO: Incomplete.
}

// 0x577590
CProjectileArea850AD4::CProjectileArea850AD4()
    : CProjectileArea(100)
{
    const char* szTravel = "BIGBOLDR";
    if (strlen(szTravel) != 0) {
        field_1DE = 1;
        SetVidCell(CResRef(szTravel));
        field_4E0 = TRUE;
    } else {
        field_1DE = 0;
        field_4E0 = FALSE;
    }

    field_540.field_34 = TRUE;
    field_540.field_35 = FALSE;
    field_4D4 = 10;
    field_4D8 = 0;
    field_4DC = 10;
    field_1D8 = 1;
    field_2AE = 100;
    m_speed = 7;
    m_fireSoundRef = CResRef("AM6103e");
    m_arrivalSoundRef = CResRef("AM5101e");
}

// 0x5776E0
void CProjectileArea850AD4::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x5778A0
CProjectileArea850B70::CProjectileArea850B70()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "SoPainX";
    field_4E2.field_10 = "EFF_P49";
    field_4E2.field_20.sub_586A90(TRUE);
    field_2AE = 300;
}

// 0x5779A0
void CProjectileArea850B70::Virtual94(std::list<LONG>& targets)
{
    // TODO: Incomplete.
}

// 0x577B20
CProjectileArea850C0C::CProjectileArea850C0C()
    : CProjectileArea(256)
{
    m_fireSoundRef = CResRef("TRA_04A");
    m_arrivalSoundRef = CResRef("CAS_P02");
    field_4E2.field_0 = "AbjuraX";
    field_4E2.field_10 = "ARE_M20";
    field_4E2.field_20.sub_586A90(TRUE);
    field_2AE = 150;
}

// 0x577C70
void CProjectileArea850C0C::Virtual98(LONG nTargetId)
{
    // TODO: Incomplete.
}
