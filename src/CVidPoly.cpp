#include "CVidPoly.h"

#include "CChitin.h"
#include "CUtil.h"
#include "CVidMode.h"
#include "CWarp.h"
#include "CVideo3d.h"

// 0x85EAA4
const BYTE CVidPoly::m_aDitherMask[] = {
    // clang-format off
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    // clang-format on
};

// 0xA07B20
CVidPolyEdge CVidPoly::m_aEdges[CVPOLY_MAX_VERTICIES];

// 0xA09F20
CVidPolyCache CVidPoly::m_cCache;

// 0x7C0DB0
CVidPoly::CVidPoly()
{
    field_0 = 0;
    m_pVertices = NULL;
    field_10 = 0;
    field_C = 0;
    m_nVertices = -1;
    m_pDrawHLineFunction = &CVidPoly::DrawHLine16;
}

// 0x7C0DD0
LONG CVidPoly::CalculateLineVIntersection(const CPoint& lineStart, const CPoint& lineEnd, LONG vertical, const CPoint& linePrev)
{
    if (lineStart.x == vertical) {
        if ((lineEnd.x <= vertical || linePrev.x <= vertical)
            && (lineEnd.x >= vertical || linePrev.x >= vertical)) {
            return lineStart.y;
        }
    } else if (lineEnd.x != vertical) {
        if ((lineStart.x >= vertical || lineEnd.x >= vertical)
            && (lineStart.x <= vertical || lineEnd.x <= vertical)) {
            return lineStart.y + (vertical - lineStart.x) * (lineEnd.y - lineStart.y) / (lineEnd.x - lineStart.x);
        }
    }

    return -1;
}

// 0x7C0E40
BOOLEAN CVidPoly::IsPtInPoly(const CPoint* pPoly, SHORT nPoly, const CPoint& pt)
{
    SHORT numIntersectingEdges = 0;
    SHORT cnt;
    LONG interSectionY;

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidPoly.cpp
    // __LINE__: 166
    UTIL_ASSERT(pPoly != NULL && nPoly > 0);

    if (nPoly == 1) {
        return pPoly[0] == pt;
    } else if (nPoly == 2) {
        interSectionY = CalculateLineVIntersection(pPoly[0], pPoly[2], pt.x, pPoly[1]);
        return interSectionY == pt.y;
    }

    for (cnt = 0; cnt < nPoly; cnt++) {
        if (cnt == nPoly - 1) {
            interSectionY = CalculateLineVIntersection(pPoly[cnt], pPoly[0], pt.x, pPoly[cnt - 1]);
        } else if (cnt != 0) {
            interSectionY = CalculateLineVIntersection(pPoly[cnt], pPoly[cnt + 1], pt.x, pPoly[cnt - 1]);
        } else {
            interSectionY = CalculateLineVIntersection(pPoly[0], pPoly[cnt + 1], pt.x, pPoly[nPoly - 1]);
        }

        if (interSectionY == pt.y) {
            return TRUE;
        }

        if (interSectionY != -1 && interSectionY < pt.y) {
            numIntersectingEdges++;
        }
    }

    return (numIntersectingEdges & 1);
}

