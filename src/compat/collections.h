#ifndef IWD2RE_COMPAT_COLLECTIONS_H_
#define IWD2RE_COMPAT_COLLECTIONS_H_

// Replacements for the MFC collection classes the game uses: lists, arrays
// and maps, with `POSITION` iteration.
//
// The objects have the same members in the same order as MFC's, and list
// nodes are {next, previous, element}, because game code reads list nodes
// through a `POSITION` and the game's classes must keep their sizes.
// Like MFC, lists and maps take their nodes from blocks that are only
// returned to the heap when the collection becomes empty, so a `POSITION`
// stays readable (though stale) after its element is removed.

#include <new>
#include <stdlib.h>
#include <string.h>

struct __POSITION {
};
typedef __POSITION* POSITION;

#define BEFORE_START_POSITION ((POSITION)-1L)

// A block of nodes. Blocks of one collection form a chain.
struct CCompatBlock {
    CCompatBlock* pNext;

    void* data() { return this + 1; }

    // Allocates a block for `nCount` elements of `cbElement` bytes and puts
    // it at the head of the chain.
    static CCompatBlock* Create(CCompatBlock*& pHead, size_t nCount, size_t cbElement)
    {
        CCompatBlock* p = static_cast<CCompatBlock*>(malloc(sizeof(CCompatBlock) + nCount * cbElement));
        if (p == NULL) {
            throw std::bad_alloc();
        }
        p->pNext = pHead;
        pHead = p;
        return p;
    }

    void FreeChain()
    {
        CCompatBlock* p = this;
        while (p != NULL) {
            CCompatBlock* pNext = p->pNext;
            free(p);
            p = pNext;
        }
    }
};

template <class TYPE, class ARG_TYPE>
inline BOOL CompareElements(const TYPE* pElement1, const ARG_TYPE* pElement2)
{
    return *pElement1 == *pElement2;
}

// -----------------------------------------------------------------------------
// CList
// -----------------------------------------------------------------------------

template <class TYPE, class ARG_TYPE = const TYPE&>
class CList : public CObject {
protected:
    struct CNode {
        CNode* pNext;
        CNode* pPrev;
        TYPE data;
    };

public:
    CList(INT_PTR nBlockSize = 10)
    {
        m_nCount = 0;
        m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
        m_pBlocks = NULL;
        m_nBlockSize = nBlockSize;
    }

    ~CList() override { RemoveAll(); }

    INT_PTR GetCount() const { return m_nCount; }
    INT_PTR GetSize() const { return m_nCount; }
    BOOL IsEmpty() const { return m_nCount == 0; }

    // Head and tail of an empty list do not exist: asking for them throws.
    TYPE& GetHead() { return Checked(m_pNodeHead)->data; }
    const TYPE& GetHead() const { return Checked(m_pNodeHead)->data; }
    TYPE& GetTail() { return Checked(m_pNodeTail)->data; }
    const TYPE& GetTail() const { return Checked(m_pNodeTail)->data; }

    TYPE RemoveHead()
    {
        CNode* pOldNode = Checked(m_pNodeHead);
        TYPE returnValue = pOldNode->data;
        m_pNodeHead = pOldNode->pNext;
        if (m_pNodeHead != NULL) {
            m_pNodeHead->pPrev = NULL;
        } else {
            m_pNodeTail = NULL;
        }
        FreeNode(pOldNode);
        return returnValue;
    }

    TYPE RemoveTail()
    {
        CNode* pOldNode = Checked(m_pNodeTail);
        TYPE returnValue = pOldNode->data;
        m_pNodeTail = pOldNode->pPrev;
        if (m_pNodeTail != NULL) {
            m_pNodeTail->pNext = NULL;
        } else {
            m_pNodeHead = NULL;
        }
        FreeNode(pOldNode);
        return returnValue;
    }

    POSITION AddHead(ARG_TYPE newElement)
    {
        CNode* pNewNode = NewNode(NULL, m_pNodeHead);
        pNewNode->data = newElement;
        if (m_pNodeHead != NULL) {
            m_pNodeHead->pPrev = pNewNode;
        } else {
            m_pNodeTail = pNewNode;
        }
        m_pNodeHead = pNewNode;
        return reinterpret_cast<POSITION>(pNewNode);
    }

    POSITION AddTail(ARG_TYPE newElement)
    {
        CNode* pNewNode = NewNode(m_pNodeTail, NULL);
        pNewNode->data = newElement;
        if (m_pNodeTail != NULL) {
            m_pNodeTail->pNext = pNewNode;
        } else {
            m_pNodeHead = pNewNode;
        }
        m_pNodeTail = pNewNode;
        return reinterpret_cast<POSITION>(pNewNode);
    }

    // Puts the elements of another list in front of this one, in order.
    void AddHead(CList* pNewList)
    {
        POSITION pos = pNewList->GetTailPosition();
        while (pos != NULL) {
            AddHead(pNewList->GetPrev(pos));
        }
    }

