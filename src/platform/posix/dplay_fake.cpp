// A DirectPlay that offers no connections, for non-Windows builds.
//
// The game creates a DirectPlay object and a lobby object even for a
// single-player game, to list the service providers; it then adds its own
// "no network" provider (number 0) and never calls DirectPlay for it. These
// objects exist, list nothing, and refuse everything else, so the only
// provider is the single-player one. Multiplayer is not available.
//
// The method lists are generated from third_party/directplay/include.

#include <windows.h>

#include <dplay.h>
#include <dplobby.h>

#include <string.h>

namespace {

struct FakeDirectPlay final : IDirectPlay4 {
    ULONG m_refs = 1;

    HRESULT QueryInterface(REFIID riid, LPVOID * ppvObj) override { AddRef(); *ppvObj = this; return S_OK; }
    ULONG AddRef() override { return ++m_refs; }
    ULONG Release() override { ULONG refs = --m_refs; if (refs == 0) { delete this; } return refs; }
    HRESULT AddPlayerToGroup(DPID, DPID) override { return DPERR_UNSUPPORTED; }
    HRESULT Close() override { return DP_OK; }
    HRESULT CreateGroup(LPDPID,LPDPNAME,LPVOID,DWORD,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT CreatePlayer(LPDPID,LPDPNAME,HANDLE,LPVOID,DWORD,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT DeletePlayerFromGroup(DPID,DPID) override { return DPERR_UNSUPPORTED; }
    HRESULT DestroyGroup(DPID) override { return DPERR_UNSUPPORTED; }
    HRESULT DestroyPlayer(DPID) override { return DPERR_UNSUPPORTED; }
    HRESULT EnumGroupPlayers(DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT EnumGroups(LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT EnumPlayers(LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT EnumSessions(LPDPSESSIONDESC2,DWORD,LPDPENUMSESSIONSCALLBACK2,LPVOID,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetCaps(LPDPCAPS,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetGroupData(DPID,LPVOID,LPDWORD,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetGroupName(DPID,LPVOID,LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetMessageCount(DPID, LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetPlayerAddress(DPID,LPVOID,LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetPlayerCaps(DPID,LPDPCAPS,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetPlayerData(DPID,LPVOID,LPDWORD,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetPlayerName(DPID,LPVOID,LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetSessionDesc(LPVOID,LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT Initialize(LPGUID) override { return DPERR_UNSUPPORTED; }
    HRESULT Open(LPDPSESSIONDESC2,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT Receive(LPDPID,LPDPID,DWORD,LPVOID,LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT Send(DPID, DPID, DWORD, LPVOID, DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT SetGroupData(DPID,LPVOID,DWORD,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT SetGroupName(DPID,LPDPNAME,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT SetPlayerData(DPID,LPVOID,DWORD,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT SetPlayerName(DPID,LPDPNAME,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT SetSessionDesc(LPDPSESSIONDESC2,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT AddGroupToGroup(DPID, DPID) override { return DPERR_UNSUPPORTED; }
    HRESULT CreateGroupInGroup(DPID,LPDPID,LPDPNAME,LPVOID,DWORD,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT DeleteGroupFromGroup(DPID,DPID) override { return DPERR_UNSUPPORTED; }
    HRESULT EnumConnections(LPCGUID,LPDPENUMCONNECTIONSCALLBACK,LPVOID,DWORD) override { return DP_OK; }
    HRESULT EnumGroupsInGroup(DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetGroupConnectionSettings(DWORD, DPID, LPVOID, LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT InitializeConnection(LPVOID,DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT SecureOpen(LPCDPSESSIONDESC2,DWORD,LPCDPSECURITYDESC,LPCDPCREDENTIALS) override { return DPERR_UNSUPPORTED; }
    HRESULT SendChatMessage(DPID,DPID,DWORD,LPDPCHAT) override { return DPERR_UNSUPPORTED; }
    HRESULT SetGroupConnectionSettings(DWORD,DPID,LPDPLCONNECTION) override { return DPERR_UNSUPPORTED; }
    HRESULT StartSession(DWORD,DPID) override { return DPERR_UNSUPPORTED; }
    HRESULT GetGroupFlags(DPID,LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetGroupParent(DPID,LPDPID) override { return DPERR_UNSUPPORTED; }
    HRESULT GetPlayerAccount(DPID, DWORD, LPVOID, LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetPlayerFlags(DPID,LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetGroupOwner(DPID, LPDPID) override { return DPERR_UNSUPPORTED; }
    HRESULT SetGroupOwner(DPID, DPID) override { return DPERR_UNSUPPORTED; }
    HRESULT SendEx(DPID, DPID, DWORD, LPVOID, DWORD, DWORD, DWORD, LPVOID, DWORD_PTR *) override { return DPERR_UNSUPPORTED; }
    HRESULT GetMessageQueue(DPID, DPID, DWORD, LPDWORD, LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT CancelMessage(DWORD, DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT CancelPriority(DWORD, DWORD, DWORD) override { return DPERR_UNSUPPORTED; }
};

struct FakeDirectPlayLobby final : IDirectPlayLobby3 {
    ULONG m_refs = 1;

    HRESULT QueryInterface(REFIID riid, LPVOID * ppvObj) override { AddRef(); *ppvObj = this; return S_OK; }
    ULONG AddRef() override { return ++m_refs; }
    ULONG Release() override { ULONG refs = --m_refs; if (refs == 0) { delete this; } return refs; }
    HRESULT Connect(DWORD, LPDIRECTPLAY2 *, IUnknown FAR *) override { return DPERR_UNSUPPORTED; }
    HRESULT CreateAddress(REFGUID, REFGUID, LPCVOID, DWORD, LPVOID, LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT EnumAddress(LPDPENUMADDRESSCALLBACK, LPCVOID, DWORD, LPVOID) override { return DPERR_UNSUPPORTED; }
    HRESULT EnumAddressTypes(LPDPLENUMADDRESSTYPESCALLBACK, REFGUID, LPVOID, DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT EnumLocalApplications(LPDPLENUMLOCALAPPLICATIONSCALLBACK, LPVOID, DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT GetConnectionSettings(DWORD, LPVOID, LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT ReceiveLobbyMessage(DWORD, DWORD, LPDWORD, LPVOID, LPDWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT RunApplication(DWORD, LPDWORD, LPDPLCONNECTION, HANDLE) override { return DPERR_UNSUPPORTED; }
    HRESULT SendLobbyMessage(DWORD, DWORD, LPVOID, DWORD) override { return DPERR_UNSUPPORTED; }
    HRESULT SetConnectionSettings(DWORD, DWORD, LPDPLCONNECTION) override { return DPERR_UNSUPPORTED; }
    HRESULT SetLobbyMessageEvent(DWORD, DWORD, HANDLE) override { return DPERR_UNSUPPORTED; }
    // The game builds an address even for its "no network" provider, asking
    // for the size first. The address is never used for anything.
    HRESULT CreateCompoundAddress(LPCDPCOMPOUNDADDRESSELEMENT, DWORD, LPVOID lpAddress, LPDWORD lpdwAddressSize) override
    {
        const DWORD size = 64;
        if (lpAddress == NULL || *lpdwAddressSize < size) {
            *lpdwAddressSize = size;
            return DPERR_BUFFERTOOSMALL;
        }
        memset(lpAddress, 0, size);
        *lpdwAddressSize = size;
        return DP_OK;
    }
    HRESULT ConnectEx(DWORD, REFIID, LPVOID *, IUnknown FAR *) override { return DPERR_UNSUPPORTED; }
    HRESULT RegisterApplication(DWORD, LPVOID) override { return DPERR_UNSUPPORTED; }
    HRESULT UnregisterApplication(DWORD, REFGUID) override { return DPERR_UNSUPPORTED; }
    HRESULT WaitForConnectionSettings(DWORD) override { return DPERR_UNSUPPORTED; }
};

} // namespace

namespace platform {

void* CreateFakeDirectPlay()
{
    return static_cast<IDirectPlay4*>(new FakeDirectPlay());
}

HRESULT WINAPI FakeDirectPlayCreate(LPGUID, LPDIRECTPLAY* lplpDP, IUnknown*)
{
    *lplpDP = NULL;
    return DPERR_UNAVAILABLE;
}

HRESULT WINAPI FakeDirectPlayLobbyCreateA(LPGUID, LPDIRECTPLAYLOBBYA* lplpDPL, IUnknown*, LPVOID, DWORD)
{
    *lplpDPL = static_cast<IDirectPlayLobby*>(new FakeDirectPlayLobby());
    return DP_OK;
}

void* FakeDirectPlayCreateAddress()
{
    return reinterpret_cast<void*>(&FakeDirectPlayCreate);
}

void* FakeDirectPlayLobbyCreateAddress()
{
    return reinterpret_cast<void*>(&FakeDirectPlayLobbyCreateA);
}

} // namespace platform
