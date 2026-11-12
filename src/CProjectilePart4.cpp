#include "CProjectilePart4.h"

#include <stdlib.h>

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CProjectilePart3.h"
#include "CGameSparkle.h"
#include "CUtil.h"
#include "IcewindMisc.h"

// 0x571170
CProjectileArea84F274::CProjectileArea84F274()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "AStormX";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "AStormA";
    field_540.field_10 = "ARE_M04";
    field_540.field_20.sub_586A90(TRUE);
    field_540.field_35 = TRUE;
    field_540.field_36 = TRUE;
    field_4D4 = 10;
    field_4D8 = 0;
    field_4DC = 10000;
    field_4C0 = 45;
    field_1D8 = 1;
    field_2AE = 200;
}

// 0x571310
CProjectileArea84F310::CProjectileArea84F310()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "CloudKX";
    field_4E2.field_10 = "RNG_M01";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "CloudKR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 13;
    field_540.field_0 = "CloudKA";
    field_540.field_10 = "ARE_M02";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 500;
    field_4D8 = 0;
    field_4C0 = 1000;
    field_1D8 = 1;
    field_2AE = 150;
}

// 0x5714E0
CProjectileArea84F3AC::CProjectileArea84F3AC()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "DFogX";
    field_4E2.field_10 = "RNG_M01";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "DFogR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 20;
    field_540.field_0 = "DFogA";
    field_540.field_10 = "ARE_M02";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_4D8 = 0;
    field_4C0 = 1100;
    field_1D8 = 1;
    field_2AE = 150;
}

// 0x5716B0
void CProjectileArea84F3AC::Virtual98(LONG nTargetId)
{
    CGameObject* pSource;
    CGameObject* pTarget;

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            &pSource,
            INFINITE)
        != CGameObjectArray::SUCCESS) {
        return;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nTargetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pTarget,
            INFINITE)
        != CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    if (pTarget->GetObjectType() != CGameObject::TYPE_SPRITE) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return;
    }

    if (pTarget->GetObjectType() != CGameObject::TYPE_SPRITE) {
        pTarget->GetPos();
    }

    if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE) {
        static_cast<CGameSprite*>(pTarget)->m_posOld = pTarget->GetPos();
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

    // NOTE: Result unused.
    static_cast<void>(field_4C4.find(nTargetId));

    pProjectile->Fire(field_EA,
        m_id,
        nTargetId,
        pTarget->GetPos(),
        50,
        0);

    // NOTE: Releases share on `m_callBackProjectile`, which was never
    // acquired here.
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_callBackProjectile,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x571930
CProjectileArea84F448::CProjectileArea84F448()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "DSpellX";
    field_4E2.field_10 = "EFF_M42";
    field_4E2.field_20.sub_586A90(TRUE);
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 300;
}

// 0x571A60
int CProjectileArea84F448::sub_571A60()
{
    CGameObject* pObject;
    SHORT nLevel;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        &pObject,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileAreas.cpp
        // __LINE__: 395
        UTIL_ASSERT(FALSE);
    }

    if (pObject->GetObjectType() != CGameObject::TYPE_SPRITE) {
        nLevel = 11;
    } else {
        nLevel = static_cast<CGameSprite*>(pObject)->GetDerivedStats()->SumSorcererWizardLevels();
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    int nTotal = 0;
    for (int nCount = nLevel; nCount > 0; nCount--) {
        nTotal += rand() % 4 + 1;
    }

    return nTotal;
}

// 0x571B30
void CProjectileArea84F448::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG> filtered;

    int nBudget = sub_571A60();

    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end() && nBudget > 0; ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                int nLevel = static_cast<CGameSprite*>(pObject)->GetDerivedStats()->GetLevel();
                if (nLevel <= nBudget && nLevel <= 9) {
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

// 0x571CE0
CProjectileArea84F4E4::CProjectileArea84F4E4()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "EntangX";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "EntangA";
    field_540.field_10 = "ARE_P01";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 450;
    field_4D8 = 0;
    field_4C0 = 1000;
    field_2AE = 200;
}

