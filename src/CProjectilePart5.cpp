#include "CProjectilePart5.h"

#include <stdlib.h>

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CGameStatic.h"
#include "CInfGame.h"
#include "CMessage.h"
#include "CParticle.h"
#include "CProjectilePart3.h"
#include "CUtil.h"
#include "IcewindMisc.h"

// 0x574140
CProjectileArea84FE08::CProjectileArea84FE08()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "PFireX";
    field_4E2.field_10 = "EFF_P45";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "PFireA";
    field_540.field_10 = "ARE_P03";
    field_540.field_20.sub_586A90(TRUE);
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 750;
    field_4D4 = 10;
    field_4D8 = 0;
    field_4DC = 10000;
    field_4C0 = 100;
    field_1D8 = 1;
    field_2AE = 60;
}

// 0x574300
CProjectileArea84FEA4::CProjectileArea84FEA4()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "EnchanX";
    field_4E2.field_10 = "ARE_M21";
    field_4E2.field_20.sub_586A90(TRUE);
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 150;
}

// 0x574430
void CProjectileArea84FEA4::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG> filtered;

    int nBudget = rand() % 4 + rand() % 4 + 2;

    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                int nLevel = static_cast<CGameSprite*>(pObject)->GetDerivedStats()->GetLevel();
                if (nLevel <= 4 && nBudget - nLevel >= 0) {
                    nBudget -= nLevel;
                    filtered.push_back(*it);
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    CProjectileArea::Virtual94(filtered);
}

// 0x5745E0
CProjectileArea84FF40::CProjectileArea84FF40()
    : CProjectileArea(256)
{
    const char* szTravel = "SSSwarT";
    if (strlen(szTravel) != 0) {
        field_1DE = 1;
        SetVidCell(CResRef(szTravel));
        field_4E0 = TRUE;
    } else {
        field_1DE = 0;
        field_4E0 = FALSE;
    }

    m_fireSoundRef = "TRA_18";
    m_vfx.sub_586A90(TRUE);
    field_4E2.field_0 = "SSSwarX";
    field_4E2.field_10 = "RNG_M02";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "SSSwarR";
    field_50E.field_20.sub_586A90(TRUE);
    field_4D8 = 0;
    m_speed *= 2;
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 26;
    field_4D4 = 10000;
    field_4DC = 10;
    field_4C0 = 45;
    field_1D8 = 16;
    field_2AE = 250;
}

// 0x5747A0
CProjectileArea84FFDC::CProjectileArea84FFDC()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "SStoneA";
    field_4E2.field_10 = "EFF_P48";
    field_540.field_0 = "SStoneA";
    field_540.field_10 = "ARE_P04";
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 500;
    field_4D8 = 0;
    field_4C0 = 1200;
    field_2AE = 150;
}

// 0x574970
CProjectileArea850078::CProjectileArea850078()
    : CProjectileArea(256)
{
    field_2AE = 275;
}

