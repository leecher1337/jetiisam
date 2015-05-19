/* Includes most Jet definitions */
#undef WINVER
#define WINVER 0x0501
#include <esent.h>

/**********************************************************************/
/***********************     ERROR CODES     **************************/
/**********************************************************************/

/* Here are some missing error codes not available in the official header      */
#define JET_errNoMoreThreads                                 -204  /* Could not start thread */
#define JET_errNoComputerName                                -205  /* fail to get computername */
#define JET_wrnTargetInstanceRunning                          578  /* TargetInstance specified for restore is running */

#define JET_errReferenceNotFound                             -1005 /* No such reference */
#define JET_errDiskNotReady                                  -1021 /* Disk not ready */
#define JET_errSharingBufferExceeded                         -1033 /* OS sharing buffer exceeded */
#define JET_errQueryNotSupported                             -1034 /* Query support unavailable */
#define JET_errSQLLinkNotSupported                           -1035 /* SQL Link support unavailable */
#define JET_errTaskLimitExceeded                             -1036 /* Too many client tasks */
#define JET_errUnsupportedOSVersion                          -1037 /* Unsupported OS version */
#define JET_errTooManyFixedColumns                           -1041 /* Too many fixed columns defined */
#define JET_errTooManyVariableColumns                        -1042 /* Too many variable columns defined */
#define JET_wrnCommitNotFlushed                               1049 /* Commit did not flush to disk */
#define JET_errAbortSalvage                                  -1050 /* Forced Salvager abort */
#define JET_errstallableIsamNotFound                         -1056 /* Installable ISAM not found */
#define JET_errOperationCancelled                            -1057 /* Operation canceled by client */
#define JET_errvalidAppend                                   -1060 /* Cannot append long value */
#define JET_errCommitConflict                                -1106 /* Read lock failed due to outstanding commit lock */
#define JET_errMustCommitDistributedTransactionToLevel0      -1150 /*  Attempted to PrepareToCommit a distributed transaction  to non-zero level */
#define JET_errDistributedTransactionAlreadyPreparedToCommit -1151 /* Attempted a write-operation after a distributed transaction has called PrepareToCommit */
#define JET_errNotInDistributedTransaction					 -1152 /* Attempted to PrepareToCommit a non-distributed transaction */
#define JET_errDistributedTransactionNotYetPreparedToCommit  -1153 /* Attempted to commit a distributed transaction, but PrepareToCommit has not yet been called */
#define JET_errCannotNestDistributedTransactions             -1154 /* Attempted to begin a distributed transaction when not at level 0 */
#define JET_errDTCMissingCallback                            -1160 /* Attempted to begin a distributed transaction but no callback for DTC coordination was specified on initialisation */
#define JET_errDTCMissingCallbackOnRecovery                  -1161 /* Attempted to recover a distributed transaction but no callback for DTC coordination was specified on initialisation */
#define JET_errDTCCallbackUnexpectedError                    -1162 /* Unexpected error code returned from DTC callback */
#define JET_wrnDTCCommitTransaction                           1163 /* Warning code DTC callback should return if the specified transaction is to be committed */
#define JET_wrnDTCRollbackTransaction			              1164 /* Warning code DTC callback should return if the specified transaction is to be rolled back */
#define JET_errCannotRename                                  -1306 /* Cannot rename temporary file */
#define JET_errTableNotLocked                                -1309 /* No DDLs w/o exclusive lock */
#define JET_errRulesLoaded                                   -1315 /* Rules loaded, can't define more */
#define JET_errSelfReference                                 -1407 /* Referencing/Referenced index is the same */
#define JET_errIndexHasClustered                             -1408 /* Clustered index already defined */
#define JET_errColumnCannotIndex                             -1513 /* Cannot index Bit, LongText, LongBinary */
#define JET_errRelNoPrimaryIndex                             -1522 /* No primary index on referenced table */
#define JET_errNoBlank                                       -1523 /* Zero length string invalid */
#define JET_wrnTaggedColumnsRemaining                         1523 /* RetrieveTaggedColumnList ran out of copy buffer before retrieving all tagged columns */
#define JET_wrnColumnDefault                                  1537 /* Column value(s) not returned because they were set to their default value(s) and JET_bitEnumerateIgnoreDefault was specified */
#define JET_errCannotsertBefore                              -1606 /* Cannot insert before current */
#define JET_errIntegrityViolationMaster                      -1612 /* References to key exist */
#define JET_errIntegrityViolationslave                       -1613 /* No referenced key exists */
#define JET_wrnMuchDataChanged                                1614 /* Repaint whole datasheet */
#define JET_errIncorrectJoinKey                              -1615 /* Master key does not match lookup key */
#define JET_wrnCannotUndo                                     1616 /* Cannot undo JetUpdate */
#define JET_wrnMuchChangedNoUndo                              1617 /* Repaint datasheet, cannot undo */
#define JET_errRIKeyNullDisallowed                           -1621 /* null value as a result of RI action disallowed */
#define JET_errRIKeyDuplicate                                -1622 /* duplicate key as a result of RI action */
#define JET_errRIUpdateTwice                                 -1623 /* tried to update some field in a record twice */
#define JET_wrnRIRecordsUpdated                               1624 /* some additional records got updated due to RI actions */
#define JET_wrnRIRecordsDeleted                               1625 /* some additional records got deleted due to RI actions */
#define JET_wrnRIRecordsUpdAndDel                             1626 /* some additional records got updated and deleted due to RI actions */
#define JET_errRIInvalidBufferSize                           -1627 /* data size exceeds column size */
#define JET_errRIWriteConflict                               -1628 /* write conflict due to cascading */
#define JET_errRISessWriteConflict                           -1629 /* session write conflict due to cascading */
#define JET_errRedoPrepUpdate                                 1698 /* CONSIDER: QJET  */
#define JET_wrnSyncedToDelRec                                 1699 /* CONSIDER: QJET INTERNAL */
#define JET_errSysDatabaseOpenError                          -1802 /* System db could not be opened */
#define JET_errDatabaseOpenError                             -1804 /* Database file can't be opened */
#define JET_errDatabaseCloseError                            -1806 /* Db file could not be closed */
#define JET_errTooManyOpenFiles                              -1807 /* Too many files open */
#define JET_errCannotOpenSystemDb                            -1901 /* failed sysdb on beginsession */
#define JET_errInvalidLogon                                  -1902 /* invalid logon at beginsession */
#define JET_errInvalidAccountName                            -1903 /* invalid account name */
#define JET_errInvalidSid                                    -1904 /* invalid SID */
#define JET_errInvalidPassword                               -1905 /* invalid password */
#define JET_errRmtMissingOdbcDll                             -2006 /* RMT: Can't load ODBC DLL */
#define JET_errRmtsertFailed                                 -2007 /* RMT: Insert statement failed */
#define JET_errRmtDeleteFailed                               -2008 /* RMT: Delete statement failed */
#define JET_errRmtUpdateFailed                               -2009 /* RMT: Update statement failed */
#define JET_errRmtColDataTruncated                           -2010 /* RMT: data truncated */
#define JET_errRmtTypeIncompat                               -2011 /* RMT: Can't create JET type on server */
#define JET_errRmtCreateTableFailed                          -2012 /* RMT: Create table stmt failed */
#define JET_errRmtNotSupported                               -2014 /* RMT: Function not legal for rdb */
#define JET_errRmtValueOutOfRange                            -2020 /* RMT: Data value out of range */
#define JET_errRmtStillExec                                  -2021 /* RMT INTERNAL: SQL_STILL_EXECUTING */
#define JET_errRmtQueryTimeout                               -2022 /* RMT: Server Not Responding */
#define JET_wrnRmtNeedLvData                                  2023 /* RMT: Internal only - need Lv data */
#define JET_wrnFatCursorUseless                               2024 /* Fat cursor has no effect ** */
#define JET_errRmtWrongSPVer                                 -2025 /* RMT: INTERNAL: wrong SProc ver ** */
#define JET_errRmtLinkOutOfSync                              -2026 /* RMT: the def for the rmt tbl has changed */
#define JET_errRmtDenyWriteIsevalid                          -2027 /* RMT: Can't open DenyWrite */
#define JET_errRmtDriverCantConv                             -2029 /* RMT: INTERNAL: driver cannot convert */
#define JET_errRmtTableAmbiguous                             -2030 /* RMT: Table ambiguous: must specifier owner */
#define JET_errRmtBogusConnStr                               -2031 /* RMT: SPT: Bad connect string */
#define JET_errRmtMultiRowUpdate                             -2032 /* RMT: unique index not really unique */
#define JET_errBadMSysConf                                   -2033 /* RMT: improper MSysConf */
#define JET_errRmtDriverNotSupported                         -2034 /* RMT: ODBC driver not supported through JET */
#define JET_errSLVSpaceCorrupted                             -2201 /* Corruption encountered in space manager of streaming file */
#define JET_errSLVCorrupted                                  -2202 /* Corruption encountered in streaming file */
#define JET_errSLVColumnDefaultValueNotAllowed               -2203 /* SLV columns cannot have a default value */
#define JET_errSLVStreamingFileMissing                       -2204 /* Cannot find streaming file associated with this database */
#define JET_errSLVDatabaseMissing                            -2205 /* Streaming file exists, but database to which it belongs is missing */
#define JET_errSLVStreamingFileAlreadyExists                 -2206 /* Tried to create a streaming file when one already exists or is already recorded in the catalog */
#define JET_errSLVInvalidPath                                -2207 /* Specified path to a streaming file is invalid */
#define JET_errSLVStreamingFileNotCreated                    -2208 /* Tried to perform an SLV operation but streaming file was never created */
#define JET_errSLVStreamingFileReadOnly                      -2209 /* Attach a readonly streaming file for read/write operations */
#define JET_errSLVHeaderBadChecksum                          -2210 /* SLV file header failed checksum verification */
#define JET_errSLVHeaderCorrupted                            -2211 /* SLV file header contains invalid information */
#define JET_errSLVPagesNotFree                               -2213 /* Tried to move pages from the Free state when they were not in that state */
#define JET_errSLVPagesNotReserved                           -2214 /* Tried to move pages from the Reserved state when they were not in that state */
#define JET_errSLVPagesNotCommitted                          -2215 /* Tried to move pages from the Committed state when they were not in that state */
#define JET_errSLVPagesNotDeleted                            -2216 /* Tried to move pages from the Deleted state when they were not in that state */
#define JET_errSLVSpaceWriteConflict                         -2217 /* Unexpected conflict detected trying to write-latch SLV space pages */
#define JET_errSLVRootStillOpen                              -2218 /* The database can not be created/attached because its corresponding SLV Root is still open by another process. */
#define JET_errSLVProviderNotLoaded                          -2219 /* The database can not be created/attached because the SLV Provider has not been loaded. */
#define JET_errSLVEAListCorrupt                              -2220 /* The specified SLV EA List is corrupted. */
#define JET_errSLVRootNotSpecified                           -2221 /* The database cannot be created/attached because the SLV Root Name was omitted */
#define JET_errSLVRootPathInvalid                            -2222 /* The specified SLV Root path was invalid. */
#define JET_errSLVEAListZeroAllocation                       -2223 /* The specified SLV EA List has no allocated space. */
#define JET_errSLVColumnCannotDelete                         -2224 /* Deletion of SLV columns is not currently supported. */
#define JET_errSLVOwnerMapAlreadyExists                      -2225 /* Tried to create a new catalog entry for SLV Ownership Map when one already exists */
#define JET_errSLVSpaceMapAlreadyExists                      -2225 /* OBSOLETE: Renamed to JET_errSLVOwnerMapCorrupted */
#define JET_errSLVOwnerMapCorrupted                          -2226 /* Corruption encountered in SLV Ownership Map */
#define JET_errSLVSpaceMapCorrupted                          -2226 /* OBSOLETE: Renamed to JET_errSLVOwnerMapCorrupted */
#define JET_errSLVOwnerMapPageNotFound                       -2227 /* Corruption encountered in SLV Ownership Map */
#define JET_errSLVSpaceMapPageNotFound                       -2227 /* OBSOLETE: Renamed to JET_errSLVOwnerMapPageNotFound */
#define JET_errSLVFileStale                                  -2229 /* The specified SLV File handle belongs to a SLV Root that no longer exists. */
#define JET_errSLVFileInUse                                  -2230 /* The specified SLV File is currently in use */
#define JET_errSLVStreamingFileInUse                         -2231 /* The specified streaming file is currently in use */
#define JET_errSLVFileIO                                     -2232 /* An I/O error occurred while accessing an SLV File (general read / write failure) */
#define JET_errSLVStreamingFileFull                          -2233 /* No space left in the streaming file */
#define JET_errSLVFileInvalidPath                            -2234 /* Specified path to a SLV File was invalid */
#define JET_errSLVFileAccessDenied                           -2235 /* Cannot access SLV File, the SLV File is locked or is in use */
#define JET_errSLVFileNotFound                               -2236 /* The specified SLV File was not found */
#define JET_errSLVFileUnknown                                -2237 /* An unknown error occurred while accessing an SLV File */
#define JET_errSLVEAListTooBig                               -2238 /* The specified SLV EA List could not be returned because it is too large to fit in the standard EA format.  Retrieve the SLV File as a file handle instead. */
#define JET_errSLVProviderVersionMismatch                    -2239 /* The loaded SLV Provider's version does not match the database engine's version. */
#define JET_errSLVBufferTooSmall                             -2243 /* Buffer allocated for SLV data or meta-data was too small */


