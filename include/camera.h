#ifndef _CAMERA_H
#define _CAMERA_H


#include "main.h"


#ifdef __cplusplus
extern "C" {
#endif


int Camera_Init(char *device, PT_Manager ptManager);
int Camera_Start(PT_Manager ptManager);
int Camera_Thread_Start(PT_Manager ptManager);
int Camera_Thread_Join(PT_Manager ptManager);
int Camera_Stop(PT_Manager ptManager);
int Camera_Exit(PT_Manager ptManager);


#ifdef __cplusplus
}
#endif


#endif
