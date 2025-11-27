#ifndef __MNLD_FM_H__
#define __MNLD_FM_H__

#include"hal_mnl_interface_common.h"
#include"gps_mtk.h"

typedef enum{
    MNLD_FM_TEST_OPEN = 0,
    MNLD_FM_TEST_OPENED,
    MNLD_FM_TEST_ENGINE_STARTED,
    MNLD_FM_TEST_SV_SEARCHED,
    MNLD_FM_TEST_STAGE_MAX
} MNLD_FM_TEST_STAGE;

typedef struct {
    int sv_num;
    int sv_list[GNSS_MAX_SVS];
    MNLD_FM_TEST_STAGE test_stage;
}mnld_fm_test_result;

typedef enum{
    MNLD_FM_RESTART_TYPE_UNKNOWN = -1,
    MNLD_FM_RESTART_TYPE_HOT,
    MNLD_FM_RESTART_TYPE_WARM,
    MNLD_FM_RESTART_TYPE_COLD,
    MNLD_FM_RESTART_TYPE_FULL,
    MNLD_FM_RESTART_TYPE_MAX
} MNLD_FM_RESTART_TYPE;


#define MNLD_FM_CMD_CNT_MIN 1
#define MNLD_FM_CMD_CNT_MAX 2
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

GpsCallbacks_ext* mnld_fm__get_gps_callbacks(void);
//GpsInterface* gps_device__get_gps_interface(struct gps_device_t* device);

#endif //__MNLD_FM_H__
