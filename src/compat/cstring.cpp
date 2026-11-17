#include "compat/afxcompat.h"

#include <mbstring.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

// -----------------------------------------------------------------------------
// Platform section: everything below that is not standard C++ is in here.
// -----------------------------------------------------------------------------

namespace {

// The shared empty string. Constant-initialized, so strings constructed
// during static initialization of other files can already point at it.
struct NilString {
    CStringData header;
    char ch[4];
};
NilString g_nil = { { -1, 0, 0 }, { 0, 0, 0, 0 } };

HINSTANCE g_hResource = NULL;

inline LONG AtomicIncrement(LONG* p) { return InterlockedIncrement(p); }
inline LONG AtomicDecrement(LONG* p) { return InterlockedDecrement(p); }

// Multi-byte aware text primitives (the CRT's current multi-byte code page).
inline const unsigned char* U(const char* p) { return reinterpret_cast<const unsigned char*>(p); }
inline unsigned char* U(char* p) { return reinterpret_cast<unsigned char*>(p); }

inline const char* MbNext(const char* p) { return reinterpret_cast<const char*>(_mbsinc(U(p))); }
inline bool MbIsSpace(char ch) { return _ismbcspace(static_cast<unsigned char>(ch)) != 0; }
inline const char* MbFindChar(const char* psz, char ch) { return reinterpret_cast<const char*>(_mbschr(U(psz), static_cast<unsigned char>(ch))); }
inline const char* MbFindCharRev(const char* psz, char ch) { return reinterpret_cast<const char*>(_mbsrchr(U(psz), static_cast<unsigned char>(ch))); }
inline const char* MbFindString(const char* psz, const char* pszSub) { return reinterpret_cast<const char*>(_mbsstr(U(psz), U(pszSub))); }
inline const char* MbScanSet(const char* psz, const char* pszSet) { return reinterpret_cast<const char*>(_mbspbrk(U(psz), U(pszSet))); }
inline int MbSpanIncluding(const char* psz, const char* pszSet) { return static_cast<int>(_mbsspn(U(psz), U(pszSet))); }
inline int MbSpanExcluding(const char* psz, const char* pszSet) { return static_cast<int>(_mbscspn(U(psz), U(pszSet))); }
inline int MbCompare(const char* a, const char* b) { return _mbscmp(U(a), U(b)); }
inline int MbCompareNoCase(const char* a, const char* b) { return _mbsicmp(U(a), U(b)); }
inline int MbCollate(const char* a, const char* b) { return _mbscoll(U(a), U(b)); }
inline int MbCollateNoCase(const char* a, const char* b) { return _mbsicoll(U(a), U(b)); }
inline void MbUpper(char* psz, size_t size) { _mbsupr_s(U(psz), size); }
inline void MbLower(char* psz, size_t size) { _mbslwr_s(U(psz), size); }
inline void MbReverse(char* psz) { _mbsrev(U(psz)); }

// Returns the text of a string resource, or false when there is none (an
// empty string resource counts as none, as in MFC).
bool LoadResourceString(unsigned int nID, std::string& text)
{
    HINSTANCE hInstance = g_hResource != NULL ? g_hResource : GetModuleHandleA(NULL);

    int size = 256;
    for (;;) {
        text.resize(size);
        int len = ::LoadStringA(hInstance, nID, &text[0], size);
        if (len == 0) {
            return false;
        }
        if (len < size - 1) {
            text.resize(len);
            return true;
        }
        size *= 2;
    }
}

} // namespace

HINSTANCE AfxGetResourceHandle()
{
    return g_hResource != NULL ? g_hResource : GetModuleHandleA(NULL);
}

void AfxSetResourceHandle(HINSTANCE hInstResource)
{
    g_hResource = hInstResource;
}

// -----------------------------------------------------------------------------
// Buffer management
//
// The capacity a string reports (GetAllocLength) follows MFC exactly, because
// game code copies that many bytes out of strings:
//  - a new buffer has exactly the capacity asked for;
//  - a private buffer that is too small grows to one and a half times its
//    capacity, or to what is needed if that is more;
//  - writing to a shared buffer first copies it into a new one with room for
//    the longer of the old text and what is needed.
// -----------------------------------------------------------------------------

