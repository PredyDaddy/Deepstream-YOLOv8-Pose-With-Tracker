cmake_minimum_required(VERSION 3.11)
project(pose_tracker VERSION 0.1.0 LANGUAGES CXX CUDA)

set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CXX_STANDARD 14)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# find_package(TensorRT REQUIRED)


set(TENSORRT_ROOT /usr/src/tensorrt)

find_path(TENSORRT_INCLUDE_DIR NvInfer.h
  HINTS ${TENSORRT_ROOT} PATH_SUFFIXES include)
find_library(TENSORRT_LIBRARY nvinfer
  HINTS ${TENSORRT_ROOT} PATH_SUFFIXES lib lib64)

if(NOT TENSORRT_INCLUDE_DIR OR NOT TENSORRT_LIBRARY)
  message(FATAL_ERROR "Could not find TensorRT")
endif()

include_directories(${TENSORRT_INCLUDE_DIR})


find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-rtsp-server-1.0)

set(LIB_INSTALL_DIR /opt/nvidia/deepstream/deepstream/lib/)
set(APP_INSTALL_DIR /opt/nvidia/deepstream/deepstream/bin/)
set(APP_INSTALL_ROOT /opt/nvidia/deepstream/deepstream)

link_directories(
    ${LIB_INSTALL_DIR}
    /usr/local/cuda/lib64
)


add_executable(v8_pose_bytetrack_RTSPfile v8_pose_bytetrack_RTSPfile.cpp )
add_executable(v8_pose_bytetrack_sinkfile v8_pose_bytetrack_sinkfile.cpp )


target_include_directories(v8_pose_bytetrack_RTSPfile PRIVATE
    /usr/local/cuda/include
    ${GSTREAMER_INCLUDE_DIRS}
    ${APP_INSTALL_ROOT}/sources/includes
    ${TensorRT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}  
)
target_include_directories(v8_pose_bytetrack_sinkfile PRIVATE
    /usr/local/cuda/include
    ${GSTREAMER_INCLUDE_DIRS}
    ${APP_INSTALL_ROOT}/sources/includes
    ${TensorRT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}  
)

target_link_libraries(v8_pose_bytetrack_RTSPfile PRIVATE
    ${GSTREAMER_LIBRARIES} cudart nvdsgst_meta nvds_meta nvds_yml_parser ${TENSORRT_LIBRARY}
)
target_link_libraries(v8_pose_bytetrack_sinkfile PRIVATE
    ${GSTREAMER_LIBRARIES} cudart nvdsgst_meta nvds_meta nvds_yml_parser ${TENSORRT_LIBRARY}
)