#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include "iisam.h"
#include "isamutil.h"
#include "jetuwrap.h"

static ISAM_Task *ISAMDB_Task_list = NULL;
static JET_SESID m_CurrentSession = 0;
static DWORD m_dwSessions;

typedef int (WINAPI *ft_DBCompareStringW)(
    LCID Locale,
    DWORD dwCmpFlags,
    UNALIGNED WCHAR * lpString1,
    int cchCount1,
    UNALIGNED WCHAR * lpString2,
    int cchCount2);
ft_DBCompareStringW DBCompareStringW = NULL;

static LPWSTR ConvertErrString(LPCWSTR lpInput);

DWORD CurrentTaskHandle()
{
	return GetCurrentProcessId();
}

void ISAMDBInitialize()
{
	HMODULE hMsWstr;

	ISAMDB_Task_list = NULL;
	m_dwSessions  = 1;
	/* Originally, DBCompareStringW is bound via IAT by M$, but as there is 
	 * no official lib file for it, we're loading it manually by Ordinal 
	 * here */
	if (hMsWstr = JetLoadLibraryW(L"mswstr10.dll"))
		DBCompareStringW = (ft_DBCompareStringW)GetProcAddress(hMsWstr, (LPCSTR)1);
}

int DBlstrcmpi(LPCWSTR lpString1, LPCWSTR lpString2)
{
	return DBCompareStringW(GetUserDefaultLCID(), NORM_IGNORECASE | NORM_IGNOREWIDTH | NORM_IGNOREKANATYPE,
		(UNALIGNED WCHAR *)lpString1, -1, (UNALIGNED WCHAR *)lpString2, -1) - 2;
}

// ----------------------------------------------------------------------

ISAM_Task *ISAMDBTaskList()
{
	return ISAMDB_Task_list;
}

ISAM_Task *ISAMDBAddTask(DWORD dwTaskHandle)
{
	ISAM_Task *pTask;

	if (pTask = MemAllocate(sizeof(ISAM_Task)))
	{
		pTask->next = ISAMDB_Task_list;
		pTask->dwUsers   = 1;
		pTask->dwObjects = 1;
		pTask->dwTaskHandle = dwTaskHandle;
		ISAMDB_Task_list = pTask;
	}
	return pTask;
}

ISAM_Task *ISAMDBFindTask(DWORD dwTaskHandle)
{
	ISAM_Task *pTask;

	for (pTask = ISAMDB_Task_list; pTask; pTask = pTask->next)
		if (pTask->dwTaskHandle == dwTaskHandle) return pTask;
	return NULL;
}

ISAM_Cursor *ISAMDBFindCursor(JET_SESID sesid, JET_HANDLE cursorid)
{
	ISAM_Task *pTask;
	ISAM_DB *pDB;
	ISAM_DBTable *pTable;
	ISAM_Cursor *pCursor;

	if (!ISAMDB_Task_list) return NULL;
	for (pTask = ISAMDB_Task_list; pTask; pTask = pTask->next)
	{
		if (pTask->pSession && pTask->pSession->sesid == sesid)
		{
			ErrorSetSession(pTask->pSession->sesid);
			for (pDB=pTask->pDatabase; pDB; pDB = pDB->next)
			{
				for (pTable = pDB->pTables; pTable; pTable = pTable->next)
				{
					for (pCursor = pTable->pCursors; pCursor; pCursor = pCursor->next)
					{
						if (pCursor->cursorid == cursorid)
							return pCursor;
					}
				}
			}
		}
	}
	return NULL;
}

ISAM_Cursor *ISAMDBFindCursorFromExternalTID(JET_SESID sesid, JET_TABLEID externalTID)
{
	ISAM_Task *pTask;
	ISAM_DB *pDB;
	ISAM_DBTable *pTable;
	ISAM_Cursor *pCursor;

	if (!ISAMDB_Task_list) return NULL;
	for (pTask = ISAMDB_Task_list; pTask; pTask = pTask->next)
	{
		if (pTask->pSession && pTask->pSession->sesid == sesid)
		{
			ErrorSetSession(pTask->pSession->sesid);
			for (pDB=pTask->pDatabase; pDB; pDB = pDB->next)
			{
				for (pTable = pDB->pTables; pTable; pTable = pTable->next)
				{
					for (pCursor = pTable->pCursors; pCursor; pCursor = pCursor->next)
					{
						if (pCursor->externalTID == externalTID)
							return pCursor;
					}
				}
			}
		}
	}
	return NULL;
}

ISAM_DB *ISAMDBFindDatabase(ISAM_Task *pTask, LPCWSTR lpDBName)
{
	ISAM_DB *pDB;

	for (pDB = pTask->pDatabase; pDB; pDB = pDB->next)
	{
		if (!DBlstrcmpi(pDB->szDBName, lpDBName))
			return pDB;
	}
	return NULL;
}

