#include "CGameSparkle.h"

#include <stdlib.h>

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfinity.h"
#include "CVidInf.h"
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
    m_nBurstMax = a3;
    m_nGravity = 50;
    m_nBurstCount = 0;

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


// 0x51C330
// NOTE: 0x51C310 is the scalar deleting destructor (compiler-generated).
CGameSparkle::~CGameSparkle()
{
    POSITION pos = m_particleList.GetHeadPosition();
    while (pos != NULL) {
        delete m_particleList.GetNext(pos);
    }
    m_particleList.RemoveAll();
}

// 0x51CE50
void CGameSparkle::sub_51CE50()
{
    LONG nGravity = 50;
    SHORT nDirection;
    LONG dx;
    LONG dy;

    // NOTE: Inlined eight-direction test from `m_pos` towards `m_velocity`
    // (the explosion target).
    if (m_velocity.y >= m_pos.y) {
        if (m_velocity.x >= m_pos.x) {
            nDirection = CGameSprite::DIR_E;
            dx = m_velocity.x - m_pos.x;
            dy = m_velocity.y - m_pos.y;
            if (27 * dy > 16 * dx) {
                nDirection = CGameSprite::DIR_SE;
                if (7 * dy > 38 * dx) {
                    nDirection = CGameSprite::DIR_S;
                }
            }
        } else {
            nDirection = CGameSprite::DIR_W;
            dx = m_pos.x - m_velocity.x;
            dy = m_velocity.y - m_pos.y;
            if (27 * dy > 16 * dx) {
                nDirection = CGameSprite::DIR_SW;
                if (7 * dy > 38 * dx) {
                    nDirection = CGameSprite::DIR_S;
                }
            }
        }
    } else {
        if (m_velocity.x >= m_pos.x) {
            nDirection = CGameSprite::DIR_E;
            dy = m_pos.y - m_velocity.y;
            dx = m_velocity.x - m_pos.x;
            if (27 * dy > 16 * dx) {
                nDirection = CGameSprite::DIR_NE;
                if (7 * dy > 38 * dx) {
                    nDirection = CGameSprite::DIR_N;
                }
            }
        } else {
            nDirection = CGameSprite::DIR_W;
            dy = m_pos.y - m_velocity.y;
            dx = m_pos.x - m_velocity.x;
            if (27 * dy > 16 * dx) {
                nDirection = CGameSprite::DIR_NW;
                if (7 * dy > 38 * dx) {
                    nDirection = CGameSprite::DIR_N;
                }
            }
        }
    }

    m_rBounding.SetRect(max(m_pos.x - 50, 0),
        max(m_pos.y - m_posZ - 50, 0),
        m_pos.x + 50,
        m_pos.y - m_posZ + 50);

    LONG nVelX1;
    LONG nVelY1;
    LONG nVelX2;
    LONG nVelY2;
    LONG nVelX3;
    LONG nVelY3;

    // NOTE: The original's jump table has a default case that reads
    // uninitialised locals; it is unreachable.
    switch (nDirection) {
    case 0: // CGameSprite::DIR_S
        nVelX1 = 700;
        nVelY1 = 700;
        nVelX2 = 0;
        nVelY2 = 1000;
        nVelX3 = -700;
        nVelY3 = 700;
        break;
    case 2: // CGameSprite::DIR_SW
        nVelX1 = 0;
        nVelY1 = 1000;
        nVelX2 = -700;
        nVelY2 = 700;
        nVelX3 = -1000;
        nVelY3 = 0;
        break;
    case 4: // CGameSprite::DIR_W
        nVelX1 = -700;
        nVelY1 = 700;
        nVelX2 = -1000;
        nVelY2 = 0;
        nVelX3 = -700;
        nVelY3 = -700;
        break;
    case 6: // CGameSprite::DIR_NW
        nVelX1 = -1000;
        nVelY1 = 0;
        nVelX2 = -700;
        nVelY2 = -700;
        nVelX3 = 0;
        nVelY3 = -1000;
        break;
    case 8: // CGameSprite::DIR_N
        nVelX1 = -700;
        nVelY1 = -700;
        nVelX2 = 0;
        nVelY2 = -1000;
        nVelX3 = 700;
        nVelY3 = -700;
        break;
    case 10: // CGameSprite::DIR_NE
        nVelX1 = 0;
        nVelY1 = -1000;
        nVelX2 = 700;
        nVelY2 = -700;
        nVelX3 = 1000;
        nVelY3 = 0;
        break;
    case 12: // CGameSprite::DIR_E
        nVelX1 = 700;
        nVelY1 = -700;
        nVelX2 = 1000;
        nVelY2 = 0;
        nVelX3 = 700;
        nVelY3 = 700;
        break;
    case 14: // CGameSprite::DIR_SE
        nVelX1 = 1000;
        nVelY1 = 0;
        nVelX2 = 700;
        nVelY2 = 700;
        nVelX3 = 0;
        nVelY3 = 1000;
        break;
    }

    LONG nBaseY = m_pos.y - 10;
    LONG nPosZ = m_posZ << CParticle::RESOLUTION_INC;
    LONG nBaseX = m_pos.x - 10;
    COLORREF rgbColor = g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(m_aColors[0]));

    // NOTE: The velocities are truncated to a SHORT.
    for (INT k = 0; k < 14; k++) {
        LONG r1 = rand() % 10;
        LONG r2 = rand() % 10;

        LONG nPosX = (r1 * 2 + nBaseX) << CParticle::RESOLUTION_INC;
        LONG nPosY = ((r2 * 2 + nBaseY) * 4 / 3) << CParticle::RESOLUTION_INC;

        m_particleList.AddTail(new CParticle(nPosX,
            nPosY,
            nPosZ,
            static_cast<SHORT>(nVelX1),
            static_cast<SHORT>(nVelY1),
            1000,
            rgbColor,
            0,
            nGravity,
            CParticle::CONSTANT_VELOCITY));

        nGravity += 10;
    }

    nGravity = 50;
    for (INT k = 0; k < 14; k++) {
        LONG r1 = rand() % 10;
        LONG r2 = rand() % 10;

        LONG nPosX = (r1 * 2 + nBaseX) << CParticle::RESOLUTION_INC;
        LONG nPosY = ((r2 * 2 + nBaseY) * 4 / 3) << CParticle::RESOLUTION_INC;

        m_particleList.AddTail(new CParticle(nPosX,
            nPosY,
            nPosZ,
            static_cast<SHORT>(nVelX2),
            static_cast<SHORT>(nVelY2),
            1000,
            rgbColor,
            0,
            nGravity,
            CParticle::CONSTANT_VELOCITY));

        nGravity += 10;
    }

    nGravity = 50;
    for (INT k = 0; k < 14; k++) {
        LONG r1 = rand() % 10;
        LONG r2 = rand() % 10;

        LONG nPosX = (r1 * 2 + nBaseX) << CParticle::RESOLUTION_INC;
        LONG nPosY = ((r2 * 2 + nBaseY) * 4 / 3) << CParticle::RESOLUTION_INC;

        m_particleList.AddTail(new CParticle(nPosX,
            nPosY,
            nPosZ,
            static_cast<SHORT>(nVelX3),
            static_cast<SHORT>(nVelY3),
            1000,
            rgbColor,
            0,
            nGravity,
            CParticle::CONSTANT_VELOCITY));

        nGravity += 10;
    }
}

