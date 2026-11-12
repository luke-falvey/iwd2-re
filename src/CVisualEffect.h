#ifndef CVISUALEFFECT_H_
#define CVISUALEFFECT_H_

#include "CGameObject.h"
#include "CResBinary.h"
#include "mfc.h"

class CVisualEffect;

// Layout of the VEF resource header (only the parts used by 0x5545E0).
struct VEF_HEADER {
    /* 0000 */ unsigned char field_0[8];
    /* 0008 */ DWORD nComponentsOffset1;
    /* 000C */ DWORD nComponents1;
    /* 0010 */ DWORD nComponentsOffset2;
    /* 0014 */ DWORD nComponents2;
    /* 0018 */ LONG field_18;
    /* 001C */ LONG field_1C;
    /* 0020 */ DWORD field_20;
};

// A 0xE0 byte VEF component, same layout in the file and in memory.
struct CVisualEffectComponent {
    CVisualEffectComponent();

    /* 0000 */ DWORD field_0;
    /* 0004 */ DWORD field_4;
    /* 0008 */ DWORD field_8;
    /* 000C */ DWORD field_C;
    /* 0010 */ CResRef cResRef;
    /* 0018 */ DWORD field_18[50];
};

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
    // 0x554870
    CVisualEffect();

    // 0x554D20
    static LONG sub_554D20(CString sVefName, CGameArea* pArea, CPoint& ptStart, LONG target, CPoint& targetPos, LONG nHeight);

    // 0x555200
    void SetAllDelete();

    /* 006E */ unsigned char field_6E[16];
    /* 007E */ LONG field_7E; // #guess: set from VEF resource data (header +0x18) by 0x5545E0
    /* 0082 */ LONG field_82; // #guess: set from VEF resource data (header +0x1C) by 0x5545E0, default -1
    /* 0086 */ DWORD m_dwFlags; // #guess: set from VEF resource data (header +0x20) by 0x5545E0, default -1
    /* 008A */ DWORD field_8A;
    /* 008E */ DWORD field_8E;
    /* 0092 */ unsigned char field_92[8];
    /* 009A */ DWORD field_9A;
    /* 009E */ unsigned char field_9E[400];
    /* 022E */ CTypedPtrList<CPtrList, LONG*> m_lChildren; // #guess: object ids
    /* 024A */ CTypedPtrList<CPtrList, CVisualEffectComponent*> field_24A; // #guess: VEF components (0xE0 byte entries), filled by 0x5545E0
    /* 0266 */ CTypedPtrList<CPtrList, CVisualEffectComponent*> field_266;
    /* 0282 */ LONG field_282; // x of the target position (or of the start point when none was given)
    /* 0286 */ LONG field_286; // y of the target position (or of the start point when none was given)
    /* 028A */ LONG field_28A; // #guess: target object id, initialised to INVALID_INDEX
    /* 028E */ LONG field_28E; // pos.x << CParticle::RESOLUTION_INC, set by sub_554D20
    /* 0292 */ LONG field_292; // (pos.y * 4 / 3) << CParticle::RESOLUTION_INC, set by sub_554D20
    /* 0296 */ DWORD field_296;
    /* 029A */ DWORD field_29A;
    /* 029E */ DWORD field_29E;
    /* 02A2 */ DWORD field_2A2;
    /* 02A6 */ DWORD field_2A6;
    /* 02AA */ DWORD field_2AA;
    /* 02AE */ DWORD field_2AE;
    /* 02B2 */ unsigned char field_2B2[16];
};

#endif /* CVISUALEFFECT_H_ */
