// DirectSound in software, mixed into an SDL3 audio stream.
//
// Behaviour the game relies on (read from CSoundMixer.cpp, CSound.cpp, CSoundProperties.cpp,
// CVoice.cpp, music/sound.cpp):
//  - DirectSoundCreate is called twice over a run (mixer, and the music streamer when it is
//    not handed the mixer's object); one ref-counted device object serves both.
//  - CreateSoundBuffer: the primary buffer (DSBCAPS_PRIMARYBUFFER, no format, 0 bytes) only
//    gets SetFormat (22050 Hz, 16 bit stereo from GetCaps); it makes no sound of its own and
//    does not change the mix. Secondary buffers are 8/16 bit PCM, mono/stereo, any rate, with
//    flags ignored (CTRLVOLUME/PAN/FREQUENCY are always available).
//  - Streaming music: one looping buffer, written with Lock/Unlock at offsets chased from
//    GetCurrentPosition's play cursor, so the play cursor must advance in real time and wrap.
//    Lock may return two spans; ptr2 is NULL when the second span is empty (callers test it).
//    Lock(.., DSBLOCK_FROMWRITECURSOR) ignores the offset and starts at the write cursor; a
//    stopped buffer has write cursor == play cursor (static sounds are filled at 0).
//  - SetCurrentPosition(0) before Play, Stop keeps the position (pause), a non-looping buffer
//    that ends stops with GetStatus == 0 (CVoice::IsSoundPlaying, soundGetStatus) and position 0.
//  - SetVolume(30 * (v - 100)) is hundredths of dB, 0..-10000; -10000 is silent
//    (soundGetVolume tests == -10000). GetVolume returns what was set. SetPan(50 * pan), SetFrequency(500 * f).
//  - Every method may be called from several threads (game thread, music thread, SDL
//    callback); one global mutex guards all state.
//  - QueryInterface for IDirectSound3DBuffer / IDirectSound3DListener / IKsPropertySet fails with
//    E_NOINTERFACE: CSoundProperties::Init returns 0 (no EAX), the mixer never fetches the listener,
//    CSound::Create3DBuffer takes the "no 3D buffer" branch (volume from range, m_b3DPositioning
//    FALSE) and SetBufferReverbMix is a no-op. All sites cope.
//  - GetCaps: stereo, 16 bit, emulated driver, continuous rate 100..200000, no hardware voices and
//    no 3D voices. Only DSCAPS_PRIMARY16BIT and DSCAPS_PRIMARYSTEREO are read by the game; the
//    game then takes the plain software-mixing path (CSound::Create3DBuffer fails QI and uses
//    2D volume emulation of position).
//
// Output: 44100 Hz stereo, mixed in float, clamped to S16. Resampling is linear interpolation.
// Volume gain 10^(v/2000) (exactly 0 at -10000); pan attenuates the opposite channel the same way.
//
// Environment:
//   IWD2RE_SOUND=off      no audio device; buffer positions advance from a clock instead.
//   IWD2RE_SOUND_LOG=file one line per API call, written as
//       <ms> <id> <Method> <args>[ => <results>][ FAILED 0x<hresult>]
//     ms since the first DirectSoundCreate; id is the buffer's number (1, 2, ... in creation
//     order) or 0 for the device. PCM data is never logged; Unlock logs byte counts. Results
//     (after "=>") are positions and spans, which depend on timing: strip them to compare builds.
//     Lines: DirectSoundCreate; SetCooperativeLevel level=; GetCaps; CreateSoundBuffer flags= bytes=
//     rate= bits= ch= => id=; and per buffer Release, Play flags=, Stop, SetVolume v=, GetVolume,
//     SetPan v=, SetFrequency hz=, SetCurrentPosition pos=, GetCurrentPosition => play= write=,
//     GetStatus => status=, Lock off= bytes= flags= => n1= n2=, Unlock n1= n2=, Restore, SetFormat.

