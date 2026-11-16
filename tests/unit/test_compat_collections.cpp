// List, array and map behaviour the game relies on. Runs against MFC in the
// default build and against src/compat in the IWD2RE_NO_MFC build.

#include "test.h"

#include <string>
#include <vector>

namespace {

void* P(int n)
{
    return reinterpret_cast<void*>(static_cast<INT_PTR>(n));
}

int N(void* p)
{
    return static_cast<int>(reinterpret_cast<INT_PTR>(p));
}

std::vector<int> Forward(const CPtrList& list)
{
    std::vector<int> v;
    POSITION pos = list.GetHeadPosition();
    while (pos != NULL) {
        v.push_back(N(const_cast<void*>(list.GetNext(pos))));
    }
    return v;
}

std::vector<int> Backward(const CPtrList& list)
{
    std::vector<int> v;
    POSITION pos = list.GetTailPosition();
    while (pos != NULL) {
        v.push_back(N(const_cast<void*>(list.GetPrev(pos))));
    }
    return v;
}

typedef std::vector<int> V;

// Counts live objects to check that containers construct and destroy
// their elements in pairs.
struct Counted {
    static int live;
    int value;
    Counted()
        : value(0)
    {
        live++;
    }
    Counted(const Counted& other)
        : value(other.value)
    {
        live++;
    }
    explicit Counted(int v)
        : value(v)
    {
        live++;
    }
    ~Counted() { live--; }
    Counted& operator=(const Counted& other)
    {
        value = other.value;
        return *this;
    }
    bool operator==(const Counted& other) const { return value == other.value; }
};
int Counted::live = 0;

class CTestObject : public CObject {
public:
    explicit CTestObject(int n)
        : m_n(n)
    {
    }
    int m_n;
};

// Gives a test access to the list's node type, like the game's own classes
// that derive from the MFC lists.
class CNodeList : public CPtrList {
public:
    static size_t NodeSize() { return sizeof(CNode); }
    static size_t DataOffset() { return offsetof(CNode, data); }
    static void* NextOf(POSITION pos) { return reinterpret_cast<CNode*>(pos)->pNext; }
    static void* PrevOf(POSITION pos) { return reinterpret_cast<CNode*>(pos)->pPrev; }
    size_t HeadOffset() const { return reinterpret_cast<const char*>(&m_pNodeHead) - reinterpret_cast<const char*>(this); }
    size_t CountOffset() const { return reinterpret_cast<const char*>(&m_nCount) - reinterpret_cast<const char*>(this); }
    size_t BlockSizeOffset() const { return reinterpret_cast<const char*>(&m_nBlockSize) - reinterpret_cast<const char*>(this); }
};

class CSizedArray : public CPtrArray {
public:
    size_t DataOffset() const { return reinterpret_cast<const char*>(&m_pData) - reinterpret_cast<const char*>(this); }
    size_t SizeOffset() const { return reinterpret_cast<const char*>(&m_nSize) - reinterpret_cast<const char*>(this); }
    size_t MaxSizeOffset() const { return reinterpret_cast<const char*>(&m_nMaxSize) - reinterpret_cast<const char*>(this); }
    size_t GrowByOffset() const { return reinterpret_cast<const char*>(&m_nGrowBy) - reinterpret_cast<const char*>(this); }
};

} // namespace

TEST_CASE("collection object sizes (32-bit)")
{
#if defined(_M_IX86)
    CHECK(sizeof(POSITION) == 4);
    CHECK(sizeof(CObject) == 0x4);
    CHECK(sizeof(CPtrList) == 0x1C);
    CHECK(sizeof(CObList) == 0x1C);
    CHECK(sizeof(CStringList) == 0x1C);
    CHECK(sizeof(CList<int, int>) == 0x1C);
    CHECK(sizeof(CList<CRect>) == 0x1C);
    CHECK(sizeof(CTypedPtrList<CPtrList, int*>) == 0x1C);
    CHECK(sizeof(CPtrArray) == 0x14);
    CHECK(sizeof(CObArray) == 0x14);
    CHECK(sizeof(CStringArray) == 0x14);
    CHECK(sizeof(CDWordArray) == 0x14);
    CHECK(sizeof(CArray<CRect>) == 0x14);
    CHECK(sizeof(CTypedPtrArray<CPtrArray, int*>) == 0x14);
    CHECK(sizeof(CMapWordToPtr) == 0x1C);
    CHECK(sizeof(CMapStringToString) == 0x1C);
    CHECK(sizeof(CTypedPtrMap<CMapWordToPtr, USHORT, int*>) == 0x1C);

    CNodeList list;
    CHECK(list.HeadOffset() == 0x4);
    CHECK(list.CountOffset() == 0xC);
    CHECK(list.BlockSizeOffset() == 0x18);
    CHECK(CNodeList::NodeSize() == 12);
    CHECK(CNodeList::DataOffset() == 8);

    CSizedArray ary;
    CHECK(ary.DataOffset() == 0x4);
    CHECK(ary.SizeOffset() == 0x8);
    CHECK(ary.MaxSizeOffset() == 0xC);
    CHECK(ary.GrowByOffset() == 0x10);
#endif
}