// 0x7C0F40
BOOL CVidPoly::FillConvexPoly(WORD* pSurface, LONG lPitch, const CRect& rClipRect, DWORD dwColor, DWORD dwFlags, const CPoint& ptRef)
{
    SHORT nRightXDir;
    SHORT nLeftXDir;
    UINT nMinYIndex;
    UINT nMinYPt;
    UINT nMaxYPt;
    UINT nRightX;
    UINT nLeftX;
    UINT nRightIndex;
    UINT nLeftIndex;
    INT nRightDx;
    INT nRightDy;
    UINT nRightAdjUp;
    UINT nRightRun;
    INT nRightErrTerm;
    INT nLeftDx;
    INT nLeftDy;
    UINT nLeftAdjUp;
    UINT nLeftRun;
    INT nLeftErrTerm;
    SHORT nIndex;
    CRect rClip;
    CPoint ptRefAdjusted;

    ptRefAdjusted.x = ptRef.x;
    ptRefAdjusted.y = ptRef.y;

    rClip.left = rClipRect.left;
    rClip.top = rClipRect.top;
    rClip.right = rClipRect.right - 1;
    rClip.bottom = rClipRect.bottom - 1;

    nLeftXDir = 1;
    nRightXDir = 1;

    if (m_nVertices < 3) {
        return FALSE;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidPoly.cpp
    // __LINE__: 267
    UTIL_ASSERT_MSG(m_nVertices < CVPOLY_MAX_VERTICIES, "Excessive poly vertex count");

    SetHLineFunction(dwFlags);

    nMinYIndex = 0;
    nMinYPt = m_pVertices[nMinYIndex].y;
    nMaxYPt = m_pVertices[nMinYIndex].y;
    ;
    for (nIndex = 1; nIndex < m_nVertices; nIndex++) {
        if (m_pVertices[nIndex].y < nMinYPt) {
            nMinYPt = m_pVertices[nIndex].y;
            nMinYIndex = nIndex;
        } else if (m_pVertices[nIndex].y > nMaxYPt) {
            nMaxYPt = m_pVertices[nIndex].y;
        }
    }

    nRightX = m_pVertices[nMinYIndex].x;
    nLeftX = m_pVertices[nMinYIndex].x;
    nRightIndex = (nMinYIndex + 1) % m_nVertices;
    nLeftIndex = (nMinYIndex + m_nVertices - 1) % m_nVertices;

    nRightDx = m_pVertices[nRightIndex].x - nRightX;
    if (nRightDx < 0) {
        nRightXDir = -1;
        nRightDx = -nRightDx;
    }
    nRightDy = m_pVertices[nRightIndex].y - nMaxYPt;

    if (nRightDy > 0) {
        nRightRun = nRightDx / nRightDy;
        nRightAdjUp = nRightDx % nRightDy;
        nRightErrTerm = nRightAdjUp - nRightDy;
    } else {
        nRightRun = nRightDx;
        nRightAdjUp = nRightDx;
        nRightErrTerm = nRightDx;
    }

    nLeftDx = m_pVertices[nLeftIndex].x - nLeftX;
    if (nLeftDx < 0) {
        nLeftXDir = -1;
        nLeftDx = -nLeftDx;
    }
    nLeftDy = m_pVertices[nLeftIndex].y - nMaxYPt;

    if (nLeftDy > 0) {
        nLeftRun = nLeftDx / nLeftDy;
        nLeftAdjUp = nLeftDx % nLeftDy;
        nLeftErrTerm = nRightAdjUp - nLeftDy;
    } else {
        nLeftRun = nLeftDx;
        nLeftAdjUp = nLeftDx;
        nLeftErrTerm = nLeftDx;
    }

    if ((dwFlags & 0x8) != 0) {
        pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + lPitch * (rClip.bottom - nMinYPt));
        lPitch = -lPitch;
    } else {
        pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + lPitch * (nMinYPt - rClip.top));
    }

    ptRefAdjusted.y += nMinYPt - rClip.top;

    while (nMinYPt <= nMaxYPt) {
        if (nMinYPt == m_pVertices[nRightIndex].y) {
            do {
                nRightX = m_pVertices[nRightIndex].x;
                nRightIndex = (nRightIndex + 1) % m_nVertices;
                nRightDy = m_pVertices[nRightIndex].y - nMinYPt;
            } while (nRightDy == 0);

            nRightDx = m_pVertices[nRightIndex].x - nRightX;
            if (nRightDx < 0) {
                nRightXDir = -1;
                nRightDx = -nRightDx;
            } else {
                nRightXDir = 1;
            }

            nRightRun = nRightDx / nRightDy;
            nRightAdjUp = nRightDx % nRightDy;
            nRightErrTerm = nRightAdjUp - nRightDy;
        }

        if (nMinYPt >= m_pVertices[nLeftIndex].y) {
            do {
                nLeftX = m_pVertices[nLeftIndex].x;
                nLeftIndex = (nLeftIndex + m_nVertices - 1) % m_nVertices;
                nLeftDy = m_pVertices[nLeftIndex].y - nMinYPt;
            } while (nLeftDy == 0);

            nLeftDx = m_pVertices[nLeftIndex].x - nLeftX;
            if (nLeftDx < 0) {
                nLeftXDir = -1;
                nLeftDx = -nLeftDx;
            } else {
                nLeftXDir = 1;
            }

            nLeftRun = nLeftDx / nLeftDy;
            nLeftAdjUp = nLeftDx % nLeftDy;
            nLeftErrTerm = nLeftAdjUp - nLeftDy;
        }

        // NOTE: Signed compare.
        if (static_cast<LONG>(nMinYPt) >= rClip.top && static_cast<LONG>(nMinYPt) < rClip.bottom) {
            INT xMin;
            INT xMax;
            if (nLeftX > nRightX) {
                xMax = min(static_cast<LONG>(nLeftX), rClip.right) - rClip.left;
                xMin = max(static_cast<LONG>(nRightX) - rClip.left, 0);
            } else {
                xMax = min(static_cast<LONG>(nRightX), rClip.right) - rClip.left;
                xMin = max(static_cast<LONG>(nLeftX) - rClip.left, 0);
            }

            (this->*m_pDrawHLineFunction)(pSurface,
                xMin,
                xMax,
                dwColor,
                rClip,
                ptRefAdjusted);
        }

        nLeftX += nLeftRun * nLeftXDir;
        nLeftErrTerm += nLeftAdjUp;
        if (nLeftErrTerm > 0) {
            nLeftX += nLeftXDir;
            nLeftErrTerm -= nLeftAdjUp + nLeftDx;
        }

        nRightX += nRightRun * nRightXDir;
        nRightErrTerm += nRightAdjUp;
        if (nRightErrTerm > 0) {
            nRightX += nRightXDir;
            nRightErrTerm -= nRightAdjUp + nRightDx;
        }

        nMinYPt++;
        ptRefAdjusted.y++;
        pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + lPitch);
    }

    return TRUE;
}

