// Files, directories, find-file, attributes, drives (winbase_file.h).

#include "kobject.h"
#include "path.h"
#include "platform.h"

#include <win32shim/winbase_file.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

#define ERROR_INSUFFICIENT_BUFFER_PORT 122L

namespace platform {

uint32_t ErrnoToWin32(int err)
{
    switch (err) {
    case 0:
        return ERROR_SUCCESS;
    case ENOENT:
        return ERROR_FILE_NOT_FOUND;
    case ENOTDIR:
        return ERROR_PATH_NOT_FOUND;
    case EACCES:
    case EPERM:
    case EROFS:
    case EISDIR:
    case EBUSY:
        return ERROR_ACCESS_DENIED;
    case EEXIST:
        return ERROR_ALREADY_EXISTS;
    case ENOTEMPTY:
        return ERROR_DIR_NOT_EMPTY;
    case ENOSPC:
    case EDQUOT:
        return ERROR_DISK_FULL;
    case EMFILE:
    case ENFILE:
        return ERROR_TOO_MANY_OPEN_FILES;
    case ENAMETOOLONG:
        return ERROR_INVALID_NAME;
    case EINVAL:
        return ERROR_INVALID_PARAMETER;
    case EBADF:
        return ERROR_INVALID_HANDLE;
    default:
        return ERROR_ACCESS_DENIED;
    }
}

void SetLastErrorFromErrno()
{
    SetLastError(ErrnoToWin32(errno));
}

// ENOENT means "file not found" if the directory is there, "path not found"
// if it is not.
static uint32_t NotFoundCode(const std::string& path)
{
    size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) {
        return ERROR_FILE_NOT_FOUND;
    }
    std::string dir = slash == 0 ? "/" : path.substr(0, slash);
    struct stat st;
    if (stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        return ERROR_FILE_NOT_FOUND;
    }
    return ERROR_PATH_NOT_FOUND;
}

static void SetErrorForPath(int err, const std::string& path)
{
    if (err == ENOENT) {
        SetLastError(NotFoundCode(path));
    } else {
        SetLastError(ErrnoToWin32(err));
    }
}

static void FillFileTimes(const struct stat& st, uint64_t& create, uint64_t& access, uint64_t& write)
{
    create = UnixToFileTime(st.st_birthtimespec.tv_sec, st.st_birthtimespec.tv_nsec);
    access = UnixToFileTime(st.st_atimespec.tv_sec, st.st_atimespec.tv_nsec);
    write = UnixToFileTime(st.st_mtimespec.tv_sec, st.st_mtimespec.tv_nsec);
}

static void ToFileTime(uint64_t v, FILETIME* ft)
{
    ft->dwLowDateTime = static_cast<DWORD>(v & 0xFFFFFFFFu);
    ft->dwHighDateTime = static_cast<DWORD>(v >> 32);
}

static DWORD AttributesFromStat(const struct stat& st)
{
    if (S_ISDIR(st.st_mode)) {
        return FILE_ATTRIBUTE_DIRECTORY;
    }
    DWORD attr = FILE_ATTRIBUTE_ARCHIVE;
    if ((st.st_mode & S_IWUSR) == 0) {
        attr |= FILE_ATTRIBUTE_READONLY;
    }
    return attr;
}

// Share modes: Windows refuses an open that conflicts with what the open
// handles of the same file allow. Only handles of this process exist here, so
// a table of them is enough.
static std::mutex g_shareMutex;
static std::vector<FileObject*> g_openFiles;

static bool ShareConflict(uint64_t dev, uint64_t ino, bool reads, bool writes, DWORD share)
{
    for (size_t i = 0; i < g_openFiles.size(); i++) {
        const FileObject* e = g_openFiles[i];
        if (e->dev != dev || e->ino != ino) {
            continue;
        }
        if ((reads && !(e->shareMode & FILE_SHARE_READ)) || (writes && !(e->shareMode & FILE_SHARE_WRITE))
            || (e->reads && !(share & FILE_SHARE_READ)) || (e->writes && !(share & FILE_SHARE_WRITE))) {
            return true;
        }
    }
    return false;
}

