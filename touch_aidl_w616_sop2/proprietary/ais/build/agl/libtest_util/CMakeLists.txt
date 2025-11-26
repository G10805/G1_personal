project(test_util)
cmake_minimum_required(VERSION 2.6)


set(UTIL_PATH "${AIS_ROOT_PATH}/test/test_util")

set (WAYLANDSCANNER_PATH "${SYSTEM_ROOT_HOST}/usr/bin")
set (WAYLAND_PROTOCOL_PATH "${UTIL_PATH}/src/agl/")
set (WAYLAND_PROTOCOL_XML_PATH "${UTIL_PATH}/src/agl/disp_protocol/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-int-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror ")

add_definitions(
    -D__AGL__
    -D_GNU_SOURCE
)

if ("$ENV{AIS_WESTON_ENABLE}" STREQUAL "true")
add_definitions(
    -DUSE_GBM_BACKEND
)
endif ("$ENV{AIS_WESTON_ENABLE}" STREQUAL "true")

function (generate_header arg arg1)
    add_custom_command(OUTPUT ${WAYLAND_PROTOCOL_PATH}/${arg}.h
    WORKING_DIRECTORY ${WAYLAND_PROTOCOL_PATH}
    DEPENDS ${WAYLAND_PROTOCOL_XML_PATH}/${arg1}.xml ${WAYLANDSCANNER_PATH}/wayland-scanner
    COMMAND
        ${WAYLANDSCANNER_PATH}/wayland-scanner client-header < ${WAYLAND_PROTOCOL_XML_PATH}/${arg1}.xml > ${arg}.h
    COMMENT "Generating ${arg} header" VERBATIM
    )

endfunction ()

function (generate_src arg arg1)
    add_custom_command(OUTPUT ${WAYLAND_PROTOCOL_PATH}/${arg}.c
    WORKING_DIRECTORY ${WAYLAND_PROTOCOL_PATH}
    DEPENDS ${WAYLAND_PROTOCOL_XML_PATH}/${arg1}.xml ${WAYLANDSCANNER_PATH}/wayland-scanner
    COMMAND
        ${WAYLANDSCANNER_PATH}/wayland-scanner code < ${WAYLAND_PROTOCOL_XML_PATH}/${arg1}.xml > ${arg}.c
    COMMENT "Generating ${arg}" VERBATIM
    )

endfunction ()

generate_header(ivi-application-client-protocol ivi-application)
generate_header(gbm-buffer-backend-client-protocol gbm-buffer-backend)
generate_header(scaler-client-protocol scaler)
generate_header(xdg-shell-client-protocol xdg-shell)
generate_header(viewporter-client-protocol viewporter)

generate_src(ivi-application-client-protocol ivi-application)
generate_src(scaler-client-protocol scaler)
generate_src(gbm-buffer-backend-client-protocol gbm-buffer-backend)
generate_src(viewporter-client-protocol viewporter)

add_custom_target(wayland_protocol_files
    DEPENDS
        ${WAYLAND_PROTOCOL_PATH}/gbm-buffer-backend-client-protocol.h
        ${WAYLAND_PROTOCOL_PATH}/ivi-application-client-protocol.h
        ${WAYLAND_PROTOCOL_PATH}/scaler-client-protocol.h
        ${WAYLAND_PROTOCOL_PATH}/xdg-shell-client-protocol.h
        ${WAYLAND_PROTOCOL_PATH}/viewporter-client-protocol.h
        ${WAYLAND_PROTOCOL_PATH}/ivi-application-client-protocol.c
        ${WAYLAND_PROTOCOL_PATH}/scaler-client-protocol.c
        ${WAYLAND_PROTOCOL_PATH}/gbm-buffer-backend-client-protocol.c
        ${WAYLAND_PROTOCOL_PATH}/viewporter-client-protocol.c
)

set(SOURCE_FILES
    ${UTIL_PATH}/src/test_util.cpp
    ${UTIL_PATH}/src/agl/test_util_agl.cpp
    ${WAYLAND_PROTOCOL_PATH}/ivi-application-client-protocol.c
    ${WAYLAND_PROTOCOL_PATH}/scaler-client-protocol.c
    ${WAYLAND_PROTOCOL_PATH}/gbm-buffer-backend-client-protocol.c
    ${WAYLAND_PROTOCOL_PATH}/viewporter-client-protocol.c
)
add_library (test_util SHARED ${SOURCE_FILES})


include_directories (${MM-OSAL_INCPATH})
include_directories (${XML2_INCPATH})
include_directories (${AIS_ROOT_PATH}/API/inc)
include_directories (${AIS_ROOT_PATH}/CameraOSServices/CameraOSServices/inc)
include_directories (${AIS_ROOT_PATH}/Common/inc)
include_directories (${UTIL_PATH}/inc)
include_directories (${UTIL_PATH}/src/agl)

include_directories (${SYSROOTINC_PATH})
include_directories (${SYSROOT_INCLUDEDIR})

link_directories(${SYSROOT_LIBDIR})


target_link_libraries (test_util ais_log)
target_link_libraries (test_util xml2)
target_link_libraries (test_util wayland-client)
target_link_libraries (test_util gbm)
target_link_libraries (test_util EGL)
target_link_libraries (test_util dl)
target_link_libraries (test_util glib-2.0)
target_link_libraries (test_util C2D2)
target_link_libraries (test_util mmosal)



add_dependencies (test_util wayland_protocol_files)

install (TARGETS test_util DESTINATION ${CMAKE_INSTALL_LIBDIR})
