#include "CProjectilePart6.h"

#include <math.h>
#include <stdlib.h>

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameSparkle.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CInfinity.h"
#include "CMessage.h"
#include "CParticle.h"
#include "CPathSearch.h"
#include "CUtil.h"
#include "IcewindMisc.h"

// 0x55ACC0
CProjectileHelper84EEA8::CProjectileHelper84EEA8()
{
    field_18 = 10;
    field_20 = 10;
    field_24 = 100000;
    field_28 = 100000;
    field_1C = 0;
    field_2C = 0;
    field_30 = FALSE;
    field_31 = FALSE;
}

// 0x55A610
void CProjectileHelper84EEA8::Virtual0()
{
    if (field_30 != TRUE) {
        if (field_1C <= 0) {
            std::set<DWORD> targets = Virtual4();
            std::list<LONG> filtered = sub_55A890(targets);
            sub_55AB20(filtered);

            field_1C = field_18;
        }

        field_1C--;
    }
}

// 0x55A890
std::list<LONG> CProjectileHelper84EEA8::sub_55A890(const std::set<DWORD>& targets)
{
    // NOTE: Inlined `std::set::find`.
    for (std::map<LONG, CPoint>::iterator it = field_8.begin(); it != field_8.end(); it++) {
        if (targets.find(it->first) == targets.end()) {
            it->second.x = 0;
        }
    }

    std::list<LONG> result;

    for (std::set<DWORD>::const_iterator it = targets.begin(); it != targets.end(); it++) {
        // NOTE: Inlined `std::map::operator[]`. A new entry starts as (0, 0);
        // `CPoint`'s default constructor would leave it uninitialised.
        std::map<LONG, CPoint>::iterator itEntry = field_8.find(*it);
        if (itEntry == field_8.end()) {
            itEntry = field_8.insert(std::make_pair(static_cast<LONG>(*it), CPoint(0, 0))).first;
        }
        CPoint& entry = itEntry->second;

        if (entry.y < field_24) {
            if (entry.x % field_20 == 0) {
                // NOTE: Original inserts at the front, so the ids come out in
                // reverse set order.
                result.push_front(*it);
                entry.y++;
            }
        }

        entry.x++;
    }

    return result;
}

// 0x55AB20
void CProjectileHelper84EEA8::sub_55AB20(const std::list<LONG>& targets)
{
    for (std::list<LONG>::const_iterator it = targets.begin(); it != targets.end(); it++) {
        if (field_31 == TRUE && *it == m_pProjectile->m_sourceId) {
            continue;
        }

        field_2C++;
        if (field_2C > field_28) {
            field_30 = TRUE;
            return;
        }

        sub_55AB80(*it);
    }
}

// 0x55AB80
void CProjectileHelper84EEA8::sub_55AB80(LONG nTarget)
{
    // TODO: Incomplete.
}

// 0x55AD90
std::set<DWORD> CProjectileHelper84EEA8::Virtual4()
{
    CTypedPtrList<CPtrList, LONG*> targets(10);

    m_pProjectile->m_pArea->GetAllInRange(m_pProjectile->GetVertListPos(),
        m_pProjectile->GetPos(),
        CAIObjectType::ANYONE,
        static_cast<WORD>(field_32),
        m_pProjectile->m_terrainTable,
        targets,
        TRUE,
        FALSE);

    m_pProjectile->m_pArea->GetAllInRangeBack(m_pProjectile->GetPos(),
        CAIObjectType::ANYONE,
        static_cast<WORD>(field_32),
        m_pProjectile->m_terrainTable,
        targets,
        TRUE,
        FALSE,
        FALSE);

    std::set<DWORD> result;

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nTarget = reinterpret_cast<LONG>(targets.GetNext(pos));

        // NOTE: Inlined `std::set::insert` with a begin() hint.
        result.insert(result.begin(), nTarget);
    }

    return result;
}

// 0x55AF40
CProjectileHelper84EEB0::CProjectileHelper84EEB0()
{
    field_3A = FALSE;
}

// 0x55B010
void CProjectileHelper84EEB0::sub_55B010(CPoint& a1, CPoint& a2)
{
    CPoint ptTarget(a2.x, a2.y * 4 / 3);
    CPoint ptStart(a1.x, a1.y * 4 / 3);
    INT nDirection = CGameSprite::GetDirection(ptStart, ptTarget);

    CPoint ptDirection = IcewindMisc::sub_584610(nDirection);
    CPoint ptPerpendicular = IcewindMisc::sub_584610((nDirection + 4) % 16);

    CPoint ptSide1 = IcewindMisc::sub_5847B0(a1, ptPerpendicular.x, ptPerpendicular.y, field_32);
    CPoint ptSide2 = IcewindMisc::sub_5847B0(a1, -ptPerpendicular.x, -ptPerpendicular.y, field_32);

    CPoint pt = IcewindMisc::sub_5847B0(ptSide1, ptDirection.x, ptDirection.y, field_36);
    field_3C[0].x = pt.x - a1.x;
    field_3C[0].y = pt.y - a1.y;

    pt = IcewindMisc::sub_5847B0(ptSide1, -ptDirection.x, -ptDirection.y, field_36);
    field_3C[1].x = pt.x - a1.x;
    field_3C[1].y = pt.y - a1.y;

    pt = IcewindMisc::sub_5847B0(ptSide2, -ptDirection.x, -ptDirection.y, field_36);
    field_3C[2].x = pt.x - a1.x;
    field_3C[2].y = pt.y - a1.y;

    pt = IcewindMisc::sub_5847B0(ptSide2, ptDirection.x, ptDirection.y, field_36);
    field_3C[3].x = pt.x - a1.x;
    field_3C[3].y = pt.y - a1.y;

    field_3A = TRUE;
}

// 0x55B1B0
std::set<DWORD> CProjectileHelper84EEB0::Virtual4()
{
    CPoint ptPoly[4];

    CPoint& pos0 = m_pProjectile->GetPos();
    ptPoly[0].x = pos0.x + field_3C[0].x;
    ptPoly[0].y = pos0.y + field_3C[0].y;

    CPoint& pos1 = m_pProjectile->GetPos();
    ptPoly[1].x = pos1.x + field_3C[1].x;
    ptPoly[1].y = pos1.y + field_3C[1].y;

    CPoint& pos2 = m_pProjectile->GetPos();
    ptPoly[2].x = pos2.x + field_3C[2].x;
    ptPoly[2].y = pos2.y + field_3C[2].y;

    CPoint& pos3 = m_pProjectile->GetPos();
    ptPoly[3].x = pos3.x + field_3C[3].x;
    ptPoly[3].y = pos3.y + field_3C[3].y;

    CRect rBounding;
    rBounding.left = min(min(ptPoly[0].x, ptPoly[1].x), min(ptPoly[2].x, ptPoly[3].x));
    rBounding.top = min(min(ptPoly[0].y, ptPoly[1].y), min(ptPoly[2].y, ptPoly[3].y));
    rBounding.right = max(max(ptPoly[0].x, ptPoly[1].x), max(ptPoly[2].x, ptPoly[3].x));
    rBounding.bottom = max(max(ptPoly[0].y, ptPoly[1].y), max(ptPoly[2].y, ptPoly[3].y));

    CTypedPtrList<CPtrList, LONG*> targets(10);

    m_pProjectile->m_pArea->GetAllInPoly(rBounding,
        ptPoly,
        4,
        CAIObjectType::ANYONE,
        m_pProjectile->m_pArea->m_visibleTerrainTable,
        targets,
        FALSE);

    std::set<DWORD> result;

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nTarget = reinterpret_cast<LONG>(targets.GetNext(pos));

        // NOTE: Inlined `std::set::insert` with a begin() hint.
        result.insert(result.begin(), nTarget);
    }

    return result;
}

