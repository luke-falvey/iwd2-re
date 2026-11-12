#include "CGameSparkle.h"

#include "CBaldurChitin.h"
#include "CInfGame.h"
#include "CParticle.h"
#include "CUtil.h"
#include "CWarp.h"
#include "CVidMode.h"

// 0x84D604
const WORD CGameSparkle::TYPE_EXPLOSION = 4;

// NOTE: Uninlined. The original reads each color from `SPARKLE.2DA` with the
// `C2DArray::GetAt` bounds check and `sscanf` inlined in every case.
static COLORREF GetSparkleColor(const C2DArray& table, SHORT x, SHORT y)
{
    UINT r;
    UINT g;
    UINT b;
    sscanf(table.GetAt(CPoint(x, y)), "RGB(%u,%u,%u)", &r, &g, &b);
    return RGB(r, g, b);
}

// 0x51AE40
CGameSparkle::CGameSparkle(SHORT nSparkleType, WORD wType, WORD a3)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    const C2DArray& table = pGame->GetRuleTables().m_tSparkleColors;

    m_pVidMode = g_pBaldurChitin->pActiveEngine != NULL ? g_pBaldurChitin->pActiveEngine->pVidMode : NULL;
    m_rBounding.SetRect(0, 0, 0, 0);
    m_wType = wType;
    field_BC = a3;
    field_C6 = 50;
    field_B8 = 0;

    switch (nSparkleType) {
    case 1:
        m_aColors[0] = GetSparkleColor(table, 0, 5);
        m_aColors[1] = GetSparkleColor(table, 1, 5);
        m_aColors[2] = GetSparkleColor(table, 2, 5);
        m_aColors[3] = GetSparkleColor(table, 3, 5);
        m_aColors[4] = GetSparkleColor(table, 4, 5);
        break;
    case 2:
        m_aColors[0] = GetSparkleColor(table, 0, 0);
        m_aColors[1] = GetSparkleColor(table, 1, 0);
        m_aColors[2] = GetSparkleColor(table, 2, 0);
        m_aColors[3] = GetSparkleColor(table, 3, 0);
        m_aColors[4] = GetSparkleColor(table, 4, 0);
        break;
    case 3:
        m_aColors[0] = GetSparkleColor(table, 0, 6);
        m_aColors[1] = GetSparkleColor(table, 1, 6);
        m_aColors[2] = GetSparkleColor(table, 2, 6);
        m_aColors[3] = GetSparkleColor(table, 3, 6);
        m_aColors[4] = GetSparkleColor(table, 4, 6);
        break;
    case 4:
        m_aColors[0] = GetSparkleColor(table, 0, 1);
        m_aColors[1] = GetSparkleColor(table, 1, 1);
        m_aColors[2] = GetSparkleColor(table, 2, 1);
        m_aColors[3] = GetSparkleColor(table, 3, 1);
        m_aColors[4] = GetSparkleColor(table, 4, 1);
        break;
    case 5:
        m_aColors[0] = GetSparkleColor(table, 0, 8);
        m_aColors[1] = GetSparkleColor(table, 1, 8);
        m_aColors[2] = GetSparkleColor(table, 2, 8);
        m_aColors[3] = GetSparkleColor(table, 3, 8);
        m_aColors[4] = GetSparkleColor(table, 4, 8);
        break;
    case 6:
        m_aColors[0] = GetSparkleColor(table, 0, 2);
        m_aColors[1] = GetSparkleColor(table, 1, 2);
        m_aColors[2] = GetSparkleColor(table, 2, 2);
        m_aColors[3] = GetSparkleColor(table, 3, 2);
        m_aColors[4] = GetSparkleColor(table, 4, 2);
        break;
    case 7:
        m_aColors[0] = GetSparkleColor(table, 0, 7);
        m_aColors[1] = GetSparkleColor(table, 1, 7);
        m_aColors[2] = GetSparkleColor(table, 2, 7);
        m_aColors[3] = GetSparkleColor(table, 3, 7);
        m_aColors[4] = GetSparkleColor(table, 4, 7);
        break;
    case 8:
        m_aColors[0] = GetSparkleColor(table, 0, 9);
        m_aColors[1] = GetSparkleColor(table, 1, 9);
        m_aColors[2] = GetSparkleColor(table, 2, 9);
        m_aColors[3] = GetSparkleColor(table, 3, 9);
        m_aColors[4] = GetSparkleColor(table, 4, 9);
        break;
    case 9:
        m_aColors[0] = GetSparkleColor(table, 0, 3);
        m_aColors[1] = GetSparkleColor(table, 1, 3);
        m_aColors[2] = GetSparkleColor(table, 2, 3);
        m_aColors[3] = GetSparkleColor(table, 3, 3);
        m_aColors[4] = GetSparkleColor(table, 4, 3);
        break;
    case 10:
        m_aColors[0] = GetSparkleColor(table, 0, 4);
        m_aColors[1] = GetSparkleColor(table, 1, 4);
        m_aColors[2] = GetSparkleColor(table, 2, 4);
        m_aColors[3] = GetSparkleColor(table, 3, 4);
        m_aColors[4] = GetSparkleColor(table, 4, 4);
        break;
    case 11:
        m_aColors[0] = GetSparkleColor(table, 0, 10);
        m_aColors[1] = GetSparkleColor(table, 1, 10);
        m_aColors[2] = GetSparkleColor(table, 2, 10);
        m_aColors[3] = GetSparkleColor(table, 3, 10);
        m_aColors[4] = GetSparkleColor(table, 4, 10);
        break;
    case 12:
        m_aColors[0] = GetSparkleColor(table, 0, 11);
        m_aColors[1] = GetSparkleColor(table, 1, 11);
        m_aColors[2] = GetSparkleColor(table, 2, 11);
        m_aColors[3] = GetSparkleColor(table, 3, 11);
        m_aColors[4] = GetSparkleColor(table, 4, 11);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CParticle.cpp
        // __LINE__: 1975
        UTIL_ASSERT(FALSE);
        break;
    }
}

// NOTE: Inlined in `CGameEffectSparkle::ApplyEffect` (name is a guess).
void CGameSparkle::SetExplosionVelocity(const CPoint& velocity)
{
    // __FILE__: .\Include\CParticle.h
    // __LINE__: 511
    UTIL_ASSERT(m_wType & TYPE_EXPLOSION);

    m_velocity = velocity;
}

// 0x51C310
CGameSparkle::~CGameSparkle()
{
    // TODO: Incomplete. Body at 0x51C330 (deletes the particles in the list).
}

// 0x51C3D0
void CGameSparkle::AddToArea(CGameArea* pNewArea, const CPoint& pos, LONG posZ, BYTE listType)
{
    // TODO: Incomplete.
}

// 0x51C6E0
void CGameSparkle::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x51CC00
void CGameSparkle::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}
