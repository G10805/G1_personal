AUTOMAKE_OPTIONS=foreign subdir-objects
bin_PROGRAMS = mnld0

mnld0_SOURCES = \
        mnld_entity/src/mnl2hal_interface.c \
        utility/src/data_coder.c \
        utility/src/mtk_lbs_utility.c \
        utility/src/mtk_socket_data_coder.c \
        utility/src/mtk_socket_utils.c \
        mnl_agps_interface/src/mnl_agps_interface.c \
        mnl_agps_interface/src/mnl2agps_interface.c \
        mnl_agps_interface/src/agps2mnl_interface.c \
        mnl_flp_interface/src/mtk_flp_controller.c \
        mnl_flp_interface/src/mtk_flp_main.c \
        mnl_flp_interface/src/mtk_flp_mnl_interface.c \
        mnl_flp_interface/src/mtk_flp_screen_monitor.c \
        mnl_flp_interface/src/mnl_flp_test_interface.c \
        mnl_geofence_interface/src/mtk_geofence_controller.c \
        mnl_geofence_interface/src/mtk_geofence_main.c \
        mnl_nlp_interface/src/Mnld2NlpUtilsInterface.c \
        mnl_meta_interface/src/Meta2MnldInterface.c \
        mnl_debug_interface/src/Debug2MnldInterface.c \
        mnl_debug_interface/src/Mnld2DebugInterface.c \
        mnl_at_cmd_interface/src/mnl_at_interface.c \
        mnld_entity/src/mnld.c \
        mnld_entity/src/mnld_uti.c \
        mnld_entity/src/gps_controller.c \
        mnld_entity/src/nmea_parser.c \
        mnld_entity/src/epo.c \
        mnld_entity/src/qepo.c \
        mnld_entity/src/mtknav.c \
        mnld_entity/src/mnl_common.c \
        mnld_entity/src/op01_log.c \
        mnld_entity/src/gps_dbg_log.c \
        mnl/src/pseudo_mnl.c \
        utility/src/mtk_auto_log.c \
        mnld_entity/src/mpe.c \
        mnl_mpe_interface/src/mpe_main.c \
        mnl_mpe_interface/src/mpe_logger.c \
        mnld_entity/src/mt3333_controller.c \
        mnld_entity/src/flashdownload/flashtool.c \
	 mnld_entity/src/flashdownload/brom_base.c \
	 mnld_entity/src/flashdownload/brom_mt3301.c \
	 mnld_entity/src/flashdownload/da_cmd.c \
 	 mnld_entity/src/flashdownload/gps_uart.c

AM_CFLAGS = \
        -Iutility/inc \
        -Imnl_agps_interface/inc \
        -Imnl_at_cmd_interface/inc \
        -Imnl_flp_interface/inc \
        -Imnl_mpe_interface/inc \
        -Imnl_nlp_interface/inc \
        -Imnl_meta_interface/inc \
        -Imnl_debug_interface/inc \
        -Imnl_geofence_interface/inc \
        -Imnld_entity/inc \
        -Imnld_entity/hardware \
        -Imnld_entity/src/flashdownload \
        -Imnl/inc \
        -Icurl/inc \
        -Imnl_mpe_interface/inc \
        -D__LINUX_OS__ \
        -DMTK_GPS_DATA_PATH="\"/usr/share/gps/\""

AM_CFLAGS+=$(DNS_FLAGS)

mnld0_LDADD = mnl/libs/linux/${PACKAGE_ARCH}/libmnl_gnss.so \
        mnl/libs/linux/${PACKAGE_ARCH}/libhotstill.a \
        mnl/libs/linux/${PACKAGE_ARCH}/libsupl.a \
        -lm -lrt -lpthread -lz -lssl -lcrypto -lcurl

include_HEADERS = mnl/inc/mtk_gps_type.h