CStringData* CString::AllocData(int nLength)
{
    // The block itself is rounded up to 8 characters (terminator included).
    size_t nBlock = (static_cast<size_t>(nLength) + 8) & ~static_cast<size_t>(7);
    CStringData* pData = static_cast<CStringData*>(malloc(sizeof(CStringData) + nBlock));
    if (pData == NULL) {
        throw std::bad_alloc();
    }
    pData->nRefs = 1;
    pData->nDataLength = 0;
    pData->nAllocLength = nLength;
    pData->data()[0] = '\0';
    return pData;
}

void CString::Release(CStringData* pData)
{
    if (pData->nRefs != -1 && AtomicDecrement(&pData->nRefs) <= 0) {
        free(pData);
    }
}

void CString::Init()
{
    m_pchData = g_nil.header.data();
}

// Makes the buffer private to this string with room for `nLength` characters,
// keeping the current text.
void CString::PrepareWrite(int nLength)
{
    if (nLength < 0) {
        AfxThrowInvalidStringArgException();
    }

    CStringData* pOld = GetData();
    if (nLength < pOld->nDataLength) {
        nLength = pOld->nDataLength;
    }

    if (pOld->nRefs == 1) {
        if (nLength <= pOld->nAllocLength) {
            return;
        }
        int nGrown = pOld->nAllocLength > 1024 * 1024 * 1024
            ? pOld->nAllocLength + 1024 * 1024
            : pOld->nAllocLength * 3 / 2;
        if (nGrown > nLength) {
            nLength = nGrown;
        }
    }

    CStringData* pNew = AllocData(nLength);
    memcpy(pNew->data(), pOld->data(), pOld->nDataLength + 1);
    pNew->nDataLength = pOld->nDataLength;
    m_pchData = pNew->data();
    Release(pOld);
}

void CString::SetLength(int nLength)
{
    CStringData* pData = GetData();
    if (nLength < 0 || nLength > pData->nAllocLength) {
        AfxThrowInvalidStringArgException();
    }
    if (pData->nRefs == -1) {
        // The shared empty string: nothing to write.
        return;
    }
    pData->nDataLength = nLength;
    m_pchData[nLength] = '\0';
}

void CString::AssignCopy(const char* pch, int nLength)
{
    if (nLength == 0) {
        Empty();
        return;
    }
    if (pch == NULL) {
        AfxThrowInvalidStringArgException();
    }

    // The source may be part of this string, and making room can move the
    // buffer (the text moves with it).
    int nOldLength = GetLength();
    size_t nOffset = static_cast<size_t>(pch - m_pchData);

    PrepareWrite(nLength);
    if (nOffset <= static_cast<size_t>(nOldLength)) {
        memmove(m_pchData, m_pchData + nOffset, nLength);
    } else {
        memcpy(m_pchData, pch, nLength);
    }
    SetLength(nLength);
}

// -----------------------------------------------------------------------------
// Construction, assignment
// -----------------------------------------------------------------------------

CString::CString()
{
    Init();
}

CString::CString(const CString& strSrc)
{
    CStringData* pData = strSrc.GetData();
    if (pData->nRefs != -1) {
        AtomicIncrement(&pData->nRefs);
    }
    m_pchData = strSrc.m_pchData;
}

CString::CString(const char* pszSrc)
{
    Init();
    if (pszSrc == NULL) {
        return;
    }
    if ((reinterpret_cast<size_t>(pszSrc) >> 16) == 0) {
        // A resource id passed with MAKEINTRESOURCE.
        LoadString(static_cast<unsigned int>(reinterpret_cast<size_t>(pszSrc) & 0xFFFF));
        return;
    }
    AssignCopy(pszSrc, static_cast<int>(strlen(pszSrc)));
}

CString::CString(const unsigned char* pszSrc)
{
    Init();
    if (pszSrc != NULL) {
        const char* psz = reinterpret_cast<const char*>(pszSrc);
        AssignCopy(psz, static_cast<int>(strlen(psz)));
    }
}

