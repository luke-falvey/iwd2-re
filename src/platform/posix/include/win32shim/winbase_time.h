#ifndef WIN32SHIM_WINBASE_TIME_H_
#define WIN32SHIM_WINBASE_TIME_H_
#include <win32shim/wintypes.h>

// Time: tick counts, sleeping, system and file times.

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

DWORD GetTickCount();
void Sleep(DWORD dwMilliseconds);
DWORD SleepEx(DWORD dwMilliseconds, BOOL bAlertable);

BOOL FileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime);
BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime);
BOOL LocalFileTimeToFileTime(const FILETIME* lpLocalFileTime, LPFILETIME lpFileTime);
BOOL SystemTimeToTzSpecificLocalTime(const void* lpTimeZoneInformation, const SYSTEMTIME* lpUniversalTime, LPSYSTEMTIME lpLocalTime);

#endif /* WIN32SHIM_WINBASE_TIME_H_ */