// 0x5749A0
void CProjectileArea850078::Virtual94(std::list<LONG>& targets)
{
    if (targets.begin() != targets.end()) {
        Virtual98(*targets.begin());
        return;
    }

    CGameObject* pObject;
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE)
        == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == 0x31) {
            // NOTE: Sprite effect `2` and intensity `0` are BYTE constants at
            // `0x85BB4B` and `0x85BB4E`.
            static_cast<CGameSprite*>(pObject)->StartSpriteEffect(2, 0, 30, FALSE);
            static_cast<CGameSprite*>(pObject)->FeedBack(CGameSprite::FEEDBACK_STATICCHARGEEND, 0, 0, 0, -1, 0, 0);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x574A70
CProjectileArea850114::CProjectileArea850114()
    : CProjectileArea(256)
{
    // NOTE: The original reads the constructor argument from a global at
    // 0x84F1BC and this value from a constant at 0x84F084.
    field_2AE = 275;
}

// 0x574AA0
void CProjectileArea850114::Virtual94(std::list<LONG>& targets)
{
    CGameObject* pObject;
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE)
        == CGameObjectArray::SUCCESS) {
        CGameSprite* pSprite = NULL;
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite = static_cast<CGameSprite*>(pObject);
        }

        // Lightning can only be called outdoors.
        if ((pObject->GetArea()->GetHeader()->m_areaType & 0x1) != 0) {
            if (targets.begin() != targets.end()) {
                Virtual98(*targets.begin());
            } else if (pSprite != NULL) {
                // NOTE: Sprite effect `2` and intensity `0` are BYTE
                // constants at `0x85BB4B` and `0x85BB4E`.
                pSprite->StartSpriteEffect(2, 0, 30, FALSE);
                pSprite->FeedBack(CGameSprite::FEEDBACK_CALLLIGHTNINGEND, 0, 0, 0, -1, 0, 0);
            }
        } else {
            // NOTE: The original does not check `pSprite` for `NULL` here.
            pSprite->FeedBack(CGameSprite::FEEDBACK_CALLLIGHTNINGEND, 0, 0, 0, -1, 0, 0);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x574B80
CProjectileArea8501B0::CProjectileArea8501B0()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "SCloudX";
    field_4E2.field_10 = "RNG_M01";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "SCloudR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 13;
    field_540.field_0 = "SCloudA";
    field_540.field_10 = "ARE_M02";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_38 = 1000;
    field_4C0 = 1000;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_4D8 = 0;
    field_1D8 = 1;
    field_2AE = 100;
}

// 0x574D50
CProjectileArea85024C::CProjectileArea85024C()
    : CProjectileArea(256)
{
    field_4D4 = 10;
    field_4DC = 10;
    field_4D8 = 0;
    field_4C0 = 325;
    field_2AE = 150;
}

// 0x574DA0
void CProjectileArea85024C::OnArrival()
{
    CProjectile8512BC* pProjectile = new CProjectile8512BC(CString("SunrayX"),
        CString(""),
        0xFF,
        1,
        0,
        0x145);

    pProjectile->Fire(field_EA, m_sourceId, m_targetId, m_pos, 0, 0);

    CProjectileArea::OnArrival();
}

// 0x574EB0
CProjectileArea8502E8::CProjectileArea8502E8()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "WebX";
    field_4E2.field_10 = "EFF_M19";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "WebA";
    field_540.field_10 = "ARE_M03";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 450;
    field_4D8 = 0;
    field_4C0 = 1500;
    field_2AE = 150;
}

// 0x575090
CProjectileArea850384::CProjectileArea850384()
    : CProjectileArea(256)
{
    field_66E = 0;
    field_2AE = 250;
}

