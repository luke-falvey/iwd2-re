#include "CResFile.h"

#include <direct.h>

#include "CChitin.h"
#include "CUtil.h"

// NOTE: Inlined in `CDimm::CreateKeyTable`.
CResFile::CResFile()
{
    m_nIndex = 0;
    m_pHeader = NULL;
    m_pVarEntries = NULL;
    m_pFixedEntries = NULL;
    field_20 = NULL;
    m_bOpen = FALSE;
    field_4 = 0;
    m_nCacheCount = 0;
    m_nRefCount = 0;
    field_38 = 0;
}

// 0x7898F0
CResFile::~CResFile()
{
    if (m_bOpen) {
        CloseFile();
    }
}

// 0x789950
BOOL CResFile::AddCacheCount()
{
    EnterCriticalSection(&(g_pChitin->field_35C));

    while (g_pChitin->cDimm.cResCache.field_110 == 1) {
        LeaveCriticalSection(&(g_pChitin->field_35C));
        while (g_pChitin->cDimm.cResCache.field_110 == 1) {
            SleepEx(50, FALSE);
        }
        EnterCriticalSection(&(g_pChitin->field_35C));
    }

    if (m_nRefCount != 0) {
        LeaveCriticalSection(&(g_pChitin->field_35C));
        return FALSE;
    }

    if (m_nCacheCount == 0) {
        m_nCacheCount = 0;
    }
    m_nCacheCount++;

    g_pChitin->cDimm.cResCache.field_110 = 1;
    LeaveCriticalSection(&(g_pChitin->field_35C));

    return TRUE;
}

// 0x78A080
BOOL CResFile::CloseFile()
{
    if (m_bOpen) {
        if (m_pHeader != NULL) {
            delete m_pHeader;
            m_pHeader = NULL;
        }

        if (m_pVarEntries != NULL) {
            delete m_pVarEntries;
            m_pVarEntries = NULL;
        }

        if (m_pFixedEntries != NULL) {
            delete m_pFixedEntries;
            m_pFixedEntries = NULL;
        }

        if (field_20 != NULL) {
            delete field_20;
            field_20 = NULL;
        }

        m_cFile.Close();
        m_bOpen = FALSE;
        field_38 = 0;
    }

    return TRUE;
}

// 0x789A20
DWORD CResFile::GetCacheSize()
{
    CString sDirName;
    CString sResFileName;
    CString sFileName;
    CString sResolvedResFileName;
    CString sUnused;
    WORD nDrive = 0;
    CFile cFile;
    char szCurrentDirectory[MAX_PATH];
    BYTE header[12];

    CResCache* pResCache = &(g_pChitin->cDimm.cResCache);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChDimm.cpp
    // __LINE__: 7326
    UTIL_ASSERT(pResCache != NULL);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChDimm.cpp
    // __LINE__: 7328
    UTIL_ASSERT(pResCache->m_bInitialized == TRUE);

    if (m_nRefCount > 0) {
        return 0;
    }

    // NOTE: The working directory is fetched but never used or restored.
    if (_getcwd(szCurrentDirectory, MAX_PATH) == NULL) {
        return 0;
    }

    sDirName = pResCache->m_sDirName;

    if (!g_pChitin->cDimm.GetResFileName(m_nIndex, sResFileName, nDrive, TRUE)) {
        return 0;
    }

    if (g_pChitin->cDimm.m_cKeyTable.m_bInitialized && m_nIndex < g_pChitin->cDimm.m_cKeyTable.m_nResFiles) {
        sFileName = reinterpret_cast<char*>(g_pChitin->cDimm.m_cKeyTable.m_pResFileNameEntries) + g_pChitin->cDimm.m_cKeyTable.m_pResFileNameEntries[m_nIndex].nFileNameOffset;
    }

    // Files on the hard drive (1) or already in the cache directory (0x200)
    // don't need any cache space.
    if ((nDrive & 1) != 0 || (nDrive & 0x200) != 0) {
        return 0;
    }

    if (!g_pChitin->lAliases.ResolveFileName(sResFileName, sResolvedResFileName)) {
        return 0;
    }

    if (!cFile.Open(sResolvedResFileName, CFile::OpenFlags::typeBinary | CFile::OpenFlags::shareDenyWrite, NULL)) {
        return 0;
    }

    DWORD nSize;
    if (cFile.Read(header, sizeof(header)) == sizeof(header)
        && _strnicmp(reinterpret_cast<char*>(header), "BIFCV1.0", 8) == 0) {
        // Compressed BIF, the header holds the uncompressed size.
        nSize = *reinterpret_cast<DWORD*>(header + 8);
    } else {
        nSize = static_cast<DWORD>(cFile.GetLength());
    }

    cFile.Close();

    return nSize;
}