TEST_CASE("CPtrList add, count, head and tail")
{
    CPtrList list;
    CHECK(list.IsEmpty());
    CHECK(list.GetCount() == 0);
    CHECK(list.GetHeadPosition() == NULL);
    CHECK(list.GetTailPosition() == NULL);

    list.AddTail(P(2));
    list.AddTail(P(3));
    list.AddHead(P(1));
    CHECK_FALSE(list.IsEmpty());
    CHECK(list.GetCount() == 3);
    CHECK(Forward(list) == V { 1, 2, 3 });
    CHECK(Backward(list) == V { 3, 2, 1 });
    CHECK(N(list.GetHead()) == 1);
    CHECK(N(list.GetTail()) == 3);

    // GetHead and GetTail give access to the stored element.
    list.GetHead() = P(10);
    list.GetTail() = P(30);
    CHECK(Forward(list) == V { 10, 2, 30 });

    CHECK(N(list.RemoveHead()) == 10);
    CHECK(Forward(list) == V { 2, 30 });
    CHECK(N(list.RemoveTail()) == 30);
    CHECK(Forward(list) == V { 2 });
    CHECK(N(list.RemoveTail()) == 2);
    CHECK(list.IsEmpty());
    CHECK(list.GetHeadPosition() == NULL);
    CHECK(list.GetTailPosition() == NULL);

    // Usable again after becoming empty.
    list.AddHead(P(5));
    list.AddHead(P(4));
    CHECK(Forward(list) == V { 4, 5 });
    list.RemoveAll();
    CHECK(list.IsEmpty());
    CHECK(list.GetCount() == 0);
    list.RemoveAll();
    CHECK(list.GetHeadPosition() == NULL);
}

TEST_CASE("CPtrList positions")
{
    CPtrList list;
    POSITION p1 = list.AddTail(P(1));
    POSITION p2 = list.AddTail(P(2));
    POSITION p3 = list.AddTail(P(3));

    CHECK(list.GetHeadPosition() == p1);
    CHECK(list.GetTailPosition() == p3);
    CHECK(N(list.GetAt(p2)) == 2);

    // GetNext returns the element at the position and steps forward.
    POSITION pos = p1;
    CHECK(N(list.GetNext(pos)) == 1);
    CHECK(pos == p2);
    CHECK(N(list.GetNext(pos)) == 2);
    CHECK(pos == p3);
    CHECK(N(list.GetNext(pos)) == 3);
    CHECK(pos == NULL);

    pos = p3;
    CHECK(N(list.GetPrev(pos)) == 3);
    CHECK(pos == p2);
    CHECK(N(list.GetPrev(pos)) == 2);
    CHECK(N(list.GetPrev(pos)) == 1);
    CHECK(pos == NULL);

    list.SetAt(p2, P(20));
    CHECK(Forward(list) == V { 1, 20, 3 });
    list.GetAt(p2) = P(21);
    CHECK(Forward(list) == V { 1, 21, 3 });
    pos = p1;
    list.GetNext(pos) = P(11);
    CHECK(Forward(list) == V { 11, 21, 3 });

    // Positions of other elements stay valid across inserts and removals.
    POSITION p0 = list.AddHead(P(0));
    list.RemoveAt(p2);
    CHECK(N(list.GetAt(p0)) == 0);
    CHECK(N(list.GetAt(p1)) == 11);
    CHECK(N(list.GetAt(p3)) == 3);
    CHECK(Forward(list) == V { 0, 11, 3 });
}

TEST_CASE("a POSITION is the address of a node {next, previous, element}")
{
    // Game code reads list nodes directly (CGameEffectList), so this layout
    // is part of the contract.
    CNodeList list;
    int a = 1, b = 2, c = 3;
    POSITION p1 = list.AddTail(&a);
    POSITION p2 = list.AddTail(&b);
    POSITION p3 = list.AddTail(&c);

    void** node2 = reinterpret_cast<void**>(p2);
    CHECK(node2[0] == reinterpret_cast<void*>(p3));
    CHECK(node2[1] == reinterpret_cast<void*>(p1));
    CHECK(node2[2] == &b);

    CHECK(CNodeList::NextOf(p1) == reinterpret_cast<void*>(p2));
    CHECK(CNodeList::PrevOf(p1) == NULL);
    CHECK(CNodeList::NextOf(p3) == NULL);
    CHECK(CNodeList::PrevOf(p3) == reinterpret_cast<void*>(p2));
    CHECK(&list.GetAt(p2) == reinterpret_cast<void**>(p2) + 2);
}

TEST_CASE("list nodes come from blocks and are reused")
{
    CPtrList list; // default block size: 10 nodes
    std::vector<POSITION> pos;
    for (int i = 0; i < 25; i++) {
        pos.push_back(list.AddTail(P(i)));
    }

    // Nodes of one block are handed out in address order.
    for (int block = 0; block < 2; block++) {
        for (int i = 1; i < 10; i++) {
            const char* prev = reinterpret_cast<const char*>(pos[block * 10 + i - 1]);
            const char* cur = reinterpret_cast<const char*>(pos[block * 10 + i]);
            CHECK(cur - prev == 3 * static_cast<int>(sizeof(void*)));
        }
    }

    // A removed node is the next one handed out, and stays readable memory
    // in between.
    POSITION removed = pos[7];
    list.RemoveAt(removed);
    CHECK(list.GetCount() == 24);
    POSITION reused = list.AddTail(P(100));
    CHECK(reused == removed);

    POSITION r1 = pos[3];
    POSITION r2 = pos[4];
    list.RemoveAt(r1);
    list.RemoveAt(r2);
    CHECK(list.AddHead(P(200)) == r2);
    CHECK(list.AddHead(P(201)) == r1);

    CPtrList tiny(3);
    POSITION s1 = tiny.AddTail(P(1));
    POSITION s2 = tiny.AddTail(P(2));
    POSITION s3 = tiny.AddTail(P(3));
    CHECK(reinterpret_cast<const char*>(s2) - reinterpret_cast<const char*>(s1) == 3 * static_cast<int>(sizeof(void*)));
    CHECK(reinterpret_cast<const char*>(s3) - reinterpret_cast<const char*>(s2) == 3 * static_cast<int>(sizeof(void*)));
    tiny.AddTail(P(4));
    CHECK(Forward(tiny) == V { 1, 2, 3, 4 });
}

