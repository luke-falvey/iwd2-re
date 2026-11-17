// The DirectSound shim (src/platform/posix/dsound_soft.cpp), driven through the DirectSound API
// and the mixer entry point platform::MixSound. Run without an audio device.

#include "test.h"

#include <dsound.h>

#include "../../src/platform/posix/sound_mixer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace {

IDirectSound* Device()
{
    static IDirectSound* ds = nullptr;
    if (ds == nullptr) {
        setenv("IWD2RE_SOUND", "off", 1);
        REQUIRE(DirectSoundCreate(NULL, &ds, NULL) == DS_OK);
        platform::SoundSetManualClock(true);
    }
    return ds;
}

IDirectSoundBuffer* MakeBuffer(int channels, int bits, int rate, int bytes)
{
    WAVEFORMATEX f = { 0 };
    f.wFormatTag = WAVE_FORMAT_PCM;
    f.nChannels = static_cast<WORD>(channels);
    f.wBitsPerSample = static_cast<WORD>(bits);
    f.nSamplesPerSec = rate;
    f.nBlockAlign = static_cast<WORD>(channels * bits / 8);
    f.nAvgBytesPerSec = rate * f.nBlockAlign;
    DSBUFFERDESC d = { 0 };
    d.dwSize = sizeof(d);
    d.dwFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
    d.dwBufferBytes = bytes;
    d.lpwfxFormat = &f;
    IDirectSoundBuffer* b = nullptr;
    REQUIRE(Device()->CreateSoundBuffer(&d, &b, NULL) == DS_OK);
    return b;
}

void Fill(IDirectSoundBuffer* b, const void* src, int bytes)
{
    void *p1, *p2;
    DWORD n1, n2;
    REQUIRE(b->Lock(0, bytes, &p1, &n1, &p2, &n2, 0) == DS_OK);
    REQUIRE(n1 == static_cast<DWORD>(bytes));
    CHECK(p2 == nullptr);
    CHECK(n2 == 0);
    memcpy(p1, src, bytes);
    CHECK(b->Unlock(p1, n1, p2, n2) == DS_OK);
}

// Mono 16-bit buffer of `frames` samples, all `value`.
IDirectSoundBuffer* ConstBuffer16(int frames, int16_t value, int rate = 44100)
{
    IDirectSoundBuffer* b = MakeBuffer(1, 16, rate, frames * 2);
    std::vector<int16_t> v(frames, value);
    Fill(b, v.data(), frames * 2);
    return b;
}

std::vector<int16_t> Mix(int frames)
{
    std::vector<int16_t> out(static_cast<size_t>(frames) * 2, 12345);
    platform::MixSound(out.data(), frames);
    return out;
}

DWORD Status(IDirectSoundBuffer* b)
{
    DWORD s = 99;
    CHECK(b->GetStatus(&s) == DS_OK);
    return s;
}

DWORD PlayPos(IDirectSoundBuffer* b)
{
    DWORD p = 0, w = 0;
    CHECK(b->GetCurrentPosition(&p, &w) == DS_OK);
    return p;
}

} // namespace

TEST_CASE("sound: a one-shot buffer plays on both channels and stops at its end")
{
    IDirectSoundBuffer* b = ConstBuffer16(100, 32767);
    CHECK(Status(b) == 0);
    CHECK(b->Play(0, 0, 0) == DS_OK);
    CHECK(Status(b) == DSBSTATUS_PLAYING);
    std::vector<int16_t> o = Mix(60);
    CHECK(o[0] == 32767);
    CHECK(o[1] == 32767);
    CHECK(o[2 * 59] == 32767);
    CHECK(PlayPos(b) == 120);
    o = Mix(60);
    CHECK(o[0] == 32767);
    CHECK(o[2 * 39 + 1] == 32767);
    CHECK(o[2 * 40] == 0);
    CHECK(o[2 * 40 + 1] == 0);
    CHECK(Status(b) == 0);
    CHECK(PlayPos(b) == 0);
    b->Release();
}

TEST_CASE("sound: looping wraps, Stop keeps the position")
{
    IDirectSoundBuffer* b = ConstBuffer16(10, 1000);
    CHECK(b->Play(0, 0, DSBPLAY_LOOPING) == DS_OK);
    Mix(25);
    CHECK(Status(b) == (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING));
    CHECK(PlayPos(b) == 10);
    CHECK(b->Stop() == DS_OK);
    CHECK(Status(b) == 0);
    std::vector<int16_t> o = Mix(5);
    CHECK(o[0] == 0);
    CHECK(PlayPos(b) == 10);
    CHECK(b->Play(0, 0, 0) == DS_OK); // starts from the current position
    o = Mix(2);
    CHECK(o[0] == 1000);
    CHECK(b->SetCurrentPosition(7) == DS_OK); // aligned down to a block
    CHECK(PlayPos(b) == 6);
    b->Release();
}

