#include "CProjectilePart1.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameChunk.h"
#include "CGameEffect.h"
#include "CGameSparkle.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CInfinity.h"
#include "CParticle.h"
#include "CPathSearch.h"
#include "CProjectilePart6.h"
#include "CVidPalette.h"
#include "CUtil.h"

// 0x529A90
CProjectileBAM84DA4C::CProjectileBAM84DA4C(CResRef resRef)
    : CProjectileBAM(resRef)
{
}

// 0x536D40
void CProjectileBAM84DA4C::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    m_pVidCell->FrameAdvance();

    if (m_posZ <= m_speed) {
        OnArrival();
        return;
    }

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

        m_pos = pTarget->GetPos();
        m_posZ -= m_speed;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        m_posZ = m_posZ - m_speed;
    }

    if (field_E2 != 0) {
        CGameSparkle* pSparkle = new CGameSparkle(field_9A, 1 /* 0x84D600 */, 25 /* 0x84D5F0 */);
        pSparkle->AddToArea(field_EA, m_pos, m_posZ, CGameObject::LIST_FRONT);
    }

    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}

// 0x5369A0
void CProjectileBAM84DA4C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    field_EA = pArea;
    m_sourceId = source;
    m_targetId = target;

    CGameObject* pTarget;
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(target,
        CGameObjectArray::THREAD_ASYNCH,
        &pTarget,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    CPoint ptTarget = pTarget->GetPos();

    CRect rFx;
    CPoint ptReference;

    if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                &pTarget,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED);

        if (rc == CGameObjectArray::SUCCESS) {
            static_cast<CGameSprite*>(pTarget)->m_animation.CalculateFxRect(rFx, ptReference, pTarget->m_posZ);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            ptReference.x = 32;
            ptReference.y = 50;
            rFx.left = 0;
            rFx.top = 0;
            rFx.right = 64;
            rFx.bottom = 64;
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    CRect rViewPort;
    CopyRect(&rViewPort, &field_EA->m_cInfinity.rViewPort);

    LONG nViewHeight = rViewPort.bottom - rViewPort.top;

    // NOTE: A second copy of the target position.
    CPoint ptStart(ptTarget.x, ptTarget.y);

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) != CGameObjectArray::SUCCESS) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    AddToArea(pArea, ptStart, nViewHeight, CGameObject::LIST_FRONT);

    CResRef resFireSound = m_fireSoundRef;
    BOOL bLoop = m_loopFireSound;

    m_sound.Stop();

    if (resFireSound != "") {
        // NOTE: Inlined.
        m_sound.SetResRef(resFireSound, TRUE, TRUE);

        if (bLoop) {
            m_sound.SetLoopingFlag(TRUE);
        }

        m_sound.SetChannel(15, reinterpret_cast<DWORD_PTR>(field_EA));
        m_sound.Play(m_pos.x, m_pos.y, 0, 0);
    }

    m_posExact.x = ptStart.x << CParticle::RESOLUTION_INC;
    m_posExact.y = (ptStart.y << CParticle::RESOLUTION_INC) * 4 / 3;
    m_posDest.x = ptTarget.x;
    m_posDest.y = ptTarget.y;
    m_nDirection = 0;
}

// 0x52CA10
CProjectileBAM84DB54::CProjectileBAM84DB54(WORD a1)
    : CProjectileBAM(CResRef((BYTE*)"SPMAGMIS"))
{
    field_196 = NULL;
    field_1D8 = 0;
    field_1DA = 0;
    m_nDirection = 0;
    field_1DE = 0;
    field_29C = FALSE;
    field_29D = FALSE;
    field_29E = 0;
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
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 5032
        UTIL_ASSERT(FALSE);
    }

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x52CCE0
CProjectileBAM84DBDC::CProjectileBAM84DBDC(CResRef resRef)
    : CProjectileBAM(resRef)
{
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    field_2A4 = 78;
    field_2A6 = 1;
    field_2A8 = 0;
    field_2AC = 100;
    field_2AE = 0;
    field_2B4.Set(CAIObjectType::ANYONE);
    field_2A0 = 256;
    field_2A2 = 256;
    field_2B0 = 0;
    field_2F0 = 0;
    field_1BE = 0;
    field_2F6 = FALSE;
    field_3D4 = FALSE;
    field_2F2 = 0;
    field_2F1 = 0xFF;
}

