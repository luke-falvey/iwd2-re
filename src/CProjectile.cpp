#include "CProjectile.h"

#include "CBaldurChitin.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CTimerWorld.h"
#include "CUtil.h"

// 0x6A3130
BOOLEAN CProjectile::IsProjectile()
{
    return TRUE;
}

// 0x529490
CProjectile::CProjectile()
{
    field_186 = 0;
    m_projectileType = 0;
    m_speed = 0;
    m_sourceId = 0;
    m_targetId = 0;
    field_9A = 0;
    field_E0 = 0;
    field_E6 = 0;
    field_EA = NULL;
    field_16C = 0;
    field_16E = 0;
    field_170 = 0;
    field_17C = 0;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_bHasHeight = FALSE;
    field_C4 = 0;
    field_E2 = 0;
    m_fireSoundRef = "";
    m_loopFireSound = FALSE;
    m_arrivalSoundRef = "";
    m_loopArrivalSound = FALSE;
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x51E9B0
CProjectile::~CProjectile()
{
    // NOTE: Uninline.
    ClearEffects();
}

// 0x536F40
BOOLEAN CProjectile::CompressTime(DWORD deltaTime)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
    // __LINE__: 10139
    UTIL_ASSERT(deltaTime > static_cast<DWORD>(CTimerWorld::TIMESCALE_MSEC_PER_SEC * CTimerWorld::TIMESCALE_SEC_PER_MIN));

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }

    return FALSE;
}

// 0x5551B0
void CProjectile::RemoveSelf()
{
    RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x51EA90
void CProjectile::AddEffect(CGameEffect* pEffect)
{
    pEffect->m_projectileType = m_projectileType;
    m_effectList.AddTail(pEffect);
}

// 0x51EAF0
CProjectile* CProjectile::DecodeProjectile(WORD projectileType, CGameAIBase* pSource, BOOL a3)
{
    // TODO: Incomplete.

    return NULL;
}

// 0x529F10
void CProjectile::ClearEffects()
{
    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* node = m_effectList.GetNext(pos);
        delete node;
    }
    m_effectList.RemoveAll();
}

// 0x529F40
LONG CProjectile::DetermineHeight(CGameSprite* pSprite)
{
    if (!m_bHasHeight) {
        return 0;
    }

    if (pSprite->GetObjectType() != TYPE_SPRITE) {
        return 32;
    }

    return pSprite->GetAnimation()->GetCastHeight();
}

// 0x78E740
void CProjectile::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
}

// 0x529FB0
void CProjectile::OnArrival()
{
    CProjectile* pProjectile;
    BYTE rc;

    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_callBackProjectile,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        pProjectile->CallBack();

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_callBackProjectile,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    // NOTE: Uninline.
    PlaySound(m_arrivalSoundRef, m_loopArrivalSound, TRUE);

    if (field_182 != CGameObjectArray::INVALID_INDEX) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(field_182,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pProjectile->RemoveSelf();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(field_182,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    DeliverEffects();
    RemoveFromArea();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x52A1A0
void CProjectile::DeliverEffects()
{
    // TODO: Incomplete.
}

// 0x52A480
SHORT CProjectile::GetDirection(CPoint target)
{
    CPoint ptStart;
    ptStart.x = m_pos.x;
    ptStart.y = 4 * m_pos.y / 3;

    CPoint ptTarget;
    ptTarget.x = target.x;
    ptTarget.y = 4 * target.y / 3;

    return CGameSprite::GetDirection(ptStart, ptTarget);
}

// 0x52A4E0
void CProjectile::PlaySound(CResRef resRef, BOOL loop, BOOL fireAndForget)
{
    m_sound.Stop();
    if (resRef != "") {
        m_sound.SetResRef(resRef, TRUE, TRUE);
        if (loop) {
            m_sound.SetLoopingFlag(TRUE);
        }
        if (fireAndForget) {
            m_sound.SetFireForget(TRUE);
        }
        m_sound.SetChannel(15, reinterpret_cast<DWORD>(field_EA));
        m_sound.Play(m_pos.x, m_pos.y, 0, FALSE);
    }
}

// 0x52A640
void CProjectile::sub_52A640(LONG nObjectId, CPoint& pt, BOOLEAN a3)
{
    // TODO: Incomplete.
}

// 0x560310
CProjectile* CProjectile::sub_560310(WORD projectileType, CGameAIBase* pSource, BOOL a3)
{
    // TODO: Incomplete.

    return NULL;
}

// 0x78E730
void CProjectile::CallBack()
{
}
