#include "iisam.h"
#include <wininet.h>
#include <stdlib.h>
#include <stdio.h>
#include "netutil.h"
#include "windos.h"
#include "jetuwrap.h"

typedef HINTERNET (WINAPI *ft_InternetOpenA)(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD);
typedef HINTERNET (WINAPI *ft_InternetOpenUrlA)(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD);
typedef BOOL      (WINAPI *ft_InternetCanonicalizeUrlA)(LPCSTR, LPSTR, LPDWORD, DWORD);
typedef HINTERNET (WINAPI *ft_InternetConnectA)(HINTERNET, LPCSTR, INTERNET_PORT, LPCSTR, LPCSTR, DWORD, DWORD, DWORD);
typedef BOOL      (WINAPI *ft_InternetFindNextFileA)(HINTERNET, LPVOID);
typedef HINTERNET (WINAPI *ft_InternetReadFile)(HINTERNET, LPVOID, DWORD, LPDWORD);
typedef BOOL      (WINAPI *ft_InternetWriteFile)(HINTERNET, LPCVOID, DWORD, LPDWORD);
typedef BOOL      (WINAPI *ft_InternetCloseHandle)(HINTERNET);
typedef BOOL      (WINAPI *ft_InternetGetLastResponseInfoA)(LPDWORD, LPSTR, LPDWORD);
typedef BOOL      (WINAPI *ft_InternetCrackUrlA)(LPCSTR, DWORD, DWORD, LPURL_COMPONENTS);
typedef BOOL      (WINAPI *ft_HttpQueryInfoA)(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD);
typedef BOOL      (WINAPI *ft_FtpGetFileA)(HINTERNET, LPCSTR, LPCSTR, BOOL, DWORD, DWORD, DWORD);
typedef BOOL      (WINAPI *ft_FtpPutFileA)(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD);
typedef BOOL      (WINAPI *ft_FtpDeleteFileA)(HINTERNET, LPCSTR);
typedef HINTERNET (WINAPI *ft_FtpFindFirstFileA)(HINTERNET, LPCSTR, LPWIN32_FIND_DATAA, DWORD, DWORD);
typedef HINTERNET (WINAPI *ft_FtpFindFirstFileW)(HINTERNET, LPCWSTR, LPWIN32_FIND_DATAW, DWORD, DWORD);
typedef BOOL      (WINAPI *ft_FtpSetCurrentDirectoryA)(HINTERNET, LPCSTR);

ft_InternetOpenA				_InternetOpenA;
ft_InternetOpenUrlA				_InternetOpenUrlA;
ft_InternetCanonicalizeUrlA		_InternetCanonicalizeUrlA;
ft_InternetConnectA				_InternetConnectA;
ft_InternetFindNextFileA        _InternetFindNextFileA;
ft_InternetReadFile				_InternetReadFile;
ft_InternetWriteFile			_InternetWriteFile;
ft_InternetCloseHandle			_InternetCloseHandle;
ft_InternetGetLastResponseInfoA	_InternetGetLastResponseInfoA;
ft_InternetCrackUrlA			_InternetCrackUrlA;
ft_HttpQueryInfoA				_HttpQueryInfoA;
ft_FtpGetFileA					_FtpGetFileA;
ft_FtpPutFileA					_FtpPutFileA;
ft_FtpDeleteFileA				_FtpDeleteFileA;
ft_FtpFindFirstFileA			_FtpFindFirstFileA;
ft_FtpFindFirstFileW			_FtpFindFirstFileW;
ft_FtpSetCurrentDirectoryA		_FtpSetCurrentDirectoryA;

typedef struct {
	INTERNET_SCHEME nScheme;
	char szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
	char szURLPath[INTERNET_MAX_URL_LENGTH];
	char szUserName[INTERNET_MAX_USER_NAME_LENGTH];
	char szPassword[INTERNET_MAX_PASSWORD_LENGTH];
} URL_COMP;

extern BOOL g_bUseUnicode;

static HINTERNET m_hInternet = NULL;
static HMODULE m_hLibWinInet = NULL;

