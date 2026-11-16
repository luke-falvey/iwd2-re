#ifndef IWD2RE_COMPAT_CFILE_H_
#define IWD2RE_COMPAT_CFILE_H_

// Replacements for MFC's file classes: `CFile`, `CFileStatus`, `CFileFind`,
// `CFileException`, plus `CTime`, which they need.
//
// Members are those of MFC in the same order (the game derives from `CFile`
// and holds these objects by value). The platform calls are all in cfile.cpp.

// A point in time, in seconds since 1970-01-01 UTC. Fields are local time.
class CTime {
public:
    CTime() { m_time = 0; }
    CTime(__time64_t time) { m_time = time; }
    // Local date and time. `nDST`: 0 standard time, positive daylight
    // saving time, negative works it out. Throws for fields out of range
    // and for dates before 1970.
    CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST = -1);
    // From a UTC file time.
    CTime(const FILETIME& fileTime, int nDST = -1);

    static CTime WINAPI GetCurrentTime();

    __time64_t GetTime() const { return m_time; }
    int GetYear() const;
    int GetMonth() const; // 1 to 12
    int GetDay() const; // 1 to 31
    int GetHour() const;
    int GetMinute() const;
    int GetSecond() const;
    int GetDayOfWeek() const; // 1 = Sunday

    // The local time as a file-system time record. FALSE when out of range.
    bool GetAsSystemTime(SYSTEMTIME& st) const;

    // `strftime` format of the local time. Empty when the text would be
    // longer than 127 characters.
    CString Format(const char* pszFormat) const;

    bool operator==(CTime time) const { return m_time == time.m_time; }
    bool operator!=(CTime time) const { return m_time != time.m_time; }
    bool operator<(CTime time) const { return m_time < time.m_time; }
    bool operator>(CTime time) const { return m_time > time.m_time; }
    bool operator<=(CTime time) const { return m_time <= time.m_time; }
    bool operator>=(CTime time) const { return m_time >= time.m_time; }

private:
    __time64_t m_time;
};

class CFileException : public CException {
public:
    enum {
        none,
        genericException,
        fileNotFound,
        badPath,
        tooManyOpenFiles,
        accessDenied,
        invalidFile,
        removeCurrentDir,
        directoryFull,
        badSeek,
        hardIO,
        sharingViolation,
        lockViolation,
        diskFull,
        endOfFile
    };

    CFileException(int cause = CFileException::none, LONG lOsError = -1, const char* lpszArchiveName = NULL)
    {
        m_cause = cause;
        m_lOsError = lOsError;
        m_strFileName = lpszArchiveName;
    }

    ~CFileException() override { }

    // The `cause` value for an operating-system error code.
    static int OsErrorToException(LONG lOsError);
    // Throws a `CFileException*` for the error code.
    static void ThrowOsError(LONG lOsError, const char* lpszFileName = NULL);

    int m_cause;
    LONG m_lOsError;
    CString m_strFileName;
};

struct CFileStatus {
    CTime m_ctime; // creation
    CTime m_mtime; // last change
    CTime m_atime; // last access
    ULONGLONG m_size; // bytes
    DWORD m_attribute; // `CFile::Attribute` bits
    char m_szFullName[_MAX_PATH]; // absolute path
};

class CFile : public CObject {
public:
    enum OpenFlags {
        modeRead = 0x00000,
        modeWrite = 0x00001,
        modeReadWrite = 0x00002,
        shareCompat = 0x00000,
        shareExclusive = 0x00010,
        shareDenyWrite = 0x00020,
        shareDenyRead = 0x00030,
        shareDenyNone = 0x00040,
        modeNoInherit = 0x00080,
        typeUnicode = 0x00400,
        modeCreate = 0x01000,
        modeNoTruncate = 0x02000,
        typeText = 0x04000,
        typeBinary = 0x08000,
        osNoBuffer = 0x10000,
        osWriteThrough = 0x20000,
        osRandomAccess = 0x40000,
        osSequentialScan = 0x80000,
    };

