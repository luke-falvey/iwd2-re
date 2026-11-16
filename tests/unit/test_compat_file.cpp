// CFile, CFileFind, CFileStatus, CTime, CException, locks and the few other
// MFC pieces the game uses. Runs against MFC in the default build and against
// src/compat in the IWD2RE_NO_MFC build. Works in a scratch folder under the
// user's temporary directory.

#include "test.h"

#include <mbctype.h>

#include <algorithm>
#include <string>
#include <thread>
#include <vector>

namespace {

// A scratch folder that is removed with everything in it.
class TempDir {
public:
    TempDir()
    {
        char base[MAX_PATH];
        GetTempPathA(MAX_PATH, base);
        char name[MAX_PATH];
        sprintf(name, "%siwd2re-compat-%lu-%lu", base, GetCurrentProcessId(), GetTickCount());
        m_path = name;
        CreateDirectoryA(m_path.c_str(), NULL);
    }

    ~TempDir() { RemoveTree(m_path); }

    std::string Path(const char* name) const { return m_path + "\\" + name; }
    const std::string& Root() const { return m_path; }

    void WriteFile(const char* name, const std::string& content) const
    {
        HANDLE h = CreateFileA(Path(name).c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        DWORD written;
        ::WriteFile(h, content.data(), static_cast<DWORD>(content.size()), &written, NULL);
        CloseHandle(h);
    }

    std::string ReadFile(const char* name) const
    {
        std::string content;
        HANDLE h = CreateFileA(Path(name).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (h == INVALID_HANDLE_VALUE) {
            return "<missing>";
        }
        char buf[256];
        DWORD read;
        while (::ReadFile(h, buf, sizeof(buf), &read, NULL) && read > 0) {
            content.append(buf, read);
        }
        CloseHandle(h);
        return content;
    }

    bool Exists(const char* name) const { return GetFileAttributesA(Path(name).c_str()) != INVALID_FILE_ATTRIBUTES; }

private:
    static void RemoveTree(const std::string& dir)
    {
        WIN32_FIND_DATAA data;
        HANDLE h = FindFirstFileA((dir + "\\*").c_str(), &data);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                std::string name = data.cFileName;
                if (name == "." || name == "..") {
                    continue;
                }
                std::string full = dir + "\\" + name;
                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    RemoveTree(full);
                } else {
                    SetFileAttributesA(full.c_str(), FILE_ATTRIBUTE_NORMAL);
                    DeleteFileA(full.c_str());
                }
            } while (FindNextFileA(h, &data));
            FindClose(h);
        }
        RemoveDirectoryA(dir.c_str());
    }

    std::string m_path;
};

std::string S(const CString& s)
{
    return std::string(static_cast<LPCSTR>(s), s.GetLength());
}

std::string Lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return s;
}

std::string FullPath(const std::string& path)
{
    char buf[MAX_PATH];
    GetFullPathNameA(path.c_str(), MAX_PATH, buf, NULL);
    return buf;
}

ULONGLONG FileTimeValue(const FILETIME& ft)
{
    return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

// Seconds since 1970 (UTC) of a file time.
__time64_t UnixTime(const FILETIME& ft)
{
    return static_cast<__time64_t>((FileTimeValue(ft) - 116444736000000000ULL) / 10000000ULL);
}

FILETIME FileTimeFromUnix(__time64_t t)
{
    ULONGLONG value = static_cast<ULONGLONG>(t) * 10000000ULL + 116444736000000000ULL;
    FILETIME ft;
    ft.dwLowDateTime = static_cast<DWORD>(value);
    ft.dwHighDateTime = static_cast<DWORD>(value >> 32);
    return ft;
}

// The modification time stored by the file system.
ULONGLONG RawWriteTime(const std::string& path)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &data)) {
        return 0;
    }
    return FileTimeValue(data.ftLastWriteTime);
}

// Exposes the protected members whose layout the game depends on.
class CProbeFile : public CFile {
public:
    size_t CloseOnDeleteOffset() const { return reinterpret_cast<const char*>(&m_bCloseOnDelete) - reinterpret_cast<const char*>(static_cast<const CFile*>(this)); }
    size_t FileNameOffset() const { return reinterpret_cast<const char*>(&m_strFileName) - reinterpret_cast<const char*>(static_cast<const CFile*>(this)); }
    BOOL CloseOnDelete() const { return m_bCloseOnDelete; }
};

} // namespace

TEST_CASE("file and lock object sizes (32-bit)")
{
#if defined(_M_IX86)
    CHECK(sizeof(CFile) == 0x14);
    CHECK(offsetof(CFile, m_hFile) == 0x4);
    CProbeFile probe;
    CHECK(probe.CloseOnDeleteOffset() == 0x8);
    CHECK(probe.FileNameOffset() == 0xC);

    CHECK(sizeof(CFileStatus) == 0x128);
    CHECK(offsetof(CFileStatus, m_ctime) == 0x0);
    CHECK(offsetof(CFileStatus, m_mtime) == 0x8);
    CHECK(offsetof(CFileStatus, m_atime) == 0x10);
    CHECK(offsetof(CFileStatus, m_size) == 0x18);
    CHECK(offsetof(CFileStatus, m_attribute) == 0x20);
    CHECK(offsetof(CFileStatus, m_szFullName) == 0x24);

    CHECK(sizeof(CFileFind) == 0x1C);
    CHECK(sizeof(CTime) == 0x8);
    CHECK(sizeof(CException) == 0x8);
    CHECK(sizeof(CFileException) == 0x14);
    CHECK(offsetof(CFileException, m_cause) == 0x8);
    CHECK(offsetof(CFileException, m_lOsError) == 0xC);
    CHECK(offsetof(CFileException, m_strFileName) == 0x10);

    CHECK(sizeof(CCriticalSection) == 0x20);
    CHECK(offsetof(CCriticalSection, m_hObject) == 0x4);
    CHECK(offsetof(CCriticalSection, m_sect) == 0x8);
    CHECK(sizeof(CSingleLock) == 0xC);

    CHECK(sizeof(CWnd) == 0x80);
    CHECK(offsetof(CWnd, m_hWnd) == 0x20);
#endif
}

