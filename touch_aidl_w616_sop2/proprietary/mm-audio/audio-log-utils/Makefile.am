
h_sources = ${top_srcdir}/inc/log_utils.h \
            ${top_srcdir}/inc/log_xml_parser.h

library_include_HEADERS = $(h_sources)
library_includedir = $(includedir)

AM_CPPFLAGS := -I $(top_srcdir)/inc/ -DLOG_XML_PATH=\"/etc/audio_dynamic_log.xml\"
lib_LTLIBRARIES      = libaudio_log_utils.la
libaudio_log_utils_la_SOURCES   = src/log_xml_parser.c src/log_utils.c
libaudio_log_utils_la_CPPFLAGS := $(AM_CPPFLAGS)
libaudio_log_utils_la_LDFLAGS   = -lcutils -llog -shared -avoid-version -lutils -lrt -lexpat