// 0x789CE0
BOOL CResFile::Cache()
{
    CString sDirName;
    CString sResFileName;
    CString sFileName;
    CString sResolvedResFileName;
    CString sCacheFileName;
    WORD nDrive = 0;
    char szCurrentDirectory[MAX_PATH];

    CResCache* pResCache = &(g_pChitin->cDimm.cResCache);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChDimm.cpp
    // __LINE__: 7481
    UTIL_ASSERT(pResCache != NULL);

    // __FILE__: C:\Projects\Icewind2\src\chitin\ChDimm.cpp
    // __LINE__: 7483
    UTIL_ASSERT(pResCache->m_bInitialized == TRUE);

    if (m_nRefCount > 0) {
        return FALSE;
    }

    if (_getcwd(szCurrentDirectory, MAX_PATH) == NULL) {
        return FALSE;
    }

    sDirName = pResCache->m_sDirName;

    if (!g_pChitin->cDimm.GetResFileName(m_nIndex, sResFileName, nDrive, TRUE)) {
        return FALSE;
    }

    if (g_pChitin->cDimm.m_cKeyTable.m_bInitialized && m_nIndex < g_pChitin->cDimm.m_cKeyTable.m_nResFiles) {
        sFileName = reinterpret_cast<char*>(g_pChitin->cDimm.m_cKeyTable.m_pResFileNameEntries) + g_pChitin->cDimm.m_cKeyTable.m_pResFileNameEntries[m_nIndex].nFileNameOffset;
    }

    // Files on the hard drive are never cached.
    if ((nDrive & 1) != 0) {
        return FALSE;
    }

    if ((nDrive & 0x200) != 0) {
        // Already in the cache directory, just lock it and touch it.
        if (!AddCacheCount()) {
            return FALSE;
        }

        pResCache->AccessFileInCache(m_nIndex);

        EnterCriticalSection(&(g_pChitin->field_35C));
        pResCache->field_110 = 0;
        LeaveCriticalSection(&(g_pChitin->field_35C));

        return TRUE;
    }

    if (!g_pChitin->lAliases.ResolveFileName(sResFileName, sResolvedResFileName)) {
        return FALSE;
    }

    if (!g_pChitin->cDimm.WriteSetUp(sDirName + sFileName, sCacheFileName)) {
        return FALSE;
    }

    while (g_pChitin->m_bInSynchronousUpdate == TRUE || g_pChitin->m_bDisplayStale == TRUE) {
        SleepEx(5, FALSE);
    }

    if (!AddCacheCount()) {
        return FALSE;
    }

    if (!pResCache->CopyFile(m_nIndex, sFileName, sResolvedResFileName, sCacheFileName)) {
        EnterCriticalSection(&(g_pChitin->field_35C));

        if (m_nCacheCount > 1) {
            m_nCacheCount--;
        } else {
            m_nCacheCount = 0;
        }

        g_pChitin->cDimm.cResCache.field_110 = 0;
        LeaveCriticalSection(&(g_pChitin->field_35C));

        _chdir(szCurrentDirectory);

        return FALSE;
    }

    EnterCriticalSection(&(g_pChitin->field_35C));
    pResCache->field_110 = 0;
    LeaveCriticalSection(&(g_pChitin->field_35C));

    _chdir(szCurrentDirectory);

    return TRUE;
}

// 0x78A0F0
BOOLEAN CResFile::sub_78A0F0(CString& sResFileName)
{
    CString sUnused;
    CString sFileName;
    CString sBaseName;

    if (g_pChitin->cDimm.m_cKeyTable.m_bInitialized && m_nIndex < g_pChitin->cDimm.m_cKeyTable.m_nResFiles) {
        sFileName = reinterpret_cast<char*>(g_pChitin->cDimm.m_cKeyTable.m_pResFileNameEntries) + g_pChitin->cDimm.m_cKeyTable.m_pResFileNameEntries[m_nIndex].nFileNameOffset;
        sBaseName = sFileName;

        int nPos;
        while ((nPos = sBaseName.Find('\\')) != -1) {
            sBaseName = sBaseName.Right(sBaseName.GetLength() - nPos - 1);
        }

        sResFileName = sBaseName.Left(sBaseName.GetLength() - 4);

        return TRUE;
    }

    return FALSE;
}

