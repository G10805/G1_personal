/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
***************************************************************/
#ifndef MPE_MAIN_C
#define MPE_MAIN_C

#ifdef __cplusplus
  extern "C" {
#endif

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <math.h>
#include <inttypes.h>

#include "mpe_common.h"
#include "mpe_DR.h"
#include "mpe_sensor.h"
#include "mpe.h"
#include "data_coder.h"
#include "mtk_lbs_utility.h"
#include "mnld.h"
#include "mtk_auto_log.h"

#if ANDROID_MNLD_PROP_SUPPORT
#include <cutils/properties.h>
#endif
#if defined(__ANDROID_OS__)
#include <private/android_filesystem_config.h>
#endif

#if defined(__ANDROID_OS__)
#define MPE_DEFAULT_CONFIG_FILE "/system/vendor/etc/adr/mpe.conf"
#elif defined(__LINUX_OS__)
#define MPE_DEFAULT_CONFIG_FILE "/etc/adr/mpe.conf"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MPE_MA"
#endif

/*****************************************************************************
 * GLobal Variable
 *****************************************************************************/
static const char pMPEVersion[] = {MPE_VER_INFO,0x00};
unsigned char isUninit_SE = FALSE;
static MNL2MPE_AIDING_DATA mnl_latest_in;
static MNL2MPE_AIDING_DATA mnl_glo_in;
static MPE2MNL_AIDING_DATA mnl_glo_out;
static UINT32 gMPEConfFlag = 0;

//Mtklogger function
char mpe_debuglog_file_name[MPE_LOG_NAME_MAX_LEN];

//check for baro validity
//extern unsigned char isfirst_baro;
unsigned char isfirst_baro = 0;

// se log
extern FILE *data_file_local;

float gyr_temperature = 0.0f;
float gyr_scp_calib_done = 0.0f;

unsigned char gMpeThreadExist = 0;

UINT32 mpe_sys_get_mpe_conf_flag(void) {
    return gMPEConfFlag;
}

void mpe_sys_read_mpe_conf_flag(void) {
    const char mpe_prop_path[] = MPE_DEFAULT_CONFIG_FILE;
    char propbuf[512];
    UINT32 flag = 0;
    FILE *fp = fopen(mpe_prop_path, "rb");
    LOGD("%s\n", pMPEVersion);
    if(!fp) {
        LOGD("mpe config flag - can't open");
        gMPEConfFlag = 0;
        return;
    }
    while(fgets(propbuf, sizeof(propbuf), fp)) {
        if(strstr(propbuf, "mpe_enable=1")) {
            flag |= MPE_CONF_MPE_ENABLE;
        } else if(strstr(propbuf, "print_rawdata=1")) {
            flag |= MPE_CONF_PRT_RAWDATA;
        } else if(strstr(propbuf, "auto_calib=1")) {
            flag |= MPE_CONF_AUTO_CALIB;
        } else if(strstr(propbuf, "indoor_enable=1")) {
            flag |= MPE_CONF_INDOOR_ENABLE;
        }
    }
    LOGD("mpe config flag %u", flag);
    fclose(fp);
    gMPEConfFlag = flag;
    return;
}

INT32 mpe_sys_gps_to_sys_time( UINT32 gps_sec ) {
    return (INT32) ( gps_sec + DIFF_GPS_C_TIME );// difference between GPS and
}

void mpe_sys_get_time_stamp(double* timetag, UINT32 leap_sec) {
    struct tm tm_pt;
    struct timeval tv;
    //get second and usec
    gettimeofday(&tv, NULL);
    //convert to local time
    localtime_r(&tv.tv_sec, &tm_pt);
    (*timetag) = mktime(&tm_pt);
    (*timetag)= (*timetag) + leap_sec +((float)tv.tv_usec)/1000000;
}

void mnl2mpe_hdlr_init(void) {
    memset(&mnl_latest_in, 0 , sizeof(MNL2MPE_AIDING_DATA));
    memset(&mnl_glo_in, 0 , sizeof(MNL2MPE_AIDING_DATA));
    memset(&mnl_glo_out, 0 , sizeof(MPE2MNL_AIDING_DATA));
}

int adr2mnl_send_nmea_data(const char * nmea_buffer, const UINT32 length) {
    LOGD("adr2mnl_send_nmea_data");
    char buff[MNL_MPE_NMEA_MAX_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_ADR_TYPE_ADR2MNL_NMEA_DATA);
    put_int(buff, &offset, length);
    put_binary(buff, &offset, nmea_buffer, length);

    return safe_sendto(MNLD_ADR2MNL_SOCKET, buff, MNL_MPE_NMEA_MAX_SIZE);
}

int mnl2mpe_hdlr(int fd) {
    char mnl2mpe_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int mnl2mpe_offset = 0;
    int read_len;
    int log_rec = 0, rec_loc =0;
    UINT32 type, length;

    read_len = safe_recvfrom(fd, mnl2mpe_buff, sizeof(mnl2mpe_buff));
    if (read_len <= 0) {
        LOGE("safe_recvfrom() failed read_len=%d", read_len);
        return -1;
    }

    type = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
    length = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
    LOGD("mpe recv, type = %u, len = %u", type, length);

    switch (type) {
        case CMD_SEND_FROM_MNLD: {
            log_rec = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
            rec_loc = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
            get_binary(mnl2mpe_buff, &mnl2mpe_offset, mpe_debuglog_file_name, sizeof(mnl2mpe_buff), sizeof(mpe_debuglog_file_name));
            LOGD("log_rec =%d, rec_loc=%d, mpelog_path:%s", log_rec, rec_loc,mpe_debuglog_file_name );
            mpe_log_mtklogger_check((INT16)log_rec, mpe_debuglog_file_name, (INT8)rec_loc);
            break;
        }
        case MNL_ADR_TYPE_MNL2ADR_NMEA_DATA: {
            char nmea[MNL_MPE_NMEA_MAX_SIZE] = {0};
            get_binary(mnl2mpe_buff, &mnl2mpe_offset, nmea, sizeof(mnl2mpe_buff), sizeof(nmea));
            LOGD("rec nmea:%s", nmea);
            if(length <= MNL_MPE_NMEA_MAX_SIZE) {
                adr2mnl_send_nmea_data(nmea, length);
            }
            break;
        }
        default: {
            LOGD("MPE_DBG: invalid msg type = %d", type);
        }
    }
    return 0;
}

int mpe_kernel_inject(IMU* data, UINT16 len, MPE2MNL_AIDING_DATA *mnl_out) {
    int i=0;
    UNUSED(mnl_out);

    if(data == NULL) {
        LOGD("allocate sensor cb error\n");
        return MTK_GPS_ERROR;
    }

    for(i = 0; i < len; i++ ) {
        if(data + i != NULL) {
            if (mpe_sys_get_mpe_conf_flag() & MPE_CONF_PRT_RAWDATA) {
                LOGD("[%d] MPErpy %"PRId64" %f %f %f %f %f %f %f %f %f %f %lf %lf %lf %lf %lf %lf %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %u %u %f",
                    i, (data+i)->input_time_gyro,
                    (data+i)->acceleration[0], (data+i)->acceleration[1], (data+i)->acceleration[2],
                    (data+i)->angularVelocity[0], (data+i)->angularVelocity[1], (data+i)->angularVelocity[2],
                    (data+i)->magnetic[0], (data+i)->magnetic[1], (data+i)->magnetic[2],
                    (data+i)->pressure,
                    mnl_glo_in.latitude[0], mnl_glo_in.longitude[0], mnl_glo_in.altitude[0],
                    mnl_glo_in.latitude[1], mnl_glo_in.longitude[1], mnl_glo_in.altitude[1],
                    mnl_glo_in.LS_velocity[0], mnl_glo_in.LS_velocity[1], mnl_glo_in.LS_velocity[2],
                    mnl_glo_in.KF_velocity[0], mnl_glo_in.KF_velocity[1], mnl_glo_in.KF_velocity[2],
                    mnl_glo_in.LS_velocitySigma[0], mnl_glo_in.LS_velocitySigma[1], mnl_glo_in.LS_velocitySigma[2],
                    mnl_glo_in.KF_velocitySigma[0], mnl_glo_in.KF_velocitySigma[1], mnl_glo_in.KF_velocitySigma[2],
                    mnl_glo_in.HACC, mnl_glo_in.VACC, mnl_glo_in.HDOP,
                    mnl_glo_in.confidenceIndex[0], mnl_glo_in.confidenceIndex[1], mnl_glo_in.confidenceIndex[2],
                    mnl_glo_in.gps_sec, mnl_glo_in.leap_sec, (data+i)->thermometer);
            }

            /*if (mpe_get_dr_entry(data+i, &mnl_glo_in, mnl_out)) {
                if (mpe_sys_get_mpe_conf_flag() & MPE_CONF_PRT_RAWDATA) {
                    LOGD("MPE_DBG: MPE mpe_update_posture return false");
                }
            }*/
        } else {
            LOGD("accept null data\n");
        }
    }
    return MTK_GPS_SUCCESS;
}

void mpe_set_mnl_out_flag(MPE2MNL_AIDING_DATA *mnl_out) {
    //if (mpe_sensor_get_listen_mode() == MPE_START_MODE) {
    if (0) {
        mnl_out->valid_flag[MTK_MPE_SYS_FLAG] = 1;
    } else {
        mnl_out->valid_flag[MTK_MPE_SYS_FLAG] = 0;
    }
    mnl_out->valid_flag[MTK_MPE_RAW_FLAG] = 1;
    if(isfirst_baro == 1) {
        mnl_out->barometerHeight= -16000;
    }
    if (!(mpe_sys_get_mpe_conf_flag() & MPE_CONF_INDOOR_ENABLE) || gyr_scp_calib_done == 0) {
        mnl_out->HACC= 20000;
    }
}

/*void mpe_sensor_stop_notify( void ) {
    char mpe2mpe_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(mpe2mpe_buff, &offset, CMD_STOP_MPE_REQ);
    put_int(mpe2mpe_buff, &offset, sizeof(INT32));
    put_int(mpe2mpe_buff, &offset, 2);
    safe_sendto(MNLD_MPE_SOCKET, mpe2mpe_buff, MNL_MPE_MAX_BUFF_SIZE);
    isUninit_SE = TRUE;
    LOGD("send uninit request from listener \n");
}*/

void mpe_run_algo( void ) {
#if 0
    int data_ret = 0;
    IMU SE_data[MAX_NUM_SAMPLES];
    UINT16 data_cnt = 0;
    MPE2MNL_AIDING_DATA mnl_out;

    memset(SE_data, 0 ,MAX_NUM_SAMPLES*sizeof(IMU));
    memset(&mnl_out, 0, sizeof(MPE2MNL_AIDING_DATA));

    //data_cnt = mpe_sensor_acquire_Data(SE_data);
    if(data_cnt) {
        data_ret = mpe_kernel_inject(SE_data, data_cnt, &mnl_out);
        mpe_set_mnl_out_flag(&mnl_out);
        memcpy(&mnl_glo_out, &mnl_out, sizeof(MPE2MNL_AIDING_DATA));
    }
#endif
    memcpy(&mnl_glo_in, &mnl_latest_in, sizeof(MNL2MPE_AIDING_DATA));
}

#ifdef __cplusplus
  extern "C" }
#endif

#endif //#ifndef MPE_FUSION_MAIN_C
