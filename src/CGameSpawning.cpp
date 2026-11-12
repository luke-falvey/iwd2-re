#include "CGameSpawning.h"

#include "CBaldurChitin.h"
#include "CCreatureFile.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CPathSearch.h"
#include "CUtil.h"

// 0x4C9780
CGameSpawning::CGameSpawning(CGameArea* pArea, CAreaFileRandomMonsterSpawningPoint* pSpawningObject)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameSpawning.cpp
    // __LINE__: 108
    UTIL_ASSERT(pArea != NULL && pSpawningObject != NULL);

    m_objectType = TYPE_SPAWNING;

    memcpy(&m_spawningObject, pSpawningObject, sizeof(CAreaFileRandomMonsterSpawningPoint));

    CVariable variable;
    variable.SetName(m_spawningObject.m_scriptName);
    pArea->GetNamedCreatures()->AddKey(variable);

    if (m_spawningObject.m_probDay == 0) {
        m_spawningObject.m_probDay = 100;
    }

    if (m_spawningObject.m_probNight == 0) {
        m_spawningObject.m_probNight = 100;
    }

    field_136 = 0;

    // NOTE: This assignment is slightly incorrect. Original code refers to
    // other offsets (0x8A8168 and 0x8A8154 respectively). It can mean two
    // things:
    // - There was second set of terrain table defaults, which is exactly the
    // same as in `CGameObject`.
    // - Such split was a result of some compiler optimization.
    //
    // Since both sets have the same values, there is no harm to reuse what's
    // already present.
    memcpy(m_terrainTable, DEFAULT_TERRAIN_TABLE, 16);
    memcpy(m_visibleTerrainTable, DEFAULT_VISIBLE_TERRAIN_TABLE, 16);

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) == CGameObjectArray::SUCCESS) {
        AddToArea(pArea,
            CPoint(m_spawningObject.m_posX, m_spawningObject.m_posY),
            0,
            LIST_BACK);
    } else {
        // FIXME: Doesn't look cool.
        delete this;
    }
}

// 0x4C9990
CGameSpawning::~CGameSpawning()
{
}

// 0x47C830
BOOLEAN CGameSpawning::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x766660
BOOLEAN CGameSpawning::CompressTime(DWORD deltaTime)
{
    return TRUE;
}

