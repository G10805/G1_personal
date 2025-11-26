# User Variables
set (SUPPORT_FUNCTION_CALL_TRACE 1)

# NO DATA FOR SECTION: Custom additions (begin)

# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)

# Files and Build Type
add_library( com.qti.sensor.max9296_0_ar0231isp SHARED
    ../../max9296_ar0231isp.cpp
)

add_library( com.qti.sensor.max9296_1_ar0231isp SHARED
    ../../max9296_ar0231isp.cpp
)

add_library( com.qti.sensor.max9296_2_ar0231isp SHARED
    ../../max9296_ar0231isp.cpp
)

add_library( com.qti.sensor.max9296_3_ar0231isp SHARED
    ../../max9296_ar0231isp.cpp
)

# Libraries to compile
target_link_libraries (com.qti.sensor.max9296_0_ar0231isp dl)
target_link_libraries (com.qti.sensor.max9296_1_ar0231isp dl)
target_link_libraries (com.qti.sensor.max9296_2_ar0231isp dl)
target_link_libraries (com.qti.sensor.max9296_3_ar0231isp dl)

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

# Auto-Gen
#add_dependencies(com.qti.sensor.max9296_ar0231isp autogen)

install(TARGETS com.qti.sensor.max9296_0_ar0231isp LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/camera)
install(TARGETS com.qti.sensor.max9296_1_ar0231isp LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/camera)
install(TARGETS com.qti.sensor.max9296_2_ar0231isp LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/camera)
install(TARGETS com.qti.sensor.max9296_3_ar0231isp LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/camera)

if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()