#define JET_errQueryInvalidAttribut     -3001 /* Invalid query attribute */
#define JET_errQueryOnlyOneRow          -3002 /* Only 1 such row allowed */
#define JET_errQueryIncompleteRow       -3003 /* Missing value in row */
#define JET_errQueryInvalidFlag         -3004 /* Invalid value in Flag field */
#define JET_errQueryCycle               -3005 /* Cycle in query definition */
#define JET_errQueryInvalidJoinTable    -3006 /* Invalid table in join */
#define JET_errQueryAmbigRef            -3007 /* Ambiguous column reference */
#define JET_errQueryUnboundRef          -3008 /* Cannot bind name */
#define JET_errQueryParmRedef           -3009 /* Parm redefined with different type */
#define JET_errQueryMissingParms        -3010 /* Too few parameters supplied */
#define JET_errQueryInvalidOutput       -3011 /* Invalid query output */
#define JET_errQueryInvalidHaving       -3012 /* HAVING clause without aggregation */
#define JET_errQueryDuplicateAlias      -3013 /* Duplicate output alias */
#define JET_errQueryInvalidMGBInput     -3014 /* Cannot input from MGB */
#define JET_errQueryInvalidOrder        -3015 /* Invalid ORDER BY expression */
#define JET_errQueryTooManyLevels       -3016 /* Too many levels on MGB */
#define JET_errQueryMissingLevel        -3017 /* Missing intermediate MGB level */
#define JET_errQueryIllegalAggregate    -3018 /* Aggregates not allowed */
#define JET_errQueryDuplicateOutput     -3019 /* Duplicate destination output */
#define JET_errQueryIsBulkOp            -3020 /* Grbit should be set for Bulk Operation */
#define JET_errQueryIsNotBulkOp         -3021 /* Query is not a Bulk Operation */
#define JET_errQueryIllegalOuterJoin    -3022 /* No inconsistent updates on outer joins */
#define JET_errQueryNullRequired        -3023 /* Column must be NULL */
#define JET_errQueryNoOutputs           -3024 /* Query must have an output */
#define JET_errQueryNoInputTables       -3025 /* Query must have an input */
#define JET_wrnQueryNonUpdatableRvt      3026 /* Query is not updatable (but IS RVT) */
#define JET_errQueryInvalidAlias        -3027 /* Bogus character in alias name */
#define JET_errQueryInvalidBulkeput     -3028 /* Cannot input from bulk operation */
#define JET_errQueryNotDirectChild      -3029 /* T.* must use direct child */
#define JET_errQueryExprEvaluation      -3030 /* Expression evaluation error */
#define JET_errQueryIsNotRowReturning   -3031 /* Query does not return rows */
#define JET_wrnQueryNonRvt               3032 /* Can't create RVT, query is static */
#define JET_errQueryParmTypeMismatch    -3033 /* Wrong parameter type given */
#define JET_errQueryChanging            -3034 /* Query Objects are being updated */
#define JET_errQueryNotUpdatable        -3035 /* Operation must use an updatable query */
#define JET_errQueryMissingColumnName   -3036 /* Missing destination column */
#define JET_errQueryTableDuplicate      -3037 /* Repeated table name in FROM list */
#define JET_errQueryIsMGB               -3038 /* Query is an MGB */
#define JET_errQueryInsetoBulkMGB       -3039 /* Cannot insert into Bulk/MGB */
#define JET_errQueryDistinctNotAllowed  -3040 /* DISTINCT not allowed for MGB */
#define JET_errQueryDistinctRowNotAllow -3041 /* DISTINCTROW not allowed for MGB */
#define JET_errQueryNoDbForParmDestTbl  -3045 /* Dest DB for VT parm not allowed */
#define JET_errQueryDuplicatedFixedSet  -3047 /* Duplicated Fixed Value */
#define JET_errQueryNoDeleteTables      -3048 /* Must specify tables to delete from */
#define JET_errQueryCannotDelete        -3049 /* Cannot delete from specified tables */
#define JET_errQueryTooManyGroupExprs   -3050 /* Too many GROUP BY expressions */
#define JET_errQueryTooManyOrderExprs   -3051 /* Too many ORDER BY expressions */
#define JET_errQueryTooManyDistExprs    -3052 /* Too many DISTINCT output expressions */
#define JET_errQueryBadValueList        -3053 /* Malformed value list in Transform */
#define JET_errConnStrTooLong           -3054 /* Connect string too long */
#define JET_errQueryInvalidParm         -3055 /* Invalid Parmeter Name (&gt;64 char) */
#define JET_errQueryContainsDbParm      -3056 /* Can't get parameters with Db Parm */
#define JET_errQueryBadUpwardRefed      -3057 /* Illegally Upward ref'ed */
#define JET_errQueryAmbiguousJoins      -3058 /* Joins in a QO are ambiguous */
#define JET_errQueryIsNotDDL            -3059 /* Not a DDL Operation */
#define JET_errNoDbConnStr              -3060 /* No database in connect string */
#define JET_wrnQueryIsNotRowReturning    3061 /* Not row returning */
#define JET_errSingleColumnExpected     -3063 /* At most one column can be returned from this subquery */
#define JET_errColumnCountMismatch      -3064 /* Union Query: number of columns in children don't match */
#define JET_errQueryTopNotAllowed       -3065 /* Top not allowed in query */
#define JET_errQueryIsDDL               -3066 /* Must set JET_bitTableDDL */
#define JET_errQueryIsCorrupt           -3067 /* Query is Corrupt */
#define JET_errQuerySPTBulkSucceeded    -3068 /* INTERNAL only */
#define JET_errSPTReturnedNoRecords     -3069 /* SPT marked as RowReturning did not return a table */
#define JET_errSingleRecordExpected     -3070 /* At most one record can be returned from this subquery */
#define JET_errQueryTooComplex          -3071 /* Query too complex; push expressions into UDFs and reduce number of columns */
#define JET_errSetOpvalideSubquery      -3072 /* Unions not allowed in a subquery */
#define JET_errQueryKeyTooBig           -3073 /* join, group, or sort key is too big */
#define JET_errNoMostMany               -3074 /* most many info can not be obtained */
#define JET_wrnAmbiguousMostMany         3075 /* a unique mostmany table was unable to be determined */
#define JET_errQueryIsNotRvt            -3076 /* query is not an RVT */
#define JET_errTooManyFindSessions      -3077 /* Too many FastFind Sessions were invoked */
#define JET_errQueryNotCheap            -3078 /* INTERNAL: For use with JET_bitTableOnlyIfCheap */
#define JET_errExprSyntax               -3100 /* Syntax error in expression */
#define JET_errExprIllegalType          -3101 /* Illegal type in expression */
#define JET_errExprUnknownFunction      -3102 /* Unknown function in expression */
#define JET_errSQLSyntax                -3500 /* Bogus SQL statement type */
#define JET_errSQLParameterSyntax       -3501 /* Parameter clause syntax error */
#define JET_errSQLInsertSyntax          -3502 /* INSERT clause syntax error */
#define JET_errSQLUpdateSyntax          -3503 /* UPDATE clause syntax error */
#define JET_errSQLSelectSyntax          -3504 /* SELECT clause syntax error */
#define JET_errSQLDeleteSyntax          -3505 /* Expected 'FROM' after 'DELETE' */
#define JET_errSQLFromSyntax            -3506 /* FROM clause syntax error */
#define JET_errSQLGroupBySyntax         -3507 /* GROUP BY clause syntax error */
#define JET_errSQLOrderBySyntax         -3508 /* ORDER BY clause syntax error */
#define JET_errSQLLevelSyntax           -3509 /* LEVEL syntax error */
#define JET_errSQLJoinSyntax            -3510 /* JOIN syntax error */
#define JET_errSQLTransformSyntax       -3511 /* TRANSFORM syntax error */
#define JET_errSQLHavingSyntax          -3512 /* HAVING clause syntax error */
#define JET_errSQLWhereSyntax           -3513 /* WHERE clause syntax error */
#define JET_errSQLProcedureSyntax       -3514 /* Expected query name after 'PROCEDURE' */
#define JET_errSQLNotEnoughBuf          -3515 /* Buffer too small for SQL string */
#define JET_errSQLMissingSemicolon      -3516 /* Missing ; at end of SQL statement */
#define JET_errSQLTooManyTokens         -3517 /* Characters after end of SQL statement */
#define JET_errSQLOwnerAccessSyntax     -3518 /* OWNERACCESS OPTION syntax error */
#define JET_errNoInsertColumnName       -3519 /* No dest. col. name in INSERT stmt */
#define JET_errQueryWrongNumDestCol     -3520 /* Too few/many destination columns */
#define JET_errV11NotSupported          -3530 /* not supported in V11 */
#define JET_errV10Format                -3531 /* can be present in V10 format only */
#define JET_errSQLUnionSyntax           -3532 /* UNION query syntax error */
#define JET_wrnSqlPassThrough            3534 /* Pass Through query involved */
#define JET_errSQLInvalidSPT            -3535 /* SPT Must be at least one char */
#define JET_errDDLConstraintSyntax      -3550 /* constraint syntax error */
#define JET_errDDLCreateTableSyntax     -3551 /* create table syntax error */
#define JET_errDDLCreateIndexSyntax     -3552 /* create index syntax error */
#define JET_errDDLColumnDefSyntax       -3553 /* column def syntax error */
#define JET_errDDLAlterTableSyntax      -3554 /* alter table syntax error */
#define JET_errDDLDropIndexSyntax       -3555 /* drop index syntax error */
#define JET_errDDLDropSyntax            -3556 /* drop column or constraint syntax error */
#define JET_errDDLDiffNumRelCols        -3560 /* different number of cols in relationship */
#define JET_errDDLIndexColNotFound      -3561 /* creating index on non- existant column */
#define JET_errNoSuchProperty           -3600 /* Property was not found */
#define JET_errPropertyTooLarge         -3601 /* Small Property larger than 2K */
#define JET_errJPMInvalidForV1x         -3602 /* No JPM for non Red-V2 databases */
#define JET_errPropertyExists           -3603 /* Property already exists */
#define JET_errvalidDelete              -3604 /* DeleteOnly called with non- zero cbData */
#define JET_wrnPropCouldNotSave          3605 /* Optimization to save props failed */
#define JET_errPropMustBeDDL            -3606 /* You must be set JET_bitPropDDL to set/delete this property */
#define JET_wrnFindWrapped               3700 /* Cursor wrapped during fast find */
#define JET_errTLVNativeUserTablesOnly  -3700 /* TLVs can only be placed on native user tables/columns */
#define JET_errTLVNoNull                -3701 /* This field cannot be null */
#define JET_errTLVNoBlank               -3702 /* This column cannot be blank */
#define JET_errTLVRuleViolation         -3703 /* This validation rule must be met */
#define JET_errTLVInvalidColumn         -3704 /* This TLV property cannot be placed on this column */
#define JET_errTLVDefaultExprEvaluation -3705 /* Default value expression evaluation error */
#define JET_errTLVExprUnknownFunc       -3706 /* Unknown function in TLV expression */
#define JET_errTLVExprSyntax            -3707 /* Syntax error in TLV expression */
#define JET_errDefaultExprSyntax        -3708 /* Syntax error in Default expression */
#define JET_errTLVRuleExprEvaluation    -3709 /* Rule expression evaluation error */
#define JET_errGeneral                  -5001 /* I-ISAM: assert failure */
#define JET_errRecordLocked             -5002 /* I-ISAM: record locked */
#define JET_wrnColumnDataTruncated       5003 /* I-ISAM: data truncated */
#define JET_errTableNotOpen             -5004 /* I-ISAM: table is not open */
#define JET_errDecryptFail              -5005 /* I-ISAM: incorrect password */
#define JET_wrnCurrencyLost              5007 /* I-ISAM: currency lost - must first/last */
#define JET_errDateOutOfRange           -5008 /* I-ISAM: invalid date */
#define JET_wrnOptionsIgnored            5011 /* I-ISAM: options were ignored */
#define JET_errTableNotComplete         -5012 /* I-ISAM: incomplete table definition */
#define JET_errIllegalNetworkOption     -5013 /* I-ISAM: illegal network option */
#define JET_errIllegalTimeoutOption     -5014 /* I-ISAM: illegal timeout option */
#define JET_errNotExternalFormat        -5015 /* I-ISAM: invalid file format */
#define JET_errUnexpectedEngineReturn   -5016 /* I-ISAM: unexpected engine error code */
#define JET_errNumericFieldOverflow     -5017 /* I-ISAM: can't convert to native type */
#define JET_errIndexHasNoPrimary        -5020 /* Paradox: no primary index */
#define JET_errTableSortOrderMismatch   -5021 /* Paradox: sort order mismatch */
#define JET_errNoConfigParameters       -5023 /* Paradox: net path or user name missing */
#define JET_errCantAccessParadoxNetDir  -5024 /* Paradox: bad Paradox net path */
#define JET_errObsoleteLockFile         -5025 /* Paradox: obsolete lock file */
#define JET_errIllegalCollatingSequence -5026 /* Paradox: invalid sort sequence */
#define JET_errWrongCollatingSequence   -5027 /* Paradox: wrong sort sequence */
#define JET_errCantUseUnkeyedTable      -5028 /* Paradox: can't open unkeyed table */
#define JET_errNetCtrlMismatch          -5029 /* Paradox: Open with different .net files */
#define JET_err4xTableWith3xLocking     -5030 /* Paradox: Can't open 4x table with 3x lock */
#define JET_errINFFileError             -5101 /* dBase: invalid .INF file */
#define JET_errCantMakeINFFile          -5102 /* dBase: can't open .INF file */
#define JET_wrnCantMaintainIndex         5103 /* dBase: unmaintainable index */
#define JET_errMissingMemoFile          -5104 /* dBase: missing memo file */
#define JET_errIllegalCenturyOption     -5105 /* dBase: Illegal century option */
#define JET_errIllegalDeletedOption     -5106 /* dBase: Illegal deleted option */
#define JET_errIllegalStatsOption       -5107 /* dBase: Illegal statistics option */
#define JET_errIllegalDateOption        -5108 /* dBase: Illegal date option */
#define JET_errIllegalMarkOption        -5109 /* dBase: Illegal mark option */
#define JET_wrnDuplicateIndexes          5110 /* dBase: duplicate indexes in INF file */
#define JET_errINFIndexNotFound         -5111 /* dBase: missing index in INF file */
#define JET_errWrongMemoFileType        -5112 /* dBase: wrong memo file type */
#define JET_errTooManyLongFields        -5200 /* Btrieve: more than one memo field */
#define JET_errCantStartBtrieve         -5201 /* Btrieve: wbtrcall.dll missing */
#define JET_errBadConfigParameters      -5202 /* Btrieve: win.ini [btrieve] options wrong */
#define JET_errIndexesChanged           -5203 /* Btrieve: need to GetIndexInfo */
#define JET_errNonModifiableKey         -5204 /* Btrieve: can't modify record column */
#define JET_errOutOfBVResources         -5205 /* Btrieve: out of resources */
#define JET_errBtrieveDeadlock          -5206 /* Btrieve: locking deadlock */
#define JET_errBtrieveFailure           -5207 /* Btrieve: Btrieve DLL failure */
#define JET_errBtrieveDDCorrupted       -5208 /* Btrieve: data dictionary corrupted */
#define JET_errBtrieveTooManyTasks      -5209 /* Btrieve: too many tasks */
#define JET_errIllegalIndexDDFOption    -5210 /* Btrieve: Illegal IndexDDF option */
#define JET_errIllegalDataCodePage      -5211 /* Btrieve: Illegal DataCodePage option */
#define JET_errXtrieveEnvironmentError  -5212 /* Btrieve: Xtrieve INI options bad */
#define JET_errMissingDDFFile           -5213 /* Btrieve: Missing field.ddf */
#define JET_errIllegalIndexNumberOption -5214 /* Btrieve: Illegal IndexRenumber option */
#define JET_errFindExprSyntax           -8001 /* Syntax error in FastFind expression */
#define JET_errQbeExprSyntax            -8002 /* Syntax error in QBE expression */
#define JET_errputTableNotFound         -8003 /* Non-existant object in FROM list */
#define JET_errQueryExprSyntax          -8004 /* Syntax error in some query expression */
#define JET_errQodefExprSyntax          -8005 /* Syntax error in expression column */
#define JET_errExpAliasAfterAS          -8006 /* Expected alias after 'AS' in FROM list */
#define JET_errExpBYAfterGROUP          -8007 /* Expected 'BY' after 'GROUP' */
#define JET_errExpBYAfterORDER          -8008 /* Expected 'BY' after 'ORDER' */
#define JET_errExpClsParenAfterColList  -8009 /* Expected ')' after column list */
#define JET_errExpColNameAfterPIVOT     -8010 /* Expected column name after 'PIVOT' */
#define JET_errExpDatabaseAfterIN       -8011 /* Expected database name after 'IN' */
#define JET_errExpDatatypeAfterParmName -8012 /* Expected datatype after parameter name */
#define JET_errExpEqualAfterUpdColName  -8013 /* Expected '=' after update column name */
#define JET_errExpExprAfterON           -8014 /* Expected join expression after 'ON' */
#define JET_errExpExprAfterTRANSFORM    -8015 /* Expected expression after 'TRANSFORM' */
#define JET_errExpExprAfterWHERE        -8016 /* Expected expression after 'WHERE' */
#define JET_errExpGroupClauseInXform    -8017 /* Transform expects GROUP BY clause */
#define JET_errExpGroupingExpr          -8018 /* Expected grouping expression */
#define JET_errExpHavingExpr            -8019 /* Expected HAVING expression */
#define JET_errExpINTOAfterINSERT       -8020 /* Expected 'INTO' after 'INSERT' */
#define JET_errExpJOINAfterJoinType     -8021 /* Expected 'JOIN' after INNER/LEFT/RIGHT */
#define JET_errExpLEVELAfterSelectList  -8022 /* Expected LEVEL after select list */
#define JET_errExpNumberAfterLEVEL      -8023 /* Expected number after 'LEVEL' */
#define JET_errExpONAfterRightTable     -8024 /* Expected 'ON' after right join table */
#define JET_errExpOrderExpr             -8025 /* Expected ordering expression */
#define JET_errExpOutputAliasAfterAS    -8026 /* Expected output alias after 'AS' */
#define JET_errExpOutputExpr            -8027 /* Expected output expression */
#define JET_errExpPIVOTAfterSelectStmt  -8028 /* Expected 'PIVOT' after SELECT statement */
#define JET_errExpRightJoinTable        -8029 /* Expected right join table after 'JOIN' */
#define JET_errExpSELECTAftersClause    -8030 /* Expected 'SELECT' after INSERT clause */
#define JET_errExpSELECTAfterXformExpr  -8031 /* Expected 'SELECT' after Transform fact */
#define JET_errExpSETAfterTableName     -8032 /* Expected 'SET' after table name */
#define JET_errExpSemiAfterLevelNumber  -8033 /* Expected ';' after level number */
#define JET_errExpSemiAfterParmList     -8034 /* Expected ';' after parmeter list */
#define JET_errExpSemiAfterPivotClause  -8035 /* Expected ';' after PIVOT clause */
#define JET_errExpSemiAtEndOfSQL        -8036 /* Expected ';' at end of SQL statement */
#define JET_errExpTableName             -8037 /* Expected table name */
#define JET_errExpTableNameAfterINTO    -8038 /* Expected table name after 'INTO' */
#define JET_errExpUpdExprAfterEqual     -8039 /* Expected update expression after '=' */
#define JET_errExpUpdateColName         -8040 /* Expected update column name */
#define JET_errvTokenAfterFromList      -8041 /* Bogus token after FROM list */
#define JET_errvTokenAfterGroupList     -8042 /* Bogus token after GROUP BY list */
#define JET_errvTokenAfterHavingCls     -8043 /* Bogus token after HAVING clause */
#define JET_errvTokenAfterOrderClause   -8044 /* Bogus token after ORDER BY clause */
#define JET_errvTokenAfterSelectCls     -8045 /* Bogus token after SELECT clause */
#define JET_errvTokenAfterWhereClause   -8046 /* Bogus token after WHERE clause */
#define JET_errLevelNumberTooBig        -8047 /* Number after 'LEVEL' too big */
#define JET_errLevelOnNonMGB            -8048 /* LEVEL allowed only in MGB */
#define JET_errIllegalDetailReference   -8049 /* Not group key or agg, but not MGB detail */
#define JET_errAggOverMixedLevels       -8050 /* Agg. arg. uses outputs from > 1 level */
#define JET_errAggregatingHigherLevel   -8051 /* Agg. over output of same/higher level */
#define JET_errNullJoinKey              -8052 /* Cannot set column in join key to NULL */
#define JET_errValueBreaksJoin          -8053 /* Join is broken by column value(s) */
#define JET_errsertetoUnknownColumn     -8054 /* INSERT INTO unknown column name */
#define JET_errNoSelecttoColumnName     -8055 /* No dest. col. name in SELECT INTO stmt */
#define JET_errNoInsertColumnNameM      -8056 /* No dest. col. name in INSERT stmt */
#define JET_errColumnNotJoinTable       -8057 /* Join expr refers to non-join table */
#define JET_errAggregateInJoin          -8058 /* Aggregate in JOIN clause */
#define JET_errAggregateInWhere         -8059 /* Aggregate in WHERE clause */
#define JET_errAggregateInOrderBy       -8060 /* Aggregate in ORDER BY clause */
#define JET_errAggregateInGroupBy       -8061 /* Aggregate in GROUP BY clause */
#define JET_errAggregateInArgument      -8062 /* Aggregate in argument expression */
#define JET_errHavingOnTransform        -8063 /* HAVING clause on TRANSFORM query */
#define JET_errHavingWithoutGrouping    -8064 /* HAVING clause w/o grouping/aggregation */
#define JET_errHavingOnMGB              -8065 /* HAVING clause on MGB query */
#define JET_errOutputAliasCycle         -8066 /* Cycle in SELECT list (via aliases) */
#define JET_errDotStarWithGrouping      -8067 /* 'T.*' with grouping, but not MGB level 0 */
#define JET_errStarWithGrouping         -8068 /* '*' with grouping, but not MGB detail */
#define JET_errQueryTreeCycle           -8069 /* Cycle in tree of query objects */
#define JET_errTableRepeatFromList      -8072 /* Table appears twice in FROM list */
#define JET_errTooManyXformLevels       -8073 /* Level &gt; 2 in TRANSFORM query */
#define JET_errTooManyMGBLevels         -8074 /* Too many levels in MGB */
#define JET_errNoUpdateColumnName       -8075 /* No dest. column name in UPDATE stmt */
#define JET_errJoinTableNotput          -8076 /* Join table not in FROM list */
#define JET_errUnaliasedSelfJoin        -8077 /* Join tables have same name */
#define JET_errOutputLevelTooBig        -8078 /* Output w/ level &gt; 1+max group level */
#define JET_errOrderVsGroup             -8079 /* ORDER BY conflicts with GROUP BY */
#define JET_errOrderVsDistinct          -8080 /* ORDER BY conflicts with DISTINCT */
#define JET_errExpLeftParenthesis       -8082 /* Expected '(' */
#define JET_errExpRightParenthesis      -8083 /* Expected ')' */
#define JET_errEvalEBESErr              -8084 /* EB/ES error evaluating expression */
#define JET_errQueryExpCloseQuote       -8085 /* Unmatched quote for database name */
#define JET_errQueryParmNotDatabase     -8086 /* Parameter type should be database */
#define JET_errQueryParmNotTableid      -8087 /* Parameter type should be tableid */
#define JET_errExpIdentifierM           -8088 /* Expected identifier */
#define JET_errExpQueryName             -8089 /* Expected query name after PROCEDURE */
#define JET_errExprUnknownFunctionM     -8090 /* Unknown function in expression */
#define JET_errQueryAmbigRefM           -8091 /* Ambiguous column reference */
#define JET_errQueryBadBracketing       -8092 /* Bad bracketing of identifier */
#define JET_errQueryBadQodefName        -8093 /* Invalid name in QODEF row */
#define JET_errQueryBulkColNotUpd       -8094 /* Column not updatable (bulk op) */
#define JET_errQueryDistinctNotAllowedM -8095 /* DISTINCT not allowed for MGB */
#define JET_errQueryDuplicateAliasM     -8096 /* Duplicate output alias */
#define JET_errQueryDuplicateOutputM    -8097 /* Duplicate destination output */
#define JET_errQueryDuplicatedFixedSetM -8098 /* Duplicated Fixed Value */
#define JET_errQueryIllegalOuterJoinM   -8099 /* No inconsistent updates on outer joins */
#define JET_errQueryIncompleteRowM      -8100 /* Missing value in row */
#define JET_errQueryInvalidAttributeM   -8101 /* Invalid query attribute */
#define JET_errQueryInvalidBulkeputM    -8102 /* Cannot input from bulk operation */
#define JET_errQueryInvalidFlagM        -8103 /* Invalid value in Flag field */
#define JET_errQueryInvalidMGBInputM    -8104 /* Cannot input from MGB */
#define JET_errQueryLVInAggregate       -8105 /* Illegal long value in aggregate */
#define JET_errQueryLVInDistinct        -8106 /* Illegal long value in DISTINCT */
#define JET_errQueryLVInGroupBy         -8107 /* Illegal long value in GROUP BY */
#define JET_errQueryLVInHaving          -8108 /* Illegal long value in HAVING */
#define JET_errQueryLVInJoin            -8109 /* Illegal long value in JOIN */
#define JET_errQueryLVInOrderBy         -8110 /* Illegal long value in ORDER BY */
#define JET_errQueryMissingLevelM       -8111 /* Missing intermediate MGB level */
#define JET_errQueryMissingParmsM       -8112 /* Too few parameters supplied */
#define JET_errQueryNoDbForParmDestTblM -8113 /* Dest DB for VT parm not allowed */
#define JET_errQueryNoDeletePerm        -8114 /* No delete permission on table/query */
#define JET_errQueryNoInputTablesM      -8115 /* Query must have an input */
#define JET_errQueryNoInsertPerm        -8116 /* No insert permission on table/query */
#define JET_errQueryNoOutputsM          -8117 /* Query must have an output */
#define JET_errQueryNoReadDefPerm       -8118 /* No permission to read query definition */
#define JET_errQueryNoReadPerm          -8119 /* No read permission on table/query */
#define JET_errQueryNoReplacePerm       -8120 /* No replace permission on table/query */
#define JET_errQueryNoTblCrtPerm        -8121 /* No CreateTable permission (bulk op) */
#define JET_errQueryNotDirectChildM     -8122 /* T.* must use direct child */
#define JET_errQueryNullRequiredM       -8123 /* Column must be NULL */
#define JET_errQueryOnlyOneRowM         -8124 /* Only 1 such row allowed */
#define JET_errQueryOutputColNotUpd     -8125 /* Query output column not updatable */
#define JET_errQueryParmRedefM          -8126 /* Parm redefined with different type */
#define JET_errQueryParmTypeMismatchM   -8127 /* Wrong parameter type given */
#define JET_errQueryUnboundRefM         -8128 /* Cannot bind name */
#define JET_errRmtConnectFailedM        -8129 /* RMT: Connection attempt failed */
#define JET_errRmtDeleteFailedM         -8130 /* RMT: Delete statement failed */
#define JET_errRmtsertFailedM           -8131 /* RMT: Insert statement failed */
#define JET_errRmtMissingOdbcDllM       -8132 /* RMT: Can't load ODBC DLL */
#define JET_errRmtSqlErrorM             -8133 /* RMT: ODBC call failed */
#define JET_errRmtUpdateFailedM         -8134 /* RMT: Update statement failed */
#define JET_errSQLDeleteSyntaxM         -8135 /* Expected 'FROM' after 'DELETE' */
#define JET_errSQLSyntaxM               -8136 /* Bogus SQL statement type */
#define JET_errSQLTooManyTokensM        -8137 /* Characters after end of SQL statement */
#define JET_errStarNotAtLevel0          -8138 /* '*' illegal above level 0 */
#define JET_errQueryParmTypeNotAllowed  -8139 /* Parameter type not allowed for expression */
#define JET_errRmtLinkNotFound          -8144 /* RMT: link not found */
#define JET_errRmtTooManyColumns        -8145 /* RMT: Too many columns on Select Into */
#define JET_errWriteConflictM           -8146 /* Write lock failed due to outstanding write lock */
#define JET_errReadConflictM            -8147 /* Commit lock failed due to outstanding read lock */
#define JET_errCommitConflictM          -8148 /* Read lock failed due to outstanding commit lock */
#define JET_errTableLockedM             -8149 /* Table is exclusively locked */
#define JET_errTableInUseM              -8150 /* Table is in use, cannot lock */
#define JET_errQueryTooManyXvtColumn    -8151 /* Too many cross table column headers */
#define JET_errOutputTableNotFound      -8152 /* Non-existent table in Insert Into */
#define JET_errTableLockedQM            -8153 /* Table is exclusively locked */
#define JET_errTableInUseQM             -8154 /* Table is in use, cannot lock */
#define JET_errTableLockedMUQM          -8155 /* Table is exclusively locked */
#define JET_errTableInUseMUQM           -8156 /* Table is in use, cannot lock */
#define JET_errQueryInvalidParmM        -8157 /* Invalid Parmeter Name (&gt;64 char) */
#define JET_errFileNotFoundM            -8158 /* File not found */
#define JET_errFileShareViolationM      -8159 /* File sharing violation */
#define JET_errFileAccessDeniedM        -8160 /* Access denied */
#define JET_errvalidPathM               -8161 /* Invalid Path */
#define JET_errTableDuplicateM          -8162 /* Table already exists */
#define JET_errQueryBadUpwardRefedM     -8163 /* Illegally Upward ref'ed */
#define JET_errIntegrityViolMasterM     -8164 /* References to key exist */
#define JET_errIntegrityViolslaveM      -8165 /* No referenced key exists */
#define JET_errSQLUnexpectedWithM       -8166 /* Unexpected 'with' in this place */
#define JET_errSQLOwnerAccessM          -8167 /* Owner Access Option is defined Twice */
#define JET_errSQLOwnerAccessSyntaxM    -8168 /* Owner Access Option Syntax Error */
#define JET_errSQLOwnerAccessDef        -8169 /* Owner Access Option is defined more than once */
#define JET_errAccessDeniedM            -8170 /* Generic Access Denied */
#define JET_errUnexpectedEngineReturnM  -8171 /* I-ISAM: unexpected engine error code */
#define JET_errQueryTopNotAllowedM      -8172 /* query cannot contain Top N */
#define JET_errvTokenAfterTableCls      -8173 /* Bogus token after table clause */
#define JET_errvTokenAfterRParen        -8174 /* Unexpected tokens after a closing paren */
#define JET_errQueryBadValueListM       -8175 /* Malformed value list in Transform */
#define JET_errQueryIsCorruptM          -8176 /* Query is Corrupt */
#define JET_errvalidTopArgumentM        -8177 /* Select Top argument is invalid */
#define JET_errQueryIsSnapshot          -8178 /* Query is a snapshot */
#define JET_errQueryExprOutput          -8179 /* Output is a calculated column */
#define JET_errQueryTableRO             -8180 /* Column comes from read-only table */
#define JET_errQueryRowDeleted          -8181 /* Column comes from deleted row */
#define JET_errQueryRowLocked           -8182 /* Column comes from locked row */
#define JET_errQueryFixupChanged        -8183 /* Would row-fixup away from pending changes */
#define JET_errQueryCantFill            -8184 /* Fill-in-the-blank only on most-many */
#define JET_errQueryWouldOrphan         -8185 /* Would orphan joined records */
#define JET_errIncorrectJoinKeyM        -8186 /* Must match join key in lookup table */
#define JET_errQueryLVInSubqueryM       -8187 /* Illegal long value in subquery */
#define JET_errvalidDatabaseM           -8188 /* Unrecognized database format */
#define JET_errOrderVsUnion             -8189 /* You can only order by an outputted column in a union */
#define JET_errTLVCouldNotBindRef       -8190 /* Unknown token in TLV expression */
#define JET_errCouldNotBindRef          -8191 /* Unknown token in FastFind expression */
#define JET_errQueryPKeyNotOutput       -8192 /* Primary key not output */
#define JET_errQueryJKeyNotOutput       -8193 /* Join key not output */
#define JET_errExclusiveDBConflict      -8194 /* Conflict with exclusive user */
#define JET_errQueryNoJoinedRecord      -8195 /* No F.I.T.B. insert if no joined record */
#define JET_errQueryLVInSetOp           -8196 /* Illegal long value in set operation */
#define JET_errTLVExprUnknownFunctionM  -8197 /* Unknown function in TLV expression */
#define JET_errvalidNameM               -8198 /* Invalid name */
#define JET_errExpValue                 -8200 /* Expected value in VALUES list */
#define JET_errDDLExpColName            -8201 /* expect column name */
#define JET_errDDLExpLP                 -8202 /* expect '(' */
#define JET_errDDLExpRP                 -8203 /* expect ')' */
#define JET_errDDLExpIndex              -8204 /* expect INDEX */
#define JET_errDDLExpIndexName          -8205 /* expect index name */
#define JET_errDDLExpOn                 -8206 /* expect ON */
#define JET_errDDLExpKey                -8207 /* expect KEY */
#define JET_errDDLExpReferences         -8208 /* expect REFERENCES */
#define JET_errDDLExpTableName          -8209 /* expect table name */
#define JET_errDDLExpNull               -8210 /* expect NULL */
#define JET_errDDLExpConstraintName     -8211 /* expect constraint name */
#define JET_errDDLExpDatatype           -8212 /* expect data type */
#define JET_errDDLExpTable              -8213 /* expect TABLE */
#define JET_errDDLExpEos                -8214 /* expect End Of String */
#define JET_errDDLExpAddOrDrop          -8215 /* expect ADD or DROP */
#define JET_errDDLExpColOrConstr        -8216 /* expect COLUMN or CONSTRAINT */
#define JET_errDDLExpTableOrIndex       -8217 /* expect TABLE or INDEX */
#define JET_errDDLExpNumber             -8218 /* expect a number */
#define JET_errDDLExpConstr             -8219 /* expect a constraint */
#define JET_errDDLPermissionDenied      -8220 /* DDL on read-only database */
#define JET_errDDLObjectNotFound        -8221 /* table/constraint not found */
#define JET_errDDLIndexNotFound         -8222 /* no such index */
#define JET_errDDLNoPkeyOnRefdTable     -8223 /* no primary key on referenced table */
#define JET_errDDLColumnsNotUnique      -8224 /* no unique key on referenced columns */
#define JET_errDDLIndexDuplicate        -8225 /* index already exists */
#define JET_errDDLTableNotFound         -8226 /* no such table */
#define JET_errDDLRelNotFound           -8227 /* no such relationship */
#define JET_errDDLRelDuplicate          -8228 /* relationship already exists */
#define JET_errDDLIntegrityViolation    -8229 /* existing data violates new relationship */
#define JET_errDDLColumnDuplicate       -8230 /* column already exists */
#define JET_errDDLColumnNotFound        -8231 /* no such column */
#define JET_errDDLColumnTooBig          -8232 /* column length too long */
#define JET_errDDLColumneRel            -8233 /* cannot drop column, in relationship */
#define JET_errRelNoPrimaryIndexM       -8234 /* No primary index on referenced table */
#define JET_errvalidColumnM             -8235 /* Invalid column in index or reference definition */
#define JET_errV11TableNameNotScope     -8250 /* referenced table not in join clause */
#define JET_errV11OnlyTwoTables         -8251 /* exactly two tables should be referenced in join */
#define JET_errV11OneSided              -8252 /* all tables come from one side of input */
#define JET_errV11Ambiguous             -8253 /* Join clause is ambiguous when stored in V1 format */
#define JET_errTLVExprSyntaxM           -8260 /* Syntax error in TLV expression */
#define JET_errTLVNoNullM               -8261 /* This field cannot be null */
#define JET_errTLVNoBlankM              -8262 /* This column cannot be blank */
#define JET_errTLVRuleViolationM        -8263 /* This validation rule must be met */
#define JET_errRIViolationMasterCM      -8264 /* references to key exist while doing a cascade RI action */
#define JET_errRIViolationslaveCM       -8265 /* no referenced key exist while doing a cascade RI action */
#define JET_errRIKeyNullDisallowedCM    -8266 /* null value as a result of a cascade RI action */
#define JET_errRIKeyDuplicateCM         -8267 /* duplicate key as a result of a cascade RI action */
#define JET_errRIUpdateTwiceCM          -8268 /* tried to update some field in a record twice */
#define JET_errRITLVNoNullCM            -8269 /* This field cannot be null */
#define JET_errRITLVNoBlankCM           -8270 /* This column cannot be blank */
#define JET_errRITLVRuleViolationCM     -8271 /* This validation rule must be met */
#define JET_errRIInvalidBufferSizeCM    -8272 /* data size exceeds the column size as a result of a cascade action */
#define JET_errRIWriteConflictCM        -8273 /* write conflict due to cascading */
#define JET_errRISessWriteConflictCM    -8274 /* session write conflict due to cascading */
#define JET_errREPReadOnly             -20000 /* can't open this for update */
#define JET_errREPSetRepid             -20001 /* users can't change the rep ID */
#define JET_errREPDBNotRep             -20002 /* database not replicable */
#define JET_errREPDBNotMaster          -20003 /* database not master */
#define JET_errREPSecondGuid           -20004 /* don't put second guid on table */
#define JET_errREPOnlyBuiltin          -20005 /* Can't make non-builtin replicable */
#define JET_errREPNotOwner             -20006 /* Only owner can make DB replicable */
#define JET_errREPCantRelate           -20007 /* Can't relate repl. and local table */

