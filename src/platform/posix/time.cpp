// Tick counts, sleeping, system/file times, multimedia timers.

#include "kobject.h"

#include <win32shim/winbase_file.h>
#include <win32shim/winbase_time.h>
#include <win32shim/winmm.h>

#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

namespace platform {

static const uint64_t TICKS_PER_SECOND = 10000000ull;
static const uint64_t UNIX_EPOCH_FILETIME = 116444736000000000ull; // 1970 in FILETIME
static const int64_t DAYS_1601_TO_1970 = 134774;

uint64_t UnixToFileTime(int64_t seconds, int64_t nanoseconds)
{
    return static_cast<uint64_t>(seconds) * TICKS_PER_SECOND + static_cast<uint64_t>(nanoseconds / 100) + UNIX_EPOCH_FILETIME;
}

static uint64_t MonotonicNs()
{
    return clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
}

static uint64_t FileTimeValue(const FILETIME* ft)
{
    return (static_cast<uint64_t>(ft->dwHighDateTime) << 32) | ft->dwLowDateTime;
}

static void SetFileTimeValue(FILETIME* ft, uint64_t v)
{
    ft->dwLowDateTime = static_cast<DWORD>(v & 0xFFFFFFFFu);
    ft->dwHighDateTime = static_cast<DWORD>(v >> 32);
}

// Days since 1970-01-01 to civil date (proleptic Gregorian).
static void CivilFromUnixDays(int64_t days, int& y, unsigned& m, unsigned& d)
{
    int64_t z = days + 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    if (m <= 2) y++;
}

static int64_t UnixDaysFromCivil(int y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

} // namespace platform

using namespace platform;

// ---------------------------------------------------------------------------

DWORD GetTickCount()
{
    // Milliseconds as a 32-bit value, wrapping at 2^32 like Windows. Callers
    // subtract unsigned.
    return static_cast<DWORD>(MonotonicNs() / 1000000ull);
}

DWORD timeGetTime()
{
    return GetTickCount();
}

void Sleep(DWORD dwMilliseconds)
{
    if (dwMilliseconds == 0) {
        sched_yield();
        return;
    }
    if (dwMilliseconds == INFINITE) {
        for (;;) {
            sleep(3600);
        }
    }
    struct timespec ts;
    ts.tv_sec = dwMilliseconds / 1000;
    ts.tv_nsec = static_cast<long>(dwMilliseconds % 1000) * 1000000L;
    while (nanosleep(&ts, &ts) != 0) {
    }
}

DWORD SleepEx(DWORD dwMilliseconds, BOOL)
{
    Sleep(dwMilliseconds);
    return 0;
}

BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
    lpPerformanceCount->QuadPart = static_cast<LONGLONG>(MonotonicNs());
    return TRUE;
}

BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
{
    lpFrequency->QuadPart = 1000000000LL;
    return TRUE;
}

// --- FILETIME / SYSTEMTIME ---

BOOL FileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime)
{
    if (lpFileTime == NULL || lpSystemTime == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    uint64_t v = FileTimeValue(lpFileTime);
    if (v > 0x7FFFFFFFFFFFFFFFull) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    uint64_t ms = v / 10000ull;
    int64_t days = static_cast<int64_t>(ms / 86400000ull); // since 1601-01-01
    uint64_t msOfDay = ms % 86400000ull;

    int y;
    unsigned m, d;
    CivilFromUnixDays(days - DAYS_1601_TO_1970, y, m, d);

    lpSystemTime->wYear = static_cast<WORD>(y);
    lpSystemTime->wMonth = static_cast<WORD>(m);
    lpSystemTime->wDay = static_cast<WORD>(d);
    lpSystemTime->wDayOfWeek = static_cast<WORD>((days + 1) % 7); // 1601-01-01 was a Monday
    lpSystemTime->wHour = static_cast<WORD>(msOfDay / 3600000ull);
    lpSystemTime->wMinute = static_cast<WORD>((msOfDay / 60000ull) % 60);
    lpSystemTime->wSecond = static_cast<WORD>((msOfDay / 1000ull) % 60);
    lpSystemTime->wMilliseconds = static_cast<WORD>(msOfDay % 1000ull);
    return TRUE;
}

BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime)
{
    if (lpSystemTime == NULL || lpFileTime == NULL
        || lpSystemTime->wMonth < 1 || lpSystemTime->wMonth > 12
        || lpSystemTime->wDay < 1 || lpSystemTime->wDay > 31
        || lpSystemTime->wHour > 23 || lpSystemTime->wMinute > 59
        || lpSystemTime->wSecond > 59 || lpSystemTime->wMilliseconds > 999
        || lpSystemTime->wYear < 1601) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    int64_t days = UnixDaysFromCivil(lpSystemTime->wYear, lpSystemTime->wMonth, lpSystemTime->wDay) + DAYS_1601_TO_1970;
    uint64_t ms = static_cast<uint64_t>(days) * 86400000ull
        + lpSystemTime->wHour * 3600000ull + lpSystemTime->wMinute * 60000ull
        + lpSystemTime->wSecond * 1000ull + lpSystemTime->wMilliseconds;
    SetFileTimeValue(lpFileTime, ms * 10000ull);
    return TRUE;
}

