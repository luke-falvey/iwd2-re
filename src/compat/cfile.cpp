#include "compat/afxcompat.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

// Everything in this file that touches the operating system is Win32 for
// now. A port replaces the bodies; the class interfaces stay.

// -----------------------------------------------------------------------------
// CTime
// -----------------------------------------------------------------------------

namespace {

bool LocalTime(__time64_t time, struct tm& out)
{
    return _localtime64_s(&out, &time) == 0;
}

// Local calendar date and time of a UTC file time, with the daylight-saving
// rule of that date (not of today).
bool FileTimeToLocalFields(const FILETIME& fileTime, SYSTEMTIME& st)
{
    SYSTEMTIME utc;
    return FileTimeToSystemTime(&fileTime, &utc) && SystemTimeToTzSpecificLocalTime(NULL, &utc, &st);
}

// The other direction, as MFC does it when it sets file times: through the
// local time with today's daylight-saving offset. For a date on the other
// side of a daylight-saving change the result is an hour off; kept, so files
// get the same time stamps as before.
bool TimeToFileTime(const CTime& time, FILETIME& fileTime)
{
    SYSTEMTIME st;
    FILETIME localTime;
    return time.GetAsSystemTime(st)
        && SystemTimeToFileTime(&st, &localTime)
        && LocalFileTimeToFileTime(&localTime, &fileTime);
}

CTime FileTimeToTime(const FILETIME& fileTime)
{
    SYSTEMTIME st;
    if (!FileTimeToLocalFields(fileTime, st)) {
        return CTime();
    }
    return CTime(fileTime);
}

} // namespace

CTime::CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST)
{
    // Fields outside these ranges, and dates the C runtime cannot represent
    // (before 1970), are rejected by throwing, as in MFC.
    if (nYear < 1900 || nMonth < 1 || nMonth > 12 || nDay < 1 || nDay > 31
        || nHour < 0 || nHour > 23 || nMin < 0 || nMin > 59 || nSec < 0 || nSec > 59) {
        AfxThrowInvalidArgException();
    }

    struct tm atm;
    atm.tm_sec = nSec;
    atm.tm_min = nMin;
    atm.tm_hour = nHour;
    atm.tm_mday = nDay;
    atm.tm_mon = nMonth - 1;
    atm.tm_year = nYear - 1900;
    atm.tm_wday = 0;
    atm.tm_yday = 0;
    atm.tm_isdst = nDST;
    m_time = _mktime64(&atm);
    if (m_time == -1) {
        AfxThrowInvalidArgException();
    }
}

CTime::CTime(const FILETIME& fileTime, int nDST)
{
    SYSTEMTIME st;
    if (!FileTimeToLocalFields(fileTime, st)) {
        m_time = 0;
        return;
    }
    if (st.wYear < 1900) {
        m_time = 0;
        return;
    }
    CTime time(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, nDST);
    m_time = time.m_time;
}

CTime WINAPI CTime::GetCurrentTime()
{
    return CTime(_time64(NULL));
}

int CTime::GetYear() const
{
    struct tm t;
    return LocalTime(m_time, t) ? t.tm_year + 1900 : 0;
}

int CTime::GetMonth() const
{
    struct tm t;
    return LocalTime(m_time, t) ? t.tm_mon + 1 : 0;
}

int CTime::GetDay() const
{
    struct tm t;
    return LocalTime(m_time, t) ? t.tm_mday : 0;
}

int CTime::GetHour() const
{
    struct tm t;
    return LocalTime(m_time, t) ? t.tm_hour : -1;
}

int CTime::GetMinute() const
{
    struct tm t;
    return LocalTime(m_time, t) ? t.tm_min : -1;
}

int CTime::GetSecond() const
{
    struct tm t;
    return LocalTime(m_time, t) ? t.tm_sec : -1;
}

int CTime::GetDayOfWeek() const
{
    struct tm t;
    return LocalTime(m_time, t) ? t.tm_wday + 1 : 0;
}

bool CTime::GetAsSystemTime(SYSTEMTIME& st) const
{
    struct tm t;
    if (!LocalTime(m_time, t)) {
        return false;
    }
    st.wYear = static_cast<WORD>(1900 + t.tm_year);
    st.wMonth = static_cast<WORD>(1 + t.tm_mon);
    st.wDayOfWeek = static_cast<WORD>(t.tm_wday);
    st.wDay = static_cast<WORD>(t.tm_mday);
    st.wHour = static_cast<WORD>(t.tm_hour);
    st.wMinute = static_cast<WORD>(t.tm_min);
    st.wSecond = static_cast<WORD>(t.tm_sec);
    st.wMilliseconds = 0;
    return true;
}

