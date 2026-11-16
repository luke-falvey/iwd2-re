#ifndef IWD2RE_COMPAT_GEOMETRY_H_
#define IWD2RE_COMPAT_GEOMETRY_H_

// Replacements for MFC's `CPoint`, `CSize` and `CRect`. They derive from the
// plain `POINT`, `SIZE` and `RECT` structures and add no data, so they can be
// passed wherever those are expected. All arithmetic is done here; nothing
// calls the operating system.

class CSize;
class CPoint;
class CRect;

class CSize : public tagSIZE {
public:
    CSize() { cx = 0; cy = 0; }
    CSize(int initCX, int initCY) { cx = initCX; cy = initCY; }
    CSize(SIZE initSize) { cx = initSize.cx; cy = initSize.cy; }
    CSize(POINT initPt) { cx = initPt.x; cy = initPt.y; }
    CSize(DWORD dwSize) { cx = static_cast<short>(dwSize & 0xFFFF); cy = static_cast<short>((dwSize >> 16) & 0xFFFF); }

    BOOL operator==(SIZE size) const { return cx == size.cx && cy == size.cy; }
    BOOL operator!=(SIZE size) const { return cx != size.cx || cy != size.cy; }
    void operator+=(SIZE size) { cx += size.cx; cy += size.cy; }
    void operator-=(SIZE size) { cx -= size.cx; cy -= size.cy; }
    void SetSize(int CX, int CY) { cx = CX; cy = CY; }

    CSize operator+(SIZE size) const { return CSize(cx + size.cx, cy + size.cy); }
    CSize operator-(SIZE size) const { return CSize(cx - size.cx, cy - size.cy); }
    CSize operator-() const { return CSize(-cx, -cy); }

    inline CPoint operator+(POINT point) const;
    inline CPoint operator-(POINT point) const;
    inline CRect operator+(const RECT* lpRect) const;
    inline CRect operator-(const RECT* lpRect) const;
};

class CPoint : public tagPOINT {
public:
    CPoint() { x = 0; y = 0; }
    CPoint(int initX, int initY) { x = initX; y = initY; }
    CPoint(POINT initPt) { x = initPt.x; y = initPt.y; }
    CPoint(SIZE initSize) { x = initSize.cx; y = initSize.cy; }
    CPoint(LPARAM dwPoint) { x = static_cast<short>(dwPoint & 0xFFFF); y = static_cast<short>((dwPoint >> 16) & 0xFFFF); }

    void Offset(int xOffset, int yOffset) { x += xOffset; y += yOffset; }
    void Offset(POINT point) { x += point.x; y += point.y; }
    void Offset(SIZE size) { x += size.cx; y += size.cy; }
    void SetPoint(int X, int Y) { x = X; y = Y; }

    BOOL operator==(POINT point) const { return x == point.x && y == point.y; }
    BOOL operator!=(POINT point) const { return x != point.x || y != point.y; }
    void operator+=(SIZE size) { x += size.cx; y += size.cy; }
    void operator-=(SIZE size) { x -= size.cx; y -= size.cy; }
    void operator+=(POINT point) { x += point.x; y += point.y; }
    void operator-=(POINT point) { x -= point.x; y -= point.y; }

    CPoint operator+(SIZE size) const { return CPoint(x + size.cx, y + size.cy); }
    CPoint operator-(SIZE size) const { return CPoint(x - size.cx, y - size.cy); }
    CPoint operator-() const { return CPoint(-x, -y); }
    CPoint operator+(POINT point) const { return CPoint(x + point.x, y + point.y); }
    CSize operator-(POINT point) const { return CSize(x - point.x, y - point.y); }

    inline CRect operator+(const RECT* lpRect) const;
    inline CRect operator-(const RECT* lpRect) const;
};