    // Appends the elements of another list, in order.
    void AddTail(CList* pNewList)
    {
        POSITION pos = pNewList->GetHeadPosition();
        while (pos != NULL) {
            AddTail(pNewList->GetNext(pos));
        }
    }

    void RemoveAll()
    {
        for (CNode* pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext) {
            pNode->data.~TYPE();
        }
        m_nCount = 0;
        m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
        if (m_pBlocks != NULL) {
            m_pBlocks->FreeChain();
            m_pBlocks = NULL;
        }
    }

    POSITION GetHeadPosition() const { return reinterpret_cast<POSITION>(m_pNodeHead); }
    POSITION GetTailPosition() const { return reinterpret_cast<POSITION>(m_pNodeTail); }

    // Return the element at the position and move the position on; it
    // becomes NULL after the last (first) element.
    TYPE& GetNext(POSITION& rPosition)
    {
        CNode* pNode = reinterpret_cast<CNode*>(rPosition);
        rPosition = reinterpret_cast<POSITION>(pNode->pNext);
        return pNode->data;
    }
    const TYPE& GetNext(POSITION& rPosition) const
    {
        CNode* pNode = reinterpret_cast<CNode*>(rPosition);
        rPosition = reinterpret_cast<POSITION>(pNode->pNext);
        return pNode->data;
    }
    TYPE& GetPrev(POSITION& rPosition)
    {
        CNode* pNode = reinterpret_cast<CNode*>(rPosition);
        rPosition = reinterpret_cast<POSITION>(pNode->pPrev);
        return pNode->data;
    }
    const TYPE& GetPrev(POSITION& rPosition) const
    {
        CNode* pNode = reinterpret_cast<CNode*>(rPosition);
        rPosition = reinterpret_cast<POSITION>(pNode->pPrev);
        return pNode->data;
    }

    TYPE& GetAt(POSITION position) { return reinterpret_cast<CNode*>(position)->data; }
    const TYPE& GetAt(POSITION position) const { return reinterpret_cast<CNode*>(position)->data; }
    void SetAt(POSITION pos, ARG_TYPE newElement) { reinterpret_cast<CNode*>(pos)->data = newElement; }

    void RemoveAt(POSITION position)
    {
        CNode* pOldNode = reinterpret_cast<CNode*>(position);
        if (pOldNode == m_pNodeHead) {
            m_pNodeHead = pOldNode->pNext;
        } else {
            pOldNode->pPrev->pNext = pOldNode->pNext;
        }
        if (pOldNode == m_pNodeTail) {
            m_pNodeTail = pOldNode->pPrev;
        } else {
            pOldNode->pNext->pPrev = pOldNode->pPrev;
        }
        FreeNode(pOldNode);
    }

    // A NULL position inserts at the head.
    POSITION InsertBefore(POSITION position, ARG_TYPE newElement)
    {
        if (position == NULL) {
            return AddHead(newElement);
        }
        CNode* pOldNode = reinterpret_cast<CNode*>(position);
        CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
        pNewNode->data = newElement;
        if (pOldNode->pPrev != NULL) {
            pOldNode->pPrev->pNext = pNewNode;
        } else {
            m_pNodeHead = pNewNode;
        }
        pOldNode->pPrev = pNewNode;
        return reinterpret_cast<POSITION>(pNewNode);
    }

    // A NULL position inserts at the tail.
    POSITION InsertAfter(POSITION position, ARG_TYPE newElement)
    {
        if (position == NULL) {
            return AddTail(newElement);
        }
        CNode* pOldNode = reinterpret_cast<CNode*>(position);
        CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
        pNewNode->data = newElement;
        if (pOldNode->pNext != NULL) {
            pOldNode->pNext->pPrev = pNewNode;
        } else {
            m_pNodeTail = pNewNode;
        }
        pOldNode->pNext = pNewNode;
        return reinterpret_cast<POSITION>(pNewNode);
    }

    // Position of the first equal element, searching from the head or from
    // the element after `startAfter`. NULL when there is none.
    POSITION Find(ARG_TYPE searchValue, POSITION startAfter = NULL) const
    {
        CNode* pNode = reinterpret_cast<CNode*>(startAfter);
        if (pNode == NULL) {
            pNode = m_pNodeHead;
        } else {
            pNode = pNode->pNext;
        }
        for (; pNode != NULL; pNode = pNode->pNext) {
            if (CompareElements<TYPE>(&pNode->data, &searchValue)) {
                return reinterpret_cast<POSITION>(pNode);
            }
        }
        return NULL;
    }

