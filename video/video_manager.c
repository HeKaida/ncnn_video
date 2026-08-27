#include <stdio.h>
#include <string.h>
#include "video_manager.h"
#include "v4l2.h"


/*
	definit global video operation link list head. 
*/
static PT_VideoOpr g_ptVideoOprHead = NULL;


/*
	add link list function.
*/
int RegisterVideoOpr(PT_VideoOpr ptVideoOpr)
{
	PT_VideoOpr ptTmp;

	if (!g_ptVideoOprHead)
	{
		g_ptVideoOprHead   = ptVideoOpr;
		ptVideoOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptVideoOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext     = ptVideoOpr;
		ptVideoOpr->ptNext = NULL;
	}

	return 0;
}


/*
	print link list all operation name info function.
*/
void ShowVideoOpr(void)
{
	int i = 0;
	PT_VideoOpr ptTmp = g_ptVideoOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


/*
	get link list operation name the pointr struct PT_VideoOpr function.
*/
PT_VideoOpr GetVideoOpr(const char *pcName)
{
	PT_VideoOpr ptTmp = g_ptVideoOprHead;
	
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


int VidoeDeviceInit(char *strDevName, PT_VideoDevice ptVideoDevice)
{
	int iError;

	PT_VideoOpr ptTmp = g_ptVideoOprHead;

	while (ptTmp)
	{
		iError = ptTmp->InitDevice(strDevName, ptVideoDevice);
		if(!iError)
		{
			return 0;
		}
		ptTmp = ptTmp->ptNext;
	}

	return -1;
}


/*
	init video device function.
*/
int VideoInit(void)
{
	int iError;

	iError = V4l2Init();

	return iError;
}
