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

    m_bFlying = 1;
    m_ptVelocity = a6;
    m_nVelocityZ = a8;
    m_nTicksLeft = a9;
    m_nFadeTicks = a10;

    // NOTE: Original copies the table from 0x8A8154, a second copy of the
    // visible terrain table (not the default one).
    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, 16);

    m_nBounces = 3;

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

    if (m_bFlying == 0) {
        m_nTicksLeft--;
        if (m_nTicksLeft == 0) {
            RemoveFromArea();
        }
        return;
    }

    CPoint ptGrid(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);

    m_posZ -= m_nVelocityZ;
    m_nVelocityZ--;

    if (m_posZ >= 0) {
        m_posZ = 0;

        SHORT nTableIndex;
        m_pArea->m_search.GetLOSCost(ptGrid, m_terrainTable, nTableIndex, FALSE);

        if (nTableIndex == 12) {
            RemoveFromArea();
            return;
        }

        if (m_nBounces > 0) {
            m_nVelocityZ = (rand() % 3 + 1) * m_nBounces;
            bPlaySound = TRUE;
            m_nBounces--;
        } else {
            m_bFlying = 0;
        }
    }

    LONG nOldHeight = m_pArea->GetHeightOffset(m_pos, CGameObject::LIST_BACK);

    m_pos.x += m_ptVelocity.x;
    m_pos.y += m_ptVelocity.y;

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
                m_pos.x += -m_ptVelocity.x * 2;
                m_ptVelocity.x = -m_ptVelocity.x;
            }

            if (ptGrid.y != ptNewGrid.y) {
                m_pos.y += -m_ptVelocity.y * 2;
                m_ptVelocity.y = -m_ptVelocity.y;
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
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 13954
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x70FA90
void CGameChunk::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 13882
    UTIL_ASSERT(pVidMode != NULL);

    CInfinity* pInfinity = &m_pArea->m_cInfinity;

    BYTE bDithered = FALSE;
    BYTE bFadeOut = FALSE;
    if (m_nTicksLeft < m_nFadeTicks) {
        bFadeOut = TRUE;
    }

    // NOTE: Uninline.
    if (!m_pArea->m_visibility.IsTileVisible(m_pArea->m_visibility.PointToTile(m_pos))) {
        return;
    }

    CRect rView(pInfinity->nCurrentX,
        pInfinity->nCurrentY,
        pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX,
        pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY);

    CRect rFx;
    CPoint ptReference;
    m_animation.CalculateFxRect(rFx, ptReference, m_posZ);

    CPoint pos = m_pos;
    pos.y += m_pArea->GetHeightOffset(m_pos, CGameObject::LIST_BACK);

    CRect rGCBounds;
    m_animation.CalculateGCBoundsRect(rGCBounds,
        pos,
        ptReference,
        m_posZ,
        rFx.right - rFx.left,
        rFx.bottom - rFx.top);

    if (!IntersectRect(&rView, &rGCBounds, &rView)) {
        return;
    }

    CPoint ptTint(pos.x, pos.y + m_posZ);
    COLORREF rgbTint = m_pArea->GetTintColor(ptTint, m_listType);

    if (m_id == m_pArea->m_iPicked
        || g_pBaldurChitin->GetObjectGame()->m_cOptions.m_bAlwaysDither
        || g_pBaldurChitin->GetObjectGame()->m_bForceDither) {
        bDithered = TRUE;
    }

    m_animation.Render(pInfinity,
        pVidMode,
        a3,
        rFx,
        pos,
        ptReference,
        0x30000,
        rgbTint,
        rGCBounds,
        bDithered,
        bFadeOut,
        m_posZ,
        0);
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

    m_bVisible = 1;
    m_ptFixedPos = pos;
    m_ptVelocity = a7;
    m_nTicksLeft = a8;
    m_nFadeTicks = a9;
    m_nMode = a10;

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
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (m_nTicksLeft == 0) {
        if (m_animation.IsEndOfSequence()) {
            RemoveFromArea();
            return;
        }
    } else {
        m_nTicksLeft--;
        if (m_nTicksLeft == 0) {
            RemoveFromArea();
            return;
        }
    }

    CPoint ptGrid(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);

    m_ptFixedPos.x += m_ptVelocity.x;
    m_ptFixedPos.y += m_ptVelocity.y;
    m_pos.x = m_ptFixedPos.x >> 10;
    m_pos.y = m_ptFixedPos.y >> 10;

    CPoint ptNewGrid(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);

    if (ptGrid.x != ptNewGrid.x || ptGrid.y != ptNewGrid.y) {
        SHORT nTableIndex;
        if (m_pArea->m_search.GetLOSCost(ptNewGrid, m_terrainTable, nTableIndex, FALSE) == CPathSearch::COST_IMPASSABLE) {
            if (m_nMode == 1) {
                RemoveFromArea();
                return;
            }

            if (m_nMode == 0) {
                CPoint ptOld = m_ptFixedPos;

                if (ptGrid.x != ptNewGrid.x) {
                    m_ptFixedPos.x += -m_ptVelocity.x * 2;
                    m_pos.x = m_ptFixedPos.x >> 10;
                    m_ptVelocity.x = -m_ptVelocity.x;
                }

                if (ptGrid.y != ptNewGrid.y) {
                    m_ptFixedPos.y += -m_ptVelocity.y * 2;
                    m_pos.y = m_ptFixedPos.y >> 10;
                    m_ptVelocity.y = -m_ptVelocity.y;
                }

                SHORT nDirection = CGameSprite::GetDirection(ptOld, m_ptFixedPos);
                m_animation.ChangeDirection(nDirection);
            } else {
                m_bVisible = 0;
            }
        } else {
            m_bVisible = 1;
        }
    }

    m_animation.IncrementFrame();

    if (rand() % 200 == 0) {
        if (!m_sound.IsSoundPlaying()) {
            m_sound.Play(m_pos.x, m_pos.y, 0, 0);
        }
    }
}

