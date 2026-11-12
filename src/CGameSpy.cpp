#include "CGameSpy.h"

#include <winsock2.h>

// #binary-identical
// 0x4D0930
CGameSpy::CGameSpy()
{
    field_2 = 0;
    field_12 = 0;
    field_16 = 0;
    field_22 = 0;
    field_1 = 0;
    field_1E = 0;
    field_1A = 0;
    field_28 = 0;
    field_0 = 0;
    field_26 = 0;
}

// #binary-identical
// 0x4D0960
CGameSpy::~CGameSpy()
{
    if (field_0 == 1) {
        WSACleanup();
    }
    field_0 = 0;
    field_1 = 0;
}

// 0x4D0B20
BOOLEAN CGameSpy::sub_4D0B20(SOCKET* pSocket, BOOLEAN a2)
{
    if (field_0) {
        SOCKET sock;
        if (a2 == TRUE) {
            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        } else {
            sock = socket(AF_INET, SOCK_STREAM, 0);
        }

        if (sock != INVALID_SOCKET) {
            // NOTE: Original code reuses the `a2` argument slot for the
            // non-blocking flag.
            u_long nNonBlocking = 1;
            if (ioctlsocket(sock, FIONBIO, &nNonBlocking) == 0) {
                *pSocket = sock;
                return TRUE;
            }

            closesocket(sock);
        }
    }

    return FALSE;
}

// 0x4D1EC0
BOOLEAN CGameSpy::sub_4D1EC0(BOOL a1)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4D2060
void CGameSpy::sub_4D2060()
{
    // TODO: Incomplete.
}

// 0x4D2210
BOOLEAN CGameSpy::sub_4D2210()
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4D2310
BOOLEAN CGameSpy::sub_4D2310()
{
    // TODO: Incomplete.

    return FALSE;
}