CString CTime::Format(const char* pszFormat) const
{
    if (pszFormat == NULL) {
        return CString();
    }

    struct tm t;
    if (!LocalTime(m_time, t)) {
        AfxThrowInvalidArgException();
    }

    // Text that does not fit in the buffer gives an empty result.
    char szBuffer[128];
    if (strftime(szBuffer, sizeof(szBuffer), pszFormat, &t) == 0) {
        szBuffer[0] = '\0';
    }
    return CString(szBuffer);
}

// -----------------------------------------------------------------------------
// CFileException
// -----------------------------------------------------------------------------

namespace {

// Which `cause` goes with which operating-system error code. Each entry
// starts a run of codes with the same cause, up to the next entry. The table
// was read off MFC by asking it for every code, so that `m_cause` agrees.
struct ErrorRun {
    LONG first;
    int cause;
};

const ErrorRun kErrorRuns[] = {
    { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 }, { 6, 2 }, { 7, 1 },
    { 11, 6 }, { 12, 5 }, { 13, 1 }, { 15, 3 }, { 16, 7 }, { 17, 3 }, { 18, 2 }, { 19, 5 },
    { 20, 10 }, { 24, 9 }, { 26, 6 }, { 27, 9 }, { 28, 1 }, { 29, 5 }, { 30, 9 }, { 31, 1 },
    { 32, 11 }, { 33, 12 }, { 34, 3 }, { 35, 1 }, { 36, 4 }, { 37, 1 }, { 38, 14 }, { 39, 13 },
    { 40, 1 }, { 52, 3 }, { 54, 5 }, { 55, 3 }, { 56, 1 }, { 57, 10 }, { 58, 5 }, { 59, 10 },
    { 60, 6 }, { 61, 1 }, { 62, 8 }, { 63, 1 }, { 64, 5 }, { 66, 6 }, { 67, 3 }, { 68, 4 },
    { 69, 1 }, { 70, 3 }, { 71, 5 }, { 72, 1 }, { 80, 5 }, { 81, 1 }, { 82, 5 }, { 83, 1 },
    { 85, 3 }, { 86, 5 }, { 87, 1 }, { 88, 10 }, { 89, 1 }, { 107, 2 }, { 108, 12 }, { 109, 1 },
    { 111, 3 }, { 112, 13 }, { 113, 4 }, { 114, 6 }, { 115, 1 }, { 117, 10 }, { 118, 1 }, { 123, 3 },
    { 126, 1 }, { 131, 9 }, { 133, 1 }, { 144, 3 }, { 145, 7 }, { 146, 1 }, { 154, 3 }, { 155, 1 },
    { 161, 3 }, { 162, 1 }, { 167, 12 }, { 168, 1 }, { 170, 5 }, { 171, 1 }, { 182, 6 }, { 183, 5 },
    { 184, 1 }, { 191, 6 }, { 192, 1 }, { 193, 6 }, { 194, 1 }, { 206, 3 }, { 207, 1 }, { 208, 3 },
    { 209, 1 }, { 267, 3 }, { 268, 1 }, { 995, 10 }, { 998, 1 }, { 999, 5 }, { 1000, 1 },
};

} // namespace

int CFileException::OsErrorToException(LONG lOsError)
{
    int cause = CFileException::genericException;
    for (const ErrorRun& run : kErrorRuns) {
        if (lOsError < run.first) {
            break;
        }
        cause = run.cause;
    }
    return cause;
}

void CFileException::ThrowOsError(LONG lOsError, const char* lpszFileName)
{
    if (lOsError != 0) {
        throw new CFileException(OsErrorToException(lOsError), lOsError, lpszFileName);
    }
}

// -----------------------------------------------------------------------------
// CFile
// -----------------------------------------------------------------------------

namespace {

// Absolute form of a path. FALSE (with the path copied as given) when it does
// not fit in `_MAX_PATH` characters.
bool FullPath(char* lpszPathOut, const char* lpszFileIn)
{
    char* lpszFilePart;
    DWORD dwRet = GetFullPathNameA(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart);
    if (dwRet == 0 || dwRet >= _MAX_PATH) {
        strncpy(lpszPathOut, lpszFileIn, _MAX_PATH - 1);
        lpszPathOut[_MAX_PATH - 1] = '\0';
        return false;
    }
    return true;
}

// Start of the file name within a path.
const char* FileNamePart(const char* lpszPath)
{
    const char* lpszName = lpszPath;
    for (const char* p = lpszPath; *p != '\0'; p = CharNextA(p)) {
        if (*p == '\\' || *p == '/' || *p == ':') {
            lpszName = p + 1;
        }
    }
    return lpszName;
}

} // namespace

