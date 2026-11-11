#include "CProjectilePart3.h"

#include "CBaldurChitin.h"
#include "CCreatureFile.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CInfinity.h"
#include "CParticle.h"
#include "CPathSearch.h"
#include "CProjectilePart6.h"
#include "CUtil.h"
#include "IcewindMisc.h"

// 0x529CE0
CProjectile84DAD4::CProjectile84DAD4()
{
}

// 0x5818F0
void CProjectile84DAD4::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
        CPoint pt;

        pt.x = 0xB72;
        pt.y = 0x737;
        sub_5819B0(pArea, pt, 0xE);

        pt.x = 0xAE1;
        pt.y = 0x6FA;
        sub_5819B0(pArea, pt, 0xE);

        pt.x = 0xAE8;
        pt.y = 0x772;
        sub_5819B0(pArea, pt, 0xE);

        pt.x = 0xA61;
        pt.y = 0x7F5;
        sub_5819B0(pArea, pt, 0xC);

        pt.x = 0xB00;
        pt.y = 0x814;
        sub_5819B0(pArea, pt, 0xC);
    }
}

// 0x5819B0
void CProjectile84DAD4::sub_5819B0(CGameArea* pArea, const CPoint& pt, SHORT nFacing)
{
    static CString sCreature("corcat");

    CCreatureFile creatureFile;

    // NOTE: Uninline.
    creatureFile.SetResRef(CResRef(sCreature), TRUE, TRUE);

    CGameSprite* pSprite = new CGameSprite(creatureFile.GetData(),
        creatureFile.GetDataSize(),
        0,
        0,
        -1,
        0,
        0,
        0x7FFFFFFF,
        CPoint(-1, -1),
        0);

    // NOTE: The original `SetResRef` (0x42FDC0) takes a `CString` by value,
    // here it's converted through `CResRef`.
    pSprite->SetResRef(CResRef(creatureFile.GetResRef().GetResRefStr()));
    creatureFile.ReleaseData();

    if (pSprite != NULL) {
        pSprite->SetResRef(CResRef(sCreature));

        LONG nSpriteId = pSprite->GetId();

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nSpriteId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            BYTE listType = pSprite->m_animation.GetListType();
            pSprite->AddToArea(pArea,
                CPoint(pt.x * CPathSearch::GRID_SQUARE_SIZEX, pt.y * CPathSearch::GRID_SQUARE_SIZEY),
                0,
                listType);
            pSprite->SetFacing(nFacing);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nSpriteId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x530790
CProjectile84D8B8::CProjectile84D8B8()
{
}

// 0x529950
void CProjectile84D8B8::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    m_targetId = target;
    m_sourceId = source;
    field_EA = pArea;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;

    OnArrival();

    delete this;
}

// 0x5348C0
CProjectile84E8C0::CProjectile84E8C0(const CString& sVidCell, const CString& sSound, BYTE nColor, LONG a4, LONG a5, SHORT a6)
    : m_palette(CVidPalette::TYPE_RANGE)
{
    // NOTE: Uninline.
    m_vidCell.SetResRef(CResRef(sVidCell), FALSE, TRUE, TRUE);

    if (nColor != 0xFF) {
        m_palette.SetRange(0, nColor, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_vidCell.SetPalette(m_palette);
    }

    m_vidCell.FrameSet(0);
    m_fireSoundRef = sSound;
    field_294 = a5;
    field_298 = a6;
    field_290 = a4;
}

// 0x534C10
void CProjectile84E8C0::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        field_298--;

        if (field_298 == 0) {
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

        if (field_294 != 0) {
            BYTE nLength = m_vidCell.GetSequenceLength(0, FALSE);
            if (nLength != 0) {
                m_vidCell.FrameSet(rand() % nLength);
            } else {
                m_vidCell.FrameSet(0);
            }
        } else {
            if (m_vidCell.IsEndOfSequence(FALSE)) {
                m_vidCell.FrameSet(0);
            } else {
                m_vidCell.FrameAdvance();
            }
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

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    }
}

// 0x534DD0
void CProjectile84E8C0::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
    // __LINE__: 9080
    UTIL_ASSERT(pVidMode != NULL);

    CInfinity* pInfinity = &field_EA->m_cInfinity;

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    DWORD dwFlags = 0;

    CPoint pos;
    pos.x = m_pos.x;
    pos.y = field_EA->GetHeightOffset(m_pos, m_listType) + m_pos.y;
    if (pos.y >= pInfinity->nAreaY) {
        pos.y = pInfinity->nAreaY - 1;
    }

    CPoint ptStart(pos.x, pos.y);

    BYTE nSequence = 0;

    // NOTE: Only the low byte of the result is tested.
    if (static_cast<BYTE>(m_vidCell.GetNumberSequences(FALSE)) > 0) {
        do {
            if (pos.y < 0) {
                return;
            }

            // NOTE: Uninline.
            if (!field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(pos))) {
                return;
            }

            m_vidCell.SequenceSet(nSequence);

            CPoint ptRef;
            m_vidCell.GetCurrentCenterPoint(ptRef, FALSE);

            CSize size;
            m_vidCell.GetCurrentFrameSize(size, FALSE);

            CRect rFx;
            rFx.SetRect(0, 0, size.cx, size.cy);

            CRect rGCBounds;
            rGCBounds.left = pos.x - ptRef.x;
            rGCBounds.top = m_posZ - ptRef.y + pos.y;
            rGCBounds.right = rFx.right - rFx.left + rGCBounds.left;
            rGCBounds.bottom = rFx.bottom - rFx.top + rGCBounds.top;

            CRect rIntersect;
            IntersectRect(&rIntersect, &rGCBounds, &rScreen);

            CRect rClip(rIntersect.left, rIntersect.top, rIntersect.right, rIntersect.bottom);
            if (IsRectEmpty(&rClip)) {
                return;
            }

            if (field_290 != 0) {
                dwFlags |= CInfinity::FXPREP_COPYFROMBACK | 0x20200;
            } else {
                dwFlags |= CInfinity::FXPREP_CLEARFILL | 0x20001;
            }

            pInfinity->FXPrep(rFx, dwFlags, a3, pos, ptRef);

            if (pInfinity->FXLock(rFx, dwFlags)) {
                pInfinity->FXRender(&m_vidCell, ptRef.x, ptRef.y, dwFlags, 0);

                CRect rClipPolys(rGCBounds.left,
                    m_posZ + rGCBounds.top,
                    rGCBounds.right,
                    m_posZ + rGCBounds.bottom);

                pInfinity->FXRenderClippingPolys(ptStart.x,
                    m_posZ + ptStart.y,
                    -m_posZ,
                    ptRef,
                    rClipPolys,
                    FALSE,
                    dwFlags);

                pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
                pInfinity->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, dwFlags);
            }

            pos.y -= ptRef.y;
            nSequence++;
        } while (nSequence < static_cast<BYTE>(m_vidCell.GetNumberSequences(FALSE)));
    }
}

