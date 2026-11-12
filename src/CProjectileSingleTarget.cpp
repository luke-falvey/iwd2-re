#include "CProjectileSingleTarget.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameAnimation.h"
#include "CGameSparkle.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CInfinity.h"
#include "CParticle.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CUtil.h"
#include "CVisualEffect.h"

// 0x577E40
CProjectileSingleTarget::CProjectileSingleTarget()
    : m_palette(CVidPalette::TYPE_RANGE)
{
    m_speed = 20;
    m_sourceId = 0;
    m_targetId = 0;
    m_posExact.x = 0;
    m_posDelta.x = 0;
    m_posDest.x = 0;
    field_AC.x = 0;
    field_B4.x = 0;
    field_BC.x = 0;
    field_16C = 0;
    field_1BE = 0;
    field_1C2 = 0;
    field_1D2 = 0;
    field_1D8 = 0;
    m_nDirection = 0;
    m_posExact.y = 0;
    m_posDelta.y = 0;
    m_posDest.y = 0;
    field_AC.y = 0;
    field_B4.y = 0;
    field_BC.y = 0;
    field_29D = FALSE;
    field_29E = 0x7FFF;
    m_pVidCell = NULL;
    field_196 = NULL;
    field_1D4 = 0;
    field_1C6 = 0;
    field_E6 = 0x20000;
    field_1DA = 0;
    field_29C = FALSE;
    field_1DE = 1;
    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, 16);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_1C2 = 1;
    field_182 = CGameObjectArray::INVALID_INDEX;
    m_bHasHeight = TRUE;
    field_2A0 = TRUE;
    field_2A1 = TRUE;
}

// 0x578110
CProjectileSingleTarget::CProjectileSingleTarget(CResRef resRef)
    : m_palette(CVidPalette::TYPE_RANGE)
{
    m_speed = 20;
    m_posExact.x = 0;
    m_posDelta.x = 0;
    m_posDest.x = 0;
    field_AC.x = 0;
    field_B4.x = 0;
    field_BC.x = 0;
    m_sourceId = 0;
    m_posExact.y = 0;
    m_posDelta.y = 0;
    m_posDest.y = 0;
    field_AC.y = 0;
    field_B4.y = 0;
    field_BC.y = 0;
    m_targetId = 0;
    field_16C = 0;
    field_29D = FALSE;
    field_29E = 0x7FFF;
    m_pVidCell = new CVidCell(resRef, FALSE);
    field_196 = NULL;
    field_1D4 = 0;
    field_1C6 = 0;
    field_E6 = 0x20000;
    field_1DA = 0;
    field_29C = FALSE;
    field_1DE = 1;
    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, 16);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_1C2 = 1;
    field_182 = CGameObjectArray::INVALID_INDEX;
    m_bHasHeight = TRUE;
    field_2A0 = TRUE;
    field_2A1 = TRUE;
}

// 0x5783E0
CProjectileSingleTarget::~CProjectileSingleTarget()
{
    if (m_pVidCell != NULL) {
        delete m_pVidCell;
    }

    if (field_196 != NULL) {
        delete field_196;
    }
}

// 0x572080
void CProjectileSingleTarget::SetVidCell(CResRef resRef)
{
    if (m_pVidCell != NULL) {
        delete m_pVidCell;
    }

    m_pVidCell = new CVidCell(resRef, FALSE);
}

