#include "CProjectileArea.h"

#include <math.h>
#include <string.h>

#include "CBaldurChitin.h"
#include "CGameAIBase.h"
#include "CGameArea.h"
#include "CGameSparkle.h"
#include "CGameSprite.h"
#include "CGameEffect.h"
#include "CInfGame.h"
#include "CParticle.h"
#include "CPathSearch.h"
#include "CUtil.h"
#include "IcewindMisc.h"

// 0x5868E0
CProjectileArea5868E0::CProjectileArea5868E0()
{
    field_10 = 0;
    field_0.reserve(512);
}

// 0x56FDC0
CProjectileAreaVisual2::CProjectileAreaVisual2()
{
    field_2C = FALSE;
    field_2E = 0x7FFFFFFF;
}

// 0x56FE30
CProjectileAreaVisual3::CProjectileAreaVisual3()
{
    field_2C = NULL;
    field_30 = 0;
    field_34 = FALSE;
    field_35 = FALSE;
    field_36 = FALSE;
    field_44 = FALSE;
    field_38 = 250;
    field_3C = 6;
    field_40 = 30;
}

// 0x56EDD0
CProjectileArea::CProjectileArea(SHORT a1)
    : CProjectileSingleTarget(CResRef("SPMAGMIS"))
{
    field_50E.field_2C = FALSE;
    field_50E.field_2E = 0x7FFFFFFF;
    field_540.field_2C = NULL;
    field_540.field_30 = 0;
    field_540.field_34 = FALSE;
    field_540.field_35 = FALSE;
    field_540.field_36 = FALSE;
    field_540.field_38 = 250;
    field_540.field_3C = 6;
    field_540.field_40 = 30;
    field_540.field_44 = FALSE;
    field_2B4 = 0;
    field_2BA = 0;
    field_2BC = 0;
    field_2FA = 0;
    field_2FE = FALSE;
    field_2FF = FALSE;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
    field_2B2 = 78;
    field_2B6 = 0;
    field_2BE.Set(CAIObjectType::ANYONE);
    field_2AE = a1;
    field_304 = 0;
    field_3E2 = 0;
    field_300 = 0;
    field_4D8 = 0;
    field_4E0 = FALSE;
    field_1DE = 0;
    field_4C0 = 45;
    field_4D4 = 10000;
    field_4DC = 10;
}