ISAM_DBUser *ISAMDBFindDatabaseUser(JET_SESID sesid, JET_HANDLE userid)
{
	ISAM_Task *pTask;
	ISAM_DB *pDB;
	ISAM_DBUser *pUser;

	for (pTask = ISAMDB_Task_list; pTask; pTask = pTask->next)
	{
		if (pTask->pSession && pTask->pSession->sesid == sesid)
		{
			ErrorSetSession(pTask->pSession->sesid);
			for (pDB=pTask->pDatabase; pDB; pDB = pDB->next)
			{
				for (pUser=pDB->pUsers; pUser; pUser=pUser->next)
				{
					if (pUser->userid == userid)
						return pUser;
				}
			}
		}
	}
	return NULL;
}

ISAM_NetFile *ISAMDBFindNetFile(ISAM_DB *pDB, LPCWSTR lpRemoteFile)
{
	ISAM_NetFile *pNetFile;

	for (pNetFile = pDB->pNetFiles; pNetFile; pNetFile = pNetFile->next)
		if (wcsicmp(pNetFile->pszRemoteFile, lpRemoteFile) == 0)
			return pNetFile;
	return NULL;
}

ISAM_Session *ISAMDBFindSession(JET_SESID sesid)
{
	ISAM_Task *pTask;

	for (pTask = ISAMDB_Task_list; pTask; pTask = pTask->next)
	{
		if (pTask->pSession && pTask->pSession->sesid == sesid)
		{
			ErrorSetSession(pTask->pSession->sesid);
			return pTask->pSession;
		}
	}
	return NULL;
}


void ISAMDBDeleteTask(ISAM_Task *pTask)
{
	ISAM_Task *pTaskIter, *pTaskPrev = NULL;
	ISAM_Session *pSession, *pSessNext;
	ISAM_DB *pDatabase, *pDBNext;
	
	for (pTaskIter = ISAMDB_Task_list; pTaskIter != pTask; pTaskIter = pTaskIter->next)
	{
		pTaskPrev = pTaskIter;
		if (!pTaskIter) return;
	}
	for (pSession = pTask->pSession; pSession; pSession=pSessNext)
	{
		pSessNext = pSession->next;
		ISAMDBDeleteSession(pTask, pSession);
	}
	for (pDatabase = pTask->pDatabase ; pDatabase; pDatabase=pDBNext)
	{
		pDBNext = pDatabase->next;
		ISAMDBDeleteDatabase(pTask, pDatabase);
	}
	if (pTaskPrev) pTaskPrev->next = pTask->next;
	else ISAMDB_Task_list = pTask->next;
	MemFree(pTask);
}

JET_ERR ISAMDBDeleteCursor(ISAM_DBTable *pTable, ISAM_Cursor *pCursor)
{
	ISAM_Cursor *pCursorIter, *pCursorPrev = NULL;

	for (pCursorIter = pTable->pCursors; pCursorIter != pCursor; pCursorIter = pCursorIter->next)
	{
		pCursorPrev = pCursorIter;
		if (!pCursorIter) return JET_errInvalidTableId;
	}
	if (pCursorPrev) pCursorPrev->next = pCursor->next;
	else pTable->pCursors = pCursor->next;
	MemFree(pCursor);
	return JET_errSuccess;
}

void ISAMDBDeleteDatabase(ISAM_Task *pTask, ISAM_DB *pDatabase)
{
	ISAM_DB *pDBIter, *pDBPrev = NULL;
	ISAM_DBTable *pTable, *pTableNext;
	ISAM_DBUser *pUser, *pUserNext;
	
	for (pDBIter = pTask->pDatabase; pDBIter != pDatabase; pDBIter = pDBIter->next)
	{
		pDBPrev = pDBIter;
		if (!pDBIter) return;
	}
	for (pTable = pDatabase->pTables; pTable ; pTable=pTableNext)
	{
		pTableNext = pTable->next;
		ISAMDBDeleteTable(pDatabase, pTable);
	}
	for (pUser = pDatabase->pUsers ; pUser; pUser=pUserNext)
	{
		pUserNext = pUser->next;
		ISAMDBDeleteDatabaseUser(pDatabase, pUser);
	}
	if (pDBPrev) pDBPrev->next = pDatabase->next;
	else pTask->pDatabase = pDatabase->next;
	MemFree(pDatabase);
}

void ISAMDBDeleteDatabaseUser(ISAM_DB *pDB, ISAM_DBUser *pUser)
{
	ISAM_DBUser *pUserIter, *pUserPrev = NULL;
	BOOL bRet=FALSE;

	for (pUserIter = pDB->pUsers; pUserIter != pUser; pUserIter = pUserIter->next)
	{
		pUserPrev = pUserIter;
		if (!pUserIter) return;
	}
	if (pUserPrev) pUserPrev->next = pUser->next;
	else pDB->pUsers = pUser->next;
	//if (pUser->ColumnList) DestroyColumnList(pUser->ColumnList);
	if (pUser->settings) MemFree(pUser->settings);
	MemFree(pUser);
}

