#ifndef WIN32SHIM_WINSOCK_H_
#define WIN32SHIM_WINSOCK_H_
#include <win32shim/wintypes.h>

// Winsock subset used by CGameSpy. Layouts are the Windows ones, not the BSD
// ones. u_short comes from <sys/types.h>.
#include <sys/types.h>

// macOS defines these as macros in <sys/_endian.h>.
#undef htons
#undef ntohs
#undef htonl
#undef ntohl

typedef UINT_PTR SOCKET;

struct in_addr {
    union {
        struct {
            BYTE s_b1, s_b2, s_b3, s_b4;
        } S_un_b;
        struct {
            u_short s_w1, s_w2;
        } S_un_w;
        uint32_t S_addr;
    } S_un;
};
#define s_addr S_un.S_addr

struct sockaddr {
    u_short sa_family;
    char sa_data[14];
};

struct sockaddr_in {
    short sin_family;
    u_short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};

struct hostent {
    char* h_name;
    char** h_aliases;
    short h_addrtype;
    short h_length;
    char** h_addr_list;
};

#define WSADESCRIPTION_LEN 256
#define WSASYS_STATUS_LEN 128
typedef struct WSAData {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[WSADESCRIPTION_LEN + 1];
    char szSystemStatus[WSASYS_STATUS_LEN + 1];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
} WSADATA, *LPWSADATA;

#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17
#define INVALID_SOCKET ((SOCKET)(~0))
#define SOCKET_ERROR (-1)
#define FIONBIO 0x8004667E
#define MSG_PEEK 0x2
#define INADDR_NONE 0xffffffff
#define WSAEWOULDBLOCK 10035
#define FD_CONNECT 0x10

int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
int WSACleanup();
int WSAGetLastError();
HANDLE WSAAsyncGetHostByName(HWND hWnd, UINT wMsg, const char* name, char* buf, int buflen);
int WSACancelAsyncRequest(HANDLE hAsyncTaskHandle);
int WSAAsyncSelect(SOCKET s, HWND hWnd, UINT wMsg, LONG lEvent);

SOCKET socket(int af, int type, int protocol);
int closesocket(SOCKET s);
int ioctlsocket(SOCKET s, LONG cmd, u_long* argp);
int connect(SOCKET s, const struct sockaddr* name, int namelen);
int bind(SOCKET s, const struct sockaddr* name, int namelen);
int recvfrom(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen);
int sendto(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen);
uint32_t inet_addr(const char* cp);
u_short htons(u_short hostshort);
u_short ntohs(u_short netshort);

#endif /* WIN32SHIM_WINSOCK_H_ */
