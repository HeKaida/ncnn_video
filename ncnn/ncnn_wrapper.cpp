#include <string.h>
#include <vector>
#include <stdlib.h>

#include "config.h"
#include "ncnn_wrapper.h"
#include "ncnn.h"
#include "convert_manager.h"
#include "main.h"


static Yolo11Detector detector;
static cv::Mat bgr;
static std::vector<Object> objects;


int NCNNInit(const char *param, const char *bin, int target_sz)
{
	
	
	std::string param_path = param;
	std::string bin_path   = bin;
	
	if (detector.init(param_path, bin_path, target_sz) != 0)
    {
        DBG_SPRINTF(stderr, "YOLO11 init failed\n");
        return -1;
    }

	return 0;
}


int NCNNDetect(PT_VideoConvert_Buf ptVideoConvert_Buf)
{
	bgr = cv::Mat(ptVideoConvert_Buf->iHeight,
            ptVideoConvert_Buf->iWidth,
            CV_8UC3,
            ptVideoConvert_Buf->rgb_out);

    objects.clear();

	if (detector.detect(bgr, objects) != 0) {
        DBG_SPRINTF(stderr, "YOLO11 detect failed\n");
        
        return -1;
    }
    
	for (size_t i = 0; i < objects.size(); i++)
    {
		const Object& obj = objects[i];
    	
    	fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
                obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);
    }

    return 0;
}


int NCNNStatistics(size_t *size, PT_DetectObject ptDetectObject)
{
	*size = objects.size();
	
	ptDetectObject->p_Objects = (PT_Objects)calloc(*size, sizeof(T_Objects));
	if(ptDetectObject->p_Objects == NULL)
	{
		return -1;
	}
	for (size_t i = 0; i < *size; i++)
    {
		const Object& obj = objects[i];

    	ptDetectObject->p_Objects[i].label = obj.label;
    	ptDetectObject->p_Objects[i].prob = obj.prob;
		fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
                obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);
    }

    return 0;
}

void NCNNCleanObjects(PT_DetectObject ptDetectObject)
{
	free(ptDetectObject->p_Objects);
}

void NCNNDrawObjects(void)
{
	detector.draw_objects(bgr, objects);
}