// 0x51C3D0
void CGameSparkle::AddToArea(CGameArea* pNewArea, const CPoint& pos, LONG posZ, BYTE listType)
{
    LONG nGravity = m_nGravity;

    m_aHeightThresholds[0] = posZ * 99 / 100;
    m_aHeightThresholds[1] = posZ * 85 / 100;
    m_aHeightThresholds[2] = posZ * 75 / 100;
    m_aHeightThresholds[3] = posZ * 55 / 100;

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    CGameObject::AddToArea(pNewArea, pos, posZ, listType);

    if ((m_wType & 0x4) != 0) {
        sub_51CE50();
        return;
    }

    LONG nBaseX;
    LONG nBaseY;

    if ((m_wType & 0x2) != 0) {
        nBaseX = m_pos.x + 3;
        nBaseY = m_pos.y - 5;
        m_nBurstCount = 0;
        m_rBounding.SetRect(max(m_pos.x - 13, 0),
            max(m_pos.y - posZ - 13, 0),
            m_pos.x + 13,
            m_pos.y + 13);
    } else {
        nBaseX = m_pos.x - 5;
        nBaseY = m_pos.y - 5;
        m_rBounding.SetRect(max(m_pos.x - 5, 0),
            max(m_pos.y - 5 - posZ, 0),
            m_pos.x + 4,
            m_pos.y + 4);
    }

    LONG nPosZ = posZ << CParticle::RESOLUTION_INC;

    for (SHORT k = 1; k < 15; k++) {
        LONG r1 = rand() % 10;
        LONG r2 = rand() % 10;

        LONG nPosX = (r1 + nBaseX) << CParticle::RESOLUTION_INC;
        LONG nPosY = ((r2 + nBaseY) * 4 / 3) << CParticle::RESOLUTION_INC;

        COLORREF rgbColor = (r1 & 1) != 0 ? m_aColors[1] : m_aColors[0];
        rgbColor = g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(rgbColor));

        m_particleList.AddTail(new CParticle(nPosX,
            nPosY,
            nPosZ,
            (k & 1) != 0 ? 4 : -4,
            0,
            0,
            rgbColor,
            0,
            nGravity,
            CParticle::CONSTANT_VELOCITY));

        nGravity += 10;
    }
}

