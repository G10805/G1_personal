# -----------------------------------------------------------------------------
#               Common definitons
# -----------------------------------------------------------------------------

AM_CPPFLAGS = -g -O3
AM_CPPFLAGS += -Werror
AM_CPPFLAGS += -DUSE_BUFFER_HANDLE

AM_CPPFLAGS += -I$(srcdir)/inc
AM_CPPFLAGS += -I$(srcdir)/../common/inc
AM_CPPFLAGS += -I$(srcdir)/../../common/inc

if SUPPORT_DMABUF_HEAP
AM_CPPFLAGS += -DSUPPORT_DMABUF
endif

# -----------------------------------------------------------------------------
#           Make the hyp_video_be deamon (hyp_video_be)
# -----------------------------------------------------------------------------

c_sources  = ../../common/src/hyp_buffer_manager.cpp
c_sources += ../../common/src/hyp_queue_utility.cpp
c_sources += ../common/src/hyp_video_be.cpp
c_sources += ../common/src/hyp_video_be_translation.cpp
c_sources += src/linux_video_be.cpp

bin_PROGRAMS = hyp_video_be
hyp_video_be_SOURCES = $(c_sources)
hyp_video_be_CFLAGS = $(AM_CPPFLAGS) -fPIC
hyp_video_be_LDFLAGS = -shared -lcutils -ldl