// 0x52DD60
void CProjectileBAM84DBDC::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (field_2A8 == 0) {
        m_pVidCell->FrameAdvance();

        LONG nDistance = (m_posDest.y - m_pos.y) * (m_posDest.y - m_pos.y) * 16 / 9
            + (m_posDest.x - m_pos.x) * (m_posDest.x - m_pos.x);
        if (nDistance <= (m_speed + 1) * (m_speed + 1)) {
            OnArrival();
            return;
        }

        Virtual84(m_posDest);

        if (field_E2 != 0) {
            CGameSparkle* pSparkle = new CGameSparkle(field_9A, 1 /* 0x84D600 */, 25 /* 0x84D5F0 */);
            pSparkle->AddToArea(field_EA, m_pos, m_posZ, CGameObject::LIST_FRONT);
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        return;
    }

    if (field_2F6 != 0) {
        // NOTE: Virtual call through the object's vtable in the original.
        field_2FA.FrameAdvance();
    }

    if (field_3D4 != 0) {
        // NOTE: Virtual call through the object's vtable in the original.
        field_3D8.FrameAdvance();
    }

    if (field_2AE != 0) {
        field_2AE--;
        return;
    }

    field_2AE = field_2AC;

    if (sub_52D430(FALSE)) {
        field_2A6--;
    }

    if (field_2A6 <= 0) {
        RemoveFromArea();

        BYTE rc = pGame->GetObjectArray()->Delete(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            NULL,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            delete this;
        }

        return;
    }

    if (field_2F0 == 0) {
        return;
    }

    BYTE aColors[7];

    switch (field_2F1) {
    case 0:
        memset(aColors, 'C', sizeof(aColors));
        field_2F0 = 0;
        break;
    case 1:
        memset(aColors, '5', sizeof(aColors));
        field_2F0 = 0;
        break;
    case 2:
        memset(aColors, '1', sizeof(aColors));
        field_2F0 = 0;
        break;
    case 3:
        memset(aColors, 'G', sizeof(aColors));
        field_2F0 = 0;
        break;
    case 4:
        memset(aColors, 'B', sizeof(aColors));
        field_2F0 = 0;
        break;
    case 5:
        memset(aColors, 'A', sizeof(aColors));
        field_2F0 = 0;
        break;
    case 0xFF:
        field_2F0 = 0;
        return;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 5675
        UTIL_ASSERT(FALSE);
    }

    // NOTE: The object is not kept.
    new CGame85BFE8(field_2F1,
        aColors,
        field_EA,
        m_pos,
        field_2A0,
        m_speed,
        1 /* 0x85BD6D */,
        field_2AC * field_2A6);
}

