#ifndef CPROJECTILEPART6_H_
#define CPROJECTILEPART6_H_

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "CProjectileSingleTarget.h"
#include "CSound.h"

// vtable 0x84EEA8, sizeof 0x36
//
// NOTE: Original layout uses MSVC 6 `std::map` (16 bytes, 28 bytes nodes).
class CProjectileHelper84EEA8 {
public:
    CProjectileHelper84EEA8();
    /* 0000 */ virtual void Virtual0();
    /* 0004 */ virtual std::list<LONG> Virtual4();

    /* 0004 */ CProjectile* m_pProjectile;
    /* 0008 */ std::map<LONG, CPoint> field_8;
    /* 0018 */ LONG field_18;
    /* 001C */ LONG field_1C;
    /* 0020 */ LONG field_20;
    /* 0024 */ LONG field_24;
    /* 0028 */ LONG field_28;
    /* 002C */ LONG field_2C;
    /* 0030 */ BOOLEAN field_30;
    /* 0031 */ BOOLEAN field_31;
    /* 0032 */ LONG field_32;
};

// vtable 0x84EEB0, sizeof 0x5C
class CProjectileHelper84EEB0 : public CProjectileHelper84EEA8 {
public:
    CProjectileHelper84EEB0();
    /* 0004 */ std::list<LONG> Virtual4() override;
    void sub_55B010(CPoint& a1, CPoint& a2);

    /* 0036 */ LONG field_36;
    /* 003A */ BOOLEAN field_3A;
    /* 003B */ unsigned char field_3B;
    /* 003C */ CPoint field_3C[4];
};

// vtable 0x850DDC, sizeof 0x334
//
// NOTE: Original layout uses MSVC 6 `std::string` and `std::vector` (16 bytes
// each).
class CProjectileSingleTarget850DDC : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget850DDC(const char* a1, const char* a2);
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0070 */ void OnArrival() override;
    /* 0078 */ void DeliverEffects() override;
    /* 0088 */ virtual void Virtual88();

    /* 02AE */ CPoint field_2AE[4];
    /* 02CE */ LONG field_2CE;
    /* 02D2 */ LONG field_2D2;
    /* 02D6 */ LONG field_2D6;
    /* 02DA */ LONG field_2DA;
    /* 02DE */ std::string field_2DE;
    /* 02EE */ LONG field_2EE;
    /* 02F2 */ std::vector<CPoint> field_2F2;
    /* 0302 */ LONG field_302;
    /* 0306 */ LONG field_306;
    /* 030A */ LONG field_30A;
    /* 030E */ LONG field_30E;
    /* 0312 */ LONG field_312;
    /* 0316 */ BOOLEAN field_316;
    /* 0317 */ BOOLEAN field_317;
    /* 0318 */ CPoint field_318;
    /* 0320 */ unsigned char field_320[8];
    /* 0328 */ LONG field_328;
    /* 032C */ SHORT field_32C;
    /* 032E */ LONG field_32E;
    /* 0332 */ BOOLEAN field_332;
};

// vtable 0x850E84, sizeof 0x2D6
//
// NOTE: Original layout uses MSVC 6 `std::set` (16 bytes).
class CProjectileSingleTarget850E84 : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget850E84();
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0078 */ void DeliverEffects() override;
    /* 0088 */ virtual void Virtual88();
    /* 008C */ virtual void Virtual8C(CPoint pt);

    static void sub_57BBC0();

    static std::vector<CPoint> stru_8E52D0;
    static BOOLEAN byte_8E52E8;

    /* 02AE */ BYTE field_2AE;
    /* 02AF */ BYTE field_2AF;
    /* 02B0 */ LONG field_2B0;
    /* 02B4 */ BOOLEAN field_2B4;
    /* 02B6 */ std::set<LONG> field_2B6;
    /* 02C6 */ LONG field_2C6;
    /* 02CA */ LONG field_2CA;
    /* 02CE */ LONG field_2CE;
    /* 02D2 */ LONG field_2D2;
};

// vtable 0x850F14, sizeof 0x2BE
//
// NOTE: Original layout uses MSVC 6 `std::set` (16 bytes).
class CProjectileSingleTarget850F14 : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget850F14(const std::set<LONG>& a1);
    /* 0078 */ void DeliverEffects() override;

    /* 02AE */ std::set<LONG> field_2AE;
};

// vtable 0x85101C, sizeof 0x2EC
class CProjectileSingleTarget85101C : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget85101C();
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 02AE */ LONG field_2AE;
    /* 02B2 */ LONG field_2B2;
    /* 02B6 */ CProjectileHelper84EEA8 field_2B6;
};

// vtable 0x8511AC, sizeof 0x2AE
class CProjectileSingleTarget8511AC : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget8511AC(const char* szResRef);
};

// vtable 0x851234, sizeof 0x2BA
class CProjectileSingleTarget851234 : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget851234(const char* szResRef, const IcewindCVisualEffect& vfx);
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 02AE */ BOOLEAN field_2AE;
    /* 02AF */ BOOLEAN field_2AF;
    /* 02B0 */ LONG field_2B0;
    /* 02B4 */ LONG field_2B4;
    /* 02B8 */ BOOLEAN field_2B8;
    /* 02B9 */ BOOLEAN field_2B9;
};

// vtable 0x851444, sizeof 0x35A
class CProjectileSingleTarget851444 : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget851444();
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0070 */ void OnArrival() override;
    CPoint sub_5800E0(int a2);

    /* 02AE */ LONG field_2AE;
    /* 02B2 */ LONG field_2B2;
    /* 02B6 */ LONG field_2B6;
    /* 02BA */ CProjectileHelper84EEA8 field_2BA;
    /* 02F0 */ CSound field_2F0;
    /* 0354 */ BOOLEAN field_354;
    /* 0356 */ LONG field_356;
};

// vtable 0x8514CC, sizeof 0x374
class CProjectileSingleTarget8514CC : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget8514CC();
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0070 */ void OnArrival() override;

    /* 02AE */ LONG field_2AE;
    /* 02B2 */ BOOLEAN field_2B2;
    /* 02B4 */ CProjectileHelper84EEB0 field_2B4;
    /* 0310 */ CSound field_310;
};

// vtable 0x8513BC, sizeof 0x2E8
class CProjectileSingleTarget8513BC : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget8513BC(const char* szResRef);
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 02AE */ LONG field_2AE;
    /* 02B2 */ CProjectileHelper84EEA8 field_2B2;
};

// vtable 0x851554, sizeof 0x2E4
class CProjectileSingleTarget851554 : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget851554();
    /* 0070 */ void OnArrival() override;

    /* 02AE */ CProjectileHelper84EEA8 field_2AE;
};

// vtable 0x8515DC, sizeof 0x2B6
class CProjectileSingleTarget8515DC : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget8515DC();
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 02AE */ BYTE field_2AE;
    /* 02AF */ BYTE field_2AF;
    /* 02B0 */ LONG field_2B0;
    /* 02B4 */ BOOLEAN field_2B4;
};

// vtable 0x851664, sizeof 0x374
class CProjectileSingleTarget851664 : public CProjectileSingleTarget {
public:
    CProjectileSingleTarget851664();
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0070 */ void OnArrival() override;

    /* 02AE */ LONG field_2AE;
    /* 02B2 */ BOOLEAN field_2B2;
    /* 02B4 */ CProjectileHelper84EEB0 field_2B4;
    /* 0310 */ CSound field_310;
};

#endif /* CPROJECTILEPART6_H_ */