// True when a handle without FILE_SHARE_DELETE is open on the file.
static bool OpenWithoutDeleteShare(const struct stat& st)
{
    std::lock_guard<std::mutex> lock(g_shareMutex);
    for (size_t i = 0; i < g_openFiles.size(); i++) {
        const FileObject* e = g_openFiles[i];
        if (e->dev == static_cast<uint64_t>(st.st_dev) && e->ino == static_cast<uint64_t>(st.st_ino)
            && (e->shareMode & FILE_SHARE_DELETE) == 0) {
            return true;
        }
    }
    return false;
}

FileObject::~FileObject()
{
    if (registered) {
        std::lock_guard<std::mutex> lock(g_shareMutex);
        g_openFiles.erase(std::remove(g_openFiles.begin(), g_openFiles.end(), this), g_openFiles.end());
    }
    if (fd >= 0) {
        close(fd);
    }
}

} // namespace platform

using namespace platform;

// ---------------------------------------------------------------------------
// Handles

HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE)
{
    if (lpFileName == NULL) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    std::string path = TranslatePath(lpFileName);

    bool wantRead = (dwDesiredAccess & GENERIC_READ) != 0;
    bool wantWrite = (dwDesiredAccess & GENERIC_WRITE) != 0;
    bool wantAppend = (dwDesiredAccess & FILE_APPEND_DATA) != 0 && !wantWrite;

    int oflags;
    if (wantRead && (wantWrite || wantAppend)) {
        oflags = O_RDWR;
    } else if (wantWrite || wantAppend) {
        oflags = O_WRONLY;
    } else {
        oflags = O_RDONLY;
    }
    if (wantAppend) {
        oflags |= O_APPEND;
    }

    struct stat before;
    bool existed = stat(path.c_str(), &before) == 0;

    switch (dwCreationDisposition) {
    case CREATE_NEW:
        oflags |= O_CREAT | O_EXCL;
        break;
    case CREATE_ALWAYS:
        oflags |= O_CREAT | O_TRUNC;
        break;
    case OPEN_EXISTING:
        break;
    case OPEN_ALWAYS:
        oflags |= O_CREAT;
        break;
    case TRUNCATE_EXISTING:
        oflags |= O_TRUNC;
        break;
    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }
    if ((oflags & O_TRUNC) && (oflags & O_ACCMODE) == O_RDONLY) {
        oflags = (oflags & ~O_ACCMODE) | O_RDWR;
    }

    if (existed && S_ISDIR(before.st_mode)) {
        SetLastError(dwCreationDisposition == CREATE_NEW ? ERROR_FILE_EXISTS : ERROR_ACCESS_DENIED);
        return INVALID_HANDLE_VALUE;
    }

    bool reads = wantRead;
    bool writes = wantWrite || wantAppend;

    // The check and the open are one step, so two opens cannot both pass.
    std::lock_guard<std::mutex> shareLock(g_shareMutex);
    if (existed && ShareConflict(static_cast<uint64_t>(before.st_dev), static_cast<uint64_t>(before.st_ino), reads, writes, dwShareMode)) {
        SetLastError(ERROR_SHARING_VIOLATION);
        return INVALID_HANDLE_VALUE;
    }

    mode_t mode = (dwFlagsAndAttributes & FILE_ATTRIBUTE_READONLY) ? 0444 : 0666;
    int fd = open(path.c_str(), oflags | O_CLOEXEC, mode);
    if (fd < 0) {
        int err = errno;
        if (err == EEXIST) {
            SetLastError(ERROR_FILE_EXISTS);
        } else {
            SetErrorForPath(err, path);
        }
        return INVALID_HANDLE_VALUE;
    }

    FileObject* obj = new FileObject(fd);
    obj->append = wantAppend;
    struct stat after;
    if (fstat(fd, &after) == 0) {
        obj->dev = static_cast<uint64_t>(after.st_dev);
        obj->ino = static_cast<uint64_t>(after.st_ino);
    }
    obj->reads = reads;
    obj->writes = writes;
    obj->shareMode = dwShareMode;
    obj->registered = true;
    g_openFiles.push_back(obj);

    if (existed && (dwCreationDisposition == CREATE_ALWAYS || dwCreationDisposition == OPEN_ALWAYS)) {
        SetLastError(ERROR_ALREADY_EXISTS);
    } else {
        SetLastError(ERROR_SUCCESS);
    }
    return obj;
}

