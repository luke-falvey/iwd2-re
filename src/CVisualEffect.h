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

class CVidCell;

// NOTE: Name is invented. These are the objects whose ids are kept in
// `CVisualEffect::m_lChildren` (`CVisualEffect::AIUpdate` fetches them by id).
// They are bigger than a `CVisualEffect` (>= 0x340 bytes) and have two
// `CVidCell`s. Their constructor and vtable have not been located, so only
// the members used by the translated functions are named.
class CVisualEffectObject : public CGameObject {
public:
    // 0x5513F0
    void sub_5513F0(LONG nCounter);

    // 0x551490
    void sub_551490(const CPoint* pt);

    // 0x551520
    void sub_551520(const CPoint* ptPos, const CPoint* ptPrevPos, const CPoint* ptTarget, SHORT nDirection);

    // 0x551670
    static SHORT sub_551670(const CPoint* ptFrom, const CPoint* ptTo, SHORT nDirection);

    /* 006E */ unsigned char field_6E[24];
    /* 0086 */ DWORD m_dwFlags; // 0x4: follows the height of the parent, 0x20: delete
    /* 008A */ unsigned char field_8A[4];
    /* 008E */ LONG field_8E; // offset from the parent position
    /* 0092 */ LONG field_92;
    /* 0096 */ DWORD field_96; // 0x1: offset is rotated by `field_33C`
    /* 009A */ LONG field_9A;
    /* 009E */ unsigned char field_9E[4];
    /* 00A2 */ WORD field_A2; // direction used when the parent has not moved
    /* 00A4 */ unsigned char field_A4[2];
    /* 00A6 */ DWORD field_A6; // 0x1: face the target, 0x2: fixed direction
    /* 00AA */ unsigned char field_AA[24];
    /* 00C2 */ LONG field_C2; // #guess: lifetime in ticks (<= 0 for none)
    /* 00C6 */ unsigned char field_C6[396];
    /* 0252 */ CVidCell* field_252;
    /* 0256 */ CVidCell* field_256;
    /* 025A */ unsigned char field_25A[222];
    /* 0338 */ LONG field_338; // set when the object is finished
    /* 033C */ LONG field_33C; // direction
};

class CVisualEffect : public CGameObject {
public:
    // 0x554870
    CVisualEffect();

    // 0x5549D0 (0x5549B0 is the scalar deleting destructor)
    /* 0000 */ ~CVisualEffect() override;

    // 0x554B70
    /* 000C */ void AIUpdate() override;

    // 0x554B60
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;

    // 0x554F00
    void sub_554F00(CPoint* pt);

    // 0x554D20
    static LONG sub_554D20(CString sVefName, CGameArea* pArea, CPoint& ptStart, LONG target, CPoint& targetPos, LONG nHeight);

    // 0x555200
    void SetAllDelete();

    /* 006E */ unsigned char field_6E[16];
    /* 007E */ LONG field_7E; // #guess: set from VEF resource data (header +0x18) by 0x5545E0
    /* 0082 */ LONG field_82; // #guess: set from VEF resource data (header +0x1C) by 0x5545E0, default -1
    /* 0086 */ DWORD m_dwFlags; // #guess: set from VEF resource data (header +0x20) by 0x5545E0, default -1
    /* 008A */ LONG field_8A;
    /* 008E */ LONG field_8E;
    /* 0092 */ LONG field_92;
    /* 0096 */ unsigned char field_96[4];
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
    /* 0296 */ CPoint field_296; // position at the previous update
    /* 029E */ LONG field_29E; // update counter
    /* 02A2 */ DWORD field_2A2;
    /* 02A6 */ LONG field_2A6; // direction
    /* 02AA */ LONG field_2AA;
    /* 02AE */ LONG field_2AE;
    /* 02B2 */ unsigned char field_2B2[16];
};

#endif /* CVISUALEFFECT_H_ */