// 0x579B40
CProjectileSingleTarget850DDC::CProjectileSingleTarget850DDC(const char* a1, const char* a2)
{
    field_2CE = 200;
    field_2D2 = 350;
    field_2DA = 25;
    field_2DE = a2;
    field_2EE = 1;
    field_302 = 20;
    field_306 = 35;
    field_30A = 45;
    field_30E = 15;
    field_312 = 5;
    field_316 = FALSE;
    field_317 = TRUE;
    field_32E = 0;
    field_332 = FALSE;
    field_2D6 = 0;
    field_328 = 0;
    field_32C = 0;
    field_2A0 = FALSE;
    field_2A1 = TRUE;

    // NOTE: Uninline.
    SetVidCell(CResRef(CString(a1)));
    field_1DE = 1;

    // NOTE: Uninline.
    SetVidCell(CResRef("MMissiT"));
    field_1DE = 0;

    m_bHasHeight = TRUE;
}

// 0x579E50
void CProjectileSingleTarget850DDC::AIUpdate()
{
    field_32E++;

    if (field_32E > field_30E) {
        if (field_332 == TRUE) {
            // NOTE: Inlined `CProjectile::RemoveSelf`.
            RemoveFromArea();

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                delete this;
                return;
            }
        }
    } else {
        if (field_32E % field_312 == 1) {
            Virtual88();
        }
    }

    if (!field_332) {
        CProjectileSingleTarget::AIUpdate();
    }
}

// 0x579EF0
void CProjectileSingleTarget850DDC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    field_328 = nHeight;
    field_32C = nType;
    m_sourceId = source;
    field_EA = pArea;

    CGameObject* pObject;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        sub_52A640(m_sourceId, field_318, TRUE);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        field_2D6 = field_2CE / field_302 / 2;

        LONG dx = targetPos.x - field_318.x;
        LONG dy = targetPos.y - field_318.y;

        double t = sqrt(static_cast<double>(field_2DA * field_2DA) / static_cast<double>(dy * dy + dx * dx));
        field_2AE[0].x = static_cast<LONG>(static_cast<double>(-dy) * t + static_cast<double>(field_318.x));
        field_2AE[0].y = static_cast<LONG>(static_cast<double>(dx) * t + static_cast<double>(field_318.y));

        t = sqrt(static_cast<double>(field_2DA * field_2DA) / static_cast<double>(dx * dx + dy * dy));
        field_2AE[1].x = static_cast<LONG>(static_cast<double>(dy) * t + static_cast<double>(field_318.x));
        field_2AE[1].y = static_cast<LONG>(static_cast<double>(-dx) * t + static_cast<double>(field_318.y));

        LONG nAhead = field_2DA + field_2D2;
        t = sqrt(static_cast<double>(nAhead * nAhead) / static_cast<double>(dy * dy + dx * dx));
        LONG nAheadX = static_cast<LONG>(static_cast<double>(dx) * t + static_cast<double>(field_318.x));
        LONG nAheadY = static_cast<LONG>(static_cast<double>(dy) * t + static_cast<double>(field_318.y));

        t = sqrt(static_cast<double>(field_2D2 * field_2D2) / static_cast<double>(dy * dy + dx * dx));
        field_320.x = static_cast<LONG>(static_cast<double>(dx) * t + static_cast<double>(field_318.x));
        field_320.y = static_cast<LONG>(static_cast<double>(dy) * t + static_cast<double>(field_318.y));

        LONG nHalfWidth = field_2CE / 2;
        LONG nSide = field_2DA + nHalfWidth;
        t = sqrt(static_cast<double>(nSide * nSide) / static_cast<double>(dx * dx + dy * dy));
        field_2AE[2].x = static_cast<LONG>(static_cast<double>(nAheadX) + static_cast<double>(dy) * t);
        field_2AE[2].y = static_cast<LONG>(static_cast<double>(-dx) * t + static_cast<double>(nAheadY));

        t = sqrt(static_cast<double>(nSide * nSide) / static_cast<double>(dy * dy + dx * dx));
        field_2AE[3].x = static_cast<LONG>(static_cast<double>(-dy) * t + static_cast<double>(nAheadX));
        field_2AE[3].y = static_cast<LONG>(static_cast<double>(dx) * t + static_cast<double>(nAheadY));

        for (LONG i = -field_2D6; i < field_2D6; i++) {
            CPoint pt;
            if (i < 0) {
                LONG nStep = -(field_302 * i);
                t = sqrt(static_cast<double>(nStep * nStep) / static_cast<double>(dx * dx + dy * dy));
                pt.x = static_cast<LONG>(static_cast<double>(-dy) * t + static_cast<double>(field_320.x));
                pt.y = static_cast<LONG>(static_cast<double>(dx) * t + static_cast<double>(field_320.y));
            } else if (i > 0) {
                LONG nStep = field_302 * i;
                t = sqrt(static_cast<double>(nStep * nStep) / static_cast<double>(dx * dx + dy * dy));
                pt.x = static_cast<LONG>(static_cast<double>(dy) * t + static_cast<double>(field_320.x));
                pt.y = static_cast<LONG>(static_cast<double>(-dx) * t + static_cast<double>(field_320.y));
            } else {
                pt = field_320;
            }

            field_2F2.insert(field_2F2.end(), 1, pt);
        }

        field_2F2.insert(field_2F2.end(), 1, field_320);

        CProjectileSingleTarget::Fire(pArea, source, target, targetPos, nHeight, nType);
    }
}

// 0x57A530
void CProjectileSingleTarget850DDC::OnArrival()
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
}

