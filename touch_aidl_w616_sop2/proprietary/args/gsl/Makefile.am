pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gsl.pc
EXTRA_DIST = $(pkgconfig_DATA)

if BUILDSYSTEM_OPENWRT
AM_CFLAGS = -I ./inc
AM_CFLAGS += -I ./api
AM_CFLAGS += -I ./stubs
AM_CFLAGS += -I ./rtc_api
AM_CFLAGS += -I ./hw_rsc_api

gsl_sources = ./inc/gsl_common.h \
              ./inc/gsl_graph.h \
              ./inc/gsl_main.h \
              ./inc/gsl_shmem_mgr.h \
              ./inc/gsl_subgraph.h \
              ./inc/gsl_subgraph_pool.h \
              ./inc/gsl_spf_ss_state.h \
              ./inc/gsl_rtc.h \
              ./inc/gsl_rtc_main.h \
              ./api/gsl_intf.h \
              ./inc/gsl_dynamic_module_mgr.h \
              ./rtc_api/gsl_rtc_intf.h \
              ./hw_rsc_api/gsl_hw_rsc_intf.h \
              ./inc/gsl_subgraph_driver_props_generic.h \
              ./api/gsl_subgraph_driver_props.h \
              ./inc/gsl_mdf_utils.h

gsl_c_sources = ./src/gsl_graph.c \
                ./src/gsl_main.c \
                ./src/gsl_shmem_mgr.c \
                ./src/gsl_subgraph.c \
                ./src/gsl_subgraph_pool.c \
                ./src/gsl_common.c \
                ./src/gsl_dynamic_module_mgr.c \
                ./src/gsl_spf_ss_state.c \
                ./src/gsl_rtc.c \
                ./src/gsl_rtc_main.c \
                ./src/gsl_hw_rsc_mgr.c \
                ./src/gsl_mdf_utils.c

else
AM_CFLAGS = -I ${top_srcdir}/inc
AM_CFLAGS += -I ${top_srcdir}/api
AM_CFLAGS += -I ${top_srcdir}/rtc_api
AM_CFLAGS += -I ${top_srcdir}/stubs
AM_CFLAGS += -I $(top_srcdir)/hw_rsc_api
AM_CFLAGS += $(SPF_CFLAGS)

gsl_sources = ${top_srcdir}/inc/gsl_common.h \
              ${top_srcdir}/inc/gsl_graph.h \
              ${top_srcdir}/inc/gsl_main.h \
              ${top_srcdir}/inc/gsl_shmem_mgr.h \
              ${top_srcdir}/inc/gsl_subgraph.h \
              ${top_srcdir}/inc/gsl_subgraph_pool.h \
              ${top_srcdir}/inc/gsl_spf_ss_state.h \
              ${top_srcdir}/inc/gsl_rtc.h \
              ${top_srcdir}/inc/gsl_rtc_main.h \
              ${top_srcdir}/api/gsl_intf.h \
              ${top_srcdir}/inc/gsl_dynamic_module_mgr.h \
              ${top_srcdir}/rtc_api/gsl_rtc_intf.h\
              ${top_srcdir}/hw_rsc_api/gsl_hw_rsc_intf.h \
              ${top_srcdir}/inc/gsl_subgraph_driver_props_generic.h \
              ${top_srcdir}/api/gsl_subgraph_driver_props.h \
              ${top_srcdir}/inc/gsl_mdf_utils.h \
              ${top_srcdir}/inc/gsl_global_persist_cal.h

gsl_c_sources =  ${top_srcdir}/src/gsl_graph.c \
                 ${top_srcdir}/src/gsl_main.c \
                 ${top_srcdir}/src/gsl_shmem_mgr.c \
                 ${top_srcdir}/src/gsl_subgraph.c \
                 ${top_srcdir}/src/gsl_subgraph_pool.c \
                 ${top_srcdir}/src/gsl_common.c \
                 ${top_srcdir}/src/gsl_dynamic_module_mgr.c \
                 ${top_srcdir}/src/gsl_spf_ss_state.c \
                 ${top_srcdir}/src/gsl_rtc.c \
                 ${top_srcdir}/src/gsl_rtc_main.c \
                 $(top_srcdir)/src/gsl_hw_rsc_mgr.c \
                 ${top_srcdir}/src/gsl_mdf_utils.c \
                 ${top_srcdir}/src/gsl_datapath.c \
                 ${top_srcdir}/src/gsl_msg_builder.c \
                 ${top_srcdir}/src/gsl_global_persist_cal.c

endif

lib_includedir = $(includedir)
lib_include_HEADERS = $(gsl_sources)
#LIBRARY for LE apps
lib_LTLIBRARIES = libar_gsl.la
libar_gsl_la_CC = @CC@
libar_gsl_la_LIBADD = -lacdb -lar_osal -lar_util -lgpr
libar_gsl_la_SOURCES = $(gsl_c_sources)
libar_gsl_la_CFLAGS = $(AM_CFLAGS) -DGSL_DEBUG_ENABLE
libar_gsl_la_LDFLAGS = -shared -avoid-version

