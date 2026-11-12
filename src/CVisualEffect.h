#ifndef CVISUALEFFECT_H_
#define CVISUALEFFECT_H_

#include "CGameObject.h"
#include "mfc.h"

class CVisualEffect : public CGameObject {
public:
    // 0x554D20
    static LONG sub_554D20(CString sVefName, CGameArea* pArea, CPoint& ptStart, LONG target, CPoint& targetPos, LONG nHeight);

    // 0x555200
    void SetAllDelete();

    /* 006E */ unsigned char field_6E[20];
    /* 0082 */ LONG field_82;
    /* 0086 */ DWORD m_dwFlags; // #guess
    /* 008A */ unsigned char field_8A[16];
    /* 009A */ DWORD field_9A;
    /* 009E */ unsigned char field_9E[400];
    /* 022E */ CTypedPtrList<CPtrList, LONG*> m_lChildren; // #guess: object ids
};

#endif /* CVISUALEFFECT_H_ */
