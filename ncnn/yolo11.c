#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>


#include "config.h"
#include "ncnn_wrapper.h"
#include "convert_manager.h"
#include "main.h"

static pthread_t g_thread;

int Yolo11_init(const char *param, const char *bin, int target_sz)
{
	int iError;
	
	iError = NCNNInit(param, bin, target_sz);
	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "NCNNInit fail!\n");
		return -1;
	}

	return 0;
}

static void *Consumer_thread(void *arg)
{
	int iError;
	
	struct timespec start, end;
	double elapsed;

	PT_Manager ptManager = (PT_Manager)arg;
	
	while(!ptManager->g_stop)
	{
		memset(&ptManager->tVideoConvert_Buf, 0, sizeof(T_VideoConvert_Buf));
		
		pthread_mutex_lock(&ptManager->frame_lock);
		while(ptManager->tVideoDevice.tVideoBuf.status == 0)
		{
			pthread_cond_wait(&ptManager->frame_cond, &ptManager->frame_lock);
		}
		
		clock_gettime(CLOCK_MONOTONIC, &start); 
		ptManager->tConvertDevice.ptOpr->Convert(&ptManager->tVideoDevice, &ptManager->tVideoConvert_Buf);
		iError = ptManager->tVideoDevice.ptOpr->ReleaseFrame(&ptManager->tVideoDevice);
		if(iError < 0)
		{
			DBG_SPRINTF(stderr, "ReleaseFrame fail!\n");
			goto ReleaseFrame_err_exit;
		}
		pthread_mutex_unlock(&ptManager->frame_lock);
		

		
		iError = NCNNDetect(&ptManager->tVideoConvert_Buf);
		if(iError != 0)
		{
			DBG_SPRINTF(stderr, "NCNNDetect fail!\n");
			goto ncnn_err_exit;
		}

		clock_gettime(CLOCK_MONOTONIC, &end);
		
		elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    	printf("Elapsed time: %.6f seconds\n", elapsed);
    	
		free(ptManager->tVideoConvert_Buf.rgb_out);
        ptManager->tVideoConvert_Buf.rgb_out = NULL;
		
		
		pthread_mutex_lock(&ptManager->frame_lock);
		ptManager->tVideoDevice.tVideoBuf.status = 0;
		pthread_mutex_unlock(&ptManager->frame_lock);	
	}

	return NULL;
	
ncnn_err_exit:

	pthread_mutex_lock(&ptManager->frame_lock);

ReleaseFrame_err_exit:

	ptManager->tVideoDevice.tVideoBuf.status = 0;
	pthread_mutex_unlock(&ptManager->frame_lock);

	free(ptManager->tVideoConvert_Buf.rgb_out);
	ptManager->g_stop = 1;

	pthread_exit(&g_thread);
}


int Yolo11_Thread_Start(PT_Manager ptManager)
{
	int iError;
	
	iError = pthread_create(&g_thread, NULL, Consumer_thread, (void *)ptManager);
	if(iError != 0)
	{
		return -1;
	}

	return 0;
}

int Yolo11_Thread_Join(void)
{
	int iError;

	iError = pthread_join(g_thread, NULL);
	if(iError != 0)
	{
		return -1;
	}

	return 0;
}


int Yolo11_Detect(PT_VideoConvert_Buf ptVideoConvert_Buf)
{
	int iError;
	
	iError = NCNNDetect(ptVideoConvert_Buf);
	if(iError < 0)
	{
		return -1;
	}

	return 0;
}
