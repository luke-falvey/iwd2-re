// Handles, events, semaphores, mutexes, threads, critical sections and
// interlocked operations (winbase_thread_sync.h, process.h's _beginthread).

#include "kobject.h"
#include "platform.h"

#include <win32shim/crt_msvc.h>
#include <win32shim/winbase_file.h>
#include <win32shim/winbase_thread_sync.h>

#include <errno.h>
#include <mach/mach.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#define PSEUDO_PROCESS (reinterpret_cast<HANDLE>(static_cast<intptr_t>(-1)))
#define PSEUDO_THREAD (reinterpret_cast<HANDLE>(static_cast<intptr_t>(-2)))

#define ERROR_TOO_MANY_POSTS_PORT 298L

namespace platform {

// ---------------------------------------------------------------------------
// Last error

static thread_local DWORD t_lastError = 0;

// ---------------------------------------------------------------------------
// Objects and handles

DWORD KObject::Wait(DWORD)
{
    t_lastError = ERROR_INVALID_HANDLE;
    return WAIT_FAILED;
}

KObject* ObjectFromHandle(HANDLE h, KKind kind)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(h);
    if (v == 0 || v >= static_cast<uintptr_t>(-16) || (v & 7) != 0) {
        return NULL;
    }
    KObject* obj = static_cast<KObject*>(h);
    if (obj->magic != KObject::MAGIC) {
        return NULL;
    }
    if (kind != 0 && obj->kind != kind) {
        return NULL;
    }
    return obj;
}

void AddRef(KObject* obj)
{
    obj->refs.fetch_add(1);
}

void Release(KObject* obj)
{
    if (obj->refs.fetch_sub(1) == 1) {
        delete obj;
    }
}

template <class Pred>
static DWORD WaitOn(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, DWORD ms, Pred pred)
{
    if (pred()) {
        return WAIT_OBJECT_0;
    }
    if (ms == INFINITE) {
        cv.wait(lock, pred);
        return WAIT_OBJECT_0;
    }
    if (ms == 0) {
        return WAIT_TIMEOUT;
    }
    return cv.wait_for(lock, std::chrono::milliseconds(ms), pred) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
}

struct EventObject : KObject {
    std::mutex m;
    std::condition_variable cv;
    bool manual;
    bool signaled;
    EventObject(bool man, bool sig) : KObject(K_EVENT), manual(man), signaled(sig) {}

    DWORD Wait(DWORD ms) override
    {
        std::unique_lock<std::mutex> lock(m);
        DWORD rc = WaitOn(cv, lock, ms, [this] { return signaled; });
        if (rc == WAIT_OBJECT_0 && !manual) {
            signaled = false;
        }
        return rc;
    }
};

struct SemaphoreObject : KObject {
    std::mutex m;
    std::condition_variable cv;
    LONG count;
    LONG maximum;
    SemaphoreObject(LONG c, LONG mx) : KObject(K_SEMAPHORE), count(c), maximum(mx) {}

    DWORD Wait(DWORD ms) override
    {
        std::unique_lock<std::mutex> lock(m);
        DWORD rc = WaitOn(cv, lock, ms, [this] { return count > 0; });
        if (rc == WAIT_OBJECT_0) {
            count--;
        }
        return rc;
    }
};

struct MutexObject : KObject {
    std::mutex m;
    std::condition_variable cv;
    pthread_t owner;
    int count;
    MutexObject() : KObject(K_MUTEX), owner(), count(0) {}

    DWORD Wait(DWORD ms) override
    {
        pthread_t self = pthread_self();
        std::unique_lock<std::mutex> lock(m);
        DWORD rc = WaitOn(cv, lock, ms, [this, self] { return count == 0 || pthread_equal(owner, self); });
        if (rc == WAIT_OBJECT_0) {
            owner = self;
            count++;
        }
        return rc;
    }
};

// ---------------------------------------------------------------------------
// Threads

