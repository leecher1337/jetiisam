#include "iisam.h"
#include "isamutil.h"
#include "driver.h"
#include "trace.h"
#include <stdio.h>
#include <time.h>

#define INITBUF 1024		/* Initial size of buffer */

static FILE *m_fpLog = NULL;
static WCHAR *m_szLogBuf = NULL;
static DWORD m_iBufSize = 0;
static CRITICAL_SECTION m_cs;

extern JET_CALLIN *g_pCaller;

/******************************************************
 *********************** STATIC ***********************
 ******************************************************/
static void DumpColumnInfo(const void *pvData, const ULONG cbData, ULONG InfoLevel)
{
		switch (InfoLevel)
		{
		case JET_ColInfoList:
		case JET_ColInfoListCompact:
		case JET_ColInfoListSortColumnid:
			if (cbData >= sizeof(JET_COLUMNLIST_OLD))
			{
				JET_COLUMNLIST_OLD *pInfo = (JET_COLUMNLIST_OLD*)pvData;
				TraceLog (L"\nColumnList {\n\t.cbStruct = %d\n\t.tableid = %d\n\t.cRecord = %d\n\t.columnidPresentationOrder = %d\n"
					L"\t.columnidcolumnname = %d\n\t.columnidcolumnid = %d\n\t.columnidcoltyp = %d\n\t.columnidLangid = %d\n"
					L"\t.columnidcbMax = %d\n\t.columnidgrbit = %d\n\t.columnidCp = %d\n\t.columnidBaseTableName = %d\n"
					L"\t.columnidBaseColumnName = %d\n\t.columnidDefinitionName = %d\n};",
					pInfo->cbStruct, pInfo->tableid, pInfo->cRecord, pInfo->columnidPresentationOrder, pInfo->columnidcolumnname,
					pInfo->columnidcolumnid, pInfo->columnidcoltyp, pInfo->columnidLangid, pInfo->columnidcbMax,
					pInfo->columnidgrbit, pInfo->columnidCp, pInfo->columnidBaseTableName, pInfo->columnidBaseColumnName,
					pInfo->columnidDefinitionName);
			}
			break;
		case JET_ColInfoBase:
			if (cbData >= sizeof(JET_COLUMNBASE_OLD))
			{
				JET_COLUMNBASE_OLD *pBase = (JET_COLUMNBASE_OLD*)pvData;
				TraceLog (L"\nColumnBase:\n\t.cbStruct = %d\n\t.columnid = %d\n\t.coltyp = %d\n\t.wCountry = %d\n\t.langid = %d\n"
					L"\tbMax = %d\n\t.grbit = %d\n\t.szBaseTableName = %s\n\t.szBaseColumnName = %s\n};",
					pBase->cbStruct, pBase->columnid, pBase->coltyp, pBase->wCountry, pBase->langid, pBase->cbMax,
					pBase->grbit, pBase->szBaseTableName, pBase->szBaseColumnName);
			}
			break;
		case JET_ColInfoColumnFormat:
			if (cbData >= sizeof(JET_COLUMNFORMAT))
			{
				JET_COLUMNFORMAT *pFmt = (JET_COLUMNFORMAT*)pvData;
				TraceLog (L"\nColumnFormat {\n\t.cbStruct = %d\n\tunk1 = %d\n\t.szColumnFormat = %s\n};",
					pFmt->cbStruct, pFmt->dwUnk1, pFmt->szColumnFormat);
			}
			break;
		case JET_ColInfo:
			if (cbData >= sizeof(JET_COLUMNDEF_OLD))
			{
				JET_COLUMNDEF_OLD *pFmt = (JET_COLUMNDEF_OLD*)pvData;
				TraceLog (L"\nColumnDef {\n\t.cbStruct = %d\n\t.columnid = %d\n\t.coltyp = %d\n\t.wCountry = %d\n\t"
					L".langid = %d\n\t.cbMax = %d\n\t.grbit = %d\n};",
					pFmt->cbStruct, pFmt->columnid, pFmt->coltyp, pFmt->wCountry, pFmt->langid, pFmt->cbMax,
					pFmt->grbit);
			}
			break;
		}
}

static void DumpIndexInfo(const void *pvData, const ULONG cbData, ULONG InfoLevel)
{
		switch (InfoLevel)
		{
		case JET_IdxInfo:
		case JET_IdxInfoList:
			if (cbData >= sizeof(JET_INDEXLIST_OLD))
			{
				JET_INDEXLIST_OLD *pList = (JET_INDEXLIST_OLD*)pvData;
				TraceLog (L"\nIndexList {\n\t.cbStruct = %d\n\t.tableid = %d\n\t.cRecord = %d\n\t.columnidindexname = %d\n"
					L"\t.columnidgrbitIndex = %d\n\t.columnidcKey = %d\n\t.columnidcEntry = %d\n\t.columnidcPage = %d\n"
					L"\t.columnidcColumn = %d\n\t.columnidiColumn = %d\n\t.columnidcolumnid = %d\n\t.columnidcoltyp = %d\n"
					L"\t.columnidLangid = %d\n\t.columnidgrbitColumn = %d\n\t.columnidcolumnname = %d\n};", 
					pList->cbStruct, pList->tableid, pList->cRecord, pList->columnidindexname, pList->columnidgrbitIndex,
					pList->columnidcKey, pList->columnidcEntry, pList->columnidcPage, pList->columnidcColumn, 
					pList->columnidiColumn, pList->columnidcolumnid, pList->columnidcoltyp, pList->columnidLangid,
					pList->columnidgrbitColumn, pList->columnidcolumnname);
			}
			break;
		case JET_IdxInfoSpaceAlloc:
		case JET_IdxInfoCount:
			if (cbData >= sizeof(ULONG))
				TraceLog( L" [Info = %d]", *(ULONG*)pvData);
			break;
		case JET_IdxInfoLCID:
			if (cbData >= sizeof(LCID))
				TraceLog( L" [Info = %d]", *(LCID*)pvData);
			break;
#ifdef JET_IdxInfoKeyMost
		case JET_IdxInfoKeyMost:
			if (cbData >= sizeof(USHORT))
				TraceLog( L" [Info = %d]", *(USHORT*)pvData);
			break;
#endif
		}
}

static void DumpRecPos(JET_RECPOS *precpos)
{
	TraceLog (L"RecPos {\n\t.cbStruct=%d\n\t.centriesLT=\n\t.centriesInRange=%d\n\t.centriesTotal=%d\n};\n",
		precpos->cbStruct, precpos->centriesLT, precpos->centriesInRange, precpos->centriesTotal);
}

static void DumpObjectInfo(JET_OBJECTINFO *pInfo)
{
	TraceLog (L"\nObjectInfo {\n\t.cbStruct = %d\n\t.objtyp = %d\n\t.dtCreate = %f\n\t.dtUpdate = %f\n"
		L"\t.grbit = %d\n\t.flags = %d\n\t.cRecord = %d\n\t.cPage = %d\n};",
		pInfo->cbStruct, pInfo->objtyp, pInfo->dtCreate, pInfo->dtUpdate, pInfo->grbit, pInfo->flags,
		pInfo->cRecord, pInfo->cPage);
}

static void DumpRetinfo(JET_RETINFO *pretinfo)
{
	TraceLog (L"\npretInfo {\n\t.cbStruct = %d\n\t.ibLongValue = %d\n"
		L"\t.itagSequence = %d\n\t.columnidNextTagged = %d\n};\n", pretinfo->cbStruct,
		pretinfo->ibLongValue, pretinfo->itagSequence, pretinfo->columnidNextTagged);
}

static void DumpSetInfo(JET_SETINFO *psetinfo)
{
	TraceLog( L" \npsetinfo {\n\t.cbStruct = %d\n\t.ibLongValue = %d\n\t.itagSequence = %d\n};\n",
		psetinfo->cbStruct, psetinfo->ibLongValue, psetinfo->itagSequence);
}

