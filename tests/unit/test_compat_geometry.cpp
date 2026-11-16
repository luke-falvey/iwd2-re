// CPoint, CSize and CRect behaviour the game relies on. Runs against MFC in
// the default build and against src/compat in the IWD2RE_NO_MFC build.

#include "test.h"

#include <new>
#include <vector>

namespace {

bool Same(const RECT& a, const RECT& b)
{
    return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
}

// Rectangles of every kind: normal, touching, nested, empty and inverted.
std::vector<CRect> SampleRects()
{
    std::vector<CRect> v;
    const int coords[] = { -20, -5, 0, 5, 10, 30 };
    for (int l : coords) {
        for (int r : coords) {
            v.push_back(CRect(l, 0, r, 10));
            v.push_back(CRect(l, -5, r, 5));
            v.push_back(CRect(0, l, 10, r));
            v.push_back(CRect(l, l, r, r));
            v.push_back(CRect(l, r, r, l));
        }
    }
    return v;
}

} // namespace

TEST_CASE("geometry types are the plain structures with no extra data")
{
    CHECK(sizeof(CPoint) == sizeof(POINT));
    CHECK(sizeof(CSize) == sizeof(SIZE));
    CHECK(sizeof(CRect) == sizeof(RECT));

    CRect r(1, 2, 3, 4);
    RECT* pr = &r;
    CHECK(pr->left == 1);
    CHECK(pr->bottom == 4);
    LPRECT lp = r;
    CHECK(lp == &r);
    LPCRECT lpc = r;
    CHECK(lpc == &r);

    CPoint p(5, 6);
    POINT* pp = &p;
    CHECK(pp->x == 5);
    CHECK(pp->y == 6);
}

TEST_CASE("default-constructed geometry is zero")
{
    unsigned char raw[sizeof(CRect)];

    memset(raw, 0xCC, sizeof(raw));
    CPoint* p = new (raw) CPoint;
    CHECK(p->x == 0);
    CHECK(p->y == 0);

    memset(raw, 0xCC, sizeof(raw));
    CSize* s = new (raw) CSize;
    CHECK(s->cx == 0);
    CHECK(s->cy == 0);

    memset(raw, 0xCC, sizeof(raw));
    CRect* r = new (raw) CRect;
    CHECK(r->left == 0);
    CHECK(r->top == 0);
    CHECK(r->right == 0);
    CHECK(r->bottom == 0);
}

TEST_CASE("CPoint")
{
    CPoint p(3, 4);
    CHECK(p.x == 3);
    CHECK(p.y == 4);

    POINT raw = { 7, 8 };
    CPoint fromPoint(raw);
    CHECK(fromPoint.x == 7);
    CHECK(fromPoint.y == 8);

    SIZE rawSize = { 9, 10 };
    CPoint fromSize(rawSize);
    CHECK(fromSize.x == 9);
    CHECK(fromSize.y == 10);

    // From a packed message parameter: two signed 16-bit halves.
    CPoint fromParam(static_cast<LPARAM>(MAKELONG(100, 200)));
    CHECK(fromParam.x == 100);
    CHECK(fromParam.y == 200);
    CPoint negParam(static_cast<LPARAM>(MAKELONG(static_cast<WORD>(-3), static_cast<WORD>(-4))));
    CHECK(negParam.x == -3);
    CHECK(negParam.y == -4);

    CHECK(p == CPoint(3, 4));
    CHECK(p != CPoint(3, 5));
    CHECK(p != CPoint(2, 4));
    CHECK_FALSE(p == CPoint(4, 3));
    CHECK_FALSE(p != CPoint(3, 4));

    p.Offset(1, -1);
    CHECK(p == CPoint(4, 3));
    p.Offset(CPoint(10, 10));
    CHECK(p == CPoint(14, 13));
    p.Offset(CSize(-4, -3));
    CHECK(p == CPoint(10, 10));

    p += CSize(1, 2);
    CHECK(p == CPoint(11, 12));
    p -= CSize(1, 2);
    CHECK(p == CPoint(10, 10));
    p += CPoint(5, 6);
    CHECK(p == CPoint(15, 16));
    p -= CPoint(5, 6);
    CHECK(p == CPoint(10, 10));

    CHECK(p + CSize(1, 2) == CPoint(11, 12));
    CHECK(p - CSize(1, 2) == CPoint(9, 8));
    CHECK(p + CPoint(1, 2) == CPoint(11, 12));
    CHECK(-p == CPoint(-10, -10));

    CSize diff = p - CPoint(3, 4);
    CHECK(diff.cx == 7);
    CHECK(diff.cy == 6);

    CRect moved = p + CRect(0, 0, 5, 5);
    CHECK(Same(moved, CRect(10, 10, 15, 15)));
    CRect movedBack = p - CRect(10, 10, 15, 15);
    CHECK(Same(movedBack, CRect(0, 0, 5, 5)));

    p.SetPoint(-1, -2);
    CHECK(p.x == -1);
    CHECK(p.y == -2);

    // Copy and assignment.
    CPoint q = p;
    q.x = 50;
    CHECK(p.x == -1);
    q = raw;
    CHECK(q == CPoint(7, 8));
}

