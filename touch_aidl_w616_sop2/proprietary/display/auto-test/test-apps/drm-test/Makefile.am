AM_CPPFLAGS    = -Wno-missing-field-initializers

if DEBUG
AM_CPPFLAGS   += -DDEBUG_BUILD
endif

bin_PROGRAMS = drm-test
drm_test_SOURCES = drm-test.cpp
drm_test_CPPFLAGS = $(AM_CPPFLAGS)
drm_test_LDADD = -ldrm -lion -lpthread

