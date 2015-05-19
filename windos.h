typedef struct 
{
	enum eProtoType prot;	// + 0
	HANDLE hFind;			// + 4
	NET_FIND_HANDLE conn;	// + 8
} DOSFIND;

BOOL DOSChangeDirectory(LPCWSTR lpPathName);
short DOSCloseFile(HANDLE hFile);
short DOSCreateFile(LPCWSTR lpFileName, HANDLE *phFile);
int DOSCurrentTime();
BOOL DOSFileDateTime(WCHAR *pwszFile, short fmt, WORD *pyy, WORD *pmm, WORD *pdd, WORD *phh, WORD *pmi, WORD *pss);
BOOL DOSFileDelete(LPCWSTR lpFileName);
short DOSFileExists(LPCWSTR lpFileName);
BOOL DOSFileRename(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
BOOL DOSFindFirstMatchingFile(LPCWSTR lpFileName, LPWSTR lpFoundFile, DOSFIND *pNet, JET_ERR *pRet);
BOOL DOSFindLocalDirectory(LPWSTR lpDir);
BOOL DOSFindNextMatchingFile(LPWSTR lpFileName, DOSFIND *pNet, JET_ERR *pRet);
BOOL DOSGetCurrentDirectory(LPWSTR lpBuffer, int dummy);
BOOL DOSLocatesDirectory(LPCWSTR lpFileName, JET_ERR *pRet);
short DOSMakeDirectory(LPCWSTR lpPathName);
short DOSOpenFile(LPCWSTR lpFileName, UINT uStyle, HANDLE *phFile);
long  DOSReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD cbBuffer);
short DOSRemoveDirectory(LPCWSTR lpFileName);
short DOSSetFilePosition(HANDLE hFile, DWORD dwMoveMethod, LONG lDistanceToMove);
short DOSGetFilePosition(HANDLE hFile, DWORD *pdwPos);
short DOSUniquePathname(LPWSTR lpFile, LPCWSTR lpDir, LPCWSTR lpPfx, LPCWSTR lpExt);
long  DOSWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD cbBuffer);
short DOSFileSize(LPCWSTR lpFile, DWORD *pdwFileSize);
BOOL DOSChannelDateTime(HANDLE hFile, WORD fmt, WORD *pyy, WORD *pmm, WORD *pdd, WORD *phh, WORD *pmi, WORD *pss);
