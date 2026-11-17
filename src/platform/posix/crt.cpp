// The MSVC C runtime names the game uses (crt_msvc.h).

#include "path.h"
#include "platform.h"

#include <win32shim/crt_msvc.h>

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// rand / srand: the MSVC generator, one state per thread (multithreaded CRT:
// each thread starts at seed 1).

static thread_local uint32_t t_randSeed = 1;

int iwd2re_rand(void)
{
    t_randSeed = t_randSeed * 214013u + 2531011u;
    return static_cast<int>((t_randSeed >> 16) & 0x7FFF);
}

void iwd2re_srand(unsigned int seed)
{
    t_randSeed = seed;
}

// ---------------------------------------------------------------------------
// printf family with MSVC truncation rules: -1 and no terminator when the
// output does not fit; exactly filling the buffer gives no terminator either.

int _vsnprintf(char* buffer, size_t count, const char* format, va_list argptr)
{
    va_list copy;
    va_copy(copy, argptr);
    int needed = vsnprintf(NULL, 0, format, copy);
    va_end(copy);
    if (needed < 0) {
        return -1;
    }
    size_t n = static_cast<size_t>(needed);
    if (buffer == NULL || count == 0) {
        return n == 0 ? 0 : -1;
    }
    if (n < count) {
        vsnprintf(buffer, count, format, argptr);
        return needed;
    }
    // Too long, or exactly count characters: copy the first `count`
    // characters without a terminator.
    std::vector<char> temp(n + 1);
    vsnprintf(&temp[0], temp.size(), format, argptr);
    memcpy(buffer, &temp[0], count);
    return n == count ? static_cast<int>(count) : -1;
}

int _snprintf(char* buffer, size_t count, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int rc = _vsnprintf(buffer, count, format, args);
    va_end(args);
    return rc;
}

// ---------------------------------------------------------------------------
// Files: names go through TranslatePath.

FILE* iwd2re_fopen(const char* filename, const char* mode)
{
    // NOTE: Port: the text mode flag `t` is not a POSIX mode; there is no
    // CR/LF translation here.
    std::string m;
    for (const char* p = mode; p != NULL && *p != 0; p++) {
        if (*p != 't') {
            m += *p;
        }
    }
    return (fopen)(platform::TranslatePath(filename).c_str(), m.c_str());
}

int _chdir(const char* dirname)
{
    return platform::ChangeDirectory(platform::TranslatePath(dirname));
}

char* _getcwd(char* buffer, int maxlen)
{
    std::string cwd = platform::CurrentDirectory();
    if (cwd.empty()) {
        return NULL;
    }
    if (buffer == NULL) {
        buffer = static_cast<char*>(malloc(maxlen > 0 ? static_cast<size_t>(maxlen) : cwd.size() + 1));
        if (buffer == NULL) {
            return NULL;
        }
    }
    if (cwd.size() + 1 > static_cast<size_t>(maxlen)) {
        errno = ERANGE;
        return NULL;
    }
    memcpy(buffer, cwd.c_str(), cwd.size() + 1);
    return buffer;
}

int _mkdir(const char* dirname)
{
    return mkdir(platform::TranslatePath(dirname).c_str(), 0777);
}

// ---------------------------------------------------------------------------
// Strings

char* _itoa(int value, char* buffer, int radix)
{
    if (radix < 2 || radix > 36) {
        buffer[0] = 0;
        return buffer;
    }
    char temp[40];
    int n = 0;
    bool negative = radix == 10 && value < 0;
    uint32_t v = negative ? 0u - static_cast<uint32_t>(value) : static_cast<uint32_t>(value);
    do {
        int digit = static_cast<int>(v % static_cast<uint32_t>(radix));
        temp[n++] = static_cast<char>(digit < 10 ? '0' + digit : 'a' + digit - 10);
        v /= static_cast<uint32_t>(radix);
    } while (v != 0);
    char* out = buffer;
    if (negative) {
        *out++ = '-';
    }
    while (n > 0) {
        *out++ = temp[--n];
    }
    *out = 0;
    return buffer;
}

char* _ltoa(int value, char* buffer, int radix)
{
    return _itoa(value, buffer, radix);
}

char* _strupr(char* s)
{
    for (char* p = s; *p != 0; p++) {
        if (*p >= 'a' && *p <= 'z') *p = static_cast<char>(*p - 32);
    }
    return s;
}

char* _strlwr(char* s)
{
    for (char* p = s; *p != 0; p++) {
        if (*p >= 'A' && *p <= 'Z') *p = static_cast<char>(*p + 32);
    }
    return s;
}

