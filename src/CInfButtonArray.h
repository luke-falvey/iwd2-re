#ifndef CINFBUTTONARRAY_H_
#define CINFBUTTONARRAY_H_

#include <deque>

#include "CResRef.h"
#include "CVidCell.h"

class CButtonData;
class CGameButtonList;
class CGameSprite;

class CInfButtonSettings {
public:
    CInfButtonSettings();
    ~CInfButtonSettings();

    /* 0000 */ int field_0;
    /* 0004 */ int field_4;
    /* 0008 */ int field_8;
    /* 000C */ int field_C;
    /* 0010 */ int field_10;
    /* 0014 */ CVidCell field_14;
    /* 00EE */ CVidCell field_EE;
    /* 01C8 */ int field_1C8;
    /* 01CC */ int field_1CC;
    /* 01D0 */ int field_1D0;
    /* 01D4 */ int field_1D4;
    /* 01D8 */ int field_1D8;
    /* 01DC */ BOOL m_bGreyOut;
};

class CInfButtonArray {
public:
    static const BYTE STATE_NONE;
    static const BYTE byte_851701;
    static const BYTE byte_85170E;

    static CGameButtonList* m_pButtonList;

    CInfButtonArray();
    ~CInfButtonArray();

    static void ClearList();
    static CGameButtonList* GetData(INT nButton, INT nType, BYTE& nClass, DWORD a4, BOOLEAN a5);
    CGameButtonList* GetFeatData(const CResRef& cResRef);
    static void GetQuickButtonData(BYTE buttonNum, CButtonData& cButtonData, INT nType);
    static void GetSelectedQuickWeaponData(CButtonData& cButtonData);
    static void sub_588300(BYTE nButtonNum, CButtonData& cButtonData);
    static BYTE GetSelectedModalMode();
    BYTE GetButtonId(INT buttonType);
    static BOOLEAN sub_5884B0(CButtonData& cButtonData, BOOL a2);
    static void sub_588570(INT buttonNum, INT nType);
    static BOOLEAN sub_5886A0(CButtonData& cButtonData, BOOL a2);
    static BOOLEAN sub_588760(CButtonData& cButtonData, BOOL a2);
    BOOLEAN sub_588820(CButtonData& cButtonData, BOOL a2);
    static void SetQuickButtonData(CButtonData& cButtonData, BYTE buttonNum, INT nType);
    BOOL ResetState();
    void UpdateState();
    BOOL SetState(INT nState, BOOLEAN bPush);
    void sub_589FF0(BOOLEAN bPush, BOOLEAN bAll);
    BOOL UpdateButtons();
    BOOL OnLButtonPressed(int buttonID);
    BOOL OnRButtonPressed(int buttonID);
    BOOL sub_594170(INT buttonType);
    void sub_594280(INT buttonType, CGameSprite* pSprite);
    BOOL PostRender(const CPoint& pos, const CRect& rClip, BOOL bPressed, INT nButton);
    BOOL PreRender(const CPoint& pos, const CRect& rClip, BOOL bPressed, INT nButton);
    BOOL sub_5959A0(INT buttonType);
    BOOL sub_595C80();
    BOOL sub_595D60(const BYTE& nClass);
    BOOLEAN sub_595EB0(CGameSprite* pSprite);
    INT ScrollRight(CGameButtonList* pButtonList);
    INT ScrollLeft(CGameButtonList* pButtonList);
    void UpdateTempSelectedWeaponSet();

    INT GetState();
    void SetSelectedButton(INT nSelectedButton);
    void UpdateCustomizableButtonTypes(INT* pButtonList);
    void UpdateSelectedWeaponSet(BYTE nIndex);

    /* 0000 */ CInfButtonSettings m_buttonArray[12];
    /* 1680 */ INT field_1680[12];
    /* 16B0 */ INT m_buttonTypes[12];
    /* 16E0 */ int field_16E0;
    /* 16E4 */ int field_16E4;
    /* 16E8 */ CVidCell field_16E8;
    /* 17C2 */ CVidCell field_17C2;
    /* 189C */ CVidCell field_189C;
    /* 1976 */ INT field_1976;
    /* 197A */ INT field_197A;
    /* 197E */ INT m_nSelectedButton;
    /* 1982 */ INT m_nState;
    // NOTE: A `std::deque<INT>` in the original (MSVC 6 STL, 0x30 bytes). The
    // helpers at 0x596250 (`pop_front`), 0x5962E0 (`clear`), 0x5961A0
    // (destructor) and 0x596580-0x596B50 are its template instantiations.
    /* 1986 */ std::deque<INT> m_stateStack;
    unsigned char m_stateStackPadding[0x30 - sizeof(std::deque<INT>)];
    /* 19B6 */ INT m_customizableButtonTypes[9];
    /* 19DA */ BYTE m_nCurrentSelectedSpellClass;
    /* 19DC */ DWORD field_19DC;
    /* 19E0 */ CResRef field_19E0;
    /* 19E8 */ BYTE m_nSelectedWeaponSet;
};

#endif /* CINFBUTTONARRAY_H_ */
