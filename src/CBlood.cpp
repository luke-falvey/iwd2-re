#include "CBlood.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CInfinity.h"
#include "CParticle.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVidMode.h"

// 0x84D5AC
const BYTE CBlood::NUM_COLORS = CBLOOD_NUM_COLORS;

// 0x84D5B0
const COLORREF CBlood::RGB_COLOR1 = RGB(109, 36, 0);

// 0x84D5B4
const COLORREF CBlood::RGB_COLOR2 = RGB(159, 42, 0);

// 0x84D5B8
const COLORREF CBlood::RGB_COLOR3 = RGB(180, 50, 15);

// 0x84D5BC
const USHORT CBlood::SQUIRT_HIGH = 0;

// 0x84D5BE
const USHORT CBlood::SQUIRT_MEDIUM = 1;

// 0x84D5C0
const USHORT CBlood::SQUIRT_LOW = 2;

// 0x84D5C2
const USHORT CBlood::GUSH_HIGH = 3;

// 0x84D5C4
const USHORT CBlood::GUSH_MEDIUM = 4;

// 0x84D5C6
const USHORT CBlood::GUSH_LOW = 5;

// 0x84D5C8
const USHORT CBlood::PULSATING_ARTERY = 6;

// 0x84D5CA
const USHORT CBlood::EXPLODING_DEATH = 7;

// 0x84D5CC
const SHORT CBlood::STREAM_LENGTH = 5;

// 0x84D5CE
const SHORT CBlood::SQUIRT_STREAMS = 20;

// 0x84D5D0
const SHORT CBlood::FULL_VELOCITY = 3000;

// 0x84D5D2
const SHORT CBlood::GUSH_HEIGHT_VELOCITY = 4000;

// 0x84D5D4
const SHORT CBlood::SQUIRT_HEIGHT_VELOCITY = 8000;

// 0x84D5D6
const SHORT CBlood::VELOCITY_INC = 100;

// 0x84D5D8
const SHORT CBlood::SPLASH_LENGTH = 2;

// 0x84D5DA
const SHORT CBlood::SQUIRT_LIFESPAN = 15;

// 0x84D5DC
const USHORT CBlood::NUM_EFFECTS = 8;

// 0x5195B0
CBlood::CBlood(CGameArea* pArea, LONG nX, LONG nY, LONG nZ, SHORT nDirection, USHORT nType, LONG nCharacterHeight)
{
    m_pArea = pArea;
    m_pos.x = nX;
    m_pos.y = nY;
    m_posZ = nZ;
    m_nCharHeight = nCharacterHeight;
    m_nDirection = nDirection;
    m_nTimeStamp = 0;
    m_aColors[0] = RGB_COLOR1;
    m_aColors[1] = RGB_COLOR2;
    m_aColors[2] = RGB_COLOR3;
    m_particleType = CParticle::CONNECT;
    m_bloodType = nType;
    m_bLeavePool = TRUE;
    CalcBoundingRect();
    GenerateParticles();
}

// 0x519680
CBlood::~CBlood()
{
    POSITION pos;

    pos = m_lstSplashParticles.GetHeadPosition();
    while (pos != NULL) {
        delete m_lstSplashParticles.GetNext(pos);
    }
    m_lstSplashParticles.RemoveAll();

    pos = m_particleList.GetHeadPosition();
    while (pos != NULL) {
        delete m_particleList.GetNext(pos);
    }
    m_particleList.RemoveAll();
}

