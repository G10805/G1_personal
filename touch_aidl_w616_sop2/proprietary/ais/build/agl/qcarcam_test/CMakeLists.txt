project(qcarcam_test)
cmake_minimum_required(VERSION 2.6)

set(QCARCAM_SRC_PATH "${AIS_ROOT_PATH}/test/qcarcam_test/src")
set(UTIL_PATH "${AIS_ROOT_PATH}/test/test_util")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-int-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror ")

add_definitions(
    -D__AGL__
    -D_GNU_SOURCE
)

set(SOURCE_FILES
    ${QCARCAM_SRC_PATH}/qcarcam_test.cpp
)
add_executable (qcarcam_test ${SOURCE_FILES})

include_directories (${MM-OSAL_INCPATH})
include_directories (${XML2_INCPATH})
include_directories (${AIS_ROOT_PATH}/API/inc)
include_directories (${AIS_ROOT_PATH}/API/vendor)
include_directories (${AIS_ROOT_PATH}/CameraOSServices/CameraOSServices/inc)
include_directories (${AIS_ROOT_PATH}/Common/inc)
include_directories (${UTIL_PATH}/inc)
include_directories (${UTIL_PATH}/src/agl)

include_directories (${SYSROOTINC_PATH})
include_directories (${SYSROOT_INCLUDEDIR})

link_directories(${SYSROOT_LIBDIR})

target_link_libraries (qcarcam_test ais_client)
target_link_libraries (qcarcam_test ais_log)
target_link_libraries (qcarcam_test mmosal)
target_link_libraries (qcarcam_test pthread)
target_link_libraries (qcarcam_test cutils)
target_link_libraries (qcarcam_test C2D2)
target_link_libraries (qcarcam_test xml2)
target_link_libraries (qcarcam_test wayland-client)
target_link_libraries (qcarcam_test gbm)
target_link_libraries (qcarcam_test EGL)
target_link_libraries (qcarcam_test dl)
target_link_libraries (qcarcam_test glib-2.0)
target_link_libraries (qcarcam_test test_util)

install (TARGETS qcarcam_test DESTINATION ${CMAKE_INSTALL_BINDIR})