struct ThreadState {
    pthread_t thread;
    mach_port_t port;
    DWORD id;

    std::mutex m;
    std::condition_variable cv;
    int others; // suspensions done by other threads (Mach suspend count)
    int selfHold; // 1 while the thread waits in its own SuspendThread
    int priority;
    bool done;

    ThreadState() : thread(), port(MACH_PORT_NULL), id(0), others(0), selfHold(0), priority(0), done(false) {}
};

struct ThreadObject : KObject {
    std::shared_ptr<ThreadState> state;
    bool autoHandle; // from _beginthread: closed by the runtime, not the game
    ThreadObject(std::shared_ptr<ThreadState> s, bool autoClose) : KObject(K_THREAD), state(s), autoHandle(autoClose) {}

    DWORD Wait(DWORD ms) override
    {
        std::unique_lock<std::mutex> lock(state->m);
        ThreadState* st = state.get();
        return WaitOn(st->cv, lock, ms, [st] { return st->done; });
    }
};

static std::mutex g_threadRegistryMutex;
static std::vector<std::shared_ptr<ThreadState>> g_threadRegistry;
static thread_local std::shared_ptr<ThreadState> t_state;

static DWORD CurrentThreadIdValue()
{
    uint64_t tid = 0;
    pthread_threadid_np(NULL, &tid);
    return static_cast<DWORD>(tid);
}

static std::shared_ptr<ThreadState> CurrentState()
{
    if (!t_state) {
        std::shared_ptr<ThreadState> st = std::make_shared<ThreadState>();
        st->thread = pthread_self();
        st->port = pthread_mach_thread_np(st->thread);
        st->id = CurrentThreadIdValue();
        t_state = st;
        std::lock_guard<std::mutex> lock(g_threadRegistryMutex);
        g_threadRegistry.push_back(st);
    }
    return t_state;
}

static std::shared_ptr<ThreadState> StateFromHandle(HANDLE h)
{
    if (h == PSEUDO_THREAD) {
        return CurrentState();
    }
    ThreadObject* obj = static_cast<ThreadObject*>(ObjectFromHandle(h, K_THREAD));
    if (obj == NULL) {
        return std::shared_ptr<ThreadState>();
    }
    return obj->state;
}

struct StartInfo {
    _beginthread_proc_type proc;
    void* arg;
    std::shared_ptr<ThreadState> state;
};

static void MarkDone(const std::shared_ptr<ThreadState>& st)
{
    std::lock_guard<std::mutex> lock(st->m);
    st->done = true;
    st->cv.notify_all();
}

static void* ThreadThunk(void* p)
{
    StartInfo* info = static_cast<StartInfo*>(p);
    _beginthread_proc_type proc = info->proc;
    void* arg = info->arg;
    std::shared_ptr<ThreadState> st = info->state;
    delete info;

    t_state = st;
    proc(arg);
    MarkDone(st);
    t_state.reset();
    return NULL;
}

} // namespace platform

using namespace platform;

uintptr_t _beginthread(_beginthread_proc_type start_address, unsigned stack_size, void* arglist)
{
    std::shared_ptr<ThreadState> st = std::make_shared<ThreadState>();

    StartInfo* info = new StartInfo;
    info->proc = start_address;
    info->arg = arglist;
    info->state = st;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // NOTE: Port: secondary threads get 512 KB by default on macOS, the
    // Windows default is 1 MB and the game's threads run deep AI code.
    size_t stack = stack_size > (8u << 20) ? stack_size : (8u << 20);
    pthread_attr_setstacksize(&attr, stack);

    // The thread object exists before the thread runs, as the game may use
    // the handle from inside the new thread.
    ThreadObject* obj = new ThreadObject(st, true);

    {
        std::lock_guard<std::mutex> lock(g_threadRegistryMutex);
        g_threadRegistry.push_back(st);
    }

    pthread_t th;
    st->m.lock();
    int rc = pthread_create(&th, &attr, ThreadThunk, info);
    if (rc == 0) {
        st->thread = th;
        st->port = pthread_mach_thread_np(th);
    }
    st->m.unlock();
    pthread_attr_destroy(&attr);

    if (rc != 0) {
        delete info;
        delete obj;
        errno = EAGAIN;
        return static_cast<uintptr_t>(-1);
    }

    // Never freed: the game keeps using the handle for the life of the thread
    // and `_beginthread` closes the real handle itself.
    return reinterpret_cast<uintptr_t>(obj);
}

