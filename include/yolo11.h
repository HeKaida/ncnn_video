#ifndef _YOLO11_H
#define _YOLO11_H


#include "main.h"
#include "convert_manager.h"


#ifdef __cplusplus
	extern "C" {
#endif


int Yolo11_init(const char *param, const char *bin, int target_sz);
int Yolo11_Thread_Start(PT_Manager ptManager);
int Yolo11_Thread_Join(void);
int Yolo11_Detect(PT_VideoConvert_Buf ptVideoConvert_Buf);


#ifdef __cplusplus
}
#endif


#endif