static FileObject* FileFromHandle(HANDLE h)
{
    FileObject* f = static_cast<FileObject*>(ObjectFromHandle(h, K_FILE));
    if (f == NULL) {
        SetLastError(ERROR_INVALID_HANDLE);
    }
    return f;
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPVOID)
{
    if (lpNumberOfBytesRead != NULL) {
        *lpNumberOfBytesRead = 0;
    }
    FileObject* f = FileFromHandle(hFile);
    if (f == NULL) {
        return FALSE;
    }
    char* p = static_cast<char*>(lpBuffer);
    DWORD total = 0;
    while (total < nNumberOfBytesToRead) {
        ssize_t n = read(f->fd, p + total, nNumberOfBytesToRead - total);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (lpNumberOfBytesRead != NULL) {
                *lpNumberOfBytesRead = total;
            }
            // A handle that was not opened for reading: ACCESS_DENIED.
            SetLastError(errno == EBADF ? ERROR_ACCESS_DENIED : ErrnoToWin32(errno));
            return FALSE;
        }
        if (n == 0) {
            break;
        }
        total += static_cast<DWORD>(n);
    }
    if (lpNumberOfBytesRead != NULL) {
        *lpNumberOfBytesRead = total;
    }
    return TRUE;
}

BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPVOID)
{
    if (lpNumberOfBytesWritten != NULL) {
        *lpNumberOfBytesWritten = 0;
    }
    FileObject* f = FileFromHandle(hFile);
    if (f == NULL) {
        return FALSE;
    }
    const char* p = static_cast<const char*>(lpBuffer);
    DWORD total = 0;
    while (total < nNumberOfBytesToWrite) {
        ssize_t n = write(f->fd, p + total, nNumberOfBytesToWrite - total);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (lpNumberOfBytesWritten != NULL) {
                *lpNumberOfBytesWritten = total;
            }
            SetLastError(errno == EBADF ? ERROR_ACCESS_DENIED : ErrnoToWin32(errno));
            return FALSE;
        }
        total += static_cast<DWORD>(n);
    }
    if (lpNumberOfBytesWritten != NULL) {
        *lpNumberOfBytesWritten = total;
    }
    return TRUE;
}

DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
    FileObject* f = FileFromHandle(hFile);
    if (f == NULL) {
        return INVALID_SET_FILE_POINTER;
    }

    int whence;
    switch (dwMoveMethod) {
    case 0: // FILE_BEGIN
        whence = SEEK_SET;
        break;
    case FILE_CURRENT:
        whence = SEEK_CUR;
        break;
    case 2: // FILE_END
        whence = SEEK_END;
        break;
    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_SET_FILE_POINTER;
    }

    int64_t distance;
    if (lpDistanceToMoveHigh != NULL) {
        distance = (static_cast<int64_t>(*lpDistanceToMoveHigh) << 32) | static_cast<uint32_t>(lDistanceToMove);
    } else {
        distance = lDistanceToMove;
    }

    off_t pos = lseek(f->fd, static_cast<off_t>(distance), whence);
    if (pos < 0) {
        SetLastError(errno == EINVAL ? ERROR_NEGATIVE_SEEK : ErrnoToWin32(errno));
        return INVALID_SET_FILE_POINTER;
    }

    if (lpDistanceToMoveHigh != NULL) {
        *lpDistanceToMoveHigh = static_cast<LONG>(static_cast<uint64_t>(pos) >> 32);
    }
    SetLastError(ERROR_SUCCESS);
    return static_cast<DWORD>(static_cast<uint64_t>(pos) & 0xFFFFFFFFu);
}

DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
    FileObject* f = FileFromHandle(hFile);
    if (f == NULL) {
        return INVALID_FILE_SIZE;
    }
    struct stat st;
    if (fstat(f->fd, &st) != 0) {
        SetLastErrorFromErrno();
        return INVALID_FILE_SIZE;
    }
    uint64_t size = static_cast<uint64_t>(st.st_size);
    if (lpFileSizeHigh != NULL) {
        *lpFileSizeHigh = static_cast<DWORD>(size >> 32);
    }
    SetLastError(ERROR_SUCCESS);
    return static_cast<DWORD>(size & 0xFFFFFFFFu);
}