/* Error code constant I invented as it was missing and could not be found */
#define JET_errOpenFile					-5036

/**********************************************************************/
/***********************      PARAMETERS     **************************/
/**********************************************************************/
/* Some #define I made up for 55, as it's not defined in esent.h */
#define JET_paramRegPath	55

/* Possible values for IsamType: Transfer macro ISAM driver applies to */
#define JET_isamTypeDatabase	0
#define JET_isamTypeSpreadsheet	1
#define JET_isamTypeText		2


/**********************************************************************/
/*********************** DIFFERENT STRUCTURES *************************/
/**********************************************************************/
/* There seem to be some old versions of the structures in esent.h being
 * used  by the IISAM drivers, these are my assumptions about their 
 * structure */
typedef struct 
{
  unsigned long cbStruct;
  JET_TABLEID tableid;
  unsigned long cRecord;
  JET_COLUMNID columnidPresentationOrder;
  JET_COLUMNID columnidcolumnname;
  JET_COLUMNID columnidcolumnid;
  JET_COLUMNID columnidcoltyp;
  JET_COLUMNID columnidLangid;
  JET_COLUMNID columnidcbMax;
  JET_COLUMNID columnidgrbit;
  JET_COLUMNID columnidCp;
  JET_COLUMNID columnidBaseTableName;
  JET_COLUMNID columnidBaseColumnName;
  JET_COLUMNID columnidDefinitionName;
} JET_COLUMNLIST_OLD;