void ISAMDBDeleteSession(ISAM_Task *pTask, ISAM_Session *pSession)
{
	ISAM_Session *pSessIter, *pSessPrev = NULL;
	ISAM_VTDef *pVTDef, *pVTDefNext;
	ISAM_Trace *pTrace, *pTraceNext;
	
	for (pSessIter = pTask->pSession; pSessIter != pSession; pSessIter = pSessIter->next)
	{
		pSessPrev = pSessIter;
		if (!pSessIter) return;
	}
	for (pVTDef = pSession->pVTDef; pVTDef ; pVTDef=pVTDefNext)
	{
		pVTDefNext = pVTDef->next;
		MemFree(pVTDef);
	}
	for (pTrace = pSession->pTrace; pTrace ; pTrace=pTraceNext)
	{
		pTraceNext = pTrace->next;
		MemFree(pTrace);
	}

	if (pSessPrev) pSessPrev->next = pSession->next;
	else pTask->pSession = pSession->next;
	MemFree(pSession);
}

JET_ERR ISAMDBDeleteTable(ISAM_DB *pDB, ISAM_DBTable *pTable)
{
	ISAM_DBTable *pTableIter, *pTablePrev = NULL;
	ISAM_Cursor *pCursor, *pCursorNext;
	
	for (pTableIter = pDB->pTables; pTableIter != pTable; pTableIter = pTableIter->next)
	{
		pTablePrev = pTableIter;
		if (!pTableIter) 
			return JET_errInvalidTableId;
	}
	ISAMDBDeleteColumns(pTable);
	for (pCursor = pTable->pCursors; pCursor ; pCursor=pCursorNext)
	{
		pCursorNext = pCursor->next;
		ISAMDBDeleteCursor(pTable, pCursor);
	}
	if (pTablePrev) pTablePrev->next = pTable->next;
	else pDB->pTables = pTable->next;
	MemFree(pTable);
	return JET_errSuccess;
}

JET_ERR ISAMDBDeleteVTDef(ISAM_Session *pSession, ISAM_VTDef *pVTDef)
{
	ISAM_VTDef *pVTDIter, *pVTDPrev = NULL;
	PVOID pData, pDataNext;

	for (pVTDIter = pSession->pVTDef; pVTDIter != pVTDef; pVTDIter = pVTDIter->next)
	{
		pVTDPrev = pVTDIter;
		if (!pVTDIter) 
			return JET_errInvalidTableId;
	}
	if (pVTDPrev) pVTDPrev->next = pVTDef->next;
	else pSession->pVTDef = pVTDef->next;
	
	for (pData = pVTDef->data.pData; pData ; pData=pDataNext)
	{
		pDataNext = *(PVOID*)pData;
		MemFree(pData);
	}
	MemFree(pVTDef);
	return JET_errSuccess;
}

JET_ERR ISAMDBDeleteColumn(ISAM_DBTable *pTable, LPCWSTR pszColumn)
{
	ISAM_Column *pColumnIter, *pColumnPrev = NULL;

	for (pColumnIter = pTable->pColumns; pColumnIter; pColumnIter = pColumnIter->next)
	{
		if (!DBlstrcmpi(pColumnIter->szColumnName, pszColumn)) break;
		pColumnPrev = pColumnIter;
	}
	if (!pColumnIter) return JET_errColumnNotFound;
	if (pColumnPrev) pColumnPrev->next = pColumnIter->next;
	else pTable->pColumns = pColumnIter->next;
	pTable->nColumns--;
	MemFree(pColumnIter);
	return JET_errSuccess;
}

JET_ERR ISAMDBDeleteColumns(ISAM_DBTable *pTable)
{
	ISAM_Column *pColumn, *pColumnNext;
	for (pColumn = pTable->pColumns; pColumn ; pColumn=pColumnNext)
	{
		pColumnNext = pColumn->next;
		MemFree(pColumn);
	}
	pTable->pColumns = NULL;
	pTable->nColumns = 0;
	return JET_errSuccess;
}

JET_CALLIN *ISAMCurrentTaskCallbacks()
{
	return ISAMDBFindTask(CurrentTaskHandle())->pCaller;
}

ISAM_DB *ISAMDBAddDatabase(ISAM_Task *pTask, WCHAR *pwszDBName)
{
	ISAM_DB *pDB;

	if (pDB=MemAllocate(sizeof(ISAM_DB)+sizeof(WCHAR)*wcslen(pwszDBName)))
	{
		pDB->pTask = pTask;
		pDB->next = pTask->pDatabase;
		pTask->pDatabase = pDB;
		wcscpy(pDB->szDBName, pwszDBName);
	}
	return pDB;
}

ISAM_DBUser *ISAMDBAddDatabaseUser(ISAM_Session *pSession, ISAM_DB *pDB, LPCWSTR pwszConnect)
{
	ISAM_DBUser *pDBUser;

	if (pDBUser=MemAllocate(sizeof(ISAM_DBUser)+sizeof(WCHAR)*wcslen(pwszConnect)))
	{
		pDBUser->next = pDB->pUsers;
		pDB->pUsers = pDBUser;
		pDBUser->pDB = pDB;
		pDBUser->pSession = pSession;
		wcscpy(pDBUser->szConnectStr, pwszConnect);
		pDBUser->userid = pDB->pTask->dwUsers++;
	}
	return pDBUser;
}

