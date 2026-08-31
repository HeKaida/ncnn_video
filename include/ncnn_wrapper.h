#ifndef _NCNN_WRAPPER_H
#define _NCNN_WRAPPER_H


#include "convert_manager.h"
#include "aio.h"

#define class_bjects 8


#ifdef __cplusplus
	extern "C" {
#endif


typedef struct Objects
{	
		int label;
		
		float prob;
}T_Objects, *PT_Objects;

typedef struct DetectObject
{	
		size_t size;
		
		PT_Objects p_Objects;
}T_DetectObject, *PT_DetectObject;


int NCNNInit(const char *param, const char *bin, int target_sz);
int NCNNDetect(PT_VideoConvert_Buf ptVideoConvert_Buf);
void NCNNDrawObjects(void);
int NCNNStatistics(size_t *size, PT_DetectObject ptDetectObject);
void NCNNCleanObjects(PT_DetectObject ptDetectObject);




#ifdef __cplusplus
}
#endif


#endif