// 0x51C6E0
void CGameSparkle::AIUpdate()
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CParticle.cpp
    // __LINE__: 2143
    UTIL_ASSERT(!m_particleList.IsEmpty());

    POSITION pos = m_particleList.GetHeadPosition();
    while (pos != NULL) {
        POSITION posCur = pos;
        CParticle* pParticle = m_particleList.GetNext(pos);

        LONG nHeight = pParticle->m_pos.z >> CParticle::RESOLUTION_INC;
        BYTE nScale = (nHeight & 0x3) + 1;

        // NOTE: The colour of the height band is brightened by `nScale`, per channel.
        int nBand = -1;
        if (nHeight < m_aHeightThresholds[3]) {
            nBand = 4;
        } else if (nHeight < m_aHeightThresholds[2]) {
            nBand = 3;
        } else if (nHeight < m_aHeightThresholds[1]) {
            nBand = 2;
        } else if (nHeight < m_aHeightThresholds[0]) {
            nBand = 1;
        }

        if (nBand != -1) {
            COLORREF rgbBand = m_aColors[nBand];
            LONG r = min(GetRValue(rgbBand) * nScale, 255);
            LONG g = min(GetGValue(rgbBand) * nScale, 255);
            LONG b = min(GetBValue(rgbBand) * nScale, 255);
            pParticle->m_rgbColor = RGB(r, g, b);
        }

        if ((m_wType & 0x4) == 0) {
            if ((rand() & 0x4) != 0) {
                pParticle->m_vel.x = -pParticle->m_vel.x;
                pParticle->m_vel.y = (pParticle->m_vel.y * 3) >> 2;
            }
        }

        LONG nVelZ = pParticle->m_vel.z - pParticle->m_nGravity;
        pParticle->m_pos.z = max(pParticle->m_pos.z + nVelZ, 0);

        if (pParticle->m_pos.z > 0) {
            pParticle->m_pos.x += pParticle->m_vel.x;
            pParticle->m_pos.y += pParticle->m_vel.y;
            pParticle->m_vel.z = nVelZ;
        } else {
            m_particleList.RemoveAt(posCur);
            delete pParticle;

            if (m_particleList.GetCount() == 0) {
                RemoveFromArea();

                if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                        CGameObjectArray::THREAD_ASYNCH,
                        NULL,
                        INFINITE)
                    == CGameObjectArray::SUCCESS) {
                    delete this;
                }

                return;
            }
        }
    }

    if ((m_wType & 0x2) != 0 && m_nBurstCount < m_nBurstMax) {
        LONG nGravity = 50;
        LONG nBaseX;
        LONG nBaseY;

        DWORD nBurst = m_nBurstCount;
        m_nBurstCount++;

        // NOTE: The original's jump table has a default case that reads
        // uninitialised locals; it is unreachable.
        switch (nBurst & 0x7) {
        case 0:
            nBaseX = m_pos.x + 3;
            nBaseY = m_pos.y - 5;
            break;
        case 1:
            nBaseX = m_pos.x + 5;
            nBaseY = m_pos.y + 5;
            break;
        case 2:
            nBaseX = m_pos.x - 5;
            nBaseY = m_pos.y + 3;
            break;
        case 3:
            nBaseX = m_pos.x - 15;
            nBaseY = m_pos.y + 5;
            break;
        case 4:
            nBaseX = m_pos.x - 13;
            nBaseY = m_pos.y - 5;
            break;
        case 5:
            nBaseX = m_pos.x - 15;
            nBaseY = m_pos.y - 15;
            break;
        case 6:
            nBaseX = m_pos.x - 5;
            nBaseY = m_pos.y - 13;
            break;
        case 7:
            nBaseX = m_pos.x + 5;
            nBaseY = m_pos.y - 15;
            break;
        }

        LONG nPosZ = m_posZ << CParticle::RESOLUTION_INC;
        COLORREF rgbColor = g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(m_aColors[0]));

        for (SHORT k = 1; k < 15; k++) {
            WORD r1 = rand() % 10;
            WORD r2 = rand() % 10;

            LONG nPosX = (r1 + nBaseX) << CParticle::RESOLUTION_INC;
            LONG nPosY = ((r2 + nBaseY) * 4 / 3) << CParticle::RESOLUTION_INC;

            // NOTE: The gravity is masked to a WORD.
            m_particleList.AddTail(new CParticle(nPosX,
                nPosY,
                nPosZ,
                (k & 1) != 0 ? 4 : -4,
                0,
                0,
                rgbColor,
                0,
                static_cast<WORD>(nGravity),
                CParticle::CONSTANT_VELOCITY));

            nGravity += 10;
        }
    }
}

