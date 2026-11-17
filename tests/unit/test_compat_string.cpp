// CString behaviour the game relies on. Runs against MFC in the default
// build and against src/compat in the IWD2RE_NO_MFC build; both must pass,
// which shows that the replacement behaves like MFC.

#include "test.h"

#include <mbstring.h>

#include <string>
#include <thread>
#include <vector>

namespace {

// The text of a CString including any embedded terminators.
std::string S(const CString& s)
{
    return std::string(static_cast<LPCSTR>(s), s.GetLength());
}

const CString g_staticString("static");

// True when the call throws a `CException*`, which is how MFC reports
// arguments it rejects.
template <class F>
bool Throws(F f)
{
    try {
        f();
    } catch (CException* e) {
        e->Delete();
        return true;
    }
    return false;
}

volatile int g_sink;

} // namespace

TEST_CASE("CString is one pointer and empty strings share a buffer")
{
    CHECK(sizeof(CString) == sizeof(void*));

    CString a;
    CString b;
    CHECK(a.GetLength() == 0);
    CHECK(a.IsEmpty());
    CHECK(static_cast<LPCSTR>(a) != NULL);
    CHECK(static_cast<LPCSTR>(a)[0] == '\0');
    CHECK(static_cast<LPCSTR>(a) == static_cast<LPCSTR>(b));

    CString c("text");
    c.Empty();
    CHECK(c.IsEmpty());
    CHECK(static_cast<LPCSTR>(c) == static_cast<LPCSTR>(a));

    CString d("");
    CHECK(static_cast<LPCSTR>(d) == static_cast<LPCSTR>(a));

    // The object holds nothing but the pointer to its characters, which is
    // what lets game code pass a CString to printf-style functions.
    CString e("abc");
    CHECK(*reinterpret_cast<LPCSTR*>(&e) == static_cast<LPCSTR>(e));
}

TEST_CASE("CString constructors")
{
    CHECK(S(CString("hello")) == "hello");
    CHECK(S(CString(static_cast<LPCSTR>(NULL))) == "");
    CHECK(S(CString("hello", 3)) == "hel");
    CHECK(S(CString("hello", 0)) == "");
    CHECK(S(CString('x')) == "x");
    CHECK(S(CString('x', 3)) == "xxx");
    CHECK(S(CString('x', 0)) == "");
    CHECK(S(CString(reinterpret_cast<const unsigned char*>("bytes"))) == "bytes");
    CHECK(S(g_staticString) == "static");

    // Explicit length keeps embedded terminators.
    CString embedded("ab\0cd", 5);
    CHECK(embedded.GetLength() == 5);
    CHECK(S(embedded) == std::string("ab\0cd", 5));
    CHECK(strlen(embedded) == 2);
}

TEST_CASE("CString copies share the buffer until one is written")
{
    CString a("shared");
    CString b(a);
    CHECK(static_cast<LPCSTR>(a) == static_cast<LPCSTR>(b));

    CString c;
    c = a;
    CHECK(static_cast<LPCSTR>(a) == static_cast<LPCSTR>(c));

    b.SetAt(0, 'S');
    CHECK(static_cast<LPCSTR>(a) != static_cast<LPCSTR>(b));
    CHECK(S(a) == "shared");
    CHECK(S(b) == "Shared");
    CHECK(S(c) == "shared");

    c += "!";
    CHECK(S(a) == "shared");
    CHECK(S(c) == "shared!");

    CString d(a);
    d.MakeUpper();
    CHECK(S(a) == "shared");
    CHECK(S(d) == "SHARED");

    CString e(a);
    char* p = e.GetBuffer(0);
    p[0] = 'X';
    e.ReleaseBuffer();
    CHECK(S(a) == "shared");
    CHECK(S(e) == "Xhared");
}

TEST_CASE("CString assignment")
{
    CString s;
    s = "abc";
    CHECK(S(s) == "abc");
    s = 'z';
    CHECK(S(s) == "z");
    s = static_cast<LPCSTR>(NULL);
    CHECK(S(s) == "");
    CHECK(s.IsEmpty());

    s = "abcdef";
    s = s;
    CHECK(S(s) == "abcdef");

    // Assigning part of itself.
    s = static_cast<LPCSTR>(s) + 2;
    CHECK(S(s) == "cdef");

    CString t = s;
    t = static_cast<LPCSTR>(t) + 1;
    CHECK(S(t) == "def");
    CHECK(S(s) == "cdef");

    s = reinterpret_cast<const unsigned char*>("uchar");
    CHECK(S(s) == "uchar");
}