TEST_CASE("CFile flag values")
{
    CHECK(CFile::modeRead == 0x0000);
    CHECK(CFile::modeWrite == 0x0001);
    CHECK(CFile::modeReadWrite == 0x0002);
    CHECK(CFile::shareCompat == 0x0000);
    CHECK(CFile::shareExclusive == 0x0010);
    CHECK(CFile::shareDenyWrite == 0x0020);
    CHECK(CFile::shareDenyRead == 0x0030);
    CHECK(CFile::shareDenyNone == 0x0040);
    CHECK(CFile::modeNoInherit == 0x0080);
    CHECK(CFile::modeCreate == 0x1000);
    CHECK(CFile::modeNoTruncate == 0x2000);
    CHECK(CFile::typeText == 0x4000);
    CHECK(CFile::typeBinary == 0x8000);
    CHECK(CFile::begin == 0);
    CHECK(CFile::current == 1);
    CHECK(CFile::end == 2);
    CHECK(CFile::OpenFlags::modeReadWrite == 2);
    CHECK(CFile::SeekPosition::end == 2);
    CHECK(CFile::hFileNull == INVALID_HANDLE_VALUE);

    CHECK(CFileException::none == 0);
    CHECK(CFileException::fileNotFound == 2);
    CHECK(CFileException::badPath == 3);
    CHECK(CFileException::accessDenied == 5);
    CHECK(CFileException::sharingViolation == 11);
    CHECK(CFileException::diskFull == 13);
    CHECK(CFileException::endOfFile == 14);
}

TEST_CASE("CFile write, seek and read")
{
    TempDir dir;
    std::string path = dir.Path("data.bin");

    CFile file;
    CHECK(file.m_hFile == CFile::hFileNull);
    REQUIRE(file.Open(path.c_str(), CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary));
    CHECK(file.m_hFile != CFile::hFileNull);
    CHECK(file.GetLength() == 0);
    CHECK(file.GetPosition() == 0);

    file.Write("0123456789", 10);
    CHECK(file.GetLength() == 10);
    CHECK(file.GetPosition() == 10);
    file.Write("", 0);
    CHECK(file.GetLength() == 10);

    CHECK(file.Seek(2, CFile::begin) == 2);
    char buf[32];
    memset(buf, '#', sizeof(buf));
    CHECK(file.Read(buf, 3) == 3);
    CHECK(std::string(buf, 4) == "234#");
    CHECK(file.GetPosition() == 5);

    CHECK(file.Seek(2, CFile::current) == 7);
    CHECK(file.Seek(-3, CFile::current) == 4);
    CHECK(file.Seek(-1, CFile::end) == 9);
    CHECK(file.Read(buf, 1) == 1);
    CHECK(buf[0] == '9');
    CHECK(file.Seek(0, CFile::end) == 10);

    // Reading at and across the end gives a short count, not an error.
    CHECK(file.Read(buf, 10) == 0);
    file.Seek(6, CFile::begin);
    memset(buf, '#', sizeof(buf));
    CHECK(file.Read(buf, 32) == 4);
    CHECK(std::string(buf, 5) == "6789#");
    CHECK(file.Read(buf, 0) == 0);

    // Seeking past the end and writing extends the file.
    CHECK(file.Seek(4, CFile::end) == 14);
    file.Write("AB", 2);
    CHECK(file.GetLength() == 16);

    // Overwriting in the middle.
    file.SeekToBegin();
    CHECK(file.GetPosition() == 0);
    file.Write("ab", 2);
    CHECK(file.SeekToEnd() == 16);

    file.SetLength(12);
    CHECK(file.GetLength() == 12);
    file.Flush();
    file.Close();
    CHECK(file.m_hFile == CFile::hFileNull);

    CHECK(dir.ReadFile("data.bin") == std::string("ab23456789\0\0", 12));
}

TEST_CASE("CFile open modes")
{
    TempDir dir;
    dir.WriteFile("exist.txt", "existing content");
    std::string exist = dir.Path("exist.txt");
    std::string fresh = dir.Path("fresh.txt");
    char buf[64];

    SUBCASE("modeRead opens an existing file for reading only")
    {
        CFile file;
        REQUIRE(file.Open(exist.c_str(), CFile::modeRead | CFile::typeBinary, NULL));
        CHECK(file.GetLength() == 16);
        CHECK(file.Read(buf, 64) == 16);
        CHECK(std::string(buf, 16) == "existing content");
        file.Close();
    }

    SUBCASE("a missing file fails without modeCreate and is not created")
    {
        CFile file;
        CHECK_FALSE(file.Open(fresh.c_str(), CFile::modeRead));
        CHECK_FALSE(file.Open(fresh.c_str(), CFile::modeWrite));
        CHECK_FALSE(file.Open(fresh.c_str(), CFile::modeReadWrite | CFile::modeNoTruncate));
        CHECK(file.m_hFile == CFile::hFileNull);
        CHECK_FALSE(dir.Exists("fresh.txt"));
    }

    SUBCASE("modeCreate creates a missing file")
    {
        CFile file;
        REQUIRE(file.Open(fresh.c_str(), CFile::modeCreate | CFile::modeWrite | CFile::typeBinary, NULL));
        file.Write("new", 3);
        file.Close();
        CHECK(dir.ReadFile("fresh.txt") == "new");
    }

    SUBCASE("modeCreate truncates an existing file")
    {
        CFile file;
        REQUIRE(file.Open(exist.c_str(), CFile::modeCreate | CFile::modeWrite));
        CHECK(file.GetLength() == 0);
        file.Close();
        CHECK(dir.ReadFile("exist.txt") == "");
    }

    SUBCASE("modeCreate with modeNoTruncate keeps the content")
    {
        CFile file;
        REQUIRE(file.Open(exist.c_str(), CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite));
        CHECK(file.GetLength() == 16);
        CHECK(file.GetPosition() == 0);
        file.SeekToEnd();
        file.Write("!", 1);
        file.Close();
        CHECK(dir.ReadFile("exist.txt") == "existing content!");

        CFile created;
        REQUIRE(created.Open(fresh.c_str(), CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite));
        CHECK(created.GetLength() == 0);
        created.Close();
        CHECK(dir.Exists("fresh.txt"));
    }

    SUBCASE("modeWrite on an existing file keeps the content and starts at 0")
    {
        CFile file;
        REQUIRE(file.Open(exist.c_str(), CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone));
        CHECK(file.GetPosition() == 0);
        file.Write("EXIST", 5);
        file.Close();
        CHECK(dir.ReadFile("exist.txt") == "EXISTing content");
    }

    SUBCASE("the object can be opened again after Close")
    {
        CFile file;
        REQUIRE(file.Open(exist.c_str(), CFile::modeRead));
        file.Close();
        REQUIRE(file.Open(exist.c_str(), CFile::modeRead));
        CHECK(file.Read(buf, 5) == 5);
        file.Close();
    }

    SUBCASE("the destructor closes the file")
    {
        {
            CFile file;
            REQUIRE(file.Open(exist.c_str(), CFile::modeReadWrite | CFile::shareExclusive));
        }
        CFile again;
        CHECK(again.Open(exist.c_str(), CFile::modeReadWrite | CFile::shareExclusive));
    }

    SUBCASE("the throwing constructor opens the file")
    {
        CFile file(exist.c_str(), CFile::modeRead);
        CHECK(file.GetLength() == 16);
    }
}