// 0x57A670
void CProjectileSingleTarget850DDC::DeliverEffects()
{
    CRect rBounding;
    rBounding.left = min(min(field_2AE[0].x, field_2AE[1].x), min(field_2AE[2].x, field_2AE[3].x));
    rBounding.top = min(min(field_2AE[0].y, field_2AE[1].y), min(field_2AE[2].y, field_2AE[3].y));
    rBounding.right = max(max(field_2AE[0].x, field_2AE[1].x), max(field_2AE[2].x, field_2AE[3].x));
    rBounding.bottom = max(max(field_2AE[0].y, field_2AE[1].y), max(field_2AE[2].y, field_2AE[3].y));

    CTypedPtrList<CPtrList, LONG*> targets(10);

    field_EA->GetAllInPoly(rBounding,
        field_2AE,
        4,
        CAIObjectType::ANYONE,
        field_EA->m_visibleTerrainTable,
        targets,
        TRUE);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nTarget = reinterpret_cast<LONG>(targets.GetNext(pos));

        if (nTarget != m_sourceId) {
            CGameObject* pObject;
            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nTarget,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                if ((pObject->GetObjectType() & CGameObject::TYPE_AIBASE) != 0
                    && field_EA->CheckLOS(field_318, pObject->GetPos(), m_terrainTable, FALSE)) {
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
    }

    field_332 = TRUE;
}

// 0x57A970
void CProjectileSingleTarget850DDC::Virtual88()
{
    if (field_2DE.length() == 0) {
        return;
    }

    for (std::vector<CPoint>::iterator it = field_2F2.begin(); it != field_2F2.end(); it++) {
        CProjectileSingleTarget* pProjectile = new CProjectileSingleTarget(CResRef(CString(field_2DE.c_str())));

        // NOTE: Dereferences `pProjectile` without checking for NULL.
        pProjectile->m_vfx = m_vfx;
        pProjectile->m_bHasHeight = TRUE;

        if (field_316 == TRUE) {
            BYTE nSequenceLength = pProjectile->m_pVidCell->GetSequenceLength(0, FALSE);
            if (nSequenceLength == 0) {
                pProjectile->m_pVidCell->FrameSet(0);
            } else {
                pProjectile->m_pVidCell->FrameSet(rand() % nSequenceLength);
            }
        }

        pProjectile->field_2A0 = field_2A0;
        pProjectile->field_1D8 = static_cast<SHORT>(field_2EE);

        LONG nRandom = field_30A == 0 ? 0 : rand() % field_30A;
        pProjectile->m_speed = static_cast<SHORT>((field_306 + nRandom) * m_speed / 100);

        pProjectile->Fire(field_EA,
            m_sourceId,
            CGameObjectArray::INVALID_INDEX,
            *it,
            field_328,
            static_cast<WORD>(field_32C));
    }
}

// 0x57AEB0
CProjectileSingleTarget850E84::CProjectileSingleTarget850E84()
    : CProjectileSingleTarget(CResRef((BYTE*)"AScorcT"))
{
    field_2C6 = 0;
    field_2CA = 3;
    field_2CE = 60;
    field_2B0 = 0;
    field_2B4 = FALSE;
    field_2D2 = 0;
    sub_57BBC0();
    m_pVidCell->FrameSet(0);
    m_fireSoundRef = "TRA_M04";
    field_1C2 = 0;
    field_1D4 = 0;
    field_1D8 = 16;

    CSize frameSize;
    m_pVidCell->GetFrameSize(0, 0, frameSize, FALSE);
    m_speed = static_cast<SHORT>(3 * frameSize.cx / 4);

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    field_2AF = 0;
    field_2AE = 110;
    m_bHasHeight = TRUE;
    field_1C2 = 1;
}

// 0x57B210
void CProjectileSingleTarget850E84::AIUpdate()
{
    field_2C6++;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (field_2AE == 0) {
            RemoveFromArea();

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                delete this;
            }

            return;
        }

        field_2AE--;

        m_pVidCell->FrameAdvance();

        if (m_targetId != CGameObjectArray::INVALID_INDEX) {
            CGameObject* pTarget;
            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                &pTarget,
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                RemoveSelf();
                return;
            }

            if (field_EA != pTarget->m_pArea) {
                LONG nTargetId = m_targetId;
                RemoveSelf();

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return;
            }

            CPoint& ptTarget = pTarget->GetPos();
            m_posDest.x = ptTarget.x;
            m_posDest.y = ptTarget.y;
            field_2B0 = pTarget->m_posZ - m_posZ;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            field_2B0 = 0;
        }

        CGameObject* pSource;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_sourceId,
                CGameObjectArray::THREAD_ASYNCH,
                &pSource,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED);

        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        if (pSource->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pSource);
            m_nDirection = pSprite->GetDirection(m_posDest);
            pSprite->SetDirection(m_nDirection);
        } else {
            m_nDirection = GetDirection(m_posDest);
        }

        sub_579860(NULL);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        sub_52A640(m_sourceId, m_pos, TRUE);

        if (field_2C6 % field_2CE == 1) {
            // NOTE: Inlined `std::set::clear`.
            field_2B6.clear();
        }

        if (field_2C6 % field_2CA == 1) {
            // NOTE: Inlined `std::set::find`.
            if (field_2B6.find(field_2D2) == field_2B6.end()) {
                Virtual8C(CPoint(field_2D2, 1));

                // NOTE: Inlined `std::set::insert`.
                field_2B6.insert(field_2D2);
            }

            Virtual88();
        }

        LONG nDx = m_posDest.x - m_pos.x;
        LONG nDy = 4 * m_posDest.y / 3 - 4 * m_pos.y / 3;

        m_posExact.x = m_pos.x << CParticle::RESOLUTION_INC;
        m_posExact.y = 4 * (m_pos.y << CParticle::RESOLUTION_INC) / 3;

        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(nDx * nDx + nDy * nDy)) + 0.5);
        if (nDistance == 0) {
            field_2AE = 0;
            field_2AF = 0;
        } else {
            LONG nSteps = (m_speed + nDistance - 1) / m_speed;
            field_2AF = static_cast<BYTE>(nSteps);
            m_posDelta.x = (nDx << CParticle::RESOLUTION_INC) / (nSteps & 0xFF);
            m_posDelta.y = (nDy << CParticle::RESOLUTION_INC) / (nSteps & 0xFF);
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    }
}

// 0x57B780
void CProjectileSingleTarget850E84::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
    // __LINE__: 207
    UTIL_ASSERT(pVidMode != NULL);

    DWORD dwFlags = 0x200;
    if (field_1DA > CGameSprite::DIR_N) {
        dwFlags |= CInfinity::MIRROR_FX;
    }

    if (field_1DA > CGameSprite::DIR_W && field_1DA < CGameSprite::DIR_E) {
        dwFlags |= CInfinity::MIRROR_FX_UPDOWN;
    }

    CInfinity* pInfinity = &(field_EA->m_cInfinity);

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    CPoint pos;
    pos.x = m_pos.x;
    pos.y = field_EA->GetHeightOffset(m_pos, m_listType) + (m_pos.y - m_posZ);

    CPoint ptRef;
    m_pVidCell->GetCurrentCenterPoint(ptRef, FALSE);

    CSize size;
    m_pVidCell->GetCurrentFrameSize(size, FALSE);

    CRect rFx;
    rFx.SetRect(0, 0, size.cx, size.cy);

    CRect rGCBounds;

    for (BYTE i = 0; i < field_2AF; i++) {
        // NOTE: Uninline.
        if (field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(pos))) {
            if (field_1DA > CGameSprite::DIR_N) {
                rGCBounds.left = ptRef.x - rFx.right + pos.x + rFx.left;
            } else {
                rGCBounds.left = pos.x - ptRef.x;
            }

            if (field_1DA > CGameSprite::DIR_W && field_1DA < CGameSprite::DIR_E) {
                rGCBounds.top = rFx.top - rFx.bottom + ptRef.y + pos.y;
            } else {
                rGCBounds.top = pos.y - ptRef.y;
            }

            rGCBounds.right = rFx.right - rFx.left + rGCBounds.left;
            rGCBounds.bottom = rFx.bottom - rFx.top + rGCBounds.top;

            if (!IsRectEmpty(rGCBounds & rScreen)) {
                dwFlags |= CInfinity::FXPREP_COPYFROMBACK;

                pInfinity->FXPrep(rFx, dwFlags, a3, pos, ptRef);

                if (pInfinity->FXLock(rFx, dwFlags)) {
                    pInfinity->FXRender(m_pVidCell, ptRef.x, ptRef.y, dwFlags, 0);
                    pInfinity->FXRenderClippingPolys(pos.x, pos.y, 0, ptRef, rGCBounds, FALSE, dwFlags);
                    pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
                    pInfinity->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, dwFlags);
                }

                pos.x = (m_posDelta.x * (i + 1) + m_posExact.x) >> CParticle::RESOLUTION_INC;
                pos.y = field_EA->GetHeightOffset(m_pos, m_listType)
                    + (((3 * (m_posDelta.y * (i + 1) + m_posExact.y) / 4) >> CParticle::RESOLUTION_INC) - m_posZ);

                if (field_2B0 > 0) {
                    pos.y -= min(i + 1, field_2B0);
                } else if (field_2B0 < 0) {
                    pos.y += min(i + 1, -field_2B0);
                }
            }
        }
    }
}

// 0x57B1D0
void CProjectileSingleTarget850E84::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CProjectileSingleTarget::Fire(pArea, source, target, targetPos, nHeight, nType);

    field_2D2 = target;
    DeliverEffects();
}

// 0x799CA0
void CProjectileSingleTarget850E84::DeliverEffects()
{
}

// 0x57BE80
void CProjectileSingleTarget850E84::Virtual88()
{
    CPoint ptOffset = stru_8E52D0[m_nDirection];

    CPoint ptPoly[4];
    ptPoly[0].x = ptOffset.x + m_pos.x;
    ptPoly[0].y = ptOffset.y + m_pos.y;
    ptPoly[1].x = m_pos.x - ptOffset.x;
    ptPoly[1].y = m_pos.y - ptOffset.y;
    ptPoly[2].x = m_posDest.x - ptOffset.x;
    ptPoly[2].y = m_posDest.y - ptOffset.y;
    ptPoly[3].x = ptOffset.x + m_posDest.x;
    ptPoly[3].y = ptOffset.y + m_posDest.y;

    CRect rBounding;
    rBounding.left = min(min(ptPoly[0].x, ptPoly[1].x), min(ptPoly[2].x, ptPoly[3].x));
    rBounding.top = min(min(ptPoly[0].y, ptPoly[1].y), min(ptPoly[2].y, ptPoly[3].y));
    rBounding.right = max(max(ptPoly[0].x, ptPoly[1].x), max(ptPoly[2].x, ptPoly[3].x));
    rBounding.bottom = max(max(ptPoly[0].y, ptPoly[1].y), max(ptPoly[2].y, ptPoly[3].y));

    CTypedPtrList<CPtrList, LONG*> targets(10);

    field_EA->GetAllInPoly(rBounding,
        ptPoly,
        4,
        CAIObjectType::ANYONE,
        field_EA->m_visibleTerrainTable,
        targets,
        TRUE);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nTarget = reinterpret_cast<LONG>(targets.GetNext(pos));

        // NOTE: Inlined `std::set::find`.
        if (field_2B6.find(nTarget) == field_2B6.end() && nTarget != m_sourceId) {
            Virtual8C(CPoint(nTarget, 0));

            // NOTE: Inlined `std::set::insert`.
            field_2B6.insert(nTarget);
        }
    }
}