CString::CString(const char* pch, int nLength)
{
    Init();
    if (nLength < 0 || (pch == NULL && nLength != 0)) {
        AfxThrowInvalidStringArgException();
    }
    // Always a buffer of its own, also for no characters.
    CStringData* pData = AllocData(nLength);
    if (nLength > 0) {
        memcpy(pData->data(), pch, nLength);
    }
    pData->nDataLength = nLength;
    pData->data()[nLength] = '\0';
    m_pchData = pData->data();
}

CString::CString(char ch, int nRepeat)
{
    Init();
    if (nRepeat > 0) {
        char* pch = GetBuffer(nRepeat);
        memset(pch, static_cast<unsigned char>(ch), nRepeat);
        SetLength(nRepeat);
    }
}

CString::~CString()
{
    Release(GetData());
}

CString& CString::operator=(const CString& strSrc)
{
    if (m_pchData != strSrc.m_pchData) {
        CStringData* pNew = strSrc.GetData();
        if (pNew->nRefs != -1) {
            AtomicIncrement(&pNew->nRefs);
        }
        CStringData* pOld = GetData();
        m_pchData = strSrc.m_pchData;
        Release(pOld);
    }
    return *this;
}

CString& CString::operator=(const char* pszSrc)
{
    SetString(pszSrc);
    return *this;
}

CString& CString::operator=(const unsigned char* pszSrc)
{
    SetString(reinterpret_cast<const char*>(pszSrc));
    return *this;
}

CString& CString::operator=(char ch)
{
    char sz[2] = { ch, '\0' };
    SetString(sz);
    return *this;
}

void CString::SetString(const char* pszSrc)
{
    AssignCopy(pszSrc, pszSrc != NULL ? static_cast<int>(strlen(pszSrc)) : 0);
}

void CString::SetString(const char* pszSrc, int nLength)
{
    AssignCopy(pszSrc, nLength);
}

void CString::Empty()
{
    CStringData* pOld = GetData();
    if (pOld->nDataLength == 0) {
        return;
    }
    Init();
    Release(pOld);
}

// -----------------------------------------------------------------------------
// Buffer access
// -----------------------------------------------------------------------------

void CString::SetAt(int nIndex, char ch)
{
    if (nIndex < 0 || nIndex >= GetLength()) {
        AfxThrowInvalidStringArgException();
    }
    PrepareWrite(GetLength());
    m_pchData[nIndex] = ch;
}

char* CString::GetBuffer()
{
    PrepareWrite(GetLength());
    return m_pchData;
}

char* CString::GetBuffer(int nMinBufferLength)
{
    PrepareWrite(nMinBufferLength);
    return m_pchData;
}

char* CString::GetBufferSetLength(int nLength)
{
    PrepareWrite(nLength);
    SetLength(nLength);
    return m_pchData;
}

void CString::ReleaseBuffer(int nNewLength)
{
    if (nNewLength == -1) {
        nNewLength = static_cast<int>(strnlen(m_pchData, GetData()->nAllocLength));
    }
    SetLength(nNewLength);
}

void CString::ReleaseBufferSetLength(int nNewLength)
{
    SetLength(nNewLength);
}

void CString::Preallocate(int nLength)
{
    PrepareWrite(nLength);
}

void CString::FreeExtra()
{
    CStringData* pOld = GetData();
    if (pOld->nRefs != 1 || pOld->nDataLength == pOld->nAllocLength) {
        return;
    }
    CStringData* pNew = AllocData(pOld->nDataLength);
    memcpy(pNew->data(), pOld->data(), pOld->nDataLength + 1);
    pNew->nDataLength = pOld->nDataLength;
    m_pchData = pNew->data();
    Release(pOld);
}

void CString::Truncate(int nNewLength)
{
    PrepareWrite(nNewLength);
    SetLength(nNewLength);
}

// -----------------------------------------------------------------------------
// Concatenation
// -----------------------------------------------------------------------------

void CString::Append(const char* pszSrc, int nLength)
{
    if (nLength < 0) {
        AfxThrowInvalidStringArgException();
    }

    // Appended text ends at its first terminator, also when it comes from a
    // CString that holds more (operator+ is different: it takes all of it).
    nLength = pszSrc != NULL ? static_cast<int>(strnlen(pszSrc, nLength)) : 0;

    // The source may be part of this string, and growing moves the buffer.
    int nOldLength = GetLength();
    size_t nOffset = static_cast<size_t>(pszSrc - m_pchData);

    PrepareWrite(nOldLength + nLength);
    if (nOffset <= static_cast<size_t>(nOldLength)) {
        pszSrc = m_pchData + nOffset;
    }
    if (nLength > 0) {
        memcpy(m_pchData + nOldLength, pszSrc, nLength);
    }
    SetLength(nOldLength + nLength);
}

