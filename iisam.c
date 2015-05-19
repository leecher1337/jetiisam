#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "esent98.h"
#include "winalloc.h"
#include "jetuwrap.h"
#include "spec.h"
#include "isamutil.h"
#include "driver.h"
#include "trace.h"
#include "vtfunct.h"
#include "netutil.h"
#include "windos.h"

/* Extern */
extern JET_FNDEFDB g_drvdbvtbl;

/* Globals */
JET_CALLIN *g_pCaller;
DWORD g_dwReferences = 0;
HINSTANCE g_hInstance;
static IISAM_SETTINGS g_Sett;

static IISAM_CFG_DEF m_stDrvCfg[] = 
{
	{
		L"Dummy 1.0",
		DRIVER_NAME,
		L"",
		L"Dummy files (*.DF)",
		0,
		0,
		TRUE,
		TRUE,
		JET_isamTypeDatabase,
		FALSE,
		0,
		FALSE,
		0,
		0,
		0,
		TRUE
	}
};

/* Installable ISAM Base class */

JET_ERR JET_API IISAMAttachDatabase(JET_SESID sesid, LPCWSTR szFilename, JET_GRBIT grbit)
{
	DebugTrace((L"IISAMAttachDatabase(%d, %s, %d)\n", sesid, szFilename, grbit));
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMBackup(LPCWSTR szBackupPath, JET_GRBIT grbit, JET_PFNSTATUS pfnStatus)
{
	DebugTrace((L"IISAMBackup(%s, %d, %p)\n", szBackupPath, grbit, pfnStatus));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMBeginSession(JET_SESID	*psesid)
{
	ISAM_Task *pTask;

	DebugTrace((L"IISAMBeginSession(%p)\n", psesid));
	if (pTask = ISAMDBFindTask(CurrentTaskHandle()))
	{
		ISAM_Session *pSession;

		if (pSession = ISAMDBAddSession(pTask, *psesid))
		{
			pSession->sesid = *psesid;
			ErrorSetSession(pSession->sesid);
			// *psesid = pSession->sesid;
			return JET_errSuccess;
		}
		return JET_errOutOfMemory;
	}
	return JET_errNotInitialized;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMBeginTransaction( JET_SESID sesid )
{
	DebugTrace((L"IISAMBeginTransaction(%d)\n", sesid));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMCommitTransaction( JET_SESID sesid, JET_GRBIT grbit )
{
	DebugTrace((L"IISAMCommitTransaction(%d, %d)\n", sesid, grbit));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------


JET_ERR JET_API IISAMCreateDatabase(JET_SESID sesid, LPWSTR szFilename, LPWSTR szConnect, JET_DBID* pdbid, JET_GRBIT grbit)
{
	DebugTrace((L"IISAMCreateDatabase(%d, %s, %s, %p, %d)\n", sesid, szFilename, szConnect, pdbid, grbit));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMDetachDatabase(JET_SESID sesid, LPCWSTR szFilename)
{
	DebugTrace((L"IISAMDetachDatabase(%d, %s)\n", sesid, szFilename));
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMEndSession( JET_SESID sesid, JET_GRBIT grbit )
{
	ISAM_Session *pSession;
	ISAM_DB *pDB;
	ISAM_DBUser *pUser;
	ISAM_VTDef *pVTDef;
	JET_ERR ret = JET_errSuccess;

	DebugTrace((L"IISAMDetachDatabase(%d, %d)\n", sesid, grbit));
	if (!(pSession = ISAMDBFindSession(sesid)))
		return JET_errInvalidSesid;
	for (pDB = pSession->pTask->pDatabase; pDB; pDB=pDB->next)
	{
		for (pUser = pDB->pUsers; pUser; pUser=pUser->next)
		{
			if (pUser->pSession == pSession)
				ret = IsamCloseDatabase(sesid, pUser->userid, 0);
		}
	}
	for (pVTDef = pSession->pVTDef; pVTDef; pVTDef = pVTDef->next)
		VTCloseTable(sesid, pVTDef->vtdid);
	ISAMDBDeleteSession(pSession->pTask, pSession);
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMIdle( JET_SESID sesid, JET_GRBIT grbit )
{
	DebugTrace((L"IISAMIdle(%d, %d)\n", sesid, grbit));
	return ISAMDBFindSession(sesid)?JET_wrnNoIdleActivity:JET_errInvalidSesid;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMInit( JET_INSTANCE instance)
{
	HKEY hKey;
	WCHAR szRegPath[261];
	IISAM_CFG *pCfg;
	ISAM_Task *pTask;
	DWORD dwTask;

	DebugTrace((L"IISAMInit(%08X)\n", instance));
	if (!g_dwReferences)
	{
		/* Introduce ourself to the caller ... */
		if (!g_pCaller->ErrGetSystemParameterInst(instance,0,JET_paramRegPath,(JET_API_PTR*)&hKey,szRegPath,sizeof(szRegPath)))		
			*szRegPath = 0;

		/* Load settings from registry */
		InitializeSpec(&g_Sett);
		if (pCfg = ConfigInit(&g_Sett))
		{
			ConfigReadRegSpec(DRIVER_NAME, L"Software\\Microsoft\\Jet\\4.0", hKey, szRegPath, pCfg);
			ConfigRelease(pCfg);
		}

		/* Initialize the driver */
		ISAMDBInitialize();
		DriverInitialize(&g_Sett);
	}

	/* Now add us to the task list */
	if (!(pTask = ISAMDBFindTask(dwTask = CurrentTaskHandle())))
	{
		if (!(pTask = ISAMDBAddTask(dwTask)))
			return JET_errOutOfMemory;
		pTask->pCaller = g_pCaller;
		if (g_Sett.bTrace)
		{
			pTask->pCaller = TraceInit(g_Sett.szTraceFile);
		}
#ifdef _DEBUG
		else
		{
			pTask->pCaller = TraceInit(NULL);
		}
#endif
		pTask->JetInst = instance;
	}

	g_dwReferences++;
	pTask->dwRefCount++;
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMLoggingOn(JET_SESID sesid)
{
	DebugTrace((L"IISAMLoggingOn(%d)\n", sesid));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMLoggingOff(JET_SESID sesid)
{
	DebugTrace((L"IISAMLoggingOff(%d)\n", sesid));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMOpenDatabase(JET_SESID sesid, LPCWSTR szFilename, LPCWSTR szConnect,
								  JET_DBID *pdbid, JET_GRBIT grbit)
{
	WCHAR szLocalFile[MAX_PATH+1]={0}, szFullPath[MAX_PATH+1]={0};
	enum eProtoType type;
	JET_ERR ret = JET_errSuccess;
	ISAM_Task *pTask, *pTaskITR;
	ISAM_Session *pSession;
	ISAM_DB *pDB;
	ISAM_DBUser *pUser;
	int len;

	DebugTrace((L"IISAMOpenDatabase(%d, %s, %s, %p, %d)\n", sesid, szFilename, szConnect, pdbid, grbit));
	if (!szFilename || wcslen(szFilename)>sizeof(szLocalFile)/sizeof(WCHAR)) 
		return JET_errInvalidParameter;

	/* In our use case here, a database is always a directory */
	if (type = NetProtocolType(szFilename))
	{		
		int len;

		if (DOSLocatesDirectory (szFilename, &ret))
		{
			wcscpy(szFullPath, szFilename);
			len = wcslen(szFullPath)-1;
			if (!type && szFullPath[len]!=L'\\' &&  szFullPath[len]!=L':')
				wcscat (szFullPath, L"\\");
			else if (type && szFullPath[len]!=L'/') wcscat (szFullPath, L"/");
		}
	}
	else
	{
		if (DOSLocatesDirectory (szFilename, &ret))
		{
			Jetwfullpath(szFullPath, szFilename, sizeof(szFullPath)/sizeof(WCHAR));
			len = wcslen(szFullPath)-1;
			if (szFullPath[len]!=L'\\' &&  szFullPath[len]!=L':')
				wcscat (szFullPath, L"\\");
			wcscpy(szLocalFile, szFullPath);
		}
	}
	if (!*szFullPath)
	{
		if (ret != JET_errSuccess)
			return ret;
		ErrorSetExtendedInfoSz1(JET_errvalidPathM, szFilename, 0);
		return JET_errInvalidPath;
	}
	/* Existance of table (DB file) etc. will be checked upon opening a table */

	if (!(pTask = ISAMDBFindTask(CurrentTaskHandle())))
		return JET_errNotInitialized;
	if (!(pSession = ISAMDBFindSession(sesid)))
		return JET_errInvalidSesid;
	if (grbit & 8)
	{
		if (ISAMDBFindDatabase(pTask, szFullPath))
			return JET_errDatabaseInUse;
		grbit |= JET_bitDbExclusive;
	}
	for (pTaskITR = ISAMDBTaskList(); pTaskITR; pTaskITR = pTaskITR->next)
	{
		if (pTaskITR != pTask)
		{
			if (pDB = ISAMDBFindDatabase(pTask, szFullPath))
			{
				if (grbit & JET_bitDbExclusive)
					return JET_errDatabaseInUse;
				for (pUser = pDB->pUsers; pUser && !(pUser->flags & JET_bitDbExclusive); pUser = pUser->next);
				if (pUser) return JET_errDatabaseLocked;
			}
		}
	}
	if (!(pDB = ISAMDBFindDatabase(pTask, szFullPath)))
	{
		if (!(pDB = ISAMDBAddDatabase(pTask, szFullPath)))
			return JET_errOutOfMemory;
		pDB->flags = grbit;
		/*** TODO: Parse szConnect and set various parameters in pDB accordingly */

		if (type) NetCreateLocalDirectory(pDB->szLocalDir);
	}
	if (!(pUser = ISAMDBAddDatabaseUser(pSession, pDB, szConnect)))
	{
		if (!pDB->pUsers && type) NetDestroyLocalDirectory(pDB->szLocalDir);
		return JET_errOutOfMemory;
	}
	/*** TODO: Parse szConnect and set various parameters in pUser accoirdingly */
	pUser->flags = pDB->flags;
	if ((pUser->settings = MemAllocate(sizeof(g_Sett))))
	{
		memcpy (pUser->settings, &g_Sett, sizeof(g_Sett));
		ret = ISAMDBFindTask(CurrentTaskHandle())->pCaller->ErrAllocateDbid(pdbid, pUser->userid, &g_drvdbvtbl);
		if (ret == JET_errSuccess)
			pUser->dbid = *pdbid;
	} else ret = JET_errOutOfMemory;
	if (ret != JET_errSuccess)
	{
		ISAMDBDeleteDatabaseUser(pDB, pUser);
		if (!pDB->pUsers)
		{
			if (type) NetDestroyLocalDirectory(pDB->szLocalDir);
			ISAMDBDeleteDatabase(pTask, pDB);
		}
	}
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMOpenTempTable(JET_SESID sesid, const JET_COLUMNDEF* prgcolumndef, ULONG ccolumn, JET_GRBIT grbit,
	JET_TABLEID* ptableid, JET_COLUMNID* prgcolumnid)
{
	DebugTrace((L"IISAMOpenTempTable(%d, %p, %d, %d, %p, %p)\n", sesid, prgcolumndef, ccolumn, grbit, ptableid, prgcolumnid));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------


JET_ERR JET_API IISAMRepairDatabase(JET_SESID sesid, LPCWSTR lpDatabase, JET_PFNSTATUS pfnStatus)
{
	DebugTrace((L"IISAMRepairDatabase(%d, %s, %p)\n", sesid, lpDatabase, pfnStatus));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMRestore(LPCWSTR szDatabase, JET_PFNSTATUS pfnStatus)
{
	DebugTrace((L"IISAMRestore(%s, %p)\n", szDatabase, pfnStatus));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMRollback(JET_SESID sesid, JET_GRBIT grbit)
{
	DebugTrace((L"IISAMRollback(%d, %d)\n", sesid, grbit));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMSetSystemParameter(JET_INSTANCE instance, ULONG paramid, JET_API_PTR lParam, LPCWSTR szParam)
{
	DebugTrace((L"IISAMSetSystemParameter(%p, %d, %d, %s)\n", instance, paramid, lParam, szParam));
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IISAMTerm()
{
	ISAM_Task *pTask;
	JET_ERR ret = JET_errSuccess;

	DebugTrace((L"IISAMTerm()\n"));
	if (!(pTask = ISAMDBFindTask(CurrentTaskHandle())))
		return JET_errNotInitialized;
	g_dwReferences--;
	if (--pTask->dwRefCount == 0)
	{
		ISAM_Session *pSession;

		for (pSession = pTask->pSession; pSession; pSession = pSession->next)
			ret = IISAMEndSession(pSession->nSession, 0);
		ISAMDBDeleteTask(pTask);
		if (!g_dwReferences)
		{
			NetTerminateInternetServices();
			DriverTerminate();
			TraceExit();
			MemFreeAllPages();
		}
	}
	return ret;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

IISAM_Vtbl m_pAPI =
{
	8,
	IISAMAttachDatabase,
	IISAMBackup,
	IISAMBeginSession,
	IISAMBeginTransaction,
	IISAMCommitTransaction,
	IISAMCreateDatabase,
	IISAMDetachDatabase,
	IISAMEndSession,
	IISAMIdle,
	IISAMInit,
	IISAMLoggingOn,
	IISAMLoggingOff,
	IISAMOpenDatabase,
	IISAMOpenTempTable,
	IISAMRepairDatabase,
	IISAMRestore,
	IISAMRollback,
	IISAMSetSystemParameter,
	IISAMTerm
};

DWORD WINAPI ErrIsamLoad(PVOID pCaller, DWORD *pVtbl)
{
	g_pCaller = pCaller;
	*pVtbl = (DWORD)&m_pAPI;
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		IsUnicodeOS();
		g_hInstance = hinstDLL;
		break;
	}
	return TRUE;
}

STDAPI DllRegisterServer(void)
{
	char szModule[MAX_PATH];
	WCHAR szwModule[MAX_PATH];
	IISAM_CFG *pCfg;
	HRESULT hRes = E_FAIL;

	/* Compatibility with Win9x, there is no *W API */
	if (GetModuleFileNameA(g_hInstance, szModule, sizeof(szModule)) && (pCfg = ConfigInit(&g_Sett)))
	{
		if (MultiByteToWideChar(CP_ACP, 0, szModule, -1, szwModule, sizeof(szwModule)/sizeof(WCHAR)))
		{
			if (ConfigRegister(L"Software\\Microsoft\\Jet\\4.0", m_stDrvCfg, sizeof(m_stDrvCfg)/sizeof(m_stDrvCfg[0]), 
				pCfg, szwModule) == 0)
				hRes = S_OK;
		}
		ConfigRelease(pCfg);
	}
	return hRes;
}

STDAPI DllUnregisterServer(void)
{

	IISAM_CFG *pCfg;
	HRESULT hRes = E_FAIL;

	if (pCfg = ConfigInit(&g_Sett))
	{
		if (ConfigUnregister(L"Software\\Microsoft\\Jet\\4.0", m_stDrvCfg, sizeof(m_stDrvCfg)/sizeof(m_stDrvCfg[0]), pCfg) == 0)
			hRes = S_OK;
		ConfigRelease(pCfg);
	}
	return hRes;
}
