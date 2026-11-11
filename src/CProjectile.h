#ifndef CPROJECTILE_H_
#define CPROJECTILE_H_

#include "CResRef.h"
#include "CGameObject.h"
#include "CSound.h"

class CGameAIBase;
class CGameArea;
class CGameEffect;
class CGameSprite;

class CProjectile : public CGameObject {
public:
    CProjectile();
    /* 0000 */ ~CProjectile() override;
    /* 002C */ BOOLEAN CompressTime(DWORD deltaTime) override;
    /* 0068 */ BOOLEAN IsProjectile() override;
    /* 006C */ virtual void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType);
    /* 0070 */ virtual void OnArrival();
    /* 0074 */ virtual void RemoveSelf();
    /* 0078 */ virtual void DeliverEffects();
    /* 007C */ virtual void CallBack();

    void AddEffect(CGameEffect* pEffect);
    void ClearEffects();
    LONG DetermineHeight(CGameSprite* pSprite);
    SHORT GetDirection(CPoint target);
    void PlaySound(CResRef resRef, BOOL loop, BOOL fireAndForget);

    static CProjectile* DecodeProjectile(WORD projectileType, CGameAIBase* pSource, BOOL a3);
    static void sub_52A640(LONG nObjectId, CPoint& pt, BOOLEAN a3);
    static CProjectile* sub_560310(WORD projectileType, CGameAIBase* pSource, BOOL a3);

    /* 006E */ WORD m_projectileType;
    /* 0070 */ SHORT m_speed;
    /* 0072 */ LONG m_sourceId;
    /* 0076 */ LONG m_targetId;
    /* 007A */ LONG m_callBackProjectile;
    /* 007E */ CTypedPtrList<CPtrList, CGameEffect*> m_effectList;
    /* 009A */ SHORT field_9A;
    /* 009C */ CPoint m_posExact;
    /* 00A4 */ CPoint m_posDelta;
    /* 00AC */ CPoint field_AC;
    /* 00B4 */ CPoint field_B4;
    /* 00BC */ CPoint field_BC;
    /* 00C4 */ LONG field_C4;
    /* 00C8 */ CPoint m_posDest;
    /* 00D0 */ BYTE m_terrainTable[16];
    /* 00E0 */ WORD field_E0;
    /* 00E2 */ LONG field_E2;
    /* 00E6 */ DWORD field_E6;
    /* 00EA */ CGameArea* field_EA;
    /* 00EE */ CSound m_sound;
    /* 0152 */ CResRef m_fireSoundRef;
    /* 015A */ BOOL m_loopFireSound;
    /* 015E */ CResRef m_arrivalSoundRef;
    /* 0166 */ BOOL m_loopArrivalSound;
    /* 016A */ BOOLEAN m_bHasHeight;
    /* 016C */ SHORT field_16C;
    /* 016E */ SHORT field_16E;
    /* 0170 */ LONG field_170;
    /* 0174 */ LONG field_174;
    /* 0178 */ LONG field_178;
    /* 017C */ BOOLEAN field_17C;
    /* 017E */ CString field_17E;
    /* 0182 */ LONG field_182;
    /* 0186 */ LONG field_186;
    /* 018A */ CResRef field_18A;
};

#endif /* CPROJECTILE_H_ */
