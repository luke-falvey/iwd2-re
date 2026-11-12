#ifndef CPROJECTILEPART4_H_
#define CPROJECTILEPART4_H_

#include "CProjectileArea.h"

// vtable 0x84F274
class CProjectileArea84F274 : public CProjectileArea {
public:
    CProjectileArea84F274();
};

// vtable 0x84F310
class CProjectileArea84F310 : public CProjectileArea {
public:
    CProjectileArea84F310();
};

// vtable 0x84F3AC
class CProjectileArea84F3AC : public CProjectileArea {
public:
    CProjectileArea84F3AC();
    /* 0098 */ void Virtual98(LONG nTargetId) override;
};

// vtable 0x84F448
class CProjectileArea84F448 : public CProjectileArea {
public:
    CProjectileArea84F448();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
    int sub_571A60();
};

// vtable 0x84F4E4
class CProjectileArea84F4E4 : public CProjectileArea {
public:
    CProjectileArea84F4E4();
};

// vtable 0x84F580
class CProjectileArea84F580 : public CProjectileArea {
public:
    CProjectileArea84F580();
};

// vtable 0x84F61C
class CProjectileArea84F61C : public CProjectileArea {
public:
    CProjectileArea84F61C();
};

// vtable 0x84F6B8
class CProjectileArea84F6B8 : public CProjectileArea {
public:
    CProjectileArea84F6B8();
};

// vtable 0x84F754
class CProjectileArea84F754 : public CProjectileArea {
public:
    CProjectileArea84F754();
    /* 006C */ void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;
};

// vtable 0x84F7F0
class CProjectileArea84F7F0 : public CProjectileArea {
public:
    CProjectileArea84F7F0();
};

// vtable 0x84F88C
class CProjectileArea84F88C : public CProjectileArea {
public:
    CProjectileArea84F88C();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

// vtable 0x84F928
class CProjectileArea84F928 : public CProjectileArea {
public:
    CProjectileArea84F928();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

// vtable 0x84F9C4
class CProjectileArea84F9C4 : public CProjectileArea {
public:
    CProjectileArea84F9C4();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

// vtable 0x84FA60
class CProjectileArea84FA60 : public CProjectileArea {
public:
    CProjectileArea84FA60();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

// vtable 0x84FAFC
class CProjectileArea84FAFC : public CProjectileArea {
public:
    CProjectileArea84FAFC();
};

// vtable 0x84FB98
class CProjectileArea84FB98 : public CProjectileArea {
public:
    CProjectileArea84FB98();
    /* 000C */ void AIUpdate() override;

    /* 065E */ LONG field_65E;
};

// vtable 0x84FC34
class CProjectileArea84FC34 : public CProjectileArea {
public:
    CProjectileArea84FC34();
};

// vtable 0x84FCD0
class CProjectileArea84FCD0 : public CProjectileArea {
public:
    CProjectileArea84FCD0();
};

// vtable 0x84FD6C
class CProjectileArea84FD6C : public CProjectileArea {
public:
    CProjectileArea84FD6C();
    /* 0094 */ void Virtual94(std::list<LONG>& targets) override;
};

#endif /* CPROJECTILEPART4_H_ */