// 0x51CC00
void CGameSparkle::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    CInfinity* pInfinity = &m_pArea->m_cInfinity;

    CRect rView(pInfinity->nCurrentX,
        pInfinity->nCurrentY,
        pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX,
        pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY);

    DWORD dwFlags = CInfinity::FXPREP_CLEARFILL | 0x1;

    if (!m_pArea->m_visibility.IsTileVisible(m_pArea->m_visibility.PointToTile(m_pos))) {
        return;
    }

    if (!rView.IntersectRect(&rView, &m_rBounding)) {
        return;
    }

    CRect rFXRect(0, 0, m_rBounding.right - m_rBounding.left, m_rBounding.bottom - m_rBounding.top);

    CPoint ptPos(0, 0);
    CPoint ptReference(0, 0);
    pInfinity->FXPrep(rFXRect, dwFlags, -1, ptPos, ptReference);

    ptReference.x = m_pos.x - ((m_rBounding.right - m_rBounding.left) >> 1);

    if (m_wType == 2) {
        ptReference.y = m_pos.y - m_posZ - 8;
    } else if (m_wType == 4) {
        ptReference.y = m_pos.y - m_posZ - 50;
    } else {
        ptReference.y = m_pos.y - m_posZ;
    }

    if (!pInfinity->FXLock(rFXRect, dwFlags)) {
        return;
    }

    POSITION pos = m_particleList.GetHeadPosition();
    while (pos != NULL) {
        CParticle* pParticle = m_particleList.GetNext(pos);

        COLORREF rgbSaved = pParticle->m_rgbColor;
        pParticle->m_rgbColor = g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(rgbSaved));

        static_cast<CVidInf*>(pInfinity->pVidMode)->FXRender(pParticle, m_rBounding, CParticle::DOTS, 5);

        pParticle->m_rgbColor = rgbSaved;
    }

    pInfinity->FXRenderClippingPolys(m_pos.x, m_pos.y, 0, CPoint(0, 0), m_rBounding, FALSE, dwFlags);
    pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
    pInfinity->FXBltFrom(a3, rFXRect, ptReference.x, ptReference.y, 0, 0, dwFlags);
}