#define HEXDUMP_BREAK	16
static void HexDump(PVOID lpData, int cbLength)
{
	PBYTE pData = (PBYTE)lpData;
	WCHAR szLine[128]={0}, *p = szLine;
	int i;

	for (i=0; i<cbLength; i++)
	{
		if (i%HEXDUMP_BREAK == 0)
		{
			if (i>=HEXDUMP_BREAK)
			{
				int j;

				p+=swprintf(p, L"    ");
				for (j=i-HEXDUMP_BREAK; j<i; j++, p++) *p =
					(isprint(pData[j]) && pData[j]!=0x0D && pData[j]!=0x0A && pData[j]!=0x09)?pData[j]:'.';
			}
			TraceLog(L"%s\r\n", szLine);
			ZeroMemory(szLine, sizeof(szLine));
			p=szLine;
		}
		p+=swprintf(p, L"%02X ", pData[i]);
		if (i == cbLength-1)
		{
			int j;

			for (j=0; j<HEXDUMP_BREAK - i%HEXDUMP_BREAK - 1; j++) p+=swprintf(p, L"   ");
			p+=swprintf(p, L"    ");
			for (j=i-i%HEXDUMP_BREAK; j<i; j++, p++) *p =
				(isprint(pData[j]) && pData[j]!=0x0D && pData[j]!=0x0A && pData[j]!=0x09)?pData[j]:'.';
		}
	}
	if (p!=szLine) TraceLog(L"%s\r\n", szLine);
}

/******************************************************
 *************** TABLE LEVEL FUNCTIONS ****************
 ******************************************************/
