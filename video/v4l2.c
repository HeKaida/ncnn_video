#include "config.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <poll.h>
#include <unistd.h>

#include <linux/types.h>
#include <linux/videodev2.h>

#include "video_manager.h"


T_VideoBuf g_tVideoLastBuf;



/*
	V4L2 init camera device function.	
*/
static int V4L2InitDevice(char *strDevName, PT_VideoDevice ptVideoDevice)
{
	int iFd;
	int iError;
	int fmt_desc_index = 0;
	int frmsize_index  = 0;
	
	struct v4l2_capability       v4l2_cap;
	struct v4l2_fmtdesc          v4l2_fmt_desc;
	struct v4l2_frmsizeenum      v4l2_frmsize;
	struct v4l2_format           v4l2_fmt;
	struct v4l2_streamparm 		 v4l2_parm;
	struct v4l2_requestbuffers   v4l2_reqbufs;
	struct v4l2_buffer           v4l2_buf;


	/*
	1.
		try to open device.
		set file type is readwrite and non blcok.
		set PT_VideoDevice iFd val.
	*/
	iFd = open(strDevName, O_RDWR | O_NONBLOCK);
	if(iFd < 0)
	{
		DBG_SPRINTF(stderr, "Can't open: %s\n", strDevName);
		return -1;
	}
	ptVideoDevice->iFd = iFd;


	/*
	2.
		init struct v4l2_capability.
		check camera device ability is a camera capture device?
		check camera device is streaming device?
	*/
	memset(&v4l2_cap, 0, sizeof(struct v4l2_capability));
	iError = ioctl(iFd, VIDIOC_QUERYCAP, &v4l2_cap);
	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "Failed to query capabilities for %s: %s\n", strDevName, strerror(errno));
		goto err_exit;
	}
	if(!(v4l2_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))	
	{
		DBG_SPRINTF(stderr, "%s is not a video capture device\n", strDevName);
		goto err_exit;
	}
	if(v4l2_cap.capabilities & V4L2_CAP_STREAMING)	
	{
		DBG_PRINTF("%s supports streaming i/o\n", strDevName);
	}
	/*
	if(v4l2_cap.capabilities & V4L2_CAP_READWRITE)	
	{
		DBG_PRINTF("%s supports read i/o\n", strDevName);
	}
	*/


	/*
	3.
		init struct v4l2_fmtdesc.
		check camera device support enum format.
			init struct v4l2_frmsizeenum.
			check camera device support enmu frame sizes.
		set PT_VideoDevice iPixelFormat val.
	*/
	while(1)
	{
		memset(&v4l2_fmt_desc, 0, sizeof(struct v4l2_fmtdesc));
		v4l2_fmt_desc.index = fmt_desc_index;
		v4l2_fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		iError = ioctl(iFd , VIDIOC_ENUM_FMT, &v4l2_fmt_desc);
		if(iError < 0)
		{
			break;
		}
		
		frmsize_index = 0;
		while(1)
		{
			memset(&v4l2_frmsize, 0, sizeof(struct v4l2_frmsizeenum));
			v4l2_frmsize.index = frmsize_index;
			v4l2_frmsize.pixel_format = v4l2_fmt_desc.pixelformat;

			iError = ioctl(iFd, VIDIOC_ENUM_FRAMESIZES, &v4l2_frmsize);
			if(iError == 0)
			{
				DBG_PRINTF("format: %s, %d, framesize %d x%d\n", v4l2_fmt_desc.description, v4l2_fmt_desc.pixelformat, v4l2_frmsize.discrete.width, v4l2_frmsize.discrete.height);
			}
			else if(iError < 0)
			{
				break;
			}
			frmsize_index++;
		}

		if(ptVideoDevice->iPixelFormat & v4l2_fmt_desc.pixelformat)
		{
			DBG_PRINTF("---support pixelformat: %d ---", ptVideoDevice->iPixelFormat);
		}
		
		fmt_desc_index++;
	}


	/*
	4.
		init struct v4l2_format.
		set v4l2 video format mode.
		set PT_VideoDevice iWitdth and iHeight val.
	*/
	memset(&v4l2_fmt , 0, sizeof(struct v4l2_format));
	v4l2_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_fmt.fmt.pix.width = ptVideoDevice->iWitdth;
	v4l2_fmt.fmt.pix.height = ptVideoDevice->iHeight;
	v4l2_fmt.fmt.pix.pixelformat = ptVideoDevice->iPixelFormat;
	v4l2_fmt.fmt.pix.field = V4L2_FIELD_ANY;

	iError = ioctl(iFd, VIDIOC_S_FMT, &v4l2_fmt);
	if(iError == 0)
	{
		DBG_PRINTF("set format: %d, %d success!\n", v4l2_fmt.fmt.pix.width, v4l2_fmt.fmt.pix.height);
	}
	else if(iError < 0)
	{
		DBG_SPRINTF(stderr, "set format: %d, %d fail!\n", v4l2_fmt.fmt.pix.width, v4l2_fmt.fmt.pix.height);
		goto err_exit;
	}
	
	ptVideoDevice->iWitdth = v4l2_fmt.fmt.pix.width;
	ptVideoDevice->iHeight = v4l2_fmt.fmt.pix.height;


	/*
		set camera device frame
	*/
	memset(&v4l2_parm, 0, sizeof(struct v4l2_streamparm));
	v4l2_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_parm.parm.capture.timeperframe.numerator = 1;
    v4l2_parm.parm.capture.timeperframe.denominator = 30;
	iError = ioctl(iFd, VIDIOC_S_PARM, &v4l2_parm);
	if(iError == 0)
	{
		DBG_PRINTF("set param: %d / %d fps success!\n", v4l2_parm.parm.capture.timeperframe.denominator, v4l2_parm.parm.capture.timeperframe.numerator);
	}
	else if(iError < 0)
	{
		DBG_PRINTF("set param: %d / %d fps fail!\n", v4l2_parm.parm.capture.timeperframe.denominator, v4l2_parm.parm.capture.timeperframe.numerator);
	}

	
	/*
	5.
		init struct v4l2_requestbuffers.
		set v4l2 video request buffer is mmap mode
		set T_VideoDevice iVideoBufCnt val.
	*/
	memset(&v4l2_reqbufs, 0, sizeof(struct v4l2_requestbuffers));
	v4l2_reqbufs.count = ptVideoDevice->iVideoBufCnt;
	v4l2_reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_reqbufs.memory = V4L2_MEMORY_MMAP;

	iError = ioctl(iFd, VIDIOC_REQBUFS, &v4l2_reqbufs);	
	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "Unable to allocate buffers\n");
		goto err_exit;
	}

	ptVideoDevice->iVideoBufCnt = v4l2_reqbufs.count;


	/*
	7_1.
		when camera device ability is streaming video.
		calloc struct T_VideoDevice's struct T_V4l2MmapBuf size and numbers.
		check query buffers info.
			storage mmap() return a virtual process address struct info.
		set buffers regist to Idle queue.
	*/
	if(v4l2_cap.capabilities & V4L2_CAP_STREAMING)
	{
		ptVideoDevice->ptMmapBuf = calloc(ptVideoDevice->iVideoBufCnt, sizeof(T_V4l2MmapBuf));

		if(ptVideoDevice->ptMmapBuf == NULL)
		{
			DBG_SPRINTF(stderr, "calloc mmap buffer failed\n");
			goto err_buf;
		}
		
		for(int i = 0; i < ptVideoDevice->iVideoBufCnt; i++)
		{
			memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
			v4l2_buf.index = i;
			v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			v4l2_buf.memory = V4L2_MEMORY_MMAP;
			iError = ioctl(iFd, VIDIOC_QUERYBUF, &v4l2_buf);
			if(iError == 0)
			{
				ptVideoDevice->ptMmapBuf[i].addr = mmap(NULL, v4l2_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, v4l2_buf.m.offset);						
				if(ptVideoDevice->ptMmapBuf[i].addr == MAP_FAILED)
				{
					DBG_SPRINTF(stderr, "Unable to map buffer\n");
					goto err_buf;
				}
				ptVideoDevice->ptMmapBuf[i].length = v4l2_buf.length; 
			}
			else if(iError < 0)
			{
				DBG_SPRINTF(stderr, "Unable to query buffer\n");
				goto err_buf;
			} 

			iError = ioctl(iFd, VIDIOC_QBUF, &v4l2_buf);
			if (iError < 0)
			{
				DBG_SPRINTF(stderr, "Unable to queue buffer\n");
				goto err_buf;
			}
		}
	}


	/*
	7_2.
		NULL
	*/
	/*
	if(v4l2_cap.capabilities & V4L2_CAP_READWRITE)
	{
		memset(ptVideoDevice->T_V4l2MmapBuf[i], 0, sizeof(V4l2MmapBuf));
		ptVideoDevice->iVideoBufCnt = 1;
		ptVideoDevice->T_V4l2MmapBuf[0].length = ptVideoDevice->iWitdth * ptVideoDevice->iHeight * 2
		ptVideoDevice->T_V4l2MmapBuf[0].addr = malloc(ptVideoDevice->T_V4l2MmapBuf[0].length);
	}
	*/

	ptVideoDevice->ptOpr = GetVideoOpr("v4l2");
	
	return 0;