    enum Attribute {
        normal = 0x00,
        readOnly = 0x01,
        hidden = 0x02,
        system = 0x04,
        volume = 0x08,
        directory = 0x10,
        archive = 0x20,
    };

    enum SeekPosition {
        begin = 0x0,
        current = 0x1,
        end = 0x2,
    };

    static const HANDLE hFileNull;

    CFile();
    // Opens the file; throws a `CFileException*` when that fails.
    CFile(const char* lpszFileName, UINT nOpenFlags);
    ~CFile() override;

    HANDLE m_hFile;

    virtual ULONGLONG GetPosition() const;
    // The file's attributes, size and times. FALSE when no file is open.
    BOOL GetStatus(CFileStatus& rStatus) const;
    virtual CString GetFileName() const; // name with extension, no directory
    virtual CString GetFileTitle() const;
    virtual CString GetFilePath() const; // full path
    virtual void SetFilePath(const char* lpszNewName);

    // Opens the file. Returns FALSE on failure and, when `pError` is given,
    // describes the failure there. Does not throw.
    virtual BOOL Open(const char* lpszFileName, UINT nOpenFlags, CFileException* pError = NULL);

    static void PASCAL Rename(const char* lpszOldName, const char* lpszNewName);
    static void PASCAL Remove(const char* lpszFileName);
    // FALSE when the file or directory does not exist.
    static BOOL PASCAL GetStatus(const char* lpszFileName, CFileStatus& rStatus);
    // Applies the times and attributes of `status` to the file.
    static void PASCAL SetStatus(const char* lpszFileName, const CFileStatus& status);

    ULONGLONG SeekToEnd() { return Seek(0, CFile::end); }
    void SeekToBegin() { Seek(0, CFile::begin); }

    // These throw a `CFileException*` when the operating system reports an
    // error. `Read` returns the number of bytes read, which is less than
    // asked for at the end of the file.
    virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
    virtual void SetLength(ULONGLONG dwNewLen);
    virtual ULONGLONG GetLength() const;
    virtual UINT Read(void* lpBuf, UINT nCount);
    virtual void Write(const void* lpBuf, UINT nCount);
    virtual void Flush();
    virtual void Close();
    // Closes without reporting errors.
    virtual void Abort();

protected:
    BOOL m_bCloseOnDelete;
    CString m_strFileName;
    void* m_pTM; // unused; keeps MFC's size
};

// Directory search with wildcards. After `FindFile` succeeds, each
// `FindNextFile` call makes the next match current; its result says whether
// more matches follow.
class CFileFind : public CObject {
public:
    CFileFind();
    ~CFileFind() override;

    ULONGLONG GetLength() const;
    virtual CString GetFileName() const;
    virtual CString GetFilePath() const;
    virtual CString GetFileTitle() const;
    virtual CString GetRoot() const;

    virtual BOOL GetLastWriteTime(CTime& refTime) const;

    virtual BOOL MatchesMask(DWORD dwMask) const;
    virtual BOOL IsDots() const;
    BOOL IsReadOnly() const { return MatchesMask(0x01); }
    BOOL IsDirectory() const { return MatchesMask(0x10); }
    BOOL IsHidden() const { return MatchesMask(0x02); }
    BOOL IsSystem() const { return MatchesMask(0x04); }
    BOOL IsArchived() const { return MatchesMask(0x20); }

    void Close();
    virtual BOOL FindFile(const char* pstrName = NULL, DWORD dwUnused = 0);
    virtual BOOL FindNextFile();

protected:
    void* m_pFoundInfo;
    void* m_pNextInfo;
    HANDLE m_hContext;
    CString m_strRoot;
    char m_chDirSeparator;
    void* m_pTM; // unused; keeps MFC's size
};

#endif /* IWD2RE_COMPAT_CFILE_H_ */