TEST_CASE("CFile sharing modes")
{
    TempDir dir;
    dir.WriteFile("shared.txt", "x");
    std::string path = dir.Path("shared.txt");

    SUBCASE("default sharing is exclusive")
    {
        CFile first;
        REQUIRE(first.Open(path.c_str(), CFile::modeRead));
        CFile second;
        CHECK_FALSE(second.Open(path.c_str(), CFile::modeRead));
        CHECK_FALSE(second.Open(path.c_str(), CFile::modeRead | CFile::shareDenyNone));
    }

    SUBCASE("shareExclusive")
    {
        CFile first;
        REQUIRE(first.Open(path.c_str(), CFile::modeRead | CFile::shareExclusive));
        CFile second;
        CHECK_FALSE(second.Open(path.c_str(), CFile::modeRead | CFile::shareDenyNone));
    }

    SUBCASE("shareDenyWrite lets others read but not write")
    {
        CFile first;
        REQUIRE(first.Open(path.c_str(), CFile::modeRead | CFile::shareDenyWrite));
        CFile reader;
        CHECK(reader.Open(path.c_str(), CFile::modeRead | CFile::shareDenyWrite));
        CFile reader2;
        CHECK(reader2.Open(path.c_str(), CFile::modeRead | CFile::shareDenyNone));
        CFile writer;
        CFileException error;
        CHECK_FALSE(writer.Open(path.c_str(), CFile::modeWrite | CFile::shareDenyNone, &error));
        CHECK(error.m_cause == CFileException::sharingViolation);
        CHECK(error.m_lOsError == ERROR_SHARING_VIOLATION);
    }

    SUBCASE("shareDenyRead lets others write but not read")
    {
        CFile first;
        REQUIRE(first.Open(path.c_str(), CFile::modeWrite | CFile::shareDenyRead));
        CFile writer;
        CHECK(writer.Open(path.c_str(), CFile::modeWrite | CFile::shareDenyNone));
        CFile reader;
        CHECK_FALSE(reader.Open(path.c_str(), CFile::modeRead | CFile::shareDenyNone));
    }

    SUBCASE("shareDenyNone lets others read and write")
    {
        CFile first;
        REQUIRE(first.Open(path.c_str(), CFile::modeReadWrite | CFile::shareDenyNone));
        CFile second;
        CHECK(second.Open(path.c_str(), CFile::modeReadWrite | CFile::shareDenyNone));
    }
}

TEST_CASE("CFile errors")
{
    TempDir dir;
    dir.WriteFile("exist.txt", "content");
    std::string exist = dir.Path("exist.txt");
    char buf[16];

    SUBCASE("Open reports the failure through the exception object")
    {
        CFile file;
        CFileException error;
        CHECK_FALSE(file.Open(dir.Path("missing.txt").c_str(), CFile::modeRead, &error));
        CHECK(error.m_cause == CFileException::fileNotFound);
        CHECK(error.m_lOsError == ERROR_FILE_NOT_FOUND);
        CHECK(Lower(S(error.m_strFileName)) == Lower(dir.Path("missing.txt")));

        CFileException error2;
        CHECK_FALSE(file.Open(dir.Path("no-such-dir\\missing.txt").c_str(), CFile::modeRead, &error2));
        CHECK(error2.m_cause == CFileException::badPath);
        CHECK(error2.m_lOsError == ERROR_PATH_NOT_FOUND);

        // A directory cannot be opened as a file.
        CFileException error3;
        CHECK_FALSE(file.Open(dir.Root().c_str(), CFile::modeRead, &error3));
        CHECK(error3.m_cause == CFileException::accessDenied);
    }

    SUBCASE("the constructor throws a CFileException pointer")
    {
        bool caught = false;
        try {
            CFile file(dir.Path("missing.txt").c_str(), CFile::modeRead);
        } catch (CFileException* e) {
            caught = true;
            CHECK(e->m_cause == CFileException::fileNotFound);
            CHECK(e->m_lOsError == ERROR_FILE_NOT_FOUND);
            e->Delete();
        }
        CHECK(caught);
    }

    SUBCASE("file exceptions are caught as CException pointers")
    {
        // The game's handlers all have this form.
        bool caught = false;
        try {
            CFile::Remove(dir.Path("missing.txt").c_str());
        } catch (CException* e) {
            caught = true;
            e->Delete();
        }
        CHECK(caught);
    }

    SUBCASE("reading a file that is not open throws")
    {
        CFile file;
        bool caught = false;
        try {
            file.Read(buf, 4);
        } catch (CException* e) {
            caught = true;
            e->Delete();
        }
        CHECK(caught);
    }

    SUBCASE("writing a file opened for reading throws")
    {
        CFile file;
        REQUIRE(file.Open(exist.c_str(), CFile::modeRead));
        bool caught = false;
        try {
            file.Write("x", 1);
        } catch (CFileException* e) {
            caught = true;
            CHECK(e->m_cause == CFileException::accessDenied);
            CHECK(e->m_lOsError == ERROR_ACCESS_DENIED);
            e->Delete();
        }
        CHECK(caught);
        file.Close();
    }

    SUBCASE("reading a file opened for writing throws")
    {
        CFile file;
        REQUIRE(file.Open(exist.c_str(), CFile::modeWrite));
        bool caught = false;
        try {
            file.Read(buf, 1);
        } catch (CException* e) {
            caught = true;
            e->Delete();
        }
        CHECK(caught);
        file.Close();
    }

    SUBCASE("seeking before the start throws and leaves the position")
    {
        CFile file;
        REQUIRE(file.Open(exist.c_str(), CFile::modeRead));
        file.Seek(3, CFile::begin);
        bool caught = false;
        try {
            file.Seek(-10, CFile::begin);
        } catch (CFileException* e) {
            caught = true;
            CHECK(e->m_cause == CFileException::badSeek);
            e->Delete();
        }
        CHECK(caught);
        CHECK(file.GetPosition() == 3);
        file.Close();
    }
}

