#ifndef _MAIN_H
#define _MAIN_H


#include <pthread.h>

#include "video_manager.h"
#include "convert_manager.h"
#include "ncnn_wrapper.h"

typedef struct Manager
{
	volatile int g_stop;
	volatile int g_camera_sta;
	volatile int g_consumer_sta;
	volatile int g_ready_count;
	
	pthread_mutex_t frame_lock;
	pthread_cond_t frame_cond;

	pthread_mutex_t status_lock;
	pthread_cond_t status_cond;

	pthread_mutex_t serial_lock;
	pthread_cond_t serial_cond;
	
	T_VideoDevice tVideoDevice;
	T_ConvertDevice tConvertDevice;
	T_VideoConvert_Buf tVideoConvert_Buf;
	T_DetectObject tDetectObject;
	
}T_Manager, *PT_Manager;


#endif
