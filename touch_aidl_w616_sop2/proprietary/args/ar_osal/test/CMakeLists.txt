cmake_minimum_required(VERSION 3.5)

project(ar_osal_test)

set(ar_osal_test_includes
    inc/)

include_directories(${ar_osal_test_includes})

set(ar_osal_test_SOURCE
    src/ar_osal_test.c
    src/ar_test_file_io.c
    src/ar_test_shmem.c
    src/ar_test_signal_mutex_thread.c)

add_executable(ar_osal_test ${ar_osal_test_SOURCE})

target_link_libraries(ar_osal_test PRIVATE ar_osal)

install(TARGETS ar_osal_test RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
##install(DIRECTORY ${ar_osal_test_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