// 0x57C1D0
void CProjectileSingleTarget850E84::Virtual8C(CPoint pt)
{
    LONG nTarget = pt.x;

    CGameObject* pObject;
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nTarget,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    if ((CGameObject::TYPE_AIBASE & pObject->GetObjectType()) == 0) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTarget,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
        if (sub_536FC0(pSprite) == TRUE) {
            CGameEffect::FeedBackImmuneToResource(pSprite, field_18A);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTarget,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return;
        }
    }

    CGameObject* pSource;
    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        &pSource,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTarget,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    CPoint ptSource;
    sub_52A640(m_sourceId, ptSource, TRUE);

    if (field_EA->CheckLOS(ptSource, pObject->GetPos(), m_terrainTable, FALSE)) {
        if (pt.y == 1) {
            std::list<CGameEffect*> effects = IcewindMisc::GetSpellEffects(static_cast<CGameAIBase*>(pSource), "EffAS1", 1);

            for (std::list<CGameEffect*>::iterator it = effects.begin(); it != effects.end(); it++) {
                (*it)->m_sourceRes = CResRef("SpWi217");

                CMessage* message = new CMessageAddEffect(*it, m_sourceId, nTarget);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        } else {
            std::list<CGameEffect*> effects = IcewindMisc::GetSpellEffects(static_cast<CGameAIBase*>(pSource), "EffAS2", 1);

            for (std::list<CGameEffect*>::iterator it = effects.begin(); it != effects.end(); it++) {
                (*it)->m_sourceRes = CResRef("SpWi217");

                CMessage* message = new CMessageAddEffect(*it, m_sourceId, nTarget);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTarget,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x57C510
CProjectileSingleTarget850F14::CProjectileSingleTarget850F14(const std::set<LONG>& a1)
    : CProjectileSingleTarget(CResRef((BYTE*)"CLightT"))
    , field_2AE(a1)
{
    m_pVidCell->SequenceSet(0);
    field_1D8 = 16;
    m_fireSoundRef = "TRA_09";
    m_speed *= 2;
    m_vfx.sub_586A90(TRUE);
    m_bHasHeight = FALSE;
    field_1C2 = 0;
}

// 0x57C7B0
void CProjectileSingleTarget850F14::DeliverEffects()
{
    field_2AE.insert(m_targetId);

    CGameObject* pObject;
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        CGameArea* areaPtr = pObject->m_pArea;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
        // __LINE__: 552
        UTIL_ASSERT(areaPtr != NULL);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        BYTE nCount = 0;
        LONG nNearest = 0;

        while (TRUE) {
            CAIObjectType type(CAIObjectType::ANYONE);
            type.m_nEnemyAlly = CAIObjectType::EA_ENEMY;

            LONG nPrevious = nNearest;
            nNearest = areaPtr->GetNearest(m_targetId,
                type,
                0x7FFF,
                field_EA->m_visibleTerrainTable,
                TRUE,
                TRUE,
                TRUE,
                nCount,
                FALSE);
            if (nNearest == nPrevious
                || nNearest == CGameObjectArray::INVALID_INDEX
                || nCount > 20) {
                CProjectile::DeliverEffects();
                return;
            }

            nCount++;

            if (field_2AE.find(nNearest) == field_2AE.end()) {
                break;
            }
        }

        CProjectileSingleTarget850F14* pProjectile = new CProjectileSingleTarget850F14(field_2AE);

        CGameObject* pNearest;
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nNearest,
            CGameObjectArray::THREAD_ASYNCH,
            &pNearest,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            delete pProjectile;
            CProjectile::DeliverEffects();
            return;
        }

        CPoint ptNearest = pNearest->GetPos();
        LONG nNearestId = pNearest->m_id;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nNearest,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        POSITION pos = m_effectList.GetHeadPosition();
        while (pos != NULL) {
            // NOTE: The copy leaks when the projectile is deleted below.
            CGameEffect* pEffect = m_effectList.GetNext(pos)->Copy();
            if (pEffect->m_effectID == CGAMEEFFECT_DAMAGE) {
                if (field_2AE.size() == 1) {
                    pEffect->m_numDice >>= 1;
                }

                if (field_2AE.size() >= static_cast<BYTE>(pEffect->m_casterLevel)) {
                    delete pProjectile;
                    CProjectile::DeliverEffects();
                    return;
                }
            }

            pProjectile->AddEffect(pEffect);
        }

        pProjectile->Fire(field_EA,
            m_id,
            nNearestId,
            ptNearest,
            m_posZ,
            0);
    }

    CProjectile::DeliverEffects();
}

// 0x57D390
CProjectileSingleTarget85101C::CProjectileSingleTarget85101C()
    : CProjectileSingleTarget(CResRef((BYTE*)"LightnT"))
{
    field_2B2 = -1;
    m_pVidCell->SequenceSet(0);
    field_E6 |= 0x8;
    field_1BE = 0;
    field_1C2 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "TRA_09b";
    m_speed *= 2;
    field_2AE = 60;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    m_vfx.sub_586A90(TRUE);
    field_2B6.m_pProjectile = this;
    field_2B6.field_20 = 33;
    field_2B6.field_18 = 2;
    field_2B6.field_32 = 50;
}

// 0x57D9F0
void CProjectileSingleTarget85101C::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (field_2AE == 0) {
        // NOTE: Inlined `CProjectile::RemoveSelf`.
        RemoveFromArea();

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            NULL,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            delete this;
        }

        return;
    }

    field_2AE--;

    m_pVidCell->FrameAdvance();

    LONG nOldGridY = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
    LONG nOldGridX = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;

    m_posExact.x += m_posDelta.x;
    m_posExact.y += m_posDelta.y;

    m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
    m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;

    if (m_pos.x < 0
        || m_pos.y < 0
        || m_pos.x >= field_EA->m_cInfinity.nAreaX
        || m_pos.y >= field_EA->m_cInfinity.nAreaY) {
        // NOTE: Inlined `CProjectile::RemoveSelf`.
        RemoveFromArea();

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            NULL,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            delete this;
        }

        return;
    }

    CPoint ptSearch;
    ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
    ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;

    if (nOldGridX != ptSearch.x || nOldGridY != ptSearch.y) {
        SHORT nTableIndex;
        BYTE nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
        if (nCost == CPathSearch::COST_IMPASSABLE || nTableIndex == 8) {
            if (field_2B2 > 0) {
                field_2B2--;
                if (field_2B2 <= 0) {
                    // NOTE: Inlined `CProjectile::RemoveSelf`.
                    RemoveFromArea();

                    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                        CGameObjectArray::THREAD_ASYNCH,
                        NULL,
                        INFINITE);
                    if (rc == CGameObjectArray::SUCCESS) {
                        delete this;
                    }


                    return;
                }
            }

            if (abs(m_posDelta.x) < abs(m_posDelta.y)) {
                ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
                ptSearch.y = ((3 * (m_posExact.y - 2 * m_posDelta.y) / 4) >> CParticle::RESOLUTION_INC) / CPathSearch::GRID_SQUARE_SIZEY;
                nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
                if (nCost == CPathSearch::COST_IMPASSABLE || nTableIndex == 8) {
                    m_posExact.x += -m_posDelta.x * 2;
                    m_posDelta.x = -m_posDelta.x;
                    m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
                    ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
                    ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
                    nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
                    if (nCost == CPathSearch::COST_IMPASSABLE) {
                        m_posExact.y += -m_posDelta.y * 2;
                        m_posDelta.y = -m_posDelta.y;
                        m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;
                    }
                } else {
                    m_posExact.y += -m_posDelta.y * 2;
                    m_posDelta.y = -m_posDelta.y;
                    m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;
                }
            } else {
                ptSearch.x = ((m_posExact.x - 2 * m_posDelta.x) >> CParticle::RESOLUTION_INC) / CPathSearch::GRID_SQUARE_SIZEX;
                ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
                nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
                if (nCost == CPathSearch::COST_IMPASSABLE || nTableIndex == 8) {
                    m_posExact.y += -m_posDelta.y * 2;
                    m_posDelta.y = -m_posDelta.y;
                    m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;
                    ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
                    ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
                    nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
                    if (nCost == CPathSearch::COST_IMPASSABLE || nTableIndex == 8) {
                        m_posExact.x += -m_posDelta.x * 2;
                        m_posDelta.x = -m_posDelta.x;
                        m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
                    }
                } else {
                    m_posExact.x += -m_posDelta.x * 2;
                    m_posDelta.x = -m_posDelta.x;
                    m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
                }
            }

            CPoint ptDest;
            ptDest.y = (3 * (m_posExact.y + m_posDelta.y) / 4) >> CParticle::RESOLUTION_INC;
            ptDest.x = (m_posExact.x + m_posDelta.x) >> CParticle::RESOLUTION_INC;
            m_nDirection = GetDirection(ptDest);
            m_posDest = ptDest;
        }

        if (field_E2 != 0) {
            CGameSparkle* pSparkle = new CGameSparkle(field_9A, 1 /* 0x84D600 */, 25 /* 0x84D5F0 */);
            pSparkle->AddToArea(field_EA, m_pos, m_posZ, CGameObject::LIST_FRONT);
        }
    }

    field_2B6.Virtual0();

    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}