TEST_CASE("CFile names")
{
    TempDir dir;
    dir.WriteFile("Name.Ext", "x");
    std::string path = dir.Path("Name.Ext");

    CFile file;
    CHECK(S(file.GetFilePath()) == "");
    CHECK(S(file.GetFileName()) == "");

    REQUIRE(file.Open(path.c_str(), CFile::modeRead));
    CHECK(Lower(S(file.GetFilePath())) == Lower(FullPath(path)));
    CHECK(S(file.GetFileName()) == "Name.Ext");

    // Closing forgets the name.
    file.Close();
    CHECK(S(file.GetFilePath()) == "");

    // A relative name is stored as a full path.
    char oldDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, oldDir);
    SetCurrentDirectoryA(dir.Root().c_str());
    REQUIRE(file.Open("Name.Ext", CFile::modeRead));
    CHECK(Lower(S(file.GetFilePath())) == Lower(FullPath(path)));
    file.Close();
    REQUIRE(file.Open(".\\Name.Ext", CFile::modeRead));
    CHECK(Lower(S(file.GetFilePath())) == Lower(FullPath(path)));
    file.Close();
    SetCurrentDirectoryA(oldDir);

    // A path set by hand is returned as given, and the game opens files by
    // the path stored on the object (CTlkTable::OpenFile).
    file.SetFilePath(path.c_str());
    CHECK(S(file.GetFilePath()) == path);
    CHECK(S(file.GetFileName()) == "Name.Ext");
    REQUIRE(file.Open(file.GetFilePath(), CFile::modeRead | CFile::shareDenyWrite));
    char ch = 0;
    CHECK(file.Read(&ch, 1) == 1);
    CHECK(ch == 'x');
    file.Close();

    // A failed Open leaves the object closed.
    CHECK_FALSE(file.Open(dir.Path("missing").c_str(), CFile::modeRead));
    CHECK(file.m_hFile == CFile::hFileNull);
}

TEST_CASE("CFile::Remove and CFile::Rename")
{
    TempDir dir;
    dir.WriteFile("a.txt", "a");

    CFile::Rename(dir.Path("a.txt").c_str(), dir.Path("b.txt").c_str());
    CHECK_FALSE(dir.Exists("a.txt"));
    CHECK(dir.ReadFile("b.txt") == "a");

    CFile::Remove(dir.Path("b.txt").c_str());
    CHECK_FALSE(dir.Exists("b.txt"));

    bool caught = false;
    try {
        CFile::Remove(dir.Path("b.txt").c_str());
    } catch (CFileException* e) {
        caught = true;
        CHECK(e->m_cause == CFileException::fileNotFound);
        e->Delete();
    }
    CHECK(caught);

    caught = false;
    try {
        CFile::Rename(dir.Path("missing.txt").c_str(), dir.Path("c.txt").c_str());
    } catch (CException* e) {
        caught = true;
        e->Delete();
    }
    CHECK(caught);

    // A file that is open cannot be removed.
    dir.WriteFile("open.txt", "o");
    CFile file;
    REQUIRE(file.Open(dir.Path("open.txt").c_str(), CFile::modeRead | CFile::shareDenyNone));
    caught = false;
    try {
        CFile::Remove(dir.Path("open.txt").c_str());
    } catch (CFileException* e) {
        caught = true;
        CHECK(e->m_cause == CFileException::sharingViolation);
        e->Delete();
    }
    CHECK(caught);
    file.Close();
    CFile::Remove(dir.Path("open.txt").c_str());
    CHECK_FALSE(dir.Exists("open.txt"));
}