    // Position of the element with the zero-based index, or NULL when the
    // index is outside the list.
    POSITION FindIndex(INT_PTR nIndex) const
    {
        if (nIndex >= m_nCount || nIndex < 0) {
            return NULL;
        }
        CNode* pNode = m_pNodeHead;
        while (nIndex--) {
            pNode = pNode->pNext;
        }
        return reinterpret_cast<POSITION>(pNode);
    }

protected:
    CNode* m_pNodeHead;
    CNode* m_pNodeTail;
    INT_PTR m_nCount;
    CNode* m_pNodeFree;
    CCompatBlock* m_pBlocks;
    INT_PTR m_nBlockSize;

    static CNode* Checked(CNode* pNode)
    {
        if (pNode == NULL) {
            AfxThrowInvalidArgException();
        }
        return pNode;
    }

    CNode* NewNode(CNode* pPrev, CNode* pNext)
    {
        if (m_pNodeFree == NULL) {
            // Add a block and chain its nodes into the free list so that the
            // first node of the block is handed out first.
            CCompatBlock* pNewBlock = CCompatBlock::Create(m_pBlocks, m_nBlockSize, sizeof(CNode));
            CNode* pNode = static_cast<CNode*>(pNewBlock->data()) + m_nBlockSize - 1;
            for (INT_PTR i = m_nBlockSize - 1; i >= 0; i--, pNode--) {
                pNode->pNext = m_pNodeFree;
                m_pNodeFree = pNode;
            }
        }

        CNode* pNode = m_pNodeFree;
        m_pNodeFree = m_pNodeFree->pNext;
        pNode->pPrev = pPrev;
        pNode->pNext = pNext;
        m_nCount++;

        // Elements start zero-filled and default-constructed.
        memset(static_cast<void*>(&pNode->data), 0, sizeof(TYPE));
        ::new (static_cast<void*>(&pNode->data)) TYPE;
        return pNode;
    }

    void FreeNode(CNode* pNode)
    {
        pNode->data.~TYPE();
        pNode->pNext = m_pNodeFree;
        m_pNodeFree = pNode;
        m_nCount--;

        // All blocks go back to the heap when the list becomes empty.
        if (m_nCount == 0) {
            RemoveAll();
        }
    }
};

class CPtrList : public CList<void*, void*> {
public:
    CPtrList(INT_PTR nBlockSize = 10)
        : CList<void*, void*>(nBlockSize)
    {
    }
};

class CObList : public CList<CObject*, CObject*> {
public:
    CObList(INT_PTR nBlockSize = 10)
        : CList<CObject*, CObject*>(nBlockSize)
    {
    }
};

class CStringList : public CList<CString, const char*> {
    typedef CList<CString, const char*> Base;

public:
    CStringList(INT_PTR nBlockSize = 10)
        : Base(nBlockSize)
    {
    }

    using Base::AddHead;
    using Base::AddTail;
    using Base::InsertAfter;
    using Base::InsertBefore;
    using Base::SetAt;

    // Taking a `CString` shares its buffer instead of copying the text.
    POSITION AddHead(const CString& newElement)
    {
        POSITION pos = Base::AddHead(static_cast<const char*>(NULL));
        GetAt(pos) = newElement;
        return pos;
    }
    POSITION AddTail(const CString& newElement)
    {
        POSITION pos = Base::AddTail(static_cast<const char*>(NULL));
        GetAt(pos) = newElement;
        return pos;
    }
    POSITION InsertBefore(POSITION position, const CString& newElement)
    {
        POSITION pos = Base::InsertBefore(position, static_cast<const char*>(NULL));
        GetAt(pos) = newElement;
        return pos;
    }
    POSITION InsertAfter(POSITION position, const CString& newElement)
    {
        POSITION pos = Base::InsertAfter(position, static_cast<const char*>(NULL));
        GetAt(pos) = newElement;
        return pos;
    }
    void SetAt(POSITION pos, const CString& newElement) { GetAt(pos) = newElement; }
};

// A list of pointers of one type on top of `CPtrList` or `CObList`.
template <class BASE_CLASS, class TYPE>
class CTypedPtrList : public BASE_CLASS {
public:
    CTypedPtrList(INT_PTR nBlockSize = 10)
        : BASE_CLASS(nBlockSize)
    {
    }

    TYPE& GetHead() { return (TYPE&)BASE_CLASS::GetHead(); }
    TYPE GetHead() const { return (TYPE)BASE_CLASS::GetHead(); }
    TYPE& GetTail() { return (TYPE&)BASE_CLASS::GetTail(); }
    TYPE GetTail() const { return (TYPE)BASE_CLASS::GetTail(); }

    TYPE RemoveHead() { return (TYPE)BASE_CLASS::RemoveHead(); }
    TYPE RemoveTail() { return (TYPE)BASE_CLASS::RemoveTail(); }

    TYPE& GetNext(POSITION& rPosition) { return (TYPE&)BASE_CLASS::GetNext(rPosition); }
    TYPE GetNext(POSITION& rPosition) const { return (TYPE)BASE_CLASS::GetNext(rPosition); }
    TYPE& GetPrev(POSITION& rPosition) { return (TYPE&)BASE_CLASS::GetPrev(rPosition); }
    TYPE GetPrev(POSITION& rPosition) const { return (TYPE)BASE_CLASS::GetPrev(rPosition); }

