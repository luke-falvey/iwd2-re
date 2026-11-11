#include "CProjectilePart3.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CUtil.h"

// 0x529CE0
CProjectile84DAD4::CProjectile84DAD4()
{
}

// 0x5818F0
void CProjectile84DAD4::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
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

            // NOTE: Inlined `CVisibilityMap::PointToTile`, with a modified
            // position.
            if (!field_EA->m_visibility.IsTileVisible(static_cast<SHORT>(static_cast<SHORT>(pos.y / CVisibilityMap::SQUARE_SIZEY) * field_EA->m_visibility.m_nWidth + pos.x / CVisibilityMap::SQUARE_SIZEX))) {
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

        pt.x = pTarget->GetPos().x;
        pt.y = pTarget->GetPos().y + 13;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    AddToArea(pArea, pt, 0, CGameObject::LIST_FRONT);

    // NOTE: Inlined `CProjectile::PlaySound`.
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
    // TODO: Incomplete.
}

// 0x535A40
void CProjectile84E940::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x535D20
void CProjectile84E940::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x5363F0
void CProjectile84E9C0::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x5366F0
void CProjectile84E9C0::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x57CFB0
void CProjectile850F9C::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x57D230
void CProjectile850F9C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x57ED30
void CProjectile8512BC::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x57F050
void CProjectile8512BC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x57F210
CProjectile85133C::CProjectile85133C()
{
}

// 0x57F390
void CProjectile85133C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x57E1B0
CProjectile85112C::CProjectile85112C()
{
}

// 0x57E1D0
void CProjectile85112C::DeliverEffects()
{
    // TODO: Incomplete.
}

// 0x5367C0
void CProjectile84D838::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}
