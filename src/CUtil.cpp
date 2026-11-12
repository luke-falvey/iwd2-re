#include "CUtil.h"

#include <zlib.h>

#include "CChitin.h"
#include "CVidFont.h"

// #guess
// 0x8FB8E8
CCriticalSection CUtil::ZLIB_CRITICAL_SECTION;

// 0x780AD0
int CUtil::Compress(BYTE* pDest, DWORD* nDestSize, const BYTE* pSrc, DWORD nSrcSize, int nLevel)
{
    CSingleLock compressionLock(&ZLIB_CRITICAL_SECTION, FALSE);
    compressionLock.Lock(INFINITE);

    int err = compress2(pDest, nDestSize, pSrc, nSrcSize, nLevel);

    compressionLock.Unlock();

    return err;
}

// 0x780B60
int CUtil::Uncompress(BYTE* dest, DWORD* destLen, const BYTE* source, DWORD sourceLen)
{
    CSingleLock decompressionLock(&ZLIB_CRITICAL_SECTION, FALSE);
    decompressionLock.Lock(INFINITE);

    int err = uncompress(dest, destLen, source, sourceLen);

    decompressionLock.Unlock();

    return err;
}

// 0x780C00
void CUtil::UtilAssert(int nLineNumber, const char* szFileName, const char* pExp, const char* pText)
{
    const char* fileName = strrchr(szFileName, '\\') + 1;
    if (pText == NULL) {
        pText = "no msg.";
    }

    char buffer[1024];
    sprintf(buffer, "ASSERTION FAILED!\nFile: %s\nLine: %d\n\nExp: %s\n\nMsg: %s\nRun Debugger?", fileName, nLineNumber, pExp, pText);

    g_pChitin->SuspendThreads();
    g_pChitin->ShutDown(nLineNumber, fileName, pText);

    abort();
}

// 0x780C80
LONG CUtil::UtilRandInt(LONG nRange, LONG nWeight)
{
    LONG nValue = nWeight + ((nRange * rand()) >> 15);

    if (nValue >= nRange) {
        nValue = nRange - 1;
    }

    if (nValue < 0) {
        nValue = 0;
    }

    return nValue;
}

// 0x780CB0
int CUtil::GetCurrentBitsPerPixels()
{
    HDC dc = GetDC(NULL);
    int bpp = GetDeviceCaps(dc, PLANES) * GetDeviceCaps(dc, BITSPIXEL);
    ReleaseDC(NULL, dc);
    return bpp;
}

// 0x780CF0
BYTE CUtil::SplitString(CVidFont* pTextFont, const CString& sSource, WORD nLineLength, BYTE nMaxStrings, CString* pStringsOut, BOOL bDivideWords, BOOL bDemanded, BOOL bCheckRemainderForNewLine, WORD nSuggestedSplitLength)
{
    if (sSource.Compare("") == 0) {
        return 0;
    }

    if (pStringsOut == NULL || nMaxStrings == 0 || nLineLength == 0) {
        return 0;
    }

    if (!bDemanded) {
        if (pTextFont->pRes->Demand() == NULL) {
            return 0;
        }
    }

    pStringsOut[0] = sSource;
    for (BYTE nNext = 1; nNext < nMaxStrings; nNext++) {
        pStringsOut[nNext] = "";
    }

    BYTE nCurrentLine = 0;
    while (nCurrentLine < nMaxStrings) {
        if (pStringsOut[nCurrentLine].IsEmpty()) {
            break;
        }

        int nSpace = -1;

        int nIndex = 0;
        while (nIndex < pStringsOut[nCurrentLine].GetLength()) {
            int nNext = nIndex;

            if (g_pChitin->field_1A0 && IsDBCSLeadByte(pStringsOut[nCurrentLine][nIndex])) {
                nNext = nIndex + 1;
            } else {
                char ch = pStringsOut[nCurrentLine][nIndex];
                if (ch == '\n' || ch == '\r') {
                    if (nCurrentLine + 1 < nMaxStrings) {
                        pStringsOut[nCurrentLine + 1] = pStringsOut[nCurrentLine].Mid(nIndex + 1);
                    }

                    if (nCurrentLine + 1 != nMaxStrings || bCheckRemainderForNewLine) {
                        pStringsOut[nCurrentLine] = pStringsOut[nCurrentLine].Left(nIndex);
                    }

                    break;
                }

                if (ch == ' ') {
                    nSpace = nIndex;
                }
            }

            nNext++;

            LONG nSubStringWidth = pTextFont->GetStringLength(pStringsOut[nCurrentLine].Left(nNext), TRUE);

            // NOTE: Signed compare.
            if (nSubStringWidth > nLineLength - (g_pChitin->field_1A0 ? 10 : 0)
                || (nSubStringWidth >= nSuggestedSplitLength
                    && nSpace == nIndex)) {
                if (nCurrentLine + 1 < nMaxStrings) {
                    if (bDivideWords || nSpace == -1) {
                        if (g_pChitin->field_1A0 && IsDBCSLeadByte(pStringsOut[nCurrentLine][nIndex])) {
                            pStringsOut[nCurrentLine + 1] = pStringsOut[nCurrentLine].Mid(nIndex);
                            pStringsOut[nCurrentLine] = pStringsOut[nCurrentLine].Left(nIndex);
                        } else {
                            pStringsOut[nCurrentLine + 1] = pStringsOut[nCurrentLine].Mid(nIndex);
                            pStringsOut[nCurrentLine] = pStringsOut[nCurrentLine].Left(nIndex);
                        }
                    } else {
                        pStringsOut[nCurrentLine + 1] = pStringsOut[nCurrentLine].Mid(nSpace + 1);
                        pStringsOut[nCurrentLine] = pStringsOut[nCurrentLine].Left(nSpace);
                    }
                }

                break;
            }

            nIndex = nNext;
        }

        nCurrentLine++;
    }

    if (!bDemanded) {
        pTextFont->pRes->Release();
    }

    return nCurrentLine;
}