TEST_CASE("CString concatenation")
{
    CString a("foo");
    CString b("bar");
    CHECK(S(a + b) == "foobar");
    CHECK(S(a + "bar") == "foobar");
    CHECK(S("foo" + b) == "foobar");
    CHECK(S(a + '!') == "foo!");
    CHECK(S('!' + a) == "!foo");
    CHECK(S(a + CString()) == "foo");
    CHECK(S(CString() + CString()) == "");
    CHECK(S(a) == "foo");

    CString s;
    s += "ab";
    s += 'c';
    s += CString("de");
    s += "";
    s += CString();
    CHECK(S(s) == "abcde");
    CHECK(s.GetLength() == 5);

    // Appending itself, whole and in part.
    s += s;
    CHECK(S(s) == "abcdeabcde");
    s += static_cast<LPCSTR>(s) + 8;
    CHECK(S(s) == "abcdeabcdede");

    // Many appends keep the text intact while the buffer grows.
    CString big;
    std::string expected;
    for (int i = 0; i < 2000; i++) {
        char ch = static_cast<char>('a' + i % 26);
        big += ch;
        expected += ch;
    }
    CHECK(S(big) == expected);

    // Embedded terminators survive concatenation of CStrings.
    CString e1("a\0b", 3);
    CString e2 = e1 + CString("c\0d", 3);
    CHECK(S(e2) == std::string("a\0bc\0d", 6));
}

TEST_CASE("CString Mid, Left, Right clamp to the string")
{
    CString s("0123456789");

    CHECK(S(s.Mid(3)) == "3456789");
    CHECK(S(s.Mid(0)) == "0123456789");
    CHECK(S(s.Mid(10)) == "");
    CHECK(S(s.Mid(11)) == "");
    CHECK(S(s.Mid(100)) == "");
    CHECK(S(s.Mid(-3)) == "0123456789");

    CHECK(S(s.Mid(2, 3)) == "234");
    CHECK(S(s.Mid(2, 0)) == "");
    CHECK(S(s.Mid(2, -1)) == "");
    CHECK(S(s.Mid(2, 100)) == "23456789");
    CHECK(S(s.Mid(8, 5)) == "89");
    CHECK(S(s.Mid(10, 1)) == "");
    CHECK(S(s.Mid(20, 1)) == "");
    CHECK(S(s.Mid(-2, 3)) == "012");
    CHECK(S(s.Mid(0, 10)) == "0123456789");
    CHECK(S(s.Mid(0, 11)) == "0123456789");

    CHECK(S(s.Left(0)) == "");
    CHECK(S(s.Left(-1)) == "");
    CHECK(S(s.Left(4)) == "0123");
    CHECK(S(s.Left(10)) == "0123456789");
    CHECK(S(s.Left(50)) == "0123456789");

    CHECK(S(s.Right(0)) == "");
    CHECK(S(s.Right(-1)) == "");
    CHECK(S(s.Right(4)) == "6789");
    CHECK(S(s.Right(10)) == "0123456789");
    CHECK(S(s.Right(50)) == "0123456789");

    CString empty;
    CHECK(S(empty.Mid(0)) == "");
    CHECK(S(empty.Mid(1, 2)) == "");
    CHECK(S(empty.Left(3)) == "");
    CHECK(S(empty.Right(3)) == "");

    // The source is not changed.
    CHECK(S(s) == "0123456789");

    // Counts are bytes: embedded terminators are carried along.
    CString e("ab\0cd", 5);
    CHECK(S(e.Mid(1, 3)) == std::string("b\0c", 3));
    CHECK(S(e.Right(2)) == "cd");
}

TEST_CASE("CString Find")
{
    CString s("abcabcabc");

    CHECK(s.Find('a') == 0);
    CHECK(s.Find('c') == 2);
    CHECK(s.Find('z') == -1);
    CHECK(s.Find('a', 1) == 3);
    CHECK(s.Find('a', 3) == 3);
    CHECK(s.Find('a', 7) == -1);
    CHECK(s.Find('c', 8) == 8);
    CHECK(s.Find('c', 9) == -1);
    CHECK(s.Find('a', 100) == -1);
    CHECK(s.Find('a', -1) == -1);

    CHECK(s.Find("abc") == 0);
    CHECK(s.Find("bca") == 1);
    CHECK(s.Find("abc", 1) == 3);
    CHECK(s.Find("abc", 6) == 6);
    CHECK(s.Find("abc", 7) == -1);
    CHECK(s.Find("abcd") == -1);
    CHECK(s.Find("") == 0);
    CHECK(s.Find("", 4) == 4);
    CHECK(s.Find("", 9) == 9);
    CHECK(s.Find("", 10) == -1);
    CHECK(s.Find("a", 9) == -1);
    CHECK(s.Find("a", -1) == -1);
    CHECK(s.Find(CString("cab")) == 2);

    CString empty;
    CHECK(empty.Find('a') == -1);
    CHECK(empty.Find("a") == -1);
    CHECK(empty.Find("") == 0);

    CHECK(s.ReverseFind('a') == 6);
    CHECK(s.ReverseFind('c') == 8);
    CHECK(s.ReverseFind('z') == -1);
    CHECK(empty.ReverseFind('a') == -1);

    CHECK(s.FindOneOf("xcb") == 1);
    CHECK(s.FindOneOf("xyz") == -1);
    CHECK(s.FindOneOf("") == -1);
    CHECK(empty.FindOneOf("abc") == -1);

    // The search for a character stops at an embedded terminator.
    CString e("ab\0cd", 5);
    CHECK(e.Find('c') == -1);
    CHECK(e.Find('c', 3) == 3);
    CHECK(e.Find("cd") == -1);
    CHECK(e.Find("cd", 3) == 3);
}

