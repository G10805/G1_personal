project(ais)
cmake_minimum_required(VERSION 2.6)

set(CHI_CDK_PATH "${AIS_ROOT_PATH}/../chi-cdk")

aux_source_directory(${AIS_ROOT_PATH}/CameraEventLog/src EVENT_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/CameraQueue/CameraQueueSCQ/src QUEUE_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/Engine/src ENGINE_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/Engine/pproc ENGINE_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/Engine/pproc/chi ENGINE_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/Engine/pproc/chi/src ENGINE_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/CameraOSServices/CameraOSServicesMMOSAL/src OS_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/CameraDevice/src DEVICE_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/CameraPlatform/linux PLATFORM_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/HWDrivers/Linux/IFEDriver PLATFORM_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/HWDrivers/Linux/CSIPHYDriver PLATFORM_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/ImagingInputs/ImagingInputDriver/src INPUT_SRCS)
aux_source_directory(${AIS_ROOT_PATH}/ImagingInputs/ImagingInputDriver/src/linux INPUT_LINUX_SRCS)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -fvisibility=hidden")

add_definitions(
    -D_GNU_SOURCE
    -D_LINUX
    -D__AGL__
    -Dstrlcat=g_strlcat
    -DCAMX_ANDROID_API=28
)

if ("${AIS_BUILD_WITH_CAMX}" STREQUAL "true")
add_definitions (-DAIS_WITH_CAMX)
endif ("${AIS_BUILD_WITH_CAMX}" STREQUAL "true")

set(SOURCE_FILES
    ${EVENT_SRCS}
    ${QUEUE_SRCS}
    ${ENGINE_SRCS}
    ${OS_SRCS}
    ${DEVICE_SRCS}
    ${DEVICE_S_SRCS}
    ${PLATFORM_SRCS}
    ${INPUT_SRCS}
    ${INPUT_LINUX_SRCS}
)
add_library (ais SHARED ${SOURCE_FILES})


include_directories (${MM-OSAL_INCPATH})
include_directories (${AIS_ROOT_PATH}/API/inc)
include_directories (${AIS_ROOT_PATH}/CameraDevice/inc)
include_directories (${AIS_ROOT_PATH}/CameraConfig/inc)
include_directories (${AIS_ROOT_PATH}/CameraEventLog/inc)
include_directories (${AIS_ROOT_PATH}/Common/inc)
include_directories (${AIS_ROOT_PATH}/CameraQueue/CameraQueue/inc)
include_directories (${AIS_ROOT_PATH}/CameraOSServices/CameraOSServices/inc)
include_directories (${AIS_ROOT_PATH}/CameraOSServices/CameraOSServicesMMOSAL/inc)
include_directories (${AIS_ROOT_PATH}/CameraPlatform/inc)
include_directories (${AIS_ROOT_PATH}/CameraPlatform/linux)
include_directories (${AIS_ROOT_PATH}/Engine/inc)
include_directories (${AIS_ROOT_PATH}/Engine/src)
include_directories (${AIS_ROOT_PATH}/Engine/pproc)
include_directories (${AIS_ROOT_PATH}/Engine/pproc/chi)
include_directories (${AIS_ROOT_PATH}/Engine/pproc/chi/inc)
include_directories (${AIS_ROOT_PATH}/HWDrivers/API/MIPICSIDriver)
include_directories (${AIS_ROOT_PATH}/HWDrivers/API/IFEDriver)
include_directories (${AIS_ROOT_PATH}/HWDrivers/Linux/IFEDriver)
include_directories (${AIS_ROOT_PATH}/HWDrivers/Linux/CSIPHYDriver)
include_directories (${AIS_ROOT_PATH}/ImagingInputs/ImagingInputDriver/inc)
include_directories (${AIS_ROOT_PATH}/ImagingInputs/ImagingInputDriver/src)
include_directories (${AIS_ROOT_PATH}/CameraMulticlient/common/inc)
include_directories (${AIS_ROOT_PATH}/PostProc/inc)
include_directories (${CHI_CDK_PATH}/cdk/chi)
include_directories (${CHI_CDK_PATH}/cdk)
include_directories (${CHI_CDK_PATH}/cdk/isp)
include_directories (${CHI_CDK_PATH}/cdk/common)
include_directories (${CHI_CDK_PATH}/cdk/pdlib)
include_directories (${CHI_CDK_PATH}/cdk/stats)
include_directories (${CHI_CDK_PATH}/cdk/node)
include_directories (${CHI_CDK_PATH}/cdk/sensor)
include_directories (${CHI_CDK_PATH}/vendor/node/chiutils)
include_directories (${CHI_CDK_PATH}/vendor/chioverride/default)
include_directories (${AIS_ROOT_PATH})
include_directories (${SYSROOTINC_PATH})
include_directories (${SYSROOT_INCLUDEDIR})

link_directories(${SYSROOT_LIBDIR})

target_link_libraries (ais dl)
target_link_libraries (ais glib-2.0)
target_link_libraries (ais pthread)
target_link_libraries (ais cutils)
target_link_libraries (ais mmosal)
target_link_libraries (ais C2D2)

install (TARGETS ais LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