typedef struct {
  unsigned long cbStruct;
  JET_TABLEID tableid;
  unsigned long cRecord;
  JET_COLUMNID columnidindexname;
  JET_COLUMNID columnidgrbitIndex;
  JET_COLUMNID columnidcKey;
  JET_COLUMNID columnidcEntry;
  JET_COLUMNID columnidcPage;
  JET_COLUMNID columnidcColumn;
  JET_COLUMNID columnidiColumn;
  JET_COLUMNID columnidcolumnid;
  JET_COLUMNID columnidcoltyp;
  JET_COLUMNID columnidLangid;
  JET_COLUMNID columnidgrbitColumn;
  JET_COLUMNID columnidcolumnname;
} JET_INDEXLIST_OLD;

typedef struct
	{
	unsigned long	cbStruct;
	JET_COLUMNID	columnid;
	JET_COLTYP		coltyp;
	unsigned short	wCountry;
	unsigned short	langid;
	unsigned long	cbMax;
	JET_GRBIT		grbit;
	WCHAR			szBaseTableName[256];
	WCHAR			szBaseColumnName[256];
} JET_COLUMNBASE_OLD;

typedef struct {
  unsigned long cbStruct;	// +0
  JET_COLUMNID columnid;	// +4
  JET_COLTYP coltyp;		// +8
  unsigned short wCountry;	// +12
  unsigned short langid;	// +14
  unsigned long cbMax;		// +16
  JET_GRBIT grbit;			// +20
} JET_COLUMNDEF_OLD;