/*
	clean calloc() PT_V4l2MmapBuf cache.
	clean mmap() cache.
	close devic.
*/
err_buf:

	
	for(int i = 0; i < ptVideoDevice->iVideoBufCnt; i++)
	{
		if(ptVideoDevice->ptMmapBuf[i].addr && ptVideoDevice->ptMmapBuf[i].addr != MAP_FAILED)
		{
			munmap(ptVideoDevice->ptMmapBuf[i].addr, ptVideoDevice->ptMmapBuf[i].length);
			ptVideoDevice->ptMmapBuf[i].addr = NULL;
		}
	}

	free(ptVideoDevice->ptMmapBuf);
	ptVideoDevice->ptMmapBuf = NULL;

err_exit:

	close(ptVideoDevice->iFd);
	ptVideoDevice->iFd = -1;
	
	return -1;

}


/*
	V4L2 exit camera device function.	
*/
static int V4L2ExitDevice(PT_VideoDevice ptVideoDevice)
{

	/*
		clean calloc() PT_V4l2MmapBuf cache.
		clean mmap() cache.
		close devic.
	*/
	for(int i = 0; i < ptVideoDevice->iVideoBufCnt; i++)
	{
		if(ptVideoDevice->ptMmapBuf[i].addr && ptVideoDevice->ptMmapBuf[i].addr != MAP_FAILED)
		{
			munmap(ptVideoDevice->ptMmapBuf[i].addr, ptVideoDevice->ptMmapBuf[i].length);
		}
	}
	
	free(ptVideoDevice->ptMmapBuf);
	ptVideoDevice->ptMmapBuf = NULL;
	
	close(ptVideoDevice->iFd);
	ptVideoDevice->iFd = -1;

	return 0;
}


