/* Virtual table functions */
#include "iisam.h"
#include "driver.h"
#include "isamutil.h"
#include <stdio.h>

/******************************************************
 *************** TABLE LEVEL FUNCTIONS ****************
 ******************************************************/
JET_ERR JET_API VTAddColumn(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szColumnName,
	const JET_COLUMNDEF_OLD	*pcolumndef, const void *pvDefault, ULONG cbDefault,
	JET_COLUMNID *pcolumnid )
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTCloseTable(JET_SESID sesid, JET_TABLEID tableid)
{
	ISAM_Session *pSession;
	ISAM_VTDef *pVTDef;

	if (!(pSession = ISAMDBFindSession(sesid)))
		return JET_errInvalidSesid;
	if (!(pVTDef = ISAMDBLocateVTDef(pSession, tableid)))
		return JET_errInvalidTableId;
	ISAMDBFindTask(CurrentTaskHandle())->pCaller->ReleaseTableid(pVTDef->externalTID);
	ISAMDBDeleteVTDef(pSession, pVTDef);
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTComputeStats(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTCopyBookmarks(JET_SESID sesid, JET_TABLEID tableid, DWORD unk3, 
	DWORD unk4, DWORD unk5, DWORD *unk6, DWORD *unk7)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTCreateIndex(JET_SESID sesid,	JET_TABLEID	tableid, LPCWSTR szIndexName,
	JET_GRBIT grbit, LPCWSTR szKey, ULONG cbKey, ULONG lDensity)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTCreateReference(JET_SESID sesid, JET_DBID dbid,
	LPCWSTR szTableName, LPCWSTR szColumnName, const void *pvData, const ULONG cbData,
	const JET_GRBIT	grbit)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTDelete(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTDeleteColumn(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szColumnName)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTDeleteIndex(JET_SESID sesid, JET_TABLEID	tableid,  LPCWSTR szIndexName)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTDeleteReference(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szReferenceName)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTDupCursor(JET_SESID sesid, JET_TABLEID tableid, JET_TABLEID *ptableid,
	JET_GRBIT grbit)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGetBookmark(JET_SESID sesid, JET_TABLEID	tableid, PVOID pvBookmark,
	ULONG cbMax, PULONG pcbActual)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGetChecksum(JET_SESID sesid, JET_TABLEID tableid, PDWORD pdwChecksum)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid, LPWSTR szIndexName,
	ULONG cchIndexName )
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGetCursorInfo(JET_SESID sesid, JET_TABLEID tableid, PVOID pvResult,
	ULONG cbMax, ULONG InfoLevel)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGetRecordPosition(JET_SESID sesid, JET_TABLEID tableid, 
	JET_RECPOS *precpos, ULONG cbRecpos)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGetTableColumnInfo(JET_SESID	sesid, JET_TABLEID tableid,
	LPCWSTR szColumnName, PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGetTableIndexInfo(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName, PVOID pvResult, ULONG cbResult, ULONG InfoLevel)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------
JET_ERR JET_API VTGetTableInfo(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Session *pSession;
	ISAM_VTDef *pVTDef;
	ULONG uResult[2] = {0};
	WCHAR szInfoName[16];
	JET_OBJECTINFO objInfo={0};

	switch (InfoLevel)
	{
	case JET_TblInfo:
	case JET_TblInfoName:
	case JET_TblInfoDbid:
		if (!(pSession = ISAMDBFindSession(sesid)))
			return JET_errInvalidSesid;
		if (!(pVTDef = ISAMDBLocateVTDef(pSession, tableid)))
			return JET_errInvalidTableId;

		switch (InfoLevel)
		{
		case JET_TblInfo:
			objInfo.cbStruct = sizeof(objInfo);
			objInfo.objtyp = JET_objtypTable;
			objInfo.cRecord = pVTDef->cRecord;
			return AssignResult (&objInfo, sizeof(objInfo), pvResult, cbMax);
		case JET_TblInfoName:
			return AssignResult (szInfoName, 
				swprintf (szInfoName, L"T%d", tableid)+1, pvResult, cbMax);
		case JET_TblInfoDbid:
			uResult[0] = pVTDef->dbid;
			uResult[1] = pVTDef->userid;
			return AssignResult (&uResult[0], sizeof(uResult), pvResult, cbMax);
		}
		break;
	}
	return JET_errInvalidParameter;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGetTableReferenceInfo(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName, PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGotoBookmark(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTGotoPosition(JET_SESID sesid, JET_TABLEID tableid, JET_RECPOS *precpos)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTIdle(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTMakeKey(JET_SESID sesid, JET_TABLEID tableid, const void *pvData,
	ULONG cbData, JET_GRBIT	grbit)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

#define VTSEEK_NEXT		0
#define VTSEEK_PREV		1
#define VTSEEK_BEGIN	2
#define VTSEEK_END		3
#define VTSEEK_CURRENT	4
#define VTSEEK_NEG		5

static JET_ERR VTDoSeek(ISAM_VTDef *pVTDef, ISAM_DATAPTR *pData, char vtseek, long cRow)
{
	PVOID pNext;
	long i;

	switch (vtseek)
	{
	case VTSEEK_NEXT: 
		if (pNext = (PVOID)*((DWORD*)pData->pCurData))
		{
			pData->pCurData = pNext;
			pVTDef->currency = CURRENCY_CURRENT;
		}
		else
		{
			pVTDef->currency = CURRENCY_END;
		}
		return JET_errSuccess;
	case VTSEEK_BEGIN:
		pData->pCurData = pData->pData;
		pVTDef->currency = CURRENCY_CURRENT;
		return JET_errSuccess;
	case VTSEEK_END:
		for (pNext = pData->pData; pNext; pNext = (PVOID)*((DWORD*)pNext))
			if (!*((DWORD*)pNext)) break;
		if (pNext)
		{
			pData->pCurData = pNext;
			pVTDef->currency = CURRENCY_CURRENT;
		}
		return JET_errSuccess;
	case VTSEEK_PREV:
		if (pData->pCurData == pData->pData)
		{
			pVTDef->currency = CURRENCY_BEGIN;
			return JET_errSuccess;
		}
		for (pNext = pData->pData; pNext; pNext = (PVOID)*((DWORD*)pNext))
			if (*((DWORD*)pNext) == (DWORD)pData->pCurData) break;
		if (pNext)
		{
			pData->pCurData = pNext;
			pVTDef->currency = CURRENCY_CURRENT;
		}
		return JET_errSuccess;
	case VTSEEK_CURRENT:
		if (cRow <=0) return JET_errSuccess;
		for (i=0; i<cRow; i++)
		{
			if (!(pNext = (PVOID)*((DWORD*)pData->pCurData))) break;
			pData->pCurData = pNext;
			pVTDef->currency = CURRENCY_CURRENT;
		}
		if (!pNext) pVTDef->currency = CURRENCY_END;
		return JET_errSuccess;
	case VTSEEK_NEG:
		if (cRow >= 0) return JET_errSuccess;
		cRow *= -1;
		for (i=0; i<cRow; i++)
		{
			if (pData->pCurData == pData->pData) 
			{
				pVTDef->currency = CURRENCY_BEGIN;
				return JET_errSuccess;
			}
			if (pData->pData) 
			{
				for (pNext = pData->pData; pNext; pNext = (PVOID)*((DWORD*)pNext))
				{
					if ((PVOID)*((DWORD*)pNext) == pData->pCurData)
					{
						pData->pCurData = pNext;
						pVTDef->currency = CURRENCY_CURRENT;
						break;
					}
				}
			}
		}
		return JET_errSuccess;
	}
	return JET_errSuccess;
}

JET_ERR JET_API VTMove(JET_SESID sesid, JET_TABLEID tableid, long cRow,
	JET_GRBIT grbit)
{
	ISAM_Session *pSession;
	ISAM_VTDef *pVTDef;
	char vtseek;

	if (!(pSession = ISAMDBFindSession(sesid)))
		return JET_errInvalidSesid;
	if (!(pVTDef = ISAMDBLocateVTDef(pSession, tableid)))
		return JET_errInvalidTableId;
	if (grbit)
		return JET_errIllegalOperation;
	if (!pVTDef->cRecord)
		return JET_errNoCurrentRecord;
	if (cRow)
	{
		switch (cRow)
		{
		case JET_MoveFirst:
			vtseek = VTSEEK_BEGIN;
			break;
		case JET_MoveLast:
			vtseek = VTSEEK_END;
			break;
		case JET_MoveNext:
			vtseek = VTSEEK_NEXT;
			break;
		case JET_MovePrevious:
			vtseek = VTSEEK_PREV;
			break;
		default:
			vtseek = cRow<=0?VTSEEK_NEG:VTSEEK_CURRENT;
			break;
		}
		if (pVTDef->currency || vtseek)
		{
			if (pVTDef->currency == CURRENCY_END && vtseek == VTSEEK_PREV)
				vtseek = VTSEEK_END;
		} else vtseek = VTSEEK_BEGIN;

		VTDoSeek(pVTDef, &pVTDef->data, vtseek, cRow);
	}
	return pVTDef->currency != CURRENCY_CURRENT ? JET_errNoCurrentRecord : JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTNotifyUpdateUfn(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTNotifyRollback(JET_SESID sesid, JET_TABLEID tableid, int unk3)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTNotifyUnknown(JET_SESID sesid, JET_TABLEID tableid, int unk3)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTNotifyUnknown2(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTPrepareUpdate(JET_SESID sesid, JET_TABLEID tableid, ULONG prep)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTRenameColumn(JET_SESID  sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTRenameIndex(JET_SESID  sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTRenameReference(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

#define SETFIELD(x) { pRet = &x; cbRet = sizeof(x); }

static JET_ERR VTRetrieveObjinfoColumn(ISAM_VTDef *pVTDef, JET_COLUMNID columnid, 
									   PVOID pvData, ULONG cbData, ULONG *pcbActual)
{
	ISAM_OBJListData *pData = (ISAM_OBJListData *)pVTDef->data.pCurData;
	PVOID pRet;
	ULONG cbRet;

	if (pcbActual)  *pcbActual = 0;
	else ZeroMemory(pvData, cbData);
	switch (columnid)
	{
	case 0:
		pRet = L"Tables";
		cbRet = sizeof(L"Tables")-sizeof(WCHAR);
		break;
	case 1:
		pRet = pData->szObjectName;
		cbRet = wcslen(pData->szObjectName) * sizeof(WCHAR) + sizeof(WCHAR);
		break;
	case 2:
		SETFIELD(pData->objInfo.objtyp);
		break;
	case 3:
		SETFIELD(pData->objInfo.dtCreate);
		break;
	case 4:
		SETFIELD(pData->objInfo.dtUpdate);
		break;
	case 5:
		SETFIELD(pData->objInfo.cRecord);
		break;
	case 6:
		SETFIELD(pData->objInfo.cPage);
		break;
	case 7:
		SETFIELD(pData->objInfo.grbit);
		break;
	case 8:
		SETFIELD(pData->objInfo.flags);
		break;
	default:
		return JET_errBadColumnId;
	}
	if (pcbActual)
		*pcbActual = cbRet>=cbData?cbData:cbRet;
	return AssignResult(pRet, cbRet, pvData, cbData);
}

static JET_ERR VTRetrieveIndexinfoColumn(ISAM_VTDef *pVTDef, JET_COLUMNID columnid, 
									   PVOID pvData, ULONG cbData, ULONG *pcbActual)
{
	ISAM_IDXListData *pData = (ISAM_IDXListData*)pVTDef->data.pCurData;
	PVOID pRet;
	ULONG cbRet;

	if (pcbActual)  *pcbActual = 0;
	else ZeroMemory(pvData, cbData);
	switch (columnid)
	{
	case 0:
		pRet = pData->szIdxName;
		cbRet = wcslen(pData->szIdxName) * sizeof(WCHAR) + sizeof(WCHAR);
		break;
	case 1:
		SETFIELD(pData->grbit);
		break;
	case 2:
		SETFIELD(pData->cKey);
		break;
	case 3:
		SETFIELD(pData->cEntry);
		break;
	case 4:
		SETFIELD(pData->cPage);
		break;
	case 5:
		SETFIELD(pData->cColumn);
		break;
	case 6:
		SETFIELD(pData->iColumn);
		break;
	case 7:
		SETFIELD(pData->columnid);
		break;
	case 8:
		SETFIELD(pData->coltyp);
		break;
	case 9:
		SETFIELD(pData->Langid);
		break;
	case 10:
		SETFIELD(pData->grbitCol);
		break;
	case 11:
		pRet = pData->szColName;
		cbRet = wcslen(pData->szColName) * sizeof(WCHAR) + sizeof(WCHAR);
		break;
	default:
		return JET_errBadColumnId;
	}
	if (pcbActual)
		*pcbActual = cbRet>=cbData?cbData:cbRet;
	return AssignResult(pRet, cbRet, pvData, cbData);
}

static JET_ERR VTRetrieveColinfoColumn(ISAM_VTDef *pVTDef, JET_COLUMNID columnid, 
									   PVOID pvData, ULONG cbData, ULONG *pcbActual)
{
	ISAM_COLListData *pData = (ISAM_COLListData*)pVTDef->data.pCurData;
	PVOID pRet;
	ULONG cbRet;
	unsigned short cp;
	
	if (pcbActual)  *pcbActual = 0;
	else ZeroMemory(pvData, cbData);
	switch (columnid)
	{
	case 0:
	case 2:
		SETFIELD(pData->colDef.columnid);
		break;
	case 1:
	case 9:
		pRet = pData->colDef.szBaseColumnName;
		cbRet = wcslen(pData->colDef.szBaseColumnName) * sizeof(WCHAR) + sizeof(WCHAR);
		break;
	case 3:
		SETFIELD(pData->colDef.coltyp);
		break;
	case 4:
		SETFIELD(pData->colDef.langid);
		break;
	case 5:
		SETFIELD(pData->colDef.cbMax);
		break;
	case 6:
		SETFIELD(pData->colDef.grbit);
		break;
	case 7:
		cp = 1004;
		SETFIELD(cp);
		break;
	case 8:
		pRet = pData->colDef.szBaseTableName;
		cbRet = wcslen(pData->colDef.szBaseTableName) * sizeof(WCHAR) + sizeof(WCHAR);
		break;
	default:
		return JET_errBadColumnId;
	}
	if (pcbActual)
		*pcbActual = cbRet>=cbData?cbData:cbRet;
	return AssignResult(pRet, cbRet, pvData, cbData);
}


JET_ERR JET_API VTRetrieveColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, PVOID pvData, ULONG cbData, ULONG *pcbActual,
	JET_GRBIT grbit, JET_RETINFO *pretinfo)
{
	ISAM_Session *pSession;
	ISAM_VTDef *pVTDef;

	if (!(pSession = ISAMDBFindSession(sesid)))
		return JET_errInvalidSesid;
	if (!(pVTDef = ISAMDBLocateVTDef(pSession, tableid)))
		return JET_errInvalidTableId;
	if (grbit)
		return JET_errIllegalOperation;
	if (!pVTDef->cRecord)
		return JET_errNoCurrentRecord;
	switch (pVTDef->type)
	{
	case VTDEF_TYPE_INDEXINFO:
		return VTRetrieveIndexinfoColumn(pVTDef, columnid, pvData, cbData, pcbActual);
	case VTDEF_TYPE_OBJINFO:
		return VTRetrieveObjinfoColumn(pVTDef, columnid, pvData, cbData, pcbActual);
	case VTDEF_TYPE_COLUMNINFO:
		return VTRetrieveColinfoColumn(pVTDef, columnid, pvData, cbData, pcbActual);
	default: 
		return JET_errNoCurrentRecord;
	}
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTRetrieveKey(JET_SESID sesid, JET_TABLEID tableid, 
	PVOID pvData, ULONG cbMax, ULONG *pcbActual, JET_GRBIT grbit)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTSeek(JET_SESID sesid, JET_TABLEID tableid, JET_GRBIT grbit)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTSetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTSetColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, const void *pvData, ULONG cbData,
	JET_GRBIT grbit, JET_SETINFO *psetinfo)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTSetIndexRange(JET_SESID sesid, JET_TABLEID tableidSrc,
	JET_GRBIT grbit)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTUpdate(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark,	ULONG *pcbActual)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTEmptyTable(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark,	ULONG *pcbActual)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTCollectRecids(JET_SESID sesid, JET_TABLEID tableid,
	RECID_CALLBACK callback, ULONG cbParam1, ULONG cbParam2, LPCWSTR unk6,
	int cbunk6, int unk8, int unk9, int unk10, int unk11)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTOpenILockBytes(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, PVOID pvLockBytes, JET_GRBIT grbit)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTModifyColumn(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szColumnName, int unk4, int unk5)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API VTReadAhead(JET_SESID sesid, JET_TABLEID tableid,
	PVOID unk3, int unk4, int unk5)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

JET_FNDEFTBL g_drvvttblvtbl = 
{
	sizeof(JET_FNDEFTBL),
	L"X\0"DRIVER_DESC L" VT ISAM",	
	VTAddColumn,
	VTCloseTable,
	VTComputeStats,
	VTCopyBookmarks,
	VTCreateIndex,
	VTCreateReference,
	VTDelete,
	VTDeleteColumn,
	VTDeleteIndex,
	VTDeleteReference,
	VTDupCursor,
	VTGetBookmark,
	VTGetChecksum,
	VTGetCurrentIndex,
	VTGetCursorInfo,
	VTGetRecordPosition,
	VTGetTableColumnInfo,
	VTGetTableIndexInfo,
	VTGetTableInfo,
	VTGetTableReferenceInfo,
	VTGotoBookmark,
	VTGotoPosition,
	VTIdle,
	VTMakeKey,
	VTMove,
	VTNotifyUpdateUfn,
	VTNotifyRollback,
	VTNotifyUnknown,
	VTNotifyUnknown2,
	VTPrepareUpdate,
	VTRenameColumn,
	VTRenameIndex,
	VTRenameReference,
	VTRetrieveColumn,
	VTRetrieveKey,
	VTSeek,
	VTSetCurrentIndex,
	VTSetColumn,
	VTSetIndexRange,
	VTUpdate,
	VTEmptyTable,
	VTCollectRecids,
	VTOpenILockBytes,
	NULL,
	NULL,
	NULL,
	NULL,
	VTModifyColumn,
	VTReadAhead
};