TEST_CASE("CPtrList insert and remove at the ends and in the middle")
{
    CPtrList list;
    POSITION p2 = list.AddTail(P(2));
    POSITION p4 = list.AddTail(P(4));

    POSITION p1 = list.InsertBefore(p2, P(1));
    CHECK(list.GetHeadPosition() == p1);
    POSITION p3 = list.InsertBefore(p4, P(3));
    POSITION p5 = list.InsertAfter(p4, P(5));
    CHECK(list.GetTailPosition() == p5);
    list.InsertAfter(p1, P(15));
    CHECK(Forward(list) == V { 1, 15, 2, 3, 4, 5 });
    CHECK(Backward(list) == V { 5, 4, 3, 2, 15, 1 });
    CHECK(list.GetCount() == 6);

    // A NULL position means the head for InsertBefore, the tail for
    // InsertAfter.
    list.InsertBefore(NULL, P(0));
    list.InsertAfter(NULL, P(6));
    CHECK(Forward(list) == V { 0, 1, 15, 2, 3, 4, 5, 6 });
    CHECK(Backward(list) == V { 6, 5, 4, 3, 2, 15, 1, 0 });

    list.RemoveAt(p3);
    CHECK(Forward(list) == V { 0, 1, 15, 2, 4, 5, 6 });
    list.RemoveAt(list.GetHeadPosition());
    CHECK(Forward(list) == V { 1, 15, 2, 4, 5, 6 });
    list.RemoveAt(list.GetTailPosition());
    CHECK(Forward(list) == V { 1, 15, 2, 4, 5 });
    CHECK(Backward(list) == V { 5, 4, 2, 15, 1 });
    CHECK(list.GetHeadPosition() == p1);
    CHECK(list.GetTailPosition() == p5);

    // Into an empty list.
    CPtrList empty;
    empty.InsertBefore(NULL, P(7));
    CHECK(Forward(empty) == V { 7 });
    CPtrList empty2;
    empty2.InsertAfter(NULL, P(8));
    CHECK(Forward(empty2) == V { 8 });

    // The only element.
    empty.RemoveAt(empty.GetHeadPosition());
    CHECK(empty.IsEmpty());
    CHECK(empty.GetHeadPosition() == NULL);
    CHECK(empty.GetTailPosition() == NULL);
}

TEST_CASE("CPtrList removing while iterating")
{
    CPtrList list;
    for (int i = 1; i <= 10; i++) {
        list.AddTail(P(i));
    }

    // The game's usual pattern: remember the position, step, then remove.
    POSITION pos = list.GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        void* value = list.GetNext(pos);
        if (N(value) % 2 == 0) {
            list.RemoveAt(posOld);
        }
    }
    CHECK(Forward(list) == V { 1, 3, 5, 7, 9 });
    CHECK(Backward(list) == V { 9, 7, 5, 3, 1 });
    CHECK(list.GetCount() == 5);

    // Backwards, removing everything.
    pos = list.GetTailPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        list.GetPrev(pos);
        list.RemoveAt(posOld);
    }
    CHECK(list.IsEmpty());

    // Draining with RemoveHead.
    for (int i = 0; i < 30; i++) {
        list.AddTail(P(i));
    }
    int expected = 0;
    while (!list.IsEmpty()) {
        CHECK(N(list.RemoveHead()) == expected++);
    }
    CHECK(expected == 30);
}

TEST_CASE("CPtrList Find and FindIndex")
{
    CPtrList list;
    POSITION p1 = list.AddTail(P(1));
    POSITION p2 = list.AddTail(P(2));
    POSITION p3 = list.AddTail(P(1));
    POSITION p4 = list.AddTail(P(3));

    CHECK(list.Find(P(1)) == p1);
    CHECK(list.Find(P(2)) == p2);
    CHECK(list.Find(P(3)) == p4);
    CHECK(list.Find(P(9)) == NULL);
    CHECK(list.Find(NULL) == NULL);

    // The search starts after the given position.
    CHECK(list.Find(P(1), p1) == p3);
    CHECK(list.Find(P(1), p3) == NULL);
    CHECK(list.Find(P(2), p2) == NULL);
    CHECK(list.Find(P(3), p4) == NULL);

    CHECK(list.FindIndex(0) == p1);
    CHECK(list.FindIndex(1) == p2);
    CHECK(list.FindIndex(3) == p4);
    CHECK(list.FindIndex(4) == NULL);
    CHECK(list.FindIndex(100) == NULL);
    CHECK(list.FindIndex(-1) == NULL);

    CPtrList empty;
    CHECK(empty.Find(P(1)) == NULL);
    CHECK(empty.FindIndex(0) == NULL);
}

TEST_CASE("CPtrList appending and prepending another list")
{
    CPtrList a;
    a.AddTail(P(1));
    a.AddTail(P(2));
    CPtrList b;
    b.AddTail(P(3));
    b.AddTail(P(4));

    a.AddTail(&b);
    CHECK(Forward(a) == V { 1, 2, 3, 4 });
    CHECK(Forward(b) == V { 3, 4 });

    CPtrList c;
    c.AddTail(P(8));
    c.AddTail(P(9));
    a.AddHead(&c);
    CHECK(Forward(a) == V { 8, 9, 1, 2, 3, 4 });
    CHECK(a.GetCount() == 6);

    CPtrList empty;
    a.AddTail(&empty);
    a.AddHead(&empty);
    CHECK(a.GetCount() == 6);
}

