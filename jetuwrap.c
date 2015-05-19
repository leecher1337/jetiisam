#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "winalloc.h"

BOOL g_bUseUnicode = FALSE;

void IsUnicodeOS()
{
	OSVERSIONINFO VersionInfo;

	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	g_bUseUnicode = FALSE;
	if (GetVersionExA(&VersionInfo) && VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		g_bUseUnicode = TRUE;
}

/**************************************************
 * Jet registry functions                         *
 **************************************************/

LONG JetRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
	char szSubKey[256];

	if (g_bUseUnicode)
		return RegOpenKeyW(hKey, lpSubKey, phkResult);
	WideCharToMultiByte(CP_ACP, 0, lpSubKey, -1, szSubKey, sizeof(szSubKey), 0, NULL);
	return RegOpenKeyA(hKey, szSubKey, phkResult);
}

LONG JetRegCreateKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
	char szSubKey[256];

	if (g_bUseUnicode)
		return RegCreateKeyW(hKey, lpSubKey, phkResult);
	WideCharToMultiByte(CP_ACP, 0, lpSubKey, -1, szSubKey, sizeof(szSubKey), 0, NULL);
	return RegCreateKeyA(hKey, szSubKey, phkResult);
}

LONG JetRegDeleteKeyW(HKEY hKey, LPCWSTR lpSubKey)
{
	char szSubKey[256];

	if (g_bUseUnicode)
		return RegDeleteKeyW(hKey, lpSubKey);
	WideCharToMultiByte(CP_ACP, 0, lpSubKey, -1, szSubKey, sizeof(szSubKey), 0, NULL);
	return RegDeleteKeyA(hKey, szSubKey);
}

LONG JetRegDeleteValueW(HKEY hKey, LPCWSTR lpValueName)
{
	char szValueName[256];

	if (g_bUseUnicode)
		return RegDeleteValueW(hKey, lpValueName);
	WideCharToMultiByte(CP_ACP, 0, lpValueName, -1, szValueName, sizeof(szValueName), 0, NULL);
	return RegDeleteValueA(hKey, szValueName);
}

LONG JetRegQueryInfoKeyW(HKEY hKey, LPWSTR lpClass, LPDWORD lpcbClass, LPDWORD lpReserved, LPDWORD lpcSubKeys,
						 LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues, 
						 LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, 
						 PFILETIME lpftLastWriteTime)
{
	char szClass[256];

	if (g_bUseUnicode)
		return RegQueryInfoKeyW(hKey, lpClass, lpcbClass, lpReserved, lpcSubKeys, lpcbMaxSubKeyLen, 
			lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen, lpcbMaxValueLen, lpcbSecurityDescriptor, 
			lpftLastWriteTime);
	WideCharToMultiByte(CP_ACP, 0, lpClass, -1, szClass, sizeof(szClass), 0, NULL);
	return RegQueryInfoKeyA(hKey, szClass, lpcbClass, lpReserved, lpcSubKeys, lpcbMaxSubKeyLen, 
		lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen, lpcbMaxValueLen, lpcbSecurityDescriptor, 
		lpftLastWriteTime);
}


LONG JetRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType,
						LPBYTE lpData, LPDWORD lpcbData)
{
	char szValueName[256];
	DWORD dwType, cbData;
	LONG ret;

	if (g_bUseUnicode)
		return RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	WideCharToMultiByte(CP_ACP, 0, lpValueName, -1, szValueName, sizeof(szValueName), 0, NULL);
	if ((ret = RegQueryValueExA(hKey, szValueName, lpReserved, &dwType, NULL, &cbData))
		== ERROR_SUCCESS)
	{
		if (dwType == REG_SZ || dwType == REG_EXPAND_SZ || dwType==REG_MULTI_SZ)
		{
			char *pszBuf = MemAllocate(*lpcbData >> 1);
			
			if (!pszBuf) return ERROR_NOT_ENOUGH_MEMORY;
			if (ret = (RegQueryValueExA(hKey, szValueName, lpReserved, lpType, pszBuf, &cbData))
				== ERROR_SUCCESS)
			{
				*lpcbData = MultiByteToWideChar(CP_ACP, 0, pszBuf, cbData, (LPWSTR)lpData, *lpcbData>>1)<<1;
			}
			MemFree (pszBuf);
		}
		return RegQueryValueExA(hKey, szValueName, lpReserved, lpType, lpData, lpcbData);
	}
	return ret;
}

