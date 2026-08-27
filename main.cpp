#include <stdio.h>
#include <string.h>
#include "camera.h"
#include "yolo11.h"
#include "main.h"

T_Manager tManager;



int main(int argc, char **argv)
{
	int iError;
	
	if(argc != 2)
  	{
		fprintf(stderr, "Usage: ./* (/dev/video*)\n");
		return -1;
  	}


	
	iError = Camera_Init(argv[1], &tManager);
	if(iError != 0)
	{
		fprintf(stderr, "Camera_Init() fail!\n");
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

	iError = Yolo11_init("models/yolo11n-int8.param", "models/yolo11n-int8.bin", 640);
	if(iError != 0)
	{
		fprintf(stderr, "Yolo11_init() fail!\n");
		return -1;
	}


	
 	iError = Camera_Start(&tManager);
	if(iError != 0)
	{
		fprintf(stderr, "Camera_Start() fail!\n");
		return -1;
	}

	iError = Camera_Thread_Start(&tManager);
	if(iError != 0)
	{
		fprintf(stderr, "Camera_Thread_Start() fail!\n");
		return -1;
	}

	iError = Yolo11_Thread_Start(&tManager);
	if(iError != 0)
	{
		fprintf(stderr, "Yolo11_Thread_Start() fail!\n");
		return -1;
	}



	iError = Camera_Thread_Join(&tManager);
	if(iError != 0)
	{
		fprintf(stderr, "Camera_Thread_Join() fail!\n");
		return -1;
	}
	
	iError = Yolo11_Thread_Join();
	if(iError != 0)
	{
		fprintf(stderr, "Yolo11_Thread_Join() fail!\n");
		return -1;
	}

	
	iError = Camera_Stop(&tManager);
	if(iError != 0)
	{
		fprintf(stderr, "Camera_Stop() fail!\n");
		return -1;
	}

	iError = Camera_Exit(&tManager);
	if(iError != 0)
	{
		fprintf(stderr, "Camera_Exit() fail!\n");
		return -1;
	}
		
    return 0;
}