// 0x535100
void CProjectile84E8C0::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CPoint pt;
    CGameObject* pTarget;
    BYTE rc;

    m_sourceId = source;
    m_targetId = target;
    field_EA = pArea;

    DeliverEffects();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    if (m_targetId == CGameObjectArray::INVALID_INDEX) {
        pt.x = targetPos.x;
        pt.y = targetPos.y + 13;
    } else {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pTarget,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        const CPoint& ptTargetPos = pTarget->GetPos();
        pt.x = ptTargetPos.x;
        pt.y = ptTargetPos.y + 13;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    CGameObject::AddToArea(pArea, pt, 0, CGameObject::LIST_FRONT);

    // NOTE: Uninline.
    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        CProjectile* pProjectile;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_callBackProjectile,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pProjectile->CallBack();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_callBackProjectile,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x535320
CProjectile84E940::CProjectile84E940(const CString& sVidCell, const CString& sSound, const CString& sBitmap, LONG a4, BYTE a5)
{
    field_32C = sVidCell;

    // NOTE: Uninline.
    m_vidCell.SetResRef(CResRef(sVidCell + "2"), FALSE, TRUE, TRUE);

    if (!sBitmap.IsEmpty()) {
        // NOTE: Uninline.
        m_bitmap.SetResRef(CResRef(sBitmap), TRUE, TRUE);
        m_bitmap.m_bDoubleSize = FALSE;
        field_32A = TRUE;
    } else {
        field_32A = FALSE;
    }

    m_vidCell.SequenceSet(0);
    m_vidCell.FrameSet(0);
    m_fireSoundRef = sSound;
    field_32B = a5;
    field_330 = FALSE;
    field_326 = a4;
}

// 0x5357B0
void CProjectile84E940::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        switch (field_330) {
        case 0:
            if (m_vidCell.IsEndOfSequence(FALSE)) {
                // NOTE: Uninline.
                m_vidCell.SetResRef(CResRef(field_32C + "1"), FALSE, TRUE, TRUE);

                m_vidCell.SequenceSet(0);
                m_vidCell.FrameSet(0);
                field_330++;
            } else {
                m_vidCell.FrameAdvance();
            }
            break;
        case 1:
        case 2:
        case 3:
            if (m_vidCell.IsEndOfSequence(FALSE)) {
                m_vidCell.FrameSet(0);
                field_330++;
            }

            m_vidCell.FrameAdvance();
            break;
        case 4:
            if (m_vidCell.IsEndOfSequence(FALSE)) {
                // NOTE: Uninline.
                m_vidCell.SetResRef(CResRef(field_32C + "2"), FALSE, TRUE, TRUE);

                m_vidCell.SequenceSet(0);
                m_vidCell.FrameSet(m_vidCell.GetSequenceLength(m_vidCell.m_nCurrentSequence, FALSE) - 1);
                field_330++;
            } else {
                m_vidCell.FrameAdvance();
            }
            break;
        case 5:
            if (m_vidCell.m_nCurrentFrame == 0) {
                OnArrival();
                return;
            }

            m_vidCell.m_nCurrentFrame--;
            break;
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    }
}

