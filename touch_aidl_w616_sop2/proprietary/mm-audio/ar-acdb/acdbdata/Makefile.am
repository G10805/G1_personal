pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = acdbdata.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wall -Werror
if BUILDSYSTEM_OPENWRT
AM_CFLAGS += -I ./inc

acdbdata_sources = ./inc/kvh2xml.h
endif

lib_includedir = $(includedir)
lib_include_HEADERS = $(acdbdata_sources)

#LIBRARY for LE apps
lib_LTLIBRARIES = libacdbdata.la
libacdbdata_la_CC = @CC@
libacdbdata_la_SOURCES = $(acdbdata_sources)
libacdbdata_la_CFLAGS = $(AM_CFLAGS)
libacdbdata_la_LDFLAGS = -shared -avoid-version
