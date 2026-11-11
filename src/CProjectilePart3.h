#ifndef CPROJECTILEPART3_H_
#define CPROJECTILEPART3_H_

#include "CProjectile.h"
#include "CVidBitmap.h"
#include "CVidCell.h"
#include "CVidPalette.h"
#include "IcewindCVisualEffect.h"

class CProjectile84DAD4 : public CProjectile {
public:
    CProjectile84DAD4();
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
};

class CProjectile84D8B8 : public CProjectile {
public:
    CProjectile84D8B8();
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
};

// NOTE: Ghidra database calls it `CProjectileShaft`.
class CProjectile84E8C0 : public CProjectile {
public:
    CProjectile84E8C0(const CString& sVidCell, const CString& sSound, BYTE nColor, LONG a4, LONG a5, SHORT a6);
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 0192 */ CVidCell m_vidCell;
    /* 026C */ CVidPalette m_palette;
    /* 0290 */ LONG field_290;
    /* 0294 */ LONG field_294;
    /* 0298 */ SHORT field_298;
};

class CProjectile84E940 : public CProjectile {
public:
    CProjectile84E940(const CString& sVidCell, const CString& sSound, const CString& sBitmap, LONG a4, BYTE a5);
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 0192 */ CVidCell m_vidCell;
    /* 026C */ CVidBitmap m_bitmap;
    /* 0326 */ LONG field_326;
    /* 032A */ BOOLEAN field_32A;
    /* 032B */ BYTE field_32B;
    /* 032C */ CString field_32C;
    /* 0330 */ BOOLEAN field_330;
};

class CProjectile84E9C0 : public CProjectile {
public:
    CProjectile84E9C0(const CString& sVidCell, const CString& sSound, const CString& sBitmap, LONG a4, BYTE a5);
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 0192 */ CVidCell m_vidCell;
    /* 026C */ CVidBitmap m_bitmap;
    /* 0326 */ LONG field_326;
    /* 032A */ BOOLEAN field_32A;
    /* 032B */ BYTE field_32B;
};

class CProjectile850F9C : public CProjectile {
public:
    CProjectile850F9C(const CString& sVidCell, const CString& sSound, BYTE a3, BYTE a4, const IcewindCVisualEffect& vfx);
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 0192 */ CVidCell m_vidCell;
    /* 026C */ CVidBitmap m_bitmap;
    /* 0326 */ LONG field_326;
    /* 032A */ BYTE field_32A;
    /* 032B */ BYTE field_32B;
    /* 032C */ BYTE field_32C;
    /* 032D */ BYTE field_32D;
    /* 032E */ IcewindCVisualEffect m_vfx;
};

class CProjectile8512BC : public CProjectile {
public:
    CProjectile8512BC(const CString& sVidCell, const CString& sSound, BYTE nColor, LONG a4, LONG a5, SHORT a6);
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    /* 0192 */ CVidCell m_vidCell;
    /* 026C */ CVidPalette m_palette;
    /* 0290 */ LONG field_290;
    /* 0294 */ LONG field_294;
    /* 0298 */ SHORT field_298;
    /* 029A */ LONG field_29A;
};

class CProjectile85133C : public CProjectile {
public:
    CProjectile85133C();
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
};

class CProjectile85112C : public CProjectile84D8B8 {
public:
    CProjectile85112C();
    /* 0078 */ void DeliverEffects() override;
};

#endif /* CPROJECTILEPART3_H_ */