// 0x535A40
void CProjectile84E940::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
    // __LINE__: 9365
    UTIL_ASSERT(pVidMode != NULL);

    DWORD dwFlags = field_326;
    CInfinity* pInfinity = &field_EA->m_cInfinity;

    // NOTE: Uninline.
    if (!field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(m_pos))) {
        return;
    }

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    CPoint pos;
    pos.x = m_pos.x;
    pos.y = field_EA->GetHeightOffset(m_pos, m_listType) + (m_pos.y - m_posZ);

    CPoint ptRef;
    m_vidCell.GetCurrentCenterPoint(ptRef, FALSE);

    CSize size;
    m_vidCell.GetCurrentFrameSize(size, FALSE);

    CRect rFx;
    rFx.SetRect(0, 0, size.cx, size.cy);

    CRect rGCBounds;
    rGCBounds.left = pos.x - ptRef.x;
    rGCBounds.top = pos.y - ptRef.y;
    rGCBounds.right = rFx.right - rFx.left + rGCBounds.left;
    rGCBounds.bottom = rFx.bottom - rFx.top + rGCBounds.top;

    CRect rIntersect;
    IntersectRect(&rIntersect, &rGCBounds, &rScreen);

    CRect rClip(rIntersect.left, rIntersect.top, rIntersect.right, rIntersect.bottom);
    if (IsRectEmpty(&rClip)) {
        return;
    }

    if (dwFlags != 0) {
        dwFlags |= CInfinity::FXPREP_COPYFROMBACK;
    } else {
        dwFlags = CInfinity::FXPREP_CLEARFILL;
        dwFlags |= 0x1;
    }

    pInfinity->FXPrep(rFx, dwFlags, a3, pos, ptRef);

    if (pInfinity->FXLock(rFx, dwFlags)) {
        if (field_32A) {
            m_bitmap.GetRes()->Demand();
            m_vidCell.SetPalette(m_bitmap.GetRes()->GetColorTable(),
                m_bitmap.GetRes()->GetColorCount(),
                CVidPalette::TYPE_RESOURCE);
            pInfinity->FXRender(&m_vidCell, ptRef.x, ptRef.y, dwFlags, field_32B);
            m_bitmap.GetRes()->Release();
        } else {
            pInfinity->FXRender(&m_vidCell, ptRef.x, ptRef.y, dwFlags, field_32B);
        }

        pInfinity->FXRenderClippingPolys(pos.x, pos.y, 0, ptRef, rGCBounds, FALSE, dwFlags);
        pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
        pInfinity->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, dwFlags);
    }
}