TEST_CASE("CFile::GetStatus and CFile::SetStatus")
{
    TempDir dir;
    dir.WriteFile("status.txt", "twelve bytes");
    CreateDirectoryA(dir.Path("sub").c_str(), NULL);
    std::string path = dir.Path("status.txt");

    CFileStatus status;
    REQUIRE(CFile::GetStatus(path.c_str(), status));
    CHECK(status.m_size == 12);
    CHECK((status.m_attribute & CFile::directory) == 0);
    CHECK((status.m_attribute & CFile::readOnly) == 0);
    CHECK(Lower(status.m_szFullName) == Lower(FullPath(path)));

    // Written a moment ago.
    __time64_t now = _time64(NULL);
    CHECK(status.m_mtime.GetTime() >= now - 120);
    CHECK(status.m_mtime.GetTime() <= now + 120);
    CHECK(status.m_ctime.GetTime() != 0);
    CHECK(status.m_atime.GetTime() != 0);

    CHECK_FALSE(CFile::GetStatus(dir.Path("missing.txt").c_str(), status));
    CHECK_FALSE(CFile::GetStatus(dir.Path("no-dir\\missing.txt").c_str(), status));

    // Directories have a status too (the game checks for save folders).
    CFileStatus dirStatus;
    REQUIRE(CFile::GetStatus(dir.Path("sub").c_str(), dirStatus));
    CHECK((dirStatus.m_attribute & CFile::directory) != 0);
    CHECK((CFile::GetStatus(dir.Path("sub\\").c_str(), dirStatus) != FALSE) == true);
    CHECK_FALSE(CFile::GetStatus(dir.Path("*.txt").c_str(), dirStatus));

    // Status of an open file.
    CFile file;
    REQUIRE(file.Open(path.c_str(), CFile::modeRead | CFile::typeBinary, NULL));
    CFileStatus openStatus;
    CHECK(file.GetStatus(openStatus));
    CHECK(openStatus.m_size == 12);
    CHECK(openStatus.m_mtime == status.m_mtime);
    CHECK(Lower(openStatus.m_szFullName) == Lower(FullPath(path)));
    file.Close();

    // SetStatus changes the modification time (CResCache touches files with
    // the current time).
    CFileStatus changed = status;
    changed.m_mtime = CTime(now - 3600);
    CFile::SetStatus(path.c_str(), changed);
    CFileStatus after;
    REQUIRE(CFile::GetStatus(path.c_str(), after));
    CHECK(after.m_mtime.GetTime() == now - 3600);
    CHECK(after.m_size == 12);
    CHECK(dir.ReadFile("status.txt") == "twelve bytes");

    // For other dates the time is written through the local time with
    // today's daylight-saving offset (so it can be an hour off across a
    // daylight-saving change), and read back with the rule of its own date.
    const CTime dates[] = { CTime(2020, 1, 2, 3, 4, 6), CTime(2021, 6, 7, 8, 9, 10), CTime(2022, 10, 1, 12, 0, 0), CTime(2023, 4, 1, 12, 0, 0) };
    for (const CTime& date : dates) {
        changed.m_mtime = date;
        CFile::SetStatus(path.c_str(), changed);

        SYSTEMTIME st;
        date.GetAsSystemTime(st);
        FILETIME local, expected;
        SystemTimeToFileTime(&st, &local);
        LocalFileTimeToFileTime(&local, &expected);

        CHECK(RawWriteTime(path) == FileTimeValue(expected));
        REQUIRE(CFile::GetStatus(path.c_str(), after));
        CHECK(after.m_mtime == CTime(expected));
        CHECK(after.m_mtime.GetTime() == UnixTime(expected));
    }

    // A zero modification time leaves the file's times alone.
    ULONGLONG before = RawWriteTime(path);
    changed.m_mtime = CTime();
    CFile::SetStatus(path.c_str(), changed);
    CHECK(RawWriteTime(path) == before);

    // A read-only attribute is applied and reported, and can be taken off
    // again together with a change of time.
    changed.m_attribute = status.m_attribute | CFile::readOnly;
    changed.m_mtime = CTime(now - 7200);
    CFile::SetStatus(path.c_str(), changed);
    REQUIRE(CFile::GetStatus(path.c_str(), after));
    CHECK((after.m_attribute & CFile::readOnly) != 0);
    CHECK(after.m_mtime.GetTime() == now - 7200);

    // The times of a read-only file cannot be changed while it stays
    // read-only.
    bool refused = false;
    changed.m_mtime = CTime(now - 10000);
    try {
        CFile::SetStatus(path.c_str(), changed);
    } catch (CFileException* e) {
        refused = true;
        CHECK(e->m_cause == CFileException::accessDenied);
        e->Delete();
    }
    CHECK(refused);

    changed.m_attribute = status.m_attribute & ~CFile::readOnly;
    changed.m_mtime = CTime(now - 9000);
    CFile::SetStatus(path.c_str(), changed);
    REQUIRE(CFile::GetStatus(path.c_str(), after));
    CHECK((after.m_attribute & CFile::readOnly) == 0);
    CHECK(after.m_mtime.GetTime() == now - 9000);

    bool caught = false;
    try {
        CFile::SetStatus(dir.Path("missing.txt").c_str(), changed);
    } catch (CException* e) {
        caught = true;
        e->Delete();
    }
    CHECK(caught);
}

