#include "iisam.h"
#include <stdio.h>
#include "driver.h"
#include "isamutil.h"
#include "netutil.h"
#include "windos.h"
#include "jetuwrap.h"

static WCHAR *m_pwszExt[] = DRIVER_FILE_EXT;

extern JET_FNDEFTBL g_drvvttblvtbl;

JET_ERR OpenTable(JET_SESID sesid, ISAM_Session *pSession, ISAM_DBUser *pUser, JET_TABLEID *ptableid,
				  LPCWSTR szTableName, JET_GRBIT grbit, BOOL bIgnoreLocks, BOOL bUnk1);
JET_ERR LocateTableInDatabase(ISAM_DBUser *pUser, LPCWSTR lpObjectName, 
							  ISAM_DBTable **ppTable);
static JET_ERR SetNetError(JET_ERR ret, WCHAR *szTable);

/* Initialize your private data structures */
JET_ERR DriverInitialize(IISAM_SETTINGS *pSettings)
{
	return JET_errSuccess;
}

void DriverTerminate()
{
}

/******************************************************
 ************* REUSABLE TABLE FUNCTIONS ***************
 ******************************************************/
static UINT EngineFindColumnName(ISAM_Cursor *pCursor, LPCWSTR lpColumnName)
{
	/*** TODO :Return 1-based index of column, if found or 0, if not found */
	return 0;
}

// ---------------------------------------------------------------------------

static int EngineGetColumnType(ISAM_Cursor *pCursor, JET_COLUMNID columnid)
{
	/*** TODO: Return JET-Datatype for given column or -1 on error */
	return -1;
}

// ---------------------------------------------------------------------------

static BOOL EngineFillTableColumnInfo(ISAM_Cursor *pCursor, UINT nColumn, JET_COLUMNBASE_OLD *pcoldef)
{
	/*** TODO: Implement filling of pcoldef. nColumn is 1-based index
	 ***       Return TRUE, as long as nColumn identifier is valid, FALSE when all is read.
	 ***       You don't need to fill szBaseTableName
	 */
	if (nColumn > pCursor->pTable->nColumns) return FALSE;
	pcoldef->columnid = nColumn;
	pcoldef->coltyp   = EngineGetColumnType(pCursor, pcoldef->columnid);
	switch (pcoldef->coltyp)
	{
	case JET_coltypLong:
		pcoldef->cbMax = sizeof(ULONG);
		break;
	case JET_coltypText:
		/* Attention: The size is in BYTES, so multiply with WCHAR */
		pcoldef->cbMax = 255 * sizeof(WCHAR);
		break;
	case JET_coltypBit:
		pcoldef->cbMax = sizeof(BYTE);
		break;
	case JET_coltypUnsignedByte:
		pcoldef->cbMax = sizeof(BYTE);
		break;
	case JET_coltypShort:
		pcoldef->cbMax = sizeof(WORD);
		break;
	case JET_coltypIEEESingle:
		pcoldef->cbMax = sizeof(float);
		break;
	case JET_coltypIEEEDouble:
		pcoldef->cbMax = sizeof(double);
		break;
	case JET_coltypCurrency:
		pcoldef->cbMax = sizeof(double);
		break;
	case JET_coltypDateTime:
		pcoldef->cbMax = sizeof(double);
		break;
	case JET_coltypLongText:
		pcoldef->cbMax = DRIVER_MAX_DS;
		break;
	case JET_coltypBinary:
		pcoldef->cbMax = DRIVER_MAX_DS;
		break;
	case JET_coltypLongBinary:
		pcoldef->cbMax = DRIVER_MAX_DS;
		break;
	}
	return TRUE;
}

// ---------------------------------------------------------------------------

static long EngineRecordCount(PVOID pDrvInst)
{
	/** TODO: Implement, return number of records available */
	return 0;
}

// ---------------------------------------------------------------------------

