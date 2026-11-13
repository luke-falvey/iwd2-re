#include "CStore.h"

#include "CBaldurChitin.h"
#include "CInfGame.h"
#include "CItem.h"
#include "CUtil.h"

// 0x54B0E0
CStore::CStore()
{
    m_nBuyTypes = 0;
    m_nDrinks = 0;
    m_nSpells = 0;
    m_pBuyTypes = NULL;
    m_pDrinks = NULL;
    m_pSpells = NULL;
    memset(m_pVersion, 0, sizeof(m_pVersion));
    m_bLocalCopy = FALSE;
}

// 0x54B150
CStore::CStore(const CResRef& resRef)
{
    m_pBuyTypes = NULL;
    m_pDrinks = NULL;
    m_pSpells = NULL;
    memset(m_pVersion, 0, sizeof(m_pVersion));
    m_bLocalCopy = FALSE;
    SetResRef(resRef);
}

// 0x54B1E0
CStore::~CStore()
{
    while (!m_lInventory.IsEmpty()) {
        CStoreFileItem* pStoreItem = m_lInventory.RemoveHead();
        delete pStoreItem;
    }
    m_lInventory.RemoveAll();

    if (m_pBuyTypes != NULL) {
        delete m_pBuyTypes;
    }

    if (m_pDrinks != NULL) {
        delete m_pDrinks;
    }

    if (m_pSpells != NULL) {
        delete m_pSpells;
    }
}

