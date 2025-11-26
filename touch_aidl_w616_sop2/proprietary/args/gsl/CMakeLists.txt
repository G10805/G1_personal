cmake_minimum_required(VERSION 3.5)

project(ar_gsl)

set(ar_gsl_includes
    api/
    hw_rsc_api/
    inc/
    rtc_api/)

include_directories(${ar_gsl_includes})

set(ar_gsl_SOURCE
    src/gsl_common.c
    src/gsl_datapath.c
    src/gsl_dynamic_module_mgr.c
    src/gsl_global_persist_cal.c
    src/gsl_graph.c
    src/gsl_hw_rsc_mgr.c
    src/gsl_main.c
    src/gsl_mdf_utils.c
    src/gsl_msg_builder.c
    src/gsl_rtc.c
    src/gsl_rtc_main.c
    src/gsl_shmem_mgr.c
    src/gsl_spf_ss_state.c
    src/gsl_subgraph.c
    src/gsl_subgraph_pool.c)

add_definitions(-DGSL_DEBUG_ENABLE)
add_definitions(-DGSL_EXT_MEM_CACHE_DISABLE)

add_library(ar_gsl SHARED ${ar_gsl_SOURCE})
target_link_libraries(ar_gsl PUBLIC acdb ar_osal ar_util gpr)

install(TARGETS ar_gsl LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY ${ar_gsl_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
