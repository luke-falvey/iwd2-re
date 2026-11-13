#include "CUnknown2.h"

// 0x7C24C0
CUnknown2::CUnknown2()
{
    field_0 = 0;
    field_4 = 0;
    field_8 = 0;
    field_28 = 0;
    field_2C = 0;
    field_30 = 0;
}

// 0x7C24F0
CUnknown2::~CUnknown2()
{
    // NOTE: Uninline.
    sub_7C25B0();
}

// 0x799C90
BOOL CUnknown2::sub_799C90(CSoundMixer* pMixer)
{
    return TRUE;
}

// 0x7C25B0
void CUnknown2::sub_7C25B0()
{
    POSITION pos = field_C.GetHeadPosition();
    while (pos != NULL) {
        CUnknown2Entry* pEntry = static_cast<CUnknown2Entry*>(field_C.GetAt(pos));
        IUnknown2Entry* pInterface = pEntry->pInterface;

        // NOTE: When `pInterface` is NULL, the original code does not advance
        // to the next node.
        if (pInterface != NULL) {
            pInterface->field_38();
            pInterface->Release();

            POSITION posRemove = pos;
            pEntry = static_cast<CUnknown2Entry*>(field_C.GetNext(pos));
            field_C.RemoveAt(posRemove);
            delete pEntry;
        }
    }

    if (field_8 != NULL) {
        field_8->Release();
    }

    field_8 = NULL;
    if (field_4 != NULL) {
        field_4->Release();
    }

    field_4 = NULL;
    if (field_0 != NULL) {
        field_0->Release();
    }

    field_0 = NULL;
}
