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
    // TODO: Incomplete.
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
    // TODO: Incomplete.
}

// 0x534DD0
void CProjectile84E8C0::Render(CGameArea* pArea, CVidMode* pVidMode, int a3)
{
    // TODO: Incomplete.
}

// 0x535100
void CProjectile84E8C0::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    // TODO: Incomplete.
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