// 0x5752E0
void CProjectileArea850384::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    field_66E++;

    if (field_66E % 100 == 1) {
        CTypedPtrList<CPtrList, LONG*> targets(10);
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        m_pos = pObject->GetPos();

        CGameSprite* pSprite = NULL;
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite = static_cast<CGameSprite*>(pObject);
        }

        // NOTE: `pSprite` can be `NULL`.
        if (IcewindMisc::IsDead(pSprite) == TRUE) {
            LONG nSourceId = m_sourceId;
            RemoveSelf();
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nSourceId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return;
        }

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

        POSITION pos = targets.GetHeadPosition();
        while (pos != NULL) {
            DWORD nId = reinterpret_cast<DWORD>(targets.GetNext(pos));
            if (nId != m_sourceId) {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nId,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pObject,
                    INFINITE);
                if (rc != CGameObjectArray::SUCCESS) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    return;
                }

                if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    if (field_65E.find(nId) == field_65E.end()) {
                        field_65E.insert(nId);
                        Virtual98(nId);
                    }
                } else {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    CGameObject* pObject;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    LONG nSourceId = m_sourceId;
    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE
            && IcewindMisc::IsDead(static_cast<CGameSprite*>(pObject)) == TRUE) {
            RemoveSelf();
        }
    } else {
        RemoveSelf();
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nSourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x5756C0
CProjectileArea850420::CProjectileArea850420()
    : CProjectileArea(2000)
{
    field_2BE.Set(CAIObjectType::NOT_SPRITE);
}

// 0x575720
void CProjectileArea850420::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (field_2B6 == 0 && !field_4E0) {
        m_pos.x = m_posDest.x;
        m_pos.y = m_posDest.y;
        OnArrival();
        return;
    }

    CTypedPtrList<CPtrList, LONG*> targets(10);

    m_pArea->GetAllInRange(m_posVertList,
        m_pos,
        field_2BE,
        field_2AE,
        m_terrainTable,
        targets,
        FALSE,
        TRUE);
    m_pArea->GetAllInRangeBack(m_pos,
        field_2BE,
        field_2AE,
        m_terrainTable,
        targets,
        FALSE,
        FALSE,
        TRUE);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nId = reinterpret_cast<LONG>(targets.GetNext(pos));
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_STATIC) {
            CGameStatic* pStatic = static_cast<CGameStatic*>(pObject);
            if (strcmp(pStatic->m_vidCell.cResRef.GetResRefStr(), "1105MS1") == 0) {
                // NOTE: The original switches on the word zero-extended.
                switch (static_cast<WORD>(pStatic->m_vidCell.m_nCurrentSequence)) {
                case 0:
                    pStatic->m_vidCell.SequenceSet(4);
                    break;
                case 1:
                    pStatic->m_vidCell.SequenceSet(5);
                    break;
                case 2:
                    pStatic->m_vidCell.SequenceSet(6);
                    break;
                case 3:
                    pStatic->m_vidCell.SequenceSet(7);
                    break;
                case 4:
                    pStatic->m_vidCell.SequenceSet(0);
                    break;
                case 5:
                    pStatic->m_vidCell.SequenceSet(1);
                    break;
                case 6:
                    pStatic->m_vidCell.SequenceSet(2);
                    break;
                case 7:
                    pStatic->m_vidCell.SequenceSet(3);
                    break;
                default:
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileAreas.cpp
                    // __LINE__: 2939
                    UTIL_ASSERT(FALSE);
                    break;
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    // NOTE: Locals are not initialised in the original.
    LONG nX;
    LONG nY;
    WORD nProjectileType;
    for (int nIndex = 0; nIndex < 5; nIndex++) {
        switch (nIndex) {
        case 0:
            nX = 0x240;
            nY = 0x140;
            nProjectileType = 0x47;
            break;
        case 1:
            nX = 0x25F;
            nY = 0x15F;
            nProjectileType = 0x47;
            break;
        case 2:
            nX = 0x229;
            nY = 0x155;
            nProjectileType = 0x47;
            break;
        case 3:
            nX = 0x21F;
            nY = 0x12C;
            nProjectileType = 0x47;
            break;
        case 4:
            nX = 0x269;
            nY = 0x127;
            nProjectileType = 0x47;
            break;
        }

        CProjectile* pProjectile = CProjectile::sub_560310(nProjectileType, NULL, FALSE);

        CMessage* message = new CMessageFireProjectile(nProjectileType + 0x1000,
            m_id,
            CPoint(nX, nY),
            0,
            m_id,
            m_id,
            0);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        pProjectile->Fire(m_pArea, m_id, m_id, CPoint(nX, nY), 0, 0);
    }

    RemoveSelf();
}

// 0x575B00
CProjectileArea8504BC::CProjectileArea8504BC(SHORT a1)
    : CProjectileArea(a1)
{
}

// 0x575B30
void CProjectileArea8504BC::Virtual94(std::list<LONG>& targets)
{
    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        if (*it == m_sourceId) {
            // NOTE: Only the first occurrence is removed.
            targets.erase(it);
            break;
        }
    }

    CProjectileArea::Virtual94(targets);
}

// 0x5705D0
void CProjectileArea::sub_5705D0(LONG nSourceId)
{
    CMessage* message = new CMessageScreenShake(30, 10, -20, FALSE, nSourceId, nSourceId);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, TRUE);
}