TEST_CASE("CSize")
{
    CSize s(3, 4);
    CHECK(s.cx == 3);
    CHECK(s.cy == 4);

    SIZE raw = { 7, 8 };
    CHECK(CSize(raw) == CSize(7, 8));
    POINT rawPoint = { 9, 10 };
    CHECK(CSize(rawPoint) == CSize(9, 10));
    CSize packed(static_cast<DWORD>(MAKELONG(100, 200)));
    CHECK(packed == CSize(100, 200));

    CHECK(s == CSize(3, 4));
    CHECK(s != CSize(3, 5));
    CHECK_FALSE(s == CSize(4, 4));

    s += CSize(1, 1);
    CHECK(s == CSize(4, 5));
    s -= CSize(2, 2);
    CHECK(s == CSize(2, 3));
    s.SetSize(10, 20);
    CHECK(s == CSize(10, 20));

    CHECK(s + CSize(1, 2) == CSize(11, 22));
    CHECK(s - CSize(1, 2) == CSize(9, 18));
    CHECK(-s == CSize(-10, -20));
    CHECK(s + CPoint(1, 2) == CPoint(11, 22));
    CHECK(s - CPoint(1, 2) == CPoint(9, 18));

    CRect moved = s + CRect(0, 0, 5, 5);
    CHECK(Same(moved, CRect(10, 20, 15, 25)));
    CRect movedBack = s - CRect(10, 20, 15, 25);
    CHECK(Same(movedBack, CRect(0, 0, 5, 5)));
}

TEST_CASE("CRect construction and measures")
{
    CRect r(10, 20, 110, 70);
    CHECK(r.left == 10);
    CHECK(r.top == 20);
    CHECK(r.right == 110);
    CHECK(r.bottom == 70);
    CHECK(r.Width() == 100);
    CHECK(r.Height() == 50);
    CHECK(r.Size() == CSize(100, 50));
    CHECK(r.TopLeft() == CPoint(10, 20));
    CHECK(r.BottomRight() == CPoint(110, 70));
    CHECK(r.CenterPoint() == CPoint(60, 45));

    RECT raw = { 1, 2, 3, 4 };
    CHECK(Same(CRect(raw), raw));
    CHECK(Same(CRect(&raw), raw));
    CHECK(Same(CRect(CPoint(5, 6), CSize(10, 20)), CRect(5, 6, 15, 26)));
    CHECK(Same(CRect(CPoint(5, 6), CPoint(10, 20)), CRect(5, 6, 10, 20)));

    // Negative extents are reported as they are.
    CRect inverted(10, 10, 0, 0);
    CHECK(inverted.Width() == -10);
    CHECK(inverted.Height() == -10);
    CHECK(CRect(0, 0, 5, 5).CenterPoint() == CPoint(2, 2));
    CHECK(CRect(-5, -5, 0, 0).CenterPoint() == CPoint(-2, -2));

    // TopLeft and BottomRight are views of the rectangle's own coordinates.
    r.TopLeft().x = 0;
    r.BottomRight().y = 100;
    CHECK(Same(r, CRect(0, 20, 110, 100)));
    r.TopLeft() = CPoint(1, 2);
    CHECK(r.left == 1);
    CHECK(r.top == 2);

    const CRect cr(1, 2, 3, 4);
    CHECK(cr.TopLeft() == CPoint(1, 2));
    CHECK(cr.BottomRight() == CPoint(3, 4));
}

