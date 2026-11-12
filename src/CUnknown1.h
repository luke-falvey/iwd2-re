#ifndef CUNKNOWN1_H_
#define CUNKNOWN1_H_

#include <windows.h>

// NOTE: There is unknown object with non-inlined (albeit empty) constructor and
// destructor in `CChitin`. Probably related to GameSpy functionality. Its
// methods never touch `this`; they implement the (RC4-based) GameSpy secure
// key encoding, with the cipher state (0x102 bytes) passed by the caller.
class CUnknown1 {
public:
    CUnknown1();
    ~CUnknown1();
    void sub_4D0520(BYTE* pKey, int nKeyLength, BYTE* pState);
    void sub_4D05A0(BYTE* pData, int nLength, BYTE* pState);
    void sub_4D0680(BYTE* pIn, BYTE* pOut);
    char sub_4D06D0(BYTE a1);
};

#endif /* CUNKNOWN1_H_ */