// 0x7C13A0
BOOL CVidPoly::FillPoly(WORD* pSurface, LONG lPitch, const CRect& rClipRect, DWORD dwColor, DWORD dwFlags, const CPoint& ptRef)
{
    CRect rClip;
    CPoint ptRefAdjusted;
    SHORT nY;

    ptRefAdjusted.x = ptRef.x;
    ptRefAdjusted.y = ptRef.y;

    rClip.left = rClipRect.left;
    rClip.top = rClipRect.top;
    rClip.right = rClipRect.right - 1;
    rClip.bottom = rClipRect.bottom - 1;

    LONG lStride = lPitch;

    if (m_nVertices < 3) {
        return FALSE;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidPoly.cpp
    // __LINE__: 546
    UTIL_ASSERT_MSG(m_nVertices < CVPOLY_MAX_VERTICIES, "Excessive poly vertex count");

    if (m_nVertices >= CVPOLY_MAX_VERTICIES) {
        return FALSE;
    }

    SetHLineFunction(dwFlags);

    if ((dwFlags & 0x2) == 0) {
        if (!m_cCache.sub_7C1DE0(this)) {
            sub_7C1970(m_aEdges);
            m_cCache.sub_7C1EC0(this);
        }
    } else {
        sub_7C1970(m_aEdges);
    }

    field_10 = NULL;

    nY = static_cast<SHORT>(field_C->y);

    if ((dwFlags & 0x8) != 0) {
        pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + (rClip.bottom - nY) * lPitch);
        lStride = -lPitch;
    } else {
        pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + (nY - rClip.top) * lPitch);
    }

    ptRefAdjusted.y += nY - rClip.top;

    while (field_C != NULL || field_10 != NULL) {
        CVidPolyEdge** ppLink = &field_10;
        CVidPolyEdge* pActive = field_10;

        while (field_C != NULL && field_C->y == nY) {
            CVidPolyEdge* pPending = field_C;
            SHORT nPendingX = static_cast<SHORT>(pPending->x);

            while (pActive != NULL && pActive->x < nPendingX) {
                ppLink = &pActive->pNext;
                pActive = pActive->pNext;
            }

            *ppLink = pPending;
            field_C = field_C->pNext;
            (*ppLink)->pNext = pActive;

            ppLink = &(*ppLink)->pNext;
        }

        if (nY >= rClip.top && nY <= rClip.bottom && field_10 != NULL) {
            CVidPolyEdge* pEdge = field_10;
            do {
                INT xMin;
                if (pEdge->x > rClip.left) {
                    xMin = pEdge->x - rClip.left;
                } else {
                    xMin = 0;
                }

                CVidPolyEdge* pEdgeEnd = pEdge->pNext;

                INT xMax;
                if (pEdgeEnd->x < rClip.right) {
                    xMax = pEdgeEnd->x - rClip.left;
                } else {
                    xMax = rClip.right - rClip.left;
                }

                (this->*m_pDrawHLineFunction)(pSurface,
                    xMin,
                    xMax,
                    dwColor,
                    rClip,
                    ptRefAdjusted);

                pEdge = pEdgeEnd->pNext;
            } while (pEdge != NULL);
        }

        sub_7C18F0();

        if (field_10 != NULL) {
            BOOL bSwapped;
            do {
                CVidPolyEdge* pEdge = field_10;
                CVidPolyEdge** ppEdgeLink = &field_10;
                bSwapped = FALSE;

                while (pEdge->pNext != NULL) {
                    CVidPolyEdge* pEdgeNext = pEdge->pNext;
                    if (pEdge->x > pEdgeNext->x) {
                        *ppEdgeLink = pEdgeNext;
                        bSwapped = TRUE;
                        pEdge->pNext = pEdgeNext->pNext;
                        (*ppEdgeLink)->pNext = pEdge;
                    }

                    ppEdgeLink = &(*ppEdgeLink)->pNext;
                    pEdge = *ppEdgeLink;
                }
            } while (bSwapped);
        }

        nY++;
        ptRefAdjusted.y++;
        pSurface = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(pSurface) + lStride);
    }

    return TRUE;
}