TEST_CASE("CRect set, copy, compare")
{
    CRect r;
    r.SetRect(1, 2, 3, 4);
    CHECK(Same(r, CRect(1, 2, 3, 4)));
    r.SetRect(CPoint(5, 6), CPoint(7, 8));
    CHECK(Same(r, CRect(5, 6, 7, 8)));

    CRect copy;
    copy.CopyRect(&r);
    CHECK(Same(copy, r));
    CHECK(copy.EqualRect(&r));
    CHECK(copy == r);
    CHECK_FALSE(copy != r);
    copy.left++;
    CHECK_FALSE(copy.EqualRect(&r));
    CHECK(copy != r);
    CHECK_FALSE(copy == r);

    RECT raw = { 9, 9, 9, 9 };
    copy = raw;
    CHECK(Same(copy, raw));

    r.SetRectEmpty();
    CHECK(Same(r, CRect(0, 0, 0, 0)));
}

TEST_CASE("CRect IsRectEmpty and IsRectNull")
{
    CHECK(CRect(0, 0, 0, 0).IsRectEmpty());
    CHECK(CRect(0, 0, 0, 0).IsRectNull());
    CHECK(CRect(5, 5, 5, 5).IsRectEmpty());
    CHECK_FALSE(CRect(5, 5, 5, 5).IsRectNull());
    CHECK(CRect(0, 0, 10, 0).IsRectEmpty());
    CHECK(CRect(0, 0, 0, 10).IsRectEmpty());
    CHECK(CRect(10, 0, 0, 10).IsRectEmpty());
    CHECK(CRect(0, 10, 10, 0).IsRectEmpty());
    CHECK_FALSE(CRect(0, 0, 1, 1).IsRectEmpty());
    CHECK_FALSE(CRect(-10, -10, -9, -9).IsRectEmpty());
    CHECK_FALSE(CRect(0, 0, 1, 1).IsRectNull());
    CHECK_FALSE(CRect(0, 0, 0, 1).IsRectNull());

    for (const CRect& r : SampleRects()) {
        CHECK((r.IsRectEmpty() != FALSE) == (::IsRectEmpty(&r) != FALSE));
    }
}

TEST_CASE("CRect PtInRect excludes the right and bottom edges")
{
    CRect r(10, 20, 30, 40);
    CHECK(r.PtInRect(CPoint(10, 20)));
    CHECK(r.PtInRect(CPoint(29, 39)));
    CHECK(r.PtInRect(CPoint(15, 25)));
    CHECK_FALSE(r.PtInRect(CPoint(30, 25)));
    CHECK_FALSE(r.PtInRect(CPoint(15, 40)));
    CHECK_FALSE(r.PtInRect(CPoint(30, 40)));
    CHECK_FALSE(r.PtInRect(CPoint(9, 25)));
    CHECK_FALSE(r.PtInRect(CPoint(15, 19)));

    // An inverted rectangle contains nothing.
    CRect inverted(30, 40, 10, 20);
    CHECK_FALSE(inverted.PtInRect(CPoint(15, 25)));
    CHECK_FALSE(CRect(0, 0, 0, 0).PtInRect(CPoint(0, 0)));

    const int coords[] = { -21, -20, -6, -5, -1, 0, 4, 5, 9, 10, 29, 30, 31 };
    for (const CRect& rect : SampleRects()) {
        for (int x : coords) {
            for (int y : coords) {
                POINT pt = { x, y };
                REQUIRE((rect.PtInRect(pt) != FALSE) == (::PtInRect(&rect, pt) != FALSE));
            }
        }
    }
}

