#ifndef IWD2RE_COMPAT_CSTRING_H_
#define IWD2RE_COMPAT_CSTRING_H_

// Replacement for MFC's `CString` (multi-byte build, `char` elements).
//
// The object is a single pointer to the characters, as in MFC, so it can be
// passed through `...` to `Format("%s")` and keeps every class that holds a
// `CString` at its MFC size. The length, capacity and reference count live in
// a header in front of the characters. Copies share the buffer; a write to a
// shared buffer copies it first.
//
// Text functions that depend on the code page (case, search, compare, white
// space) are multi-byte aware like MFC's `_MBCS` build. The platform calls
// are all in cstring.cpp.

#include <stdarg.h>
#include <stddef.h>

// Thrown (as a `CException*`) for arguments that MFC's CString rejects too.
[[noreturn]] void AfxThrowInvalidStringArgException();

struct CStringData {
    LONG nRefs; // -1 marks the shared empty string, which is never written
    int nDataLength;
    int nAllocLength;

    char* data() { return reinterpret_cast<char*>(this + 1); }
};

class CString {
public:
    CString();
    CString(const CString& strSrc);
    CString(const char* pszSrc);
    CString(const unsigned char* pszSrc);
    CString(const char* pch, int nLength);
    CString(char ch, int nRepeat = 1);
    ~CString();

    CString& operator=(const CString& strSrc);
    CString& operator=(const char* pszSrc);
    CString& operator=(const unsigned char* pszSrc);
    CString& operator=(char ch);

    CString& operator+=(const CString& strSrc);
    CString& operator+=(const char* pszSrc);
    CString& operator+=(char ch);
    CString& operator+=(unsigned char ch);

    operator const char*() const { return m_pchData; }
    const char* GetString() const { return m_pchData; }

    int GetLength() const { return GetData()->nDataLength; }
    int GetAllocLength() const { return GetData()->nAllocLength; }
    bool IsEmpty() const { return GetData()->nDataLength == 0; }
    void Empty();

    // The index may be the length (that reads the terminator). An index
    // outside the string throws, as in MFC.
    char GetAt(int nIndex) const
    {
        if (nIndex < 0 || nIndex > GetLength()) {
            AfxThrowInvalidStringArgException();
        }
        return m_pchData[nIndex];
    }
    char operator[](int nIndex) const { return GetAt(nIndex); }
    // The index must be below the length.
    void SetAt(int nIndex, char ch);

    // Direct access to the buffer. GetAllocLength is the capacity; it follows
    // the same rules as MFC's (see cstring.cpp).
    char* GetBuffer();
    char* GetBuffer(int nMinBufferLength);
    char* GetBufferSetLength(int nLength);
    void ReleaseBuffer(int nNewLength = -1);
    void ReleaseBufferSetLength(int nNewLength);
    void Preallocate(int nLength);
    void FreeExtra();
    void Truncate(int nNewLength);
    void SetString(const char* pszSrc);
    void SetString(const char* pszSrc, int nLength);

    void Append(const char* pszSrc);
    void Append(const char* pszSrc, int nLength);
    void Append(const CString& strSrc);
    void AppendChar(char ch);

    // Comparison.
    int Compare(const char* psz) const;
    int CompareNoCase(const char* psz) const;
    int Collate(const char* psz) const;
    int CollateNoCase(const char* psz) const;

    // Extraction. Counts are bytes and are clamped to the string.
    CString Mid(int nFirst) const;
    CString Mid(int nFirst, int nCount) const;
    CString Left(int nCount) const;
    CString Right(int nCount) const;
    CString SpanIncluding(const char* pszCharSet) const;
    CString SpanExcluding(const char* pszCharSet) const;
    CString Tokenize(const char* pszTokens, int& iStart) const;

    // Searching. All return -1 when nothing is found.
    int Find(char ch, int nStart = 0) const;
    int Find(const char* pszSub, int nStart = 0) const;
    int ReverseFind(char ch) const;
    int FindOneOf(const char* pszCharSet) const;