/* If I'm correct, there is a bug in the esent.h header file regarding
 * the info levels for JetGetColumnInfo and JetGetTableColumnInfo.
 * In my opinion, the JET_ColInfoListCompact is 2 and not 5, whereas
 * 5 is a special structure that contains the column formatting info.
 * Otherwise the implementation in the Text IISAM driver wouldn't make
 * any sense.
 */
#undef JET_ColInfoListCompact
#define JET_ColInfoListCompact	2U
#define JET_ColInfoColumnFormat	5U

typedef struct
{
	unsigned long cbStruct;
	unsigned long dwUnk1;
	WCHAR szColumnFormat[256];
} JET_COLUMNFORMAT;

/**********************************************************************/
/*********************** INTERNAL STRUCTURES **************************/
/**********************************************************************/

/* Structures were gathered by Reverse engineering, so correct original
 * naming is unknown. */

typedef struct 
{
	WCHAR wszPfx[2];
	WCHAR wszName[82];
} DRV_NAM;

typedef struct 
{
	DWORD cbResult;		// +0
	DWORD dwUnk4;		// +4
	DWORD dwUnk8;		// +8
	PVOID pResult;		// +12  (JET_OBJECTLIST for Objects, JET_INDEXLIST for indexes)
} OBJINFO; // = JET_ENUMCOLUMN  ?

