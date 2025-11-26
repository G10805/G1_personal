project(qcarcam_rvc)
cmake_minimum_required(VERSION 2.6)

set(QCARCAM_RVC_SRC_PATH "${AIS_ROOT_PATH}/test/qcarcam_rvc/src")
set(UTIL_PATH "${AIS_ROOT_PATH}/test/test_util")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-int-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror ")

add_definitions(
    -D__AGL__
    -D_GNU_SOURCE
    -DUSE_GBM_BACKEND
)

set(SOURCE_FILES
    ${QCARCAM_RVC_SRC_PATH}/qcarcam_rvc.cpp
)
add_executable (qcarcam_rvc ${SOURCE_FILES})


include_directories (${MM-OSAL_INCPATH})
include_directories (${XML2_INCPATH})
include_directories (${AIS_ROOT_PATH}/API/inc)
include_directories (${AIS_ROOT_PATH}/CameraOSServices/CameraOSServices/inc)
include_directories (${AIS_ROOT_PATH}/Common/inc)
include_directories (${UTIL_PATH}/inc)
include_directories (${UTIL_PATH}/src/agl)

include_directories (${SYSROOTINC_PATH})
include_directories (${SYSROOT_INCLUDEDIR})

link_directories(${SYSROOT_LIBDIR})

target_link_libraries (qcarcam_rvc ais_client)
target_link_libraries (qcarcam_rvc ais_log)
target_link_libraries (qcarcam_rvc mmosal)
target_link_libraries (qcarcam_rvc pthread)
target_link_libraries (qcarcam_rvc cutils)
target_link_libraries (qcarcam_rvc C2D2)
target_link_libraries (qcarcam_rvc xml2)
target_link_libraries (qcarcam_rvc wayland-client)
target_link_libraries (qcarcam_rvc gbm)
target_link_libraries (qcarcam_rvc EGL)
target_link_libraries (qcarcam_rvc dl)
target_link_libraries (qcarcam_rvc glib-2.0)
target_link_libraries (qcarcam_rvc test_util)

install (TARGETS qcarcam_rvc DESTINATION ${CMAKE_INSTALL_BINDIR})