const HANDLE CFile::hFileNull = INVALID_HANDLE_VALUE;

CFile::CFile()
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_bCloseOnDelete = FALSE;
    m_pTM = NULL;
}

CFile::CFile(const char* lpszFileName, UINT nOpenFlags)
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_bCloseOnDelete = FALSE;
    m_pTM = NULL;

    CFileException e;
    if (!Open(lpszFileName, nOpenFlags, &e)) {
        throw new CFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
    }
}

CFile::~CFile()
{
    if (m_hFile != INVALID_HANDLE_VALUE && m_bCloseOnDelete) {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

BOOL CFile::Open(const char* lpszFileName, UINT nOpenFlags, CFileException* pError)
{
    if (lpszFileName == NULL || strlen(lpszFileName) > _MAX_PATH) {
        if (pError != NULL) {
            pError->m_cause = CFileException::badPath;
            pError->m_lOsError = -1;
            pError->m_strFileName = lpszFileName;
        }
        return FALSE;
    }

    // The type flags only matter to text-mode classes.
    nOpenFlags &= ~static_cast<UINT>(typeBinary);

    // The name may be this object's own path, so copy and resolve it before
    // the members are reset.
    char szFileName[_MAX_PATH + 1];
    strcpy(szFileName, lpszFileName);
    lpszFileName = szFileName;

    char szFullPath[_MAX_PATH];
    bool bFullPath = FullPath(szFullPath, lpszFileName);
    LONG lFullPathError = static_cast<LONG>(GetLastError());

    m_bCloseOnDelete = FALSE;
    m_hFile = INVALID_HANDLE_VALUE;
    m_strFileName.Empty();

    if (!bFullPath) {
        if (pError != NULL) {
            pError->m_cause = CFileException::badPath;
            pError->m_lOsError = lFullPathError;
            pError->m_strFileName = lpszFileName;
        }
        return FALSE;
    }
    m_strFileName = szFullPath;

    DWORD dwAccess = 0;
    switch (nOpenFlags & 3) {
    case modeRead:
        dwAccess = GENERIC_READ;
        break;
    case modeWrite:
        dwAccess = GENERIC_WRITE;
        break;
    case modeReadWrite:
        dwAccess = GENERIC_READ | GENERIC_WRITE;
        break;
    }

    DWORD dwShareMode = 0;
    switch (nOpenFlags & 0x70) {
    case shareCompat:
    case shareExclusive:
        dwShareMode = 0;
        break;
    case shareDenyWrite:
        dwShareMode = FILE_SHARE_READ;
        break;
    case shareDenyRead:
        dwShareMode = FILE_SHARE_WRITE;
        break;
    case shareDenyNone:
        dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
        break;
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

    DWORD dwCreateFlag;
    if (nOpenFlags & modeCreate) {
        dwCreateFlag = (nOpenFlags & modeNoTruncate) ? OPEN_ALWAYS : CREATE_ALWAYS;
    } else {
        dwCreateFlag = OPEN_EXISTING;
    }

    DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
    if (nOpenFlags & osNoBuffer) {
        dwFlags |= FILE_FLAG_NO_BUFFERING;
    }
    if (nOpenFlags & osWriteThrough) {
        dwFlags |= FILE_FLAG_WRITE_THROUGH;
    }
    if (nOpenFlags & osRandomAccess) {
        dwFlags |= FILE_FLAG_RANDOM_ACCESS;
    }
    if (nOpenFlags & osSequentialScan) {
        dwFlags |= FILE_FLAG_SEQUENTIAL_SCAN;
    }

    HANDLE hFile = CreateFileA(lpszFileName, dwAccess, dwShareMode, &sa, dwCreateFlag, dwFlags, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        if (pError != NULL) {
            pError->m_lOsError = static_cast<LONG>(GetLastError());
            pError->m_cause = CFileException::OsErrorToException(pError->m_lOsError);
            pError->m_strFileName = lpszFileName;
        }
        return FALSE;
    }

    m_hFile = hFile;
    m_bCloseOnDelete = TRUE;
    return TRUE;
}

UINT CFile::Read(void* lpBuf, UINT nCount)
{
    if (nCount == 0) {
        return 0;
    }
    DWORD dwRead;
    if (!ReadFile(m_hFile, lpBuf, nCount, &dwRead, NULL)) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), m_strFileName);
    }
    return dwRead;
}

