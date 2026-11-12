#include "CGameChunk.h"

#include <math.h>
#include <string.h>

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameObjectArray.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CPathSearch.h"
#include "CUtil.h"
#include "CVidMode.h"

// NOTE: Words from a table of animation ids at 0x85AA18 (0x85AA1A is 0x100,
// the id `CGameSprite::Shatter` passes for the first kind of chunk).
// 0x85AA18
static const WORD ANIMATION_ID_NONE = 0x0000;
// 0x85AA2E
static const WORD ANIMATION_ID_500 = 0x0500;
// 0x85AA30
static const WORD ANIMATION_ID_510 = 0x0510;
// 0x85AA32
static const WORD ANIMATION_ID_600 = 0x0600;
// 0x85AA34
static const WORD ANIMATION_ID_610 = 0x0610;
// 0x85AA36
static const WORD ANIMATION_ID_700 = 0x0700;
// 0x85AA38
static const WORD ANIMATION_ID_710 = 0x0710;
// 0x85AA3A
static const WORD ANIMATION_ID_800 = 0x0800;
// 0x85AA3C
static const WORD ANIMATION_ID_810 = 0x0810;

// 0x70F4F0
CGameChunk::CGameChunk(USHORT animationID, BYTE* pColors, const CString& sSoundName, CGameArea* pArea, const CPoint& pos, const CPoint& a6, LONG nPosZ, LONG a8, BYTE a9, BYTE a10)
{
    m_animation.SetAnimationType(animationID, pColors, 0);

    // NOTE: Inlined.
    m_sound.SetResRef(CResRef(sSoundName), TRUE, TRUE);
    m_sound.SetChannel(14, reinterpret_cast<DWORD>(pArea));

    field_88 = 1;
    field_8C = a6;
    field_94 = a8;
    field_99 = a9;
    field_9A = a10;

    // NOTE: Original copies the table from 0x8A8154, a second copy of the
    // visible terrain table (not the default one).
    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, 16);

    field_98 = 3;

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) != CGameObjectArray::SUCCESS) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    CGameObject::AddToArea(pArea, pos, nPosZ, CGameObject::LIST_FRONT);
}

// 0x70F770
// NOTE: 0x70F750 is the scalar deleting destructor (compiler-generated).
CGameChunk::~CGameChunk()
{
    // NOTE: The members and the base class are destroyed implicitly.
}

// 0x70F800
void CGameChunk::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    BOOL bPlaySound = FALSE;

    if (field_88 == 0) {
        field_99--;
        if (field_99 == 0) {
            RemoveFromArea();
        }
        return;
    }

    CPoint ptGrid(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);

    m_posZ -= field_94;
    field_94--;

    if (m_posZ >= 0) {
        m_posZ = 0;

        SHORT nTableIndex;
        m_pArea->m_search.GetLOSCost(ptGrid, m_terrainTable, nTableIndex, FALSE);

        if (nTableIndex == 12) {
            RemoveFromArea();
            return;
        }

        if (field_98 > 0) {
            field_94 = (rand() % 3 + 1) * field_98;
            bPlaySound = TRUE;
            field_98--;
        } else {
            field_88 = 0;
        }
    }

    LONG nOldHeight = m_pArea->GetHeightOffset(m_pos, CGameObject::LIST_BACK);

    m_pos.x += field_8C.x;
    m_pos.y += field_8C.y;

    LONG nNewHeight = m_pArea->GetHeightOffset(m_pos, CGameObject::LIST_BACK);

    CPoint ptNewGrid(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);

    if (ptGrid.x != ptNewGrid.x || ptGrid.y != ptNewGrid.y) {
        SHORT nTableIndex;
        if (ptNewGrid.x < m_pArea->m_search.m_GridSquareDimensions.cx
            && ptNewGrid.y < m_pArea->m_search.m_GridSquareDimensions.cy
            && m_pArea->m_search.GetLOSCost(ptNewGrid, m_terrainTable, nTableIndex, FALSE) != CPathSearch::COST_IMPASSABLE) {
            // NOTE: The heights are truncated to a SHORT.
            m_posZ += static_cast<SHORT>(nNewHeight) - static_cast<SHORT>(nOldHeight);
        } else {
            if (ptGrid.x != ptNewGrid.x) {
                m_pos.x += -field_8C.x * 2;
                field_8C.x = -field_8C.x;
            }

            if (ptGrid.y != ptNewGrid.y) {
                m_pos.y += -field_8C.y * 2;
                field_8C.y = -field_8C.y;
            }

            bPlaySound = TRUE;
        }
    }

    m_animation.IncrementFrame();

    if (bPlaySound) {
        if (!m_sound.IsSoundPlaying()) {
            m_sound.Play(m_pos.x, m_pos.y, 0, 0);
        }
    }
}

