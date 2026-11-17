// The MSVC C runtime names the shim provides.

#include "test.h"

#include <mbstring.h>

#include <atomic>
#include <string>
#include <thread>

TEST_CASE("rand follows the MSVC generator after srand(1)")
{
    srand(1);
    CHECK(rand() == 41);
    CHECK(rand() == 18467);
    CHECK(rand() == 6334);
    CHECK(rand() == 26500);
    CHECK(rand() == 19169);

    // The state is per thread, a new thread starts at seed 1.
    int first = -1;
    std::thread t([&] { first = rand(); });
    t.join();
    CHECK(first == 41);

    srand(1);
    CHECK(rand() == 41);
    CHECK(RAND_MAX == 0x7FFF);
}

TEST_CASE("_snprintf truncates the MSVC way")
{
    char buffer[16];

    memset(buffer, 'x', sizeof(buffer));
    CHECK(_snprintf(buffer, 10, "%s", "abc") == 3);
    CHECK(std::string(buffer) == "abc");

    // The output fills the buffer exactly: no terminator.
    memset(buffer, 'x', sizeof(buffer));
    CHECK(_snprintf(buffer, 5, "%s", "abcde") == 5);
    CHECK(memcmp(buffer, "abcde", 5) == 0);
    CHECK(buffer[5] == 'x');

    // Too long: -1, the buffer is filled, no terminator.
    memset(buffer, 'x', sizeof(buffer));
    CHECK(_snprintf(buffer, 5, "%s", "abcdefgh") == -1);
    CHECK(memcmp(buffer, "abcde", 5) == 0);
    CHECK(buffer[5] == 'x');

    memset(buffer, 'x', sizeof(buffer));
    CHECK(_snprintf(buffer, 4, "%d", 123456) == -1);
    CHECK(memcmp(buffer, "1234", 4) == 0);
    CHECK(buffer[4] == 'x');
}

namespace {
int CallVsnprintf(char* buffer, size_t count, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int rc = _vsnprintf(buffer, count, format, args);
    va_end(args);
    return rc;
}
} // namespace

TEST_CASE("_vsnprintf truncates the MSVC way")
{
    char buffer[8];
    memset(buffer, 'x', sizeof(buffer));
    CHECK(CallVsnprintf(buffer, 4, "%s", "wxyz") == 4);
    CHECK(memcmp(buffer, "wxyz", 4) == 0);
    CHECK(buffer[4] == 'x');
    CHECK(CallVsnprintf(buffer, 4, "%s", "wxyzv") == -1);
    CHECK(CallVsnprintf(buffer, 8, "%d-%s", 7, "ok") == 4);
    CHECK(std::string(buffer) == "7-ok");
}

TEST_CASE("string helpers: _itoa, _strupr, _strlwr, _strrev")
{
    char buffer[40];
    CHECK(std::string(_itoa(0, buffer, 10)) == "0");
    CHECK(std::string(_itoa(12345, buffer, 10)) == "12345");
    CHECK(std::string(_itoa(-45, buffer, 10)) == "-45");
    CHECK(std::string(_itoa(255, buffer, 16)) == "ff");
    CHECK(std::string(_itoa(5, buffer, 2)) == "101");

    char s[] = "Mixed Case 1";
    CHECK(std::string(_strupr(s)) == "MIXED CASE 1");
    CHECK(std::string(_strlwr(s)) == "mixed case 1");
    CHECK(std::string(_strrev(s)) == "1 esac dexim");
}

