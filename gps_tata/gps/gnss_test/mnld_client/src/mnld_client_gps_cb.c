#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

#include"mnld_test.h"
#include"mtk_lbs_utility.h"
#include "mtk_auto_log.h"

extern struct timespec mnld_test_gnss_open_tm;
extern int mnld_test_ttff;
extern int mnld_test_session_end;
GpsLocation_ext location_rslt;
mnld_test_result mnld_test_result_body;

void mnld_test_gps_location_callback(GpsLocation_ext* location)
{

//    memset(&result,0,sizeof(mnld_test_result));
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
        memcpy(&(mnld_test_result_body.location), location, sizeof(GpsLocation_ext));
    }else {
        LOGE("GpsLocation_ext size is wrong");
    }
#if 0
        memcpy(&(result.location), location, sizeof(GpsLocation_ext));
        if((location->flags & GPS_LOCATION_HAS_ACCURACY) && (mnld_test_ttff == 0))
        {
            struct timespec fix_tm;
            if(clock_gettime(CLOCK_BOOTTIME,&fix_tm) == -1)
            {
                result.ttff = 0;
                LOGE("[%s]Fail to get time(%s)\r\n",__func__,strerror(errno));
            }else{
                LOGD("Flags:0x%x,start time:%ds,%dns; ttff time:%ds,%dns\r\n",result.location.flags,mnld_test_gnss_open_tm.tv_sec,mnld_test_gnss_open_tm.tv_nsec,fix_tm.tv_sec,fix_tm.tv_nsec);
                result.ttff = (fix_tm.tv_sec-mnld_test_gnss_open_tm.tv_sec)*1000+((fix_tm.tv_nsec-mnld_test_gnss_open_tm.tv_nsec)/1000000);
                mnld_test_ttff = result.ttff;
                LOGD("TTFF:%dms",mnld_test_ttff);
            }
        }else{
            result.ttff = mnld_test_ttff;
        }
        result.location.timestamp = location->timestamp;
        LOGD("ts:%d,result.timestamp:%ld\r\n",location->timestamp,result.location.timestamp);
        mnld_test_show_test_result(&result);
    }else{
        LOGE("[%s]size error!\r\n", __func__);
    }
#endif
    
}

void mnld_test_gps_status_callback(GpsStatus* status)
{
    if(status->size == sizeof(GpsStatus))
    {
        LOGD("GPS Status:%d", status->status);
        if(status->status == GPS_STATUS_SESSION_BEGIN)
        {
            mnld_test_ttff = 0;
            mnld_test_session_end = 0;
//          usleep(500000);
            mnld_test_get_mnl_ver();
        #ifdef MNLD_TEST_TTFF_SESSION_BEGIN
            if(clock_gettime(CLOCK_BOOTTIME,&mnld_test_gnss_open_tm) == -1)
            {
                LOGE("Fail to get time(%s).", strerror(errno));
            }
        #endif
        }

        if(status->status == GPS_STATUS_SESSION_END)
        {
            mnld_test_session_end = 1;
        }
    }else{
        LOGE("size error!");
    }
}

void mnld_test_gps_sv_status_callback(GpsSvStatus* sv_info)
{
    LOGD("gps sv status");
}

