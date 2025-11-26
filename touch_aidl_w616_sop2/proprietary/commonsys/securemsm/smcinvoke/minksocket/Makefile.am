AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = -Wall -g \
    -std=c11 \
    -Wno-unused-parameter \
    -Werror \
    $(SECUREMSMNOSHIP_CFLAGS) \
    -DOE \
    -Dstrlcpy=g_strlcpy -include glib.h -DUSE_GLIB $(GLIB_CFLAGS) -D_XOPEN_SOURCE=600 \
    -I$(WORKSPACE)/qmi-framework/qrtr/lib \

LOCAL_INCLUDE_PATHS = -Iinclude -Ikernel -Isock

AM_CPPFLAGS = $(LOCAL_INCLUDE_PATHS)

lib_LTLIBRARIES = libminksocket.la
libminksocket_la_LIBADD = $(GLIB_LIBS) -lpthread -lqrtr

libminksocket_la_SOURCES = \
    sock/bbuf.c \
    sock/minkipc.c \
    sock/minksocket.c \
    sock/msforwarder.c \
    sock/fdwrapper.c \
    sock/threadpool.c

library_includedir = $(pkgincludedir)

library_include_HEADERS = include/heap.h \
                          sock/fdwrapper.h \
                          sock/heap_port.h \
                          sock/minkipc.h

libminksocket_la_LDFLAGS = -shared -version-info 1:0:0
