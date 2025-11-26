pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mcs.pc
EXTRA_DIST = $(pkgconfig_DATA)

if BUILDSYSTEM_OPENWRT
h_sources = ./inc/mcs.h \
            ./inc/ats.h

AM_CFLAGS := -I ./inc
else
h_sources = ${top_srcdir}/inc/mcs.h

AM_CFLAGS := -I ${top_srcdir}/inc
AM_CFLAGS +=  -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/pal/
endif

AM_CFLAGS += $(ACDBDATA_CFLAGS)

library_include_HEADERS = $(h_sources)
library_includedir = $(includedir)/mcs

if BUILDSYSTEM_OPENWRT
mcs_sources = ./src/mcs.c
lib_LTLIBRARIES = libaudiomcs.la
libaudiomcs_la_SOURCES = $(mcs_sources)

libaudiomcs_la_LIBADD = -lpal -lar_osal $(GLIB_LIBS)

libaudiomcs_la_CFLAGS = $(AM_CFLAGS) $(GLIB_CFLAGS) -Dstrlcpy=g_strlcpy -Dstrlcat=g_strlcat -include glib.h
libaudiomcs_la_CFLAGS += -DACDB_PATH="/vendor/etc/acdbdata/MTP" -DACDB_DELTA_FILE_PATH="/data/vendor/audio/acdbdata/delta"
libaudiomcs_la_LDFLAGS = -shared -avoid-version -ltinyalsa
else
lib_LTLIBRARIES = libmcs.la
mcs_sources = ${top_srcdir}/src/mcs.c
libmcs_la_SOURCES = $(mcs_sources)
libmcs_la_LIBADD = -laudioroute -lar_osal $(GLIB_LIBS)
libmcs_la_CFLAGS = $(AM_CFLAGS) $(GLIB_CFLAGS) -Dstrlcpy=g_strlcpy -Dstrlcat=g_strlcat -include glib.h
libmcs_la_CFLAGS += -DACDB_PATH="/etc/acdbdata/IDP" -DACDB_DELTA_FILE_PATH="/etc/acdbdata/delta"
libmcs_la_LDFLAGS = -shared -avoid-version -ltinyalsa
endif

