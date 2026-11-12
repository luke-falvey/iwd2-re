#ifndef CPROJECTILEAREA_H_
#define CPROJECTILEAREA_H_

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "CAIObjectType.h"
#include "CGameAnimation.h"
#include "CGameObject.h"
#include "CProjectileSingleTarget.h"
#include "CSound.h"
#include "CVidCell.h"
#include "CVidPalette.h"

// NOTE: Original layout uses MSVC 6 `std::string` (16 bytes).
class CProjectileAreaVisual {
public:
    /* 0000 */ std::string field_0;
    /* 0010 */ std::string field_10;
    /* 0020 */ IcewindCVisualEffect field_20;
};

class CProjectileAreaVisual2 : public CProjectileAreaVisual {
public:
    CProjectileAreaVisual2();

    /* 002C */ BOOLEAN field_2C;
    /* 002E */ LONG field_2E;
};

// NOTE: Original layout uses MSVC 6 `std::vector` (16 bytes).
class CProjectileArea5868E0 {
public:
    class Entry {
    public:
        /* 0000 */ LONG field_0;
        /* 0004 */ LONG field_4;
        /* 0008 */ BYTE field_8;

        Entry(LONG a1, LONG a2, BYTE a3);
    };

    CProjectileArea5868E0();

    /* 0000 */ std::vector<Entry> field_0;
    /* 0010 */ LONG field_10;
};

class CProjectileAreaVisual3 : public CProjectileAreaVisual {
public:
    CProjectileAreaVisual3();

    LONG GetRandomMax() const;

    /* 002C */ CProjectileArea5868E0* field_2C;
    /* 0030 */ LONG field_30;
    /* 0034 */ BOOLEAN field_34;
    /* 0035 */ BOOLEAN field_35;
    /* 0036 */ BOOLEAN field_36;
    /* 0038 */ LONG field_38;
    /* 003C */ LONG field_3C;
    /* 0040 */ LONG field_40;
    /* 0044 */ BOOLEAN field_44;
};

// vtable 0x84F0DC, sizeof 0x2A2
// NOTE: Created by `CProjectileArea::OnArrival`. The original class name is
// not known.
class CProjectileArea84F0DC : public CGameObject {
public:
    // NOTE: Element of `m_pCells` (16 bytes each).
    struct CELL {
        LONG nOffsetY;
        LONG nOffsetX;
        LONG nVelocityY;
        LONG nVelocityX;
    };

    CProjectileArea84F0DC(const CProjectileAreaVisual* a1, const CProjectileAreaVisual2* a2, const CProjectileAreaVisual3* a3, CGameArea* pArea, const CPoint* pos, SHORT a6, BYTE a7, BYTE a8, WORD a9);
    /* 0000 */ ~CProjectileArea84F0DC() override;
    /* 000C */ void AIUpdate() override;
    /* 0048 */ void RemoveFromArea() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;

    /* 006E */ BYTE m_terrainTable[16];
    /* 007E */ SHORT field_7E;
    /* 0080 */ SHORT field_80;
    /* 0082 */ BYTE field_82[8];
    /* 008A */ CVidCell field_8A;
    /* 0164 */ CVidPalette field_164;
    /* 0188 */ WORD field_188;
    /* 018A */ BYTE field_18A;
    /* 018B */ BYTE field_18B;
    /* 018C */ LONG field_18C;
    /* 0190 */ LONG field_190;
    /* 0194 */ BYTE* m_pGrid;
    /* 0198 */ LONG field_198;
    /* 019C */ LONG field_19C;
    /* 01A0 */ CELL* m_pCells;
    /* 01A4 */ BYTE* m_pCellStates;
    /* 01A8 */ CSound field_1A8;
    /* 020C */ CString field_20C;
    /* 0210 */ CProjectileAreaVisual2 field_210;
    /* 0242 */ CProjectileAreaVisual3 field_242;
    /* 0288 */ LONG field_288;
    /* 028C */ LONG field_28C;
    /* 0290 */ IcewindCVisualEffect field_290;
    /* 029C */ LONG field_29C;
    /* 02A0 */ BOOLEAN field_2A0;
};