// 0x56BF30
CProjectileArea84F0DC::CProjectileArea84F0DC(const CProjectileAreaVisual* a1, const CProjectileAreaVisual2* a2, const CProjectileAreaVisual3* a3, CGameArea* pArea, const CPoint* pos, SHORT a6, BYTE a7, BYTE a8, WORD a9)
    : field_164(CVidPalette::TYPE_RANGE)
{
    field_29C = 0;
    field_2A0 = FALSE;
    field_28C = 0;
    field_188 = a9;
    field_288 = 0;
    field_18A = (a6 - 1) / a7 + 1;
    field_18B = a8;

    if (a1->field_0.length() != 0) {
        CString sName(a1->field_0.c_str());
        CResRef resRef(sName);

        // NOTE: Uninline.
        field_8A.SetResRef(resRef, FALSE, TRUE, TRUE);

        field_2A0 = TRUE;
    } else {
        field_2A0 = FALSE;
    }

    field_290 = a1->field_20;
    field_210 = *a2;
    field_242 = *a3;

    field_8A.SequenceSet(0);
    field_8A.FrameSet(0);

    field_7E = 0;
    field_80 = 0;
    m_pCells = NULL;

    LONG nRadius = a6 - 1;
    LONG nCells = nRadius / CPathSearch::GRID_SQUARE_SIZEX + 1;
    field_18C = nCells;
    field_190 = nCells;

    m_pGrid = new BYTE[(nCells * 2 + 1) * (nCells * 2 + 1)];
    if (m_pGrid == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    nCells = nRadius / (CPathSearch::GRID_SQUARE_SIZEX * 2) + 1;
    field_18C = nCells;
    field_190 = nCells;

    LONG nIndex = g_pBaldurChitin->GetCurrentVideoMode()->GetEllipseArcPixelList(field_18C, field_18C, m_pGrid);
    field_198 = nIndex + 1;
    m_pGrid[field_198 - 1] = 1;

    nIndex = g_pBaldurChitin->GetCurrentVideoMode()->GetEllipseArcPixelList(field_190, field_18C, m_pGrid + field_198);
    field_19C = nIndex + 1;
    m_pGrid[field_198 + field_19C - 1] = 1;

    nCells = nRadius / CPathSearch::GRID_SQUARE_SIZEX + 1;
    field_18C = nCells;
    field_190 = nCells;

    m_pCells = new CELL[(field_19C + field_198) * 4];
    if (m_pCells == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    m_pCellStates = new BYTE[(field_19C + field_198) * 4];
    if (m_pCellStates == NULL) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    memset(m_pCellStates, 0, (field_198 + field_19C) * 4);

    LONG nOriginX = pos->x;
    LONG nOriginY = pos->y * 4 / 3;
    LONG nCurX = nOriginX;
    LONG nCurY = a6 + nOriginY;

    for (SHORT i = 0; i < field_198; i++) {
        LONG dx = nCurX - nOriginX;
        LONG dy = nCurY - nOriginY;
        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

        m_pCells[i].nOffsetX = 0;
        m_pCells[field_198 + field_19C + i].nOffsetX = 0;
        m_pCells[i + (field_198 + field_19C) * 2].nOffsetX = 0;
        m_pCells[i + (field_198 + field_19C) * 3].nOffsetX = 0;

        m_pCells[i].nOffsetY = 0;
        m_pCells[field_198 + field_19C + i].nOffsetY = 0;
        m_pCells[i + (field_198 + field_19C) * 2].nOffsetY = 0;
        m_pCells[i + (field_198 + field_19C) * 3].nOffsetY = 0;

        m_pCells[i].nVelocityX = ((dx << CGameSprite::EXACT_SCALE) * a7) / nDistance;
        m_pCells[field_198 + field_19C + i].nVelocityX = m_pCells[i].nVelocityX;

        if (m_pCells[i].nVelocityX == 0) {
            m_pCellStates[i + (field_198 + field_19C) * 2] = 2;
            m_pCellStates[i + (field_198 + field_19C) * 3] = 2;
        }

        m_pCells[i + (field_198 + field_19C) * 2].nVelocityX = -m_pCells[i].nVelocityX;
        m_pCells[i + (field_198 + field_19C) * 3].nVelocityX = -m_pCells[i].nVelocityX;

        m_pCells[i].nVelocityY = ((dy << CGameSprite::EXACT_SCALE) * a7) / nDistance;
        m_pCells[field_198 + field_19C + i].nVelocityY = -m_pCells[i].nVelocityY;
        m_pCells[i + (field_198 + field_19C) * 2].nVelocityY = m_pCells[i].nVelocityY;
        m_pCells[i + (field_198 + field_19C) * 3].nVelocityY = -m_pCells[i].nVelocityY;

        nCurX += CPathSearch::GRID_SQUARE_SIZEX * 2;
        nCurY -= m_pGrid[i] * CPathSearch::GRID_SQUARE_SIZEY * 2;
    }

    nCurX = a6 + nOriginX;
    nCurY = nOriginY;

    for (SHORT i = 0; i < field_19C; i++) {
        LONG dx = nCurX - nOriginX;
        LONG dy = nCurY - nOriginY;
        LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);

        m_pCells[field_198 + i].nOffsetX = 0;
        m_pCells[field_198 * 2 + field_19C + i].nOffsetX = 0;
        m_pCells[i + (field_198 + field_19C) * 2 + field_198].nOffsetX = 0;
        m_pCells[i + (field_198 + field_19C) * 3 + field_198].nOffsetX = 0;

        m_pCells[field_198 + i].nOffsetY = 0;
        m_pCells[field_198 * 2 + field_19C + i].nOffsetY = 0;
        m_pCells[i + (field_198 + field_19C) * 2 + field_198].nOffsetY = 0;
        m_pCells[i + (field_198 + field_19C) * 3 + field_198].nOffsetY = 0;

        m_pCells[field_198 + i].nVelocityX = ((dx << CGameSprite::EXACT_SCALE) * a7) / nDistance;
        m_pCells[field_198 * 2 + field_19C + i].nVelocityX = m_pCells[field_198 + i].nVelocityX;
        m_pCells[i + (field_198 + field_19C) * 2 + field_198].nVelocityX = -m_pCells[field_198 + i].nVelocityX;
        m_pCells[i + (field_198 + field_19C) * 3 + field_198].nVelocityX = -m_pCells[field_198 + i].nVelocityX;

        m_pCells[field_198 + i].nVelocityY = ((dy << CGameSprite::EXACT_SCALE) * a7) / nDistance;
        m_pCells[field_198 * 2 + field_19C + i].nVelocityY = -m_pCells[field_198 + i].nVelocityY;
        m_pCells[i + (field_198 + field_19C) * 2 + field_198].nVelocityY = m_pCells[field_198 + i].nVelocityY;
        m_pCells[i + (field_198 + field_19C) * 3 + field_198].nVelocityY = -m_pCells[field_198 + i].nVelocityY;

        // NOTE: Tests the entry at `i`, not at `field_198 + i`.
        if (m_pCells[i].nVelocityY == 0) {
            m_pCellStates[field_198 * 2 + field_19C + i] = 2;
            m_pCellStates[i + (field_198 + field_19C) * 3 + field_198] = 2;
        }

        nCurX -= m_pGrid[field_198 + i] * CPathSearch::GRID_SQUARE_SIZEX * 2;
        nCurY += CPathSearch::GRID_SQUARE_SIZEY * 2;
    }

    // NOTE: Original copies the table from 0x8A8154.
    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, 16);

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) != CGameObjectArray::SUCCESS) {
        // NOTE: Deletes the object that is still under construction.
        delete this;
        return;
    }

    CGameObject::AddToArea(pArea, *pos, 0, CGameObject::LIST_FRONT);
}

