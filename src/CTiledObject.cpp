#include "CTiledObject.h"

#include "CInfinity.h"
#include "CResWED.h"
#include "CUtil.h"

// 0x84EC08
const WORD CTiledObject::STATE_PRIMARY_TILE = 1;

// 0x84EC0A
const WORD CTiledObject::STATE_SECONDARY_TILE = 2;

// NOTE: Inlined.
CTiledObject::CTiledObject()
{
    m_posAreaList = NULL;
    m_pResWed = NULL;
    m_nWedIndex = 0;
    m_wAIState = 0;
    m_wRenderState = 0;
}

// 0x54E5B0
void CTiledObject::CheckTileState(CInfinity& cInfinity)
{
    WORD wAIState = m_wAIState;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CTiledObject.cpp
    // __LINE__: 84
    UTIL_ASSERT(m_pResWed != NULL);

    if (wAIState != m_wRenderState && m_nWedIndex != -1) {
        m_wRenderState = wAIState;

        WORD* pTiledObjectList = m_pResWed->GetTiledObjectList();
        WORD* pTileList = m_pResWed->GetTileList(0);

        if (pTiledObjectList == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CTiledObject.cpp
            // __LINE__: 99
            UTIL_ASSERT(FALSE);
            return;
        }

        WED_TILEDOBJECT* pTiledObject = m_pResWed->GetTiledObject(static_cast<USHORT>(m_nWedIndex));
        WORD wStartingTile = pTiledObject->nStartingTile;
        WORD wEndingTile = pTiledObject->nNumTiles + wStartingTile;

        switch (m_wRenderState) {
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CTiledObject.cpp
            // __LINE__: 130
            UTIL_ASSERT(FALSE);
            return;
        case 2:
            if (wStartingTile < wEndingTile) {
                WORD* pTile = pTiledObjectList + wStartingTile;
                WORD nTiles = wEndingTile - wStartingTile;
                do {
                    WED_TILEDATA* pTileData = m_pResWed->GetTileData(0, *pTile);
                    pTileData->wFlags = m_wRenderState;
                    cInfinity.SwapVRamTiles(pTileList[pTileData->nStartingTile], pTileData->nSecondary);
                    pTile++;
                } while (--nTiles != 0);
            }

            if (pTiledObject->nNumPrimaryPolys != 0xFFFF) {
                for (WORD i = 0; i < pTiledObject->nNumPrimaryPolys; i++) {
                    reinterpret_cast<WED_POLYLIST*>(reinterpret_cast<BYTE*>(m_pResWed->m_pHeader) + sizeof(WED_POLYLIST) * i + pTiledObject->nOffsetToPrimaryPolys)->nType |= 0x2;
                }
            }

            if (pTiledObject->nNumSecondaryPolys != 0xFFFF) {
                for (WORD i = 0; i < pTiledObject->nNumSecondaryPolys; i++) {
                    reinterpret_cast<WED_POLYLIST*>(reinterpret_cast<BYTE*>(m_pResWed->m_pHeader) + sizeof(WED_POLYLIST) * i + pTiledObject->nOffsetToSecondaryPolys)->nType &= 0xFD;
                }
            }
            break;
        case 1:
            if (wStartingTile < wEndingTile) {
                WORD* pTile = pTiledObjectList + wStartingTile;
                WORD nTiles = wEndingTile - wStartingTile;
                do {
                    WED_TILEDATA* pTileData = m_pResWed->GetTileData(0, *pTile);
                    pTileData->wFlags = m_wRenderState;
                    cInfinity.SwapVRamTiles(pTileData->nSecondary, pTileList[pTileData->nStartingTile]);
                    pTile++;
                } while (--nTiles != 0);
            }

            if (pTiledObject->nNumPrimaryPolys != 0xFFFF) {
                for (WORD i = 0; i < pTiledObject->nNumPrimaryPolys; i++) {
                    reinterpret_cast<WED_POLYLIST*>(reinterpret_cast<BYTE*>(m_pResWed->m_pHeader) + sizeof(WED_POLYLIST) * i + pTiledObject->nOffsetToPrimaryPolys)->nType &= 0xFD;
                }
            }

            if (pTiledObject->nNumSecondaryPolys != 0xFFFF) {
                for (WORD i = 0; i < pTiledObject->nNumSecondaryPolys; i++) {
                    reinterpret_cast<WED_POLYLIST*>(reinterpret_cast<BYTE*>(m_pResWed->m_pHeader) + sizeof(WED_POLYLIST) * i + pTiledObject->nOffsetToSecondaryPolys)->nType |= 0x2;
                }
            }
            break;
        }
    }
}

