// Time, events, semaphores, critical sections, threads, multimedia timers and
// interlocked operations of the shim.

#include "test.h"

#include <mmsystem.h>
#include <process.h>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

TEST_CASE("time: GetTickCount is monotonic and counts milliseconds")
{
    DWORD t0 = GetTickCount();
    DWORD previous = t0;
    for (int i = 0; i < 1000; i++) {
        DWORD now = GetTickCount();
        CHECK(static_cast<LONG>(now - previous) >= 0);
        previous = now;
    }
    Sleep(60);
    DWORD elapsed = GetTickCount() - t0;
    CHECK(elapsed >= 50);
    CHECK(elapsed < 2000);
    CHECK(timeGetTime() - t0 >= 50);
    Sleep(0); // yields, returns
}

TEST_CASE("events: manual reset stays signalled, auto reset releases one wait")
{
    HANDLE manual = CreateEventA(NULL, TRUE, FALSE, NULL);
    REQUIRE(manual != NULL);
    CHECK(WaitForSingleObject(manual, 0) == WAIT_TIMEOUT);
    CHECK(SetEvent(manual));
    CHECK(WaitForSingleObject(manual, 0) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(manual, 0) == WAIT_OBJECT_0);
    CHECK(ResetEvent(manual));
    CHECK(WaitForSingleObject(manual, 0) == WAIT_TIMEOUT);
    CloseHandle(manual);

    HANDLE autoReset = CreateEventA(NULL, FALSE, TRUE, NULL);
    REQUIRE(autoReset != NULL);
    CHECK(WaitForSingleObject(autoReset, 0) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(autoReset, 0) == WAIT_TIMEOUT);
    CHECK(SetEvent(autoReset));
    CHECK(WaitForSingleObject(autoReset, 1000) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(autoReset, 0) == WAIT_TIMEOUT);
    CloseHandle(autoReset);
}

TEST_CASE("events: timeouts are in milliseconds and a set from another thread wakes the wait")
{
    HANDLE e = CreateEventA(NULL, FALSE, FALSE, NULL);
    DWORD t0 = GetTickCount();
    CHECK(WaitForSingleObject(e, 80) == WAIT_TIMEOUT);
    DWORD waited = GetTickCount() - t0;
    CHECK(waited >= 70);
    CHECK(waited < 2000);

    std::thread setter([e] {
        Sleep(50);
        SetEvent(e);
    });
    CHECK(WaitForSingleObject(e, INFINITE) == WAIT_OBJECT_0);
    setter.join();
    CloseHandle(e);
}

TEST_CASE("semaphores count")
{
    HANDLE s = CreateSemaphoreA(NULL, 0, 2, NULL);
    REQUIRE(s != NULL);
    CHECK(WaitForSingleObject(s, 0) == WAIT_TIMEOUT);
    LONG previous = -1;
    CHECK(ReleaseSemaphore(s, 1, &previous));
    CHECK(previous == 0);
    CHECK(ReleaseSemaphore(s, 1, &previous));
    CHECK(previous == 1);
    CHECK_FALSE(ReleaseSemaphore(s, 1, NULL)); // above the maximum
    CHECK(WaitForSingleObject(s, 0) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(s, 0) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(s, 0) == WAIT_TIMEOUT);
    CloseHandle(s);
}

TEST_CASE("mutex: a named mutex is acquired by the first wait (single instance check)")
{
    HANDLE m = CreateMutexA(NULL, FALSE, "Global\\Icewind2");
    REQUIRE(m != NULL);
    CHECK(WaitForSingleObject(m, 0) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(m, 0) == WAIT_OBJECT_0); // recursive for its owner
    std::atomic<DWORD> other(0);
    std::thread t([&] { other = WaitForSingleObject(m, 0); });
    t.join();
    CHECK(other == WAIT_TIMEOUT);
    CHECK(ReleaseMutex(m));
    CHECK(ReleaseMutex(m));
    CloseHandle(m);
}