// 0x52CF80
void CProjectileBAM84DBDC::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    CProjectileBAM::Render(pArea, pVidMode, a3);

    if (field_2A8 == 0 || field_2F6 == 0) {
        return;
    }

    CRect rGCBounds(0, 0, 0, 0);
    COLORREF rgbTint = RGB(CVidPalette::NO_TINT, CVidPalette::NO_TINT, CVidPalette::NO_TINT);

    DWORD dwFlags = Virtual80();

    // NOTE: Uninline.
    if (!field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(m_pos))) {
        return;
    }

    CPoint ptSearch(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
        m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);
    if (field_EA->m_search.GetMobileCost(ptSearch, m_terrainTable, 3, TRUE) == CPathSearch::COST_IMPASSABLE) {
        return;
    }

    if (field_1C6 != 0) {
        dwFlags |= 0x200;
    }

    if (field_1BE != 0) {
        dwFlags |= 0x10000;
    }

    if (field_3D4 != 0) {
        dwFlags |= 0x4;
    }

    CInfinity* pInfinity = field_EA->GetInfinity();

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    CRect rFx;
    CPoint ptRef;
    sub_52B6B0(rFx, ptRef, NULL);

    CPoint pos;
    LONG nPosZ;
    pos.x = m_pos.x;
    if ((field_1C6 != 0 && !g_pBaldurChitin->cVideo.m_bIs3dAccelerated) || field_3D4 != 0) {
        pos.y = m_pos.y;
        nPosZ = m_posZ;
    } else {
        pos.y = m_pos.y - m_posZ;
        nPosZ = 0;
    }

    if (field_1C2 != 0) {
        pos.y += field_EA->GetHeightOffset(m_pos, m_listType);
    }

    if (field_1DA > CGameSprite::DIR_N) {
        rGCBounds.left = ptRef.x - rFx.right + pos.x + rFx.left;
    } else {
        rGCBounds.left = pos.x - ptRef.x;
    }

    if (field_1DA > CGameSprite::DIR_W && field_1DA < CGameSprite::DIR_E) {
        if ((field_1C6 != 0 && !g_pBaldurChitin->cVideo.m_bIs3dAccelerated) || field_3D4 != 0) {
            ptRef.y = rFx.bottom - rFx.top - ptRef.y;
            rGCBounds.top = pos.y - m_posZ - ptRef.y;
        } else {
            rGCBounds.top = rFx.top - rFx.bottom + ptRef.y + pos.y;
        }
        nPosZ = -nPosZ;
    } else {
        rGCBounds.top = pos.y - ptRef.y;
    }

    rGCBounds.right = rFx.right - rFx.left + rGCBounds.left;
    rGCBounds.bottom = rFx.bottom - rFx.top + rGCBounds.top;

    if (!IntersectRect(&rScreen, &rGCBounds, &rScreen)) {
        return;
    }

    if (field_1BE != 0 || field_1C6 != 0) {
        rgbTint = field_EA->GetTintColor(pos, m_listType);
    }

    if (field_1DA > CGameSprite::DIR_N) {
        dwFlags |= CInfinity::MIRROR_FX;
    }

    if (field_1DA > CGameSprite::DIR_W && field_1DA < CGameSprite::DIR_E) {
        dwFlags |= CInfinity::MIRROR_FX_UPDOWN;
    }

    field_EA->GetInfinity()->FXPrep(rFx, CInfinity::FXPREP_COPYFROMBACK | dwFlags, a3, pos, ptRef);

    if (!field_EA->GetInfinity()->FXLock(rFx, dwFlags)) {
        return;
    }

    if (field_1BE != 0) {
        field_2FA.SetTintColor(rgbTint);
    }

    if (field_3D4 != 0) {
        field_EA->GetInfinity()->FXRender(&field_3D8, ptRef.x, ptRef.y, dwFlags, 0);
    }

    field_EA->GetInfinity()->FXRender(&field_2FA, ptRef.x, ptRef.y - nPosZ, dwFlags, 128);

    field_EA->GetInfinity()->FXRenderClippingPolys(pos.x, pos.y, 0, ptRef, rGCBounds, FALSE, dwFlags);

    field_EA->GetInfinity()->FXUnlock(dwFlags, NULL, CPoint(0, 0));
    field_EA->GetInfinity()->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, dwFlags);
}

