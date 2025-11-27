AUTOMAKE_OPTIONS=foreign subdir-objects
bin_PROGRAMS = mnld_test fm_gnss


AM_CFLAGS = -Imnld_client/inc \
            -Imnld_fm/inc \
            -I../gps_hal/inc \
            -I../gps_hal/inc/hardware \
            -DLIB_GNSS_HAL_DIR='"$(libdir)"'
AM_CFLAGS+=$(DNS_FLAGS)

mnld_test_SOURCES = mnld_client/src/mnld_client.c \
            mnld_client/src/mnld_client_gps_cb.c

mnld_test_LDADD = -lpthread -lrt -ldl -lgpshal

fm_gnss_SOURCES = mnld_fm/src/mnld_fm.c \
            mnld_fm/src/mnld_fm_gps_cb.c

fm_gnss_LDADD = -lpthread -lrt -ldl -lgpshal

DEFS+=-D__LINUX_OS__ -DMTK_GPS_DATA_PATH="\"/usr/share/gps/\""