ISAM_DBTable *ISAMDBAddTable(ISAM_DB *pDB, LPCWSTR lpTableName)
{
	ISAM_DBTable *pDBTable;

	if (pDBTable=MemAllocate(sizeof(ISAM_DBTable)))
	{
		pDBTable->next = pDB->pTables;
		pDB->pTables = pDBTable;
		pDBTable->pDB = pDB;
		pDBTable->dwUnk1 = pDB->dwUnk1;
		wcsncpy(pDBTable->szTableName, lpTableName, sizeof(pDBTable->szTableName)/sizeof(WCHAR));
	}
	return pDBTable;
}

ISAM_Cursor *ISAMDBAddCursor(ISAM_DBUser *pUser, ISAM_DBTable *pTable)
{
	ISAM_Cursor *pCursor;

	if (pCursor=MemAllocate(sizeof(ISAM_Cursor)))
	{
		pCursor->next     = pTable->pCursors;
		pTable->pCursors  = pCursor;
		pCursor->pTable   = pTable;
		pCursor->pUser    = pUser;
		pCursor->cursorid = pTable->pDB->pTask->dwObjects++;
	}
	return pCursor;
}

ISAM_NetFile *ISAMDBAddNetFile(ISAM_DB *pDB, LPCWSTR lpRemote, LPCWSTR lpLocal, BOOL bUploadOnExit)
{
	ISAM_NetFile *pNetFile;
	DWORD cbLocal, cbRemote;

	cbLocal = (wcslen(lpLocal)+1)*sizeof(WCHAR);
	cbRemote = (wcslen(lpRemote)+1)*sizeof(WCHAR);
	if (pNetFile = MemAllocate(sizeof(ISAM_NetFile)+cbLocal+cbRemote))
	{
		pNetFile->next = pDB->pNetFiles;
		pDB->pNetFiles = pNetFile;
		pNetFile->bUploadOnExit = bUploadOnExit;
		pNetFile->pszRemoteFile = (LPWSTR)((BYTE*)pNetFile+sizeof(ISAM_NetFile));
		wcscpy(pNetFile->pszRemoteFile, lpRemote);
		pNetFile->pszLocalFile = (LPWSTR)((BYTE*)pNetFile->pszRemoteFile+cbRemote);
		wcscpy(pNetFile->pszLocalFile, lpLocal);
	}
	return pNetFile;
}

ISAM_VTDef *ISAMDBAddVTDef(ISAM_Session *pSession, DWORD type)
{
	ISAM_VTDef *pVTDef;

	if (pVTDef = MemAllocate(sizeof(ISAM_VTDef)))
	{
		pVTDef->next = pSession->pVTDef;
		pSession->pVTDef = pVTDef;
		pVTDef->type = type;
		pVTDef->vtdid = pSession->pTask->dwObjects++;
		pVTDef->pSession = pSession;
	}
	return pVTDef;
}

ISAM_Session *ISAMDBAddSession(ISAM_Task *pTask, JET_SESID sesid)
{
	ISAM_Session *pSession;
	
	if (pSession = MemAllocate(sizeof(ISAM_Session)))
	{
		pSession->pTask = pTask;
		pSession->next = pTask->pSession;
		pTask->pSession = pSession;
		pSession->nSession = m_dwSessions++;
		pSession->sesid = sesid;
	}
	return pSession;
}

ISAM_Column *ISAMDBLocateColumn(ISAM_DBTable *pTable, LPCWSTR lpColumn)
{
	ISAM_Column *pColumn;

	for (pColumn = pTable->pColumns; pColumn; pColumn = pColumn->next)
	{
		if (!DBlstrcmpi(pColumn->szColumnName, lpColumn))
			return pColumn;
	}
	return NULL;
}

ISAM_Column *ISAMDBLocateColumnId(ISAM_DBTable *pTable, JET_HANDLE Id)
{
	ISAM_Column *pColumn;

	for (pColumn = pTable->pColumns; pColumn; pColumn = pColumn->next)
	{
		if (pColumn->columnid == Id)
			return pColumn;
	}
	return NULL;
}

ISAM_DBTable *ISAMDBLocateTable(ISAM_DB *pDB, LPCWSTR lpTable)
{
	ISAM_DBTable *pTable;

	for (pTable = pDB->pTables; pTable; pTable = pTable->next)
	{
		if (!DBlstrcmpi(pTable->szTableName, lpTable))
			return pTable;
	}
	return NULL;
}

ISAM_VTDef *ISAMDBLocateVTDef(ISAM_Session *pSession, JET_HANDLE vtdid)
{
	ISAM_VTDef *pVTDef;

	for (pVTDef = pSession->pVTDef; pVTDef; pVTDef=pVTDef->next)
	{
		if (pVTDef->vtdid == vtdid)
			return pVTDef;
	}
	return NULL;
}

