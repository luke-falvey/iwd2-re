#include "CVisualEffect.h"

#include "CBaldurChitin.h"
#include "CGameObjectArray.h"
#include "CInfGame.h"
#include "CParticle.h"

// 0x5544C0
CResHelperVEF::CResHelperVEF(CResRef cNewResRef)
{
    // TODO: Incomplete.
}

// 0x5545E0
CVisualEffect* CResHelperVEF::FUN_005545e0()
{
    // TODO: Incomplete.
    return NULL;
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