int V4L2Qbuf(int fd, int last_index)
{
	int iRet;

	struct v4l2_buffer v4l2_relbuf;

	memset(&v4l2_relbuf, 0, sizeof(struct v4l2_buffer));
	v4l2_relbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_relbuf.memory = V4L2_MEMORY_MMAP;
    v4l2_relbuf.index = last_index;
    iRet = ioctl(fd, VIDIOC_QBUF, &v4l2_relbuf);
	if(iRet < 0)
	{
		DBG_SPRINTF(stderr, "Unable to insert queue buffer\n");
		return -1;
	}	

	return 0;
}

/*
	v4l2 get camera device frame from completed queue function
*/
static int V4L2GetFrame(PT_VideoDevice ptVideoDevice)
{
	int iRet;
	
	struct v4l2_buffer v4l2_buf;
	struct pollfd fds[1];


	memset(&g_tVideoLastBuf, 0, sizeof(T_VideoBuf));

	ptVideoDevice->iBuf_last_index = -1;

	fds[0].fd = ptVideoDevice->iFd;
	fds[0].events = POLLIN;
	iRet = poll(fds, 1, -1);
	if(iRet < 0)
	{
		DBG_SPRINTF(stderr, "Upoll error!\n");
		return -1;
	}

	while(1)
	{
		memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
		v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2_buf.memory = V4L2_MEMORY_MMAP;
		iRet = ioctl(ptVideoDevice->iFd, VIDIOC_DQBUF, &v4l2_buf);
		if(iRet < 0)
		{
			break;
		}

		if (ptVideoDevice->iBuf_last_index != -1)
		{
			iRet = V4L2Qbuf(ptVideoDevice->iFd, ptVideoDevice->iBuf_last_index);
			if(iRet < 0)
			{
				DBG_SPRINTF(stderr, "V4L2Qbuf fail!\n");
				return -1;
			}	
		}

		ptVideoDevice->iBuf_last_index = v4l2_buf.index;
		g_tVideoLastBuf.index = v4l2_buf.index;
		g_tVideoLastBuf.bytesused = v4l2_buf.bytesused;
		g_tVideoLastBuf.addr = ptVideoDevice->ptMmapBuf[v4l2_buf.index].addr;
	}

	return 0;
}


static int V4L2ReleaseFrame(PT_VideoDevice ptVideoDevice)
{
	int iRet;
	struct v4l2_buffer v4l2_buf;

	memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
	v4l2_buf.index = ptVideoDevice->tVideoBuf.index;
	v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf.memory = V4L2_MEMORY_MMAP;
	
	iRet = ioctl(ptVideoDevice->iFd, VIDIOC_QBUF, &v4l2_buf);
	if(iRet < 0)
	{
		DBG_SPRINTF(stderr, "Unable to dequeue buffer\n");
		return -1;
	}	

	return 0;
}


/*
	v4l2 open camera device function
*/
static int V4L2StartDevice(PT_VideoDevice ptVideoDevice)
{
	int iError;
	int type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;

	iError = ioctl(ptVideoDevice->iFd, VIDIOC_STREAMON, &type);
	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "Unable to start capture\n");
		return -1;
	}

	return 0;
}


/*
	v4l2 stop camera device function
*/
static int V4L2StopDevice(PT_VideoDevice ptVideoDevice)
{
	int iError;
	int type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;

	iError = ioctl(ptVideoDevice->iFd, VIDIOC_STREAMOFF, &type);
	if(iError < 0)
	{
		DBG_SPRINTF(stderr, "Unable to stop capture\n");
		return -1;
	}

	return 0;
}



/*
	Describe struct T_VideoOpr functions
*/
static T_VideoOpr g_tV4L2VideoOpr = 
{
	.name         	 = "v4l2",
	.InitDevice   	 = V4L2InitDevice,
	.ExitDevice   	 = V4L2ExitDevice,
	.GetFrame     	 = V4L2GetFrame,
	.ReleaseFrame 	 = V4L2ReleaseFrame,
	.StartDevice  	 = V4L2StartDevice,
	.StopDevice   	 = V4L2StopDevice,
};


/*
	v4l2 init function
*/
int V4l2Init(void)
{
	return RegisterVideoOpr(&g_tV4L2VideoOpr);
}   
