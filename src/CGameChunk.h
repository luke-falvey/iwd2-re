#ifndef CGAMECHUNK_H_
#define CGAMECHUNK_H_

#include "CGameAnimation.h"
#include "CGameObject.h"
#include "CSound.h"
#include "CVidCell.h"
#include "CVidPalette.h"
#include "mfc.h"

// NOTE: These three classes live in `ObjCreature.cpp` in the original (the
// constructor assertion of `CGame85BFE8` names it), next to `CGameSprite`.

// Flying piece of a shattered (frozen, stoned, ...) creature. Created by
// `CGameSprite::Shatter`. Vtable 0x85BF10, size 0x100.
class CGameChunk : public CGameObject {
public:
    CGameChunk(USHORT animationID, BYTE* pColors, const CString& sSoundName, CGameArea* pArea, const CPoint& pos, const CPoint& a6, LONG nPosZ, LONG a8, BYTE a9, BYTE a10);
    /* 0000 */ ~CGameChunk() override;
    /* 000C */ void AIUpdate() override;
    /* 0028 */ BOOLEAN CanSaveGame(STRREF& strError) override;
    /* 0048 */ void RemoveFromArea() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;

    /* 006E */ BYTE m_terrainTable[16];
    /* 007E */ CGameAnimation m_animation;
    /* 0088 */ LONG m_bFlying;
    /* 008C */ CPoint m_ptVelocity;
    /* 0094 */ LONG m_nVelocityZ;
    /* 0098 */ BYTE m_nBounces;
    /* 0099 */ BYTE m_nTicksLeft;
    /* 009A */ BYTE m_nFadeTicks;
    /* 009C */ CSound m_sound;
};

// NOTE: The class name is a guess. Piece of a burst created by
// `CGame85BFE8` and by the projectiles. Vtable 0x85BF7C, size 0x104.
class CGame85BF7C : public CGameObject {
public:
    CGame85BF7C(USHORT animationID, BYTE* pColors, const CString& sSoundName, CGameArea* pArea, const CPoint& pos, LONG nPosZ, const CPoint& a7, WORD a8, BYTE a9, BYTE a10);
    /* 0000 */ ~CGame85BF7C() override;
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
};

// NOTE: The class name is a guess. Ground effect with a sound and a BAM, which
// spreads pieces (`CGame85BF7C`) over an ellipse around its position. Created
// by `CProjectileBAM84DBDC::AIUpdate` and `CProjectileBAM84DC68::Virtual88`.
// Vtable 0x85BFE8, size 0x210.
class CGame85BFE8 : public CGameObject {
public:
    // NOTE: Element of `m_pCells` (16 bytes each).
    struct CELL {
        LONG nOffsetY;
        LONG nOffsetX;
        LONG nVelocityY;
        LONG nVelocityX;
    };

    CGame85BFE8(BYTE a1, const BYTE* pColors, CGameArea* pArea, const CPoint& pos, WORD a5, BYTE a6, BYTE a7, WORD a8);
    /* 0000 */ ~CGame85BFE8() override;
    /* 000C */ void AIUpdate() override;
    /* 0048 */ void RemoveFromArea() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;

    /* 006E */ BYTE m_terrainTable[16];
    /* 007E */ WORD field_7E;
    /* 0080 */ WORD field_80;
    /* 0082 */ BYTE m_colors[7];
    /* 008A */ CVidCell m_vidCell;
    /* 0164 */ CVidPalette m_palette;
    /* 0188 */ WORD field_188;
    /* 018A */ BYTE m_nTicksLeft;
    /* 018B */ BYTE m_nMode;
    /* 018C */ LONG m_nHalfWidth;
    /* 0190 */ LONG m_nHalfHeight;
    /* 0194 */ BYTE* m_pGrid;
    /* 0198 */ LONG field_198;
    /* 019C */ LONG field_19C;
    /* 01A0 */ CELL* m_pCells;
    /* 01A4 */ BYTE* m_pCellStates;
    /* 01A8 */ CSound m_sound;
    /* 020C */ CString m_sSoundName;
};

#endif /* CGAMECHUNK_H_ */
