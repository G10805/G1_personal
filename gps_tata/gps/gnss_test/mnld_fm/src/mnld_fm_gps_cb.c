#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

#include"mnld_fm.h"
#include"mtk_lbs_utility.h"
#include "mtk_auto_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "mnld_fm_test"
#endif

mnld_fm_test_result mnld_fm_test_result_body;

void mnld_fm_gps_location_callback(GpsLocation_ext* location)
{

//    memset(&result,0,sizeof(mnld_fm_result));
    if(location->legacyLocation.size == sizeof(GpsLocation_ext))
    {
        LOGD("===============Update Location Info==================");
        LOGD("flags:0x%x", location->legacyLocation.flags);
        LOGD("latitude:%.10lf", location->legacyLocation.latitude);
        LOGD("longitude:%.10lf", location->legacyLocation.longitude);
        LOGD("altitude:%.10lf", location->legacyLocation.altitude);
        LOGD("speed:%f", location->legacyLocation.speed);
        LOGD("bearing:%f", location->legacyLocation.bearing);
        LOGD("timestamp:%d", location->legacyLocation.timestamp);
        LOGD("horizontalAccuracyMeters:%f", location->horizontalAccuracyMeters);
        LOGD("verticalAccuracyMeters:%f", location->verticalAccuracyMeters);
        LOGD("speedAccuracyMetersPerSecond:%f", location->speedAccuracyMetersPerSecond);
        LOGD("bearingAccuracyDegrees:%f", location->bearingAccuracyDegrees);
    }else {
        LOGE("GpsLocation_ext size is wrong");
    }
}

void mnld_fm_gps_status_callback(GpsStatus* status)
{
    if(status->size == sizeof(GpsStatus))
    {
        LOGD("GPS Status:%d\r\n", status->status);
        if(status->status == GPS_STATUS_SESSION_BEGIN)
        {
            mnld_fm_test_result_body.test_stage = MNLD_FM_TEST_ENGINE_STARTED;
        }

    }else{
        LOGE("size error!");
    }
}

void mnld_fm_gps_sv_status_callback(GpsSvStatus* sv_info)
{
    int idx = 0;
    mnld_fm_test_result_body.test_stage = MNLD_FM_TEST_SV_SEARCHED;
    for(idx=0; idx < sv_info->num_svs; idx++)
    {
        mnld_fm_test_result_body.sv_list[idx] = sv_info->sv_list[idx].prn;
    }
    mnld_fm_test_result_body.sv_num = sv_info->num_svs;
    LOGD("%d",mnld_fm_test_result_body.sv_num);
}

void mnld_fm_gps_nmea_callback(GpsUtcTime timestamp, const char* nmea, int length)
{
//    LOGD("%d",timestamp);
}

void mnld_fm_gps_set_capabilities(uint32_t capabilities)
{

    LOGD("gps set capabilities");
}

void mnld_fm_gps_acquire_wakelock(void)
{

    LOGD("gps acquire wakelock");
}

void mnld_fm_gps_release_wakelock(void)
{

    LOGD("gps release wakelock");
}

void mnld_fm_gps_request_utc_time(void)
{

    LOGD("gps request utc time");
}

void mnld_fm_set_system_info_cb(const GnssSystemInfo* info)
{
    LOGD("set system info");
}

#define BEIDOU_SVID_OFFSET 200
#define GALILEO_SVID_OFFSET 300
#define GLNOASS_SVID_OFFSET 64
void mnld_fm_gnss_sv_status_cb(GnssSvStatus_ext* sv_info)
{
    int idx = 0;
    mnld_fm_test_result_body.test_stage = MNLD_FM_TEST_SV_SEARCHED;
    for(idx=0; idx < sv_info->num_svs; idx++)
    {
        mnld_fm_test_result_body.sv_list[idx] = sv_info->gnss_sv_list[idx].legacySvInfo.svid;
        if(sv_info->gnss_sv_list[idx].legacySvInfo.constellation == GNSS_CONSTELLATION_BEIDOU) {
            mnld_fm_test_result_body.sv_list[idx] += BEIDOU_SVID_OFFSET;
        } else if(sv_info->gnss_sv_list[idx].legacySvInfo.constellation == GNSS_CONSTELLATION_GALILEO) {
            mnld_fm_test_result_body.sv_list[idx] += GALILEO_SVID_OFFSET;
        } else if(sv_info->gnss_sv_list[idx].legacySvInfo.constellation == GNSS_CONSTELLATION_GLONASS) {
            mnld_fm_test_result_body.sv_list[idx] += GLNOASS_SVID_OFFSET;
        }
    }
    mnld_fm_test_result_body.sv_num = sv_info->num_svs;
    LOGD("%d",mnld_fm_test_result_body.sv_num);
}


pthread_t mnld_fm_gps_create_thread(const char* name, void (*start)(void *), void* arg)
{
    pthread_t ntid = 0;
    int ret = 0;

    ret = pthread_create(&ntid, NULL, start, arg);

    if(ret != 0)
    {
        LOGE("thread %s create fail(%s)!\r\n", name, strerror(errno));
        ntid = 0;
    }else{
        LOGD("tread %s create success!\r\n", name);
    }

    return ntid;
}

void mnld_fm_gnss_set_name_cb(const char* name, int length)
{
    LOGD("gnss set name");
}

void mnld_fm_gnss_request_location_cb(bool independentFromGnss)
{
    LOGD("gnss request location");
}

GpsCallbacks_ext mnld_fm_gps_callbacks = {
    .size = sizeof(GpsCallbacks_ext),
    .location_cb = mnld_fm_gps_location_callback,
    .status_cb = mnld_fm_gps_status_callback,
    .sv_status_cb = mnld_fm_gps_sv_status_callback,
    .nmea_cb = mnld_fm_gps_nmea_callback,
    .set_capabilities_cb = mnld_fm_gps_set_capabilities,
    .acquire_wakelock_cb = mnld_fm_gps_acquire_wakelock,
    .release_wakelock_cb = mnld_fm_gps_release_wakelock,
    .create_thread_cb = mnld_fm_gps_create_thread,
    .request_utc_time_cb = mnld_fm_gps_request_utc_time,
    .set_system_info_cb = mnld_fm_set_system_info_cb,
    .gnss_sv_status_cb = mnld_fm_gnss_sv_status_cb,
    .set_name_cb = mnld_fm_gnss_set_name_cb,
    .request_location_cb = mnld_fm_gnss_request_location_cb,
};

GpsCallbacks_ext* mnld_fm__get_gps_callbacks(void)
{
    return &mnld_fm_gps_callbacks;
}
