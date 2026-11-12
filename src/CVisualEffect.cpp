#include "CVisualEffect.h"

#include "CBaldurChitin.h"
#include "CGameObjectArray.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CParticle.h"
#include "CVidCell.h"

// 0x84ED40
static const SHORT TABLE_84ED40[16] = { 1000, 924, 707, 383, 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924 };

// 0x84ED60
static const SHORT TABLE_84ED60[16] = { 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924, 1000, 924, 707, 383 };

// 0x84ED80
static const SHORT TABLE_84ED80[16] = { 0, 383, 707, 924, 1000, 924, 707, 383, 0, -383, -707, -924, -1000, -924, -707, -383 };

// 0x84EDA0
static const SHORT TABLE_84EDA0[16] = { 1000, 924, 707, 383, 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924 };

// 0x84EDF8
static LONG dword_84EDF8 = 30;

// NOTE: Inlined.
CVisualEffectComponent::CVisualEffectComponent()
{
    memset(this, 0, sizeof(*this));
}

// 0x554870
CVisualEffect::CVisualEffect()
    : m_lChildren(10)
    , field_24A(10)
    , field_266(10)
{
    // NOTE: Clears everything from `field_6E` up to `m_lChildren`.
    memset(field_6E, 0, 0x1C0);

    field_282 = 0;
    field_286 = 0;
    field_28A = CGameObjectArray::INVALID_INDEX;
    field_28E = 0;
    field_292 = 0;
    field_296.x = 0;
    field_296.y = 0;
    field_2AA = 0;
    field_2AE = 0;
    field_8A = 0;
    field_7E = 0;
    field_29E = 0;
    field_2A2 = 0;
    field_2A6 = 0;
    field_8E = 0;
    field_82 = -1;
    m_dwFlags = -1;

    // NOTE: This assignment is slightly incorrect. Original code refers to
    // another copy of the table (0x8A8168), which has the same values as
    // `CGameObject::DEFAULT_TERRAIN_TABLE`.
    memcpy(field_2B2, CGameObject::DEFAULT_TERRAIN_TABLE, 16);

    m_AISpeed = CGameObject::AI_SPEED_HASTED;
}

// 0x5544C0
CResHelperVEF::CResHelperVEF(CResRef cNewResRef)
{
    SetResRef(cNewResRef, TRUE, TRUE);
}

// 0x5545E0
CVisualEffect* CResHelperVEF::FUN_005545e0()
{
    if (pRes == NULL) {
        return NULL;
    }

    BYTE* pData = reinterpret_cast<BYTE*>(pRes->Demand());
    if (pData == NULL) {
        pRes->Release();
        return NULL;
    }

    const VEF_HEADER* pHeader = reinterpret_cast<const VEF_HEADER*>(pData);

    CVisualEffect* pVisualEffect = new CVisualEffect();

    // NOTE: Both component tables are appended to the same list.
    for (DWORD nIndex = 0; nIndex < pHeader->nComponents1; nIndex++) {
        const CVisualEffectComponent* pSrc = reinterpret_cast<const CVisualEffectComponent*>(pData + pHeader->nComponentsOffset1 + nIndex * sizeof(CVisualEffectComponent));

        CVisualEffectComponent* pComponent = new CVisualEffectComponent();
        pComponent->field_0 = pSrc->field_0;
        pComponent->field_4 = pSrc->field_4;
        pComponent->field_8 = pSrc->field_8;
        pComponent->field_C = pSrc->field_C;
        pComponent->cResRef = pSrc->cResRef;
        for (int k = 0; k < 50; k++) {
            pComponent->field_18[k] = pSrc->field_18[k];
        }
        pComponent->cResRef.MakeUpper();

        pVisualEffect->field_24A.AddTail(pComponent);
    }

    for (DWORD nIndex = 0; nIndex < pHeader->nComponents2; nIndex++) {
        const CVisualEffectComponent* pSrc = reinterpret_cast<const CVisualEffectComponent*>(pData + pHeader->nComponentsOffset2 + nIndex * sizeof(CVisualEffectComponent));

        CVisualEffectComponent* pComponent = new CVisualEffectComponent();
        pComponent->field_0 = pSrc->field_0;
        pComponent->field_4 = pSrc->field_4;
        pComponent->field_8 = pSrc->field_8;
        pComponent->field_C = pSrc->field_C;
        pComponent->cResRef = pSrc->cResRef;
        for (int k = 0; k < 50; k++) {
            pComponent->field_18[k] = pSrc->field_18[k];
        }
        pComponent->cResRef.MakeUpper();

        pVisualEffect->field_24A.AddTail(pComponent);
    }

    pVisualEffect->field_7E = pHeader->field_18;
    pVisualEffect->field_82 = pHeader->field_1C;
    pVisualEffect->m_dwFlags = pHeader->field_20;

    pRes->Release();

    return pVisualEffect;
}
// 0x554D20
LONG CVisualEffect::sub_554D20(CString sVefName, CGameArea* pArea, CPoint& ptStart, LONG target, CPoint& targetPos, LONG nHeight)
{
    LONG x;
    LONG y;

    if (targetPos.x == -1 && targetPos.y == -1) {
        x = ptStart.x;
        y = ptStart.y;
    } else {
        x = targetPos.x;
        y = targetPos.y;
    }

    sVefName.MakeUpper();

    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(sVefName), 0x3FC, TRUE) == NULL) {
        return CGameObjectArray::INVALID_INDEX;
    }

    CResHelperVEF cResHelper((CResRef(sVefName)));

    CVisualEffect* pVisualEffect = cResHelper.FUN_005545e0();
    if (pVisualEffect != NULL) {
        pVisualEffect->field_28A = target;
        pVisualEffect->field_282 = x;
        pVisualEffect->field_286 = y;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&(pVisualEffect->m_id),
            pVisualEffect,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            delete pVisualEffect;
        } else {
            // NOTE: Direct call of the base implementation.
            pVisualEffect->CGameObject::AddToArea(pArea, ptStart, nHeight, CGameObject::LIST_FRONT);

            pVisualEffect->field_28E = pVisualEffect->m_pos.x << CParticle::RESOLUTION_INC;
            pVisualEffect->field_292 = (pVisualEffect->m_pos.y * 4 / 3) << CParticle::RESOLUTION_INC;
        }
    }

    // NOTE: When `Add` failed the object has been deleted above, but its id
    // is still read here.
    if (pVisualEffect != NULL) {
        return pVisualEffect->m_id;
    }

    return CGameObjectArray::INVALID_INDEX;
}

