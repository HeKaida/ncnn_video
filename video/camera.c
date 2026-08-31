#include <string.h>

#include <linux/videodev2.h>

#include "config.h"
#include "v4l2.h"
#include "video_manager.h"
#include "main.h"


static pthread_t g_thread;


int Camera_Init(char *device, PT_Manager ptManager)
{
	int iError;

	iError = VideoInit();
	if(iError != 0)
	{
		DBG_SPRINTF(stderr, "VideoInit() fail!\n");
		return -1;
	}

	memset(&ptManager->tVideoDevice, 0, sizeof(T_VideoDevice));
	ptManager->tVideoDevice.iPixelFormat = V4L2_PIX_FMT_MJPEG;
	ptManager->tVideoDevice.iWitdth = 640;
	ptManager->tVideoDevice.iHeight = 480;
	ptManager->tVideoDevice.iVideoBufCnt = NB_BUFFER;
	ptManager->tVideoDevice.tVideoBuf.status = 0;

	iError = VidoeDeviceInit(device, &ptManager->tVideoDevice);
	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "VidoeDeviceInit() for %s fail!\n", device);
		return -1;
	}

	return 0;
}


int Camera_Start(PT_Manager ptManager)
{
	int iError;
	
	iError = ptManager->tVideoDevice.ptOpr->StartDevice(&ptManager->tVideoDevice);
  	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "StartDevice() fail!\n");
		return -1;
	}

	return 0;
}

static void *capture_thread(void *arg)
{
	int iError;
	
	PT_Manager ptManager = (PT_Manager)arg;

	while(1)
	{
		pthread_mutex_lock(&ptManager->status_lock);
	
		while(ptManager->g_stop == 1)
		{
			if(ptManager->g_camera_sta == 1)
			{
				ptManager->g_camera_sta = 0;
				pthread_mutex_unlock(&ptManager->status_lock);
				pthread_exit(&g_thread);
			}
			pthread_cond_wait(&ptManager->status_cond, &ptManager->status_lock);
		}	

		if(ptManager->g_camera_sta == 0)
		{
			ptManager->g_camera_sta = 1;
			ptManager->g_ready_count++;
			pthread_cond_broadcast(&ptManager->status_cond);
		}
		pthread_mutex_unlock(&ptManager->status_lock);
		
		
		iError = ptManager->tVideoDevice.ptOpr->GetFrame(&ptManager->tVideoDevice);
		if(iError < 0)
		{
			DBG_SPRINTF(stderr, "V4L2GetFrame fail exit!\n");
			goto err_exit;
		}

		pthread_mutex_lock(&ptManager->frame_lock);
		
		if(ptManager->tVideoDevice.tVideoBuf.status == 1)
		{
			pthread_mutex_unlock(&ptManager->frame_lock);

			if(ptManager->tVideoDevice.iBuf_last_index != -1)
			{
				iError = V4L2Qbuf(ptManager->tVideoDevice.iFd, ptManager->tVideoDevice.iBuf_last_index);
				if(iError < 0)
				{
					DBG_SPRINTF(stderr, "V4L2Qbuf fail exit!\n");
					goto err_exit;
				}

				continue;
			}
			
		}
		else
		{
			if(ptManager->tVideoDevice.iBuf_last_index != -1)
			{
				ptManager->tVideoDevice.tVideoBuf.status = 1;
				ptManager->tVideoDevice.tVideoBuf.index = g_tVideoLastBuf.index;
				ptManager->tVideoDevice.tVideoBuf.bytesused = g_tVideoLastBuf.bytesused;
				ptManager->tVideoDevice.tVideoBuf.addr = ptManager->tVideoDevice.ptMmapBuf[g_tVideoLastBuf.index].addr;

				pthread_mutex_unlock(&ptManager->frame_lock);

				pthread_cond_signal(&ptManager->frame_cond);
			}
		}		
	}
	

	return NULL;
	
err_exit:

	pthread_mutex_lock(&ptManager->status_lock);
	ptManager->g_stop = 1;
	ptManager->g_camera_sta = 0;
	pthread_mutex_unlock(&ptManager->status_lock);
	
	pthread_exit(&g_thread);	
}


int Camera_Thread_Start(PT_Manager ptManager)
{
	int iError;
	
	
	pthread_mutex_init(&ptManager->frame_lock, NULL);
	pthread_cond_init(&ptManager->frame_cond, NULL);

	
	iError = pthread_create(&g_thread, NULL, capture_thread, (void *)ptManager);
	if(iError != 0)
	{
		return -1;
	}

	return 0;
}


int Camera_Thread_Join(PT_Manager ptManager)
{
	int iError;

	iError = pthread_join(g_thread, NULL);
	if(iError != 0)
	{
		return -1;
	}

	return 0;
}


int Camera_Stop(PT_Manager ptManager)
{
	int iError;
	
	iError = ptManager->tVideoDevice.ptOpr->StopDevice(&ptManager->tVideoDevice);
	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "StopDevice() fail!\n");
		return -1;
	}

	return 0;
}


int Camera_Exit(PT_Manager ptManager)
{
	int iError;

	iError = ptManager->tVideoDevice.ptOpr->ExitDevice(&ptManager->tVideoDevice);
	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "ExitDevice() fail!\n");
		return -1;
	}

	return 0;
}
