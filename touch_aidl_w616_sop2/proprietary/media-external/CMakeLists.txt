#-------------------------------------------------------------------------
# media-external - CMake
#-------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5)

project(media-external)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-error=deprecated-declarations -Wno-enum-compare")
set(AGL_LINUX OFF CACHE BOOL "AGL(Automotive Grade Linux): ON|OFF. Default [OFF]")
set(PLATFORM "lahaina" CACHE STRING "PLATFORM: lahaina/direwolf. Default [lahaina]")

# C2
add_definitions(-D_ENABLE_QC2_GBM_)

if(AGL_LINUX)
  add_definitions(-D_AGL_LINUX_)
endif()


# Build debug version
# set(CMAKE_BUILD_TYPE Debug)

#C2 HAL includes
set(build_include_dirs
  codec2/
  codec2/base/include
  codec2/base/target
  codec2/codecs/include
  codec2/hooks/include
  codec2/utils/include)
include_directories(${build_include_dirs})

# Output path
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#-------------------------------------------------------------------------
# libqcodec2_hooks
#-------------------------------------------------------------------------

set(CODEC2_HOOKS_SOURCE
    codec2/hooks/QC2ComponentObserver.cpp
    codec2/hooks/QC2PerfController.cpp)
add_library(qcodec2_hooks SHARED ${CODEC2_HOOKS_SOURCE})

set(CODEC2_HEADS
    codec2/codecs/include/QC2Config.h
    codec2/base/include/QC2Constants.h
    codec2/utils/include/QC2Buffer.h)

set(CODEC2_INCLUDES
    codec2/tests/utils/include/
    codec2/hooks/include/
    codec2/utils/include/
    codec2/base/include/
    codec2/core/include/
    codec2/codecs/filters/colorconvert/include/
    codec2/codecs/filters/mock/include/
    codec2/codecs/filters/core/include/
    codec2/codecs/include/
    codec2/base/target/)

set(QCODEC2_LIBS
    qcodec2_hooks)

set(CONFIG_FILE
    codec2/platform/target/android/${PLATFORM}/data/video_system_specs.json)

install(TARGETS ${QCODEC2_LIBS} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY ${CODEC2_INCLUDES} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES ${CONFIG_FILE} DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}")
