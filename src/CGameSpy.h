#ifndef CGAMESPY_H_
#define CGAMESPY_H_

#include "mfc.h"

// #guess
class CGameSpy {
public:
    CGameSpy();
    ~CGameSpy();
    BOOLEAN sub_4D0B20(SOCKET* pSocket, BOOLEAN a2);
    BOOLEAN sub_4D1EC0(BOOL a1);
    BOOLEAN sub_4D2060();
    BOOLEAN sub_4D2210();
    BOOLEAN sub_4D2310();
    BOOLEAN sub_4D0980();
    BOOLEAN sub_4D0B80(SOCKET s, DWORD* pAddr, WORD* pPort, char** ppBuf, DWORD* pLen);
    void sub_4D15D0(DWORD nAddr, DWORD nPort, char* pBuf, DWORD nLen);

    /* 0000 */ unsigned char field_0;
    /* 0001 */ unsigned char field_1;
    /* 0002 */ int field_2;
    /* 0006 */ DWORD field_6;
    /* 000A */ SOCKET field_A;
    /* 000E */ SOCKET field_E;
    /* 0012 */ int field_12;
    /* 0016 */ int field_16;
    /* 001A */ int field_1A;
    /* 001E */ int field_1E;
    /* 0022 */ int field_22;
    /* 0026 */ unsigned char field_26;
    /* 0028 */ int field_28;
};

#endif /* CGAMESPY_H_ */