// 0x56CF00
// NOTE: 0x56CEE0 is the scalar deleting destructor (compiler-generated).
CProjectileArea84F0DC::~CProjectileArea84F0DC()
{
    if (m_pGrid != NULL) {
        delete[] m_pGrid;
    }

    if (m_pCells != NULL) {
        delete[] m_pCells;
    }

    if (m_pCellStates != NULL) {
        delete[] m_pCellStates;
    }
}

// 0x56D0A0
void CProjectileArea84F0DC::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x56D9B0
void CProjectileArea84F0DC::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameFireball.cpp
        // __LINE__: 493
        UTIL_ASSERT(FALSE);
        return;
    }

    delete this;
}

// 0x56D730
void CProjectileArea84F0DC::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameFireball.cpp
    // __LINE__: 396
    UTIL_ASSERT(pVidMode != NULL);

    CInfinity* pInfinity = &m_pArea->m_cInfinity;

    CPoint ptTile;
    ptTile.x = m_pos.x < 0 ? 0 : m_pos.x;
    if (ptTile.x >= m_pArea->m_cInfinity.nAreaX - 1) {
        ptTile.x = m_pArea->m_cInfinity.nAreaX - 1;
    }
    ptTile.y = m_pos.y < 0 ? 0 : m_pos.y;
    if (ptTile.y >= m_pArea->m_cInfinity.nAreaY - 1) {
        ptTile.y = m_pArea->m_cInfinity.nAreaY - 1;
    }

    // NOTE: Uninline.
    if (!m_pArea->m_visibility.IsTileVisible(m_pArea->m_visibility.PointToTile(ptTile))) {
        return;
    }

    if (!field_2A0) {
        return;
    }

    if (field_8A.IsEndOfSequence(FALSE)) {
        return;
    }

    CPoint ptRef;
    field_8A.GetCurrentCenterPoint(ptRef, FALSE);

    CSize size;
    field_8A.GetCurrentFrameSize(size, FALSE);

    CRect rFx;
    rFx.SetRect(0, 0, size.cx, size.cy);

    CPoint pos;
    pos.x = m_pos.x;
    pos.y = m_pos.y + m_pArea->GetHeightOffset(m_pos, m_listType);

    LONG left = pos.x - ptRef.x;
    LONG top = pos.y + (m_posZ - ptRef.y);
    LONG right = rFx.right - rFx.left + left;
    LONG bottom = rFx.bottom - rFx.top + top;

    DWORD dwFlags = CInfinity::FXPREP_COPYFROMBACK | field_290.m_dwFlags;

    pInfinity->FXPrep(rFx, dwFlags, a3, pos, ptRef);

    if (!pInfinity->FXLock(rFx, dwFlags)) {
        return;
    }

    pInfinity->FXRender(&field_8A, ptRef.x, ptRef.y, field_290.m_dwFlags, 0);

    CRect rGCBounds(left, top - m_posZ, right, bottom - m_posZ);
    pInfinity->FXRenderClippingPolys(pos.x,
        pos.y - m_posZ,
        m_posZ,
        ptRef,
        rGCBounds,
        FALSE,
        field_290.m_dwFlags);

    pInfinity->FXUnlock(dwFlags, NULL, CPoint(0, 0));
    pInfinity->FXBltFrom(a3, rFx, pos.x, pos.y, ptRef.x, ptRef.y, field_290.m_dwFlags | 1);
}

