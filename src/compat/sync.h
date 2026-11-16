#ifndef IWD2RE_COMPAT_SYNC_H_
#define IWD2RE_COMPAT_SYNC_H_

// Replacements for MFC's `CCriticalSection` and `CSingleLock`.
//
// `CCriticalSection` keeps MFC's members (the game holds these by value):
// the `CSyncObject` base with its unused handle, then the lock itself.

class CSyncObject : public CObject {
public:
    explicit CSyncObject(const char* pstrName)
    {
        (void)pstrName;
        m_hObject = NULL;
    }

    operator HANDLE() const { return m_hObject; }
    HANDLE m_hObject;

    virtual BOOL Lock(DWORD dwTimeout = INFINITE)
    {
        DWORD dwRet = ::WaitForSingleObject(m_hObject, dwTimeout);
        return dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED;
    }
    virtual BOOL Unlock() = 0;
    virtual BOOL Unlock(LONG /* lCount */, LONG* /* lpPrevCount */) { return TRUE; }

    ~CSyncObject() override
    {
        if (m_hObject != NULL) {
            ::CloseHandle(m_hObject);
            m_hObject = NULL;
        }
    }
};

// A lock that one thread at a time can hold. The holder may take it again;
// it must release it as many times.
class CCriticalSection : public CSyncObject {
public:
    CCriticalSection()
        : CSyncObject(NULL)
    {
        ::InitializeCriticalSection(&m_sect);
    }

    operator CRITICAL_SECTION*() { return &m_sect; }
    CRITICAL_SECTION m_sect;

    BOOL Unlock() override
    {
        ::LeaveCriticalSection(&m_sect);
        return TRUE;
    }
    BOOL Lock()
    {
        ::EnterCriticalSection(&m_sect);
        return TRUE;
    }
    // The timeout is not used: this always waits until the lock is free.
    BOOL Lock(DWORD /* dwTimeout */) override { return Lock(); }

    ~CCriticalSection() override { ::DeleteCriticalSection(&m_sect); }
};

// Holds a lock on a sync object for a scope; released by the destructor.
class CSingleLock {
public:
    explicit CSingleLock(CSyncObject* pObject, BOOL bInitialLock = FALSE)
    {
        m_pObject = pObject;
        m_hObject = pObject->m_hObject;
        m_bAcquired = FALSE;
        if (bInitialLock) {
            Lock();
        }
    }

    BOOL Lock(DWORD dwTimeOut = INFINITE)
    {
        m_bAcquired = m_pObject->Lock(dwTimeOut);
        return m_bAcquired;
    }

    BOOL Unlock()
    {
        if (m_bAcquired) {
            m_bAcquired = !m_pObject->Unlock();
        }
        // TRUE when the lock is not held any more.
        return !m_bAcquired;
    }

    BOOL Unlock(LONG lCount, LONG* lpPrevCount = NULL)
    {
        if (m_bAcquired) {
            m_bAcquired = !m_pObject->Unlock(lCount, lpPrevCount);
        }
        return !m_bAcquired;
    }

    BOOL IsLocked() { return m_bAcquired; }

    ~CSingleLock() { Unlock(); }

protected:
    CSyncObject* m_pObject;
    HANDLE m_hObject;
    BOOL m_bAcquired;
};

#endif /* IWD2RE_COMPAT_SYNC_H_ */
