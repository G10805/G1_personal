cmake_minimum_required(VERSION 3.5)

project(ar_util)

set(ar_util_includes
    api/
    inc/)

include_directories(${ar_util_includes})

set(ar_util_SOURCE
    src/ar_util_data_log.c
    src/ar_util_list.c)

add_library(ar_util SHARED ${ar_util_SOURCE})
target_link_libraries(ar_util PUBLIC ar_osal)

install(TARGETS ar_util LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY ${ar_util_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

##add_subdirectory(test)