// 0x535D20
void CProjectile84E940::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CPoint pt;
    BYTE rc;

    m_sourceId = source;
    m_targetId = target;
    field_EA = pArea;

    DeliverEffects();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    sub_52A640(m_targetId, pt, FALSE);
    pt.y--;

    CGameObject::AddToArea(pArea, pt, 0, CGameObject::LIST_FRONT);

    // NOTE: Uninline.
    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);
}

// 0x535F20
CProjectile84E9C0::CProjectile84E9C0(const CString& sVidCell, const CString& sSound, const CString& sBitmap, LONG a4, BYTE a5)
{
    // NOTE: Uninline.
    m_vidCell.SetResRef(CResRef(sVidCell), FALSE, TRUE, TRUE);

    if (!sBitmap.IsEmpty()) {
        m_bitmap.SetResRef(CResRef(sBitmap), TRUE, TRUE);
        m_bitmap.m_bDoubleSize = FALSE;
        field_32A = TRUE;
    } else {
        field_32A = FALSE;
    }

    m_vidCell.SequenceSet(0);
    m_vidCell.FrameSet(0);

    m_sound.Stop();

    // NOTE: Uninline.
    m_sound.SetResRef(CResRef(sSound), TRUE, TRUE);

    if (!m_sound.GetLooping()) {
        m_sound.SetFireForget(TRUE);
    }

    m_sound.SetChannel(14, reinterpret_cast<DWORD>(field_EA));
    m_fireSoundRef = sSound;
    field_32B = a5;
    field_326 = a4;
    m_bHasHeight = TRUE;
}

// 0x536380
void CProjectile84E9C0::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (m_vidCell.IsEndOfSequence(FALSE)) {
            OnArrival();
            return;
        }

        m_vidCell.FrameAdvance();

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    }
}

// 0x5363F0
void CProjectile84E9C0::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
    // __LINE__: 9833
    UTIL_ASSERT(pVidMode != NULL);

    // NOTE: Uninline.
    if (!field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(m_pos))) {
        return;
    }

    CInfinity* pInfinity = &field_EA->m_cInfinity;

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    CPoint pos;
    pos.x = m_pos.x;
    pos.y = field_EA->GetHeightOffset(m_pos, m_listType) + (m_pos.y - m_posZ);

    CPoint ptRef;
    m_vidCell.GetCurrentCenterPoint(ptRef, FALSE);

    CSize size;
    m_vidCell.GetCurrentFrameSize(size, FALSE);

    CRect rFx;
    rFx.SetRect(0, 0, size.cx, size.cy);

    CRect rGCBounds;
    rGCBounds.left = pos.x - ptRef.x;
    rGCBounds.top = pos.y - ptRef.y;
    rGCBounds.right = rFx.right - rFx.left + rGCBounds.left;
    rGCBounds.bottom = rFx.bottom - rFx.top + rGCBounds.top;

    CRect rIntersect;
    IntersectRect(&rIntersect, &rGCBounds, &rScreen);

    CRect rClip(rIntersect.left, rIntersect.top, rIntersect.right, rIntersect.bottom);
    if (IsRectEmpty(&rClip)) {
        return;
    }

    if (field_326 != 0) {
        field_326 |= CInfinity::FXPREP_COPYFROMBACK;
    }

    pInfinity->FXPrep(rFx, field_326, a3, pos, ptRef);

    if (pInfinity->FXLock(rFx, field_326)) {
        if (field_32A) {
            m_bitmap.GetRes()->Demand();
            m_vidCell.SetPalette(m_bitmap.GetRes()->GetColorTable(),
                m_bitmap.GetRes()->GetColorCount(),
                CVidPalette::TYPE_RESOURCE);
            pInfinity->FXRender(&m_vidCell, ptRef.x, ptRef.y, field_326, field_32B);
            m_bitmap.GetRes()->Release();
        } else {
            pInfinity->FXRender(&m_vidCell, ptRef.x, ptRef.y, field_326, field_32B);
        }

        pInfinity->FXRenderClippingPolys(pos.x, pos.y, 0, ptRef, rGCBounds, FALSE, field_326);
        pInfinity->FXUnlock(field_326, NULL, CPoint(0, 0));
        pInfinity->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, field_326);
    }
}

