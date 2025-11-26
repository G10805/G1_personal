cmake_minimum_required(VERSION 3.5)

project(ar_osal)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/${CMAKE_INSTALL_INCLUDEDIR}/diag_lsm/include")

set(ar_osal_includes
    api/)

include_directories(${ar_osal_includes})
include_directories(./platform/lrh/inc/)

set(ar_osal_SOURCE
    ./platform/lrh/src/ar_osal_shmem.c
    ./src/ar_osal_file_io.c
    ./src/ar_osal_heap.c
    ./src/ar_osal_log.c
    ./src/ar_osal_log_pkt_op.c
    ./src/ar_osal_mem_op.c
    ./src/ar_osal_mutex.c
    ./platform/lrh/src/ar_osal_servreg.c
    ./src/ar_osal_signal.c
    ./src/ar_osal_sleep.c
    ./src/ar_osal_string.c
    ./src/ar_osal_thread.c
    ./src/ar_osal_timer.c
    ./platform/lrh/src/ar_util_data_log.c
    ./platform/lrh/src/ar_util_list.c)

add_definitions(-DLINUX_ENABLED)
add_definitions(-Dstrlcpy=g_strlcpy)
add_definitions(-Dstrlcat=g_strlcat)
add_definitions(-D__unused=__attribute__\(\(__unused__\)\))
add_definitions(-D__LINUX__)
##add_definitions(-DAR_OSAL_USE_PD_NOTIFIER)
##add_definitions(-DUSE_GLIB)
add_definitions(-DPLATFORM_LRH)

add_library(ar_osal SHARED ${ar_osal_SOURCE})
target_link_libraries(ar_osal PUBLIC audio_utils pthread glib-2.0 cutils kiumd diag-lsm ssr-client)

install(TARGETS ar_osal LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY ${ar_osal_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

##add_subdirectory(test)
