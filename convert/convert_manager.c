#include <stdio.h>
#include <string.h>

#include "mjpg_to_bgr.h"
#include "convert_manager.h"


static PT_VideoConvert_Opr g_ptConvertOprHead = NULL;

int RegisterConvertOpr(PT_VideoConvert_Opr ptConvertOpr)
{
	PT_VideoConvert_Opr ptTmp;

	if (!g_ptConvertOprHead)
	{
		g_ptConvertOprHead   = ptConvertOpr;
		ptConvertOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptConvertOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext     = ptConvertOpr;
		ptConvertOpr->ptNext = NULL;
	}

	return 0;
}

void ShowConvertOpr(void)
{
	int i = 0;
	PT_VideoConvert_Opr ptTmp = g_ptConvertOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


PT_VideoConvert_Opr GetConvertOpr(char *pcName)
{
	PT_VideoConvert_Opr ptTmp = g_ptConvertOprHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

void ConvertDevicetInit(PT_ConvertDevice ptConvertDevice)
{
	ptConvertDevice->ptOpr = GetConvertOpr("mjpg2bgr");
}

int VideoConvertInit(void)
{
	int iError;

	iError = Mjpeg2bgrInit();

	return iError;
}



