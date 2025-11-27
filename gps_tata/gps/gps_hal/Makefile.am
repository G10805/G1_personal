AUTOMAKE_OPTIONS=foreign subdir-objects
lib_LTLIBRARIES = libgpshal.la

libgpshal_la_SOURCES =  src/hal2mnl_interface.c \
		src/hal_mnl_interface_common.c \
		src/data_coder.c \
		src/mtk_lbs_utility.c \
		src/agpsinf.c \
		src/gpshal.c \
		src/gpshal_worker.c \
		src/gfchal_mnl_interface.c \
		src/geofenceinf.c \
  		src/mtk_auto_log.c \
  		src/gpsinf.c

AM_CFLAGS = -Iinc \
		-Iinc/hardware \
		-Ihardware

include_HEADERS = hardware/gps.h \
		inc/hardware/gps_mtk.h \
		hardware/gps_internal.h \
		inc/hal_mnl_interface_common.h \
		inc/mtk_lbs_utility.h \
		inc/hal2mnl_interface.h

libgpshal_la_LIBADD = -lrt

DEFS+=-D__LINUX_OS__ -DMTK_GPS_DATA_PATH="\"/usr/share/gps/\""