// 0x519740
BOOLEAN CBlood::AsynchronousUpdate()
{
    m_nTimeStamp++;

    if (!m_lstSplashParticles.IsEmpty()) {
        POSITION pos = m_lstSplashParticles.GetHeadPosition();
        while (pos != NULL) {
            POSITION posOld = pos;
            CParticle* pParticle = m_lstSplashParticles.GetNext(pos);
            if (pParticle->AsynchronousUpdate() == CParticle::DEAD) {
                m_lstSplashParticles.RemoveAt(posOld);
                delete pParticle;
            }
        }
    }

    if (!m_particleList.IsEmpty()) {
        POSITION pos = m_particleList.GetHeadPosition();
        while (pos != NULL) {
            POSITION posOld = pos;
            CParticle* pParticle = m_particleList.GetNext(pos);
            if (pParticle->m_nRenderTime > m_nTimeStamp) {
                break;
            }

            // NOTE: The case labels are the values of `CParticle::AIRBORN`,
            // `CParticle::GROUNDED` and `CParticle::DEAD`.
            switch (pParticle->AsynchronousUpdate()) {
            case 0:
                break;
            case 1:
                if (!pParticle->m_bTag) {
                    m_lstSplashParticles.AddTail(new CParticle(*pParticle));

                    m_lstSplashParticles.GetTail()->m_pos.x = pParticle->m_pos.x;
                    m_lstSplashParticles.GetTail()->m_pos.y = pParticle->m_pos.y;
                    m_lstSplashParticles.GetTail()->m_pos.z = 0;

                    LONG nVelZ = (rand() % 4 + 1) * CParticle::GRAVITY;
                    CParticle* pSplash = m_lstSplashParticles.GetTail();
                    pSplash->m_vel.x = pParticle->m_vel.x;
                    pSplash->m_vel.y = 3 * pParticle->m_vel.y >> 2;
                    pSplash->m_vel.z = nVelZ;

                    m_lstSplashParticles.GetTail()->m_nTailLength = 2;
                }
                break;
            case 2:
                m_particleList.RemoveAt(posOld);
                delete pParticle;
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CParticle.cpp
                // __LINE__: 333
                UTIL_ASSERT(FALSE);
                break;
            }
        }
    }

    return BloodLeft();
}

// 0x519920
void CBlood::Render(CVidMode* pVidMode, INT nSurface)
{
    CInfinity* pInfinity = &m_pArea->m_cInfinity;
    COLORREF rgbLighting = pInfinity->GetGlobalLighting();

    CRect rView(pInfinity->nCurrentX,
        pInfinity->nCurrentY,
        pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX,
        pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY);

    DWORD dwFlags = CInfinity::FXPREP_CLEARFILL | 0x1;

    if (BloodLeft()) {
        if (rView.IntersectRect(&rView, &m_rBounding)) {
            CRect rFXRect(0, 0, m_rBounding.right - m_rBounding.left, m_rBounding.bottom - m_rBounding.top);

            CPoint ptPos(0, 0);
            CPoint ptReference(0, 0);
            pInfinity->FXPrep(rFXRect, dwFlags, -1, ptPos, ptReference);

            if (pInfinity->FXLock(rFXRect, dwFlags)) {
                POSITION pos = m_lstSplashParticles.GetHeadPosition();
                while (pos != NULL) {
                    CParticle* pParticle = m_lstSplashParticles.GetNext(pos);

                    COLORREF rgbSaved = pParticle->m_rgbColor;
                    pParticle->m_rgbColor = RGB((GetRValue(rgbSaved) * GetRValue(rgbLighting)) >> 8,
                        (GetGValue(rgbSaved) * GetGValue(rgbLighting)) >> 8,
                        (GetBValue(rgbSaved) * GetBValue(rgbLighting)) >> 8);

                    static_cast<CVidInf*>(pInfinity->pVidMode)->FXRender(pParticle, m_rBounding, CParticle::DOTS, 5);

                    pParticle->m_rgbColor = rgbSaved;
                }

                pos = m_particleList.GetHeadPosition();
                while (pos != NULL) {
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\CParticle.cpp
                    // __LINE__: 419
                    UTIL_ASSERT(!m_particleList.IsEmpty());

                    CParticle* pParticle = m_particleList.GetNext(pos);
                    if (pParticle->m_nRenderTime > m_nTimeStamp) {
                        break;
                    }

                    COLORREF rgbSaved = pParticle->m_rgbColor;
                    pParticle->m_rgbColor = RGB((GetRValue(rgbSaved) * GetRValue(rgbLighting)) >> 8,
                        (GetGValue(rgbSaved) * GetGValue(rgbLighting)) >> 8,
                        (GetBValue(rgbSaved) * GetBValue(rgbLighting)) >> 8);

                    static_cast<CVidInf*>(pInfinity->pVidMode)->FXRender(pParticle,
                        m_rBounding,
                        m_particleType,
                        m_particleType == CParticle::BLOB ? rand() % 5 + 1 : 5);

                    pParticle->m_rgbColor = rgbSaved;
                }

                CRect rGCBounds(m_rBounding.left,
                    m_rBounding.top + m_posZ,
                    m_rBounding.right,
                    m_rBounding.bottom + m_posZ);

                CPoint ptClipRef(0, 0);
                pInfinity->FXRenderClippingPolys(m_pos.x, m_pos.y, -m_posZ, ptClipRef, rGCBounds, FALSE, dwFlags);

                CPoint ptUnlockRef(0, 0);
                pInfinity->FXUnlock(dwFlags, NULL, ptUnlockRef);

                pInfinity->FXBltFrom(nSurface, rFXRect, m_refPoint.x, m_refPoint.y, 0, 0, dwFlags);
            }
        }
    }
}

