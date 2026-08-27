cmake_minimum_required(VERSION 3.10)
project(ncnn_video C CXX)

# 1. 设置 C 标准与编译选项（等价于原 Makefile 的 CFLAGS）
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Werror -O2 -g")
set(CMAKE_CXX_FLAGS "-Wall -Werror -O2 -g")

# 2. 明确源文件列表（等价于原 obj-y = main.o + video/）
# 注意：此处直接列出所有 .c 文件，CMake 自动处理依赖关系
set(SOURCES
    main.cpp
    video/v4l2.c
    video/video_manager.c
    video/camera.c
	convert/mjpg_to_bgr.c
	convert/convert_manager.c
	ncnn/ncnn.cpp
	ncnn/ncnn_wrapper.cpp 
	ncnn/yolo11.c
)

# 3. 设置头文件路径（等价于原 CFLAGS += -I include）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(ncnn_video PRIVATE /home/hkd/Downloads/ncnn/build-cpu/install/include)

set(CMAKE_PREFIX_PATH "/home/hkd/Desktop/ncnn_video/libjpeg_arm_install" ${CMAKE_PREFIX_PATH})

find_package(JPEG REQUIRED)

set(ncnn_DIR "/home/hkd/Downloads/ncnn/build-cpu/install/lib/cmake/ncnn" CACHE PATH "ncnn cmake dir")
find_package(ncnn REQUIRED)

# 4. 查找 pthread 库（等价于原 LDFLAGS 隐含的 -lpthread）
find_package(Threads REQUIRED)

# 5. 生成可执行文件（等价于原 TARGET = ncnn_video）
add_executable(ncnn_video ${SOURCES})

# 6. 链接 pthread 库
target_link_libraries(ncnn_video
    Threads::Threads
	JPEG::JPEG
    ncnn
)

# 显式包含头文件路径（如果 find_package 没自动加上）
target_include_directories(ncnn_video PRIVATE ${OpenCV_INCLUDE_DIRS} ${ncnn_INCLUDE_DIRS})