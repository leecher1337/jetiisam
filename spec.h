#ifndef __SPEC_H__
#define __SPEC_H__

#include "config.h"

struct tag_isam_settings;
typedef struct tag_isam_settings IISAM_SETTINGS;

/* Here are your settings, also define the approrpiate registry keys to read in spec.c */
struct tag_isam_settings
{
	BOOL bTrace;
	WCHAR szTraceFile[1025];
};

BOOL InitializeSpec(IISAM_SETTINGS *pSettings);
IISAM_CFG *ConfigInit(IISAM_SETTINGS *pSettings);

#endif