#include <win32shim/dsound_decl.h>

#include "sound_mixer.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <math.h>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace {

const int kOutRate = 44100;
const int kWriteAheadMs = 15;

std::mutex& Mx()
{
    static std::mutex m;
    return m;
}

typedef std::chrono::steady_clock Clock;
Clock::time_point g_start = Clock::now();

FILE* g_log = nullptr;
bool g_initDone = false;
bool g_deviceOpen = false;
bool g_manualClock = false;
SDL_AudioStream* g_stream = nullptr;
int64_t g_clockFramesDone = 0;
Clock::time_point g_clockStart;

bool SameGuid(const GUID& a, const GUID& b) { return memcmp(&a, &b, sizeof(GUID)) == 0; }

class SoftBuffer;
std::vector<SoftBuffer*> g_buffers;
int g_nextId = 1;

float GainFromDb(long hundredths)
{
    if (hundredths <= -10000) return 0.0f;
    if (hundredths >= 0) return 1.0f;
    return powf(10.0f, static_cast<float>(hundredths) / 2000.0f);
}

long NowMs()
{
    return static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - g_start).count());
}

// One log line per API call, written when the call object goes out of scope (the caller holds
// the mutex then). Does nothing unless IWD2RE_SOUND_LOG is set.
struct Call {
    int id;
    const char* method;
    char args[160];
    char res[96];
    long hr;
    Call(int id_, const char* m, const char* fmt = "", ...) : id(id_), method(m), hr(0)
    {
        args[0] = 0;
        res[0] = 0;
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(args, sizeof(args), fmt, ap);
        va_end(ap);
    }
    void R(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(res, sizeof(res), fmt, ap);
        va_end(ap);
    }
    HRESULT Ret(HRESULT h)
    {
        hr = static_cast<long>(static_cast<uint32_t>(h));
        return h;
    }
    ~Call()
    {
        if (g_log == nullptr) return;
        fprintf(g_log, "%ld %d %s%s%s", NowMs(), id, method, args[0] ? " " : "", args);
        if (res[0]) fprintf(g_log, " => %s", res);
        if (hr) fprintf(g_log, " FAILED 0x%08lx", hr);
        fputc('\n', g_log);
        fflush(g_log);
    }
};

void CatchUpClock();

class SoftBuffer : public IDirectSoundBuffer {
public:
    int id;
    ULONG refs;
    bool primary;
    int channels, bits, rate;
    int block;
    std::vector<unsigned char> data;
    int frames;
    double pos; // in source frames
    bool playing, looping;
    long volume, pan;
    DWORD frequency; // 0 = format rate

    SoftBuffer()
        : id(0), refs(1), primary(false), channels(2), bits(16), rate(22050), block(4), frames(0), pos(0),
          playing(false), looping(false), volume(0), pan(0), frequency(0)
    {
    }

    int CurRate() const { return frequency ? static_cast<int>(frequency) : rate; }

    // IUnknown
    HRESULT QueryInterface(REFIID riid, LPVOID* ppv) override
    {
        if (ppv == nullptr) return E_POINTER;
        *ppv = nullptr;
        std::lock_guard<std::mutex> g(Mx());
        if (SameGuid(riid, IID_IUnknown)) {
            refs++;
            *ppv = static_cast<IDirectSoundBuffer*>(this);
            return S_OK;
        }
        Call c(id, "QueryInterface", "iid=%08x", static_cast<unsigned>(riid.Data1));
        return c.Ret(E_NOINTERFACE);
    }
    ULONG AddRef() override
    {
        std::lock_guard<std::mutex> g(Mx());
        return ++refs;
    }
    ULONG Release() override
    {
        std::unique_lock<std::mutex> g(Mx());
        { Call c(id, "Release"); }
        if (--refs > 0) return refs;
        g_buffers.erase(std::remove(g_buffers.begin(), g_buffers.end(), this), g_buffers.end());
        g.unlock();
        delete this;
        return 0;
    }