    TYPE& GetAt(POSITION position) { return (TYPE&)BASE_CLASS::GetAt(position); }
    TYPE GetAt(POSITION position) const { return (TYPE)BASE_CLASS::GetAt(position); }
    void SetAt(POSITION pos, TYPE newElement) { BASE_CLASS::SetAt(pos, newElement); }

    POSITION AddHead(TYPE newElement) { return BASE_CLASS::AddHead(newElement); }
    POSITION AddTail(TYPE newElement) { return BASE_CLASS::AddTail(newElement); }
    void AddHead(CTypedPtrList<BASE_CLASS, TYPE>* pNewList) { BASE_CLASS::AddHead(pNewList); }
    void AddTail(CTypedPtrList<BASE_CLASS, TYPE>* pNewList) { BASE_CLASS::AddTail(pNewList); }
};

// -----------------------------------------------------------------------------
// CArray
// -----------------------------------------------------------------------------

// The array code shared by `CArray` and the fixed-type arrays. They differ in
// one detail, EXACT_FIRST: where the first allocation of a `CArray` is at
// least the grow-by amount, that of the fixed-type arrays is exactly the size
// asked for.
template <class TYPE, class ARG_TYPE, bool EXACT_FIRST>
class CCompatArray : public CObject {
public:
    CCompatArray()
    {
        m_pData = NULL;
        m_nSize = m_nMaxSize = m_nGrowBy = 0;
    }

    ~CCompatArray() override
    {
        if (m_pData != NULL) {
            Destroy(m_pData, m_nSize);
            free(m_pData);
        }
    }

    INT_PTR GetSize() const { return m_nSize; }
    INT_PTR GetCount() const { return m_nSize; }
    BOOL IsEmpty() const { return m_nSize == 0; }
    INT_PTR GetUpperBound() const { return m_nSize - 1; }

