#include "CProjectileBAM.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CUtil.h"

// 0x52AD60
CProjectileBAM::CProjectileBAM(CResRef resRef)
    : m_palette(CVidPalette::TYPE_RANGE)
{
    field_1CA = 0;
    field_1CE = 0;
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
    field_1DE = 1;
    field_E6 = 0x20000;
    field_1DA = 0;
    field_29C = FALSE;
    memcpy(m_terrainTable, CGameObject::DEFAULT_TERRAIN_TABLE, 16);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    field_182 = CGameObjectArray::INVALID_INDEX;
}

// 0x52B030
CProjectileBAM::~CProjectileBAM()
{
    if (m_pVidCell != NULL) {
        delete m_pVidCell;
    }

    if (field_196 != NULL) {
        delete field_196;
    }
}

// 0x5295D0
void CProjectileBAM::SetVidCell(CResRef resRef)
{
    if (m_pVidCell != NULL) {
        delete m_pVidCell;
    }

    m_pVidCell = new CVidCell(resRef, FALSE);
}

// 0x529660
void CProjectileBAM::SetBitmap(CString sName)
{
    if (!sName.IsEmpty()) {
        // NOTE: Uninline.
        field_1E2.SetResRef(CResRef(sName), TRUE, TRUE);
        field_1E2.m_bDoubleSize = FALSE;
        field_29C = TRUE;
    }
}

// 0x52B150
void CProjectileBAM::SetPaletteRange(USHORT nRange, USHORT nValue)
{
    m_pVidCell->SetPalette(m_palette);
    m_palette.SetRange(nRange, nValue, g_pBaldurChitin->GetObjectGame()->m_rgbMasterBitmap);
}

// 0x52B900
void CProjectileBAM::AIUpdate()
{
    // TODO: Incomplete.
}

// 0x52B190
void CProjectileBAM::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x52C050
void CProjectileBAM::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
}

// 0x5297D0
DWORD CProjectileBAM::Virtual80()
{
    return field_E6;
}

// 0x52BD20
void CProjectileBAM::Virtual84(CPoint dest)
{
    // TODO: Incomplete.
}