// 0x52D9F0
void CProjectileBAM84DBDC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pSource;
    BYTE rc;

    m_sourceId = source;
    m_targetId = target;
    field_EA = pArea;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pSource,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    CPoint pt;
    sub_52A640(m_sourceId, pt, TRUE);
    CPoint ptStart = pt;

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;

        // NOTE: `m_sourceId` is read after `this` is deleted.
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    // NOTE: Uninline.
    LONG nSourceHeight = DetermineHeight(static_cast<CGameSprite*>(pSource));

    CGameObject::AddToArea(pArea, ptStart, nSourceHeight, CGameObject::LIST_FRONT);

    // NOTE: Inlined.
    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    m_posDest.x = targetPos.x;
    m_pos.x = ptStart.x;
    m_posExact.x = ptStart.x << CParticle::RESOLUTION_INC;
    m_posExact.y = 4 * (ptStart.y << CParticle::RESOLUTION_INC) / 3;
    m_posDest.y = targetPos.y;
    m_pos.y = ptStart.y;

    LONG dy = 4 * m_posDest.y / 3 - 4 * m_pos.y / 3;
    LONG dx = m_posDest.x - m_pos.x;

    LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);
    if (nDistance == 0) {
        OnArrival();
    } else {
        m_posDelta.x = (dx << CParticle::RESOLUTION_INC) * m_speed / nDistance;
        m_posDelta.y = (dy << CParticle::RESOLUTION_INC) * m_speed / nDistance;

        // NOTE: Uninline.
        m_nDirection = GetDirection(m_posDest);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x52D430
BOOL CProjectileBAM84DBDC::sub_52D430(BOOLEAN a1)
{
    CTypedPtrList<CPtrList, LONG*> targets(10);
    BOOL bResult;
    BOOLEAN bFire = FALSE;

    if (field_2B0 != 0) {
        // NOTE: The range arguments are `WORD` in the original (zero-extended).
        field_EA->GetAllInRange(m_posVertList,
            m_pos,
            field_2B4,
            static_cast<WORD>(field_2A2),
            m_terrainTable,
            targets,
            TRUE,
            field_2F2);
        field_EA->GetAllInRangeBack(m_pos,
            field_2B4,
            static_cast<WORD>(field_2A2),
            m_terrainTable,
            targets,
            TRUE,
            FALSE,
            field_2F2);

        bResult = !targets.IsEmpty();
        if (!bResult) {
            return bResult;
        }
    } else {
        bResult = TRUE;
        bFire = TRUE;
    }

    targets.RemoveAll();

    field_EA->GetAllInRange(m_posVertList,
        m_pos,
        field_2B4,
        static_cast<WORD>(field_2A0),
        m_terrainTable,
        targets,
        TRUE,
        field_2F2);
    field_EA->GetAllInRangeBack(m_pos,
        field_2B4,
        static_cast<WORD>(field_2A0),
        m_terrainTable,
        targets,
        TRUE,
        FALSE,
        field_2F2);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nTargetId = static_cast<LONG>(reinterpret_cast<INT_PTR>(targets.GetNext(pos)));

        CGameObject* pObject;
        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nTargetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return bResult;
        }

        if ((CGameObject::TYPE_AIBASE & pObject->GetObjectType()) == 0) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            continue;
        }

        CPoint ptTarget;
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            ptTarget = static_cast<CGameSprite*>(pObject)->m_posOld;
        } else {
            ptTarget = pObject->GetPos();
        }

        BOOL bSkip = FALSE;
        if (a1 != 0) {
            LONG dy = ptTarget.y - m_pos.y;
            LONG dx = ptTarget.x - m_pos.x;
            if (dy * dy * 16 / 9 + dx * dx >= field_2A0 * field_2A0) {
                if (!field_EA->CheckLOS(m_pos, ptTarget, m_terrainTable, FALSE)) {
                    bSkip = TRUE;
                }
            }
        }

        if (!bSkip) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                static_cast<CGameSprite*>(pObject)->m_posOld = pObject->GetPos();
            }

            bFire = TRUE;

            CProjectile* pProjectile = CProjectile::DecodeProjectile(static_cast<WORD>(field_2A4 + 1), NULL, FALSE);
            pProjectile->field_186 = field_186;
            pProjectile->field_18A = field_18A;
            pProjectile->m_fireSoundRef = CResRef("");

            POSITION posEffect = m_effectList.GetHeadPosition();
            while (posEffect != NULL) {
                CGameEffect* pEffect = m_effectList.GetNext(posEffect);

                // NOTE: Uninline.
                pProjectile->AddEffect(pEffect->Copy());
            }

            pProjectile->Fire(field_EA,
                m_id,
                nTargetId,
                pObject->GetPos(),
                50,
                0);
        }

        // NOTE: Releases `m_callBackProjectile` rather than the target.
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_callBackProjectile,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    if (bFire) {
        Virtual88();
    }

    return bResult;
}