TEST_CASE("CTypedPtrList")
{
    int a = 1, b = 2, c = 3;
    CTypedPtrList<CPtrList, int*> list;
    list.AddTail(&b);
    list.AddHead(&a);
    POSITION pc = list.AddTail(&c);

    CHECK(list.GetCount() == 3);
    CHECK(list.GetHead() == &a);
    CHECK(list.GetTail() == &c);
    CHECK(list.GetAt(pc) == &c);

    int sum = 0;
    POSITION pos = list.GetHeadPosition();
    while (pos != NULL) {
        int* p = list.GetNext(pos);
        sum = sum * 10 + *p;
    }
    CHECK(sum == 123);

    sum = 0;
    pos = list.GetTailPosition();
    while (pos != NULL) {
        sum = sum * 10 + *list.GetPrev(pos);
    }
    CHECK(sum == 321);

    CHECK(list.Find(&b) != NULL);
    list.SetAt(pc, &a);
    CHECK(list.GetTail() == &a);
    list.GetTail() = &c;
    CHECK(list.GetAt(pc) == &c);

    CHECK(list.RemoveHead() == &a);
    CHECK(list.RemoveTail() == &c);
    CHECK(list.GetCount() == 1);

    CTypedPtrList<CPtrList, int*> other;
    other.AddTail(&a);
    list.AddTail(&other);
    list.AddHead(&other);
    CHECK(list.GetCount() == 3);
    CHECK(list.GetHead() == &a);
    CHECK(list.GetTail() == &a);

    // Through a const reference.
    const CTypedPtrList<CPtrList, int*>& constList = list;
    pos = constList.GetHeadPosition();
    CHECK(constList.GetNext(pos) == &a);
    CHECK(constList.GetAt(pos) == &b);
    CHECK(constList.GetHead() == &a);
}

TEST_CASE("CObList")
{
    CTestObject a(1), b(2), c(3);
    CObList list;
    list.AddTail(&a);
    list.AddTail(&b);
    list.AddHead(&c);
    CHECK(list.GetCount() == 3);
    CHECK(static_cast<CTestObject*>(list.GetHead())->m_n == 3);
    CHECK(static_cast<CTestObject*>(list.GetTail())->m_n == 2);
    CHECK(list.Find(&b) == list.GetTailPosition());

    int sum = 0;
    POSITION pos = list.GetHeadPosition();
    while (pos != NULL) {
        sum = sum * 10 + static_cast<CTestObject*>(list.GetNext(pos))->m_n;
    }
    CHECK(sum == 312);

    // The list does not own the objects.
    list.RemoveAll();
    CHECK(a.m_n == 1);

    CTypedPtrList<CObList, CTestObject*> typed;
    typed.AddTail(&a);
    typed.AddTail(&b);
    CHECK(typed.GetHead()->m_n == 1);
    pos = typed.GetHeadPosition();
    typed.GetNext(pos);
    CHECK(typed.GetNext(pos)->m_n == 2);
    CHECK(typed.RemoveHead() == &a);
}

TEST_CASE("CStringList")
{
    CStringList list;
    list.AddTail("two");
    list.AddTail(CString("three"));
    list.AddHead("one");
    CHECK(list.GetCount() == 3);
    CHECK(list.GetHead() == "one");
    CHECK(list.GetTail() == "three");

    std::string all;
    POSITION pos = list.GetHeadPosition();
    while (pos != NULL) {
        all += list.GetNext(pos);
        all += ",";
    }
    CHECK(all == "one,two,three,");

    // Find compares text (case-sensitive), not pointers.
    CString needle("two");
    POSITION found = list.Find(needle);
    CHECK(found != NULL);
    CHECK(list.GetAt(found) == "two");
    CHECK(list.Find("TWO") == NULL);
    CHECK(list.Find("four") == NULL);
    CHECK(list.Find("one", list.GetHeadPosition()) == NULL);

    // Elements are copies and can be changed in place.
    CString source("source");
    POSITION added = list.AddTail(source);
    source = "changed";
    CHECK(list.GetAt(added) == "source");
    list.GetAt(added) += "!";
    CHECK(list.GetTail() == "source!");
    list.SetAt(added, "set");
    CHECK(list.GetTail() == "set");

    list.InsertBefore(found, "one-and-a-half");
    list.InsertAfter(found, CString("two-and-a-half"));
    all.clear();
    pos = list.GetHeadPosition();
    while (pos != NULL) {
        all += list.GetNext(pos);
        all += ",";
    }
    CHECK(all == "one,one-and-a-half,two,two-and-a-half,three,set,");

    CString head = list.RemoveHead();
    CHECK(head == "one");
    CString tail = list.RemoveTail();
    CHECK(tail == "set");
    CHECK(list.GetCount() == 4);
    list.RemoveAt(found);
    CHECK(list.Find("two") == NULL);
    list.RemoveAll();
    CHECK(list.IsEmpty());
}

