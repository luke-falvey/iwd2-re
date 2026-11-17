#include "CVisibility.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameObjectArray.h"
#include "CInfGame.h"
#include "CPathSearch.h"
#include "CSearchBitmap.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVidMode.h"

// Maps a search map row (12 pixels) inside a 96-pixel band to a visibility
// map row (32 pixels) inside the same band.
//
// 0x84EDC4
const BYTE CVisibilityMap::SEARCH_TO_VIS_Y[8] = { 0, 0, 1, 1, 1, 2, 2, 2 };

// Maps a visibility map row inside a 96-pixel band to a search map row.
//
// 0x84EDCC
const BYTE CVisibilityMap::VIS_TO_SEARCH_Y[3] = { 0, 2, 5 };

// 0x84EDCF
const BYTE CVisibilityMap::EXPLORED_RANGE_0 = 4;

// 0x84EDD0
const BYTE CVisibilityMap::EXPLORED_RANGE_1 = 8;

// 0x84EDD1
const BYTE CVisibilityMap::EXPLORED_RANGE_2 = 12;

// 0x84EDD2
const BYTE CVisibilityMap::EXPLORED_RANGE_3 = 14;

// 0x84EDD3
const BYTE CVisibilityMap::EXPLORED_RANGE_4 = 16;

// 0x84EDD4
const BYTE CVisibilityMap::EXPLORED_RANGE_5 = 20;

// 0x84EDD5
const BYTE CVisibilityMap::EXPLORED_RANGE_6 = 24;

// 0x84EDD6
const BYTE CVisibilityMap::SQUARE_SIZEX = 32;

// 0x84EDD7
const BYTE CVisibilityMap::SQUARE_SIZEY = 32;

// 0x84EDDB
const BYTE CVisibilityMap::EXPLORED = 0;

// 0x84EDD8
const BYTE CVisibilityMap::EXPLORED_FLAG = 0x80;

// 0x84EDD9
const BYTE CVisibilityMap::VISIBILITY_MASK = 0x3F;

// 0x84EDDA
const BYTE CVisibilityMap::UNEXPLORED = 0xF;

// 0x84EDDC
const TILE_CODE CVisibilityMap::EXPLORED_FULL = { 0, 0, 0, 0 };

// 0x84EDE0
const BYTE CVisibilityMap::UNEXPLORED_N = 9;

// 0x84EDE1
const BYTE CVisibilityMap::UNEXPLORED_NE = 13;

// 0x84EDE2
const BYTE CVisibilityMap::UNEXPLORED_E = 12;

// 0x84EDE3
const BYTE CVisibilityMap::UNEXPLORED_SE = 14;

// 0x84EDE4
const BYTE CVisibilityMap::UNEXPLORED_S = 6;

// 0x84EDE5
const BYTE CVisibilityMap::UNEXPLORED_SW = 7;

// 0x84EDE6
const BYTE CVisibilityMap::UNEXPLORED_W = 3;

// 0x84EDE7
const BYTE CVisibilityMap::UNEXPLORED_NW = 11;

// 0x84EDE8
const TILE_CODE CVisibilityMap::UNEXPLORED_FULL = { 0xF, 0xF, 0xF, 0xF };

// 0x84EDEC
const BYTE CVisibilityMap::FOGOWAR_EXPLORED_MASK = 0xF;

// 0x84EDED
const BYTE CVisibilityMap::FOGOWAR_EXPLORED_SHIFT = 0;

// 0x84EDEE
const BYTE CVisibilityMap::FOGOWAR_MASK = 0xF0;

// 0x84EDEF
const BYTE CVisibilityMap::FOGOWAR_SHIFT = 4;

// 0x8AE818
POLYCOLOR CVisibilityMap::FOGOWARSHADE = { 0.0f, 0.0f, 0.0f, 0.5f };

// 0x8AE828
POLYCOLOR CVisibilityMap::EXPLORESHADE = { 0.0f, 0.0f, 0.0f, 1.0f };

// 0x8E1810
POLYCOLOR CVisibilityMap::CLEARSHADE;

// 0x551770
CVisibilityMap::CVisibilityMap()
{
    field_4 = 0;
    m_nWidth = 0;
    m_nHeight = 0;
    field_1A = 0;
    memset(m_aCharacterIds, 0, sizeof(m_aCharacterIds));
    m_pMap = NULL;
    m_pSearchMap = NULL;
    field_12 = 0;
    field_16 = 0;
    field_E = 14;
    field_10 = 10;
    sub_5518A0();
    m_bOutDoor = FALSE;
}

// 0x551800
CVisibilityMap::~CVisibilityMap()
{
    if (field_12) {
        delete field_12;
    }

    if (field_16) {
        delete field_16;
    }
}

// 0x551840
BOOL CVisibilityMap::AddCharacter(const CPoint& pos, LONG charId, const BYTE* pVisibleTerrainTable)
{
    // NOTE: Differs from the original on purpose. The exe takes the first
    // free slot even when the id already holds a later one, and gives up
    // silently when all six are taken, so a party member could be left
    // without a slot and never clear the fog of war around them. Here the
    // id's own slot is reused, then a free one, then one whose id is no
    // longer a party member.
    SHORT nIndex;
    for (nIndex = 0; nIndex < 6; nIndex++) {
        if (m_aCharacterIds[nIndex] == charId) {
            break;
        }
    }

    if (nIndex == 6) {
        for (nIndex = 0; nIndex < 6; nIndex++) {
            if (m_aCharacterIds[nIndex] == CGameObjectArray::INVALID_INDEX) {
                break;
            }
        }
    }

    if (nIndex == 6) {
        for (nIndex = 0; nIndex < 6; nIndex++) {
            if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_aCharacterIds[nIndex]) == -1) {
                break;
            }
        }
    }

    if (nIndex == 6) {
        return FALSE;
    }

    m_aCharacterIds[nIndex] = charId;

    PrivateAddCharacter(pos, 1 << nIndex, pVisibleTerrainTable);

    return TRUE;
}

