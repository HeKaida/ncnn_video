#ifndef _NCNN_WRAPPER_H
#define _NCNN_WRAPPER_H


#include "convert_manager.h"


#ifdef __cplusplus
	extern "C" {
#endif


int NCNNInit(const char *param, const char *bin, int target_sz);
int NCNNDetect(PT_VideoConvert_Buf ptVideoConvert_Buf);
void NCNNDrawObjects(void);


#ifdef __cplusplus
}
#endif


#endif
