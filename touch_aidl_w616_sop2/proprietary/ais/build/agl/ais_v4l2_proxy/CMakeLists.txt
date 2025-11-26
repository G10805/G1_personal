project(ais_v4l2_proxy)
cmake_minimum_required(VERSION 2.6)

set(V4L2_PROXY_SRC_PATH "${AIS_ROOT_PATH}/hal/ais_v4l2_proxy_v2/src")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-int-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror ")

add_definitions(
    -D__AGL__
    -D_GNU_SOURCE
)

set(SOURCE_FILES
    ${V4L2_PROXY_SRC_PATH}/ais_v4l2_xml_util.cpp
    ${V4L2_PROXY_SRC_PATH}/ais_v4l2_util.cpp
    ${V4L2_PROXY_SRC_PATH}/ais_v4l2_proxy.cpp
	${V4L2_PROXY_SRC_PATH}/ais_v4l2_proxy_stream.cpp
	${V4L2_PROXY_SRC_PATH}/ais_v4l2_proxy_input.cpp
    ${V4L2_PROXY_SRC_PATH}/ais_dmabuffer_agl.cpp
    ${V4L2_PROXY_SRC_PATH}/gbmbuffer.cpp

)
add_executable (ais_v4l2_proxy ${SOURCE_FILES})


include_directories (${MM-OSAL_INCPATH})
include_directories (${XML2_INCPATH})
include_directories (${AIS_ROOT_PATH}/API/inc)
include_directories (${AIS_ROOT_PATH}/CameraOSServices/CameraOSServices/inc)
include_directories (${AIS_ROOT_PATH}/Common/inc)
include_directories (${AIS_ROOT_PATH}/test/test_util/inc)
include_directories (${AIS_ROOT_PATH}/hal/ais_v4l2_proxy_v2/inc)
include_directories (${AIS_ROOT_PATH}/hal/ais_v4l2_proxy_v2/src)

include_directories (${SYSROOTINC_PATH})
include_directories (${SYSROOT_INCLUDEDIR})

link_directories(${SYSROOT_LIBDIR})

target_link_libraries (ais_v4l2_proxy ais_client)
target_link_libraries (ais_v4l2_proxy ais_log)
target_link_libraries (ais_v4l2_proxy mmosal)
target_link_libraries (ais_v4l2_proxy pthread)
target_link_libraries (ais_v4l2_proxy cutils)
target_link_libraries (ais_v4l2_proxy C2D2)
target_link_libraries (ais_v4l2_proxy xml2)
target_link_libraries (ais_v4l2_proxy wayland-client)
target_link_libraries (ais_v4l2_proxy gbm)
target_link_libraries (ais_v4l2_proxy EGL)
target_link_libraries (ais_v4l2_proxy dl)
target_link_libraries (ais_v4l2_proxy glib-2.0)

install (TARGETS ais_v4l2_proxy DESTINATION ${CMAKE_INSTALL_BINDIR})