// 0x70F740
BOOLEAN CGameChunk::CanSaveGame(STRREF& strError)
{
    strError = 0x4076;
    return TRUE;
}

// 0x70FCF0
void CGameChunk::RemoveFromArea()
{
    // TODO: Incomplete.
}

// 0x70FA90
void CGameChunk::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x70FD50
CGame85BF7C::CGame85BF7C(USHORT animationID, BYTE* pColors, const CString& sSoundName, CGameArea* pArea, const CPoint& pos, LONG nPosZ, const CPoint& a7, WORD a8, BYTE a9, BYTE a10)
{
    CPoint ptZero(0, 0);
    SHORT nDirection = CGameSprite::GetDirection(ptZero, a7);
    m_animation.SetAnimationType(animationID, pColors, nDirection);

    // NOTE: Inlined.
    m_sound.SetResRef(CResRef(sSoundName), TRUE, TRUE);
    m_sound.SetChannel(14, reinterpret_cast<DWORD>(pArea));

    field_88 = 1;
    field_8C = pos;
    field_94 = a7;
    field_9C = a8;
    field_9E = a9;
    field_9F = a10;

    // NOTE: Original copies the table from 0x8A8154, a second copy of the
    // visible terrain table (not the default one).
    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, 16);

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) != CGameObjectArray::SUCCESS) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    CPoint ptArea(pos.x >> 10, pos.y >> 10);
    CGameObject::AddToArea(pArea, ptArea, nPosZ, CGameObject::LIST_FRONT);
}

// 0x710000
// NOTE: 0x70FFE0 is the scalar deleting destructor (compiler-generated).
CGame85BF7C::~CGame85BF7C()
{
    // NOTE: The members and the base class are destroyed implicitly.
}

// 0x710090
void CGame85BF7C::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x710550
void CGame85BF7C::RemoveFromArea()
{
    // TODO: Incomplete.
}