// 0x781100
int CUtil::Find(const CString& sString, char ch, int start)
{
    int nIndex = start;
    while (nIndex < sString.GetLength()) {
        if (g_pChitin->field_1A0 && IsDBCSLeadByte(sString[nIndex])) {
            nIndex++;
        } else {
            if (sString[nIndex] == ch) {
                return nIndex;
            }
        }

        nIndex++;
    }

    return -1;
}

// 0x781180
int CUtil::FindOneOf(const CString& sString, const CString& sSeparators, int start)
{
    CString sMutableSeparators(sSeparators);

    if (g_pChitin->field_1A0) {
        int nIndex = 0;
        while (nIndex < sMutableSeparators.GetLength()) {
            if (IsDBCSLeadByte(sMutableSeparators[nIndex])) {
                sMutableSeparators = sMutableSeparators.Left(nIndex) + sMutableSeparators.Mid(nIndex + 1);
                nIndex--;
            }
            nIndex++;
        }

        if (sMutableSeparators.IsEmpty()) {
            return -1;
        }
    }

    int nIndex = start;
    while (nIndex < sString.GetLength()) {
        if (IsDBCSLeadByte(sMutableSeparators[nIndex])) {
            nIndex++;
        } else {
            for (int pos = 0; pos < sMutableSeparators.GetLength(); pos++) {
                if (sString[nIndex] == sMutableSeparators[pos]) {
                    return nIndex;
                }
            }
        }

        nIndex++;
    }

    return -1;
}

// 0x781330
void CUtil::MakeReverse(CString& sString)
{
    CString sReversed;

    int nIndex = 0;
    while (nIndex < sString.GetLength()) {
        if (g_pChitin->field_1A0 && IsDBCSLeadByte(sString[nIndex])) {
            sReversed = sString.Mid(nIndex, 2) + sReversed;
            nIndex++;
        } else {
            sReversed = sString.Mid(nIndex, 1) + sReversed;
        }

        nIndex++;
    }

    sString = sReversed;
}

// 0x7815D0
BOOL CUtil::IsDBCSTrailByte(const CString& sString, int end)
{
    // NOTE: Looks odd.
    int v1 = -2;

    for (int index = 0; index < end; index++) {
        if (IsDBCSLeadByte(sString[index])) {
            v1 = index++;
        }
    }

    return end - v1 == 1;
}

// 0x781470
void CUtil::TrimLeft(CString& sString)
{
    int nIndex = 0;
    while (nIndex < sString.GetLength()) {
        if (g_pChitin->field_1A0 && IsDBCSLeadByte(sString[nIndex])) {
            break;
        }

        if (sString[nIndex] != ' '
            && sString[nIndex] != '\t'
            && sString[nIndex] != '\n') {
            break;
        }

        nIndex++;
    }

    sString = sString.Mid(nIndex);
}

// 0x781520
void CUtil::TrimRight(CString& sString)
{
    int nEnd = -1;
    int nIndex = 0;
    while (nIndex < sString.GetLength()) {
        if (g_pChitin->field_1A0 && IsDBCSLeadByte(sString[nIndex])) {
            nIndex++;
            nEnd = nIndex;
        } else {
            if (sString[nIndex] != ' '
                && sString[nIndex] != '\t'
                && sString[nIndex] != '\n') {
                nEnd = nIndex;
            }
        }

        nIndex++;
    }

    sString = sString.Left(nEnd + 1);
}

// 0x781630
DWORD CUtil::RotateLeft(DWORD nValue, BYTE nBits, BYTE nShift)
{
    if (nValue == 0 || nBits == 0 || nShift == 0 || nShift >= nBits) {
        return nValue;
    }

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChUtil.cpp
    // __LINE__: 1161
    UTIL_ASSERT(nBits <= 32);

    DWORD nResult = (nValue >> (nBits - nShift)) & ((1 << nShift) - 1);
    DWORD nShifted = nValue << nShift;

    // NOTE: Compares the bit count with a byte count (probably
    // `sizeof(DWORD)`), so the high bits are only cleared for fields narrower
    // than 4 bits.
    if (nBits < 4) {
        nShifted &= (1 << nBits) - 1;
    }

    return nResult | nShifted;
}