// 0x7C18F0
void CVidPoly::sub_7C18F0()
{
    CVidPolyEdge* pPrev = NULL;
    CVidPolyEdge* pEdge = field_10;

    while (pEdge != NULL) {
        if ((field_C != NULL && --pEdge->m_nRowsLeft == 0)
            || (field_C == NULL && --pEdge->m_nRowsLeft == -1)) {
            if (pPrev == NULL) {
                field_10 = pEdge->pNext;
            } else {
                pPrev->pNext = pEdge->pNext;
            }
        } else {
            pEdge->x += pEdge->m_nXRun;
            pEdge->m_nErrTerm += pEdge->m_nAdjUp;
            if (pEdge->m_nErrTerm >= 0) {
                pEdge->x += pEdge->m_nXDir;
                pEdge->m_nErrTerm -= pEdge->m_nDy;
            }
            pPrev = pEdge;
        }

        pEdge = pEdge->pNext;
    }
}

// 0x7C1970
void CVidPoly::sub_7C1970(CVidPolyEdge* pEdges)
{
    field_C = NULL;

    for (INT nIndex = 0; nIndex < m_nVertices; nIndex++) {
        SHORT nNext = static_cast<SHORT>((nIndex + 1) % m_nVertices);

        UINT xTop;
        UINT yTop;
        UINT xBottom;
        UINT yBottom;
        if (m_pVertices[nIndex].y < m_pVertices[nNext].y) {
            xTop = m_pVertices[nIndex].x;
            yTop = m_pVertices[nIndex].y;
            xBottom = m_pVertices[nNext].x;
            yBottom = m_pVertices[nNext].y;
        } else {
            xTop = m_pVertices[nNext].x;
            yTop = m_pVertices[nNext].y;
            xBottom = m_pVertices[nIndex].x;
            yBottom = m_pVertices[nIndex].y;
        }

        INT dy = yBottom - yTop;
        if (dy != 0) {
            CVidPolyEdge* pEdge = pEdges++;
            INT dx = xBottom - xTop;

            pEdge->m_nErrTerm = 1 - dy;
            pEdge->m_nXDir = dx >= 0 ? 1 : -1;
            pEdge->pNext = NULL;
            pEdge->x = xTop;
            pEdge->y = yTop;
            pEdge->m_nRowsLeft = dy;

            INT nAbsDx = abs(dx);
            pEdge->m_nDy = dy;
            pEdge->m_nXRun = (nAbsDx / dy) * pEdge->m_nXDir;
            pEdge->m_nAdjUp = nAbsDx % dy;

            CVidPolyEdge** ppLink = &field_C;
            CVidPolyEdge* pCur = field_C;
            while (pCur != NULL) {
                if (pCur->y > static_cast<INT>(yTop)) {
                    break;
                }
                if (pCur->y == static_cast<INT>(yTop) && pCur->x > static_cast<INT>(xTop)) {
                    break;
                }
                ppLink = &pCur->pNext;
                pCur = pCur->pNext;
            }

            *ppLink = pEdge;
            pEdge->pNext = pCur;
        }
    }
}

// 0x7C1D40
CVidPolyCacheEntry::~CVidPolyCacheEntry()
{
    if (m_pEdges != NULL) {
        operator delete(m_pEdges);
    }
}

CVidPolyCacheEntry::CVidPolyCacheEntry()
{
    m_nVertices = 0;
    m_pFirstEdge = NULL;
    m_pEdges = NULL;
    m_pVertices = NULL;
}