void CFile::Write(const void* lpBuf, UINT nCount)
{
    if (nCount == 0) {
        return;
    }
    DWORD nWritten;
    if (!WriteFile(m_hFile, lpBuf, nCount, &nWritten, NULL)) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), m_strFileName);
    }
    if (nWritten != nCount) {
        throw new CFileException(CFileException::diskFull, -1, m_strFileName);
    }
}

ULONGLONG CFile::Seek(LONGLONG lOff, UINT nFrom)
{
    LARGE_INTEGER liOff;
    liOff.QuadPart = lOff;
    liOff.LowPart = SetFilePointer(m_hFile, liOff.LowPart, &liOff.HighPart, nFrom);
    if (liOff.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), m_strFileName);
    }
    return liOff.QuadPart;
}

ULONGLONG CFile::GetPosition() const
{
    LARGE_INTEGER liPos;
    liPos.QuadPart = 0;
    liPos.LowPart = SetFilePointer(m_hFile, liPos.LowPart, &liPos.HighPart, FILE_CURRENT);
    if (liPos.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), m_strFileName);
    }
    return liPos.QuadPart;
}

void CFile::Flush()
{
    if (m_hFile == INVALID_HANDLE_VALUE) {
        return;
    }
    if (!FlushFileBuffers(m_hFile)) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), m_strFileName);
    }
}

void CFile::Close()
{
    BOOL bError = FALSE;
    if (m_hFile != INVALID_HANDLE_VALUE) {
        bError = !CloseHandle(m_hFile);
    }

    m_hFile = INVALID_HANDLE_VALUE;
    m_bCloseOnDelete = FALSE;
    m_strFileName.Empty();

    if (bError) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), m_strFileName);
    }
}

void CFile::Abort()
{
    if (m_hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
    m_strFileName.Empty();
}

void CFile::SetLength(ULONGLONG dwNewLen)
{
    Seek(static_cast<LONGLONG>(dwNewLen), CFile::begin);
    if (!SetEndOfFile(m_hFile)) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), m_strFileName);
    }
}

ULONGLONG CFile::GetLength() const
{
    ULARGE_INTEGER liSize;
    liSize.LowPart = GetFileSize(m_hFile, &liSize.HighPart);
    if (liSize.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), m_strFileName);
    }
    return liSize.QuadPart;
}

CString CFile::GetFileName() const
{
    return CString(FileNamePart(m_strFileName));
}

CString CFile::GetFileTitle() const
{
    return CString(FileNamePart(m_strFileName));
}

CString CFile::GetFilePath() const
{
    return m_strFileName;
}

void CFile::SetFilePath(const char* lpszNewName)
{
    m_strFileName = lpszNewName;
}

void PASCAL CFile::Rename(const char* lpszOldName, const char* lpszNewName)
{
    if (!MoveFileA(lpszOldName, lpszNewName)) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), lpszOldName);
    }
}

void PASCAL CFile::Remove(const char* lpszFileName)
{
    if (!DeleteFileA(lpszFileName)) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), lpszFileName);
    }
}

BOOL CFile::GetStatus(CFileStatus& rStatus) const
{
    memset(&rStatus, 0, sizeof(CFileStatus));

    strncpy(rStatus.m_szFullName, m_strFileName, _MAX_PATH - 1);
    rStatus.m_szFullName[_MAX_PATH - 1] = '\0';

    if (m_hFile != INVALID_HANDLE_VALUE) {
        FILETIME ftCreate, ftAccess, ftModify;
        if (!GetFileTime(m_hFile, &ftCreate, &ftAccess, &ftModify)) {
            return FALSE;
        }

        ULARGE_INTEGER liSize;
        liSize.LowPart = GetFileSize(m_hFile, &liSize.HighPart);
        if (liSize.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
            return FALSE;
        }
        rStatus.m_size = liSize.QuadPart;

        if (m_strFileName.IsEmpty()) {
            rStatus.m_attribute = 0;
        } else {
            DWORD dwAttribute = GetFileAttributesA(m_strFileName);
            // Only the low byte holds the classic attribute bits.
            rStatus.m_attribute = dwAttribute == INVALID_FILE_ATTRIBUTES ? 0 : static_cast<BYTE>(dwAttribute);
        }

        rStatus.m_ctime = FileTimeToTime(ftCreate);
        rStatus.m_atime = FileTimeToTime(ftAccess);
        rStatus.m_mtime = FileTimeToTime(ftModify);

        if (rStatus.m_ctime.GetTime() == 0) {
            rStatus.m_ctime = rStatus.m_mtime;
        }
        if (rStatus.m_atime.GetTime() == 0) {
            rStatus.m_atime = rStatus.m_mtime;
        }
    }
    return TRUE;
}

