#ifndef WIN32SHIM_WINMM_H_
#define WIN32SHIM_WINMM_H_
#include <win32shim/wintypes.h>

// Multimedia: wave formats, multimedia timers.

#pragma pack(push, 1)
typedef struct waveformat_tag {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
} WAVEFORMAT, *PWAVEFORMAT, *LPWAVEFORMAT;

typedef struct tWAVEFORMATEX {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;
typedef const WAVEFORMATEX* LPCWAVEFORMATEX;
#pragma pack(pop)

#define WAVE_FORMAT_PCM 1

// The game calls the A name; `PlaySound` is a macro for it in the real header.
#define PlaySound PlaySoundA

#define TIMERR_NOERROR 0
#define TIME_ONESHOT 0x0000
#define TIME_PERIODIC 0x0001
#define TIME_CALLBACK_FUNCTION 0x0000

typedef struct timecaps_tag {
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS, *PTIMECAPS, *LPTIMECAPS;

typedef void(CALLBACK TIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
typedef TIMECALLBACK* LPTIMECALLBACK;
typedef UINT MMRESULT;

DWORD timeGetTime();
MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
MMRESULT timeBeginPeriod(UINT uPeriod);
MMRESULT timeEndPeriod(UINT uPeriod);
MMRESULT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK fptc, DWORD_PTR dwUser, UINT fuEvent);
MMRESULT timeKillEvent(UINT uTimerID);

#endif /* WIN32SHIM_WINMM_H_ */
