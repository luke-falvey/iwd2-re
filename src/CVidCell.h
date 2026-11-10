#ifndef CVIDCELL_H_
#define CVIDCELL_H_

#include "CResCell.h"
#include "CResCellHeader.h"
#include "CVidImage.h"

#define CVIDCELL_BRIGHTEST 0x8

class CVidPoly;

class CVidCell : public CVidImage, public CResHelper<CResCell, 1000> {
public:
    static DWORD dword_8BAC88;
    static BOOL TRANSLUCENT_BLTS_ON;
    static BOOL TRANSLUCENT_SHADOWS_ON;

    CVidCell();
    CVidCell(CResRef cNewResRef, BOOL bDoubleSize);
    ~CVidCell();
    void AddRangeAffect(USHORT nAffectType, USHORT nRangeId, COLORREF rgbAffectColor, BYTE pulsePeriod);
    void DeleteResPaletteAffect();
    void DeleteRangeAffects(USHORT nRangeId);
    BOOL Demand(BOOLEAN& bDemanded);
    BOOL GetCurrentCenterPoint(CPoint& ptReference, BOOLEAN bDemanded);
    BOOL GetCurrentFrameSize(CSize& frameSize, BOOLEAN bDemanded);
    BYTE GetSequenceLength(WORD nSequence, BOOLEAN bDemanded);
    BOOL GetFrameCenterPoint(SHORT nSequence, SHORT nFrame, CSize& ptCtr, BOOLEAN bDemanded);
    BOOL GetFrameSize(SHORT nSequence, SHORT nFrame, CSize& frameSize, BOOLEAN bDemanded);
    SHORT GetNumberSequences(BOOLEAN bDemanded);
    BOOL FrameSet(SHORT nFrame);
    BOOL IsEndOfSequence(BOOLEAN bDemanded);
    void RestoreBackground(INT nFrom, INT nTo, const CRect& rClip);
    void RealizePalette(DWORD dwFlags);
    BOOL SequenceSet(SHORT nSequence);
    void SetPalette(RGBQUAD* pPalette, int nEntries, USHORT nType);
    void SetPalette(const CVidPalette& cPalette);
    void AddResPaletteAffect(USHORT nAffectType, COLORREF rgbAffectColor, BYTE pulsePeriod);
    void SuppressTint(SHORT nRange);
    void UnsuppressTint(SHORT nRange);
    void BltBackGroundToFx(LPDIRECTDRAWSURFACE pSurface, CRect& rDest, const CRect& rClip, DWORD dwFlags);
    BOOL BltFromFX(LPDIRECTDRAWSURFACE pSurface, int x, int y, const CRect& rClip, DWORD dwFlags);

    void SetResRef(const CResRef& cNewResRef, BOOL bDoubleSize, BOOL bSetAutoRequest, BOOL bWarningIfMissing);
    WORD GetCurrentFrameId();
    WORD GetCurrentSequenceId();

    /* 0000 */ virtual BOOL FrameAdvance();
    /* 0004 */ virtual BOOL Render(WORD* pSurface, LONG lPitch, INT nRefPtX, INT nRefPtY, const CRect& rClip, BOOLEAN bDemanded, DWORD dwFlags, const CPoint& ptSource);
    /* 0008 */ virtual BOOL Render(INT nSurface, int x, int y, const CRect& rClip, CVidPoly* pClipPoly, int nPolys, DWORD dwFlags, int nTransVal);
    /* 000C */ virtual BOOL Render(WORD* pSurface, LONG lPitch, INT nRefPointX, INT nRefPointY, DWORD dwFlags, INT nTransVal);
    /* 0010 */ virtual void StoreBackground(INT x, INT y, const CRect& rClip, CRect& rStorage, BOOLEAN bNumbered);
    /* 0014 */ virtual void StoreBackground(INT nFrom, INT nTo, INT x, INT y, const CRect& rClip, CRect& rStorage, BOOLEAN bNumbered);
    /* 0018 */ virtual BOOL GetFrame(BOOLEAN bDemanded);

    BOOL Blt8To16(WORD* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To16(WORD* pSurface, LONG lPitch, const CPoint& pt, const CRect& rClip, DWORD dwFlags, const CPoint& ptSource);
    BOOL Blt8To16ClearShadow(WORD* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To16Brightest(WORD* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To16Translucent(WORD* pSurface, LONG lPitch, DWORD dwFlags, INT nTransVal);

    BOOL Blt8To24(BYTE* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To24(BYTE* pSurface, LONG lPitch, const CPoint& pt, const CRect& rClip, DWORD dwFlags, const CPoint& ptSource);
    BOOL Blt8To24ClearShadow(BYTE* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To24Brightest(BYTE* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To24Translucent(BYTE* pSurface, LONG lPitch, DWORD dwFlags, INT nTransVal);

    BOOL Blt8To32(DWORD* pSurface, LONG lPitch, DWORD dwFlags, INT nTransVal);
    BOOL Blt8To32(DWORD* pSurface, LONG lPitch, const CPoint& pt, const CRect& rClip, DWORD dwFlags, const CPoint& ptSource, INT nTransVal);
    BOOL Blt8To32ClearShadow(DWORD* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To32Brightest(DWORD* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To32Translucent(DWORD* pSurface, LONG lPitch, DWORD dwFlags, INT nTransVal);

    BOOL Blt8To16Glow(WORD* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To24Glow(BYTE* pSurface, LONG lPitch, DWORD dwFlags);
    BOOL Blt8To32Glow(DWORD* pSurface, LONG lPitch, DWORD dwFlags);

    static void RenderTexture(INT x, INT y, const CRect& rFxRect, CSize dataPitch, const CRect& rClip, DWORD dwFlags);
    BOOL Render3d(INT nRefPtX, INT nRefPtY, DWORD dwFlags, INT nTransVal);
    BOOL Render3d(INT nRefPtX, INT nRefPtY, const CRect& rClip, CVidPoly* pClipPoly, int nPolys, DWORD dwFlags, INT nTransVal, BOOL bDemanded);
    BOOL Render3d(INT nRefPtX, INT nRefPtY, const CRect& rClip, BOOLEAN bDemanded, DWORD dwFlags);
    BOOL FXRender3d(INT nRefPtX, INT nRefPtY, const CRect& rFXRect, DWORD dwFlags, INT nTransVal, BOOL bAlphaValues);
    BOOL FXRender3d(INT nRefPtX, INT nRefPtY, const CRect& rFXRect, const CRect& rClip, BOOLEAN a5, DWORD dwFlags, BOOL bAlphaValues);

    BAMHEADER* GetResBamHeader();
    FRAMEENTRY* GetResFrame(WORD nSequence, WORD nFrame);
    SEQUENCEENTRY* GetResSequences();
    void Release(BOOLEAN& bDemanded);

    void UnsuppressTintAllRanges();

    /* 00B4 */ CResHelper<CResCellHeader, 1100> m_header;
    /* 00C4 */ SHORT m_nCurrentFrame;
    /* 00C6 */ SHORT m_nCurrentSequence;
    /* 00C8 */ int field_C8;
    /* 00CC */ BOOL m_bPaletteChanged;
    /* 00D0 */ FRAMEENTRY* m_pFrame;
    /* 00D4 */ BOOLEAN m_bShadowOn;
    /* 00D6 */ BOOL m_bDoubleSize;
};

#endif /* CVIDCELL_H_ */
