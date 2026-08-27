#ifndef _NCNN_H
#define _NCNN_H


#include "simpleocv.h"
#include "net.h"


struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};


class Yolo11Detector
{
public:
    Yolo11Detector();
    ~Yolo11Detector();

    int init(const std::string& param_path, const std::string& bin_path, const int target_size);

    int detect(const cv::Mat& bgr, std::vector<Object>& objects);

    void draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects);

private:
    ncnn::Net yolo11;    
    bool is_initialized; 
    int target_size;     
    float prob_threshold;
    float nms_threshold; 
};


#endif
