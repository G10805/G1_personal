pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ar_util.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wall
if BUILDSYSTEM_OPENWRT
AM_CFLAGS += -I ./api
AM_CFLAGS += -I ./test/inc

util_sources = ./api/ar_util_list.h \
               ./api/ar_util_data_log.h \
               ./api/ar_util_data_log_codes.h \
               ./test/inc/ar_osal_test.h
else
AM_CFLAGS += -I ${top_srcdir}/inc
AM_CFLAGS += -I ${top_srcdir}/api
AM_CFLAGS += -I ${top_srcdir}/test/inc

util_sources = ${top_srcdir}/inc/ar_util_log_pkt_i.h \
               ${top_srcdir}/api/ar_util_list.h \
               ${top_srcdir}/api/ar_util_data_log.h \
               ${top_srcdir}/api/ar_util_data_log_codes.h \
               ${top_srcdir}/test/inc/ar_osal_test.h
endif

lib_includedir = $(includedir)
lib_include_HEADERS = $(util_sources)

util_c_sources = src/ar_util_list.c \
                 src/ar_util_data_log.c

#LIBRARY for LE apps
lib_LTLIBRARIES = libar_util.la
libar_util_la_CC = @CC@
libar_util_la_LIBADD = -lar_osal
libar_util_la_SOURCES = $(util_c_sources)
libar_util_la_CFLAGS = $(AM_CFLAGS)
libar_util_la_LDFLAGS = -shared -avoid-version

AM_CFLAGS += -I ${WORKSPACE}/vendor/qcom/proprietary/args/ar_osal/api/
AM_CFLAGS += -I ${WORKSPACE}/vendor/qcom/proprietary/args/ar_osal/test/inc/

test_sources = test/src/ar_osal_mutex_thread.c \
               test/src/ar_osal_signal_thread.c \
               test/src/ar_osal_test.c \
               test/src/ar_osal_test_service.c \
               test/src/ar_test_file_io.c \
               test/src/ar_test_heap.c \
               test/src/ar_test_list.c \
               test/src/ar_test_mem_op.c \
               test/src/ar_test_shmem.c \
               test/src/ar_test_sleep.c \
               test/src/ar_test_string.c

bin_PROGRAMS = ar_util_test
ar_util_test_CC = @CC@
ar_util_test_SOURCES = $(test_sources)
ar_util_test_LDADD = libar_util.la
ar_util_test_CFLAGS = $(AM_CFLAGS) -D__unused=__attribute__\(\(__unused__\)\)
