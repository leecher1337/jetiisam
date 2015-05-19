#include "iisam.h"
#include "jetuwrap.h"
#include "netutil.h"
#include "windos.h"
#include <stdlib.h>

BOOL DOSChangeDirectory(LPCWSTR lpPathName)
{
	return JetSetCurrentDirectoryW(lpPathName);
}

short DOSCloseFile(HANDLE hFile)
{
	return -(CloseHandle(hFile) != TRUE);
}

short DOSCreateFile(LPCWSTR lpFileName, HANDLE *phFile)
{
	if (NetProtocolType(lpFileName) == Proto_HTTP)
		return -3;
	*phFile = JetCreateFileW(lpFileName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (*phFile == INVALID_HANDLE_VALUE)
		return -(USHORT)GetLastError();
	return 0;
}

int DOSCurrentTime()
{
	SYSTEMTIME SystemTime;

	GetLocalTime(&SystemTime);
	return 100 * (SystemTime.wSecond + 60 * (SystemTime.wMinute + 60 * SystemTime.wHour));
}

static BOOL _FileTimeToDosDateTime(FILETIME ft, WORD *pyy, WORD *pmm, WORD *pdd, WORD *phh, WORD *pmi, WORD *pss)
{
	WORD FatDate, FatTime;

	FileTimeToLocalFileTime(&ft, &ft);
	if (FileTimeToDosDateTime(&ft, &FatDate, &FatTime))
	{
		*pyy = (FatDate>>9) + 1980;
		*pmm = (FatDate>>5) & 0xF;
		*pdd = FatDate & 0x1F;
		*phh = FatTime >> 11;
		*pmi = (FatTime >> 5) & 0x3F;
		*pss = (FatTime & 0x1F) << 1;
		return TRUE;
	}
	return FALSE;
}

BOOL DOSFileDateTime(WCHAR *pwszFile, short fmt, WORD *pyy, WORD *pmm, WORD *pdd, WORD *phh, WORD *pmi, WORD *pss)
{
	WIN32_FIND_DATAW finddata = {0};
	HANDLE hFind;
	FILETIME ft;
	int type = NetProtocolType(pwszFile);

	if (type == Proto_HTTP)
	{
		*pyy = 0;
		*pmm = 0;
		*pdd = 0;
		*phh = 0;
		*pmi = 0;
		*pss = 0;
		return 1;
	}
	if (type != Proto_FTP)
	{
		if ((hFind = JetFindFirstFileW(pwszFile, &finddata)) == (HANDLE)-1)
			return 0;

		switch (fmt)
		{
		case 0: ft = finddata.ftLastWriteTime; break;
		case 1: ft = finddata.ftLastAccessTime; break;
		case 2: ft = finddata.ftCreationTime; break;
		default: fmt = -1;
		}

		if (fmt!=-1) _FileTimeToDosDateTime(ft, pyy, pmm, pdd, phh, pmi, pss);
		FindClose(hFind); 
		return 1;
	}
	if (NetFileInfo(pwszFile, &finddata))
		return 0;
	return _FileTimeToDosDateTime(finddata.ftCreationTime, pyy, pmm, pdd, phh, pmi, pss);
}

BOOL DOSFileDelete(LPCWSTR lpFileName)
{
	return JetDeleteFileW(lpFileName);
}

short DOSFileExists(LPCWSTR lpFileName)
{
	if (NetProtocolType(lpFileName))
		return (short)NetFileExists(lpFileName);

	if (JetGetFileAttributesW(lpFileName) == -1)
		return -(USHORT)GetLastError();
	return 0;
}

BOOL DOSFileRename(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	return JetMoveFileW(lpExistingFileName, lpNewFileName);
}

BOOL DOSFindFirstMatchingFile(LPCWSTR lpFileName, LPWSTR lpFoundFile, DOSFIND *pNet, JET_ERR *pRet)
{
	WIN32_FIND_DATAW finddata = {0};

	switch (pNet->prot = NetProtocolType(lpFileName))
	{
	case Proto_HTTP: return FALSE;
	case Proto_FTP : return NetFindFirstMatchingFile(lpFileName, lpFoundFile, &pNet->conn, pRet);
	default: 
		pNet->hFind = JetFindFirstFileW(lpFileName, &finddata);
		if (pNet->hFind == (HANDLE)-1) return FALSE;
		wcscpy(lpFoundFile, finddata.cFileName);
		*pRet = JET_errSuccess;
		return TRUE;
	}
}

BOOL DOSFindLocalDirectory(LPWSTR lpDir)
{
	return JetGetTempPathW(MAX_PATH, lpDir);
}

BOOL DOSFindNextMatchingFile(LPWSTR lpFileName, DOSFIND *pNet, JET_ERR *pRet)
{
	WIN32_FIND_DATAW finddata;

	if (pNet->prot == Proto_FTP)
		return NetFindNextMatchingFile(lpFileName, &pNet->conn, pRet);
	if (JetFindNextFileW(pNet->hFind, &finddata))
	{
		wcscpy(lpFileName, finddata.cFileName);
		return 1;
	}
	FindClose(pNet->hFind);
	return 0;
}

BOOL DOSGetCurrentDirectory(LPWSTR lpBuffer, int dummy)
{
	return JetGetCurrentDirectoryW(MAX_PATH, lpBuffer);
}

BOOL DOSLocatesDirectory(LPCWSTR lpFileName, JET_ERR *pRet)
{
	WCHAR *pw, wszCWD[260];
	BOOL bRet;

	if (NetProtocolType(lpFileName))
		return NetDirectoryExists(lpFileName, pRet);
	pw = (WCHAR*)&lpFileName[(wcslen(lpFileName)-1)];
	if (*pw == L':') return 1;
	if (lpFileName[0] != L'\\' || lpFileName[1] != L'\\')
	{
		if (*pw != L'\\' || pw == lpFileName || pw[-1] == L':')
			pw = NULL; else *pw = 0;
		DOSGetCurrentDirectory(wszCWD, 0);
		if (bRet = DOSChangeDirectory(lpFileName))
			DOSChangeDirectory(wszCWD);
		if (pw) *pw = L'\\';
		return bRet;
	}
	else
	{
		DWORD dwAttr = JetGetFileAttributesW(lpFileName);

		if (dwAttr == (DWORD)-1) return 0;
		return (dwAttr >> 4) & 1;
	}
}

short DOSMakeDirectory(LPCWSTR lpPathName)
{
	if (JetCreateDirectoryW(lpPathName, 0))
		return 0;
	return -(USHORT)GetLastError();
}

short DOSOpenFile(LPCWSTR lpFileName, UINT uStyle, HANDLE *phFile)
{
	DWORD dwDesiredAccess, dwShareMode;

	if (uStyle & OF_READWRITE)
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	else
		dwDesiredAccess = GENERIC_READ | (((uStyle & OF_WRITE)*-1) & GENERIC_WRITE);
	if (uStyle & OF_SHARE_EXCLUSIVE)
		dwShareMode = 0;
	else
	{
		if (uStyle & OF_SHARE_DENY_WRITE)
			dwShareMode = FILE_SHARE_READ;
		else
			dwShareMode = (FILE_SHARE_READ | FILE_SHARE_WRITE) - ((uStyle & OF_SHARE_DENY_READ) != 0);
	}
	*phFile = JetCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (*phFile == (HANDLE)-1)
	{
		int ret = -(USHORT)GetLastError();
		if (!ret) return -2;
		return ret;
	}
	return 0;
}

long DOSReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD cbBuffer)
{
	if (ReadFile(hFile, lpBuffer, cbBuffer, &cbBuffer, NULL))
		return cbBuffer;
	return -1;
}