TEST_CASE("sound: volume and pan")
{
    IDirectSoundBuffer* b = ConstBuffer16(100, 20000);
    LONG v = 1;
    CHECK(b->SetVolume(-600) == DS_OK);
    CHECK(b->GetVolume(&v) == DS_OK);
    CHECK(v == -600);
    b->Play(0, 0, DSBPLAY_LOOPING);
    std::vector<int16_t> o = Mix(4);
    CHECK(fabs(o[0] / 20000.0 - 0.5012) < 0.002);
    CHECK(b->SetVolume(-10000) == DS_OK);
    o = Mix(4);
    CHECK(o[0] == 0);
    CHECK(o[1] == 0);
    CHECK(b->SetVolume(0) == DS_OK);
    CHECK(b->SetPan(-10000) == DS_OK);
    o = Mix(4);
    CHECK(o[0] == 20000);
    CHECK(o[1] == 0);
    CHECK(b->SetPan(10000) == DS_OK);
    o = Mix(4);
    CHECK(o[0] == 0);
    CHECK(o[1] == 20000);
    CHECK(b->SetVolume(1) != DS_OK);
    CHECK(b->SetPan(10001) != DS_OK);
    b->Release();
}

TEST_CASE("sound: 8-bit unsigned and stereo")
{
    IDirectSoundBuffer* b = MakeBuffer(1, 8, 44100, 4);
    unsigned char d[4] = { 0x80, 0xFF, 0x00, 0x80 };
    Fill(b, d, 4);
    b->Play(0, 0, 0);
    std::vector<int16_t> o = Mix(4);
    CHECK(o[0] == 0);
    CHECK(o[2] == 32767 - 255); // 127/128 of full scale
    CHECK(o[4] == -32768);
    CHECK(o[6] == 0);
    b->Release();

    IDirectSoundBuffer* s = MakeBuffer(2, 16, 44100, 8);
    int16_t st[4] = { 1000, -2000, 3000, -4000 };
    Fill(s, st, 8);
    s->Play(0, 0, 0);
    o = Mix(2);
    CHECK(o[0] == 1000);
    CHECK(o[1] == -2000);
    CHECK(o[2] == 3000);
    CHECK(o[3] == -4000);
    s->Release();
}

TEST_CASE("sound: sample rates and SetFrequency")
{
    IDirectSoundBuffer* b = ConstBuffer16(100, 5000, 22050);
    b->Play(0, 0, 0);
    Mix(199);
    CHECK(Status(b) == DSBSTATUS_PLAYING);
    Mix(1);
    CHECK(Status(b) == 0);

    CHECK(b->SetFrequency(44100) == DS_OK); // double speed
    b->Play(0, 0, 0);
    Mix(99);
    CHECK(Status(b) == DSBSTATUS_PLAYING);
    Mix(1);
    CHECK(Status(b) == 0);

    CHECK(b->SetFrequency(0) == DS_OK); // back to the format's rate
    b->Play(0, 0, 0);
    Mix(199);
    CHECK(Status(b) == DSBSTATUS_PLAYING);
    Mix(1);
    CHECK(Status(b) == 0);
    b->Release();

    // Linear interpolation of a ramp when slowed down.
    IDirectSoundBuffer* r = MakeBuffer(1, 16, 22050, 8);
    int16_t ramp[4] = { 0, 1000, 2000, 3000 };
    Fill(r, ramp, 8);
    r->Play(0, 0, 0);
    std::vector<int16_t> o = Mix(4);
    CHECK(o[0] == 0);
    CHECK(o[2] == 500);
    CHECK(o[4] == 1000);
    CHECK(o[6] == 1500);
    r->Release();
}