    // Changes the number of elements. New elements are zero-filled and
    // default-constructed. `nGrowBy` (when not -1) sets how many elements
    // the storage grows by; 0 picks an amount from the current size.
    void SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1)
    {
        if (nNewSize < 0) {
            AfxThrowInvalidArgException();
        }
        if (nGrowBy >= 0) {
            m_nGrowBy = nGrowBy;
        }

        if (nNewSize == 0) {
            if (m_pData != NULL) {
                Destroy(m_pData, m_nSize);
                free(m_pData);
                m_pData = NULL;
            }
            m_nSize = m_nMaxSize = 0;
        } else if (m_pData == NULL) {
            INT_PTR nAllocSize = (EXACT_FIRST || nNewSize > m_nGrowBy) ? nNewSize : m_nGrowBy;
            m_pData = Allocate(nAllocSize);
            Construct(m_pData, nNewSize);
            m_nSize = nNewSize;
            m_nMaxSize = nAllocSize;
        } else if (nNewSize <= m_nMaxSize) {
            if (nNewSize > m_nSize) {
                Construct(m_pData + m_nSize, nNewSize - m_nSize);
            } else if (m_nSize > nNewSize) {
                Destroy(m_pData + nNewSize, m_nSize - nNewSize);
            }
            m_nSize = nNewSize;
        } else {
            INT_PTR nGrowArrayBy = m_nGrowBy;
            if (nGrowArrayBy == 0) {
                // An eighth of the size, between 4 and 1024 elements.
                nGrowArrayBy = m_nSize / 8;
                nGrowArrayBy = nGrowArrayBy < 4 ? 4 : (nGrowArrayBy > 1024 ? 1024 : nGrowArrayBy);
            }
            INT_PTR nNewMax = nNewSize < m_nMaxSize + nGrowArrayBy ? m_nMaxSize + nGrowArrayBy : nNewSize;

            // Elements are moved as raw memory, without copy construction.
            TYPE* pNewData = Allocate(nNewMax);
            memcpy(static_cast<void*>(pNewData), static_cast<const void*>(m_pData), m_nSize * sizeof(TYPE));
            Construct(pNewData + m_nSize, nNewSize - m_nSize);
            free(m_pData);
            m_pData = pNewData;
            m_nSize = nNewSize;
            m_nMaxSize = nNewMax;
        }
    }

    // Gives back storage above the current size.
    void FreeExtra()
    {
        if (m_nSize != m_nMaxSize) {
            TYPE* pNewData = NULL;
            if (m_nSize != 0) {
                pNewData = Allocate(m_nSize);
                memcpy(static_cast<void*>(pNewData), static_cast<const void*>(m_pData), m_nSize * sizeof(TYPE));
            }
            free(m_pData);
            m_pData = pNewData;
            m_nMaxSize = m_nSize;
        }
    }

    void RemoveAll() { SetSize(0, -1); }

    // An index outside the array throws, as in MFC.
    const TYPE& GetAt(INT_PTR nIndex) const { return m_pData[Checked(nIndex)]; }
    TYPE& GetAt(INT_PTR nIndex) { return m_pData[Checked(nIndex)]; }
    void SetAt(INT_PTR nIndex, ARG_TYPE newElement) { m_pData[Checked(nIndex)] = newElement; }
    const TYPE& ElementAt(INT_PTR nIndex) const { return m_pData[Checked(nIndex)]; }
    TYPE& ElementAt(INT_PTR nIndex) { return m_pData[Checked(nIndex)]; }

    const TYPE* GetData() const { return m_pData; }
    TYPE* GetData() { return m_pData; }

    void SetAtGrow(INT_PTR nIndex, ARG_TYPE newElement)
    {
        if (nIndex < 0) {
            AfxThrowInvalidArgException();
        }
        if (nIndex >= m_nSize) {
            SetSize(nIndex + 1, -1);
        }
        m_pData[nIndex] = newElement;
    }

    INT_PTR Add(ARG_TYPE newElement)
    {
        INT_PTR nIndex = m_nSize;
        SetAtGrow(nIndex, newElement);
        return nIndex;
    }

    // Appends another array; returns the index of the first added element.
    INT_PTR Append(const CCompatArray& src)
    {
        INT_PTR nOldSize = m_nSize;
        SetSize(m_nSize + src.m_nSize);
        for (INT_PTR i = 0; i < src.m_nSize; i++) {
            m_pData[nOldSize + i] = src.m_pData[i];
        }
        return nOldSize;
    }

    void Copy(const CCompatArray& src)
    {
        if (this != &src) {
            SetSize(src.m_nSize);
            for (INT_PTR i = 0; i < src.m_nSize; i++) {
                m_pData[i] = src.m_pData[i];
            }
        }
    }

    const TYPE& operator[](INT_PTR nIndex) const { return GetAt(nIndex); }
    TYPE& operator[](INT_PTR nIndex) { return ElementAt(nIndex); }

    // Inserts `nCount` copies at the index, moving later elements up. An
    // index past the end grows the array first.
    void InsertAt(INT_PTR nIndex, ARG_TYPE newElement, INT_PTR nCount = 1)
    {
        if (nIndex < 0 || nCount <= 0) {
            AfxThrowInvalidArgException();
        }
        if (nIndex >= m_nSize) {
            SetSize(nIndex + nCount, -1);
        } else {
            INT_PTR nOldSize = m_nSize;
            SetSize(m_nSize + nCount, -1);
            // Drop the new elements at the end, move the tail up over them
            // and construct fresh ones in the gap.
            Destroy(m_pData + nOldSize, nCount);
            memmove(static_cast<void*>(m_pData + nIndex + nCount), static_cast<const void*>(m_pData + nIndex), (nOldSize - nIndex) * sizeof(TYPE));
            Construct(m_pData + nIndex, nCount);
        }
        while (nCount--) {
            m_pData[nIndex++] = newElement;
        }
    }

    void InsertAt(INT_PTR nStartIndex, CCompatArray* pNewArray)
    {
        if (pNewArray == NULL || nStartIndex < 0) {
            AfxThrowInvalidArgException();
        }
        if (pNewArray->GetSize() > 0) {
            InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
            for (INT_PTR i = 0; i < pNewArray->GetSize(); i++) {
                SetAt(nStartIndex + i, pNewArray->GetAt(i));
            }
        }
    }

    void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1)
    {
        if (nIndex < 0 || nCount < 0 || nIndex + nCount > m_nSize) {
            AfxThrowInvalidArgException();
        }
        INT_PTR nMoveCount = m_nSize - (nIndex + nCount);
        Destroy(m_pData + nIndex, nCount);
        if (nMoveCount > 0) {
            memmove(static_cast<void*>(m_pData + nIndex), static_cast<const void*>(m_pData + nIndex + nCount), nMoveCount * sizeof(TYPE));
        }
        m_nSize -= nCount;
    }

protected:
    TYPE* m_pData;
    INT_PTR m_nSize;
    INT_PTR m_nMaxSize;
    INT_PTR m_nGrowBy;

    INT_PTR Checked(INT_PTR nIndex) const
    {
        if (nIndex < 0 || nIndex >= m_nSize) {
            AfxThrowInvalidArgException();
        }
        return nIndex;
    }

private:
    static TYPE* Allocate(INT_PTR nCount)
    {
        TYPE* p = static_cast<TYPE*>(malloc(nCount * sizeof(TYPE)));
        if (p == NULL) {
            throw std::bad_alloc();
        }
        return p;
    }

    static void Construct(TYPE* pElements, INT_PTR nCount)
    {
        memset(static_cast<void*>(pElements), 0, nCount * sizeof(TYPE));
        for (INT_PTR i = 0; i < nCount; i++) {
            ::new (static_cast<void*>(pElements + i)) TYPE;
        }
    }

    static void Destroy(TYPE* pElements, INT_PTR nCount)
    {
        for (INT_PTR i = 0; i < nCount; i++) {
            pElements[i].~TYPE();
        }
    }
};