typedef JET_API_PTR JET_OBJID;
typedef struct {
	DWORD cbSize;
	WCHAR *pDrvNam;	/* Optional, can be NULL */
	JET_ERR (JET_API *IsamCapability)(JET_SESID sesid, JET_DBID dbid, DWORD unk1, DWORD unk2, DWORD *unk3); // ?
	JET_ERR (JET_API *IsamCloseDatabase)(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit);
	JET_ERR (JET_API *IsamConnectionControl)(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit); // ?
	JET_ERR (JET_API *IsamCreateObject)(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit, LPWSTR unk1, short unk2); // ?
	JET_ERR (JET_API *IsamCreateTable)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName, ULONG lPages, 
							ULONG lDensity, JET_TABLEID	*ptableid);
	JET_ERR (JET_API *IsamDeleteObject)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szObjectName); // ?
	JET_ERR (JET_API *IsamDeleteTable)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName);
	JET_ERR (JET_API *IsamUnknownFunct)(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit); // ?
	JET_ERR (JET_API *IsamGetColumnInfo)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName,
							LPCWSTR szColumnName, OBJINFO *ColumnInfo, ULONG InfoLevel);
	JET_ERR (JET_API *IsamGetDatabaseInfo)(JET_SESID sesid, JET_DBID dbid, PVOID pvResult,
							ULONG cbMax, ULONG InfoLevel);
	JET_ERR (JET_API *IsamGetIndexInfo)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szTableName,
							LPCWSTR szIndexName, OBJINFO *IndexInfo, ULONG InfoLevel );
	JET_ERR (JET_API *IsamGetObjectInfo)(JET_SESID sesid, JET_DBID dbid, JET_OBJTYP	objtyp,
							LPCWSTR szContainerName, LPCWSTR szObjectName, OBJINFO *IndexInfo, 
							ULONG InfoLevel );
	JET_ERR (JET_API *IsamGetReferenceInfo)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szContainerName, 
							LPCWSTR szIndexName, OBJINFO *IndexInfo, JET_OBJTYP	reftyp,
							ULONG InfoLevel ); // ?
	JET_ERR (JET_API *IsamOpenTable)(JET_SESID sesid, JET_DBID dbid, JET_TABLEID *ptableid,
							LPCWSTR szTableName, JET_GRBIT grbit);
	JET_ERR (JET_API *IsamRenameObject)(JET_SESID sesid,	JET_DBID dbid, LPCWSTR szTableName, 
							LPCWSTR szName,	LPCWSTR szNameNew); // ?
	JET_ERR (JET_API *IsamRenameTable)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szName, LPCWSTR szNameNew);
	JET_ERR (JET_API *IsamGetObjidFromName)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szContainerName, 
							LPCWSTR szObjectName, JET_OBJID *objid); // ??
	JET_ERR (JET_API *IsamChangeDbPasswordEx)(JET_SESID sesid, JET_DBID dbid, LPCWSTR szNewPassword, 
							LPCWSTR szOldPassword, JET_GRBIT grbit); // ??
} JET_FNDEFDB;

