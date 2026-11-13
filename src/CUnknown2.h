#ifndef CUNKNOWN2_H_
#define CUNKNOWN2_H_

#include "mfc.h"

#include <unknwn.h>

class CSoundMixer;

// NOTE: Interface of the objects held by `CUnknown2::field_C`. Only `Release`
// and the method at vtable offset 0x38 are known.
struct IUnknown2Entry : public IUnknown {
    virtual void STDMETHODCALLTYPE field_0C() = 0;
    virtual void STDMETHODCALLTYPE field_10() = 0;
    virtual void STDMETHODCALLTYPE field_14() = 0;
    virtual void STDMETHODCALLTYPE field_18() = 0;
    virtual void STDMETHODCALLTYPE field_1C() = 0;
    virtual void STDMETHODCALLTYPE field_20() = 0;
    virtual void STDMETHODCALLTYPE field_24() = 0;
    virtual void STDMETHODCALLTYPE field_28() = 0;
    virtual void STDMETHODCALLTYPE field_2C() = 0;
    virtual void STDMETHODCALLTYPE field_30() = 0;
    virtual void STDMETHODCALLTYPE field_34() = 0;
    virtual void STDMETHODCALLTYPE field_38() = 0;
};

struct CUnknown2Entry {
    IUnknown2Entry* pInterface;
};

// Unknown class in `CSoundMixer`.
class CUnknown2 {
public:
    CUnknown2();
    ~CUnknown2();
    BOOL sub_799C90(CSoundMixer* pMixer);
    void sub_7C25B0();

    IUnknown* field_0;
    IUnknown* field_4;
    IUnknown* field_8;
    CPtrList field_C;
    int field_28;
    int field_2C;
    int field_30;
};

#endif /* CUNKNOWN2_H_ */