// 0x578AB0
void CProjectileSingleTarget::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    m_pVidCell->FrameAdvance();

    if (m_posDest.x / 16 == m_pos.x / 16
        && m_posDest.y / 12 == m_pos.y / 12) {
        OnArrival();
        return;
    }

    LONG nDistance = (m_posDest.y - m_pos.y) * (m_posDest.y - m_pos.y) * 16 / 9
        + (m_posDest.x - m_pos.x) * (m_posDest.x - m_pos.x);
    if (nDistance <= (m_speed + 1) * (m_speed + 1)) {
        OnArrival();
        return;
    }

    if (field_170 == 0) {
        OnArrival();
        return;
    }

    if (field_29E-- == 0) {
        RemoveSelf();
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

        if (field_C4 == 0) {
            SHORT nHeight = static_cast<SHORT>(field_16C * nDistance / field_170);
            m_posZ += nHeight - field_16E;
            field_16E = nHeight;
        } else {
            if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE) {
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
                        CGameObjectArray::THREAD_ASYNCH,
                        &pTarget,
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED);

                if (rc == CGameObjectArray::SUCCESS) {
                    CRect rFx;
                    CPoint ptRef;
                    static_cast<CGameSprite*>(pTarget)->GetAnimation()->CalculateFxRect(rFx, ptRef, pTarget->m_posZ);

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    LONG posZ = m_posZ;
                    if (posZ != (rFx.bottom - rFx.top) >> 1) {
                        if (field_16C < 0) {
                            m_posZ = posZ + 1;
                        } else {
                            m_posZ = posZ - 1;
                        }

                        // FIXME: Original code overwrites the adjustment
                        // above with the previous value.
                        m_posZ = posZ;
                    }
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        Virtual84(ptTarget);
    } else {
        Virtual84(m_posDest);
    }

    if (field_E2 != 0) {
        CGameSparkle* pSparkle = new CGameSparkle(field_9A, 1 /* 0x84D600 */, 25 /* 0x84D5F0 */);
        pSparkle->AddToArea(field_EA, m_pos, m_posZ, CGameObject::LIST_FRONT);
    }

    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}

// 0x578480
void CProjectileSingleTarget::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileBAM.cpp
    // __LINE__: 140
    UTIL_ASSERT(pVidMode != NULL);

    CRect rGCBounds(0, 0, 0, 0);
    DWORD dwFlags = m_vfx.m_dwFlags;
    COLORREF rgbTint = RGB(CVidPalette::NO_TINT, CVidPalette::NO_TINT, CVidPalette::NO_TINT);

    // NOTE: Uninline.
    if (!field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(m_pos))) {
        return;
    }

    CPoint ptSearch(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
        m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);
    if (field_EA->m_search.GetMobileCost(ptSearch, m_terrainTable, 3, TRUE) == CPathSearch::COST_IMPASSABLE) {
        return;
    }

    if (field_1DE == 0) {
        return;
    }

    if (field_1D4 != 0) {
        dwFlags |= 0x4;
    }

    if (field_1D8 > 1) {
        sub_579860(NULL);
    }

    CInfinity* pInfinity = field_EA->GetInfinity();

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    CRect rFx;
    CPoint ptRef;
    sub_578970(rFx, ptRef, NULL);

    CPoint pos;
    LONG nPosZ;
    pos.x = m_pos.x;
    if (field_1D4 != 0) {
        pos.y = m_pos.y;
        nPosZ = m_posZ;
    } else {
        pos.y = m_pos.y - m_posZ;
        nPosZ = 0;
    }

    if (field_1C2 != 0 || m_bHasHeight) {
        pos.y += field_EA->GetHeightOffset(m_pos, m_listType);
    }

    if (field_1DA > CGameSprite::DIR_N) {
        rGCBounds.left = ptRef.x - rFx.right + pos.x + rFx.left;
    } else {
        rGCBounds.left = pos.x - ptRef.x;
    }

    if (field_1DA > CGameSprite::DIR_W && field_1DA < CGameSprite::DIR_E) {
        if (field_1D4 != 0) {
            ptRef.y = rFx.bottom - rFx.top - ptRef.y;
            rGCBounds.top = pos.y - m_posZ - ptRef.y;
        } else {
            rGCBounds.top = pos.y - ptRef.y;
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

    if (m_vfx.field_0 == TRUE || m_vfx.field_1 == TRUE) {
        rgbTint = field_EA->GetTintColor(pos, m_listType);
    }

    if (field_1DA > CGameSprite::DIR_N && field_2A1 == TRUE) {
        dwFlags |= CInfinity::MIRROR_FX;
    }

    if (field_1DA > CGameSprite::DIR_W && field_1DA < CGameSprite::DIR_E && field_2A0 == TRUE) {
        // NOTE: Not `CInfinity::MIRROR_FX_UPDOWN`.
        dwFlags ^= CInfinity::MIRROR_FX;
    }

    field_EA->GetInfinity()->FXPrep(rFx, dwFlags | CInfinity::FXPREP_COPYFROMBACK, a3, pos, ptRef);

    if (!field_EA->GetInfinity()->FXLock(rFx, dwFlags)) {
        return;
    }

    if (m_vfx.field_0 == TRUE) {
        m_pVidCell->SetTintColor(rgbTint);
    }

    if (field_1D4 != 0) {
        field_EA->GetInfinity()->FXRender(field_196, ptRef.x, ptRef.y, dwFlags, 0);
    }

    if (field_29C) {
        pInfinity = field_EA->GetInfinity();

        field_1E2.GetRes()->Demand();
        m_pVidCell->SetPalette(field_1E2.GetRes()->GetColorTable(),
            field_1E2.GetRes()->GetColorCount(),
            CVidPalette::TYPE_RESOURCE);
        pInfinity->FXRender(m_pVidCell, ptRef.x, ptRef.y - nPosZ, dwFlags, m_vfx.m_nTransValue);
        field_1E2.GetRes()->Release();
    } else {
        field_EA->GetInfinity()->FXRender(m_pVidCell, ptRef.x, ptRef.y - nPosZ, dwFlags, m_vfx.m_nTransValue);
    }

    field_EA->GetInfinity()->FXRenderClippingPolys(pos.x, pos.y, 0, ptRef, rGCBounds, FALSE, dwFlags);

    field_EA->GetInfinity()->FXUnlock(dwFlags, NULL, CPoint(0, 0));
    field_EA->GetInfinity()->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, dwFlags);
}