// 0x710550
void CGame85BF7C::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 14270
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x710330
void CGame85BF7C::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 14202
    UTIL_ASSERT(pVidMode != NULL);

    BYTE bFadeOut = FALSE;
    CInfinity* pInfinity = &m_pArea->m_cInfinity;

    if (m_bVisible == 0) {
        return;
    }

    // NOTE: Uninline.
    if (!m_pArea->m_visibility.IsTileVisible(m_pArea->m_visibility.PointToTile(m_pos))) {
        return;
    }

    if (static_cast<SHORT>(m_nTicksLeft) < m_nFadeTicks) {
        bFadeOut = TRUE;
    }

    CRect rView(pInfinity->nCurrentX,
        pInfinity->nCurrentY,
        pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX,
        pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY);

    CRect rFx;
    CPoint ptReference;
    m_animation.CalculateFxRect(rFx, ptReference, m_posZ);

    CPoint pos = m_pos;
    pos.y += m_pArea->GetHeightOffset(m_pos, CGameObject::LIST_BACK);

    CRect rGCBounds;
    m_animation.CalculateGCBoundsRect(rGCBounds,
        pos,
        ptReference,
        m_posZ,
        rFx.right - rFx.left,
        rFx.bottom - rFx.top);

    if (!IntersectRect(&rView, &rGCBounds, &rView)) {
        return;
    }

    CPoint ptTint(pos.x, pos.y + m_posZ);
    COLORREF rgbTint = m_pArea->GetTintColor(ptTint, m_listType);

    m_animation.Render(pInfinity,
        pVidMode,
        a3,
        rFx,
        pos,
        ptReference,
        0x30000,
        rgbTint,
        rGCBounds,
        FALSE,
        bFadeOut,
        m_posZ,
        0);
}

