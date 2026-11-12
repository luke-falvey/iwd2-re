#include "CProjectilePart1.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CUtil.h"

// 0x529A90
CProjectileBAM84DA4C::CProjectileBAM84DA4C(CResRef resRef)
    : CProjectileBAM(resRef)
{
}

// 0x536D40
void CProjectileBAM84DA4C::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x5369A0
void CProjectileBAM84DA4C::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x52CF80
void CProjectileBAM84DBDC::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x52D9F0
void CProjectileBAM84DBDC::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
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
        LONG nTargetId = reinterpret_cast<LONG>(targets.GetNext(pos));

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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x52E940
void CProjectileBAM84DC68::Virtual88()
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x52F1C0
void CProjectileBAM84DCF4::Virtual88()
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x52F760
void CProjectileBAM84DD80::Virtual88()
{
    // TODO: Incomplete.
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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x530510
void CProjectileBAM84DFA4::Virtual88(CGameArea* pArea, CPoint pos)
{
    // TODO: Incomplete.
}