BOOL FlushFileBuffers(HANDLE hFile)
{
    FileObject* f = FileFromHandle(hFile);
    if (f == NULL) {
        return FALSE;
    }
    if (fsync(f->fd) != 0) {
        SetLastErrorFromErrno();
        return FALSE;
    }
    return TRUE;
}

BOOL SetEndOfFile(HANDLE hFile)
{
    FileObject* f = FileFromHandle(hFile);
    if (f == NULL) {
        return FALSE;
    }
    off_t pos = lseek(f->fd, 0, SEEK_CUR);
    if (pos < 0 || ftruncate(f->fd, pos) != 0) {
        SetLastErrorFromErrno();
        return FALSE;
    }
    return TRUE;
}

BOOL GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime)
{
    FileObject* f = FileFromHandle(hFile);
    if (f == NULL) {
        return FALSE;
    }
    struct stat st;
    if (fstat(f->fd, &st) != 0) {
        SetLastErrorFromErrno();
        return FALSE;
    }
    uint64_t c, a, w;
    FillFileTimes(st, c, a, w);
    if (lpCreationTime != NULL) ToFileTime(c, lpCreationTime);
    if (lpLastAccessTime != NULL) ToFileTime(a, lpLastAccessTime);
    if (lpLastWriteTime != NULL) ToFileTime(w, lpLastWriteTime);
    return TRUE;
}

static void FileTimeToTimespec(const FILETIME* ft, struct timespec* ts)
{
    if (ft == NULL) {
        ts->tv_sec = 0;
        ts->tv_nsec = UTIME_OMIT;
        return;
    }
    uint64_t v = (static_cast<uint64_t>(ft->dwHighDateTime) << 32) | ft->dwLowDateTime;
    const int64_t epoch = 116444736000000000ll;
    int64_t d = static_cast<int64_t>(v) - epoch; // may be before 1970
    int64_t sec = d / 10000000ll;
    int64_t rem = d % 10000000ll;
    if (rem < 0) {
        rem += 10000000ll;
        sec -= 1;
    }
    ts->tv_sec = static_cast<time_t>(sec);
    ts->tv_nsec = static_cast<long>(rem * 100);
}

BOOL SetFileTime(HANDLE hFile, const FILETIME*, const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime)
{
    FileObject* f = FileFromHandle(hFile);
    if (f == NULL) {
        return FALSE;
    }
    // The creation time cannot be set through POSIX.
    struct timespec times[2];
    FileTimeToTimespec(lpLastAccessTime, &times[0]);
    FileTimeToTimespec(lpLastWriteTime, &times[1]);
    if (futimens(f->fd, times) != 0) {
        SetLastErrorFromErrno();
        return FALSE;
    }
    return TRUE;
}

// ---------------------------------------------------------------------------
// Names

BOOL DeleteFileA(LPCSTR lpFileName)
{
    std::string path = TranslatePath(lpFileName);
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        SetErrorForPath(errno, path);
        return FALSE;
    }
    // As on Windows: directories and read-only files are not deleted.
    if (S_ISDIR(st.st_mode) || (st.st_mode & S_IWUSR) == 0) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
    if (OpenWithoutDeleteShare(st)) {
        SetLastError(ERROR_SHARING_VIOLATION);
        return FALSE;
    }
    if (unlink(path.c_str()) != 0) {
        SetErrorForPath(errno, path);
        return FALSE;
    }
    return TRUE;
}