short DOSRemoveDirectory(LPCWSTR lpFileName)
{
	if (JetRemoveDirectoryW(lpFileName))
		return 0;
	return -(USHORT)GetLastError();
}

short DOSSetFilePosition(HANDLE hFile, DWORD dwMoveMethod, LONG lDistanceToMove)
{
	return (SetFilePointer(hFile, lDistanceToMove, NULL, dwMoveMethod) != -1) - 1;
}

short DOSGetFilePosition(HANDLE hFile, DWORD *pdwPos)
{

	if ((*pdwPos = SetFilePointer (hFile, 0, 0, FILE_CURRENT)) == 0xFFFFFFFF)
		return -1;
	return 0;
}

short DOSUniquePathname(LPWSTR lpFile, LPCWSTR lpDir, LPCWSTR lpPfx, LPCWSTR lpExt)
{
	WCHAR wcTerm = 0, wszNum[6], wszFile[9];
	int iCnt;
	short ret;

	if (lpDir && *lpDir)
	{
		WCHAR wc = lpDir[wcslen(lpDir)-1];
		if (wc != L'\\' && wc != L':') wcTerm = L'\\';
	}
	for (iCnt = 0; iCnt < 0xFFFF; iCnt++)
	{
		_itow(iCnt, wszNum, 10);
		wcscpy(wszFile, L"00000000");
		memcpy(wszFile, lpPfx, wcslen(lpPfx)*sizeof(WCHAR));
		wcscpy(&wszFile[8-wcslen(wszNum)], wszNum);
		*lpFile = 0;
		if (lpDir && *lpDir)
		{
			wcscpy (lpFile, lpDir);
			if (wcTerm) wcscat(lpFile, L"\\");
		}
		wcscat(lpFile, wszFile);
		wcscat(lpFile, lpExt);
		if (ret = DOSFileExists(lpFile)) break;
	}
	return ret & -(ret != -2);
}

long DOSWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD cbBuffer)
{
	if (WriteFile(hFile, lpBuffer, cbBuffer, &cbBuffer, NULL))
		return cbBuffer;
	return -1;
}

short DOSFileSize(LPCWSTR lpFile, DWORD *pdwFileSize)
{
	HANDLE hFile;

	if (NetProtocolType(lpFile))
	{
		*pdwFileSize = 0x2000;
		return 1;
	}
	if (DOSOpenFile(lpFile, OF_SHARE_DENY_NONE, &hFile))
		return 0;
	DOSSetFilePosition(hFile, FILE_END, 0);
	DOSGetFilePosition(hFile, pdwFileSize);
	DOSCloseFile(hFile);
	return 1;
}

BOOL DOSChannelDateTime(HANDLE hFile, WORD fmt, WORD *pyy, WORD *pmm, WORD *pdd, WORD *phh, WORD *pmi, WORD *pss)
{
	FILETIME ft;
	BOOL bRet;

	switch (fmt)
	{
	case 0: bRet = GetFileTime (hFile, &ft, NULL, NULL); break;
	case 1: bRet = GetFileTime (hFile, NULL, &ft, NULL); break;
	case 2: bRet = GetFileTime (hFile, NULL, NULL, &ft); break;
	}
	if (bRet) return _FileTimeToDosDateTime(ft, pyy, pmm, pdd, phh, pmi, pss);
	return FALSE;
}