// Offset of local time from UTC at the given Unix time, in seconds.
static int64_t LocalOffsetAtUtc(int64_t unixSeconds)
{
    time_t t = static_cast<time_t>(unixSeconds);
    struct tm tmv;
    if (localtime_r(&t, &tmv) == NULL) {
        return 0;
    }
    return tmv.tm_gmtoff;
}

BOOL SystemTimeToTzSpecificLocalTime(const void* lpTimeZoneInformation, const SYSTEMTIME* lpUniversalTime, LPSYSTEMTIME lpLocalTime)
{
    // NOTE: Port: only the "current time zone" form (NULL) is supported.
    (void)lpTimeZoneInformation;
    FILETIME utc, local;
    if (!SystemTimeToFileTime(lpUniversalTime, &utc)) {
        return FALSE;
    }
    uint64_t v = FileTimeValue(&utc);
    int64_t unixSeconds = v >= UNIX_EPOCH_FILETIME ? static_cast<int64_t>((v - UNIX_EPOCH_FILETIME) / TICKS_PER_SECOND) : 0;
    int64_t offset = LocalOffsetAtUtc(unixSeconds);
    SetFileTimeValue(&local, v + static_cast<uint64_t>(offset * static_cast<int64_t>(TICKS_PER_SECOND)));
    return FileTimeToSystemTime(&local, lpLocalTime);
}

BOOL LocalFileTimeToFileTime(const FILETIME* lpLocalFileTime, LPFILETIME lpFileTime)
{
    if (lpLocalFileTime == NULL || lpFileTime == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    uint64_t v = FileTimeValue(lpLocalFileTime);
    if (v < UNIX_EPOCH_FILETIME) {
        SetFileTimeValue(lpFileTime, v);
        return TRUE;
    }
    int64_t localSeconds = static_cast<int64_t>((v - UNIX_EPOCH_FILETIME) / TICKS_PER_SECOND);
    // Read the broken-down time as local time and let mktime find the UTC.
    time_t lt = static_cast<time_t>(localSeconds);
    struct tm tmv;
    gmtime_r(&lt, &tmv);
    tmv.tm_isdst = -1;
    time_t utc = mktime(&tmv);
    int64_t delta = static_cast<int64_t>(utc) - localSeconds;
    SetFileTimeValue(lpFileTime, v + static_cast<uint64_t>(delta * static_cast<int64_t>(TICKS_PER_SECOND)));
    return TRUE;
}

void GetSystemTime(LPSYSTEMTIME lpSystemTime)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    FILETIME ft;
    SetFileTimeValue(&ft, UnixToFileTime(tv.tv_sec, static_cast<int64_t>(tv.tv_usec) * 1000));
    FileTimeToSystemTime(&ft, lpSystemTime);
}

void GetLocalTime(LPSYSTEMTIME lpSystemTime)
{
    SYSTEMTIME utc;
    GetSystemTime(&utc);
    SystemTimeToTzSpecificLocalTime(NULL, &utc, lpSystemTime);
}

// ---------------------------------------------------------------------------
// Multimedia timers

