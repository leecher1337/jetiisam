PVOID MemAllocate(DWORD dwBytes);
LPVOID MemReAllocate(PVOID lpMem, DWORD dwBytes);
void MemFree(PVOID pMem);
void MemFreeAllPages();
