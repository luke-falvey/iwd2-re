#ifndef WIN32SHIM_WINBASE_THREAD_SYNC_H_
#define WIN32SHIM_WINBASE_THREAD_SYNC_H_
#include <win32shim/wintypes.h>

// Threads, events, semaphores, mutexes, critical sections, interlocked ops.

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

// The real mutex lives on the heap; the structure only holds its address.
typedef struct _CRITICAL_SECTION {
    void* pImpl;
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

#define WAIT_OBJECT_0 ((DWORD)0x00000000)
#define WAIT_ABANDONED ((DWORD)0x00000080)
#define WAIT_TIMEOUT ((DWORD)0x00000102)
#define WAIT_FAILED ((DWORD)0xFFFFFFFF)

#define DUPLICATE_SAME_ACCESS 0x00000002

BOOL CloseHandle(HANDLE hObject);
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
BOOL DuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwOptions);

void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

HANDLE GetCurrentThread();
DWORD GetCurrentThreadId();
HANDLE GetCurrentProcess();
DWORD GetCurrentProcessId();
BOOL SetThreadPriority(HANDLE hThread, int nPriority);
int GetThreadPriority(HANDLE hThread);
DWORD SuspendThread(HANDLE hThread);
DWORD ResumeThread(HANDLE hThread);

HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName);
BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount);
HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName);
BOOL ReleaseMutex(HANDLE hMutex);
BOOL ResetEvent(HANDLE hEvent);
BOOL SetEvent(HANDLE hEvent);

LONG InterlockedExchange(volatile LONG* Target, LONG Value);
LONG InterlockedIncrement(volatile LONG* lpAddend);
LONG InterlockedDecrement(volatile LONG* lpAddend);

HANDLE CreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName);

#endif /* WIN32SHIM_WINBASE_THREAD_SYNC_H_ */