typedef JET_ERR (JET_API *RECID_CALLBACK) (ULONG param1, ULONG param2, ULONG cRecord);

typedef struct {
	DWORD cbSize;	/* 0xCC */
	WCHAR *pDrvNam;	/* Optional, can be NULL */
	JET_ERR (JET_API *IsamAddColumn)(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szColumnName,
		const JET_COLUMNDEF_OLD	*pcolumndef, const void *pvDefault, ULONG cbDefault,
		JET_COLUMNID *pcolumnid );
	JET_ERR (JET_API *IsamCloseTable)(JET_SESID sesid, JET_TABLEID tableid);
	JET_ERR (JET_API *IsamComputeStats)(JET_SESID sesid, JET_TABLEID tableid);
	JET_ERR (JET_API *IsamCopyBookmarks)(JET_SESID sesid, JET_TABLEID tableid, DWORD unk3, 
		DWORD unk4, DWORD unk5, DWORD *a6, DWORD *a7);
	JET_ERR (JET_API *IsamCreateIndex)(JET_SESID sesid,	JET_TABLEID	tableid, LPCWSTR szIndexName,
		JET_GRBIT grbit, LPCWSTR szKey, ULONG cbKey, ULONG lDensity);
	JET_ERR (JET_API *IsamCreateReference)(JET_SESID sesid, JET_DBID dbid,
		LPCWSTR szReferenceName, LPCWSTR szColumnName, const void *pvData, const ULONG cbData,
		const JET_GRBIT	grbit);
	JET_ERR (JET_API *IsamDelete)(JET_SESID sesid, JET_TABLEID tableid);
	JET_ERR (JET_API *IsamDeleteColumn)(JET_SESID sesid, JET_TABLEID tableid, LPCWSTR szColumnName);
	JET_ERR (JET_API *IsamDeleteIndex)(JET_SESID sesid, JET_TABLEID	tableid,  LPCWSTR szIndexName);
	JET_ERR (JET_API *IsamDeleteReference)(JET_SESID sesid, JET_TABLEID	tableid,  LPCWSTR szReferenceName);
	JET_ERR (JET_API *IsamDupCursor)(JET_SESID sesid, JET_TABLEID tableid, JET_TABLEID *ptableid,
		JET_GRBIT grbit);
	JET_ERR (JET_API *IsamGetBookmark)(JET_SESID sesid, JET_TABLEID	tableid, PVOID pvBookmark,
		ULONG cbMax, PULONG pcbActual);
	JET_ERR (JET_API *IsamGetChecksum)(JET_SESID sesid, JET_TABLEID tableid, PDWORD pdwChecksum);
	JET_ERR (JET_API *IsamGetCurrentIndex)(JET_SESID sesid, JET_TABLEID tableid, LPWSTR szIndexName,
		ULONG cchIndexName );
	JET_ERR (JET_API *IsamGetCursorInfo)(JET_SESID sesid, JET_TABLEID tableid, PVOID pvResult,
		ULONG cbMax, ULONG InfoLevel);
	JET_ERR (JET_API *IsamGetRecordPosition)(JET_SESID sesid, JET_TABLEID tableid, 
		JET_RECPOS *precpos, ULONG cbRecpos);
	JET_ERR (JET_API *IsamGetTableColumnInfo)(JET_SESID	sesid, JET_TABLEID tableid,
		LPCWSTR szColumnName, PVOID pvResult, ULONG cbMax, ULONG InfoLevel);
	JET_ERR (JET_API *IsamGetTableIndexInfo)(JET_SESID sesid, JET_TABLEID tableid,
		LPCWSTR szIndexName, PVOID pvResult, ULONG cbResult, ULONG InfoLevel);
	JET_ERR (JET_API *IsamGetTableInfo)(JET_SESID sesid, JET_TABLEID tableid,
		PVOID pvResult, ULONG cbMax, ULONG InfoLevel);
	JET_ERR (JET_API *IsamGetTableReferenceInfo)(JET_SESID sesid, JET_TABLEID tableid,
		LPCWSTR szIndexName, PVOID pvResult, ULONG cbMax, ULONG InfoLevel);
	JET_ERR (JET_API *IsamGotoBookmark)(JET_SESID sesid, JET_TABLEID tableid,
		PVOID pvBookmark, ULONG cbBookmark);
	JET_ERR (JET_API *IsamGotoPosition)(JET_SESID sesid, JET_TABLEID tableid, JET_RECPOS *precpos);
	JET_ERR (JET_API *IsamIdle)(JET_SESID sesid, JET_TABLEID tableid);
	JET_ERR (JET_API *IsamMakeKey)(JET_SESID sesid, JET_TABLEID tableid, const void *pvData,
		ULONG cbData, JET_GRBIT	grbit);
	JET_ERR (JET_API *IsamMove)(JET_SESID sesid, JET_TABLEID tableid, long cRow,
		JET_GRBIT grbit);
	JET_ERR (JET_API *IsamNotifyUpdateUfn)(JET_SESID sesid, JET_TABLEID tableid);
	JET_ERR (JET_API *IsamNotifyRollback)(JET_SESID sesid, JET_TABLEID tableid, int unk3);
	JET_ERR (JET_API *IsamNotifyUnknown)(JET_SESID sesid, JET_TABLEID tableid, int unk3);
	JET_ERR (JET_API *IsamNotifyUnknown2)(JET_SESID sesid, JET_TABLEID tableid);
	JET_ERR (JET_API *IsamPrepareUpdate)(JET_SESID sesid, JET_TABLEID tableid, ULONG prep);
	JET_ERR (JET_API *IsamRenameColumn)(JET_SESID  sesid, JET_TABLEID tableid,
		LPCWSTR szName, LPCWSTR szNameNew);
	JET_ERR (JET_API *IsamRenameIndex)(JET_SESID  sesid, JET_TABLEID tableid,
		LPCWSTR szName, LPCWSTR szNameNew);
	JET_ERR (JET_API *IsamRenameReference)(JET_SESID sesid, JET_TABLEID tableid,
		LPCWSTR szName, LPCWSTR szNameNew);
	JET_ERR (JET_API *IsamRetrieveColumn)(JET_SESID sesid, JET_TABLEID tableid,
		JET_COLUMNID columnid, PVOID pvData, ULONG cbData, ULONG *pcbActual,
		JET_GRBIT grbit, JET_RETINFO *pretinfo);
	JET_ERR (JET_API *IsamRetrieveKey)(JET_SESID sesid, JET_TABLEID tableid, 
		PVOID pvData, ULONG cbMax, ULONG *pcbActual, JET_GRBIT grbit);
	JET_ERR (JET_API *IsamSeek)(JET_SESID sesid, JET_TABLEID tableid, JET_GRBIT grbit);
	JET_ERR (JET_API *IsamSetCurrentIndex)(JET_SESID sesid, JET_TABLEID tableid,
		LPCWSTR szIndexName);
	JET_ERR (JET_API *IsamSetColumn)(JET_SESID sesid, JET_TABLEID tableid,
		JET_COLUMNID columnid, const void *pvData, ULONG cbData,
		JET_GRBIT grbit, JET_SETINFO *psetinfo);
	JET_ERR (JET_API *IsamSetIndexRange)(JET_SESID sesid, JET_TABLEID tableidSrc,
		JET_GRBIT grbit);
	JET_ERR (JET_API *IsamUpdate)(JET_SESID sesid, JET_TABLEID tableid,
		PVOID pvBookmark, ULONG cbBookmark,	ULONG *pcbActual);
	JET_ERR (JET_API *IsamEmptyTable)(JET_SESID sesid, JET_TABLEID tableid,
		PVOID pvBookmark, ULONG cbBookmark,	ULONG *pcbActual);
	JET_ERR (JET_API *IsamCollectRecids)(JET_SESID sesid, JET_TABLEID tableid,
		RECID_CALLBACK callback, ULONG cbParam1, ULONG cbParam2, LPCWSTR unk6,
		int cbunk6, int unk8, int unk9, int unk10, int unk11);
	JET_ERR (JET_API *IsamOpenILockBytes)(JET_SESID sesid, JET_TABLEID tableid,
		JET_COLUMNID columnid, PVOID pvLockBytes, JET_GRBIT grbit);
	JET_ERR (JET_API *IsamNull1)();	
	JET_ERR (JET_API *IsamNull2)();
	JET_ERR (JET_API *IsamDeleteColumn2)(JET_SESID sesid, JET_TABLEID tableid,
		LPCWSTR szColumnName, const JET_GRBIT grbit);
	JET_ERR (JET_API *IsamCopyRecords)(JET_SESID sesid, JET_TABLEID tableid,
		int unk3, int unk4, int unk5, int unk6, int unk7, int unk8);
	JET_ERR (JET_API *IsamModifyColumn)(JET_SESID sesid, JET_TABLEID tableid,
		LPCWSTR szColumnName, int unk4, int unk5);
	JET_ERR (JET_API *IsamReadAhead)(JET_SESID sesid, JET_TABLEID tableid,
		PVOID unk3, int unk4, int unk5);
} JET_FNDEFTBL;


