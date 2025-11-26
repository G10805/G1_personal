project(cvbs)
cmake_minimum_required(VERSION 2.6)

set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-int-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror ")

add_definitions(
    -D__AGL__
    -D_GNU_SOURCE
    )

find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

# wayland_add_protocol_client(outfiles inputfile basename)
function(WAYLAND_ADD_PROTOCOL_CLIENT _sources _protocol _basename)
    if(NOT WAYLAND_SCANNER_EXECUTABLE)
        message(FATAL "The wayland-scanner executable has nto been found on your system. You must install it.")
    endif()

    get_filename_component(_infile ${_protocol} ABSOLUTE)
    set(_client_header "${ROOT_PATH}/${_basename}-client-protocol.h")
    set(_code "${ROOT_PATH}/${_basename}-protocol.c")

    add_custom_command(OUTPUT "${_client_header}"
        COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header < ${_infile} > ${_client_header}
        DEPENDS ${_infile} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ${WAYLAND_SCANNER_EXECUTABLE} code < ${_infile} > ${_code}
        DEPENDS ${_infile} VERBATIM)

    list(APPEND ${_sources} "${_client_header}" "${_code}")
    set(${_sources} ${${_sources}} PARENT_SCOPE)
endfunction()

# wayland_add_protocol_server(outfiles inputfile basename)
function(WAYLAND_ADD_PROTOCOL_SERVER _sources _protocol _basename)
    if(NOT WAYLAND_SCANNER_EXECUTABLE)
        message(FATAL "The wayland-scanner executable has nto been found on your system. You must install it.")
    endif()

    get_filename_component(_infile ${_protocol} ABSOLUTE)
    set(_server_header "${ROOT_PATH}/${_basename}-server-protocol.h")
    set(_code "${ROOT_PATH}/${_basename}-protocol.c")

    add_custom_command(OUTPUT "${_server_header}"
        COMMAND ${WAYLAND_SCANNER_EXECUTABLE} server-header < ${_infile} > ${_server_header}
        DEPENDS ${_infile} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ${WAYLAND_SCANNER_EXECUTABLE} code < ${_infile} > ${_code}
        DEPENDS ${_infile} VERBATIM)

    list(APPEND ${_sources} "${_server_header}" "${_code}")
    set(${_sources} ${${_sources}} PARENT_SCOPE)
endfunction()

set(SOURCE_FILES ${ROOT_PATH}/cvbs.cpp)

wayland_add_protocol_client(SOURCE_FILES ${PROTOCOL_PATH}/gbm-buffer-backend.xml gbm-buffer-backend)
wayland_add_protocol_client(SOURCE_FILES ${PROTOCOL_PATH}/ivi-application.xml ivi-application)
wayland_add_protocol_client(SOURCE_FILES ${PROTOCOL_PATH}/linux-dmabuf.xml linux-dmabuf)

add_executable (cvbs ${SOURCE_FILES})
include_directories (${DRM_INCDIR})

target_link_libraries (cvbs ais_client)
target_link_libraries (cvbs drm)
target_link_libraries (cvbs gbm)
target_link_libraries (cvbs ilmControl)
target_link_libraries (cvbs ilmCommon)
target_link_libraries (cvbs pthread)
target_link_libraries (cvbs wayland-client)

install (TARGETS cvbs DESTINATION ${CMAKE_INSTALL_BINDIR})
