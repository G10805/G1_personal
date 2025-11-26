cmake_minimum_required(VERSION 3.5)

project(ar_util_test)

set(ar_util_test_includes
    inc/)

include_directories(${ar_util_test_includes})

set(ar_util_test_SOURCE
    src/ar_osal_mutex_thread.c
    src/ar_osal_signal_thread.c
    src/ar_osal_test.c
    src/ar_osal_test_service.c
    src/ar_test_file_io.c
    src/ar_test_heap.c
    src/ar_test_list.c
    src/ar_test_mem_op.c
    src/ar_test_shmem.c
    src/ar_test_sleep.c
    src/ar_test_string.c)

add_definitions(-D__unused=__attribute__\(\(__unused__\)\))

add_executable(ar_util_test ${ar_util_test_SOURCE})

target_link_libraries(ar_util_test PRIVATE ar_util)

install(TARGETS ar_util_test RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
##install(DIRECTORY ${ar_util_test_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
