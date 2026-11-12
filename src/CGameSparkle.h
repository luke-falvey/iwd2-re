#ifndef CGAMESPARKLE_H_
#define CGAMESPARKLE_H_

#include "CGameObject.h"
#include "mfc.h"

#define CGAMESPARKLE_NUM_COLORS 5

class CParticle;
class CVidMode;

// NOTE: The class name is a guess. The object is the particle cluster created
// by `CGameEffectSparkle::ApplyEffect` and by the projectiles' arrival code
// (size 0xCA, vtable 0x84D624, colors from `SPARKLE.2DA`).
class CGameSparkle : public CGameObject {
public:
    static const WORD TYPE_EXPLOSION;

    CGameSparkle(SHORT nSparkleType, WORD wType, WORD a3);
    /* 0000 */ ~CGameSparkle() override;
    /* 0008 */ void AddToArea(CGameArea* pNewArea, const CPoint& pos, LONG posZ, BYTE listType) override;
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;

    void SetExplosionVelocity(const CPoint& velocity);
    void sub_51CE50();

    /* 006E */ CVidMode* m_pVidMode;
    /* 0072 */ CTypedPtrList<CPtrList, CParticle*> m_particleList;
    /* 008E */ COLORREF m_aColors[CGAMESPARKLE_NUM_COLORS];
    // NOTE: Height thresholds set by `AddToArea` from the start height (99%,
    // 85%, 75%, 55%); `AIUpdate` picks the particle colour by height band.
    /* 00A2 */ BYTE m_aHeightThresholds[4];
    /* 00A6 */ CRect m_rBounding;
    /* 00B6 */ WORD m_wType;
    // NOTE: Bursts emitted so far / to emit (only when `m_wType & 2`).
    /* 00B8 */ DWORD m_nBurstCount;
    /* 00BC */ WORD m_nBurstMax;
    /* 00BE */ CPoint m_velocity;
    // NOTE: Base gravity of the particles made by `AddToArea` (50).
    /* 00C6 */ LONG m_nGravity;
};

#endif /* CGAMESPARKLE_H_ */