void CString::Append(const char* pszSrc)
{
    Append(pszSrc, pszSrc != NULL ? static_cast<int>(strlen(pszSrc)) : 0);
}

void CString::Append(const CString& strSrc)
{
    Append(strSrc.m_pchData, strSrc.GetLength());
}

void CString::AppendChar(char ch)
{
    int nOldLength = GetLength();
    PrepareWrite(nOldLength + 1);
    m_pchData[nOldLength] = ch;
    SetLength(nOldLength + 1);
}

CString& CString::operator+=(const CString& strSrc)
{
    Append(strSrc.m_pchData, strSrc.GetLength());
    return *this;
}

CString& CString::operator+=(const char* pszSrc)
{
    Append(pszSrc);
    return *this;
}

CString& CString::operator+=(char ch)
{
    AppendChar(ch);
    return *this;
}

CString& CString::operator+=(unsigned char ch)
{
    AppendChar(static_cast<char>(ch));
    return *this;
}

CString CString::Concat(const char* pch1, int nLength1, const char* pch2, int nLength2)
{
    CString result;
    int nLength = nLength1 + nLength2;
    char* pch = result.GetBuffer(nLength);
    if (nLength1 > 0) {
        memcpy(pch, pch1, nLength1);
    }
    if (nLength2 > 0) {
        memcpy(pch + nLength1, pch2, nLength2);
    }
    result.SetLength(nLength);
    return result;
}

CString operator+(const CString& str1, const CString& str2)
{
    return CString::Concat(str1.m_pchData, str1.GetLength(), str2.m_pchData, str2.GetLength());
}

CString operator+(const CString& str1, const char* psz2)
{
    return CString::Concat(str1.m_pchData, str1.GetLength(), psz2, psz2 != NULL ? static_cast<int>(strlen(psz2)) : 0);
}

CString operator+(const char* psz1, const CString& str2)
{
    return CString::Concat(psz1, psz1 != NULL ? static_cast<int>(strlen(psz1)) : 0, str2.m_pchData, str2.GetLength());
}

CString operator+(const CString& str1, char ch2)
{
    return CString::Concat(str1.m_pchData, str1.GetLength(), &ch2, 1);
}

CString operator+(char ch1, const CString& str2)
{
    return CString::Concat(&ch1, 1, str2.m_pchData, str2.GetLength());
}

// -----------------------------------------------------------------------------
// Comparison
// -----------------------------------------------------------------------------

int CString::Compare(const char* psz) const
{
    if (psz == NULL) {
        AfxThrowInvalidStringArgException();
    }
    return MbCompare(m_pchData, psz);
}

int CString::CompareNoCase(const char* psz) const
{
    if (psz == NULL) {
        AfxThrowInvalidStringArgException();
    }
    return MbCompareNoCase(m_pchData, psz);
}

int CString::Collate(const char* psz) const
{
    if (psz == NULL) {
        AfxThrowInvalidStringArgException();
    }
    return MbCollate(m_pchData, psz);
}

int CString::CollateNoCase(const char* psz) const
{
    if (psz == NULL) {
        AfxThrowInvalidStringArgException();
    }
    return MbCollateNoCase(m_pchData, psz);
}

// -----------------------------------------------------------------------------
// Extraction
// -----------------------------------------------------------------------------

CString CString::Mid(int nFirst) const
{
    return Mid(nFirst, GetLength() - nFirst);
}

CString CString::Mid(int nFirst, int nCount) const
{
    int nLength = GetLength();
    if (nFirst < 0) {
        nFirst = 0;
    }
    if (nCount < 0) {
        nCount = 0;
    }
    if (nFirst > nLength) {
        nCount = 0;
    } else if (nCount > nLength - nFirst) {
        nCount = nLength - nFirst;
    }
    if (nFirst == 0 && nCount == nLength) {
        return *this;
    }
    return CString(m_pchData + nFirst, nCount);
}

