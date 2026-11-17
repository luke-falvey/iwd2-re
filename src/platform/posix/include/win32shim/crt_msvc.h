#ifndef WIN32SHIM_CRT_MSVC_H_
#define WIN32SHIM_CRT_MSVC_H_
#include <win32shim/wintypes.h>
#include <win32shim/std_headers.h>

// Names of the MSVC C runtime that the game uses (direct.h, io.h, process.h,
// tchar.h, mbstring.h, mbctype.h, time.h, string.h, stdio.h).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define _MAX_PATH 260
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256

typedef int64_t __time64_t;

// One-line mappings onto POSIX.
inline __time64_t _time64(__time64_t* timer)
{
    time_t t = time(NULL);
    if (timer != NULL) *timer = t;
    return t;
}
// MSVC's range starts at 1970-01-01: earlier dates give -1.
inline __time64_t _mktime64(struct tm* timeptr)
{
    time_t t = mktime(timeptr);
    return t < 0 ? -1 : t;
}
inline int _stricmp(const char* a, const char* b) { return strcasecmp(a, b); }
inline int _strnicmp(const char* a, const char* b, size_t n) { return strncasecmp(a, b, n); }
inline int stricmp(const char* a, const char* b) { return strcasecmp(a, b); }
inline int strcmpi(const char* a, const char* b) { return strcasecmp(a, b); }
inline int strnicmp(const char* a, const char* b, size_t n) { return strncasecmp(a, b, n); }
char* _getcwd(char* buffer, int maxlen);

// MSVC semantics: -1 and no terminator when the output does not fit.
int _vsnprintf(char* buffer, size_t count, const char* format, va_list argptr);
int _snprintf(char* buffer, size_t count, const char* format, ...);

// Names go through platform::TranslatePath (backslashes, drive letters, case).
int _chdir(const char* dirname);
int _mkdir(const char* dirname);
// `fopen` is redirected the same way. A function-like macro, so that
// `using ::fopen;` in the standard headers still works.
FILE* iwd2re_fopen(const char* filename, const char* mode);
#define fopen(filename, mode) iwd2re_fopen(filename, mode)

// The MSVC generator (multithreaded CRT: one state per thread, seed 1 at the
// start of each thread). The libc one gives other numbers.
int iwd2re_rand(void);
void iwd2re_srand(unsigned int seed);
#undef RAND_MAX
#define RAND_MAX 0x7FFF
#define rand() iwd2re_rand()
#define srand(seed) iwd2re_srand(seed)
// So that `std::rand()` (which the macros turn into `std::iwd2re_rand()`) works.
namespace std {
using ::iwd2re_rand;
using ::iwd2re_srand;
} // namespace std

inline long tell(int fd) { return static_cast<long>(lseek(fd, 0, SEEK_CUR)); }

typedef int errno_t;
errno_t _localtime64_s(struct tm* tmDest, const __time64_t* sourceTime);
errno_t _splitpath_s(const char* path, char* drive, size_t driveNumberOfElements, char* dir, size_t dirNumberOfElements, char* fname, size_t nameNumberOfElements, char* ext, size_t extNumberOfElements);
errno_t _makepath_s(char* path, size_t sizeInBytes, const char* drive, const char* dir, const char* fname, const char* ext);

char* _itoa(int value, char* buffer, int radix);
char* _ltoa(int value, char* buffer, int radix);
char* _strupr(char* str);
char* _strlwr(char* str);
char* _strrev(char* str);
void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext);
void _makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext);
char* _fullpath(char* absPath, const char* relPath, size_t maxLength);

// process.h
typedef void(__cdecl* _beginthread_proc_type)(void*);
uintptr_t _beginthread(_beginthread_proc_type start_address, unsigned stack_size, void* arglist);
void _endthread();

// mbstring.h / mbctype.h (the code page is "MBCS" as in the original build).
int _getmbcp();
unsigned char* _mbsinc(const unsigned char* current);
int _ismbcspace(unsigned int c);
unsigned char* _mbschr(const unsigned char* str, unsigned int c);
unsigned char* _mbsrchr(const unsigned char* str, unsigned int c);
unsigned char* _mbsstr(const unsigned char* str, const unsigned char* strSearch);
unsigned char* _mbspbrk(const unsigned char* str, const unsigned char* strCharSet);
size_t _mbsspn(const unsigned char* str, const unsigned char* strCharSet);
size_t _mbscspn(const unsigned char* str, const unsigned char* strCharSet);
int _mbscmp(const unsigned char* string1, const unsigned char* string2);
int _mbsicmp(const unsigned char* string1, const unsigned char* string2);
int _mbscoll(const unsigned char* string1, const unsigned char* string2);
int _mbsicoll(const unsigned char* string1, const unsigned char* string2);
int _mbsupr_s(unsigned char* str, size_t sizeInBytes);
int _mbslwr_s(unsigned char* str, size_t sizeInBytes);
unsigned char* _mbsrev(unsigned char* str);

#endif /* WIN32SHIM_CRT_MSVC_H_ */