TEST_CASE("critical sections are recursive and exclude other threads")
{
    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    EnterCriticalSection(&cs);
    EnterCriticalSection(&cs);
    CHECK(TryEnterCriticalSection(&cs));
    LeaveCriticalSection(&cs);

    std::atomic<int> got(0);
    std::thread t([&] { got = TryEnterCriticalSection(&cs) ? 1 : 2; if (got == 1) LeaveCriticalSection(&cs); });
    t.join();
    CHECK(got == 2); // held by this thread

    LeaveCriticalSection(&cs);
    LeaveCriticalSection(&cs);

    std::thread t2([&] { got = TryEnterCriticalSection(&cs) ? 1 : 2; if (got == 1) LeaveCriticalSection(&cs); });
    t2.join();
    CHECK(got == 1);

    DeleteCriticalSection(&cs);
}

TEST_CASE("critical section protects a counter")
{
    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);
    int counter = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back([&] {
            for (int k = 0; k < 20000; k++) {
                EnterCriticalSection(&cs);
                EnterCriticalSection(&cs);
                counter++;
                LeaveCriticalSection(&cs);
                LeaveCriticalSection(&cs);
            }
        });
    }
    for (size_t i = 0; i < threads.size(); i++) threads[i].join();
    CHECK(counter == 80000);
    DeleteCriticalSection(&cs);
}

TEST_CASE("interlocked operations work on misaligned targets")
{
#pragma pack(push, 1)
    struct Packed {
        char pad;
        LONG value;
    } packed = { 0, 5 };
#pragma pack(pop)
    CHECK(InterlockedIncrement(&packed.value) == 6);
    CHECK(InterlockedDecrement(&packed.value) == 5);
    CHECK(InterlockedExchange(&packed.value, 9) == 5);
    CHECK(packed.value == 9);

    LONG shared = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back([&] {
            for (int k = 0; k < 20000; k++) {
                InterlockedIncrement(&shared);
            }
        });
    }
    for (size_t i = 0; i < threads.size(); i++) threads[i].join();
    CHECK(shared == 80000);
}

namespace {
struct ThreadArgs {
    std::atomic<int> started;
    std::atomic<int> counter;
    std::atomic<bool> stop;
    HANDLE selfHandle;
    HANDLE selfDuplicate;
    DWORD threadId;
    int firstRand;
};

void __cdecl CountingThread(void* p)
{
    ThreadArgs* a = static_cast<ThreadArgs*>(p);
    a->threadId = GetCurrentThreadId();
    HANDLE process = GetCurrentProcess();
    DuplicateHandle(process, GetCurrentThread(), process, &a->selfDuplicate, 0, TRUE, DUPLICATE_SAME_ACCESS);
    a->firstRand = rand();
    a->started = 1;
    while (!a->stop) {
        a->counter++;
        Sleep(1);
    }
}

void __cdecl SelfSuspendingThread(void* p)
{
    ThreadArgs* a = static_cast<ThreadArgs*>(p);
    a->started = 1;
    SuspendThread(a->selfHandle);
    a->counter = 1;
}
} // namespace

TEST_CASE("threads: _beginthread, suspend and resume return the previous count, wait for the end")
{
    ThreadArgs args;
    args.started = 0;
    args.counter = 0;
    args.stop = false;
    args.selfHandle = NULL;
    args.selfDuplicate = NULL;

    HANDLE h = reinterpret_cast<HANDLE>(_beginthread(CountingThread, 0, &args));
    REQUIRE(reinterpret_cast<uintptr_t>(h) != static_cast<uintptr_t>(-1));
    while (!args.started) Sleep(1);

    CHECK(args.threadId != GetCurrentThreadId());
    CHECK(args.firstRand == 41); // a new thread starts at seed 1
    REQUIRE(args.selfDuplicate != NULL);

    CHECK(GetThreadPriority(h) == 0);
    CHECK(SetThreadPriority(h, 2));
    CHECK(GetThreadPriority(h) == 2);

    Sleep(30);
    CHECK(args.counter > 0);

    CHECK(SuspendThread(h) == 0);
    Sleep(20);
    int frozen = args.counter;
    Sleep(50);
    CHECK(args.counter == frozen);
    CHECK(SuspendThread(h) == 1);
    CHECK(ResumeThread(h) == 2);
    CHECK(ResumeThread(h) == 1);
    CHECK(ResumeThread(h) == 0);
    Sleep(50);
    CHECK(args.counter > frozen);

    args.stop = true;
    CHECK(WaitForSingleObject(h, 5000) == WAIT_OBJECT_0);
    // A handle made with DuplicateHandle names the same thread.
    CHECK(WaitForSingleObject(args.selfDuplicate, 0) == WAIT_OBJECT_0);
    CHECK(CloseHandle(args.selfDuplicate));
}

