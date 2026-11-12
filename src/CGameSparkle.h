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

    /* 006E */ CVidMode* m_pVidMode;
    /* 0072 */ CTypedPtrList<CPtrList, CParticle*> m_particleList;
    /* 008E */ COLORREF m_aColors[CGAMESPARKLE_NUM_COLORS];
    /* 00A2 */ BYTE field_A2;
    /* 00A3 */ BYTE field_A3;
    /* 00A4 */ BYTE field_A4;
    /* 00A5 */ BYTE field_A5;
    /* 00A6 */ CRect m_rBounding;
    /* 00B6 */ WORD m_wType;
    /* 00B8 */ LONG field_B8;
    /* 00BC */ WORD field_BC;
    /* 00BE */ CPoint m_velocity;
    /* 00C6 */ LONG field_C6;
};

#endif /* CGAMESPARKLE_H_ */
