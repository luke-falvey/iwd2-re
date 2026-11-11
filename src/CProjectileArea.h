#ifndef CPROJECTILEAREA_H_
#define CPROJECTILEAREA_H_

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "CAIObjectType.h"
#include "CProjectileSingleTarget.h"

// NOTE: Original layout uses MSVC 6 `std::string` (16 bytes).
class CProjectileAreaVisual {
public:
    /* 0000 */ std::string field_0;
    /* 0010 */ std::string field_10;
    /* 0020 */ IcewindCVisualEffect field_20;
};

class CProjectileAreaVisual2 : public CProjectileAreaVisual {
public:
    /* 002C */ BOOLEAN field_2C;
    /* 002E */ LONG field_2E;
};

// NOTE: Original layout uses MSVC 6 `std::vector` (16 bytes).
class CProjectileArea5868E0 {
public:
    class Entry {
    public:
        /* 0000 */ LONG field_0;
        /* 0004 */ LONG field_4;
        /* 0008 */ SHORT field_8;
    };

    CProjectileArea5868E0();

    /* 0000 */ std::vector<Entry> field_0;
    /* 0010 */ LONG field_10;
};

class CProjectileAreaVisual3 : public CProjectileAreaVisual {
public:
    /* 002C */ CProjectileArea5868E0* field_2C;
    /* 0030 */ LONG field_30;
    /* 0034 */ BOOLEAN field_34;
    /* 0035 */ BOOLEAN field_35;
    /* 0036 */ BOOLEAN field_36;
    /* 0038 */ LONG field_38;
    /* 003C */ LONG field_3C;
    /* 0040 */ LONG field_40;
    /* 0044 */ BOOLEAN field_44;
};

class CProjectileArea : public CProjectileSingleTarget {
public:
    CProjectileArea(SHORT a1);
    /* 000C */ void AIUpdate() override;
    /* 004C */ void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0070 */ void OnArrival() override;
    /* 0088 */ virtual void Virtual88();
    /* 008C */ virtual LONG Virtual8C(BYTE a1);
    /* 0090 */ virtual std::list<LONG> Virtual90();
    /* 0094 */ virtual void Virtual94(std::list<LONG>& targets);
    /* 0098 */ virtual void Virtual98(LONG nTargetId);

    void sub_5703F0(CGameAIBase* pSource);
    void sub_5704E0(CGameAIBase* pSource);

    /* 02AE */ SHORT field_2AE;
    /* 02B0 */ SHORT field_2B0;
    /* 02B2 */ SHORT field_2B2;
    /* 02B4 */ SHORT field_2B4;
    /* 02B6 */ LONG field_2B6;
    /* 02BA */ SHORT field_2BA;
    /* 02BC */ SHORT field_2BC;
    /* 02BE */ CAIObjectType field_2BE;
    /* 02FA */ LONG field_2FA;
    /* 02FE */ BOOLEAN field_2FE;
    /* 02FF */ BOOLEAN field_2FF;
    /* 0300 */ LONG field_300;
    /* 0304 */ LONG field_304;
    /* 0308 */ CVidCell field_308;
    /* 03E2 */ LONG field_3E2;
    /* 03E6 */ CVidCell field_3E6;
    /* 04C0 */ LONG field_4C0;
    /* 04C4 */ std::map<LONG, LONG> field_4C4;
    /* 04D4 */ LONG field_4D4;
    /* 04D8 */ LONG field_4D8;
    /* 04DC */ LONG field_4DC;
    /* 04E0 */ BOOLEAN field_4E0;
    /* 04E2 */ CProjectileAreaVisual field_4E2;
    /* 050E */ CProjectileAreaVisual2 field_50E;
    /* 0540 */ CProjectileAreaVisual3 field_540;
    /* 0586 */ CSound field_586;
    /* 05EA */ CSound field_5EA;
    /* 064E */ std::set<LONG> field_64E;
};

#endif /* CPROJECTILEAREA_H_ */
