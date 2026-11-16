#ifndef IWD2RE_COMPAT_OBJECT_H_
#define IWD2RE_COMPAT_OBJECT_H_

// Root class and exceptions of the MFC replacement.

struct CRuntimeClass;
class CArchive;
class CDumpContext;

// Root of the MFC class tree. Only the virtual table matters here; the
// virtual functions are declared in MFC's order and do nothing.
class CObject {
public:
    virtual CRuntimeClass* GetRuntimeClass() const { return NULL; }
    virtual ~CObject() { }
    virtual void Serialize(CArchive&) { }
    virtual void AssertValid() const { }
    virtual void Dump(CDumpContext&) const { }

protected:
    CObject() { }

private:
    CObject(const CObject&) = delete;
    void operator=(const CObject&) = delete;
};

// Base of exceptions that are thrown as pointers and released by the
// handler with `Delete()`:
//
//     try { ... } catch (CException* e) { e->Delete(); }
class CException : public CObject {
public:
    CException() { m_bAutoDelete = TRUE; }
    explicit CException(BOOL bAutoDelete) { m_bAutoDelete = bAutoDelete; }

    void Delete()
    {
        if (m_bAutoDelete > 0) {
            delete this;
        }
    }

    virtual BOOL GetErrorMessage(char* lpszError, UINT nMaxError, UINT* pnHelpContext = NULL) const
    {
        if (pnHelpContext != NULL) {
            *pnHelpContext = 0;
        }
        if (nMaxError != 0 && lpszError != NULL) {
            *lpszError = '\0';
        }
        return FALSE;
    }

    ~CException() override { }

    BOOL m_bAutoDelete;
};

// Thrown for arguments that MFC rejects, such as an array index out of
// range. Game code does not rely on these; they are here so a mistake fails
// the same way with and without MFC.
class CInvalidArgException : public CException {
public:
    CInvalidArgException() { }
    explicit CInvalidArgException(BOOL bAutoDelete)
        : CException(bAutoDelete)
    {
    }
};

// Throws a shared `CInvalidArgException` (collections do this).
[[noreturn]] void AfxThrowInvalidArgException();

// Throws a `CInvalidArgException` of its own (strings do this; MFC throws
// another `CException` class there).
[[noreturn]] void AfxThrowInvalidStringArgException();

#endif /* IWD2RE_COMPAT_OBJECT_H_ */