typedef struct /* This is the callback structure to the MS Jet DLL */
{
	JET_TABLEID (JET_API *TableidFromVtid)(JET_TABLEID vtid, JET_FNDEFTBL *pfndef);
	JET_ERR (JET_API *ErrDispGetBookmark2)(JET_SESID sesid, JET_TABLEID	tableid, 
		PVOID pvBookmark, ULONG cbMax, PULONG pcbActual);
	JET_DBID (JET_API *DbidOfVdbid)(JET_SESID sesid, JET_DBID vdbid);
	JET_PFNSTATUS (JET_API *UtilGetpfnStatusOfSesid)(JET_SESID sesid, JET_PFNSTATUS *pStatus);
	JET_ERR (JET_API *ErrDispCloseTable)(JET_SESID sesid, JET_TABLEID tableid);
	JET_ERR (JET_API *ErrDispPrepareUpdate2)(JET_SESID sesid, JET_TABLEID tableid, ULONG prep);
	JET_ERR (JET_API *ErrDispRetrieveColumn2)(JET_SESID	sesid, JET_TABLEID tableid,
		JET_COLUMNID columnid, PVOID pvData, ULONG cbData, ULONG *pcbActual,
		JET_GRBIT grbit, JET_RETINFO *pretinfo);
	JET_ERR (JET_API *ErrGetVtidTableid)(JET_SESID sesid, JET_TABLEID tableid, DWORD *pdwVtid);
	LPWSTR (JET_API *UtilGetNameOfSesid)(JET_SESID sesid, LPWSTR lpNameOut);
	JET_ERR (JET_API *ErrAllocateTableidForVsesid)(JET_SESID sesid, JET_TABLEID *ptableid, JET_HANDLE vtdid, JET_FNDEFTBL *pVtbl);
	JET_ERR (JET_API *ErrAllocateDbid)(JET_DBID *pdbid, JET_HANDLE userid, JET_FNDEFDB *pVtbl);
	LPCWSTR (JET_API *UtilSetErrorInfoReal)(JET_SESID sesid, LPCWSTR lpError1, LPCWSTR lpError2, LPCWSTR lpError3, 
		JET_ERR err1, JET_ERR err2, JET_ERR err3, DWORD unk8);
	JET_ERR (JET_API *ErrUpdateTableid)(JET_TABLEID tableid, JET_HANDLE vtdid, JET_FNDEFTBL *pfndef);
	JET_ERR (JET_API *ErrGetPvtfndefTableid)(JET_SESID sesid, JET_TABLEID tableid, JET_FNDEFTBL **pfndefout);
	JET_ERR (JET_API *ErrDispSetColumn2)(JET_SESID sesid, JET_TABLEID tableid, JET_COLUMNID	columnid,
		const void	*pvData, ULONG cbData, JET_GRBIT grbit, JET_SETINFO	*psetinfo);
	JET_ERR (JET_API *ErrDispSeek2)(JET_SESID sesid, JET_TABLEID tableid, JET_GRBIT	grbit);
	JET_ERR (JET_API *ErrDispUpdate2)( JET_SESID sesid,	JET_TABLEID tableid, PVOID pvBookmark,
		ULONG cbBookmark, PULONG pcbActual);
	JET_TABLEID (JET_API *ReleaseTableid)(JET_TABLEID tableid);
	JET_DBID (JET_API *ReleaseDbid)(JET_DBID dbid);
	JET_ERR (JET_API *ErrDispMove)(JET_SESID sesid,	JET_TABLEID	tableid, long cRow, JET_GRBIT grbit);
	JET_ERR (JET_API *ErrGetSystemParameter)(JET_INSTANCE	instance, JET_SESID	sesid, ULONG paramid,
		JET_API_PTR	*plParam, LPCWSTR sz); // This always returns JET_errInvalidParameter, so this is dummy
	JET_ERR (JET_API *UtilGetErrorInfo)(JET_SESID sesid, LPWSTR *lpErrOut1, LPWSTR *lpErrOut2, 
		LPWSTR *lpErrOut3, BOOL *pbInvalidErrOut1, BOOL *pbInvalidErrOut2, BOOL *pbInvalidErrOut3, 
		DWORD *perr1, DWORD *perr2, DWORD *perr3, JET_ERR *pret);
	JET_ERR (JET_API *ClearErrorInfo)(JET_SESID sesid);
	JET_ERR (JET_API *UtilUpdateErrorInfo)(JET_SESID sesid, UINT uiErrCl1, LPCWSTR pErr1, DWORD cbErr1, 
		UINT uiErrCl2, ULONG ulErr2, JET_ERR err);  /* Errcl = 1-3 */
	JET_ERR (JET_API *NullStubReserved)();
	JET_ERR (JET_API *ErrTLVLoadInfo)(PVOID pTLV);
	JET_ERR (JET_API *ErrTLVZnloadInfo)(PVOID pTLV);
	JET_ERR (JET_API *ErrTLVEvalExpr)(PVOID pUnk1);
	UINT (JET_API *UtilGetProfileInt)(JET_SESID sesid, LPCWSTR lpAppName, LPCWSTR lpKeyName, UINT dwDefault); /* Just returns dwDefault */
	DWORD (JET_API *UtilGetProfileString)(JET_SESID sesid, LPCWSTR lpAppName, LPCWSTR lpKeyName,
		LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize); /* Copies default value */
	JET_INSTANCE (JET_API *InstanceOfItib)(JET_SESID sesid);
	JET_SESID (JET_API *ItibOfInstance)(JET_INSTANCE instance);
	JET_ERR (JET_API *ErrGetSystemParameterInst)(JET_INSTANCE instance, JET_SESID sesid, ULONG paramid,
		JET_API_PTR	*plParam, LPWSTR sz, ULONG cbMax);
} JET_CALLIN;


typedef struct
{
	DWORD dwVersion;		// ?
	JET_ERR (JET_API *IISAMAttachDatabase)(JET_SESID sesid, LPCWSTR szFilename, JET_GRBIT grbit);
	JET_ERR (JET_API *IISAMBackup)(LPCWSTR szBackupPath, JET_GRBIT grbit, JET_PFNSTATUS pfnStatus);
	JET_ERR (JET_API *IISAMBeginSession)(JET_SESID	*psesid);
	JET_ERR (JET_API *IISAMBeginTransaction)( JET_SESID sesid );
	JET_ERR (JET_API *IISAMCommitTransaction)( JET_SESID sesid, JET_GRBIT grbit );
	JET_ERR (JET_API *IISAMCreateDatabase)(JET_SESID sesid, LPWSTR szFilename, LPWSTR szConnect, JET_DBID* pdbid, JET_GRBIT grbit);
	JET_ERR (JET_API *IISAMDetachDatabase)(JET_SESID sesid, LPCWSTR szFilename);
	JET_ERR (JET_API *IISAMEndSession)( JET_SESID sesid, JET_GRBIT grbit );
	JET_ERR (JET_API *IISAMIdle)( JET_SESID sesid, JET_TABLEID tableid );
	JET_ERR (JET_API *IISAMInit)( JET_INSTANCE instance);
	JET_ERR (JET_API *IISAMLoggingOn)(JET_SESID sesid);
	JET_ERR (JET_API *IISAMLoggingOff)(JET_SESID sesid);
	JET_ERR (JET_API *IISAMOpenDatabase)(JET_SESID sesid, LPCWSTR szFilename, LPCWSTR szConnect,
									  JET_DBID *pdbid, JET_GRBIT grbit);
	JET_ERR (JET_API *IISAMOpenTempTable)(JET_SESID sesid, const JET_COLUMNDEF* prgcolumndef, ULONG ccolumn, JET_GRBIT grbit,
		JET_TABLEID* ptableid, JET_COLUMNID* prgcolumnid);
	JET_ERR (JET_API *IISAMRepairDatabase)(JET_SESID sesid, LPCWSTR lpDatabase, JET_PFNSTATUS pfnStatus);
	JET_ERR (JET_API *IISAMRestore)(LPCWSTR szDatabase, JET_PFNSTATUS pfnStatus);
	JET_ERR (JET_API *IISAMRollback)(JET_SESID sesid, JET_GRBIT grbit);
	JET_ERR (JET_API *IISAMSetSystemParameter)(JET_INSTANCE instance, ULONG paramid, JET_API_PTR lParam, LPCWSTR szParam);
	JET_ERR (JET_API *IISAMTerm)();
} IISAM_Vtbl;
