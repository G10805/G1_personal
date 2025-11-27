/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#ifndef MODULE_GNSS_HELPER_H
#define MODULE_GNSS_HELPER_H

#include <errno.h>
#include <string.h>

#include "log.h"
#include "gnss_util.h"
#include "adr_data_struct.h"
#include "hal_mnl_interface_common.h"    //from gps/mtk_mnld/mnld_entity/inc

#define NMEA_MAX_SV_INFO 4
#define BIT(n) 1 << n
#define GPS_MAX_RAW 512
#define PROTO_MAX_SIZE 256    //maximum length of each packet
#define GNSS_MAX_SVID 450   //maximum SVID(PRN)

typedef enum {
    FRAME_START = BIT(0),
    FRAME_END = BIT(1),
} FRM_FLAG;

// for different SV parse
typedef enum {
    GPS_SV = 0,
    GLONASS_SV,
    BDS_SV,
    GALILEO_SV,
} SV_TYPE;

struct date_time{
    int date_yyyy;  //date::year since B.C.
    int date_mm;    //date::month - january = 1
    int date_dd;    //date::day - day of month 
    int time_hh;    //time::hour
    int time_mm;    //time::minute
    int time_ss;    //time::second
    int time_ms;    //time::millisecond
};

enum FRAME_TYPE_UPDATE {
    FRAME_TYPE_UPDATE_NO = 0,
    FRAME_TYPE_UPDATE_CHL = BIT(0),
    FRAME_TYPE_UPDATE_GRP = BIT(1),
    FRAME_TYPE_UPDATE_VNED = BIT(2),
    FRAME_TYPE_UPDATE_RMC = BIT(3),
    FRAME_TYPE_UPDATE_GGA = BIT(4),
    FRAME_TYPE_UPDATE_GSA = BIT(5),
    FRAME_TYPE_UPDATE_GSV = BIT(6),
    FRAME_TYPE_UPDATE_EPE = BIT(7),
    FRAME_TYPE_UPDATE_ACCURACY = BIT(8),
    FRAME_TYPE_UPDATE_EP = BIT(9),

    /*if there is no satellite discovered, PMTKCHL will not exist*/
    FRAME_TYPE_UPDATE_ALL = BIT(1)|BIT(2)|BIT(3)|BIT(4)|
                            BIT(5)|BIT(6)|BIT(7)|BIT(8)|BIT(9),//BIT(0)|
};

typedef struct {
    uint32_t sysId;     //0:gps 1:glonass 2:beidou

    int16_t sateId;    //satellite ID minimum 3 char(decimal)
    double psedorange;
    double sync_carr_phase; //raw carrier phase measurement

    double doppler;
    uint32_t cycle_slip;
    uint32_t snr;    //c/n0

    double ecef_x;
    double ecef_y;
    double ecef_z;

    uint32_t channel_hz; //only valid for GLONASS
    //iode for gps/beidou, tb for glonass.gps(00 - 0xff)
    //beidou(00 - 0x1f), no iode = empty field.
    uint32_t iode_tb;

    double iono_corr;    //ionosphere correction value
    uint32_t iode_src;    //0:none, 1:broadcast, 2:sbas

    uint32_t sync_status;    //0:none, 1:bit sync, 2:subframe sync, 3:exact sync

    bool used_in_fix;
} mtk_sv_chl;

typedef struct {
    int32_t num_sv;
    mtk_sv_chl sv_chl[MTK_MNLD_GNSS_MAX_SVS];//from hal_mnl_interface_common.h
} mtk_gnss_chl;