BOOL PASCAL CFile::GetStatus(const char* lpszFileName, CFileStatus& rStatus)
{
    if (lpszFileName == NULL || strlen(lpszFileName) >= _MAX_PATH) {
        return FALSE;
    }

    if (!FullPath(rStatus.m_szFullName, lpszFileName)) {
        rStatus.m_szFullName[0] = '\0';
        return FALSE;
    }

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExA(lpszFileName, GetFileExInfoStandard, &data)) {
        return FALSE;
    }

    // "Normal" is the absence of the other bits here.
    rStatus.m_attribute = static_cast<BYTE>(data.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);
    rStatus.m_size = (static_cast<ULONGLONG>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;

    rStatus.m_ctime = FileTimeToTime(data.ftCreationTime);
    rStatus.m_atime = FileTimeToTime(data.ftLastAccessTime);
    rStatus.m_mtime = FileTimeToTime(data.ftLastWriteTime);

    if (rStatus.m_ctime.GetTime() == 0) {
        rStatus.m_ctime = rStatus.m_mtime;
    }
    if (rStatus.m_atime.GetTime() == 0) {
        rStatus.m_atime = rStatus.m_mtime;
    }
    return TRUE;
}

void PASCAL CFile::SetStatus(const char* lpszFileName, const CFileStatus& status)
{
    DWORD wAttr = GetFileAttributesA(lpszFileName);
    if (wAttr == INVALID_FILE_ATTRIBUTES) {
        CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), lpszFileName);
    }

    // A read-only file has to be made writable before its times can change.
    if (status.m_attribute != wAttr && (wAttr & FILE_ATTRIBUTE_READONLY)) {
        if (!SetFileAttributesA(lpszFileName, status.m_attribute)) {
            CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), lpszFileName);
        }
    }

    if (status.m_mtime.GetTime() != 0) {
        FILETIME lastWriteTime;
        FILETIME lastAccessTime;
        FILETIME creationTime;
        FILETIME* lpLastWriteTime = NULL;
        FILETIME* lpLastAccessTime = NULL;
        FILETIME* lpCreationTime = NULL;

        if (TimeToFileTime(status.m_mtime, lastWriteTime)) {
            lpLastWriteTime = &lastWriteTime;
        }
        if (status.m_atime.GetTime() != 0 && TimeToFileTime(status.m_atime, lastAccessTime)) {
            lpLastAccessTime = &lastAccessTime;
        }
        if (status.m_ctime.GetTime() != 0 && TimeToFileTime(status.m_ctime, creationTime)) {
            lpCreationTime = &creationTime;
        }

        HANDLE hFile = CreateFileA(lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), lpszFileName);
        }

        if (!SetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime)) {
            LONG lOsError = static_cast<LONG>(GetLastError());
            CloseHandle(hFile);
            CFileException::ThrowOsError(lOsError, lpszFileName);
        }

        if (!CloseHandle(hFile)) {
            CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), lpszFileName);
        }
    }

    if (status.m_attribute != wAttr && !(wAttr & FILE_ATTRIBUTE_READONLY)) {
        if (!SetFileAttributesA(lpszFileName, status.m_attribute)) {
            CFileException::ThrowOsError(static_cast<LONG>(GetLastError()), lpszFileName);
        }
    }
}

// -----------------------------------------------------------------------------
// CFileFind
// -----------------------------------------------------------------------------

CFileFind::CFileFind()
{
    m_pFoundInfo = NULL;
    m_pNextInfo = NULL;
    m_hContext = NULL;
    m_chDirSeparator = '\\';
    m_pTM = NULL;
}

CFileFind::~CFileFind()
{
    Close();
}

void CFileFind::Close()
{
    if (m_pFoundInfo != NULL) {
        delete static_cast<WIN32_FIND_DATAA*>(m_pFoundInfo);
        m_pFoundInfo = NULL;
    }
    if (m_pNextInfo != NULL) {
        delete static_cast<WIN32_FIND_DATAA*>(m_pNextInfo);
        m_pNextInfo = NULL;
    }
    if (m_hContext != NULL) {
        if (m_hContext != INVALID_HANDLE_VALUE) {
            FindClose(m_hContext);
        }
        m_hContext = NULL;
    }
}