static JET_ERR JET_API TraceAddColumn(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szColumnName,
	const JET_COLUMNDEF_OLD	*pcolumndef, const void *pvDefault, ULONG cbDefault,
	JET_COLUMNID *pcolumnid )
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamAddColumn(%d, %d, %s, %p, %p, %d, %p)\n", sesid, pTrace->handle, szColumnName,
		pcolumndef, pvDefault, cbDefault, pcolumnid);
	if (pcolumndef) DumpColumnInfo(pcolumndef, sizeof(JET_COLUMNDEF_OLD), JET_ColInfo);
	ret = pTrace->pTblFN->IsamAddColumn(sesid, pTrace->handle, szColumnName,
		pcolumndef, pvDefault, cbDefault, pcolumnid);
	TraceLog (L"\t = %d [*pcolumnid = %d]\n", ret, pcolumnid?*pcolumnid:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCloseTable(JET_SESID sesid, JET_TABLEID tableid)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamCloseTable(%d, %d)\n", sesid, pTrace->handle);
	ret = pTrace->pTblFN->IsamCloseTable(sesid, pTrace->handle);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceComputeStats(JET_SESID sesid, JET_TABLEID tableid)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamComputeStats(%d, %d)\n", sesid, pTrace->handle);
	ret = pTrace->pTblFN->IsamComputeStats(sesid, pTrace->handle);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCopyBookmarks(JET_SESID sesid, JET_TABLEID tableid, DWORD unk3, 
	DWORD unk4, DWORD unk5, DWORD *unk6, DWORD *unk7)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamCopyBookmarks(%d, %d, %d, %d, %d, %p, %p)\n", sesid, pTrace->handle,
		unk3, unk4, unk5, unk6, unk7);
	ret = pTrace->pTblFN->IsamCopyBookmarks(sesid, pTrace->handle, unk3, unk4, unk5, unk6, unk7);
	TraceLog (L"\t = %d  [*unk6 = %d, *unk7 = %d]\n", ret, unk6?*unk6:0, unk7?*unk7:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCreateIndex(JET_SESID sesid,	JET_TABLEID	tableid, LPCWSTR szIndexName,
	JET_GRBIT grbit, LPCWSTR szKey, ULONG cbKey, ULONG lDensity)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamCreateIndex(%d, %d, %s, %d, %s, %d, %d)\n", sesid, pTrace->handle,
		szIndexName, grbit, szKey, cbKey, lDensity);
	ret = pTrace->pTblFN->IsamCreateIndex(sesid, pTrace->handle, szIndexName, grbit, szKey, cbKey, lDensity);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCreateReference(JET_SESID sesid, JET_DBID dbid,
	LPCWSTR szReferenceName, LPCWSTR szColumnName, const void *pvData, const ULONG cbData,
	const JET_GRBIT	grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamCreateReference(%d, %d, %s, %s, %p, %d, %d)\n", sesid, pTrace->handle,
		szReferenceName, szColumnName, pvData, cbData, grbit);
	if (pvData && cbData) HexDump((PVOID)pvData, cbData);
	ret = pTrace->pTblFN->IsamCreateReference(sesid, pTrace->handle,
		szReferenceName, szColumnName, pvData, cbData, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceDelete(JET_SESID sesid, JET_TABLEID tableid)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamDelete(%d, %d)\n", sesid, pTrace->handle);
	ret = pTrace->pTblFN->IsamDelete(sesid, pTrace->handle);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceDeleteColumn(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szColumnName)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamDeleteColumn(%d, %d, %s)\n", sesid, pTrace->handle, szColumnName);
	ret = pTrace->pTblFN->IsamDeleteColumn(sesid, pTrace->handle, szColumnName);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceDeleteIndex(JET_SESID sesid, JET_TABLEID	tableid,  LPCWSTR szIndexName)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamDeleteIndex(%d, %d, %s)\n", sesid, pTrace->handle, szIndexName);
	ret = pTrace->pTblFN->IsamDeleteIndex(sesid, pTrace->handle, szIndexName);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceDeleteReference(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szReferenceName)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamDeleteReference(%d, %d, %s)\n", sesid, pTrace->handle, szReferenceName);
	ret = pTrace->pTblFN->IsamDeleteReference(sesid, pTrace->handle, szReferenceName);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceDupCursor(JET_SESID sesid, JET_TABLEID tableid, JET_TABLEID *ptableid,
	JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamDupCursor(%d, %d, %p, %d)\n", sesid, pTrace->handle, ptableid, grbit);
	ret = pTrace->pTblFN->IsamDupCursor(sesid, pTrace->handle, ptableid, grbit);
	TraceLog (L"\t = %d [*ptableid = %d]\n", ret, ptableid?*ptableid:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetBookmark(JET_SESID sesid, JET_TABLEID tableid, PVOID pvBookmark,
	ULONG cbMax, PULONG pcbActual)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetBookmark(%d, %d, %p, %d, %p)\n", sesid, pTrace->handle, pvBookmark, cbMax, pcbActual);
	ret = pTrace->pTblFN->IsamGetBookmark(sesid, pTrace->handle, pvBookmark, cbMax, pcbActual);
	TraceLog (L"\t = %d [*pcbActual = %d]\n", ret, pcbActual?*pcbActual:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetChecksum(JET_SESID sesid, JET_TABLEID tableid, PDWORD pdwChecksum)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetChecksum(%d, %d, %p)\n", sesid, pTrace->handle, pdwChecksum);
	ret = pTrace->pTblFN->IsamGetChecksum(sesid, pTrace->handle, pdwChecksum);
	TraceLog (L"\t = %d [*pdwChecksum = %d]\n", ret, pdwChecksum?*pdwChecksum:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid, LPWSTR szIndexName,
	ULONG cchIndexName )
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetCurrentIndex(%d, %d, %p, %d)\n", sesid, pTrace->handle, 
		szIndexName, cchIndexName);
	ret = pTrace->pTblFN->IsamGetCurrentIndex(sesid, pTrace->handle, szIndexName, cchIndexName);
	TraceLog (L"\t = %d [szIndexName = %s]\n", ret, szIndexName?szIndexName:NULL);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetCursorInfo(JET_SESID sesid, JET_TABLEID tableid, PVOID pvResult,
	ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetCursorInfo(%d, %d, %p, %d, %d)\n", sesid, pTrace->handle, 
		pvResult, cbMax, InfoLevel);
	ret = pTrace->pTblFN->IsamGetCursorInfo(sesid, pTrace->handle, pvResult, cbMax, InfoLevel);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetRecordPosition(JET_SESID sesid, JET_TABLEID tableid, 
	JET_RECPOS *precpos, ULONG cbRecpos)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetRecordPosition(%d, %d, %p, %d)\n", sesid, pTrace->handle, 
		precpos, cbRecpos);
	ret = pTrace->pTblFN->IsamGetRecordPosition(sesid, pTrace->handle, precpos, cbRecpos);
	TraceLog (L"\t = %d", ret);
	if (ret == JET_errSuccess && precpos && cbRecpos>=sizeof(JET_RECPOS)) DumpRecPos(precpos);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetTableColumnInfo(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szColumnName, PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetTableColumnInfo(%d, %d, %s, %p, %d, %d)\n", sesid, pTrace->handle, szColumnName,
		pvResult, cbMax, InfoLevel);
	ret = pTrace->pTblFN->IsamGetTableColumnInfo(sesid, pTrace->handle, szColumnName, pvResult, cbMax, InfoLevel);
	TraceLog (L"\t = %d", ret);
	if (ret == JET_errSuccess && pvResult && cbMax)
		DumpColumnInfo(pvResult, cbMax, InfoLevel);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetTableIndexInfo(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName, PVOID pvResult, ULONG cbResult, ULONG InfoLevel)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetTableIndexInfo(%d, %d, %s, %p, %d, %d)\n", sesid, pTrace->handle, 
		szIndexName, pvResult, cbResult, InfoLevel);
	ret = pTrace->pTblFN->IsamGetTableIndexInfo(sesid, pTrace->handle, szIndexName, pvResult, 
		cbResult, InfoLevel);
	TraceLog (L"\t = %d", ret);
	if (ret == JET_errSuccess && pvResult && cbResult)
		DumpIndexInfo (pvResult, cbResult, InfoLevel);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------
static JET_ERR JET_API TraceGetTableInfo(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetTableInfo(%d, %d, %p, %d, %d)\n", sesid, pTrace->handle, 
		pvResult, cbMax, InfoLevel);
	ret = pTrace->pTblFN->IsamGetTableInfo(sesid, pTrace->handle, pvResult, cbMax, InfoLevel);
	TraceLog (L"\t = %d", ret);
	if (ret == JET_errSuccess && pvResult && cbMax)
	{
		switch (InfoLevel)
		{
		case JET_TblInfo:
			if (cbMax>=sizeof(JET_OBJECTINFO)) DumpObjectInfo((JET_OBJECTINFO*)pvResult);
			break;
		case JET_TblInfoName:
		case JET_TblInfoTemplateTableName:
			TraceLog (L" [pvResult = %s]", (WCHAR*)pvResult);
			break;
		case JET_TblInfoDbid:
			TraceLog (L" [pvResult = %d %d]", *(JET_DBID*)pvResult, *((JET_DBID*)pvResult+1));
			break;
		case JET_TblInfoResetOLC:
		case JET_TblInfoSpaceAvailable:
		case JET_TblInfoSpaceOwned:
			TraceLog (L" [*pvResult = %d]", *(ULONG*)pvResult);
			break;
		case JET_TblInfoSpaceAlloc:
		case JET_TblInfoSpaceUsage:
			TraceLog (L" [pvResult = %d %d]", *(ULONG*)pvResult, *((ULONG*)pvResult+1));
			break;
		}
	}
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetTableReferenceInfo(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName, PVOID pvResult, ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGetTableReferenceInfo(%d, %d, %s, %p, %d, %d)\n", sesid, pTrace->handle, 
		szIndexName, pvResult, cbMax, InfoLevel);
	ret = pTrace->pTblFN->IsamGetTableReferenceInfo(sesid, pTrace->handle, szIndexName, pvResult, 
		cbMax, InfoLevel);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGotoBookmark(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGotoBookmark(%d, %d, %p, %d)\n", sesid, pTrace->handle, 
		pvBookmark, cbBookmark);
	if (pvBookmark  && cbBookmark) HexDump(pvBookmark, cbBookmark);
	ret = pTrace->pTblFN->IsamGotoBookmark(sesid, pTrace->handle, pvBookmark, cbBookmark);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGotoPosition(JET_SESID sesid, JET_TABLEID tableid, JET_RECPOS *precpos)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamGotoPosition(%d, %d, %p)\n", sesid, pTrace->handle, precpos);
	if (precpos) DumpRecPos(precpos);
	ret = pTrace->pTblFN->IsamGotoPosition(sesid, pTrace->handle, precpos);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceIdle(JET_SESID sesid, JET_TABLEID tableid)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamIdle(%d, %d)\n", sesid, tableid);
	ret = pTrace->pTblFN->IsamIdle(sesid, tableid);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceMakeKey(JET_SESID sesid, JET_TABLEID tableid, const void *pvData,
	ULONG cbData, JET_GRBIT	grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamMakeKey(%d, %d, %p, %d, %d)\n", sesid, pTrace->handle, pvData,
		cbData, grbit);
	ret = pTrace->pTblFN->IsamMakeKey(sesid, pTrace->handle, pvData, cbData, grbit);
	if (ret == JET_errSuccess && pvData && cbData) HexDump((PVOID)pvData, cbData);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceMove(JET_SESID sesid, JET_TABLEID tableid, long cRow,
	JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamMove(%d, %d, %d, %d)\n", sesid, pTrace->handle, 
		cRow, grbit);
	ret = pTrace->pTblFN->IsamMove(sesid, pTrace->handle, cRow, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceNotifyUpdateUfn(JET_SESID sesid, JET_TABLEID tableid)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamNotifyUpdateUfn(%d, %d)\n", sesid, pTrace->handle);
	ret = pTrace->pTblFN->IsamNotifyUpdateUfn(sesid, pTrace->handle);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceNotifyRollback(JET_SESID sesid, JET_TABLEID tableid, int unk3)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamNotifyRollback(%d, %d, %d)\n", sesid, pTrace->handle, unk3);
	ret = pTrace->pTblFN->IsamNotifyRollback(sesid, pTrace->handle, unk3);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceNotifyUnknown(JET_SESID sesid, JET_TABLEID tableid, int unk3)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamNotifyUnknown(%d, %d, %d)\n", sesid, pTrace->handle, unk3);
	ret = pTrace->pTblFN->IsamNotifyUnknown(sesid, pTrace->handle, unk3);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceNotifyUnknown2(JET_SESID sesid, JET_TABLEID tableid)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamNotifyUnknown2(%d, %d)\n", sesid, pTrace->handle);
	ret = pTrace->pTblFN->IsamNotifyUnknown2(sesid, pTrace->handle);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TracePrepareUpdate(JET_SESID sesid, JET_TABLEID tableid, ULONG prep)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamPrepareUpdate(%d, %d, %d)\n", sesid, pTrace->handle, prep);
	ret = pTrace->pTblFN->IsamPrepareUpdate(sesid, pTrace->handle, prep);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceRenameColumn(JET_SESID  sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamRenameColumn(%d, %d, %s, %s)\n", sesid, pTrace->handle, 
		szName, szNameNew);
	ret = pTrace->pTblFN->IsamRenameColumn(sesid, pTrace->handle, szName, szNameNew);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceRenameIndex(JET_SESID  sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamRenameIndex(%d, %d, %s, %s)\n", sesid, pTrace->handle, 
		szName, szNameNew);
	ret = pTrace->pTblFN->IsamRenameIndex(sesid, pTrace->handle, szName, szNameNew);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceRenameReference(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szName, LPCWSTR szNameNew)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamRenameReference(%d, %d, %s, %s)\n", sesid, pTrace->handle, 
		szName, szNameNew);
	ret = pTrace->pTblFN->IsamRenameReference(sesid, pTrace->handle, szName, szNameNew);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceRetrieveColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, PVOID pvData, ULONG cbData, ULONG *pcbActual,
	JET_GRBIT grbit, JET_RETINFO *pretinfo)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamRetrieveColumn(%d, (tabid: %d) %d, %d, %p, %d, %p, %d, %p)\n", 
		sesid, tableid, pTrace->handle, columnid, pvData, cbData, pcbActual, grbit, pretinfo);
	ret = pTrace->pTblFN->IsamRetrieveColumn(sesid, pTrace->handle, columnid, pvData, 
		cbData, pcbActual, grbit, pretinfo);
	TraceLog (L"\t = %d ", ret);
	if (pcbActual) TraceLog (L"[*pcbActual = %d]\n", *pcbActual);
	if (pretinfo) DumpRetinfo(pretinfo);
	if (pvData)
	{
		if (pcbActual) HexDump(pvData, *pcbActual);
		else HexDump(pvData, cbData);
	}
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceRetrieveKey(JET_SESID sesid, JET_TABLEID tableid, 
	PVOID pvData, ULONG cbMax, ULONG *pcbActual, JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamRetrieveKey(%d, %d, %p, %d, %p, %d)\n", 
		sesid, pTrace->handle, pvData, cbMax, pcbActual, grbit);
	ret = pTrace->pTblFN->IsamRetrieveKey(sesid, pTrace->handle, 
		pvData, cbMax, pcbActual, grbit);
	TraceLog (L"\t = %d ", ret);
	if (pcbActual) TraceLog (L"[*pcbActual = %d]\n", *pcbActual);
	if (pvData && cbMax) HexDump(pvData, cbMax);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceSeek(JET_SESID sesid, JET_TABLEID tableid, JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamSeek(%d, %d, %d)\n", sesid, pTrace->handle, grbit);
	ret = pTrace->pTblFN->IsamSeek(sesid, pTrace->handle, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceSetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szIndexName)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamSetCurrentIndex(%d, %d, %s)\n", sesid, pTrace->handle, szIndexName);
	ret = pTrace->pTblFN->IsamSetCurrentIndex(sesid, pTrace->handle, szIndexName);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceSetColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, const void *pvData, ULONG cbData,
	JET_GRBIT grbit, JET_SETINFO *psetinfo)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamSetColumn(%d, %d, %d, %p, %d, %d, %p)\n", sesid, 
		pTrace->handle, columnid, pvData, cbData, grbit, psetinfo);
	if (pvData && cbData) HexDump((PVOID)pvData, cbData);
	if (psetinfo) DumpSetInfo(psetinfo);
	ret = pTrace->pTblFN->IsamSetColumn(sesid, pTrace->handle, columnid, 
		pvData, cbData, grbit, psetinfo);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceSetIndexRange(JET_SESID sesid, JET_TABLEID tableid,
	JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamSetIndexRange(%d, %d, %d)\n", sesid, pTrace->handle, grbit);
	ret = pTrace->pTblFN->IsamSetIndexRange(sesid, pTrace->handle, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceUpdate(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark,	ULONG *pcbActual)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamUpdate(%d, %d, %p, %d, %p)\n", sesid, pTrace->handle, 
		pvBookmark, cbBookmark, pcbActual);
	ret = pTrace->pTblFN->IsamUpdate(sesid, pTrace->handle, pvBookmark, 
		cbBookmark, pcbActual);
	TraceLog (L"\t = %d ", ret);
	if (pcbActual) TraceLog (L"[*pcbActual = %d]", *pcbActual);
	TraceLog (L"\n");
	if (pvBookmark && cbBookmark) HexDump(pvBookmark, cbBookmark);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceEmptyTable(JET_SESID sesid, JET_TABLEID tableid,
	PVOID pvBookmark, ULONG cbBookmark,	ULONG *pcbActual)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamEmptyTable(%d, %d, %p, %d, %p)\n", sesid, pTrace->handle, 
		pvBookmark, cbBookmark, pcbActual);
	ret = pTrace->pTblFN->IsamEmptyTable(sesid, pTrace->handle, pvBookmark, 
		cbBookmark, pcbActual);
	TraceLog (L"\t = %d ", ret);
	if (pcbActual) TraceLog (L"[*pcbActual = %d]", *pcbActual);
	TraceLog (L"\n");
	if (pvBookmark && cbBookmark) HexDump(pvBookmark, cbBookmark);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCollectRecids(JET_SESID sesid, JET_TABLEID tableid,
	RECID_CALLBACK callback, ULONG cbParam1, ULONG cbParam2, LPCWSTR unk6,
	int cbunk6, int unk8, int unk9, int unk10, int unk11)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamCollectRecids(%d, %d, %p, %d, %d, %s, %d, %d, %d, %d, %d)\n", 
		sesid, pTrace->handle, callback, cbParam1, cbParam2, unk6, cbunk6, unk8,
		unk9, unk10, unk11);
	ret = pTrace->pTblFN->IsamCollectRecids(sesid, pTrace->handle, callback, cbParam1, 
		cbParam2, unk6, cbunk6, unk8, unk9, unk10, unk11);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceOpenILockBytes(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, PVOID pvLockBytes, JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamOpenILockBytes(%d, %d, %d, %p, %d)\n", 
		sesid, pTrace->handle, columnid, pvLockBytes, grbit);
	ret = pTrace->pTblFN->IsamOpenILockBytes(sesid, pTrace->handle, columnid, 
		pvLockBytes, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceDeleteColumn2(JET_SESID sesid, JET_TABLEID tableid,
		LPCWSTR szColumnName, const JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamDeleteColumn2(%d, %d, %s, %d)\n", sesid, 
		pTrace->handle, szColumnName, grbit);
	ret = pTrace->pTblFN->IsamDeleteColumn2(sesid, pTrace->handle, 
		szColumnName, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCopyRecords(JET_SESID sesid, JET_TABLEID tableid,
		int unk3, int unk4, int unk5, int unk6, int unk7, int unk8)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamCopyRecords(%d, %d, %d, %d, %d, %d, %d, %d)\n", 
		sesid, pTrace->handle, unk3, unk4, unk5, unk6, unk7, unk8);
	ret = pTrace->pTblFN->IsamCopyRecords(sesid, pTrace->handle, unk3, 
		unk4, unk5, unk6, unk7, unk8);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------


static JET_ERR JET_API TraceModifyColumn(JET_SESID sesid, JET_TABLEID tableid,
	LPCWSTR szColumnName, int unk4, int unk5)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamModifyColumn(%d, %d, %s, %d, %d)\n", 
		sesid, pTrace->handle, szColumnName, unk4, unk5);
	ret = pTrace->pTblFN->IsamModifyColumn(sesid, pTrace->handle, szColumnName, 
		unk4, unk5);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceReadAhead(JET_SESID sesid, JET_TABLEID tableid,
	PVOID unk3, int unk4, int unk5)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, tableid)))
		return JET_errInvalidTableId;
	TraceLog (L"IsamReadAhead(%d, %d, %d, %d, %d)\n", 
		sesid, pTrace->handle, unk3, unk4, unk5);
	ret = pTrace->pTblFN->IsamReadAhead(sesid, pTrace->handle, unk3, 
		unk4, unk5);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_FNDEFTBL m_TraceTblVtbl = 
{
	sizeof(JET_FNDEFTBL),
	L"X\0"DRIVER_DESC,
	TraceAddColumn,
	TraceCloseTable,
	TraceComputeStats,
	TraceCopyBookmarks,
	TraceCreateIndex,
	TraceCreateReference,
	TraceDelete,
	TraceDeleteColumn,
	TraceDeleteIndex,
	TraceDeleteReference,
	TraceDupCursor,
	TraceGetBookmark,
	TraceGetChecksum,
	TraceGetCurrentIndex,
	TraceGetCursorInfo,
	TraceGetRecordPosition,
	TraceGetTableColumnInfo,
	TraceGetTableIndexInfo,
	TraceGetTableInfo,
	TraceGetTableReferenceInfo,
	TraceGotoBookmark,
	TraceGotoPosition,
	TraceIdle,
	TraceMakeKey,
	TraceMove,
	TraceNotifyUpdateUfn,
	TraceNotifyRollback,
	TraceNotifyUnknown,
	TraceNotifyUnknown2,
	TracePrepareUpdate,
	TraceRenameColumn,
	TraceRenameIndex,
	TraceRenameReference,
	TraceRetrieveColumn,
	TraceRetrieveKey,
	TraceSeek,
	TraceSetCurrentIndex,
	TraceSetColumn,
	TraceSetIndexRange,
	TraceUpdate,
	TraceEmptyTable,
	TraceCollectRecids,
	TraceOpenILockBytes,
	NULL,
	NULL,
	NULL,
	NULL,
	TraceModifyColumn,
	TraceReadAhead
};