// 0x54B2A0
void CStore::SetResRef(const CResRef& resRef)
{
    CStoreFile storeData;

    while (!m_lInventory.IsEmpty()) {
        CStoreFileItem* pStoreItem = m_lInventory.RemoveHead();
        delete pStoreItem;
    }
    m_lInventory.RemoveAll();

    if (m_pBuyTypes != NULL) {
        delete m_pBuyTypes;
    }

    if (m_pDrinks != NULL) {
        delete m_pDrinks;
    }

    if (m_pSpells != NULL) {
        delete m_pSpells;
    }

    m_resRef = resRef;

    storeData.SetResRef(resRef, TRUE, TRUE);

    BYTE* pData = reinterpret_cast<BYTE*>(storeData.GetData());

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 200
    UTIL_ASSERT(pData != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 209
    UTIL_ASSERT(pData != NULL);

    DWORD nData = storeData.GetDataSize();
    DWORD cnt = 0;

    memcpy(m_pVersion, pData, 8);
    cnt += 8;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 224
    UTIL_ASSERT(cnt <= nData);

    if (memcmp(m_pVersion, "STORV9.0", 8) == 0) {
        if ((storeData.pRes->GetID() & 0xFFF00000) < 0xFC000000) {
            m_bLocalCopy = FALSE;
        } else {
            CString sStoreDir;
            CString sTempDir;

            g_pChitin->cDimm.GetElementInDirectoryList(~storeData.pRes->GetID() >> 20, sStoreDir);
            g_pChitin->lAliases.ResolveFileName(g_pBaldurChitin->GetObjectGame()->m_sTempDir, sTempDir);
            m_bLocalCopy = sStoreDir == sTempDir;
        }

        memcpy(&m_header, pData + 8, sizeof(m_header));
        cnt += sizeof(m_header);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
        // __LINE__: 247
        UTIL_ASSERT(cnt <= nData);

        // NOTE: Original code is slightly different.
        for (DWORD nIndex = 0; nIndex < m_header.m_nInventoryCount; nIndex++) {
            CStoreFileItem* pItem = new CStoreFileItem;

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
            // __LINE__: 256
            UTIL_ASSERT(pItem);

            memcpy(pItem, pData + m_header.m_nInventoryOffset + sizeof(CStoreFileItem) * nIndex, sizeof(CStoreFileItem));
            m_lInventory.AddTail(pItem);

            cnt += sizeof(CStoreFileItem);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
            // __LINE__: 260
            UTIL_ASSERT(cnt <= nData);
        }
        m_header.m_nInventoryCount = 0;

        m_nBuyTypes = m_header.m_nBuyTypesCount;
        if (m_nBuyTypes != 0) {
            m_pBuyTypes = new DWORD[m_nBuyTypes];

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
            // __LINE__: 268
            UTIL_ASSERT(m_pBuyTypes != NULL);

            memcpy(m_pBuyTypes, pData + m_header.m_nbuyTypesOffset, sizeof(DWORD) * m_nBuyTypes);

            cnt += sizeof(DWORD) * m_nBuyTypes;

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
            // __LINE__: 271
            UTIL_ASSERT(cnt <= nData);
        }

        m_nDrinks = m_header.m_drinkCount;
        if (m_nDrinks != 0) {
            m_pDrinks = new CStoreFileDrinks[m_nDrinks];

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
            // __LINE__: 278
            UTIL_ASSERT(m_pDrinks != NULL);

            memcpy(m_pDrinks, pData + m_header.m_drinkOffset, sizeof(CStoreFileDrinks) * m_nDrinks);

            cnt += sizeof(CStoreFileDrinks) * m_nDrinks;

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
            // __LINE__: 281
            UTIL_ASSERT(cnt <= nData);
        }

        m_nSpells = m_header.m_spellCount;
        if (m_nSpells != 0) {
            m_pSpells = new CStoreFileSpell[m_nSpells];

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
            // __LINE__: 288
            UTIL_ASSERT(m_pSpells != NULL);

            memcpy(m_pSpells, pData + m_header.m_spellOffset, sizeof(CStoreFileSpell) * m_nSpells);

            cnt += sizeof(CStoreFileSpell) * m_nSpells;

            // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
            // __LINE__: 291
            UTIL_ASSERT(cnt <= nData);
        }

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
        // __LINE__: 294
        UTIL_ASSERT(cnt == nData);
    }

    storeData.Release();
}

// 0x54B9A0
void CStore::Marshal(const CString& sDirName)
{
    sub_54CF80();

    CStoreFile storeFile;

    DWORD nData = 8
        + sizeof(m_header)
        + sizeof(CStoreFileItem) * m_lInventory.GetCount()
        + sizeof(DWORD) * m_nBuyTypes
        + sizeof(CStoreFileDrinks) * m_nDrinks
        + sizeof(CStoreFileSpell) * m_nSpells;

    BYTE* pData = new BYTE[nData];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 339
    UTIL_ASSERT(pData != NULL);

    memset(pData, 0, nData);

    DWORD cnt = 0;

    memcpy(pData, "STORV9.0", 8);
    cnt += 8;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 346
    UTIL_ASSERT(cnt <= nData);

    memcpy(pData + cnt, &m_header, sizeof(m_header));
    cnt += sizeof(m_header);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 350
    UTIL_ASSERT(cnt <= nData);

    CStoreFileHeader* pHeader = reinterpret_cast<CStoreFileHeader*>(pData + 8);
    pHeader->m_nInventoryOffset = cnt;
    pHeader->m_nInventoryCount = m_lInventory.GetCount();

    POSITION pos = m_lInventory.GetHeadPosition();
    for (INT nIndex = 0; pos != NULL && nIndex < m_lInventory.GetCount(); nIndex++) {
        CStoreFileItem* pStoreItem = m_lInventory.GetNext(pos);

        memcpy(pData + cnt, pStoreItem, sizeof(CStoreFileItem));
        cnt += sizeof(CStoreFileItem);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
        // __LINE__: 359
        UTIL_ASSERT(cnt <= nData);
    }

    pHeader->m_nbuyTypesOffset = cnt;
    pHeader->m_nBuyTypesCount = m_nBuyTypes;

    memcpy(pData + cnt, m_pBuyTypes, sizeof(DWORD) * m_nBuyTypes);
    cnt += sizeof(DWORD) * m_nBuyTypes;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 366
    UTIL_ASSERT(cnt <= nData);

    pHeader->m_drinkOffset = cnt;
    pHeader->m_drinkCount = m_nDrinks;

    memcpy(pData + cnt, m_pDrinks, sizeof(CStoreFileDrinks) * m_nDrinks);
    cnt += sizeof(CStoreFileDrinks) * m_nDrinks;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 372
    UTIL_ASSERT(cnt <= nData);

    pHeader->m_spellOffset = cnt;
    pHeader->m_spellCount = m_nSpells;

    memcpy(pData + cnt, m_pSpells, sizeof(CStoreFileSpell) * m_nSpells);
    cnt += sizeof(CStoreFileSpell) * m_nSpells;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 378
    UTIL_ASSERT(cnt == nData);

    storeFile.SetResRef(m_resRef, TRUE, TRUE);

    storeFile.pRes->Write(sDirName, pData, nData);

    delete pData;
}

// 0x54BDC0
DWORD CStore::GetItemSellValue(CItem& cItem, unsigned char a2)
{
    DWORD dwBaseValue = cItem.GetBaseValue();

    if ((cItem.m_flags & 0x1) == 0) {
        return 1;
    }

    if (dwBaseValue <= 0) {
        return 0;
    }

    DWORD dwMarkDown;

    DWORD dwItemType = cItem.GetItemType();
    if (dwItemType == 34 || dwItemType == 10 || dwItemType == 1 || dwItemType == 70) {
        dwMarkDown = m_header.m_nSellMarkDown;
    } else {
        DWORD v1 = 0;
        BOOL v2 = FALSE;
        POSITION pos = m_lInventory.GetHeadPosition();
        while (pos != NULL) {
            CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
            if (cItem.GetResRef() == CResRef(pStoreItem->m_itemId)) {
                if ((pStoreItem->m_nStoreFlags & 0x1) != 0) {
                    dwMarkDown = 20;
                    v2 = TRUE;
                    break;
                }

                if (cItem.GetMaxStackable() > 1) {
                    v1 += pStoreItem->m_nInStock * pStoreItem->m_usageCount[0];
                } else {
                    v1 += pStoreItem->m_nInStock;
                }
            }

            m_lInventory.GetNext(pos);
        }

        if (!v2) {
            dwMarkDown = m_header.m_nSellMarkDown - v1 * m_header.m_nAdditionalMarkDown;
            if (static_cast<INT>(dwMarkDown) < 20) {
                dwMarkDown = 20;
            }
        }
    }

    if (cItem.GetMaxStackable() > 1) {
        dwBaseValue *= cItem.GetUsageCount(0);
    } else {
        INT nTotalCharges = 0;
        INT nChargesLeft = 0;
        for (INT nAbility = 0; nAbility < 3; nAbility++) {
            INT nMaxUsageCount = cItem.GetMaxUsageCount(nAbility);
            if (nMaxUsageCount > 0) {
                nTotalCharges += nMaxUsageCount;
                nChargesLeft += cItem.GetUsageCount(nAbility);
            }
        }

        if (nTotalCharges > 0) {
            dwBaseValue *= nChargesLeft;
            dwBaseValue /= nTotalCharges;
        }
    }

    DWORD dwValue = dwBaseValue * (a2 + dwMarkDown) / 100;
    if (dwValue <= 0) {
        dwValue = 1;
    }

    return dwValue;
}

// 0x54BF40
DWORD CStore::GetItemBuyValue(CItem& cItem, BYTE nCHR, SHORT nReputation, unsigned char a4)
{
    DWORD dwMarkUp = m_header.m_nBuyMarkUp;

    // NOTE: Unused.
    cItem.GetItemType();

    DWORD dwBaseValue = cItem.GetBaseValue();

    if (cItem.GetMaxStackable() > 1) {
        dwBaseValue *= cItem.GetUsageCount(0);
    } else {
        INT nTotalCharges = 0;
        INT nChargesLeft = 0;
        for (INT nAbility = 0; nAbility < 3; nAbility++) {
            INT nMaxUsageCount = cItem.GetMaxUsageCount(nAbility);
            if (nMaxUsageCount > 0) {
                nTotalCharges += nMaxUsageCount;
                nChargesLeft += cItem.GetUsageCount(nAbility);
            }
        }

        if (nTotalCharges > 0) {
            dwBaseValue *= nChargesLeft;
            dwBaseValue /= nTotalCharges;
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetRuleTables().AdjustStoreMarkup(nCHR, nReputation, dwMarkUp);

    if (dwMarkUp > a4) {
        dwMarkUp -= a4;
    } else {
        dwMarkUp = 0;
    }

    DWORD dwValue = dwMarkUp * dwBaseValue / 100;
    if (dwValue <= 0) {
        dwValue = 1;
    }

    return dwValue;
}

// 0x54C020
INT CStore::GetNumItems()
{
    INT nItems = 0;

    POSITION pos = m_lInventory.GetHeadPosition();
    while (pos != NULL) {
        CStoreFileItem* pStoreItem = m_lInventory.GetNext(pos);
        if ((pStoreItem->m_nStoreFlags & 0x1) != 0) {
            return INT_MAX;
        }

        nItems += pStoreItem->m_nInStock;
    }

    return nItems;
}

// 0x54C050
DWORD CStore::GetItemNumInStock(INT nIndex)
{
    POSITION pos = m_lInventory.FindIndex(nIndex);
    if (pos == NULL) {
        return 0;
    }

    CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
    if ((pStoreItem->m_nStoreFlags & 0x1) != 0) {
        // FIXME: Why?
        return 65535;
    }

    return pStoreItem->m_nInStock;
}

// 0x54C080
BOOL CStore::GetItem(INT nIndex, CItem& cItem)
{
    POSITION pos = m_lInventory.FindIndex(nIndex);
    if (pos == NULL) {
        return FALSE;
    }

    CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
    cItem.SetResRef(CResRef(pStoreItem->m_itemId), TRUE);
    cItem.m_wear = pStoreItem->m_wear;
    cItem.m_flags = pStoreItem->m_dynamicFlags;

    if (cItem.GetLoreValue() == 0 || (pStoreItem->m_nStoreFlags & 0x1) != 0) {
        cItem.m_flags |= 0x1;
    }

    if ((pStoreItem->m_nStoreFlags & 0x1) == 0 && cItem.GetLoreValue() > 0) {
        cItem.m_flags |= 0x8;
    }

    for (INT nAbility = 0; nAbility < 3; nAbility++) {
        cItem.SetUsageCount(nAbility, pStoreItem->m_usageCount[nAbility]);
    }

    return TRUE;
}

// 0x54C130
CResRef CStore::GetItemId(INT nIndex)
{
    POSITION pos = m_lInventory.FindIndex(nIndex);
    if (pos == NULL) {
        return CResRef("");
    }

    CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
    return CResRef(pStoreItem->m_itemId);
}

// 0x54C170
INT CStore::GetItemIndex(const CResRef& itemId)
{
    INT nIndex = 0;
    POSITION pos = m_lInventory.GetHeadPosition();
    while (pos != NULL) {
        CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
        if (itemId == CResRef(pStoreItem->m_itemId)) {
            return nIndex;
        }

        m_lInventory.GetNext(pos);
        nIndex++;
    }

    return INT_MAX;
}

// 0x54CA80
BOOL CStore::IsItemAllowed(CItem& cItem)
{
    // NOTE: Bags cannot be put into bags.
    if (m_header.m_nStoreType == 4 && cItem.GetItemType() == 58) {
        return FALSE;
    }

    for (DWORD nIndex = 0; nIndex < m_nBuyTypes; nIndex++) {
        if (m_pBuyTypes[nIndex] == cItem.GetItemType()) {
            return (cItem.GetFlagsFile() & 0x800) == 0;
        }
    }

    return FALSE;
}

// 0x54C1D0
INT CStore::RemoveItemExt(CResRef resRef, DWORD dwFlags, INT nIndex, INT nCount, BOOLEAN* pRemoved)
{
    if (resRef == "") {
        return 0;
    }

    if (m_header.m_nStoreType == 4) {
        return RemoveContainerItem(resRef, dwFlags, nIndex, nCount, pRemoved);
    }

    return RemoveStoreItem(resRef, dwFlags, nIndex, nCount, pRemoved);
}

// 0x54C230
INT CStore::RemoveStoreItem(CResRef resRef, DWORD dwFlags, INT nIndex, INT nCount, BOOLEAN* pRemoved)
{
    if (pRemoved != NULL) {
        *pRemoved = FALSE;
    }

    INT nCurrent = 0;
    POSITION pos = m_lInventory.GetHeadPosition();
    while (pos != NULL) {
        if (nIndex == -1 || nIndex == nCurrent) {
            CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);

            // NOTE: The flag is set on the stored item even when it does not
            // match.
            pStoreItem->m_dynamicFlags |= 0x1;

            if (CResRef(pStoreItem->m_itemId) == resRef
                && ((pStoreItem->m_dynamicFlags ^ dwFlags) & 0x1) == 0) {
                if ((pStoreItem->m_nStoreFlags & 0x1) != 0) {
                    return 1;
                }

                CItem cItem(resRef, 0, 0, 0, 0, 0);

                if (pStoreItem->m_nInStock > 0) {
                    if (nCount < pStoreItem->m_usageCount[0] && cItem.GetMaxStackable() > 1) {
                        pStoreItem->m_usageCount[0] -= nCount;
                    } else if (cItem.GetMaxStackable() > 1
                        && cItem.GetItemType() != 9
                        && cItem.GetItemType() != 71
                        && cItem.GetItemType() != 11
                        && cItem.GetItemType() != 13
                        && cItem.GetItemType() != 34
                        && cItem.GetItemType() != 0) {
                        INT nStacks;
                        if (cItem.GetMaxStackable() > 1) {
                            nStacks = nCount / pStoreItem->m_usageCount[0];
                        } else {
                            nStacks = nCount;
                        }

                        if (nStacks != 0) {
                            pStoreItem->m_nInStock -= nStacks;
                        } else {
                            pStoreItem->m_nInStock = pStoreItem->m_nInStock - 1;
                        }
                    } else {
                        pStoreItem->m_nInStock = pStoreItem->m_nInStock - nCount;
                    }

                    if (pStoreItem->m_nInStock == 0) {
                        if (pRemoved != NULL) {
                            *pRemoved = TRUE;
                        }

                        delete pStoreItem;
                        m_lInventory.RemoveAt(pos);
                    }

                    return 1;
                }
            }
        }

        m_lInventory.GetNext(pos);
        nCurrent++;
    }

    return 0;
}

// 0x54C460
INT CStore::RemoveContainerItem(CResRef resRef, DWORD dwFlags, INT nIndex, INT nCount, BOOLEAN* pRemoved)
{
    INT nCurrent = 0;

    if (pRemoved != NULL) {
        *pRemoved = FALSE;
    }

    POSITION pos = m_lInventory.GetHeadPosition();
    while (pos != NULL) {
        if (nIndex == -1 || nIndex == nCurrent) {
            CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
            if (CResRef(pStoreItem->m_itemId) == resRef
                && ((pStoreItem->m_dynamicFlags ^ dwFlags) & 0x1) == 0) {
                CItem cItem(resRef, 0, 0, 0, 0, 0);

                if (cItem.GetMaxStackable() > 1) {
                    pStoreItem->m_usageCount[0] -= nCount;
                    if (pStoreItem->m_usageCount[0] == 0) {
                        if (pRemoved != NULL) {
                            *pRemoved = TRUE;
                        }

                        delete pStoreItem;
                        m_lInventory.RemoveAt(pos);
                    }
                } else {
                    if (pRemoved != NULL) {
                        *pRemoved = TRUE;
                    }

                    delete pStoreItem;
                    m_lInventory.RemoveAt(pos);
                }

                return 1;
            }
        }

        m_lInventory.GetNext(pos);
        nCurrent++;
    }

    return 0;
}

// 0x54C5B0
INT CStore::AddItemExt(CItem& cItem, DWORD storeFlags)
{
    if (cItem.GetResRef() == "") {
        return -1;
    }

    if (m_header.m_nStoreType == 4) {
        return AddContainerItem(cItem);
    }

    return AddStoreItem(cItem);
}

// 0x54C5F0
INT CStore::AddContainerItem(CItem& cItem)
{
    // NOTE: Unused.
    CResRef resRef;

    INT nCount;
    if (cItem.GetMaxStackable() > 1) {
        nCount = cItem.GetUsageCount(0);
    } else {
        nCount = 0;
    }

    INT nIndex = 0;
    POSITION pos = m_lInventory.GetHeadPosition();
    while (pos != NULL) {
        CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
        if (cItem.GetResRef() == CResRef(pStoreItem->m_itemId)
            && pStoreItem->m_dynamicFlags == cItem.m_flags
            && nCount != 0) {
            pStoreItem->m_usageCount[0] += nCount;
            if (pStoreItem->m_usageCount[0] > cItem.GetMaxStackable()) {
                nCount = pStoreItem->m_usageCount[0] - cItem.GetMaxStackable();
                pStoreItem->m_usageCount[0] = cItem.GetMaxStackable();
            } else {
                break;
            }
        }

        m_lInventory.GetNext(pos);
        nIndex++;
    }

    if (pos != NULL) {
        return nIndex;
    }

    if (nIndex >= m_header.m_nCapacity) {
        return -1;
    }

    CStoreFileItem* pStoreItem = new CStoreFileItem();

    cItem.GetResRef().GetResRef(pStoreItem->m_itemId);
    pStoreItem->m_wear = cItem.m_wear;
    pStoreItem->m_dynamicFlags = cItem.m_flags;
    pStoreItem->m_nInStock = 1;
    pStoreItem->m_nStoreFlags = 0;

    for (INT nAbility = 0; nAbility < 3; nAbility++) {
        if (nCount > 0 && nAbility == 0) {
            pStoreItem->m_usageCount[0] = nCount;
        } else {
            pStoreItem->m_usageCount[nAbility] = cItem.GetUsageCount(nAbility);
        }
    }

    m_lInventory.AddTail(pStoreItem);

    return m_lInventory.GetCount() - 1;
}

// 0x54C770
INT CStore::AddStoreItem(CItem& cItem)
{
    // NOTE: Unused.
    CResRef resRef;

    // NOTE: The original shares one stack slot between `posOverflow` and
    // `nUsageCount`.
    POSITION posOverflow = NULL;

    INT nCount;
    if (cItem.GetMaxStackable() > 1) {
        nCount = cItem.GetUsageCount(0);
    } else {
        nCount = 0;
    }

    INT nIndex = 0;
    POSITION pos = m_lInventory.GetHeadPosition();
    while (pos != NULL) {
        CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
        if (cItem.GetResRef() == CResRef(pStoreItem->m_itemId)) {
            if ((pStoreItem->m_nStoreFlags & 0x1) != 0) {
                break;
            }

            if (nCount == 0) {
                pStoreItem->m_nInStock++;
                break;
            }

            if (cItem.GetItemType() == 9
                || cItem.GetItemType() == 71
                || cItem.GetItemType() == 11
                || cItem.GetItemType() == 13
                || cItem.GetItemType() == 34
                || cItem.GetItemType() == 0) {
                pStoreItem->m_nInStock += cItem.GetUsageCount(0);
                break;
            }

            if (pStoreItem->m_nInStock <= 1 || pStoreItem->m_usageCount[0] >= cItem.GetMaxStackable()) {
                pStoreItem->m_usageCount[0] += nCount;
                if (pStoreItem->m_usageCount[0] <= cItem.GetMaxStackable()) {
                    break;
                }

                if (posOverflow != NULL) {
                    pStoreItem->m_usageCount[0] -= cItem.GetMaxStackable();
                    m_lInventory.GetAt(posOverflow)->m_nInStock++;
                    break;
                }

                nCount = pStoreItem->m_usageCount[0] - cItem.GetMaxStackable();
                pStoreItem->m_usageCount[0] = cItem.GetMaxStackable();
                posOverflow = pos;

                if (nCount == cItem.GetMaxStackable()) {
                    pStoreItem->m_nInStock++;
                    return nIndex;
                }

                if (nCount == 0) {
                    return nIndex;
                }
            }
        }

        m_lInventory.GetNext(pos);
        nIndex++;
    }

    if (pos != NULL) {
        return nIndex;
    }

    CStoreFileItem* pStoreItem = new CStoreFileItem();

    cItem.GetResRef().GetResRef(pStoreItem->m_itemId);
    pStoreItem->m_wear = cItem.m_wear;
    pStoreItem->m_dynamicFlags = cItem.m_flags;
    pStoreItem->m_nStoreFlags = 0;

    INT nUsageCount = -1;

    if (cItem.GetMaxStackable() > 1
        && (cItem.GetItemType() == 9
            || cItem.GetItemType() == 71
            || cItem.GetItemType() == 11
            || cItem.GetItemType() == 13
            || cItem.GetItemType() == 34
            || cItem.GetItemType() == 0)) {
        nUsageCount = cItem.GetUsageCount(0);
        pStoreItem->m_usageCount[0] = 1;
        pStoreItem->m_nInStock = cItem.GetUsageCount(0);
    } else {
        if (cItem.GetMaxStackable() > 1) {
            pStoreItem->m_usageCount[0] = cItem.GetUsageCount(0);
        } else {
            pStoreItem->m_usageCount[0] = cItem.GetMaxUsageCount(0);
        }
        pStoreItem->m_nInStock = 1;
    }

    for (INT nAbility = 1; nAbility < 3; nAbility++) {
        if (cItem.GetMaxStackable() > 1) {
            pStoreItem->m_usageCount[nAbility] = cItem.GetUsageCount(nAbility);
        } else {
            pStoreItem->m_usageCount[nAbility] = cItem.GetMaxUsageCount(nAbility);
        }
    }

    m_lInventory.AddTail(pStoreItem);

    if (nUsageCount != -1) {
        cItem.SetUsageCount(0, nUsageCount);
    }

    return nIndex;
}

// 0x54CAF0
BOOL CStore::IsValidIdentifyItem(CItem* pItem)
{
    for (DWORD nIndex = 0; nIndex < m_nBuyTypes; nIndex++) {
        if (m_pBuyTypes[nIndex] == pItem->GetItemType()) {
            return (pItem->m_flags & 0x1) == 0
                && m_header.m_nLore >= pItem->GetLoreValue();
        }
    }
    return FALSE;
}

// 0x54CB60
void CStore::CompressItems()
{
    POSITION pos = m_lInventory.GetHeadPosition();
    while (pos != NULL) {
        CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
        if (pStoreItem->m_nInStock == 0 && (pStoreItem->m_nStoreFlags & 0x1) == 0) {
            POSITION posToRemove = pos;
            m_lInventory.GetNext(pos);
            m_lInventory.RemoveAt(posToRemove);
            delete pStoreItem;
        } else {
            m_lInventory.GetNext(pos);
        }
    }
}

// 0x54CBB0
BOOL CStore::GetSpell(INT nIndex, CResRef& spellResRef, DWORD& dwCost)
{
    // NOTE: Signed compare.
    if (nIndex < 0 || nIndex >= static_cast<INT>(m_nSpells)) {
        return FALSE;
    }

    spellResRef = m_pSpells[nIndex].m_spell;
    dwCost = m_pSpells[nIndex].m_cost;
    return TRUE;
}

// 0x54CC10
BOOL CStore::GetDrink(INT nIndex, STRREF& strName, DWORD& dwCost, DWORD& dwRumorChance)
{
    // NOTE: Signed compare.
    if (nIndex < 0 || nIndex >= static_cast<INT>(m_nDrinks)) {
        return FALSE;
    }

    strName = m_pDrinks[nIndex].m_strName;
    dwCost = m_pDrinks[nIndex].m_nCost;
    dwRumorChance = m_pDrinks[nIndex].m_nRumorChance;
    return TRUE;
}

// 0x54CCB0
void CStore::InvalidateStore(const CResRef& resRef)
{
    CStoreFile storeData;

    storeData.SetResRef(resRef, TRUE, TRUE);

    BYTE* pData = reinterpret_cast<BYTE*>(storeData.GetData());

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 1565
    UTIL_ASSERT(pData != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CStore.cpp
    // __LINE__: 1566
    UTIL_ASSERT(storeData.GetDataSize() >= 2*sizeof(DWORD));

    if (memcmp(pData, "STORV9.0", 8) == 0) {
        if ((storeData.pRes->GetID() & 0xFFF00000) >= 0xFC000000) {
            CString sStoreDir;
            CString sTempDir;

            g_pChitin->cDimm.GetElementInDirectoryList(~storeData.pRes->GetID() >> 20, sStoreDir);
            g_pChitin->lAliases.ResolveFileName(g_pBaldurChitin->GetObjectGame()->m_sTempDir, sTempDir);

            if (sStoreDir == sTempDir) {
                pData[3] = 'O';
                storeData.pRes->Write(g_pBaldurChitin->GetObjectGame()->m_sTempDir, pData, 8);
            }
        }
    }

    storeData.Release();
}

// 0x54CF80
void CStore::sub_54CF80()
{
    if (m_lInventory.GetCount() != 0) {
        POSITION pos = m_lInventory.GetHeadPosition();
        for (INT nIndex = 0; nIndex < m_lInventory.GetCount(); nIndex++) {
            POSITION posCurrent = pos;
            CStoreFileItem* pStoreItem = m_lInventory.GetNext(pos);
            if (pStoreItem != NULL && pStoreItem->m_nInStock == 0) {
                delete pStoreItem;
                m_lInventory.RemoveAt(posCurrent);
            }
        }
    }
}

// 0x54CFE0
BOOLEAN CStore::DoItemRandomize(INT nIndex, CItem& cItem)
{
    CString sResRef(reinterpret_cast<const char*>(cItem.GetResRef().GetResRef()));

    g_pBaldurChitin->GetObjectGame()->GetRuleTables().sub_5463E0(sResRef);

    if (strcmp(sResRef, "") == 0) {
        return FALSE;
    }

    cItem.SetResRef(CResRef(sResRef), TRUE);

    if (cItem.GetResRef() == "") {
        CString sMessage;
        sMessage.Format("Random Treasure Item not found - %s", static_cast<LPCSTR>(sResRef));
        return FALSE;
    }

    MergeStoreItemFlags(nIndex, cItem);

    return TRUE;
}

// 0x54D130
BOOLEAN CStore::MergeStoreItemFlags(INT nIndex, CItem& cItem)
{
    POSITION pos = m_lInventory.FindIndex(nIndex);
    if (pos == NULL) {
        return FALSE;
    }

    CStoreFileItem* pStoreItem = m_lInventory.GetAt(pos);
    cItem.m_wear = pStoreItem->m_wear;
    cItem.m_flags = pStoreItem->m_dynamicFlags;

    if (cItem.GetLoreValue() == 0 || (pStoreItem->m_nStoreFlags & 0x1) != 0) {
        cItem.m_flags |= 0x1;
    }

    if ((pStoreItem->m_nStoreFlags & 0x1) == 0 && cItem.GetLoreValue() > 0) {
        cItem.m_flags |= 0x8;
    }

    for (INT nAbility = 0; nAbility < 3; nAbility++) {
        cItem.SetUsageCount(nAbility, pStoreItem->m_usageCount[nAbility]);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x54CC70
void* CStoreFile::GetData()
{
    if (pRes != NULL) {
        return pRes->Demand();
    } else {
        return NULL;
    }
}

// 0x54CC80
DWORD CStoreFile::GetDataSize()
{
    if (pRes == NULL) {
        return 0;
    }

    if (pRes->Demand() == NULL) {
        return 0;
    }

    DWORD nSize = pRes->m_nResSizeActual;

    pRes->Release();

    return nSize;
}

// 0x5C7B00
void CStoreFile::ReleaseData()
{
    if (pRes != NULL) {
        pRes->Release();
    }
}