// 0x5366F0
void CProjectile84E9C0::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CPoint pt;
    BYTE rc;

    m_sourceId = source;
    m_targetId = target;
    field_EA = pArea;

    DeliverEffects();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    if (m_targetId != CGameObjectArray::INVALID_INDEX) {
        sub_52A640(m_targetId, pt, FALSE);
    } else {
        pt = targetPos;
    }

    m_sound.Play(pt.x, pt.y, 0, FALSE);

    pt.y++;

    CGameObject::AddToArea(pArea, pt, nHeight, CGameObject::LIST_FRONT);
}

// 0x57CAC0
CProjectile850F9C::CProjectile850F9C(const CString& sVidCell, const CString& sSound, BYTE a3, BYTE a4, const IcewindCVisualEffect& vfx)
{
    field_326 = 0;
    field_32A = 0;
    field_32B = 0;

    // NOTE: Uninline.
    m_vidCell.SetResRef(CResRef(sVidCell), FALSE, TRUE, TRUE);

    m_vidCell.FrameSet(0);

    m_sound.Stop();

    // NOTE: Uninline.
    m_sound.SetResRef(CResRef(sSound), TRUE, TRUE);

    if (!m_sound.GetLooping()) {
        m_sound.SetFireForget(TRUE);
    }

    m_sound.SetChannel(4, reinterpret_cast<DWORD>(field_EA));
    m_fireSoundRef = sSound;

    field_32C = a3;
    if (field_32C > m_vidCell.GetSequenceLength(2, FALSE)) {
        m_vidCell.SequenceSet(0);
        field_32C -= m_vidCell.GetSequenceLength(2, FALSE);
    } else {
        m_vidCell.SequenceSet(2);
        field_32C = 0;
    }

    field_32D = a4;
    m_bHasHeight = TRUE;
    m_vfx = vfx;
}

// 0x57CEE0
void CProjectile850F9C::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (field_32D > 0) {
            field_32D--;
            return;
        }

        if (field_32C > 0) {
            field_32C--;

            if (m_vidCell.IsEndOfSequence(FALSE)) {
                m_vidCell.SequenceSet(1);
                m_vidCell.FrameSet(0);
            } else {
                m_vidCell.FrameAdvance();
            }
        } else {
            if (m_vidCell.m_nCurrentSequence != 2) {
                m_vidCell.SequenceSet(2);
                m_vidCell.FrameSet(0);
            } else {
                if (m_vidCell.IsEndOfSequence(FALSE)) {
                    OnArrival();
                    return;
                }

                m_vidCell.FrameAdvance();
            }
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    }
}

