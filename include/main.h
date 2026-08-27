#ifndef _MAIN_H
#define _MAIN_H


#include <pthread.h>

#include "video_manager.h"
#include "convert_manager.h"


typedef struct Manager
{
	pthread_mutex_t frame_lock;
	pthread_cond_t frame_cond;

	T_VideoDevice tVideoDevice;
	T_ConvertDevice tConvertDevice;
	T_VideoConvert_Buf tVideoConvert_Buf;
}T_Manager, *PT_Manager;


#endif
