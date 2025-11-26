# ---------------------------------------------------------------------------------
#                       Make fastcv_nsp_simple_test
# ---------------------------------------------------------------------------------

common_flags = -O3 \
               -fPIC \
               -Wall \
               -Wundef \
               -Wstrict-prototypes \
               -Wno-trigraphs \
               -Wno-error=uninitialized \
               -fno-exceptions \
               -fno-permissive \
               -fno-short-enums \
               -fsigned-char \
               -Istub/inc

if IS_ARM_NATIVE
common_flags += -I /usr/include/glib-2.0 \
                -I /usr/lib64/glib-2.0/include
else
common_flags += -I${PKG_CONFIG_SYSROOT_DIR}/usr/include/glib-2.0 \
                -I${PKG_CONFIG_SYSROOT_DIR}/usr/lib/glib-2.0/include
endif

if ENABLE_ASAN
common_flags += -fsanitize=address
endif

AM_CPPFLAGS = -include unistd.h \
    -include sys/time.h \
    -DLE_ENABLE \
    -include stdint.h \
    -Dstrlcpy=g_strlcpy \
    -Dstrlcat=g_strlcat \
    -include glib.h \
    -include glibconfig.h \
    -include stdlib.h

AM_CPPFLAGS += $(common_flags)

bin_PROGRAMS = fastcv_nsp_simple_test
fastcv_nsp_simple_test_SOURCES  = src/fastcvNSPSimpleTest.cpp stub/src/fastcvNSPSimpleTest_stub.c
fastcv_nsp_simple_test_LDADD = -lglib-2.0 -lcdsprpc