// 0x519C40
void CBlood::CalcBoundingRect()
{
    switch (m_bloodType) {
    case PULSATING_ARTERY:
        switch (m_nDirection) {
        case 0:
            m_rBounding.SetRect(((m_pos.x - 0x14) < 0) ? 0 : (m_pos.x - 0x14),
                                ((m_pos.y - m_posZ - 0x28) < 0) ? 0 : (m_pos.y - m_posZ - 0x28),
                                m_pos.x + 0x14,
                                m_pos.y - m_posZ + 0x28);
            m_refPoint.x = m_pos.x - 0x14;
            m_refPoint.y = m_pos.y - m_posZ - 0x28;
            break;
        case 1:
        case 2:
        case 3:
            m_rBounding.SetRect(((m_pos.x - 0x41) < 0) ? 0 : (m_pos.x - 0x41),
                                ((m_pos.y - m_posZ - 0x28) < 0) ? 0 : (m_pos.y - m_posZ - 0x28),
                                m_pos.x + 0xA,
                                m_pos.y - m_posZ + 0x28);
            m_refPoint.x = m_pos.x - 0x41;
            m_refPoint.y = m_pos.y - m_posZ - 0x28;
            break;
        case 4:
            m_rBounding.SetRect(((m_pos.x - 0x41) < 0) ? 0 : (m_pos.x - 0x41),
                                ((m_pos.y - m_posZ - 0x28) < 0) ? 0 : (m_pos.y - m_posZ - 0x28),
                                m_pos.x + 0xF,
                                m_pos.y - m_posZ + 0x28);
            m_refPoint.x = m_pos.x - 0x41;
            m_refPoint.y = m_pos.y - m_posZ - 0x28;
            break;
        case 5:
        case 6:
        case 7:
            m_rBounding.SetRect(((m_pos.x - 0x41) < 0) ? 0 : (m_pos.x - 0x41),
                                ((m_pos.y - m_posZ - 0x32) < 0) ? 0 : (m_pos.y - m_posZ - 0x32),
                                m_pos.x + 0x14,
                                m_pos.y - m_posZ + 0x28);
            m_refPoint.x = m_pos.x - 0x41;
            m_refPoint.y = m_pos.y - m_posZ - 0x32;
            break;
        case 8:
            m_rBounding.SetRect(((m_pos.x - 0x14) < 0) ? 0 : (m_pos.x - 0x14),
                                ((m_pos.y - m_posZ - 0x32) < 0) ? 0 : (m_pos.y - m_posZ - 0x32),
                                m_pos.x + 0x14,
                                m_pos.y - m_posZ + 0x14);
            m_refPoint.x = m_pos.x - 0x14;
            m_refPoint.y = m_pos.y - m_posZ - 0x32;
            break;
        case 9:
        case 10:
        case 11:
            m_rBounding.SetRect(((m_pos.x - 0xA) < 0) ? 0 : (m_pos.x - 0xA),
                                ((m_pos.y - m_posZ - 0x32) < 0) ? 0 : (m_pos.y - m_posZ - 0x32),
                                m_pos.x + 0x3C,
                                m_pos.y - m_posZ + 0x28);
            m_refPoint.x = m_pos.x - 0xA;
            m_refPoint.y = m_pos.y - m_posZ - 0x32;
            break;
        case 12:
            m_rBounding.SetRect(((m_pos.x - 0xA) < 0) ? 0 : (m_pos.x - 0xA),
                                ((m_pos.y - m_posZ - 0x28) < 0) ? 0 : (m_pos.y - m_posZ - 0x28),
                                m_pos.x + 0x46,
                                m_pos.y - m_posZ + 0x28);
            m_refPoint.x = m_pos.x - 0xA;
            m_refPoint.y = m_pos.y - m_posZ - 0x28;
            break;
        case 13:
        case 14:
        case 15:
            m_rBounding.SetRect(((m_pos.x - 0xA) < 0) ? 0 : (m_pos.x - 0xA),
                                ((m_pos.y - m_posZ - 0x28) < 0) ? 0 : (m_pos.y - m_posZ - 0x28),
                                m_pos.x + 0x41,
                                m_pos.y - m_posZ + 0x2D);
            m_refPoint.x = m_pos.x - 0xA;
            m_refPoint.y = m_pos.y - m_posZ - 0x28;
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CParticle.cpp
            // __LINE__: 559
            UTIL_ASSERT(FALSE);
            break;
        }
        break;
    case EXPLODING_DEATH: {
        CSize size;
        g_pBaldurChitin->GetCurrentVideoMode()->GetFXSize(size);

        size.cx--;
        size.cy--;

        m_rBounding.SetRect(((m_pos.x - (size.cx >> 1)) < 0) ? 0 : (m_pos.x - (size.cx >> 1)),
                            ((m_pos.y - m_posZ - (size.cy >> 1)) < 0) ? 0 : (m_pos.y - m_posZ - (size.cy >> 1)),
                            m_pos.x + (size.cx >> 1),
                            m_pos.y - m_posZ + (size.cy >> 1));
        m_refPoint.x = m_pos.x - (size.cx >> 1);
        m_refPoint.y = m_pos.y - (size.cy >> 1) - m_posZ;
        break;
    }
    default:
        switch (m_nDirection) {
        case 0:
            m_rBounding.SetRect(((m_pos.x - 0x2D) < 0) ? 0 : (m_pos.x - 0x2D),
                                ((m_pos.y - m_posZ - 0xA) < 0) ? 0 : (m_pos.y - m_posZ - 0xA),
                                m_pos.x + 0x2D,
                                m_pos.y - m_posZ + 0x50);
            m_refPoint.x = m_pos.x - 0x2D;
            m_refPoint.y = m_pos.y - m_posZ - 0xA;
            break;
        case 1:
        case 2:
        case 3:
            m_rBounding.SetRect(((m_pos.x - 0x41) < 0) ? 0 : (m_pos.x - 0x41),
                                ((m_pos.y - m_posZ - 0xA) < 0) ? 0 : (m_pos.y - m_posZ - 0xA),
                                m_pos.x + 0xA,
                                m_pos.y - m_posZ + 0x50);
            m_refPoint.x = m_pos.x - 0x41;
            m_refPoint.y = m_pos.y - m_posZ - 0xA;
            break;
        case 4:
            m_rBounding.SetRect(((m_pos.x - 0x41) < 0) ? 0 : (m_pos.x - 0x41),
                                ((m_pos.y - m_posZ - 0xF) < 0) ? 0 : (m_pos.y - m_posZ - 0xF),
                                m_pos.x + 0xF,
                                m_pos.y - m_posZ + 0x46);
            m_refPoint.x = m_pos.x - 0x41;
            m_refPoint.y = m_pos.y - m_posZ - 0xF;
            break;
        case 5:
        case 6:
        case 7:
            m_rBounding.SetRect(((m_pos.x - 0x41) < 0) ? 0 : (m_pos.x - 0x41),
                                ((m_pos.y - m_posZ - 0x1E) < 0) ? 0 : (m_pos.y - m_posZ - 0x1E),
                                m_pos.x + 0x14,
                                m_pos.y - m_posZ + 0x3C);
            m_refPoint.x = m_pos.x - 0x41;
            m_refPoint.y = m_pos.y - m_posZ - 0x1E;
            break;
        case 8:
            m_rBounding.SetRect(((m_pos.x - 0x2D) < 0) ? 0 : (m_pos.x - 0x2D),
                                ((m_pos.y - m_posZ - 0x1E) < 0) ? 0 : (m_pos.y - m_posZ - 0x1E),
                                m_pos.x + 0x2D,
                                m_pos.y - m_posZ + 0x32);
            m_refPoint.x = m_pos.x - 0x2D;
            m_refPoint.y = m_pos.y - m_posZ - 0x1E;
            break;
        case 9:
        case 10:
        case 11:
            m_rBounding.SetRect(((m_pos.x - 0xA) < 0) ? 0 : (m_pos.x - 0xA),
                                ((m_pos.y - m_posZ - 0x1E) < 0) ? 0 : (m_pos.y - m_posZ - 0x1E),
                                m_pos.x + 0x3C,
                                m_pos.y - m_posZ + 0x32);
            m_refPoint.x = m_pos.x - 0xA;
            m_refPoint.y = m_pos.y - m_posZ - 0x1E;
            break;
        case 12:
            m_rBounding.SetRect(((m_pos.x - 0xA) < 0) ? 0 : (m_pos.x - 0xA),
                                ((m_pos.y - m_posZ - 0xA) < 0) ? 0 : (m_pos.y - m_posZ - 0xA),
                                m_pos.x + 0x46,
                                m_pos.y - m_posZ + 0x46);
            m_refPoint.x = m_pos.x - 0xA;
            m_refPoint.y = m_pos.y - m_posZ - 0xA;
            break;
        case 13:
        case 14:
        case 15:
            m_rBounding.SetRect(((m_pos.x - 0xA) < 0) ? 0 : (m_pos.x - 0xA),
                                ((m_pos.y - m_posZ - 0xA) < 0) ? 0 : (m_pos.y - m_posZ - 0xA),
                                m_pos.x + 0x41,
                                m_pos.y - m_posZ + 0x4B);
            m_refPoint.x = m_pos.x - 0xA;
            m_refPoint.y = m_pos.y - m_posZ - 0xA;
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CParticle.cpp
            // __LINE__: 624
            UTIL_ASSERT(FALSE);
            break;
        }
        break;
    }
}