// 0x575B90
CProjectileArea850558::CProjectileArea850558()
    : CProjectileArea(256)
{
    field_4E2.field_10 = "ARE_P27";
    field_2AE = 350;
}

// 0x575C90
void CProjectileArea850558::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    sub_5705D0(source);

    CProjectileArea::Fire(pArea, source, target, targetPos, nHeight, nType);
}

// 0x575CD0
CProjectileArea8505F4::CProjectileArea8505F4(SHORT a1)
    : CProjectileArea(a1)
{
}

// 0x575CF0
void CProjectileArea8505F4::Virtual94(std::list<LONG>& targets)
{
    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            // NOTE: Stops at the first target it cannot obtain.
            return;
        }

        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE
            && !static_cast<CGameSprite*>(pObject)->GetDerivedStats()->m_spellStates.test(SPLSTATE_MOLD_TOUCH)) {
            Virtual98(*it);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            // NOTE: Only the first eligible target is affected.
            return;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x575DD0
CProjectileArea850690::CProjectileArea850690(SHORT a1)
    : CProjectileArea(a1)
{
}

// 0x575DF0
void CProjectileArea850690::Virtual94(std::list<LONG>& targets)
{
    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE
                && !static_cast<CGameSprite*>(pObject)->GetDerivedStats()->m_spellStates.test(SPLSTATE_SHROUD_OF_FLAME)) {
                Virtual98(*it);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x575EA0
CProjectileArea85072C::CProjectileArea85072C()
    : CProjectileArea(256)
{
    field_65E = 0;
    field_4E2.field_0 = "DBreatH";
    field_4E2.field_10 = "ARE_M21";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "DBreatA";
    field_540.field_20.sub_586A90(TRUE);
    field_4D8 = 0;
    field_1DE = 0;
    field_540.field_35 = TRUE;
    field_540.field_36 = TRUE;
    field_540.field_38 = 500;
    field_4D4 = 10000;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 200;
}

// 0x576040
void CProjectileArea85072C::AIUpdate()
{
    field_65E++;

    if (field_65E == 7) {
        field_1DE = 1;
        CProjectileArea::OnArrival();
    }

    if (field_65E >= 7) {
        CProjectileArea::AIUpdate();
    }
}

// 0x799CA0
void CProjectileArea85072C::OnArrival()
{
}

// 0x576080
CProjectileArea8507C8::CProjectileArea8507C8()
    : CProjectileArea(256)
{
    field_2AE = 450;
}

// 0x5760B0
void CProjectileArea8507C8::AIUpdate()
{
    std::list<LONG> targets = Virtual90();
    Virtual94(targets);
    RemoveSelf();
}

// 0x576150
void CProjectileArea8507C8::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    if (pArea == NULL) {
        return;
    }

    m_sourceId = source;
    m_targetId = target;
    field_EA = pArea;

    CGameObject* pObject;
    BYTE rc;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
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
        LONG nSourceId = m_sourceId;
        delete this;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nSourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    CGameObject::AddToArea(pArea,
        ptStart,
        DetermineHeight(static_cast<CGameSprite*>(pObject)),
        CGameObject::LIST_FRONT);

    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    m_posExact.x = ptStart.x << CParticle::RESOLUTION_INC;
    m_posExact.y = 4 * (ptStart.y << CParticle::RESOLUTION_INC) / 3;
    m_posDest.x = targetPos.x;
    m_pos.x = ptStart.x;
    m_pos.y = ptStart.y;
    m_posDest.y = targetPos.y;

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x576310
std::list<LONG> CProjectileArea8507C8::Virtual90()
{
    CTypedPtrList<CPtrList, LONG*> targets(10);

    field_EA->GetAllInRange(m_posVertList,
        m_pos,
        field_2BE,
        field_2AE,
        m_terrainTable,
        targets,
        FALSE,
        field_300);
    field_EA->GetAllInRangeBack(m_pos,
        field_2BE,
        field_2AE,
        m_terrainTable,
        targets,
        FALSE,
        FALSE,
        field_300);

    // Forget the entries for objects that are no longer in range.
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
            field_4C4.erase(it++);
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

// 0x5765F0
void CProjectileArea8507C8::Virtual94(std::list<LONG>& targets)
{
    CGameObject* pObject;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    if (pObject->GetObjectType() != CGameObject::TYPE_SPRITE) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    // NOTE: The share of the source is not released below.
    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pTarget;

        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pTarget,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        if (pTarget->GetObjectType() == 0x31
            && IcewindMisc::sub_5852A0(static_cast<CGameSprite*>(pTarget), static_cast<CGameSprite*>(pObject)) == TRUE) {
            Virtual98(*it);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            // NOTE: Only the first eligible target is affected.
            return;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// NOTE: Inlines `CProjectileArea8507C8` constructor.
//
// 0x576770
CProjectileArea850864::CProjectileArea850864()
{
}

// 0x5767A0
void CProjectileArea850864::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG>::iterator it = targets.begin();
    while (it != targets.end()) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        // NOTE: The iterator is not advanced for non-sprites, so the loop
        // does not terminate.
        if (pObject->GetObjectType() == 0x31) {
            if ((pObject->GetAIType().m_nClassMask & 0xD1) != 0) {
                ++it;
            } else {
                it = targets.erase(it);
            }
        }

        // NOTE: Releases the share of the object the iterator now points to.
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    CProjectileArea8507C8::Virtual94(targets);
}

// NOTE: Inlines `CProjectileArea8507C8` constructor.
//
// 0x576870
CProjectileArea850900::CProjectileArea850900()
{
}

// 0x5768C0
void CProjectileArea850900::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG>::iterator it = targets.begin();
    while (it != targets.end()) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        // NOTE: The iterator is not advanced for non-sprites, so the loop
        // does not terminate.
        if (pObject->GetObjectType() == 0x31) {
            if ((pObject->GetAIType().m_nClassMask & 0x10) != 0) {
                ++it;
            } else {
                it = targets.erase(it);
            }
        }

        // NOTE: Releases the share of the object the iterator now points to.
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    CProjectileArea8507C8::Virtual94(targets);
}

// 0x576990
CProjectileArea85099C::CProjectileArea85099C()
    : CProjectileArea(256)
{
    field_65E = FALSE;
    field_660 = 0;

    const char* szTravel = "FirebaT";
    if (strlen(szTravel) != 0) {
        field_1DE = 1;
        SetVidCell(CResRef(szTravel));
        field_4E0 = TRUE;
    } else {
        field_1DE = 0;
        field_4E0 = FALSE;
    }

    m_fireSoundRef = "TRA_06";
    m_vfx.sub_586A90(TRUE);
    field_4E2.field_0 = "FirebaX";
    field_4E2.field_10 = "RNG_M03";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "FirebaR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 20;
    field_540.field_0 = "FirebaA";
    field_540.field_20.sub_586A90(TRUE);
    field_4D8 = 0;
    m_speed *= 2;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_36 = TRUE;
    field_4D4 = 10000;
    field_4DC = 10;
    field_4C0 = 45;
    field_1D8 = 16;
    field_2AE = 300;
}

// 0x576BA0
void CProjectileArea85099C::AIUpdate()
{
    if (field_2B6 == 0) {
        CProjectileArea::AIUpdate();
        return;
    }

    if (field_65E == TRUE) {
        CProjectileArea::AIUpdate();
        return;
    }

    field_660++;

    if (field_660 == 5) {
        field_660 = 0;

        CTypedPtrList<CPtrList, LONG*> targets(10);

        m_pArea->GetAllInRange(GetVertListPos(),
            GetPos(),
            CAIObjectType::ANYONE,
            100,
            m_terrainTable,
            targets,
            TRUE,
            FALSE);
        m_pArea->GetAllInRangeBack(GetPos(),
            CAIObjectType::ANYONE,
            100,
            m_terrainTable,
            targets,
            TRUE,
            FALSE,
            FALSE);

        if (!targets.IsEmpty()) {
            CProjectileArea::OnArrival();
            field_65E = TRUE;
        }
    }

    m_pVidCell->FrameAdvance();
}

// 0x576CE0
void CProjectileArea85099C::OnArrival()
{
    field_2B6 = 1;
}

// 0x576CF0
CProjectileArea850A38::CProjectileArea850A38()
    : CProjectileArea(256)
{
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 300;
}

// 0x577380
static void sub_577380(std::list<LONG>& list)
{
    std::list<LONG> result;

    while (list.size() != 0) {
        // NOTE: The original starts from the address of the list.
        std::list<LONG>::iterator itBest = list.end();

        for (std::list<LONG>::iterator it = list.begin(); it != list.end(); ++it) {
            CGameObject* pObject;

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                if (pObject->GetObjectType() == 0x31) {
                    // NOTE: The level is compared to a constant `-1` and the
                    // second call to `GetLevel` is not used.
                    if (static_cast<CGameSprite*>(pObject)->GetLevel() > -1) {
                        static_cast<CGameSprite*>(pObject)->GetLevel();
                        itBest = it;
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        result.push_back(*itBest);
        list.erase(itBest);
    }

    list = result;
}

// 0x576DC0
void CProjectileArea850A38::Virtual94(std::list<LONG>& targets)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CGameObject* pObject;

    BYTE rc = pGame->GetObjectArray()->GetShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    if (pObject->GetObjectType() != 0x31) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

    INT nMaxLevel = g_pBaldurChitin->GetObjectGame()->m_ruleTables.GetAbilityScoreModifier(pSprite->m_derivedStats.m_nCHR)
        + CUtil::UtilRandInt(20, pSprite->m_derivedStats.m_nLuck)
        + 1;
    SHORT nTurnLevel = pSprite->GetTurnUndeadLevel();
    nMaxLevel -= 10;
    if (nMaxLevel >= 0) {
        nMaxLevel = nMaxLevel / 3 + nTurnLevel;
    } else {
        nMaxLevel = (nMaxLevel + 1) / 3 + nTurnLevel - 1;
    }

    INT nMaxHitDice = pGame->m_ruleTables.GetAbilityScoreModifier(pSprite->m_derivedStats.m_nCHR);
    nMaxHitDice += CUtil::UtilRandInt(6, pSprite->m_derivedStats.m_nLuck);
    nMaxHitDice += CUtil::UtilRandInt(6, pSprite->m_derivedStats.m_nLuck);
    nMaxHitDice += 2;

    if (pSprite->sub_763150(0x21) == TRUE) {
        nMaxHitDice += 2;
    }

    sub_577380(targets);

    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pTargetObject;

        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pTargetObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            continue;
        }

        if (pTargetObject->GetObjectType() != 0x31) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            continue;
        }

        CGameSprite* pTarget = static_cast<CGameSprite*>(pTargetObject);

        if (IcewindMisc::IsUndead(pTarget) != TRUE) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            continue;
        }

        if (pTarget->m_derivedStats.m_spellStates.test(SPLSTATE_TURNED)) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            continue;
        }

        LONG nLevel = pTarget->GetLevel();
        if (nLevel > nMaxLevel) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            continue;
        }

        if (nLevel > nMaxHitDice) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            continue;
        }

        nTurnLevel = pSprite->GetTurnUndeadLevel();

        std::string sEffect;
        if (IcewindMisc::IsEvil(pSprite) == TRUE) {
            if (nTurnLevel < 2 * nLevel) {
                sEffect = "EffTU2";
            } else {
                sEffect = "EffTU1";
            }
        } else {
            if (nTurnLevel < 2 * nLevel) {
                sEffect = "EffTU4";
            } else {
                sEffect = "EffTU3";
            }
        }

        std::list<CGameEffect*> effects = IcewindMisc::GetSpellEffects(pSprite, sEffect.c_str(), 1);

        for (std::list<CGameEffect*>::iterator itEffect = effects.begin(); itEffect != effects.end(); ++itEffect) {
            CGameEffect* pEffect = *itEffect;

            pEffect->m_casterLevel = nTurnLevel;

            if (pEffect->m_effectID == 0x118) {
                pEffect->m_numDice = m_sourceId;
            }

            CMessage* message = new CMessageAddEffect(pEffect, m_sourceId, *it);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x577590
CProjectileArea850AD4::CProjectileArea850AD4()
    : CProjectileArea(100)
{
    const char* szTravel = "BIGBOLDR";
    if (strlen(szTravel) != 0) {
        field_1DE = 1;
        SetVidCell(CResRef(szTravel));
        field_4E0 = TRUE;
    } else {
        field_1DE = 0;
        field_4E0 = FALSE;
    }

    field_540.field_34 = TRUE;
    field_540.field_35 = FALSE;
    field_4D4 = 10;
    field_4D8 = 0;
    field_4DC = 10;
    field_1D8 = 1;
    field_2AE = 100;
    m_speed = 7;
    m_fireSoundRef = CResRef("AM6103e");
    m_arrivalSoundRef = CResRef("AM5101e");
}

// 0x5776E0
void CProjectileArea850AD4::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    m_pVidCell->FrameAdvance();

    LONG dy = m_posDest.y - m_pos.y;
    LONG dx = m_posDest.x - m_pos.x;
    if (dy * dy * 16 / 9 + dx * dx <= (m_speed + 1) * (m_speed + 1)) {
        CMessage* message = new CMessageScreenShake(30, 30, -30, FALSE, m_sourceId, m_sourceId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, TRUE);

        OnArrival();
        RemoveSelf();
        return;
    }

    Virtual84(m_posDest);

    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);

    field_4D8--;
    if (field_4D8 <= 0) {
        std::list<LONG> targets = Virtual90();
        Virtual94(targets);
        field_4D8 = field_4D4;
    }
}