#define NMEA_GGA "GGA"
#define NMEA_GSA "GSA"
#define NMEA_ACC "ACCURACY"
extern int valid_ttff_cnt;
extern int valid_ttff_sum;
void mnld_test_gps_nmea_callback(GpsUtcTime timestamp, const char* nmea, int length)
{
//    LOGD("%d",timestamp);

    //$GPGSA,A,3,...
    if( strncmp(nmea+3,NMEA_GSA,strlen(NMEA_GSA)) == 0 )
    {
        mnld_test_result_body.fix_type = *(nmea+9) - '0';
        if(mnld_test_result_body.fix_type == 1)
        {
            mnld_test_result_body.fix_type = 0;
        }
 
        if((mnld_test_ttff == 0) && mnld_test_result_body.fix_type != 0)
        {
            struct timespec fix_tm;

            if(clock_gettime(CLOCK_BOOTTIME,&fix_tm) == -1)
            {
                mnld_test_result_body.ttff[CURR] = 0;
                LOGE("[%s]Fail to get time(%s)\r\n",__func__,strerror(errno));
            }else{
               // LOGD("Flags:0x%x,start time:%ds,%dns; ttff time:%ds,%dns\r\n",mnld_test_result_body.location.flags,mnld_test_gnss_open_tm.tv_sec,mnld_test_gnss_open_tm.tv_nsec,fix_tm.tv_sec,fix_tm.tv_nsec);
                mnld_test_ttff = (fix_tm.tv_sec-mnld_test_gnss_open_tm.tv_sec)*1000+((fix_tm.tv_nsec-mnld_test_gnss_open_tm.tv_nsec)/1000000);
                mnld_test_result_body.ttff[CURR] = mnld_test_ttff;
                valid_ttff_cnt++;
                valid_ttff_sum+=mnld_test_result_body.ttff[CURR];

                mnld_test_result_body.ttff[MEAN] = valid_ttff_sum/valid_ttff_cnt;
                //Find the MIN TTFF
                if((mnld_test_result_body.ttff[MIN] == 0 ) || (mnld_test_result_body.ttff[MIN] > mnld_test_result_body.ttff[CURR]))
                {
                    mnld_test_result_body.ttff[MIN] = mnld_test_result_body.ttff[CURR];
                }
                // Find the MAX TTFF
                if(mnld_test_result_body.ttff[MAX] < mnld_test_result_body.ttff[CURR])
                {
                    mnld_test_result_body.ttff[MAX] = mnld_test_result_body.ttff[CURR];
                }
                LOGD("TTFF:%dms",mnld_test_ttff);
            }

        }else{
            mnld_test_result_body.ttff[CURR] = mnld_test_ttff;
        }
    }

    if(mnld_test_result_body.fix_type != 0)
    {
        //GNGGA,hhmmss.mss,...
        if(strncmp(nmea+3,NMEA_GGA,strlen(NMEA_GGA)) == 0)
        {
            strncpy(mnld_test_result_body.utc_time, nmea+7, MNL_UTC_TIME_LEN);
            mnld_test_result_body.utc_time[MNL_UTC_TIME_LEN-1] = '\0';
        }
    }else{
        memset(mnld_test_result_body.utc_time, 0, MNL_UTC_TIME_LEN);
        mnld_test_result_body.utc_time[0] = '-';
    }

    if(strncmp(nmea+3,NMEA_ACC,strlen(NMEA_ACC)) == 0)
    {
        mnld_test_result_body.location.legacyLocation.timestamp = timestamp;
        mnld_test_show_test_result(&mnld_test_result_body);
    }
    
}

void mnld_test_gps_set_capabilities(uint32_t capabilities)
{

    LOGD("gps set capabilities");
}

void mnld_test_gps_acquire_wakelock(void)
{

    LOGD("gps acquire wakelock");
}

void mnld_test_gps_release_wakelock(void)
{

    LOGD("gps release wakelock");
}

void mnld_test_gps_request_utc_time(void)
{

    LOGD("gps request utc time");
}

void mnld_test_set_system_info_cb(const GnssSystemInfo* info)
{
    LOGD("set system info");
}

void mnld_test_gnss_sv_status_cb(GnssSvStatus_ext* sv_info)
{
    LOGD("gnss sv status");
}

pthread_t mnld_test_gps_create_thread(const char* name, void (*start)(void *), void* arg)
{
    pthread_t ntid = 0;
    int ret = 0;

    ret = pthread_create(&ntid, NULL, start, arg);

    if(ret != 0)
    {
        LOGE("thread %s create fail(%s)!", name, strerror(errno));
        ntid = 0;
    }else{
        LOGD("tread %s create success!", name);
    }

    return ntid;
}

void mnld_test_gnss_set_name_cb(const char* name, int length)
{
    LOGD("gnss set name");
}

void mnld_test_gnss_request_location_cb(bool independentFromGnss)
{
    LOGD("gnss request location");
}

GpsCallbacks_ext mnld_test_gps_callbacks = {
    .size = sizeof(GpsCallbacks_ext),
    .location_cb = mnld_test_gps_location_callback,
    .status_cb = mnld_test_gps_status_callback,
    .sv_status_cb = mnld_test_gps_sv_status_callback,
    .nmea_cb = mnld_test_gps_nmea_callback,
    .set_capabilities_cb = mnld_test_gps_set_capabilities,
    .acquire_wakelock_cb = mnld_test_gps_acquire_wakelock,
    .release_wakelock_cb = mnld_test_gps_release_wakelock,
    .create_thread_cb = mnld_test_gps_create_thread,
    .request_utc_time_cb = mnld_test_gps_request_utc_time,
    .set_system_info_cb = mnld_test_set_system_info_cb,
    .gnss_sv_status_cb = mnld_test_gnss_sv_status_cb,
    .set_name_cb = mnld_test_gnss_set_name_cb,
    .request_location_cb = mnld_test_gnss_request_location_cb,
};

GpsCallbacks_ext* mnld_test__get_gps_callbacks(void)
{
    return &mnld_test_gps_callbacks;
}