// 0x57CFB0
void CProjectile850F9C::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
    // __LINE__: 821
    UTIL_ASSERT(pVidMode != NULL);

    CInfinity* pInfinity = &field_EA->m_cInfinity;

    if (field_32D > 0) {
        return;
    }

    // NOTE: Uninline.
    if (!field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(m_pos))) {
        return;
    }

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    CPoint pos;
    pos.x = m_pos.x;
    pos.y = field_EA->GetHeightOffset(m_pos, m_listType) + (m_pos.y - m_posZ);

    CPoint ptRef;
    m_vidCell.GetCurrentCenterPoint(ptRef, FALSE);

    CSize size;
    m_vidCell.GetCurrentFrameSize(size, FALSE);

    CRect rFx;
    rFx.SetRect(0, 0, size.cx, size.cy);

    CRect rGCBounds;
    rGCBounds.left = pos.x - ptRef.x;
    rGCBounds.top = pos.y - ptRef.y;
    rGCBounds.right = rFx.right - rFx.left + rGCBounds.left;
    rGCBounds.bottom = rFx.bottom - rFx.top + rGCBounds.top;

    CRect rIntersect;
    IntersectRect(&rIntersect, &rGCBounds, &rScreen);

    CRect rClip(rIntersect.left, rIntersect.top, rIntersect.right, rIntersect.bottom);
    if (IsRectEmpty(&rClip)) {
        return;
    }

    DWORD dwFlags = CInfinity::FXPREP_COPYFROMBACK;
    if (m_vfx.m_dwFlags == 0) {
        dwFlags = CInfinity::FXPREP_CLEARFILL;
    }

    pInfinity->FXPrep(rFx, dwFlags, a3, pos, ptRef);

    if (pInfinity->FXLock(rFx, dwFlags)) {
        pInfinity->FXRender(&m_vidCell, ptRef.x, ptRef.y, m_vfx.m_dwFlags, m_vfx.m_nTransValue);
        pInfinity->FXRenderClippingPolys(pos.x, pos.y, 0, ptRef, rGCBounds, FALSE, dwFlags);
        pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
        pInfinity->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, m_vfx.m_dwFlags | 0x1);
    }
}

// 0x57D230
void CProjectile850F9C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pTarget;
    CPoint ptTarget;
    CPoint pt;
    BYTE rc;

    m_sourceId = source;
    m_targetId = target;
    field_EA = pArea;

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(target,
        CGameObjectArray::THREAD_ASYNCH,
        &pTarget,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    ptTarget = pTarget->GetPos();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    sub_52A640(m_sourceId, pt, TRUE);

    m_sound.Play(pt.x, pt.y, 0, FALSE);

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    CGameObject::AddToArea(pArea, pt, nHeight, CGameObject::LIST_FRONT);

    m_posDest.x = ptTarget.x;
    m_posExact.x = pt.x << CParticle::RESOLUTION_INC;
    m_posExact.y = 4 * (pt.y << CParticle::RESOLUTION_INC) / 3;
    m_posDest.y = ptTarget.y;

    DeliverEffects();
}