// 0x7105B0
CGame85BFE8::CGame85BFE8(BYTE a1, const BYTE* pColors, CGameArea* pArea, const CPoint& pos, WORD a5, BYTE a6, BYTE a7, WORD a8)
{
    memcpy(m_colors, pColors, sizeof(m_colors));

    LONG nRadius = static_cast<SHORT>(a5) - 1;
    m_nMode = a7;
    m_nTicksLeft = static_cast<BYTE>(nRadius / a6 + 1);
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

    m_pCells = NULL;

    LONG nCells = nRadius / CPathSearch::GRID_SQUARE_SIZEX + 1;
    m_nHalfWidth = nCells;
    m_nHalfHeight = nCells;

    m_pGrid = new BYTE[(nCells * 2 + 1) * (nCells * 2 + 1)];
    if (m_pGrid == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    nCells = nRadius / (CPathSearch::GRID_SQUARE_SIZEX * 2) + 1;
    m_nHalfWidth = nCells;
    m_nHalfHeight = nCells;

    LONG nIndex = g_pChitin->GetCurrentVideoMode()->GetEllipseArcPixelList(m_nHalfWidth, m_nHalfHeight, m_pGrid);
    field_198 = nIndex + 1;
    m_pGrid[field_198 - 1] = 1;

    nIndex = g_pChitin->GetCurrentVideoMode()->GetEllipseArcPixelList(m_nHalfHeight, m_nHalfWidth, m_pGrid + field_198);
    field_19C = nIndex + 1;
    m_pGrid[field_198 + field_19C - 1] = 1;

    nCells = nRadius / CPathSearch::GRID_SQUARE_SIZEX + 1;
    m_nHalfWidth = nCells;
    m_nHalfHeight = nCells;

    m_pCells = new CELL[(field_19C + field_198) * 4];
    if (m_pCells == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    m_pCellStates = new BYTE[(field_19C + field_198) * 4];
    if (m_pCellStates == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    memset(m_pCellStates, 0, (field_198 + field_19C) * 4);

    LONG nOriginX = pos.x;
    LONG nOriginY = pos.y * 4 / 3;
    LONG nCurX = nOriginX;
    LONG nCurY = nOriginY + static_cast<SHORT>(a5);
    LONG nSpeed = a6;

    for (SHORT i = 0; i < field_198; i++) {
        LONG dx = nCurX - nOriginX;
        LONG dy = nCurY - nOriginY;
        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

        m_pCells[i].nOffsetX = 0;
        m_pCells[field_198 + field_19C + i].nOffsetX = 0;
        m_pCells[i + (field_198 + field_19C) * 2].nOffsetX = 0;
        m_pCells[i + (field_198 + field_19C) * 3].nOffsetX = 0;

        m_pCells[i].nOffsetY = 0;
        m_pCells[field_198 + field_19C + i].nOffsetY = 0;
        m_pCells[i + (field_198 + field_19C) * 2].nOffsetY = 0;
        m_pCells[i + (field_198 + field_19C) * 3].nOffsetY = 0;

        m_pCells[i].nVelocityX = (dx * nSpeed << 10) / nDistance;
        m_pCells[field_198 + field_19C + i].nVelocityX = m_pCells[i].nVelocityX;

        if (m_pCells[i].nVelocityX == 0) {
            m_pCellStates[i + (field_19C + field_198) * 2] = 2;
            m_pCellStates[i + (field_198 + field_19C) * 3] = 2;
        }

        m_pCells[i + (field_198 + field_19C) * 2].nVelocityX = -m_pCells[i].nVelocityX;
        m_pCells[i + (field_198 + field_19C) * 3].nVelocityX = -m_pCells[i].nVelocityX;

        m_pCells[i].nVelocityY = (dy * nSpeed << 10) / nDistance;
        m_pCells[field_198 + field_19C + i].nVelocityY = -m_pCells[i].nVelocityY;
        m_pCells[i + (field_198 + field_19C) * 2].nVelocityY = m_pCells[i].nVelocityY;
        m_pCells[i + (field_198 + field_19C) * 3].nVelocityY = -m_pCells[i].nVelocityY;

        nCurX += CPathSearch::GRID_SQUARE_SIZEX * 2;
        nCurY -= m_pGrid[i] * CPathSearch::GRID_SQUARE_SIZEY * 2;
    }

    nCurX = static_cast<SHORT>(a5) + nOriginX;
    nCurY = nOriginY;

    for (SHORT i = 0; i < field_19C; i++) {
        LONG dx = nCurX - nOriginX;
        LONG dy = nCurY - nOriginY;
        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

        m_pCells[field_198 + i].nOffsetX = 0;
        m_pCells[field_198 * 2 + field_19C + i].nOffsetX = 0;
        m_pCells[i + (field_198 + field_19C) * 2 + field_198].nOffsetX = 0;
        m_pCells[i + (field_198 + field_19C) * 3 + field_198].nOffsetX = 0;

        m_pCells[field_198 + i].nOffsetY = 0;
        m_pCells[field_198 * 2 + field_19C + i].nOffsetY = 0;
        m_pCells[i + (field_198 + field_19C) * 2 + field_198].nOffsetY = 0;
        m_pCells[i + (field_198 + field_19C) * 3 + field_198].nOffsetY = 0;

        m_pCells[field_198 + i].nVelocityX = (dx * nSpeed << 10) / nDistance;
        m_pCells[field_198 * 2 + field_19C + i].nVelocityX = m_pCells[field_198 + i].nVelocityX;
        m_pCells[i + (field_198 + field_19C) * 2 + field_198].nVelocityX = -m_pCells[field_198 + i].nVelocityX;
        m_pCells[i + (field_198 + field_19C) * 3 + field_198].nVelocityX = -m_pCells[field_198 + i].nVelocityX;

        m_pCells[field_198 + i].nVelocityY = (dy * nSpeed << 10) / nDistance;
        m_pCells[field_198 * 2 + field_19C + i].nVelocityY = -m_pCells[field_198 + i].nVelocityY;
        m_pCells[i + (field_198 + field_19C) * 2 + field_198].nVelocityY = m_pCells[field_198 + i].nVelocityY;
        m_pCells[i + (field_198 + field_19C) * 3 + field_198].nVelocityY = -m_pCells[field_198 + i].nVelocityY;

        // NOTE: Tests the entry at `i`, not at `field_198 + i`.
        if (m_pCells[i].nVelocityY == 0) {
            m_pCellStates[field_198 * 2 + field_19C + i] = 2;
            m_pCellStates[i + (field_198 + field_19C) * 3 + field_198] = 2;
        }

        nCurX -= m_pGrid[field_198 + i] * CPathSearch::GRID_SQUARE_SIZEX * 2;
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
            static_cast<WORD>(m_nTicksLeft * 2 + field_188),
            0,
            m_nMode);
    }
}

// 0x711CB0
// NOTE: 0x711C90 is the scalar deleting destructor (compiler-generated).
CGame85BFE8::~CGame85BFE8()
{
    if (m_pGrid != NULL) {
        delete[] m_pGrid;
    }

    if (m_pCells != NULL) {
        delete[] m_pCells;
    }

    if (m_pCellStates != NULL) {
        delete[] m_pCellStates;
    }
}

// 0x711D90
void CGame85BFE8::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    BOOL bDone = TRUE;

    m_nTicksLeft--;
    if (m_nTicksLeft == 0) {
        if (m_vidCell.IsEndOfSequence(FALSE)) {
            RemoveFromArea();
            return;
        }

        m_nTicksLeft++;
        m_vidCell.FrameAdvance();
        return;
    }

    memset(m_pGrid, 0, (m_nHalfHeight * 2 + 1) * (m_nHalfWidth * 2 + 1));

    for (SHORT i = 0; i < (field_198 + field_19C) * 4; i++) {
        if (m_pCellStates[i] == 2) {
            continue;
        }

        m_pCells[i].nOffsetX += m_pCells[i].nVelocityX;
        m_pCells[i].nOffsetY += m_pCells[i].nVelocityY;

        CPoint ptCell(((m_pCells[i].nOffsetX >> 10) + m_pos.x) / CPathSearch::GRID_SQUARE_SIZEX,
            (((m_pCells[i].nOffsetY * 3 / 4) >> 10) + m_pos.y) / CPathSearch::GRID_SQUARE_SIZEY);

        SHORT nTableIndex;
        if (m_pArea->m_search.GetLOSCost(ptCell, m_terrainTable, nTableIndex, FALSE) == CPathSearch::COST_IMPASSABLE) {
            if (m_nMode == 1) {
                m_pCellStates[i] = 2;
            }

            if (m_nMode != 0) {
                if (m_pCellStates[i] == 0) {
                    bDone = FALSE;
                }

                continue;
            }

            CPoint ptPrevCell(((((m_pCells[i].nOffsetX - m_pCells[i].nVelocityX) >> 10) + m_pos.x)) / CPathSearch::GRID_SQUARE_SIZEX,
                ((((m_pCells[i].nOffsetY - m_pCells[i].nVelocityY) * 3 / 4) >> 10) + m_pos.y) / CPathSearch::GRID_SQUARE_SIZEY);

            if (ptPrevCell.x != ptCell.x) {
                m_pCells[i].nOffsetX -= m_pCells[i].nVelocityX * 2;
                m_pCells[i].nVelocityX = -m_pCells[i].nVelocityX;
            }

            if (ptPrevCell.y != ptCell.y) {
                m_pCells[i].nOffsetY += -m_pCells[i].nVelocityY * 2;
                m_pCells[i].nVelocityY = -m_pCells[i].nVelocityY;
            }
        }

        LONG nRow = (m_pCells[i].nOffsetY / CPathSearch::GRID_SQUARE_SIZEX >> 10) + m_nHalfHeight;
        LONG nIndex = (m_pCells[i].nOffsetX / CPathSearch::GRID_SQUARE_SIZEX >> 10) + m_nHalfWidth + nRow * (m_nHalfWidth * 2 + 1);

        if (field_7E != ANIMATION_ID_NONE) {
            DWORD nCube = m_nTicksLeft * m_nTicksLeft * m_nTicksLeft;
            DWORD nChance = ((nCube / 1000 + 10) * 3) >> 1;
            if (nChance == 0 || rand() % static_cast<LONG>(nChance) == 0) {
                CPoint ptNew(m_pCells[i].nOffsetX + (m_pos.x << 10),
                    m_pCells[i].nOffsetY * 3 / 4 + (m_pos.y << 10));
                CPoint ptZero(0, 0);

                new CGame85BF7C(field_80,
                    m_colors,
                    m_sSoundName,
                    m_pArea,
                    ptNew,
                    0,
                    ptZero,
                    static_cast<WORD>(m_nTicksLeft * 2 + field_188),
                    0,
                    m_nMode);
            }
        }

        BYTE nOld = m_pGrid[nIndex]++;
        if (nOld == 0) {
            if (m_pCellStates[i] == 0) {
                CPoint ptNew(m_pCells[i].nOffsetX + (m_pos.x << 10),
                    m_pCells[i].nOffsetY * 3 / 4 + (m_pos.y << 10));
                CPoint ptVelocity(m_pCells[i].nVelocityX, m_pCells[i].nVelocityY * 3 / 4);

                new CGame85BF7C(field_7E,
                    m_colors,
                    m_sSoundName,
                    m_pArea,
                    ptNew,
                    0,
                    ptVelocity,
                    m_nTicksLeft,
                    0,
                    m_nMode);

                m_pCellStates[i] = 1;
            }
        } else if (m_pCellStates[i] == 0) {
            bDone = FALSE;
        }
    }

    if (bDone) {
        if (m_vidCell.IsEndOfSequence(FALSE)) {
            RemoveFromArea();
            return;
        }
    }

    m_vidCell.FrameAdvance();
}

// 0x712310
void CGame85BFE8::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 14752
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x7122E0
void CGame85BFE8::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 14661
    UTIL_ASSERT(pVidMode != NULL);
}