// 0x51A2A0
SHORT CBlood::GenerateExplodingBlood(LONG nX, LONG nY, LONG nZ)
{
    CParticle particle;
    LONG nParticles = 0;

    LONG nHeight = m_nCharHeight << CParticle::RESOLUTION_INC;
    LONG nMinZ = 2 * nHeight / 5;
    LONG nMaxZ = 8 * nHeight / 5;

    particle.m_nRenderTime = m_nTimeStamp;

    // NOTE: Original code computes `abs(-6000) * 2` at runtime.
    LONG nVelRange = abs(-6000) * 2;
    LONG nZRange = nMaxZ - nMinZ;

    m_particleType = CParticle::BLOB;
    particle.m_nTailLength = 1;

    LONG nTotal = 0;
    do {
        LONG nRandVelX = nVelRange != 0 ? rand() % nVelRange : 0;
        LONG nVelX = nRandVelX - 6000;
        LONG nRandVelY = nVelRange != 0 ? rand() % nVelRange : 0;
        LONG nRandZ = nZRange != 0 ? rand() % nZRange : 0;
        SHORT nTrail = rand() % 20;

        // NOTE: `nZ` is not used.
        particle.m_pos.x = nX;
        particle.m_pos.y = nY;
        particle.m_pos.z = nRandZ + nMinZ;
        particle.m_rgbColor = m_aColors[rand() % 2];
        m_particleList.AddTail(new CParticle(particle));

        nParticles++;

        if (nTrail > 0) {
            nParticles += nTrail;

            LONG nVelY = nRandVelY - 6000;
            LONG nLoops = nTrail;
            do {
                particle.m_rgbColor = m_aColors[rand() % 2];
                particle.m_vel.z = rand() % 8000;
                particle.m_vel.x = nVelX;
                particle.m_vel.y = 3 * nVelY >> 2;
                m_particleList.AddTail(new CParticle(particle));

                nVelX += 50;
                nVelY += 50;
            } while (--nLoops != 0);
        }

        nTotal += rand() % 5;
    } while (nTotal < 30);

    return static_cast<SHORT>(nParticles);
}

