#ifndef WIN32SHIM_DSOUND_DECL_H_
#define WIN32SHIM_DSOUND_DECL_H_
#include <win32shim/wintypes.h>
#include <win32shim/com.h>
#include <win32shim/winmm.h>

// DirectSound (DirectX 7 era): types, constants, interfaces.

#define MAKE_DSHRESULT(code) MAKE_HRESULT(1, 0x878, code)

#define DS_OK 0
#define DSERR_ALLOCATED MAKE_DSHRESULT(10)
#define DSERR_INVALIDPARAM E_INVALIDARG
#define DSERR_BUFFERLOST MAKE_DSHRESULT(150)
#define DSERR_INVALIDCALL MAKE_DSHRESULT(50)

typedef float D3DVALUE, *LPD3DVALUE;

struct IDirectSound;
struct IDirectSoundBuffer;
struct IDirectSound3DBuffer;
struct IDirectSound3DListener;
struct IKsPropertySet;
typedef IDirectSound* LPDIRECTSOUND;
typedef IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;
typedef IDirectSoundBuffer** LPLPDIRECTSOUNDBUFFER;
typedef IDirectSound3DBuffer* LPDIRECTSOUND3DBUFFER;
typedef IDirectSound3DListener* LPDIRECTSOUND3DLISTENER;
typedef IKsPropertySet* LPKSPROPERTYSET;

typedef struct _DSBUFFERDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    GUID guid3DAlgorithm;
} DSBUFFERDESC, *LPDSBUFFERDESC;
typedef const DSBUFFERDESC* LPCDSBUFFERDESC;

typedef struct _DSCAPS {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwMinSecondarySampleRate;
    DWORD dwMaxSecondarySampleRate;
    DWORD dwPrimaryBuffers;
    DWORD dwMaxHwMixingAllBuffers;
    DWORD dwMaxHwMixingStaticBuffers;
    DWORD dwMaxHwMixingStreamingBuffers;
    DWORD dwFreeHwMixingAllBuffers;
    DWORD dwFreeHwMixingStaticBuffers;
    DWORD dwFreeHwMixingStreamingBuffers;
    DWORD dwMaxHw3DAllBuffers;
    DWORD dwMaxHw3DStaticBuffers;
    DWORD dwMaxHw3DStreamingBuffers;
    DWORD dwFreeHw3DAllBuffers;
    DWORD dwFreeHw3DStaticBuffers;
    DWORD dwFreeHw3DStreamingBuffers;
    DWORD dwTotalHwMemBytes;
    DWORD dwFreeHwMemBytes;
    DWORD dwMaxContigFreeHwMemBytes;
    DWORD dwUnlockTransferRateHwBuffers;
    DWORD dwPlayCpuOverheadSwBuffers;
    DWORD dwReserved1;
    DWORD dwReserved2;
} DSCAPS, *LPDSCAPS;

#define DSCAPS_PRIMARYMONO 0x00000001
#define DSCAPS_PRIMARY8BIT 0x00000004
#define DSCAPS_CONTINUOUSRATE 0x00000010
#define DSCAPS_EMULDRIVER 0x00000020
#define DSCAPS_PRIMARYSTEREO 0x00000002
#define DSCAPS_PRIMARY16BIT 0x00000008

#define DSSCL_EXCLUSIVE 0x00000003

#define DSBCAPS_PRIMARYBUFFER 0x00000001
#define DSBCAPS_STATIC 0x00000002
#define DSBCAPS_CTRL3D 0x00000010
#define DSBCAPS_CTRLFREQUENCY 0x00000020
#define DSBCAPS_CTRLPAN 0x00000040
#define DSBCAPS_CTRLVOLUME 0x00000080
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000

#define DSBPLAY_LOOPING 0x00000001

#define DSBSTATUS_PLAYING 0x00000001
#define DSBSTATUS_BUFFERLOST 0x00000002
#define DSBSTATUS_LOOPING 0x00000004

#define DSBLOCK_FROMWRITECURSOR 0x00000001
#define DSBLOCK_ENTIREBUFFER 0x00000002

#define DS3D_IMMEDIATE 0x00000000

#define KSPROPERTY_SUPPORT_GET 0x00000001
#define KSPROPERTY_SUPPORT_SET 0x00000002