    HRESULT GetCurrentPosition(LPDWORD pPlay, LPDWORD pWrite) override
    {
        std::lock_guard<std::mutex> g(Mx());
        CatchUpClock();
        DWORD play = PlayCursor();
        DWORD write = play;
        if (playing && !primary && !data.empty()) {
            long ahead = static_cast<long>(static_cast<int64_t>(CurRate()) * kWriteAheadMs / 1000) * block;
            write = static_cast<DWORD>((play + ahead) % data.size());
        }
        if (pPlay) *pPlay = play;
        if (pWrite) *pWrite = write;
        Call c(id, "GetCurrentPosition");
        c.R("play=%u write=%u", play, write);
        return S_OK;
    }
    HRESULT GetStatus(LPDWORD pStatus) override
    {
        if (!pStatus) return E_POINTER;
        std::lock_guard<std::mutex> g(Mx());
        CatchUpClock();
        *pStatus = playing ? (DSBSTATUS_PLAYING | (looping ? DSBSTATUS_LOOPING : 0)) : 0;
        Call c(id, "GetStatus");
        c.R("status=%u", static_cast<unsigned>(*pStatus));
        return S_OK;
    }
    HRESULT GetVolume(LPLONG pVol) override
    {
        if (!pVol) return E_POINTER;
        std::lock_guard<std::mutex> g(Mx());
        *pVol = volume;
        Call c(id, "GetVolume");
        return S_OK;
    }
    HRESULT Lock(DWORD off, DWORD bytes, LPVOID* pp1, LPDWORD pn1, LPVOID* pp2, LPDWORD pn2, DWORD flags) override
    {
        if (pp1) *pp1 = nullptr;
        if (pn1) *pn1 = 0;
        if (pp2) *pp2 = nullptr;
        if (pn2) *pn2 = 0;
        std::lock_guard<std::mutex> g(Mx());
        CatchUpClock();
        Call c(id, "Lock", "off=%u bytes=%u flags=%u", off, bytes, flags);
        if (primary || data.empty() || !pp1 || !pn1) return c.Ret(DSERR_INVALIDPARAM);
        DWORD size = static_cast<DWORD>(data.size());
        if (flags & DSBLOCK_ENTIREBUFFER) {
            off = 0;
            bytes = size;
        } else if (flags & DSBLOCK_FROMWRITECURSOR) {
            DWORD play = PlayCursor();
            off = play;
            if (playing) {
                long ahead = static_cast<long>(static_cast<int64_t>(CurRate()) * kWriteAheadMs / 1000) * block;
                off = static_cast<DWORD>((play + ahead) % size);
            }
        }
        if (off >= size || bytes > size) return c.Ret(DSERR_INVALIDPARAM);
        DWORD n1 = std::min(bytes, size - off);
        DWORD n2 = bytes - n1;
        *pp1 = &data[off];
        *pn1 = n1;
        if (pp2 && n2 > 0) {
            *pp2 = &data[0];
            if (pn2) *pn2 = n2;
        }
        c.R("n1=%u n2=%u", n1, (pp2 && n2) ? n2 : 0u);
        return S_OK;
    }
    HRESULT Restore() override
    {
        std::lock_guard<std::mutex> g(Mx());
        Call c(id, "Restore");
        return S_OK;
    }
    HRESULT Play(DWORD, DWORD, DWORD flags) override
    {
        std::lock_guard<std::mutex> g(Mx());
        CatchUpClock();
        Call c(id, "Play", "flags=%u", flags);
        playing = true;
        looping = (flags & DSBPLAY_LOOPING) != 0;
        return S_OK;
    }
    HRESULT SetCurrentPosition(DWORD newPos) override
    {
        std::lock_guard<std::mutex> g(Mx());
        CatchUpClock();
        Call c(id, "SetCurrentPosition", "pos=%u", newPos);
        if (primary) return S_OK;
        if (newPos >= data.size()) return c.Ret(DSERR_INVALIDPARAM);
        pos = static_cast<double>(newPos / block);
        return S_OK;
    }
    HRESULT SetFormat(LPCWAVEFORMATEX f) override
    {
        std::lock_guard<std::mutex> g(Mx());
        Call c(id, "SetFormat", "rate=%u bits=%u ch=%u", f ? f->nSamplesPerSec : 0, f ? f->wBitsPerSample : 0, f ? f->nChannels : 0);
        if (!primary) return c.Ret(DSERR_INVALIDCALL);
        if (!f || !ValidFormat(f)) return c.Ret(DSERR_INVALIDPARAM);
        // The primary format is recorded, but the mixer's output stays 44100 Hz stereo.
        channels = f->nChannels;
        bits = f->wBitsPerSample;
        rate = static_cast<int>(f->nSamplesPerSec);
        return S_OK;
    }
    HRESULT SetFrequency(DWORD hz) override
    {
        std::lock_guard<std::mutex> g(Mx());
        Call c(id, "SetFrequency", "hz=%u", hz);
        if (primary) return c.Ret(DSERR_INVALIDCALL);
        if (hz != 0 && (hz < 100 || hz > 200000)) return c.Ret(DSERR_INVALIDPARAM);
        frequency = hz;
        return S_OK;
    }
    HRESULT SetPan(LONG p) override
    {
        std::lock_guard<std::mutex> g(Mx());
        Call c(id, "SetPan", "v=%d", static_cast<int>(p));
        if (p < -10000 || p > 10000) return c.Ret(DSERR_INVALIDPARAM);
        pan = p;
        return S_OK;
    }
    HRESULT SetVolume(LONG v) override
    {
        std::lock_guard<std::mutex> g(Mx());
        Call c(id, "SetVolume", "v=%d", static_cast<int>(v));
        if (v < -10000 || v > 0) return c.Ret(DSERR_INVALIDPARAM);
        volume = v;
        return S_OK;
    }
    HRESULT Stop() override
    {
        std::lock_guard<std::mutex> g(Mx());
        CatchUpClock();
        Call c(id, "Stop");
        playing = false;
        return S_OK;
    }
    HRESULT Unlock(LPVOID p1, DWORD n1, LPVOID p2, DWORD n2) override
    {
        std::lock_guard<std::mutex> g(Mx());
        Call c(id, "Unlock", "n1=%u n2=%u", n1, n2);
        if (primary || data.empty()) return c.Ret(DSERR_INVALIDPARAM);
        const unsigned char* b = data.data();
        const unsigned char* e = b + data.size();
        const unsigned char* c1 = static_cast<const unsigned char*>(p1);
        const unsigned char* c2 = static_cast<const unsigned char*>(p2);
        if (p1 == nullptr || c1 < b || c1 + n1 > e) return c.Ret(DSERR_INVALIDPARAM);
        if (p2 != nullptr && (c2 < b || c2 + n2 > e)) return c.Ret(DSERR_INVALIDPARAM);
        return S_OK;
    }