TEST_CASE("_splitpath_s and _makepath_s")
{
    char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
    CHECK(_splitpath_s("C:\\Games\\IWD2\\override\\file.name.tga", drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT) == 0);
    CHECK(std::string(drive) == "C:");
    CHECK(std::string(dir) == "\\Games\\IWD2\\override\\");
    CHECK(std::string(fname) == "file.name");
    CHECK(std::string(ext) == ".tga");

    CHECK(_splitpath_s("/usr/lib/libz.dylib", drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT) == 0);
    CHECK(std::string(drive) == "");
    CHECK(std::string(dir) == "/usr/lib/");
    CHECK(std::string(fname) == "libz");
    CHECK(std::string(ext) == ".dylib");

    // Parts the caller does not want.
    CHECK(_splitpath_s("a\\b.c", NULL, 0, dir, _MAX_DIR, NULL, 0, NULL, 0) == 0);
    CHECK(std::string(dir) == "a\\");

    char path[_MAX_PATH];
    CHECK(_makepath_s(path, _MAX_PATH, "C:", "\\dir\\", "name", "ext") == 0);
    CHECK(std::string(path) == "C:\\dir\\name.ext");
    CHECK(_makepath_s(path, _MAX_PATH, "", "/a/b/", "n", ".e") == 0);
    CHECK(std::string(path) == "/a/b/n.e");
    char small[4];
    CHECK(_makepath_s(small, sizeof(small), "C:", "\\dir\\", "name", "ext") != 0);
}

TEST_CASE("_fullpath makes a path absolute")
{
    char full[_MAX_PATH];
    char cwd[_MAX_PATH];
    REQUIRE(_getcwd(cwd, sizeof(cwd)) != NULL);
    REQUIRE(_fullpath(full, "some\\..\\dir/./file.txt", _MAX_PATH) != NULL);
    CHECK(std::string(full) == std::string(cwd) + "/dir/file.txt");
    REQUIRE(_fullpath(full, "/abs/x/../y", _MAX_PATH) != NULL);
    CHECK(std::string(full) == "/abs/y");
    char tiny[4];
    CHECK(_fullpath(tiny, "/abs/y", sizeof(tiny)) == NULL);
}

TEST_CASE("mbs functions behave as single-byte string functions")
{
    const unsigned char* text = reinterpret_cast<const unsigned char*>("Hello, World");
    CHECK(_mbschr(text, 'o') == text + 4);
    CHECK(_mbsrchr(text, 'o') == text + 8);
    CHECK(_mbschr(text, 'z') == NULL);
    CHECK(_mbsstr(text, reinterpret_cast<const unsigned char*>("World")) == text + 7);
    CHECK(_mbsspn(text, reinterpret_cast<const unsigned char*>("Hel")) == 4);
    CHECK(_mbscspn(text, reinterpret_cast<const unsigned char*>(",")) == 5);
    CHECK(_mbspbrk(text, reinterpret_cast<const unsigned char*>("xW")) == text + 7);
    CHECK(_mbscmp(reinterpret_cast<const unsigned char*>("a"), reinterpret_cast<const unsigned char*>("b")) < 0);
    CHECK(_mbsicmp(reinterpret_cast<const unsigned char*>("ABC"), reinterpret_cast<const unsigned char*>("abc")) == 0);
    CHECK(_mbsicmp(reinterpret_cast<const unsigned char*>("abd"), reinterpret_cast<const unsigned char*>("ABC")) > 0);
    CHECK(_mbsinc(text) == text + 1);

    unsigned char buffer[] = "Mixed";
    CHECK(_mbsupr_s(buffer, sizeof(buffer)) == 0);
    CHECK(std::string(reinterpret_cast<char*>(buffer)) == "MIXED");
    CHECK(_mbslwr_s(buffer, sizeof(buffer)) == 0);
    CHECK(std::string(reinterpret_cast<char*>(buffer)) == "mixed");
    CHECK(std::string(reinterpret_cast<char*>(_mbsrev(buffer))) == "dexim");
}