// 0x57D700
void CProjectileSingleTarget85101C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pObject;
    CPoint ptStart;
    BYTE rc;

    m_sourceId = target;
    m_targetId = CGameObjectArray::INVALID_INDEX;
    field_EA = pArea;

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(source,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    sub_52A640(source, ptStart, TRUE);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(source,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    CPoint ptTarget = pObject->GetPos();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    field_17C = TRUE;

    AddToArea(pArea, ptStart, nHeight, CGameObject::LIST_FRONT);

    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    m_posExact.x = ptStart.x << CParticle::RESOLUTION_INC;
    m_posExact.y = 4 * (ptStart.y << CParticle::RESOLUTION_INC) / 3;
    m_nDirection = GetDirection(targetPos);

    LONG dx = targetPos.x - m_pos.x;
    LONG dy = 4 * targetPos.y / 3 - 4 * m_pos.y / 3;
    LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

    if (nDistance == 0) {
        dx = targetPos.x - ptTarget.x;
        dy = 4 * targetPos.y / 3 - 4 * ptTarget.y / 3;
        nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

        if (nDistance == 0) {
            field_2AE = 0;
            return;
        }
    }

    m_posDelta.x = (dx << CParticle::RESOLUTION_INC) * m_speed / nDistance;
    m_posDelta.y = (dy << CParticle::RESOLUTION_INC) * m_speed / nDistance;
}

// 0x57E370
CProjectileSingleTarget8511AC::CProjectileSingleTarget8511AC(const char* szResRef)
    : CProjectileSingleTarget(CResRef(CString(szResRef)))
{
    m_vfx.sub_586A90(TRUE);
    m_pVidCell->SequenceSet(0);
    field_1BE = 0;
    field_1C2 = 0;
    field_1D4 = 0;
    m_speed *= 2;
    m_fireSoundRef = "";
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    m_bHasHeight = TRUE;
    m_fireSoundRef = "TRA_02";
    m_arrivalSoundRef = "";
}

// 0x57E490
CProjectileSingleTarget851234::CProjectileSingleTarget851234(const char* szResRef, const IcewindCVisualEffect& vfx)
    : CProjectileSingleTarget(CResRef(CString(szResRef)))
{
    field_2AE = FALSE;
    field_2AF = FALSE;
    field_2B0 = 0;
    field_2B4 = 0;
    field_2B8 = TRUE;
    field_2B9 = TRUE;
    field_1D4 = 0;
    field_1D8 = 1;
    m_bHasHeight = FALSE;
    field_1C2 = 0;
    m_vfx = vfx;
}

// 0x57E580
void CProjectileSingleTarget851234::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (!m_pVidCell->IsEndOfSequence(FALSE)) {
            m_pVidCell->FrameAdvance();
        }

        if (field_2AF == TRUE
            && static_cast<WORD>(m_pVidCell->m_nCurrentSequence) == field_2B0
            && field_2B4 > 0) {
            field_2B4--;

            if (m_pVidCell->IsEndOfSequence(FALSE)) {
                m_pVidCell->FrameSet(0);
            }
        }

        if (m_pVidCell->IsEndOfSequence(FALSE)) {
            if (field_2B9 == TRUE
                || static_cast<WORD>(m_pVidCell->m_nCurrentSequence) >= static_cast<BYTE>(m_pVidCell->GetNumberSequences(FALSE)) - 1) {
                if (m_sound.IsSoundPlaying()) {
                    field_1DE = 0;
                    return;
                }

                // NOTE: Inlined `CProjectile::RemoveSelf`.
                RemoveFromArea();

                BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                    CGameObjectArray::THREAD_ASYNCH,
                    NULL,
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS) {
                    delete this;
                    return;
                }
            } else {
                m_pVidCell->SequenceSet(static_cast<WORD>(m_pVidCell->m_nCurrentSequence + 1));
                m_pVidCell->FrameSet(0);
            }
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    }
}

// 0x57E710
void CProjectileSingleTarget851234::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
    // __LINE__: 1714
    UTIL_ASSERT(!(field_2AF && field_2B8));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
    // __LINE__: 1715
    UTIL_ASSERT(!(field_2AF && field_2B9));

    DeliverEffects();

    BYTE nSequences = static_cast<BYTE>(m_pVidCell->GetNumberSequences(FALSE));
    if (field_2B8 == TRUE && nSequences > 1) {
        // NOTE: The zero check is dead code.
        m_pVidCell->SequenceSet(nSequences == 0 ? 0 : rand() % nSequences);
    } else {
        m_pVidCell->SequenceSet(0);
    }

    m_pos.x = targetPos.x;
    field_EA = pArea;

    if (field_2AE == TRUE) {
        m_pos.y = targetPos.y - 100;
    } else {
        m_pos.y = targetPos.y + 1;
    }

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    AddToArea(pArea, m_pos, 0, CGameObject::LIST_FRONT);

    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);
}

// 0x57F640
CProjectileSingleTarget851444::CProjectileSingleTarget851444()
    : CProjectileSingleTarget(CResRef((BYTE*)"WhirlwX"))
{
    field_2AE = 1000;
    field_2B2 = 1000;
    field_2B6 = 0;
    field_354 = FALSE;
    field_356 = 0;
    field_1D8 = 1;
    m_speed = 5;
    field_2BA.m_pProjectile = this;
    field_2BA.field_28 = 8;
    field_2BA.field_32 = 70;
    field_2BA.field_20 = 33;
    field_2BA.field_18 = 3;
    field_2BA.field_31 = TRUE;
    m_vfx.sub_586A90(TRUE);
    m_bHasHeight = FALSE;
    field_1C2 = 0;
    m_fireSoundRef = "EFF_P105";
    m_loopFireSound = TRUE;
}

