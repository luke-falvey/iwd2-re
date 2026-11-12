#include "CProjectilePart2.h"

#include <math.h>
#include <stdlib.h>

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameChunk.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CMessage.h"
#include "CParticle.h"
#include "CUtil.h"


// 0x84D6B8
static const SHORT TABLE_84D6B8[16] = { -1000, -924, -707, -383, 0, 383, 707, 924, 1000, 924, 707, 383, 0, -383, -707, -924 };
// 0x84D6D8
static const SHORT TABLE_84D6D8[16] = { 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924, 1000, 924, 707, 383 };
// 0x84D6F8
static const SHORT TABLE_84D6F8[16] = { 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924, 1000, 924, 707, 383 };
// 0x84D718
static const SHORT TABLE_84D718[16] = { 1000, 924, 707, 383, 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924 };
// 0x84D738
static const SHORT TABLE_84D738[16] = { 1000, 924, 707, 383, 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924 };
// 0x84D758
static const SHORT TABLE_84D758[16] = { 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924, 1000, 924, 707, 383 };
// 0x84D778
static const SHORT TABLE_84D778[16] = { 0, 383, 707, 924, 1000, 924, 707, 383, 0, -383, -707, -924, -1000, -924, -707, -383 };
// 0x84D798
static const SHORT TABLE_84D798[16] = { 1000, 924, 707, 383, 0, -383, -707, -924, -1000, -924, -707, -383, 0, 383, 707, 924 };

