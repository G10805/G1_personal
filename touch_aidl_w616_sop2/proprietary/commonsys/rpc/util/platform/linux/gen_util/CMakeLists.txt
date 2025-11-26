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

set(TARGET_LIB qti_property_util )
#add_definitions("-DENABLE_PROP_DEBUG")

set(SRCS
./src/properties.cpp
)

set(HDRS
./include/properties.h
)

SET(LIBS
PRIVATE
    glib-2.0
)

include_directories(include)
add_compile_options(-fPIC)

add_library(${TARGET_LIB} STATIC ${SRCS} ${HDRS})
target_link_libraries(${TARGET_LIB} ${LIBS})

add_subdirectory(tool)

install(TARGETS ${TARGET_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gen_util PATTERN "*.h")
install(DIRECTORY conf/ DESTINATION /etc/prop/ PATTERN "*.conf")