TEST_CASE("CString comparison")
{
    CString a("apple");
    CString b("banana");
    CString a2("apple");
    CString upper("APPLE");

    CHECK(a.Compare("apple") == 0);
    CHECK(a.Compare("banana") < 0);
    CHECK(b.Compare("apple") > 0);
    CHECK(a.Compare("APPLE") > 0);
    CHECK(a.Compare("app") > 0);
    CHECK(a.Compare("apples") < 0);
    CHECK(a.Compare("") > 0);
    CHECK(CString().Compare("") == 0);

    CHECK(a.CompareNoCase("APPLE") == 0);
    CHECK(a.CompareNoCase("ApPlE") == 0);
    CHECK(a.CompareNoCase("BANANA") < 0);
    CHECK(b.CompareNoCase("APPLE") > 0);
    CHECK(upper.CompareNoCase(a) == 0);

    CHECK(a == a2);
    CHECK(a == "apple");
    CHECK("apple" == a);
    CHECK_FALSE(a == b);
    CHECK_FALSE(a == "APPLE");
    CHECK_FALSE(a == "");
    CHECK(CString() == "");
    CHECK(a != b);
    CHECK(a != "pear");
    CHECK("pear" != a);
    CHECK_FALSE(a != a2);

    CHECK(a < b);
    CHECK(a < "b");
    CHECK("a" < a);
    CHECK(b > a);
    CHECK(a <= a2);
    CHECK(a >= a2);
    CHECK(a <= b);
    CHECK(b >= a);
    CHECK_FALSE(b < a);

    // Comparison is by text, so it stops at an embedded terminator.
    CString e1("ab\0cd", 5);
    CString e2("ab\0xy", 5);
    CHECK(e1 == e2);
    CHECK(e1 == "ab");
}

TEST_CASE("CString case conversion")
{
    CString s("Hello, World 123 []{}");
    s.MakeUpper();
    CHECK(S(s) == "HELLO, WORLD 123 []{}");
    s.MakeLower();
    CHECK(S(s) == "hello, world 123 []{}");

    CString empty;
    empty.MakeUpper();
    empty.MakeLower();
    CHECK(S(empty) == "");

    // Characters above 0x7F follow the C runtime's multi-byte rules.
    unsigned char raw[] = { 'a', 0xE9, 0xE0, 'z', 0xC9, 0xFF, 0xDF, 0 };
    unsigned char up[sizeof(raw)];
    unsigned char low[sizeof(raw)];
    memcpy(up, raw, sizeof(raw));
    memcpy(low, raw, sizeof(raw));
    _mbsupr_s(up, sizeof(up));
    _mbslwr_s(low, sizeof(low));

    CString high(reinterpret_cast<const char*>(raw));
    CString highUp(high);
    highUp.MakeUpper();
    CString highLow(high);
    highLow.MakeLower();
    CHECK(S(highUp) == std::string(reinterpret_cast<const char*>(up)));
    CHECK(S(highLow) == std::string(reinterpret_cast<const char*>(low)));
    CHECK(high.CompareNoCase(highUp) == _mbsicmp(raw, up));

    // MakeUpper returns the string itself (game code chains on it).
    CString chained("abc");
    CHECK(S(chained.MakeUpper()) == "ABC");
    CHECK(S(chained) == "ABC");
}

TEST_CASE("CString TrimLeft and TrimRight")
{
    CString s("  \t\r\n hello world \t\r\n  ");
    s.TrimLeft();
    CHECK(S(s) == "hello world \t\r\n  ");
    s.TrimRight();
    CHECK(S(s) == "hello world");
    s.TrimLeft();
    s.TrimRight();
    CHECK(S(s) == "hello world");

    CString ws(" \t\v\f\r\n ");
    CString ws2(ws);
    ws.TrimLeft();
    CHECK(S(ws) == "");
    ws2.TrimRight();
    CHECK(S(ws2) == "");

    CString empty;
    empty.TrimLeft();
    empty.TrimRight();
    CHECK(S(empty) == "");

    // Only white space is removed; other control and high characters stay.
    CString ctl("\x01 a \x01");
    ctl.TrimLeft();
    ctl.TrimRight();
    CHECK(S(ctl) == "\x01 a \x01");

    CString high("\xA0 x \xA0");
    high.TrimLeft();
    high.TrimRight();
    CHECK(S(high) == "\xA0 x \xA0");

    CString c("xxhixx");
    c.TrimLeft('x');
    CHECK(S(c) == "hixx");
    c.TrimRight('x');
    CHECK(S(c) == "hi");
    c.TrimRight('z');
    c.TrimLeft('z');
    CHECK(S(c) == "hi");

    CString set("-=-hi-there=-=");
    set.TrimLeft("=-");
    CHECK(S(set) == "hi-there=-=");
    set.TrimRight("=-");
    CHECK(S(set) == "hi-there");
    set.TrimRight("");
    set.TrimLeft("");
    CHECK(S(set) == "hi-there");

    // Trimming works on the text up to the first terminator.
    CString e("  ab  \0  ", 9);
    e.TrimRight();
    CHECK(S(e) == "  ab");
    CString e2("  ab\0cd", 7);
    e2.TrimLeft();
    CHECK(S(e2) == std::string("ab\0cd", 5));

    // A copy is not affected.
    CString orig("  pad  ");
    CString copy(orig);
    copy.TrimLeft();
    copy.TrimRight();
    CHECK(S(orig) == "  pad  ");
    CHECK(S(copy) == "pad");
}

