AUTOMAKE_OPTIONS=foreign subdir-objects
bin_PROGRAMS = adrd

adrd_SOURCES = src/main.c \
        src/module.c \
        src/module_sensor.c \
        src/module_sensor_helper.c \
        src/module_gnss.c \
        src/module_gnss_helper.c \
        src/module_odom.c \
        src/module_adr_algo.c \
        src/module_adr_algo_helper.c \
        src/module_debug.c \
        src/adr_data_convert.c \
        src/config-parser.c \
        src/adr_util.c \
        src/log.c \
        src/module_sanity.c \
        src/module_data_store.c \
        src/module_daemon.c \
        src/data_coder.c

AM_CFLAGS = -Iinc \
		-Iinc/GNSS

AM_CFLAGS+=$(DNS_FLAGS)

adrd_LDADD   = -lpthread -ldl -lm -ladr -ludev -lvehicle_info
adrd_LDADD+=$(Lib_FLAGS)

DEFS+=-D__LINUX_OS__ -DV_LIBDIR='"$(libdir)/libvehicle_info.so.0"'
