#ifndef CPROJECTILEBAM_H_
#define CPROJECTILEBAM_H_

#include "CProjectile.h"
#include "CVidBitmap.h"
#include "CVidCell.h"
#include "CVidPalette.h"

class CProjectileBAM : public CProjectile {
public:
    CProjectileBAM(CResRef resRef);
    /* 0000 */ ~CProjectileBAM() override;
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0080 */ virtual DWORD Virtual80();
    /* 0084 */ virtual void Virtual84(CPoint dest);

    void SetVidCell(CResRef resRef);
    void SetBitmap(CString sName);
    void SetPaletteRange(USHORT nRange, USHORT nValue);
    void sub_52B6B0(CRect& rFx, CPoint& ptRef, CVidCell* pVidCell);
    void sub_52C8C0(CVidCell* pVidCell);

    /* 0192 */ CVidCell* m_pVidCell;
    /* 0196 */ CVidCell* field_196;
    /* 019A */ CVidPalette m_palette;
    /* 01BE */ LONG field_1BE;
    /* 01C2 */ LONG field_1C2;
    /* 01C6 */ LONG field_1C6;
    /* 01CA */ LONG field_1CA;
    /* 01CE */ LONG field_1CE;
    /* 01D2 */ SHORT field_1D2;
    /* 01D4 */ LONG field_1D4;
    /* 01D8 */ SHORT field_1D8;
    /* 01DA */ SHORT field_1DA;
    /* 01DC */ SHORT m_nDirection;
    /* 01DE */ LONG field_1DE;
    /* 01E2 */ CVidBitmap field_1E2;
    /* 029C */ BOOLEAN field_29C;
    /* 029D */ BOOLEAN field_29D;
    /* 029E */ SHORT field_29E;
};

#endif /* CPROJECTILEBAM_H_ */