ISAM_VTDef *ISAMDBLocateVTDefExtTID(ISAM_Session *pSession, JET_TABLEID externalTID)
{
	ISAM_VTDef *pVTDef;

	for (pVTDef = pSession->pVTDef; pVTDef; pVTDef=pVTDef->next)
	{
		if (pVTDef->externalTID == externalTID)
			return pVTDef;
	}
	return NULL;
}

// ----------------------------------------------------------------------

JET_SESID ErrorSetSession(JET_SESID sesid)
{
	m_CurrentSession = sesid;
	return sesid;
}


LPCWSTR ErrorSetExtendedInfoSz1(JET_ERR error, LPCWSTR lpError, JET_GRBIT grbit)
{
	LPCWSTR ret;

	if (!m_CurrentSession) return NULL;
	if (grbit & 1) lpError = ConvertErrString(lpError);
	if (lpError)
	{
		ret = ISAMCurrentTaskCallbacks()->UtilSetErrorInfoReal(m_CurrentSession, lpError, L"", L"", error, 0, 0, 0);
		if (grbit & 1) MemFree((LPWSTR)lpError);
	}
	else
	{
		ret = ISAMCurrentTaskCallbacks()->UtilSetErrorInfoReal(m_CurrentSession, L"", L"", L"", error, 0, 0, 0);
	}
	return ret;
}

LPCWSTR ErrorSetExtendedInfoSz2(JET_ERR error, LPCWSTR lpError, JET_GRBIT grbit)
{
	LPCWSTR ret;

	if (!m_CurrentSession) return FALSE;
	if (grbit & 4) lpError = ConvertErrString(lpError);
	if (lpError)
	{
		ret = ISAMCurrentTaskCallbacks()->UtilSetErrorInfoReal(m_CurrentSession, L"", lpError, L"", error, 0, 0, 0);
		if (grbit & 4) MemFree((LPWSTR)lpError);
	}
	else
	{
		ret = ISAMCurrentTaskCallbacks()->UtilSetErrorInfoReal(m_CurrentSession, L"", L"", L"", error, 0, 0, 0);
	}
	return ret;
}


