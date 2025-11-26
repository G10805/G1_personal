# User Variables
set (SUPPORT_FUNCTION_CALL_TRACE 1)

# NO DATA FOR SECTION: Custom additions (begin)

# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)

# Files and Build Type
add_library( com.qti.sensor.ov_ti960_0_ox03a10 SHARED
    ../../ti960_ox03a10.cpp
)

add_library( com.qti.sensor.ov_ti960_1_ox03a10 SHARED
    ../../ti960_ox03a10.cpp
)

add_library( com.qti.sensor.ov_ti960_2_ox03a10 SHARED
    ../../ti960_ox03a10.cpp
)

add_library( com.qti.sensor.ov_ti960_3_ox03a10 SHARED
    ../../ti960_ox03a10.cpp
)

# Libraries to compile
target_link_libraries (com.qti.sensor.ov_ti960_0_ox03a10 dl)
target_link_libraries (com.qti.sensor.ov_ti960_1_ox03a10 dl)
target_link_libraries (com.qti.sensor.ov_ti960_2_ox03a10 dl)
target_link_libraries (com.qti.sensor.ov_ti960_3_ox03a10 dl)

# Include Paths
include_directories (${CAMX_C_INCLUDES})
include_directories (../..)
include_directories (${CAMX_CDK_PATH}/node/)
include_directories (${CAMX_CDK_PATH}/sensor)

# Dependencies


# Definitions
add_definitions (${CAMX_CFLAGS})
add_definitions (${CAMX_CPPFLAGS})
# NO DATA FOR SECTION: Custom additions (end)


install(TARGETS com.qti.sensor.ov_ti960_0_ox03a10 LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/camera)
install(TARGETS com.qti.sensor.ov_ti960_1_ox03a10 LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/camera)
install(TARGETS com.qti.sensor.ov_ti960_2_ox03a10 LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/camera)
install(TARGETS com.qti.sensor.ov_ti960_3_ox03a10 LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/camera)

if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()