// 0x578970
void CProjectileSingleTarget::sub_578970(CRect& rFx, CPoint& ptRef, CVidCell* pVidCell)
{
    CSize size;

    if (pVidCell == NULL) {
        pVidCell = m_pVidCell;
    }

    if (field_1D4 == 0) {
        pVidCell->GetCurrentCenterPoint(ptRef, FALSE);
        pVidCell->GetCurrentFrameSize(size, FALSE);
        rFx.SetRect(0, 0, size.cx, size.cy);
    }

    if (field_1D4 != 0) {
        CPoint pt1;
        pVidCell->GetCurrentCenterPoint(pt1, FALSE);
        pt1.y += m_posZ;

        CPoint pt2;
        field_196->GetCurrentCenterPoint(pt2, FALSE);

        ptRef = pt1;
        if (pt2.x > ptRef.x) {
            ptRef.x = pt2.x;
        }
        if (pt2.y > ptRef.y) {
            ptRef.y = pt2.y;
        }

        pVidCell->GetCurrentFrameSize(size, FALSE);
        size.cx += ptRef.x - pt1.x;
        size.cy += ptRef.y - pt1.y;
        rFx.SetRect(0, 0, size.cx, size.cy);

        field_196->GetCurrentFrameSize(size, FALSE);
        size.cx += ptRef.x - pt2.x;
        size.cy += ptRef.y - pt2.y;
        if (size.cx > rFx.right) {
            rFx.right = size.cx;
        }
        if (size.cy > rFx.bottom) {
            rFx.bottom = size.cy;
        }
    }
}