static LPWSTR ConvertErrString(LPCWSTR lpInput)
{
	LPWSTR lpRet, p;

	if (lpInput)
	{
		if (lpRet = MemAllocate((wcslen(lpInput)+1)*sizeof(WCHAR)))
		{
			wcscpy(lpRet, lpInput);
			for (p=lpRet; *p; p++)
				switch (*p)
			{
				case L'#': *p=L'.'; break;
				case L'(': *p=L'['; break;
				case L')': *p=L']'; break;
			}
			return lpRet;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------

// Set CodePage to GetACP()
int MakeValidJetName(LPCWSTR lpOldName, LPWSTR lpNewName, UINT CodePage)
{
	int ret, len;
	char szBuf[128]={0};
	LPWSTR p;

	ret = 1;
	if ((len = wcslen(lpOldName)) > 64)
	{
		ret = 2;
		len = 64;
	}
	if (CodePage == 1200) wcsncpy(lpNewName, lpOldName, len);
	else
	{
		MultiByteToWideChar (CodePage, 0, szBuf, 
			WideCharToMultiByte(CodePage, 0, lpOldName, len, szBuf, sizeof(szBuf)-1, 0, 0),
			lpNewName, len);
	}
	for (p=lpNewName; *p; p++)
	{
		switch(*p)
		{
		case '.': *p='#'; break;
		case '[': *p='('; break;
		case ']': *p=')'; break;
		default : if (*p<' ' || *p=='!' || *p=='`')
				  {
					  *p='_';
					  ret = 2;
				  }
			break;
		}
	}
	return ret;
}

BOOL IsValidJETName(LPCWSTR lpName, UINT CodePage)
{
	WCHAR szName[65]={0}, *p;

	if (!lpName || !*lpName) return FALSE;
	if (CodePage == 1200)
	{
		if (wcslen(lpName) > 64) return FALSE;
		wcsncpy(szName, lpName, 64);
	}
	else
	{
		char szBuf[132]={0};
		int len = wcslen(lpName), r;

		if (len && !(r = WideCharToMultiByte(CodePage, 0, lpName, len, szBuf, 130, 0, 0)))
			return FALSE;
		MultiByteToWideChar(CodePage, 0, szBuf, r, szName, 64);
	}
	if (*szName && *szName!=' ')
	{
		for (p=szName; *p; p++)
		{
			switch (*p)
			{
			case '!':
			case '`':
			case '[':
			case ']':
				return FALSE;
			default:
				if (*p<' ') return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

int MakeIntoValidFilename(LPCWSTR lpOldName, LPWSTR lpNewName, 
						  int cbFName, int cbExt, BOOL bHasExt, UINT CodePage)
{
	int ret = 0;
	WCHAR szWork[133]={0}, *pHash, *p, c;
	BOOL bExt = FALSE;

	*lpNewName = 0;
	if (IsValidJETName(lpOldName, CodePage))
	{
		ret = 1;
		wcsncpy(szWork, lpOldName, 132);
		pHash=wcsrchr(szWork, '#');
		for (p=szWork; *p; p++)
		{
			if (bExt || *p!='.' && p != pHash)
			{
				c = *p;
				if (c < ' ' || wcschr(L"?\"/\\<>*|:;", c))
				{
					c = '_';
					ret = 2;
				}
				if (cbFName <= 0) ret = 2;
				else 
				{
					*lpNewName = c;
					lpNewName++;
					cbFName--;
				}
			}
			else
			{
				if (!bHasExt) break;
				cbFName = cbExt;
				*lpNewName = '.';
				lpNewName++;
				bExt = TRUE;
			}
		}
		*lpNewName = 0;
	}
	return ret;
}

// ----------------------------------------------------------------------

JET_ERR AssignResult(PVOID pSrc, ULONG cbSrc, PVOID pDest, ULONG cbDest)
{
	memcpy(pDest, pSrc, cbDest>cbSrc?cbSrc:cbDest);
	return cbDest<cbSrc?JET_wrnBufferTruncated:JET_errSuccess;
}

BYTE SetCursorCurrency(ISAM_Cursor *pCursor, BYTE currency)
{
	return pCursor->currency = currency;
}

JET_ERR EstablishAsCurrentCursor(ISAM_Cursor *pCursor)
{
	if (pCursor->pTable->pCurrCursor == pCursor)
		return JET_errSuccess;
	pCursor->pTable->pCurrCursor=pCursor;
	switch (pCursor->currency)
	{
	case CURRENCY_BEGIN:
	case CURRENCY_END:
		return JET_errSuccess;
	case CURRENCY_LOST:
		return JET_wrnCurrencyLost;
	default:
		/*** TODO: Implement: Move to pCursor->rowid if needed */
		return JET_errSuccess;
	}
}

// ----------------------------------------------------------------------

static int _afxMonthDays[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};   

void DtfParsedToSerial(DATETIME *dt, double *dtDest)
{   
	BOOL bLeapYear;
	int nDaysInMonth = 0;
    int nDate = 0;
    double dblTime = 0;

	// It's directly implemented that way in the text driver, however we may just use
	// SystemTimeToVariantTime() function?

    // Validate year and month (ignore day of week and milliseconds)  
	if (dt->nYear < 100) dt->nYear = 1900+dt->nYear;
    if ((dt->nYear > 9999) || (dt->nMonth > 11))
        return;
   
    //  Check for leap year and set the number of days in the month   
    bLeapYear = ((dt->nYear & 3) == 0) && ((dt->nYear % 100) != 0 || (dt->nYear % 400) == 0);   
   
    if(bLeapYear && dt->nDay == 29 && dt->nMonth == 1)
    {   
        nDaysInMonth = _afxMonthDays[dt->nMonth+1] - _afxMonthDays[dt->nMonth] + 1;   
    }   
    else   
    {   
        nDaysInMonth = _afxMonthDays[dt->nMonth+1] - _afxMonthDays[dt->nMonth];
    }   
   
    // Finish validating the date   
    if ((dt->nDay < 1) || (dt->nDay > nDaysInMonth) ||   
        (dt->nHour > 23) || (dt->nMinute > 59) ||   
        (dt->nSecond > 59))   
    {   
        return;
    }   
     
    //It is a valid date; make Jan 1, 1AD be 1   
    nDate = dt->nYear*365 + dt->nYear/4 - dt->nYear/100 + dt->nYear/400 +   
        _afxMonthDays[dt->nMonth] + dt->nDay;   
   
    //  If leap year and it's before March, subtract 1:   
    if ((dt->nMonth <= 1) && bLeapYear) --nDate;   
   
    //  Offset so that 12/30/1899 is 0   
    nDate -= 693959;
   
    dblTime = (((int)dt->nHour * 3600) +  // hrs in seconds   
        ((int)dt->nMinute * 60) +  // mins in seconds   
        ((int)dt->nSecond)) / 86400.;   
   
    if(nDate >= 0)   
    {   
        *dtDest = (double)nDate + dblTime;   
    }   
    else   
    {   
        *dtDest = (double)nDate - dblTime;   
    }
}   


/*
 * Custom function to sort lists, not part of JET Text driver,
 * slightly modified, original taken from:
 * http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html 
 */
typedef struct element element;
struct element {
    element *next;
};
PVOID SortList(PVOID arg_list, int (__cdecl *cmp)(const void *elem1, const void *elem2)) 
{
	element *list = (element *)arg_list;
    element *p, *q, *e, *tail;
    int insize = 1, nmerges, psize, qsize, i;

    while (1) {
        p = list;
        list = NULL;
        tail = NULL;

        nmerges = 0;  /* count number of merges we do in this pass */

        while (p) {
            nmerges++;  /* there exists a merge to be done */
            /* step `insize' places along from p */
            q = p;
            psize = 0;
            for (i = 0; i < insize; i++) {
                psize++;
		    q = q->next;
                if (!q) break;
            }

            /* if q hasn't fallen off end, we have two lists to merge */
            qsize = insize;

            /* now we have two lists; merge them */
            while (psize > 0 || (qsize > 0 && q)) {

                /* decide whether next element of merge comes from p or q */
                if (psize == 0) {
		    /* p is empty; e must come from q. */
		    e = q; q = q->next; qsize--;
		} else if (qsize == 0 || !q) {
		    /* q is empty; e must come from p. */
		    e = p; p = p->next; psize--;
		} else if (cmp(p,q) <= 0) {
		    /* First element of p is lower (or same);
		     * e must come from p. */
		    e = p; p = p->next; psize--;
		} else {
		    /* First element of q is lower; e must come from q. */
		    e = q; q = q->next; qsize--;
		}

                /* add the next element to the merged list */
		if (tail) {
		    tail->next = e;
		} else {
		    list = e;
		}
		tail = e;
            }

            /* now p has stepped `insize' places along, and q has too */
            p = q;
        }
	    tail->next = NULL;

        /* If we have done only one merge, we're finished. */
        if (nmerges <= 1)   /* allow for nmerges==0, the empty list case */
            return list;

        /* Otherwise repeat, merging lists twice the size */
        insize *= 2;
    }
}

// ----------------------------------------------------------------------
// Curstom functions for tracing

ISAM_Trace *ISAMDBFindTrace(JET_SESID sesid, JET_HANDLE traceid)
{
	ISAM_Session *pSession;
	ISAM_Trace *pTrace;

	if (pSession = ISAMDBFindSession(sesid))
	{
		for (pTrace = pSession->pTrace; pTrace; pTrace=pTrace->next)
		{
			if (pTrace->traceid == traceid)
				return pTrace;
		}
	}
	return NULL;
}

ISAM_Trace *ISAMDBFindTraceByTID(BYTE type, JET_HANDLE tableid)
{
	ISAM_Task *pTask;
	ISAM_Trace *pTrace;

	for (pTask = ISAMDB_Task_list; pTask; pTask = pTask->next)
	{
		if (pTask->pSession && pTask->pSession->pTrace)
		{
			for (pTrace = pTask->pSession->pTrace; pTrace; pTrace=pTrace->next)
			{
				if (pTrace->type == type && pTrace->tableid == tableid)
					return pTrace;
			}
		}
	}
	return NULL;
}

JET_TABLEID ISAMDBFindTraceTableidFromVtid(JET_TABLEID vtid, JET_FNDEFTBL *pfndef)
{
	ISAM_Task *pTask;
	ISAM_Trace *pTrace;

	for (pTask = ISAMDB_Task_list; pTask; pTask = pTask->next)
	{
		if (pTask->pSession && pTask->pSession->pTrace)
		{
			for (pTrace = pTask->pSession->pTrace; pTrace; pTrace=pTrace->next)
			{
				if (pTrace->handle == vtid && pTrace->pTblFN == pfndef)
					return pTrace->tableid;
			}
		}
	}
	return 0;
}

ISAM_DBUser *ISAMDBFindDatabaseUserId(JET_HANDLE userid)
{
	ISAM_Task *pTask;
	ISAM_DB *pDB;
	ISAM_DBUser *pUser;

	for (pTask = ISAMDB_Task_list; pTask; pTask = pTask->next)
	{
		for (pDB=pTask->pDatabase; pDB; pDB = pDB->next)
		{
			for (pUser=pDB->pUsers; pUser; pUser=pUser->next)
			{
				if (pUser->userid == userid)
					return pUser;
			}
		}
	}
	return NULL;
}



ISAM_Trace *ISAMDBAddTrace(ISAM_Session *pSession, BYTE type, JET_HANDLE handle, PVOID pvFnDef)
{
	ISAM_Trace *pTrace;

	if (pTrace = MemAllocate(sizeof(ISAM_Trace)))
	{
		pTrace->next = pSession->pTrace;
		pSession->pTrace = pTrace;
		pTrace->handle = handle;
		pTrace->pTblFN = pvFnDef;
		pTrace->type   = type;
		pTrace->traceid = pSession->pTask->dwObjects++;
		pTrace->pSession = pSession;
	}
	return pTrace;
}

void ISAMDBDeleteTrace(ISAM_Session *pSession, ISAM_Trace *pTrace)
{
	ISAM_Trace *pTraceIter, *pTracePrev = NULL;

	for (pTraceIter = pSession->pTrace; pTraceIter != pTrace; pTraceIter = pTraceIter->next)
	{
		pTracePrev = pTraceIter;
		if (!pTraceIter) return;
	}
	if (pTracePrev) pTracePrev->next = pTrace->next;
	else pSession->pTrace = pTrace->next;

	MemFree(pTrace);
	return;
}


/* Custom function stolen from OLEVAR.CPP of MFC
 * to convert JET_DATESERIAL to struct tm
 */
#define MAX_TIME_BUFFER_SIZE    128         // matches that in timecore.cpp   
#define MIN_DATE                (-657434)  // about year 100   
#define MAX_DATE                2958465    // about year 9999   
#define HALF_SECOND  (1.0/172800.0)   
#define HALF_MILLINSEC  (1.0/172800.0/1000.0)   
BOOL TmFromOleDate(double dtSrc, struct tm *tmDest)
{
	long nDays;             // Number of days since Dec. 30, 1899
	long nDaysAbsolute;     // Number of days since 1/1/0
	long nSecsInDay;        // Time in seconds since midnight
	long nMinutesInDay;     // Minutes in day

	long n400Years;         // Number of 400 year increments since 1/1/0
	long n400Century;       // Century within 400 year block (0,1,2 or 3)
	long n4Years;           // Number of 4 year increments since 1/1/0
	long n4Day;             // Day within 4 year block
							//  (0 is 1/1/yr1, 1460 is 12/31/yr4)
	long n4Yr;              // Year within 4 year block (0,1,2 or 3)
	BOOL bLeap4 = TRUE;     // TRUE if 4 year block includes leap year

	double dblDate = dtSrc; // tempory serial date

	// Maybe this is just VariantTimeToSystemTime() ?

	// The legal range does not actually span year 0 to 9999.
	if (dtSrc > MAX_DATE || dtSrc < MIN_DATE) // about year 100 to about 9999
		return FALSE;

	// If a valid date, then this conversion should not overflow
	nDays = (long)dblDate;

	// Round to the second
	dblDate += ((dtSrc > 0.0) ? HALF_SECOND : -HALF_SECOND);

	nDaysAbsolute = (long)dblDate + 693959L; // Add days from 1/1/0 to 12/30/1899

	dblDate = fabs(dblDate);
	nSecsInDay = (long)((dblDate - floor(dblDate)) * 86400.);

	// Calculate the day of week (sun=1, mon=2...)
	//   -1 because 1/1/0 is Sat.  +1 because we want 1-based
	tmDest->tm_wday = (int)((nDaysAbsolute - 1) % 7L) + 1;

	// Leap years every 4 yrs except centuries not multiples of 400.
	n400Years = (long)(nDaysAbsolute / 146097L);

	// Set nDaysAbsolute to day within 400-year block
	nDaysAbsolute %= 146097L;

	// -1 because first century has extra day
	n400Century = (long)((nDaysAbsolute - 1) / 36524L);

	// Non-leap century
	if (n400Century != 0)
	{
		// Set nDaysAbsolute to day within century
		nDaysAbsolute = (nDaysAbsolute - 1) % 36524L;

		// +1 because 1st 4 year increment has 1460 days
		n4Years = (long)((nDaysAbsolute + 1) / 1461L);

		if (n4Years != 0)
			n4Day = (long)((nDaysAbsolute + 1) % 1461L);
		else
		{
			bLeap4 = FALSE;
			n4Day = (long)nDaysAbsolute;
		}
	}
	else
	{
		// Leap century - not special case!
		n4Years = (long)(nDaysAbsolute / 1461L);
		n4Day = (long)(nDaysAbsolute % 1461L);
	}

	if (bLeap4)
	{
		// -1 because first year has 366 days
		n4Yr = (n4Day - 1) / 365;

		if (n4Yr != 0)
			n4Day = (n4Day - 1) % 365;
	}
	else
	{
		n4Yr = n4Day / 365;
		n4Day %= 365;
	}

	// n4Day is now 0-based day of year. Save 1-based day of year, year number
	tmDest->tm_yday = (int)n4Day + 1;
	tmDest->tm_year = n400Years * 400 + n400Century * 100 + n4Years * 4 + n4Yr;

	// Handle leap year: before, on, and after Feb. 29.
	if (n4Yr == 0 && bLeap4)
	{
		// Leap Year
		if (n4Day == 59)
		{
			/* Feb. 29 */
			tmDest->tm_mon = 2;
			tmDest->tm_mday = 29;
			goto DoTime;
		}

		// Pretend it's not a leap year for month/day comp.
		if (n4Day >= 60)
			--n4Day;
	}

	// Make n4DaY a 1-based day of non-leap year and compute
	//  month/day for everything but Feb. 29.
	++n4Day;

	// Month number always >= n/32, so save some loop time */
	for (tmDest->tm_mon = (n4Day >> 5) + 1;
		n4Day > _afxMonthDays[tmDest->tm_mon]; tmDest->tm_mon++);

	tmDest->tm_mday = (int)(n4Day - _afxMonthDays[tmDest->tm_mon-1]);

DoTime:
	if (nSecsInDay == 0)
		tmDest->tm_hour = tmDest->tm_min = tmDest->tm_sec = 0;
	else
	{
		tmDest->tm_sec = (int)nSecsInDay % 60L;
		nMinutesInDay = nSecsInDay / 60L;
		tmDest->tm_min = (int)nMinutesInDay % 60;
		tmDest->tm_hour = (int)nMinutesInDay / 60;
	}

	return TRUE;
}