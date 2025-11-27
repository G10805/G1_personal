#ifndef __MNLD_TEST_H__
#define __MNLD_TEST_H__

#include"hal_mnl_interface_common.h"
#include"gps_mtk.h"

#define MNLD_TEST_CMD_OPEN "start"
#define MNLD_TEST_CMD_CLOSE "stop"
#define MNLD_TEST_CMD_NETWORK "network"
#define MNLD_TEST_NETWORK_WIFI "wifi"
#define MNLD_TEST_NETWORK_MOBILE "mobile"
#define MNLD_TEST_NETWORK_ROAMING "roaming"
#define MNLD_TEST_NETWORK_DISABLE "disable"

#define MNL_VER_LEN 52
#define MNL_UTC_TIME_LEN 11
#define MNLD_TEST_TTFF_SESSION_BEGIN

typedef enum{
    MNLD_TEST_ACTION_UNKNOWN = -1,
    MNLD_TEST_ACTION_GNSS_OPEN,
    MNLD_TEST_ACTION_GNSS_CLOSE,
    MNLD_TEST_ACTION_SET_NETWORK,
    MNLD_TEST_ACTION_MAX
} MNLD_TEST_ACTION;

typedef enum{
    MNLD_TEST_RESTART_TYPE_UNKNOWN = -1,
    MNLD_TEST_RESTART_TYPE_HOT,
    MNLD_TEST_RESTART_TYPE_WARM,
    MNLD_TEST_RESTART_TYPE_COLD,
    MNLD_TEST_RESTART_TYPE_FULL,
    MNLD_TEST_RESTART_TYPE_MAX
} MNLD_TEST_RESTART_TYPE;

typedef enum{
    CURR = 0,
    MIN = 1,
    MAX = 2,
    MEAN = 3,
    TTFF_NUM
}MNLD_TEST_TTFF;

typedef struct {
    char chip_ver[12];
    char mnl_ver[MNL_VER_LEN];
    char clk_type;
    char clk_buff;
    int ttff[TTFF_NUM];//time-to-first_fix in ms
    int fix_type;
    GpsLocation_ext location;
    char utc_time[MNL_UTC_TIME_LEN];
}mnld_test_result;

typedef struct{
    char type_int;
    char type_str[10];
}clock_type;

#define MNLD_TEST_CMD_CNT_MAX 5
#define GPS_DELETE_EPHEMERIS        0x0001
#define GPS_DELETE_ALMANAC          0x0002
#define GPS_DELETE_POSITION         0x0004
#define GPS_DELETE_TIME             0x0008
#define GPS_DELETE_IONO             0x0010
#define GPS_DELETE_UTC              0x0020
#define GPS_DELETE_HEALTH           0x0040
#define GPS_DELETE_SVDIR            0x0080
#define GPS_DELETE_SVSTEER          0x0100
#define GPS_DELETE_SADATA           0x0200
#define GPS_DELETE_RTI              0x0400
#define GPS_DELETE_CELLDB_INFO      0x8000
#define GPS_DELETE_ALL              0xFFFF


GpsCallbacks_ext* mnld_test__get_gps_callbacks(void);
//GpsInterface* gps_device__get_gps_interface(struct gps_device_t* device);

void mnld_test_show_test_result(mnld_test_result* result);
void mnld_test_get_mnl_ver(void);

#endif //__MNLD_TEST_H__