    // Editing.
    CString& MakeUpper();
    CString& MakeLower();
    CString& MakeReverse();
    CString& TrimLeft();
    CString& TrimLeft(char chTarget);
    CString& TrimLeft(const char* pszTargets);
    CString& TrimRight();
    CString& TrimRight(char chTarget);
    CString& TrimRight(const char* pszTargets);
    CString& Trim();
    int Insert(int nIndex, char ch);
    int Insert(int nIndex, const char* psz);
    int Delete(int nIndex, int nCount = 1);
    int Remove(char chRemove);
    int Replace(char chOld, char chNew);
    int Replace(const char* pszOld, const char* pszNew);

    // Formatting (printf rules).
    void Format(const char* pszFormat, ...);
    void Format(unsigned int nFormatID, ...);
    void FormatV(const char* pszFormat, va_list args);
    void AppendFormat(const char* pszFormat, ...);
    void AppendFormatV(const char* pszFormat, va_list args);

    // Loads a string resource of the module set with `AfxSetResourceHandle`.
    // Leaves the string alone and returns FALSE when there is none.
    int LoadString(unsigned int nID);

    friend CString operator+(const CString& str1, const CString& str2);
    friend CString operator+(const CString& str1, const char* psz2);
    friend CString operator+(const char* psz1, const CString& str2);
    friend CString operator+(const CString& str1, char ch2);
    friend CString operator+(char ch1, const CString& str2);

private:
    CStringData* GetData() const { return reinterpret_cast<CStringData*>(m_pchData) - 1; }
    static CStringData* AllocData(int nLength);
    static void Release(CStringData* pData);
    void Init();
    void AssignCopy(const char* pch, int nLength);
    void PrepareWrite(int nLength);
    void SetLength(int nLength);
    static CString Concat(const char* pch1, int nLength1, const char* pch2, int nLength2);

    char* m_pchData;
};

inline bool operator==(const CString& str1, const CString& str2) { return str1.Compare(str2) == 0; }
inline bool operator==(const CString& str1, const char* psz2) { return str1.Compare(psz2) == 0; }
inline bool operator==(const char* psz1, const CString& str2) { return str2.Compare(psz1) == 0; }
inline bool operator==(const CString& str1, char ch2) { return str1.GetLength() == 1 && str1[0] == ch2; }
inline bool operator==(char ch1, const CString& str2) { return str2.GetLength() == 1 && str2[0] == ch1; }
inline bool operator!=(const CString& str1, const CString& str2) { return str1.Compare(str2) != 0; }
inline bool operator!=(const CString& str1, const char* psz2) { return str1.Compare(psz2) != 0; }
inline bool operator!=(const char* psz1, const CString& str2) { return str2.Compare(psz1) != 0; }
inline bool operator!=(const CString& str1, char ch2) { return !(str1 == ch2); }
inline bool operator!=(char ch1, const CString& str2) { return !(str2 == ch1); }
inline bool operator<(const CString& str1, const CString& str2) { return str1.Compare(str2) < 0; }
inline bool operator<(const CString& str1, const char* psz2) { return str1.Compare(psz2) < 0; }
inline bool operator<(const char* psz1, const CString& str2) { return str2.Compare(psz1) > 0; }
inline bool operator>(const CString& str1, const CString& str2) { return str1.Compare(str2) > 0; }
inline bool operator>(const CString& str1, const char* psz2) { return str1.Compare(psz2) > 0; }
inline bool operator>(const char* psz1, const CString& str2) { return str2.Compare(psz1) < 0; }
inline bool operator<=(const CString& str1, const CString& str2) { return str1.Compare(str2) <= 0; }
inline bool operator<=(const CString& str1, const char* psz2) { return str1.Compare(psz2) <= 0; }
inline bool operator<=(const char* psz1, const CString& str2) { return str2.Compare(psz1) >= 0; }
inline bool operator>=(const CString& str1, const CString& str2) { return str1.Compare(str2) >= 0; }
inline bool operator>=(const CString& str1, const char* psz2) { return str1.Compare(psz2) >= 0; }
inline bool operator>=(const char* psz1, const CString& str2) { return str2.Compare(psz1) <= 0; }

#endif /* IWD2RE_COMPAT_CSTRING_H_ */