LONG JetRegSetValueExW(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType,
					   LPBYTE lpData, DWORD cbData)
{
	char szValueName[256];

	if (g_bUseUnicode)
		return RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);
	WideCharToMultiByte(CP_ACP, 0, lpValueName, -1, szValueName, sizeof(szValueName), 0, NULL);
	if (dwType == REG_SZ || dwType == REG_EXPAND_SZ || dwType==REG_MULTI_SZ)
	{
		DWORD cbBuf = cbData >> 1;
		char *pszBuf = MemAllocate(cbBuf);
		LONG ret;

		if (!pszBuf) return ERROR_NOT_ENOUGH_MEMORY;
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)lpData, cbData, pszBuf, cbBuf, 0, NULL);
		ret = RegSetValueExA(hKey, szValueName, Reserved, dwType, pszBuf, cbBuf);
		MemFree (pszBuf);
		return ret;
	}
	return RegSetValueExA(hKey, szValueName, Reserved, dwType, lpData, cbData);
}

/**************************************************
 * Jet filesystem functions                       *
 **************************************************/

BOOL JetSetCurrentDirectoryW(LPCWSTR lpPathName)
{
	char szPathName[MAX_PATH]={0};

	if (g_bUseUnicode)
		return SetCurrentDirectoryW(lpPathName);
	if (WideCharToMultiByte(CP_ACP, 0, lpPathName, -1, szPathName, sizeof(szPathName), 0, NULL)
		>= MAX_PATH)
	{
	
		SetLastError(ERROR_BAD_PATHNAME);
		return FALSE;
	}
	return SetCurrentDirectoryA(szPathName);
}

HANDLE JetCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
					  LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
					  DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	char szPath[MAX_PATH*2];

	if (g_bUseUnicode)
		return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
						   dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, szPath, sizeof(szPath), 0, NULL);
	return CreateFileA(szPath, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
					   dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL JetCreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	char szPathName[MAX_PATH] = {0};

	if (g_bUseUnicode)
		return CreateDirectoryW(lpPathName, lpSecurityAttributes);
	if (WideCharToMultiByte(CP_ACP, 0, lpPathName, -1, szPathName, sizeof(szPathName), 0, NULL)
		>= MAX_PATH)
	{
	
		SetLastError(ERROR_BAD_PATHNAME);
		return FALSE;
	}
	return CreateDirectoryA(szPathName, lpSecurityAttributes);

}

BOOL JetDeleteFileW(LPCWSTR lpFileName)
{
	char szPathName[MAX_PATH];

	if (g_bUseUnicode)
		return DeleteFileW(lpFileName);
	WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, szPathName, sizeof(szPathName), 0, NULL);
	return DeleteFileA(szPathName);
}

HANDLE JetFindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)
{
	char szPathName[MAX_PATH];
	WIN32_FIND_DATAA finddata;
	HANDLE hFind;

	if (g_bUseUnicode)
		return FindFirstFileW(lpFileName, lpFindFileData);
	WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, szPathName, sizeof(szPathName), 0, NULL);
	memcpy (&finddata, lpFindFileData, sizeof(finddata));
	hFind = FindFirstFileA(szPathName, &finddata);
	memcpy (lpFindFileData, &finddata, sizeof(finddata));
	MultiByteToWideChar (CP_ACP, 0, finddata.cFileName, -1, lpFindFileData->cFileName, MAX_PATH);
	MultiByteToWideChar (CP_ACP, 0, finddata.cAlternateFileName, -1, lpFindFileData->cAlternateFileName, MAX_PATH);
	return hFind;
}

BOOL JetFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)
{
	WIN32_FIND_DATAA finddata;
	BOOL bRet;

	if (g_bUseUnicode)
		return FindNextFileW(hFindFile, lpFindFileData);
	memcpy (&finddata, lpFindFileData, sizeof(finddata));
	bRet = FindNextFileA(hFindFile, &finddata);
	memcpy (lpFindFileData, &finddata, sizeof(finddata));
	MultiByteToWideChar (CP_ACP, 0, finddata.cFileName, -1, lpFindFileData->cFileName, MAX_PATH);
	MultiByteToWideChar (CP_ACP, 0, finddata.cAlternateFileName, -1, lpFindFileData->cAlternateFileName, MAX_PATH);
	return bRet;
}