static BOOL CrackURL(LPCWSTR lpcwUrl, URL_COMP *pUrlComp)
{
	char szURL[INTERNET_MAX_URL_LENGTH];
	URL_COMPONENTSA urlInfo = {0};
	BOOL bRet;

	if (!WideCharToMultiByte(CP_ACP, 0, lpcwUrl, -1, szURL, sizeof(szURL), 0, NULL)) return FALSE;
	urlInfo.dwStructSize = sizeof (URL_COMPONENTS);
	urlInfo.lpszHostName = pUrlComp->szHostName;
	urlInfo.dwHostNameLength = sizeof(pUrlComp->szHostName);
	urlInfo.lpszUrlPath = pUrlComp->szURLPath;
	urlInfo.dwUrlPathLength = sizeof(pUrlComp->szURLPath);
	urlInfo.lpszUserName = pUrlComp->szUserName;
	urlInfo.dwUserNameLength = sizeof(pUrlComp->szUserName);
	urlInfo.lpszPassword = pUrlComp->szPassword;
	urlInfo.dwPasswordLength = sizeof(pUrlComp->szPassword);
	if (bRet = _InternetCrackUrlA(szURL, strlen(szURL), 0, &urlInfo))
		pUrlComp->nScheme = urlInfo.nScheme;
	return bRet;
}

static JET_ERR ParseFTPReturn(char *pszResponse)
{
	if (strstr(pszResponse, "550 "))
		return JET_errObjectNotFound;
	return strstr(pszResponse, "530 ")?JET_errInvalidAccountName:JET_errNetStatusUnknown;
}

static JET_ERR CheckReturn()
{
	DWORD dwErr = GetLastError(), dwBuffLen;
	char szResponse[512];

	switch (dwErr)
	{
	case ERROR_INTERNET_TIMEOUT:
		return JET_errNetTimeout;
	case ERROR_BAD_PATHNAME:
	case ERROR_INTERNET_INVALID_URL:
	case ERROR_INTERNET_NAME_NOT_RESOLVED:
		return JET_errNetResolveHost;
	case ERROR_INTERNET_INCORRECT_USER_NAME:
	case ERROR_INTERNET_INCORRECT_PASSWORD:
	case ERROR_INTERNET_LOGIN_FAILURE:
		return JET_errNetLoginFailed;
	case ERROR_INTERNET_PROTOCOL_NOT_FOUND:
		return JET_errNetInvalidProtocol;
	}
	dwBuffLen = sizeof(szResponse);
	_InternetGetLastResponseInfoA(&dwErr, szResponse, &dwBuffLen);
	if (dwErr == ERROR_SUCCESS) return ParseFTPReturn(szResponse);
	return JET_errSuccess;
}

static JET_ERR ConnectFTP(URL_COMP *pURL, HANDLE *phInternet)
{
	char szDir[_MAX_DIR];

	if (!(*phInternet = _InternetConnectA(m_hInternet, pURL->szHostName, 
		INTERNET_INVALID_PORT_NUMBER, 
		*pURL->szUserName?pURL->szUserName:NULL, 
		*pURL->szPassword?pURL->szUserName:NULL,
		INTERNET_SERVICE_FTP, 0, 0)))
		return CheckReturn();
	_splitpath(pURL->szURLPath, NULL, szDir, NULL, NULL);
	if (!_FtpSetCurrentDirectoryA(*phInternet, szDir))
	{
		_InternetCloseHandle(*phInternet);
		return CheckReturn();
	}
	return JET_errSuccess;
}

static BOOL MatchWildcard(LPCWSTR lpMask, LPWSTR lpExt)
{
	DWORD ccMask, ccExt, dwLen, i;

	_wcsupr(lpExt);
	ccMask = wcslen(lpMask);
	ccExt  = wcslen(lpExt);
	dwLen  = ccMask>=ccExt?ccExt:ccMask;
	if (dwLen <= 0) return FALSE;
	for (i=0; i<dwLen; i++)
		if (lpMask[i] != L'?')
			if (lpExt[i] != lpMask[i]) break;
	return i!=dwLen;
}