void _endthread()
{
    std::shared_ptr<ThreadState> st = CurrentState();
    MarkDone(st);
    pthread_exit(NULL);
}

DWORD GetCurrentThreadId()
{
    return CurrentThreadIdValue();
}

DWORD GetCurrentProcessId()
{
    return static_cast<DWORD>(getpid());
}

HANDLE GetCurrentThread()
{
    return PSEUDO_THREAD;
}

HANDLE GetCurrentProcess()
{
    return PSEUDO_PROCESS;
}

BOOL SetThreadPriority(HANDLE hThread, int nPriority)
{
    std::shared_ptr<ThreadState> st = StateFromHandle(hThread);
    if (!st) {
        t_lastError = ERROR_INVALID_HANDLE;
        return FALSE;
    }
    // NOTE: Port: only recorded. macOS threads keep their default priority.
    std::lock_guard<std::mutex> lock(st->m);
    st->priority = nPriority;
    return TRUE;
}

int GetThreadPriority(HANDLE hThread)
{
    std::shared_ptr<ThreadState> st = StateFromHandle(hThread);
    if (!st) {
        t_lastError = ERROR_INVALID_HANDLE;
        return 0x7FFFFFFF; // THREAD_PRIORITY_ERROR_RETURN
    }
    std::lock_guard<std::mutex> lock(st->m);
    return st->priority;
}

DWORD SuspendThread(HANDLE hThread)
{
    std::shared_ptr<ThreadState> st = StateFromHandle(hThread);
    if (!st) {
        t_lastError = ERROR_INVALID_HANDLE;
        return static_cast<DWORD>(-1);
    }

    std::unique_lock<std::mutex> lock(st->m);
    if (st->done) {
        t_lastError = ERROR_ACCESS_DENIED;
        return static_cast<DWORD>(-1);
    }

    DWORD previous = static_cast<DWORD>(st->others + st->selfHold);

    if (pthread_equal(st->thread, pthread_self())) {
        // A thread suspending itself waits until another thread resumes it.
        // (Mach's thread_suspend on the calling thread races with the
        // resume, so the wait is done with a condition variable.)
        st->selfHold = 1;
        ThreadState* raw = st.get();
        st->cv.wait(lock, [raw] { return raw->selfHold == 0; });
        return previous;
    }

    st->others++;
    thread_suspend(st->port);
    return previous;
}

DWORD ResumeThread(HANDLE hThread)
{
    std::shared_ptr<ThreadState> st = StateFromHandle(hThread);
    if (!st) {
        t_lastError = ERROR_INVALID_HANDLE;
        return static_cast<DWORD>(-1);
    }

    std::lock_guard<std::mutex> lock(st->m);
    DWORD previous = static_cast<DWORD>(st->others + st->selfHold);
    if (previous > 0) {
        if (st->others > 0) {
            st->others--;
            thread_resume(st->port);
        } else {
            st->selfHold = 0;
            st->cv.notify_all();
        }
    }
    return previous;
}

// ---------------------------------------------------------------------------
// Handles

BOOL CloseHandle(HANDLE hObject)
{
    if (hObject == PSEUDO_PROCESS || hObject == PSEUDO_THREAD) {
        return TRUE;
    }
    KObject* obj = ObjectFromHandle(hObject);
    if (obj == NULL) {
        t_lastError = ERROR_INVALID_HANDLE;
        return FALSE;
    }
    if (obj->kind == K_THREAD && static_cast<ThreadObject*>(obj)->autoHandle) {
        return TRUE;
    }
    Release(obj);
    return TRUE;
}

DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
    KObject* obj = ObjectFromHandle(hHandle);
    if (obj == NULL) {
        t_lastError = ERROR_INVALID_HANDLE;
        return WAIT_FAILED;
    }
    return obj->Wait(dwMilliseconds);
}

BOOL DuplicateHandle(HANDLE, HANDLE hSourceHandle, HANDLE, LPHANDLE lpTargetHandle, DWORD, BOOL, DWORD)
{
    if (lpTargetHandle == NULL) {
        t_lastError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    if (hSourceHandle == PSEUDO_THREAD) {
        *lpTargetHandle = new ThreadObject(CurrentState(), false);
        return TRUE;
    }

    if (hSourceHandle == PSEUDO_PROCESS) {
        *lpTargetHandle = new KObject(K_PROCESS);
        return TRUE;
    }

    KObject* obj = ObjectFromHandle(hSourceHandle);
    if (obj == NULL) {
        t_lastError = ERROR_INVALID_HANDLE;
        return FALSE;
    }
    AddRef(obj);
    *lpTargetHandle = hSourceHandle;
    return TRUE;
}

// ---------------------------------------------------------------------------
// Events, semaphores, mutexes

HANDLE CreateEventA(LPSECURITY_ATTRIBUTES, BOOL bManualReset, BOOL bInitialState, LPCSTR)
{
    // Names are ignored: there is only one process.
    return new EventObject(bManualReset != FALSE, bInitialState != FALSE);
}

BOOL SetEvent(HANDLE hEvent)
{
    EventObject* e = static_cast<EventObject*>(ObjectFromHandle(hEvent, K_EVENT));
    if (e == NULL) {
        t_lastError = ERROR_INVALID_HANDLE;
        return FALSE;
    }
    std::lock_guard<std::mutex> lock(e->m);
    e->signaled = true;
    // An auto-reset event releases one waiter; the waiter that gets it
    // clears the flag.
    if (e->manual) {
        e->cv.notify_all();
    } else {
        e->cv.notify_one();
    }
    return TRUE;
}

BOOL ResetEvent(HANDLE hEvent)
{
    EventObject* e = static_cast<EventObject*>(ObjectFromHandle(hEvent, K_EVENT));
    if (e == NULL) {
        t_lastError = ERROR_INVALID_HANDLE;
        return FALSE;
    }
    std::lock_guard<std::mutex> lock(e->m);
    e->signaled = false;
    return TRUE;
}

HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES, LONG lInitialCount, LONG lMaximumCount, LPCSTR)
{
    if (lMaximumCount <= 0 || lInitialCount < 0 || lInitialCount > lMaximumCount) {
        t_lastError = ERROR_INVALID_PARAMETER;
        return NULL;
    }
    return new SemaphoreObject(lInitialCount, lMaximumCount);
}

BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount)
{
    SemaphoreObject* s = static_cast<SemaphoreObject*>(ObjectFromHandle(hSemaphore, K_SEMAPHORE));
    if (s == NULL) {
        t_lastError = ERROR_INVALID_HANDLE;
        return FALSE;
    }
    std::lock_guard<std::mutex> lock(s->m);
    if (lReleaseCount <= 0 || s->count > s->maximum - lReleaseCount) {
        t_lastError = ERROR_TOO_MANY_POSTS_PORT;
        return FALSE;
    }
    if (lpPreviousCount != NULL) {
        *lpPreviousCount = s->count;
    }
    s->count += lReleaseCount;
    s->cv.notify_all();
    return TRUE;
}

HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES, BOOL bInitialOwner, LPCSTR)
{
    // Names are ignored: there is only one process, so the single-instance
    // check of main.cpp always succeeds.
    MutexObject* m = new MutexObject;
    if (bInitialOwner) {
        m->owner = pthread_self();
        m->count = 1;
    }
    return m;
}

BOOL ReleaseMutex(HANDLE hMutex)
{
    MutexObject* m = static_cast<MutexObject*>(ObjectFromHandle(hMutex, K_MUTEX));
    if (m == NULL) {
        t_lastError = ERROR_INVALID_HANDLE;
        return FALSE;
    }
    std::lock_guard<std::mutex> lock(m->m);
    if (m->count == 0 || !pthread_equal(m->owner, pthread_self())) {
        t_lastError = 288; // ERROR_NOT_OWNER
        return FALSE;
    }
    if (--m->count == 0) {
        m->cv.notify_one();
    }
    return TRUE;
}

// ---------------------------------------------------------------------------
// Critical sections

static std::mutex g_csInitMutex;

static std::recursive_mutex* CsImpl(LPCRITICAL_SECTION cs)
{
    if (cs->pImpl == NULL) {
        // Not initialised (a statically allocated object that the game never
        // initialised): create it on first use.
        std::lock_guard<std::mutex> lock(g_csInitMutex);
        if (cs->pImpl == NULL) {
            cs->pImpl = new std::recursive_mutex;
        }
    }
    return static_cast<std::recursive_mutex*>(cs->pImpl);
}

void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    lpCriticalSection->pImpl = new std::recursive_mutex;
}

void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    delete static_cast<std::recursive_mutex*>(lpCriticalSection->pImpl);
    lpCriticalSection->pImpl = NULL;
}

void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    CsImpl(lpCriticalSection)->lock();
}

void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    CsImpl(lpCriticalSection)->unlock();
}

BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    return CsImpl(lpCriticalSection)->try_lock() ? TRUE : FALSE;
}

// ---------------------------------------------------------------------------
// Interlocked operations
//
// NOTE: Port: the game packs its classes to 2 bytes, so the target of an
// interlocked operation can be misaligned, and arm64 atomics fault on that.
// All of them run under one global spin lock instead. The access is a byte
// copy for the same reason.

static std::atomic_flag g_interlockedLock = ATOMIC_FLAG_INIT;

namespace {
struct InterlockedGuard {
    InterlockedGuard()
    {
        while (g_interlockedLock.test_and_set(std::memory_order_acquire)) {
            sched_yield();
        }
    }
    ~InterlockedGuard() { g_interlockedLock.clear(std::memory_order_release); }
};
} // namespace

LONG InterlockedExchange(volatile LONG* Target, LONG Value)
{
    InterlockedGuard guard;
    LONG old;
    memcpy(&old, const_cast<LONG*>(Target), sizeof(old));
    memcpy(const_cast<LONG*>(Target), &Value, sizeof(Value));
    return old;
}

LONG InterlockedIncrement(volatile LONG* lpAddend)
{
    InterlockedGuard guard;
    LONG v;
    memcpy(&v, const_cast<LONG*>(lpAddend), sizeof(v));
    v = static_cast<LONG>(static_cast<uint32_t>(v) + 1u);
    memcpy(const_cast<LONG*>(lpAddend), &v, sizeof(v));
    return v;
}

LONG InterlockedDecrement(volatile LONG* lpAddend)
{
    InterlockedGuard guard;
    LONG v;
    memcpy(&v, const_cast<LONG*>(lpAddend), sizeof(v));
    v = static_cast<LONG>(static_cast<uint32_t>(v) - 1u);
    memcpy(const_cast<LONG*>(lpAddend), &v, sizeof(v));
    return v;
}

// ---------------------------------------------------------------------------
// Last error (declared in winbase_file.h)

DWORD GetLastError()
{
    return t_lastError;
}

void SetLastError(DWORD dwErrCode)
{
    t_lastError = dwErrCode;
}
