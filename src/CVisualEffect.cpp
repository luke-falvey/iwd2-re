#include "CVisualEffect.h"

#include "CBaldurChitin.h"
#include "CGameObjectArray.h"
#include "CInfGame.h"
#include "CParticle.h"

// NOTE: Inlined.
CVisualEffectComponent::CVisualEffectComponent()
{
    memset(this, 0, sizeof(*this));
}

// 0x402940
void* CResBinary::Demand()
{
    void* pData = CRes::Demand();

    if (field_54 == 0 || CRes::GetDemands() <= 1) {
        if (pData != NULL) {
            field_50 = reinterpret_cast<BYTE*>(pData) + 4;
            field_54 = 1;
        }

        if (field_54 == 0) {
            return NULL;
        }
    }

    return pData;
}

// 0x402980
int CResBinary::Release()
{
    int rc = CRes::Release();

    if (CRes::GetDemands() <= 0) {
        field_54 = 0;
    }

    return rc;
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
    field_296 = 0;
    field_29A = 0;
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