template <class TYPE, class ARG_TYPE = const TYPE&>
class CArray : public CCompatArray<TYPE, ARG_TYPE, false> {
};

class CPtrArray : public CCompatArray<void*, void*, true> {
};

class CObArray : public CCompatArray<CObject*, CObject*, true> {
};

class CDWordArray : public CCompatArray<DWORD, DWORD, true> {
};

class CUIntArray : public CCompatArray<UINT, UINT, true> {
};

class CWordArray : public CCompatArray<WORD, WORD, true> {
};

class CByteArray : public CCompatArray<BYTE, BYTE, true> {
};

class CStringArray : public CCompatArray<CString, const char*, true> {
    typedef CCompatArray<CString, const char*, true> Base;

public:
    using Base::Add;
    using Base::InsertAt;
    using Base::SetAt;
    using Base::SetAtGrow;

    // Taking a `CString` shares its buffer instead of copying the text.
    void SetAt(INT_PTR nIndex, const CString& newElement) { m_pData[Checked(nIndex)] = newElement; }
    void SetAtGrow(INT_PTR nIndex, const CString& newElement)
    {
        if (nIndex < 0) {
            AfxThrowInvalidArgException();
        }
        if (nIndex >= m_nSize) {
            // An element of this array cannot be passed when the storage has
            // to grow: growing would move it away. MFC rejects that too.
            if (nIndex >= m_nMaxSize && &newElement >= m_pData && &newElement < m_pData + m_nMaxSize) {
                AfxThrowInvalidArgException();
            }
            SetSize(nIndex + 1, -1);
        }
        m_pData[nIndex] = newElement;
    }
    INT_PTR Add(const CString& newElement)
    {
        INT_PTR nIndex = m_nSize;
        SetAtGrow(nIndex, newElement);
        return nIndex;
    }
    void InsertAt(INT_PTR nIndex, const CString& newElement, INT_PTR nCount = 1)
    {
        CString copy(newElement);
        Base::InsertAt(nIndex, static_cast<const char*>(NULL), nCount);
        while (nCount--) {
            m_pData[nIndex++] = copy;
        }
    }
};

// An array of pointers of one type on top of `CPtrArray` or `CObArray`.
template <class BASE_CLASS, class TYPE>
class CTypedPtrArray : public BASE_CLASS {
public:
    TYPE GetAt(INT_PTR nIndex) const { return (TYPE)BASE_CLASS::GetAt(nIndex); }
    TYPE& ElementAt(INT_PTR nIndex) { return (TYPE&)BASE_CLASS::ElementAt(nIndex); }
    void SetAt(INT_PTR nIndex, TYPE ptr) { BASE_CLASS::SetAt(nIndex, ptr); }
    void SetAtGrow(INT_PTR nIndex, TYPE newElement) { BASE_CLASS::SetAtGrow(nIndex, newElement); }
    INT_PTR Add(TYPE newElement) { return BASE_CLASS::Add(newElement); }
    INT_PTR Append(const CTypedPtrArray<BASE_CLASS, TYPE>& src) { return BASE_CLASS::Append(src); }
    void Copy(const CTypedPtrArray<BASE_CLASS, TYPE>& src) { BASE_CLASS::Copy(src); }
    void InsertAt(INT_PTR nIndex, TYPE newElement, INT_PTR nCount = 1) { BASE_CLASS::InsertAt(nIndex, newElement, nCount); }
    void InsertAt(INT_PTR nStartIndex, CTypedPtrArray<BASE_CLASS, TYPE>* pNewArray) { BASE_CLASS::InsertAt(nStartIndex, pNewArray); }
    TYPE operator[](INT_PTR nIndex) const { return (TYPE)BASE_CLASS::operator[](nIndex); }
    TYPE& operator[](INT_PTR nIndex) { return (TYPE&)BASE_CLASS::operator[](nIndex); }
};

// -----------------------------------------------------------------------------
// CMap
// -----------------------------------------------------------------------------

// Hash of a numeric key: one step of the Park-Miller "minimal standard"
// generator (multiplier 16807 modulo 2^31 - 1, by Schrage's method). The
// hash functions decide the order in which a map is iterated, so they are
// the ones current MFC uses.
template <class ARG_KEY>
inline UINT HashKey(ARG_KEY key)
{
    long value = static_cast<long>(key);
    long quot = value / 127773;
    long rem = value % 127773;
    rem = 16807 * rem - 2836 * quot;
    if (rem < 0) {
        rem += 2147483647;
    }
    return static_cast<UINT>(rem);
}