static BOOL CopyContents(const std::string& from, const std::string& to, BOOL bFailIfExists)
{
    int in = open(from.c_str(), O_RDONLY | O_CLOEXEC);
    if (in < 0) {
        SetErrorForPath(errno, from);
        return FALSE;
    }
    struct stat st;
    if (fstat(in, &st) != 0 || S_ISDIR(st.st_mode)) {
        close(in);
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    int flags = O_WRONLY | O_CREAT | O_CLOEXEC | (bFailIfExists ? O_EXCL : O_TRUNC);
    int out = open(to.c_str(), flags, st.st_mode & 0777);
    if (out < 0) {
        int err = errno;
        close(in);
        if (err == EEXIST) {
            SetLastError(ERROR_FILE_EXISTS);
        } else {
            SetErrorForPath(err, to);
        }
        return FALSE;
    }

    std::vector<char> buffer(1 << 16);
    BOOL ok = TRUE;
    for (;;) {
        ssize_t n = read(in, &buffer[0], buffer.size());
        if (n < 0) {
            if (errno == EINTR) continue;
            SetLastErrorFromErrno();
            ok = FALSE;
            break;
        }
        if (n == 0) break;
        ssize_t done = 0;
        while (done < n) {
            ssize_t w = write(out, &buffer[0] + done, static_cast<size_t>(n - done));
            if (w < 0) {
                if (errno == EINTR) continue;
                SetLastErrorFromErrno();
                ok = FALSE;
                break;
            }
            done += w;
        }
        if (!ok) break;
    }

    if (ok) {
        // Windows keeps the last-write time of the source.
        struct timespec times[2] = { st.st_atimespec, st.st_mtimespec };
        futimens(out, times);
    }
    close(in);
    close(out);
    if (!ok) {
        unlink(to.c_str());
    }
    return ok;
}

BOOL CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists)
{
    return CopyContents(TranslatePath(lpExistingFileName), TranslatePath(lpNewFileName), bFailIfExists);
}

BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName)
{
    std::string from = TranslatePath(lpExistingFileName);
    std::string to = TranslatePath(lpNewFileName);

    struct stat st;
    if (stat(from.c_str(), &st) != 0) {
        SetErrorForPath(errno, from);
        return FALSE;
    }
    if (lstat(to.c_str(), &st) == 0) {
        SetLastError(ERROR_ALREADY_EXISTS);
        return FALSE;
    }
    if (stat(from.c_str(), &st) == 0 && OpenWithoutDeleteShare(st)) {
        SetLastError(ERROR_SHARING_VIOLATION);
        return FALSE;
    }
    if (rename(from.c_str(), to.c_str()) == 0) {
        return TRUE;
    }
    if (errno == EXDEV) {
        if (CopyContents(from, to, TRUE)) {
            unlink(from.c_str());
            return TRUE;
        }
        return FALSE;
    }
    SetErrorForPath(errno, to);
    return FALSE;
}

DWORD GetFileAttributesA(LPCSTR lpFileName)
{
    if (lpFileName == NULL) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_FILE_ATTRIBUTES;
    }
    std::string path = TranslatePath(lpFileName);
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        SetErrorForPath(errno, path);
        return INVALID_FILE_ATTRIBUTES;
    }
    return AttributesFromStat(st);
}

BOOL GetFileAttributesExA(LPCSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
    if (lpFileName == NULL || lpFileInformation == NULL || fInfoLevelId != GetFileExInfoStandard) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    std::string path = TranslatePath(lpFileName);
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        SetErrorForPath(errno, path);
        return FALSE;
    }
    WIN32_FILE_ATTRIBUTE_DATA* data = static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(lpFileInformation);
    uint64_t c, a, w;
    FillFileTimes(st, c, a, w);
    data->dwFileAttributes = AttributesFromStat(st);
    ToFileTime(c, &data->ftCreationTime);
    ToFileTime(a, &data->ftLastAccessTime);
    ToFileTime(w, &data->ftLastWriteTime);
    uint64_t size = S_ISDIR(st.st_mode) ? 0 : static_cast<uint64_t>(st.st_size);
    data->nFileSizeHigh = static_cast<DWORD>(size >> 32);
    data->nFileSizeLow = static_cast<DWORD>(size & 0xFFFFFFFFu);
    return TRUE;
}

BOOL SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes)
{
    if (lpFileName == NULL) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }
    std::string path = TranslatePath(lpFileName);
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        SetErrorForPath(errno, path);
        return FALSE;
    }
    // Only the read-only bit has an effect (all of the write permission bits).
    mode_t mode = st.st_mode & 07777;
    if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
        mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    } else {
        mode |= S_IWUSR;
    }
    if (mode != (st.st_mode & 07777) && chmod(path.c_str(), mode) != 0) {
        SetErrorForPath(errno, path);
        return FALSE;
    }
    return TRUE;
}

BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES)
{
    if (lpPathName == NULL) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }
    std::string path = TranslatePath(lpPathName);
    if (mkdir(path.c_str(), 0777) != 0) {
        int err = errno;
        if (err == EEXIST) {
            SetLastError(ERROR_ALREADY_EXISTS);
        } else if (err == ENOENT) {
            SetLastError(ERROR_PATH_NOT_FOUND);
        } else {
            SetLastError(ErrnoToWin32(err));
        }
        return FALSE;
    }
    return TRUE;
}

BOOL RemoveDirectoryA(LPCSTR lpPathName)
{
    if (lpPathName == NULL) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }
    std::string path = TranslatePath(lpPathName);
    if (rmdir(path.c_str()) != 0) {
        SetErrorForPath(errno, path);
        return FALSE;
    }
    return TRUE;
}

BOOL SetCurrentDirectoryA(LPCSTR lpPathName)
{
    if (lpPathName == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    std::string path = TranslatePath(lpPathName);
    if (ChangeDirectory(path) != 0) {
        SetLastError(errno == ENOENT ? ERROR_FILE_NOT_FOUND : ErrnoToWin32(errno));
        return FALSE;
    }
    return TRUE;
}

DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer)
{
    std::string cwd = CurrentDirectory();
    if (cwd.empty()) {
        SetLastErrorFromErrno();
        return 0;
    }
    DWORD len = static_cast<DWORD>(cwd.size());
    if (nBufferLength <= len) {
        return len + 1;
    }
    memcpy(lpBuffer, cwd.c_str(), len + 1);
    return len;
}

DWORD GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR* lpFilePart)
{
    if (lpFileName == NULL || lpFileName[0] == 0) {
        SetLastError(ERROR_INVALID_NAME);
        return 0;
    }
    std::string full = AbsolutePath(lpFileName);
    // A trailing slash stays (the path names a directory).
    char last = lpFileName[strlen(lpFileName) - 1];
    if ((last == '/' || last == '\\') && full.size() > 1) {
        full += '/';
    }
    DWORD len = static_cast<DWORD>(full.size());
    if (nBufferLength <= len) {
        return len + 1;
    }
    memcpy(lpBuffer, full.c_str(), len + 1);
    if (lpFilePart != NULL) {
        size_t slash = full.find_last_of('/');
        if (slash == std::string::npos || slash + 1 == full.size()) {
            *lpFilePart = NULL;
        } else {
            *lpFilePart = lpBuffer + slash + 1;
        }
    }
    return len;
}

DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer)
{
    const char* tmp = getenv("TMPDIR");
    std::string dir = (tmp != NULL && tmp[0] != 0) ? tmp : "/tmp";
    if (dir[dir.size() - 1] != '/') {
        dir += '/';
    }
    DWORD len = static_cast<DWORD>(dir.size());
    if (nBufferLength <= len) {
        return len + 1;
    }
    memcpy(lpBuffer, dir.c_str(), len + 1);
    return len;
}

// ---------------------------------------------------------------------------
// Find

