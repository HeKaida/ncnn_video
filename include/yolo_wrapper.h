#ifndef _YOLO_WRAPPER_H
#define _YOLO_WRAPPER_H

#include "convert_manager.h"
#include "simpleocv.h"

#ifdef __cplusplus
	extern "C" {
#endif

int YoloInit(int target_sz);
int YoloDetect(PT_VideoConvert_Buf ptVideoConvert_Buf);
void YoloDrawObjects(void);

#ifdef __cplusplus
}
#endif

#endif
