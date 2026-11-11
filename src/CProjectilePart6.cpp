#include "CProjectilePart6.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameEffect.h"
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
    // TODO: Incomplete.
}

// 0x55AD90
std::list<LONG> CProjectileHelper84EEA8::Virtual4()
{
    // TODO: Incomplete.

    return std::list<LONG>();
}

// 0x55AF40
CProjectileHelper84EEB0::CProjectileHelper84EEB0()
{
    field_3A = FALSE;
}

// 0x55B1B0
std::list<LONG> CProjectileHelper84EEB0::Virtual4()
{
    // TODO: Incomplete.

    return std::list<LONG>();
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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x799CA0
void CProjectileSingleTarget850E84::DeliverEffects()
{
}

// 0x57BE80
void CProjectileSingleTarget850E84::Virtual88()
{
    // TODO: Incomplete.
}

// 0x57C1D0
void CProjectileSingleTarget850E84::Virtual8C(CPoint pt)
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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

// 0x580270
void CProjectileSingleTarget851444::OnArrival()
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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

// 0x583500
void CProjectileSingleTarget851664::Virtual88()
{
    // TODO: Incomplete.
}

// 0x8E52D0
std::vector<CPoint> CProjectileSingleTarget850E84::stru_8E52D0;

// 0x8E52E8
BOOLEAN CProjectileSingleTarget850E84::byte_8E52E8;

// 0x57BBC0
void CProjectileSingleTarget850E84::sub_57BBC0()
{
    // TODO: Incomplete.
}
