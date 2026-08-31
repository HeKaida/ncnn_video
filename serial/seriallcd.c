#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "serial.h"
#include "convert_manager.h"
#include "ncnn_wrapper.h"
#include "main.h"
#include "seriallcd.h"


//static pthread_t g_thread_s;
static pthread_t g_thread_r;

static int g_fd;

int SerialLCD_Init(const char *device, int baudrate)
{
	g_fd = Serial_Init(device, baudrate);
	if(g_fd == -1)
	{
		DBG_SPRINTF(stderr, "Serial_Init fail!\n");
		return -1;
	}

	return 0;
}


int SerialLCD_Sned(const unsigned char *buf, int len)
{
	size_t ret;
	
	ret = Serial_Send(g_fd, buf, len);
	if(ret < 0)
	{
		DBG_SPRINTF(stderr, "Serial_Send fail!\n");
		return -1;
	}

	return 0;	
}


int SerialLCD_Read(unsigned char *buf, int max_len, int timeout_ms)
{
	
	int index = 0;
	int ff_count = 0;
	unsigned char ch;
	fd_set fds;
	size_t ret;
	struct timeval tv;

	FD_ZERO(&fds);
    FD_SET(g_fd, &fds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(g_fd + 1, &fds, NULL, NULL, &tv);
    if (ret < 0)
    {
    	DBG_PRINTF("select: %ld %s\n", ret,strerror(errno));
        return -1;
    }
    else if(ret == 0)
    {
		return 0;
    }

	while (index < max_len) 
	{
        ret =Serial_Read(g_fd, &ch, 1);
		if(ret != 1)
		{
			DBG_PRINTF("Serial_Read: %ld\n", ret);
			break;
		}

		buf[index++] = ch;
		if(ch == 0xFF)
		{
			ff_count++;
			if(ff_count == 3)
			{
				return index;
			}
		}
		else
		{
			ff_count = 0;
		}
	}
	
	return -1;
}


/*
static void *SerialLCD_S_thread(void *arg)
{
	int size;
	int objects_label;
	unsigned char number[class_bjects] = {0};
	
	PT_Manager ptManager = (PT_Manager)arg;

	while(1)
	{
		memset(number, 0, sizeof(unsigned char) * class_bjects);
		pthread_mutex_lock(&ptManager->serial_lock);
		pthread_cond_wait(&ptManager->serial_cond, &ptManager->status_lock);

		size = ptManager->tDetectObject.size;
		objects_label = ptManager->tDetectObject.p_Objects->label;
		for(int i = 0; i < size; i++)
		{
			switch(objects_label)
			{
				case Coca_cola_Can_330ml:number[Coca_cola_Can_330ml]++;break;
				case Fanta_Can_330ml:number[Fanta_Can_330ml]++;break;
				case Master_Kong_lced_Black_Tea_500ml:number[Master_Kong_lced_Black_Tea_500ml]++;break;
				case Nongfu_Spring_550ml:number[Nongfu_Spring_550ml]++;break;
				case Pepsi_Can_330ml:number[Pepsi_Can_330ml]++;break;
				case Scream_550ml:number[Scream_550ml]++;break;
				case Sprite_Can_330ml:number[Sprite_Can_330ml]++;break;
				case WALOVI_Can_310ml:number[WALOVI_Can_310ml]++;break;
				
				default:break;
			}
		}
		NCNNCleanObjects(&ptManager->tDetectObject);
		pthread_mutex_unlock(&ptManager->serial_lock);

		SerialLCD_Sned(number, size);
	}
		
	return NULL;
}
*/


static void *SerialLCD_R_thread(void *arg)
{
	int len;

	unsigned char buffer[32] = {0};

	PT_Manager ptManager = (PT_Manager)arg;
	
	while(1)
	{
		len = SerialLCD_Read(&buffer[0], 32, 200);
		if(len > 0)
		{
			if(buffer[0] == 0x65 && buffer[1] == 0x00 && buffer[2] == 0x01 && buffer[3] == 0x00)
			{
				DBG_PRINTF("seriallcd: %d lock status_lock\n", __LINE__);
				pthread_mutex_lock(&ptManager->status_lock);
				
				ptManager->g_stop = 0;
				ptManager->g_camera_sta = 0;
				ptManager->g_consumer_sta = 0;
				ptManager->g_ready_count = 0;

				DBG_PRINTF("seriallcd: %d broadcast status_cond\n", __LINE__);
				pthread_cond_broadcast(&ptManager->status_cond);
				while(ptManager->g_ready_count < 2)
				{
					pthread_cond_wait(&ptManager->status_cond, &ptManager->status_lock);
				}

				DBG_PRINTF("seriallcd: %d unlock status_lock\n", __LINE__);
				pthread_mutex_unlock(&ptManager->status_lock);
			}
			else if(len < 0)
			{
				DBG_SPRINTF(stderr, "65 00 01 00 FF FF FF fail!\n");
			}

			if(buffer[0] == 0x65 && buffer[1] == 0x01 && buffer[2] == 0x07 && buffer[3] == 0x00)
			{
				DBG_PRINTF("seriallcd: %d lock status_lock\n", __LINE__);
				pthread_mutex_lock(&ptManager->status_lock);
				ptManager->g_stop = 1;
				DBG_PRINTF("seriallcd: %d unlock status_lock\n", __LINE__);
				pthread_mutex_unlock(&ptManager->status_lock);
			}
		}
		else if(len == 0)
		{
			continue;
		}
		else if(len < 0)
		{
			DBG_SPRINTF(stderr, "SerialLCD_Read fail! :%s\n", strerror(errno));
		}
	}
	
	return NULL;
}


int SerialLCD_Thread_Start(PT_Manager ptManager)
{
	int iError;

	pthread_mutex_init(&ptManager->status_lock, NULL);
	pthread_cond_init(&ptManager->status_cond, NULL);
	pthread_mutex_init(&ptManager->serial_lock, NULL);
	pthread_cond_init(&ptManager->serial_cond, NULL);

	/*
	iError = pthread_create(&g_thread_s, NULL, SerialLCD_S_thread, (void *)ptManager);
	if(iError != 0)
	{
		return -1;
	}
	*/

	
	iError = pthread_create(&g_thread_r, NULL, SerialLCD_R_thread, (void *)ptManager);
	if(iError != 0)
	{		
		return -1;
	}

	return 0;
}

int SerialLCD_Thread_Join(void)
{
	int iError;

	iError = pthread_join(g_thread_r, NULL);
	if(iError != 0)
	{
		return -1;
	}

	/*
	iError = pthread_join(g_thread_s, NULL);
	if(iError != 0)
	{
		return -1;
	}
*/
	return 0;
}


int SerialLCD_Close(void)
{
	int ret;
	
	ret = Serial_Close(g_fd);
	if(ret < 0)
	{
		DBG_SPRINTF(stderr, "Serial_Close fail!\n");
		return -1;
	}

	return 0;
}