TEST_CASE("CRect InflateRect, DeflateRect, OffsetRect")
{
    CRect r(10, 10, 20, 20);
    r.InflateRect(1, 2);
    CHECK(Same(r, CRect(9, 8, 21, 22)));
    r.InflateRect(-1, -2);
    CHECK(Same(r, CRect(10, 10, 20, 20)));
    r.InflateRect(CSize(5, 5));
    CHECK(Same(r, CRect(5, 5, 25, 25)));
    r.InflateRect(1, 2, 3, 4);
    CHECK(Same(r, CRect(4, 3, 28, 29)));
    r.InflateRect(CRect(1, 1, 1, 1));
    CHECK(Same(r, CRect(3, 2, 29, 30)));

    r.DeflateRect(CRect(1, 1, 1, 1));
    CHECK(Same(r, CRect(4, 3, 28, 29)));
    r.DeflateRect(1, 2, 3, 4);
    CHECK(Same(r, CRect(5, 5, 25, 25)));
    r.DeflateRect(CSize(5, 5));
    CHECK(Same(r, CRect(10, 10, 20, 20)));
    r.DeflateRect(1, 2);
    CHECK(Same(r, CRect(11, 12, 19, 18)));
    // Deflating past zero inverts the rectangle.
    r.DeflateRect(10, 10);
    CHECK(Same(r, CRect(21, 22, 9, 8)));

    CRect o(0, 0, 10, 10);
    o.OffsetRect(5, -5);
    CHECK(Same(o, CRect(5, -5, 15, 5)));
    o.OffsetRect(CPoint(-5, 5));
    CHECK(Same(o, CRect(0, 0, 10, 10)));
    o.OffsetRect(CSize(100, 200));
    CHECK(Same(o, CRect(100, 200, 110, 210)));

    o.MoveToXY(1, 2);
    CHECK(Same(o, CRect(1, 2, 11, 12)));
    o.MoveToX(5);
    CHECK(Same(o, CRect(5, 2, 15, 12)));
    o.MoveToY(7);
    CHECK(Same(o, CRect(5, 7, 15, 17)));
}

TEST_CASE("CRect NormalizeRect")
{
    CRect r(10, 20, 0, 5);
    r.NormalizeRect();
    CHECK(Same(r, CRect(0, 5, 10, 20)));
    r.NormalizeRect();
    CHECK(Same(r, CRect(0, 5, 10, 20)));

    CRect onlyX(10, 0, 0, 5);
    onlyX.NormalizeRect();
    CHECK(Same(onlyX, CRect(0, 0, 10, 5)));

    CRect onlyY(0, 5, 10, 0);
    onlyY.NormalizeRect();
    CHECK(Same(onlyY, CRect(0, 0, 10, 5)));

    CRect flat(3, 3, 3, 3);
    flat.NormalizeRect();
    CHECK(Same(flat, CRect(3, 3, 3, 3)));
}

