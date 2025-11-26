# ---------------------------------------------------------------------------------
#                       Make fastcv_hetero_compute_test
# ---------------------------------------------------------------------------------

FASTCV_PATH = ${top_srcdir}
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
               -I${FASTCV_PATH}/fastcv_headers \
               -I${FASTCV_PATH}/fastcv_headers/fastcv

AM_CPPFLAGS = $(common_flags)

if ENABLE_ASAN
common_flags += -fsanitize=address
endif

bin_PROGRAMS = fastcv_hetero_compute_test
fastcv_hetero_compute_test_SOURCES  = src/fastcvHeteroComputeTest.cpp
fastcv_hetero_compute_test_LDADD = -lglib-2.0 -lfastcvopt
