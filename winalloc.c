#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include "winalloc.h"

struct tag_memobj_hdr;
typedef struct tag_memobj_hdr MEMOBJ_HDR;
struct tag_memobj_hdr{
	DWORD Tag;			// +0	1..Large block, 2..small block
	DWORD dwSize;		// +4
	union
	{
	HGLOBAL hMem;		// +8
	MEMOBJ_HDR *next;
	};
};
#define MEM_PTR(ptr) (((BYTE*)ptr)+sizeof(MEMOBJ_HDR))
#define HDR_PTR(ptr) (MEMOBJ_HDR*)(((BYTE*)ptr)-sizeof(MEMOBJ_HDR))

struct tag_mempage_hdr;
typedef struct tag_mempage_hdr MEMPAGE_HDR;
struct tag_mempage_hdr {
	HGLOBAL hMem;		// +0
	DWORD dwUnk1;		// +4
	MEMPAGE_HDR *next;	// +8
	MEMPAGE_HDR *prev;	// +12
	MEMOBJ_HDR memObj;	// +16
};

#define PAGE_PTR(ptr) (MEMPAGE_HDR*)(((BYTE*)ptr)-(sizeof(MEMPAGE_HDR)-sizeof(MEMOBJ_HDR)))

static LPVOID AllocLargeBlock(DWORD dwBytes);
static BOOL Free1(MEMOBJ_HDR *pObj);
static BOOL Free2(MEMOBJ_HDR *pObj, MEMOBJ_HDR *pObjLast);

static MEMOBJ_HDR *m_pMemCurrent = NULL;
static MEMPAGE_HDR *m_pHeapList = NULL;

LPVOID AllocateSpace(DWORD dwBytes, HGLOBAL *phMem)
{
	*phMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, dwBytes);
	assert(*phMem);
	return GlobalLock(*phMem);
}

