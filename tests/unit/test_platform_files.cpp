// Win32 file functions of the shim (CreateFileA and friends, FindFirstFileA
// wildcards, attributes) and, on non-Windows builds, platform::TranslatePath.

#include "test.h"

#include <algorithm>
#include <string>
#include <vector>

#ifndef _WIN32
#include "../../src/platform/posix/path.h"
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace {

class Scratch {
public:
    Scratch()
    {
        char base[MAX_PATH];
        GetTempPathA(MAX_PATH, base);
        static int counter = 0;
        char name[MAX_PATH];
        sprintf(name, "%siwd2re-files-%lu-%d", base, static_cast<unsigned long>(GetCurrentProcessId()), counter++);
        m_path = name;
        CreateDirectoryA(m_path.c_str(), NULL);
    }

    ~Scratch() { RemoveTree(m_path); }

    std::string operator/(const char* name) const { return m_path + "/" + name; }
    const std::string& Path() const { return m_path; }

    static void RemoveTree(const std::string& dir)
    {
        WIN32_FIND_DATAA data;
        HANDLE h = FindFirstFileA((dir + "/*.*").c_str(), &data);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                std::string name = data.cFileName;
                if (name == "." || name == "..") {
                    continue;
                }
                std::string full = dir + "/" + name;
                SetFileAttributesA(full.c_str(), FILE_ATTRIBUTE_NORMAL);
                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    RemoveTree(full);
                } else {
                    DeleteFileA(full.c_str());
                }
            } while (FindNextFileA(h, &data));
            FindClose(h);
        }
        RemoveDirectoryA(dir.c_str());
    }

private:
    std::string m_path;
};

void Put(const std::string& path, const char* text)
{
    HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    DWORD n;
    WriteFile(h, text, static_cast<DWORD>(strlen(text)), &n, NULL);
    CloseHandle(h);
}

std::vector<std::string> Find(const std::string& pattern)
{
    std::vector<std::string> names;
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA(pattern.c_str(), &data);
    if (h == INVALID_HANDLE_VALUE) {
        return names;
    }
    do {
        names.push_back(data.cFileName);
    } while (FindNextFileA(h, &data));
    CHECK(GetLastError() == ERROR_NO_MORE_FILES);
    FindClose(h);
    std::sort(names.begin(), names.end());
    return names;
}

bool Has(const std::vector<std::string>& v, const char* name)
{
    return std::find(v.begin(), v.end(), name) != v.end();
}

// True when the volume of `dir` tells "A" from "a". On a case-insensitive
// volume (the macOS default) a path that exists as written is returned as
// written, so those checks compare without regard to case there.
bool VolumeIsCaseSensitive(const std::string& dir)
{
    Put(dir + "/CaseProbe", "x");
    bool sensitive = GetFileAttributesA((dir + "/caseprobe").c_str()) == INVALID_FILE_ATTRIBUTES;
    DeleteFileA((dir + "/CaseProbe").c_str());
    return sensitive;
}

std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = static_cast<char>(s[i] + 32);
    }
    return s;
}

bool SamePath(bool sensitive, const std::string& actual, const std::string& expected)
{
    return sensitive ? actual == expected : Lower(actual) == Lower(expected);
}

} // namespace