DWORD JetGetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer)
{
	char szPathName[MAX_PATH];

	if (g_bUseUnicode)
		return GetCurrentDirectoryW(nBufferLength, lpBuffer);
	GetCurrentDirectoryA(sizeof(szPathName), szPathName);
	return MultiByteToWideChar (CP_ACP, 0, szPathName, -1, lpBuffer, nBufferLength);
}

DWORD JetGetFileAttributesW(LPCWSTR lpFileName)
{
	char szPathName[MAX_PATH];

	if (g_bUseUnicode)
		return GetFileAttributesW(lpFileName);
	WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, szPathName, sizeof(szPathName), 0, NULL);
	return GetFileAttributesA(szPathName);
}

DWORD JetGetTempPathW(DWORD nBufferLength, LPWSTR lpBuffer)
{
	char szPathName[MAX_PATH];

	if (g_bUseUnicode)
		return GetTempPathW(nBufferLength, lpBuffer);
	GetTempPathA(sizeof(szPathName), szPathName);
	return MultiByteToWideChar (CP_ACP, 0, szPathName, -1, lpBuffer, nBufferLength);
}

BOOL JetMoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	char szExistingFileName[MAX_PATH], szNewFileName[MAX_PATH];

	if (g_bUseUnicode)
		return MoveFileW(lpExistingFileName, lpNewFileName);
	WideCharToMultiByte(CP_ACP, 0, lpExistingFileName, -1, szExistingFileName, sizeof(szExistingFileName), 0, NULL);
	WideCharToMultiByte(CP_ACP, 0, lpNewFileName, -1, szNewFileName, sizeof(szNewFileName), 0, NULL);
	return MoveFileA(szExistingFileName, szNewFileName);
}

DWORD JetRemoveDirectoryW(LPCWSTR lpPathName)
{
	char szPathName[MAX_PATH];

	if (g_bUseUnicode)
		return RemoveDirectoryW(lpPathName);
	if (WideCharToMultiByte(CP_ACP, 0, lpPathName, -1, szPathName, sizeof(szPathName), 0, NULL)>=MAX_PATH)
	{
		SetLastError(ERROR_BAD_PATHNAME);
		return FALSE;
	}
	return RemoveDirectoryA(szPathName);
}

WCHAR *Jetwfullpath(LPWSTR absPath, LPCWSTR relPath, int maxLength)
{
	char szPathName[MAX_PATH], szRelPath[MAX_PATH];

	if (g_bUseUnicode)
		return _wfullpath(absPath, relPath, maxLength);
	WideCharToMultiByte(CP_ACP, 0, relPath, -1, szRelPath, sizeof(szRelPath), 0, NULL);
	if (_fullpath(szPathName, szRelPath, sizeof(szPathName)))
	{
		MultiByteToWideChar (CP_ACP, 0, szPathName, -1, absPath, maxLength);
		return absPath;
	}
	return NULL;
}


/**************************************************
 * Jet character class functions                  *
 **************************************************/
LPWSTR JetCharUpperW(LPWSTR lpwsz)
{
	int cbMBS;
	char *psz;

	if (g_bUseUnicode)
		return CharUpperW(lpwsz);
	if ((cbMBS = WideCharToMultiByte(CP_ACP, 0, lpwsz, -1, NULL, 0, NULL, NULL)) && 
		(psz = MemAllocate(cbMBS)))
	{
		WideCharToMultiByte(CP_ACP, 0, lpwsz, -1, psz, cbMBS, NULL, NULL);
		CharUpperA(psz);
		MultiByteToWideChar(CP_ACP, 0, psz, cbMBS, lpwsz, wcslen(lpwsz));
		MemFree(psz);
	}
	return lpwsz;
}

/**************************************************
 * Jet base system functions                      *
 **************************************************/
HMODULE JetLoadLibraryW(LPCWSTR lpLibFileName)
{
	char szPathName[MAX_PATH];

	if (g_bUseUnicode)
		return LoadLibraryW(lpLibFileName);
	WideCharToMultiByte(CP_ACP, 0, lpLibFileName, -1, szPathName, sizeof(szPathName), 0, NULL);
	return LoadLibraryA(szPathName);
}