// 0x57F8D0
void CProjectileSingleTarget851444::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (field_354 == TRUE) {
        if (field_2F0.IsSoundPlaying()) {
            return;
        }

        // NOTE: Inlined `CProjectile::RemoveSelf`.
        RemoveFromArea();

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            NULL,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            delete this;
        }

        return;
    }

    if (field_2B2 <= 0) {
        m_posDest = sub_5800E0(0);
        field_2B2 = rand() % 50 + 50;
    } else {
        field_2B2--;
    }

    if (field_2AE > 0 && field_2BA.field_30 != TRUE) {
        field_2AE--;

        m_posExact.x += m_posDelta.x;
        m_posExact.y += m_posDelta.y;

        m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
        m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;

        if (m_pos.x < 0
            || m_pos.y < 0
            || m_pos.x >= field_EA->m_cInfinity.nAreaX
            || m_pos.y >= field_EA->m_cInfinity.nAreaY) {
            // NOTE: Inlined `CProjectile::RemoveSelf`.
            RemoveFromArea();

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                delete this;
            }

            return;
        }

        CPoint ptSearch;
        ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
        ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;

        SHORT nTableIndex;
        BYTE nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
        if (nCost == CPathSearch::COST_IMPASSABLE || nTableIndex == 8) {
            if (abs(m_posDelta.x) < abs(m_posDelta.y)) {
                ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
                ptSearch.y = ((3 * (m_posExact.y - 2 * m_posDelta.y) / 4) >> CParticle::RESOLUTION_INC) / CPathSearch::GRID_SQUARE_SIZEY;
                nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
                if (nCost == CPathSearch::COST_IMPASSABLE || nTableIndex == 8) {
                    m_posExact.x += -m_posDelta.x * 2;
                    m_posDelta.x = -m_posDelta.x;
                    m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
                    ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
                    ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
                    nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
                    if (nCost == CPathSearch::COST_IMPASSABLE || nTableIndex == 8) {
                        m_posExact.y += -m_posDelta.y * 2;
                        m_posDelta.y = -m_posDelta.y;
                        m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;
                    }
                } else {
                    m_posExact.y += -m_posDelta.y * 2;
                    m_posDelta.y = -m_posDelta.y;
                    m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;
                }
            } else {
                ptSearch.x = ((m_posExact.x - 2 * m_posDelta.x) >> CParticle::RESOLUTION_INC) / CPathSearch::GRID_SQUARE_SIZEX;
                ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
                nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
                if (nCost == CPathSearch::COST_IMPASSABLE || nTableIndex == 8) {
                    m_posExact.y += -m_posDelta.y * 2;
                    m_posDelta.y = -m_posDelta.y;
                    m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;
                    ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
                    ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
                    nCost = field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE);
                    if (nCost == CPathSearch::COST_IMPASSABLE) {
                        m_posExact.x += -m_posDelta.x * 2;
                        m_posDelta.x = -m_posDelta.x;
                        m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
                    }
                } else {
                    m_posExact.x += -m_posDelta.x * 2;
                    m_posDelta.x = -m_posDelta.x;
                    m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
                }
            }

            CPoint ptDest;
            ptDest.y = (3 * (m_posExact.y + 3 * m_posDelta.y) / 4) >> CParticle::RESOLUTION_INC;
            ptDest.x = (3 * m_posDelta.x + m_posExact.x) >> CParticle::RESOLUTION_INC;
            m_nDirection = GetDirection(ptDest);
            m_posDest = ptDest;

            field_2B2 = rand() % 50 + 50;
        }

        field_2BA.Virtual0();

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);

        if (m_posDest.x < m_pos.x + 32
            && m_posDest.x > m_pos.x - 32
            && m_posDest.y < m_pos.y + 24
            && m_posDest.y > m_pos.y - 24) {
            m_posDest = sub_5800E0(0);
            field_2B2 = rand() % 50 + 50;
        }

        CProjectileSingleTarget::AIUpdate();
        return;
    }

    // NOTE: Uninline.
    field_2F0.SetResRef(CResRef("AFT_P23"), TRUE, TRUE);

    field_2F0.SetFireForget(TRUE);
    field_2F0.SetChannel(15, reinterpret_cast<DWORD>(field_EA));
    field_2F0.Play(m_pos.x, m_pos.y, 0, FALSE);

    field_354 = TRUE;
    field_1DE = 0;
}

// 0x57FF80
void CProjectileSingleTarget851444::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // NOTE: Uninline.
    field_2F0.SetResRef(CResRef("ARE_P23"), TRUE, TRUE);

    field_2F0.SetFireForget(TRUE);
    field_2F0.SetChannel(15, reinterpret_cast<DWORD>(field_EA));
    field_2F0.Play(m_pos.x, m_pos.y, 0, FALSE);

    CProjectileSingleTarget::Fire(pArea, source, CGameObjectArray::INVALID_INDEX, targetPos, nHeight, nType);
}

// 0x5800E0
CPoint CProjectileSingleTarget851444::sub_5800E0(int a2)
{
    int nDirection;
    int x;
    int y;

    // NOTE: Only the low byte of the argument is compared.
    if (static_cast<BYTE>(a2) == 1) {
        nDirection = (GetDirection(m_posDest) + 8) % 16;
    } else {
        srand(field_356);
        field_356 = rand();
        nDirection = field_356 % 16;
    }

    switch (nDirection) {
    case 0:
        x = 0;
        y = 27;
        break;
    case 1:
        x = -10;
        y = 18;
        break;
    case 2:
        x = -19;
        y = 14;
        break;
    case 3:
        x = -25;
        y = 8;
        break;
    case 4:
        x = -20;
        y = 0;
        break;
    case 5:
        x = -25;
        y = -8;
        break;
    case 6:
        x = -19;
        y = -14;
        break;
    case 7:
        x = -10;
        y = -18;
        break;
    case 8:
        x = 0;
        y = -27;
        break;
    case 9:
        x = 10;
        y = -18;
        break;
    case 10:
        x = 19;
        y = -14;
        break;
    case 11:
        x = 25;
        y = -8;
        break;
    case 12:
        x = 20;
        y = 0;
        break;
    case 13:
        x = 25;
        y = 8;
        break;
    case 14:
        x = 19;
        y = 14;
        break;
    case 15:
        x = 10;
        y = 18;
        break;
    default:
        x = a2;
        y = a2;
        break;
    }

    return IcewindMisc::sub_5847B0(m_pos, x, y, 100);
}

// 0x580270
void CProjectileSingleTarget851444::OnArrival()
{
    m_posDest = sub_5800E0(0);
    field_2B2 = rand() % 50 + 50;
}

// 0x5802B0
CProjectileSingleTarget8514CC::CProjectileSingleTarget8514CC()
    : CProjectileSingleTarget(CResRef((BYTE*)"WoMoonX"))
{
    field_2AE = 500;
    field_2B2 = FALSE;
    field_1D8 = 16;
    field_2B4.m_pProjectile = this;
    field_2B4.field_24 = 1;
    field_2B4.field_32 = 70;
    field_2B4.field_36 = 20;
    field_2B4.field_18 = 5;
    m_vfx.sub_586A90(TRUE);
    field_1DE = 0;
    m_speed *= 3;
    m_bHasHeight = FALSE;
    field_1C2 = 0;
    field_2A0 = FALSE;
    m_fireSoundRef = "AFT_P21";
    m_loopFireSound = TRUE;
}

// 0x580550
void CProjectileSingleTarget8514CC::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (field_2AE > 0 && field_2B4.field_30 != TRUE) {
            if (field_2B2 == TRUE) {
                field_2AE--;
                field_2B4.Virtual0();
            }

            CProjectileSingleTarget::AIUpdate();
            return;
        }

        // NOTE: Inlined `CProjectile::RemoveSelf`.
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

// 0x580600
void CProjectileSingleTarget8514CC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    if (CInfGame::dword_8E52EC < 1) {
        CInfGame::dword_8E52EC++;

        CGameObject* pObject;
        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(source,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            field_2B4.sub_55B010(pObject->GetPos(), targetPos);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(source,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            CProjectileSingleTarget::Fire(pArea, source, target, targetPos, nHeight, nType);

            m_pos.x = targetPos.x;
            m_pos.y = targetPos.y;
        }
    }
}