static BOOL MatchWildcards(LPSTR pszFileName, LPCWSTR *alpWildcards)
{
	char szExt[_MAX_EXT]={0};
	WCHAR wszExt[_MAX_EXT]={0};
	int i;

	_splitpath(pszFileName, NULL, NULL, NULL, szExt);
	MultiByteToWideChar(CP_ACP, 0, szExt, -1, wszExt, _MAX_EXT);
	for (i=0; alpWildcards[i]; i++)
	{
		if (MatchWildcard(alpWildcards[i], wszExt))
			return TRUE;
	}
	return FALSE;

}

static JET_ERR DownloadFTP(URL_COMP *pURL, LPCWSTR *alpWildcards, LPCWSTR lpDownloadPath, LPWSTR lpLocalFile)
{
	JET_ERR ret;
	DWORD dwFileOffs;
	HINTERNET hInternet, hFind;
	WIN32_FIND_DATAA findfile={0};
	char szLocalFile[MAX_PATH];
	char szDir[_MAX_DIR], szFname[_MAX_FNAME], szExt[_MAX_EXT], szFile[MAX_PATH];

	if ((ret = ConnectFTP(pURL, &hInternet)) != JET_errSuccess)
		return ret;
	
	dwFileOffs = WideCharToMultiByte(CP_ACP, 0, lpDownloadPath, -1, szLocalFile, sizeof(szLocalFile), NULL, FALSE);
	_splitpath(pURL->szURLPath, NULL, szDir, szFname, szExt);
	strcpy (szFile, szFname);
	strcat (szFile, szExt);
	strcat (szLocalFile, szFile);
	MultiByteToWideChar(CP_ACP, 0, szLocalFile, -1, lpLocalFile, MAX_PATH);
	if (!_FtpGetFileA(hInternet, szFile, szLocalFile, FALSE, 0, FTP_TRANSFER_TYPE_BINARY, 0))
	{
		_InternetCloseHandle(hInternet);
		return CheckReturn();
	}
	if (!alpWildcards)
	{
		_InternetCloseHandle(hInternet);
		return JET_errSuccess;
	}
	strcpy(&szFile[strlen(szFname)], ".*");
	ret = JET_errSuccess;
	if ((hFind = _FtpFindFirstFileA(hInternet, szFile, &findfile, 0, 0)))
	{
		do
		{
			if (MatchWildcards(findfile.cFileName, alpWildcards))
			{
				strcpy (&szLocalFile[dwFileOffs], findfile.cFileName);
				if (!_FtpGetFileA(hInternet, findfile.cFileName, szLocalFile, 
					FALSE, 0, FTP_TRANSFER_TYPE_BINARY, 0))
					break;
			}
		}
		while (_InternetFindNextFileA(hFind, &findfile));
		_InternetCloseHandle(hFind);
	}
	if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		ret = CheckReturn();
		if (!hFind && ret==JET_errObjectNotFound) ret=JET_errSuccess;
	}
	
	_InternetCloseHandle(hInternet);
	return ret;
}

static JET_ERR HTTPStatusToJETERR(DWORD HTTPStatus)
{
	switch (HTTPStatus)
	{
	case HTTP_STATUS_DENIED:
		return JET_errAccessDenied;
	case HTTP_STATUS_NOT_FOUND:
		return JET_errObjectNotFound;
	case HTTP_STATUS_REQUEST_TIMEOUT:
		return JET_errNetTimeout;
	default:
		return JET_errNetStatusUnknown;
	}
}

