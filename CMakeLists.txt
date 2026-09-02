cmake_minimum_required(VERSION 3.10)
project(ncnn_video C CXX)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Werror -O2 -g")
set(CMAKE_CXX_FLAGS "-Wall -Werror -O2 -g")


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
	serial/serial.c
	serial/seriallcd.c
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(ncnn_video PRIVATE /home/hkd/Downloads/ncnn/build-cpu/install/include)

set(CMAKE_PREFIX_PATH "/home/hkd/Desktop/ncnn_video/libjpeg_arm_install" ${CMAKE_PREFIX_PATH})

find_package(JPEG REQUIRED)

set(ncnn_DIR "/home/hkd/Downloads/ncnn/build-cpu/install/lib/cmake/ncnn" CACHE PATH "ncnn cmake dir")
find_package(ncnn REQUIRED)


find_package(Threads REQUIRED)


add_executable(ncnn_video ${SOURCES})


target_link_libraries(ncnn_video
    Threads::Threads
	JPEG::JPEG
    ncnn
)
