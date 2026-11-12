#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "CGameSpy.h"

#include <winsock2.h>

#include "CChitin.h"

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

// 0x4D0980
BOOLEAN CGameSpy::sub_4D0980()
{
    CString sHost;
    sHost = "master.gamespy.com";

    hostent* pHost = NULL;
    char buffer[0x400];

    EnterCriticalSection(&(g_pChitin->cNetwork.field_F6A));

    if (!field_0) {
        WSADATA wsaData;
        WSAStartup(2, &wsaData);
        if (wsaData.wVersion != 2) {
            WSACleanup();
            field_0 = 0;
        } else {
            field_0 = 1;
        }

        if (!field_0) {
            // NOTE: Original code leaves the critical section entered on
            // this path.
            return FALSE;
        }
    }

    CWnd* pWnd = g_pChitin->GetWnd();
    HANDLE hRequest = WSAAsyncGetHostByName(pWnd != NULL ? pWnd->GetSafeHwnd() : NULL,
        0x402,
        sHost,
        buffer,
        0x400);

    field_26 = 0;

    if (hRequest == NULL) {
        field_0 = 0;
        WSACleanup();
    } else {
        BYTE nTries = 0;
        do {
            nTries++;
            SleepEx(300, FALSE);
        } while (!field_26 && nTries < 10);

        if (!field_26) {
            WSACancelAsyncRequest(hRequest);
            field_0 = 0;
            WSACleanup();
        } else {
            pHost = reinterpret_cast<hostent*>(buffer);
        }
    }

    if (field_0 == 1) {
        if (pHost != NULL) {
            field_6 = *reinterpret_cast<DWORD*>(pHost->h_addr_list[0]);
        } else {
            field_0 = 0;
            WSACleanup();
        }
    }

    LeaveCriticalSection(&(g_pChitin->cNetwork.field_F6A));
    return field_0;
}

// 0x4D0B80
BOOLEAN CGameSpy::sub_4D0B80(SOCKET s, DWORD* pAddr, WORD* pPort, char** ppBuf, DWORD* pLen)
{
    char* pBuf = new char[0x100];
    if (pBuf == NULL) {
        return FALSE;
    }

    sockaddr_in from;
    int nFromLen;

    memset(pBuf, 0, 0x100);
    nFromLen = sizeof(from);
    if (recvfrom(s, pBuf, 0x100, MSG_PEEK, reinterpret_cast<sockaddr*>(&from), &nFromLen) == -1) {
        delete[] pBuf;
        return FALSE;
    }

    memset(pBuf, 0, 0x100);
    nFromLen = sizeof(from);
    int nReceived = recvfrom(s, pBuf, 0x100, 0, reinterpret_cast<sockaddr*>(&from), &nFromLen);
    if (nReceived == -1) {
        delete[] pBuf;
        return FALSE;
    }

    *pAddr = from.sin_addr.s_addr;
    *pPort = ntohs(from.sin_port);
    *ppBuf = pBuf;
    *pLen = nReceived;
    return TRUE;
}

// 0x4D15D0
void CGameSpy::sub_4D15D0(DWORD nAddr, DWORD nPort, char* pBuf, DWORD nLen)
{
    // TODO: Incomplete.
}

