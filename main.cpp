#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "video_manager.h"
#include "convert_manager.h"
#include "yolo_wrapper.h"

typedef struct Manager
{
	pthread_t thread;

	T_VideoDevice tVideoDevice;
	T_ConvertDevice tConvertDevice;
	T_VideoConvert_Buf tVideoConvert_Buf;
}T_Manager, *PT_Manager;

static void *Consumer_thread(void *arg)
{
	int iError;
	
	struct timespec start, end;
	double elapsed;

	PT_Manager ptManager = (PT_Manager)arg;
	

	pthread_mutex_lock(&ptManager->tVideoDevice.frame_lock);
	while(ptManager->tVideoDevice.status)
	{
		pthread_cond_wait(&ptManager->tVideoDevice.frame_cond, &ptManager->tVideoDevice.frame_lock);
		
		memset(&ptManager->tVideoConvert_Buf, 0, sizeof(T_VideoConvert_Buf));
		
		clock_gettime(CLOCK_MONOTONIC, &start); 
		ptManager->tConvertDevice.ptOpr->Convert(&ptManager->tVideoDevice, &ptManager->tVideoConvert_Buf);

		pthread_mutex_unlock(&ptManager->tVideoDevice.frame_lock);
		
		iError = YoloDetect(&ptManager->tVideoConvert_Buf);
		if(iError != 0)
		{
			fprintf(stderr, "YoloDetect() fail!\n");
			break;
		}
		YoloDrawObjects();
		clock_gettime(CLOCK_MONOTONIC, &end);
		
		elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    	printf("Elapsed time: %.6f seconds\n", elapsed);
    	
		free(ptManager->tVideoConvert_Buf.rgb_out);
        ptManager->tVideoConvert_Buf.rgb_out = NULL;
		
		
		
		
		iError = ptManager->tVideoDevice.ptOpr->ReleaseFrame(&ptManager->tVideoDevice);
		if(iError < 0)
		{
			fprintf(stderr, "GetFrame() fail!\n");
			break;
		}

		pthread_mutex_lock(&ptManager->tVideoDevice.frame_lock);
		ptManager->tVideoDevice.tVideoBuf.status = 0;
		//ptManager->tVideoDevice.status = 0;
		pthread_mutex_unlock(&ptManager->tVideoDevice.frame_lock);	
	}

	return NULL;
}

int main(int argc, char **argv)
{
	int iError;
	T_Manager tManager;
	
	
    
  	if(argc != 2)
  	{
		fprintf(stderr, "Usage: ./* (/dev/video*)\n");
		return -1;
  	}

	iError = VideoInit();
	if(iError != 0)
	{
		fprintf(stderr, "VideoInit() fail!\n");
		return -1;
	}
 	
	memset(&tManager.tVideoDevice, 0, sizeof(T_VideoDevice));
	tManager.tVideoDevice.iPixelFormat = V4L2_PIX_FMT_MJPEG;
	tManager.tVideoDevice.iWitdth = 640;
	tManager.tVideoDevice.iHeight = 480;
	tManager.tVideoDevice.iVideoBufCnt = NB_BUFFER;
	tManager.tVideoDevice.status = 1;
	tManager.tVideoDevice.tVideoBuf.status = 0;

	iError = VidoeDeviceInit(argv[1], &tManager.tVideoDevice);
	if(iError < 0)
	{
		fprintf(stderr, "VidoeDeviceInit() for %s fail!\n", argv[1]);
		return -1;
	}

	memset(&tManager.tConvertDevice, 0, sizeof(T_ConvertDevice));
	iError = VideoConvertInit();
	if(iError != 0)
	{
		fprintf(stderr, "VideoConvertInit() fail!\n");
		return -1;
	}
	ConvertDevicetInit(&tManager.tConvertDevice);

	iError = YoloInit(640);
	if(iError != 0)
	{
		fprintf(stderr, "VideoInit() fail!\n");
		return -1;
	}

	iError = tManager.tVideoDevice.ptOpr->StartDevice(&tManager.tVideoDevice);
  	if(iError < 0)
	{
		fprintf(stderr, "StartDevice() fail!\n");
		return -1;
	}

	pthread_create(&tManager.thread, NULL, Consumer_thread, (void *)&tManager);
	pthread_join(tManager.thread, NULL);
	
	iError = tManager.tVideoDevice.ptOpr->StopDevice(&tManager.tVideoDevice);
	if(iError < 0)
	{
		fprintf(stderr, "StopDevice() fail!\n");
		return -1;
	}

	iError = tManager.tVideoDevice.ptOpr->ExitDevice(&tManager.tVideoDevice);
	if(iError < 0)
	{
		fprintf(stderr, "ExitDevice() fail!\n");
		return -1;
	}
	
    return 0;
}