static JET_ERR DownloadHTTP(LPCWSTR lpURL, URL_COMP *pURL, LPCWSTR lpDownloadPath, LPWSTR lpLocalFile)
{
	JET_ERR ret = JET_errSuccess;
	HINTERNET hFTP, hUrl;
	char szLocalFile[MAX_PATH];
	char szDir[_MAX_DIR], szFname[_MAX_FNAME], szExt[_MAX_EXT], szFile[MAX_PATH];
	char szURL[521], szCanonUrl[521];
	DWORD dwBufferLength;

	WideCharToMultiByte(CP_ACP, 0, lpDownloadPath, -1, szLocalFile, sizeof(szLocalFile), NULL, FALSE);
	_splitpath(pURL->szURLPath, NULL, szDir, szFname, szExt);
	strcpy (szFile, szFname);
	strcat (szFile, szExt);
	strcat (szLocalFile, szFile);
	MultiByteToWideChar(CP_ACP, 0, szLocalFile, -1, lpLocalFile, MAX_PATH);
	if (hFTP = _InternetConnectA(m_hInternet, pURL->szHostName, INTERNET_INVALID_PORT_NUMBER ,
		NULL, NULL, INTERNET_SERVICE_FTP, 0, 0))
	{
		if (_FtpSetCurrentDirectoryA(hFTP, szDir))
		{
			_InternetCloseHandle(hFTP);
			return JET_errNetResolveHost;
		}
		_InternetCloseHandle(hFTP);
		dwBufferLength = sizeof(szCanonUrl);
		WideCharToMultiByte(CP_ACP, 0, lpURL, -1, szURL, sizeof(szURL), NULL, FALSE);
		if (_InternetCanonicalizeUrlA(szURL, szCanonUrl, &dwBufferLength, 0))
		{
			if (hUrl = _InternetOpenUrlA(m_hInternet, szCanonUrl, "Accept: */*", -1L, 0, 0))
			{
				int HTTPStatus;
				DWORD cbStatus = sizeof(HTTPStatus);

				if (_HttpQueryInfoA(hUrl, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, 
					&HTTPStatus, &cbStatus, NULL))
				{
					if (HTTPStatus == HTTP_STATUS_OK)
					{
						short cret;
						HANDLE hFile;

						if ((cret = DOSCreateFile(lpLocalFile, &hFile)) == 0)
						{
							BYTE Buffer[4096];
							DWORD dwWritten;

							if (_InternetReadFile(hUrl, Buffer, sizeof(Buffer), &dwWritten))
							{
								while (dwWritten)
								{
									if (DOSWriteFile(hFile, Buffer, dwWritten) != -1)
									{
										if (!_InternetReadFile(hUrl, Buffer, sizeof(Buffer), &dwWritten))
										{
											ret = CheckReturn();
											break;
										}
									}
									else
									{
										ret = GetLastError()==ERROR_DISK_FULL?JET_errDiskFull:JET_errDiskIO;
										break;
									}
								}
								
								if (DOSCloseFile(hFile))
									ret = GetLastError()==ERROR_DISK_FULL?JET_errDiskFull:JET_errDiskIO;
							}
							if (ret != JET_errSuccess) DOSFileDelete(lpLocalFile);
						}
						else
						{
							ret = cret==-ERROR_TOO_MANY_OPEN_FILES?JET_errTooManyOpenFiles:JET_errOpenFile;
						}
					}
					else
					{
						ret = HTTPStatusToJETERR(HTTPStatus);
					}
				}
				_InternetCloseHandle(hUrl);
			}
		}
	}
	return ret;
}