// 0x5778A0
CProjectileArea850B70::CProjectileArea850B70()
    : CProjectileArea(256)
{
    field_4E2.field_0 = "SoPainX";
    field_4E2.field_10 = "EFF_P49";
    field_4E2.field_20.sub_586A90(TRUE);
    field_2AE = 300;
}

// 0x5779A0
void CProjectileArea850B70::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG> filtered;

    int nBudget = 150;

    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                int nHitPoints = static_cast<CGameSprite*>(pObject)->m_baseStats.m_hitPoints;
                if (nHitPoints <= nBudget) {
                    nBudget -= nHitPoints;
                    filtered.push_back(*it);
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    CProjectileArea::Virtual94(filtered);
}

// 0x577B20
CProjectileArea850C0C::CProjectileArea850C0C()
    : CProjectileArea(256)
{
    m_fireSoundRef = CResRef("TRA_04A");
    m_arrivalSoundRef = CResRef("CAS_P02");
    field_4E2.field_0 = "AbjuraX";
    field_4E2.field_10 = "ARE_M20";
    field_4E2.field_20.sub_586A90(TRUE);
    field_2AE = 150;
}

// 0x577C70
void CProjectileArea850C0C::Virtual98(LONG nTargetId)
{
    CGameObject* pObject;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nTargetId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    if (pObject->GetObjectType() != CGameObject::TYPE_SPRITE) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    if (g_pChitin->cNetwork.m_nServiceProvider == CNetwork::SERV_PROV_NULL
        || g_pChitin->cNetwork.m_idLocalPlayer == pObject->m_remotePlayerID) {
        static_cast<CGameSprite*>(pObject)->AddEffect(IcewindMisc::CreateEffectRemoveAllOfType(pObject, 450, 0),
            CGameAIBase::EFFECT_LIST_TIMED,
            TRUE,
            TRUE);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    CProjectileArea::Virtual98(nTargetId);
}
