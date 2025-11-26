pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gpr.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wall
AM_CFLAGS += -DSESSION_ARRAY_SIZE=200

if BUILDSYSTEM_OPENWRT
AM_CFLAGS += -I ./api
AM_CFLAGS += -I ./core/inc
AM_CFLAGS += -I ./core/src
AM_CFLAGS += -I ./datalinks/gpr_lx/inc

gpr_sources =  ./api/gpr_api.h \
               ./api/gpr_api_inline.h \
               ./api/gpr_comdef.h \
               ./api/gpr_ids_domains.h \
               ./api/gpr_msg_if.h \
               ./api/gpr_pack_begin.h \
               ./api/gpr_pack_end.h \
               ./api/gpr_packet.h \
               ./api/ipc_dl_api.h \
               ./api/ar_guids.h \
               ./api/ar_types.h \
               ./api/ar_std.h \
               ./core/inc/gpr_api_i.h \
               ./core/inc/gpr_list.h \
               ./core/inc/ar_msg.h \
               ./core/src/gpr_memq.h \
               ./datalinks/gpr_lx/inc/gpr_lx.h

gpr_c_sources =  ./core/src/gpr_drv.c \
                 ./core/src/gpr_list.c \
                 ./core/src/gpr_main.c \
                 ./core/src/gpr_memq.c \
                 ./core/src/ar_msg.c \
                 ./core/src/ar_std.c \
                 ./datalinks/gpr_lx/src/gpr_lx.c \
                 ./platform_wrappers/lx/gpr_init_lx_wrapper.c
else
AM_CFLAGS += -I ${top_srcdir}/api
AM_CFLAGS += -I ${top_srcdir}/api/private
AM_CFLAGS += -I ${top_srcdir}/core/inc
AM_CFLAGS += -I ${top_srcdir}/core/src
AM_CFLAGS += -I ${top_srcdir}/datalinks/gpr_lx/inc
AM_CFLAGS += -I ${top_srcdir}/ext/inc
AM_CFLAGS += -I ${top_srcdir}/ext/dynamic_allocation/inc

gpr_sources =  ${top_srcdir}/api/gpr_api.h \
               ${top_srcdir}/api/gpr_api_inline.h \
               ${top_srcdir}/api/gpr_comdef.h \
               ${top_srcdir}/api/gpr_ids_domains.h \
               ${top_srcdir}/api/gpr_msg_if.h \
               ${top_srcdir}/api/gpr_pack_begin.h \
               ${top_srcdir}/api/gpr_pack_end.h \
               ${top_srcdir}/api/gpr_packet.h \
               ${top_srcdir}/api/ipc_dl_api.h \
               ${top_srcdir}/api/ar_types.h \
               ${top_srcdir}/core/inc/gpr_api_i.h \
               ${top_srcdir}/core/inc/gpr_list.h \
               ${top_srcdir}/core/inc/ar_msg.h \
               ${top_srcdir}/core/src/gpr_memq.h \
               ${top_srcdir}/datalinks/gpr_lx/inc/gpr_lx.h \
               $(top_srcdir)/ext/inc/gpr_api_log.h \
               $(top_srcdir)/api/private/gpr_private_api.h \
               $(top_srcdir)/ext/dynamic_allocation/inc/gpr_dynamic_allocation.h

gpr_c_sources =  ${top_srcdir}/core/src/gpr_drv.c \
                 ${top_srcdir}/core/src/gpr_list.c \
                 ${top_srcdir}/core/src/gpr_main.c \
                 ${top_srcdir}/core/src/gpr_memq.c \
                 ${top_srcdir}/datalinks/gpr_lx/src/gpr_lx.c \
                 ${top_srcdir}/platform_wrappers/lx/gpr_init_lx_wrapper.c \
                 ${top_srcdir}/core/src/gpr_drv_island.c \
                 ${top_srcdir}/core/src/gpr_list_island.c \
                 ${top_srcdir}/core/src/gpr_memq_island.c \
                 ${top_srcdir}/core/src/hash_based/gpr_session.c \
                 ${top_srcdir}/core/src/hash_based/gpr_session_island.c \
                 ${top_srcdir}/ext/dynamic_allocation/src/gpr_dynamic_allocation.c \
                 ${top_srcdir}/ext/src/gpr_log.c
endif

lib_includedir = $(includedir)
lib_include_HEADERS = $(gpr_sources)

#LIBRARY for LE apps
lib_LTLIBRARIES = libgpr.la
libgpr_la_CC = @CC@
libgpr_la_LIBADD = $(GLIB_LIBS) -lpthread -lar_osal
if BUILDSYSTEM_OPENWRT
libgpr_la_LIBADD += -lglib-2.0
endif
if CUTILS_SUPPORTED
libgpr_la_LIBADD += -lcutils
endif
libgpr_la_SOURCES = $(gpr_c_sources)
libgpr_la_CFLAGS = $(AM_CFLAGS)
libgpr_la_CFLAGS += $(GLIB_CFLAGS) -Dstrlcpy=g_strlcpy -include glib.h
libgpr_la_LDFLAGS = -shared -avoid-version

