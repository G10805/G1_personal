# -----------------------------------------------------------------------------
#               Common definitons
# -----------------------------------------------------------------------------

AM_CPPFLAGS = -g -O3
AM_CPPFLAGS += -Werror -Wno-multichar
# set FLAGS for omx_vdec.h
AM_CPPFLAGS += -DNATIVE_BASE_DISABLE

AM_CPPFLAGS += -I$(srcdir)/inc
AM_CPPFLAGS += -I$(srcdir)/../common/inc
AM_CPPFLAGS += -I$(srcdir)/../../common/inc

if SUPPORT_DMABUF_HEAP
AM_CPPFLAGS += -DSUPPORT_DMABUF
endif

# -----------------------------------------------------------------------------
#           Make the Shared library (libhyp_video_fe)
# -----------------------------------------------------------------------------

c_sources  = ../../common/src/hyp_buffer_manager.cpp
c_sources += ../../common/src/hyp_queue_utility.cpp
c_sources += ../common/src/hyp_video_fe.cpp
c_sources += ../common/src/hyp_video_fe_translation.cpp
c_sources += ../common/src/hyp_video_meta_translator.cpp
c_sources += src/linux_video_fe.cpp
c_sources += src/linux_vdec_fe.cpp
c_sources += src/linux_vdec_venc_common.cpp
c_sources += src/linux_venc_fe.cpp
c_sources += src/linux_vinput_fe.cpp

lib_LTLIBRARIES = libhyp_video_fe.la
libhyp_video_fe_la_SOURCES = $(c_sources)
libhyp_video_fe_la_CFLAGS = $(AM_CPPFLAGS) -fPIC
libhyp_video_fe_la_CPPFLAGS = $(AM_CPPFLAGS) -fPIC
libhyp_video_fe_la_LDFLAGS = -shared -lcutils -ldl
if SUPPORT_DMABUF_HEAP
libhyp_video_fe_la_LDFLAGS += -ldmabufheap
endif