TEST_CASE("CRect IntersectRect")
{
    CRect a(0, 0, 10, 10);
    CRect b(5, 5, 15, 15);
    CRect r;

    CHECK(r.IntersectRect(a, b));
    CHECK(Same(r, CRect(5, 5, 10, 10)));
    CHECK(r.IntersectRect(&b, &a));
    CHECK(Same(r, CRect(5, 5, 10, 10)));

    // Contained.
    CHECK(r.IntersectRect(a, CRect(2, 2, 4, 4)));
    CHECK(Same(r, CRect(2, 2, 4, 4)));

    // Touching edges and disjoint: no overlap, result is all zeros.
    r.SetRect(1, 1, 1, 1);
    CHECK_FALSE(r.IntersectRect(a, CRect(10, 0, 20, 10)));
    CHECK(Same(r, CRect(0, 0, 0, 0)));
    r.SetRect(1, 1, 1, 1);
    CHECK_FALSE(r.IntersectRect(a, CRect(0, 10, 10, 20)));
    CHECK(Same(r, CRect(0, 0, 0, 0)));
    r.SetRect(1, 1, 1, 1);
    CHECK_FALSE(r.IntersectRect(a, CRect(50, 50, 60, 60)));
    CHECK(Same(r, CRect(0, 0, 0, 0)));

    // Empty sources.
    r.SetRect(1, 1, 1, 1);
    CHECK_FALSE(r.IntersectRect(a, CRect(0, 0, 0, 0)));
    CHECK(Same(r, CRect(0, 0, 0, 0)));
    CHECK_FALSE(r.IntersectRect(CRect(5, 5, 5, 5), a));
    CHECK(Same(r, CRect(0, 0, 0, 0)));

    // The destination may be one of the sources (very common in the game).
    CRect self(0, 0, 10, 10);
    CHECK(self.IntersectRect(self, CRect(3, 4, 20, 20)));
    CHECK(Same(self, CRect(3, 4, 10, 10)));
    CHECK(self.IntersectRect(CRect(0, 0, 5, 6), self));
    CHECK(Same(self, CRect(3, 4, 5, 6)));
    CHECK_FALSE(self.IntersectRect(self, CRect(100, 100, 200, 200)));
    CHECK(Same(self, CRect(0, 0, 0, 0)));

    CRect viaOp = a & b;
    CHECK(Same(viaOp, CRect(5, 5, 10, 10)));
    CRect inPlace(a);
    inPlace &= b;
    CHECK(Same(inPlace, CRect(5, 5, 10, 10)));
    inPlace &= CRect(100, 100, 101, 101);
    CHECK(Same(inPlace, CRect(0, 0, 0, 0)));

    // Same results as the operating system's function for every pairing.
    std::vector<CRect> rects = SampleRects();
    for (const CRect& x : rects) {
        for (const CRect& y : rects) {
            CRect mine(77, 77, 77, 77);
            RECT theirs = { 77, 77, 77, 77 };
            BOOL mineResult = mine.IntersectRect(&x, &y);
            BOOL theirResult = ::IntersectRect(&theirs, &x, &y);
            REQUIRE((mineResult != FALSE) == (theirResult != FALSE));
            REQUIRE(Same(mine, theirs));
        }
    }
}

TEST_CASE("CRect UnionRect")
{
    CRect a(0, 0, 10, 10);
    CRect b(5, 5, 15, 20);
    CRect r;

    CHECK(r.UnionRect(a, b));
    CHECK(Same(r, CRect(0, 0, 15, 20)));
    CHECK(r.UnionRect(a, CRect(50, 50, 60, 60)));
    CHECK(Same(r, CRect(0, 0, 60, 60)));

    // An empty source is ignored.
    CHECK(r.UnionRect(a, CRect(100, 100, 100, 100)));
    CHECK(Same(r, a));
    CHECK(r.UnionRect(CRect(0, 0, 0, 0), b));
    CHECK(Same(r, b));
    r.SetRect(1, 1, 2, 2);
    CHECK_FALSE(r.UnionRect(CRect(3, 3, 3, 3), CRect(9, 9, 1, 1)));
    CHECK(Same(r, CRect(0, 0, 0, 0)));

    CRect self(0, 0, 10, 10);
    CHECK(self.UnionRect(self, CRect(-5, 2, 3, 30)));
    CHECK(Same(self, CRect(-5, 0, 10, 30)));

    CRect viaOp = a | b;
    CHECK(Same(viaOp, CRect(0, 0, 15, 20)));
    CRect inPlace(a);
    inPlace |= b;
    CHECK(Same(inPlace, CRect(0, 0, 15, 20)));

    std::vector<CRect> rects = SampleRects();
    for (const CRect& x : rects) {
        for (const CRect& y : rects) {
            CRect mine(77, 77, 77, 77);
            RECT theirs = { 77, 77, 77, 77 };
            BOOL mineResult = mine.UnionRect(&x, &y);
            BOOL theirResult = ::UnionRect(&theirs, &x, &y);
            REQUIRE((mineResult != FALSE) == (theirResult != FALSE));
            REQUIRE(Same(mine, theirs));
        }
    }
}