    static bool ValidFormat(const WAVEFORMATEX* f)
    {
        return f->wFormatTag == WAVE_FORMAT_PCM && (f->wBitsPerSample == 8 || f->wBitsPerSample == 16)
            && (f->nChannels == 1 || f->nChannels == 2) && f->nSamplesPerSec > 0;
    }

    DWORD PlayCursor() const { return static_cast<DWORD>(static_cast<int64_t>(pos) * block); }

    float Sample(int frame, int ch) const
    {
        const unsigned char* p = &data[static_cast<size_t>(frame) * block];
        if (channels == 1) ch = 0;
        if (bits == 8) return (static_cast<int>(p[ch]) - 128) / 128.0f;
        int16_t s;
        memcpy(&s, p + ch * 2, 2);
        return s / 32768.0f;
    }
};

// Mixes into float accumulators `acc` (stereo, `frames` frames) or discards when acc is null.
// Caller holds the mutex.
void MixLocked(float* acc, int frames)
{
    for (SoftBuffer* b : g_buffers) {
        if (!b->playing || b->primary) continue;
        if (b->frames <= 0) {
            b->playing = false;
            continue;
        }
        double step = static_cast<double>(b->CurRate()) / kOutRate;
        float vg = GainFromDb(b->volume);
        float gl = vg, gr = vg;
        if (b->pan < 0) gr *= GainFromDb(b->pan);
        else if (b->pan > 0) gl *= GainFromDb(-b->pan);
        for (int i = 0; i < frames && b->playing; i++) {
            if (acc) {
                int i0 = static_cast<int>(b->pos);
                double fr = b->pos - i0;
                int i1 = i0 + 1;
                if (i1 >= b->frames) i1 = b->looping ? 0 : i0;
                float l0 = b->Sample(i0, 0), r0 = b->Sample(i0, 1);
                float l = l0 + (b->Sample(i1, 0) - l0) * static_cast<float>(fr);
                float r = r0 + (b->Sample(i1, 1) - r0) * static_cast<float>(fr);
                acc[i * 2] += l * gl;
                acc[i * 2 + 1] += r * gr;
            }
            b->pos += step;
            if (b->pos >= b->frames) {
                if (b->looping) {
                    b->pos = fmod(b->pos, static_cast<double>(b->frames));
                } else {
                    b->pos = 0;
                    b->playing = false;
                }
            }
        }
    }
}

