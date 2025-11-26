cmake_minimum_required(VERSION 3.5)

project(ats)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/${CMAKE_INSTALL_INCLUDEDIR}/diag_lsm/include")

set(ats_includes
    adie/api/
    adie/inc/
    api/
    diag/Platform/actp/inc/
    diag/Platform/audtp/inc/
    fts/platform/inc/
    inc/
    mcs/api/
    mcs/inc/
    online/inc/
    rtc/inc/
    tcpip/platform/inc/)

include_directories(${ats_includes})

set(ats_SOURCE
    adie/platform/lrh/ats_adie_rtc.c
    diag/Platform/actp/src/actp.c
    diag/Platform/audtp/src/audtp.c
    fts/platform/src/ats_fts.c
    mcs/platform/lrh/ats_mcs.c
    online/src/ats_online.c
    rtc/src/ats_rtc.c
    src/ats.c
    src/ats_command.c)

add_definitions(-Dstrlcpy=g_strlcpy)
add_definitions(-Dstrlcat=g_strlcat)
add_definitions(-DPLATFORM_LRH)

add_library(ats SHARED ${ats_SOURCE})

target_link_libraries(ats PUBLIC dl acdb ar_osal ar_gsl diag-lsm)

install(TARGETS ats LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY ${ats_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