TEST_CASE("threads: a thread that suspends itself is resumed by another")
{
    ThreadArgs args;
    args.started = 0;
    args.counter = 0;
    args.stop = false;
    args.selfDuplicate = NULL;
    args.selfHandle = NULL;

    // The thread needs its own handle, which is what _beginthread returns.
    std::atomic<bool> go(false);
    struct Gate {
        static void __cdecl Run(void* p)
        {
            ThreadArgs* a = static_cast<ThreadArgs*>(p);
            while (a->selfHandle == NULL) Sleep(1);
            SelfSuspendingThread(p);
        }
    };
    HANDLE h = reinterpret_cast<HANDLE>(_beginthread(Gate::Run, 0, &args));
    args.selfHandle = h;
    while (!args.started) Sleep(1);
    Sleep(50);
    CHECK(args.counter == 0);
    CHECK(WaitForSingleObject(h, 0) == WAIT_TIMEOUT);
    CHECK(ResumeThread(h) == 1);
    CHECK(WaitForSingleObject(h, 5000) == WAIT_OBJECT_0);
    CHECK(args.counter == 1);
    (void)go;
}

namespace {
// Plain volatile fields: the project's 2-byte packing can misalign an atomic.
struct TimerState {
    volatile int ticks;
    volatile DWORD_PTR user;
    volatile UINT id;
};

void CALLBACK TimerCallback(UINT id, UINT, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
{
    TimerState* s = reinterpret_cast<TimerState*>(user);
    s->id = id;
    s->user = user;
    s->ticks = s->ticks + 1;
}
} // namespace

TEST_CASE("timers: a periodic timer calls back without drift, kill stops it")
{
    TIMECAPS caps;
    CHECK(timeGetDevCaps(&caps, sizeof(caps)) == TIMERR_NOERROR);
    CHECK(caps.wPeriodMin >= 1);
    CHECK(timeBeginPeriod(caps.wPeriodMin) == TIMERR_NOERROR);

    TimerState state;
    state.ticks = 0;
    state.user = 0;
    state.id = 0;

    DWORD t0 = GetTickCount();
    UINT id = timeSetEvent(20, 5, TimerCallback, reinterpret_cast<DWORD_PTR>(&state), TIME_PERIODIC);
    REQUIRE(id != 0);
    Sleep(1010);
    int ticks = state.ticks;
    DWORD elapsed = GetTickCount() - t0;
    // 50 per second against absolute deadlines.
    CHECK(ticks >= 40);
    CHECK(ticks <= 52);
    CHECK(elapsed >= 1000);
    CHECK(state.id == id);
    CHECK(state.user == reinterpret_cast<DWORD_PTR>(&state));

    CHECK(timeKillEvent(id) == TIMERR_NOERROR);
    int after = state.ticks;
    Sleep(100);
    CHECK(state.ticks == after);
    CHECK(timeEndPeriod(caps.wPeriodMin) == TIMERR_NOERROR);
}

TEST_CASE("timers: a one-shot timer fires once")
{
    TimerState state;
    state.ticks = 0;
    state.user = 0;
    state.id = 0;
    UINT id = timeSetEvent(20, 5, TimerCallback, reinterpret_cast<DWORD_PTR>(&state), TIME_ONESHOT);
    REQUIRE(id != 0);
    Sleep(200);
    CHECK(state.ticks == 1);
    timeKillEvent(id);
}