// 0x7C1DE0
BOOL CVidPolyCache::sub_7C1DE0(CVidPoly* pPoly)
{
    m_cs.Lock();

    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        POSITION posCur = pos;
        CVidPolyCacheEntry* pEntry = static_cast<CVidPolyCacheEntry*>(GetNext(pos));

        if (pEntry != NULL
            && pPoly->m_pVertices == pEntry->m_pVertices
            && pPoly->m_nVertices == pEntry->m_nVertices) {
            INT nVertices = pEntry->m_nVertices;
            CVidPolyEdge* pEdges = pEntry->m_pEdges;
            CVidPolyEdge* pFirstEdge = pEntry->m_pFirstEdge;
            CVIDPOLY_VERTEX* pVertices = pEntry->m_pVertices;

            if (pEdges != NULL && pFirstEdge != NULL && pVertices != NULL) {
                pPoly->field_C = pFirstEdge;
                memcpy(CVidPoly::m_aEdges, pEdges, nVertices * sizeof(CVidPolyEdge));
                pPoly->m_nVertices = nVertices;
                pPoly->m_pVertices = pVertices;
            }

            RemoveAt(posCur);
            AddHead(pEntry);

            m_cs.Unlock();
            return TRUE;
        }
    }

    m_cs.Unlock();
    return FALSE;
}

// 0x7C1EC0
void CVidPolyCache::sub_7C1EC0(CVidPoly* pPoly)
{
    m_cs.Lock();

    if (GetCount() >= 16) {
        do {
            CVidPolyCacheEntry* pOld = static_cast<CVidPolyCacheEntry*>(RemoveTail());
            if (pOld != NULL) {
                delete pOld;
            }
        } while (GetCount() >= 16);
    }

    if (pPoly != NULL) {
        CVidPolyCacheEntry* pEntry = new CVidPolyCacheEntry();

        if (pPoly->m_nVertices != 0) {
            pEntry->m_pEdges = static_cast<CVidPolyEdge*>(operator new(pPoly->m_nVertices * sizeof(CVidPolyEdge)));
            if (pEntry->m_pEdges != NULL) {
                memcpy(pEntry->m_pEdges, CVidPoly::m_aEdges, pPoly->m_nVertices * sizeof(CVidPolyEdge));
                pEntry->m_pFirstEdge = pPoly->field_C;
                pEntry->m_nVertices = pPoly->m_nVertices;
                pEntry->m_pVertices = pPoly->m_pVertices;
            }
        }

        AddTail(pEntry);
    }

    m_cs.Unlock();
}

// 0x7C15F0
BOOL CVidPoly::FillConvexPoly3d(const CRect& rClip, DWORD dwColor, DWORD dwFlags, const CPoint& ptRef)
{
    if (m_nVertices < 3) {
        return FALSE;
    }

    CVideo3d::glColor4f(static_cast<float>(GetRValue(dwColor)) / 255.0f,
        static_cast<float>(GetRValue(dwColor)) / 255.0f,
        static_cast<float>(GetRValue(dwColor)) / 255.0f,
        static_cast<float>(dwColor >> 24) / 255.0f);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x2) != 0) {
        CVideo3d::glPolygonStipple(m_aDitherMask);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glEnable(GL_POLYGON_STIPPLE);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }

    CVideo3d::glDisable(GL_BLEND);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glDisable(GL_TEXTURE_2D);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glBegin(GL_POLYGON);
    for (INT nIndex = 0; nIndex < m_nVertices - 1; nIndex++) {
        INT nXFrom = m_pVertices[nIndex].x;
        INT nYFrom = m_pVertices[nIndex].y;
        INT nXTo = m_pVertices[nIndex + 1].x;
        INT nYTo = m_pVertices[nIndex + 1].y;
        if (CVidMode::ClipLine(nXFrom, nYFrom, nXTo, nYTo, rClip)) {
            CVideo3d::glVertex3f(static_cast<float>(nXFrom) + CVideo3d::SUB_PIXEL_SHIFT,
                static_cast<float>(nYFrom) + CVideo3d::SUB_PIXEL_SHIFT,
                0.0f);
            CVideo3d::glVertex3f(static_cast<float>(nXTo) + CVideo3d::SUB_PIXEL_SHIFT,
                static_cast<float>(nYTo) + CVideo3d::SUB_PIXEL_SHIFT,
                0.0f);
        }
    }

    CVideo3d::glEnd();
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x2) != 0) {
        CVideo3d::glDisable(GL_POLYGON_STIPPLE);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }

    return TRUE;
}

// 0x7C18B0
void CVidPoly::SetPoly(WORD* pVertices, WORD nVertices)
{
    m_pVertices = reinterpret_cast<CVIDPOLY_VERTEX*>(pVertices);
    m_nVertices = nVertices;
}

// 0x7C18D0
void CVidPoly::SetPoly(CVIDPOLY_VERTEX* pVertices, WORD nVertices)
{
    m_pVertices = pVertices;
    m_nVertices = nVertices;
}

// 0x7C1AA0
void CVidPoly::DrawHLine16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned short* pSurface16 = reinterpret_cast<unsigned short*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface16 += xMin;

        for (int x = 0; x < width; x++) {
            *pSurface16++ = static_cast<unsigned short>(dwColor);
        }
    }
}

