###############################################################################
#
# Copyright (c) 2024 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# REVISION:      $Revision: #1 $
###############################################################################

include_directories("/usr/include")
include_directories("./include")

set(TARGET_SETPROP setprop)
set(TARGET_GETPROP getprop)

set(SETPROP_SRCS
./setprop.cpp
)

set(GETPROP_SRCS
./getprop.cpp
)

SET(LIBS
    PUBLIC
    qti_property_util
)

add_executable(${TARGET_SETPROP} ${SETPROP_SRCS})
target_link_libraries(${TARGET_SETPROP} ${LIBS})

add_executable(${TARGET_GETPROP} ${GETPROP_SRCS})
target_link_libraries(${TARGET_GETPROP} ${LIBS})

install(TARGETS ${TARGET_SETPROP} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS ${TARGET_GETPROP} DESTINATION ${CMAKE_INSTALL_BINDIR})

