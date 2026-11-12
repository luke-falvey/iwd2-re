#include "CVisualEffect.h"

#include "CBaldurChitin.h"
#include "CGameObjectArray.h"
#include "CInfGame.h"

// 0x554D20
LONG CVisualEffect::sub_554D20(CString sVefName, CGameArea* pArea, CPoint& ptStart, LONG target, CPoint& targetPos, LONG nHeight)
{
    // TODO: Incomplete.
    return -1;
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
