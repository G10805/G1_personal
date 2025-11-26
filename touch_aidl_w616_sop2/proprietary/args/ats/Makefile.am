pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ats.pc
EXTRA_DIST = $(pkgconfig_DATA)

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -Wall \
            -Wundef \
            -O2
AM_CFLAGS += $(GLIB_CFLAGS) -Dstrlcpy=g_strlcpy -Dstrlcat=g_strlcat -include glib.h

if BUILDSYSTEM_OPENWRT
AM_CFLAGS += -I ./inc \
             -I ./api \
             -I ./diag/Platform/actp/inc \
             -I ./diag/Platform/audtp/inc \
             -I ./diag/Platform/inc \
             -I ./online/inc \
             -I ./rtc/inc \
             -I ./fts/platform/inc \
             -I ./mcs/platform/inc \
             -I ./tcpip/platform/inc \
             -I ./adie/platform/inc \
             -Wno-unknown-pragmas \
             $(DIAG_CFLAGS)

AM_CPPFLAGS = $(AM_CFLAGS)

else

AM_CFLAGS += -I $(WORKSPACE)/common/inc \
             -I $(WORKSPACE)/diag/src \
             -I $(WORKSPACE)/diag/include


AM_CFLAGS += -I ${top_srcdir}/inc \
             -I ${top_srcdir}/api \
             -I ${top_srcdir}/diag/Platform/actp/inc \
             -I ${top_srcdir}/diag/Platform/audtp/inc \
             -I ${top_srcdir}/online/inc \
             -I ${top_srcdir}/rtc/inc \
             -I ${top_srcdir}/fts/platform/inc \
             -I ${top_srcdir}/mcs/platform/inc \
             -I ${top_srcdir}/tcpip/platform/inc \
             -I ${top_srcdir}/adie/platform/inc \
             -I ${top_srcdir}/mcs/inc \
             -I ${top_srcdir}/mcs/api \
             -I ${top_srcdir}/adie/inc \
             -I ${top_srcdir}/adie/api \
              $(DIAG_CFLAGS)
if !LIBDIAG_SUPPORTED
AM_CFLAGS += -I ${top_srcdir}/diag/Platform/inc
endif

AM_CPPFLAGS = $(AM_CFLAGS) \
             -I ${top_srcdir}/tcpip/platform/inc \
             -I ${top_srcdir}/inc \
             -I ${top_srcdir}/api \
             -DFEATURE_LE_DIAG
endif

if BUILDSYSTEM_OPENWRT
ats_headers = ./inc/ats_command.h \
              ./inc/ats_common.h \
              ./api/ats.h \
              ./diag/Platform/actp/inc/actp.h \
              ./diag/Platform/audtp/inc/audtp.h \
              ./diag/Platform/audtp/inc/audtpi.h \
              ./online/inc/ats_online.h \
              ./rtc/inc/ats-rtc.h \
              ./fts/platform/inc/ats-fts.h \
              ./mcs/platform/inc/ats-mcs.h \
              ./tcpip/platform/inc/ats-server.h \
              ./adie/platform/inc/adie_rtc.h \
              ./adie/platform/inc/ats-adie-rtc.h

ats_c_sources = ./src/ats.c \
                ./src/ats_command.c \
                ./diag/Platform/actp/src/actp.c \
                ./diag/Platform/audtp/src/audtp.c \
                ./online/src/ats_online.c \
                ./rtc/src/ats-rtc.c \
                ./fts/platform/src/ats-fts.c \
                ./mcs/platform/src/ats-mcs.c \
                ./tcpip/platform/src/ats_server.cpp \
                ./adie/platform/src/ats-adie-rtc.c
else
ats_headers = ${top_srcdir}/inc/ats_command.h \
               ${top_srcdir}/inc/ats_common.h \
               ${top_srcdir}/api/ats.h \
               ${top_srcdir}/diag/Platform/actp/inc/actp.h \
               ${top_srcdir}/diag/Platform/audtp/inc/audtp.h \
               ${top_srcdir}/diag/Platform/audtp/inc/audtpi.h \
               ${top_srcdir}/online/inc/ats_online.h \
               ${top_srcdir}/rtc/inc/ats_rtc.h \
               ${top_srcdir}/fts/platform/inc/ats_fts.h \
               ${top_srcdir}/mcs/inc/ats_mcs.h \
               ${top_srcdir}/mcs/api/mcs_api.h \
               ${top_srcdir}/tcpip/platform/inc/ats_server.h \
               ${top_srcdir}/adie/inc/ats_adie_rtc.h \
               ${top_srcdir}/adie/api/adie_rtc_api.h

ats_c_sources = ${top_srcdir}/src/ats.c \
                ${top_srcdir}/src/ats_command.c \
                ${top_srcdir}/diag/Platform/actp/src/actp.c \
                ${top_srcdir}/diag/Platform/audtp/src/audtp.c \
                ${top_srcdir}/online/src/ats_online.c \
                ${top_srcdir}/rtc/src/ats_rtc.c \
                ${top_srcdir}/fts/platform/src/ats_fts.c \
                ${top_srcdir}/mcs/src/ats_mcs.c \
                ${top_srcdir}/tcpip/platform/src/ats_server.cpp \
                ${top_srcdir}/adie/src/ats_adie_rtc.c \
                ${top_srcdir}/adie/platform/lx/adie_rtc.c \
                ${top_srcdir}/mcs/platform/lx/mcs.c
endif

#Includes
lib_includedir = $(includedir)
lib_include_HEADERS = $(ats_headers)
requiredlibs = -lrt
if BUILDSYSTEM_OPENWRT
requiredlibs += -ldiag
else
requiredlibs += $(DIAG_LIBS)
endif

#LIBRARY for LE apps
lib_LTLIBRARIES = libats.la
libats_la_CC = @CC@
libats_la_LIBADD = -lacdb -lar_osal -lar_gsl -lutils -lcutils
if BUILDSYSTEM_OPENWRT
libats_la_LIBADD += -ldiag
else
if LIBDIAG_SUPPORTED
libats_la_LIBADD += -ldiag
else
libats_la_LIBADD += $(DIAG_LIBS)
endif
endif

libats_la_SOURCES = $(ats_c_sources)
libats_la_CFLAGS = $(AM_CFLAGS)
libats_la_CPPFLAGS = $(AM_CPPFLAGS)
libats_la_LDFLAGS = $(requiredlibs) -shared -avoid-version
