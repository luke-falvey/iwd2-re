#ifndef CVIDPOLY_H_
#define CVIDPOLY_H_

#include "mfc.h"

#define CVPOLY_MAX_VERTICIES 256

typedef struct {
    WORD x;
    WORD y;
} CVIDPOLY_VERTEX;

class CVidPoly;

// Edge of the polygon's edge list (36 bytes).
class CVidPolyEdge {
public:
    /* 0000 */ CVidPolyEdge* pNext;
    /* 0004 */ INT x;
    /* 0008 */ INT y;
    /* 000C */ int m_nXDir;
    /* 0010 */ int m_nXRun;
    /* 0014 */ int m_nErrTerm;
    /* 0018 */ int m_nAdjUp;
    /* 001C */ int m_nDy;
    /* 0020 */ int m_nRowsLeft;
};

// Cached edge list of a polygon (20 bytes).
class CVidPolyCacheEntry {
public:
    CVidPolyCacheEntry();
    virtual ~CVidPolyCacheEntry();

    /* 0004 */ INT m_nVertices;
    /* 0008 */ CVidPolyEdge* m_pEdges;
    /* 000C */ CVidPolyEdge* m_pFirstEdge;
    /* 0010 */ CVIDPOLY_VERTEX* m_pVertices;
};

class CVidPolyCache : public CPtrList {
public:
    CVidPolyCache();
    ~CVidPolyCache() override;
    BOOL sub_7C1DE0(CVidPoly* pPoly);
    void sub_7C1EC0(CVidPoly* pPoly);

    /* 001C */ CCriticalSection m_cs;
};

class CVidPoly {
public:
    typedef void (CVidPoly::*DrawHLineFunc)(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);

    static const BYTE m_aDitherMask[];
    static CVidPolyEdge m_aEdges[CVPOLY_MAX_VERTICIES];
    static CVidPolyCache m_cCache;

    CVidPoly();

    static LONG CalculateLineVIntersection(const CPoint& lineStart, const CPoint& lineEnd, LONG vertical, const CPoint& linePrev);
    static BOOLEAN IsPtInPoly(const CPoint* pPoly, SHORT nPoly, const CPoint& pt);
    BOOL FillConvexPoly(WORD* pSurface, LONG lPitch, const CRect& rClip, DWORD dwColor, DWORD dwFlags, const CPoint& ptRef);
    BOOL FillPoly(WORD* pSurface, LONG lPitch, const CRect& rClip, DWORD dwColor, DWORD dwFlags, const CPoint& ptRef);
    BOOL FillConvexPoly3d(const CRect& rClip, DWORD dwColor, DWORD dwFlags, const CPoint& ptRef);
    void SetPoly(WORD* pVertices, WORD nVertices);
    void SetPoly(CVIDPOLY_VERTEX* pVertices, WORD nVertices);
    void DrawHLine16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineMirrored16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineDithered16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineDitheredMirrored16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void SetHLineFunction(DWORD dwFlags);
    void sub_7C18F0();
    void sub_7C1970(CVidPolyEdge* pEdges);

    void DrawHLineShaded16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineShadedMirrored16(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);

    void DrawHLine24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineMirrored24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineDithered24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineDitheredMirrored24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLine32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineMirrored32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineDithered32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineDitheredMirrored32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineShaded24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineShadedMirrored24(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineShaded32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);
    void DrawHLineShadedMirrored32(void* pSurface, int xMin, int xMax, DWORD dwColor, const CRect& rSurface, const CPoint& ptRef);

    int field_0;
    CVIDPOLY_VERTEX* m_pVertices;
    INT m_nVertices;
    CVidPolyEdge* field_C;
    CVidPolyEdge* field_10;
    DrawHLineFunc m_pDrawHLineFunction;
};

#endif /* CVIDPOLY_H_ */