// 0x571E80
CProjectileArea84F580::CProjectileArea84F580()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    // NOTE: The original checks the length of the (constant) resource name
    // here, which looks like an inlined helper.
    if (strlen("FirebaT") != 0) {
        field_1DE = 1;
        SetVidCell(CResRef("FirebaT"));
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
    field_2AE = 200;
}

// 0x572110
CProjectileArea84F61C::CProjectileArea84F61C()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    // NOTE: The original checks the length of the (constant) resource name
    // here, which looks like an inlined helper.
    if (strlen("MagicStn") != 0) {
        field_1DE = 1;
        SetVidCell(CResRef("MagicStn"));
        field_4E0 = TRUE;
    } else {
        field_1DE = 0;
        field_4E0 = FALSE;
    }
    field_4E2.field_0 = "FSeedsX";
    field_4E2.field_10 = "EFF_P45";
    field_4E2.field_20.sub_586A90(TRUE);
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_1D8 = 16;
    field_2AE = 70;
}

// 0x572290
CProjectileArea84F6B8::CProjectileArea84F6B8()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "FStormX";
    field_4E2.field_10 = "EFF_P45";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "FStormA";
    field_540.field_10 = "ARE_P03";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_4D8 = 0;
    field_4C0 = 105;
    field_1D8 = 1;
    field_2AE = 200;
}

// 0x572470
CProjectileArea84F754::CProjectileArea84F754()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_10 = "EFF_P16";
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 140;
}

// 0x572540
void CProjectileArea84F754::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CProjectile84E8C0* pProjectile = new CProjectile84E8C0(CString("FStrikH"),
        CString(""),
        0xFF,
        0,
        0,
        0x1C);
    pProjectile->Fire(pArea, source, target, targetPos, nHeight, nType);

    CProjectileArea::Fire(pArea, source, target, targetPos, nHeight, nType);
}

// 0x572670
CProjectileArea84F7F0::CProjectileArea84F7F0()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "GreaseX";
    field_4E2.field_10 = "EFF_M31b";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "GreaseA";
    field_540.field_10 = "ARE_M01";
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

// 0x572830
CProjectileArea84F88C::CProjectileArea84F88C()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "EnchanX";
    field_4E2.field_10 = "ARE_M21";
    field_4E2.field_20.sub_586A90(TRUE);
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 200;
}

// 0x572960
void CProjectileArea84F88C::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG> filtered;

    int nCount = rand() % 4 + 1;
    int nFound = 0;

    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                filtered.push_back(*it);
                if (IcewindMisc::IsAnimal(static_cast<CGameSprite*>(pObject)) == TRUE) {
                    nFound++;
                    if (nFound == nCount) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        break;
                    }
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    CProjectileArea::Virtual94(filtered);
}

// 0x572B30
CProjectileArea84F928::CProjectileArea84F928()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "EnchanX";
    field_4E2.field_10 = "ARE_M21";
    field_4E2.field_20.sub_586A90(TRUE);
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 200;
}

// 0x572C60
void CProjectileArea84F928::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG> filtered;

    int nCount = rand() % 4 + 1;
    int nFound = 0;

    std::list<LONG>::iterator it = targets.begin();
    for (; it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() != CGameObject::TYPE_SPRITE) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            } else if (IcewindMisc::IsUndead(static_cast<CGameSprite*>(pObject)) == TRUE) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            } else {
                filtered.push_back(*it);
                nFound++;
                if (nFound == nCount) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    break;
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }

    CProjectileArea::Virtual94(filtered);

    // NOTE: `it` is `targets.end()` unless the loop was left early. The
    // original releases the share on it again here.
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x572E60
CProjectileArea84F9C4::CProjectileArea84F9C4()
    // NOTE: The argument is the value of the global at 0x84F1BC.
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

// 0x572F90
void CProjectileArea84F9C4::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG> filtered;

    int nCount = rand() % 4 + 1;
    int nFound = 0;

    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                filtered.push_back(*it);
                if (IcewindMisc::IsHumanoid(static_cast<CGameSprite*>(pObject)) == TRUE) {
                    nFound++;
                    if (nFound == nCount) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        break;
                    }
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    CProjectileArea::Virtual94(filtered);
}

// 0x573160
CProjectileArea84FA60::CProjectileArea84FA60()
    // NOTE: The argument is the value of the global at 0x84F1BC.
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