/*****************************************************
 ************** DATABASE LEVEL FUNCTIONS **************
 ******************************************************/

static JET_ERR JET_API TraceCapability(JET_SESID sesid, JET_DBID dbid, DWORD unk1, DWORD unk2, DWORD *unk3)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamCapability(%d, %d, %d, %d, %p)\n", sesid, pTrace->handle, unk1, unk2, unk3);
	ret = pTrace->pDBFN->IsamCapability(sesid, pTrace->handle, unk1, unk2, unk3);
	TraceLog (L"\t = %d [*unk3 = %d]\n", ret, unk3?*unk3:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCloseDatabase(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamCloseDatabase(%d, %d, %d)\n", sesid, pTrace->handle, grbit);
	ret = pTrace->pDBFN->IsamCloseDatabase(sesid, pTrace->handle, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceConnectionControl(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamConnectionControl(%d, %d, %d)\n", sesid, pTrace->handle, grbit);
	ret = pTrace->pDBFN->IsamConnectionControl(sesid, pTrace->handle, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCreateObject(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit, LPWSTR unk1, short unk2)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamCreateObject(%d, %d, %d, %s, %d)\n", sesid, pTrace->handle, grbit, unk1, unk2);
	ret = pTrace->pDBFN->IsamCreateObject(sesid, pTrace->handle, grbit, unk1, unk2);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceCreateTable(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName, ULONG lPages, 
						ULONG lDensity, JET_TABLEID	*ptableid)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamCreateTable(%d, %d, %s, %d, %d, %p)\n", sesid, pTrace->handle, szTableName, lPages, lDensity, ptableid);
	ret = pTrace->pDBFN->IsamCreateTable(sesid, pTrace->handle, szTableName, lPages, lDensity, ptableid);
	TraceLog (L"\t = %d (*ptableid = %d)\n", ret, ptableid?*ptableid:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceDeleteObject(JET_SESID sesid, JET_DBID dbid, LPCWSTR szObjectName)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamDeleteObject(%d, %d, %s)\n", sesid, pTrace->handle, szObjectName);
	ret = pTrace->pDBFN->IsamDeleteObject(sesid, pTrace->handle, szObjectName);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceDeleteTable(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamDeleteTable(%d, %d, %s)\n", sesid, pTrace->handle, szTableName);
	ret = pTrace->pDBFN->IsamDeleteTable(sesid, pTrace->handle, szTableName);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceUnknownFunct(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamUnknownFunct(%d, %d, %d)\n", sesid, pTrace->handle, grbit);
	ret = pTrace->pDBFN->IsamUnknownFunct(sesid, pTrace->handle, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetColumnInfo(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName,
						LPCWSTR szColumnName, OBJINFO *ColumnInfo, ULONG InfoLevel)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamGetColumnInfo(%d, %d, %s, %s, %p (%d, %p), %d)\n", sesid, pTrace->handle, szTableName,
		szColumnName, ColumnInfo, ColumnInfo?ColumnInfo->cbResult:0, ColumnInfo?ColumnInfo->pResult:NULL, InfoLevel);
	ret = pTrace->pDBFN->IsamGetColumnInfo(sesid, pTrace->handle, szTableName, szColumnName, ColumnInfo, InfoLevel);
	TraceLog (L"\t = %d", ret);
	if (ret == JET_errSuccess && ColumnInfo && ColumnInfo->pResult)
		DumpColumnInfo(ColumnInfo->pResult, ColumnInfo->cbResult, InfoLevel);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetDatabaseInfo(JET_SESID sesid, JET_HANDLE dbid, PVOID pvResult,
						ULONG cbMax, ULONG InfoLevel)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamGetDatabaseInfo(%d, %d, %p, %d, %d)\n", sesid, pTrace->handle, pvResult, cbMax, InfoLevel);
	ret = pTrace->pDBFN->IsamGetDatabaseInfo(sesid, pTrace->handle, pvResult, cbMax, InfoLevel);
	TraceLog (L"\t = %d", ret);
	if (ret == JET_errSuccess && pvResult && cbMax)
	{
		switch (InfoLevel)
		{
		case JET_DbInfoFilename:
		case JET_DbInfoConnect:
			TraceLog (L" [*pvResult = %s]", (WCHAR*)pvResult);
			break;
		case JET_DbInfoCountry:
		case JET_DbInfoCollate:
		case JET_DbInfoTransactions:
		case 18:
		case JET_DbInfoVersion:
		case JET_DbInfoLCID:
		case JET_DbInfoCp:
		case JET_DbInfoOptions:
			if (cbMax>=sizeof(ULONG))
				TraceLog (L" [*pvResult = %d]", *(ULONG*)pvResult);
			break;
		case JET_DbInfoIsam:
			if (cbMax>=(sizeof(ULONG)*2))
				TraceLog (L" [*pvResult = %d %d]", *(ULONG*)pvResult, *(((ULONG*)pvResult)+1));
			break;
		}
	}
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetIndexInfo(JET_SESID sesid, JET_HANDLE dbid, LPCWSTR szTableName,
						LPCWSTR szIndexName, OBJINFO *IndexInfo, ULONG InfoLevel )
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamGetIndexInfo(%d, %d, %s, %s, %p (%d, %p), %d)\n", sesid, pTrace->handle, 
		szTableName, szIndexName, IndexInfo, IndexInfo?IndexInfo->cbResult:0, IndexInfo?IndexInfo->pResult:NULL, 
		InfoLevel);
	ret = pTrace->pDBFN->IsamGetIndexInfo(sesid, pTrace->handle, szTableName, szIndexName, IndexInfo, InfoLevel);
	TraceLog (L"\t = %d", ret);
	if (ret == JET_errSuccess && IndexInfo && IndexInfo->cbResult && IndexInfo->pResult)
		DumpIndexInfo (IndexInfo->pResult, IndexInfo->cbResult, InfoLevel);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetObjectInfo(JET_SESID sesid, JET_DBID dbid, JET_OBJTYP	objtyp,
						LPCWSTR szContainerName, LPCWSTR szObjectName, OBJINFO *IndexInfo, 
						ULONG InfoLevel )
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamGetObjectInfo(%d, %d, %d, %s, %s, %p (%d, %p), %d)\n", sesid, pTrace->handle, 
		objtyp, szContainerName, szObjectName, IndexInfo, IndexInfo?IndexInfo->cbResult:0, 
		IndexInfo?IndexInfo->pResult:NULL, InfoLevel);
	ret = pTrace->pDBFN->IsamGetObjectInfo(sesid, pTrace->handle, objtyp, szContainerName, szObjectName, 
		IndexInfo, InfoLevel);
	TraceLog (L"\t = %d", ret);
	if (ret == JET_errSuccess && IndexInfo && IndexInfo->cbResult && IndexInfo->pResult)
	{
		switch (InfoLevel)
		{
		case JET_ObjInfo:
		case JET_ObjInfoListNoStats:
		case JET_ObjInfoMax:
		case JET_ObjInfoNoStats:
			if (IndexInfo->cbResult >= sizeof(JET_OBJECTINFO))
				DumpObjectInfo((JET_OBJECTINFO*)IndexInfo->pResult);
			break;
		}
	}
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetReferenceInfo(JET_SESID sesid, JET_DBID dbid, LPCWSTR szContainerName, 
						LPCWSTR szIndexName, OBJINFO *IndexInfo, JET_OBJTYP	reftyp,
						ULONG InfoLevel )
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamGetReferenceInfo(%d, %d, %s, %s, %p (%d, %p), %d, %d)\n", sesid, pTrace->handle, 
		szContainerName, szIndexName, IndexInfo, IndexInfo?IndexInfo->cbResult:0, 
		IndexInfo?IndexInfo->pResult:NULL, reftyp, InfoLevel);
	ret = pTrace->pDBFN->IsamGetReferenceInfo(sesid, pTrace->handle, szContainerName, szIndexName, 
		IndexInfo, reftyp, InfoLevel);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceOpenTable(JET_SESID sesid, JET_DBID dbid, JET_TABLEID *ptableid,
						LPCWSTR szTableName, JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamOpenTable(%d, %d, %p, %s, %d)\n", sesid, pTrace->handle, 
		ptableid, szTableName, grbit);
	ret = pTrace->pDBFN->IsamOpenTable(sesid, pTrace->handle, ptableid, szTableName, grbit);
	TraceLog (L"\t = %d [*ptableid = %d]\n", ret, ptableid?*ptableid:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceRenameObject(JET_SESID sesid,	JET_DBID dbid, LPCWSTR szTableName, 
						LPCWSTR szName,	LPCWSTR szNameNew)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamRenameObject(%d, %d, %s, %s, %s)\n", sesid, pTrace->handle, 
		szTableName, szName, szNameNew);
	ret = pTrace->pDBFN->IsamRenameObject(sesid, pTrace->handle, szTableName, szName, szNameNew);
	TraceLog (L"\t = %d\n", ret);
	return ret;

}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceRenameTable(JET_SESID sesid, JET_DBID dbid, LPCWSTR szName, LPCWSTR szNameNew)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamRenameTable(%d, %d, %s, %s)\n", sesid, pTrace->handle, szName, szNameNew);
	ret = pTrace->pDBFN->IsamRenameTable(sesid, pTrace->handle, szName, szNameNew);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceGetObjidFromName(JET_SESID sesid, JET_DBID dbid, LPCWSTR szContainerName, 
						LPCWSTR szObjectName, JET_OBJID *objid)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamGetObjidFromName(%d, %d, %s, %s, %p)\n", sesid, pTrace->handle, 
		szContainerName, szObjectName, objid);
	ret = pTrace->pDBFN->IsamGetObjidFromName(sesid, pTrace->handle, 
		szContainerName, szObjectName, objid);
	TraceLog (L"\t = %d [*objid = %d]\n", ret, objid?*objid:0);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceChangeDbPasswordEx(JET_SESID sesid, JET_DBID dbid, LPCWSTR szNewPassword, 
						LPCWSTR szOldPassword, JET_GRBIT grbit)
{
	ISAM_Trace *pTrace;
	JET_ERR ret;

	if (!(pTrace = ISAMDBFindTrace(sesid, dbid)))
		return JET_errInvalidDatabaseId;
	TraceLog (L"IsamChangeDbPasswordEx(%d, %d, %s, %s, %d)\n", sesid, 
		pTrace->handle, szNewPassword, szOldPassword, grbit);
	ret = pTrace->pDBFN->IsamChangeDbPasswordEx(sesid, pTrace->handle, szNewPassword, szOldPassword, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_FNDEFDB m_TraceDrvVtbl = 
{
	sizeof(JET_FNDEFDB),
	L"X\0"DRIVER_DESC,
	TraceCapability,
	TraceCloseDatabase,
	TraceConnectionControl,
	TraceCreateObject,
	TraceCreateTable,
	TraceDeleteObject,
	TraceDeleteTable,
	TraceUnknownFunct,
	TraceGetColumnInfo,
	TraceGetDatabaseInfo,
	TraceGetIndexInfo,
	TraceGetObjectInfo,
	TraceGetReferenceInfo,
	TraceOpenTable,
	TraceRenameObject,
	TraceRenameTable,
	TraceGetObjidFromName,
	TraceChangeDbPasswordEx
};


/******************************************************
 ****************** CALLOUT FUNCTIONS *****************
 ******************************************************/

static JET_TABLEID JET_API TraceTableidFromVtid(JET_TABLEID vtid, JET_FNDEFTBL *pfndef)
{
	JET_TABLEID ret;
	
	TraceLog (L"\t->TableidFromVtid(%d, %p)\n", vtid, pfndef);
	ret = ISAMDBFindTraceTableidFromVtid(vtid, pfndef);
	//ret = g_pCaller->TableidFromVtid(vtid, pfndef);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrDispGetBookmark2(JET_SESID sesid, JET_TABLEID tableid, 
	PVOID pvBookmark, ULONG cbMax, PULONG pcbActual)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrDispGetBookmark2(%d, %d, %p, %d, %p)\n", 
		sesid, tableid, pvBookmark, cbMax, pcbActual);
	ret = g_pCaller->ErrDispGetBookmark2(sesid, tableid, pvBookmark, cbMax, pcbActual);
	TraceLog (L"\t = %d", ret);
	if (pcbActual) TraceLog( L" [*pcbActual = %d]", *pcbActual);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_DBID JET_API TraceDbidOfVdbid(JET_SESID sesid, JET_DBID vdbid)
{
	JET_DBID ret;

	TraceLog (L"\t->DbidOfVdbid(%d, %d)\n", sesid, vdbid);
	ret = g_pCaller->DbidOfVdbid(sesid, vdbid);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_PFNSTATUS JET_API TraceUtilGetpfnStatusOfSesid(JET_SESID sesid, JET_PFNSTATUS *pStatus)
{
	JET_PFNSTATUS ret;

	TraceLog (L"\t->UtilGetpfnStatusOfSesid(%d, %p)\n", sesid, pStatus);
	ret = g_pCaller->UtilGetpfnStatusOfSesid(sesid, pStatus);
	TraceLog (L"\t = %p\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrDispCloseTable(JET_SESID sesid, JET_TABLEID tableid)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrDispCloseTable(%d, %d)\n", sesid, tableid);
	ret = g_pCaller->ErrDispCloseTable(sesid, tableid);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrDispPrepareUpdate2(JET_SESID sesid, JET_TABLEID tableid, ULONG prep)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrDispPrepareUpdate2(%d, %d, %d)\n", sesid, tableid, prep);
	ret = g_pCaller->ErrDispPrepareUpdate2(sesid, tableid, prep);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrDispRetrieveColumn2(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, PVOID pvData, ULONG cbData, ULONG *pcbActual,
	JET_GRBIT grbit, JET_RETINFO *pretinfo)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrDispRetrieveColumn2(%d, %d, %d, %p, %d, %p, %d, %p)\n", 
		sesid, tableid, columnid, pvData, cbData, pcbActual, grbit, pretinfo);
	ret = g_pCaller->ErrDispRetrieveColumn2(sesid, tableid, columnid, pvData, 
		cbData, pcbActual, grbit, pretinfo);
	TraceLog (L"\t = %d", ret);
	if (pcbActual) TraceLog (L" [*pcbActual = %d]", *pcbActual);
	if (pretinfo) DumpRetinfo(pretinfo);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrGetVtidTableid(JET_SESID sesid, JET_TABLEID tableid, DWORD *pdwVtid)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrGetVtidTableid(%d, %d, %p)\n", sesid, tableid, pdwVtid);
	ret = g_pCaller->ErrGetVtidTableid(sesid, tableid, pdwVtid);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static LPWSTR JET_API TraceUtilGetNameOfSesid(JET_SESID sesid, LPWSTR lpNameOut)
{
	LPWSTR ret;

	TraceLog( L"\t->UtilGetNameOfSesid(%d, %p)\n", sesid, lpNameOut);
	ret = g_pCaller->UtilGetNameOfSesid(sesid, lpNameOut);
	TraceLog (L"\t = %s\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrAllocateTableidForVsesid(JET_SESID sesid, JET_TABLEID *ptableid, JET_HANDLE vtdid, JET_FNDEFTBL *pVtbl)
{
	JET_ERR ret;
	ISAM_Session *pSession;
	ISAM_Trace *pTrace;

	TraceLog( L"\t->ErrAllocateTableidForVsesid(%d, %p, %d, %p)\n", sesid, ptableid, vtdid, pVtbl);
	if (!(pSession = ISAMDBFindSession(sesid)))
		return JET_errInvalidSesid;
	if (!(pTrace = ISAMDBAddTrace (pSession, 2, vtdid, pVtbl)))
		return JET_errOutOfMemory;
	ret = g_pCaller->ErrAllocateTableidForVsesid(sesid, ptableid, pTrace->traceid, &m_TraceTblVtbl);
	if (ret == JET_errSuccess)
	{
		if (ptableid) pTrace->tableid = *ptableid;
	}
	else
	{
		ISAMDBDeleteTrace(pSession, pTrace);
	}
	TraceLog (L"\t = %d", ret);
	if (ptableid) TraceLog( L" [*ptableid = %d]", *ptableid);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrAllocateDbid(JET_DBID *pdbid, JET_HANDLE userid, JET_FNDEFDB *pVtbl)
{
	JET_ERR ret;
	ISAM_DBUser *pUser;
	ISAM_Trace *pTrace;

	TraceLog( L"\t->ErrAllocateDbid(%p, %d, %p)\n", pdbid, userid, pVtbl);
	if (!(pUser = ISAMDBFindDatabaseUserId(userid)))
		return JET_errInvalidDatabaseId;
	if (!(pTrace = ISAMDBAddTrace (pUser->pSession, 1, userid, pVtbl)))
		return JET_errOutOfMemory;
	ret = g_pCaller->ErrAllocateDbid(pdbid, pTrace->traceid, &m_TraceDrvVtbl);
	if (ret == JET_errSuccess)
	{
		if (pdbid) pTrace->tableid = *pdbid;
	}
	else
	{
		ISAMDBDeleteTrace(pUser->pSession, pTrace);
	}
	TraceLog (L"\t = %d", ret);
	if (pdbid) TraceLog( L" [*pdbid = %d]", *pdbid);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static LPCWSTR JET_API TraceUtilSetErrorInfoReal(JET_SESID sesid, LPCWSTR lpError1, LPCWSTR lpError2, LPCWSTR lpError3, 
	JET_ERR err1, JET_ERR err2, JET_ERR err3, DWORD unk8)
{
	LPCWSTR ret;

	TraceLog( L"\t->UtilSetErrorInfoReal(%d, %s, %s, %s, %d, %d, %d, %d)\n", sesid, lpError1,
		lpError2, lpError3, err1, err2, err3, unk8);
	ret = g_pCaller->UtilSetErrorInfoReal(sesid, lpError1, lpError2, lpError3, err1, err2, err3, unk8);
	TraceLog (L"\t = %s\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrUpdateTableid(JET_TABLEID tableid, JET_HANDLE vtdid, JET_FNDEFTBL *pfndef)
{
	JET_ERR ret;
	ISAM_Trace *pTrace;
	
	TraceLog( L"\t->ErrUpdateTableid(%d, %d, %p)\n", tableid, vtdid, pfndef);
	if (pTrace = ISAMDBFindTraceByTID(2, tableid))
	{
		pTrace->pTblFN = pfndef;
		pTrace->handle = vtdid;
		ret = JET_errSuccess;
	} else ret = g_pCaller->ErrUpdateTableid(tableid, vtdid, pfndef);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrGetPvtfndefTableid(JET_SESID sesid, JET_TABLEID tableid, JET_FNDEFTBL **pfndefout)
{
	JET_ERR ret;
	ISAM_Trace *pTrace;

	TraceLog( L"\t->ErrGetPvtfndefTableid(%d, %d, %p)\n", sesid, tableid, pfndefout);
	if (pTrace = ISAMDBFindTraceByTID(2, tableid))
	{
		*pfndefout = pTrace->pTblFN;
		ret = JET_errSuccess;
	} else ret = JET_errInvalidTableId;
	//ret = g_pCaller->ErrGetPvtfndefTableid(sesid, tableid, pfndefout);
	TraceLog (L"\t = %d", ret);
	if (pfndefout) TraceLog (L" [*pfndefout = %p]", *pfndefout);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrDispSetColumn2(JET_SESID sesid, JET_TABLEID tableid, JET_COLUMNID	columnid,
	const void	*pvData, ULONG cbData, JET_GRBIT grbit, JET_SETINFO	*psetinfo)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrDispSetColumn2(%d, %d, %d, %p, %d, %d, %p)", sesid, tableid, columnid,
		pvData, cbData, grbit, psetinfo);
	if (psetinfo) TraceLog( L" [*psetinfo = .cbStruct = %d, .ibLongValue = %d, .itagSequence = %d]",
		psetinfo->cbStruct, psetinfo->ibLongValue, psetinfo->itagSequence);
	TraceLog( L"\n");
	ret = g_pCaller->ErrDispSetColumn2(sesid, tableid, columnid, pvData, cbData, grbit, psetinfo);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrDispSeek2(JET_SESID sesid, JET_TABLEID tableid, JET_GRBIT	grbit)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrDispSeek2(%d, %d, %d)\n", sesid, tableid, grbit);
	ret = g_pCaller->ErrDispSeek2(sesid, tableid, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrDispUpdate2( JET_SESID sesid,	JET_TABLEID tableid, PVOID pvBookmark,
	ULONG cbBookmark, PULONG pcbActual)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrDispUpdate2(%d, %d, %p, %d, %p)\n", 
		sesid, tableid, pvBookmark, cbBookmark, pcbActual);
	ret = g_pCaller->ErrDispUpdate2(sesid, tableid, pvBookmark, cbBookmark, pcbActual);
	TraceLog (L"\t = %d", ret);
	if (pcbActual) TraceLog( L" [*pcbActual = %d]", *pcbActual);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_TABLEID JET_API TraceReleaseTableid(JET_TABLEID tableid)
{
	JET_TABLEID ret;

	TraceLog( L"\t->ReleaseTableid(%d)\n", tableid);
	ret = g_pCaller->ReleaseTableid(tableid);
	if (ret == JET_errSuccess)
	{
		ISAM_Trace *pTrace;
		
		if (pTrace = ISAMDBFindTraceByTID(2, tableid))
			ISAMDBDeleteTrace(pTrace->pSession, pTrace);

	}
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_DBID JET_API TraceReleaseDbid(JET_DBID dbid)
{
	JET_DBID ret;

	TraceLog( L"\t->ReleaseDbid(%d)\n", dbid);
	ret = g_pCaller->ReleaseDbid(dbid);
	if (ret == JET_errSuccess)
	{
		ISAM_Trace *pTrace;
		
		if (pTrace = ISAMDBFindTraceByTID(1, dbid))
			ISAMDBDeleteTrace(pTrace->pSession, pTrace);

	}
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrDispMove(JET_SESID sesid,	JET_TABLEID	tableid, long cRow, JET_GRBIT grbit)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrDispMove(%d, %d, %d, %d)\n", sesid, tableid, cRow, grbit);
	ret = g_pCaller->ErrDispMove(sesid, tableid, cRow, grbit);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrGetSystemParameter(JET_INSTANCE instance, JET_SESID sesid, ULONG paramid,
	JET_API_PTR	*plParam, LPCWSTR sz)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrGetSystemParameter(%d, %d, %d, %p, %s)\n", instance, sesid, paramid,
		plParam, sz);
	ret = g_pCaller->ErrGetSystemParameter(instance, sesid, paramid, plParam, sz);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceUtilGetErrorInfo(JET_SESID sesid, LPWSTR *lpErrOut1, LPWSTR *lpErrOut2, 
	LPWSTR *lpErrOut3, BOOL *pbInvalidErrOut1, BOOL *pbInvalidErrOut2, BOOL *pbInvalidErrOut3, 
	DWORD *perr1, DWORD *perr2, DWORD *perr3, JET_ERR *pret)
{
	JET_ERR ret;

	TraceLog( L"\t->UtilGetErrorInfo(%d, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p)\n", 
		sesid, lpErrOut1, lpErrOut2, lpErrOut3, pbInvalidErrOut1, pbInvalidErrOut2,
		pbInvalidErrOut3, perr1, perr2, perr3, pret);
	ret = g_pCaller->UtilGetErrorInfo(sesid, lpErrOut1, lpErrOut2, lpErrOut3, pbInvalidErrOut1, 
		pbInvalidErrOut2, pbInvalidErrOut3, perr1, perr2, perr3, pret);
	TraceLog (L"\t = %d ", ret);
	if (lpErrOut1) TraceLog (L" [*lpErrOut1 = %s]", *lpErrOut1);
	if (lpErrOut2) TraceLog (L" [*lpErrOut2 = %s]", *lpErrOut2);
	if (lpErrOut3) TraceLog (L" [*lpErrOut3 = %s]", *lpErrOut3);
	if (pbInvalidErrOut1) TraceLog (L" [*pbInvalidErrOut1 = %d]", *pbInvalidErrOut1);
	if (pbInvalidErrOut2) TraceLog (L" [*pbInvalidErrOut2 = %d]", *pbInvalidErrOut2);
	if (pbInvalidErrOut3) TraceLog (L" [*pbInvalidErrOut3 = %d]", *pbInvalidErrOut3);
	if (perr1) TraceLog (L" [*perr1 = %d]", *perr1);
	if (perr2) TraceLog (L" [*perr2 = %d]", *perr2);
	if (perr3) TraceLog (L" [*perr3 = %d]", *perr3);
	if (pret) TraceLog (L" [*pret = %d]", *pret);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceClearErrorInfo(JET_SESID sesid)
{
	JET_ERR ret;

	TraceLog( L"\t->ClearErrorInfo(%d)\n", sesid);
	ret = g_pCaller->ClearErrorInfo(sesid);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceUtilUpdateErrorInfo(JET_SESID sesid, UINT uiErrCl1, LPCWSTR pErr1, DWORD cbErr1, 
	UINT uiErrCl2, ULONG ulErr2, JET_ERR err)
{
	JET_ERR ret;

	TraceLog( L"\t->UtilUpdateErrorInfo(%d, %d, %s, %d, %d, %d, %d)\n", 
		sesid, uiErrCl1, pErr1, cbErr1, uiErrCl2, ulErr2, err);
	ret = g_pCaller->UtilUpdateErrorInfo(sesid, uiErrCl1, pErr1, cbErr1, uiErrCl2, ulErr2, err);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrTLVLoadInfo(PVOID pTLV)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrTLVLoadInfo(%p)\n", pTLV);
	ret = g_pCaller->ErrTLVLoadInfo(pTLV);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrTLVZnloadInfo(PVOID pTLV)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrTLVZnloadInfo(%p)\n", pTLV);
	ret = g_pCaller->ErrTLVZnloadInfo(pTLV);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrTLVEvalExpr(PVOID pUnk1)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrTLVEvalExpr(%p)\n", pUnk1);
	ret = g_pCaller->ErrTLVEvalExpr(pUnk1);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static UINT JET_API TraceUtilGetProfileInt(JET_SESID sesid, LPCWSTR lpAppName, LPCWSTR lpKeyName, UINT dwDefault)
{
	UINT ret;

	TraceLog( L"\t->UtilGetProfileInt(%d, %s, %s, %d)\n", sesid, lpAppName, lpKeyName, dwDefault);
	ret = g_pCaller->UtilGetProfileInt(sesid, lpAppName, lpKeyName, dwDefault);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static DWORD JET_API TraceUtilGetProfileString(JET_SESID sesid, LPCWSTR lpAppName, LPCWSTR lpKeyName,
	LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize)
{
	DWORD ret;

	TraceLog( L"\t->UtilGetProfileString(%d, %s, %s, %d, %p, %d)\n", 
		sesid, lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize);
	ret = g_pCaller->UtilGetProfileString(sesid, lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize);
	TraceLog (L"\t = %d", ret);
	if (lpReturnedString) TraceLog (L" [lpReturnedString = %s]", lpReturnedString);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_INSTANCE JET_API TraceInstanceOfItib(JET_SESID sesid)
{
	JET_INSTANCE ret;

	TraceLog( L"\t->InstanceOfItib(%d)\n", sesid);
	ret = g_pCaller->InstanceOfItib(sesid);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_SESID JET_API TraceItibOfInstance(JET_INSTANCE instance)
{
	JET_SESID ret;

	TraceLog( L"\t->ItibOfInstance(%d)\n", instance);
	ret = g_pCaller->ItibOfInstance(instance);
	TraceLog (L"\t = %d\n", ret);
	return ret;
}

// ---------------------------------------------------------------------------

static JET_ERR JET_API TraceErrGetSystemParameterInst(JET_INSTANCE instance, JET_SESID sesid, ULONG paramid,
	JET_API_PTR	*plParam, LPWSTR sz, ULONG cbMax)
{
	JET_ERR ret;

	TraceLog( L"\t->ErrGetSystemParameterInst(%d, %d, %d, %p, %p, %d)\n", 
		instance, sesid, paramid, plParam, sz, cbMax);
	ret = g_pCaller->ErrGetSystemParameterInst(instance, sesid, paramid, plParam, sz, cbMax);
	TraceLog (L"\t = %d", ret);
	if (plParam) TraceLog (L" [*plParam = %08X]", *(DWORD*)plParam);
	if (sz) TraceLog (L" [sz = %s]", sz);
	TraceLog (L"\n");
	return ret;
}

// ---------------------------------------------------------------------------

static JET_CALLIN m_TraceCallerVtbl =
{
	TraceTableidFromVtid,
	TraceErrDispGetBookmark2,
	TraceDbidOfVdbid,
	TraceUtilGetpfnStatusOfSesid,
	TraceErrDispCloseTable,
	TraceErrDispPrepareUpdate2,
	TraceErrDispRetrieveColumn2,
	TraceErrGetVtidTableid,
	TraceUtilGetNameOfSesid,
	TraceErrAllocateTableidForVsesid,
	TraceErrAllocateDbid,
	TraceUtilSetErrorInfoReal,
	TraceErrUpdateTableid,
	TraceErrGetPvtfndefTableid,
	TraceErrDispSetColumn2,
	TraceErrDispSeek2,
	TraceErrDispUpdate2,
	TraceReleaseTableid,
	TraceReleaseDbid,
	TraceErrDispMove,
	TraceErrGetSystemParameter,
	TraceUtilGetErrorInfo,
	TraceClearErrorInfo,
	TraceUtilUpdateErrorInfo,
	NULL,
	TraceErrTLVLoadInfo,
	TraceErrTLVZnloadInfo,
	TraceErrTLVEvalExpr,
	TraceUtilGetProfileInt,
	TraceUtilGetProfileString,
	TraceInstanceOfItib,
	TraceItibOfInstance,
	TraceErrGetSystemParameterInst
};

/******************************************************
 *********************** PUBLIC ***********************
 ******************************************************/
JET_CALLIN *TraceInit(WCHAR *pszTraceFile)
{
	m_fpLog = pszTraceFile?_wfopen(pszTraceFile, L"a"):NULL;
#ifdef _DEBUG
	m_szLogBuf = MemAllocate(m_iBufSize = INITBUF);
#endif
	InitializeCriticalSection(&m_cs);
	return &m_TraceCallerVtbl;
}

void TraceExit()
{
	if (m_fpLog)
	{
		fclose(m_fpLog);
		m_fpLog = NULL;
	}
	if (m_szLogBuf)
	{
		MemFree(m_szLogBuf);
		m_szLogBuf = NULL;
	}
	DeleteCriticalSection(&m_cs);
}

void TraceLog(LPCWSTR pszFormat, ...)
{
	WCHAR *ct, *pNewBuf;
	va_list ap;
	time_t lt;
	int iLen;

	if (!m_szLogBuf) return;
	EnterCriticalSection(&m_cs);
	time(&lt);
	ct=_wctime(&lt);
	ct[wcslen(ct)-1]=0;
	do
	{
		va_start(ap, pszFormat);
		iLen = _vsnwprintf(m_szLogBuf, m_iBufSize, pszFormat, ap); 
		va_end(ap);
		if (iLen == -1)
		{
		  if (!(pNewBuf = (WCHAR*)MemReAllocate(m_szLogBuf, m_iBufSize*2)))
		  {
			  iLen = wcslen(m_szLogBuf);
			  break;
		  }
		  m_szLogBuf = pNewBuf;
		  m_iBufSize*=2;
		}
	} while (iLen == -1);
	if (m_fpLog)
	{
		if (wcschr(m_szLogBuf, '\n')) fwprintf (m_fpLog, L"%s [%08X]   ", ct, GetCurrentThreadId());
		fwprintf (m_fpLog, L"%s", m_szLogBuf);
	}
	OutputDebugStringW(m_szLogBuf);
	fflush (m_fpLog);
	LeaveCriticalSection(&m_cs);
}