TEST_CASE("CString SpanIncluding and SpanExcluding")
{
    CString s("123abc456");
    CHECK(S(s.SpanIncluding("0123456789")) == "123");
    CHECK(S(s.SpanIncluding("abc")) == "");
    CHECK(S(s.SpanIncluding("")) == "");
    CHECK(S(s.SpanIncluding("123abc456")) == "123abc456");
    CHECK(S(s.SpanExcluding("abc")) == "123");
    CHECK(S(s.SpanExcluding("1")) == "");
    CHECK(S(s.SpanExcluding("xyz")) == "123abc456");
    CHECK(S(s.SpanExcluding("")) == "123abc456");
    CHECK(S(CString().SpanExcluding("a")) == "");
    CHECK(S(CString().SpanIncluding("a")) == "");
}

TEST_CASE("CString Remove, Replace, Delete, Insert")
{
    CString s("a-b--c---");
    CHECK(s.Remove('-') == 6);
    CHECK(S(s) == "abc");
    CHECK(s.Remove('-') == 0);
    CHECK(S(s) == "abc");
    CHECK(CString().Remove('a') == 0);

    CString all("xxxx");
    CHECK(all.Remove('x') == 4);
    CHECK(S(all) == "");
    CHECK(all.GetLength() == 0);

    CString r("banana");
    CHECK(r.Replace('a', 'o') == 3);
    CHECK(S(r) == "bonono");
    CHECK(r.Replace('z', 'y') == 0);
    CHECK(r.Replace('o', 'o') == 0);
    CHECK(S(r) == "bonono");

    CString rs("one two one two one");
    CHECK(rs.Replace("one", "1") == 3);
    CHECK(S(rs) == "1 two 1 two 1");
    CHECK(rs.Replace("two", "three") == 2);
    CHECK(S(rs) == "1 three 1 three 1");
    CHECK(rs.Replace("1", "") == 3);
    CHECK(S(rs) == " three  three ");
    CHECK(rs.Replace("zzz", "y") == 0);
    CHECK(rs.Replace("", "y") == 0);
    CHECK(S(rs) == " three  three ");

    CString overlap("aaaa");
    CHECK(overlap.Replace("aa", "a") == 2);
    CHECK(S(overlap) == "aa");
    CString grow("abab");
    CHECK(grow.Replace("b", "bbb") == 2);
    CHECK(S(grow) == "abbbabbb");
    CString same("abc");
    CHECK(same.Replace("b", "b") == 1);
    CHECK(S(same) == "abc");

    CString d("0123456789");
    CHECK(d.Delete(0) == 9);
    CHECK(S(d) == "123456789");
    CHECK(d.Delete(2, 3) == 6);
    CHECK(S(d) == "126789");
    CHECK(d.Delete(4, 100) == 4);
    CHECK(S(d) == "1267");
    CHECK(d.Delete(10, 1) == 4);
    CHECK(S(d) == "1267");
    CHECK(d.Delete(4, 1) == 4);
    CHECK(S(d) == "1267");
    CHECK(d.Delete(1, 0) == 4);
    CHECK(d.Delete(1, -1) == 4);
    CHECK(S(d) == "1267");
    CHECK(d.Delete(-1, 1) == 3);
    CHECK(S(d) == "267");
    CHECK(d.Delete(0, 3) == 0);
    CHECK(S(d) == "");

    CString i("ac");
    CHECK(i.Insert(1, 'b') == 3);
    CHECK(S(i) == "abc");
    CHECK(i.Insert(0, "<<") == 5);
    CHECK(S(i) == "<<abc");
    CHECK(i.Insert(100, ">>") == 7);
    CHECK(S(i) == "<<abc>>");
    CHECK(i.Insert(-5, '!') == 8);
    CHECK(S(i) == "!<<abc>>");
}