// 0x573290
void CProjectileArea84FA60::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG> filtered;

    int nCount = rand() % 4 + 1;
    int nFound = 0;

    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                filtered.push_back(*it);
                if (IcewindMisc::IsUndead(static_cast<CGameSprite*>(pObject)) == TRUE) {
                    nFound++;
                    if (nFound == nCount) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        break;
                    }
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*it,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    CProjectileArea::Virtual94(filtered);
}

// 0x573460
CProjectileArea84FAFC::CProjectileArea84FAFC()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "IStormX";
    field_4E2.field_20.sub_586A90(TRUE);
    field_540.field_0 = "IStormA";
    field_540.field_10 = "ARE_M04";
    field_540.field_20.sub_586A90(TRUE);
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_4D4 = 10;
    field_4D8 = 0;
    field_4DC = 10000;
    field_4C0 = 100;
    field_1D8 = 1;
    field_2AE = 200;
}

// 0x573600
CProjectileArea84FB98::CProjectileArea84FB98()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_65E = 0;
    field_4E2.field_0 = "ICloudX";
    field_4E2.field_10 = "EFF_M40";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "ICloudR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 13;
    field_540.field_0 = "ICloudA";
    field_540.field_10 = "ARE_M05";
    field_540.field_20.sub_586A90(TRUE);
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 1500;
    field_540.field_44 = TRUE;
    field_4C0 = 1050;
    field_1D8 = 1;
    field_2AE = 100;
}

// 0x5737F0
void CProjectileArea84FB98::AIUpdate()
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

    if (field_65E != 0 && field_65E % 100 == 0) {
        PlaySound(CResRef("ARE_M05a"), FALSE, TRUE);

        CTypedPtrList<CPtrList, LONG*> targets(10);

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
            Virtual98(reinterpret_cast<LONG>(targets.GetNext(pos)));
        }
    }

    field_65E++;
    field_4C0--;
    if (field_4C0 <= 0) {
        RemoveSelf();
    }
}

// 0x573AF0
CProjectileArea84FC34::CProjectileArea84FC34()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "IPlaguX";
    field_4E2.field_10 = "RNG_P01";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "IPlaguR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 13;
    field_540.field_0 = "IPlaguA";
    field_540.field_10 = "ARE_P02";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_4D8 = 0;
    field_4C0 = 1500;
    field_1D8 = 1;
    field_2AE = 250;
}

// 0x573CC0
CProjectileArea84FCD0::CProjectileArea84FCD0()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "DFogX";
    field_4E2.field_10 = "RNG_M01";
    field_4E2.field_20.sub_586A90(TRUE);
    field_50E.field_0 = "DFogR";
    field_50E.field_20.sub_586A90(TRUE);
    field_50E.field_2C = TRUE;
    field_50E.field_2E = 26;
    field_540.field_0 = "DFogA";
    field_540.field_10 = "ARE_M02";
    field_540.field_20.sub_586A90(TRUE);
    field_4D4 = 10;
    field_4DC = 10;
    field_540.field_34 = TRUE;
    field_540.field_35 = TRUE;
    field_540.field_38 = 500;
    field_4D8 = 0;
    field_4C0 = 2000;
    field_1D8 = 1;
    field_2AE = 250;
}

// 0x573E90
CProjectileArea84FD6C::CProjectileArea84FD6C()
    // NOTE: The argument is the value of the global at 0x84F1BC.
    : CProjectileArea(256)
{
    field_4E2.field_0 = "PWKillX";
    field_4E2.field_10 = "EFF_M39";
    field_4E2.field_20.sub_586A90(TRUE);
    field_4D4 = 10000;
    field_4D8 = 0;
    field_4DC = 10;
    field_4C0 = 45;
    field_2AE = 150;
}

// 0x573FC0
void CProjectileArea84FD6C::Virtual94(std::list<LONG>& targets)
{
    std::list<LONG> filtered;

    int nBudget = 200;

    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it) {
        CGameObject* pObject;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*it,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                SHORT nHitPoints = static_cast<CGameSprite*>(pObject)->m_baseStats.m_hitPoints;
                if (nHitPoints <= 100 && nHitPoints <= nBudget) {
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