TEST_CASE("CRect SubtractRect")
{
    CRect a(0, 0, 10, 10);
    CRect r;

    // Cut from the left, right, top, bottom.
    CHECK(r.SubtractRect(a, CRect(0, 0, 4, 10)));
    CHECK(Same(r, CRect(4, 0, 10, 10)));
    CHECK(r.SubtractRect(a, CRect(6, -5, 20, 20)));
    CHECK(Same(r, CRect(0, 0, 6, 10)));
    CHECK(r.SubtractRect(a, CRect(-1, -1, 11, 3)));
    CHECK(Same(r, CRect(0, 3, 10, 10)));
    CHECK(r.SubtractRect(a, CRect(0, 7, 10, 10)));
    CHECK(Same(r, CRect(0, 0, 10, 7)));

    // A partial cut leaves the rectangle whole.
    CHECK(r.SubtractRect(a, CRect(0, 0, 5, 5)));
    CHECK(Same(r, a));
    // No overlap.
    CHECK(r.SubtractRect(a, CRect(20, 20, 30, 30)));
    CHECK(Same(r, a));
    // Fully covered.
    CHECK_FALSE(r.SubtractRect(a, CRect(-1, -1, 11, 11)));
    CHECK(Same(r, CRect(0, 0, 0, 0)));
    // An empty rectangle has nothing to cut from; it is copied as it is.
    CHECK_FALSE(r.SubtractRect(CRect(5, 5, 5, 5), a));
    CHECK(Same(r, CRect(5, 5, 5, 5)));
    CHECK_FALSE(r.SubtractRect(CRect(10, 10, 0, 0), a));
    CHECK(Same(r, CRect(10, 10, 0, 0)));

    std::vector<CRect> rects = SampleRects();
    for (const CRect& x : rects) {
        for (const CRect& y : rects) {
            CRect mine(77, 77, 77, 77);
            RECT theirs = { 77, 77, 77, 77 };
            BOOL mineResult = mine.SubtractRect(&x, &y);
            BOOL theirResult = ::SubtractRect(&theirs, &x, &y);
            REQUIRE((mineResult != FALSE) == (theirResult != FALSE));
            REQUIRE(Same(mine, theirs));
        }
    }
}

TEST_CASE("CRect operators")
{
    CRect r(0, 0, 10, 10);

    r += CPoint(5, 5);
    CHECK(Same(r, CRect(5, 5, 15, 15)));
    r -= CPoint(5, 5);
    CHECK(Same(r, CRect(0, 0, 10, 10)));
    r += CSize(1, 2);
    CHECK(Same(r, CRect(1, 2, 11, 12)));
    r -= CSize(1, 2);
    CHECK(Same(r, CRect(0, 0, 10, 10)));

    CRect margins(1, 2, 3, 4);
    r += &margins;
    CHECK(Same(r, CRect(-1, -2, 13, 14)));
    r -= &margins;
    CHECK(Same(r, CRect(0, 0, 10, 10)));

    CHECK(Same(r + CPoint(1, 1), CRect(1, 1, 11, 11)));
    CHECK(Same(r - CPoint(1, 1), CRect(-1, -1, 9, 9)));
    CHECK(Same(r + CSize(2, 3), CRect(2, 3, 12, 13)));
    CHECK(Same(r - CSize(2, 3), CRect(-2, -3, 8, 7)));
    LPCRECT pMargins = &margins;
    CHECK(Same(r + pMargins, CRect(-1, -2, 13, 14)));
    CHECK(Same(r - pMargins, CRect(1, 2, 7, 6)));
    CHECK(Same(r, CRect(0, 0, 10, 10)));
}
