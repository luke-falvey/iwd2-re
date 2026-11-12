#ifndef CVISUALEFFECT_H_
#define CVISUALEFFECT_H_

#include "CGameObject.h"
#include "mfc.h"

class CVisualEffect : public CGameObject {
public:
    // 0x555200
    void SetAllDelete();

    /* 006E */ unsigned char field_6E[24];
    /* 0086 */ DWORD m_dwFlags; // #guess
    /* 008A */ unsigned char field_8A[420];
    /* 022E */ CTypedPtrList<CPtrList, LONG*> m_lChildren; // #guess: object ids
};

#endif /* CVISUALEFFECT_H_ */