TEST_CASE("CList of values")
{
    CList<int, int> ints;
    ints.AddTail(3);
    ints.AddHead(1);
    ints.InsertAfter(ints.GetHeadPosition(), 2);
    CHECK(ints.GetCount() == 3);
    CHECK(ints.GetHead() == 1);
    CHECK(ints.GetTail() == 3);
    CHECK(ints.Find(2) == ints.FindIndex(1));
    CHECK(ints.Find(7) == NULL);
    CHECK(ints.RemoveHead() == 1);
    CHECK(ints.RemoveTail() == 3);
    CHECK(ints.GetCount() == 1);

    // The default argument type, as in `CList<LONG>`.
    CList<LONG> longs;
    longs.AddTail(5);
    longs.AddTail(6);
    CHECK(longs.Find(6) == longs.GetTailPosition());
    POSITION pos = longs.GetHeadPosition();
    CHECK(longs.GetNext(pos) == 5);
    CHECK(longs.GetNext(pos) == 6);
    CHECK(pos == NULL);

    CList<CString, CString> strings;
    strings.AddTail(CString("b"));
    strings.AddHead(CString("a"));
    CHECK(strings.GetHead() == "a");
    CHECK(strings.Find(CString("b")) == strings.GetTailPosition());
    CHECK(strings.RemoveTail() == "b");

    CList<CRect> rects;
    rects.AddTail(CRect(1, 2, 3, 4));
    CHECK(rects.GetHead().right == 3);
    rects.GetHead().OffsetRect(1, 1);
    CHECK(rects.GetHead().right == 4);
}

TEST_CASE("CList constructs and destroys elements in pairs")
{
    Counted::live = 0;
    {
        CList<Counted, const Counted&> list;
        for (int i = 0; i < 25; i++) {
            list.AddTail(Counted(i));
        }
        CHECK(Counted::live == 25);
        CHECK(list.GetCount() == 25);
        CHECK(list.GetAt(list.FindIndex(13)).value == 13);

        list.RemoveAt(list.FindIndex(3));
        CHECK(Counted::live == 24);
        {
            Counted head = list.RemoveHead();
            CHECK(head.value == 0);
        }
        {
            Counted tail = list.RemoveTail();
            CHECK(tail.value == 24);
        }
        CHECK(Counted::live == 22);

        list.InsertBefore(list.GetHeadPosition(), Counted(100));
        list.InsertAfter(list.GetTailPosition(), Counted(101));
        CHECK(Counted::live == 24);
        CHECK(list.GetHead().value == 100);
        CHECK(list.GetTail().value == 101);
        CHECK(list.Find(Counted(13)) != NULL);
        CHECK(list.Find(Counted(3)) == NULL);
    }
    CHECK(Counted::live == 0);

    {
        CList<Counted, const Counted&> list;
        list.AddTail(Counted(1));
        list.AddTail(Counted(2));
        list.RemoveAll();
        CHECK(Counted::live == 0);
        list.AddTail(Counted(3));
        CHECK(Counted::live == 1);
    }
    CHECK(Counted::live == 0);
}

TEST_CASE("CPtrArray")
{
    CPtrArray ary;
    CHECK(ary.GetSize() == 0);
    CHECK(ary.GetUpperBound() == -1);
    CHECK(ary.GetData() == NULL);

    CHECK(ary.Add(P(10)) == 0);
    CHECK(ary.Add(P(11)) == 1);
    CHECK(ary.Add(P(12)) == 2);
    CHECK(ary.GetSize() == 3);
    CHECK(ary.GetUpperBound() == 2);
    CHECK(N(ary.GetAt(1)) == 11);
    CHECK(N(ary[2]) == 12);
    CHECK(N(ary.ElementAt(0)) == 10);
    CHECK(ary.GetData() != NULL);
    CHECK(N(ary.GetData()[1]) == 11);

    ary.SetAt(1, P(21));
    ary[2] = P(22);
    ary.ElementAt(0) = P(20);
    CHECK(N(ary[0]) == 20);
    CHECK(N(ary[1]) == 21);
    CHECK(N(ary[2]) == 22);

    // Growing zero-fills the new elements.
    ary.SetSize(6);
    CHECK(ary.GetSize() == 6);
    CHECK(N(ary[2]) == 22);
    CHECK(ary[3] == NULL);
    CHECK(ary[5] == NULL);

    // Shrinking and growing again does not bring old values back.
    ary[5] = P(55);
    ary.SetSize(4);
    CHECK(ary.GetSize() == 4);
    ary.SetSize(6);
    CHECK(ary[4] == NULL);
    CHECK(ary[5] == NULL);

    ary.SetAtGrow(9, P(99));
    CHECK(ary.GetSize() == 10);
    CHECK(N(ary[9]) == 99);
    CHECK(ary[8] == NULL);
    ary.SetAtGrow(0, P(1));
    CHECK(ary.GetSize() == 10);
    CHECK(N(ary[0]) == 1);

    ary.RemoveAll();
    CHECK(ary.GetSize() == 0);
    CHECK(ary.GetUpperBound() == -1);
    CHECK(ary.Add(P(5)) == 0);
    ary.SetSize(0);
    CHECK(ary.GetSize() == 0);
}