// 0x7C1AF0
void CVidPoly::DrawHLineMirrored16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned short* pSurface16 = reinterpret_cast<unsigned short*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface16 += rSurface.Width() - xMin;

        for (int x = 0; x < width; x++) {
            *pSurface16-- = static_cast<unsigned short>(dwColor);
        }
    }
}

// 0x7C1B50
void CVidPoly::DrawHLineDithered16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned short* pSurface16 = reinterpret_cast<unsigned short*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface16 += xMin;

        if ((ptRef.y & 1) != 0) {
            if (((xMin + ptRef.x) & 1) != 0) {
                pSurface16++;
                width--;
            }
        } else {
            if (((xMin + ptRef.x) & 1) == 0) {
                pSurface16++;
                width--;
            }
        }

        width /= 2;
        for (int x = 0; x < width; x++) {
            *pSurface16 = static_cast<unsigned short>(dwColor);
            pSurface16 += 2;
        }
    }
}

// 0x7C1BB0
void CVidPoly::DrawHLineDitheredMirrored16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned short* pSurface16 = reinterpret_cast<unsigned short*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface16 += rSurface.Width() - xMin;

        if ((ptRef.y & 1) != 0) {
            if (((rSurface.Width() + xMin + ptRef.x - 1) & 1) != 0) {
                pSurface16--;
                width--;
            }
        } else {
            if (((rSurface.Width() + xMin + ptRef.x - 1) & 1) == 0) {
                pSurface16--;
                width--;
            }
        }

        width /= 2;
        for (int x = 0; x < width; x++) {
            *pSurface16 = static_cast<unsigned short>(dwColor);
            pSurface16 -= 2;
        }
    }
}

// 0x7C1C20
void CVidPoly::SetHLineFunction(DWORD dwFlags)
{
    switch (g_pChitin->cVideo.m_nBpp) {
    case 32:
        if ((dwFlags & 0x1) != 0) {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineMirrored32;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLine32;
            }
        } else if ((dwFlags & 0x10) != 0) {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineShadedMirrored32;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineShaded32;
            }
        } else {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineDitheredMirrored32;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineDithered32;
            }
        }
        break;
    case 24:
        if ((dwFlags & 0x1) != 0) {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineMirrored24;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLine24;
            }
        } else if ((dwFlags & 0x10) != 0) {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineShadedMirrored24;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineShaded24;
            }
        } else {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineDitheredMirrored24;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineDithered24;
            }
        }
        break;
    default:
        if ((dwFlags & 0x1) != 0) {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineMirrored16;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLine16;
            }
        } else if ((dwFlags & 0x10) != 0) {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineShadedMirrored16;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineShaded16;
            }
        } else {
            if ((dwFlags & 0x4) != 0) {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineDitheredMirrored16;
            } else {
                m_pDrawHLineFunction = &CVidPoly::DrawHLineDithered16;
            }
        }
        break;
    }
}

// 0x7C1FD0
void CVidPoly::DrawHLineShaded16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    CWarp* pEngine = g_pChitin->pActiveEngine;
    CVidMode* pVidMode;
    if (pEngine != NULL) {
        pVidMode = pEngine->pVidMode;
    } else {
        pVidMode = NULL;
    }

    unsigned short* pSurface16 = reinterpret_cast<unsigned short*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface16 += xMin;

        for (int x = 0; x < width; x++) {
            *pSurface16 = static_cast<unsigned short>(pVidMode->FUN_0079ad20(pVidMode->FUN_0079ad90(*pSurface16) >> 2, dwColor & 0xFFFF));
            pSurface16++;
        }
    }
}

// 0x7C2040
void CVidPoly::DrawHLineShadedMirrored16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    CWarp* pEngine = g_pChitin->pActiveEngine;
    CVidMode* pVidMode;
    if (pEngine != NULL) {
        pVidMode = pEngine->pVidMode;
    } else {
        pVidMode = NULL;
    }

    unsigned short* pSurface16 = reinterpret_cast<unsigned short*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface16 += rSurface.Width() - xMin;

        for (int x = 0; x < width; x++) {
            *pSurface16 = static_cast<unsigned short>(pVidMode->FUN_0079ad20(pVidMode->FUN_0079ad90(*pSurface16) >> 2, dwColor & 0xFFFF));
            pSurface16--;
        }
    }
}