// 0x5703F0
void CProjectileArea::sub_5703F0(CGameAIBase* pSource)
{
    if (pSource != NULL) {
        if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_ALLY;
            field_2BE.Set(type);
        } else {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_ENEMY;
            field_2BE.Set(type);
        }
    }
}

// 0x5704E0
void CProjectileArea::sub_5704E0(CGameAIBase* pSource)
{
    if (pSource != NULL) {
        if (pSource->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_ENEMY;
            field_2BE.Set(type);
        } else {
            CAIObjectType type;
            type.m_nEnemyAlly = CAIObjectType::EA_ALLY;
            field_2BE.Set(type);
        }
    }
}

// 0x56FAF0
void CProjectileArea::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (field_2B6 == 0) {
        if (!field_4E0) {
            m_pos.x = m_posDest.x;
            m_pos.y = m_posDest.y;
            OnArrival();
            return;
        }

        m_pVidCell->FrameAdvance();

        LONG dy = m_posDest.y - m_pos.y;
        LONG dx = m_posDest.x - m_pos.x;
        if (dy * dy * 16 / 9 + dx * dx > (m_speed + 1) * (m_speed + 1)) {
            Virtual84(m_posDest);

            if (field_E2 != 0) {
                CGameSparkle* pSparkle = new CGameSparkle(field_9A, 1 /* 0x84D600 */, 25 /* 0x84D5F0 */);
                pSparkle->AddToArea(field_EA, m_pos, m_posZ, CGameObject::LIST_FRONT);
            }

            m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        } else {
            OnArrival();
        }

        return;
    }

    if (field_304 != 0) {
        field_308.FrameAdvance();
    }

    if (field_3E2 != 0) {
        field_3E6.FrameAdvance();
    }

    field_4D8--;
    if (field_4D8 <= 0) {
        std::list<LONG> targets = Virtual90();
        Virtual94(targets);
        field_4D8 = field_4D4;
    }

    if (field_540.field_10.length() != 0) {
        if (!field_5EA.IsSoundPlaying()) {
            field_5EA.Play(m_pos.x, m_pos.y, 0, FALSE);
        }
    }

    field_4C0--;
    if (field_4C0 <= 0) {
        RemoveSelf();
    }
}

// 0x56F3F0
void CProjectileArea::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    CProjectileSingleTarget::Render(pArea, pVidMode, a3);
}