TEST_CASE("CPtrArray InsertAt and RemoveAt")
{
    CPtrArray ary;
    for (int i = 0; i < 5; i++) {
        ary.Add(P(i));
    }

    auto values = [&ary]() {
        V v;
        for (int i = 0; i < ary.GetSize(); i++) {
            v.push_back(N(ary[i]));
        }
        return v;
    };

    ary.InsertAt(0, P(100));
    CHECK(values() == V { 100, 0, 1, 2, 3, 4 });
    ary.InsertAt(3, P(101));
    CHECK(values() == V { 100, 0, 1, 101, 2, 3, 4 });
    ary.InsertAt(7, P(102));
    CHECK(values() == V { 100, 0, 1, 101, 2, 3, 4, 102 });
    ary.InsertAt(2, P(7), 3);
    CHECK(values() == V { 100, 0, 7, 7, 7, 1, 101, 2, 3, 4, 102 });

    // Past the end: the gap is zero-filled.
    ary.InsertAt(13, P(103));
    CHECK(values() == V { 100, 0, 7, 7, 7, 1, 101, 2, 3, 4, 102, 0, 0, 103 });

    ary.RemoveAt(0);
    CHECK(values() == V { 0, 7, 7, 7, 1, 101, 2, 3, 4, 102, 0, 0, 103 });
    ary.RemoveAt(1, 3);
    CHECK(values() == V { 0, 1, 101, 2, 3, 4, 102, 0, 0, 103 });
    ary.RemoveAt(9);
    CHECK(values() == V { 0, 1, 101, 2, 3, 4, 102, 0, 0 });
    ary.RemoveAt(6, 3);
    CHECK(values() == V { 0, 1, 101, 2, 3, 4 });
    CHECK(ary.GetSize() == 6);

    CPtrArray other;
    other.Add(P(50));
    other.Add(P(51));
    ary.InsertAt(1, &other);
    CHECK(values() == V { 0, 50, 51, 1, 101, 2, 3, 4 });

    CHECK(ary.Append(other) == 8);
    CHECK(values() == V { 0, 50, 51, 1, 101, 2, 3, 4, 50, 51 });

    ary.Copy(other);
    CHECK(values() == V { 50, 51 });

    ary.FreeExtra();
    CHECK(values() == V { 50, 51 });
}

TEST_CASE("arrays keep their content while growing")
{
    CDWordArray words;
    for (DWORD i = 0; i < 5000; i++) {
        CHECK(words.Add(i * 3) == static_cast<int>(i));
    }
    CHECK(words.GetSize() == 5000);
    bool ok = true;
    for (DWORD i = 0; i < 5000; i++) {
        ok = ok && words[i] == i * 3;
    }
    CHECK(ok);
    CHECK(words.GetAt(4999) == 14997);
    words.SetAt(0, 0xFFFFFFFF);
    CHECK(words[0] == 0xFFFFFFFF);

    CDWordArray sized;
    sized.SetSize(100, 50);
    ok = true;
    for (int i = 0; i < 100; i++) {
        ok = ok && sized[i] == 0;
    }
    CHECK(ok);
    sized.Add(1);
    CHECK(sized.GetSize() == 101);

    CArray<DWORD, DWORD> state;
    state.SetSize(8);
    CHECK(state.GetSize() == 8);
    CHECK(state[7] == 0);
    state[7] = 3;
    state.SetSize(16);
    CHECK(state[7] == 3);
    CHECK(state[15] == 0);
}

TEST_CASE("CArray of objects")
{
    CArray<CRect> rects;
    rects.Add(CRect(1, 2, 3, 4));
    rects.Add(CRect(5, 6, 7, 8));
    CHECK(rects.GetSize() == 2);
    CHECK(rects[1].left == 5);
    rects[1].OffsetRect(1, 1);
    CHECK(rects.GetAt(1).left == 6);
    rects.SetSize(3);
    CHECK(rects[2].IsRectNull());

    CArray<CRect*> pointers;
    CRect r;
    pointers.Add(&r);
    pointers.Add(NULL);
    CHECK(pointers.GetSize() == 2);
    CHECK(pointers[0] == &r);
    CHECK(pointers.GetAt(1) == NULL);

    Counted::live = 0;
    {
        CArray<Counted, const Counted&> ary;
        for (int i = 0; i < 40; i++) {
            ary.Add(Counted(i));
        }
        CHECK(Counted::live == 40);
        CHECK(ary[39].value == 39);

        ary.RemoveAt(10, 5);
        CHECK(Counted::live == 35);
        CHECK(ary[10].value == 15);

        ary.InsertAt(0, Counted(-1), 2);
        CHECK(Counted::live == 37);
        CHECK(ary[0].value == -1);
        CHECK(ary[1].value == -1);
        CHECK(ary[2].value == 0);

        ary.SetSize(10);
        CHECK(Counted::live == 10);
        ary.SetSize(12);
        CHECK(Counted::live == 12);
        CHECK(ary[11].value == 0);
    }
    CHECK(Counted::live == 0);
}

TEST_CASE("CStringArray, CObArray, CTypedPtrArray")
{
    CStringArray strings;
    strings.Add("zero");
    strings.Add(CString("one"));
    strings.SetAtGrow(3, "three");
    CHECK(strings.GetSize() == 4);
    CHECK(strings[0] == "zero");
    CHECK(strings.GetAt(1) == "one");
    CHECK(strings[2].IsEmpty());
    CHECK(strings[3] == "three");
    strings[2] = "two";
    strings.ElementAt(0) += "!";
    CHECK(strings[0] == "zero!");
    strings.InsertAt(1, "half");
    CHECK(strings[1] == "half");
    CHECK(strings[2] == "one");
    strings.RemoveAt(0);
    CHECK(strings[0] == "half");
    CHECK(strings.GetSize() == 4);

    // Adding a copy of an element. (Passing the element itself is rejected
    // by MFC when the array has to grow.)
    for (int i = 0; i < 100; i++) {
        CString copy = strings[0];
        strings.Add(copy);
    }
    CHECK(strings[103] == "half");
    strings.SetSize(2);
    CHECK(strings.GetSize() == 2);
    strings.RemoveAll();
    CHECK(strings.GetSize() == 0);

    CTestObject a(1), b(2);
    CObArray objects;
    objects.Add(&a);
    objects.Add(&b);
    objects.SetSize(4);
    CHECK(objects.GetSize() == 4);
    CHECK(static_cast<CTestObject*>(objects[1])->m_n == 2);
    CHECK(objects[3] == NULL);
    objects.SetAt(3, &a);
    CHECK(objects.GetAt(3) == &a);

    int x = 1, y = 2;
    CTypedPtrArray<CPtrArray, int*> typed;
    typed.Add(&x);
    typed.Add(&y);
    CHECK(typed.GetSize() == 2);
    CHECK(typed[0] == &x);
    CHECK(*typed.GetAt(1) == 2);
    typed.SetAt(0, &y);
    CHECK(typed[0] == &y);
    typed[1] = &x;
    CHECK(typed.ElementAt(1) == &x);
    typed.SetAtGrow(4, &x);
    CHECK(typed.GetSize() == 5);
    CHECK(typed[3] == NULL);
    typed.InsertAt(0, &x);
    CHECK(typed[0] == &x);
    CHECK(typed.GetSize() == 6);
    typed.RemoveAt(0);
    CHECK(typed.GetUpperBound() == 4);
}

