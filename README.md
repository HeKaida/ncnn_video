# 端侧AI视觉识别终端

![C](https://img.shields.io/badge/C-17-blue.svg?style=flat-square&logo=c)
![C++](https://img.shields.io/badge/C++-11-blue.svg?style=flat-square&logo=c%2B%2B)
![Platform](https://img.shields.io/badge/Platform-Ubuntu_22.04-orange.svg?style=flat-square&logo=ubuntu)
![NCNN](https://img.shields.io/badge/AI-NCNN-green.svg?style=flat-square)

## 📖项目简介
目前该项目主要是嵌入式Linux端侧AI部署识别散件物资(目前使用自己训练的YOLO11基于饮料数据集目标检测‌模型)终端项目，由于目前搭载瑞芯微rk3588芯片平台开发板价值十分昂贵，严重溢价，于是计划暂时尝试将端侧AI功能部署在我手里21年买的香橙派Zero 2 1G开发板。该项目重要是采用多线程加生产者与消费者模式框架，为了适配该开发板性能导致生产者快-消费者慢的问题：V4L2摄像头采集策略重新构建Buffer队列管理和最新帧处理，并不太适合实时采集并识别推理的高性能平台摄像头采集策略；转码和视觉识别采用libjpeg-turbo开源转码工具进行.JPG文件转码成.BGR文件和NCNN推理框架使用官方YOLO11模型运行实例文件进行封装。以及目前暂时使用HMI串口协议的陶晶驰32寸电阻触摸屏进行人机交互和数据显示和SQLite3数据库进行本地存储视觉识别记录数据。同时，项目中摄像头和转码代码借鉴采用韦东山先生嵌入式Linux相机项目的框架来进行管理。

> ⚠️ **项目状态：开发中**
> 由于目前该项目整体构思和框架都是通过我一个人在开发实践过程中遇到不同问题时不断修改优化可行和高效的解决方案，所以目前项目整体框架不一定是最优的，而且可能还有隐藏的弊端或者Bug还没有来得及发现处理。目前最新上传的代码能暂时稳定运行，持续1个小时连续采集和识别并没有出现段错误或者程序出现死锁，但是串口通讯解析逻辑和配置上编写的代码在使用串口屏虚拟仿真时可能会出现丢包导致程序无法启动识别和采集功能！！！正在处理中。

## 🛠️ 硬件与软件环境
- **硬件平台**：Orangepi Zero 2(全志 H616, ARM Cortex-A53, 无NPU)
- **系统内核**：Linux (Kernel 6.1.31-sun50iw9)
- **软件平台**：Ubuntu 22.04.5 LTS
- **系统内核**：Linux (Kernel 6.8.0-138-generic)
- **构建工具**：CMake, GCC，交叉编译
- **核心技术栈**：
  - 视频采集：Linux V4L2(mmap零拷贝) 
  - 格式转码：libjpeg-turbo
  - AI推理：NCNN, YOLO11(INT8量化)
  - 系统编程：POSIX多线程, 互斥锁, 条件变量
  - 外设通信：Linux termios(陶晶驰开发中)
  - 数据库：SQLite3(等待开发)

## 🏗️ 系统架构

    V4L2 Camera(v4l2.c)      -->      V4L2 Manager(video_manager.c) --> Camera Business(camera.c)        ---    
                                                                                                                    |
    libjpeg-turbo(mjpg_to_bgr.c) --> Transcoding Manager(convert_manager.c)  ---
                                                                                |
    NCNN(ncnn.cpp)        -->        NCNN wrapper(ncnn_wrapper.cpp)    -->     AI Business(yolo11.c)       ---       Business(main.cpp)
                                                                                                                    |
    Serial(serial.c)      -->        Serial Business(seriallcd.c)                       ---

    SQLite3(未完成)       ---

## 测试结果(仅供参考)

    官方实例加载模型(FLOAT16)到转码和识别总耗时(time指令测试)：
    320x320分辨率        416x416分辨率        640x640分辨率
    real    0m0.416s    real    0m0.520s     real    0m1.018s
    user    0m1.244s    user    0m1.534s     user    0m2.956s
    sys     0m0.089s    sys     0m0.170s     sys     0m0.229s

    官方实例加载模型(INT8)到转码和识别总耗时(time指令测试)：
    320x320分辨率        416x416分辨率        640x640分辨率
    real    0m0.292s    real    0m0.393s     real    0m0.758s
    user    0m0.729s    user    0m1.068s     user    0m2.111s
    sys     0m0.078s    sys     0m0.053s     sys     0m0.167s

    项目加载模型(INT8)和640x640分辨率下：
    OpenCV转码   libjpeg-turbo转码    NCNN推理       
    7.2~7.5ms    5.3~6.0ms           458~481ms               

## BUGS记录
1、摄像头在默认自动曝光情况下，摄像头从冷启动时，程序捕获的画面亮度严重黑暗。
    ---由于刚开始项目摄像头模块搭建时候通过程序一启动运行就立马捕抓一张图像时候发现问题，使用mjpg-streamer调试观察现象和反复对比测试、查询摄像头原理资料后，
        在设置摄像头自动曝光设置下发现这是无法避免的，通过查询mjpg-streame源码是检测STREAMING_PAUSED状态然后进行usleep(1)睡眠一微秒操作。目前先保留这个Bug，
        目前不影响程序功能正常运行，后续会修复跟进解决方案。

2、摄像头采集(生产者)线程与端侧视觉识别(消费者)运行一段时间后出现终端无任何调试信息打印
    ---主要问题是，由于重构整个摄像头采集、端侧视觉识别(消费者)以及主函数业务逻辑函数调用，重新函数封装、线程业务调用函数逻辑以及各底层应用文件结构体重新构建，
        导致出现段错误和没有真正释放mmap映射的buffer重新归还到空闲队列，以及很多离谱问题出现，加上当时并没有使用github管理跟踪项目代码和保留备份项目完整代码。
        最后通过重新梳理思路修改代码，添加printf打印调试信息和GDB工具调试信息监控程序暂时解决。

3、串口屏触摸控件，串口屏接收线程无法同时唤醒摄像头采集(生产者)线程与端侧视觉识别(消费者)线程导致死锁阻塞。
    ---目前通过重新构思互斥锁与条件变量业务逻辑和代码修改，暂时解决。

4、串口配置和接收业务线程解析并未遵循适合串口屏控件发送数据格式协议，导致丢包和解析混乱数据。
    ---目前正在解决中