TEST_CASE("CFileFind")
{
    TempDir dir;
    dir.WriteFile("a.txt", "1");
    dir.WriteFile("b.txt", "22");
    dir.WriteFile("c.dat", "333");
    CreateDirectoryA(dir.Path("sub").c_str(), NULL);

    SUBCASE("enumerating a folder")
    {
        std::vector<std::string> files;
        std::vector<std::string> dirs;
        int dots = 0;

        CFileFind find;
        BOOL working = find.FindFile((dir.Root() + "\\*.*").c_str());
        CHECK(working);
        while (working) {
            working = find.FindNextFile();
            if (find.IsDots()) {
                CHECK(find.IsDirectory());
                dots++;
                continue;
            }
            std::string name = S(find.GetFileName());
            CHECK(Lower(S(find.GetFilePath())) == Lower(FullPath(dir.Path(name.c_str()))));
            if (find.IsDirectory()) {
                dirs.push_back(Lower(name));
            } else {
                files.push_back(Lower(name));
            }
        }
        std::sort(files.begin(), files.end());
        CHECK(files == std::vector<std::string> { "a.txt", "b.txt", "c.dat" });
        CHECK(dirs == std::vector<std::string> { "sub" });
        CHECK(dots == 2);
    }

    SUBCASE("wildcards")
    {
        std::vector<std::string> files;
        CFileFind find;
        BOOL working = find.FindFile(dir.Path("*.txt").c_str());
        while (working) {
            working = find.FindNextFile();
            files.push_back(Lower(S(find.GetFileName())));
            CHECK_FALSE(find.IsDirectory());
            CHECK_FALSE(find.IsDots());
        }
        std::sort(files.begin(), files.end());
        CHECK(files == std::vector<std::string> { "a.txt", "b.txt" });
    }

    SUBCASE("existence checks")
    {
        // The game uses FindFile alone to ask whether something exists.
        CFileFind find;
        CHECK(find.FindFile(dir.Path("a.txt").c_str()));
        CHECK(find.FindFile(dir.Path("sub").c_str()));
        CHECK(find.FindFile(dir.Root().c_str()));
        CHECK_FALSE(find.FindFile(dir.Path("missing.txt").c_str()));
        CHECK_FALSE(find.FindFile(dir.Path("*.zzz").c_str()));
        CHECK_FALSE(find.FindFile(dir.Path("nodir\\*.*").c_str()));
        CHECK_FALSE(find.FindFile((dir.Root() + "\\").c_str()));
        CHECK(find.FindFile(dir.Path("b.txt").c_str(), 0));
        find.Close();
        find.Close();
        CHECK(find.FindFile(dir.Path("c.dat").c_str()));
    }

    SUBCASE("a single match")
    {
        CFileFind find;
        REQUIRE(find.FindFile(dir.Path("c.dat").c_str()));
        // Before the first FindNextFile there is no current file.
        CHECK_FALSE(find.FindNextFile());
        CHECK(Lower(S(find.GetFileName())) == "c.dat");
        CHECK(Lower(S(find.GetFilePath())) == Lower(FullPath(dir.Path("c.dat"))));
        CHECK(find.GetLength() == 3);
        CHECK_FALSE(find.IsDirectory());
        CHECK_FALSE(find.IsDots());
    }

    SUBCASE("relative patterns give full paths")
    {
        char oldDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, oldDir);
        SetCurrentDirectoryA(dir.Root().c_str());
        {
            CFileFind find;
            REQUIRE(find.FindFile("sub"));
            find.FindNextFile();
            CHECK(find.IsDirectory());
            CHECK(Lower(S(find.GetFilePath())) == Lower(FullPath(dir.Path("sub"))));

            CFileFind all;
            BOOL working = all.FindFile();
            int count = 0;
            while (working) {
                working = all.FindNextFile();
                count++;
            }
            CHECK(count == 6);
        }
        SetCurrentDirectoryA(oldDir);
    }

    SUBCASE("calls without a search are harmless")
    {
        CFileFind find;
        CHECK_FALSE(find.FindNextFile());
        find.Close();
    }
}

TEST_CASE("CTime")
{
    CTime zero;
    CHECK(zero.GetTime() == 0);

    CTime t(2020, 1, 2, 3, 4, 5);
    CHECK(t.GetYear() == 2020);
    CHECK(t.GetMonth() == 1);
    CHECK(t.GetDay() == 2);
    CHECK(t.GetHour() == 3);
    CHECK(t.GetMinute() == 4);
    CHECK(t.GetSecond() == 5);
    CHECK(t.GetDayOfWeek() == 5); // a Thursday; Sunday is 1

    CHECK(S(t.Format("%Y-%m-%d %H:%M:%S")) == "2020-01-02 03:04:05");
    // The format of the save-game time stamp.
    CHECK(S(t.Format("%a, %b %d, %Y - %I:%M %p")) == "Thu, Jan 02, 2020 - 03:04 AM");
    CHECK(S(CTime(2021, 12, 25, 18, 30, 0).Format("%a, %b %d, %Y - %I:%M %p")) == "Sat, Dec 25, 2021 - 06:30 PM");

    CTime later(2020, 1, 2, 3, 4, 6);
    CHECK(later.GetTime() - t.GetTime() == 1);
    CHECK(t < later);
    CHECK(later > t);
    CHECK(t <= later);
    CHECK(t <= t);
    CHECK(later >= t);
    CHECK(t == CTime(2020, 1, 2, 3, 4, 5));
    CHECK(t != later);
    CHECK_FALSE(t == later);
    CHECK_FALSE(later < t);

    CTime copy = t;
    CHECK(copy == t);
    copy = later;
    CHECK(copy == later);

    // Seconds since 1970 in UTC, like the C runtime's clock.
    CTime now = CTime::GetCurrentTime();
    __time64_t crt = _time64(NULL);
    CHECK(now.GetTime() >= crt - 5);
    CHECK(now.GetTime() <= crt + 5);
    CHECK(now.GetHour() >= 0);
    CHECK(now.GetHour() <= 23);

    struct tm local;
    __time64_t value = now.GetTime();
    _localtime64_s(&local, &value);
    CHECK(now.GetHour() == local.tm_hour);
    CHECK(now.GetYear() == local.tm_year + 1900);

    // From a file time (UTC): the same instant, whatever the season of the
    // date or of today. Save-game time stamps are shown from this.
    const __time64_t instants[] = {
        1579089600, // 2020-01-15 12:00 UTC
        1594814400, // 2020-07-15 12:00 UTC
        1711800000, // 2024-03-30 12:00 UTC
        1729944000, // 2024-10-26 12:00 UTC
        172800, // 1970-01-03
        4102444800, // 2100-01-01
    };
    for (__time64_t instant : instants) {
        CTime fromFile(FileTimeFromUnix(instant));
        CHECK(fromFile.GetTime() == instant);
        struct tm fields;
        _localtime64_s(&fields, &instant);
        CHECK(fromFile.GetHour() == fields.tm_hour);
        CHECK(fromFile.GetDay() == fields.tm_mday);
    }

    CTime fromValue(static_cast<__time64_t>(86400 * 365));
    CHECK(fromValue.GetTime() == 86400 * 365);

    // A date in summer goes through the same daylight-saving rules as the
    // C runtime.
    struct tm summer = {};
    summer.tm_year = 2022 - 1900;
    summer.tm_mon = 6;
    summer.tm_mday = 15;
    summer.tm_hour = 12;
    summer.tm_isdst = -1;
    CHECK(CTime(2022, 7, 15, 12, 0, 0).GetTime() == _mktime64(&summer));
    CHECK(CTime(2022, 7, 15, 12, 0, 0).GetHour() == 12);
}