TEST_CASE("files: create, write, read, seek, size, truncate")
{
    Scratch dir;
    std::string path = dir / "data.bin";

    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    DWORD n;
    CHECK(WriteFile(h, "0123456789", 10, &n, NULL));
    CHECK(n == 10);
    CHECK(GetFileSize(h, NULL) == 10);

    CHECK(SetFilePointer(h, 0, NULL, FILE_BEGIN) == 0);
    char buffer[16] = { 0 };
    CHECK(ReadFile(h, buffer, 4, &n, NULL));
    CHECK(n == 4);
    CHECK(std::string(buffer, 4) == "0123");
    CHECK(SetFilePointer(h, 2, NULL, FILE_CURRENT) == 6);
    CHECK(SetFilePointer(h, -3, NULL, 2 /* FILE_END */) == 7);
    CHECK(ReadFile(h, buffer, 16, &n, NULL));
    CHECK(n == 3);
    CHECK(std::string(buffer, 3) == "789");
    // At the end: success with nothing read.
    CHECK(ReadFile(h, buffer, 16, &n, NULL));
    CHECK(n == 0);

    // A seek before the start fails.
    CHECK(SetFilePointer(h, -1, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER);
    CHECK(GetLastError() == ERROR_NEGATIVE_SEEK);

    CHECK(SetFilePointer(h, 4, NULL, FILE_BEGIN) == 4);
    CHECK(SetEndOfFile(h));
    CHECK(GetFileSize(h, NULL) == 4);
    CHECK(FlushFileBuffers(h));

    FILETIME created, accessed, written;
    CHECK(GetFileTime(h, &created, &accessed, &written));
    CHECK(written.dwHighDateTime != 0);
    CloseHandle(h);
}

TEST_CASE("files: creation dispositions and their error codes")
{
    Scratch dir;
    std::string path = dir / "f.txt";

    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(h == INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == ERROR_FILE_NOT_FOUND);

    h = CreateFileA((dir / "no/such/f.txt").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(h == INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == ERROR_PATH_NOT_FOUND);

    h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    DWORD n;
    WriteFile(h, "abc", 3, &n, NULL);
    CloseHandle(h);

    h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(h == INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == ERROR_FILE_EXISTS);

    h = CreateFileA(path.c_str(), GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == ERROR_ALREADY_EXISTS);
    CHECK(GetFileSize(h, NULL) == 3);
    CloseHandle(h);

    h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == ERROR_ALREADY_EXISTS);
    CHECK(GetFileSize(h, NULL) == 0);
    CloseHandle(h);

    h = CreateFileA((dir / "fresh.txt").c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == 0);
    CloseHandle(h);

    // A directory cannot be opened as a file.
    h = CreateFileA(dir.Path().c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(h == INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == ERROR_ACCESS_DENIED);
}

TEST_CASE("files: attributes, read-only and the INVALID value")
{
    Scratch dir;
    std::string path = dir / "attr.txt";
    Put(path, "x");

    DWORD a = GetFileAttributesA(path.c_str());
    CHECK(a != INVALID_FILE_ATTRIBUTES);
    CHECK((a & FILE_ATTRIBUTE_DIRECTORY) == 0);
    CHECK((a & FILE_ATTRIBUTE_READONLY) == 0);

    CHECK(SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_READONLY));
    a = GetFileAttributesA(path.c_str());
    CHECK((a & FILE_ATTRIBUTE_READONLY) != 0);

    // Read-only files are not deleted.
    CHECK_FALSE(DeleteFileA(path.c_str()));
    CHECK(GetLastError() == ERROR_ACCESS_DENIED);
    CHECK(SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_NORMAL));
    CHECK((GetFileAttributesA(path.c_str()) & FILE_ATTRIBUTE_READONLY) == 0);

    a = GetFileAttributesA(dir.Path().c_str());
    CHECK((a & FILE_ATTRIBUTE_DIRECTORY) != 0);

    CHECK(GetFileAttributesA((dir / "missing").c_str()) == 0xFFFFFFFFu);
    CHECK(GetLastError() == ERROR_FILE_NOT_FOUND);

    WIN32_FILE_ATTRIBUTE_DATA data;
    CHECK(GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &data));
    CHECK(data.nFileSizeLow == 1);
    CHECK(data.nFileSizeHigh == 0);
    CHECK_FALSE(GetFileAttributesExA((dir / "missing").c_str(), GetFileExInfoStandard, &data));
}