TEST_CASE("CString character access")
{
    CString s("abc");
    CHECK(s.GetAt(0) == 'a');
    CHECK(s.GetAt(2) == 'c');
    CHECK(s[1] == 'b');
    CHECK(s[3] == '\0');

    s.SetAt(1, 'X');
    CHECK(S(s) == "aXc");

    // A terminator written into the text does not change the length.
    s.SetAt(1, '\0');
    CHECK(s.GetLength() == 3);
    CHECK(S(s) == std::string("a\0c", 3));
    CHECK(strlen(s) == 1);

    unsigned char u = 200;
    CString h(static_cast<char>(u));
    CHECK(static_cast<unsigned char>(h[0]) == 200);
    CHECK(h.GetAt(0) < 0);
}

TEST_CASE("CString GetBuffer and ReleaseBuffer")
{
    CString s;
    char* p = s.GetBuffer(32);
    CHECK(s.GetAllocLength() >= 32);
    strcpy(p, "written");
    s.ReleaseBuffer();
    CHECK(S(s) == "written");
    CHECK(s.GetLength() == 7);

    p = s.GetBuffer(64);
    CHECK(strcmp(p, "written") == 0);
    strcat(p, " more");
    s.ReleaseBuffer(-1);
    CHECK(S(s) == "written more");

    p = s.GetBuffer(0);
    s.ReleaseBuffer(4);
    CHECK(S(s) == "writ");
    CHECK(static_cast<LPCSTR>(s)[4] == '\0');

    s.ReleaseBuffer(0);
    CHECK(S(s) == "");

    CString t("abc");
    p = t.GetBufferSetLength(6);
    CHECK(t.GetLength() == 6);
    CHECK(memcmp(p, "abc", 3) == 0);
    memcpy(p, "ABCDEF", 6);
    CHECK(S(t) == "ABCDEF");
    t.GetBufferSetLength(2);
    CHECK(S(t) == "AB");

    // GetBuffer on a shared string gives a private buffer.
    CString a("orig");
    CString b(a);
    p = b.GetBuffer(10);
    strcpy(p, "changed");
    b.ReleaseBuffer();
    CHECK(S(a) == "orig");
    CHECK(S(b) == "changed");

    // ReleaseBuffer on a string that was never written is harmless.
    CString never;
    never.ReleaseBuffer();
    CHECK(S(never) == "");
    CString lit("lit");
    lit.ReleaseBuffer();
    CHECK(S(lit) == "lit");
}

#pragma warning(push)
#pragma warning(disable : 4840) // CString passed through "..." on purpose

TEST_CASE("CString Format")
{
    CString s;
    s.Format("%d", 42);
    CHECK(S(s) == "42");
    s.Format("%s=%d (%c) %05.2f %x %u %%", "key", -7, 'q', 3.14159, 255, 3000000000u);
    CHECK(S(s) == "key=-7 (q) 03.14 ff 3000000000 %");
    s.Format("%3d|%-3d|%03d", 5, 5, 5);
    CHECK(S(s) == "  5|5  |005");
    s.Format("");
    CHECK(S(s) == "");
    s.Format("no arguments");
    CHECK(S(s) == "no arguments");

    // A CString passed as a "%s" argument works because the object is just
    // the character pointer.
    CString name("Imoen");
    CString t;
#ifdef _WIN32
    t.Format("%s has %d hp", name, 12);
#else
    // NOTE: Port: Clang refuses to pass a class with a destructor through `...`.
    t.Format("%s has %d hp", static_cast<LPCSTR>(name), 12);
#endif
    CHECK(S(t) == "Imoen has 12 hp");
    t.Format("%s/%s", static_cast<LPCSTR>(name), static_cast<LPCSTR>(name));
    CHECK(S(t) == "Imoen/Imoen");

    // Output longer than any fixed buffer.
    std::string longText(10000, 'x');
    s.Format("[%s]%d", longText.c_str(), 7);
    CHECK(s.GetLength() == 10003);
    CHECK(S(s) == "[" + longText + "]7");

    // Formatting replaces the old content and leaves copies alone.
    CString copy(s);
    s.Format("%d", 1);
    CHECK(S(s) == "1");
    CHECK(copy.GetLength() == 10003);
}

#pragma warning(pop)

TEST_CASE("CString LoadString")
{
    CString s("before");
    CHECK_FALSE(s.LoadString(54321));
    CHECK(S(s) == "before");

    CHECK(s.LoadString(101));
    CHECK(S(s) == "Loaded from the string table.");
    CHECK(s.GetAllocLength() == 29);

    CString t;
    CHECK(t.LoadString(102));
    CHECK(S(t) == "%s has %d items");

    // A resource id in place of the format text.
    CString f;
    f.Format(102, "Bag", 3);
    CHECK(S(f) == "Bag has 3 items");

    // A resource id in place of the text.
    CString fromId(MAKEINTRESOURCE(101));
    CHECK(S(fromId) == "Loaded from the string table.");
}