// 0x5309C0
CProjectileBAM84E030::CProjectileBAM84E030(WORD a1, CResRef resRef, SHORT nCount, WORD a4)
    : CProjectileBAM(resRef)
{
    m_pVidCell->SequenceSet(0);
    field_2A0 = nCount;
    field_2BE = a1;
    field_E6 |= 0x8;

    switch (a4) {
    case 0:
        break;
    case 1:
        m_palette.SetRange(0, 33, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_pVidCell->SetPalette(m_palette);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 7083
        UTIL_ASSERT(FALSE);
    }

    for (SHORT index = 0; index < nCount; index++) {
        CProjectile* pProjectile = CProjectile::DecodeProjectile(field_2BE + 1, NULL, FALSE);
        if (index != 0) {
            pProjectile->m_fireSoundRef = CResRef("");
        }
        field_2A2.AddTail(pProjectile);
    }

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x530B60
CProjectileBAM84E030::~CProjectileBAM84E030()
{
}

// 0x530C90
void CProjectileBAM84E030::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pObject;
    BYTE rc;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(target,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    CPoint ptTarget = pObject->GetPos();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(target,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(source,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    m_pos = pObject->GetPos();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(source,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    LONG dy = (ptTarget.y * 4) / 3 - (m_pos.y * 4) / 3;
    LONG dx = ptTarget.x - m_pos.x;

    LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dy * dy + dx * dx)) + 0.5);

    LONG nStepX;
    LONG nStepY;
    if (nDistance == 0) {
        nStepX = 1;
        nStepY = 1;
    } else {
        nStepX = (dx << CParticle::RESOLUTION_INC) * m_speed / nDistance;
        nStepY = (dy << CParticle::RESOLUTION_INC) * m_speed / nDistance;
    }

    BYTE nIndex = 1;
    LONG nLength = static_cast<LONG>(sqrt(static_cast<double>(nStepY * nStepY + nStepX * nStepX)));

    POSITION pos = field_2A2.GetHeadPosition();
    while (pos != NULL) {
        CProjectile* pProjectile = field_2A2.GetNext(pos);

        POSITION posEffect = m_effectList.GetHeadPosition();
        while (posEffect != NULL) {
            CGameEffect* pEffect = m_effectList.GetNext(posEffect);
            // NOTE: Inlined AddEffect.
            pProjectile->AddEffect(pEffect->Copy());
        }

        pProjectile->m_speed = pProjectile->m_speed + rand() % 20 - 10;
        pProjectile->field_186 = field_186;
        pProjectile->field_18A = field_18A;
        pProjectile->Fire(pArea, source, target, targetPos, 0, nType);

        CProjectile* pProjectile2 = NULL;
        if (pos != NULL) {
            pProjectile2 = field_2A2.GetNext(pos);

            POSITION posEffect2 = m_effectList.GetHeadPosition();
            while (posEffect2 != NULL) {
                CGameEffect* pEffect = m_effectList.GetNext(posEffect2);
                // NOTE: Inlined AddEffect.
                pProjectile2->AddEffect(pEffect->Copy());
            }

            pProjectile2->m_speed = pProjectile2->m_speed + rand() % 20 - 10;
            pProjectile2->field_186 = field_186;
            pProjectile2->field_18A = field_18A;
            pProjectile2->Fire(pArea, source, target, targetPos, 0, nType);
        }

        if (pProjectile2 != NULL) {
            pProjectile->field_AC.x = nIndex * nStepY;
            pProjectile->field_AC.y = -(nIndex * nStepX);
            pProjectile->field_E0 = (nIndex * nLength) / 40;
            if (pProjectile->field_AC.x == 0 && pProjectile->field_AC.y == 0) {
                pProjectile->field_C4 = 0;
            } else {
                pProjectile->field_C4 = 1;
            }

            pProjectile2->field_AC.x = -pProjectile->field_AC.x;
            pProjectile2->field_AC.y = nIndex * nStepX;
            pProjectile2->field_E0 = (nIndex * nLength) / 40;
            if (pProjectile2->field_AC.x == 0 && pProjectile2->field_AC.y == 0) {
                pProjectile2->field_C4 = 0;
            } else {
                pProjectile2->field_C4 = 1;
            }
            nIndex++;
        } else {
            pProjectile->field_AC.x = 0;
            pProjectile->field_AC.y = 0;
            pProjectile->field_E0 = m_speed;
            pProjectile->field_C4 = 0;
        }
    }

    field_2A2.RemoveAll();

    delete this;
}

// 0x531120
CProjectileBAM84E0C4::CProjectileBAM84E0C4(SHORT nCount, WORD a2)
    : CProjectileBAM84E030(217, CResRef((BYTE*)"SPMAGMIS"), nCount, a2)
{
    field_1BE = 0;
    field_1C2 = 0;
    field_1D4 = 0;
    field_1CA = 10;
    field_1CE = 7;
    field_1C6 = 1;
    field_1D2 = 128;
    field_1D8 = 1;
    m_fireSoundRef = "";
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531360
CProjectileBAM84E14C::CProjectileBAM84E14C()
    : CProjectileBAM(CResRef((BYTE*)"BOLT"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 5;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531570
CProjectileBAM84E1D4::CProjectileBAM84E1D4()
    : CProjectileBAM(CResRef((BYTE*)"BOULDER"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed = 3 * m_speed / 2;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531790
CProjectileBAM84E25C::CProjectileBAM84E25C()
    : CProjectileBAM(CResRef((BYTE*)"DART"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 3;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x5319A0
CProjectileBAM84E2E4::CProjectileBAM84E2E4()
    : CProjectileBAM(CResRef((BYTE*)"MAGICSTN"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 27, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 1;
    m_fireSoundRef = "";
    m_speed *= 5;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531BE0
CProjectileBAM84E36C::CProjectileBAM84E36C()
    : CProjectileBAM(CResRef((BYTE*)"AXE"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 2;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x531DF0
CProjectileBAM84E3F4::CProjectileBAM84E3F4()
    : CProjectileBAM(CResRef((BYTE*)"SPEAR"))
{
    m_pVidCell->SequenceSet(0);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 2;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x532000
CProjectileBAM84E47C::CProjectileBAM84E47C()
    : CProjectileBAM(CResRef((BYTE*)"DAGGER"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(5, 27, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(4, 30, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 2;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x532260
CProjectileBAM84E504::CProjectileBAM84E504()
    : CProjectileBAM(CResRef((BYTE*)"ACIDBLOB"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 73, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x5324A0
CProjectileBAM84E58C::CProjectileBAM84E58C()
    : CProjectileBAM(CResRef((BYTE*)"ARARROW"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(5, 27, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(4, 23, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(2, 46, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 1;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 5;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x532720
CProjectileBAM84E614::CProjectileBAM84E614()
    : CProjectileBAM(CResRef((BYTE*)"SPFLMARR"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 67, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(1, 46, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 5;
    field_2A0[7] = 0;
    memset(field_2A0, 'A', 7);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x5329A0
void CProjectileBAM84E614::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    CPoint ptZero(0, 0);
    CString sEmpty("");

    field_2A0[7]++;
    if (field_2A0[7] == 1) {
        field_2A0[7] = 0;

        // NOTE: The objects are not kept.
        LONG r1 = rand() % 5;
        LONG r2 = rand() % 5;
        CPoint ptPos1(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x * 4 / 3,
            ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y * 4 / 3);
        new CGame85BF7C(0x301, field_2A0, sEmpty, field_EA, ptPos1, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);

        r1 = rand() % 5;
        r2 = rand() % 5;
        CPoint ptPos2(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x,
            ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y);
        new CGame85BF7C(0x301, field_2A0, sEmpty, field_EA, ptPos2, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);

        r1 = rand() % 5;
        r2 = rand() % 5;
        CPoint ptPos3(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x * 2 / 3,
            ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y * 2 / 3);
        new CGame85BF7C(0x301, field_2A0, sEmpty, field_EA, ptPos3, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);
    }

    CProjectileBAM::AIUpdate();
}

// 0x532CA0
CProjectileBAM84E69C::CProjectileBAM84E69C()
    : CProjectileBAM84E69CBase(CResRef((BYTE*)"SPBRNHND"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 71, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_E6 |= 0x8;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    field_2A0 = 55;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    field_174 = 23;
    field_178 = 50;
}

// 0x533A50
void CProjectileBAM84E69CBase::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    field_2A0--;
    m_pVidCell->FrameAdvance();

    if (field_2A0 <= 0) {
        RemoveFromArea();

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            NULL,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            delete this;
        }
    }
}

// 0x532F00
void CProjectileBAM84E69C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pObject;
    BYTE rc;

    m_targetId = target;
    m_sourceId = source;
    field_EA = pArea;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    CPoint ptStart;
    sub_52A640(m_sourceId, ptStart, TRUE);

    // NOTE: The position stays at `ptStart` when the target is within 10 units.
    CPoint ptPos;
    ptPos.x = ptStart.x;
    if (targetPos.x - ptStart.x < -10) {
        ptPos.x = ptStart.x - 10;
    } else if (targetPos.x - ptStart.x > 10) {
        ptPos.x = ptStart.x + 10;
    }

    ptPos.y = ptStart.y;
    if (targetPos.y - ptStart.y < -10) {
        ptPos.y = ptStart.y - 10;
    } else if (targetPos.y - ptStart.y > 10) {
        ptPos.y = ptStart.y + 10;
    }

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        LONG nSourceId = m_sourceId;
        delete this;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nSourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    AddToArea(pArea, ptPos, nHeight, CGameObject::LIST_FRONT);

    // NOTE: Inlined PlaySound.
    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    m_pos = ptPos;
    m_posDest.x = targetPos.x;
    m_posExact.x = ptPos.x << CParticle::RESOLUTION_INC;
    m_posDest.y = targetPos.y;
    m_posExact.y = 4 * (ptPos.y << CParticle::RESOLUTION_INC) / 3;

    CPoint ptFrom(m_pos.x, m_pos.y * 4 / 3);
    CPoint ptTo(targetPos.x, targetPos.y * 4 / 3);
    m_nDirection = CGameSprite::GetDirection(ptFrom, ptTo);

    LONG dx = m_pos.x - m_posDest.x;
    LONG dy = m_pos.y - m_posDest.y;
    LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)));

    sub_533600(&m_pos, nDistance / 32, m_nDirection, FALSE);

    SHORT n = static_cast<SHORT>(nDistance / 32);

    CPoint pt[3];
    pt[0] = m_pos;
    pt[1].x = pt[0].x + (TABLE_84D6D8[m_nDirection] * 32 + TABLE_84D6B8[m_nDirection] * 14) * n / 1000;
    pt[1].y = pt[0].y + (TABLE_84D718[m_nDirection] * 32 + TABLE_84D6F8[m_nDirection] * 14) * n / 1000;
    pt[1].x += (field_174 * TABLE_84D6B8[m_nDirection] + field_178 * TABLE_84D6D8[m_nDirection]) * n / 1000;
    pt[1].y += (field_174 * TABLE_84D6F8[m_nDirection] + field_178 * TABLE_84D718[m_nDirection]) * n / 1000;
    pt[2].x = pt[0].x + (TABLE_84D758[m_nDirection] * 32 + TABLE_84D738[m_nDirection] * 14) * n / 1000;
    pt[2].y = pt[0].y + (TABLE_84D798[m_nDirection] * 32 + TABLE_84D778[m_nDirection] * 14) * n / 1000;
    pt[2].x += (field_174 + field_178) * TABLE_84D738[m_nDirection] * n / 1000;
    pt[2].y += (field_174 + field_178) * TABLE_84D778[m_nDirection] * n / 1000;

    CRect rBounding;
    rBounding.left = min(pt[0].x, min(pt[1].x, pt[2].x));
    rBounding.top = min(pt[0].y, min(pt[1].y, pt[2].y));
    rBounding.right = max(pt[0].x, max(pt[1].x, pt[2].x));
    rBounding.bottom = max(pt[0].y, max(pt[1].y, pt[2].y));

    CTypedPtrList<CPtrList, LONG*> targets(10);

    field_EA->GetAllInPoly(rBounding,
        pt,
        3,
        CAIObjectType::ANYONE,
        field_EA->m_visibleTerrainTable,
        targets,
        TRUE);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nTarget = reinterpret_cast<LONG>(targets.GetNext(pos));

        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nTarget,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if ((pObject->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
                POSITION posEffect = m_effectList.GetHeadPosition();
                while (posEffect != NULL) {
                    CGameEffect* pEffect = m_effectList.GetNext(posEffect);

                    CMessage* message = new CMessageAddEffect(pEffect->Copy(), m_sourceId, nTarget);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTarget,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    DeliverEffects();
}

// 0x533600
void CProjectileBAM84E69C::sub_533600(CPoint* pt, SHORT nCount, SHORT nDirection, BOOL a4)
{
    CPoint pt1;
    pt1.x = pt->x + static_cast<SHORT>((TABLE_84D6D8[nDirection] * 32 + TABLE_84D6B8[nDirection] * 14) / 1000);
    pt1.y = pt->y + static_cast<SHORT>((TABLE_84D718[nDirection] * 32 + TABLE_84D6F8[nDirection] * 14) / 1000);

    CPoint pt2;
    pt2.x = pt->x + static_cast<SHORT>((TABLE_84D758[nDirection] * 32 + TABLE_84D738[nDirection] * 14) / 1000);
    pt2.y = pt->y + static_cast<SHORT>((TABLE_84D798[nDirection] * 32 + TABLE_84D778[nDirection] * 14) / 1000);

    CProjectileBAM84E724* pProjectile = new CProjectileBAM84E724(m_pVidCell->cResRef, field_2A0, nDirection);
    pProjectile->m_pVidCell->SetPalette(m_palette);

    // NOTE: Dereferences `pProjectile` without checking for NULL.
    pProjectile->m_bHasHeight = m_bHasHeight;
    pProjectile->Virtual88(field_EA, &pt1, m_posZ);

    if (a4 == FALSE) {
        pProjectile = new CProjectileBAM84E724(m_pVidCell->cResRef, field_2A0, nDirection);
        pProjectile->m_pVidCell->SetPalette(m_palette);

        // NOTE: Dereferences `pProjectile` without checking for NULL.
        pProjectile->m_bHasHeight = m_bHasHeight;
        pProjectile->Virtual88(field_EA, &pt2, m_posZ);
    }

    if (nCount - 1 > 0) {
        sub_533600(&pt1, nCount - 1, nDirection, TRUE);
        if (a4 == FALSE) {
            sub_533600(&pt2, nCount - 1, nDirection, FALSE);
        }
    }
}

// NOTE: Inlined in 0x533600 (not a separate function in the original).
CProjectileBAM84E724::CProjectileBAM84E724(CResRef resRef, SHORT a2, SHORT nDirection)
    : CProjectileBAM84E69CBase(resRef)
{
    field_2A0 = a2;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_nDirection = nDirection;
    sub_52C8C0(NULL);
}

// 0x533930
CProjectileBAM84E724::~CProjectileBAM84E724()
{
    // TODO: Incomplete.
}

// 0x533AE0
void CProjectileBAM84E724::Virtual88(CGameArea* pArea, CPoint* pt, LONG posZ)
{
    // TODO: Incomplete.
}

// 0x533CB0
CProjectileBAM84E7B0::CProjectileBAM84E7B0(WORD a1, CResRef resRef, WORD a3, WORD a4, LONG a5, CAIObjectType type)
    : CProjectileBAM(resRef)
{
    field_2EE = 0;
    field_2F2 = 0;
    field_2F4 = 0;
    field_2F8 = 0;
    m_pVidCell->SequenceSet(0);
    field_2A8 = a1;
    field_2A0 = a3;
    field_2A2 = a4;
    field_2A4 = a5;
    field_2AE = 0;
    field_2B2.Set(type);
    field_2AA = CProjectile::DecodeProjectile(field_2A8 + 1, NULL, FALSE);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x533E10
CProjectileBAM84E7B0::~CProjectileBAM84E7B0()
{
}

// 0x534140
void CProjectileBAM84E7B0::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x78E750
void CProjectileBAM84E7B0::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
}

// 0x533F40
void CProjectileBAM84E7B0::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    m_sourceId = source;
    m_posDest = targetPos;
    m_targetId = target;
    m_nDirection = 0;

    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* pEffect = m_effectList.GetNext(pos);
        // NOTE: Inlined AddEffect.
        field_2AA->AddEffect(pEffect->Copy());
    }

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    AddToArea(pArea, m_posDest, nHeight, CGameObject::LIST_FRONT);

    // NOTE: Inlined PlaySound. It runs before `field_EA` is set.
    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    field_EA = pArea;
    field_2AA->m_callBackProjectile = m_id;
    field_2EE = nHeight;
    field_2AE = 0;
    field_2AA->Fire(pArea, source, target, targetPos, nHeight, nType);

    field_2AA = CProjectile::DecodeProjectile(field_2A8 + 1, NULL, FALSE);
    field_2AA->field_186 = field_186;
    field_2AA->field_18A = field_18A;

    field_2A0--;
    field_2F8 = field_2A2;
}

// 0x533DE0
void CProjectileBAM84E7B0::CallBack()
{
    field_2AE = 1;
}

// 0x534600
CProjectileBAM84E838::CProjectileBAM84E838(WORD a1, LONG a2)
    : CProjectileBAM84E7B0(22, CResRef((BYTE*)"MAGMISS"), a1, 300, 1, CAIObjectType::ANYONE)
{
    field_1CA = 10;
    field_1CE = 7;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 1;
    field_1D2 = 128;
    field_1D4 = 0;
    field_1D8 = 1;
    m_fireSoundRef = "";
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;

    CAIObjectType type;
    type.m_nEnemyAlly = CAIObjectType::EA_NOTGOOD;
    field_2B2.Set(type);
    field_2F4 = 1;
}

// 0x57E030
CProjectileBAM8510A4::CProjectileBAM8510A4(WORD a1)
    : CProjectileBAM(CResRef((BYTE*)"MMissiT"))
{
    m_pVidCell->SequenceSet(0);
    m_speed *= 2;
    field_1CA = 15;
    field_1CE = 11;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 1;
    field_1D2 = 128;
    field_1D4 = 0;
    m_fireSoundRef = "";
    field_E6 |= 0x8;

    switch (a1) {
    case 0:
        break;
    case 1:
        m_palette.SetRange(0, 33, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_pVidCell->SetPalette(m_palette);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
        // __LINE__: 1481
        UTIL_ASSERT(FALSE);
    }

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}