// 0x5820B0
void CProjectileSingleTarget8514CC::OnArrival()
{
    if (!field_2B2) {
        // NOTE: Uninline.
        field_310.SetResRef(CResRef("ARE_P22"), TRUE, TRUE);

        field_310.SetFireForget(TRUE);
        field_310.SetChannel(15, reinterpret_cast<DWORD>(field_EA));
        field_310.Play(m_pos.x, m_pos.y, 0, FALSE);
    }

    field_2B2 = TRUE;
    field_1DE = TRUE;
}

// 0x5806C0
CProjectileSingleTarget8513BC::CProjectileSingleTarget8513BC(const char* szResRef)
    : CProjectileSingleTarget(CResRef((BYTE*)szResRef))
{
    field_2AE = 400;
    field_2B2.m_pProjectile = this;
    field_2B2.field_20 = 33;
    field_2B2.field_18 = 2;
}

// 0x5808A0
void CProjectileSingleTarget8513BC::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (field_2B2.field_30 == TRUE) {
            // NOTE: Inlined `CProjectile::RemoveSelf`.
            RemoveFromArea();

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                delete this;
            }


            return;
        }

        LONG nOldGridX = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
        LONG nOldGridY = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;

        m_posExact.x += m_posDelta.x;
        m_posExact.y += m_posDelta.y;

        m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
        m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;

        if (m_pos.x < 0
            || m_pos.y < 0
            || m_pos.x >= field_EA->m_cInfinity.nAreaX
            || m_pos.y >= field_EA->m_cInfinity.nAreaY) {
            // NOTE: Inlined `CProjectile::RemoveSelf`.
            RemoveFromArea();

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                delete this;
            }


            return;
        }

        CPoint ptSearch;
        ptSearch.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
        ptSearch.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;

        if (nOldGridX != ptSearch.x || nOldGridY != ptSearch.y) {
            SHORT nTableIndex;
            if (field_EA->m_search.GetLOSCost(ptSearch, m_terrainTable, nTableIndex, FALSE) == CPathSearch::COST_IMPASSABLE) {
                // NOTE: Inlined `CProjectile::RemoveSelf`.
                RemoveFromArea();

                BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                    CGameObjectArray::THREAD_ASYNCH,
                    NULL,
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS) {
                    delete this;
                }


                return;
            }
        }

        field_2B2.Virtual0();

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);

        if (m_posDest.x < m_pos.x + 32
            && m_posDest.x > m_pos.x - 32
            && m_posDest.y < m_pos.y + 24
            && m_posDest.y > m_pos.y - 24) {
            OnArrival();
            return;
        }

        CProjectileSingleTarget::AIUpdate();
    }
}

// 0x580B30
void CProjectileSingleTarget8513BC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pObject;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(source,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        CPoint ptObject = pObject->GetPos();

        CPoint ptTarget = IcewindMisc::sub_5847B0(ptObject,
            targetPos.x - ptObject.x,
            targetPos.y - ptObject.y,
            field_2AE);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(source,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        CProjectileSingleTarget::Fire(pArea, source, target, ptTarget, nHeight, nType);
    }
}

// 0x580C00
CProjectileSingleTarget851554::CProjectileSingleTarget851554()
    : CProjectileSingleTarget(CResRef((BYTE*)"MFMissT"))
{
    field_1D8 = 1;
    m_speed *= 2;
    field_2AE.m_pProjectile = this;
    field_2AE.field_32 = 70;
    m_vfx.sub_586A90(TRUE);
    m_bHasHeight = TRUE;
    field_1C2 = 1;
    m_fireSoundRef = "TRA_55";
}

// 0x580E10
void CProjectileSingleTarget851554::OnArrival()
{
    CGameObject* pTarget;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH,
        &pTarget,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE
        && static_cast<CGameSprite*>(pTarget)->m_derivedStats.m_cImmunitiesProjectile.OnList(m_projectileType)) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        CProjectile::OnArrival();
        return;
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    CTypedPtrList<CPtrList, CGameEffect*> effects(10);
    BYTE nCasterLevel = 7;

    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* pEffect = m_effectList.GetNext(pos);
        nCasterLevel = static_cast<BYTE>(pEffect->m_casterLevel);
        effects.AddTail(pEffect);
    }
    m_effectList.RemoveAll();

    CGameObject* pSource;
    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        &pSource,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        CGameEffect* pDamage = IcewindMisc::sub_585380(pSource, 0, 0, nCasterLevel, 4, 8);
        pDamage->m_sourceRes = CResRef(CString("SpWi422"));
        AddEffect2(pDamage);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        field_2AE.Virtual0();

        m_effectList.RemoveAll();

        pos = effects.GetHeadPosition();
        while (pos != NULL) {
            m_effectList.AddTail(effects.GetNext(pos));
        }

        CProjectile::OnArrival();
    }
}

// 0x581060
CProjectileSingleTarget8515DC::CProjectileSingleTarget8515DC()
    : CProjectileSingleTarget(CResRef((BYTE*)"AScorcT"))
{
    field_2B0 = 0;
    field_2B4 = FALSE;
    m_pVidCell->FrameSet(0);
    m_fireSoundRef = "TRA_M04";
    field_1C2 = 0;
    field_1D4 = 0;
    field_1D8 = 16;

    CSize frameSize;
    m_pVidCell->GetFrameSize(0, 0, frameSize, FALSE);
    m_speed = static_cast<SHORT>(3 * frameSize.cx / 4);

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    m_bHasHeight = TRUE;
    field_1C2 = 1;
    field_2AF = 0;
    field_2AE = 10;
}

// 0x5811B0
void CProjectileSingleTarget8515DC::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (field_2AE == 0) {
            // NOTE: Inlined `CProjectile::RemoveSelf`.
            RemoveFromArea();

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                delete this;
            }


            return;
        }

        field_2AE--;

        m_pVidCell->FrameAdvance();

        BYTE rc;

        if (m_targetId != CGameObjectArray::INVALID_INDEX) {
            CGameObject* pTarget;
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                &pTarget,
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                RemoveSelf();
                return;
            }

            if (field_EA != pTarget->m_pArea) {
                LONG nTargetId = m_targetId;
                RemoveSelf();
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return;
            }

            m_posDest = pTarget->GetPos();
            field_2B0 = pTarget->m_posZ - m_posZ;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            field_2B0 = 0;
        }

        CGameObject* pSource;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_sourceId,
                CGameObjectArray::THREAD_ASYNCH,
                &pSource,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED);

        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        if (pSource->GetObjectType() == CGameObject::TYPE_SPRITE) {
            m_nDirection = static_cast<CGameSprite*>(pSource)->GetDirection(m_posDest);
            static_cast<CGameSprite*>(pSource)->SetDirection(m_nDirection);
        } else {
            m_nDirection = GetDirection(m_posDest);
        }

        sub_579860(NULL);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        sub_52A640(m_sourceId, m_pos, TRUE);

        m_posExact.x = m_pos.x << CParticle::RESOLUTION_INC;
        m_posExact.y = 4 * (m_pos.y << CParticle::RESOLUTION_INC) / 3;

        LONG dx = m_posDest.x - m_pos.x;
        LONG dy = 4 * m_posDest.y / 3 - 4 * m_pos.y / 3;
        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

        if (nDistance == 0) {
            field_2AE = 0;
            field_2AF = 0;
        } else {
            field_2AF = static_cast<BYTE>((m_speed + nDistance - 1) / m_speed);

            m_posDelta.x = (dx << CParticle::RESOLUTION_INC) / field_2AF;
            m_posDelta.y = (dy << CParticle::RESOLUTION_INC) / field_2AF;
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    }
}