TEST_CASE("CString += stops at a terminator inside the appended text")
{
    CString e("ab\0cd", 5);

    // Appending takes the text up to its first terminator...
    CString a("x");
    a += e;
    CHECK(S(a) == "xab");
    CString b;
    b += e;
    CHECK(S(b) == "ab");
    CString self(e);
    self += self;
    CHECK(S(self) == std::string("ab\0cdab", 7));

    // ...but what is already in the string stays, and operator+ keeps both
    // sides whole.
    CString c(e);
    c += "z";
    CHECK(S(c) == std::string("ab\0cdz", 6));
    c += 'q';
    CHECK(S(c) == std::string("ab\0cdzq", 7));
    CHECK(S(e + e) == std::string("ab\0cdab\0cd", 10));
    CHECK(S("x" + e) == std::string("xab\0cd", 6));
    CHECK(S(e + "x") == std::string("ab\0cdx", 6));

    // The way the game cuts an extension off (CResCache) and goes on using
    // the string.
    CString name("Name.Ext");
    name.SetAt(name.Find('.'), '\0');
    CHECK(name.GetLength() == 8);
    CString dir("dir\\");
    CHECK(S(dir + name) == std::string("dir\\Name\0Ext", 12));
    CString appended(dir);
    appended += name;
    CHECK(S(appended) == "dir\\Name");

    // Assignment from the character pointer stops at the terminator too;
    // assignment from the CString does not.
    CString byPointer;
    byPointer = static_cast<LPCSTR>(e);
    CHECK(S(byPointer) == "ab");
    CString byString;
    byString = e;
    CHECK(S(byString) == std::string("ab\0cd", 5));

    // Other editing with a terminator inside the text.
    CString r(e);
    CHECK(r.Remove('c') == 1);
    CHECK(S(r) == std::string("ab\0d", 4));
    r = e;
    CHECK(r.Replace('c', 'x') == 1);
    CHECK(S(r) == std::string("ab\0xd", 5));
    r = e;
    CHECK(r.Replace("cd", "xyz") == 1);
    CHECK(S(r) == std::string("ab\0xyz", 6));
    r = e;
    CHECK(r.Replace("b", "") == 1);
    CHECK(S(r) == std::string("a\0cd", 4));
    r = e;
    r.MakeUpper();
    CHECK(S(r) == std::string("AB\0cd", 5));
    r = e;
    CHECK(r.Insert(4, "XY") == 7);
    CHECK(S(r) == std::string("ab\0cXYd", 7));
    r = e;
    r.GetBuffer(0);
    r.ReleaseBuffer();
    CHECK(S(r) == "ab");
}