// 0x5513F0
void CVisualEffectObject::sub_5513F0(LONG nCounter)
{
    BOOL bUpdate;
    if (field_9A > dword_84EDF8 / 2) {
        LONG nPeriod = dword_84EDF8 / (dword_84EDF8 - field_9A);
        bUpdate = nCounter % nPeriod != 0;
    } else {
        LONG nPeriod = dword_84EDF8 / field_9A;
        bUpdate = nCounter % nPeriod == 0;
    }

    if (bUpdate) {
        if ((m_dwFlags & 0x1) == 0) {
            if (field_252->IsEndOfSequence(FALSE)) {
                field_338 = 1;
            }
        }

        field_252->FrameAdvance();

        if (field_256 != NULL) {
            field_256->FrameAdvance();
        }
    }

    if (nCounter > field_C2 && field_C2 > 0) {
        field_338 = 1;
    }
}

// 0x551490
void CVisualEffectObject::sub_551490(const CPoint* pt)
{
    LONG dx = field_8E;
    LONG dy = field_92;

    if (field_96 & 0x1) {
        LONG nDirection = field_33C;
        LONG nNewX = (TABLE_84ED40[nDirection] * dx + TABLE_84ED60[nDirection] * dy) / 1000;
        LONG nNewY = (TABLE_84ED80[nDirection] * dx + TABLE_84EDA0[nDirection] * dy) / 1000;
        dx = nNewX;
        dy = nNewY;
    }

    m_pos.x = pt->x + dx;
    m_pos.y = pt->y + dy;
}

// 0x551520
void CVisualEffectObject::sub_551520(const CPoint* ptPos, const CPoint* ptPrevPos, const CPoint* ptTarget, SHORT nDirection)
{
    if (field_A6 & 0x1) {
        if (m_pos.x == ptTarget->x && m_pos.y == ptTarget->y) {
            field_33C = static_cast<SHORT>(field_A2);
            return;
        }

        CPoint ptFrom(m_pos.x, m_pos.y * 4 / 3);
        CPoint ptTo(ptTarget->x, ptTarget->y * 4 / 3);
        field_33C = CGameSprite::GetDirection(ptFrom, ptTo);
    } else if (field_A6 & 0x2) {
        field_33C = nDirection;
    } else {
        if (ptPrevPos->x == ptPos->x && ptPrevPos->y == ptPos->y) {
            field_33C = static_cast<SHORT>(field_A2);
            return;
        }

        CPoint ptFrom(ptPrevPos->x, ptPrevPos->y * 4 / 3);
        CPoint ptTo(ptPos->x, ptPos->y * 4 / 3);
        field_33C = CGameSprite::GetDirection(ptFrom, ptTo);
    }
}

// 0x551670
SHORT CVisualEffectObject::sub_551670(const CPoint* ptFrom, const CPoint* ptTo, SHORT nDirection)
{
    if (ptFrom->x == ptTo->x && ptFrom->y == ptTo->y) {
        return nDirection;
    }

    CPoint pt1(ptFrom->x, ptFrom->y * 4 / 3);
    CPoint pt2(ptTo->x, ptTo->y * 4 / 3);
    return CGameSprite::GetDirection(pt1, pt2);
}