TEST_CASE("collections reject invalid arguments by throwing")
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

    CPtrArray ary;
    ary.Add(P(1));
    void* p = NULL;
    CHECK_FALSE(throws([&] { p = ary.GetAt(0); }));
    CHECK(throws([&] { p = ary.GetAt(1); }));
    CHECK(throws([&] { p = ary.GetAt(-1); }));
    CHECK(throws([&] { p = ary[5]; }));
    CHECK(throws([&] { p = ary.ElementAt(3); }));
    CHECK(throws([&] { ary.SetAt(3, NULL); }));
    CHECK(throws([&] { ary.RemoveAt(3); }));
    CHECK(throws([&] { ary.RemoveAt(0, 5); }));
    CHECK(throws([&] { ary.RemoveAt(-1); }));
    CHECK(throws([&] { ary.SetSize(-1); }));
    CHECK(throws([&] { ary.InsertAt(-1, P(0)); }));
    CHECK(throws([&] { ary.SetAtGrow(-1, P(0)); }));
    CHECK(ary.GetSize() == 1);
    CHECK(N(ary[0]) == 1);

    CArray<int, int> ints;
    ints.Add(1);
    int n = 0;
    CHECK(throws([&] { n = ints.GetAt(1); }));
    CHECK(throws([&] { n = ints[5]; }));
    CHECK(throws([&] { ints.SetAt(3, 0); }));
    CHECK(throws([&] { ints.RemoveAt(3); }));

    CStringArray strings;
    strings.Add("a");
    CHECK(throws([&] { n = strings.GetAt(2).GetLength(); }));
    CHECK(throws([&] { n = strings[2].GetLength(); }));

    // An element of a string array cannot be added to the same array when
    // the storage is full (it would move while being read).
    CHECK(throws([&] { strings.Add(strings[0]); }));
    strings.Add("b");
    CHECK_FALSE(throws([&] { strings.Add(strings[0]); }));
    CHECK_FALSE(throws([&] { strings.Add(strings[0]); }));
    CHECK_FALSE(throws([&] { strings.Add(strings[0]); }));
    CHECK(strings.GetSize() == 5);
    CHECK(throws([&] { strings.Add(strings[0]); }));
    CHECK(throws([&] { strings.SetAtGrow(20, strings[0]); }));
    CHECK_FALSE(throws([&] { strings.SetAt(1, strings[0]); }));
    CHECK_FALSE(throws([&] { strings.InsertAt(1, strings[0]); }));
    CHECK(strings.GetSize() == 6);
    CHECK(strings[1] == "a");

    // Other arrays take their own elements.
    CPtrArray self;
    self.Add(P(7));
    for (int i = 0; i < 20; i++) {
        self.Add(self[0]);
    }
    CHECK(N(self[20]) == 7);

    CPtrList list;
    CHECK(throws([&] { p = list.RemoveHead(); }));
    CList<int, int> typed;
    CHECK(throws([&] { n = typed.RemoveHead(); }));
    CHECK(throws([&] { n = typed.GetHead(); }));

    CMapStringToString map;
    CString value;
    LPCSTR null = NULL;
    CHECK(throws([&] { n = map.Lookup(null, value); }));
    (void)p;
    (void)n;
}

TEST_CASE("CMapStringToString")
{
    CMapStringToString map;
    CString value;
    CHECK(map.GetCount() == 0);
    CHECK(map.IsEmpty());
    CHECK(map.GetStartPosition() == NULL);
    CHECK_FALSE(map.Lookup("missing", value));
    CHECK_FALSE(map.RemoveKey("missing"));

    map.SetAt("CHARNAME", "Imoen");
    map["CLASS"] = "Thief";
    map.SetAt(CString("LEVEL"), CString("3"));
    CHECK(map.GetCount() == 3);
    CHECK(map.GetStartPosition() != NULL);

    CHECK(map.Lookup("CHARNAME", value));
    CHECK(value == "Imoen");
    CHECK(map.Lookup(CString("CLASS"), value));
    CHECK(value == "Thief");

    // Keys are case-sensitive.
    value = "untouched";
    CHECK_FALSE(map.Lookup("charname", value));
    CHECK(value == "untouched");

    // Setting an existing key replaces the value.
    map.SetAt("CHARNAME", "Minsc");
    CHECK(map.GetCount() == 3);
    CHECK(map.Lookup("CHARNAME", value));
    CHECK(value == "Minsc");

    // operator[] adds a missing key with an empty value.
    CHECK(map["NEW"].IsEmpty());
    CHECK(map.GetCount() == 4);

    CHECK(map.RemoveKey("CLASS"));
    CHECK_FALSE(map.RemoveKey("CLASS"));
    CHECK_FALSE(map.Lookup("CLASS", value));
    CHECK(map.GetCount() == 3);

    map.RemoveAll();
    CHECK(map.GetCount() == 0);
    CHECK(map.GetStartPosition() == NULL);
    CHECK_FALSE(map.Lookup("CHARNAME", value));
    map.SetAt("again", "works");
    CHECK(map.Lookup("again", value));
}