char* _strrev(char* s)
{
    size_t n = strlen(s);
    for (size_t i = 0; i < n / 2; i++) {
        char t = s[i];
        s[i] = s[n - 1 - i];
        s[n - 1 - i] = t;
    }
    return s;
}

errno_t _localtime64_s(struct tm* tmDest, const __time64_t* sourceTime)
{
    if (tmDest == NULL || sourceTime == NULL || *sourceTime < 0) {
        return EINVAL;
    }
    time_t t = static_cast<time_t>(*sourceTime);
    return localtime_r(&t, tmDest) != NULL ? 0 : EINVAL;
}

// --- paths ---

static bool IsSlash(char c)
{
    return c == '/' || c == '\\';
}

// Copies into an optional buffer; false when it does not fit.
static bool PutPart(char* dest, size_t size, const std::string& part)
{
    if (dest == NULL) {
        return true;
    }
    if (size == 0 || part.size() + 1 > size) {
        if (size > 0) dest[0] = 0;
        return false;
    }
    memcpy(dest, part.c_str(), part.size() + 1);
    return true;
}

static void SplitPath(const char* path, std::string& drive, std::string& dir, std::string& fname, std::string& ext)
{
    std::string s(path != NULL ? path : "");
    size_t start = 0;
    if (s.size() >= 2 && s[1] == ':') {
        drive = s.substr(0, 2);
        start = 2;
    }
    size_t slash = std::string::npos;
    for (size_t i = start; i < s.size(); i++) {
        if (IsSlash(s[i])) slash = i;
    }
    size_t nameStart = start;
    if (slash != std::string::npos) {
        dir = s.substr(start, slash + 1 - start);
        nameStart = slash + 1;
    }
    std::string name = s.substr(nameStart);
    size_t dot = name.rfind('.');
    if (dot != std::string::npos) {
        fname = name.substr(0, dot);
        ext = name.substr(dot);
    } else {
        fname = name;
    }
}

errno_t _splitpath_s(const char* path, char* drive, size_t driveNumberOfElements, char* dir, size_t dirNumberOfElements, char* fname, size_t nameNumberOfElements, char* ext, size_t extNumberOfElements)
{
    if (path == NULL) {
        return EINVAL;
    }
    std::string d, r, f, e;
    SplitPath(path, d, r, f, e);
    bool ok = true;
    ok &= PutPart(drive, driveNumberOfElements, d);
    ok &= PutPart(dir, dirNumberOfElements, r);
    ok &= PutPart(fname, nameNumberOfElements, f);
    ok &= PutPart(ext, extNumberOfElements, e);
    return ok ? 0 : ERANGE;
}

void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
    _splitpath_s(path, drive, drive != NULL ? _MAX_DRIVE : 0, dir, dir != NULL ? _MAX_DIR : 0,
        fname, fname != NULL ? _MAX_FNAME : 0, ext, ext != NULL ? _MAX_EXT : 0);
}

static std::string MakePath(const char* drive, const char* dir, const char* fname, const char* ext)
{
    std::string out;
    if (drive != NULL && drive[0] != 0) {
        out += drive[0];
        out += ':';
    }
    if (dir != NULL && dir[0] != 0) {
        out += dir;
        char last = dir[strlen(dir) - 1];
        if (!IsSlash(last)) {
            // The POSIX form stays POSIX.
            out += (strchr(dir, '/') != NULL && strchr(dir, '\\') == NULL) ? '/' : '\\';
        }
    }
    if (fname != NULL) {
        out += fname;
    }
    if (ext != NULL && ext[0] != 0) {
        if (ext[0] != '.') {
            out += '.';
        }
        out += ext;
    }
    return out;
}

errno_t _makepath_s(char* path, size_t sizeInBytes, const char* drive, const char* dir, const char* fname, const char* ext)
{
    if (path == NULL || sizeInBytes == 0) {
        return EINVAL;
    }
    std::string out = MakePath(drive, dir, fname, ext);
    if (out.size() + 1 > sizeInBytes) {
        path[0] = 0;
        return ERANGE;
    }
    memcpy(path, out.c_str(), out.size() + 1);
    return 0;
}

void _makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext)
{
    std::string out = MakePath(drive, dir, fname, ext);
    memcpy(path, out.c_str(), out.size() + 1);
}

