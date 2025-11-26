set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --verbose -Wl,-Bsymbolic -Wl,-Bsymbolic-functions")

set (CAMX_OS linuxembedded)


set (CMAKE_SHARED_LIBRARY_PREFIX "")

set (IQSETTING "")

SET(CMAKE_INSTALL_RPATH "/usr/${CAMX_INSTALL_LIBDIR}/camera/components/")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

set (CAMX_SYSTEM_PATH ${CMAKE_CURRENT_LIST_DIR}/../../camx-lib/system)
set (CAMX_PATH ${CMAKE_CURRENT_LIST_DIR}/../../camx)

set (CAMX_C_INCLUDES
    ${CAMX_CDK_PATH}
    ${CAMX_CDK_PATH}/common
    ${CAMX_CDK_PATH}/chi
    ${CAMX_CDK_PATH}/fd
    ${CAMX_CDK_PATH}/generated/g_chromatix
    ${CAMX_CDK_PATH}/generated/g_parser
    ${CAMX_CDK_PATH}/generated/g_sensor
    ${CAMX_CDK_PATH}/isp
    ${CAMX_CDK_PATH}/node
    ${CAMX_CDK_PATH}/pdlib
    ${CAMX_CDK_PATH}/sensor
    ${CAMX_CDK_PATH}/stats
    ${CAMX_EXT_INCLUDE}
    ${CAMX_PATH}/src/core
    ${CAMX_PATH}/src/core/chi
    ${CAMX_PATH}/src/core/hal
    ${CAMX_PATH}/src/core/halutils
    ${CAMX_PATH}/src/core/ncs
    ${CAMX_PATH}/src/csl
    ${CAMX_PATH}/src/osutils
    ${CAMX_PATH}/src/sdk
    ${CAMX_PATH}/src/swl
    ${CAMX_PATH}/src/swl/sensor
    ${CAMX_PATH}/src/swl/stats
    ${CAMX_PATH}/src/utils
    ${CAMX_PATH}/src/utils/scope
    ${CAMX_SYSTEM_PATH}/debugdata/common/inc
    ${CAMX_SYSTEM_PATH}/firmware
    ${CAMX_SYSTEM_PATH}/ifestripinglib/stripinglibrary/inc
    ${CAMX_SYSTEM_PATH}/stripinglib/fw_core/common/chipinforeader/inc
    ${CAMX_SYSTEM_PATH}/stripinglib/fw_core/hld/stripinglibrary/inc
    ${CAMX_SYSTEM_PATH}/swprocessalgo
    ${CAMX_SYSTEM_PATH}/tintlessalgo/inc
   )

set (CAMX_C_LIBS "")

# Common C flags for the project
set (CAMX_CFLAGS
    -fPIC
    -DCAMX
    -D_LINUX
    -DANDROID
    -D__AGL__
    -DSNS_LE_QCS605
    -DUSE_LIBGBM
    -D_GNU_SOURCE
    -Dstrlcpy=g_strlcpy
    -Dstrlcat=g_strlcat
    -DCAMX_LOGS_ENABLED=1
    -DCAMX_TRACES_ENABLED=0
    -DPTHREAD_TLS=1
    -fstack-protector-strong
    -Wall
    -Wno-error
    -D__CAMX_AUTO_CHANGES__
    -DCAMX_USE_GRALLOC1
    )

# Common C++ flags for the project
set (CAMX_CPPFLAGS
    -DCAMX_LOGS_ENABLED=1
    -fPIC
    -Wno-invalid-offsetof
    -include stdint.h
    -include sys/ioctl.h
    -include glib.h
    -include iostream
    -include algorithm
    -include vector
    -include sys/time.h
     -fstack-protector-strong
    )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread -lrt -lm -lglib-2.0 -ldl -latomic -lsync")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -lpthread -lrt -lm -lglib-2.0 -ldl -latomic -lsync")