// Clock-driven advance, used only when no device is open. Caller holds the mutex.
void CatchUpClock()
{
    if (g_deviceOpen || g_manualClock || !g_initDone) return;
    int64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - g_clockStart).count();
    int64_t target = ns * kOutRate / 1000000000LL;
    while (g_clockFramesDone < target) {
        int n = static_cast<int>(std::min<int64_t>(target - g_clockFramesDone, 4096));
        MixLocked(nullptr, n);
        g_clockFramesDone += n;
    }
}

void SDLCALL AudioCallback(void*, SDL_AudioStream* stream, int additional, int)
{
    int16_t tmp[2048 * 2];
    while (additional > 0) {
        int frames = std::min(additional / 4 + 1, 2048);
        platform::MixSound(tmp, frames);
        SDL_PutAudioStreamData(stream, tmp, frames * 4);
        additional -= frames * 4;
    }
}

void InitOnce()
{
    {
        std::lock_guard<std::mutex> g(Mx());
        if (g_initDone) return;
        const char* lg = getenv("IWD2RE_SOUND_LOG");
        if (lg && *lg) g_log = fopen(lg, "w");
        g_clockStart = Clock::now();
        g_initDone = true;
    }
    const char* off = getenv("IWD2RE_SOUND");
    if (off && strcmp(off, "off") == 0) return;
    if (!SDL_Init(SDL_INIT_AUDIO)) return;
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    spec.freq = kOutRate;
    SDL_AudioStream* s = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, AudioCallback, nullptr);
    if (s == nullptr) return;
    {
        std::lock_guard<std::mutex> g(Mx());
        g_stream = s;
        g_deviceOpen = true;
    }
    SDL_ResumeAudioStreamDevice(s);
}

class SoftDevice : public IDirectSound {
public:
    ULONG refs = 0;