// 0x581520
void CProjectileSingleTarget8515DC::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
    // __LINE__: 3167
    UTIL_ASSERT(pVidMode != NULL);

    DWORD dwFlags = 0x200;
    if (field_1DA > CGameSprite::DIR_N) {
        dwFlags |= CInfinity::MIRROR_FX;
    }

    if (field_1DA > CGameSprite::DIR_W && field_1DA < CGameSprite::DIR_E) {
        dwFlags |= CInfinity::MIRROR_FX_UPDOWN;
    }

    CInfinity* pInfinity = &(field_EA->m_cInfinity);

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    CPoint pos;
    pos.x = m_pos.x;
    pos.y = field_EA->GetHeightOffset(m_pos, m_listType) + (m_pos.y - m_posZ);

    CPoint ptRef;
    m_pVidCell->GetCurrentCenterPoint(ptRef, FALSE);

    CSize size;
    m_pVidCell->GetCurrentFrameSize(size, FALSE);

    CRect rFx;
    rFx.SetRect(0, 0, size.cx, size.cy);

    CRect rGCBounds;

    for (BYTE i = 0; i < field_2AF; i++) {
        // NOTE: Uninline.
        if (field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(pos))) {
            if (field_1DA > CGameSprite::DIR_N) {
                rGCBounds.left = ptRef.x - rFx.right + pos.x + rFx.left;
            } else {
                rGCBounds.left = pos.x - ptRef.x;
            }

            if (field_1DA > CGameSprite::DIR_W && field_1DA < CGameSprite::DIR_E) {
                rGCBounds.top = rFx.top - rFx.bottom + ptRef.y + pos.y;
            } else {
                rGCBounds.top = pos.y - ptRef.y;
            }

            rGCBounds.right = rFx.right - rFx.left + rGCBounds.left;
            rGCBounds.bottom = rFx.bottom - rFx.top + rGCBounds.top;

            if (!IsRectEmpty(rGCBounds & rScreen)) {
                dwFlags |= CInfinity::FXPREP_COPYFROMBACK;

                pInfinity->FXPrep(rFx, dwFlags, a3, pos, ptRef);

                if (pInfinity->FXLock(rFx, dwFlags)) {
                    pInfinity->FXRender(m_pVidCell, ptRef.x, ptRef.y, dwFlags, 0);
                    pInfinity->FXRenderClippingPolys(pos.x, pos.y, 0, ptRef, rGCBounds, FALSE, dwFlags);
                    pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
                    pInfinity->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, dwFlags);
                }

                pos.x = (m_posDelta.x * (i + 1) + m_posExact.x) >> CParticle::RESOLUTION_INC;
                pos.y = field_EA->GetHeightOffset(m_pos, m_listType)
                    + (((3 * (m_posDelta.y * (i + 1) + m_posExact.y) / 4) >> CParticle::RESOLUTION_INC) - m_posZ);

                if (field_2B0 > 0) {
                    pos.y -= min(i + 1, field_2B0);
                } else if (field_2B0 < 0) {
                    pos.y += min(i + 1, -field_2B0);
                }
            }
        }
    }
}

// 0x581170
void CProjectileSingleTarget8515DC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CProjectileSingleTarget::Fire(pArea, source, target, targetPos, nHeight, nType);
    DeliverEffects();
}

// 0x581CA0
CProjectileSingleTarget851664::CProjectileSingleTarget851664()
    : CProjectileSingleTarget(CResRef((BYTE*)"WoFireX"))
{
    field_2AE = 500;
    field_2B2 = FALSE;
    field_1D8 = 16;
    field_2B4.m_pProjectile = this;
    field_2B4.field_32 = 70;
    field_2B4.field_36 = 20;
    field_2B4.field_20 = 20;
    field_2B4.field_18 = 5;
    m_vfx.sub_586A90(TRUE);
    field_1DE = 0;
    m_speed *= 3;
    m_bHasHeight = FALSE;
    field_1C2 = 0;
    m_fireSoundRef = "AFT_P21";
    m_loopFireSound = TRUE;
}

// 0x581F40
void CProjectileSingleTarget851664::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (field_2AE > 0 && field_2B4.field_30 != TRUE) {
            if (field_2B2 == TRUE) {
                field_2AE--;
                field_2B4.Virtual0();
            }

            CProjectileSingleTarget::AIUpdate();
            return;
        }

        CInfGame::dword_8E52F0--;

        // NOTE: Inlined `CProjectile::RemoveSelf`.
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

// 0x581FF0
void CProjectileSingleTarget851664::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    if (CInfGame::dword_8E52F0 < 1) {
        CInfGame::dword_8E52F0++;

        CGameObject* pObject;
        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(source,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            field_2B4.sub_55B010(pObject->GetPos(), targetPos);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(source,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            CProjectileSingleTarget::Fire(pArea, source, target, targetPos, nHeight, nType);

            m_pos.x = targetPos.x;
            m_pos.y = targetPos.y;
        }
    }
}

// 0x5820B0
void CProjectileSingleTarget851664::OnArrival()
{
    if (!field_2B2) {
        // NOTE: Uninline.
        field_310.SetResRef(CResRef("ARE_P22"), TRUE, TRUE);

        field_310.SetFireForget(TRUE);
        field_310.SetChannel(15, reinterpret_cast<DWORD>(field_EA));
        field_310.Play(m_pos.x, m_pos.y, 0, FALSE);
    }

    field_2B2 = TRUE;
    field_1DE = TRUE;
}

// 0x8E52D0
std::vector<CPoint> CProjectileSingleTarget850E84::stru_8E52D0;

// 0x8E52E8
BOOLEAN CProjectileSingleTarget850E84::byte_8E52E8;

// 0x57BBC0
void CProjectileSingleTarget850E84::sub_57BBC0()
{
    if (byte_8E52E8 != TRUE) {
        // NOTE: Inlined `std::vector::resize`.
        stru_8E52D0.resize(CGameSprite::NUM_DIR);

        stru_8E52D0[CGameSprite::DIR_E].x = 0;
        stru_8E52D0[CGameSprite::DIR_E].y = -20;
        stru_8E52D0[CGameSprite::DIR_NEE].x = -10;
        stru_8E52D0[CGameSprite::DIR_NEE].y = -18;
        stru_8E52D0[CGameSprite::DIR_NE].x = -19;
        stru_8E52D0[CGameSprite::DIR_NE].y = -14;
        stru_8E52D0[CGameSprite::DIR_NNE].x = -25;
        stru_8E52D0[CGameSprite::DIR_NNE].y = -8;
        stru_8E52D0[CGameSprite::DIR_N].x = -27;
        stru_8E52D0[CGameSprite::DIR_N].y = 0;
        stru_8E52D0[CGameSprite::DIR_NNW].x = -25;
        stru_8E52D0[CGameSprite::DIR_NNW].y = 8;
        stru_8E52D0[CGameSprite::DIR_NW].x = -19;
        stru_8E52D0[CGameSprite::DIR_NW].y = 14;
        stru_8E52D0[CGameSprite::DIR_NWW].x = -10;
        stru_8E52D0[CGameSprite::DIR_NWW].y = 18;
        stru_8E52D0[CGameSprite::DIR_W].x = 0;
        stru_8E52D0[CGameSprite::DIR_W].y = 20;
        stru_8E52D0[CGameSprite::DIR_SWW].x = 10;
        stru_8E52D0[CGameSprite::DIR_SWW].y = 18;
        stru_8E52D0[CGameSprite::DIR_SW].x = 19;
        stru_8E52D0[CGameSprite::DIR_SW].y = 14;
        stru_8E52D0[CGameSprite::DIR_SSW].x = 25;
        stru_8E52D0[CGameSprite::DIR_SSW].y = 8;
        stru_8E52D0[CGameSprite::DIR_S].x = -20;
        stru_8E52D0[CGameSprite::DIR_S].y = 0;
        stru_8E52D0[CGameSprite::DIR_SSE].x = -24;
        stru_8E52D0[CGameSprite::DIR_SSE].y = 8;
        stru_8E52D0[CGameSprite::DIR_SE].x = -19;
        stru_8E52D0[CGameSprite::DIR_SE].y = 14;
        stru_8E52D0[CGameSprite::DIR_SEE].x = -10;
        stru_8E52D0[CGameSprite::DIR_SEE].y = 18;

        byte_8E52E8 = TRUE;
    }
}
