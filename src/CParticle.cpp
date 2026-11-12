#include "CParticle.h"

#include "CChitin.h"
#include "CUtil.h"
#include "CVidMode.h"
#include "CVideo.h"
#include "CVideo3d.h"

// 0x85EB60
const USHORT CParticle::CONNECT = 0;

// 0x85EB62
const USHORT CParticle::DOTS = 1;

// 0x85EB64
const USHORT CParticle::BLOB = 2;

// 0x85EB66
const USHORT CParticle::RESOLUTION_INC = 10;

// 0x85EB68
const USHORT CParticle::GRAVITY = 1024;

// 0x85EB6A
const WORD CParticle::CONSTANT_VELOCITY = 1;

// 0x85EB6C
const WORD CParticle::NORMAL_VELOCITY = 2;

// 0x85EB6E
const BYTE CParticle::AIRBORN = 0;

// 0x85EB6F
const BYTE CParticle::GROUNDED = 1;

// 0x85EB70
const BYTE CParticle::DEAD = 2;

// 0x85EB74
const LONG CParticle::LIFESPAN_INFINITE = LONG_MAX;

// 0x7C3500
CParticle::CParticle()
{
    m_pos.x = 0;
    m_pos.y = 0;
    m_pos.z = -1;
    m_vel.x = 0;
    m_vel.y = 0;
    m_vel.z = 0;
    m_nTimeStamp = 0;
    m_nTailLength = 0;
    m_nRenderTime = 0;
    m_bTag = FALSE;
    m_nGravity = GRAVITY;
    m_wType = BLOB;
    m_nLifeSpan = LIFESPAN_INFINITE;
    m_rgbColor = 0;
}

// 0x7C3540
CParticle::CParticle(LONG posX, LONG posY, LONG posZ, LONG velX, LONG velY, LONG velZ, COLORREF rgbInitColor, SHORT nTailLength, LONG nGravity, WORD wType)
{
    m_pos.x = posX;
    m_pos.y = posY;
    m_pos.z = posZ;
    m_vel.x = velX;
    m_vel.y = velY;
    m_vel.z = velZ;
    m_rgbColor = rgbInitColor;
    m_nTimeStamp = 0;
    m_nRenderTime = 0;
    m_nTailLength = nTailLength;
    m_bTag = FALSE;
    m_nGravity = nGravity;
    m_wType = wType;
    m_nLifeSpan = LIFESPAN_INFINITE;
}

// 0x7C35B0
CParticle::CParticle(const CParticle& particle)
{
    m_pos = particle.m_pos;
    m_vel = particle.m_vel;
    m_rgbColor = particle.m_rgbColor;
    m_nTimeStamp = particle.m_nTimeStamp;
    m_nTailLength = particle.m_nTailLength;
    m_nRenderTime = particle.m_nRenderTime;
    m_bTag = FALSE;
    m_nGravity = particle.m_nGravity;
    m_wType = particle.m_wType;
    m_nLifeSpan = particle.m_nLifeSpan;
}

// 0x7C3620
BYTE CParticle::AsynchronousUpdate()
{
    LONG nTmpZVel = 0;

    if (m_nLifeSpan--) {
        if ((m_wType & 0x1) != 0) {
            m_pos.z = max(m_pos.z - m_nGravity, 0);
        } else {
            nTmpZVel = m_vel.z - m_nGravity;
            m_pos.z = max(m_pos.z + nTmpZVel, 0);
        }

        if (m_pos.z > 0) {
            m_nTimeStamp++;
            m_pos.x += m_vel.x;
            m_pos.y += m_vel.y;
            m_vel.z = nTmpZVel;
            return AIRBORN;
        } else {
            if (m_nTailLength != 0) {
                m_nTailLength--;
                return GROUNDED;
            }
        }
    }

    return DEAD;
}

