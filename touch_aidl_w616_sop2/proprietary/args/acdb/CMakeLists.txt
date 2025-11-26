cmake_minimum_required(VERSION 3.5)

project(acdb)

set(acdb_includes
    api/
    inc/)

include_directories(${acdb_includes})

set(acdb_SOURCE
    src/acdb.c
    src/acdb_command.c
    src/acdb_data_proc.c
    src/acdb_delta_file_mgr.c
    src/acdb_delta_parser.c
    src/acdb_file_mgr.c
    src/acdb_heap.c
    src/acdb_init.c
    src/acdb_init_utility.c
    src/acdb_parser.c
    src/acdb_utility.c)

add_library(acdb SHARED ${acdb_SOURCE})
target_link_libraries(acdb PUBLIC pthread ar_osal)

install(TARGETS acdb LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY ${acdb_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")