TEST_CASE("sound: Lock across the end gives two spans, the cursors advance")
{
    IDirectSoundBuffer* b = ConstBuffer16(1000, 0, 22050);
    void *p1, *p2;
    DWORD n1, n2;
    REQUIRE(b->Lock(1900, 300, &p1, &n1, &p2, &n2, 0) == DS_OK);
    CHECK(n1 == 100);
    CHECK(n2 == 200);
    CHECK(p2 != nullptr);
    CHECK(b->Unlock(p1, n1, p2, n2) == DS_OK);
    REQUIRE(b->Lock(0, 0, &p1, &n1, &p2, &n2, DSBLOCK_ENTIREBUFFER) == DS_OK);
    CHECK(n1 == 2000);
    CHECK(p2 == nullptr);
    CHECK(b->Unlock(p1, n1, p2, n2) == DS_OK);
    CHECK(b->Lock(0, 2001, &p1, &n1, &p2, &n2, 0) != DS_OK);

    // Stopped: the write cursor is the play cursor; playing: a little ahead.
    DWORD play, write;
    b->GetCurrentPosition(&play, &write);
    CHECK(write == play);
    b->Play(0, 0, DSBPLAY_LOOPING);
    Mix(441);
    b->GetCurrentPosition(&play, &write);
    CHECK(play == 2 * 220);
    CHECK(write == play + 2 * 330); // 15 ms at 22050 Hz = 330 frames
    REQUIRE(b->Lock(0, 100, &p1, &n1, &p2, &n2, DSBLOCK_FROMWRITECURSOR) == DS_OK);
    CHECK(p1 == static_cast<char*>(p1)); // any pointer
    CHECK(n1 + n2 == 100);
    b->Unlock(p1, n1, p2, n2);
    b->Release();
}

TEST_CASE("sound: two playing buffers add")
{
    IDirectSoundBuffer* a = ConstBuffer16(50, 1000);
    IDirectSoundBuffer* b = ConstBuffer16(50, 2500);
    a->Play(0, 0, 0);
    b->Play(0, 0, 0);
    std::vector<int16_t> o = Mix(4);
    CHECK(o[0] == 3500);
    CHECK(o[1] == 3500);
    a->Release();
    b->Release();
}

TEST_CASE("sound: Release while playing is safe, a primary buffer is silent")
{
    IDirectSoundBuffer* a = ConstBuffer16(50, 1000);
    a->Play(0, 0, DSBPLAY_LOOPING);
    a->Release();
    std::vector<int16_t> o = Mix(4);
    CHECK(o[0] == 0);

    DSBUFFERDESC d = { 0 };
    d.dwSize = sizeof(d);
    d.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
    IDirectSoundBuffer* p = nullptr;
    REQUIRE(Device()->CreateSoundBuffer(&d, &p, NULL) == DS_OK);
    WAVEFORMATEX f = { 0 };
    f.wFormatTag = WAVE_FORMAT_PCM;
    f.nChannels = 2;
    f.wBitsPerSample = 16;
    f.nSamplesPerSec = 22050;
    f.nBlockAlign = 4;
    f.nAvgBytesPerSec = 88200;
    CHECK(p->SetFormat(&f) == DS_OK);
    CHECK(p->Play(0, 0, DSBPLAY_LOOPING) == DS_OK);
    CHECK(p->SetVolume(0) == DS_OK);
    o = Mix(4);
    CHECK(o[0] == 0);
    CHECK(p->Stop() == DS_OK);

    IDirectSound3DListener* l = reinterpret_cast<IDirectSound3DListener*>(1);
    CHECK(p->QueryInterface(IID_IDirectSound3DListener, reinterpret_cast<LPVOID*>(&l)) == E_NOINTERFACE);
    CHECK(l == nullptr);
    p->Release();
}

TEST_CASE("sound: QueryInterface of 3D buffer, listener and property set fails")
{
    IDirectSoundBuffer* b = ConstBuffer16(50, 0);
    void* out = reinterpret_cast<void*>(1);
    CHECK(b->QueryInterface(IID_IDirectSound3DBuffer, &out) == E_NOINTERFACE);
    CHECK(out == nullptr);
    CHECK(b->QueryInterface(IID_IDirectSound3DListener, &out) == E_NOINTERFACE);
    CHECK(b->QueryInterface(IID_IKsPropertySet, &out) == E_NOINTERFACE);
    b->Release();
}

TEST_CASE("sound: GetCaps and SetCooperativeLevel")
{
    DSCAPS caps = { 0 };
    caps.dwSize = sizeof(caps);
    REQUIRE(Device()->GetCaps(&caps) == DS_OK);
    CHECK((caps.dwFlags & DSCAPS_PRIMARY16BIT) != 0);
    CHECK((caps.dwFlags & DSCAPS_PRIMARYSTEREO) != 0);
    CHECK(Device()->SetCooperativeLevel(NULL, DSSCL_EXCLUSIVE) == DS_OK);
}
