cmake_minimum_required(VERSION 3.5)

project(gpr)

set(gpr_includes
    api/
    api/private/
    core/inc/
    datalinks/gpr_lrh/inc/
    ext/dynamic_allocation/inc/
    ext/inc/)

include_directories(${gpr_includes})

set(gpr_SOURCE
    core/src/gpr_drv.c
    core/src/gpr_drv_island.c
    core/src/gpr_list.c
    core/src/gpr_list_island.c
    core/src/gpr_main.c
    core/src/gpr_memq.c
    core/src/gpr_memq_island.c
    core/src/hash_based/gpr_session.c
    core/src/hash_based/gpr_session_island.c
    datalinks/gpr_lrh/src/gpr_glink.c
    datalinks/gpr_lrh/src/target/cmn/glink_port_info_cmn.c
    ext/dynamic_allocation/src/gpr_dynamic_allocation.c
    ext/src/gpr_log.c
    platform_wrappers/lrh/gpr_init_lrh_wrapper.c)

add_definitions(-DSESSION_ARRAY_SIZE=200)
add_definitions(-Dstrlcat=g_strlcat)

add_library(gpr SHARED ${gpr_SOURCE})
target_link_libraries(gpr PUBLIC ar_osal glink_client)

install(TARGETS gpr LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY ${gpr_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")