/* The ANSI APIs are used here, as Win9x doesn't implement the Wide APIs */
BOOL NetInitializeInternetServices()
{
	if (m_hLibWinInet = JetLoadLibraryW(L"wininet.dll"))
	{
		if ( (_InternetOpenA = (ft_InternetOpenA) GetProcAddress(m_hLibWinInet, "InternetOpenA"))
		  && (_InternetOpenUrlA = (ft_InternetOpenUrlA)GetProcAddress(m_hLibWinInet, "InternetOpenUrlA"))
		  && (_InternetCanonicalizeUrlA = (ft_InternetCanonicalizeUrlA)GetProcAddress(m_hLibWinInet, "InternetCanonicalizeUrlA"))
		  && (_InternetConnectA = (ft_InternetConnectA)GetProcAddress(m_hLibWinInet, "InternetConnectA"))
		  && (_InternetReadFile = (ft_InternetReadFile)GetProcAddress(m_hLibWinInet, "InternetReadFile"))
		  && (_InternetWriteFile = (ft_InternetWriteFile)GetProcAddress(m_hLibWinInet, "InternetWriteFile"))
		  && (_InternetCloseHandle = (ft_InternetCloseHandle)GetProcAddress(m_hLibWinInet, "InternetCloseHandle"))
		  && (_FtpGetFileA = (ft_FtpGetFileA)GetProcAddress(m_hLibWinInet, "FtpGetFileA"))
		  && (_FtpPutFileA = (ft_FtpPutFileA)GetProcAddress(m_hLibWinInet, "FtpPutFileA"))
		  && (_FtpDeleteFileA = (ft_FtpDeleteFileA)GetProcAddress(m_hLibWinInet, "FtpDeleteFileA"))
		  && (_FtpFindFirstFileA = (ft_FtpFindFirstFileA)GetProcAddress(m_hLibWinInet, "FtpFindFirstFileA"))
		  && (_InternetFindNextFileA = (ft_InternetFindNextFileA)GetProcAddress(m_hLibWinInet, "InternetFindNextFileA"))
		  && (_FtpSetCurrentDirectoryA = (ft_FtpSetCurrentDirectoryA)GetProcAddress(m_hLibWinInet, "FtpSetCurrentDirectoryA"))
		  && (_InternetGetLastResponseInfoA = (ft_InternetGetLastResponseInfoA)GetProcAddress(m_hLibWinInet, "InternetGetLastResponseInfoA"))
		  && (_InternetCrackUrlA = (ft_InternetCrackUrlA)GetProcAddress(m_hLibWinInet, "InternetCrackUrlA"))
		  && (_HttpQueryInfoA = (ft_HttpQueryInfoA)GetProcAddress(m_hLibWinInet, "HttpQueryInfoA"))
		  && (m_hInternet = _InternetOpenA("JET IISAM", 0, 0, 0, 0)))
		{
			// Optional, introduced by us to speed up things a bit on NT or higher
			_FtpFindFirstFileW = (ft_FtpFindFirstFileW)GetProcAddress(m_hLibWinInet, "FtpFindFirstFileW");
			return TRUE;
		}
		else
		{
		  FreeLibrary(m_hLibWinInet);
		  m_hLibWinInet = NULL;
		}
	}
	return FALSE;
}

BOOL NetTerminateInternetServices()
{
	BOOL bRet = FALSE;

	if (m_hLibWinInet)
	{
		_InternetCloseHandle(m_hInternet);
		bRet = FreeLibrary(m_hLibWinInet);
		m_hLibWinInet = NULL;
	}
	return bRet;
}

LPWSTR NetCreateLocalDirectory(LPWSTR lpDirectory)
{
	WCHAR wszDir[MAX_PATH];

	*lpDirectory = 0;
	DOSFindLocalDirectory(wszDir);
	if ( DOSUniquePathname(lpDirectory, wszDir, L"NET", L".TMP") || DOSMakeDirectory(lpDirectory) )
		wcscpy(lpDirectory, wszDir);
	return wcscat(lpDirectory, L"\\");
}

short NetDestroyLocalDirectory(LPCWSTR lpDirectory)
{
  return DOSRemoveDirectory(lpDirectory);
}

JET_ERR NetDirectoryExists(LPCWSTR lpDirectory, JET_ERR *pRet)
{
	URL_COMP URL={0};
	HINTERNET hInternet;

	*pRet = JET_errSuccess;
	if (!m_hInternet && !NetInitializeInternetServices())
		return *pRet = JET_errNetInitialize;
	if (!CrackURL(lpDirectory, &URL))
		return *pRet = JET_errNetLoginFailed;
	if (URL.nScheme != INTERNET_SCHEME_FTP && URL.nScheme != INTERNET_SCHEME_HTTP)
		return *pRet = JET_errObjectNotFound;
	if (!(hInternet = _InternetConnectA(m_hInternet, URL.szHostName, 
		INTERNET_INVALID_PORT_NUMBER, 
		*URL.szUserName?URL.szUserName:NULL, 
		*URL.szPassword?URL.szUserName:NULL,
		INTERNET_SERVICE_FTP, 0, 0)))
		return *pRet = CheckReturn();
	if (!(!*URL.szURLPath || (*URL.szURLPath=='/' && !URL.szURLPath[1]) || 
		_FtpSetCurrentDirectoryA(hInternet, URL.szURLPath)))
		*pRet = CheckReturn();
	_InternetCloseHandle(hInternet);
	return *pRet;
}