TEST_CASE("file classes: misuse fails the same way as in MFC")
{
    auto throws = [](auto f) {
        try {
            f();
        } catch (CException* e) {
            e->Delete();
            return true;
        }
        return false;
    };
    auto cause = [](auto f) {
        try {
            f();
        } catch (CFileException* e) {
            int c = e->m_cause;
            e->Delete();
            return c;
        }
        return -1;
    };

    TempDir dir;
    dir.WriteFile("a.txt", "1");
    dir.WriteFile("b.txt", "2");
    std::string a = dir.Path("a.txt");
    char buf[8];

    // A file object that is not open.
    CHECK(cause([&] { CFile f; f.Read(buf, 4); }) == CFileException::fileNotFound);
    CHECK(cause([&] { CFile f; f.Write("x", 1); }) == CFileException::fileNotFound);
    CHECK(cause([&] { CFile f; f.Seek(0, CFile::begin); }) == CFileException::fileNotFound);
    CHECK(cause([&] { CFile f; f.GetLength(); }) == CFileException::fileNotFound);
    CHECK(cause([&] { CFile f; f.GetPosition(); }) == CFileException::fileNotFound);
    CHECK_FALSE(throws([&] { CFile f; f.Read(buf, 0); }));
    CHECK_FALSE(throws([&] { CFile f; f.Write("x", 0); }));
    CHECK_FALSE(throws([&] { CFile f; f.Flush(); }));
    CHECK_FALSE(throws([&] { CFile f; f.Close(); }));
    CHECK_FALSE(throws([&] { CFile f; f.Abort(); }));
    CHECK_FALSE(throws([&] { CFile f; f.Open(a.c_str(), CFile::modeRead); f.Close(); f.Close(); }));

    // Error codes and their causes.
    CHECK(cause([&] { CFile::Rename(a.c_str(), dir.Path("b.txt").c_str()); }) == CFileException::accessDenied);
    CHECK(cause([&] { CFile::Remove(dir.Root().c_str()); }) == CFileException::accessDenied);
    CHECK(CFileException::OsErrorToException(0) == CFileException::none);
    CHECK(CFileException::OsErrorToException(ERROR_FILE_NOT_FOUND) == CFileException::fileNotFound);
    CHECK(CFileException::OsErrorToException(ERROR_PATH_NOT_FOUND) == CFileException::badPath);
    CHECK(CFileException::OsErrorToException(ERROR_TOO_MANY_OPEN_FILES) == CFileException::tooManyOpenFiles);
    CHECK(CFileException::OsErrorToException(ERROR_ACCESS_DENIED) == CFileException::accessDenied);
    CHECK(CFileException::OsErrorToException(ERROR_INVALID_HANDLE) == CFileException::fileNotFound);
    CHECK(CFileException::OsErrorToException(ERROR_SHARING_VIOLATION) == CFileException::sharingViolation);
    CHECK(CFileException::OsErrorToException(ERROR_LOCK_VIOLATION) == CFileException::lockViolation);
    CHECK(CFileException::OsErrorToException(ERROR_HANDLE_EOF) == CFileException::endOfFile);
    CHECK(CFileException::OsErrorToException(ERROR_HANDLE_DISK_FULL) == CFileException::diskFull);
    CHECK(CFileException::OsErrorToException(ERROR_DISK_FULL) == CFileException::diskFull);
    CHECK(CFileException::OsErrorToException(ERROR_NEGATIVE_SEEK) == CFileException::badSeek);
    CHECK(CFileException::OsErrorToException(ERROR_ALREADY_EXISTS) == CFileException::accessDenied);
    CHECK(CFileException::OsErrorToException(ERROR_INVALID_NAME) == CFileException::badPath);
    CHECK(CFileException::OsErrorToException(ERROR_DIR_NOT_EMPTY) == CFileException::removeCurrentDir);
    CHECK(CFileException::OsErrorToException(ERROR_INVALID_PARAMETER) == CFileException::genericException);
    CHECK(CFileException::OsErrorToException(99999) == CFileException::genericException);
    CHECK(CFileException::OsErrorToException(-1) == CFileException::genericException);

    // The whole table, as a checksum over every code.
    unsigned sum = 0;
    for (LONG code = 0; code < 2000; code++) {
        sum = sum * 31 + static_cast<unsigned>(CFileException::OsErrorToException(code));
    }
    CHECK(sum == 1364694747u);

    // An empty name is a bad path, with the system's error code.
    {
        CFile f;
        CFileException e;
        CHECK_FALSE(f.Open("", CFile::modeRead, &e));
        CHECK(e.m_cause == CFileException::badPath);
        CHECK(e.m_lOsError == ERROR_INVALID_NAME);
    }

    // A search object without a search has no path.
    CHECK(throws([&] { CFileFind find; find.GetFilePath(); }));
    CHECK(throws([&] { CFileFind find; find.FindFile(dir.Path("missing").c_str()); find.GetFilePath(); }));
    {
        CFileFind find;
        CHECK(S(find.GetFileName()) == "");
        CHECK_FALSE(find.IsDirectory());
        CHECK_FALSE(find.IsDots());
        CHECK(find.GetLength() == 0);
        CHECK(S(find.GetRoot()) == "");
    }

    // Dates: fields out of range and dates before 1970 are rejected.
    CHECK(throws([] { CTime t(2020, 13, 1, 0, 0, 0); }));
    CHECK(throws([] { CTime t(2020, 0, 1, 0, 0, 0); }));
    CHECK(throws([] { CTime t(2020, 1, 0, 0, 0, 0); }));
    CHECK(throws([] { CTime t(2020, 1, 32, 0, 0, 0); }));
    CHECK(throws([] { CTime t(2020, 1, 1, 24, 0, 0); }));
    CHECK(throws([] { CTime t(2020, 1, 1, 0, 60, 0); }));
    CHECK(throws([] { CTime t(2020, 1, 1, 0, 0, 60); }));
    CHECK(throws([] { CTime t(2020, 1, 1, 0, -1, 0); }));
    CHECK(throws([] { CTime t(1899, 1, 1, 0, 0, 0); }));
    CHECK(throws([] { CTime t(1969, 6, 1, 0, 0, 0); }));
    CHECK_FALSE(throws([] { CTime t(3001, 1, 1, 0, 0, 0); }));
    // A day that the month does not have rolls over.
    CTime rolled(2020, 2, 31, 0, 0, 0);
    CHECK(rolled.GetMonth() == 3);
    CHECK(rolled.GetDay() == 2);
    // A time that cannot be shown as a date.
    CHECK(throws([] { CTime t(static_cast<__time64_t>(-1)); t.Format("%Y"); }));
    CHECK(CTime(static_cast<__time64_t>(-1)).GetYear() == 0);
    CHECK(CTime(static_cast<__time64_t>(-1)).GetHour() == -1);
    // A file time from before 1900 counts as "no time".
    FILETIME ancient = { 0, 0 };
    CHECK(CTime(ancient).GetTime() == 0);
    // Text longer than the formatting buffer comes out empty.
    std::string longFormat;
    for (int i = 0; i < 40; i++) {
        longFormat += "%A ";
    }
    CHECK(CTime(2020, 1, 2, 3, 4, 5).Format(longFormat.c_str()).IsEmpty());
    CHECK(CTime(2020, 1, 2, 3, 4, 5).Format(static_cast<LPCSTR>(NULL)).IsEmpty());

    // A file dated before 1970 has no valid status.
    {
        HANDLE h = CreateFileA(a.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        FILETIME old = FileTimeFromUnix(0);
        ULONGLONG value = FileTimeValue(old) - 20ULL * 365 * 86400 * 10000000ULL;
        old.dwLowDateTime = static_cast<DWORD>(value);
        old.dwHighDateTime = static_cast<DWORD>(value >> 32);
        SetFileTime(h, &old, &old, &old);
        CloseHandle(h);
        CFileStatus status;
        CHECK(throws([&] { CFile::GetStatus(a.c_str(), status); }));
    }
}

TEST_CASE("CCriticalSection and CSingleLock")
{
    CCriticalSection section;
    CHECK(section.m_hObject == NULL);

    CHECK(section.Lock());
    CHECK(section.Lock()); // the owner may lock again
    CHECK(section.Unlock());
    CHECK(section.Unlock());

    {
        CSingleLock lock(&section, FALSE);
        CHECK_FALSE(lock.IsLocked());
        CHECK(lock.Lock(INFINITE));
        CHECK(lock.IsLocked());
        CHECK(lock.Unlock());
        CHECK_FALSE(lock.IsLocked());
        // Unlocking when not locked is allowed.
        CHECK(lock.Unlock());

        CHECK(lock.Lock());
        CHECK(lock.IsLocked());
        // Left locked: the destructor releases it.
    }
    {
        CSingleLock initial(&section, TRUE);
        CHECK(initial.IsLocked());
    }
    {
        CSingleLock defaulted(&section);
        CHECK_FALSE(defaulted.IsLocked());
    }

    // The critical section is free again: another thread can take it.
    bool taken = false;
    std::thread other([&]() {
        taken = TryEnterCriticalSection(&section.m_sect) != FALSE;
        if (taken) {
            LeaveCriticalSection(&section.m_sect);
        }
    });
    other.join();
    CHECK(taken);

    // While it is held, another thread cannot take it.
    section.Lock();
    taken = true;
    std::thread blocked([&]() {
        taken = TryEnterCriticalSection(&section.m_sect) != FALSE;
        if (taken) {
            LeaveCriticalSection(&section.m_sect);
        }
    });
    blocked.join();
    CHECK_FALSE(taken);
    section.Unlock();

    // Mutual exclusion under contention.
    int counter = 0;
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; t++) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 20000; i++) {
                CSingleLock lock(&section, FALSE);
                lock.Lock(INFINITE);
                int value = counter;
                counter = value + 1;
                lock.Unlock();
            }
        });
    }
    for (std::thread& thread : threads) {
        thread.join();
    }
    CHECK(counter == 80000);
}

