#ifndef CGAMEANIMATIONTYPE84EEB8_H_
#define CGAMEANIMATIONTYPE84EEB8_H_

#include <string>

#include "CGameAnimationType.h"
#include "CVidCell.h"
#include "CVidPalette.h"
#include "IcewindCVisualEffect.h"

class CProjectileAreaVisual2;
class CProjectileAreaVisual3;

// NOTE: Assertion strings name the class `IcewindCGameAnimationTypeEffect`
// (not to be confused with `CGameAnimationTypeEffect`). Animation of the
// pieces created by `CProjectileArea84F0DC::AIUpdate`. Vtable 0x84EEB8, size
// 0x606.
class CGameAnimationType84EEB8 : public CGameAnimationType {
public:
    CGameAnimationType84EEB8(const CProjectileAreaVisual3* a1, SHORT nDirection);
    CGameAnimationType84EEB8(const CProjectileAreaVisual2* a1, SHORT nDirection);
    /* 0000 */ ~CGameAnimationType84EEB8() override;
    /* 0004 */ void CalculateFxRect(CRect& rFx, CPoint& ptReference, LONG posZ) override;
    /* 0008 */ void CalculateGCBoundsRect(CRect& rGCBounds, const CPoint& pos, const CPoint& ptReference, LONG posZ, LONG nWidth, LONG nHeight) override;
    /* 000C */ void ChangeDirection(SHORT nDirection) override;
    /* 002C */ BOOLEAN DetectedByInfravision() override;
    /* 007C */ BOOL IsMirroring() override;
    /* 0084 */ BOOL IsEndOfSequence() override;
    /* 0088 */ void IncrementFrame() override;
    /* 0090 */ void Render(CInfinity* pInfinity, CVidMode* pVidMode, INT nSurface, const CRect& rectFX, const CPoint& ptNewPos, const CPoint& ptReference, DWORD dwRenderFlags, COLORREF rgbTintColor, const CRect& rGCBounds, BOOL bDithered, BOOL bFadeOut, LONG posZ, BYTE transparency) override;
    /* 00AC */ SHORT SetSequence(SHORT nSequence) override;

    // NOTE: Sets the BAM to play from its resource name.
    void sub_55DBD0(const char* pName);

    /* 03FE */ CVidCell* m_currentVidCell;
    /* 0402 */ CVidCell* m_currentVidCellShadow;
    /* 0406 */ CVidCell m_g1VidCell;
    /* 04E0 */ CVidCell m_g1VidCellShadow;
    /* 05BA */ CVidPalette m_palette;
    /* 05DE */ BYTE field_5DE;
    /* 05DF */ BYTE field_5DF;
    /* 05E0 */ BOOLEAN m_bRender;
    /* 05E1 */ BYTE field_5E1;
    /* 05E2 */ SHORT m_currentBamSequence;
    /* 05E4 */ SHORT m_currentBamDirection;
    /* 05E6 */ BYTE m_extendDirectionTest;
    /* 05E7 */ BOOLEAN field_5E7;
    /* 05E8 */ BOOLEAN field_5E8;
    /* 05EA */ IcewindCVisualEffect m_visualEffect;
    // NOTE: Original layout uses MSVC 6 `std::string` (16 bytes).
    /* 05F6 */ std::string field_5F6;
};

#endif /* CGAMEANIMATIONTYPE84EEB8_H_ */