JET_ERR NetDownloadToLocal(LPCWSTR lpURL, LPCWSTR *alpWildcards, LPCWSTR lpDownloadPath, LPWSTR lpLocalFile)
{
	URL_COMP URL={0};

	if (!m_hInternet && !NetInitializeInternetServices())
		return JET_errNetInitialize;
	if (!CrackURL(lpURL, &URL))
		return JET_errNetLoginFailed;
	switch (URL.nScheme)
	{
	case INTERNET_SCHEME_FTP:
		return DownloadFTP(&URL, alpWildcards, lpDownloadPath, lpLocalFile);
	case INTERNET_SCHEME_HTTP:
		return DownloadHTTP(lpURL, &URL, lpDownloadPath, lpLocalFile);
	}
	return JET_errObjectNotFound;
}

JET_ERR NetFileExists(LPCWSTR lpURL)
{
	URL_COMP URL={0};
	char szURL[521], szCanonUrl[521];
	DWORD dwBufferLength;
	HINTERNET hUrl;

	if (!m_hInternet && !NetInitializeInternetServices())
		return JET_errNetInitialize;
	if (!CrackURL(lpURL, &URL))
		return JET_errNetLoginFailed;
	if (URL.nScheme != INTERNET_SCHEME_FTP && URL.nScheme != INTERNET_SCHEME_HTTP)
		return JET_errInvalidPath;
	dwBufferLength = sizeof(szCanonUrl);
	WideCharToMultiByte(CP_ACP, 0, lpURL, -1, szURL, sizeof(szURL), NULL, FALSE);
	if (_InternetCanonicalizeUrlA(szURL, szCanonUrl, &dwBufferLength, 0))
	{
		if (hUrl = _InternetOpenUrlA(m_hInternet, szCanonUrl, NULL, 0, 0, 0))
		{
			_InternetCloseHandle(hUrl);
			return JET_errSuccess;
		}
	}
	return CheckReturn();
}

JET_ERR NetFileInfo(LPCWSTR lpURL, WIN32_FIND_DATAW *pInfo)
{
	URL_COMP URL={0};
	JET_ERR ret;
	HINTERNET hInternet, hFind;
	WIN32_FIND_DATAA finddata={0};
	char szURL[521];

	if (!m_hInternet && !NetInitializeInternetServices())
		return JET_errNetInitialize;
	if (!CrackURL(lpURL, &URL))
		return JET_errNetLoginFailed;
	if ((ret = ConnectFTP(&URL, &hInternet)) != JET_errSuccess)
		return ret;
	if (g_bUseUnicode && _FtpFindFirstFileW)
	{
		if (!(hFind = _FtpFindFirstFileW(hInternet, lpURL, pInfo, 0, 0)))
			ret = GetLastError() == ERROR_NO_MORE_FILES?JET_errObjectNotFound:CheckReturn();
		else _InternetCloseHandle(hFind);
		_InternetCloseHandle(hInternet);
		return ret;
	}
	/* No UNICODE Windows, take the long route */
	WideCharToMultiByte(CP_ACP, 0, lpURL, -1, szURL, sizeof(szURL), NULL, FALSE);
	if (!(hFind = _FtpFindFirstFileA(hInternet, szURL, &finddata, 0, 0)))
		ret = GetLastError() == ERROR_NO_MORE_FILES?JET_errObjectNotFound:CheckReturn();
	else 
	{
		pInfo->dwFileAttributes = finddata.dwFileAttributes;
		pInfo->ftCreationTime   = finddata.ftCreationTime;
		pInfo->ftLastAccessTime = finddata.ftLastAccessTime;
		pInfo->ftLastWriteTime  = finddata.ftLastWriteTime;
		pInfo->nFileSizeHigh    = finddata.nFileSizeHigh;
		pInfo->nFileSizeLow     = finddata.nFileSizeLow;
		pInfo->dwReserved0      = finddata.dwReserved0;
		pInfo->dwReserved1      = finddata.dwReserved1;
		MultiByteToWideChar (CP_ACP, 0, finddata.cFileName, -1, pInfo->cFileName, sizeof(finddata.cFileName));
		MultiByteToWideChar (CP_ACP, 0, finddata.cAlternateFileName, -1, pInfo->cAlternateFileName, 
			sizeof(finddata.cAlternateFileName));
		_InternetCloseHandle(hFind);
	}
	_InternetCloseHandle(hInternet);
	return ret;
}