BOOL CFileFind::FindFile(const char* pstrName, DWORD dwUnused)
{
    (void)dwUnused;
    Close();

    if (pstrName == NULL) {
        pstrName = "*.*";
    } else if (strlen(pstrName) >= _MAX_PATH) {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    WIN32_FIND_DATAA* pFindData = new WIN32_FIND_DATAA;
    m_pNextInfo = pFindData;
    strcpy(pFindData->cFileName, pstrName);

    m_hContext = FindFirstFileA(pstrName, pFindData);
    if (m_hContext == INVALID_HANDLE_VALUE) {
        DWORD dwTemp = GetLastError();
        Close();
        SetLastError(dwTemp);
        return FALSE;
    }

    // The root is the absolute directory of the pattern, with its trailing
    // separator.
    char szRoot[_MAX_PATH];
    if (_fullpath(szRoot, pstrName, _MAX_PATH) == NULL) {
        Close();
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    _splitpath_s(szRoot, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);
    _makepath_s(szRoot, _MAX_PATH, szDrive, szDir, NULL, NULL);
    m_strRoot = szRoot;

    return TRUE;
}

BOOL CFileFind::FindNextFile()
{
    if (m_hContext == NULL) {
        return FALSE;
    }
    if (m_pFoundInfo == NULL) {
        m_pFoundInfo = new WIN32_FIND_DATAA;
    }

    void* pTemp = m_pFoundInfo;
    m_pFoundInfo = m_pNextInfo;
    m_pNextInfo = pTemp;

    return ::FindNextFileA(m_hContext, static_cast<WIN32_FIND_DATAA*>(m_pNextInfo));
}

ULONGLONG CFileFind::GetLength() const
{
    if (m_pFoundInfo == NULL) {
        return 0;
    }
    const WIN32_FIND_DATAA* pData = static_cast<const WIN32_FIND_DATAA*>(m_pFoundInfo);
    return (static_cast<ULONGLONG>(pData->nFileSizeHigh) << 32) | pData->nFileSizeLow;
}

CString CFileFind::GetFileName() const
{
    CString ret;
    if (m_pFoundInfo != NULL) {
        ret = static_cast<const WIN32_FIND_DATAA*>(m_pFoundInfo)->cFileName;
    }
    return ret;
}

CString CFileFind::GetFilePath() const
{
    // Without a search there is no path; MFC throws here too.
    if (m_hContext == NULL) {
        AfxThrowInvalidArgException();
    }

    CString strResult = m_strRoot;
    int nLength = strResult.GetLength();
    if (nLength == 0 || (strResult[nLength - 1] != '\\' && strResult[nLength - 1] != '/')) {
        strResult += m_chDirSeparator;
    }
    strResult += GetFileName();
    return strResult;
}

CString CFileFind::GetFileTitle() const
{
    CString strFullName = GetFileName();
    char szTitle[_MAX_FNAME];
    szTitle[0] = '\0';
    _splitpath_s(strFullName, NULL, 0, NULL, 0, szTitle, _MAX_FNAME, NULL, 0);
    return CString(szTitle);
}

CString CFileFind::GetRoot() const
{
    return m_strRoot;
}

BOOL CFileFind::GetLastWriteTime(CTime& refTime) const
{
    if (m_pFoundInfo == NULL) {
        return FALSE;
    }
    refTime = FileTimeToTime(static_cast<const WIN32_FIND_DATAA*>(m_pFoundInfo)->ftLastWriteTime);
    return TRUE;
}

BOOL CFileFind::MatchesMask(DWORD dwMask) const
{
    if (m_pFoundInfo == NULL) {
        return FALSE;
    }
    return (static_cast<const WIN32_FIND_DATAA*>(m_pFoundInfo)->dwFileAttributes & dwMask) != 0;
}

BOOL CFileFind::IsDots() const
{
    if (m_pFoundInfo == NULL || !IsDirectory()) {
        return FALSE;
    }
    const char* pszName = static_cast<const WIN32_FIND_DATAA*>(m_pFoundInfo)->cFileName;
    if (pszName[0] != '.') {
        return FALSE;
    }
    return pszName[1] == '\0' || (pszName[1] == '.' && pszName[2] == '\0');
}