// 0x52D7F0
void CProjectileBAM84DBDC::OnArrival()
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

    field_2A8 = 1;
    field_1DE = 0;

    // NOTE: Inlined.
    PlaySound(m_arrivalSoundRef, m_loopArrivalSound, TRUE);
}

// 0x78E730
void CProjectileBAM84DBDC::Virtual88()
{
}

// 0x5297B0
CProjectileBAM84D938::CProjectileBAM84D938(CResRef resRef)
    : CProjectileBAM84DBDC(resRef)
{
}

// 0x52E160
void CProjectileBAM84D938::Virtual88()
{
    WORD nProjectileType;

    switch (field_9A) {
    case 1:
    case 9:
    case 10:
        nProjectileType = 0xC3;
        break;
    case 2:
        nProjectileType = 0xC4;
        break;
    case 3:
    case 11:
        nProjectileType = 0xC7;
        break;
    case 4:
        nProjectileType = 0xC5;
        break;
    case 5:
        nProjectileType = 0xC6;
        break;
    case 6:
        nProjectileType = 0xC8;
        break;
    case 7:
    case 8:
        nProjectileType = 0xC9;
        break;
    case 12:
        nProjectileType = 0xCA;
        break;
    default:
        return;
    }

    CProjectile* pProjectile = CProjectile::DecodeProjectile(nProjectileType + 1, NULL, FALSE);
    if (pProjectile != NULL) {
        pProjectile->Fire(field_EA, m_sourceId, m_targetId, m_posDest, 30, field_9A);
    }
}