// Hash of a string key: FNV-1 (multiply by the prime, then mix in the
// character, sign-extended) over at most about ten characters: a string of
// ten or more characters is sampled at every (1 + length / 10)th position.
// Found by comparing hash values with MFC's for chosen strings.
template <>
inline UINT HashKey<const char*>(const char* key)
{
    if (key == NULL) {
        AfxThrowInvalidArgException();
    }
    size_t nLength = strlen(key);
    size_t nStride = 1 + nLength / 10;
    UINT nHash = 2166136261U;
    for (size_t i = 0; i < nLength; i += nStride) {
        nHash = (16777619U * nHash) ^ static_cast<UINT>(key[i]);
    }
    return nHash;
}

// A hash map with chained buckets. Iteration goes through the buckets in
// order, and within a bucket from the newest entry to the oldest.
template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CMap : public CObject {
public:
    typedef KEY BASE_KEY;
    typedef ARG_KEY BASE_ARG_KEY;
    typedef VALUE BASE_VALUE;
    typedef ARG_VALUE BASE_ARG_VALUE;

protected:
    struct CAssoc {
        CAssoc* pNext;
        UINT nHashValue;
        KEY key;
        VALUE value;
    };

public:
    CMap(INT_PTR nBlockSize = 10)
    {
        m_pHashTable = NULL;
        m_nHashTableSize = 17;
        m_nCount = 0;
        m_pFreeList = NULL;
        m_pBlocks = NULL;
        m_nBlockSize = nBlockSize;
    }

    ~CMap() override { RemoveAll(); }

    INT_PTR GetCount() const { return m_nCount; }
    INT_PTR GetSize() const { return m_nCount; }
    BOOL IsEmpty() const { return m_nCount == 0; }
    UINT GetHashTableSize() const { return m_nHashTableSize; }

    void InitHashTable(UINT nHashSize, BOOL bAllocNow = TRUE)
    {
        if (m_pHashTable != NULL) {
            free(m_pHashTable);
            m_pHashTable = NULL;
        }
        if (bAllocNow) {
            m_pHashTable = static_cast<CAssoc**>(calloc(nHashSize, sizeof(CAssoc*)));
            if (m_pHashTable == NULL) {
                throw std::bad_alloc();
            }
        }
        m_nHashTableSize = nHashSize;
    }

    BOOL Lookup(ARG_KEY key, VALUE& rValue) const
    {
        UINT nHashBucket, nHashValue;
        CAssoc* pAssoc = GetAssocAt(key, nHashBucket, nHashValue);
        if (pAssoc == NULL) {
            return FALSE;
        }
        rValue = pAssoc->value;
        return TRUE;
    }

    // Returns the value for the key, adding a default one when missing.
    VALUE& operator[](ARG_KEY key)
    {
        UINT nHashBucket, nHashValue;
        CAssoc* pAssoc = GetAssocAt(key, nHashBucket, nHashValue);
        if (pAssoc == NULL) {
            if (m_pHashTable == NULL) {
                InitHashTable(m_nHashTableSize);
            }
            pAssoc = NewAssoc();
            pAssoc->nHashValue = nHashValue;
            pAssoc->key = key;
            pAssoc->pNext = m_pHashTable[nHashBucket];
            m_pHashTable[nHashBucket] = pAssoc;
        }
        return pAssoc->value;
    }

    void SetAt(ARG_KEY key, ARG_VALUE newValue) { (*this)[key] = newValue; }

    BOOL RemoveKey(ARG_KEY key)
    {
        if (m_pHashTable == NULL) {
            return FALSE;
        }
        UINT nHashValue = HashKey<ARG_KEY>(key);
        CAssoc** ppAssocPrev = &m_pHashTable[nHashValue % m_nHashTableSize];
        for (CAssoc* pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext) {
            if (pAssoc->nHashValue == nHashValue && CompareElements<KEY>(&pAssoc->key, &key)) {
                *ppAssocPrev = pAssoc->pNext;
                FreeAssoc(pAssoc);
                return TRUE;
            }
            ppAssocPrev = &pAssoc->pNext;
        }
        return FALSE;
    }

    void RemoveAll()
    {
        if (m_pHashTable != NULL) {
            for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++) {
                for (CAssoc* pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext) {
                    pAssoc->value.~VALUE();
                    pAssoc->key.~KEY();
                }
            }
            free(m_pHashTable);
            m_pHashTable = NULL;
        }
        m_nCount = 0;
        m_pFreeList = NULL;
        if (m_pBlocks != NULL) {
            m_pBlocks->FreeChain();
            m_pBlocks = NULL;
        }
    }

    POSITION GetStartPosition() const { return m_nCount == 0 ? NULL : BEFORE_START_POSITION; }

    // Returns the entry at the position and moves the position on; it
    // becomes NULL after the last entry.
    void GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const
    {
        CAssoc* pAssocRet = reinterpret_cast<CAssoc*>(rNextPosition);
        if (pAssocRet == reinterpret_cast<CAssoc*>(BEFORE_START_POSITION)) {
            pAssocRet = NULL;
            for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++) {
                if ((pAssocRet = m_pHashTable[nBucket]) != NULL) {
                    break;
                }
            }
        }

        CAssoc* pAssocNext = pAssocRet->pNext;
        if (pAssocNext == NULL) {
            for (UINT nBucket = (pAssocRet->nHashValue % m_nHashTableSize) + 1; nBucket < m_nHashTableSize; nBucket++) {
                if ((pAssocNext = m_pHashTable[nBucket]) != NULL) {
                    break;
                }
            }
        }

        rNextPosition = reinterpret_cast<POSITION>(pAssocNext);
        rKey = pAssocRet->key;
        rValue = pAssocRet->value;
    }