TEST_CASE("CString capacity (GetAllocLength)")
{
    // Game code copies GetAllocLength() bytes out of strings, so the
    // capacity has to follow MFC's rules exactly.
    auto text = [](int n) { return std::string(n, 'x'); };

    CHECK(CString().GetAllocLength() == 0);
    for (int n : { 1, 5, 7, 8, 15, 16, 31, 32, 33, 100 }) {
        CString s(text(n).c_str());
        CHECK(s.GetAllocLength() == n);
    }
    CHECK(CString("hello", 0).GetAllocLength() == 0);
    CHECK(CString("hello", 3).GetAllocLength() == 3);
    CHECK(CString('x', 9).GetAllocLength() == 9);
    CHECK(CString("").GetAllocLength() == 0);

    // A copy shares the buffer and so reports the same capacity.
    {
        CString a("hello");
        CString s(a);
        CHECK(s.GetAllocLength() == 5);
        CString assigned;
        assigned = a;
        CHECK(assigned.GetAllocLength() == 5);
    }

    // Assignment of text: a buffer that is big enough is kept; one that is
    // too small grows to one and a half times its size or to what is needed.
    {
        CString s;
        s = "abc";
        CHECK(s.GetAllocLength() == 3);
        s = "0123456789";
        CHECK(s.GetAllocLength() == 10);
        s = "ab";
        CHECK(s.GetAllocLength() == 10);
        s = 'z';
        CHECK(s.GetAllocLength() == 10);
        s = "";
        CHECK(s.GetAllocLength() == 0);
        s = 'z';
        CHECK(s.GetAllocLength() == 1);
    }
    {
        CString s(text(31).c_str());
        s = text(33).c_str();
        CHECK(s.GetAllocLength() == 46);
    }
    // A shared buffer is replaced by one for the longer of old and new text.
    {
        CString s(text(31).c_str());
        CString keep(s);
        s = "ab";
        CHECK(s.GetLength() == 2);
        CHECK(s.GetAllocLength() == 31);
        CHECK(keep.GetAllocLength() == 31);
    }
    {
        CString s(text(31).c_str());
        CString keep(s);
        s = text(40).c_str();
        CHECK(s.GetAllocLength() == 40);
    }

    // Appending.
    {
        CString s("abc");
        s += "d";
        CHECK(s.GetAllocLength() == 4);
        s += "efgh";
        CHECK(s.GetAllocLength() == 8);
        s += 'i';
        CHECK(s.GetAllocLength() == 12);
        s += "jkl";
        CHECK(s.GetAllocLength() == 12);
        s += 'm';
        CHECK(s.GetAllocLength() == 18);
    }
    {
        CString s;
        s += "d";
        CHECK(s.GetAllocLength() == 1);
        CString c;
        c += 'd';
        CHECK(c.GetAllocLength() == 1);
        CString shared("abc");
        CString keep(shared);
        shared += "d";
        CHECK(shared.GetAllocLength() == 4);
    }
    {
        CString s;
        for (int i = 0; i < 100; i++) {
            s += 'x';
        }
        CHECK(s.GetAllocLength() == 141);
        CString t;
        for (int i = 0; i < 20; i++) {
            t += "abcdefghij";
        }
        CHECK(t.GetAllocLength() == 225);
    }

    // Concatenation and extraction give exact-size strings.
    {
        CString a("abc"), b("defgh"), e;
        CHECK((a + b).GetAllocLength() == 8);
        CHECK((a + "xy").GetAllocLength() == 5);
        CHECK((a + 'c').GetAllocLength() == 4);
        CHECK((a + e).GetAllocLength() == 3);
        CHECK((e + e).GetAllocLength() == 0);

        CString d("0123456789");
        CHECK(d.Mid(2, 3).GetAllocLength() == 3);
        CHECK(d.Mid(2, 0).GetAllocLength() == 0);
        CHECK(d.Mid(20).GetAllocLength() == 0);
        CHECK(d.Mid(0).GetAllocLength() == 10);
        CHECK(d.Left(4).GetAllocLength() == 4);
        CHECK(d.Left(0).GetAllocLength() == 0);
        CHECK(d.Left(10).GetAllocLength() == 10);
        CHECK(d.Right(3).GetAllocLength() == 3);
        CHECK(d.SpanIncluding("012").GetAllocLength() == 3);
    }

    // Formatting.
    {
        CString s;
        s.Format("%d", 5);
        CHECK(s.GetAllocLength() == 1);
        s.Format("%s", text(40).c_str());
        CHECK(s.GetAllocLength() == 40);
        s.Format("%d", 5);
        CHECK(s.GetAllocLength() == 40);
        s.Format("");
        CHECK(s.GetLength() == 0);
        CHECK(s.GetAllocLength() == 40);

        CString grown(text(31).c_str());
        grown.Format("%s", text(33).c_str());
        CHECK(grown.GetAllocLength() == 46);

        CString shared(text(31).c_str());
        CString keep(shared);
        shared.Format("%d", 5);
        CHECK(shared.GetAllocLength() == 31);
    }

    // Editing in place keeps the buffer.
    {
        CString s("  abc  ");
        s.TrimLeft();
        CHECK(s.GetAllocLength() == 7);
        s.TrimRight();
        CHECK(s.GetAllocLength() == 7);
        s.MakeUpper();
        CHECK(s.GetAllocLength() == 7);
        s.Delete(0, 3);
        CHECK(s.GetLength() == 0);
        CHECK(s.GetAllocLength() == 7);
        s.Empty();
        CHECK(s.GetAllocLength() == 7);

        CString full("abc");
        full.Empty();
        CHECK(full.GetAllocLength() == 0);

        CString blank("   ");
        blank.TrimLeft();
        CHECK(blank.GetAllocLength() == 3);
        CString removed("aaaa");
        removed.Remove('a');
        CHECK(removed.GetAllocLength() == 4);
    }
    {
        CString s("abcabc");
        s.Replace("abc", "x");
        CHECK(s.GetAllocLength() == 6);
        s.Replace("x", "0123456789");
        CHECK(s.GetAllocLength() == 20);

        CString shared("abcabc");
        CString keep(shared);
        shared.Replace("abc", "x");
        CHECK(shared.GetAllocLength() == 6);

        CString ins("abc");
        ins.Insert(1, 'x');
        CHECK(ins.GetAllocLength() == 4);
        ins.Insert(1, "0123456789");
        CHECK(ins.GetAllocLength() == 14);
        CString seven("abcdefg");
        seven.Insert(1, 'x');
        CHECK(seven.GetAllocLength() == 10);
    }

    // Buffer access.
    {
        CString s;
        s.GetBuffer(32);
        CHECK(s.GetAllocLength() == 32);
        CString a("abc");
        a.GetBuffer(8);
        CHECK(a.GetAllocLength() == 8);
        a.GetBuffer(2);
        CHECK(a.GetAllocLength() == 8);
        a.GetBuffer(20);
        CHECK(a.GetAllocLength() == 20);
        a.ReleaseBuffer();
        CHECK(a.GetAllocLength() == 20);
        a.GetBufferSetLength(0);
        CHECK(a.GetAllocLength() == 20);

        CString shared("abc");
        CString keep(shared);
        shared.GetBuffer(1);
        CHECK(shared.GetAllocLength() == 3);
        CString big(text(31).c_str());
        CString keepBig(big);
        big.SetAt(0, 'y');
        CHECK(big.GetAllocLength() == 31);
    }

    // The pattern the game uses to copy a name into a fixed field: with an
    // exact capacity the copy is the text and nothing else.
    {
        CString name("SoundSet");
        char field[32];
        memset(field, '#', sizeof(field));
        DWORD nLength = name.GetAllocLength();
        memcpy(field, name.GetBuffer(nLength), min(nLength, 32));
        if (nLength < 32) {
            field[nLength] = '\0';
        }
        CHECK(nLength == 8);
        CHECK(std::string(field, 10) == std::string("SoundSet\0#", 10));
    }
}