// 0x7C36A0
void CParticle::Render(LPVOID pSurface, LONG lPitch, const CRect& rClipRect, USHORT nFlag, USHORT nBlobSize)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();
    DWORD color = pVidMode->ReduceColor(m_rgbColor);
    LONG nXPrev = m_pos.x;
    LONG nYPrev = m_pos.y;
    LONG nZPrev = m_pos.z;
    LONG nZPrevVel = m_vel.z;
    LONG nXLast = nXPrev;
    LONG nYLast = nYPrev;
    LONG nZLast = nZPrev;
    SHORT nLoops = min(m_nTailLength, m_nTimeStamp) + 1;
    USHORT nBpp = g_pChitin->cVideo.Is3dAccelerated() ? CVideo::word_85DE2E : g_pChitin->cVideo.GetBitsPerPixels();

    switch (nFlag) {
    case CONNECT: {
        CRect rLine;
        rLine.left = rClipRect.left;
        rLine.top = rClipRect.top;
        rLine.right = rClipRect.right - 1;
        rLine.bottom = rClipRect.bottom - 1;

        switch (nBpp) {
        case 16:
            while (nLoops > 0) {
                nYPrev -= m_vel.y;
                nXPrev -= m_vel.x;
                nZPrev -= nZPrevVel;
                nZPrevVel += GRAVITY;
                pVidMode->DrawLine16(nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    nXLast >> RESOLUTION_INC,
                    ((3 * nYLast >> 2) - nZLast) >> RESOLUTION_INC,
                    reinterpret_cast<WORD*>(pSurface),
                    lPitch >> 1,
                    rLine,
                    static_cast<WORD>(color),
                    FALSE);
                nXLast = nXPrev;
                nZLast = nZPrev;
                nYLast = nYPrev;
                nLoops--;
            }
            break;
        case 24:
            while (nLoops > 0) {
                nYPrev -= m_vel.y;
                nXPrev -= m_vel.x;
                nZPrev -= nZPrevVel;
                nZPrevVel += GRAVITY;
                pVidMode->DrawLine24(nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    nXLast >> RESOLUTION_INC,
                    ((3 * nYLast >> 2) - nZLast) >> RESOLUTION_INC,
                    reinterpret_cast<BYTE*>(pSurface),
                    lPitch,
                    rLine,
                    color,
                    FALSE);
                nXLast = nXPrev;
                nZLast = nZPrev;
                nYLast = nYPrev;
                nLoops--;
            }
            break;
        case 32:
            if (nLoops > 0) {
                pVidMode->DrawLine32((nXPrev - m_vel.x) >> RESOLUTION_INC,
                    ((3 * (nYPrev - m_vel.y) >> 2) - nZPrev + nZPrevVel) >> RESOLUTION_INC,
                    nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    reinterpret_cast<DWORD*>(pSurface),
                    lPitch >> 2,
                    rLine,
                    color,
                    FALSE);
            }
            break;
        }
        break;
    }
    case DOTS:
        switch (nBpp) {
        case 16:
            while (nLoops > 0) {
                RenderDot16(reinterpret_cast<WORD*>(pSurface),
                    lPitch >> 1,
                    nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    rClipRect,
                    color);
                nXPrev -= m_vel.x;
                nYPrev -= m_vel.y;
                nZPrev -= nZPrevVel;
                nZPrevVel += GRAVITY;
                nLoops--;
            }
            break;
        case 24:
            while (nLoops > 0) {
                RenderDot24(reinterpret_cast<BYTE*>(pSurface),
                    lPitch,
                    nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    rClipRect,
                    color);
                nXPrev -= m_vel.x;
                nYPrev -= m_vel.y;
                nZPrev -= nZPrevVel;
                nZPrevVel += GRAVITY;
                nLoops--;
            }
            break;
        case 32:
            while (nLoops > 0) {
                RenderDot32(reinterpret_cast<DWORD*>(pSurface),
                    lPitch >> 2,
                    nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    rClipRect,
                    color);
                nXPrev -= m_vel.x;
                nYPrev -= m_vel.y;
                nZPrev -= nZPrevVel;
                nZPrevVel += GRAVITY;
                nLoops--;
            }
            break;
        }
        break;
    case BLOB:
        switch (nBpp) {
        case 16:
            while (nLoops > 0) {
                RenderBlob16(reinterpret_cast<WORD*>(pSurface),
                    lPitch >> 1,
                    nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    rClipRect,
                    nBlobSize,
                    static_cast<WORD>(color));
                nXPrev -= m_vel.x;
                nYPrev -= m_vel.y;
                nZPrev -= nZPrevVel;
                nZPrevVel += GRAVITY;
                nLoops--;
            }
            break;
        case 24:
            while (nLoops > 0) {
                RenderBlob24(reinterpret_cast<BYTE*>(pSurface),
                    lPitch,
                    nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    rClipRect,
                    nBlobSize,
                    color);
                nXPrev -= m_vel.x;
                nYPrev -= m_vel.y;
                nZPrev -= nZPrevVel;
                nZPrevVel += GRAVITY;
                nLoops--;
            }
            break;
        case 32:
            while (nLoops > 0) {
                RenderBlob32(reinterpret_cast<DWORD*>(pSurface),
                    lPitch >> 2,
                    nXPrev >> RESOLUTION_INC,
                    ((3 * nYPrev >> 2) - nZPrev) >> RESOLUTION_INC,
                    rClipRect,
                    nBlobSize,
                    color);
                nXPrev -= m_vel.x;
                nYPrev -= m_vel.y;
                nZPrev -= nZPrevVel;
                nZPrevVel += GRAVITY;
                nLoops--;
            }
            break;
        }
        break;
    }
}