// 0x54E830
void CTiledObject::Initialize(CResWED* pResWed, CResRef resID, POSITION posAreaList, WORD wInitialState)
{
    CResRef cResRef;

    m_pResWed = pResWed;
    m_posAreaList = posAreaList;

    WED_TILEDOBJECT* pTiledObject = m_pResWed->GetTiledObjects();
    ULONG nTiledObjects = m_pResWed->GetNumTiledObjects();

    WORD nIndex;
    for (nIndex = 0; nIndex < nTiledObjects; nIndex++) {
        cResRef = pTiledObject->resID;
        if (resID == cResRef) {
            m_nWedIndex = nIndex;
            break;
        }
        pTiledObject++;
    }

    m_resId = resID;

    if (nIndex == nTiledObjects) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CTiledObject.cpp
        // __LINE__: 186
        UTIL_ASSERT_MSG(FALSE, "Tiled object resref not found");

        m_nWedIndex = -1;
    }

    m_wAIState = wInitialState;
    m_wRenderState = STATE_PRIMARY_TILE;

    if (m_nWedIndex != -1) {
        if (pTiledObject->nNumPrimaryPolys != 0xFFFF) {
            for (WORD i = 0; i < pTiledObject->nNumPrimaryPolys; i++) {
                reinterpret_cast<WED_POLYLIST*>(reinterpret_cast<BYTE*>(m_pResWed->m_pHeader) + sizeof(WED_POLYLIST) * i + pTiledObject->nOffsetToPrimaryPolys)->nType &= 0xFD;
            }
        }

        if (pTiledObject->nNumSecondaryPolys != 0xFFFF) {
            for (WORD i = 0; i < pTiledObject->nNumSecondaryPolys; i++) {
                reinterpret_cast<WED_POLYLIST*>(reinterpret_cast<BYTE*>(m_pResWed->m_pHeader) + sizeof(WED_POLYLIST) * i + pTiledObject->nOffsetToSecondaryPolys)->nType |= 0x2;
            }
        }
    }
}

// 0x54E980
void CTiledObject::SetNewResWED(CResWED* pNewResWED)
{
    CResRef cResRef;

    m_pResWed = pNewResWED;

    WED_TILEDOBJECT* pTiledObject = m_pResWed->GetTiledObjects();
    ULONG nTiledObjects = m_pResWed->GetNumTiledObjects();

    WORD nIndex;
    for (nIndex = 0; nIndex < nTiledObjects; nIndex++) {
        cResRef = pTiledObject->resID;
        if (m_resId == cResRef) {
            m_nWedIndex = nIndex;
            break;
        }
        pTiledObject++;
    }

    if (nIndex == nTiledObjects) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CTiledObject.cpp
        // __LINE__: 251
        UTIL_ASSERT_MSG(FALSE, "Tiled object resref not found");

        m_nWedIndex = -1;
    }

    m_wRenderState = STATE_PRIMARY_TILE;

    if (m_nWedIndex != -1) {
        if (pTiledObject->nNumPrimaryPolys != 0xFFFF) {
            for (WORD i = 0; i < pTiledObject->nNumPrimaryPolys; i++) {
                reinterpret_cast<WED_POLYLIST*>(reinterpret_cast<BYTE*>(m_pResWed->m_pHeader) + sizeof(WED_POLYLIST) * i + pTiledObject->nOffsetToPrimaryPolys)->nType &= 0xFD;
            }
        }

        if (pTiledObject->nNumSecondaryPolys != 0xFFFF) {
            for (WORD i = 0; i < pTiledObject->nNumSecondaryPolys; i++) {
                reinterpret_cast<WED_POLYLIST*>(reinterpret_cast<BYTE*>(m_pResWed->m_pHeader) + sizeof(WED_POLYLIST) * i + pTiledObject->nOffsetToSecondaryPolys)->nType |= 0x2;
            }
        }
    }
}