TEST_CASE("files: copy, move, delete, directories")
{
    Scratch dir;
    std::string a = dir / "a.txt";
    std::string b = dir / "b.txt";
    Put(a, "hello");

    CHECK(CopyFileA(a.c_str(), b.c_str(), TRUE));
    CHECK_FALSE(CopyFileA(a.c_str(), b.c_str(), TRUE));
    CHECK(GetLastError() == ERROR_FILE_EXISTS);
    Put(a, "changed");
    CHECK(CopyFileA(a.c_str(), b.c_str(), FALSE));

    HANDLE h = CreateFileA(b.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK(GetFileSize(h, NULL) == 7);
    CloseHandle(h);

    std::string c = dir / "c.txt";
    CHECK(MoveFileA(b.c_str(), c.c_str()));
    CHECK(GetFileAttributesA(b.c_str()) == INVALID_FILE_ATTRIBUTES);
    CHECK(GetFileAttributesA(c.c_str()) != INVALID_FILE_ATTRIBUTES);
    CHECK_FALSE(MoveFileA(a.c_str(), c.c_str()));
    CHECK(GetLastError() == ERROR_ALREADY_EXISTS);

    CHECK(DeleteFileA(c.c_str()));
    CHECK_FALSE(DeleteFileA(c.c_str()));
    CHECK(GetLastError() == ERROR_FILE_NOT_FOUND);

    std::string sub = dir / "sub";
    CHECK(CreateDirectoryA(sub.c_str(), NULL));
    CHECK_FALSE(CreateDirectoryA(sub.c_str(), NULL));
    CHECK(GetLastError() == ERROR_ALREADY_EXISTS);
    CHECK_FALSE(CreateDirectoryA((dir / "x/y").c_str(), NULL));
    CHECK(GetLastError() == ERROR_PATH_NOT_FOUND);
    Put(sub + "/inner.txt", "1");
    CHECK_FALSE(RemoveDirectoryA(sub.c_str()));
    CHECK(GetLastError() == ERROR_DIR_NOT_EMPTY);
    CHECK(DeleteFileA((sub + "/inner.txt").c_str()));
    CHECK(RemoveDirectoryA(sub.c_str()));
}

TEST_CASE("files: current directory")
{
    char before[MAX_PATH];
    DWORD n = GetCurrentDirectoryA(sizeof(before), before);
    REQUIRE(n > 0);
    CHECK(n == strlen(before));

    // A buffer that is too small gets the needed size, with the terminator.
    char tiny[2];
    CHECK(GetCurrentDirectoryA(sizeof(tiny), tiny) == n + 1);

    Scratch dir;
    CHECK(SetCurrentDirectoryA(dir.Path().c_str()));
    Put("relative.txt", "r");
    CHECK(GetFileAttributesA((dir / "relative.txt").c_str()) != INVALID_FILE_ATTRIBUTES);
    CHECK(SetCurrentDirectoryA(before));
    CHECK_FALSE(SetCurrentDirectoryA((dir / "missing").c_str()));
}

TEST_CASE("find: wildcards match as on Windows")
{
    Scratch dir;
    Put(dir / "alpha.txt", "1");
    Put(dir / "Beta.TXT", "22");
    Put(dir / "gamma.dat", "333");
    Put(dir / "noext", "4444");
    Put(dir / "ab", "5");
    CreateDirectoryA((dir / "subdir").c_str(), NULL);

    std::vector<std::string> all = Find(dir / "*");
    CHECK(Has(all, "."));
    CHECK(Has(all, ".."));
    CHECK(Has(all, "alpha.txt"));
    CHECK(Has(all, "subdir"));
    CHECK(all.size() == 8);

    // `*.*` matches everything, also names without a dot.
    std::vector<std::string> star = Find(dir / "*.*");
    CHECK(star == all);

    // Case-insensitive, `*` and `?`.
    std::vector<std::string> txt = Find(dir / "*.txt");
    REQUIRE(txt.size() == 2);
    CHECK(Has(txt, "alpha.txt"));
    CHECK(Has(txt, "Beta.TXT"));

    std::vector<std::string> q = Find(dir / "a?");
    REQUIRE(q.size() == 1);
    CHECK(q[0] == "ab");

    std::vector<std::string> prefix = Find(dir / "ALPHA*");
    REQUIRE(prefix.size() == 1);

    // A plain name finds that file; nothing found is FILE_NOT_FOUND.
    CHECK(Find(dir / "gamma.dat").size() == 1);
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA((dir / "nothing*").c_str(), &data);
    CHECK(h == INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == ERROR_FILE_NOT_FOUND);

    // A folder that is not there.
    h = FindFirstFileA((dir / "nodir/*.*").c_str(), &data);
    CHECK(h == INVALID_HANDLE_VALUE);
    CHECK(GetLastError() == ERROR_PATH_NOT_FOUND);
}

TEST_CASE("find: attributes, sizes and times are filled in")
{
    Scratch dir;
    Put(dir / "five.bin", "12345");
    CreateDirectoryA((dir / "folder").c_str(), NULL);

    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA((dir / "five.bin").c_str(), &data);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK(std::string(data.cFileName) == "five.bin");
    CHECK(data.nFileSizeLow == 5);
    CHECK(data.nFileSizeHigh == 0);
    CHECK((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
    CHECK(data.ftLastWriteTime.dwHighDateTime != 0);
    CHECK_FALSE(FindNextFileA(h, &data));
    CHECK(GetLastError() == ERROR_NO_MORE_FILES);
    CHECK(FindClose(h));

    h = FindFirstFileA((dir / "folder").c_str(), &data);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    FindClose(h);
}

TEST_CASE("time: FILETIME and SYSTEMTIME convert both ways")
{
    SYSTEMTIME st = { 2002, 10, 0, 16, 15, 36, 32, 125 };
    FILETIME ft;
    REQUIRE(SystemTimeToFileTime(&st, &ft));
    SYSTEMTIME back;
    REQUIRE(FileTimeToSystemTime(&ft, &back));
    CHECK(back.wYear == 2002);
    CHECK(back.wMonth == 10);
    CHECK(back.wDay == 16);
    CHECK(back.wHour == 15);
    CHECK(back.wMinute == 36);
    CHECK(back.wSecond == 32);
    CHECK(back.wMilliseconds == 125);
    CHECK(back.wDayOfWeek == 3); // a Wednesday

    // The epoch of FILETIME is 1601-01-01, a Monday.
    FILETIME zero = { 0, 0 };
    REQUIRE(FileTimeToSystemTime(&zero, &back));
    CHECK(back.wYear == 1601);
    CHECK(back.wMonth == 1);
    CHECK(back.wDay == 1);
    CHECK(back.wDayOfWeek == 1);

    st.wMonth = 13;
    CHECK_FALSE(SystemTimeToFileTime(&st, &ft));
}

#ifndef _WIN32

TEST_CASE("TranslatePath: slashes, drive letters, case")
{
    Scratch dir;
    CreateDirectoryA((dir / "Data").c_str(), NULL);
    CreateDirectoryA((dir / "Data/Movies").c_str(), NULL);
    Put(dir / "Data/Movies/Intro.BIK", "bik");
    Put(dir / "Data/chitin.KEY", "key");

    const std::string root = dir.Path();
    const bool cs = VolumeIsCaseSensitive(root);

    // Backslashes become slashes.
    std::string withBackslashes = root + "\\Data\\chitin.KEY";
    CHECK(SamePath(cs, platform::TranslatePath(withBackslashes.c_str()), root + "/Data/chitin.KEY"));

    // Any case, component by component. The result names the real entries.
    std::string resolved = platform::TranslatePath((root + "/DATA/movies/intro.bik").c_str());
    struct stat a, b;
    REQUIRE(stat(resolved.c_str(), &a) == 0);
    REQUIRE(stat((root + "/Data/Movies/Intro.BIK").c_str(), &b) == 0);
    CHECK(a.st_ino == b.st_ino);
    CHECK(SamePath(cs, resolved.substr(resolved.size() - 22), "/Data/Movies/Intro.BIK"));

    // Components that do not exist are kept as given, after the ones that do
    // have been fixed, so that new files can be created.
    resolved = platform::TranslatePath((root + "/data/NewFile.TXT").c_str());
    CHECK(SamePath(cs, resolved, root + "/Data/NewFile.TXT"));
    resolved = platform::TranslatePath((root + "/data/Sub/Deeper/x.y").c_str());
    CHECK(SamePath(cs, resolved, root + "/Data/Sub/Deeper/x.y"));

    // A drive letter is dropped when the rest exists as an absolute path.
    std::string drive = "C:" + root + "/DATA/CHITIN.KEY";
    CHECK(SamePath(cs, platform::TranslatePath(drive.c_str()), root + "/Data/chitin.KEY"));

    // Otherwise it is kept.
    CHECK(platform::TranslatePath("C:\\no\\such\\dir\\x.ini") == "C:/no/such/dir/x.ini");

    // Relative names are resolved against the current directory.
    char before[MAX_PATH];
    GetCurrentDirectoryA(sizeof(before), before);
    REQUIRE(chdir(root.c_str()) == 0);
    CHECK(SamePath(cs, platform::TranslatePath("DATA\\MOVIES"), "Data/Movies"));
    CHECK(SamePath(cs, platform::TranslatePath("data/CHITIN.key"), "Data/chitin.KEY"));
    REQUIRE(chdir(before) == 0);

    // The file functions go through it.
    HANDLE h = CreateFileA((root + "\\DATA\\CHITIN.key").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK(GetFileSize(h, NULL) == 3);
    CloseHandle(h);
    CHECK(Find(root + "\\data\\movies\\*.bik").size() == 1);
}

TEST_CASE("fopen, _chdir and _mkdir take Windows paths")
{
    Scratch dir;
    const std::string root = dir.Path();

    CHECK(_mkdir((root + "\\Made").c_str()) == 0);
    FILE* f = fopen((root + "\\made\\Out.txt").c_str(), "wt");
    REQUIRE(f != NULL);
    fputs("line\n", f);
    fclose(f);

    f = fopen((root + "/MADE/out.TXT").c_str(), "r");
    REQUIRE(f != NULL);
    char line[16] = { 0 };
    CHECK(fgets(line, sizeof(line), f) != NULL);
    CHECK(std::string(line) == "line\n");
    fclose(f);

    char before[MAX_PATH];
    GetCurrentDirectoryA(sizeof(before), before);
    CHECK(_chdir((root + "\\MADE").c_str()) == 0);
    CHECK(_chdir(before) == 0);
    CHECK(_chdir((root + "\\nothing").c_str()) != 0);
}

#endif