TEST_CASE("CMapStringToString iteration")
{
    const char* const keys[] = {
        "CHARNAME", "CLASS", "LEVEL", "RACE", "GABBER", "PLAYER1", "PLAYER2", "DAY", "MONTH", "YEAR",
        "HOUR", "MINUTE", "DURATION", "GAMEDAY", "GAMEDAYS", "DAYANDMONTH", "MONTHNAME", "SIRMAAM",
        "HESHE", "HISHER", "HIMHER", "MANWOMAN", "LADYLORD", "BROTHERSISTER", "GIRLBOY", "PRO_RACE",
        "TM", "MAGESCHOOL", "FIGHTERTYPE", "EXPERIENCEAMOUNT", "a", "b", "c", "", "A somewhat longer key with spaces"
    };
    const int count = sizeof(keys) / sizeof(keys[0]);

    CMapStringToString map;
    for (int i = 0; i < count; i++) {
        CString value;
        value.Format("%d", i);
        map.SetAt(keys[i], value);
    }
    CHECK(map.GetCount() == count);

    // Every entry is visited exactly once.
    std::string order;
    std::vector<bool> seen(count, false);
    int visited = 0;
    POSITION pos = map.GetStartPosition();
    while (pos != NULL) {
        CString key;
        CString value;
        map.GetNextAssoc(pos, key, value);
        int index = atoi(value);
        REQUIRE(index >= 0);
        REQUIRE(index < count);
        CHECK(key == keys[index]);
        CHECK_FALSE(seen[index]);
        seen[index] = true;
        visited++;
        order += value;
        order += ' ';
    }
    CHECK(visited == count);

    // The order of iteration is fixed by the hash function and bucket
    // count; it is the same in MFC and in the replacement.
    CHECK(order == "33 29 23 8 1 19 16 14 12 10 24 15 7 6 18 5 28 13 11 2 20 9 32 3 31 22 34 30 27 4 0 26 25 21 17 ");

    // Characters above 0x7F take part in the order too.
    CMapStringToString high;
    high.SetAt("\x80", "0");
    high.SetAt("\xFF", "1");
    high.SetAt("a", "2");
    high.SetAt("\xE9t\xE9", "3");
    order.clear();
    pos = high.GetStartPosition();
    while (pos != NULL) {
        CString key;
        CString value;
        high.GetNextAssoc(pos, key, value);
        order += value;
        order += ' ';
    }
    CHECK(order == "3 1 2 0 ");

    // Removing the entry just visited is safe (CTlkTable does this).
    pos = map.GetStartPosition();
    while (pos != NULL) {
        CString key;
        CString value;
        map.GetNextAssoc(pos, key, value);
        CHECK(map.RemoveKey(key));
    }
    CHECK(map.GetCount() == 0);
    CHECK(map.GetStartPosition() == NULL);
}

TEST_CASE("CMapWordToPtr and CTypedPtrMap")
{
    int a = 1, b = 2, c = 3;
    CTypedPtrMap<CMapWordToPtr, USHORT, int*> map;
    int* p = &c;

    CHECK_FALSE(map.Lookup(5, p));
    CHECK(p == &c);

    map[0x0100] = &a;
    map.SetAt(0x0201, &b);
    map[7] = &c;
    CHECK(map.GetCount() == 3);
    CHECK(map.Lookup(0x0100, p));
    CHECK(p == &a);
    CHECK(map.Lookup(0x0201, p));
    CHECK(p == &b);
    CHECK(map.Lookup(7, p));
    CHECK(p == &c);
    CHECK_FALSE(map.Lookup(8, p));

    map[7] = &a;
    CHECK(map.GetCount() == 3);
    CHECK(map.Lookup(7, p));
    CHECK(p == &a);

    CHECK(map.RemoveKey(7));
    CHECK_FALSE(map.RemoveKey(7));
    CHECK(map.GetCount() == 2);

    // Iteration order for a spread of keys.
    CMapWordToPtr plain;
    for (int i = 0; i < 60; i++) {
        WORD key = static_cast<WORD>(i * 37 + (i % 5) * 256);
        plain.SetAt(key, P(i));
    }
    CHECK(plain.GetCount() == 60);

    std::string order;
    int visited = 0;
    POSITION pos = plain.GetStartPosition();
    while (pos != NULL) {
        WORD key;
        void* value;
        plain.GetNextAssoc(pos, key, value);
        int i = N(value);
        CHECK(key == static_cast<WORD>(i * 37 + (i % 5) * 256));
        visited++;
        order += std::to_string(i);
        order += ' ';
    }
    CHECK(visited == 60);
    CHECK(order == "44 33 22 11 0 50 9 59 48 37 26 15 24 13 2 52 41 30 39 28 17 6 56 45 4 54 43 32 21 10 19 8 58 47 36 25 34 23 12 1 51 40 49 38 27 16 5 55 14 3 53 42 31 20 29 18 7 57 46 35 ");

    // Removing while iterating, as CTlkTable does on shutdown.
    pos = map.GetStartPosition();
    while (pos != NULL) {
        USHORT key;
        int* value;
        map.GetNextAssoc(pos, key, value);
        CHECK(value != NULL);
        map.RemoveKey(key);
    }
    CHECK(map.GetCount() == 0);

    void* found = NULL;
    CHECK(plain.Lookup(37 + 256, found));
    CHECK(N(found) == 1);
    plain.RemoveAll();
    CHECK(plain.IsEmpty());
}