CString CString::Left(int nCount) const
{
    if (nCount < 0) {
        nCount = 0;
    }
    if (nCount >= GetLength()) {
        return *this;
    }
    return CString(m_pchData, nCount);
}

CString CString::Right(int nCount) const
{
    int nLength = GetLength();
    if (nCount < 0) {
        nCount = 0;
    }
    if (nCount >= nLength) {
        return *this;
    }
    return CString(m_pchData + nLength - nCount, nCount);
}

CString CString::SpanIncluding(const char* pszCharSet) const
{
    if (pszCharSet == NULL) {
        AfxThrowInvalidStringArgException();
    }
    return Left(MbSpanIncluding(m_pchData, pszCharSet));
}

CString CString::SpanExcluding(const char* pszCharSet) const
{
    if (pszCharSet == NULL) {
        AfxThrowInvalidStringArgException();
    }
    return Left(MbSpanExcluding(m_pchData, pszCharSet));
}

CString CString::Tokenize(const char* pszTokens, int& iStart) const
{
    if (pszTokens == NULL || *pszTokens == '\0') {
        if (iStart >= 0 && iStart < GetLength()) {
            return Mid(iStart);
        }
    } else if (iStart >= 0 && iStart < GetLength()) {
        const char* pszPlace = m_pchData + iStart;
        int nIncluding = MbSpanIncluding(pszPlace, pszTokens);
        if (pszPlace + nIncluding < m_pchData + GetLength()) {
            pszPlace += nIncluding;
            int nExcluding = MbSpanExcluding(pszPlace, pszTokens);
            int nFrom = iStart + nIncluding;
            iStart = nFrom + nExcluding + 1;
            return Mid(nFrom, nExcluding);
        }
    }
    iStart = -1;
    return CString();
}

// -----------------------------------------------------------------------------
// Searching
// -----------------------------------------------------------------------------

int CString::Find(char ch, int nStart) const
{
    if (nStart < 0 || nStart >= GetLength()) {
        return -1;
    }
    const char* psz = MbFindChar(m_pchData + nStart, ch);
    return psz != NULL ? static_cast<int>(psz - m_pchData) : -1;
}

int CString::Find(const char* pszSub, int nStart) const
{
    if (pszSub == NULL || nStart < 0 || nStart > GetLength()) {
        return -1;
    }
    const char* psz = MbFindString(m_pchData + nStart, pszSub);
    return psz != NULL ? static_cast<int>(psz - m_pchData) : -1;
}

int CString::ReverseFind(char ch) const
{
    const char* psz = MbFindCharRev(m_pchData, ch);
    return psz != NULL ? static_cast<int>(psz - m_pchData) : -1;
}

int CString::FindOneOf(const char* pszCharSet) const
{
    if (pszCharSet == NULL) {
        return -1;
    }
    const char* psz = MbScanSet(m_pchData, pszCharSet);
    return psz != NULL ? static_cast<int>(psz - m_pchData) : -1;
}

// -----------------------------------------------------------------------------
// Editing
// -----------------------------------------------------------------------------

CString& CString::MakeUpper()
{
    int nLength = GetLength();
    PrepareWrite(nLength);
    MbUpper(m_pchData, nLength + 1);
    return *this;
}

CString& CString::MakeLower()
{
    int nLength = GetLength();
    PrepareWrite(nLength);
    MbLower(m_pchData, nLength + 1);
    return *this;
}

CString& CString::MakeReverse()
{
    int nLength = GetLength();
    PrepareWrite(nLength);
    MbReverse(m_pchData);
    return *this;
}

namespace {

// Offset of the first character of the trailing run that `match` accepts, or
// -1 when the text does not end with such a run.
template <class Pred>
int TrailingRun(const char* pszStart, Pred match)
{
    const char* pszLast = NULL;
    for (const char* psz = pszStart; *psz != '\0'; psz = MbNext(psz)) {
        if (match(*psz)) {
            if (pszLast == NULL) {
                pszLast = psz;
            }
        } else {
            pszLast = NULL;
        }
    }
    return pszLast != NULL ? static_cast<int>(pszLast - pszStart) : -1;
}

// Number of bytes at the start of the text that `match` accepts.
template <class Pred>
int LeadingRun(const char* pszStart, Pred match)
{
    const char* psz = pszStart;
    while (*psz != '\0' && match(*psz)) {
        psz = MbNext(psz);
    }
    return static_cast<int>(psz - pszStart);
}

} // namespace