namespace {

char Lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

// Windows wildcards: `*` any run (also empty), `?` any one character.
bool WildMatch(const char* pat, const char* str)
{
    const char* starPat = NULL;
    const char* starStr = NULL;
    while (*str != 0) {
        if (*pat == '*') {
            starPat = ++pat;
            starStr = str;
        } else if (*pat == '?' || (*pat != 0 && Lower(*pat) == Lower(*str))) {
            pat++;
            str++;
        } else if (starPat != NULL) {
            pat = starPat;
            str = ++starStr;
        } else {
            return false;
        }
    }
    while (*pat == '*') {
        pat++;
    }
    return *pat == 0;
}

bool NameMatches(const std::string& pattern, const std::string& name)
{
    if (WildMatch(pattern.c_str(), name.c_str())) {
        return true;
    }
    // `*.*` and `name.*` also match names without an extension.
    size_t n = pattern.size();
    if (n >= 2 && pattern[n - 2] == '.' && pattern[n - 1] == '*') {
        return WildMatch(pattern.substr(0, n - 2).c_str(), name.c_str());
    }
    return false;
}

bool HasWildcards(const std::string& s)
{
    return s.find_first_of("*?") != std::string::npos;
}

std::string UpperKey(const std::string& s)
{
    std::string r = s;
    for (size_t i = 0; i < r.size(); i++) {
        if (r[i] >= 'a' && r[i] <= 'z') r[i] = static_cast<char>(r[i] - 32);
    }
    return r;
}

bool EntryLess(const FindObject::Entry& a, const FindObject::Entry& b)
{
    return UpperKey(a.name) < UpperKey(b.name);
}

void FillFindData(const FindObject::Entry& e, LPWIN32_FIND_DATAA data)
{
    memset(data, 0, sizeof(*data));
    data->dwFileAttributes = e.attributes;
    ToFileTime(e.createTime, &data->ftCreationTime);
    ToFileTime(e.accessTime, &data->ftLastAccessTime);
    ToFileTime(e.writeTime, &data->ftLastWriteTime);
    data->nFileSizeHigh = static_cast<DWORD>(e.size >> 32);
    data->nFileSizeLow = static_cast<DWORD>(e.size & 0xFFFFFFFFu);
    strncpy(data->cFileName, e.name.c_str(), MAX_PATH - 1);
    data->cFileName[MAX_PATH - 1] = 0;
    data->cAlternateFileName[0] = 0;
}

FindObject::Entry MakeEntry(const std::string& name, const struct stat& st)
{
    FindObject::Entry e;
    e.name = name;
    e.attributes = AttributesFromStat(st);
    e.size = S_ISDIR(st.st_mode) ? 0 : static_cast<uint64_t>(st.st_size);
    FillFileTimes(st, e.createTime, e.accessTime, e.writeTime);
    return e;
}

} // namespace

HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    if (lpFileName == NULL || lpFileName[0] == 0 || lpFindFileData == NULL) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    std::string spec(lpFileName);
    for (size_t i = 0; i < spec.size(); i++) {
        if (spec[i] == '\\') spec[i] = '/';
    }
    size_t slash = spec.find_last_of('/');
    std::string dirPart, pattern;
    if (slash == std::string::npos) {
        // "C:*.*" form: drive letter in front of the pattern.
        if (spec.size() >= 2 && spec[1] == ':') {
            pattern = spec.substr(2);
        } else {
            pattern = spec;
        }
        dirPart = ".";
    } else {
        pattern = spec.substr(slash + 1);
        dirPart = slash == 0 ? "/" : spec.substr(0, slash);
    }
    if (pattern.empty()) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    std::string dirPath = TranslatePath(dirPart.c_str());
    DIR* dir = opendir(dirPath.c_str());
    if (dir == NULL) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    FindObject* find = new FindObject;
    std::string prefix = dirPath;
    if (prefix.empty() || prefix[prefix.size() - 1] != '/') {
        prefix += '/';
    }

    while (struct dirent* de = readdir(dir)) {
        std::string name = de->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        if (!NameMatches(pattern, name)) {
            continue;
        }
        struct stat st;
        if (stat((prefix + name).c_str(), &st) != 0) {
            continue; // a dangling link
        }
        find->entries.push_back(MakeEntry(name, st));
    }
    closedir(dir);

    std::sort(find->entries.begin(), find->entries.end(), EntryLess);

    // `.` and `..` come first, as in an NTFS listing.
    if (dirPath != "/") {
        struct stat st;
        std::vector<FindObject::Entry> dots;
        if (NameMatches(pattern, ".") && stat(dirPath.c_str(), &st) == 0) {
            dots.push_back(MakeEntry(".", st));
        }
        if (NameMatches(pattern, "..") && stat((prefix + "..").c_str(), &st) == 0) {
            dots.push_back(MakeEntry("..", st));
        }
        find->entries.insert(find->entries.begin(), dots.begin(), dots.end());
    }

    if (find->entries.empty()) {
        Release(find);
        SetLastError(HasWildcards(pattern) ? ERROR_FILE_NOT_FOUND : ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    FillFindData(find->entries[0], lpFindFileData);
    find->next = 1;
    SetLastError(ERROR_SUCCESS);
    return find;
}

BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
    FindObject* find = static_cast<FindObject*>(ObjectFromHandle(hFindFile, K_FIND));
    if (find == NULL) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    if (find->next >= find->entries.size()) {
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }
    FillFindData(find->entries[find->next++], lpFindFileData);
    return TRUE;
}

