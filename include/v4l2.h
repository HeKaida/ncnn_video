#ifndef _V4L2_H
#define _V4L2_H


#include "video_manager.h"


extern T_VideoBuf g_tVideoLastBuf;

int V4l2Init(void);
int V4L2Qbuf(int fd, int last_index);

#endif