CString& CString::TrimRight()
{
    int nLast = TrailingRun(m_pchData, [](char ch) { return MbIsSpace(ch); });
    if (nLast >= 0) {
        Truncate(nLast);
    }
    return *this;
}

CString& CString::TrimRight(char chTarget)
{
    int nLast = TrailingRun(m_pchData, [chTarget](char ch) { return ch == chTarget; });
    if (nLast >= 0) {
        Truncate(nLast);
    }
    return *this;
}

CString& CString::TrimRight(const char* pszTargets)
{
    if (pszTargets == NULL || *pszTargets == '\0') {
        return *this;
    }
    int nLast = TrailingRun(m_pchData, [pszTargets](char ch) { return MbFindChar(pszTargets, ch) != NULL; });
    if (nLast >= 0) {
        Truncate(nLast);
    }
    return *this;
}

CString& CString::TrimLeft()
{
    int nFirst = LeadingRun(m_pchData, [](char ch) { return MbIsSpace(ch); });
    if (nFirst > 0) {
        Delete(0, nFirst);
    }
    return *this;
}

CString& CString::TrimLeft(char chTarget)
{
    int nFirst = LeadingRun(m_pchData, [chTarget](char ch) { return ch == chTarget; });
    if (nFirst > 0) {
        Delete(0, nFirst);
    }
    return *this;
}

CString& CString::TrimLeft(const char* pszTargets)
{
    if (pszTargets == NULL || *pszTargets == '\0') {
        return *this;
    }
    int nFirst = LeadingRun(m_pchData, [pszTargets](char ch) { return MbFindChar(pszTargets, ch) != NULL; });
    if (nFirst > 0) {
        Delete(0, nFirst);
    }
    return *this;
}

CString& CString::Trim()
{
    TrimRight();
    return TrimLeft();
}

int CString::Insert(int nIndex, char ch)
{
    int nLength = GetLength();
    if (nIndex < 0) {
        nIndex = 0;
    }
    if (nIndex > nLength) {
        nIndex = nLength;
    }
    PrepareWrite(nLength + 1);
    memmove(m_pchData + nIndex + 1, m_pchData + nIndex, nLength - nIndex);
    m_pchData[nIndex] = ch;
    SetLength(nLength + 1);
    return nLength + 1;
}

int CString::Insert(int nIndex, const char* psz)
{
    int nLength = GetLength();
    if (nIndex < 0) {
        nIndex = 0;
    }
    if (nIndex > nLength) {
        nIndex = nLength;
    }
    int nInsert = psz != NULL ? static_cast<int>(strlen(psz)) : 0;
    if (nInsert > 0) {
        std::string copy(psz, nInsert); // `psz` may be part of this string
        PrepareWrite(nLength + nInsert);
        memmove(m_pchData + nIndex + nInsert, m_pchData + nIndex, nLength - nIndex);
        memcpy(m_pchData + nIndex, copy.data(), nInsert);
        SetLength(nLength + nInsert);
    }
    return nLength + nInsert;
}

int CString::Delete(int nIndex, int nCount)
{
    int nLength = GetLength();
    if (nIndex < 0) {
        nIndex = 0;
    }
    if (nCount < 0) {
        nCount = 0;
    }
    if (nCount > nLength - nIndex) {
        nCount = nLength - nIndex;
    }
    if (nCount > 0) {
        PrepareWrite(nLength);
        memmove(m_pchData + nIndex, m_pchData + nIndex + nCount, nLength - nIndex - nCount);
        SetLength(nLength - nCount);
    }
    return GetLength();
}

int CString::Remove(char chRemove)
{
    int nLength = GetLength();
    PrepareWrite(nLength);

    char* pszSource = m_pchData;
    char* pszDest = m_pchData;
    char* pszEnd = m_pchData + nLength;
    while (pszSource < pszEnd) {
        char* pszNext = const_cast<char*>(MbNext(pszSource));
        if (*pszSource != chRemove) {
            size_t n = pszNext - pszSource;
            memmove(pszDest, pszSource, n);
            pszDest += n;
        }
        pszSource = pszNext;
    }

    int nCount = static_cast<int>(pszSource - pszDest);
    SetLength(nLength - nCount);
    return nCount;
}

