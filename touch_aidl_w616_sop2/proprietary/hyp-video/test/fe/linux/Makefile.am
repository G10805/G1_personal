# -----------------------------------------------------------------------------
#               Common definitons
# -----------------------------------------------------------------------------

AM_CPPFLAGS  = -I$(srcdir)/inc
AM_CPPFLAGS += -I$(srcdir)/../../../common/inc
AM_CPPFLAGS += -I$(srcdir)/../../../fe/common/inc

# -----------------------------------------------------------------------------
#           Make hypervisor test binary(hypvideotest)
# -----------------------------------------------------------------------------

c_sources = src/hyp_video_test.cpp
c_sources += src/hyp_vinput_test.cpp

bin_PROGRAMS = hypvideotest
hypvideotest_SOURCES = $(c_sources)
hypvideotest_CPPFLAGS = $(AM_CPPFLAGS)
hypvideotest_LDADD = ../../../fe/linux/libhyp_video_fe.la