// 0x7D6970
void CVidPoly::DrawHLine24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned char* pSurface8 = reinterpret_cast<unsigned char*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface8 += xMin * 3;

        for (int x = 0; x < width; x++) {
            *pSurface8++ = GetRValue(dwColor);
            *pSurface8++ = GetGValue(dwColor);
            *pSurface8++ = GetBValue(dwColor);
        }
    }
}

// 0x7D69B0
void CVidPoly::DrawHLineMirrored24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned char* pSurface8 = reinterpret_cast<unsigned char*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface8 += (rSurface.Width() - xMin) * 3;

        for (int x = 0; x < width; x++) {
            pSurface8[0] = GetRValue(dwColor);
            pSurface8[1] = GetGValue(dwColor);
            pSurface8[2] = GetBValue(dwColor);
            pSurface8 -= 3;
        }
    }
}

// 0x7D6A00
void CVidPoly::DrawHLineDithered24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned char* pSurface8 = reinterpret_cast<unsigned char*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface8 += xMin;

        if ((ptRef.y & 1) != 0) {
            if (((xMin + ptRef.x) & 1) != 0) {
                pSurface8 += 3;
                width--;
            }
        } else {
            if (((xMin + ptRef.x) & 1) == 0) {
                pSurface8 += 3;
                width--;
            }
        }

        width /= 2;
        for (int x = 0; x < width; x++) {
            pSurface8[0] = GetRValue(dwColor);
            pSurface8[1] = GetGValue(dwColor);
            pSurface8[2] = GetBValue(dwColor);
            pSurface8 += 6;
        }
    }
}

// 0x7D6A70
void CVidPoly::DrawHLineDitheredMirrored24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned char* pSurface8 = reinterpret_cast<unsigned char*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface8 += (rSurface.Width() - xMin) * 3;

        if ((ptRef.y & 1) != 0) {
            if (((rSurface.Width() + xMin + ptRef.x - 1) & 1) != 0) {
                pSurface8 -= 3;
                width--;
            }
        } else {
            if (((rSurface.Width() + xMin + ptRef.x - 1) & 1) == 0) {
                pSurface8 -= 3;
                width--;
            }
        }

        width /= 2;
        for (int x = 0; x < width; x++) {
            pSurface8[0] = GetRValue(dwColor);
            pSurface8[1] = GetGValue(dwColor);
            pSurface8[2] = GetBValue(dwColor);
            pSurface8 -= 6;
        }
    }
}

// 0x7D6AF0
void CVidPoly::DrawHLine32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned int* pSurface32 = reinterpret_cast<unsigned int*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface32 += xMin;

        for (int x = 0; x < width; x++) {
            *pSurface32++ = dwColor;
        }
    }
}

// 0x7D6B20
void CVidPoly::DrawHLineMirrored32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned int* pSurface32 = reinterpret_cast<unsigned int*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface32 += rSurface.Width() - xMin;

        for (int x = 0; x < width; x++) {
            *pSurface32-- = dwColor;
        }
    }
}

// 0x7D6B60
void CVidPoly::DrawHLineDithered32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned int* pSurface32 = reinterpret_cast<unsigned int*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface32 += xMin;

        if (g_pChitin->cVideo.Is3dAccelerated()) {
            for (int x = 0; x < width; x++) {
                unsigned int rgb = *pSurface32;
                *pSurface32++ = (rgb & 0xFFFFFF) | ((rgb >> 1) & 0x7F000000);
            }
        } else {
            if ((ptRef.y & 1) != 0) {
                if (((xMin + ptRef.x) & 1) != 0) {
                    pSurface32++;
                    width--;
                }
            } else {
                if (((xMin + ptRef.x) & 1) == 0) {
                    pSurface32++;
                    width--;
                }
            }

            width /= 2;
            for (int x = 0; x < width; x++) {
                *pSurface32 = dwColor;
                pSurface32 += 2;
            }
        }
    }
}

// 0x7D6BF0
void CVidPoly::DrawHLineDitheredMirrored32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    unsigned int* pSurface32 = reinterpret_cast<unsigned int*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface32 += rSurface.Width() - xMin;

        if (g_pChitin->cVideo.Is3dAccelerated()) {
            for (int x = 0; x < width; x++) {
                unsigned int rgb = *pSurface32;
                *pSurface32-- = (rgb & 0xFFFFFF) | ((rgb >> 1) & 0x7F000000);
            }
        } else {
            if ((ptRef.y & 1) != 0) {
                if (((rSurface.Width() + xMin + ptRef.x - 1) & 1) != 0) {
                    pSurface32--;
                    width--;
                }
            } else {
                if (((rSurface.Width() + xMin + ptRef.x - 1) & 1) == 0) {
                    pSurface32--;
                    width--;
                }
            }

            width /= 2;
            for (int x = 0; x < width; x++) {
                *pSurface32 = dwColor;
                pSurface32 -= 2;
            }
        }
    }
}

