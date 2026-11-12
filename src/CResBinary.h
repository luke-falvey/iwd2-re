#ifndef CRESBINARY_H_
#define CRESBINARY_H_

#include "CRes.h"

class CResBinary : public CRes {
public:
    CResBinary();
    ~CResBinary();

    // 0x402940
    void* Demand();

    // 0x402980
    int Release();

    /* 0050 */ void* field_50;

    /* 0054 */ int field_54;
};

#endif /* CRESBINARY_H_ */