// 0x4CA3D0
void CGameSpawning::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameSpawning.cpp
        // __LINE__: 352
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x4C9CE0
void CGameSpawning::Spawn(const CPoint& ptFacingTowards, BYTE nTotalCharacterLevels)
{
    CCreatureFile creatureData;
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CVisibilityMap* pVisibility = &m_pArea->m_visibility;
    DWORD nTotalXP = 0;
    CPoint* pOffsets = NULL;

    if (m_spawningObject.m_activated == 0) {
        return;
    }

    if (field_136 != 0 && (m_spawningObject.m_wFlags & 0x1) != 0) {
        return;
    }

    ULONG nHour = pGame->m_worldTime.m_gameTime % CTimerWorld::TIMESCALE_MSEC_PER_DAY
        / CTimerWorld::TIMESCALE_MSEC_PER_SEC
        / CTimerWorld::TIMESCALE_SEC_PER_MIN
        / CTimerWorld::TIMESCALE_MIN_PER_HOUR;
    if (((m_spawningObject.m_timeOfDayVisible >> nHour) & 0x1) == 0) {
        return;
    }

    SHORT nRow = static_cast<SHORT>(m_pos.y / CVisibilityMap::SQUARE_SIZEY * pVisibility->m_nWidth);
    SHORT nTile = static_cast<SHORT>(nRow + m_pos.x / CVisibilityMap::SQUARE_SIZEX);
    if (pVisibility->IsTileVisible(nTile)) {
        return;
    }

    if (nTotalCharacterLevels == 0) {
        for (SHORT nCharacter = 0; nCharacter < pGame->m_nCharacters; nCharacter++) {
            LONG nCharacterId = pGame->GetCharacterId(nCharacter);

            CGameSprite* pCharacter;
            if (pGame->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pCharacter),
                    INFINITE)
                == CGameObjectArray::SUCCESS) {
                nTotalCharacterLevels += pCharacter->m_derivedStats.GetLevel();

                pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }

    LONG nGridX = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
    LONG nGridY = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;

    INT nRandomCreature;
    if (m_spawningObject.m_randomCreatureNum == 0) {
        nRandomCreature = 0;
    } else {
        nRandomCreature = rand() % m_spawningObject.m_randomCreatureNum;
    }

    SHORT nIndex = 0;
    if (m_spawningObject.m_maxTypeNum != 0) {
        for (nIndex = 0; nIndex < m_spawningObject.m_maxTypeNum; nIndex++) {
            RESREF* pResRef = &m_spawningObject.m_randomCreature[nRandomCreature];

            // NOTE: Inlined `CResHelper::SetResRef`.
            creatureData.SetResRef(CResRef(reinterpret_cast<BYTE*>(pResRef)), TRUE, TRUE);

            CGameSprite* pSprite = new CGameSprite(creatureData.GetData(),
                creatureData.GetDataSize(),
                0,
                1,
                m_spawningObject.m_lifeSpan * CTimerWorld::TIMESCALE_MSEC_PER_SEC + pGame->m_worldTime.m_gameTime,
                m_spawningObject.m_huntingRange,
                m_spawningObject.m_followRange,
                0x7FFFFFFF,
                m_pos,
                0);

            pSprite->SetResRef(creatureData.GetResRef().GetResRefStr());

            creatureData.ReleaseData();

            if (pSprite == NULL) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameSpawning.cpp
                // __LINE__: 280
                UTIL_ASSERT(FALSE);
                continue;
            }

            CString sResRef;
            sResRef = reinterpret_cast<char*>(pResRef);
            pSprite->SetResRef(sResRef);

            nTotalXP += pSprite->m_derivedStats.m_nXP;

            if (nIndex != 0 || nRandomCreature != 0) {
                if (nTotalXP > static_cast<DWORD>(m_spawningObject.m_difficulty * nTotalCharacterLevels) / 100) {
                    if (pGame->GetObjectArray()->Delete(pSprite->m_id,
                            CGameObjectArray::THREAD_ASYNCH,
                            NULL,
                            INFINITE)
                        == CGameObjectArray::SUCCESS) {
                        delete pSprite;
                    }

                    if (nIndex != 0) {
                        break;
                    }

                    nIndex = -1;
                    nRandomCreature = 0;
                    continue;
                }
            }

            if (nIndex == 0) {
                SHORT nFormation = rand() % 9 + 1;
                pOffsets = pGame->m_group.GetOffsets(nFormation,
                    (pSprite->GetDirection(ptFacingTowards) * 360 / 16) % 360,
                    TRUE);
            }

            CPoint* pOffset = &pOffsets[nIndex % CAIGroup::FORMATION_MAX_SIZE];
            CPoint ptTarget;
            ptTarget.x = pOffset->x / CAIGroup::OFFSET_MULTIPLIER + nGridX;
            ptTarget.y = pOffset->y / CAIGroup::OFFSET_MULTIPLIER + nGridY;

            ptTarget = m_pArea->m_search.GetNearestOpenSquare(ptTarget,
                m_terrainTable,
                pSprite->m_animation.GetPersonalSpace(),
                -1);

            ptTarget.x = ptTarget.x * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2;
            ptTarget.y = ptTarget.y * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2;

            nRow = static_cast<SHORT>(ptTarget.y / CVisibilityMap::SQUARE_SIZEY * pVisibility->m_nWidth);
            nTile = static_cast<SHORT>(nRow + ptTarget.x / CVisibilityMap::SQUARE_SIZEX);

            if (pVisibility->IsTileVisible(nTile)) {
                if (pGame->GetObjectArray()->Delete(pSprite->m_id,
                        CGameObjectArray::THREAD_ASYNCH,
                        NULL,
                        INFINITE)
                    == CGameObjectArray::SUCCESS) {
                    delete pSprite;
                }

                nIndex--;
                break;
            }

            pSprite->AddToArea(m_pArea, ptTarget, 0, pSprite->m_animation.GetListType());
            pSprite->SetFacing(pSprite->GetDirection(ptFacingTowards));

            if (nTotalXP == static_cast<DWORD>(m_spawningObject.m_difficulty * nTotalCharacterLevels) / 100) {
                break;
            }
        }
    }

    if (pOffsets != NULL) {
        delete[] pOffsets;
    }

    if (nIndex >= 0) {
        field_136 = 1;
    }
}

// 0x4CA430
void CGameSpawning::Marshal(CAreaFileRandomMonsterSpawningPoint** pSpawningObject)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameSpawning.cpp
    // __LINE__: 379
    UTIL_ASSERT(pSpawningObject != NULL);

    *pSpawningObject = new CAreaFileRandomMonsterSpawningPoint;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameSpawning.cpp
    // __LINE__: 382
    UTIL_ASSERT(*pSpawningObject != NULL);

    memcpy(*pSpawningObject, &m_spawningObject, sizeof(CAreaFileRandomMonsterSpawningPoint));
}