int CString::Replace(char chOld, char chNew)
{
    int nCount = 0;
    if (chOld != chNew) {
        int nLength = GetLength();
        bool bCopied = false;
        for (int i = 0; i < nLength;) {
            int nNext = static_cast<int>(MbNext(m_pchData + i) - m_pchData);
            if (m_pchData[i] == chOld) {
                if (!bCopied) {
                    PrepareWrite(nLength);
                    bCopied = true;
                }
                m_pchData[i] = chNew;
                nCount++;
            }
            i = nNext;
        }
    }
    return nCount;
}

int CString::Replace(const char* pszOld, const char* pszNew)
{
    size_t nOldLen = pszOld != NULL ? strlen(pszOld) : 0;
    if (nOldLen == 0) {
        return 0;
    }
    size_t nNewLen = pszNew != NULL ? strlen(pszNew) : 0;

    // The text may hold embedded terminators; each run between them is
    // searched on its own, like MFC.
    int nCount = 0;
    std::string result;
    const char* pszStart = m_pchData;
    const char* pszEnd = m_pchData + GetLength();
    while (pszStart < pszEnd) {
        const char* pszTarget;
        while ((pszTarget = MbFindString(pszStart, pszOld)) != NULL) {
            result.append(pszStart, pszTarget - pszStart);
            if (nNewLen > 0) {
                result.append(pszNew, nNewLen);
            }
            pszStart = pszTarget + nOldLen;
            nCount++;
        }
        size_t nRest = strlen(pszStart);
        result.append(pszStart, nRest);
        pszStart += nRest;
        if (pszStart < pszEnd) {
            // The embedded terminator that ended this run.
            result.push_back('\0');
            pszStart++;
        }
    }

    if (nCount > 0) {
        int nOldLength = GetLength();
        int nNewLength = static_cast<int>(result.size());
        PrepareWrite(nNewLength > nOldLength ? nNewLength : nOldLength);
        memcpy(m_pchData, result.data(), nNewLength);
        SetLength(nNewLength);
    }
    return nCount;
}

// -----------------------------------------------------------------------------
// Formatting, resources
// -----------------------------------------------------------------------------

void CString::FormatV(const char* pszFormat, va_list args)
{
    if (pszFormat == NULL) {
        AfxThrowInvalidStringArgException();
    }

    // Formatted into a separate buffer first: the arguments may point into
    // this string.
    va_list copy;
    va_copy(copy, args);
    int nLength = vsnprintf(NULL, 0, pszFormat, copy);
    va_end(copy);
    if (nLength < 0) {
        AfxThrowInvalidStringArgException();
    }

    std::string text(static_cast<size_t>(nLength) + 1, '\0');
    vsnprintf(&text[0], text.size(), pszFormat, args);

    PrepareWrite(nLength);
    memcpy(m_pchData, text.data(), nLength);
    SetLength(nLength);
}

void CString::Format(const char* pszFormat, ...)
{
    va_list args;
    va_start(args, pszFormat);
    FormatV(pszFormat, args);
    va_end(args);
}

void CString::Format(unsigned int nFormatID, ...)
{
    CString strFormat;
    if (!strFormat.LoadString(nFormatID)) {
        return;
    }
    va_list args;
    va_start(args, nFormatID);
    FormatV(strFormat, args);
    va_end(args);
}

void CString::AppendFormatV(const char* pszFormat, va_list args)
{
    CString text;
    text.FormatV(pszFormat, args);
    Append(text);
}

void CString::AppendFormat(const char* pszFormat, ...)
{
    va_list args;
    va_start(args, pszFormat);
    AppendFormatV(pszFormat, args);
    va_end(args);
}

int CString::LoadString(unsigned int nID)
{
    std::string text;
    if (!LoadResourceString(nID, text)) {
        return FALSE;
    }
    int nLength = static_cast<int>(text.size());
    PrepareWrite(nLength);
    memcpy(m_pchData, text.data(), nLength);
    SetLength(nLength);
    return TRUE;
}