// #guess
// 0x78A280
DWORD CResFile::GetFileSize(RESID resID)
{
    if (!m_bOpen) {
        return 0;
    }

    if (m_pHeader != NULL) {
        if ((resID & 0xFC000) != 0) {
            UINT nIndex = (resID >> 14) & 0x3F;
            if (nIndex <= m_pHeader->nFixedRes) {
                return m_pFixedEntries[nIndex - 1].nSize;
            }
        } else {
            UINT nIndex = resID & 0x3FFF;
            if (nIndex < m_pHeader->nVarRes) {
                return m_pVarEntries[nIndex].nSize;
            }
        }
    }

    if (field_20 != NULL) {
        // TODO: Object/structure?
        return reinterpret_cast<DWORD>(reinterpret_cast<unsigned char*>(field_20) + 2);
    }

    return 0;
}

// 0x78A2F0
BOOL CResFile::OpenFile()
{
    CString sResFileName;
    WORD nDrive = 0;
    CString sResolvedResFileName;

    if (m_bOpen) {
        CloseFile();
    }

    if (!g_pChitin->cDimm.GetResFileName(m_nIndex, sResFileName, nDrive, FALSE)) {
        return FALSE;
    }

    if (!g_pChitin->lAliases.ResolveFileName(sResFileName, sResolvedResFileName)) {
        sResolvedResFileName = sResFileName;
    }

    if (!m_cFile.Open(sResolvedResFileName, CFile::OpenFlags::typeBinary | CFile::OpenFlags::shareDenyWrite, NULL)) {
        return FALSE;
    }

    m_pHeader = new BIFF_HEADER;
    if (m_pHeader == NULL) {
        m_cFile.Close();
        return FALSE;
    }

    if (m_cFile.Read(m_pHeader, sizeof(BIFF_HEADER)) != sizeof(BIFF_HEADER)) {
        m_cFile.Close();
        delete m_pHeader;
        return FALSE;
    }

    if (memcmp(&(m_pHeader->dwFileType), "BIFF", 4) != 0 || memcmp(&(m_pHeader->dwVersion), "V1  ", 4) != 0) {
        if (memcmp(&(m_pHeader->dwFileType), "BIFC", 4) != 0) {
            m_cFile.Close();
            delete m_pHeader;
            return FALSE;
        }

        CString v34;
        CString v36;
        CString v37;
        CString v35;

        m_cFile.Close();

        v35 = g_pChitin->cDimm.cResCache.m_sDirName;

        if (!g_pChitin->cDimm.m_cKeyTable.m_bInitialized) {
            // NOTE: Leaking `m_pHeader`.
            return FALSE;
        }

        if (m_nIndex >= g_pChitin->cDimm.m_cKeyTable.m_nResFiles) {
            // NOTE: Leaking `m_pHeader`.
            return FALSE;
        }

        v34 = reinterpret_cast<char*>(g_pChitin->cDimm.m_cKeyTable.m_pResFileNameEntries) + g_pChitin->cDimm.m_cKeyTable.m_pResFileNameEntries[m_nIndex].nFileNameOffset;
        if (!g_pChitin->cDimm.WriteSetUp(v35 + v34, v36)) {
            // NOTE: Leaking `m_pHeader`.
            return FALSE;
        }

        if (!g_pChitin->lAliases.ResolveFileName(sResFileName, sResolvedResFileName)) {
            sResolvedResFileName = sResFileName;
        }

        if (!g_pChitin->cDimm.cResCache.CopyFile(m_nIndex, v34, sResolvedResFileName, v36)) {
            // NOTE: Leaking `m_pHeader`.
            return FALSE;
        }

        sResFileName = v35 + v34;
        if (!g_pChitin->lAliases.ResolveFileName(sResFileName, sResolvedResFileName)) {
            sResolvedResFileName = sResFileName;
        }

        if (!m_cFile.Open(sResolvedResFileName, CFile::OpenFlags::typeBinary | CFile::OpenFlags::shareDenyWrite, NULL)) {
            // NOTE: Leaking `m_pHeader`.
            return FALSE;
        }

        if (m_cFile.Read(m_pHeader, sizeof(BIFF_HEADER)) != sizeof(BIFF_HEADER)) {
            m_cFile.Close();
            delete m_pHeader;
            m_pHeader = NULL;
            return FALSE;
        }
    }

    m_cFile.Seek(m_pHeader->nTableOffset, CFile::SeekPosition::begin);

    if (m_pHeader->nVarRes != 0) {
        m_pVarEntries = new BIFF_VARTABLEENTRY[m_pHeader->nVarRes];
        if (m_pVarEntries == NULL) {
            m_cFile.Close();
            delete m_pHeader;
            return FALSE;
        }

        if (m_cFile.Read(m_pVarEntries, sizeof(BIFF_VARTABLEENTRY) * m_pHeader->nVarRes) < sizeof(BIFF_VARTABLEENTRY) * m_pHeader->nVarRes) {
            m_cFile.Close();
            delete m_pHeader;
            m_pHeader = NULL;
            delete m_pVarEntries;
            m_pVarEntries = NULL;
            return FALSE;
        }
    }

    if (m_pHeader->nFixedRes != 0) {
        m_pFixedEntries = new BIFF_FIXEDTABLEENTRY[m_pHeader->nFixedRes];
        if (m_pFixedEntries == NULL) {
            m_cFile.Close();
            delete m_pHeader;
            m_pHeader = NULL;

            if (m_pVarEntries != NULL) {
                delete m_pVarEntries;
                m_pVarEntries = NULL;
            }

            return FALSE;
        }

        if (m_cFile.Read(m_pFixedEntries, sizeof(BIFF_FIXEDTABLEENTRY) * m_pHeader->nFixedRes) < sizeof(BIFF_FIXEDTABLEENTRY) * m_pHeader->nFixedRes) {
            m_cFile.Close();
            delete m_pHeader;
            m_pHeader = NULL;

            if (m_pVarEntries != NULL) {
                delete m_pVarEntries;
                m_pVarEntries = NULL;
            }

            delete m_pFixedEntries;
            m_pFixedEntries = NULL;

            return FALSE;
        }
    }

    m_bOpen = TRUE;

    if (sResFileName.Left(2).Compare("cd") == 0) {
        field_38 = TRUE;
    }

    return TRUE;
}

