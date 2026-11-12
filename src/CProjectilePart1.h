#ifndef CPROJECTILEPART1_H_
#define CPROJECTILEPART1_H_

#include "CAIObjectType.h"
#include "CProjectileBAM.h"

// NOTE: Scaffold. The class name is a guess (vtable 0x85BFE8, size 0x210, base
// constructor 0x4C7B30). Created by `CProjectileBAM84DBDC::AIUpdate` and
// `CProjectileBAM84DC68::Virtual88`; the layout is not known.
class CGame85BFE8 : public CGameObject {
public:
    CGame85BFE8(BYTE a1, const BYTE* pColors, CGameArea* pArea, const CPoint& pos, WORD a5, BYTE a6, BYTE a7, WORD a8);

    /* 006E */ BYTE field_6E[0x1A2];
};

// NOTE: Scaffold. The class name is a guess (vtable 0x85BF7C, size 0x104).
// Created by `CProjectileBAM84DC68::AIUpdate`; the layout is not known.
class CGame85BF7C : public CGameObject {
public:
    CGame85BF7C(DWORD a1, const BYTE* pColors, const CString& a3, CGameArea* pArea, const CPoint& pos, LONG nPosZ, const CPoint& a7, LONG a8, LONG a9, BYTE a10);

    /* 006E */ BYTE field_6E[0x96];
};

class CProjectileBAM84DA4C : public CProjectileBAM {
public:
    CProjectileBAM84DA4C(CResRef resRef);
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
};

class CProjectileBAM84DB54 : public CProjectileBAM {
public:
    CProjectileBAM84DB54(WORD a1);
};

class CProjectileBAM84DBDC : public CProjectileBAM {
public:
    CProjectileBAM84DBDC(CResRef resRef);
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0070 */ void OnArrival() override;
    /* 0088 */ virtual void Virtual88();

    BOOL sub_52D430(BOOLEAN a1);

    /* 02A0 */ SHORT field_2A0;
    /* 02A2 */ SHORT field_2A2;
    /* 02A4 */ SHORT field_2A4;
    /* 02A6 */ SHORT field_2A6;
    /* 02A8 */ LONG field_2A8;
    /* 02AC */ SHORT field_2AC;
    /* 02AE */ SHORT field_2AE;
    /* 02B0 */ LONG field_2B0;
    /* 02B4 */ CAIObjectType field_2B4;
    /* 02F0 */ BYTE field_2F0;
    /* 02F1 */ BYTE field_2F1;
    /* 02F2 */ LONG field_2F2;
    /* 02F6 */ BOOL field_2F6;
    /* 02FA */ CVidCell field_2FA;
    /* 03D4 */ BOOL field_3D4;
    /* 03D8 */ CVidCell field_3D8;
};

class CProjectileBAM84D938 : public CProjectileBAM84DBDC {
public:
    CProjectileBAM84D938(CResRef resRef);
    /* 0088 */ void Virtual88() override;
};

class CProjectileBAM84DC68 : public CProjectileBAM84DBDC {
public:
    CProjectileBAM84DC68();
    /* 000C */ void AIUpdate() override;
    /* 0088 */ void Virtual88() override;

    /* 04B2 */ BYTE field_4B2[7];
    /* 04B9 */ BYTE field_4B9;
    /* 04BA */ BYTE field_4BA;
};

class CProjectileBAM84DCF4 : public CProjectileBAM84DBDC {
public:
    CProjectileBAM84DCF4();
    /* 000C */ void AIUpdate() override;
    /* 0088 */ void Virtual88() override;

    /* 04B2 */ BYTE field_4B2[7];
    /* 04B9 */ BYTE field_4B9;
};

class CProjectileBAM84DD80 : public CProjectileBAM84DBDC {
public:
    CProjectileBAM84DD80(const CResRef& a1, const CResRef& a2, SHORT a3);
    /* 000C */ void AIUpdate() override;
    /* 0088 */ void Virtual88() override;

    /* 04B2 */ SHORT field_4B2;
    /* 04B4 */ CString field_4B4;
};

class CProjectileBAM84DE0C : public CProjectileBAM {
public:
    CProjectileBAM84DE0C(BYTE* resRef);
};

class CProjectileBAM84DE94 : public CProjectileBAM {
public:
    CProjectileBAM84DE94();
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 02A0 */ LONG field_2A0;
};

class CProjectileBAM84DF1C : public CProjectileBAM {
public:
    CProjectileBAM84DF1C();
};

class CProjectileBAM84DFA4 : public CProjectileBAM {
public:
    CProjectileBAM84DFA4();
    /* 000C */ void AIUpdate() override;
    /* 0088 */ virtual void Virtual88(CGameArea* pArea, CPoint pos);
};

#endif /* CPROJECTILEPART1_H_ */