char* _fullpath(char* absPath, const char* relPath, size_t maxLength)
{
    std::string full = platform::AbsolutePath(relPath != NULL && relPath[0] != 0 ? relPath : ".");
    // A trailing slash stays.
    if (relPath != NULL && relPath[0] != 0 && IsSlash(relPath[strlen(relPath) - 1]) && full.size() > 1) {
        full += '/';
    }
    if (absPath == NULL) {
        absPath = static_cast<char*>(malloc(full.size() + 1));
        if (absPath == NULL) {
            return NULL;
        }
    } else if (full.size() + 1 > maxLength) {
        errno = ERANGE;
        return NULL;
    }
    memcpy(absPath, full.c_str(), full.size() + 1);
    return absPath;
}

// ---------------------------------------------------------------------------
// mbstring: the code page is single byte (the game ships a Western build), so
// every function works on bytes.

int _getmbcp()
{
    return 0; // "no multibyte code page": single-byte behaviour
}

unsigned char* _mbsinc(const unsigned char* current)
{
    return const_cast<unsigned char*>(current + 1);
}

int _ismbcspace(unsigned int c)
{
    return c == ' ' || (c >= 9 && c <= 13);
}

unsigned char* _mbschr(const unsigned char* str, unsigned int c)
{
    return reinterpret_cast<unsigned char*>(const_cast<char*>(strchr(reinterpret_cast<const char*>(str), static_cast<int>(c & 0xFF))));
}

unsigned char* _mbsrchr(const unsigned char* str, unsigned int c)
{
    return reinterpret_cast<unsigned char*>(const_cast<char*>(strrchr(reinterpret_cast<const char*>(str), static_cast<int>(c & 0xFF))));
}

unsigned char* _mbsstr(const unsigned char* str, const unsigned char* strSearch)
{
    return reinterpret_cast<unsigned char*>(const_cast<char*>(strstr(reinterpret_cast<const char*>(str), reinterpret_cast<const char*>(strSearch))));
}

unsigned char* _mbspbrk(const unsigned char* str, const unsigned char* strCharSet)
{
    return reinterpret_cast<unsigned char*>(const_cast<char*>(strpbrk(reinterpret_cast<const char*>(str), reinterpret_cast<const char*>(strCharSet))));
}

size_t _mbsspn(const unsigned char* str, const unsigned char* strCharSet)
{
    return strspn(reinterpret_cast<const char*>(str), reinterpret_cast<const char*>(strCharSet));
}

size_t _mbscspn(const unsigned char* str, const unsigned char* strCharSet)
{
    return strcspn(reinterpret_cast<const char*>(str), reinterpret_cast<const char*>(strCharSet));
}

static int Sign(int v)
{
    return v < 0 ? -1 : (v > 0 ? 1 : 0);
}

int _mbscmp(const unsigned char* string1, const unsigned char* string2)
{
    return Sign(strcmp(reinterpret_cast<const char*>(string1), reinterpret_cast<const char*>(string2)));
}

static unsigned char FoldLower(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<unsigned char>(c + 32) : c;
}

int _mbsicmp(const unsigned char* string1, const unsigned char* string2)
{
    for (;; string1++, string2++) {
        unsigned char a = FoldLower(*string1);
        unsigned char b = FoldLower(*string2);
        if (a != b) {
            return a < b ? -1 : 1;
        }
        if (a == 0) {
            return 0;
        }
    }
}

int _mbscoll(const unsigned char* string1, const unsigned char* string2)
{
    return _mbscmp(string1, string2);
}

int _mbsicoll(const unsigned char* string1, const unsigned char* string2)
{
    return _mbsicmp(string1, string2);
}

int _mbsupr_s(unsigned char* str, size_t sizeInBytes)
{
    if (str == NULL) {
        return EINVAL;
    }
    for (size_t i = 0; i < sizeInBytes; i++) {
        if (str[i] == 0) {
            return 0;
        }
        if (str[i] >= 'a' && str[i] <= 'z') str[i] = static_cast<unsigned char>(str[i] - 32);
    }
    str[0] = 0;
    return EINVAL;
}

int _mbslwr_s(unsigned char* str, size_t sizeInBytes)
{
    if (str == NULL) {
        return EINVAL;
    }
    for (size_t i = 0; i < sizeInBytes; i++) {
        if (str[i] == 0) {
            return 0;
        }
        if (str[i] >= 'A' && str[i] <= 'Z') str[i] = static_cast<unsigned char>(str[i] + 32);
    }
    str[0] = 0;
    return EINVAL;
}

unsigned char* _mbsrev(unsigned char* str)
{
    _strrev(reinterpret_cast<char*>(str));
    return str;
}