JET_ERR NetFindFirstMatchingFile(LPCWSTR lpURL, LPWSTR lpFileName, NET_FIND_HANDLE *pConn, JET_ERR *pRet)
{
	URL_COMP URL={0};
	JET_ERR ret;
	HINTERNET hInternet, hFind;
	WIN32_FIND_DATAA finddata={0};
	char szFname[_MAX_FNAME], szExt[_MAX_EXT], szFile[MAX_PATH];	

	if (!m_hInternet && !NetInitializeInternetServices())
		return JET_errNetInitialize;
	if (!CrackURL(lpURL, &URL))
		return JET_errNetLoginFailed;
	if ((ret = ConnectFTP(&URL, &hInternet)) != JET_errSuccess)
		return *pRet = ret;
	_splitpath(URL.szURLPath, NULL, NULL, szFname, szExt);
	strcpy (szFile, szFname);
	strcat (szFile, szExt);
	if (hFind = _FtpFindFirstFileA(hInternet, szFile, &finddata, 0, 0))
	{
		MultiByteToWideChar(CP_ACP, 0, finddata.cFileName, -1, lpFileName, sizeof(finddata.cFileName));
		pConn->hInternet = hInternet;
		pConn->hFind = hFind;
		return JET_errSuccess;
	}
	if (GetLastError() == ERROR_NO_MORE_FILES)
		ret = JET_errObjectNotFound;
	else ret = *pRet = CheckReturn();
	_InternetCloseHandle(hInternet);
	return ret;
}

JET_ERR NetFindNextMatchingFile(LPWSTR lpFileName, NET_FIND_HANDLE *pConn, JET_ERR *pRet)
{
	WIN32_FIND_DATAA finddata;
	JET_ERR ret;

	if (_InternetFindNextFileA(pConn->hFind, &finddata))
	{
		MultiByteToWideChar(CP_ACP, 0, finddata.cFileName, -1, lpFileName, sizeof(finddata.cFileName));
		return JET_errSuccess;
	}
	if (GetLastError() == ERROR_NO_MORE_FILES)
		ret = JET_errObjectNotFound;
	else ret = *pRet = CheckReturn();
	_InternetCloseHandle(pConn->hFind);
	_InternetCloseHandle(pConn->hInternet);
	return ret;
}

enum eProtoType NetProtocolType(LPCWSTR lpURL)
{
	// We could use CrackURL, but as there are only 2 types, 
	// wcsncmp should be faster...
	if (wcsncmp(lpURL, L"http:", 5) == 0)
		return Proto_HTTP;
	else if (wcsncmp(lpURL, L"ftp:", 4) == 0)
		return Proto_FTP;
	return Proto_Other;
}