typedef struct gnss_module_data{
    FRM_FLAG frame_flag;    //flag a while data of a frame 
    enum FRAME_TYPE_UPDATE flags;//flag which type of sentence received

    struct date_time dtime;
    struct timespec drvtime;

    struct {
        double utc_time;
        double lat;
        double lng;
        double alt;
        uint32_t fix_state;
        uint32_t nsf;            //number of satellites used for fix
        double hdop;
        uint64_t diff_time;
        uint32_t diff_stationId;
    } nmeagga;

    struct {
        double utc_data_time;    //yyyymmddhhmmss 
        int utc_time_msec; //ms
        uint32_t fix_mode;    //A:positioning V:navigation
        double lat;
        double lng;
        double speed;        //velocity knots
        double bearing;        //direction
        uint32_t magn_bias_deg;
        uint32_t magn_bias_deg_dir;
    } nmearmc;

    struct {
        uint32_t fix_pattern;    //M:manual A:auto
        int fix_mode;
        NmeaCash gps_sv_used_in_fix[GNSS_MAX_SVID];
        uint32_t sate_num;    //current satellite message
        double pdop;
        double hdop;
        double vdop;
    } nmeagsa;

    struct {
        int sv_count;
        gnss_sv_info  sv_status;
    } nmeagsv;

    struct {
        double epe_2d;
        double epe_vert;
    } nmeaepe;

    struct {
        double accuracy;
    } nmeaaccuracy;

    struct {
        mtk_gnss_chl gnss_chl;
    } pmtkchl;

    struct {
        uint32_t clocktime;    //local receiver time tick(ms)
        double   timeweeks;    //gps tow(s)
        uint32_t weeknum;   //week number
        uint32_t clk_status;    //0:none, 1:rtc, 2:synced to gps, 3:from gps fix
        uint32_t utc_offset;    //the different between gps and utc(sec)
        int32_t clk_bias;
        int32_t clk_offsetA;    //clock offset between gps and glonass
        int32_t clk_offsetB;    //clock offset between gps and beidou
    } pmtkgrp;

    struct {
        uint32_t clocktime;    //local receiver time tick(ms)
        double velocity_n;    //north velocity(m/s)
        double velocity_e;    //east velocity(m/s)
        double velocity_d;    //down velocity(m/s) (origin data is velocity (up)
        double hor_speed;    //horizontal ground speed(m/s)
        double speed;    //object speed, include horizontal and vertical speed(m/s)
        double vn_acc;
        double ve_acc;
        double vd_acc;
        double heading_acc;
    } pmtkvned;

    struct {
        double epx;                     ///< Longitude error estimate in meters, 95% confidence
        double epy;                     ///< Latitude error estimate in meters, 95% confidence
        double epv;                     ///< Estimated vertical error in meters, 95% confidence
        double climb;                   ///< Climb (positive) or sink (negative) rate, meters per second
        double epd;                     ///< Direction error estimate in degrees, 95% confidence
        double eps;                     ///< Speed error estimate in meters/sec, 95% confidence
        double epc;                     ///< Climb/sink error estimate in meters/sec, 95% confidence
    } pmtk840;

    struct {
        double baro;        //%0.2f/OMRON raw data
        double baroT;       //%0.3f/OMRON raw data
        double odo;         //%0.2f/OBD2
    } sensor;

    struct {                //cache matched items pmtkchl & nmeagsv
        adr_nsv_struct satedatas[GPS_MAX_RAW];
        int32_t nsv;
        gnss_nmea_data nmea;
        char *pmtk_cmd_ack;
    } sv_out;

    int     pos;
    int     overflow;
    /* GpsCallbacks callbacks;*/
    char    in[PROTO_MAX_SIZE + 1];
} gnss_module_data;

extern int proto_tokenizer_init(ProtoTokenizer*  t, const char*  p, const char*  end);
static Token proto_tokenizer_get(ProtoTokenizer* t, int  index);
void nmea_reader_init(gnss_module_data* const r);
void gnss_parser_postfunc(gnss_module_data* r);
void gnss_parser_prefunc(gnss_module_data* p,
                              const char *buffer, uint32_t length);
struct timespec *get_available_timespec();
int get_gnss_average_time_diff(struct timespec *result);
int get_gnss_std_deviation_time_diff(struct timespec *avg_time_diff, struct timespec *result);
int get_diff_uploadtime_avg_time(struct timespec *avg_time_diff, struct timespec *result);
void gnss_notify_time_msg(struct timespec *time);
int send_buf_to_libmnl(int *sockfd, char *buf, int len);
int gnss_command_socket_open(int port);

#endif //MODULE_GNSS_HELPER_H