TEST_CASE("lstr functions and GlobalAlloc")
{
    char buffer[8];
    CHECK(lstrlenA("abc") == 3);
    CHECK(lstrlenA(NULL) == 0);
    lstrcpynA(buffer, "0123456789", sizeof(buffer));
    CHECK(std::string(buffer) == "0123456");
    lstrcpyA(buffer, "ab");
    lstrcatA(buffer, "cd");
    CHECK(std::string(buffer) == "abcd");

    HGLOBAL g = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, 16);
    REQUIRE(g != NULL);
    char* p = static_cast<char*>(GlobalLock(g));
    REQUIRE(p != NULL);
    strcpy(p, "clipboard");
    GlobalUnlock(g);
    CHECK(std::string(static_cast<char*>(GlobalLock(g))) == "clipboard");
    CHECK(GlobalFree(g) == NULL);
}

TEST_CASE("system: version, memory, language and the things that are switched off")
{
    OSVERSIONINFOEXA info;
    memset(&info, 0, sizeof(info));
    info.dwOSVersionInfoSize = sizeof(info);
    REQUIRE(GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&info)));
    CHECK(info.dwMajorVersion == 5);
    CHECK(info.dwMinorVersion == 1);
    CHECK(info.dwBuildNumber == 2600);
    CHECK(info.dwPlatformId == VER_PLATFORM_WIN32_NT);
    int servicePack = 0;
    sscanf(info.szCSDVersion, "Service Pack %d", &servicePack);
    CHECK(servicePack == 3);

    MEMORYSTATUS status;
    memset(&status, 0, sizeof(status));
    GlobalMemoryStatus(&status);
    CHECK(status.dwLength == sizeof(status));
    CHECK(status.dwTotalPhys > 0);
    CHECK(status.dwTotalPhys <= 0x7FFFFFFF);
    CHECK(status.dwAvailPhys <= 0x7FFFFFFF);
    CHECK(status.dwMemoryLoad == 50);

    CHECK(GetSystemDefaultLangID() == 0x0409);
    CHECK(GetACP() == 1252);

    CHECK(LoadLibraryA("binkw32") == NULL);
    CHECK(GetProcAddress(NULL, "Anything") == NULL);
    DWORD handle;
    CHECK(GetFileVersionInfoSizeA("file", &handle) == 0);

    HKEY key;
    CHECK(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "System", 0, KEY_READ, &key) == ERROR_FILE_NOT_FOUND);

    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    PROCESS_INFORMATION pi;
    CHECK_FALSE(CreateProcessA(NULL, const_cast<char*>("reg32.exe FALSE"), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi));

    char module[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, module, sizeof(module));
    CHECK(n > 0);
    CHECK(module[0] == '/');
    CHECK(GetModuleHandleA(NULL) != NULL);
    CHECK(GetModuleHandleA("Kernel32") == NULL);

    DWORD sectors, bytes, freeClusters, totalClusters;
    CHECK(GetDiskFreeSpaceA(NULL, &sectors, &bytes, &freeClusters, &totalClusters));
    CHECK(static_cast<unsigned long long>(freeClusters) * sectors < 0xFFFFFFFFull);
    CHECK(freeClusters * sectors >= 1500);
    CHECK(GetDriveTypeA("C:\\") == DRIVE_FIXED);
    CHECK((GetLogicalDrives() & 4) != 0);
}

TEST_CASE("winsock and COM: the network is down, DirectPlay is not registered")
{
    WSADATA wsa;
    CHECK(WSAStartup(MAKEWORD(2, 0), &wsa) == 0);
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    CHECK(s == INVALID_SOCKET);
    CHECK(WSAGetLastError() == WSAENETDOWN);
    CHECK(htons(0x1234) == 0x3412);
    CHECK(ntohs(htons(80)) == 80);
    CHECK(inet_addr("127.0.0.1") == 0x0100007Fu);
    CHECK(inet_addr("not an address") == INADDR_NONE);
    CHECK(WSACleanup() == 0);

    CHECK(CoInitialize(NULL) == S_OK);
    void* object = reinterpret_cast<void*>(1);
    CHECK(CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay4A, &object) == REGDB_E_CLASSNOTREG);
    CoUninitialize();
}