// 0x7D6CA0
void CVidPoly::DrawHLineShaded24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    int c0 = dwColor & 0xFF;
    int c1 = (dwColor >> 8) & 0xFF;
    int c2 = (dwColor >> 16) & 0xFF;
    int gray = (c0 + c2 + c1) / 3;

    unsigned char* pSurface8 = reinterpret_cast<unsigned char*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface8 += xMin * 3;

        for (int x = 0; x < width; x++) {
            int b0 = *pSurface8;
            int b1 = b0 + 1;
            int b2 = b0 + 2;
            if (b0 + b1 + b2 >= 0x20 && gray >= ((b0 + b1 + b2) >> 3)) {
                int inv = 0xFF - gray;
                pSurface8[1] = static_cast<unsigned char>((c1 * gray + inv * b1) >> 8);
                pSurface8[0] = static_cast<unsigned char>((c2 * gray + inv * b0) >> 8);
                pSurface8[2] = static_cast<unsigned char>((c0 * gray + inv * b2) >> 8);
            }
            pSurface8 += 3;
        }
    }
}

// 0x7D6DA0
void CVidPoly::DrawHLineShadedMirrored24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    int c0 = dwColor & 0xFF;
    int c1 = (dwColor >> 8) & 0xFF;
    int c2 = (dwColor >> 16) & 0xFF;
    int gray = (c0 + c2 + c1) / 3;

    unsigned char* pSurface8 = reinterpret_cast<unsigned char*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface8 += (rSurface.Width() - xMin + 1) * 3;

        for (int x = 0; x < width; x++) {
            pSurface8 -= 3;
            int b0 = *pSurface8;
            int b1 = b0 + 1;
            int b2 = b0 + 2;
            if (b0 + b1 + b2 >= 0x20 && gray >= ((b0 + b1 + b2) >> 3)) {
                int inv = 0xFF - gray;
                pSurface8[1] = static_cast<unsigned char>((c1 * gray + inv * b1) >> 8);
                pSurface8[0] = static_cast<unsigned char>((c2 * gray + inv * b0) >> 8);
                pSurface8[2] = static_cast<unsigned char>((c0 * gray + inv * b2) >> 8);
            }

        }
    }
}

// 0x7D6EB0
void CVidPoly::DrawHLineShaded32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    int c0 = dwColor & 0xFF;
    int c1 = (dwColor >> 8) & 0xFF;
    int c2 = (dwColor >> 16) & 0xFF;
    int gray = (c0 + c2 + c1) / 3;

    unsigned int* pSurface32 = reinterpret_cast<unsigned int*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface32 += xMin;

        for (int x = 0; x < width; x++) {
            int b0 = *pSurface32 & 0xFF;
            int b1 = (*pSurface32 >> 8) & 0xFF;
            if (b0 + b1 + b0 >= 0x20 && gray >= ((b0 + b1 + b0) >> 3)) {
                int inv = 0xFF - gray;
                int r = (c2 * gray + inv * b0) >> 8;
                int g = (c1 * gray + inv * b1) >> 8;
                int b = (c0 * gray + inv * b0) >> 8;
                *pSurface32 = (((r << 8) | g) << 8) | b;
            }
            pSurface32++;
        }
    }
}

// 0x7D6FD0
void CVidPoly::DrawHLineShadedMirrored32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef)
{
    int c0 = dwColor & 0xFF;
    int c1 = (dwColor >> 8) & 0xFF;
    int c2 = (dwColor >> 16) & 0xFF;
    int gray = (c0 + c2 + c1) / 3;

    unsigned int* pSurface32 = reinterpret_cast<unsigned int*>(pSurface);

    int width = xMax - xMin + 1;
    if (width > 0) {
        pSurface32 += rSurface.Width() - xMin;

        for (int x = 0; x < width; x++) {
            int b0 = *pSurface32 & 0xFF;
            int b1 = (*pSurface32 >> 8) & 0xFF;
            if (b0 + b1 + b0 >= 0x20 && gray >= ((b0 + b1 + b0) >> 3)) {
                int inv = 0xFF - gray;
                int r = (c2 * gray + inv * b0) >> 8;
                int g = (c1 * gray + inv * b1) >> 8;
                int b = (c0 * gray + inv * b0) >> 8;
                *pSurface32 = (((r << 8) | g) << 8) | b;
            }
            pSurface32--;
        }
    }
}
