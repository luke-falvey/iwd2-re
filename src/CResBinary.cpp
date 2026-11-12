#include "CResBinary.h"

// 0x4028F0
CResBinary::CResBinary()
{
    field_54 = 0;
}

// 0x402930
CResBinary::~CResBinary()
{
}

// 0x402940
void* CResBinary::Demand()
{
    void* pData = CRes::Demand();

    if (field_54 == 0 || CRes::GetDemands() <= 1) {
        if (pData != NULL) {
            field_50 = reinterpret_cast<BYTE*>(pData) + 4;
            field_54 = 1;
        }

        if (field_54 == 0) {
            return NULL;
        }
    }

    return pData;
}

// 0x402980
int CResBinary::Release()
{
    int rc = CRes::Release();

    if (CRes::GetDemands() <= 0) {
        field_54 = 0;
    }

    return rc;
}