static JET_ERR EngineMove(ISAM_Cursor *pCursor, long cRow, rowid_t *prowid)
{
	/** TODO: Implement, move into the direction requested in cRow,
	 **       set *prowid to new row id */
	switch (cRow)
	{
	case JET_MoveNext:
		break;
	case JET_MoveFirst:
		break;
	case JET_MovePrevious:
		break;
	case JET_MoveLast:
		break;
	default:
		return JET_errInvalidParameter;
	}
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR EngineCloseTable(ISAM_DB *pDB, ISAM_DBTable *pTable)
{
	/*** TODO: Implement me */
	pTable->pDrvInst = NULL;
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR EngineOpenTable(ISAM_DBUser *pUser, ISAM_DBTable *pTable, JET_GRBIT grbit)
{
	DWORD dwAttr;
	WCHAR szFile[MAX_PATH], szLocalTable[MAX_PATH];

	/*** TODO: Implement me */
	pTable->pDrvInst = (PVOID)1;
	MakeIntoValidFilename (pTable->szTableName, szFile, 255, 255, FALSE, GetACP());
	swprintf(szLocalTable, L"%s%s%s", *pUser->pDB->szLocalDir?pUser->pDB->szLocalDir:pUser->pDB->szDBName, szFile, m_pwszExt[0]);
	if ((dwAttr = JetGetFileAttributesW(szLocalTable)) == -1)
		return JET_errObjectNotFound;
	if (dwAttr & FILE_ATTRIBUTE_READONLY)
		pTable->bReadonly = TRUE;
	return JET_errSuccess;
}


/******************************************************
 *************** TABLE LEVEL FUNCTIONS ****************
 ******************************************************/
static JET_ERR CheckUpdateColumns(ISAM_Cursor *pCursor)
{
	if (!(pCursor->flags & JET_bitDbExclusive))
		return JET_errTableNotLocked;
	if (pCursor->bReadonly) 
		return JET_errPermissionDenied;
	return JET_errSuccess;
}

JET_ERR JET_API IsamAddColumn(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szColumnName,
	const JET_COLUMNDEF_OLD	*pcolumndef, const void *pvDefault, ULONG cbDefault,
	JET_COLUMNID *pcolumnid )
{
	ISAM_Cursor *pCursor;
	JET_ERR ret;
	JET_COLTYP coltyp;

	/** TODO: If you do not support it:
	return JET_errFeatureNotAvailable; */
	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (!(ret = CheckUpdateColumns(pCursor)))
		return ret;
	coltyp = pcolumndef->coltyp;
	/* Seems to be some special column type... Dunno what that is... 
	 * 16 also seems to be valid, but I don't know what that is either */
	if (coltyp > 16) return JET_errInvalidParameter;
	if (coltyp == 15) coltyp=JET_coltypText;
	if (pCursor->pTable->nColumns >= 255)
		return JET_errTooManyColumns;
	if (coltyp == JET_coltypText && pcolumndef->cbMax > 255 * sizeof(WCHAR))
		return JET_errColumnTooBig;
	if (!IsValidJETName(szColumnName, GetACP()))
	{
		ErrorSetExtendedInfoSz1(JET_errvalidNameM, szColumnName, 1);
		return JET_errInvalidName;
	}
	if (EngineFindColumnName(pCursor, szColumnName))
		return JET_errColumnDuplicate;
	/*** TODO: Create column 
	if (pcolumnid) *pcolumnid = newcolid;
	*/
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamCloseTable(JET_SESID sesid, JET_TABLEID tableid)
{
	ISAM_Cursor *pCursor;
	ISAM_DBTable *pTable;
	JET_ERR ret = JET_errSuccess;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	pTable = pCursor->pTable;
	ISAMDBFindTask(CurrentTaskHandle())->pCaller->ReleaseTableid(pCursor->externalTID);
	if (pTable->pCurrCursor == pCursor)
		pTable->pCurrCursor = NULL;
	if (!pCursor->next && pTable->pCursors == pCursor)
		ret = EngineCloseTable(pCursor->pUser->pDB, pTable);
	ISAMDBDeleteCursor(pTable, pCursor);
	if (!pTable->pCursors) ISAMDBDeleteTable(pCursor->pUser->pDB, pTable);
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamComputeStats(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamCopyBookmarks(JET_SESID sesid, JET_TABLEID tableid, DWORD unk3, 
	DWORD unk4, DWORD unk5, DWORD *unk6, DWORD *unk7)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamCreateIndex(JET_SESID sesid,	JET_TABLEID	tableid, LPCWSTR szIndexName,
	JET_GRBIT grbit, LPCWSTR szKey, ULONG cbKey, ULONG lDensity)
{
	/*** TODO : Implement */
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamCreateReference(JET_SESID sesid, JET_DBID dbid,
	LPCWSTR szReferenceName, LPCWSTR szColumnName, const void *pvData, const ULONG cbData,
	const JET_GRBIT	grbit)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamDelete(JET_SESID sesid, JET_TABLEID tableid)
{
	/*** TODO : Implement */
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	return -5410;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamDeleteColumn(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szColumnName)
{
	/*** TODO : Implement */
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (!(pCursor->flags & JET_bitDbExclusive))
		return JET_errTableNotLocked;
	if (pCursor->bReadonly) 
		return JET_errPermissionDenied;
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamDeleteIndex(JET_SESID sesid, JET_TABLEID	tableid,  LPCWSTR szIndexName)
{
	/*** TODO : Implement */
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	return JET_errIndexNotFound;
}

// ---------------------------------------------------------------------------
JET_ERR JET_API IsamDeleteReference(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szReferenceName)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamDupCursor(JET_SESID sesid, JET_TABLEID tableid, JET_TABLEID *ptableid,
	JET_GRBIT grbit)
{
	ISAM_Cursor *pCursor;

	if (grbit) return JET_errInvalidParameter;
	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	return OpenTable(sesid, pCursor->pUser->pSession, pCursor->pUser, ptableid, 
		pCursor->pTable->szTableName, pCursor->flags, FALSE, FALSE);
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetBookmark(JET_SESID sesid, JET_TABLEID	tableid, PVOID pvBookmark,
	ULONG cbMax, PULONG pcbActual)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (pCursor->currency != CURRENCY_CURRENT)
		return JET_errNoCurrentRecord;
	if (pcbActual) *pcbActual = sizeof(pCursor->rowid);
	return AssignResult (&pCursor->rowid, sizeof(pCursor->rowid), pvBookmark, cbMax);
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetChecksum(JET_SESID sesid, JET_TABLEID tableid, PDWORD pdwChecksum)
{
	ISAM_Cursor *pCursor;
	JET_ERR ret;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (pCursor->currency != CURRENCY_CURRENT)
		return JET_errNoCurrentRecord;
	if (ret = EstablishAsCurrentCursor(pCursor))
		return ret;
	/*** TODO: Read data of all columns of current row and build checksum over data
	 ***       (i.e. add all bytes of fields to a DWORD value that becomes the
	 ***        checksum) */
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid, LPWSTR szIndexName,
	ULONG cchIndexName )
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	/*** TODO: Implement, copy index name of current index to szIndexName and return */
	*szIndexName = 0;
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetCursorInfo(JET_SESID sesid, JET_TABLEID tableid, PVOID pvResult,
	ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Cursor *pCursor;

	if (InfoLevel) return JET_errInvalidParameter;
	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (pCursor->currency != CURRENCY_CURRENT)
		return JET_errNoCurrentRecord;
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetRecordPosition(JET_SESID sesid, JET_TABLEID tableid, 
	JET_RECPOS *precpos, ULONG cbRecpos)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (pCursor->currency != CURRENCY_CURRENT)
		return JET_errNoCurrentRecord;

	/*** TODO: If you support this:
	recPos.cbStruct = cbRecpos<sizeof(recPos)?cbRecpos:sizeof(recPos);
	recPos.centriesLT = pCursor->rownum?pCursor->rownum-1:0; // TODO: Set to approximate number of index entries less than the current key 
	recPos.centriesInRange = 1;
	recPos.centriesTotal = EngineRecordCount(pCursor->pTable->pDrvInst);
	return AssignResult(&recPos, sizeof(recPos), precpos, cbRecpos);
	*/
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------
static int SortColumnByName(const void *elem1, const void *elem2)
{
	return wcscmp(((ISAM_COLListData*)elem1)->colDef.szBaseColumnName,
				  ((ISAM_COLListData*)elem2)->colDef.szBaseColumnName);
}

JET_ERR JET_API IsamGetTableColumnInfo(JET_SESID	sesid, JET_TABLEID tableid,
	LPCWSTR szColumnName, PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Cursor *pCursor;
	ISAM_Session *pSession;
	ISAM_VTDef *pVTDef;
	JET_ERR ret = JET_errSuccess;
	JET_COLUMNBASE_OLD colBase={0};
	UINT i;

	switch (InfoLevel)
	{
	case JET_ColInfo:
	case JET_ColInfoList:
	case JET_ColInfoBase:
	case JET_ColInfoListCompact:
	case JET_ColInfoListSortColumnid:
	case JET_ColInfoColumnFormat:
		if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
			return JET_errInvalidTableId;
		if (InfoLevel == JET_ColInfoList || InfoLevel == JET_ColInfoListCompact || InfoLevel == JET_ColInfoListSortColumnid)
		{
			ISAM_COLListData *pColList, **ppColListPtr;

			pSession = ISAMDBFindSession(sesid);
			if (!(pVTDef = ISAMDBAddVTDef(pSession, VTDEF_TYPE_COLUMNINFO)))
				return JET_errOutOfMemory;
			pVTDef->InfoLevel = InfoLevel;
			ppColListPtr = (ISAM_COLListData**)&pVTDef->data.pData;
			for (i=1; EngineFillTableColumnInfo(pCursor, i, &colBase); i++)
			{
				if (!(pColList = MemAllocate(sizeof(ISAM_COLListData))))
				{
					ret = JET_errOutOfMemory;
					break;
				}
				memcpy (&pColList->colDef, &colBase, sizeof(colBase));
				wcscpy(pColList->colDef.szBaseTableName, pCursor->pTable->szTableName);
				pColList->next = NULL;
				*ppColListPtr = pColList;
				ppColListPtr = &pColList->next;
			}
			pVTDef->cRecord = i-1;
			if (InfoLevel != JET_ColInfoListSortColumnid)
			{
				// Sort by column name on normal calls
				pVTDef->data.pData = SortList(pVTDef->data.pData, SortColumnByName) ;
				for (i=0, pColList=pVTDef->data.pData; pColList; pColList=pColList->next, i++)
					pColList->presidx = i;
			}
			if (ret = JET_errSuccess)
			{
				ret = ISAMDBFindTask(CurrentTaskHandle())->pCaller->ErrAllocateTableidForVsesid(
					sesid, &pVTDef->externalTID, pVTDef->vtdid, &g_drvvttblvtbl);
				if (ret == JET_errSuccess)
				{
					/* JET driver uses a reduced version of the structure with length 56 */
					JET_COLUMNLIST_OLD colList = {sizeof(JET_COLUMNLIST_OLD), 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1};

					pVTDef->userid = pCursor->pUser->userid;
					pVTDef->dbid   = pCursor->pUser->dbid;
					pVTDef->InfoLevel = InfoLevel;
					pVTDef->data.pCurData = pVTDef->data.pData;
					colList.tableid = pVTDef->externalTID;
					colList.cRecord = pVTDef->cRecord;
					return AssignResult (&colList, colList.cbStruct, pvResult, cbMax);
				}
			}
			ISAMDBDeleteVTDef(pSession, pVTDef);
			return ret;
		}
		if (!szColumnName) return JET_errInvalidParameter;
		if (!IsValidJETName(szColumnName, 0x4B0) || !(i=EngineFindColumnName(pCursor, szColumnName)))
		{
			ErrorSetExtendedInfoSz1(JET_errvalidNameM, szColumnName, 1);
			return JET_errInvalidName;
		}

		switch (InfoLevel)
		{
		case JET_ColInfoBase:
			if (!EngineFillTableColumnInfo(pCursor, i, &colBase))
				return JET_errColumnNotFound;
			return AssignResult(&colBase, sizeof(colBase), pvResult, cbMax);
		case JET_ColInfoColumnFormat:
			{
				JET_COLUMNFORMAT colFmt = {0};

				colFmt.cbStruct = cbMax<sizeof(colFmt)?cbMax:sizeof(colFmt);
				return AssignResult(&colFmt, sizeof(colFmt), pvResult, cbMax);
			}
		case JET_ColInfo:
			{
				/* JET driver uses a reduced version of the structure with length 24 */
				JET_COLUMNDEF_OLD colDef={0};

				if (!EngineFillTableColumnInfo(pCursor, i, &colBase))
					return JET_errColumnNotFound;
				// columndef_old and colbase_old are the same in the first fields
				memcpy (&colDef, &colBase, sizeof(colDef));
				colDef.cbStruct = cbMax<sizeof(colDef)?cbMax:sizeof(colDef);
				return AssignResult(&colDef, sizeof(colDef), pvResult, cbMax);
			}
		}
		break;
	case JET_ColInfoSysTabCursor:
		return JET_errFeatureNotAvailable;
	default:
		return JET_errInvalidParameter;
	}
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetTableIndexInfo(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName, PVOID pvResult, ULONG cbResult, ULONG InfoLevel)
{
	ISAM_Cursor *pCursor;
	ISAM_Session *pSession;
	ISAM_VTDef *pVTDef;
	JET_ERR ret;

	switch (InfoLevel)
	{
	case JET_IdxInfo:
	case JET_IdxInfoList:
		if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
			return JET_errInvalidTableId;
		
		/*** TODO: if szIndexName is NULL, collect information about all
		           indexes, otherwise search index with this name and then fill
				   ISAM_IDXListData with appropriate information */
		if (szIndexName)
		{
			// ....
			return JET_errIndexNotFound;
		}
		pSession = ISAMDBFindSession(sesid);
		if (!(pVTDef = ISAMDBAddVTDef(pSession, VTDEF_TYPE_INDEXINFO)))
			return JET_errOutOfMemory;
		/*** TODO: Fill pVTDef->data with ISAM_IDXListData list, don't forget to set pVTDef->cRecord
		 ***       to number of records. */

		ret = ISAMDBFindTask(CurrentTaskHandle())->pCaller->ErrAllocateTableidForVsesid(
			sesid, &pVTDef->externalTID, pVTDef->vtdid, &g_drvvttblvtbl);
		if (ret == JET_errSuccess)
		{
			/* JET driver uses a reduced version of the structure with length 60 */
			JET_INDEXLIST_OLD idxList = {sizeof(JET_INDEXLIST_OLD), 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

			pVTDef->userid = pCursor->pUser->userid;
			pVTDef->dbid   = pCursor->pUser->dbid;
			pVTDef->InfoLevel = InfoLevel;
			pVTDef->data.pCurData = pVTDef->data.pData;
			idxList.tableid = pVTDef->externalTID;
			idxList.cRecord = pVTDef->cRecord;
			return AssignResult (&idxList, idxList.cbStruct, pvResult, cbResult);
		}
		ISAMDBDeleteVTDef(pSession, pVTDef);
		return ret;
	case JET_IdxInfoSpaceAlloc:
		/*** TODO: Implement or leave it unimplemented */
	case JET_IdxInfoLCID:
		/*** TODO: Implement or leave it unimplemented */
	case JET_IdxInfoCount:
		/*** TODO: Implement or leave it unimplemented */
	case JET_IdxInfoVarSegMac:
		/*** TODO: Implement or leave it unimplemented */
	case JET_IdxInfoIndexId:
		/*** TODO: Implement or leave it unimplemented */
		return JET_errInvalidParameter;
	case JET_IdxInfoOLC:
	case JET_IdxInfoResetOLC:
	case JET_IdxInfoSysTabCursor:
		return JET_errFeatureNotAvailable;
	default:
		return JET_errInvalidParameter;
	}
}

// ---------------------------------------------------------------------------
#define JET_TblInfoNoStats	100U	// This is custom attrib internally used by IsamGetObjectInfo to save time

JET_ERR JET_API IsamGetTableInfo(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Cursor *pCursor;
	JET_OBJECTINFO objInfo={0};
	ULONG cbSize, uResult[2] = {0};
	PVOID pSrc;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	switch (InfoLevel)
	{
	case JET_TblInfo:
	case JET_TblInfoNoStats:	// custom attrib
		objInfo.cbStruct = cbSize = sizeof(objInfo);
		objInfo.objtyp = JET_objtypTable;
		if (!pCursor->bReadonly) objInfo.grbit |= JET_bitTableInfoUpdatable;
		objInfo.grbit |= JET_bitTableInfoBookmark;
		if (InfoLevel != JET_TblInfoNoStats)
		{
			/*** TODO: Fill cRecord with number of records and cPage with number of pages */
			objInfo.cRecord = EngineRecordCount(pCursor->pTable->pDrvInst);
		}
		if (cbMax<objInfo.cbStruct) objInfo.cbStruct=cbMax;
		pSrc = &objInfo;
		break;
	case JET_TblInfoName:
		pSrc = pCursor->pTable->szTableName;
		cbSize = wcslen(pCursor->pTable->szTableName)*sizeof(WCHAR)+sizeof(WCHAR);
		break;
	case JET_TblInfoDbid:
		uResult[0] = pCursor->pUser->dbid;
		uResult[1] = pCursor->pUser->userid;
		pSrc = &uResult;
		cbSize = 2*sizeof(ULONG);
		break;
	case JET_TblInfoResetOLC:
		pSrc = &pCursor->flags;
		cbSize = sizeof(pCursor->flags);
		break;
	case JET_TblInfoSpaceAlloc:
		/*** TODO: Implement or leave it unimplemented */
	case JET_TblInfoSpaceAvailable:
		/*** TODO: Implement or leave it unimplemented */
	case JET_TblInfoSpaceUsage:
		/*** TODO: Implement or leave it unimplemented */
	case JET_TblInfoTemplateTableName:
		/*** TODO: Implement or leave it unimplemented */
		return JET_errInvalidParameter;
	case JET_TblInfoSpaceOwned:
		/*** TODO: Implement: uResult[0] = ... */
		cbSize = sizeof(ULONG);
		pSrc = &uResult[0];
		break;
	case JET_TblInfoOLC:
	case JET_TblInfoDumpTable:
		return JET_errFeatureNotAvailable;
	case JET_TblInfoRvt:
		return JET_errQueryNotSupported;
	default:
		return JET_errInvalidParameter;
	}
	return AssignResult(pSrc, cbSize, pvResult, cbMax);
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetTableReferenceInfo(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName, PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGotoBookmark(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (cbBookmark != sizeof(pCursor->rowid))
		return JET_errInvalidBookmark;
	pCursor->pTable->pCurrCursor = pCursor;
	pCursor->rowid = *((ULONG*)pvBookmark);
	/*** TODO: Seek to rowid. If not reachable, return JET_errRecordNotFound
	 *** On other seek error:
	 *** SetCursorCurrency(pCursor, CURRENCY_LOST); and return error */
	SetCursorCurrency(pCursor, CURRENCY_CURRENT);

	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGotoPosition(JET_SESID sesid, JET_TABLEID tableid, JET_RECPOS *precpos)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	/*** TODO: Implement 
     if (precpos->centriesLT == 0 && EngineRecordCount(pCursor->pTable->pDrvInst) == 0)
	 {
		SetCursorCurrency(pCursor, CURRENCY_BEGIN);
		return JET_errNoCurrentRecord;
	 }
	 *** On seek error:
	 *** SetCursorCurrency(pCursor, CURRENCY_LOST); and return error
 	 On Success:
	 pCursor->rownum = precpos->centriesLT+1;
	 SetCursorCurrency(pCursor, CURRENCY_CURRENT);
	 return JET_errSuccess;
	 */

	// Or if we just do not support it:
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamIdle(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_wrnNoIdleActivity;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamMakeKey(JET_SESID sesid, JET_TABLEID tableid, const void *pvData,
	ULONG cbData, JET_GRBIT	grbit)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	return JET_errNoCurrentIndex;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamMove(JET_SESID sesid, JET_TABLEID tableid, long cRow,
	JET_GRBIT grbit)
{
	ISAM_Cursor *pCursor;
	JET_ERR ret = JET_errSuccess;
	long count;
	int i;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (grbit & JET_bitMoveKeyNE) return JET_errNoCurrentIndex;
	if (ret = EstablishAsCurrentCursor(pCursor))
		return ret;
	pCursor->pTable->pCurrCursor = pCursor;
	//if (!cRow) return pCursor->currency != CURRENCY_CURRENT ? JET_errNoCurrentRecord : JET_errSuccess;
	if (pCursor->currency == CURRENCY_LOST && cRow != JET_MoveFirst && cRow != JET_MoveLast)
		return JET_errNoCurrentRecord;
	if (pCursor->currency == CURRENCY_END && cRow == JET_MovePrevious) cRow = JET_MoveLast;
	if (pCursor->currency == CURRENCY_BEGIN && cRow == JET_MoveNext) cRow = JET_MoveFirst;
	switch (cRow)
	{
	case JET_MoveNext:
		if (ret = EngineMove(pCursor, cRow, &pCursor->rowid))
			break;
		pCursor->rownum++;
		break;
	case JET_MoveFirst:
		if (ret = EngineMove(pCursor, cRow, &pCursor->rowid))
			break;
		pCursor->rownum = 1;
		break;
	case JET_MovePrevious:
		if (ret = EngineMove(pCursor, cRow, &pCursor->rowid))
			break;
		pCursor->rownum--;
		break;
	case JET_MoveLast:
		if (ret = EngineMove(pCursor, cRow, &pCursor->rowid))
			break;
		pCursor->rownum = EngineRecordCount(pCursor->pTable->pDrvInst);
		break;
	default:
		count = cRow<0?cRow*-1:cRow;
		
		if (pCursor->currency != CURRENCY_END || cRow >= 0)
		{
			if (pCursor->currency == CURRENCY_BEGIN && cRow >= 0)
			{
				if (ret = EngineMove(pCursor, JET_MoveFirst, &pCursor->rowid))
					break;
				count--;
			}
		}	
		else
		{
			if (ret = EngineMove(pCursor, JET_MoveLast, &pCursor->rowid))
				break;
			count--;
			pCursor->rownum = EngineRecordCount(pCursor->pTable->pDrvInst);
		}
		for (i=0; i<count; i++)
		{
			if (cRow<0)
			{
				if (ret = EngineMove(pCursor, JET_MovePrevious, &pCursor->rowid))
					break;
				pCursor->rownum--;
			}
			else
			{
				if (ret = EngineMove(pCursor, JET_MoveNext, &pCursor->rowid))
					break;
				pCursor->rownum++;
			}
		}
	}
	if (ret == JET_errSuccess)
	{
		SetCursorCurrency(pCursor, CURRENCY_CURRENT);
	}
	else
	{
		if (ret == JET_errNoCurrentRecord)
		{
			if (cRow==JET_MoveNext||cRow >= 0)
				SetCursorCurrency(pCursor, CURRENCY_END);
			else
				SetCursorCurrency(pCursor, CURRENCY_BEGIN);
		}
		else
			SetCursorCurrency(pCursor, CURRENCY_LOST);
	}
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamNotifyUpdateUfn(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamNotifyRollback(JET_SESID sesid, JET_TABLEID tableid, int unk3)
{
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamNotifyUnknown(JET_SESID sesid, JET_TABLEID tableid, int unk3)
{
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamNotifyUnknown2(JET_SESID sesid, JET_TABLEID tableid)
{
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamPrepareUpdate(JET_SESID sesid, JET_TABLEID tableid, ULONG prep)
{
	ISAM_Cursor *pCursor;
	JET_ERR ret;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (pCursor->bReadonly) 
		return JET_errPermissionDenied;
	if (!(ret = EstablishAsCurrentCursor(pCursor)))
		return ret;
	/*** TODO: Implement prepare, if you need it
	 ***       You may want to allocate a column buffer here which gets 
	 ***       filled in IsamSetColumn and committed in IsamUpdate */
	switch (prep)
	{
	case JET_prepInsert:
		pCursor->bPrepUpdate = TRUE;
		break;
	case JET_prepReplace:
	case JET_prepCancel:
	case JET_prepReplaceNoLock:
		break;
	case JET_prepInsertCopy:
	case 1:
		return JET_errFeatureNotAvailable;
	default:
		return JET_errInvalidParameter;
	}
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamRenameColumn(JET_SESID  sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	ISAM_Cursor *pCursor;
	JET_ERR ret;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (!(ret = CheckUpdateColumns(pCursor)))
		return ret;

	/*** TODO: Implement */
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamRenameIndex(JET_SESID  sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	/*** TODO: Implement */
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamRenameReference(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamRetrieveColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, PVOID pvData, ULONG cbData, ULONG *pcbActual,
	JET_GRBIT grbit, JET_RETINFO *pretinfo)
{
	ISAM_Cursor *pCursor;
	JET_ERR ret;
	ULONG cbVal = 0, ibLongValue;
	PVOID pvVal = NULL;
	int colType;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if ((colType = EngineGetColumnType(pCursor, columnid)) == -1)
		return JET_errBadColumnId;
	if (pCursor->currency != CURRENCY_CURRENT)
		return JET_errNoCurrentRecord;
	if (ret = EstablishAsCurrentCursor(pCursor))
		return ret;
	/*** TODO: Implement retrieving column data, if Buffer is too small, 
	 *** return JET_wrnBufferTruncated.
	 *** You may want to use AssignResult() for this */
	switch (colType)
	{
	case JET_coltypLong:
		cbVal = sizeof(ULONG);
		break;
	case JET_coltypText:
		/* Attention: returning WCHAR-Text here, you may need to convert!
		   cbVal = length(text) * sizeof(WCHAR)
		 */
		break;
	case JET_coltypBit:
		cbVal = sizeof(BYTE);
		break;
	case JET_coltypUnsignedByte:
		cbVal = sizeof(BYTE);
		break;
	case JET_coltypShort:
		cbVal = sizeof(WORD);
		break;
	case JET_coltypIEEESingle:
		cbVal = sizeof(float);
		break;
	case JET_coltypIEEEDouble:
		cbVal = sizeof(double);
		break;
	case JET_coltypCurrency:
		cbVal = sizeof(double);
		break;
	case JET_coltypDateTime:
		cbVal = sizeof(double);
		break;
	case JET_coltypLongText:
		ibLongValue = pretinfo?pretinfo->ibLongValue:0;
		/*
		cbVal = iLenText * sizeof(WCHAR)
		if (!cbVal || ibLongValue > cbVal)
		{
			if (pcbActual) *pcbActual=NULL;
			return JET_wrnColumnNull;
		}
		cbVal -= ibLongValue;
		*/
		break;
	case JET_coltypBinary:
		break;
	case JET_coltypLongBinary:
		break;
	}

	if (!cbVal)
	{
	  if (pcbActual) *pcbActual=0;
	  return JET_wrnColumnNull;
	}
	if (pcbActual) *pcbActual = cbVal;
	return AssignResult(pvVal, cbVal, pvData, cbData);
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamRetrieveKey(JET_SESID sesid, JET_TABLEID tableid, 
	PVOID pvData, ULONG cbMax, ULONG *pcbActual, JET_GRBIT grbit)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (pCursor->currency != CURRENCY_CURRENT)
		return JET_errNoCurrentRecord;
	/*** TODO: Implement, if supported */
	return JET_errNoCurrentIndex;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamSeek(JET_SESID sesid, JET_TABLEID tableid, JET_GRBIT grbit)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	/*** TODO: Implement, if supported */
	return JET_errNoCurrentIndex;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamSetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	/*** TODO: Implement, if supported */
	return JET_errIndexNotFound;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamSetColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, const void *pvData, ULONG cbData,
	JET_GRBIT grbit, JET_SETINFO *psetinfo)
{
	ISAM_Cursor *pCursor;
	int colType;
	JET_ERR ret = JET_errSuccess;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if ((colType = EngineGetColumnType(pCursor, columnid)) == -1)
		return JET_errBadColumnId;
	if (!pCursor->bPrepUpdate)
		return JET_errUpdateNotPrepared;
	if ((pvData && cbData) || (grbit & JET_bitSetZeroLength))
	{
		/***TODO: Switch data type and fill your allocated buffer accordingly
         * If cbData doesn't match column data size, return JET_errInvalidBufferSize 
		 * If you needed to truncate column data, return JET_wrnColumnDataTruncated */
		switch (colType)
		{
		case JET_coltypLong:
			if (cbData != sizeof(ULONG))
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypText:
			/* Attention: getting WCHAR-Text here, you may need to convert! */
			if (cbData > 255*sizeof(WCHAR))
			{
				if (!(grbit & JET_bitSetSeparateLV))
					return JET_errInvalidBufferSize;
				cbData = 255*sizeof(WCHAR);
				ret = JET_wrnColumnDataTruncated;
			}
			break;
		case JET_coltypBit:
			if (cbData != sizeof(BYTE))
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypUnsignedByte:
			if (cbData != sizeof(BYTE))
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypShort:
			if (cbData != sizeof(WORD))
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypIEEESingle:
			if (cbData != sizeof(float))
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypIEEEDouble:
			if (cbData != sizeof(double))
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypCurrency:
			if (cbData != sizeof(double))
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypDateTime:
			if (cbData != sizeof(double))
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypLongText:
			/* You may need to truncate like with JET_coltypText */
			if (grbit & JET_bitSetAppendLV)
			{
				/* Append long text */
			}
			else
			{
				/* Set long text */
			}
			break;
		case JET_coltypBinary:
			if (cbData > DRIVER_MAX_DS)
				return JET_errInvalidBufferSize;
			break;
		case JET_coltypLongBinary:
			if (cbData > DRIVER_MAX_DS)
				return JET_errInvalidBufferSize;
			break;
		}
	}
	else
	{
		/***TODO: Set your column to NULL */
	}
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamSetIndexRange(JET_SESID sesid, JET_TABLEID tableid,
	JET_GRBIT grbit)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	/*** TODO: Implement, if supported */
	return JET_errNoCurrentIndex;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamUpdate(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark,	ULONG *pcbActual)
{
	ISAM_Cursor *pCursor;
	JET_ERR ret;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	if (!pCursor->bPrepUpdate)
		return JET_errUpdateNotPrepared;
	if (pCursor->bReadonly)
		return JET_errPermissionDenied;
	if (ret = EstablishAsCurrentCursor(pCursor))
		return ret;
	/*** TODO: Update/Insert Data from your allocated buffer and return
	 ***       row id of row inserted in pvBookmark */
	return JET_errSuccess;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamEmptyTable(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark,	ULONG *pcbActual)
{
	ISAM_Cursor *pCursor;

	if (!(pCursor = ISAMDBFindCursor(sesid, tableid)))
		return JET_errInvalidTableId;
	return -5410;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamCollectRecids(JET_SESID sesid, JET_TABLEID tableid,
	RECID_CALLBACK callback, ULONG cbParam1, ULONG cbParam2, LPCWSTR unk6,
	int cbunk6, int unk8, int unk9, int unk10, int unk11)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamOpenILockBytes(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, PVOID pvLockBytes, JET_GRBIT grbit)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamModifyColumn(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szColumnName, int unk4, int unk5)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamReadAhead(JET_SESID sesid, JET_TABLEID tableid,
	PVOID unk3, int unk4, int unk5)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

static JET_FNDEFTBL m_drvtblvtbl = 
{
	sizeof(JET_FNDEFTBL),
	L"X\0"DRIVER_DESC,	
	IsamAddColumn,
	IsamCloseTable,
	IsamComputeStats,
	IsamCopyBookmarks,
	IsamCreateIndex,
	IsamCreateReference,
	IsamDelete,
	IsamDeleteColumn,
	IsamDeleteIndex,
	IsamDeleteReference,
	IsamDupCursor,
	IsamGetBookmark,
	IsamGetChecksum,
	IsamGetCurrentIndex,
	IsamGetCursorInfo,
	IsamGetRecordPosition,
	IsamGetTableColumnInfo,
	IsamGetTableIndexInfo,
	IsamGetTableInfo,
	IsamGetTableReferenceInfo,
	IsamGotoBookmark,
	IsamGotoPosition,
	IsamIdle,
	IsamMakeKey,
	IsamMove,
	IsamNotifyUpdateUfn,
	IsamNotifyRollback,
	IsamNotifyUnknown,
	IsamNotifyUnknown2,
	IsamPrepareUpdate,
	IsamRenameColumn,
	IsamRenameIndex,
	IsamRenameReference,
	IsamRetrieveColumn,
	IsamRetrieveKey,
	IsamSeek,
	IsamSetCurrentIndex,
	IsamSetColumn,
	IsamSetIndexRange,
	IsamUpdate,
	IsamEmptyTable,
	IsamCollectRecids,
	IsamOpenILockBytes,
	NULL,
	NULL,
	NULL,
	NULL,
	IsamModifyColumn,
	IsamReadAhead
};



/******************************************************
 ************** DATABASE LEVEL FUNCTIONS **************
 ******************************************************/

JET_ERR JET_API IsamCapability(JET_SESID sesid, JET_DBID dbid, DWORD unk1, DWORD unk2, DWORD *unk3)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------
static JET_ERR CloseNetDatabase(ISAM_DB *pDB)
{
	ISAM_NetFile *pNetFile, *pNetFileNext;
	JET_ERR ret = JET_errSuccess;

	if (NetProtocolType(pDB->szDBName))
	{
		for (pNetFile = pDB->pNetFiles; pNetFile; pNetFile = pNetFileNext)
		{
			if (pNetFile->bUploadOnExit)
			{
				switch (ret = NetUploadToNet(pNetFile->pszLocalFile, 
					m_pwszExt[1]?&m_pwszExt[1]:NULL, pNetFile->pszRemoteFile))
				{
				case JET_errFileAccessDenied:
					ErrorSetExtendedInfoSz1(JET_errFileAccessDeniedM, pNetFile->pszRemoteFile, 0);
					break;
				case JET_errInvalidPath:
					ErrorSetExtendedInfoSz1(JET_errvalidPathM, pNetFile->pszRemoteFile, 0);
					break;
				case JET_errObjectNotFound:
					ErrorSetExtendedInfoSz1(-8300, pNetFile->pszRemoteFile, 0);
					break;
				}
			}
			NetLocalCleanup(pNetFile->pszLocalFile, m_pwszExt[1]?&m_pwszExt[1]:NULL);
			pNetFileNext = pNetFile->next;
			MemFree(pNetFile);
		}
		NetDestroyLocalDirectory(pDB->szLocalDir);
	}
	return ret;
}

JET_ERR JET_API IsamCloseDatabase(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit)
{
	ISAM_DBUser *pUser;
	ISAM_DBTable *pTable;
	ISAM_Cursor *pCursor;
	ISAM_DB *pDB;
	JET_ERR ret = JET_errSuccess;

	if (!(pUser = ISAMDBFindDatabaseUser(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	pDB = pUser->pDB;
	for (pTable = pDB->pTables; pTable; pTable = pTable->next)
	{
		for (pCursor = pTable->pCursors; pCursor; pCursor=pCursor->next)
		{
			if (pCursor->pUser == pUser)
			{
				IsamCloseTable (sesid, pCursor->cursorid);
			}
		}
	}
	ISAMDBFindTask(CurrentTaskHandle())->pCaller->ReleaseDbid(pUser->dbid);
	ISAMDBDeleteDatabaseUser(pDB, pUser);
	if (!pDB->pUsers)
	{
		ret = CloseNetDatabase(pDB);
		ISAMDBDeleteDatabase(pDB->pTask, pDB);
	}
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamConnectionControl(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamCreateObject(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit, LPWSTR unk1, short unk2)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------
static JET_ERR ProbeLocalDirectory(ISAM_DB *pDB)
{
	WCHAR szPath[MAX_PATH], szRemoteFile[MAX_PATH];
	HANDLE hFile;
	short dosRet;
	JET_ERR ret;

	swprintf (szPath, L"%sTest.txt", pDB->szLocalDir);
	if (dosRet = DOSCreateFile(szPath, &hFile))
	{
		switch (dosRet)
		{
		case -3: return JET_errInvalidPath;
		case -4: return JET_errTooManyOpenFiles;
		case -5: return JET_errFileAccessDenied;
		default: return JET_errOpenFile;
		}
	}
	if (DOSWriteFile(hFile, L"T", sizeof(WCHAR)) == sizeof(WCHAR))
	{
		if (DOSCloseFile(hFile) == 0)
		{
			if ((ret = NetUploadToNet(szPath, NULL, pDB->szDBName)) == JET_errSuccess)
			{
				swprintf (szRemoteFile, L"%sTest.txt", pDB->szDBName);
				NetDeleteFile(szRemoteFile);
			}
			else SetNetError(ret, pDB->szDBName);
		} else ret = JET_errOpenFile;
	}
	else
	{
		ret = JET_errOpenFile;
		DOSCloseFile(hFile);
	}
	DOSFileDelete(szPath);
	return ret;
}

JET_ERR JET_API IsamCreateTable(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName, ULONG lPages, 
						ULONG lDensity, JET_TABLEID	*ptableid)
{
	ISAM_DBUser *pUser;
	ISAM_DBTable *pTable;
	ISAM_Cursor *pCursor;

	WCHAR szLocalTableName[MAX_PATH], *pEnd;
	enum eProtoType type;
	JET_ERR ret;

	if (!(pUser = ISAMDBFindDatabaseUser(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	if (pUser->flags & JET_bitDbReadOnly) 
		return JET_errPermissionDenied;
	if ((type = NetProtocolType(pUser->pDB->szDBName)) == Proto_HTTP)
	{
		wcscpy(szLocalTableName, pUser->pDB->szDBName);
		pEnd = &szLocalTableName[wcslen(szLocalTableName)-1];
		if (*pEnd == '\\') *pEnd=0;
		ErrorSetExtendedInfoSz1(JET_errvalidPathM, szLocalTableName, 0);
		return JET_errInvalidPath;
	}
	if (!type || (ret = ProbeLocalDirectory(pUser->pDB)) == JET_errSuccess)
	{
		if (!ISAMDBLocateTable(pUser->pDB, szTableName))
		{
			if (ret = LocateTableInDatabase(pUser, szTableName, &pTable))
			{
				if (ret != JET_errObjectNotFound) return ret;
				if (!(pTable = ISAMDBAddTable(pUser->pDB, szTableName)))
					return JET_errOutOfMemory;
				if (pCursor = ISAMDBAddCursor(pUser, pTable))
				{
					/*** TODO: This would place a lock on the table.. Depends on whether your DB 
					   * driver needs exclusive locks on the DB file or not */
					//pCursor->flags = JET_bitTableDenyRead | JET_bitTableDenyWrite;
					ret = ISAMDBFindTask(CurrentTaskHandle())->pCaller->ErrAllocateTableidForVsesid(
						sesid, ptableid, pCursor->cursorid, &m_drvtblvtbl);
					if (ret == JET_errSuccess)
					{
						pCursor->externalTID = *ptableid;
						if (MakeIntoValidFilename (pTable->szTableName, szLocalTableName, 255, 255, FALSE, GetACP())==2) 
							ret = JET_wrnOptionsIgnored;
						return ret;
					}
					ISAMDBDeleteCursor(pTable, pCursor);
				}
				else ret = JET_errOutOfMemory;
			} else ret = JET_errTableDuplicate;
		} else ret = JET_errTableDuplicate;
		ErrorSetExtendedInfoSz1(JET_errTableDuplicateM, szTableName, 1);
		ISAMDBDeleteTable(pUser->pDB, pTable);
	}
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamDeleteObject(JET_SESID sesid, JET_DBID dbid, LPCWSTR szObjectName)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamDeleteTable(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName)
{
	ISAM_DBUser *pUser;
	ISAM_DBTable *pTable;
	JET_ERR ret;
	WCHAR szFile[MAX_PATH], szLocalTable[MAX_PATH], *pwszExt;
	HANDLE hFile;
	int i;

	if (!(pUser = ISAMDBFindDatabaseUser(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	if (pUser->flags & JET_bitDbReadOnly) 
		return JET_errPermissionDenied;
	if ((ret = LocateTableInDatabase(pUser, szTableName, &pTable)) == JET_errSuccess)
	{
		if (pTable->pDrvInst)
		{
			ErrorSetExtendedInfoSz2(JET_errTableInUseQM, szTableName, 1);
			return JET_errTableInUse;
		}
		MakeIntoValidFilename (pTable->szTableName, szFile, 255, 255, FALSE, GetACP());
		pwszExt = szLocalTable + swprintf(szLocalTable, L"%s%s", *pUser->pDB->szLocalDir?pUser->pDB->szLocalDir:pUser->pDB->szDBName, szFile);
		wcscpy(pwszExt, m_pwszExt[0]);
		if (DOSOpenFile(szLocalTable, OF_READWRITE, &hFile))
		{
			ISAMDBDeleteTable(pUser->pDB, pTable);
			ErrorSetExtendedInfoSz1(JET_errFileAccessDeniedM, szTableName, 1);
			return JET_errFileAccessDenied;
		}
		DOSCloseFile(hFile);
		if (!DOSFileDelete(szLocalTable)) ret = JET_errFileAccessDenied;
		else
		{
			for (i=1; m_pwszExt[i]; i++)
			{
				wcscpy(pwszExt, m_pwszExt[i]);
				DOSFileDelete(szLocalTable);
			}
		}
		ISAMDBDeleteTable(pUser->pDB, pTable);
	}
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamUnknownFunct(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetColumnInfo(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName,
						LPCWSTR szColumnName, OBJINFO *ColumnInfo, ULONG InfoLevel)
{
	ISAM_DBUser *pUser;
	ISAM_DBTable *pTable;
	ISAM_Cursor *pCursor;
	JET_ERR ret;

	if (!(pUser = ISAMDBFindDatabaseUser(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	if ((ret = LocateTableInDatabase(pUser, szTableName, &pTable)) == JET_errSuccess)
	{
		if (pTable->pCursors)
		{
			ret = IsamGetTableColumnInfo(sesid, pTable->pCursors->cursorid, szColumnName,
				ColumnInfo->pResult, ColumnInfo->cbResult, InfoLevel);
		}
		else
		{
			if (ret = EngineOpenTable(pUser, pTable, 0))
				return ret;
			if (pCursor = ISAMDBAddCursor(pUser, pTable))
			{
				ret = IsamGetTableColumnInfo(sesid, pCursor->cursorid, szColumnName,
					ColumnInfo->pResult, ColumnInfo->cbResult, InfoLevel);
				ISAMDBDeleteCursor(pTable, pCursor);
			} else ret = JET_errOutOfMemory;
			EngineCloseTable(pTable->pDB, pTable);
		}
	}
	return ret;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetDatabaseInfo(JET_SESID sesid, JET_HANDLE userid, PVOID pvResult,
						ULONG cbMax, ULONG InfoLevel)
{
	ISAM_DBUser *pUser;
	PVOID pSrc = NULL;
	ULONG uResult;

	if (!(pUser = ISAMDBFindDatabaseUser(sesid, userid)))
		return JET_errInvalidDatabaseId;

	switch (InfoLevel)
	{
	case JET_DbInfoFilename:
		return AssignResult(pUser->pDB->szDBName, 
			wcslen(pUser->pDB->szDBName)*sizeof(WCHAR)+sizeof(WCHAR), pvResult, cbMax);
		break;
	case JET_DbInfoConnect:
		return AssignResult(pUser->szConnectStr, 
			wcslen(pUser->szConnectStr)*sizeof(WCHAR)+sizeof(WCHAR), pvResult, cbMax);
	case JET_DbInfoCountry:
	case JET_DbInfoCollate:
	case JET_DbInfoTransactions:
	case 18:
		uResult = 0;
		return AssignResult (&uResult, sizeof(uResult), pvResult, cbMax);
	case JET_DbInfoVersion:
		uResult = 0x620;	// Text driver has 0x10000 here, but documentation states that 0x620 is used for XP+
		return AssignResult (&uResult, sizeof(uResult), pvResult, cbMax);
	case JET_DbInfoIsam:
		if (cbMax != 8) return JET_errInvalidBufferSize;
		*(ULONG*)pvResult = 27;
		*(ULONG*)((BYTE*)pvResult+sizeof(ULONG)) = 0;
		return JET_errSuccess;
	case JET_DbInfoLCID:
		uResult = LOCALE_USER_DEFAULT;
		return AssignResult (&uResult, sizeof(uResult), pvResult, cbMax);
	case JET_DbInfoCp:
		uResult = 1252;
		return AssignResult (&uResult, sizeof(uResult), pvResult, cbMax);
	case JET_DbInfoOptions:
		return AssignResult (&pUser->pDB->flags, sizeof(pUser->pDB->flags), pvResult, cbMax);
	default:
		return JET_errInvalidParameter;
	}
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetIndexInfo(JET_SESID sesid, JET_HANDLE userid, LPCWSTR szTableName,
						LPCWSTR szIndexName, OBJINFO *IndexInfo, ULONG InfoLevel )
{
	ISAM_DBUser *pUser;
	ISAM_DBTable *pTable;
	ISAM_Cursor *pCursor;
	JET_ERR ret;

	if (!(pUser = ISAMDBFindDatabaseUser(sesid, userid)))
		return JET_errInvalidDatabaseId;
	if ((ret = LocateTableInDatabase(pUser, szTableName, &pTable)) == JET_errSuccess)
	{
		if (pTable->pCursors)
		{
			ret = IsamGetTableIndexInfo(sesid, pTable->pCursors->cursorid, szIndexName,
				IndexInfo->pResult, IndexInfo->cbResult, InfoLevel);
		}
		else
		{
			if (ret = EngineOpenTable(pUser, pTable, 0))
				return ret;
			if (pCursor = ISAMDBAddCursor(pUser, pTable))
			{
				ret = IsamGetTableIndexInfo(sesid, pCursor->cursorid, szIndexName,
					IndexInfo->pResult, IndexInfo->cbResult, InfoLevel);
				ISAMDBDeleteCursor(pTable, pCursor);
			} else ret = JET_errOutOfMemory;
			EngineCloseTable(pTable->pDB, pTable);
		}
	}
	return ret;
}

// ---------------------------------------------------------------------------

static BOOL GetDBCreationTime(LPCWSTR lpFileName, JET_DATESERIAL *dtDest)
{
	HANDLE hFile;
	DATETIME dt;
	BOOL bRet = FALSE;

	if (DOSOpenFile (lpFileName, OF_READ, &hFile) == 0)
	{
		if (bRet = DOSChannelDateTime(hFile, 0, &dt.nYear, &dt.nMonth, &dt.nDay, &dt.nHour, &dt.nMinute, &dt.nSecond))
			DtfParsedToSerial(&dt, dtDest);
		DOSCloseFile(hFile);
	}
	return bRet;
}

static JET_ERR SetNetError(JET_ERR ret, WCHAR *szTable)
{
	switch (ret)
	{
	case JET_errFileAccessDenied:
		ErrorSetExtendedInfoSz1(JET_errFileAccessDeniedM, szTable, 0);
		break;
	case JET_errInvalidPath:
		ErrorSetExtendedInfoSz1(JET_errvalidPathM, szTable, 0);
		break;
	case JET_errObjectNotFound:
		ErrorSetExtendedInfoSz1(-8300, szTable, 0);
		break;
	}
	return ret;
}

JET_ERR LocateTableInDatabase(ISAM_DBUser *pUser, LPCWSTR lpObjectName, 
							  ISAM_DBTable **ppTable)
{
	WCHAR szTable[MAX_PATH], szFile[MAX_PATH], *pwszExt;
	JET_ERR ret;
	enum eProtoType type;
	short DOSret;

	if (!*lpObjectName) return JET_errInvalidParameter;
	
	if (*ppTable = ISAMDBLocateTable(pUser->pDB, lpObjectName))
		return JET_errSuccess;

	/*** TODO: Check if lpObjectName is valid for your DB and return JET_errInvalidName if not. */
	if (!IsValidJETName(lpObjectName, GetACP())) 
	{
		ErrorSetExtendedInfoSz1(JET_errvalidNameM, lpObjectName, TRUE);
		return JET_errInvalidName;
	}
	MakeIntoValidFilename (lpObjectName, szFile, 255, 255, FALSE, GetACP());

	/* Table = pDB->szDBName + szFile, i.e. c:\temp\ + [dummytbl] */
	pwszExt = szTable + swprintf(szTable, L"%s%s", pUser->pDB->szDBName, szFile);
	wcscpy(pwszExt, m_pwszExt[0]);

	/* We may need to download the stuff, if it's not there */
	if (type = NetProtocolType(pUser->pDB->szDBName))
	{
		WCHAR szLocalTable[MAX_PATH];

		swprintf(szLocalTable, L"%s%s%s", pUser->pDB->szLocalDir, szFile, m_pwszExt[0]);
		
		if (DOSFileExists(szLocalTable))
		{
			// Not there, download to local dir
			if ((ret = NetDownloadToLocal(szTable, m_pwszExt[1]?&m_pwszExt[1]:NULL, 
				pUser->pDB->szLocalDir, szLocalTable)) != JET_errSuccess)
				return SetNetError(ret, szLocalTable);

			if (!ISAMDBAddNetFile(pUser->pDB, szTable, szLocalTable, FALSE))
				return JET_errOutOfMemory;

			// With HTTP, we have to load additional files manually
			if (type == Proto_HTTP)
			{
				int i;
				WCHAR szTempFile[MAX_PATH];

				for (i=1; m_pwszExt[i]; i++)
				{
					wcscpy(pwszExt, m_pwszExt[i]);
					NetDownloadToLocal(szTable, NULL, pUser->pDB->szLocalDir, szTempFile);
				}
			}
		}
		wcscpy(szTable, szLocalTable);
	}
		
	if (DOSret = DOSFileExists(szTable))
	{
		switch (DOSret)
		{
		case -3:
			ErrorSetExtendedInfoSz1(JET_errvalidPathM, szTable, 0);
			return JET_errInvalidPath;
		case -5:
			ErrorSetExtendedInfoSz1(JET_errFileAccessDeniedM, lpObjectName, 0);
			return JET_errFileAccessDenied;
		case -21:
			return JET_errDiskNotReady;
		default:
			ErrorSetExtendedInfoSz1(-8300, lpObjectName, 0);
			return JET_errObjectNotFound;
		}
	}

	if (!(*ppTable = ISAMDBAddTable(pUser->pDB, lpObjectName)))
		return JET_errOutOfMemory;
	DOSFileSize(szTable, &(*ppTable)->dwFileSize);
	GetDBCreationTime(szTable, &(*ppTable)->dtCreate);
	return JET_errSuccess;
}

JET_ERR DestroyColumnList(ISAM_ColList *pColList)
{
	ISAM_ColList *pCol, *pColNext;

	for (pCol=pColList; pCol; pCol = pColNext)
	{
		pColNext = pCol->next;
		MemFree(pCol);
	}
	return JET_errSuccess;
}

JET_ERR ListFiles(LPCWSTR lpPath, ISAM_ColList **pColList)
{
	WCHAR szFile[MAX_PATH], szFound[MAX_PATH], *pExt;
	BOOL bFound;
	JET_ERR ret;
	DOSFIND hFind;
	ISAM_ColList *pCol;

	*pColList = NULL;
	swprintf (szFile, L"%s*%s", lpPath, m_pwszExt[0]);
	bFound = DOSFindFirstMatchingFile(szFile, szFound, &hFind, &ret);
	if (!bFound) return JET_errSuccess;
	do
	{
		if (ret) return ret;
		if (wcscmp(szFound, L".") && wcscmp(szFound, L".."))
		{
			if (!(pCol = MemAllocate(sizeof(ISAM_ColList))))
			{
				DestroyColumnList(*pColList);
				return JET_errOutOfMemory;
			}
			if (pExt = wcsrchr(szFound, '.')) *pExt=0;
			wcscpy(pCol->colname, szFound);
			pCol->next = *pColList;
			*pColList = pCol;
		}
	}
	while (DOSFindNextMatchingFile(szFound, &hFind, &ret));
	return JET_errSuccess;
}

JET_ERR JET_API IsamGetObjectInfo(JET_SESID sesid, JET_DBID dbid, JET_OBJTYP	objtyp,
						LPCWSTR szContainerName, LPCWSTR szObjectName, OBJINFO *IndexInfo, 
						ULONG InfoLevel )
{
	ISAM_DBUser *pUser;
	ISAM_ColList *pColList, *pCol;
	ISAM_DBTable *pTable;
	ISAM_Session *pSession;
	ISAM_Cursor *pCursor;
	ISAM_VTDef *pVTDef;
	ISAM_OBJListData *pListData;
	JET_ERR ret;

	switch (InfoLevel)
	{
	case JET_ObjInfo:
	case JET_ObjInfoNoStats:
	case JET_ObjInfoList:
	case JET_ObjInfoListNoStats:
		/*if (InfoLevel && objtyp != JET_objtypTable)
			return JET_errFeatureNotAvailable;*/
		if (!(pUser = ISAMDBFindDatabaseUser(sesid, dbid)))
			return JET_errInvalidDatabaseId;
		if (InfoLevel == JET_ObjInfo || InfoLevel == JET_ObjInfoNoStats)
		{
			if (!szObjectName || (szContainerName && DBlstrcmpi(szContainerName, L"Tables")))
				return JET_errInvalidParameter;
			if ((ret = LocateTableInDatabase(pUser, szObjectName, &pTable)) == JET_errSuccess)
			{
				if (!pTable->pCursors)
				{
					if (ret = EngineOpenTable(pUser, pTable, 0))
						return ret;
					if (pCursor = ISAMDBAddCursor(pUser, pTable))
					{
						ret = IsamGetTableInfo(sesid, pCursor->cursorid,
							IndexInfo->pResult, IndexInfo->cbResult, 
							InfoLevel==JET_ObjInfoNoStats?JET_TblInfoNoStats:JET_TblInfo);
							ISAMDBDeleteCursor(pTable, pCursor);
					} else ret = JET_errOutOfMemory;
					EngineCloseTable(pTable->pDB, pTable);
					return ret;
				}
				return IsamGetTableInfo(sesid, pTable->pCursors->cursorid,
					IndexInfo->pResult, IndexInfo->cbResult, 
					InfoLevel==JET_ObjInfoNoStats?JET_TblInfoNoStats:JET_TblInfo);
			}			
			return ret;
		}
		if (!(pSession = ISAMDBFindSession(sesid)))
			return JET_errInvalidSesid;
		if (ret = ListFiles(pUser->pDB->szDBName, &pColList))
			return ret;
		if (!(pVTDef = ISAMDBAddVTDef(pSession, VTDEF_TYPE_OBJINFO)))
		{
			DestroyColumnList (pColList);
			return JET_errOutOfMemory;
		}
		pVTDef->InfoLevel = InfoLevel;
		for (pCol = pColList, pVTDef->cRecord = 0; pCol; pCol=pCol->next)
		{
			if (!(pListData = MemAllocate(sizeof(ISAM_OBJListData))))
			{
				ISAMDBDeleteVTDef(pSession, pVTDef);
				DestroyColumnList (pColList);
				return JET_errOutOfMemory;
			}
			if ((ret = LocateTableInDatabase(pUser, pCol->colname, &pTable)) == JET_errSuccess)
			{
				pListData->objInfo.dtCreate = pListData->objInfo.dtUpdate = pTable->dtCreate;
				if (!pTable->pCursors)
				{
					if ((ret = EngineOpenTable(pUser, pTable, 0)) == JET_errSuccess)
					{
						if (pCursor = ISAMDBAddCursor(pUser, pTable))
						{
							ret = IsamGetTableInfo(sesid, pCursor->cursorid,
								&pListData->objInfo, sizeof(pListData->objInfo), 
								InfoLevel==JET_ObjInfoListNoStats?JET_TblInfoNoStats:JET_TblInfo);
								ISAMDBDeleteCursor(pTable, pCursor);
						} else ret = JET_errOutOfMemory;
						EngineCloseTable(pTable->pDB, pTable);
					}
				}
				else
				{
					ret = IsamGetTableInfo(sesid, pTable->pCursors->cursorid,
						&pListData->objInfo, sizeof(pListData->objInfo), 
						InfoLevel==JET_ObjInfoListNoStats?JET_TblInfoNoStats:JET_TblInfo);
				}
				if (ret == JET_errSuccess)
				{
					wcscpy(pListData->szObjectName, pCol->colname);
					pListData->next = pVTDef->data.pData;
					pVTDef->data.pData = pListData;
					pVTDef->cRecord++;
				}
			}
		}
		DestroyColumnList (pColList);
		if (pVTDef->cRecord)
		{
			JET_OBJECTLIST objList = {sizeof(JET_OBJECTLIST), 0, 0, 0, 1, 2, 3, 4, 7, 8, 5, 6};

			pVTDef->data.pCurData = pVTDef->data.pData;
			pVTDef->currency = CURRENCY_CURRENT;
			ret = ISAMDBFindTask(CurrentTaskHandle())->pCaller->ErrAllocateTableidForVsesid(
				sesid, &pVTDef->externalTID, pVTDef->vtdid, &g_drvvttblvtbl);
			if (ret != JET_errSuccess)
			{
				ISAMDBDeleteVTDef(pSession, pVTDef);
				return ret;
			}
			pVTDef->userid = pUser->userid;
			pVTDef->dbid   = pUser->dbid;
			objList.tableid = pVTDef->externalTID;
			objList.cRecord = pVTDef->cRecord;
			return AssignResult (&objList, sizeof(objList), IndexInfo->pResult, IndexInfo->cbResult);
		}
		return JET_errNoCurrentRecord;
	case JET_ObjInfoSysTabCursor:
	case JET_ObjInfoListACM:
	case JET_ObjInfoSysTabReadOnly:
	case JET_ObjInfoRulesLoaded:
		return JET_errFeatureNotAvailable;
	default:
		return JET_errInvalidParameter;
	}
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetReferenceInfo(JET_SESID sesid, JET_DBID dbid, LPCWSTR szContainerName, 
						LPCWSTR szIndexName, OBJINFO *IndexInfo, JET_OBJTYP	reftyp,
						ULONG InfoLevel )
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

static BOOL CheckTableLock(ISAM_DBTable *pTable, JET_GRBIT lock)
{
	ISAM_Cursor *pCursor;

	for (pCursor = pTable->pCursors; pCursor && !(pCursor->flags & lock); pCursor = pCursor->next);
	return pCursor?TRUE:FALSE;
}

static BOOL CheckIfAllCursorsHaveFlag(ISAM_DBTable *pTable, JET_GRBIT flag)
{
	ISAM_Cursor *pCursor;

	for (pCursor = pTable->pCursors; pCursor && (pCursor->flags & flag); pCursor = pCursor->next);
	return pCursor?FALSE:TRUE;
}

JET_ERR OpenTable(JET_SESID sesid, ISAM_Session *pSession, ISAM_DBUser *pUser, JET_TABLEID *ptableid,
				  LPCWSTR szTableName, JET_GRBIT grbit, BOOL bIgnoreLocks, BOOL bUnk1)
{
	JET_ERR ret;
	ISAM_DBTable *pTable;
	ISAM_Cursor *pCursor;

	if ((ret = LocateTableInDatabase(pUser, szTableName, &pTable)) == JET_errSuccess)
	{
		if (pUser->flags & JET_bitTableDenyWrite)
			grbit |= JET_bitTableReadOnly;

		if (!bIgnoreLocks)
		{
			if (CheckTableLock(pTable, JET_bitTableDenyRead))
			{
				ErrorSetExtendedInfoSz2(JET_errTableLockedQM, szTableName, 1);
				return JET_errTableLocked;
			}
			if ((CheckTableLock(pTable, JET_bitTableDenyWrite) && grbit != JET_bitTableReadOnly) ||
				(pTable->pCursors && ((grbit & JET_bitTableDenyRead) || 
									  (grbit & JET_bitTableDenyWrite) && !CheckIfAllCursorsHaveFlag(pTable, JET_bitTableReadOnly)) ))
			{
				ErrorSetExtendedInfoSz2(JET_errTableInUseQM, szTableName, 1);
				return JET_errTableInUse;
			}
		}
		if (pUser->flags & JET_bitDbExclusive)
			grbit |= JET_bitTableDenyWrite | JET_bitTableDenyRead;
		if (!pTable->pDrvInst)
		{
			if (ret = EngineOpenTable(pUser, pTable, grbit))
				return ret;
			if (grbit & JET_bitTableReadOnly) pTable->bReadonly = TRUE;
		}
		if (pCursor = ISAMDBAddCursor(pUser, pTable))
		{
			if (grbit & JET_bitTableReadOnly) pCursor->bReadonly = TRUE;
			pCursor->flags = grbit;
			pTable->pCurrCursor = pCursor;
			if (grbit & JET_bitTableClass8)
				pCursor->bClass8 = TRUE;
			ret = ISAMDBFindTask(CurrentTaskHandle())->pCaller->ErrAllocateTableidForVsesid(
				pSession->nSession, ptableid, pCursor->cursorid, &m_drvtblvtbl);
			if (ret == JET_errSuccess)
			{
				pCursor->externalTID = *ptableid;
				return ret;
			}
			ISAMDBDeleteCursor(pTable, pCursor);
		}
		EngineCloseTable(pTable->pDB, pTable);
		if (!pTable->pCursors)
			ISAMDBDeleteTable(pTable->pDB, pTable);
		return JET_errOutOfMemory;
	}
	return ret;
}

JET_ERR JET_API IsamOpenTable(JET_SESID sesid, JET_DBID dbid, JET_TABLEID *ptableid,
						LPCWSTR szTableName, JET_GRBIT grbit)
{
	ISAM_DBUser *pUser;

	if (!(pUser = ISAMDBFindDatabaseUser(sesid, dbid)))
		return JET_errInvalidDatabaseId;

	return OpenTable(sesid, pUser->pSession, pUser, ptableid, szTableName, grbit, FALSE, TRUE);
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamRenameObject(JET_SESID sesid,	JET_DBID dbid, LPCWSTR szTableName, 
						LPCWSTR szName,	LPCWSTR szNameNew)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamRenameTable(JET_SESID sesid, JET_DBID dbid, LPCWSTR szName, LPCWSTR szNameNew)
{
	return ISAMDBFindDatabaseUser(sesid, dbid)?JET_errFeatureNotAvailable:JET_errInvalidDatabaseId;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamGetObjidFromName(JET_SESID sesid, JET_DBID dbid, LPCWSTR szContainerName, 
						LPCWSTR szObjectName, JET_OBJID *objid)
{
	return JET_errFeatureNotAvailable;
}

// ---------------------------------------------------------------------------

JET_ERR JET_API IsamChangeDbPasswordEx(JET_SESID sesid, JET_DBID dbid, LPCWSTR szNewPassword, 
						LPCWSTR szOldPassword, JET_GRBIT grbit)
{
	return JET_errIllegalOperation;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

JET_FNDEFDB g_drvdbvtbl = 
{
	sizeof(JET_FNDEFDB),
	L"X\0"DRIVER_DESC,
	IsamCapability,
	IsamCloseDatabase,
	IsamConnectionControl,
	IsamCreateObject,
	IsamCreateTable,
	IsamDeleteObject,
	IsamDeleteTable,
	IsamUnknownFunct,
	IsamGetColumnInfo,
	IsamGetDatabaseInfo,
	IsamGetIndexInfo,
	IsamGetObjectInfo,
	IsamGetReferenceInfo,
	IsamOpenTable,
	IsamRenameObject,
	IsamRenameTable,
	IsamGetObjidFromName,
	IsamChangeDbPasswordEx
};