// 0x57E860
CProjectile8512BC::CProjectile8512BC(const CString& sVidCell, const CString& sSound, BYTE nColor, LONG a4, LONG a5, SHORT a6)
    : m_palette(CVidPalette::TYPE_RANGE)
{
    field_29A = 0;

    // NOTE: Uninline.
    m_vidCell.SetResRef(CResRef(sVidCell), FALSE, TRUE, TRUE);

    if (nColor != 0xFF) {
        m_palette.SetRange(0, nColor, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
        m_vidCell.SetPalette(m_palette);
    }

    m_vidCell.FrameSet(0);
    m_fireSoundRef = sSound;
    field_294 = a5;
    field_298 = a6;
    field_290 = a4;
}

// 0x57EB30
void CProjectile8512BC::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        field_298--;

        if (field_298 == 0) {
            field_29A = 2;
            m_vidCell.FrameSet(0);
        }

        if (field_294 != 0) {
            BYTE nLength = m_vidCell.GetSequenceLength(0, FALSE);
            if (nLength != 0) {
                m_vidCell.FrameSet(rand() % nLength);
            } else {
                m_vidCell.FrameSet(0);
            }
        } else {
            if (m_vidCell.IsEndOfSequence(FALSE)) {
                switch (field_29A) {
                case 0:
                    field_29A = 1;
                    // FALLTHROUGH
                case 1:
                    m_vidCell.FrameSet(0);
                    break;
                case 2:
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
            } else {
                m_vidCell.FrameAdvance();
            }
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

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    }
}

// 0x57ED30
void CProjectile8512BC::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectiles.cpp
    // __LINE__: 1867
    UTIL_ASSERT(pVidMode != NULL);

    CInfinity* pInfinity = &field_EA->m_cInfinity;

    CRect rScreen;
    rScreen.left = pInfinity->nCurrentX;
    rScreen.top = pInfinity->nCurrentY;
    rScreen.right = pInfinity->rViewPort.right - pInfinity->rViewPort.left + pInfinity->nCurrentX;
    rScreen.bottom = pInfinity->rViewPort.bottom - pInfinity->rViewPort.top + pInfinity->nCurrentY;

    DWORD dwFlags = 0;

    CPoint pos;
    pos.x = m_pos.x;
    pos.y = field_EA->GetHeightOffset(m_pos, m_listType) + m_pos.y;
    if (pos.y >= pInfinity->nAreaY) {
        pos.y = pInfinity->nAreaY - 1;
    }

    CPoint ptStart(pos.x, pos.y);

    BYTE nSequence = 0;

    do {
        if (pos.y < 0) {
            return;
        }

        // NOTE: Uninline.
        if (!field_EA->m_visibility.IsTileVisible(field_EA->m_visibility.PointToTile(pos))) {
            return;
        }

        m_vidCell.SequenceSet(static_cast<WORD>(field_29A << 2) + nSequence);

        CPoint ptRef;
        m_vidCell.GetCurrentCenterPoint(ptRef, FALSE);

        CSize size;
        m_vidCell.GetCurrentFrameSize(size, FALSE);

        CRect rFx;
        rFx.SetRect(0, 0, size.cx, size.cy);

        CRect rGCBounds;
        rGCBounds.left = pos.x - ptRef.x;
        rGCBounds.top = pos.y - ptRef.y + m_posZ;
        rGCBounds.right = rFx.right - rFx.left + rGCBounds.left;
        rGCBounds.bottom = rFx.bottom - rFx.top + rGCBounds.top;

        CRect rIntersect;
        IntersectRect(&rIntersect, &rGCBounds, &rScreen);

        CRect rClip(rIntersect.left, rIntersect.top, rIntersect.right, rIntersect.bottom);
        if (IsRectEmpty(&rClip)) {
            return;
        }

        if (field_290 != 0) {
            dwFlags |= CInfinity::FXPREP_COPYFROMBACK | 0x20200;
        } else {
            dwFlags |= CInfinity::FXPREP_CLEARFILL | 0x20000;
        }

        pInfinity->FXPrep(rFx, dwFlags, a3, pos, ptRef);

        if (pInfinity->FXLock(rFx, dwFlags)) {
            pInfinity->FXRender(&m_vidCell, ptRef.x, ptRef.y, dwFlags, 0);

            CRect rClipPolys(rGCBounds.left,
                m_posZ + rGCBounds.top,
                rGCBounds.right,
                m_posZ + rGCBounds.bottom);

            pInfinity->FXRenderClippingPolys(ptStart.x,
                m_posZ + ptStart.y,
                -m_posZ,
                ptRef,
                rClipPolys,
                FALSE,
                dwFlags);

            pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
            pInfinity->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, dwFlags | 0x1);
        }

        pos.y -= ptRef.y;
        nSequence++;
    } while (nSequence < 4);
}

