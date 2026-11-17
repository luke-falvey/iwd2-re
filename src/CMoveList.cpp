#include "CMoveList.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameObjectArray.h"
#include "CGameSprite.h"
#include "CInfGame.h"

// 0x5170A0
CMoveListEntry::CMoveListEntry()
{
    m_nSpriteId = CGameObjectArray::INVALID_INDEX;
    m_areaResRef = "";
    m_ptDestination.x = 0;
    m_ptDestination.y = 0;
    m_nFacing = 0;
    m_nDelay = 0;
}

// NOTE: Inlined.
CMoveList::CMoveList()
{
}

// 0x59EAD0
CMoveList::~CMoveList()
{
    ClearAll();
}

// 0x4E71B0
void CMoveList::ClearAll()
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CMoveListEntry* pEntry = GetNext(pos);
        delete pEntry;
    }
    RemoveAll();
}

// 0x5170F0
void CMoveList::sub_5170F0(CGameArea* pArea)
{
    ITEM_EFFECT effect;
    CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_JUMPTOAREA);

    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CMoveListEntry* pNode = GetNext(pos);

        effect.dwFlags = pNode->m_nFacing;
        effect.effectAmount = pNode->m_nDelay;
        pNode->m_areaResRef.GetResRef(effect.res);

        // NOTE: Original reads it from a constant at 0x8491AA.
        effect.durationType = 1;

        CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
            pNode->m_ptDestination,
            pNode->m_nSpriteId,
            CPoint(-1, -1));

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(pNode->m_nSpriteId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pArea->field_AAA.AddTail(reinterpret_cast<int*>(static_cast<INT_PTR>(pSprite->GetId())));
            pSprite->AddEffect(pEffect, CGameAIBase::EFFECT_LIST_TIMED, TRUE, FALSE);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pNode->m_nSpriteId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x517210
void CMoveList::CheckLoad(CGameArea* pArea)
{
    CResRef areaResRef = pArea->m_resRef;

    POSITION posCurrent = GetHeadPosition();
    POSITION pos = posCurrent;
    while (pos != NULL) {
        CMoveListEntry* pNode = GetNext(pos);
        if (pNode->m_areaResRef == areaResRef) {
            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(pNode->m_nSpriteId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                // NOTE: Original skips the `posCurrent` update here, so it
                // goes stale.
                continue;
            }

            pSprite->MoveOntoArea(pArea, pNode->m_ptDestination, pNode->m_nFacing);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pNode->m_nSpriteId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            // NOTE: The entry is removed without being deleted (leak).
            RemoveAt(posCurrent);
        }

        posCurrent = pos;
    }
}

// 0x517300
void CMoveList::AIUpdate()
{
    POSITION posCurrent = GetHeadPosition();
    POSITION pos = posCurrent;
    while (pos != NULL) {
        CMoveListEntry* pEntry = GetNext(pos);

        if (pEntry->m_nDelay > 0) {
            pEntry->m_nDelay--;
        }

        if (pEntry->m_nDelay <= 0) {
            RemoveAt(posCurrent);

            CString sAreaName;
            pEntry->m_areaResRef.CopyToString(sAreaName);

            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sAreaName);
            if (pArea != NULL) {
                CGameSprite* pSprite;

                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(pEntry->m_nSpriteId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    // NOTE: Original skips updating `posCurrent` here.
                    continue;
                }

                pSprite->MoveOntoArea(pArea, pEntry->m_ptDestination, pEntry->m_nFacing);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pEntry->m_nSpriteId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                delete pEntry;
            } else {
                // NOTE: Adds to the game's list rather than `this`.
                g_pBaldurChitin->GetObjectGame()->m_cMoveList.AddTail(pEntry);
            }
        }

        posCurrent = pos;
    }
}
