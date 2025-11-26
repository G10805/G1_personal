pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ar_osal.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wall -Wno-pointer-to-int-cast
AM_CFLAGS += -D__unused=__attribute__\(\(__unused__\)\) -DLINUX_ENABLED
if BUILDSYSTEM_OPENWRT
AM_CFLAGS += -I ./api
AM_CFLAGS += -I ./test/inc

ar_osal_sources = ./api/ar_osal_error.h \
                  ./api/ar_osal_file_io.h \
                  ./api/ar_osal_log.h \
                  ./api/ar_osal_mutex.h \
                  ./api/ar_osal_shmem.h \
                  ./api/ar_osal_signal.h \
                  ./api/ar_osal_sleep.h \
                  ./api/ar_osal_thread.h \
                  ./api/ar_osal_types.h \
                  ./api/ar_osal_servreg.h \
                  ./api/ar_osal_timer.h \
                  ./api/ar_osal_sys_id.h


ar_osal_c_sources = ./src/ar_osal_file_io.c \
                    ./src/ar_osal_log.c \
                    ./src/ar_osal_mutex.c \
                    ./src/ar_osal_thread.c \
                    ./src/ar_osal_signal.c \
                    ./src/ar_osal_sleep.c \
                    ./src/ar_osal_shmem.c \
                    ./src/ar_osal_timer.c \
                    ./src/ar_osal_servreg.c


else
AM_CFLAGS += -I ${top_srcdir}/api
AM_CFLAGS += -I ${top_srcdir}/test/inc

ar_osal_sources = ${top_srcdir}/api/ar_osal_error.h \
                  ${top_srcdir}/api/ar_osal_file_io.h \
                  ${top_srcdir}/api/ar_osal_log.h \
                  ${top_srcdir}/api/ar_osal_mutex.h \
                  ${top_srcdir}/api/ar_osal_shmem.h \
                  ${top_srcdir}/api/ar_osal_signal.h \
                  ${top_srcdir}/api/ar_osal_sleep.h \
                  ${top_srcdir}/api/ar_osal_thread.h \
                  ${top_srcdir}/api/ar_osal_types.h \
                  ${top_srcdir}/api/ar_osal_servreg.h \
                  ${top_srcdir}/api/ar_osal_timer.h \
                  ${top_srcdir}/api/ar_osal_sys_id.h \
                  ${top_srcdir}/api/ar_osal_mem_op.h \
                  ${top_srcdir}/api/ar_osal_heap.h \
                  ${top_srcdir}/api/ar_osal_string.h \
                  ${top_srcdir}/api/ar_osal_log_pkt_op.h

ar_osal_c_sources = ${top_srcdir}/src/ar_osal_file_io.c \
                    ${top_srcdir}/src/ar_osal_log.c \
                    ${top_srcdir}/src/ar_osal_mutex.c \
                    ${top_srcdir}/src/ar_osal_thread.c \
                    ${top_srcdir}/src/ar_osal_signal.c \
                    ${top_srcdir}/src/ar_osal_sleep.c \
                    ${top_srcdir}/src/ar_osal_timer.c \
                    ${top_srcdir}/src/ar_osal_servreg.c \
                    ${top_srcdir}/src/ar_osal_mem_op.c \
                    ${top_srcdir}/src/ar_osal_heap.c \
                    ${top_srcdir}/src/ar_osal_string.c \
                    ${top_srcdir}/src/ar_osal_log_pkt_op.c
if LIBION_SUPPORTED
ar_osal_c_sources  += ${top_srcdir}/src/ar_osal_shmem.c
else
ar_osal_c_sources += ${top_srcdir}/src/ar_osal_shmem_db.c
endif
endif


lib_includedir = $(includedir)
lib_include_HEADERS = $(ar_osal_sources)
#LIBRARY for LE apps
lib_LTLIBRARIES = libar_osal.la
libar_osal_la_CC = @CC@
libar_osal_la_LIBADD = -lrt -lpthread -lcutils -lpdnotifier -lpdmapper
if LIBION_SUPPORTED
libar_osal_la_LIBADD += -lion
endif
libar_osal_la_LIBADD += -lglib-2.0 -ldiag
libar_osal_la_SOURCES = $(ar_osal_c_sources)
libar_osal_la_CFLAGS = $(AM_CFLAGS) -Dstrlcpy=g_strlcpy -Dstrlcat=g_strlcat -DUSE_GLIB @GLIB_CFLAGS@ -D__unused=__attribute__\(\(__unused__\)\) -D__LINUX__ -DAR_OSAL_USE_PD_NOTIFIER
libar_osal_la_LDFLAGS = -shared -avoid-version

if BUILDSYSTEM_OPENWRT
test_sources = ./test/src/ar_osal_test.c \
               ./test/src/ar_test_file_io.c \
               ./test/src/ar_test_signal_mutex_thread.c \
               ./test/src/ar_test_shmem.c
else
test_sources = ${top_srcdir}/test/src/ar_osal_test.c \
               ${top_srcdir}/test/src/ar_test_file_io.c \
               ${top_srcdir}/test/src/ar_test_signal_mutex_thread.c \
               ${top_srcdir}/test/src/ar_test_shmem.c
endif

bin_PROGRAMS = ar_osal_test
ar_osal_test_CC = @CC@
ar_osal_test_SOURCES = $(test_sources)
ar_osal_test_LDADD = libar_osal.la
if LIBION_SUPPORTED
ar_osal_test_LDADD += -lion
endif
ar_osal_test_CFLAGS = $(AM_CFLAGS)

libar_osal_la_LDFLAGS += -lcutils -lm