JET_ERR NetLocalCleanup(LPCWSTR lpFile, LPCWSTR *alpWildcards)
{
	DOSFileDelete(lpFile);
	if (alpWildcards)
	{
		int i;
		DWORD dwOffs;
		WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFname[_MAX_FNAME], 
			  szFile[MAX_PATH], szFound[MAX_PATH];
		DOSFIND dosfind;
		JET_ERR ret;

		_wsplitpath(lpFile, szDrive, szDir, szFname, NULL);
		dwOffs = swprintf(szFile, L"%s%s", szDrive, szDir);
		for (i=0; alpWildcards[i]; i++)
		{
			wcscpy(&szFile[dwOffs], alpWildcards[i]);
			if (DOSFindFirstMatchingFile(szFile, szFound, &dosfind, &ret))
			{
				do
				{
					wcscpy(&szFile[dwOffs], szFound);
					DOSFileDelete(szFile);
				}
				while (DOSFindNextMatchingFile(szFound, &dosfind, &ret));
			}
		}
	}
	return JET_errSuccess;
}

JET_ERR NetDeleteFile(LPCWSTR lpFile)
{
	URL_COMP URL={0};
	JET_ERR ret = JET_errSuccess;
	HINTERNET hInternet;
	char szFname[_MAX_FNAME], szExt[_MAX_EXT], szFile[MAX_PATH];

	if (!m_hInternet && !NetInitializeInternetServices())
		return JET_errNetInitialize;
	if (!CrackURL(lpFile, &URL))
		return JET_errNetLoginFailed;
	if ((ret = ConnectFTP(&URL, &hInternet)) != JET_errSuccess)
		return ret;
	_splitpath(URL.szURLPath, NULL, NULL, szFname, szExt);
	strcpy (szFile, szFname);
	strcat (szFile, szExt);
	if (!_FtpDeleteFileA(hInternet, szFile))
		ret = CheckReturn();
	_InternetCloseHandle(hInternet);
	return ret;
}

JET_ERR NetUploadToNet(LPCWSTR lpLocalFile, LPCWSTR *alpWildcards, LPCWSTR lpURL)
{
	URL_COMP URL={0};
	JET_ERR ret, findret;
	HINTERNET hInternet;
	int i;
	WCHAR szwDrive[_MAX_DRIVE], szwDir[_MAX_DIR], szwFname[_MAX_FNAME], szwExt[_MAX_EXT], 
		  szwFile[MAX_PATH], *pwszExt, szwFoundFile[MAX_PATH];
	char szLocalFile[MAX_PATH], *pszFile;

	if (!m_hInternet && !NetInitializeInternetServices())
		return JET_errNetInitialize;
	if (!CrackURL(lpURL, &URL))
		return JET_errNetLoginFailed;
	if ((ret = ConnectFTP(&URL, &hInternet)) != JET_errSuccess)
		return ret;
	WideCharToMultiByte(CP_ACP, 0, lpLocalFile, -1, szLocalFile, sizeof(szLocalFile), NULL, FALSE);
	_wsplitpath(lpLocalFile, szwDrive, szwDir, szwFname, szwExt);
	wcscpy(szwFile, szwDrive);
	wcscat(szwFile, szwDir);
	pszFile = &szLocalFile[wcslen(szwFile)];
	wcscat(szwFile, szwFname);
	pwszExt = &szwFile[wcslen(szwFile)];
	if (_FtpPutFileA(hInternet, szLocalFile, pszFile, INTERNET_FLAG_TRANSFER_BINARY, 0))
	{
		if (alpWildcards)
		{
			DOSFIND find;

			for (i=0; alpWildcards[i] && ret==JET_errSuccess; i++)
			{
				wcscpy(pwszExt, alpWildcards[i]);
				if (DOSFindFirstMatchingFile(szwFile, szwFoundFile, &find, &findret))
					continue;
				do
				{
					WideCharToMultiByte(CP_ACP, 0, szwFoundFile, -1, pszFile, (pszFile-szLocalFile), 0, 0);
					if (!_FtpPutFileA(hInternet, szLocalFile, pszFile, INTERNET_FLAG_TRANSFER_BINARY, 0))
					{
						ret = CheckReturn();
						break;
					}
				}
				while (DOSFindNextMatchingFile(szwFile, &find, &findret) == 0);
			}
		}
	} else ret=CheckReturn();
	_InternetCloseHandle(hInternet);
	return ret;
}
