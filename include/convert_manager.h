#ifndef _CONVERT_MANAGER_H
#define _CONVERT_MANAGER_H


#include "video_manager.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct VideoConvert_Buf
{
	int iWidth;
	int iHeight;

	void *rgb_out;
}T_VideoConvert_Buf, *PT_VideoConvert_Buf;


typedef struct VideoConvert_Opr
{
	char *name;
	
	int (*Convert)(PT_VideoDevice ptVideoDeviceIn, PT_VideoConvert_Buf ptVideoConvert_Buf);

	struct VideoConvert_Opr *ptNext;
}T_VideoConvert_Opr, *PT_VideoConvert_Opr;


typedef struct ConvertDevice
{
	PT_VideoConvert_Opr ptOpr;
}T_ConvertDevice, *PT_ConvertDevice;


int RegisterConvertOpr(PT_VideoConvert_Opr ptConvertOpr);
void ShowConvertOpr(void);
PT_VideoConvert_Opr GetConvertOpr(char *pcName);
void ConvertDevicetInit(PT_ConvertDevice ptConvertDevice);
int VideoConvertInit(void);


#ifdef __cplusplus
}
#endif


#endif
