#ifndef _VIDOE_MANAGER_H
#define _VIDOE_MANAGER_H


/*
	set requst buffers count
*/
#define NB_BUFFER 4


#ifdef __cplusplus
extern "C" {
#endif


/*
	declaration struct info
*/
struct VideoDevice;
struct VideOpr;

typedef struct VideoDevice T_VideoDevice, *PT_VideoDevice;
typedef struct VideoOpr T_VideoOpr, *PT_VideoOpr;


/*
	storage mmap() return a virtual process address struct info.
		size_t:	(virtual process address length)
		void *:	(virtual process address)
*/
typedef struct V4l2MmapBuf
{
	size_t length;	

	void *addr;
}T_V4l2MmapBuf, *PT_V4l2MmapBuf;


/*
	storage camera device a frame dates.
		int (camera device format)

		T_PixelDatas (camera device frame data struct info)
*/
typedef struct VideoBuf
{
	int index;
	int status; 
	
	size_t bytesused;
	
	void *addr; 
}T_VideoBuf, *PT_VideoBuf;


/*
	description video device struct info.
		int (video device fd)
		int (video device format)
		int (video device frame size witdth)
		int (video device frame size heigth)
		int (video device reques buffer counts)
		int (video device buffer index)

		struct VideOpr PT_VideoOpr (video device operatopm functions)

		struct V4l2MmapBuf PT_V4l2MmapBuf(voide device request buffer struct info)
*/
struct VideoDevice
{
	int iFd;
	int iPixelFormat;
	int iWitdth;
	int iHeight;
	int iVideoBufCnt;
	int iBuf_last_index;
	
	T_VideoBuf tVideoBuf;
	
	PT_VideoOpr ptOpr;
	PT_V4l2MmapBuf ptMmapBuf;	
};


/*
	description video device operation functions.
		char (name)

		int (init device function)	
		int (exit device function)
		int (get device frame function)
		int	(put device frame function)
		int (open device function)
		int (stop device function)

		struct (video device struct operation link list)
*/
struct VideoOpr
{
	char *name;
	
	int 		(*InitDevice)(char *strDevName, PT_VideoDevice ptVideoDevice);
	int 		(*ExitDevice)(PT_VideoDevice ptVideoDevice);
	int 		(*GetFrame)(PT_VideoDevice ptVideoDevice);
	int 		(*ReleaseFrame)(PT_VideoDevice ptVideoDevice);
	int 		(*StartDevice)(PT_VideoDevice ptVideoDevice);
	int 		(*StopDevice)(PT_VideoDevice ptVideoDevice);

	struct VideoOpr *ptNext;
};


int RegisterVideoOpr(PT_VideoOpr ptVideoOpr);
void ShowVideoOpr(void);
PT_VideoOpr GetVideoOpr(const char *pcName);
int VideoInit(void);
int VidoeDeviceInit(char *strDevName, PT_VideoDevice ptVideoDevice);


#ifdef __cplusplus
}
#endif


#endif
