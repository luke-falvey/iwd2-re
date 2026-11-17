// GetPrivateProfileIntA / GetPrivateProfileStringA / WritePrivateProfileStringA
// against the rules of the Windows functions. Each rule is one test case; the
// same file describes what any implementation must do.

#include "test.h"

#include <string>
#include <vector>

namespace {

class IniFile {
public:
    explicit IniFile(const char* content)
    {
        char base[MAX_PATH];
        GetTempPathA(MAX_PATH, base);
        static int counter = 0;
        char name[MAX_PATH];
        sprintf(name, "%siwd2re-ini-%lu-%d.ini", base, static_cast<unsigned long>(GetCurrentProcessId()), counter++);
        m_path = name;
        if (content != NULL) {
            Put(content);
        }
    }

    ~IniFile() { DeleteFileA(m_path.c_str()); }

    const char* Path() const { return m_path.c_str(); }

    void Put(const std::string& content)
    {
        HANDLE h = CreateFileA(m_path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        REQUIRE(h != INVALID_HANDLE_VALUE);
        DWORD written;
        WriteFile(h, content.data(), static_cast<DWORD>(content.size()), &written, NULL);
        CloseHandle(h);
    }

    std::string Get() const
    {
        std::string content;
        HANDLE h = CreateFileA(m_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) {
            return "<missing>";
        }
        char buffer[512];
        DWORD n;
        while (ReadFile(h, buffer, sizeof(buffer), &n, NULL) && n > 0) {
            content.append(buffer, n);
        }
        CloseHandle(h);
        return content;
    }

private:
    std::string m_path;
};

std::string Str(const IniFile& f, const char* section, const char* key, const char* def = "")
{
    char buffer[256];
    DWORD n = GetPrivateProfileStringA(section, key, def, buffer, sizeof(buffer), f.Path());
    CHECK(n == strlen(buffer));
    return buffer;
}

// Splits a double-NUL-terminated list.
std::vector<std::string> List(const char* buffer)
{
    std::vector<std::string> items;
    while (*buffer != 0) {
        items.push_back(buffer);
        buffer += strlen(buffer) + 1;
    }
    return items;
}

} // namespace

TEST_CASE("ini: section and key names compare without regard to case")
{
    IniFile f("[Section One]\r\nSome Key=value\r\n");
    CHECK(Str(f, "SECTION ONE", "some key") == "value");
    CHECK(Str(f, "section one", "SOME KEY") == "value");
    CHECK(Str(f, "other", "some key", "dflt") == "dflt");
}

TEST_CASE("ini: whitespace around keys and values is trimmed")
{
    IniFile f("[s]\r\n   key   =   spaced value \t \r\n");
    CHECK(Str(f, "s", "key") == "spaced value");
}

TEST_CASE("ini: a value in matching quotes loses them")
{
    IniFile f("[s]\r\na=\"double\"\r\nb='single'\r\nc=\"mixed'\r\nd=\"\"\r\n");
    CHECK(Str(f, "s", "a") == "double");
    CHECK(Str(f, "s", "b") == "single");
    CHECK(Str(f, "s", "c") == "\"mixed'");
    CHECK(Str(f, "s", "d", "x") == "");
}

TEST_CASE("ini: lines starting with a semicolon are comments")
{
    IniFile f("[s]\r\n;a=1\r\n   ; b=2\r\nc=3\r\n");
    CHECK(Str(f, "s", "a", "none") == "none");
    CHECK(Str(f, "s", "b", "none") == "none");
    CHECK(Str(f, "s", "c") == "3");
}

TEST_CASE("ini: the first occurrence of a duplicate key wins")
{
    IniFile f("[s]\r\nk=first\r\nk=second\r\n[t]\r\nk=other\r\n");
    CHECK(Str(f, "s", "k") == "first");
    CHECK(Str(f, "t", "k") == "other");
}

TEST_CASE("ini: GetPrivateProfileInt parses a leading number and ignores the rest")
{
    IniFile f("[n]\r\na=42\r\nb=  17abc\r\nc=-5\r\nd=abc\r\ne=+8\r\nf=3.9\r\ng=0x10\r\n");
    CHECK(GetPrivateProfileIntA("n", "a", 7, f.Path()) == 42);
    CHECK(GetPrivateProfileIntA("n", "b", 7, f.Path()) == 17);
    CHECK(GetPrivateProfileIntA("n", "c", 7, f.Path()) == static_cast<UINT>(-5));
    CHECK(GetPrivateProfileIntA("n", "d", 7, f.Path()) == 0);
    CHECK(GetPrivateProfileIntA("n", "e", 7, f.Path()) == 8);
    CHECK(GetPrivateProfileIntA("n", "f", 7, f.Path()) == 3);
    CHECK(GetPrivateProfileIntA("n", "g", 7, f.Path()) == 0);
}

TEST_CASE("ini: GetPrivateProfileInt gives the default for a missing key or file")
{
    IniFile f("[n]\r\na=1\r\n");
    CHECK(GetPrivateProfileIntA("n", "missing", 99, f.Path()) == 99);
    CHECK(GetPrivateProfileIntA("missing", "a", -3, f.Path()) == static_cast<UINT>(-3));
    IniFile none(NULL);
    CHECK(GetPrivateProfileIntA("n", "a", 5, none.Path()) == 5);
}

TEST_CASE("ini: GetPrivateProfileInt wraps as an unsigned 32-bit value")
{
    IniFile f("[n]\r\na=4294967295\r\nb=4294967296\r\nc=2147483648\r\nd=-2147483648\r\n");
    CHECK(GetPrivateProfileIntA("n", "a", 0, f.Path()) == 0xFFFFFFFFu);
    CHECK(GetPrivateProfileIntA("n", "b", 0, f.Path()) == 0u);
    CHECK(GetPrivateProfileIntA("n", "c", 0, f.Path()) == 0x80000000u);
    CHECK(GetPrivateProfileIntA("n", "d", 0, f.Path()) == 0x80000000u);
}

TEST_CASE("ini: GetPrivateProfileString copies at most nSize-1 characters")
{
    IniFile f("[s]\r\nk=0123456789\r\n");
    char buffer[8];
    memset(buffer, 'x', sizeof(buffer));
    DWORD n = GetPrivateProfileStringA("s", "k", "", buffer, 6, f.Path());
    CHECK(n == 5);
    CHECK(std::string(buffer) == "01234");

    // The default is cut the same way.
    n = GetPrivateProfileStringA("s", "nokey", "abcdefghij", buffer, 4, f.Path());
    CHECK(n == 3);
    CHECK(std::string(buffer) == "abc");

    // A NULL default is an empty string.
    n = GetPrivateProfileStringA("s", "nokey", NULL, buffer, sizeof(buffer), f.Path());
    CHECK(n == 0);
    CHECK(buffer[0] == 0);
}

TEST_CASE("ini: a NULL key name lists the keys of a section")
{
    IniFile f("[a]\r\nk1=1\r\n;c=3\r\nk2=2\r\n[b]\r\nother=1\r\n");
    char buffer[64];
    DWORD n = GetPrivateProfileStringA("a", NULL, "", buffer, sizeof(buffer), f.Path());
    std::vector<std::string> keys = List(buffer);
    REQUIRE(keys.size() == 2);
    CHECK(keys[0] == "k1");
    CHECK(keys[1] == "k2");
    CHECK(n == 6); // "k1\0k2\0" without the final NUL
    CHECK(buffer[6] == 0);

    // Too small: cut, and still double-NUL-terminated.
    char small[5];
    n = GetPrivateProfileStringA("a", NULL, "", small, sizeof(small), f.Path());
    CHECK(n == 3);
    CHECK(small[3] == 0);
    CHECK(small[4] == 0);
}

TEST_CASE("ini: a NULL section name lists the sections")
{
    IniFile f("[first]\r\na=1\r\n[Second]\r\nb=2\r\n");
    char buffer[64];
    DWORD n = GetPrivateProfileStringA(NULL, NULL, "", buffer, sizeof(buffer), f.Path());
    std::vector<std::string> sections = List(buffer);
    REQUIRE(sections.size() == 2);
    CHECK(sections[0] == "first");
    CHECK(sections[1] == "Second");
    CHECK(n == 6 + 7);
}

TEST_CASE("ini: writing replaces the first matching key in the section")
{
    IniFile f("[a]\r\nk=old\r\nk=dup\r\n[b]\r\nk=other\r\n");
    CHECK(WritePrivateProfileStringA("A", "K", "new", f.Path()));
    CHECK(Str(f, "a", "k") == "new");
    CHECK(Str(f, "b", "k") == "other");
    CHECK(f.Get() == "[a]\r\nK=new\r\nk=dup\r\n[b]\r\nk=other\r\n");
}

TEST_CASE("ini: writing a new key appends it at the end of its section")
{
    IniFile f("[a]\r\nx=1\r\n[b]\r\ny=2\r\n");
    CHECK(WritePrivateProfileStringA("a", "z", "3", f.Path()));
    CHECK(f.Get() == "[a]\r\nx=1\r\nz=3\r\n[b]\r\ny=2\r\n");
    CHECK(Str(f, "a", "z") == "3");
}

TEST_CASE("ini: writing a new section appends it at the end of the file")
{
    IniFile f("[a]\r\nx=1\r\n");
    CHECK(WritePrivateProfileStringA("new", "k", "v", f.Path()));
    CHECK(f.Get() == "[a]\r\nx=1\r\n[new]\r\nk=v\r\n");
}

TEST_CASE("ini: writing creates the file when it is missing")
{
    IniFile f(NULL);
    CHECK(f.Get() == "<missing>");
    CHECK(WritePrivateProfileStringA("s", "k", "v", f.Path()));
    CHECK(f.Get() == "[s]\r\nk=v\r\n");
}

TEST_CASE("ini: a NULL string deletes the key, a NULL key deletes the section")
{
    IniFile f("[a]\r\nx=1\r\ny=2\r\n[b]\r\nz=3\r\n");
    CHECK(WritePrivateProfileStringA("a", "x", NULL, f.Path()));
    CHECK(f.Get() == "[a]\r\ny=2\r\n[b]\r\nz=3\r\n");
    CHECK(WritePrivateProfileStringA("a", NULL, NULL, f.Path()));
    CHECK(f.Get() == "[b]\r\nz=3\r\n");
}

TEST_CASE("ini: writing keeps every other line as it was")
{
    IniFile f("; header comment\n\n[a]\n  odd   =  spaced  \n; note\nplain line\n[b]\nq=1\n");
    CHECK(WritePrivateProfileStringA("b", "q", "2", f.Path()));
    CHECK(f.Get() == "; header comment\n\n[a]\n  odd   =  spaced  \n; note\nplain line\n[b]\nq=2\r\n");
}

TEST_CASE("ini: a file changed behind the cache is read again")
{
    IniFile f("[s]\r\nk=one\r\n");
    CHECK(Str(f, "s", "k") == "one");
    f.Put("[s]\r\nk=second value\r\n");
    CHECK(Str(f, "s", "k") == "second value");
    CHECK(WritePrivateProfileStringA("s", "k", "third", f.Path()));
    CHECK(Str(f, "s", "k") == "third");
}