DEFINE_GUID(IID_IDirectSound3DBuffer, 0x279AFA86, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IDirectSound3DListener, 0x279AFA84, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IKsPropertySet, 0x31EFAC30, 0x515C, 0x11D0, 0xA9, 0xAA, 0x00, 0xAA, 0x00, 0x61, 0xBE, 0x93);

interface IKsPropertySet : public IUnknown {
    STDMETHOD(Get)(REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData, ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned) PURE;
    STDMETHOD(Set)(REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData, ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength) PURE;
    STDMETHOD(QuerySupport)(REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport) PURE;
};

interface IDirectSound3DBuffer : public IUnknown {
    STDMETHOD(SetMinDistance)(D3DVALUE flMinDistance, DWORD dwApply) PURE;
    STDMETHOD(SetMode)(DWORD dwMode, DWORD dwApply) PURE;
    STDMETHOD(SetPosition)(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) PURE;
};

interface IDirectSound3DListener : public IUnknown {
    STDMETHOD(SetOrientation)(D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply) PURE;
    STDMETHOD(SetPosition)(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) PURE;
};

interface IDirectSoundBuffer : public IUnknown {
    STDMETHOD(GetCurrentPosition)(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) PURE;
    STDMETHOD(GetStatus)(LPDWORD lpdwStatus) PURE;
    STDMETHOD(GetVolume)(LPLONG plVolume) PURE;
    STDMETHOD(Lock)(DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) PURE;
    STDMETHOD(Restore)() PURE;
    STDMETHOD(Play)(DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags) PURE;
    STDMETHOD(SetCurrentPosition)(DWORD dwNewPosition) PURE;
    STDMETHOD(SetFormat)(LPCWAVEFORMATEX pcfxFormat) PURE;
    STDMETHOD(SetFrequency)(DWORD dwFrequency) PURE;
    STDMETHOD(SetPan)(LONG lPan) PURE;
    STDMETHOD(SetVolume)(LONG lVolume) PURE;
    STDMETHOD(Stop)() PURE;
    STDMETHOD(Unlock)(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) PURE;
};

interface IDirectSound : public IUnknown {
    STDMETHOD(CreateSoundBuffer)(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDSBuffer, LPUNKNOWN pUnkOuter) PURE;
    STDMETHOD(GetCaps)(LPDSCAPS pDSCaps) PURE;
    STDMETHOD(SetCooperativeLevel)(HWND hwnd, DWORD dwLevel) PURE;
};

HRESULT WINAPI DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);

// C-style helpers of dsound.h used by the game.
#define IDirectSoundBuffer_SetCurrentPosition(p, a) (p)->SetCurrentPosition(a)
#define IDirectSoundBuffer_SetFrequency(p, a) (p)->SetFrequency(a)
#define IDirectSoundBuffer_SetPan(p, a) (p)->SetPan(a)
#define IDirectSoundBuffer_SetVolume(p, a) (p)->SetVolume(a)
#define IDirectSoundBuffer_Play(p, a, b, c) (p)->Play(a, b, c)
#define IDirectSoundBuffer_Stop(p) (p)->Stop()
#define IDirectSoundBuffer_Release(p) (p)->Release()
#define IDirectSoundBuffer_Restore(p) (p)->Restore()
#define IDirectSoundBuffer_GetCurrentPosition(p, a, b) (p)->GetCurrentPosition(a, b)
#define IDirectSoundBuffer_GetStatus(p, a) (p)->GetStatus(a)
#define IDirectSoundBuffer_GetVolume(p, a) (p)->GetVolume(a)
#define IDirectSoundBuffer_Lock(p, a, b, c, d, e, f, g) (p)->Lock(a, b, c, d, e, f, g)
#define IDirectSoundBuffer_Unlock(p, a, b, c, d) (p)->Unlock(a, b, c, d)
#define IDirectSoundBuffer_SetFormat(p, a) (p)->SetFormat(a)
#define IDirectSound_Release(p) (p)->Release()
#define IDirectSound_CreateSoundBuffer(p, a, b, c) (p)->CreateSoundBuffer(a, b, c)
#define IDirectSound_SetCooperativeLevel(p, a, b) (p)->SetCooperativeLevel(a, b)
#define IDirectSound_GetCaps(p, a) (p)->GetCaps(a)

#endif /* WIN32SHIM_DSOUND_DECL_H_ */
