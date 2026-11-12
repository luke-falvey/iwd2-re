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

// 0x4D0C70
BOOLEAN CGameSpy::sub_4D0C70(DWORD nAddr, DWORD nPort)
{
    CString sQueryId;

    if (field_22 == NULL) {
        return FALSE;
    }

    field_1E++;
    sQueryId.Format("\\queryid\\%d.%d\n", field_1A, field_1E);

    int nLength = sQueryId.GetLength();
    memcpy(field_22 + field_16, sQueryId.GetBuffer(nLength), nLength);
    field_16 += sQueryId.GetLength();

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = nAddr;
    addr.sin_port = htons(static_cast<u_short>(nPort));
    sendto(field_E, field_22, field_16, 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    memset(field_22, 0, 0x578);
    field_16 = 0;

    return TRUE;
}

// 0x4D0DA0
BOOLEAN CGameSpy::sub_4D0DA0(const char* pBuf, DWORD nLen, CString sKey, char** ppValue, DWORD* pValueLen)
{
    CString sSearch;

    *ppValue = NULL;
    *pValueLen = 0;

    sSearch = "\\" + sKey;
    sSearch += "\\";

    CString sBuffer(pBuf, nLen);

    int nStart = sBuffer.Find(sSearch);
    if (nStart == -1) {
        return FALSE;
    }

    nStart += sSearch.GetLength();

    BYTE nEnd = static_cast<BYTE>(nStart);
    while (nEnd < nLen && pBuf[nEnd] != '\\') {
        nEnd++;
    }

    DWORD nValueLen = nEnd - nStart;
    *pValueLen = nValueLen;

    if (nValueLen != 0) {
        *ppValue = new char[nValueLen];
        if (*ppValue == NULL) {
            *pValueLen = 0;
            return FALSE;
        }

        memcpy(*ppValue, pBuf + nStart, nValueLen);
    }

    return TRUE;
}

// 0x4D0F40
BOOLEAN CGameSpy::sub_4D0F40(DWORD nAddr, DWORD nPort, char* pSecure)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4D1220
BOOLEAN CGameSpy::sub_4D1220(DWORD nAddr, DWORD nPort, char* pSecure)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4D15D0
BOOLEAN CGameSpy::sub_4D15D0(DWORD nAddr, DWORD nPort, char* pBuf, DWORD nLen)
{
    char* pSecure = NULL;
    char* pValue;
    DWORD nValueLen;
    BYTE* pResponse;
    DWORD nResponseLen;
    DWORD nCount;
    BYTE* pData;

    field_1A++;
    if (field_1A >= 100) {
        field_1A = 0;
    }

    field_1E = 0;

    field_22 = new char[0x578];
    if (field_22 == NULL) {
        return FALSE;
    }

    field_16 = 0;

    if (sub_4D0DA0(pBuf, nLen, "secure", &pValue, &nValueLen) == TRUE && pValue != NULL) {
        pSecure = new char[6];
        if (pSecure == NULL) {
            delete[] field_22;
            return FALSE;
        }

        memcpy(pSecure, pValue, 6);

        if (pValue != NULL && nValueLen != 0) {
            delete[] pValue;
        }
    }

    if (sub_4D0DA0(pBuf, nLen, "basic", &pValue, &nValueLen) == TRUE) {
        if (g_pChitin->GetGameSpyResponse(0, &pResponse, &nResponseLen) == TRUE) {
            nCount = nResponseLen;
            pData = pResponse;
            if (field_22 != NULL) {
                if (field_16 + nCount >= 0x546) {
                    sub_4D0C70(nAddr, nPort);
                }

                memcpy(field_22 + field_16, pData, nCount);
                field_16 += nCount;
            }

            if (nCount != 0) {
                delete[] pData;
            }
        }
        if (pValue != NULL && nValueLen != 0) {
            delete[] pValue;
        }
    }

    if (sub_4D0DA0(pBuf, nLen, "info", &pValue, &nValueLen) == TRUE) {
        if (field_16 != 0) {
            sub_4D0F40(nAddr, nPort, pSecure);
        }
        if (g_pChitin->GetGameSpyResponse(1, &pResponse, &nResponseLen) == TRUE) {
            nCount = nResponseLen;
            pData = pResponse;
            if (field_22 != NULL) {
                if (field_16 + nCount >= 0x546) {
                    sub_4D0C70(nAddr, nPort);
                }

                memcpy(field_22 + field_16, pData, nCount);
                field_16 += nCount;
            }

            if (nCount != 0) {
                delete[] pData;
            }
        }
        if (pValue != NULL && nValueLen != 0) {
            delete[] pValue;
        }
    }

    if (sub_4D0DA0(pBuf, nLen, "rules", &pValue, &nValueLen) == TRUE) {
        if (field_16 != 0) {
            sub_4D0F40(nAddr, nPort, pSecure);
        }
        if (g_pChitin->GetGameSpyResponse(2, &pResponse, &nResponseLen) == TRUE) {
            nCount = nResponseLen;
            pData = pResponse;
            if (field_22 != NULL) {
                if (field_16 + nCount >= 0x546) {
                    sub_4D0C70(nAddr, nPort);
                }

                memcpy(field_22 + field_16, pData, nCount);
                field_16 += nCount;
            }

            if (nCount != 0) {
                delete[] pData;
            }
        }
        if (pValue != NULL && nValueLen != 0) {
            delete[] pValue;
        }
    }

    if (sub_4D0DA0(pBuf, nLen, "players", &pValue, &nValueLen) == TRUE) {
        if (field_16 != 0) {
            sub_4D0F40(nAddr, nPort, pSecure);
        }
        if (g_pChitin->GetGameSpyResponse(3, &pResponse, &nResponseLen) == TRUE) {
            nCount = nResponseLen;
            pData = pResponse;
            if (field_22 != NULL) {
                if (field_16 + nCount >= 0x546) {
                    sub_4D0C70(nAddr, nPort);
                }

                memcpy(field_22 + field_16, pData, nCount);
                field_16 += nCount;
            }

            if (nCount != 0) {
                delete[] pData;
            }
        }
        if (pValue != NULL && nValueLen != 0) {
            delete[] pValue;
        }
    }

    if (sub_4D0DA0(pBuf, nLen, "status", &pValue, &nValueLen) == TRUE) {
        if (field_16 != 0) {
            sub_4D0F40(nAddr, nPort, pSecure);
        }
        if (g_pChitin->GetGameSpyResponse(0, &pResponse, &nResponseLen) == TRUE) {
            nCount = nResponseLen;
            pData = pResponse;
            if (field_22 != NULL) {
                if (field_16 + nCount >= 0x546) {
                    sub_4D0C70(nAddr, nPort);
                }

                memcpy(field_22 + field_16, pData, nCount);
                field_16 += nCount;
            }

            if (nCount != 0) {
                delete[] pData;
            }
        }
        if (field_16 != 0) {
            sub_4D0F40(nAddr, nPort, pSecure);
        }
        if (g_pChitin->GetGameSpyResponse(1, &pResponse, &nResponseLen) == TRUE) {
            nCount = nResponseLen;
            pData = pResponse;
            if (field_22 != NULL) {
                if (field_16 + nCount >= 0x546) {
                    sub_4D0C70(nAddr, nPort);
                }

                memcpy(field_22 + field_16, pData, nCount);
                field_16 += nCount;
            }

            if (nCount != 0) {
                delete[] pData;
            }
        }
        if (field_16 != 0) {
            sub_4D0F40(nAddr, nPort, pSecure);
        }
        if (g_pChitin->GetGameSpyResponse(2, &pResponse, &nResponseLen) == TRUE) {
            nCount = nResponseLen;
            pData = pResponse;
            if (field_22 != NULL) {
                if (field_16 + nCount >= 0x546) {
                    sub_4D0C70(nAddr, nPort);
                }

                memcpy(field_22 + field_16, pData, nCount);
                field_16 += nCount;
            }

            if (nCount != 0) {
                delete[] pData;
            }
        }
        if (field_16 != 0) {
            sub_4D0F40(nAddr, nPort, pSecure);
        }
        if (g_pChitin->GetGameSpyResponse(3, &pResponse, &nResponseLen) == TRUE) {
            nCount = nResponseLen;
            pData = pResponse;
            if (field_22 != NULL) {
                if (field_16 + nCount >= 0x546) {
                    sub_4D0C70(nAddr, nPort);
                }

                memcpy(field_22 + field_16, pData, nCount);
                field_16 += nCount;
            }

            if (nCount != 0) {
                delete[] pData;
            }
        }
        if (pValue != NULL && nValueLen != 0) {
            delete[] pValue;
        }
    }

    if (sub_4D0DA0(pBuf, nLen, "echo", &pValue, &nValueLen) == TRUE) {
        if (field_16 != 0) {
            sub_4D0F40(nAddr, nPort, pSecure);
        }

        CString sEcho("\\echo\\");

        if (nValueLen >= 0x546) {
            delete[] field_22;
            return FALSE;
        }

        nResponseLen = sEcho.GetLength() + nValueLen;
        pResponse = new BYTE[nResponseLen];
        if (pResponse == NULL) {
            delete[] field_22;
            return FALSE;
        }

        memcpy(pResponse, sEcho.GetBuffer(sEcho.GetLength()), sEcho.GetLength());
        memcpy(pResponse + sEcho.GetLength(), pValue, nValueLen);

        nCount = nResponseLen;
        pData = pResponse;
        if (field_22 != NULL) {
            if (field_16 + nCount >= 0x546) {
                sub_4D0C70(nAddr, nPort);
            }

            memcpy(field_22 + field_16, pData, nCount);
            field_16 += nCount;
        }

        delete[] pResponse;

        if (pValue != NULL && nValueLen != 0) {
            delete[] pValue;
        }
    }

    if (field_16 != 0) {
        sub_4D1220(nAddr, nPort, pSecure);
    }

    if (pSecure != NULL) {
        delete[] pSecure;
    }

    delete[] field_22;
    return TRUE;
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