protected:
    CAssoc** m_pHashTable;
    UINT m_nHashTableSize;
    INT_PTR m_nCount;
    CAssoc* m_pFreeList;
    CCompatBlock* m_pBlocks;
    INT_PTR m_nBlockSize;

    CAssoc* NewAssoc()
    {
        if (m_pFreeList == NULL) {
            CCompatBlock* pNewBlock = CCompatBlock::Create(m_pBlocks, m_nBlockSize, sizeof(CAssoc));
            CAssoc* pAssoc = static_cast<CAssoc*>(pNewBlock->data()) + m_nBlockSize - 1;
            for (INT_PTR i = m_nBlockSize - 1; i >= 0; i--, pAssoc--) {
                pAssoc->pNext = m_pFreeList;
                m_pFreeList = pAssoc;
            }
        }

        CAssoc* pAssoc = m_pFreeList;
        m_pFreeList = m_pFreeList->pNext;
        m_nCount++;
        memset(static_cast<void*>(&pAssoc->key), 0, sizeof(KEY));
        ::new (static_cast<void*>(&pAssoc->key)) KEY;
        memset(static_cast<void*>(&pAssoc->value), 0, sizeof(VALUE));
        ::new (static_cast<void*>(&pAssoc->value)) VALUE;
        return pAssoc;
    }

    void FreeAssoc(CAssoc* pAssoc)
    {
        pAssoc->value.~VALUE();
        pAssoc->key.~KEY();
        pAssoc->pNext = m_pFreeList;
        m_pFreeList = pAssoc;
        m_nCount--;

        // Everything goes back to the heap when the map becomes empty.
        if (m_nCount == 0) {
            RemoveAll();
        }
    }

    CAssoc* GetAssocAt(ARG_KEY key, UINT& nHashBucket, UINT& nHashValue) const
    {
        nHashValue = HashKey<ARG_KEY>(key);
        nHashBucket = nHashValue % m_nHashTableSize;
        if (m_pHashTable == NULL) {
            return NULL;
        }
        for (CAssoc* pAssoc = m_pHashTable[nHashBucket]; pAssoc != NULL; pAssoc = pAssoc->pNext) {
            if (pAssoc->nHashValue == nHashValue && CompareElements<KEY>(&pAssoc->key, &key)) {
                return pAssoc;
            }
        }
        return NULL;
    }
};

class CMapWordToPtr : public CMap<WORD, WORD, void*, void*> {
public:
    CMapWordToPtr(INT_PTR nBlockSize = 10)
        : CMap<WORD, WORD, void*, void*>(nBlockSize)
    {
    }
};

class CMapStringToString : public CMap<CString, const char*, CString, const char*> {
public:
    CMapStringToString(INT_PTR nBlockSize = 10)
        : CMap<CString, const char*, CString, const char*>(nBlockSize)
    {
    }
};

// A map with typed keys and pointer values on top of an untyped map class.
template <class BASE_CLASS, class KEY, class VALUE>
class CTypedPtrMap : public BASE_CLASS {
public:
    CTypedPtrMap(INT_PTR nBlockSize = 10)
        : BASE_CLASS(nBlockSize)
    {
    }

    BOOL Lookup(typename BASE_CLASS::BASE_ARG_KEY key, VALUE& rValue) const
    {
        return BASE_CLASS::Lookup(key, (typename BASE_CLASS::BASE_VALUE&)rValue);
    }

    VALUE& operator[](typename BASE_CLASS::BASE_ARG_KEY key) { return (VALUE&)BASE_CLASS::operator[](key); }
    void SetAt(KEY key, VALUE newValue) { BASE_CLASS::SetAt(key, newValue); }
    BOOL RemoveKey(KEY key) { return BASE_CLASS::RemoveKey(key); }

    void GetNextAssoc(POSITION& rPosition, KEY& rKey, VALUE& rValue) const
    {
        BASE_CLASS::GetNextAssoc(rPosition, (typename BASE_CLASS::BASE_KEY&)rKey, (typename BASE_CLASS::BASE_VALUE&)rValue);
    }
};

#endif /* IWD2RE_COMPAT_COLLECTIONS_H_ */