// 0x578ED0
void CProjectileSingleTarget::Virtual84(CPoint dest)
{
    m_nDirection = GetDirection(dest);

    LONG dx = dest.x - m_pos.x;
    LONG dy = 4 * dest.y / 3 - 4 * m_pos.y / 3;
    LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

    if (nDistance == 0) {
        m_posDelta.x = 1;
        m_posDelta.y = 1;
    } else {
        m_posDelta.x = (dx << CParticle::RESOLUTION_INC) * m_speed / nDistance;
        m_posDelta.y = (dy << CParticle::RESOLUTION_INC) * m_speed / nDistance;
        m_posDest = dest;

        m_posDelta.x += field_AC.x;
        m_posDelta.y += field_AC.y;

        LONG nRangeX = field_BC.x - field_B4.x;
        LONG nRangeY = field_BC.y - field_B4.y;

        if (nRangeX > 0) {
            m_posDelta.x += field_B4.x + rand() % nRangeX;
        }

        if (nRangeY > 0) {
            m_posDelta.y += field_B4.y + rand() % nRangeY;
        }
    }

    m_posExact.x += m_posDelta.x;
    m_posExact.y += m_posDelta.y;

    if (field_AC.x < 0) {
        if (field_AC.x > -static_cast<LONG>(field_E0)) {
            field_AC.x = 0;
        } else {
            field_AC.x += field_E0;
        }
    } else if (field_AC.x > 0) {
        if (field_AC.x < static_cast<LONG>(field_E0)) {
            field_AC.x = 0;
        } else {
            field_AC.x -= field_E0;
        }
    }

    if (field_AC.y < 0) {
        if (field_AC.y > -static_cast<LONG>(field_E0)) {
            field_AC.y = 0;
        } else {
            field_AC.y += field_E0;
        }
    } else if (field_AC.y > 0) {
        if (field_AC.y < static_cast<LONG>(field_E0)) {
            field_AC.y = 0;
        } else {
            field_AC.y -= field_E0;
        }
    }

    m_pos.x = m_posExact.x >> CParticle::RESOLUTION_INC;
    m_pos.y = (3 * m_posExact.y / 4) >> CParticle::RESOLUTION_INC;

    if (field_182 != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(field_182,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pObject->m_posZ = m_posZ;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(field_182,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5791D0
void CProjectileSingleTarget::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pTarget;
    CGameObject* pSource;
    CPoint ptTarget;
    CPoint ptRef;
    CRect rFx;
    BYTE nTargetType;
    BYTE rc;

    m_targetId = target;
    m_sourceId = source;
    field_EA = pArea;

    if (target != CGameObjectArray::INVALID_INDEX) {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(target,
            CGameObjectArray::THREAD_ASYNCH,
            &pTarget,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        nTargetType = pTarget->GetObjectType();
        ptTarget = pTarget->GetPos();

        // NOTE: `ptRef` and `rFx` are left uninitialized when target is not
        // a sprite.
        if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE) {
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pTarget,
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED);

            if (rc == CGameObjectArray::SUCCESS) {
                static_cast<CGameSprite*>(pTarget)->GetAnimation()->CalculateFxRect(rFx, ptRef, pTarget->m_posZ);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            } else {
                ptRef.x = 32;
                ptRef.y = 50;
                rFx.SetRect(0, 0, 64, 64);
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        ptTarget = targetPos;
        ptRef.x = 32;
        ptRef.y = 50;
        rFx.SetRect(0, 0, 64, 64);
        nTargetType = CGameObject::TYPE_NONE;
    }

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        &pSource,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    LONG nSourceHeight = DetermineHeight(static_cast<CGameSprite*>(pSource));
    CPoint ptSource = pSource->GetPos();
    BOOL bSprite = pSource->GetObjectType() == CGameObject::TYPE_SPRITE;

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    CPoint ptStart;
    sub_52A640(m_sourceId, ptStart, TRUE);

    if (bSprite) {
        LONG dx = ptTarget.x / CPathSearch::GRID_SQUARE_SIZEX - ptStart.x / CPathSearch::GRID_SQUARE_SIZEX;
        LONG dy = ptTarget.y / CPathSearch::GRID_SQUARE_SIZEY - ptStart.y / CPathSearch::GRID_SQUARE_SIZEY;
        if (dx * dx + dy * dy <= 2) {
            ptStart = ptSource;
        }
    }

    if (field_17E.Compare("") != 0) {
        field_182 = CVisualEffect::sub_554D20(field_17E, pArea, ptStart, target, targetPos, nSourceHeight);

        CString sAreaName;
        pArea->m_resRef.CopyToString(sAreaName);

        CMessage86* pMessage = new CMessage86(CResRef(field_17E),
            sAreaName,
            ptStart,
            targetPos,
            static_cast<WORD>(nSourceHeight),
            m_id,
            target);
        g_pBaldurChitin->m_cMessageHandler.AddMessage(pMessage, FALSE);

        CGameObject* pVef;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(field_182,
                CGameObjectArray::THREAD_ASYNCH,
                &pVef,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            static_cast<CVisualEffect*>(pVef)->field_82 = m_speed;
            static_cast<CVisualEffect*>(pVef)->field_9A |= 0x1;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(field_182,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    LONG nDistance = (ptTarget.y - ptStart.y) * (ptTarget.y - ptStart.y) * 16 / 9
        + (ptTarget.x - ptStart.x) * (ptTarget.x - ptStart.x);
    field_170 = nDistance;

    if (field_29D) {
        field_29E = static_cast<SHORT>(sqrt(static_cast<double>(nDistance)) + 0.5) / m_speed + 1;
    }

    if (nTargetType == CGameObject::TYPE_SPRITE) {
        field_16C = static_cast<SHORT>(nSourceHeight - ((rFx.bottom - rFx.top) >> 1));
        field_16E = field_16C;
    } else {
        field_170 = 1;
        field_16C = 0;
        field_16E = 0;
    }

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    AddToArea(pArea, ptStart, nSourceHeight, CGameObject::LIST_FRONT);

    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    m_posExact.x = ptStart.x << CParticle::RESOLUTION_INC;
    m_posDest.x = ptTarget.x;
    m_posExact.y = 4 * (ptStart.y << CParticle::RESOLUTION_INC) / 3;
    m_posDest.y = ptTarget.y;
    m_nDirection = GetDirection(ptTarget);
}

// 0x579860
void CProjectileSingleTarget::sub_579860(CVidCell* pVidCell)
{
    if (pVidCell == NULL) {
        pVidCell = m_pVidCell;
    }

    if (field_1D8 == 1) {
        return;
    }

    if (field_1DA == m_nDirection) {
        return;
    }

    if (field_1D8 == 16) {
        switch (m_nDirection) {
        case 0:
            pVidCell->SequenceSet(0);
            break;
        case 1:
        case 15:
            pVidCell->SequenceSet(1);
            break;
        case 2:
        case 14:
            pVidCell->SequenceSet(2);
            break;
        case 3:
        case 13:
            pVidCell->SequenceSet(3);
            break;
        case 4:
        case 12:
            pVidCell->SequenceSet(4);
            break;
        case 5:
        case 11:
            if (field_2A0) {
                pVidCell->SequenceSet(3);
            } else {
                pVidCell->SequenceSet(5);
            }
            break;
        case 6:
        case 10:
            if (field_2A0) {
                pVidCell->SequenceSet(2);
            } else {
                pVidCell->SequenceSet(6);
            }
            break;
        case 7:
        case 9:
            if (field_2A0) {
                pVidCell->SequenceSet(1);
            } else {
                pVidCell->SequenceSet(7);
            }
            break;
        case 8:
            if (field_2A0) {
                pVidCell->SequenceSet(0);
            } else {
                pVidCell->SequenceSet(8);
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileBAM.cpp
            // __LINE__: 907
            UTIL_ASSERT(FALSE);
        }
    } else if (field_1D8 == 8) {
        switch (m_nDirection / 2 * 2) {
        case 0:
        case 8:
            pVidCell->SequenceSet(0);
            break;
        case 2:
        case 6:
        case 10:
        case 14:
            pVidCell->SequenceSet(2);
            break;
        case 4:
        case 12:
            pVidCell->SequenceSet(4);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileBAM.cpp
            // __LINE__: 942
            UTIL_ASSERT(FALSE);
        }
    } else if (field_1D8 == 2) {
        switch (m_nDirection) {
        case 0:
        case 4:
        case 8:
        case 12:
            pVidCell->SequenceSet(0);
            break;
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
        case 7:
        case 9:
        case 10:
        case 11:
        case 13:
        case 14:
        case 15:
            pVidCell->SequenceSet(1);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileBAM.cpp
            // __LINE__: 974
            UTIL_ASSERT(FALSE);
        }
    }

    if (pVidCell == m_pVidCell) {
        field_1DA = m_nDirection;
    }
}

// 0x5297D0
DWORD CProjectileSingleTarget::Virtual80()
{
    return field_E6;
}
