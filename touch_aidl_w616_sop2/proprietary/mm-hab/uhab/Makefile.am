AM_CFLAGS := \
    -W \
    -Wno-undef \
    -Wall -g \
    -Wundef \
    -Wstrict-prototypes \
    -Wno-trigraphs \
    -fPIC

ACLOCAL_AMFLAGS = -I m4

libuhab_sources = uhab.c

requiredlibs = -lpthread -lglib-2.0 -shared -avoid-version

# Build libuhab.so
lib_LTLIBRARIES = libuhab.la

libuhab_la_SOURCES := $(libuhab_sources)
libuhab_la_CFLAGS := $(AM_CFLAGS)
libuhab_la_CFLAGS += -DUSE_GLIB -Dstrlcpy=g_strlcpy -shared
libuhab_la_CFLAGS += $(GLIB_CFLAGS) -include glib.h
libuhab_la_CFLAGS += -include glibconfig.h
libuhab_la_LDFLAGS := $(requiredlibs)

# Export headers
uhab_includedir=$(includedir)/
uhab_include_HEADERS := habmm.h

pkgconfigdir := $(libdir)/pkgconfig
pkgconfig_DATA := libuhab.pc
EXTRA_DIST := $(pkgconfig_DATA)