void FreeSpace(HGLOBAL hMem)
{
	if (hMem)
	{
		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
}

LPVOID MemAllocate(DWORD dwBytes)
{
	MEMPAGE_HDR *pPage;
	MEMOBJ_HDR *pObj = NULL, *pCurr1 = NULL, *pCurr2 = NULL, *pObjItr;
	HGLOBAL hMem;

	if (dwBytes > 8168)
		return AllocLargeBlock(dwBytes);
	dwBytes+=sizeof(MEMOBJ_HDR);
	if (dwBytes <= sizeof(MEMOBJ_HDR)) dwBytes = sizeof(MEMOBJ_HDR);
	if (m_pMemCurrent)
	{
		for (pObjItr = m_pMemCurrent; pObjItr; pObjItr = pObjItr->next)
		{
			if (pObjItr->dwSize == dwBytes)
			{
				if (pCurr2) pCurr2->next = pObjItr->next;
				else m_pMemCurrent = pObjItr->next;
				ZeroMemory(MEM_PTR(pObjItr), dwBytes-sizeof(MEMOBJ_HDR));
				pObjItr->Tag = 2;
				return MEM_PTR(pObjItr);
			}
			if (pObjItr->dwSize > dwBytes)
			{
				if (!pObj)
				{
					pObj = pObjItr;
					pCurr1 = pCurr2;
				}
			}
			pCurr2 = pObjItr;
		}
	}
	if (!pObj)
	{
		if (!(pPage = AllocateSpace(8196, &hMem)))
			return NULL;
		pPage->hMem = hMem;
		pPage->next = m_pHeapList;
		if (m_pHeapList) m_pHeapList->prev = pPage;
		m_pHeapList = pPage;
		pPage->memObj.dwSize = 8180;
		pObj = &pPage->memObj;
		pCurr1 = NULL;
		if (pCurr2) 
		{
			pCurr2->next = pObj;
			pCurr1 = pCurr2;
		}
		else
			m_pMemCurrent = pObj;
	}
	if (pObj->dwSize - dwBytes < sizeof(MEMOBJ_HDR))
	{
		if (pCurr1)
			pCurr1->next = pObj->next;
		else
			m_pMemCurrent = pObj->next;
	}
	else
	{
		((MEMOBJ_HDR*)((BYTE*)pObj + dwBytes))->dwSize = pObj->dwSize - dwBytes;
		((MEMOBJ_HDR*)((BYTE*)pObj + dwBytes))->next   = pObj->next;
		if (pCurr1) pCurr1->next = (MEMOBJ_HDR*)((BYTE*)pObj + dwBytes);
		else m_pMemCurrent = (MEMOBJ_HDR*)((BYTE*)pObj + dwBytes);
		pObj->dwSize = dwBytes;

	}
	ZeroMemory(MEM_PTR(pObj), dwBytes-sizeof(MEMOBJ_HDR));
	pObj->Tag = 2;
	return MEM_PTR(pObj);
}

LPVOID MemReAllocate(PVOID lpMem, DWORD dwBytes)
{
	MEMOBJ_HDR *pObj = HDR_PTR(lpMem);
	DWORD dwSize;
	LPVOID lpNew;

	if (!lpMem || !dwBytes) return NULL;
	dwSize = pObj->dwSize;
	if (pObj->Tag != 1)
		dwSize-=sizeof(MEMOBJ_HDR);
	if (dwSize >= dwBytes)
		return lpMem;
	if (!(lpNew = MemAllocate(dwBytes)))
		return NULL;
	memcpy (lpNew, lpMem, dwSize);
	MemFree(lpMem);
	return lpNew;
}


void MemFree(PVOID pMem)
{
	MEMOBJ_HDR *pObj = HDR_PTR(pMem), *pObjItr, *pObjItrLast = NULL;
	BOOL ret;

	if (!pMem || !pObj->dwSize) return;
	if (pObj->Tag == 1)
	{
		FreeSpace(pObj->hMem);
		return;
	}
	for (pObjItr = m_pMemCurrent; pObjItr; pObjItr = pObjItr->next)
	{
		if (pObj == (MEMOBJ_HDR*)((BYTE*)pObjItr + pObjItr->dwSize))
		{
			pObjItr->dwSize += pObj->dwSize;
			if ((ret = Free1(pObjItr)) != TRUE) ret = Free2(pObjItr, pObjItrLast);
			return;
		}
		if ((MEMOBJ_HDR*)((BYTE*)pObj + pObj->dwSize) == pObjItr)
		{
			if (pObjItrLast) pObjItrLast->next=pObj;
			else m_pMemCurrent = pObj;
			pObj->next = pObjItr->next;
			pObj->dwSize += pObjItr->dwSize;
			if ((ret = Free1(pObj)) != 1) ret = Free2(pObj, pObjItrLast);
			return;
		}
		pObjItrLast = pObjItr;
	}
	pObj->next = m_pMemCurrent;
	m_pMemCurrent = pObj;
	return;
}

void MemFreeAllPages()
{
	MEMPAGE_HDR *pPage;

	for (pPage = m_pHeapList; pPage; pPage = pPage->next)
		FreeSpace(pPage->hMem);
	m_pHeapList = NULL;
	m_pMemCurrent = NULL;
}

static LPVOID AllocLargeBlock(DWORD dwBytes)
{
	HGLOBAL hMem;

	MEMOBJ_HDR *pObj = AllocateSpace(dwBytes + sizeof(MEMOBJ_HDR), &hMem);
	if (pObj)
	{
		pObj->Tag = 1;
		pObj->dwSize = dwBytes;
		pObj->hMem = hMem;
		pObj = (MEMOBJ_HDR*)MEM_PTR(pObj);
	}
	return pObj;
}

static BOOL Free1(MEMOBJ_HDR *pObj)
{
	MEMOBJ_HDR *pObjItr, *pObjItrLast = NULL;
	MEMPAGE_HDR *pPage = PAGE_PTR(pObj);

	if (pObj->dwSize != 8180) return FALSE;
	for (pObjItr = m_pMemCurrent; pObjItr; pObjItr = pObjItr->next)
	{
		if (pObjItr == pObj) break;
		pObjItrLast = pObjItr;
	}
	if (pObjItrLast) pObjItrLast->next = pObj->next;
	else m_pMemCurrent = pObj->next;
	if (pPage->prev) pPage->prev->next = pPage->next;
	else m_pHeapList = pPage->next;
	if (pPage->next) pPage->next->prev = pPage->prev;
	FreeSpace(pPage->hMem);
	return TRUE;
}

static BOOL Free2(MEMOBJ_HDR *pObj, MEMOBJ_HDR *pObjLast)
{
	MEMOBJ_HDR *pObjItr, *pObjItrLast = NULL;

	for (pObjItr = m_pMemCurrent; pObjItr; pObjItr = pObjItr->next)
	{
		if (pObj == (MEMOBJ_HDR*)((BYTE*)pObjItr + pObjItr->dwSize))
		{
			pObjItr->dwSize += pObj->dwSize;
			if (pObjLast)
				pObjLast->next = pObj->next;
			else
				m_pMemCurrent = pObj->next;
			return Free1(pObjItr);
		}
		if (pObjItr == (MEMOBJ_HDR*)((BYTE*)pObj + pObj->dwSize))
		{
			pObj->dwSize += pObjItr->dwSize;
			if (pObjItrLast) pObjItrLast->next = pObjItr->next;
			else m_pMemCurrent = pObjItr->next;
			return Free1(pObj);
		}
		pObjItrLast = pObjItr;
	}
	return FALSE;
}
