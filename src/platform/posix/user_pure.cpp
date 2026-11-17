// Functions of user32 that have no window or display in them: character
// helpers (single-byte code page) and RECT arithmetic. The compat CFile needs
// CharNextA and the compat geometry tests compare with these, so they are
// real here and not pending stubs. The window/input task must not define them.

#include <win32shim/winuser_window_input.h>

#include <ctype.h>

BOOL IsDBCSLeadByte(BYTE)
{
    return FALSE;
}

BOOL IsCharAlphaNumericA(CHAR ch)
{
    return isalnum(static_cast<unsigned char>(ch)) != 0;
}

LPSTR CharNextA(LPCSTR lpsz)
{
    if (lpsz == NULL) {
        return NULL;
    }
    return const_cast<LPSTR>(*lpsz == 0 ? lpsz : lpsz + 1);
}

// ---------------------------------------------------------------------------
// RECT

BOOL SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom)
{
    if (lprc == NULL) {
        return FALSE;
    }
    lprc->left = xLeft;
    lprc->top = yTop;
    lprc->right = xRight;
    lprc->bottom = yBottom;
    return TRUE;
}

BOOL SetRectEmpty(LPRECT lprc)
{
    return SetRect(lprc, 0, 0, 0, 0);
}

BOOL CopyRect(LPRECT lprcDst, const RECT* lprcSrc)
{
    if (lprcDst == NULL || lprcSrc == NULL) {
        return FALSE;
    }
    *lprcDst = *lprcSrc;
    return TRUE;
}

BOOL IsRectEmpty(const RECT* lprc)
{
    return lprc == NULL || lprc->right <= lprc->left || lprc->bottom <= lprc->top;
}

BOOL EqualRect(const RECT* lprc1, const RECT* lprc2)
{
    return lprc1->left == lprc2->left && lprc1->top == lprc2->top && lprc1->right == lprc2->right && lprc1->bottom == lprc2->bottom;
}

BOOL IntersectRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2)
{
    RECT r;
    r.left = lprcSrc1->left > lprcSrc2->left ? lprcSrc1->left : lprcSrc2->left;
    r.top = lprcSrc1->top > lprcSrc2->top ? lprcSrc1->top : lprcSrc2->top;
    r.right = lprcSrc1->right < lprcSrc2->right ? lprcSrc1->right : lprcSrc2->right;
    r.bottom = lprcSrc1->bottom < lprcSrc2->bottom ? lprcSrc1->bottom : lprcSrc2->bottom;
    if (r.right <= r.left || r.bottom <= r.top) {
        SetRectEmpty(lprcDst);
        return FALSE;
    }
    *lprcDst = r;
    return TRUE;
}

BOOL UnionRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2)
{
    bool empty1 = IsRectEmpty(lprcSrc1) != FALSE;
    bool empty2 = IsRectEmpty(lprcSrc2) != FALSE;
    if (empty1 && empty2) {
        SetRectEmpty(lprcDst);
        return FALSE;
    }
    RECT r;
    if (empty1) {
        r = *lprcSrc2;
    } else if (empty2) {
        r = *lprcSrc1;
    } else {
        r.left = lprcSrc1->left < lprcSrc2->left ? lprcSrc1->left : lprcSrc2->left;
        r.top = lprcSrc1->top < lprcSrc2->top ? lprcSrc1->top : lprcSrc2->top;
        r.right = lprcSrc1->right > lprcSrc2->right ? lprcSrc1->right : lprcSrc2->right;
        r.bottom = lprcSrc1->bottom > lprcSrc2->bottom ? lprcSrc1->bottom : lprcSrc2->bottom;
    }
    *lprcDst = r;
    return TRUE;
}

BOOL SubtractRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2)
{
    // The result is src1 with the part that src2 covers removed, but only
    // when what is left is a rectangle.
    RECT r = *lprcSrc1;
    if (IsRectEmpty(&r)) {
        *lprcDst = r; // nothing to cut from: copied as it is
        return FALSE;
    }
    RECT overlap;
    if (IntersectRect(&overlap, &r, lprcSrc2)) {
        if (EqualRect(&overlap, &r)) {
            SetRectEmpty(lprcDst);
            return FALSE;
        }
        if (overlap.top == r.top && overlap.bottom == r.bottom) {
            if (overlap.left == r.left) {
                r.left = overlap.right;
            } else if (overlap.right == r.right) {
                r.right = overlap.left;
            }
        } else if (overlap.left == r.left && overlap.right == r.right) {
            if (overlap.top == r.top) {
                r.top = overlap.bottom;
            } else if (overlap.bottom == r.bottom) {
                r.bottom = overlap.top;
            }
        }
    }
    *lprcDst = r;
    return TRUE;
}

BOOL PtInRect(const RECT* lprc, POINT pt)
{
    return pt.x >= lprc->left && pt.x < lprc->right && pt.y >= lprc->top && pt.y < lprc->bottom;
}