class CRect : public tagRECT {
public:
    CRect() { left = 0; top = 0; right = 0; bottom = 0; }
    CRect(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
    CRect(const RECT& srcRect) { CopyRect(&srcRect); }
    CRect(const RECT* lpSrcRect) { CopyRect(lpSrcRect); }
    CRect(POINT point, SIZE size)
    {
        left = point.x;
        top = point.y;
        right = left + size.cx;
        bottom = top + size.cy;
    }
    CRect(POINT topLeft, POINT bottomRight)
    {
        left = topLeft.x;
        top = topLeft.y;
        right = bottomRight.x;
        bottom = bottomRight.y;
    }

    int Width() const { return right - left; }
    int Height() const { return bottom - top; }
    CSize Size() const { return CSize(right - left, bottom - top); }
    CPoint& TopLeft() { return *(reinterpret_cast<CPoint*>(this)); }
    CPoint& BottomRight() { return *(reinterpret_cast<CPoint*>(this) + 1); }
    const CPoint& TopLeft() const { return *(reinterpret_cast<const CPoint*>(this)); }
    const CPoint& BottomRight() const { return *(reinterpret_cast<const CPoint*>(this) + 1); }
    CPoint CenterPoint() const { return CPoint((left + right) / 2, (top + bottom) / 2); }

    void SwapLeftRight() { SwapLeftRight(this); }
    static void SwapLeftRight(RECT* lpRect)
    {
        LONG temp = lpRect->left;
        lpRect->left = lpRect->right;
        lpRect->right = temp;
    }

    operator RECT*() { return this; }
    operator const RECT*() const { return this; }

    // Empty: no area (zero or negative width or height).
    BOOL IsRectEmpty() const { return IsEmpty(this); }
    // Null: all four coordinates are zero.
    BOOL IsRectNull() const { return left == 0 && right == 0 && top == 0 && bottom == 0; }
    // The right and bottom edges are outside the rectangle.
    BOOL PtInRect(POINT point) const { return point.x >= left && point.x < right && point.y >= top && point.y < bottom; }

    void SetRect(int x1, int y1, int x2, int y2) { left = x1; top = y1; right = x2; bottom = y2; }
    void SetRect(POINT topLeft, POINT bottomRight) { SetRect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y); }
    void SetRectEmpty() { left = 0; top = 0; right = 0; bottom = 0; }
    void CopyRect(const RECT* lpSrcRect)
    {
        left = lpSrcRect->left;
        top = lpSrcRect->top;
        right = lpSrcRect->right;
        bottom = lpSrcRect->bottom;
    }
    BOOL EqualRect(const RECT* lpRect) const
    {
        return left == lpRect->left && top == lpRect->top && right == lpRect->right && bottom == lpRect->bottom;
    }

    void InflateRect(int x, int y) { left -= x; top -= y; right += x; bottom += y; }
    void InflateRect(SIZE size) { InflateRect(size.cx, size.cy); }
    void InflateRect(const RECT* lpRect) { InflateRect(lpRect->left, lpRect->top, lpRect->right, lpRect->bottom); }
    void InflateRect(int l, int t, int r, int b) { left -= l; top -= t; right += r; bottom += b; }

    void DeflateRect(int x, int y) { InflateRect(-x, -y); }
    void DeflateRect(SIZE size) { InflateRect(-size.cx, -size.cy); }
    void DeflateRect(const RECT* lpRect) { DeflateRect(lpRect->left, lpRect->top, lpRect->right, lpRect->bottom); }
    void DeflateRect(int l, int t, int r, int b) { left += l; top += t; right -= r; bottom -= b; }

    void OffsetRect(int x, int y) { left += x; top += y; right += x; bottom += y; }
    void OffsetRect(SIZE size) { OffsetRect(size.cx, size.cy); }
    void OffsetRect(POINT point) { OffsetRect(point.x, point.y); }

    // Makes width and height positive by swapping edges.
    void NormalizeRect()
    {
        LONG temp;
        if (left > right) {
            temp = left;
            left = right;
            right = temp;
        }
        if (top > bottom) {
            temp = top;
            top = bottom;
            bottom = temp;
        }
    }

    void MoveToY(int y) { bottom = Height() + y; top = y; }
    void MoveToX(int x) { right = Width() + x; left = x; }
    void MoveToXY(int x, int y) { MoveToX(x); MoveToY(y); }
    void MoveToXY(POINT point) { MoveToX(point.x); MoveToY(point.y); }

    // This rectangle becomes the overlap of the two. Without overlap it is
    // set to all zeros and the result is FALSE.
    BOOL IntersectRect(const RECT* lpRect1, const RECT* lpRect2)
    {
        LONG l = lpRect1->left > lpRect2->left ? lpRect1->left : lpRect2->left;
        LONG r = lpRect1->right < lpRect2->right ? lpRect1->right : lpRect2->right;
        LONG t = lpRect1->top > lpRect2->top ? lpRect1->top : lpRect2->top;
        LONG b = lpRect1->bottom < lpRect2->bottom ? lpRect1->bottom : lpRect2->bottom;
        if (IsEmpty(lpRect1) || IsEmpty(lpRect2) || l >= r || t >= b) {
            SetRectEmpty();
            return FALSE;
        }
        SetRect(l, t, r, b);
        return TRUE;
    }