TEST_CASE("AfxIsValidAddress only rejects a null pointer")
{
    int local = 0;
    CHECK(AfxIsValidAddress(&local, sizeof(local), TRUE));
    CHECK(AfxIsValidAddress(&local, sizeof(local)));
    CHECK(AfxIsValidAddress("literal", 8, FALSE));
    CHECK_FALSE(AfxIsValidAddress(NULL, 4, TRUE));
    CHECK(AfxIsValidAddress(reinterpret_cast<void*>(1), 4, TRUE));
}

TEST_CASE("CWnd holds a window handle")
{
    // A message-only window: never shown and not part of the desktop.
    HWND hWnd = CreateWindowExA(0, "STATIC", "", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandleA(NULL), NULL);
    REQUIRE(hWnd != NULL);

    {
        CWnd wnd;
        CHECK(wnd.m_hWnd == NULL);
        CHECK(wnd.GetSafeHwnd() == NULL);

        CHECK(wnd.Attach(hWnd));
        CHECK(wnd.m_hWnd == hWnd);
        CHECK(wnd.GetSafeHwnd() == hWnd);
        HWND converted = wnd;
        CHECK(converted == hWnd);

        CHECK(wnd.Detach() == hWnd);
        CHECK(wnd.m_hWnd == NULL);
        CHECK(wnd.GetSafeHwnd() == NULL);
        CHECK(IsWindow(hWnd));
        CHECK(wnd.Detach() == NULL);
    }
    // Detached: the window outlives the object.
    CHECK(IsWindow(hWnd));

    {
        CWnd wnd;
        wnd.Attach(hWnd);
    }
    // Still attached when the object goes away: the window is destroyed.
    CHECK_FALSE(IsWindow(hWnd));
}

TEST_CASE("C runtime multi-byte code page")
{
    // The string classes follow the C runtime's multi-byte code page, which
    // must not depend on whether MFC is linked.
    MESSAGE("_getmbcp() = " << _getmbcp() << ", GetACP() = " << GetACP());
    CHECK(_getmbcp() == 0);
}