// 0x56F820
void CProjectileArea::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pSource;
    BYTE rc;

    if (pArea == NULL) {
        return;
    }

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

    if (m_effectList.GetHeadPosition() != NULL) {
        CGameEffect* pEffect = m_effectList.GetTail();
        if (pEffect != NULL) {
            // NOTE: Only the low byte of the field at 0xC4 is passed.
            field_4C0 = Virtual8C(static_cast<BYTE>(pEffect->m_firstCall));
        }
    }

    CPoint ptStart;
    sub_52A640(m_sourceId, ptStart, TRUE);

    CPoint pos = ptStart;
    if (!field_4E0) {
        pos = targetPos;
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

    // NOTE: Not a virtual call.
    CGameObject::AddToArea(pArea,
        pos,
        DetermineHeight(static_cast<CGameSprite*>(pSource)),
        CGameObject::LIST_FRONT);

    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    m_posDest.x = targetPos.x;
    m_pos.x = pos.x;
    m_posExact.x = pos.x << CParticle::RESOLUTION_INC;
    m_posExact.y = 4 * (pos.y << CParticle::RESOLUTION_INC) / 3;
    m_posDest.y = targetPos.y;
    m_pos.y = pos.y;

    LONG dy = 4 * m_posDest.y / 3 - 4 * m_pos.y / 3;
    LONG dx = m_posDest.x - m_pos.x;
    LONG nDistance = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);
    if (nDistance != 0) {
        m_posDelta.x = (dx << CParticle::RESOLUTION_INC) * m_speed / nDistance;
        m_posDelta.y = (dy << CParticle::RESOLUTION_INC) * m_speed / nDistance;
        m_nDirection = GetDirection(m_posDest);
    } else {
        OnArrival();
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x56F410
void CProjectileArea::OnArrival()
{
    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        CProjectile* pProjectile;
        BYTE rc;

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

    field_2B6 = 1;
    field_1DE = 0;

    PlaySound(m_arrivalSoundRef, m_loopArrivalSound, TRUE);

    if (field_540.field_0.length() != 0) {
        field_540.field_2C = new CProjectileArea5868E0();
    }

    if (field_50E.field_0.length() != 0
        || field_540.field_0.length() != 0
        || field_4E2.field_0.length() != 0) {
        // NOTE: The object is not kept.
        new CProjectileArea84F0DC(&field_4E2,
            &field_50E,
            &field_540,
            field_EA,
            &m_pos,
            field_2AE,
            static_cast<BYTE>(m_speed),
            1 /* 0x85BD6D */,
            static_cast<WORD>(field_4C0));
    }

    if (field_4E2.field_10.length() != 0) {
        // NOTE: Uninline.
        field_586.SetResRef(CResRef(CString(field_4E2.field_10.c_str())), TRUE, TRUE);

        field_586.SetChannel(14, reinterpret_cast<DWORD>(field_EA));
        field_586.Play(m_pos.x, m_pos.y, 0, FALSE);
    }

    if (field_540.field_10.length() != 0) {
        // NOTE: Uninline.
        field_5EA.SetResRef(CResRef(CString(field_540.field_10.c_str())), TRUE, TRUE);

        field_5EA.SetLoopingFlag(TRUE);
        field_5EA.SetChannel(14, reinterpret_cast<DWORD>(field_EA));
    }
}

// 0x78E730
void CProjectileArea::Virtual88()
{
}

// 0x5703E0
LONG CProjectileArea::Virtual8C(BYTE a1)
{
    return field_4C0;
}

// 0x56FED0
std::list<LONG> CProjectileArea::Virtual90()
{
    CTypedPtrList<CPtrList, LONG*> targets;

    field_EA->GetAllInRange(m_posVertList,
        m_pos,
        field_2BE,
        field_2AE,
        m_terrainTable,
        targets,
        TRUE,
        field_300);

    field_EA->GetAllInRangeBack(m_pos,
        field_2BE,
        field_2AE,
        m_terrainTable,
        targets,
        TRUE,
        FALSE,
        field_300);

    // Forget objects which are no longer in range.
    std::map<LONG, LONG>::iterator it = field_4C4.begin();
    while (it != field_4C4.end()) {
        BOOL bFound = FALSE;

        POSITION pos = targets.GetHeadPosition();
        while (pos != NULL) {
            LONG nId = reinterpret_cast<LONG>(targets.GetNext(pos));
            if (nId == it->first) {
                bFound = TRUE;
                break;
            }
        }

        if (bFound) {
            ++it;
        } else {
            std::map<LONG, LONG>::iterator curr = it;
            ++it;
            field_4C4.erase(curr);
        }
    }

    std::list<LONG> result;

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nId = reinterpret_cast<LONG>(targets.GetNext(pos));

        if (field_4C4.find(nId) == field_4C4.end()) {
            field_4C4[nId] = 0;
        }

        if (field_4C4[nId] % field_4DC == 0) {
            result.push_back(nId);
        }

        field_4C4[nId]++;
    }

    return result;
}

// 0x5701B0
void CProjectileArea::Virtual94(std::list<LONG>& targets)
{
    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        Virtual98(*it);
    }
}

// 0x5701E0
void CProjectileArea::Virtual98(LONG nTargetId)
{
    CGameObject* pObject;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nTargetId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    // FIXME: Leaks share on `nTargetId`.
    if (field_64E.find(pObject->m_id) != field_64E.end()) {
        return;
    }

    if ((pObject->GetObjectType() & CGameObject::TYPE_AIBASE) != 0 || field_300 != 0) {
        if (pObject->GetObjectType() != CGameObject::TYPE_SPRITE) {
            pObject->GetPos();
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            // FIXME: Leaks share on `nTargetId`.
            if (IcewindMisc::IsDead(static_cast<CGameSprite*>(pObject)) == TRUE) {
                return;
            }
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            static_cast<CGameSprite*>(pObject)->m_posOld = pObject->GetPos();
        }

        CProjectile* pProjectile = CProjectile::DecodeProjectile(field_2B2 + 1, NULL, FALSE);
        pProjectile->field_18A = field_18A;
        pProjectile->field_186 = field_186;
        pProjectile->m_fireSoundRef = CResRef("");

        POSITION pos = m_effectList.GetHeadPosition();
        while (pos != NULL) {
            CGameEffect* pEffect = m_effectList.GetNext(pos);
            pProjectile->AddEffect(pEffect->Copy());
        }

        pProjectile->Fire(field_EA,
            m_id,
            nTargetId,
            pObject->GetPos(),
            50,
            0);

        // NOTE: Releases share on `m_callBackProjectile`, which was never
        // acquired here.
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_callBackProjectile,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}