// 0x51A500
SHORT CBlood::GenerateParticles()
{
    CParticle particle;
    SHORT nTime = m_nTimeStamp;
    LONG nX = m_pos.x << CParticle::RESOLUTION_INC;
    LONG nY = (m_pos.y << CParticle::RESOLUTION_INC) * 4 / 3;
    LONG nZ = m_posZ << CParticle::RESOLUTION_INC;
    LONG nParticles = 0;
    BOOLEAN bFirst = FALSE;
    BOOLEAN bLast = FALSE;
    BOOLEAN bBody = FALSE;
    SHORT nStreams;
    LONG nVelocity;
    LONG nHeightVelocity;
    LONG nCount;

    // NOTE: `nVelocity` is not initialized for an unknown blood type.
    switch (m_bloodType) {
    case EXPLODING_DEATH:
        return GenerateExplodingBlood(nX, nY, nZ);
    case PULSATING_ARTERY:
        m_particleType = CParticle::CONNECT;
        particle.m_pos.x = nX;
        particle.m_pos.y = nY;
        particle.m_pos.z = 0;
        particle.m_nTailLength = 4;
        nVelocity = 750;
        nStreams = 6;
        nHeightVelocity = GUSH_HEIGHT_VELOCITY;
        nCount = 6;
        break;
    case GUSH_HIGH:
        m_particleType = CParticle::CONNECT;
        bBody = TRUE;
        particle.m_nTailLength = STREAM_LENGTH;
        nVelocity = FULL_VELOCITY;
        nStreams = STREAM_LENGTH;
        nHeightVelocity = GUSH_HEIGHT_VELOCITY;
        nCount = 20;
        break;
    case GUSH_MEDIUM:
        m_particleType = CParticle::CONNECT;
        bBody = TRUE;
        particle.m_nTailLength = STREAM_LENGTH;
        nVelocity = FULL_VELOCITY;
        nStreams = STREAM_LENGTH;
        nHeightVelocity = GUSH_HEIGHT_VELOCITY;
        nCount = 10;
        break;
    case GUSH_LOW:
        m_particleType = CParticle::CONNECT;
        bBody = TRUE;
        particle.m_nTailLength = STREAM_LENGTH;
        nVelocity = 1500;
        nStreams = STREAM_LENGTH;
        nHeightVelocity = GUSH_HEIGHT_VELOCITY;
        nCount = STREAM_LENGTH;
        break;
    case SQUIRT_HIGH:
        bLast = TRUE;
        bBody = TRUE;
        bFirst = TRUE;
        m_particleType = CParticle::DOTS;
        particle.m_nTailLength = 1;
        particle.m_nLifeSpan = SQUIRT_LIFESPAN;
        nVelocity = 1500;
        nStreams = 5;
        nHeightVelocity = SQUIRT_HEIGHT_VELOCITY;
        nCount = SQUIRT_STREAMS;
        m_bLeavePool = FALSE;
        break;
    case SQUIRT_MEDIUM:
        bBody = TRUE;
        m_particleType = CParticle::DOTS;
        particle.m_nTailLength = 1;
        particle.m_nLifeSpan = SQUIRT_LIFESPAN;
        nVelocity = 1500;
        nStreams = 5;
        nHeightVelocity = SQUIRT_HEIGHT_VELOCITY;
        nCount = 10;
        m_bLeavePool = FALSE;
        break;
    case SQUIRT_LOW:
        m_particleType = CParticle::DOTS;
        bBody = TRUE;
        particle.m_nTailLength = 1;
        particle.m_nLifeSpan = SQUIRT_LIFESPAN;
        nVelocity = 1500;
        nStreams = 5;
        nHeightVelocity = SQUIRT_HEIGHT_VELOCITY;
        nCount = 5;
        m_bLeavePool = FALSE;
        break;
    }

    // NOTE: All of these are uninitialized for an unknown direction.
    LONG nYStart;
    LONG nYInc;
    LONG nXStart;
    LONG nXInc;
    LONG nFirstX;
    LONG nFirstY;
    LONG nLastX;
    LONG nLastY;

    switch (m_nDirection) {
    case 8:
        nYStart = -nVelocity;
        nYInc = 0;
        nXInc = 100;
        nXStart = -1000;
        nFirstX = -nVelocity;
        nFirstY = -nVelocity;
        nLastX = nVelocity;
        nLastY = -nVelocity;
        break;
    case 9:
    case 10:
    case 11:
        nYStart = -1000 - nVelocity;
        nYInc = 100;
        nXStart = nVelocity - 1000;
        nXInc = 100;
        nFirstX = 0;
        nFirstY = -nVelocity;
        nLastX = nVelocity;
        nLastY = 0;
        break;
    case 12:
        nYStart = -1000;
        nYInc = 100;
        nXStart = nVelocity;
        nXInc = 0;
        nFirstX = nVelocity;
        nFirstY = -nVelocity;
        nLastX = nVelocity;
        nLastY = nVelocity;
        break;
    case 13:
    case 14:
    case 15:
        nYStart = nVelocity - 1000;
        nYInc = 100;
        nXStart = nVelocity + 1000;
        nXInc = -100;
        nFirstX = nVelocity;
        nFirstY = 0;
        nLastX = 0;
        nLastY = nVelocity;
        break;
    case 0:
        nYStart = nVelocity;
        nYInc = 0;
        nXInc = 100;
        nXStart = -1000;
        nFirstX = nVelocity;
        nFirstY = nVelocity;
        nLastX = -nVelocity;
        nLastY = nVelocity;
        break;
    case 1:
    case 2:
    case 3:
        nYStart = nVelocity - 1000;
        nYInc = 100;
        nXInc = 100;
        nXStart = -1000 - nVelocity;
        nFirstX = 0;
        nFirstY = nVelocity;
        nLastX = -nVelocity;
        nLastY = 0;
        break;
    case 4:
        nYStart = -1000;
        nYInc = 100;
        nXInc = 0;
        nXStart = -nVelocity;
        nFirstX = -nVelocity;
        nFirstY = nVelocity;
        nLastX = -nVelocity;
        nLastY = -nVelocity;
        break;
    case 5:
    case 6:
    case 7:
        nYStart = -1000 - nVelocity;
        nYInc = 100;
        nXInc = -100;
        nXStart = 1000 - nVelocity;
        nFirstX = -nVelocity;
        nFirstY = 0;
        nLastX = 0;
        nLastY = -nVelocity;
        break;
    }

    LONG nStepHeight = nHeightVelocity / nStreams;
    LONG nStepXStart = nXStart / nStreams;
    LONG nStepYStart = nYStart / nStreams;
    LONG nStepYInc = nYInc / nStreams;
    LONG nStepXInc = nXInc / nStreams;
    LONG nStepCount = nCount / nStreams;

    if (m_bloodType == PULSATING_ARTERY) {
        if (nStreams > 0) {
            LONG nPulseHeight = nHeightVelocity - 500;
            LONG nLoops = nStreams;
            do {
                particle.m_nRenderTime = particle.m_nTailLength * nTime;

                LONG nVelX = nXStart;
                LONG nVelY = nYStart;
                SHORT i = 0;
                if (nCount > 0) {
                    do {
                        particle.m_rgbColor = m_aColors[rand() % 2];
                        LONG nRandZ = rand() % 500;
                        LONG nRandY = rand() % 500;
                        LONG nRandX = rand() % 500;
                        particle.m_vel.x = nRandX + nVelX - 500;
                        particle.m_vel.y = 3 * (nRandY + nVelY - 500) >> 2;
                        particle.m_vel.z = nPulseHeight + nRandZ;
                        m_particleList.AddTail(new CParticle(particle));

                        nParticles++;
                        nVelX += nXInc;
                        nVelY += nYInc;
                        i++;
                    } while (i < nCount);
                }

                nPulseHeight -= nStepHeight;
                nXStart -= nStepXStart;
                nCount -= nStepCount;
                nXInc -= nStepXInc;
                nYInc -= nStepYInc;
                nTime += 10;
                nYStart -= nStepYStart;
            } while (--nLoops != 0);
        }
    } else {
        if (nStreams > 0) {
            LONG nLoops = nStreams;
            do {
                particle.m_nRenderTime = particle.m_nTailLength * nTime;
                particle.m_pos.y = nY;
                particle.m_pos.x = nX;
                particle.m_pos.z = nZ;

                if (bFirst) {
                    particle.m_rgbColor = m_aColors[rand() % 2];
                    particle.m_vel.z = nHeightVelocity != 0 ? rand() % nHeightVelocity : 0;
                    particle.m_vel.x = nFirstX;
                    particle.m_vel.y = 3 * nFirstY >> 2;
                    m_particleList.AddTail(new CParticle(particle));

                    nParticles++;
                }

                if (bBody && nCount > 0) {
                    LONG nVelX = nXStart;
                    LONG nVelY = nYStart;
                    SHORT i = 0;
                    do {
                        particle.m_rgbColor = m_aColors[rand() % 2];
                        particle.m_vel.z = nHeightVelocity != 0 ? rand() % nHeightVelocity : 0;
                        particle.m_vel.x = nVelX;
                        particle.m_vel.y = 3 * nVelY >> 2;
                        m_particleList.AddTail(new CParticle(particle));

                        nParticles++;
                        nVelX += nXInc;
                        nVelY += nYInc;
                        i++;
                    } while (i < nCount);
                }

                if (bLast) {
                    particle.m_rgbColor = m_aColors[rand() % 2];
                    particle.m_vel.z = nHeightVelocity != 0 ? rand() % nHeightVelocity : 0;
                    particle.m_vel.x = nLastX;
                    particle.m_vel.y = 3 * nLastY >> 2;
                    m_particleList.AddTail(new CParticle(particle));

                    nParticles++;
                }

                nXStart -= nStepXStart;
                nHeightVelocity -= nStepHeight;
                nYStart -= nStepYStart;
                nCount -= nStepCount;
                nXInc -= nStepXInc;
                nYInc -= nStepYInc;
                nTime++;
            } while (--nLoops != 0);
        }
    }

    return static_cast<SHORT>(nParticles);
}
