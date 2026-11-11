#ifndef CRESDLG_H_
#define CRESDLG_H_

#include "CRes.h"

class CResDLG : public CRes {
public:
    CResDLG();
    ~CResDLG();
    void* Demand();
    BOOL Parse(void* pData);

    /* 0050 */ BOOL m_bParsed;
};

// NOTE: Every member is inlined in `CGameDialogSprite::sub_482210`.
class CDialogFile : public CResHelper<CResDLG, 1011> {
public:
    BYTE* GetData()
    {
        if (pRes != NULL) {
            return reinterpret_cast<BYTE*>(pRes->Demand());
        } else {
            return NULL;
        }
    }

    DWORD GetDataSize()
    {
        if (pRes == NULL) {
            return 0;
        }

        if (pRes->Demand() == NULL) {
            return 0;
        }

        DWORD nSize = pRes->m_nResSizeActual;

        pRes->Release();

        return nSize;
    }

    void ReleaseData()
    {
        if (pRes != NULL) {
            pRes->Release();
        }
    }
};

#endif /* CRESDLG_H_ */