// 0x710330
void CGame85BF7C::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x7105B0
CGame85BFE8::CGame85BFE8(BYTE a1, const BYTE* pColors, CGameArea* pArea, const CPoint& pos, WORD a5, BYTE a6, BYTE a7, WORD a8)
{
    memcpy(m_colors, pColors, sizeof(m_colors));

    LONG nRadius = static_cast<SHORT>(a5) - 1;
    field_18B = a7;
    field_18A = static_cast<BYTE>(nRadius / a6 + 1);
    field_188 = a8;

    switch (a1) {
    case 0:
        // NOTE: Inlined.
        m_sound.SetResRef(CResRef("EFF_M21"), TRUE, TRUE);
        m_sound.SetChannel(14, reinterpret_cast<DWORD>(pArea));
        m_sound.Play(pos.x, pos.y, 0, 0);
        m_sSoundName = "";
        field_7E = ANIMATION_ID_NONE;
        field_80 = ANIMATION_ID_NONE;
        // NOTE: Inlined.
        m_vidCell.SetResRef(CResRef("SPFIREPI"), FALSE, TRUE, TRUE);
        m_vidCell.SequenceSet(0);
        m_vidCell.FrameSet(0);
        break;
    case 1:
    case 2:
        // NOTE: Inlined.
        m_sound.SetResRef(CResRef("EFF_M18a"), TRUE, TRUE);
        m_sound.SetChannel(14, reinterpret_cast<DWORD>(pArea));
        m_sound.Play(pos.x, pos.y, 0, 0);
        m_sSoundName = "EFF_M18c";
        field_7E = ANIMATION_ID_500;
        field_80 = ANIMATION_ID_510;
        // NOTE: Inlined.
        m_vidCell.SetResRef(CResRef("SPBOOM"), FALSE, TRUE, TRUE);
        m_vidCell.SetPalette(m_palette);
        m_palette.SetRange(0, pColors[0], g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_palette.SetRange(1, pColors[1], g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_vidCell.SequenceSet(0);
        m_vidCell.FrameSet(0);
        break;
    case 3:
        // NOTE: Inlined.
        m_sound.SetResRef(CResRef("EFF_M34"), TRUE, TRUE);
        m_sound.SetChannel(14, reinterpret_cast<DWORD>(pArea));
        m_sound.Play(pos.x, pos.y, 0, 0);
        m_sSoundName = "";
        field_7E = ANIMATION_ID_600;
        field_80 = ANIMATION_ID_610;
        // NOTE: Inlined.
        m_vidCell.SetResRef(CResRef("SPBOOM"), FALSE, TRUE, TRUE);
        m_vidCell.SetPalette(m_palette);
        m_palette.SetRange(0, pColors[0], g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_palette.SetRange(1, pColors[1], g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_vidCell.SequenceSet(0);
        m_vidCell.FrameSet(0);
        break;
    case 4:
        // NOTE: Inlined.
        m_sound.SetResRef(CResRef("EFF_M31b"), TRUE, TRUE);
        m_sound.SetChannel(14, reinterpret_cast<DWORD>(pArea));
        m_sound.Play(pos.x, pos.y, 0, 0);
        m_sSoundName = "";
        field_7E = ANIMATION_ID_700;
        field_80 = ANIMATION_ID_710;
        // NOTE: Inlined.
        m_vidCell.SetResRef(CResRef("SPBOOM"), FALSE, TRUE, TRUE);
        m_vidCell.SetPalette(m_palette);
        m_palette.SetRange(0, pColors[0], g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_vidCell.SequenceSet(0);
        m_vidCell.FrameSet(m_vidCell.GetSequenceLength(m_vidCell.m_nCurrentSequence, FALSE) - 1);
        break;
    case 5:
        // NOTE: Inlined.
        m_sound.SetResRef(CResRef("EFF_M19"), TRUE, TRUE);
        m_sound.SetChannel(14, reinterpret_cast<DWORD>(pArea));
        m_sound.Play(pos.x, pos.y, 0, 0);
        m_sSoundName = "";
        field_7E = ANIMATION_ID_800;
        field_80 = ANIMATION_ID_810;
        // NOTE: Inlined.
        m_vidCell.SetResRef(CResRef("SPBOOM"), FALSE, TRUE, TRUE);
        m_vidCell.SetPalette(m_palette);
        m_palette.SetRange(0, pColors[0], g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_vidCell.SequenceSet(0);
        m_vidCell.FrameSet(m_vidCell.GetSequenceLength(m_vidCell.m_nCurrentSequence, FALSE) - 1);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 14401
        UTIL_ASSERT(FALSE);
        break;
    }

    field_1A0 = NULL;

    LONG nCells = nRadius / CPathSearch::GRID_SQUARE_SIZEX + 1;
    field_18C = nCells;
    field_190 = nCells;

    field_194 = new BYTE[(nCells * 2 + 1) * (nCells * 2 + 1)];
    if (field_194 == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    nCells = nRadius / (CPathSearch::GRID_SQUARE_SIZEX * 2) + 1;
    field_18C = nCells;
    field_190 = nCells;

    LONG nIndex = g_pChitin->GetCurrentVideoMode()->GetEllipseArcPixelList(field_18C, field_190, field_194);
    field_198 = nIndex + 1;
    field_194[field_198 - 1] = 1;

    nIndex = g_pChitin->GetCurrentVideoMode()->GetEllipseArcPixelList(field_190, field_18C, field_194 + field_198);
    field_19C = nIndex + 1;
    field_194[field_198 + field_19C - 1] = 1;

    nCells = nRadius / CPathSearch::GRID_SQUARE_SIZEX + 1;
    field_18C = nCells;
    field_190 = nCells;

    field_1A0 = new CELL[(field_19C + field_198) * 4];
    if (field_1A0 == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    field_1A4 = new BYTE[(field_19C + field_198) * 4];
    if (field_1A4 == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    memset(field_1A4, 0, (field_198 + field_19C) * 4);

    LONG nOriginX = pos.x;
    LONG nOriginY = pos.y * 4 / 3;
    LONG nCurX = nOriginX;
    LONG nCurY = nOriginY + static_cast<SHORT>(a5);
    LONG nSpeed = a6;

    for (SHORT i = 0; i < field_198; i++) {
        LONG dx = nCurX - nOriginX;
        LONG dy = nCurY - nOriginY;
        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

        field_1A0[i].field_4 = 0;
        field_1A0[field_198 + field_19C + i].field_4 = 0;
        field_1A0[i + (field_198 + field_19C) * 2].field_4 = 0;
        field_1A0[i + (field_198 + field_19C) * 3].field_4 = 0;

        field_1A0[i].field_0 = 0;
        field_1A0[field_198 + field_19C + i].field_0 = 0;
        field_1A0[i + (field_198 + field_19C) * 2].field_0 = 0;
        field_1A0[i + (field_198 + field_19C) * 3].field_0 = 0;

        field_1A0[i].field_C = (dx * nSpeed << 10) / nDistance;
        field_1A0[field_198 + field_19C + i].field_C = field_1A0[i].field_C;

        if (field_1A0[i].field_C == 0) {
            field_1A4[i + (field_19C + field_198) * 2] = 2;
            field_1A4[i + (field_198 + field_19C) * 3] = 2;
        }

        field_1A0[i + (field_198 + field_19C) * 2].field_C = -field_1A0[i].field_C;
        field_1A0[i + (field_198 + field_19C) * 3].field_C = -field_1A0[i].field_C;

        field_1A0[i].field_8 = (dy * nSpeed << 10) / nDistance;
        field_1A0[field_198 + field_19C + i].field_8 = -field_1A0[i].field_8;
        field_1A0[i + (field_198 + field_19C) * 2].field_8 = field_1A0[i].field_8;
        field_1A0[i + (field_198 + field_19C) * 3].field_8 = -field_1A0[i].field_8;

        nCurX += CPathSearch::GRID_SQUARE_SIZEX * 2;
        nCurY -= field_194[i] * CPathSearch::GRID_SQUARE_SIZEY * 2;
    }

    nCurX = static_cast<SHORT>(a5) + nOriginX;
    nCurY = nOriginY;

    for (SHORT i = 0; i < field_19C; i++) {
        LONG dx = nCurX - nOriginX;
        LONG dy = nCurY - nOriginY;
        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

        field_1A0[field_198 + i].field_4 = 0;
        field_1A0[field_198 * 2 + field_19C + i].field_4 = 0;
        field_1A0[i + (field_198 + field_19C) * 2 + field_198].field_4 = 0;
        field_1A0[i + (field_198 + field_19C) * 3 + field_198].field_4 = 0;

        field_1A0[field_198 + i].field_0 = 0;
        field_1A0[field_198 * 2 + field_19C + i].field_0 = 0;
        field_1A0[i + (field_198 + field_19C) * 2 + field_198].field_0 = 0;
        field_1A0[i + (field_198 + field_19C) * 3 + field_198].field_0 = 0;

        field_1A0[field_198 + i].field_C = (dx * nSpeed << 10) / nDistance;
        field_1A0[field_198 * 2 + field_19C + i].field_C = field_1A0[field_198 + i].field_C;
        field_1A0[i + (field_198 + field_19C) * 2 + field_198].field_C = -field_1A0[field_198 + i].field_C;
        field_1A0[i + (field_198 + field_19C) * 3 + field_198].field_C = -field_1A0[field_198 + i].field_C;

        field_1A0[field_198 + i].field_8 = (dy * nSpeed << 10) / nDistance;
        field_1A0[field_198 * 2 + field_19C + i].field_8 = -field_1A0[field_198 + i].field_8;
        field_1A0[i + (field_198 + field_19C) * 2 + field_198].field_8 = field_1A0[field_198 + i].field_8;
        field_1A0[i + (field_198 + field_19C) * 3 + field_198].field_8 = -field_1A0[field_198 + i].field_8;

        // NOTE: Tests the entry at `i`, not at `field_198 + i`.
        if (field_1A0[i].field_8 == 0) {
            field_1A4[field_198 * 2 + field_19C + i] = 2;
            field_1A4[i + (field_198 + field_19C) * 3 + field_198] = 2;
        }

        nCurX -= field_194[field_198 + i] * CPathSearch::GRID_SQUARE_SIZEX * 2;
        nCurY += CPathSearch::GRID_SQUARE_SIZEY * 2;
    }

    // NOTE: Original copies the table from 0x8A8154, a second copy of the
    // visible terrain table (not the default one).
    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, 16);

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) != CGameObjectArray::SUCCESS) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    CGameObject::AddToArea(pArea, pos, 0, CGameObject::LIST_FRONT);

    if (field_7E != ANIMATION_ID_NONE) {
        CPoint ptPos(m_pos.x << 10, m_pos.y << 10);
        CPoint ptZero(0, 0);
        new CGame85BF7C(field_80,
            m_colors,
            m_sSoundName,
            m_pArea,
            ptPos,
            0,
            ptZero,
            static_cast<WORD>(field_18A * 2 + field_188),
            0,
            field_18B);
    }
}

// 0x711CB0
// NOTE: 0x711C90 is the scalar deleting destructor (compiler-generated).
CGame85BFE8::~CGame85BFE8()
{
    if (field_194 != NULL) {
        delete[] field_194;
    }

    if (field_1A0 != NULL) {
        delete[] field_1A0;
    }

    if (field_1A4 != NULL) {
        delete[] field_1A4;
    }
}

// 0x711D90
void CGame85BFE8::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x712310
void CGame85BFE8::RemoveFromArea()
{
    // TODO: Incomplete.
}

// 0x7122E0
void CGame85BFE8::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}