// 0x7C3CC0
void CParticle::RenderBlob16(WORD* pSurface, LONG lPitch, LONG lX, LONG lY, const CRect& rClipRect, USHORT nFlag, WORD color)
{
    CRect rBlob;
    rBlob.left = 0;
    rBlob.top = 0;
    rBlob.right = rClipRect.right - rClipRect.left;
    rBlob.bottom = rClipRect.bottom - rClipRect.top;

    CPoint pt;
    lX -= rClipRect.left;
    lY -= rClipRect.top;

    switch (nFlag) {
    case 5:
        pt.x = lX;
        pt.y = lY + 1;
        if (rBlob.PtInRect(pt)) {
            pSurface[pt.x + lPitch * pt.y] = color;
        }
        // FALLTHROUGH
    case 4:
        pt.x = lX + 1;
        pt.y = lY;
        if (rBlob.PtInRect(pt)) {
            pSurface[pt.x + lPitch * pt.y] = color;
        }
        // FALLTHROUGH
    case 3:
        pt.x = lX - 1;
        pt.y = lY;
        if (rBlob.PtInRect(pt)) {
            pSurface[pt.x + lPitch * pt.y] = color;
        }
        // FALLTHROUGH
    case 2:
        pt.x = lX;
        pt.y = lY - 1;
        if (rBlob.PtInRect(pt)) {
            pSurface[pt.x + lPitch * pt.y] = color;
        }
        // FALLTHROUGH
    case 1:
        pt.x = lX;
        pt.y = lY;
        if (rBlob.PtInRect(pt)) {
            pSurface[pt.x + lPitch * pt.y] = color;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChParticle.cpp
        // __LINE__: 615
        UTIL_ASSERT(FALSE);
        break;
    }
}

// 0x7C3E20
void CParticle::RenderBlob24(BYTE* pSurface, LONG lPitch, LONG lX, LONG lY, const CRect& rClipRect, USHORT nFlag, DWORD color)
{
    CRect rBlob;
    rBlob.left = 0;
    rBlob.top = 0;
    rBlob.right = rClipRect.right - rClipRect.left;
    rBlob.bottom = rClipRect.bottom - rClipRect.top;

    CPoint pt;
    BYTE* p;
    lX -= rClipRect.left;
    lY -= rClipRect.top;

    switch (nFlag) {
    case 5:
        pt.x = lX;
        pt.y = lY + 1;
        if (rBlob.PtInRect(pt)) {
            p = pSurface + lPitch * pt.y + pt.x * 3;
            *reinterpret_cast<WORD*>(p) = static_cast<WORD>(color);
            p[2] = static_cast<BYTE>(color >> 16);
        }
        // FALLTHROUGH
    case 4:
        pt.x = lX + 1;
        pt.y = lY;
        if (rBlob.PtInRect(pt)) {
            p = pSurface + lPitch * pt.y + pt.x * 3;
            *reinterpret_cast<WORD*>(p) = static_cast<WORD>(color);
            p[2] = static_cast<BYTE>(color >> 16);
        }
        // FALLTHROUGH
    case 3:
        pt.x = lX - 1;
        pt.y = lY;
        if (rBlob.PtInRect(pt)) {
            p = pSurface + lPitch * pt.y + pt.x * 3;
            *reinterpret_cast<WORD*>(p) = static_cast<WORD>(color);
            p[2] = static_cast<BYTE>(color >> 16);
        }
        // FALLTHROUGH
    case 2:
        pt.x = lX;
        pt.y = lY - 1;
        if (rBlob.PtInRect(pt)) {
            p = pSurface + lPitch * pt.y + pt.x * 3;
            *reinterpret_cast<WORD*>(p) = static_cast<WORD>(color);
            p[2] = static_cast<BYTE>(color >> 16);
        }
        // FALLTHROUGH
    case 1:
        pt.x = lX;
        pt.y = lY;
        if (rBlob.PtInRect(pt)) {
            p = pSurface + lPitch * pt.y + pt.x * 3;
            *reinterpret_cast<WORD*>(p) = static_cast<WORD>(color);
            p[2] = static_cast<BYTE>(color >> 16);
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\chitin\ChParticle.cpp
        // __LINE__: 670
        UTIL_ASSERT(FALSE);
        break;
    }
}

// 0x7C3FD0
void CParticle::RenderBlob32(DWORD* pSurface, LONG lPitch, LONG lX, LONG lY, const CRect& rClipRect, USHORT nFlag, DWORD color)
{
    CRect rBlob;
    rBlob.left = 0;
    rBlob.top = 0;
    rBlob.right = rClipRect.right - rClipRect.left;
    rBlob.bottom = rClipRect.bottom - rClipRect.top;

    CPoint pt1;
    pt1.x = lX - rClipRect.left;
    pt1.y = lY - rClipRect.top + 1;
    if (rBlob.PtInRect(pt1)) {
        pSurface[pt1.x + lPitch * pt1.y] = color;
    }

    CPoint pt2;
    pt2.x = lX - rClipRect.left + 1;
    pt2.y = lY - rClipRect.top;
    if (rBlob.PtInRect(pt2)) {
        pSurface[pt2.x + lPitch * pt2.y] = color;
    }

    CPoint pt3;
    pt3.x = lX - rClipRect.left - 1;
    pt3.y = lY - rClipRect.top;
    if (rBlob.PtInRect(pt3)) {
        pSurface[pt3.x + lPitch * pt3.y] = color;
    }

    CPoint pt4;
    pt4.x = lX - rClipRect.left;
    pt4.y = lY - rClipRect.top - 1;
    if (rBlob.PtInRect(pt4)) {
        pSurface[pt4.x + lPitch * pt4.y] = color;
    }

    CPoint pt5;
    pt5.x = lX - rClipRect.left;
    pt5.y = lY - rClipRect.top;
    if (rBlob.PtInRect(pt5)) {
        pSurface[pt5.x + lPitch * pt5.y] = color;
    }
}

// 0x7CC340
void CParticle::Render3d(const CRect& rClipRect, const CRect& rLockedRect, USHORT nFlag, USHORT nBlobSize)
{
    LONG v1 = rClipRect.left - rLockedRect.left;
    LONG v2 = rClipRect.top - rLockedRect.top;
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();
    LONG nXPrev = m_pos.x;
    LONG nYPrev = m_pos.y;
    LONG nZPrev = m_pos.z;
    LONG nZPrevVel = m_vel.z;
    SHORT nLoops = min(m_nTailLength, m_nTimeStamp) + 1;
    CPoint pt;
    INT nPointSize;

    switch (nFlag) {
    case CONNECT:
        if (nLoops > 0) {
            pVidMode->DrawLine3d(((nXPrev - m_vel.x) >> RESOLUTION_INC) - v1,
                ((((3 * (nYPrev - m_vel.y)) >> 2) - nZPrev + nZPrevVel) >> RESOLUTION_INC) - v2,
                (nXPrev >> RESOLUTION_INC) - v1,
                ((((3 * nYPrev) >> 2) - nZPrev) >> RESOLUTION_INC) - v2,
                rLockedRect,
                m_rgbColor);
        }
        break;
    case DOTS:
        CVideo3d::glDisable(GL_BLEND);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glDisable(GL_TEXTURE_2D);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glColor4f(static_cast<float>(GetRValue(m_rgbColor)) / 255.0f,
            static_cast<float>(GetGValue(m_rgbColor)) / 255.0f,
            static_cast<float>(GetBValue(m_rgbColor)) / 255.0f,
            1.0f);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glBegin(GL_POINTS);
        while (nLoops > 0) {
            pt.x = nXPrev >> RESOLUTION_INC;
            pt.y = (((3 * nYPrev) >> 2) - nZPrev) >> RESOLUTION_INC;
            if (rClipRect.PtInRect(pt)) {
                // NOTE: Original code adds `CVideo3d::SUB_PIXEL_SHIFT`, however
                // it's value (0.2) is too small and due to rounding all points
                // (snowflakes) appear exactly between pixels (which results in
                // no snow at all).
                CVideo3d::glVertex3f(static_cast<float>(pt.x - v1) + 0.5f,
                    static_cast<float>(pt.y - v2) + 0.5f,
                    0.0f);
            }

            nXPrev -= m_vel.x;
            nYPrev -= m_vel.y;
            nZPrev -= nZPrevVel;
            nZPrevVel += GRAVITY;
            nLoops--;
        }
        CVideo3d::glEnd();
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
        break;
    case BLOB:
        CVideo3d::glDisable(GL_BLEND);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glDisable(GL_TEXTURE_2D);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glColor4f(static_cast<float>(GetRValue(m_rgbColor)) / 255.0f,
            static_cast<float>(GetGValue(m_rgbColor)) / 255.0f,
            static_cast<float>(GetBValue(m_rgbColor)) / 255.0f,
            1.0f);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        switch (nBlobSize) {
        case 3:
        case 4:
            nPointSize = 2;
            break;
        case 5:
            nPointSize = 3;
            break;
        default:
            nPointSize = 1;
            break;
        }

        CVideo3d::glPointSize(static_cast<float>(nPointSize));
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glBegin(GL_POINTS);
        while (nLoops > 0) {
            pt.x = nXPrev >> RESOLUTION_INC;
            pt.y = (((3 * nYPrev) >> 2) - nZPrev) >> RESOLUTION_INC;
            if (rClipRect.PtInRect(pt)) {
                // NOTE: See above, but in case of blobs its only meaningful
                // for point size 1. Point sizes 2 and 3 are always visible.
                CVideo3d::glVertex3f(static_cast<float>(pt.x - v1) + 0.5f,
                    static_cast<float>(pt.y - v2) + 0.5f,
                    0.0f);
            }

            nXPrev -= m_vel.x;
            nYPrev -= m_vel.y;
            nZPrev -= nZPrevVel;
            nZPrevVel += GRAVITY;
            nLoops--;
        }
        CVideo3d::glEnd();
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
        break;
    }
}

// NOTE: Convenience.
void CParticle::RenderDot32(DWORD* pSurface, LONG lPitch, LONG lX, LONG lY, const CRect& rClipRect, DWORD color)
{
    CPoint pt;
    pt.x = lX;
    pt.y = lY;
    if (rClipRect.PtInRect(pt)) {
        pSurface[pt.x + lPitch * (pt.y - rClipRect.top) - rClipRect.left] = color;
    }
}

// NOTE: Convenience.
void CParticle::RenderDot16(WORD* pSurface, LONG lPitch, LONG lX, LONG lY, const CRect& rClipRect, DWORD color)
{
    CPoint pt;
    pt.x = lX;
    pt.y = lY;
    if (rClipRect.PtInRect(pt)) {
        pSurface[pt.x + lPitch * (pt.y - rClipRect.top) - rClipRect.left] = static_cast<WORD>(color);
    }
}

// NOTE: Convenience.
void CParticle::RenderDot24(BYTE* pSurface, LONG lPitch, LONG lX, LONG lY, const CRect& rClipRect, DWORD color)
{
    CPoint pt;
    pt.x = lX;
    pt.y = lY;
    if (rClipRect.PtInRect(pt)) {
        BYTE* p = pSurface + (pt.y - rClipRect.top) * lPitch + (pt.x - rClipRect.left) * 3;
        *reinterpret_cast<WORD*>(p) = static_cast<WORD>(color);
        p[2] = static_cast<BYTE>(color >> 16);
    }
}