// 0x4D1EC0
BOOLEAN CGameSpy::sub_4D1EC0(BOOL a1)
{
    CString sGameName;

    if (field_1 && g_pChitin->cNetwork.m_bIsHost) {
        EnterCriticalSection(&(g_pChitin->cNetwork.field_F6A));

        char* pBuffer = new char[0x50];
        if (pBuffer != NULL) {
            g_pChitin->GetGameSpyGameName(sGameName);

            if (a1 == 1) {
                sprintf(pBuffer, "\\heartbeat\\%d\\gamename\\%s\\statechanged\\\n", field_12, static_cast<LPCSTR>(sGameName));
            } else {
                sprintf(pBuffer, "\\heartbeat\\%d\\gamename\\%s\\\n", field_12, static_cast<LPCSTR>(sGameName));
            }

            int nLength = strlen(pBuffer);
            char* pCopy = new char[nLength];
            if (pCopy != NULL) {
                memcpy(pCopy, pBuffer, nLength);

                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = field_6;
                addr.sin_port = htons(0x6CFC);
                sendto(field_A, pCopy, nLength, 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

                delete[] pCopy;
                delete[] pBuffer;

                LeaveCriticalSection(&(g_pChitin->cNetwork.field_F6A));
                return TRUE;
            }

            // NOTE: Original code leaves the critical section entered on
            // this path.
            delete[] pBuffer;
        }
    }

    return FALSE;
}

// 0x4D2060
BOOLEAN CGameSpy::sub_4D2060()
{
    if (!sub_4D0980()) {
        field_1 = FALSE;
        return FALSE;
    }

    EnterCriticalSection(&(g_pChitin->cNetwork.field_F6A));

    field_2 = GetTickCount() - 0x7530;

    WORD nPort = g_pChitin->GetMultiplayerGameSpyPort();
    if (nPort == 0) {
        nPort = 0x5BE;
    }

    field_1 = TRUE;
    if (!field_0 || !g_pChitin->cNetwork.m_bIsHost) {
        field_1 = FALSE;
    }

    if (field_1 == TRUE) {
        // NOTE: Uninline.
        if (!sub_4D0B20(&field_A, TRUE)) {
            field_1 = FALSE;
        }
    }

    if (field_1 == TRUE) {
        // NOTE: Uninline.
        if (!sub_4D0B20(&field_E, TRUE)) {
            field_1 = FALSE;
        }

        if (field_1 == TRUE) {
            SOCKET s = field_E;
            if (field_0) {
                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = 0;
                addr.sin_port = htons(nPort);
                if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
                    closesocket(s);
                    closesocket(field_A);
                    field_1 = FALSE;
                }
            } else {
                closesocket(field_A);
                field_1 = FALSE;
            }
        }
    }

    field_12 = nPort;
    field_28 = 0;

    if (!field_1) {
        field_0 = 1;
        WSACleanup();
        field_0 = 0;
    }

    LeaveCriticalSection(&(g_pChitin->cNetwork.field_F6A));
    return field_1;
}

// 0x4D2210
BOOLEAN CGameSpy::sub_4D2210()
{
    if (field_1 && g_pChitin->cNetwork.m_bIsHost) {
        if (field_28 <= 5) {
            if (GetTickCount() > static_cast<DWORD>(field_2 + 0xEA6)) {
                field_28++;
                field_2 = GetTickCount();
                sub_4D1EC0(0);
            }
        }

        if (GetTickCount() > static_cast<DWORD>(field_2 + 0x7530)) {
            field_2 = GetTickCount();
            sub_4D1EC0(0);
        }

        EnterCriticalSection(&(g_pChitin->cNetwork.field_F6A));

        DWORD nAddr;
        DWORD nPort;
        char* pBuf;
        DWORD nLen;
        // NOTE: Only the low word of nPort is written by the callee.
        if (sub_4D0B80(field_E, &nAddr, reinterpret_cast<WORD*>(&nPort), &pBuf, &nLen)) {
            sub_4D15D0(nAddr, nPort, pBuf, nLen);

            if (nLen > 0) {
                delete[] pBuf;
            }
        }

        LeaveCriticalSection(&(g_pChitin->cNetwork.field_F6A));
        return TRUE;
    }

    return FALSE;
}

// 0x4D2310
BOOLEAN CGameSpy::sub_4D2310()
{
    if (!field_1) {
        return FALSE;
    }

    closesocket(field_A);
    closesocket(field_E);

    if (field_0 == 1) {
        WSACleanup();
        field_0 = 0;
    }

    return TRUE;
}