BOOL FindClose(HANDLE hFindFile)
{
    FindObject* find = static_cast<FindObject*>(ObjectFromHandle(hFindFile, K_FIND));
    if (find == NULL) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    Release(find);
    return TRUE;
}

// ---------------------------------------------------------------------------
// Drives. The Mac has one volume; it is presented as a fixed drive C:.

DWORD GetLogicalDrives()
{
    return 1u << ('C' - 'A');
}

DWORD GetLogicalDriveStringsA(DWORD nBufferLength, LPSTR lpBuffer)
{
    static const char drives[] = "C:\\\0";
    const DWORD needed = sizeof(drives); // "C:\\" NUL and the closing NUL
    if (nBufferLength < needed) {
        return needed;
    }
    memcpy(lpBuffer, drives, needed);
    return needed - 1;
}

UINT GetDriveTypeA(LPCSTR)
{
    return DRIVE_FIXED;
}

BOOL GetDiskFreeSpaceA(LPCSTR, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters)
{
    // NOTE: Port: a made-up large disk. The game multiplies the free cluster
    // count by the sectors per cluster in 32 bits, so the numbers are chosen
    // to stay far from overflow: 512 * 8 * 2^21 = 8 GB.
    if (lpSectorsPerCluster != NULL) *lpSectorsPerCluster = 8;
    if (lpBytesPerSector != NULL) *lpBytesPerSector = 512;
    if (lpNumberOfFreeClusters != NULL) *lpNumberOfFreeClusters = 1u << 21;
    if (lpTotalNumberOfClusters != NULL) *lpTotalNumberOfClusters = 1u << 21;
    return TRUE;
}

BOOL GetDiskFreeSpaceExA(LPCSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes)
{
    std::string path = lpDirectoryName != NULL ? TranslatePath(lpDirectoryName) : std::string(".");
    struct statvfs sv;
    if (statvfs(path.c_str(), &sv) != 0) {
        SetLastErrorFromErrno();
        return FALSE;
    }
    uint64_t total = static_cast<uint64_t>(sv.f_blocks) * sv.f_frsize;
    uint64_t avail = static_cast<uint64_t>(sv.f_bavail) * sv.f_frsize;
    uint64_t free = static_cast<uint64_t>(sv.f_bfree) * sv.f_frsize;
    if (lpFreeBytesAvailableToCaller != NULL) lpFreeBytesAvailableToCaller->QuadPart = avail;
    if (lpTotalNumberOfBytes != NULL) lpTotalNumberOfBytes->QuadPart = total;
    if (lpTotalNumberOfFreeBytes != NULL) lpTotalNumberOfFreeBytes->QuadPart = free;
    return TRUE;
}

BOOL GetVolumeInformationA(LPCSTR, LPSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize)
{
    if (lpVolumeNameBuffer != NULL && nVolumeNameSize > 0) {
        strncpy(lpVolumeNameBuffer, "MACINTOSH", nVolumeNameSize - 1);
        lpVolumeNameBuffer[nVolumeNameSize - 1] = 0;
    }
    if (lpVolumeSerialNumber != NULL) *lpVolumeSerialNumber = 0x1D2D2002;
    if (lpMaximumComponentLength != NULL) *lpMaximumComponentLength = 255;
    if (lpFileSystemFlags != NULL) *lpFileSystemFlags = 0;
    if (lpFileSystemNameBuffer != NULL && nFileSystemNameSize > 0) {
        strncpy(lpFileSystemNameBuffer, "NTFS", nFileSystemNameSize - 1);
        lpFileSystemNameBuffer[nFileSystemNameSize - 1] = 0;
    }
    return TRUE;
}

UINT SetErrorMode(UINT uMode)
{
    static UINT mode = 0;
    UINT previous = mode;
    mode = uMode;
    return previous;
}