// 0x78A7E0
UINT CResFile::ReadResource(RESID resID, LPVOID lpBuf, UINT nCount, UINT nOffset)
{
    if (lpBuf == NULL) {
        return 0;
    }

    if (!m_bOpen) {
        return 0;
    }

    if (m_pHeader != NULL) {
        if ((resID & 0xFC000) != 0) {
            UINT nIndex = (resID >> 14) & 0x3F;
            if (nIndex > m_pHeader->nFixedRes) {
                return 0;
            }

            BIFF_FIXEDTABLEENTRY* pEntry = &(m_pFixedEntries[nIndex - 1]);
            UINT nNumber = resID & 0x3FFF;
            if (nNumber >= pEntry->nNumber) {
                return 0;
            }

            if (nCount > pEntry->nSize) {
                nCount = pEntry->nSize;
            }

            m_cFile.Seek(nOffset + pEntry->nOffset + nNumber * pEntry->nSize, CFile::SeekPosition::begin);
            return m_cFile.Read(static_cast<unsigned char*>(lpBuf) + nOffset, nCount);
        } else {
            UINT nIndex = resID & 0x3FFF;
            if (nIndex < m_pHeader->nVarRes) {
                BIFF_VARTABLEENTRY* pEntry = &(m_pVarEntries[nIndex]);
                if (nCount > pEntry->nSize) {
                    nCount = pEntry->nSize;
                }

                m_cFile.Seek(nOffset + pEntry->nOffset, CFile::SeekPosition::begin);
                return m_cFile.Read(static_cast<unsigned char*>(lpBuf) + nOffset, nCount);
            }
            return 0;
        }
    }

    if (field_20 != NULL) {
        m_cFile.Seek(0, CFile::SeekPosition::begin);
        return m_cFile.Read(static_cast<unsigned char*>(lpBuf) + nOffset, nCount);
    }

    return 0;
}

// 0x78A900
void CResFile::UnCache()
{
    EnterCriticalSection(&(g_pChitin->field_35C));

    if (m_nCacheCount > 1) {
        m_nCacheCount -= 1;
    } else {
        m_nCacheCount = 0;
    }

    LeaveCriticalSection(&(g_pChitin->field_35C));

    g_pChitin->cDimm.cResCache.AccessFileInCache(m_nIndex);
}