    HRESULT QueryInterface(REFIID riid, LPVOID* ppv) override
    {
        if (!ppv) return E_POINTER;
        *ppv = nullptr;
        if (!SameGuid(riid, IID_IUnknown)) return E_NOINTERFACE;
        AddRef();
        *ppv = static_cast<IDirectSound*>(this);
        return S_OK;
    }
    ULONG AddRef() override
    {
        std::lock_guard<std::mutex> g(Mx());
        return ++refs;
    }
    ULONG Release() override
    {
        std::lock_guard<std::mutex> g(Mx());
        { Call c(0, "Release"); }
        return refs > 0 ? --refs : 0; // the object itself is static
    }
    HRESULT CreateSoundBuffer(LPCDSBUFFERDESC d, LPDIRECTSOUNDBUFFER* pp, LPUNKNOWN) override
    {
        if (!d || !pp) return DSERR_INVALIDPARAM;
        *pp = nullptr;
        std::lock_guard<std::mutex> g(Mx());
        bool primary = (d->dwFlags & DSBCAPS_PRIMARYBUFFER) != 0;
        const WAVEFORMATEX* f = d->lpwfxFormat;
        Call c(0, "CreateSoundBuffer", "flags=%u bytes=%u rate=%u bits=%u ch=%u", d->dwFlags, d->dwBufferBytes,
            f ? f->nSamplesPerSec : 0, f ? f->wBitsPerSample : 0, f ? f->nChannels : 0);
        SoftBuffer* b = new SoftBuffer();
        b->primary = primary;
        if (!primary) {
            if (!f || !SoftBuffer::ValidFormat(f) || d->dwBufferBytes == 0) {
                delete b;
                return c.Ret(DSERR_INVALIDPARAM);
            }
            b->channels = f->nChannels;
            b->bits = f->wBitsPerSample;
            b->rate = static_cast<int>(f->nSamplesPerSec);
            b->block = b->channels * b->bits / 8;
            b->data.assign(d->dwBufferBytes, b->bits == 8 ? 0x80 : 0);
            b->frames = static_cast<int>(d->dwBufferBytes / b->block);
        }
        b->id = g_nextId++;
        g_buffers.push_back(b);
        *pp = b;
        c.R("id=%d", b->id);
        return S_OK;
    }
    HRESULT GetCaps(LPDSCAPS caps) override
    {
        if (!caps || caps->dwSize < sizeof(DSCAPS)) return DSERR_INVALIDPARAM;
        std::lock_guard<std::mutex> g(Mx());
        Call c(0, "GetCaps");
        DWORD size = caps->dwSize;
        memset(caps, 0, size);
        caps->dwSize = size;
        caps->dwFlags = DSCAPS_PRIMARYSTEREO | DSCAPS_PRIMARY16BIT | DSCAPS_CONTINUOUSRATE | DSCAPS_EMULDRIVER;
        caps->dwMinSecondarySampleRate = 100;
        caps->dwMaxSecondarySampleRate = 200000;
        caps->dwPrimaryBuffers = 1;
        return S_OK;
    }
    HRESULT SetCooperativeLevel(HWND, DWORD level) override
    {
        std::lock_guard<std::mutex> g(Mx());
        Call c(0, "SetCooperativeLevel", "level=%u", level);
        return S_OK;
    }
};

SoftDevice g_device;

} // namespace

namespace platform {

void MixSound(int16_t* out, int frames)
{
    if (frames <= 0) return;
    std::vector<float> acc(static_cast<size_t>(frames) * 2, 0.0f);
    {
        std::lock_guard<std::mutex> g(Mx());
        MixLocked(acc.data(), frames);
    }
    for (size_t i = 0; i < acc.size(); i++) {
        float v = acc[i] * 32768.0f;
        if (v > 32767.0f) v = 32767.0f;
        if (v < -32768.0f) v = -32768.0f;
        out[i] = static_cast<int16_t>(lrintf(v));
    }
}

void SoundSetManualClock(bool manual)
{
    std::lock_guard<std::mutex> g(Mx());
    g_manualClock = manual;
}

} // namespace platform

HRESULT WINAPI DirectSoundCreate(LPCGUID, LPDIRECTSOUND* ppDS, LPUNKNOWN)
{
    if (!ppDS) return DSERR_INVALIDPARAM;
    InitOnce();
    std::lock_guard<std::mutex> g(Mx());
    Call c(0, "DirectSoundCreate");
    g_device.refs++;
    *ppDS = &g_device;
    return S_OK;
}
