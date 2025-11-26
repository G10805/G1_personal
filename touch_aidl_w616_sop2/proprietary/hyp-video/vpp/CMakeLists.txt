#-------------------------------------------------------------------------
# Project HYP-VPP-FE
#-------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5)

project(hyp-vpp-fe)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-error=deprecated-declarations -Wno-enum-compare")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-D_LINUX_)

# Include directories
set(build_include_dirs
    common/inc
    fe/inc)
include_directories(${build_include_dirs})

# Output path
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#-------------------------------------------------------------------------
# libhyp_vpp_fe.so
#-------------------------------------------------------------------------
set(VPP_FE_SOURCE
    fe/src/vpp_fe.cpp)

set(VPP_COMMON_SOURCE
    common/src/hyp_vpp_buf_manager.cpp
    common/src/hyp_vpp_queue_util.cpp
    common/src/hyp_vpp_fe.cpp
    common/src/hyp_vpp_fe_translation.cpp
    )

add_library(hyp_vpp_fe SHARED ${VPP_FE_SOURCE} ${VPP_COMMON_SOURCE})
target_link_libraries(hyp_vpp_fe mmosal)

#-------------------------------------------------------------------------
# hyp_vpp_test
#-------------------------------------------------------------------------
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
set(HYPVPP_TEST_SOURCE
    test/test_vpp_main.cpp)

add_executable(hyp_vpp_test ${HYPVPP_TEST_SOURCE})
target_link_libraries(hyp_vpp_test ${GTEST_LIBRARIES} hyp_vpp_fe gbm Threads::Threads)

#-------------------------------------------------------------------------
# vpp_ais_test
#-------------------------------------------------------------------------
set(VPP_AIS_TEST_SOURCE
    test/aistest.cpp)

add_executable(vpp_ais_test ${VPP_AIS_TEST_SOURCE})
target_link_libraries(vpp_ais_test hyp_vpp_fe dmabufheap Threads::Threads)


install(TARGETS hyp_vpp_fe DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS hyp_vpp_test DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS vpp_ais_test DESTINATION "${CMAKE_INSTALL_BINDIR}")
