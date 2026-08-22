#include "config.h"
#include "yolo_wrapper.h"
#include "yolo11_int8.h"
#include "convert_manager.h"


#include <vector>

static Yolo11Detector detector;
static cv::Mat bgr;
static std::vector<Object> objects;

int YoloInit(int target_sz)
{
	
	
	std::string param_path = "models/yolo11n-int8.param";
	std::string bin_path   = "models/yolo11n-int8.bin";
	
	if (detector.init(param_path, bin_path, target_sz) != 0)
    {
        DBG_SPRINTF(stderr, "YOLO11 init failed\n");
        return -1;
    }

	return 0;
}

int YoloDetect(PT_VideoConvert_Buf ptVideoConvert_Buf)
{
	bgr = cv::Mat(ptVideoConvert_Buf->iHeight,
            ptVideoConvert_Buf->iWidth,
            CV_8UC3,
            ptVideoConvert_Buf->rgb_out);

	cv::imwrite("debug_raw_1280.jpg", bgr); 

    objects.clear();

	if (detector.detect(bgr, objects) != 0) {
        DBG_SPRINTF(stderr, "YOLO11 detect failed\n");
        
        return -1;
    }
    
    return 0;
}

void YoloDrawObjects(void)
{
	detector.draw_objects(bgr, objects);
}
