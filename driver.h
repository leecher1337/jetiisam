#define DRIVER_NAME		L"Dummy"			/* Set this to your IISAM driver name */
#define DRIVER_DESC		L"Dummy driver"		/* Set this to your driver description */
#define DRIVER_FILE_EXT	{L".DF", L".IF", L".MF", NULL}	/* Possible file extenions for a table, first is data file */
#define DRIVER_MAX_DS	0x80000000			/* Set this to the maximum size of a dataset */

#ifdef __SPEC_H__
JET_ERR DriverInitialize(IISAM_SETTINGS *pSettings);
void DriverTerminate();
#endif
JET_ERR JET_API IsamCloseDatabase(JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit);