    // This rectangle becomes the smallest one holding both. An empty source
    // is ignored; with both empty it is set to all zeros and the result is
    // FALSE.
    BOOL UnionRect(const RECT* lpRect1, const RECT* lpRect2)
    {
        BOOL bEmpty1 = IsEmpty(lpRect1);
        BOOL bEmpty2 = IsEmpty(lpRect2);
        if (bEmpty1 && bEmpty2) {
            SetRectEmpty();
            return FALSE;
        }
        if (bEmpty1) {
            RECT r = *lpRect2;
            CopyRect(&r);
        } else if (bEmpty2) {
            RECT r = *lpRect1;
            CopyRect(&r);
        } else {
            LONG l = lpRect1->left < lpRect2->left ? lpRect1->left : lpRect2->left;
            LONG r = lpRect1->right > lpRect2->right ? lpRect1->right : lpRect2->right;
            LONG t = lpRect1->top < lpRect2->top ? lpRect1->top : lpRect2->top;
            LONG b = lpRect1->bottom > lpRect2->bottom ? lpRect1->bottom : lpRect2->bottom;
            SetRect(l, t, r, b);
        }
        return TRUE;
    }

    // This rectangle becomes `lpRectSrc1` with `lpRectSrc2` cut away, which
    // only changes it when the second covers the first fully along one axis.
    // FALSE when nothing is left: the first is empty (it is still copied) or
    // fully covered (all zeros).
    BOOL SubtractRect(const RECT* lpRectSrc1, const RECT* lpRectSrc2)
    {
        RECT src1 = *lpRectSrc1;
        RECT src2 = *lpRectSrc2;
        CopyRect(&src1);
        if (IsEmpty(&src1)) {
            return FALSE;
        }

        CRect overlap;
        if (overlap.IntersectRect(&src1, &src2)) {
            if (overlap.EqualRect(&src1)) {
                SetRectEmpty();
                return FALSE;
            }
            if (overlap.top == top && overlap.bottom == bottom) {
                if (overlap.left == left) {
                    left = overlap.right;
                } else if (overlap.right == right) {
                    right = overlap.left;
                }
            } else if (overlap.left == left && overlap.right == right) {
                if (overlap.top == top) {
                    top = overlap.bottom;
                } else if (overlap.bottom == bottom) {
                    bottom = overlap.top;
                }
            }
        }
        return TRUE;
    }

    void operator=(const RECT& srcRect) { CopyRect(&srcRect); }
    BOOL operator==(const RECT& rect) const { return EqualRect(&rect); }
    BOOL operator!=(const RECT& rect) const { return !EqualRect(&rect); }
    void operator+=(POINT point) { OffsetRect(point.x, point.y); }
    void operator+=(SIZE size) { OffsetRect(size.cx, size.cy); }
    void operator+=(const RECT* lpRect) { InflateRect(lpRect); }
    void operator-=(POINT point) { OffsetRect(-point.x, -point.y); }
    void operator-=(SIZE size) { OffsetRect(-size.cx, -size.cy); }
    void operator-=(const RECT* lpRect) { DeflateRect(lpRect); }
    void operator&=(const RECT& rect) { IntersectRect(this, &rect); }
    void operator|=(const RECT& rect) { UnionRect(this, &rect); }

    CRect operator+(POINT point) const
    {
        CRect rect(*this);
        rect.OffsetRect(point.x, point.y);
        return rect;
    }
    CRect operator-(POINT point) const
    {
        CRect rect(*this);
        rect.OffsetRect(-point.x, -point.y);
        return rect;
    }
    CRect operator+(const RECT* lpRect) const
    {
        CRect rect(this);
        rect.InflateRect(lpRect);
        return rect;
    }
    CRect operator+(SIZE size) const
    {
        CRect rect(*this);
        rect.OffsetRect(size.cx, size.cy);
        return rect;
    }
    CRect operator-(SIZE size) const
    {
        CRect rect(*this);
        rect.OffsetRect(-size.cx, -size.cy);
        return rect;
    }
    CRect operator-(const RECT* lpRect) const
    {
        CRect rect(this);
        rect.DeflateRect(lpRect);
        return rect;
    }
    CRect operator&(const RECT& rect2) const
    {
        CRect rect;
        rect.IntersectRect(this, &rect2);
        return rect;
    }
    CRect operator|(const RECT& rect2) const
    {
        CRect rect;
        rect.UnionRect(this, &rect2);
        return rect;
    }

private:
    static BOOL IsEmpty(const RECT* lpRect) { return lpRect->left >= lpRect->right || lpRect->top >= lpRect->bottom; }
};

inline CPoint CSize::operator+(POINT point) const { return CPoint(cx + point.x, cy + point.y); }
inline CPoint CSize::operator-(POINT point) const { return CPoint(cx - point.x, cy - point.y); }
inline CRect CSize::operator+(const RECT* lpRect) const { return CRect(lpRect) + *this; }
inline CRect CSize::operator-(const RECT* lpRect) const { return CRect(lpRect) - *this; }
inline CRect CPoint::operator+(const RECT* lpRect) const { return CRect(lpRect) + *this; }
inline CRect CPoint::operator-(const RECT* lpRect) const { return CRect(lpRect) - *this; }

#endif /* IWD2RE_COMPAT_GEOMETRY_H_ */
