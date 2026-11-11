#ifndef CSPELL_H_
#define CSPELL_H_

#include <list>

#include "CResSpell.h"

class CGameAIBase;
class CGameEffect;
class CGameSprite;

class CSpell : public CResHelper<CResSpell, 1006> {
public:
    CSpell();
    CSpell(CResRef res);
    BOOL Demand();
    BOOL Release();
    INT GetAbilityCount();
    BOOL GetIcon(BYTE* resRef);
    SPELL_ABILITY* GetAbility(LONG nAbilityNo) const;
    CGameEffect* DecodeEffect(LONG nAbility, LONG nEffect, CGameAIBase* pCaster, DWORD a4, DWORD a5, BYTE casterLevel);
    SHORT GetLevel() const;
    DWORD GetNotUsableBy() const;
    BOOL CheckUsableBy(CGameSprite* pSprite) const;
    STRREF GetGenericName() const;
    STRREF GetDescription() const;
    SHORT GetCasterType() const;
    CGameEffect* GetCastingEffect(int nEffect) const;
    SHORT GetCastingEffectNo() const;
    WORD GetAnimationType() const;
    DWORD GetItemFlags() const;
    BYTE GetSchool() const;
};


#endif /* CSPELL_H_ */