namespace {

struct Timer {
    UINT id;
    UINT period;
    bool periodic;
    LPTIMECALLBACK callback;
    DWORD_PTR user;
    std::mutex m;
    std::condition_variable cv;
    bool stop;
    bool running; // inside the callback
    pthread_t callbackThread;
    Timer() : id(0), period(0), periodic(false), callback(NULL), user(0), stop(false), running(false), callbackThread() {}
};

std::mutex g_timersMutex;
std::map<UINT, std::shared_ptr<Timer>> g_timers;
UINT g_nextTimerId = 1;

void* TimerThread(void* p)
{
    std::shared_ptr<Timer>* holder = static_cast<std::shared_ptr<Timer>*>(p);
    std::shared_ptr<Timer> t = *holder;
    delete holder;

    typedef std::chrono::steady_clock clock;
    clock::time_point deadline = clock::now() + std::chrono::milliseconds(t->period);

    for (;;) {
        {
            std::unique_lock<std::mutex> lock(t->m);
            while (!t->stop && clock::now() < deadline) {
                t->cv.wait_until(lock, deadline);
            }
            if (t->stop) {
                break;
            }
            t->running = true;
            t->callbackThread = pthread_self();
        }

        t->callback(t->id, 0, t->user, 0, 0);

        {
            std::lock_guard<std::mutex> lock(t->m);
            t->running = false;
            t->cv.notify_all();
            if (!t->periodic) {
                t->stop = true;
                break;
            }
        }

        // Against absolute deadlines: no drift. A callback that overran its
        // period makes the next one due at once; missed ones are not replayed.
        deadline += std::chrono::milliseconds(t->period);
        clock::time_point now = clock::now();
        if (deadline < now) {
            deadline = now;
        }
    }
    return NULL;
}

} // namespace

MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
{
    if (ptc == NULL || cbtc < sizeof(TIMECAPS)) {
        return 97; // TIMERR_STRUCT
    }
    ptc->wPeriodMin = 1;
    ptc->wPeriodMax = 1000000;
    return TIMERR_NOERROR;
}

MMRESULT timeBeginPeriod(UINT)
{
    return TIMERR_NOERROR;
}

MMRESULT timeEndPeriod(UINT)
{
    return TIMERR_NOERROR;
}

MMRESULT timeSetEvent(UINT uDelay, UINT, LPTIMECALLBACK fptc, DWORD_PTR dwUser, UINT fuEvent)
{
    if (fptc == NULL) {
        return 0;
    }

    std::shared_ptr<Timer> t = std::make_shared<Timer>();
    t->period = uDelay == 0 ? 1 : uDelay;
    t->periodic = (fuEvent & 1) != 0; // TIME_PERIODIC
    t->callback = fptc;
    t->user = dwUser;

    {
        std::lock_guard<std::mutex> lock(g_timersMutex);
        t->id = g_nextTimerId++;
        if (g_nextTimerId == 0) g_nextTimerId = 1;
        g_timers[t->id] = t;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t th;
    std::shared_ptr<Timer>* holder = new std::shared_ptr<Timer>(t);
    if (pthread_create(&th, &attr, TimerThread, holder) != 0) {
        delete holder;
        pthread_attr_destroy(&attr);
        std::lock_guard<std::mutex> lock(g_timersMutex);
        g_timers.erase(t->id);
        return 0;
    }
    pthread_attr_destroy(&attr);
    return t->id;
}

MMRESULT timeKillEvent(UINT uTimerID)
{
    std::shared_ptr<Timer> t;
    {
        std::lock_guard<std::mutex> lock(g_timersMutex);
        std::map<UINT, std::shared_ptr<Timer>>::iterator it = g_timers.find(uTimerID);
        if (it == g_timers.end()) {
            return 97; // MMSYSERR_INVALPARAM
        }
        t = it->second;
        g_timers.erase(it);
    }

    std::unique_lock<std::mutex> lock(t->m);
    t->stop = true;
    t->cv.notify_all();
    // As on Windows, the callback is not running once this returns, unless
    // it is called from inside the callback itself.
    if (t->running && !pthread_equal(t->callbackThread, pthread_self())) {
        t->cv.wait(lock, [&t] { return !t->running; });
    }
    return TIMERR_NOERROR;
}
