#ifndef CPROJECTILEPART5_H_
#define CPROJECTILEPART5_H_

#include "CProjectileArea.h"

class CProjectileArea84FE08 : public CProjectileArea {
public:
    CProjectileArea84FE08();
};

class CProjectileArea84FEA4 : public CProjectileArea {
public:
    CProjectileArea84FEA4();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea84FF40 : public CProjectileArea {
public:
    CProjectileArea84FF40();
};

class CProjectileArea84FFDC : public CProjectileArea {
public:
    CProjectileArea84FFDC();
};

class CProjectileArea850078 : public CProjectileArea {
public:
    CProjectileArea850078();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea850114 : public CProjectileArea {
public:
    CProjectileArea850114();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea8501B0 : public CProjectileArea {
public:
    CProjectileArea8501B0();
};

class CProjectileArea85024C : public CProjectileArea {
public:
    CProjectileArea85024C();
    /* 0070 */ void OnArrival() override;
};

class CProjectileArea8502E8 : public CProjectileArea {
public:
    CProjectileArea8502E8();
};

class CProjectileArea850384 : public CProjectileArea {
public:
    CProjectileArea850384();
    /* 000C */ void AIUpdate() override;

    /* 065E */ std::set<LONG> field_65E;
    /* 066E */ LONG field_66E;
};

class CProjectileArea850420 : public CProjectileArea {
public:
    CProjectileArea850420();
    /* 000C */ void AIUpdate() override;
};

class CProjectileArea8504BC : public CProjectileArea {
public:
    CProjectileArea8504BC(SHORT a1);
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea850558 : public CProjectileArea {
public:
    CProjectileArea850558();
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
};

// NOTE: Only created by the mold touch persistent effect.
class CProjectileArea8505F4 : public CProjectileArea {
public:
    CProjectileArea8505F4(SHORT a1);
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

// NOTE: Only created by the shroud of flame persistent effect.
class CProjectileArea850690 : public CProjectileArea {
public:
    CProjectileArea850690(SHORT a1);
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea85072C : public CProjectileArea {
public:
    CProjectileArea85072C();
    /* 000C */ void AIUpdate() override;
    /* 0070 */ void OnArrival() override;

    /* 065E */ LONG field_65E;
};

class CProjectileArea8507C8 : public CProjectileArea {
public:
    CProjectileArea8507C8();
    /* 000C */ void AIUpdate() override;
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
    /* 0090 */ std::list<LONG> Virtual90() override;
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea850864 : public CProjectileArea8507C8 {
public:
    CProjectileArea850864();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea850900 : public CProjectileArea8507C8 {
public:
    CProjectileArea850900();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea85099C : public CProjectileArea {
public:
    CProjectileArea85099C();
    /* 000C */ void AIUpdate() override;
    /* 0070 */ void OnArrival() override;

    /* 065E */ BOOLEAN field_65E;
    /* 0660 */ LONG field_660;
};

class CProjectileArea850A38 : public CProjectileArea {
public:
    CProjectileArea850A38();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea850AD4 : public CProjectileArea {
public:
    CProjectileArea850AD4();
    /* 000C */ void AIUpdate() override;
};

class CProjectileArea850B70 : public CProjectileArea {
public:
    CProjectileArea850B70();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

class CProjectileArea850C0C : public CProjectileArea {
public:
    CProjectileArea850C0C();
    /* 0098 */ void Virtual98(LONG nTargetId) override;
};

#endif /* CPROJECTILEPART5_H_ */