// NOTE: The class name is not known. Piece created by
// `CProjectileArea84F0DC::AIUpdate`. Vtable 0x84F14C, size 0x112. Has two
// constructors (one taking `CProjectileAreaVisual3`, one taking
// `CProjectileAreaVisual2`).
class CProjectileArea84F14C : public CGameObject {
public:
    CProjectileArea84F14C(const CProjectileAreaVisual3* a1, CGameArea* pArea, const CPoint& pos, LONG nPosZ, const CPoint& ptVelocity, WORD nTicksLeft, BYTE nFadeTicks, BYTE nMode);
    CProjectileArea84F14C(const CProjectileAreaVisual2* a1, CGameArea* pArea, const CPoint& pos, LONG nPosZ, const CPoint& ptVelocity, WORD nTicksLeft, BYTE nFadeTicks, BYTE nMode);
    /* 0000 */ ~CProjectileArea84F14C() override;
    /* 000C */ void AIUpdate() override;
    /* 0048 */ void RemoveFromArea() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;

    /* 006E */ BYTE m_terrainTable[16];
    /* 007E */ CGameAnimation m_animation;
    /* 0088 */ LONG m_bVisible;
    /* 008C */ CPoint m_ptFixedPos;
    /* 0094 */ CPoint m_ptVelocity;
    /* 009C */ WORD m_nTicksLeft;
    /* 009E */ BYTE m_nFadeTicks;
    /* 009F */ BYTE m_nMode;
    /* 00A0 */ CSound m_sound;
    /* 0104 */ CProjectileArea5868E0* field_104;
    /* 0108 */ LONG field_108;
    /* 010C */ BOOLEAN field_10C;
    /* 010D */ BOOLEAN field_10D;
    /* 010E */ LONG field_10E;
};

class CProjectileArea : public CProjectileSingleTarget {
public:
    CProjectileArea(SHORT a1);
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0070 */ void OnArrival() override;
    /* 0088 */ virtual void Virtual88();
    /* 008C */ virtual LONG Virtual8C(BYTE a1);
    /* 0090 */ virtual std::list<LONG> Virtual90();
    /* 0094 */ virtual void Virtual94(std::list<LONG>& targets);
    /* 0098 */ virtual void Virtual98(LONG nTargetId);

    void sub_5703F0(CGameAIBase* pSource);
    void sub_5704E0(CGameAIBase* pSource);
    void sub_5705D0(LONG nSourceId);

    /* 02AE */ SHORT field_2AE;
    /* 02B0 */ SHORT field_2B0;
    /* 02B2 */ SHORT field_2B2;
    /* 02B4 */ SHORT field_2B4;
    /* 02B6 */ LONG field_2B6;
    /* 02BA */ SHORT field_2BA;
    /* 02BC */ SHORT field_2BC;
    /* 02BE */ CAIObjectType field_2BE;
    /* 02FA */ LONG field_2FA;
    /* 02FE */ BOOLEAN field_2FE;
    /* 02FF */ BOOLEAN field_2FF;
    /* 0300 */ LONG field_300;
    /* 0304 */ LONG field_304;
    /* 0308 */ CVidCell field_308;
    /* 03E2 */ LONG field_3E2;
    /* 03E6 */ CVidCell field_3E6;
    /* 04C0 */ LONG field_4C0;
    /* 04C4 */ std::map<LONG, LONG> field_4C4;
    /* 04D4 */ LONG field_4D4;
    /* 04D8 */ LONG field_4D8;
    /* 04DC */ LONG field_4DC;
    /* 04E0 */ BOOLEAN field_4E0;
    /* 04E2 */ CProjectileAreaVisual field_4E2;
    /* 050E */ CProjectileAreaVisual2 field_50E;
    /* 0540 */ CProjectileAreaVisual3 field_540;
    /* 0586 */ CSound field_586;
    /* 05EA */ CSound field_5EA;
    /* 064E */ std::set<LONG> field_64E;
};

#endif /* CPROJECTILEAREA_H_ */
