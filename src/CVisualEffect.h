#ifndef CVISUALEFFECT_H_
#define CVISUALEFFECT_H_

#include "CGameObject.h"
#include "CResBinary.h"
#include "mfc.h"

class CVisualEffect;

// NOTE: In the original, 0x5544C0 and 0x5545E0 are members of the
// `CResHelper<CResBinary, 1020>` instantiation (VEF resources). They are
// declared on a derived helper until that instantiation has a proper home.
class CResHelperVEF : public CResHelper<CResBinary, 1020> {
public:
    // 0x5544C0
    CResHelperVEF(CResRef cNewResRef);

    // 0x5545E0
    CVisualEffect* FUN_005545e0();
};

class CVisualEffect : public CGameObject {
public:
    // 0x554D20
    static LONG sub_554D20(CString sVefName, CGameArea* pArea, CPoint& ptStart, LONG target, CPoint& targetPos, LONG nHeight);

    // 0x555200
    void SetAllDelete();

    /* 006E */ unsigned char field_6E[16];
    /* 007E */ LONG field_7E; // #guess: set from VEF resource data (header +0x18) by 0x5545E0
    /* 0082 */ LONG field_82; // #guess: set from VEF resource data (header +0x1C) by 0x5545E0, default -1
    /* 0086 */ DWORD m_dwFlags; // #guess: set from VEF resource data (header +0x20) by 0x5545E0, default -1
    /* 008A */ unsigned char field_8A[16];
    /* 009A */ DWORD field_9A;
    /* 009E */ unsigned char field_9E[400];
    /* 022E */ CTypedPtrList<CPtrList, LONG*> m_lChildren; // #guess: object ids
    /* 024A */ CPtrList field_24A; // #guess: VEF components (0xE0 byte entries), filled by 0x5545E0
    /* 0266 */ CPtrList field_266;
    /* 0282 */ LONG field_282; // x of the target position (or of the start point when none was given)
    /* 0286 */ LONG field_286; // y of the target position (or of the start point when none was given)
    /* 028A */ LONG field_28A; // #guess: target object id, initialised to INVALID_INDEX
    /* 028E */ LONG field_28E; // pos.x << CParticle::RESOLUTION_INC, set by sub_554D20
    /* 0292 */ LONG field_292; // (pos.y * 4 / 3) << CParticle::RESOLUTION_INC, set by sub_554D20
    /* 0296 */ unsigned char field_296[28];
    /* 02B2 */ unsigned char field_2B2[16];
};

#endif /* CVISUALEFFECT_H_ */
