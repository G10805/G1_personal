# -----------------------------------------------------------------------------
#               Common definitons
# -----------------------------------------------------------------------------

AM_CFLAGS = -g -O3
AM_CFLAGS += -Werror

AM_CPPFLAGS = -D_ANDROID_
AM_CPPFLAGS += -DHAVE_ANDROID_OS
AM_CPPFLAGS += -I$(top_srcdir)/mm-video-v4l2/vidc/common/inc

# -----------------------------------------------------------------------------
#           Make the Shared library (libhyp_video_intercept)
# -----------------------------------------------------------------------------
h_sources = hyp_video_intercept.h
c_sources = hyp_video_intercept.cpp

include_HEADERS = $(h_source)
lib_LTLIBRARIES = libhyp_video_intercept.la
libhyp_video_intercept_la_SOURCES = $(c_sources)
libhyp_video_intercept_la_CFLAGS = $(AM_CFLAGS) -fPIC
libhyp_video_intercept_la_CPPFLAGS = $(AM_CPPFLAGS)
libhyp_video_intercept_la_LDFLAGS = -shared -llog -ldl