// 0x5549D0
// NOTE: 0x5549B0 is the scalar deleting destructor (compiler-generated).
CVisualEffect::~CVisualEffect()
{
    POSITION pos = m_lChildren.GetHeadPosition();
    while (pos != NULL) {
        POSITION posCurrent = pos;
        LONG nObjectId = reinterpret_cast<LONG>(m_lChildren.GetNext(pos));

        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nObjectId,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            m_lChildren.RemoveAt(posCurrent);
            delete pObject;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nObjectId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    while (field_24A.GetCount() != 0) {
        delete field_24A.RemoveHead();
    }

    while (field_266.GetCount() != 0) {
        delete field_266.RemoveHead();
    }
}

// 0x554B60
void CVisualEffect::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
}

// 0x554B70
void CVisualEffect::AIUpdate()
{
    CPoint pt(field_282, field_286);

    sub_554F00(&pt);

    POSITION pos = m_lChildren.GetHeadPosition();
    POSITION posCurrent = pos;
    while (pos != NULL) {
        LONG nObjectId = reinterpret_cast<LONG>(m_lChildren.GetNext(pos));

        CVisualEffectObject* pChild;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nObjectId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pChild),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pChild->m_dwFlags & 0x20) {
                m_lChildren.RemoveAt(posCurrent);
                delete pChild;

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nObjectId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                // NOTE: Original skips the update of `posCurrent` below, so
                // it still holds the removed node on the next iteration.
                continue;
            }

            pChild->sub_551520(&m_pos, &field_296, &pt, 0);
            pChild->sub_551490(&m_pos);

            if ((field_9A & 0x1) && (pChild->m_dwFlags & 0x4)) {
                pChild->m_posZ = m_posZ;
            }

            pChild->sub_5513F0(field_29E);

            if (pChild->field_338 != 0) {
                m_lChildren.RemoveAt(posCurrent);
                delete pChild;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nObjectId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        posCurrent = pos;
    }

    field_29E++;
    field_296.x = m_pos.x;
    field_296.y = m_pos.y;
}

// 0x554F00
void CVisualEffect::sub_554F00(CPoint* pt)
{
    LONG y = m_pos.y * 4 / 3;
    LONG ty = pt->y * 4 / 3;

    SHORT nDirection = CVisualEffectObject::sub_551670(&m_pos, pt, static_cast<WORD>(field_2A6));

    LONG dy = ty - y;
    LONG dx = pt->x - m_pos.x;
    field_2A6 = nDirection;

    if (dx * dx + dy * dy * 16 / 9 > (field_82 + 1) * (field_82 + 1)) {
        LONG nNewA = (TABLE_84ED40[nDirection] * field_8A + TABLE_84ED60[nDirection] * field_8E) / 1000;
        LONG nNewB = (TABLE_84ED80[nDirection] * field_8A + TABLE_84EDA0[nDirection] * field_8E) / 1000;

        // NOTE: Uses the unscaled `dy`.
        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);
        if (nDistance == 0) {
            field_2AA = 1;
            field_2AE = 1;
        } else {
            field_2AA = (dx << CParticle::RESOLUTION_INC) * field_82 / nDistance;
            field_2AE = (dy << CParticle::RESOLUTION_INC) * field_82 / nDistance;
            field_2AA += nNewA;
            field_2AE += nNewB;
        }

        field_28E += field_2AA;
        field_292 += field_2AE;

        if (field_8A < 0) {
            if (field_8A > -field_92) {
                field_8A = 0;
            } else {
                field_8A += field_92;
            }
        } else if (field_8A > 0) {
            if (field_8A < field_92) {
                field_8A = 0;
            } else {
                field_8A -= field_92;
            }
        }

        if (field_8E < 0) {
            if (field_8E > -field_92) {
                field_8E = 0;
            } else {
                field_8E += field_92;
            }
        } else if (field_8E > 0) {
            if (field_8E < field_92) {
                field_8E = 0;
            } else {
                field_8E -= field_92;
            }
        }

        m_pos.x = field_28E >> CParticle::RESOLUTION_INC;
        m_pos.y = (field_292 * 3 / 4) >> CParticle::RESOLUTION_INC;
    }
}

// 0x555200
void CVisualEffect::SetAllDelete()
{
    POSITION pos = m_lChildren.GetHeadPosition();
    while (pos != NULL) {
        LONG nObjectId = reinterpret_cast<LONG>(m_lChildren.GetNext(pos));

        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nObjectId,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            reinterpret_cast<CVisualEffect*>(pObject)->m_dwFlags |= 0x20;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nObjectId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}