// 0x57F050
void CProjectile8512BC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CPoint pt;
    CGameObject* pTarget;
    BYTE rc;

    m_sourceId = source;
    m_targetId = target;
    field_EA = pArea;

    DeliverEffects();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    if (m_targetId == CGameObjectArray::INVALID_INDEX) {
        pt.x = targetPos.x;
        pt.y = targetPos.y + 13;
    } else {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pTarget,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        const CPoint& ptTargetPos = pTarget->GetPos();
        pt.x = ptTargetPos.x;
        pt.y = ptTargetPos.y + 13;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    CGameObject::AddToArea(pArea, pt, 0, CGameObject::LIST_FRONT);

    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        CProjectile* pProjectile;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_callBackProjectile,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pProjectile->CallBack();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_callBackProjectile,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x57F210
CProjectile85133C::CProjectile85133C()
{
}

// 0x57F390
void CProjectile85133C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pSource;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(source,
        CGameObjectArray::THREAD_ASYNCH,
        &pSource,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    CPoint ptSource = pSource->GetPos();

    LONG dy = targetPos.y - ptSource.y;
    LONG dx = targetPos.x - ptSource.x;
    LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dy * dy + dx * dx)));

    CPoint pt[4];
    pt[0] = targetPos;
    pt[1] = IcewindMisc::sub_5847B0(ptSource, -dy, dx, nDistance);
    pt[2] = IcewindMisc::sub_5847B0(ptSource, dy, -dx, nDistance);
    pt[3] = IcewindMisc::sub_5847B0(ptSource, -dx, -dy, nDistance);

    for (int index = 0; index < 4; index++) {
        CProjectileSingleTarget8513BC* pSingleTarget = new CProjectileSingleTarget8513BC("LightnT");
        pSingleTarget->field_2AE = 300;
        pSingleTarget->field_2B2.field_32 = 50;
        pSingleTarget->m_speed = 20;
        pSingleTarget->field_1D8 = 16;
        pSingleTarget->m_vfx.sub_586A90(TRUE);
        pSingleTarget->m_bHasHeight = TRUE;
        pSingleTarget->field_1C2 = 1;
        pSingleTarget->m_fireSoundRef = CResRef("TRA_09");

        POSITION pos = m_effectList.GetHeadPosition();
        while (pos != NULL) {
            CGameEffect* pEffect = m_effectList.GetNext(pos);
            pSingleTarget->AddEffect(pEffect->Copy());
        }

        pSingleTarget->Fire(pSource->m_pArea,
            pSource->m_id,
            CGameObjectArray::INVALID_INDEX,
            pt[index],
            0,
            0);
    }

    // NOTE: `this` is not deleted.
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(source,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x57E1B0
CProjectile85112C::CProjectile85112C()
{
}

// 0x57E1D0
void CProjectile85112C::DeliverEffects()
{
    CGameObject* pSource;
    CGameObject* pTarget;
    BYTE rc;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pSource,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pTarget,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    if (pSource->GetObjectType() != CGameObject::TYPE_SPRITE) {
        CProjectile::DeliverEffects();

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    LONG nSaveMod = 0;

    // NOTE: The result is compared with `TRUE` and nothing is done with it.
    IcewindMisc::sub_585230(static_cast<CGameSprite*>(pSource), static_cast<CGameSprite*>(pTarget));

    if (IcewindMisc::sub_5852A0(static_cast<CGameSprite*>(pSource), static_cast<CGameSprite*>(pTarget)) == TRUE) {
        nSaveMod = 8;
    }

    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* pEffect = m_effectList.GetNext(pos);
        pEffect->m_saveMod = nSaveMod;
    }

    // NOTE: The shares of `pSource` and `pTarget` are not released.
    CProjectile::DeliverEffects();
}

// 0x5367C0
void CProjectile84D838::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    BYTE rc;

    m_sourceId = source;
    m_targetId = target;
    field_EA = pArea;

    DeliverEffects();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    CGameObject::AddToArea(pArea, targetPos, nHeight, CGameObject::LIST_FRONT);

    // NOTE: Uninline.
    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);
}
