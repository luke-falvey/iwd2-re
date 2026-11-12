#ifndef CPROJECTILEPART2_H_
#define CPROJECTILEPART2_H_

#include "CAIObjectType.h"
#include "CProjectileBAM.h"

// vtable 0x84E030, sizeof 0x2C0
class CProjectileBAM84E030 : public CProjectileBAM {
public:
    CProjectileBAM84E030(WORD a1, CResRef resRef, SHORT nCount, WORD a4);
    /* 0000 */ ~CProjectileBAM84E030() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 02A0 */ SHORT field_2A0;
    /* 02A2 */ CTypedPtrList<CPtrList, CProjectile*> field_2A2;
    /* 02BE */ WORD field_2BE;
};

// vtable 0x84E14C, sizeof 0x2A0
class CProjectileBAM84E14C : public CProjectileBAM {
public:
    CProjectileBAM84E14C();
};

// vtable 0x84E1D4, sizeof 0x2A0
class CProjectileBAM84E1D4 : public CProjectileBAM {
public:
    CProjectileBAM84E1D4();
};

// vtable 0x84E25C, sizeof 0x2A0
class CProjectileBAM84E25C : public CProjectileBAM {
public:
    CProjectileBAM84E25C();
};

// vtable 0x84E2E4, sizeof 0x2A0
class CProjectileBAM84E2E4 : public CProjectileBAM {
public:
    CProjectileBAM84E2E4();
};

// vtable 0x84E36C, sizeof 0x2A0
class CProjectileBAM84E36C : public CProjectileBAM {
public:
    CProjectileBAM84E36C();
};

// vtable 0x84E3F4, sizeof 0x2A0
class CProjectileBAM84E3F4 : public CProjectileBAM {
public:
    CProjectileBAM84E3F4();
};

// vtable 0x84E47C, sizeof 0x2A0
class CProjectileBAM84E47C : public CProjectileBAM {
public:
    CProjectileBAM84E47C();
};

// vtable 0x84E504, sizeof 0x2A0
class CProjectileBAM84E504 : public CProjectileBAM {
public:
    CProjectileBAM84E504();
};

// vtable 0x84E58C, sizeof 0x2A0
class CProjectileBAM84E58C : public CProjectileBAM {
public:
    CProjectileBAM84E58C();
};

// vtable 0x84E614, sizeof 0x2A8
class CProjectileBAM84E614 : public CProjectileBAM {
public:
    CProjectileBAM84E614();
    /* 000C */ void AIUpdate() override;

    /* 02A0 */ BYTE field_2A0[8];
};

// NOTE: Not in the original. Vtables 0x84E69C and 0x84E724 share AIUpdate
// (0x533A50) but not Fire, so they cannot derive from each other.
class CProjectileBAM84E69CBase : public CProjectileBAM {
public:
    CProjectileBAM84E69CBase(CResRef resRef)
        : CProjectileBAM(resRef)
    {
    }

    /* 000C */ void AIUpdate() override;

    /* 02A0 */ SHORT field_2A0;
};

// vtable 0x84E69C, sizeof 0x2A2
class CProjectileBAM84E69C : public CProjectileBAM84E69CBase {
public:
    CProjectileBAM84E69C();
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    void sub_533600(CPoint* pt, SHORT nCount, SHORT nDirection, BOOL a4);
};

// vtable 0x84E724, sizeof 0x2A2
class CProjectileBAM84E724 : public CProjectileBAM84E69CBase {
public:
    CProjectileBAM84E724(CResRef resRef, SHORT a2, SHORT nDirection);
    /* 0000 */ ~CProjectileBAM84E724() override;
    /* 0088 */ virtual void Virtual88(CGameArea* pArea, CPoint* pt, LONG posZ);
};

// vtable 0x84E7B0, sizeof 0x2FA
class CProjectileBAM84E7B0 : public CProjectileBAM {
public:
    CProjectileBAM84E7B0(WORD a1, CResRef resRef, WORD a3, WORD a4, LONG a5, CAIObjectType type);
    /* 0000 */ ~CProjectileBAM84E7B0() override;
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 007C */ void CallBack() override;

    /* 02A0 */ WORD field_2A0;
    /* 02A2 */ WORD field_2A2;
    /* 02A4 */ LONG field_2A4;
    /* 02A8 */ WORD field_2A8;
    /* 02AA */ CProjectile* field_2AA;
    /* 02AE */ LONG field_2AE;
    /* 02B2 */ CAIObjectType field_2B2;
    /* 02EE */ LONG field_2EE;
    /* 02F2 */ WORD field_2F2;
    /* 02F4 */ LONG field_2F4;
    /* 02F8 */ SHORT field_2F8;
};

// vtable 0x8510A4, sizeof 0x2A0
class CProjectileBAM8510A4 : public CProjectileBAM {
public:
    CProjectileBAM8510A4(WORD a1);
};

// vtable 0x84E0C4, sizeof 0x2C0
class CProjectileBAM84E0C4 : public CProjectileBAM84E030 {
public:
    CProjectileBAM84E0C4(SHORT nCount, WORD a2);
};

// vtable 0x84E838, sizeof 0x2FA
class CProjectileBAM84E838 : public CProjectileBAM84E7B0 {
public:
    CProjectileBAM84E838(WORD a1, LONG a2);
};

#endif /* CPROJECTILEPART2_H_ */