// 0x52E230
CProjectileBAM84DC68::CProjectileBAM84DC68()
    : CProjectileBAM84DBDC(CResRef((BYTE*)"SPFIREBL"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 67, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1CA = 15;
    field_1CE = 11;
    field_4BA = 67;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 1;
    field_1D2 = 128;
    field_1D4 = 0;
    field_1D8 = 16;
    m_speed *= 2;
    m_fireSoundRef = "";
    memset(field_4B2, 'A', sizeof(field_4B2));
    field_4B9 = 0;
    field_2A4 = 78;
}

// 0x52E4D0
void CProjectileBAM84DC68::AIUpdate()
{
    CString sEmpty("");
    CPoint ptZero(0, 0);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (field_2A8 == 0) {
        m_pVidCell->FrameAdvance();

        LONG nDistance = (m_posDest.y - m_pos.y) * (m_posDest.y - m_pos.y) * 16 / 9
            + (m_posDest.x - m_pos.x) * (m_posDest.x - m_pos.x);
        if (nDistance <= (m_speed + 1) * (m_speed + 1)) {
            OnArrival();
            return;
        }

        Virtual84(m_posDest);

        field_4B9++;
        if (field_4B9 == 1) {
            field_4B9 = 0;

            // NOTE: The objects are not kept.
            LONG r1 = rand() % 5;
            LONG r2 = rand() % 5;
            CPoint ptPos1(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x * 4 / 3,
                ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y * 4 / 3);
            new CGame85BF7C(0x300, field_4B2, sEmpty, field_EA, ptPos1, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);

            r1 = rand() % 5;
            r2 = rand() % 5;
            CPoint ptPos2(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x,
                ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y);
            new CGame85BF7C(0x300, field_4B2, sEmpty, field_EA, ptPos2, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);

            r1 = rand() % 5;
            r2 = rand() % 5;
            CPoint ptPos3(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x * 2 / 3,
                ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y * 2 / 3);
            new CGame85BF7C(0x300, field_4B2, sEmpty, field_EA, ptPos3, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);
        }

        if (field_E2 != 0) {
            CGameSparkle* pSparkle = new CGameSparkle(field_9A, 1 /* 0x84D600 */, 25 /* 0x84D5F0 */);
            pSparkle->AddToArea(field_EA, m_pos, m_posZ, CGameObject::LIST_FRONT);
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        return;
    }

    if (field_2AE == 0) {
        field_2AE = field_2AC;

        if (sub_52D430(FALSE)) {
            field_2A6--;
        }

        if (field_2A6 <= 0) {
            RemoveFromArea();

            BYTE rc = pGame->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                delete this;
            }

            return;
        }
    }

    field_2AE--;
}

// 0x52E940
void CProjectileBAM84DC68::Virtual88()
{
    BYTE aColors[7];
    memset(aColors, field_4BA, sizeof(aColors));

    // NOTE: The object is not kept.
    new CGame85BFE8(0 /* 0x85BD70 */,
        aColors,
        field_EA,
        m_pos,
        field_2A0,
        static_cast<BYTE>(m_speed) >> 1,
        1 /* 0x85BD6D */,
        0);
}

// 0x52E9F0
CProjectileBAM84DCF4::CProjectileBAM84DCF4()
    : CProjectileBAM84DBDC(CResRef((BYTE*)"SPFLMARR"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 67, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_palette.SetRange(2, 57, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    m_speed *= 3;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    memset(field_4B2, 'A', sizeof(field_4B2));
    field_4B9 = 0;
    field_2A4 = 78;
}

// 0x52EC80
void CProjectileBAM84DCF4::AIUpdate()
{
    CString sEmpty("");
    CPoint ptZero(0, 0);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (field_2A8 == 0) {
        m_pVidCell->FrameAdvance();

        LONG nDistance = (m_posDest.y - m_pos.y) * (m_posDest.y - m_pos.y) * 16 / 9
            + (m_posDest.x - m_pos.x) * (m_posDest.x - m_pos.x);
        if (nDistance <= (m_speed + 1) * (m_speed + 1)) {
            OnArrival();
            return;
        }

        if (m_targetId != CGameObjectArray::INVALID_INDEX) {
            CGameObject* pTarget;
            BYTE rc = pGame->GetObjectArray()->GetShare(m_targetId,
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

            CPoint ptTarget = pTarget->GetPos();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            Virtual84(ptTarget);
        } else {
            Virtual84(m_posDest);
        }

        field_4B9++;
        if (field_4B9 == 1) {
            field_4B9 = 0;

            // NOTE: The objects are not kept.
            LONG r1 = rand() % 5;
            LONG r2 = rand() % 5;
            CPoint ptPos1(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x * 4 / 3,
                ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y * 4 / 3);
            new CGame85BF7C(0x300, field_4B2, sEmpty, field_EA, ptPos1, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);

            r1 = rand() % 5;
            r2 = rand() % 5;
            CPoint ptPos2(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x,
                ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y);
            new CGame85BF7C(0x300, field_4B2, sEmpty, field_EA, ptPos2, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);

            r1 = rand() % 5;
            r2 = rand() % 5;
            CPoint ptPos3(((m_pos.x + r2 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.x * 2 / 3,
                ((m_pos.y + r1 - 2) << CGameSprite::EXACT_SCALE) - m_posDelta.y * 2 / 3);
            new CGame85BF7C(0x300, field_4B2, sEmpty, field_EA, ptPos3, -m_posZ, ptZero, 0, 0, 1 /* 0x85BD6D */);
        }

        if (field_E2 != 0) {
            CGameSparkle* pSparkle = new CGameSparkle(field_9A, 1 /* 0x84D600 */, 25 /* 0x84D5F0 */);
            pSparkle->AddToArea(field_EA, m_pos, m_posZ, CGameObject::LIST_FRONT);
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        return;
    }

    if (field_2AE == 0) {
        field_2AE = field_2AC;

        if (sub_52D430(FALSE)) {
            field_2A6--;
        }

        if (field_2A6 <= 0) {
            RemoveFromArea();

            BYTE rc = pGame->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                delete this;
            }

            return;
        }
    }

    field_2AE--;
}

// 0x52F1C0
void CProjectileBAM84DCF4::Virtual88()
{
    BYTE aColors[7];
    memset(aColors, 'C', sizeof(aColors));

    // NOTE: The object is not kept.
    new CGame85BFE8(0 /* 0x85BD70 */,
        aColors,
        field_EA,
        m_pos,
        field_2A0,
        static_cast<BYTE>(m_speed) >> 1,
        1 /* 0x85BD6D */,
        0);
}

// 0x52F260
CProjectileBAM84DD80::CProjectileBAM84DD80(const CResRef& a1, const CResRef& a2, SHORT a3)
    : CProjectileBAM84DBDC(a1)
{
    field_196 = new CVidCell(a2, FALSE);
    field_196->SequenceSet(0);
    m_pVidCell->SequenceSet(0);
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 1;
    field_1D8 = 1;
    m_speed *= 2;
    m_fireSoundRef = "";
    field_4B2 = a3;
    field_2A4 = 78;
    field_2A6 = 1;
    field_2AC = 100;
    field_2B0 = 1;

    // NOTE: Uninline.
    field_2FA.SetResRef(a1, FALSE, TRUE, TRUE);
    field_2F6 = CResRef(a1) != "";

    // NOTE: Uninline.
    field_3D8.SetResRef(a2, FALSE, TRUE, TRUE);
    field_3D4 = CResRef(a2) != "";
}

// 0x52F9E0
void CProjectileBAM84DD80::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (field_2A8 == 0) {
        m_pVidCell->FrameAdvance();

        LONG nDistance = (m_posDest.y - m_pos.y) * (m_posDest.y - m_pos.y) * 16 / 9
            + (m_posDest.x - m_pos.x) * (m_posDest.x - m_pos.x);
        if (nDistance <= (m_speed + 1) * (m_speed + 1)) {
            OnArrival();
            return;
        }

        Virtual84(m_posDest);

        if (field_E2 != 0) {
            CGameSparkle* pSparkle = new CGameSparkle(field_9A, 1 /* 0x84D600 */, 25 /* 0x84D5F0 */);
            pSparkle->AddToArea(field_EA, m_pos, m_posZ, CGameObject::LIST_FRONT);
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        return;
    }

    if (field_2F6 != 0) {
        // NOTE: Virtual call through the object's vtable in the original.
        field_2FA.FrameAdvance();
    }

    if (field_3D4 != 0) {
        // NOTE: Virtual call through the object's vtable in the original.
        field_3D8.FrameAdvance();
    }

    if (field_2AE != 0) {
        field_2AE--;
        return;
    }

    if (sub_52D430(FALSE)) {
        field_2A6--;
        field_2AE = field_2AC;
    }

    if (field_2A6 <= 0) {
        RemoveFromArea();

        BYTE rc = pGame->GetObjectArray()->Delete(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            NULL,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            delete this;
        }

        return;
    }
}

// 0x52F760
void CProjectileBAM84DD80::Virtual88()
{
    CSound cSound;
    CString sEmpty("");

    // NOTE: Inlined.
    cSound.SetResRef(CResRef(field_4B4), TRUE, TRUE);
    cSound.SetFireForget(TRUE);
    cSound.SetChannel(14, reinterpret_cast<DWORD_PTR>(field_EA));
    cSound.Play(m_pos.x, m_pos.y, 0, 0);

    // NOTE: Uninitialized in the original, the colors are passed to the chunks
    // as they are.
    BYTE aColors[7];

    BYTE nCount = static_cast<BYTE>(rand() % 16 + 10);
    if (nCount != 0) {
        do {
            LONG r1 = rand() % 3;
            LONG r2 = rand() % 11;
            LONG r3 = rand() % 11;
            CPoint ptVelocity(r3 - 5, r2 - 5);

            BYTE nDuration = CGameObject::CHUNK_DURATION + 1;

            // NOTE: The object registers itself in the object array.
            new CGameChunk(field_4B2,
                aColors,
                sEmpty,
                field_EA,
                m_pos,
                ptVelocity,
                -m_posZ,
                r1 * 3 + 3,
                static_cast<BYTE>(nDuration * 4),
                nDuration);

            nCount--;
        } while (nCount != 0);
    }
}

// 0x52FC20
CProjectileBAM84DE0C::CProjectileBAM84DE0C(BYTE* resRef)
    : CProjectileBAM(CResRef(resRef))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 67, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1CA = 15;
    field_1CE = 11;
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 1;
    field_1D2 = 128;
    field_1D4 = 0;
    field_1D8 = 16;
    m_fireSoundRef = "";
    m_speed *= 3;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x52FE90
CProjectileBAM84DE94::CProjectileBAM84DE94()
    : CProjectileBAM(CResRef((BYTE*)"SPLightB"))
{
    field_2A0 = -1;
}

// 0x530010
void CProjectileBAM84DE94::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CProjectileSingleTarget85101C* pProjectile = new CProjectileSingleTarget85101C();

    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* pEffect = m_effectList.GetNext(pos);

        CGameEffect* pCopy = pEffect->Copy();
        pCopy->m_projectileType = pProjectile->m_projectileType;
        pProjectile->m_effectList.AddTail(pCopy);
    }

    pProjectile->field_2B2 = field_2A0;

    // NOTE: The original passes `source` for both the source and the target.
    pProjectile->Fire(pArea, source, source, targetPos, nHeight, nType);
}

// 0x5300E0
CProjectileBAM84DF1C::CProjectileBAM84DF1C()
    : CProjectileBAM(CResRef((BYTE*)"SPFIREBL"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 67, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1DE = 0;
    field_1D8 = 16;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    field_29D = TRUE;
}

// 0x530300
CProjectileBAM84DFA4::CProjectileBAM84DFA4()
    : CProjectileBAM(CResRef((BYTE*)"SPBOOM"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 67, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
    m_pVidCell->SetPalette(m_palette);
    field_1BE = 0;
    field_1C2 = 0;
    field_1C6 = 0;
    field_1D4 = 0;
    m_fireSoundRef = "";
    field_1D8 = 1;
}

// 0x5306F0
void CProjectileBAM84DFA4::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    m_pVidCell->FrameAdvance();

    if (m_pVidCell->IsEndOfSequence(FALSE)) {
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

    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}

// 0x530510
void CProjectileBAM84DFA4::Virtual88(CGameArea* pArea, CPoint pos)
{
    m_pos = pos;
    field_EA = pArea;

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) != CGameObjectArray::SUCCESS) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    AddToArea(pArea, m_pos, 0, CGameObject::LIST_FRONT);

    CResRef resFireSound = m_fireSoundRef;
    BOOL bLoop = m_loopFireSound;

    m_sound.Stop();

    if (resFireSound != "") {
        // NOTE: Inlined.
        m_sound.SetResRef(resFireSound, TRUE, TRUE);

        if (bLoop) {
            m_sound.SetLoopingFlag(TRUE);
        }

        m_sound.SetChannel(15, reinterpret_cast<DWORD_PTR>(field_EA));
        m_sound.Play(m_pos.x, m_pos.y, 0, 0);
    }
}