// 0x5518A0
void CVisibilityMap::sub_5518A0()
{
    if (field_12 != NULL) {
        delete field_12;
    }

    if (field_16 != NULL) {
        delete field_16;
    }

    field_12 = new BYTE[field_E];
    field_16 = new BYTE[field_10];

    field_4 = static_cast<BYTE>(g_pBaldurChitin->GetCurrentVideoMode()->GetEllipseArcPixelList(field_E,
        field_10,
        field_12));

    field_1A = static_cast<BYTE>(g_pBaldurChitin->GetCurrentVideoMode()->GetEllipseArcPixelList(field_10,
        field_E,
        field_16));
}

// 0x551950
void CVisibilityMap::ClimbWall(const CPoint& ptStart, const CPoint& ptEnd, BYTE charId, const BYTE* pVisibileTerrainTable, SHORT nHighest)
{
    LONG nTopYValue = max(ptEnd.y, 0);
    LONG nCurrTile = ptStart.x + m_nWidth * ptStart.y;

    switch (nHighest) {
    case 3:
        if (ptStart.y - 2 >= nTopYValue) {
            // NOTE: Uninline.
            SetTileVisible(nCurrTile - 2 * m_nWidth, charId);
        }
        // FALLTHROUGH
    case 2:
        if (ptStart.y - 1 >= nTopYValue) {
            // NOTE: Uninline.
            SetTileVisible(nCurrTile - m_nWidth, charId);
        }
        // FALLTHROUGH
    case 1:
        if (ptStart.y >= nTopYValue) {
            // NOTE: Uninline.
            SetTileVisible(nCurrTile, charId);
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
        // __LINE__: 333
        UTIL_ASSERT(FALSE);
    }
}

// NOTE: The fog-of-war tile rect types passed to `GetFogOWarTileRect` are
// read from byte constants at 0x85DE43 (0) through 0x85DE4A (7). Types 0-3
// address the 32x32 quarters of `CVIDINF_SURFACE_6`, types 4-7 the quarters
// of `CVIDINF_SURFACE_7`.

// 0x551A90
void CVisibilityMap::BltDitherPattern(LPDIRECTDRAWSURFACE pSurface, const TILE_CODE& tileCode)
{
    CVidInf* pVidInf = static_cast<CVidInf*>(g_pBaldurChitin->GetCurrentVideoMode());
    BYTE aTileCodes[4];
    BYTE aPositions[4][2];
    CRect rSrc;
    CRect rDest;
    DDBLTFX fx;
    HRESULT hr;
    BYTE nFogWarCode;
    BYTE nExploredCode;
    INT x;
    INT y;

    aPositions[0][0] = 0;
    aPositions[0][1] = 0;
    aPositions[1][0] = 32;
    aPositions[1][1] = 0;
    aPositions[2][0] = 0;
    aPositions[2][1] = 32;
    aPositions[3][0] = 32;
    aPositions[3][1] = 32;

    aTileCodes[0] = tileCode.tileNW;
    aTileCodes[1] = tileCode.tileNE;
    aTileCodes[2] = tileCode.tileSW;
    aTileCodes[3] = tileCode.tileSE;

    fx.dwSize = sizeof(fx);

    for (int index = 0; index < 4; index++) {
        nExploredCode = (aTileCodes[index] & FOGOWAR_EXPLORED_MASK) >> FOGOWAR_EXPLORED_SHIFT;
        nFogWarCode = (aTileCodes[index] & FOGOWAR_MASK) >> FOGOWAR_SHIFT;
        x = aPositions[index][0];
        y = aPositions[index][1];

        switch (nFogWarCode) {
        case 3:
            if (nExploredCode != 3) {
                pVidInf->GetFogOWarTileRect(5, rSrc);
                do {
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.BltFast(pSurface,
                        x,
                        y,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_7],
                        &rSrc,
                        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        case 6:
            if (nExploredCode != 6) {
                pVidInf->GetFogOWarTileRect(4, rSrc);
                fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;
                do {
                    rDest.SetRect(x, y, x + 32, y + 32);
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                        &rDest,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_7],
                        &rSrc,
                        DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                        &fx);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        case 7:
            if (nExploredCode != 7) {
                pVidInf->GetFogOWarTileRect(7, rSrc);
                fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;
                do {
                    rDest.SetRect(x, y, x + 32, y + 32);
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                        &rDest,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_7],
                        &rSrc,
                        DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                        &fx);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        case 9:
            if (nExploredCode != 9) {
                pVidInf->GetFogOWarTileRect(4, rSrc);
                do {
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.BltFast(pSurface,
                        x,
                        y,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_7],
                        &rSrc,
                        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        case 11:
            if (nExploredCode != 11) {
                pVidInf->GetFogOWarTileRect(6, rSrc);
                do {
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.BltFast(pSurface,
                        x,
                        y,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_7],
                        &rSrc,
                        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        case 12:
            if (nExploredCode != 12) {
                pVidInf->GetFogOWarTileRect(5, rSrc);
                fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;
                do {
                    rDest.SetRect(x, y, x + 32, y + 32);
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                        &rDest,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_7],
                        &rSrc,
                        DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                        &fx);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        case 13:
            if (nExploredCode != 13) {
                pVidInf->GetFogOWarTileRect(7, rSrc);
                do {
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.BltFast(pSurface,
                        x,
                        y,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_7],
                        &rSrc,
                        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        case 14:
            if (nExploredCode != 14) {
                pVidInf->GetFogOWarTileRect(6, rSrc);
                fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;
                do {
                    rDest.SetRect(x, y, x + 32, y + 32);
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                        &rDest,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_7],
                        &rSrc,
                        DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                        &fx);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        case 15:
            if (nExploredCode != 15) {
                pVidInf->GetFogOWarTileRect(3, rSrc);
                do {
                    hr = g_pBaldurChitin->cVideo.cVidBlitter.BltFast(pSurface,
                        x,
                        y,
                        pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                        &rSrc,
                        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
                } while (!pVidInf->CheckResults(hr));
            }
            break;
        }
    }
}

// 0x551FF0
void CVisibilityMap::BltFogOWar(LPDIRECTDRAWSURFACE pSurface, const TILE_CODE& tileCode)
{
    CVidInf* pVidInf = static_cast<CVidInf*>(g_pBaldurChitin->GetCurrentVideoMode());
    BYTE aTileCodes[4];
    BYTE aPositions[4][2];
    CRect rSrc;
    CRect rDest;
    DDBLTFX fx;
    HRESULT hr;
    INT x;
    INT y;

    aPositions[0][0] = 0;
    aPositions[0][1] = 0;
    aPositions[1][0] = 32;
    aPositions[1][1] = 0;
    aPositions[2][0] = 0;
    aPositions[2][1] = 32;
    aPositions[3][0] = 32;
    aPositions[3][1] = 32;

    aTileCodes[0] = tileCode.tileNW;
    aTileCodes[1] = tileCode.tileNE;
    aTileCodes[2] = tileCode.tileSW;
    aTileCodes[3] = tileCode.tileSE;

    BltDitherPattern(pSurface, tileCode);

    fx.dwSize = sizeof(fx);

    for (int index = 0; index < 4; index++) {
        x = aPositions[index][0];
        y = aPositions[index][1];

        switch ((aTileCodes[index] & FOGOWAR_EXPLORED_MASK) >> FOGOWAR_EXPLORED_SHIFT) {
        case 3:
            pVidInf->GetFogOWarTileRect(1, rSrc);
            do {
                hr = g_pBaldurChitin->cVideo.cVidBlitter.BltFast(pSurface,
                    x,
                    y,
                    pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                    &rSrc,
                    DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
            } while (!pVidInf->CheckResults(hr));
            break;
        case 6:
            pVidInf->GetFogOWarTileRect(0, rSrc);
            fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;
            do {
                rDest.SetRect(x, y, x + 32, y + 32);
                hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                    &rDest,
                    pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                    &rSrc,
                    DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                    &fx);
            } while (!pVidInf->CheckResults(hr));
            break;
        case 7:
            pVidInf->GetFogOWarTileRect(2, rSrc);
            fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
            do {
                rDest.SetRect(x, y, x + 32, y + 32);
                hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                    &rDest,
                    pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                    &rSrc,
                    DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                    &fx);
            } while (!pVidInf->CheckResults(hr));
            break;
        case 9:
            pVidInf->GetFogOWarTileRect(0, rSrc);
            do {
                hr = g_pBaldurChitin->cVideo.cVidBlitter.BltFast(pSurface,
                    x,
                    y,
                    pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                    &rSrc,
                    DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
            } while (!pVidInf->CheckResults(hr));
            break;
        case 11:
            pVidInf->GetFogOWarTileRect(2, rSrc);
            do {
                hr = g_pBaldurChitin->cVideo.cVidBlitter.BltFast(pSurface,
                    x,
                    y,
                    pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                    &rSrc,
                    DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
            } while (!pVidInf->CheckResults(hr));
            break;
        case 12:
            pVidInf->GetFogOWarTileRect(1, rSrc);
            fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;
            do {
                rDest.SetRect(x, y, x + 32, y + 32);
                hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                    &rDest,
                    pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                    &rSrc,
                    DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                    &fx);
            } while (!pVidInf->CheckResults(hr));
            break;
        case 13:
            pVidInf->GetFogOWarTileRect(2, rSrc);
            fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
            do {
                rDest.SetRect(x, y, x + 32, y + 32);
                hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                    &rDest,
                    pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                    &rSrc,
                    DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                    &fx);
            } while (!pVidInf->CheckResults(hr));
            break;
        case 14:
            pVidInf->GetFogOWarTileRect(2, rSrc);
            fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;
            do {
                rDest.SetRect(x, y, x + 32, y + 32);
                hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                    &rDest,
                    pVidInf->pSurfaces[CVIDINF_SURFACE_6],
                    &rSrc,
                    DDBLT_WAIT | DDBLT_KEYSRC | DDBLT_DDFX,
                    &fx);
            } while (!pVidInf->CheckResults(hr));
            break;
        case 15:
            fx.dwFillColor = pVidInf->ConvertToSurfaceRGB(pVidInf->ApplyBrightnessContrast(RGB(0, 0, 0)));
            do {
                rDest.SetRect(x, y, x + 32, y + 32);
                hr = g_pBaldurChitin->cVideo.cVidBlitter.Blt(pSurface,
                    &rDest,
                    NULL,
                    NULL,
                    DDBLT_WAIT | DDBLT_COLORFILL,
                    &fx);
            } while (!pVidInf->CheckResults(hr));
            break;
        }
    }
}

// 0x552590
BOOL CVisibilityMap::IsCharacterIdOnMap(LONG charId)
{
    for (SHORT nIndex = 0; nIndex < 6; nIndex++) {
        if (m_aCharacterIds[nIndex] == charId) {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x5525C0
BYTE CVisibilityMap::GetSubTileCode(LONG nTile)
{
    BYTE code = 0;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
    // __LINE__: 857
    UTIL_ASSERT(nTile >= 0 && nTile < (m_nWidth * m_nHeight));

    if (!IsTileVisible(nTile)) {
        code |= 0x1;
    }

    if (!IsTileVisible(m_nWidth + nTile)) {
        code |= 0x2;
    }

    if (!IsTileVisible(m_nWidth + nTile + 1)) {
        code |= 0x4;
    }

    if (!IsTileVisible(nTile + 1)) {
        code |= 0x8;
    }

    code <<= FOGOWAR_SHIFT;

    if (!IsTileExplored(nTile)) {
        code |= 0x1;
    }

    if (!IsTileExplored(m_nWidth + nTile)) {
        code |= 0x2;
    }

    if (!IsTileExplored(m_nWidth + nTile + 1)) {
        code |= 0x4;
    }

    if (!IsTileExplored(nTile + 1)) {
        code |= 0x8;
    }

    return code;
}

// 0x5526E0
void CVisibilityMap::GetTileCode(SHORT n64x64Tile, TILE_CODE& tileCode)
{
    // NOTE: Uninline.
    LONG nBossTile = GetBossTile(n64x64Tile);
    LONG nSecondRowTile = m_nWidth + nBossTile;

    // NOTE: Only the low byte of the remainder is kept.
    BOOLEAN bCheckRight = static_cast<BOOLEAN>((nBossTile + 1) % m_nWidth);
    BOOLEAN bCheckDown = nSecondRowTile < m_nWidth * m_nHeight;

    CSingleLock mapLock(&m_critSect, FALSE);
    mapLock.Lock(INFINITE);

    tileCode.tileNW = GetSubTileCode(nBossTile);

    if (bCheckRight) {
        tileCode.tileNE = GetSubTileCode(nBossTile + 1);
    }

    if (bCheckDown) {
        tileCode.tileSW = GetSubTileCode(nSecondRowTile);

        if (bCheckRight) {
            tileCode.tileSE = GetSubTileCode(nSecondRowTile + 1);
        }
    }

    mapLock.Unlock();
}

// 0x5527E0
void CVisibilityMap::Init(CGameArea* pArea)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
    // __LINE__: 988
    UTIL_ASSERT(m_pMap == NULL);

    m_pSearchMap = &(pArea->m_search);
    m_nWidth = static_cast<SHORT>(pArea->GetInfinity()->nAreaX / 32 + 1);
    m_nHeight = static_cast<SHORT>(pArea->GetInfinity()->nAreaY / 32 + 1);
    m_nMapSize = m_nWidth * m_nHeight;

    if (m_pMap != NULL) {
        delete m_pMap;
    }

    m_pMap = new BYTE[m_nMapSize];
    memset(m_pMap, 0, m_nMapSize);

    for (SHORT nIndex = 0; nIndex < 6; nIndex++) {
        m_aCharacterIds[nIndex] = CGameObjectArray::INVALID_INDEX;
    }

    m_bOutDoor = (pArea->m_header.m_areaType & 0x1) != 0;
}

// 0x5528B0
void CVisibilityMap::Marshal(BYTE** pData, DWORD* pnData)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
    // __LINE__: 1045
    UTIL_ASSERT(pData != NULL && pnData != NULL);

    *pnData = (m_nMapSize + 7) / 8;
    *pData = new BYTE[*pnData];
    if (*pData == NULL) {
        *pnData = NULL;
        return;
    }

    LONG nTile = 0;
    LONG nByte = 0;
    BYTE byte;
    BYTE byteMask;
    BYTE nMaxBit;

    while (nTile < m_nMapSize) {
        nMaxBit = static_cast<BYTE>(min(m_nMapSize - nTile, 8));
        byte = 0;
        byteMask = 1;
        while (nMaxBit > 0) {
            // NOTE: Uninline.
            if (IsTileExplored(nTile)) {
                byte |= byteMask;
            }
            byteMask <<= 1;
            nTile++;
            nMaxBit--;
        }
        (*pData)[nByte++] = byte;
    }
}

// 0x552990
void CVisibilityMap::Unmarshal(BYTE* pData, DWORD nData)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
    // __LINE__: 1100
    UTIL_ASSERT(pData != NULL && nData > 0);

    LONG nTile = 0;
    LONG nByte = 0;
    BYTE byte;
    BYTE byteMask;
    BYTE nMaxBit;

    while (nTile < m_nMapSize) {
        nMaxBit = static_cast<BYTE>(min(m_nMapSize - nTile, 8));
        byte = pData[nByte++];
        byteMask = 1;
        while (nMaxBit > 0) {
            if ((byte & byteMask) != 0) {
                // NOTE: Uninline.
                SetTileExplored(nTile);
            }
            byteMask <<= 1;
            nTile++;
            nMaxBit--;
        }
    }
}

// 0x552A50
void CVisibilityMap::MarkTileLine(const CPoint& ptView, const CPoint& ptSearchMapView, LONG nDestX, LONG nDestY, const CPoint& ptOffset, BYTE charId, const BYTE* pVisibleTerrainTable, BOOLEAN bLookingUp)
{
    SHORT nXStep = 1;
    SHORT nYStep = 1;
    INT nFromX = ptView.x;
    INT nFromY = ptView.y;
    LONG nSearchX = ptSearchMapView.x;
    LONG nSearchY = ptSearchMapView.y;
    INT nToX = nDestX;
    INT nToY = nDestY;
    CRect rClip(0, 0, m_nWidth - 1, m_nHeight - 1);
    SHORT nHighest = 0;
    BOOLEAN bBlocked = FALSE;
    LONG nLastX = -1;
    LONG nLastY = -1;
    SHORT nTableIndex;
    SHORT nHeight;
    CPoint ptTile;
    LONG nSearchDestX;
    LONG nSearchDestY;
    LONG nDeltaX;
    LONG nDeltaY;
    LONG nError;
    LONG nErrorInc;
    LONG nErrorDiagInc;
    BOOL bXMajor;

    CVidMode::ClipLine(nFromX, nFromY, nToX, nToY, rClip);

    nSearchDestX = 2 * nToX;
    nSearchDestY = VIS_TO_SEARCH_Y[nToY % 3] + 8 * (nToY / 3);

    nDeltaX = nSearchDestX - nSearchX;
    nDeltaY = nSearchDestY - nSearchY;

    if (nDeltaX < 0) {
        nXStep = -1;
        nDeltaX = -nDeltaX;
    }

    if (nDeltaY < 0) {
        nYStep = -1;
        nDeltaY = -nDeltaY;
    }

    // NOTE: The original has two copies of the loop below, one for each major
    // axis. They only differ in how the line is stepped.
    bXMajor = nDeltaX > nDeltaY;
    if (bXMajor) {
        nErrorInc = 2 * nDeltaY;
        nError = nErrorInc - nDeltaX;
        nErrorDiagInc = 2 * (nDeltaY - nDeltaX);
    } else {
        nErrorInc = 2 * nDeltaX;
        nError = nErrorInc - nDeltaY;
        nErrorDiagInc = 2 * (nDeltaX - nDeltaY);
    }

    while (nSearchX != nSearchDestX || nSearchY != nSearchDestY) {
        if (nToX == m_nWidth - 1 && !bBlocked) {
            ptTile.x = nToX;
            ptTile.y = SEARCH_TO_VIS_Y[nSearchY & 7] + 3 * (nSearchY >> 3);
            if (nLastX != ptTile.x || nLastY != ptTile.y) {
                // NOTE: Uninline.
                SetTileVisible(m_nWidth * ptTile.y + ptTile.x, charId);
            }
        } else if (nToY == m_nHeight - 1 && !bBlocked) {
            ptTile.x = nSearchX >> 1;
            ptTile.y = nToY;
            if (nLastX != ptTile.x || nLastY != ptTile.y) {
                // NOTE: Uninline.
                SetTileVisible(m_nWidth * ptTile.y + ptTile.x, charId);
            }
        } else {
            if (m_pSearchMap->GetLOSCost(CPoint(nSearchX, nSearchY), pVisibleTerrainTable, nTableIndex, m_bOutDoor) == CPathSearch::COST_IMPASSABLE) {
                bBlocked = TRUE;

                nHeight = m_pSearchMap->GetStructureHeight(nTableIndex);
                if (nHeight < nHighest) {
                    return;
                }

                nHighest = nHeight;

                if (bLookingUp) {
                    ptTile.x = nSearchX >> 1;
                    ptTile.y = SEARCH_TO_VIS_Y[nSearchY & 7] + 3 * (nSearchY >> 3);
                    if (nLastX != ptTile.x || nLastY != ptTile.y) {
                        ClimbWall(ptTile, CPoint(nToX, nToY), charId, pVisibleTerrainTable, nHeight);
                        nLastX = ptTile.x;
                        nLastY = ptTile.y;
                    }
                }
            } else {
                if (bBlocked) {
                    return;
                }
            }
        }

        ptTile.x = nSearchX >> 1;
        ptTile.y = SEARCH_TO_VIS_Y[nSearchY & 7] + 3 * (nSearchY >> 3);
        if (nLastX != ptTile.x || nLastY != ptTile.y) {
            // NOTE: Uninline.
            SetTileVisible(m_nWidth * ptTile.y + ptTile.x, charId);
            nLastX = ptTile.x;
            nLastY = ptTile.y;
        }

        if (nError <= 0) {
            nError += nErrorInc;
            if (bXMajor) {
                nSearchX += nXStep;
            } else {
                nSearchY += nYStep;
            }
        } else {
            nError += nErrorDiagInc;
            nSearchY += nYStep;
            nSearchX += nXStep;
        }
    }

    if (nToX == m_nWidth - 1) {
        ptTile.x = nToX;
        ptTile.y = SEARCH_TO_VIS_Y[nSearchY & 7] + 3 * (nSearchY >> 3);
        if (nLastX != ptTile.x || nLastY != ptTile.y) {
            // NOTE: Uninline.
            SetTileVisible(m_nWidth * ptTile.y + ptTile.x, charId);
        }
    } else if (nToY == m_nHeight - 1) {
        ptTile.x = nSearchX >> 1;
        ptTile.y = nToY;
        if (nLastX != ptTile.x || nLastY != ptTile.y) {
            // NOTE: Uninline.
            SetTileVisible(m_nWidth * ptTile.y + ptTile.x, charId);
        }
    } else {
        if (m_pSearchMap->GetLOSCost(CPoint(nSearchX, nSearchY), pVisibleTerrainTable, nTableIndex, m_bOutDoor) == CPathSearch::COST_IMPASSABLE) {
            nHeight = m_pSearchMap->GetStructureHeight(nTableIndex);
            if (nHeight < nHighest) {
                return;
            }

            ptTile.x = nSearchX >> 1;
            ptTile.y = SEARCH_TO_VIS_Y[nSearchY & 7] + 3 * (nSearchY >> 3);
            if (nLastX == ptTile.x && nLastY == ptTile.y) {
                return;
            }

            if (bLookingUp) {
                ClimbWall(ptTile, CPoint(nToX, nToY), charId, pVisibleTerrainTable, nHeight);
                return;
            }

            // NOTE: Uninline.
            SetTileVisible(m_nWidth * ptTile.y + ptTile.x, charId);
        } else {
            if (bBlocked) {
                return;
            }

            ptTile.x = nSearchX >> 1;
            ptTile.y = SEARCH_TO_VIS_Y[nSearchY & 7] + 3 * (nSearchY >> 3);
            if (nLastX == ptTile.x && nLastY == ptTile.y) {
                return;
            }

            // NOTE: Uninline.
            SetTileVisible(m_nWidth * ptTile.y + ptTile.x, charId);
        }
    }
}

// 0x553290
void CVisibilityMap::PrivateAddCharacter(const CPoint& ptPos, BYTE visId, const BYTE* pVisibleTerrainTable)
{
    CPoint ptEar;
    LONG earZ;

    CPoint ptView;
    ptView.x = ptPos.x / 32;
    ptView.y = ptPos.y / 32;

    CPoint ptOffset;
    ptOffset.x = ptPos.x % 32;
    ptOffset.y = ptPos.y % 32;

    CPoint ptSearchMapView;
    ptSearchMapView.x = ptPos.x / CPathSearch::GRID_SQUARE_SIZEX;
    ptSearchMapView.y = ptPos.y / CPathSearch::GRID_SQUARE_SIZEY;

    MarkEllipseCaps(ptView, ptOffset, ptSearchMapView, visId, pVisibleTerrainTable);
    MarkEllipseSides(ptView, ptOffset, ptSearchMapView, visId, pVisibleTerrainTable);

    g_pBaldurChitin->cSoundMixer.GetListenPosition(ptEar, earZ);
    if (ptEar.x == -1 && ptEar.y == -1) {
        if (m_pSearchMap->GetArea() == g_pBaldurChitin->GetObjectGame()->GetVisibleArea()) {
            m_pSearchMap->GetArea()->GetInfinity()->UpdateListenPosition();
        }
    }
}

// 0x5533B0
void CVisibilityMap::RemoveCharacter(const CPoint& pos, LONG charId, const BYTE* pVisibleTerrainTable)
{
    CPoint pt;
    pt.x = pos.x / 32;
    pt.y = pos.y / 32;

    for (SHORT nIndex = 0; nIndex < 6; nIndex++) {
        if (m_aCharacterIds[nIndex] == charId) {
            m_aCharacterIds[nIndex] = CGameObjectArray::INVALID_INDEX;

            BYTE visId = 1 << nIndex;
            if (visId != static_cast<BYTE>(-1)) {
                ClearEllipseCaps(pt, visId);
                ClearEllipseSides(pt, visId);
            }

            break;
        }
    }
}

// Clears `charId` from the rows covered by the top and bottom caps of the
// visibility ellipse (see `MarkEllipseCaps`).
//
// 0x553440
void CVisibilityMap::ClearEllipseCaps(const CPoint& pt, BYTE charId)
{
    LONG y = pt.y - field_10;
    LONG nLeft = pt.x;
    LONG nRight = pt.x + 1;
    BYTE* pArc = field_12;
    SHORT nCount = field_4;
    SHORT nYStep = 1;
    BYTE nArcStep;
    LONG nStart;
    LONG nEnd;
    LONG nIndex;

    for (INT nPass = 2; nPass != 0; nPass--) {
        while (nCount-- > 0) {
            if (*pArc++ == 0) {
                while (nCount-- != 0) {
                    nArcStep = *pArc;
                    nLeft--;
                    nRight++;
                    pArc++;
                    if (nArcStep != 0) {
                        break;
                    }
                }
            }

            if (y >= 0 && y < m_nHeight) {
                nStart = nLeft >= 0 ? nLeft : 0;
                nIndex = m_nWidth * y + nStart;
                nEnd = nRight < m_nWidth - 1 ? nRight : m_nWidth - 1;
                for (LONG x = nStart; x <= nEnd; x++) {
                    // NOTE: Uninline.
                    DecrementVisibility(nIndex++, charId);
                }
            }

            y += nYStep;
            nLeft--;
            nRight++;
        }

        y = pt.y + field_10;
        nLeft = pt.x;
        pArc = field_12;
        nRight = pt.x + 1;
        nCount = field_4;
        nYStep = -1;
    }
}

// Clears `charId` from the rows covered by the middle of the visibility
// ellipse (see `MarkEllipseSides`).
//
// 0x5535D0
void CVisibilityMap::ClearEllipseSides(const CPoint& pt, BYTE charId)
{
    LONG y = pt.y;
    LONG nLeft = pt.x - field_E + 1;
    LONG nRight = pt.x + field_E;
    BYTE* pArc = field_16;
    SHORT nCount = field_1A;
    SHORT nYStep = -1;
    LONG nStart;
    LONG nEnd;
    LONG nIndex;

    for (INT nPass = 2; nPass != 0; nPass--) {
        while (nCount-- >= 0) {
            if (y >= 0 && y < m_nHeight) {
                nStart = nLeft >= 0 ? nLeft : 0;
                nIndex = m_nWidth * y + nStart;
                nEnd = nRight < m_nWidth - 1 ? nRight : m_nWidth - 1;
                for (LONG x = nStart; x <= nEnd; x++) {
                    // NOTE: Uninline.
                    DecrementVisibility(nIndex++, charId);
                }
            }

            if (*pArc++ != 0) {
                nLeft++;
                nRight--;
            }

            y += nYStep;
        }

        y = pt.y + 1;
        nYStep = 1;
        nLeft = pt.x + field_16[0] - field_E + 1;
        nRight = pt.x - field_16[0] + field_E;
        nCount = field_1A - 1;
        pArc = field_16 + 1;
    }
}

// 0x553740
void CVisibilityMap::SetAreaExplored()
{
    for (LONG nTile = 0; nTile < m_nWidth * m_nHeight; nTile++) {
        // NOTE: Uninline.
        SetTileExplored(nTile);
    }
}

// 0x5537A0
void CVisibilityMap::SetAreaUnexplored()
{
    for (LONG nTile = 0; nTile < m_nWidth * m_nHeight; nTile++) {
        // NOTE: Uninline.
        SetTileUnexplored(nTile);
    }
}

// Casts sight lines to the top and bottom arcs of the visibility ellipse
// (`field_E` x `field_10` tiles), walking outwards from the centre column
// along the `field_12` arc steps.
//
// 0x553800
void CVisibilityMap::MarkEllipseCaps(const CPoint& ptView, const CPoint& ptOffset, const CPoint& ptSearchMapView, BYTE charId, const BYTE* pVisibleTerrainTable)
{
    LONG y = ptView.y - field_10;
    LONG nLeft = ptView.x;
    LONG nRight = ptView.x + 1;
    BYTE* pArc = field_12;
    BOOLEAN bLookingUp = TRUE;
    SHORT nYStep = 1;
    LONG nInnerLeft;
    LONG nInnerRight;
    SHORT nCount;

    for (INT nPass = 2; nPass != 0; nPass--) {
        nCount = field_4;
        nInnerRight = nRight - 1;
        nInnerLeft = nLeft + 1;
        while (nCount-- > 0) {
            MarkTileLine(ptView, ptSearchMapView, nLeft, y, ptOffset, charId, pVisibleTerrainTable, bLookingUp);
            MarkTileLine(ptView, ptSearchMapView, nRight, y, ptOffset, charId, pVisibleTerrainTable, bLookingUp);

            nLeft--;
            nInnerLeft--;
            nRight++;
            nInnerRight++;

            if (*pArc++ != 0) {
                y += nYStep;
                MarkTileLine(ptView, ptSearchMapView, nInnerLeft, y, ptOffset, charId, pVisibleTerrainTable, bLookingUp);
                MarkTileLine(ptView, ptSearchMapView, nInnerRight, y, ptOffset, charId, pVisibleTerrainTable, bLookingUp);
            }
        }

        y = ptView.y + field_10;
        nLeft = ptView.x;
        nRight = ptView.x + 1;
        pArc = field_12;
        bLookingUp = FALSE;
        nYStep = -1;
    }
}

// Casts sight lines to the left and right arcs of the visibility ellipse,
// walking up and then down from the centre row along the `field_16` arc
// steps.
//
// 0x553990
void CVisibilityMap::MarkEllipseSides(const CPoint& ptView, const CPoint& ptOffset, const CPoint& ptSearchMapView, BYTE charId, const BYTE* pVisibleTerrainTable)
{
    LONG nLeft = ptView.x - field_E + 1;
    LONG nRight = ptView.x + field_E;
    LONG y = ptView.y;
    BYTE* pArc = field_16;
    SHORT nCount = field_1A;
    BOOLEAN bLookingUp = TRUE;
    SHORT nYStep = -1;

    for (INT nPass = 2; nPass != 0; nPass--) {
        while (nCount-- >= 0) {
            MarkTileLine(ptView, ptSearchMapView, nLeft, y, ptOffset, charId, pVisibleTerrainTable, bLookingUp);
            MarkTileLine(ptView, ptSearchMapView, nRight, y, ptOffset, charId, pVisibleTerrainTable, bLookingUp);

            if (pArc != NULL && *pArc++ != 0) {
                nLeft++;
                nRight--;
                MarkTileLine(ptView, ptSearchMapView, nLeft, y, ptOffset, charId, pVisibleTerrainTable, bLookingUp);
                MarkTileLine(ptView, ptSearchMapView, nRight, y, ptOffset, charId, pVisibleTerrainTable, bLookingUp);
            }

            y += nYStep;
        }

        nLeft = ptView.x + field_16[0] - field_E + 1;
        nRight = ptView.x - field_16[0] + field_E;
        y = ptView.y + 1;
        bLookingUp = FALSE;
        pArc = field_16 + 1;
        nYStep = 1;
        nCount = field_1A - 1;
    }
}

// 0x553B10
void CVisibilityMap::Uninit()
{
    if (m_pMap != NULL) {
        delete m_pMap;
        m_pMap = NULL;
    }

    for (SHORT nIndex = 0; nIndex < 6; nIndex++) {
        m_aCharacterIds[nIndex] = CGameObjectArray::INVALID_INDEX;
    }
}

// 0x553B40
void CVisibilityMap::UpDate(const CPoint& ptOldPos, const CPoint& ptNewPos, LONG charId, const BYTE* pVisibleTerrainTable)
{
    CPoint pt;
    pt.x = ptOldPos.x / 32;
    pt.y = ptOldPos.y / 32;

    // NOTE: Uninline.
    BYTE visId = GetCharacterPos(charId);

    if (visId != static_cast<BYTE>(-1)) {
        ClearEllipseCaps(pt, visId);
        ClearEllipseSides(pt, visId);
        PrivateAddCharacter(ptNewPos, visId, pVisibleTerrainTable);
    } else if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(charId) != -1) {
        // NOTE: Differs from the original on purpose. The exe does nothing
        // for a party member that has no slot, so the fog of war stayed over
        // everything that member walked through. Give it a slot now.
        AddCharacter(ptNewPos, charId, pVisibleTerrainTable);
    }
}

// 0x553BD0
BOOLEAN CVisibilityMap::IsTileExplored(LONG nIndex)
{
    if (nIndex >= 0 && nIndex < m_nMapSize) {
        return (m_pMap[nIndex] & EXPLORED_FLAG) != 0;
    } else {
        return FALSE;
    }
}

// 0x553C00
BOOLEAN CVisibilityMap::IsTileVisible(LONG nIndex)
{
    if (nIndex >= 0 && nIndex < m_nMapSize) {
        return (m_pMap[nIndex] & VISIBILITY_MASK) != 0;
    } else {
        return FALSE;
    }
}

// 0x553C30
void CVisibilityMap::SetAreaVisible(BOOLEAN bVisible)
{
    for (INT nIndex = 0; nIndex < m_nWidth * m_nHeight; nIndex++) {
        if (bVisible == TRUE) {
            m_pMap[nIndex] |= VISIBILITY_MASK;
        } else {
            m_pMap[nIndex] &= ~VISIBILITY_MASK;
        }
    }
}

// 0x553D00
void CVisibilityMap::BltFogOWar3d(INT x, INT y, const CRect& rDest, const TILE_CODE& tileCode)
{
    BYTE aTileCodes[4];
    CSize aSizes[4];
    CRect r;
    BYTE nFogWarCode;
    BYTE nExploredCode;

    aSizes[0].cx = x;
    aSizes[0].cy = y;
    aSizes[1].cx = x + 32;
    aSizes[1].cy = y;
    aSizes[2].cx = x;
    aSizes[2].cy = y + 32;
    aSizes[3].cx = x + 32;
    aSizes[3].cy = y + 32;

    aTileCodes[0] = tileCode.tileNW;
    aTileCodes[1] = tileCode.tileNE;
    aTileCodes[2] = tileCode.tileSW;
    aTileCodes[3] = tileCode.tileSE;

    COLORREF rgb = g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(RGB(0, 0, 0));

    FOGOWARSHADE.fRed = static_cast<float>(GetRValue(rgb)) / 255.0f;
    EXPLORESHADE.fRed = FOGOWARSHADE.fRed;
    CLEARSHADE.fRed = FOGOWARSHADE.fRed;

    FOGOWARSHADE.fGreen = static_cast<float>(GetGValue(rgb)) / 255.0f;
    EXPLORESHADE.fGreen = FOGOWARSHADE.fGreen;
    CLEARSHADE.fGreen = FOGOWARSHADE.fGreen;

    FOGOWARSHADE.fBlue = static_cast<float>(GetBValue(rgb)) / 255.0f;
    EXPLORESHADE.fBlue = FOGOWARSHADE.fBlue;
    CLEARSHADE.fBlue = FOGOWARSHADE.fBlue;

    for (int index = 0; index < 4; index++) {
        nExploredCode = (aTileCodes[index] & FOGOWAR_EXPLORED_MASK) >> FOGOWAR_EXPLORED_SHIFT;
        nFogWarCode = (aTileCodes[index] & FOGOWAR_MASK) >> FOGOWAR_SHIFT;
        r.SetRect(aSizes[index].cx,
            aSizes[index].cy,
            aSizes[index].cx + 32,
            aSizes[index].cy + 32);
        if (nExploredCode != nFogWarCode) {
            BltVisibility3d(nFogWarCode, r);
        }
        BltExploration3d(nExploredCode, r);
    }
}

// 0x553EA0
void CVisibilityMap::BltExploration3d(BYTE code, const CRect& rDest)
{
    VERTEX_DESC verts[6];
    int cnt = 0;

    verts[0].fX = static_cast<float>(rDest.left + 16);
    verts[0].fY = static_cast<float>(rDest.top + 16);
    verts[1].fX = static_cast<float>(rDest.left);
    verts[1].fY = static_cast<float>(rDest.top);
    verts[2].fX = static_cast<float>(rDest.left);
    verts[2].fY = static_cast<float>(rDest.bottom);
    verts[3].fX = static_cast<float>(rDest.right);
    verts[3].fY = static_cast<float>(rDest.bottom);
    verts[4].fX = static_cast<float>(rDest.right);
    verts[4].fY = static_cast<float>(rDest.top);
    verts[5].fX = static_cast<float>(rDest.left);
    verts[5].fY = static_cast<float>(rDest.top);

    if ((code & 0x1) != 0) {
        verts[1].color = EXPLORESHADE;
        verts[5].color = EXPLORESHADE;
        cnt++;
    } else {
        verts[1].color = CLEARSHADE;
        verts[5].color = CLEARSHADE;
    }

    if ((code & 0x2) != 0) {
        verts[2].color = EXPLORESHADE;
        cnt++;
    } else {
        verts[2].color = CLEARSHADE;
    }

    if ((code & 0x4) != 0) {
        verts[3].color = EXPLORESHADE;
        cnt++;
    } else {
        verts[3].color = CLEARSHADE;
    }

    if ((code & 0x8) != 0) {
        verts[4].color = EXPLORESHADE;
        cnt++;
    } else {
        verts[4].color = CLEARSHADE;
    }

    switch (cnt) {
    case 3:
        verts[0].color = EXPLORESHADE;
        CVidMode::RenderFan(verts, 6);
        break;
    case 1:
        verts[0].color = CLEARSHADE;
        CVidMode::RenderFan(verts, 6);
        break;
    default:
        CVidMode::FillRect3d(&(verts[1]));
        break;
    }
}

// 0x554170
void CVisibilityMap::BltVisibility3d(BYTE code, const CRect& rDest)
{
    VERTEX_DESC verts[6];
    int cnt = 0;

    verts[0].fX = static_cast<float>(rDest.left + 16);
    verts[0].fY = static_cast<float>(rDest.top + 16);
    verts[1].fX = static_cast<float>(rDest.left);
    verts[1].fY = static_cast<float>(rDest.top);
    verts[2].fX = static_cast<float>(rDest.left);
    verts[2].fY = static_cast<float>(rDest.bottom);
    verts[3].fX = static_cast<float>(rDest.right);
    verts[3].fY = static_cast<float>(rDest.bottom);
    verts[4].fX = static_cast<float>(rDest.right);
    verts[4].fY = static_cast<float>(rDest.top);
    verts[5].fX = static_cast<float>(rDest.left);
    verts[5].fY = static_cast<float>(rDest.top);

    if ((code & 0x1) != 0) {
        verts[1].color = FOGOWARSHADE;
        verts[5].color = FOGOWARSHADE;
        cnt++;
    } else {
        verts[1].color = CLEARSHADE;
        verts[5].color = CLEARSHADE;
    }

    if ((code & 0x2) != 0) {
        verts[2].color = FOGOWARSHADE;
        cnt++;
    } else {
        verts[2].color = CLEARSHADE;
    }

    if ((code & 0x4) != 0) {
        verts[3].color = FOGOWARSHADE;
        cnt++;
    } else {
        verts[3].color = CLEARSHADE;
    }

    if ((code & 0x8) != 0) {
        verts[4].color = FOGOWARSHADE;
        cnt++;
    } else {
        verts[4].color = CLEARSHADE;
    }

    switch (cnt) {
    case 3:
        verts[0].color = FOGOWARSHADE;
        CVidMode::RenderFan(verts, 6);
        break;
    case 1:
        verts[0].color = CLEARSHADE;
        CVidMode::RenderFan(verts, 6);
        break;
    default:
        CVidMode::FillRect3d(&(verts[1]));
        break;
    }
}

// 0x76B120
SHORT CVisibilityMap::PointToTile(const CPoint& pos)
{
    return static_cast<SHORT>(pos.x / SQUARE_SIZEX + m_nWidth * (pos.y / SQUARE_SIZEY));
}

// NOTE: Inlined.
LONG CVisibilityMap::GetBossTile(LONG n64x64Tile)
{
    return 2 * (n64x64Tile % (m_nWidth / 2) + m_nWidth * (n64x64Tile / (m_nWidth / 2)));
}

// NOTE: Inlined.
BYTE CVisibilityMap::GetCharacterPos(LONG charId)
{
    for (SHORT index = 0; index < 6; index++) {
        if (m_aCharacterIds[index] == charId) {
            return 1 << static_cast<BYTE>(index);
        }
    }
    return -1;
}

// NOTE: Inlined.
void CVisibilityMap::SetTileExplored(LONG nIndex)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
    // __LINE__: 2281
    UTIL_ASSERT_MSG((nIndex >= 0 && nIndex < m_nMapSize), "Index into vis map out of range in SetTileExplored");

    m_pMap[nIndex] |= EXPLORED_FLAG;
}

// NOTE: Inlined.
void CVisibilityMap::SetTileUnexplored(LONG nIndex)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
    // __LINE__: 2288
    UTIL_ASSERT_MSG((nIndex >= 0 && nIndex < m_nMapSize), "Index into vis map out of range in SetTileUnexplored");

    m_pMap[nIndex] &= ~EXPLORED_FLAG;
}

// NOTE: Inlined.
void CVisibilityMap::SetTileVisible(LONG nIndex, BYTE charId)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
    // __LINE__: 2295
    UTIL_ASSERT_MSG((nIndex >= 0 && nIndex < m_nMapSize), "Index into vis map out of range in SetTileVisible");

    m_pMap[nIndex] |= charId | EXPLORED_FLAG;
}

// NOTE: Inlined.
void CVisibilityMap::DecrementVisibility(LONG nIndex, BYTE charId)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CVisibility.cpp
    // __LINE__: 2274
    UTIL_ASSERT_MSG((nIndex >= 0 && nIndex < m_nMapSize), "Index into vis map out of range in DecrementVisibility");

    m_pMap[nIndex] &= ~charId;
}