TEST_CASE("CString rejects out-of-range arguments by throwing")
{
    CString s("abc");
    CString e;
    LPCSTR null = NULL;

    // Reading: the position of the terminator is allowed, beyond is not.
    CHECK_FALSE(Throws([&] { g_sink = s.GetAt(2); }));
    CHECK_FALSE(Throws([&] { g_sink = s.GetAt(3); }));
    CHECK(Throws([&] { g_sink = s.GetAt(4); }));
    CHECK(Throws([&] { g_sink = s.GetAt(-1); }));
    CHECK_FALSE(Throws([&] { g_sink = e.GetAt(0); }));
    CHECK(Throws([&] { g_sink = e.GetAt(1); }));
    CHECK_FALSE(Throws([&] { g_sink = s[3]; }));
    CHECK(Throws([&] { g_sink = s[4]; }));
    CHECK(Throws([&] { g_sink = s[-1]; }));
    CHECK_FALSE(Throws([&] { g_sink = e[0]; }));
    CHECK(Throws([&] { g_sink = e[1]; }));

    // Writing: only inside the text.
    CHECK_FALSE(Throws([&] { CString t(s); t.SetAt(2, 'x'); }));
    CHECK(Throws([&] { CString t(s); t.SetAt(3, 'x'); }));
    CHECK(Throws([&] { CString t(s); t.SetAt(-1, 'x'); }));
    CHECK(Throws([&] { CString t; t.SetAt(0, 'x'); }));

    CHECK(Throws([&] { g_sink = s.Compare(null); }));
    CHECK(Throws([&] { g_sink = s.CompareNoCase(null); }));
    CHECK(Throws([&] { g_sink = s.SpanIncluding(null).GetLength(); }));
    CHECK(Throws([&] { CString t; t.Format(null); }));

    CHECK(Throws([&] { CString t(s); t.GetBuffer(-1); }));
    CHECK(Throws([&] { CString t(s); t.GetBufferSetLength(-1); }));
    CHECK(Throws([&] { CString t(s); t.GetBuffer(0); t.ReleaseBuffer(10); }));
    CHECK(Throws([&] { CString t(s); t.GetBuffer(0); t.ReleaseBuffer(-2); }));

    CHECK(Throws([&] { CString t("abc", -1); g_sink = t.GetLength(); }));
    CHECK(Throws([&] { CString t(null, 2); g_sink = t.GetLength(); }));
    CHECK_FALSE(Throws([&] { CString t(null, 0); g_sink = t.GetLength(); }));
    CHECK_FALSE(Throws([&] { CString t('x', -1); g_sink = t.GetLength(); }));

    // A null text pointer is an empty text in these.
    CHECK(s.Find(null) == -1);
    CString t(s);
    t += null;
    CHECK(S(t) == "abc");
    CHECK(S(s + null) == "abc");
    t.TrimLeft(null);
    t.TrimRight(null);
    CHECK(S(t) == "abc");
    CHECK(t.Replace(null, "x") == 0);
    CHECK(t.Replace("a", null) == 1);
    CHECK(S(t) == "bc");
    t = null;
    CHECK(t.IsEmpty());

    // Out-of-range positions in these are clamped instead.
    CHECK(S(s.Mid(-1, -1)) == "");
    CHECK(S(s.Left(-5)) == "");
    CString d(s);
    CHECK(d.Delete(5, 1) == 3);
    CHECK(d.Insert(9, 'x') == 4);
    CHECK(S(d) == "abcx");
}

TEST_CASE("CString is usable as a C string argument and in containers")
{
    CString s("12345");
    CHECK(atoi(s) == 12345);
    CHECK(strlen(s) == 5);

    LPCSTR psz = s;
    CHECK(strcmp(psz, "12345") == 0);

    std::vector<CString> v;
    for (int i = 0; i < 100; i++) {
        CString item;
        item.Format("item%d", i);
        v.push_back(item);
    }
    CHECK(S(v[0]) == "item0");
    CHECK(S(v[99]) == "item99");
}

TEST_CASE("CString copies can be made and dropped from several threads")
{
    CString shared("shared between threads");
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; t++) {
        threads.emplace_back([&shared]() {
            for (int i = 0; i < 20000; i++) {
                CString copy(shared);
                CString other;
                other = copy;
                if (other.GetLength() != 22) {
                    abort();
                }
            }
        });
    }
    for (std::thread& thread : threads) {
        thread.join();
    }
    CHECK(S(shared) == "shared between threads");

    // The buffer is private again once the copies are gone: a write does
    // not need to move it.
    LPCSTR before = shared;
    shared.SetAt(0, 'S');
    CHECK(static_cast<LPCSTR>(shared) == before);
}
