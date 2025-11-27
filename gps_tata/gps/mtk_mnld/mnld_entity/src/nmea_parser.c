// SPDX-License-Identifier: MediaTekProprietary
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define _NMEA_PARSER_C_
/*******************************************************************************
* Dependency
*******************************************************************************/
#include <errno.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "mnld.h"
#include "mtk_gps.h"
#include "mnl2hal_interface.h"
#include "nmea_parser.h"
#include "mtk_lbs_utility.h"
#include "mnl_at_interface.h"
#include "mtk_auto_log.h"
#ifdef CONFIG_GPS_MT3303
#include "mt3333_controller.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>



extern int g_agc_level;

#define CARRIER_FREQ_GPS_L1    (1575.42*1000000)
#define CARRIER_FREQ_GPS_L5    (1176.45*1000000)
#define CARRIER_FREQ_GLO_L1    (1602*1000000)
#define CARRIER_FREQ_GAL_E1    (1575.42*1000000)
#define CARRIER_FREQ_GAL_E5A   (1176.45*1000000)
#define CARRIER_FREQ_BD_B1     (1561.098*1000000)
#define CARRIER_FREQ_BD_B2     (1207.14*1000000)
#define CARRIER_FREQ_BD_B3     (1268.52*1000000)

#define NEMA_DEBUG 0

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "nmea_parser"
#endif

/*******************************************************************************
* structure & enumeration
*******************************************************************************/
static int gps_nmea_end_tag = 0;

static NmeaCash nmea_cash[450];

extern int prn[32];
extern int snr[32];
extern int MNL_AT_TEST_FLAG;
extern int MNL_AT_SIGNAL_MODE;

#ifdef CONFIG_GPS_MT3303
extern MNL_CONFIG_T mnl_config;
extern int is_ygps_delete_data ;
extern unsigned int assist_data_bit_map;
static int has_accuracy = 0;
#endif

/******************************************************************************
 * Functions
******************************************************************************/
static void nmea_reader_update_utc_diff(NmeaReader* const r) {
    time_t         now = time(NULL);
    struct tm      tm_local;
    struct tm      tm_utc;
    unsigned long  time_local, time_utc;

    gmtime_r(&now, &tm_utc);
    localtime_r(&now, &tm_local);
    tm_local.tm_isdst = -1;
    tm_utc.tm_isdst = -1;
    time_local = mktime(&tm_local);
    time_utc = mktime(&tm_utc);

    r->utc_diff = time_utc - time_local;
}

void nmea_reader_init(NmeaReader* const r) {
    memset( r, 0, sizeof(*r) );

    r->pos      = 0;
    r->overflow = 0;
    r->utc_year = -1;
    r->utc_mon  = -1;
    r->utc_day  = -1;
    r->utc_diff = 0;
    r->sv_count = 0;
    r->fix_mode = 0;    /*no fix*/
    r->cb_status_changed = 0;
    memset((void*)&r->sv_status, 0x00, sizeof(r->sv_status));
    memset((void*)r->in, 0x00, sizeof(r->in));

    nmea_reader_update_utc_diff(r);
}

static int nmea_tokenizer_init(NmeaTokenizer*  t, const char*  p, const char*  end) {
    int    count = 0;

    // the initial '$' is optional
    if (p < end && p[0] == '$')
        p += 1;

    // remove trailing newline
    if (end > p && end[-1] == '\n') {
        end -= 1;
        if (end > p && end[-1] == '\r')
            end -= 1;
    }

    // get rid of checksum at the end of the sentecne
    if (end >= p+3 && end[-3] == '*') {
        end -= 3;
    }

    while (p < end) {
        const char*  q = p;

        q = memchr(p, ',', end-p);
        if (q == NULL)
            q = end;

        if (q >= p) {
            if (count < MAX_NMEA_TOKENS) {
                t->tokens[count].p   = p;
                t->tokens[count].end = q;
                count += 1;
            }
        }
        if (q < end)
            q += 1;

        p = q;
    }

    t->count = count;
    return count;
}

static Token nmea_tokenizer_get(NmeaTokenizer* t, int  index) {
    Token  tok;
    static const char*  dummy = "";

    if (index < 0 || index >= t->count) {
        tok.p = tok.end = dummy;
    } else
        tok = t->tokens[index];

    return tok;
}

static double str2float(const char*    p, const char*  end) {
    int   len    = end - p;
    char  temp[16];

    if (len >= (int)sizeof(temp))
        return 0.;

    memcpy(temp, p, len);
    temp[len] = 0;
    return strtod( temp, NULL );
}

static int nmea_reader_update_altitude(NmeaReader* const r,
                   Token altitude,
                   Token units) {
    Token   tok = altitude;
    UNUSED(units);
    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= MTK_GPS_LOCATION_HAS_ALT;
    r->fix.alt = str2float(tok.p, tok.end);
    return 0;
}

static int nmea_reader_update_time(NmeaReader* const r, Token  tok) {
    int        hour, minute;
    double     seconds;
    struct tm  tm;
    time_t     fix_time;

    if (tok.p + 6 > tok.end)
        return -1;

    memset((void*)&tm, 0x00, sizeof(tm));
    if (r->utc_year < 0) {
        // no date yet, get current one
        time_t  now = time(NULL);
        gmtime_r(&now, &tm);
        r->utc_year = tm.tm_year + 1900;
        r->utc_mon  = tm.tm_mon + 1;
        r->utc_day  = tm.tm_mday;
    }

    hour    = str2int(tok.p,   tok.p+2);
    minute  = str2int(tok.p+2, tok.p+4);
    seconds = str2float(tok.p+4, tok.end);

    tm.tm_hour = hour;
    tm.tm_min  = minute;
    tm.tm_sec  = (int) seconds;
    tm.tm_year = r->utc_year - 1900;
    tm.tm_mon  = r->utc_mon - 1;
    tm.tm_mday = r->utc_day;
    tm.tm_isdst = -1;

    if (mktime(&tm) == (time_t)-1)
        LOGE("mktime error: %d %s\n", errno, strerror(errno));

    // Add by ZQH to recalculate the utc_diff when the time zone is reset
    nmea_reader_update_utc_diff(r);
    fix_time = mktime(&tm) - r->utc_diff;
    r->fix.timestamp = (long long)fix_time * 1000;
    #if NEMA_DEBUG
    LOGD("add fix_time: %" PRId64, r->fix.timestamp);
    #endif
    return 0;
}

static int nmea_reader_update_date(NmeaReader* const r, Token  date, Token  time) {
    Token  tok = date;
    int    day, mon, year;

    if (tok.p + 6 != tok.end) {
        LOGE("date not properly formatted: '%.*s'", (int)(tok.end-tok.p), tok.p);
        return -1;
    }
    day  = str2int(tok.p, tok.p+2);
    mon  = str2int(tok.p+2, tok.p+4);
    year = str2int(tok.p+4, tok.p+6) + 2000;

    if ((day|mon|year) < 0) {
        LOGE("date not properly formatted: '%.*s'", (int)(tok.end-tok.p), tok.p);
        return -1;
    }

    r->utc_year  = year;
    r->utc_mon   = mon;
    r->utc_day   = day;

    return nmea_reader_update_time( r, time );
}

static double convert_from_hhmm(Token  tok) {
    double  val     = str2float(tok.p, tok.end);
    int     degrees = (int)(floor(val) / 100);
    double  minutes = val - degrees*100.;
    double  dcoord  = degrees + minutes / 60.0;
    return dcoord;
}

static int nmea_reader_update_latlong(NmeaReader* const r,
                            Token        latitude,
                            char         latitudeHemi,
                            Token        longitude,
                            char         longitudeHemi) {
    double   lat, lon;
    Token    tok;

    tok = latitude;
    if (tok.p + 6 > tok.end) {
        LOGE("latitude is too short: '%.*s'", (int)(tok.end-tok.p), tok.p);
        return -1;
    }
    lat = convert_from_hhmm(tok);
    if (latitudeHemi == 'S')
        lat = -lat;

    tok = longitude;
    if (tok.p + 6 > tok.end) {
        LOGE("longitude is too short: '%.*s'", (int)(tok.end-tok.p), tok.p);
        return -1;
    }
    lon = convert_from_hhmm(tok);
    if (longitudeHemi == 'W')
        lon = -lon;

    r->fix.flags    |= MTK_GPS_LOCATION_HAS_LAT_LONG;
    r->fix.lat  = lat;
    r->fix.lng = lon;
    return 0;
}

static int nmea_reader_update_bearing(NmeaReader* const r,
                 Token bearing) {
    Token    tok = bearing;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= MTK_GPS_LOCATION_HAS_BEARING;
    r->fix.bearing = str2float(tok.p, tok.end);
    return 0;
}

static int nmea_reader_update_speed(NmeaReader* const r,
                   Token speed) {
    Token tok = speed;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= MTK_GPS_LOCATION_HAS_SPEED;

    // Modify by ZQH to convert the speed unit from knot to m/s
    // r->fix.speed   = str2float(tok.p, tok.end);
    r->fix.speed = str2float(tok.p, tok.end) / 1.942795467;
    return 0;
}

// Add by LCH for accuracy
static int nmea_reader_update_accuracy(NmeaReader* const r,
                            Token haccuracy, Token baccuracy, Token saccuracy, Token vaccuracy) {
    Token   htok = haccuracy;
    Token   btok = baccuracy;
    Token   stok = saccuracy;
    Token   vtok = vaccuracy;
#ifdef CONFIG_GPS_MT3303
    if (htok.p >= htok.end)
#else
    if (htok.p >= htok.end || btok.p >= btok.end || stok.p >= stok.end || vtok.p >= vtok.end)
#endif
        return -1;

    r->fix.flags   |= MTK_GPS_LOCATION_HAS_HORIZONTAL_ACCURACY;
    r->fix.flags   |= MTK_GPS_LOCATION_HAS_VERTICAL_ACCURACY;
    r->fix.flags   |= MTK_GPS_LOCATION_HAS_SPEED_ACCURACY;
    r->fix.flags   |= MTK_GPS_LOCATION_HAS_BEARING_ACCURACY;

    r->fix.h_accuracy = str2float(htok.p, htok.end);
    r->fix.b_accuracy = str2float(btok.p, btok.end);
    r->fix.s_accuracy = str2float(stok.p, stok.end);
    r->fix.v_accuracy = str2float(vtok.p, vtok.end);

    #if NEMA_DEBUG
    LOGD("update accuracy: h=%f, v=%f, s=%f, b= %f", r->fix.h_accuracy, r->fix.v_accuracy,
        r->fix.s_accuracy, r->fix.b_accuracy);
    #endif
    return 0;
}

static int nmea_reader_update_sv_status(NmeaReader* r, int sv_index,
                                  int id, Token elevation,
                                  Token azimuth, Token snr) {
    // int prn = str2int(id.p, id.end);
    int prn = id;
    sv_index = r->sv_count+r->sv_status.num_svs;
    if (MTK_MNLD_GNSS_MAX_SVS <= sv_index) {
        LOGE("ERR: sv_index=[%d] is larger than MTK_MNLD_GNSS_MAX_SVS.\n", sv_index);
        return 0;
    }

    if ((prn > 0) && (prn <= 32)) {
        r->sv_status.sv_list[sv_index].svid = prn;
        r->sv_status.sv_list[sv_index].constellation = GPS_SV;
        r->sv_status.sv_list[sv_index].carrier_frequency = (float)CARRIER_FREQ_GPS_L1;
        r->sv_status.sv_list[sv_index].flags |= 0x08;
    } else if ((prn >= 65) && (prn <= 96)) {
        r->sv_status.sv_list[sv_index].svid = prn-64;
        r->sv_status.sv_list[sv_index].constellation = GLONASS_SV;
        r->sv_status.sv_list[sv_index].carrier_frequency = (float)CARRIER_FREQ_GLO_L1;
        r->sv_status.sv_list[sv_index].flags |= 0x08;
    } else if ((prn >= 201) && (prn <= 237)) {
        r->sv_status.sv_list[sv_index].svid = prn-200;
        r->sv_status.sv_list[sv_index].constellation = BDS_SV;
        r->sv_status.sv_list[sv_index].carrier_frequency = (float)CARRIER_FREQ_BD_B1;
        r->sv_status.sv_list[sv_index].flags |= 0x08;
    } else if ((prn >= 401) && (prn <= 436)) {
        r->sv_status.sv_list[sv_index].svid = prn-400;
        r->sv_status.sv_list[sv_index].constellation = GALILEO_SV;
        r->sv_status.sv_list[sv_index].carrier_frequency = (float)CARRIER_FREQ_GAL_E1;
        r->sv_status.sv_list[sv_index].flags |= 0x08;
    } else if ((prn >= 193) && (prn <= 197)) {
        r->sv_status.sv_list[sv_index].svid = prn;
        r->sv_status.sv_list[sv_index].constellation = QZSS_SV;
        r->sv_status.sv_list[sv_index].carrier_frequency = (float)CARRIER_FREQ_GPS_L1;
        r->sv_status.sv_list[sv_index].flags |= 0x08;
    } else if ((prn >= 33) && (prn <= 64)) {
        r->sv_status.sv_list[sv_index].svid = prn+87;
        r->sv_status.sv_list[sv_index].constellation = SBAS_SV;
        r->sv_status.sv_list[sv_index].carrier_frequency = (float)CARRIER_FREQ_GPS_L1;
        r->sv_status.sv_list[sv_index].flags |= 0x08;
    } else {
        LOGW("sv_status: ignore (%d)", prn);
        return 0;
    }

    r->sv_status.sv_list[sv_index].c_n0_dbhz = str2float(snr.p, snr.end);
    r->sv_status.sv_list[sv_index].elevation = str2int(elevation.p, elevation.end);
    r->sv_status.sv_list[sv_index].azimuth = str2int(azimuth.p, azimuth.end);
    if (1 == nmea_cash[prn].used_in_fix) {
        r->sv_status.sv_list[sv_index].flags |= 0x04;
    } else {
        r->sv_status.sv_list[sv_index].flags &= ~(0x04);
    }
    if (1 == nmea_cash[prn].has_almanac_data) {
        r->sv_status.sv_list[sv_index].flags |= 0x02;
    } else {
        r->sv_status.sv_list[sv_index].flags &= ~(0x02);
    }
    if (1 == nmea_cash[prn].has_ephemeris_data) {
        r->sv_status.sv_list[sv_index].flags |= 0x01;
    } else {
        r->sv_status.sv_list[sv_index].flags &= ~(0x01);
    }

    r->sv_count++;
    /*
    #ifdef CONFIG_GPS_ENG_LOAD
    LOGD("sv_status(%2d): %2d, %d, %2f, %3f, %2f, %d",
        sv_index, r->sv_status.sv_list[sv_index].svid, r->sv_status.sv_list[sv_index].constellation,
        r->sv_status.sv_list[sv_index].elevation, r->sv_status.sv_list[sv_index].azimuth,
        r->sv_status.sv_list[sv_index].c_n0_dbhz, r->sv_status.sv_list[sv_index].flags);
    #endif
    */
    return 0;
}

static void nmea_reader_parse(NmeaReader* const r) {
/* we received a complete sentence, now parse it to generate
* a new GPS fix...
*/
    nmea_parser_at_cmd_pre();  // for AT cmd test  check

    NmeaTokenizer  tzer[1];
    Token          tok;
    Token          mtok;
    SV_TYPE sv_type = 1;

    #if NEMA_DEBUG
    LOGD("Received: '%.*s'", r->pos, r->in);
    #endif
    if (r->pos < 9) {
        LOGE("Too short. discarded. '%.*s'", r->pos, r->in);
        return;
    }

    nmea_tokenizer_init(tzer, r->in, r->in + r->pos);
    #if NEMA_DEBUG
    {
        int  n;
        LOGD("Found %d tokens", tzer->count);
        for (n = 0; n < tzer->count; n++) {
            Token  tok = nmea_tokenizer_get(tzer, n);
            LOGD("%2d: '%.*s'", n, (int)(tok.end-tok.p), tok.p);
        }
    }
    #endif

    tok = nmea_tokenizer_get(tzer, 0);
    if (tok.p + 5 > tok.end) {
        LOGE("sentence id '%.*s' too short, ignored.", (int)(tok.end-tok.p), tok.p);
        return;
    }

    // ignore first two characters.
    mtok.p = tok.p;  // Mark the first two char for GPS,GLONASS,BDS , GALILEO SV parse.
    if (!memcmp(mtok.p, "BD", 2)) {
        sv_type = BDS_SV;
    } else if (!memcmp(mtok.p, "GP", 2)) {
        sv_type = GPS_SV;
    } else if (!memcmp(mtok.p, "GL", 2)) {
        sv_type = GLONASS_SV;
    } else if (!memcmp(mtok.p, "GA", 2)) {
        sv_type = GALILEO_SV;
    }
    #ifdef CONFIG_GPS_MT3303
    else if (!memcmp(mtok.p, "PMTK010", 7)) {
        Token  result          = nmea_tokenizer_get(tzer, 1);
        int pmtk010_result = str2int(result.p, result.end);
        
        LOGD("[GNSS_Performance]:pmtk010_result: %d, delete state=%d", pmtk010_result,is_ygps_delete_data);

        mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_GNSS_SYSTEM);
        mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_GNSS_FIXINTERVAL);

        if(pmtk010_result == 1){
            if(is_ygps_delete_data == 1){
                is_ygps_delete_data = 2;
                mt3333_controller_delete_aiding_data(assist_data_bit_map);
                return;
            }else if(is_ygps_delete_data == 2){
                is_ygps_delete_data = 0;
            }else if(is_ygps_delete_data == 10){
                is_ygps_delete_data = 0;
            }

            mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_REQUES1STNMEA);

            //mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_GNSS_SYSTEM);
            
        }else if(pmtk010_result == 2){  
            if(is_ygps_delete_data != 0){
                return;
            }   
            mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_NMEA_ONOFF);
            if((mnl_config.EPO_enabled == 1)){
                
                mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_REQUESTEPO); 
            }
        }
        return;
    }
    else if (!memcmp(mtok.p, "PMTK001", 7)) {
        gps_nmea_end_tag = 0;
        Token  cmd          = nmea_tokenizer_get(tzer, 1);
        Token  cmd_result          = nmea_tokenizer_get(tzer, 2);
        int pmtk001_cmd = str2int(cmd.p, cmd.end);
        int pmtk001_cmd_result = str2int(cmd_result.p, cmd_result.end);
        
        LOGD("[GNSS_Performance]:pmtk001_cmd: PMTK%03d, cmd result=%d", pmtk001_cmd ,pmtk001_cmd_result);

        
        if(pmtk001_cmd == 314){
            mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_REQUESTNTP);
            mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_REQUESTNLP);
            
            if(mnl_config.BEE_enabled == 1){
                mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_ENABLEDEASYMODE);
            }else{
                mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_DISABLEDEASYMODE);
            }
            
            if(mnl_config.dbg2file == 1){
                mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_ENABLEDEBUGLOG);
            }else{
                mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_DISABLEDEBUGLOG);
            }
        }
    }
    if(is_ygps_delete_data != 0){
        return;
    }
#endif
    #if NEMA_DEBUG
    LOGD("SV type: %d", sv_type);
    #endif
    tok.p += 2;
    if ( !memcmp(tok.p, "GGA", 3) ) {
#ifdef CONFIG_GPS_MT3303
        gps_nmea_end_tag = 0;
        r->overflow = 0;
        r->utc_year = -1;
        r->utc_mon  = -1;
        r->utc_day  = -1;
        r->utc_diff = 0;
        r->sv_count = 0;
        r->fix_mode = 0;    /*no fix*/
        r->cb_status_changed = 0;
        memset((void*)&r->sv_status, 0x00, sizeof(r->sv_status));
        memset((void*)&r->fix, 0x00, sizeof(r->fix));
        nmea_reader_update_utc_diff(r);
#endif

        // GPS fix
        Token  tok_time          = nmea_tokenizer_get(tzer, 1);
        Token  tok_latitude      = nmea_tokenizer_get(tzer, 2);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer, 3);
        Token  tok_longitude     = nmea_tokenizer_get(tzer, 4);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer, 5);
        Token  tok_altitude      = nmea_tokenizer_get(tzer, 9);
        Token  tok_altitudeUnits = nmea_tokenizer_get(tzer, 10);

        nmea_reader_update_time(r, tok_time);
        nmea_reader_update_latlong(r, tok_latitude,
            tok_latitudeHemi.p[0],
            tok_longitude,
            tok_longitudeHemi.p[0]);
        nmea_reader_update_altitude(r, tok_altitude, tok_altitudeUnits);

    } else if ( !memcmp(tok.p, "GSA", 3) ) {
        Token tok_fix = nmea_tokenizer_get(tzer, 2);
        int idx, max = 12;  /*the number of satellites in GPGSA*/
        r->fix_mode = str2int(tok_fix.p, tok_fix.end);
        #if NEMA_DEBUG
        LOGW("[ZY] fix_mode=%d",r->fix_mode);
        #endif

        if (LOC_FIXED(r)) {  /* 1: No fix; 2: 2D; 3: 3D*/
            for (idx = 0; idx < max; idx++) {
                Token tok_satellite = nmea_tokenizer_get(tzer, idx+3);
                if (tok_satellite.p == tok_satellite.end) {
                    #if NEMA_DEBUG
                    LOGD("GSA: found %d active satellites\n", idx);
                    #endif
                    break;
                }
                int sate_id = str2int(tok_satellite.p, tok_satellite.end);
                if (sv_type == BDS_SV) {
                    sate_id += 200;
                    #if NEMA_DEBUG
                    LOGD("It is BDS SV: %d", sate_id);
                    #endif
                } else if (sv_type == GALILEO_SV) {
                    sate_id += 400;
                    #if NEMA_DEBUG
                    LOGD("It is GALILEO SV: %d", sate_id);
                    #endif
                }
                if (sate_id >= 0 && sate_id < 450) {
                    nmea_cash[sate_id].prn = sate_id;
                    nmea_cash[sate_id].used_in_fix = 1;
                }
            }
        }
    }
    // VER("GPGSA: mask 0x%x", r->sv_status.used_in_fix_mask);
    else if (!memcmp(tok.p, "RMC", 3)) {
        Token  tok_time          = nmea_tokenizer_get(tzer, 1);
        Token  tok_fixStatus     = nmea_tokenizer_get(tzer, 2);
        Token  tok_latitude      = nmea_tokenizer_get(tzer, 3);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer, 4);
        Token  tok_longitude     = nmea_tokenizer_get(tzer, 5);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer, 6);
        Token  tok_speed         = nmea_tokenizer_get(tzer, 7);
        Token  tok_bearing       = nmea_tokenizer_get(tzer, 8);
        Token  tok_date          = nmea_tokenizer_get(tzer, 9);

        #if NEMA_DEBUG
        LOGW("in RMC, fixStatus=%c", tok_fixStatus.p[0]);
        #endif
        if (tok_fixStatus.p[0] == 'A') {
            nmea_reader_update_date(r, tok_date, tok_time);
            nmea_reader_update_latlong(r, tok_latitude,
                    tok_latitudeHemi.p[0],
                    tok_longitude,
                    tok_longitudeHemi.p[0]);
            nmea_reader_update_bearing(r, tok_bearing);
            nmea_reader_update_speed(r, tok_speed);
        }
    } else if (!memcmp(tok.p+3, "EPH", 3)) {
        Token tok_num = nmea_tokenizer_get(tzer, 1);    // number of EPH
        int num = str2int(tok_num.p, tok_num.end);
        UNUSED(num);
        int idx, eph = 0;
        for (idx = 0; idx < GPS_MAX_SVS; idx++) {
            Token tok_satellite = nmea_tokenizer_get(tzer, idx+2);
            eph = str2int(tok_satellite.p, tok_satellite.end);
            if (eph > 0) {
                nmea_cash[idx+1].has_ephemeris_data = 1;
                #if NEMA_DEBUG
                LOGD("eph: %d, sate_id: %d, num: %d\n", eph, idx+1, num);
                #endif
            }
        }
    } else if (!memcmp(tok.p+3, "ALM", 3)) {
        Token tok_num = nmea_tokenizer_get(tzer, 1);    // number of ALM
        int num = str2int(tok_num.p, tok_num.end);
        UNUSED(num);
        int idx, alm = 0;
        for (idx = 0; idx < GPS_MAX_SVS; idx++) {
            Token tok_satellite = nmea_tokenizer_get(tzer, idx+2);
            alm = str2int(tok_satellite.p, tok_satellite.end);
            if (alm > 0) {
                nmea_cash[idx+1].has_almanac_data = 1;
                #if NEMA_DEBUG
                LOGD("alm: %d, sate_id: %d, num: %d\n", alm, idx+1, num);
                #endif
            }
        }
    } else if (!memcmp(tok.p, "GSV", 3)) {
        Token tok_num = nmea_tokenizer_get(tzer, 1);  // number of messages
        Token tok_seq = nmea_tokenizer_get(tzer, 2);  // sequence number
        Token tok_cnt = nmea_tokenizer_get(tzer, 3);  // Satellites in view
        int num = str2int(tok_num.p, tok_num.end);
        int seq = str2int(tok_seq.p, tok_seq.end);
        int cnt = str2int(tok_cnt.p, tok_cnt.end);
        int sv_base = (seq - 1)*NMEA_MAX_SV_INFO;
        int sv_num = cnt - sv_base;
        int idx, base = 4, base_idx;
        if (sv_num > NMEA_MAX_SV_INFO)
            sv_num = NMEA_MAX_SV_INFO;
        if (seq == 1)    /*if sequence number is 1, a new set of GSV will be parsed*/
            r->sv_count = 0;
        for (idx = 0; idx < sv_num; idx++) {
            base_idx = base*(idx+1);
            Token tok_id  = nmea_tokenizer_get(tzer, base_idx+0);
            int sv_id = str2int(tok_id.p, tok_id.end);
            if (sv_type == BDS_SV) {
                sv_id += 200;
                #if NEMA_DEBUG
                LOGD("It is BDS SV: %d", sv_id);
                #endif
            } else if (sv_type == GALILEO_SV) {
                sv_id += 400;
                #if NEMA_DEBUG
                LOGD("It is GALILEO SV: %d", sv_id);
                #endif
            }
            Token tok_ele = nmea_tokenizer_get(tzer, base_idx+1);
            Token tok_azi = nmea_tokenizer_get(tzer, base_idx+2);
            Token tok_snr = nmea_tokenizer_get(tzer, base_idx+3);
            prn[r->sv_count] = str2int(tok_id.p, tok_id.end);
            snr[r->sv_count] = (int)str2float(tok_snr.p, tok_snr.end);

            nmea_reader_update_sv_status(r, sv_base+idx, sv_id, tok_ele, tok_azi, tok_snr);
        }
        if (seq == num) {
            if (r->sv_count <= cnt) {
                r->sv_status.num_svs += r->sv_count;
                if ((1 == MNL_AT_TEST_FLAG) || (1 == MNL_AT_SIGNAL_MODE)) {
                    LOGD("MNL_AT_TEST_FLAG = %d, MNL_AT_SIGNAL_MODE = %d", MNL_AT_TEST_FLAG, MNL_AT_SIGNAL_MODE);
                    gps_at_command_test_proc(r);
                }
            } else {
                LOGE("GPGSV incomplete (%d/%d), ignored!", r->sv_count, cnt);
                r->sv_count = r->sv_status.num_svs = 0;
            }
        }
    }
    // Add for Accuracy
    else if (!memcmp(tok.p, "ACCURACY", 8)) {
#ifdef CONFIG_GPS_MT3303
        if(!has_accuracy) {
            has_accuracy = 1;
        } else {
            mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_ONCEPERSECOND);
            gps_nmea_end_tag = 1;
            #if NEMA_DEBUG
            LOGD("[ZY]gps_nmea_end_tag[%d]", gps_nmea_end_tag);
            #endif
        }
#endif
        if ((r->fix_mode == 3) || (r->fix_mode == 2)) {
            //if(LOC_FIXED(r)) {
            Token  tok_haccuracy = nmea_tokenizer_get(tzer, 1);
            Token  tok_baccuracy = nmea_tokenizer_get(tzer, 2);
            Token  tok_saccuracy = nmea_tokenizer_get(tzer, 3);
            Token  tok_vaccuracy = nmea_tokenizer_get(tzer, 4);

            nmea_reader_update_accuracy(r, tok_haccuracy, tok_baccuracy, tok_saccuracy, tok_vaccuracy);
            #if NEMA_DEBUG
            LOGD("GPS get accuracy from driver:%f\n", r->fix.h_accuracy);
            #endif
        }
    } else if (!memcmp(mtok.p+2, "AG2", 3)) {
        Token tok_agc = nmea_tokenizer_get(tzer, 2);
        g_agc_level = str2int(tok_agc.p, tok_agc.end);
        LOGD("receive PMTKAG2 command, agc: %d\n", g_agc_level);
    } 
#ifdef CONFIG_GPS_MT3303
    else if (!memcmp(mtok.p, "PMTK001", 7) || !memcmp(mtok.p, "PMTK010", 7)) {
    }else if (!memcmp(tok.p+2, "GRP", 3)) {
        if(!has_accuracy) {
            mt3333_controller_socket_send_cmd(MAIN_MT3333_CONTROLLER_EVENT_ONCEPERSECOND);
            gps_nmea_end_tag = 1;
            #if NEMA_DEBUG
            LOGD("[ZY]gps_nmea_end_tag[%d]", gps_nmea_end_tag);
            #endif
        }
    }
#endif
    else {
        tok.p -= 2;
        #if NEMA_DEBUG
        LOGW("unknown sentence '%.*s", (int)(tok.end-tok.p), tok.p);
        #endif
    }

    #if NEMA_DEBUG
    LOGD("flags = %d,1fix_mode=%d gps_nmea_end_tag = %d", r->fix.flags, r->fix_mode,gps_nmea_end_tag);
    #endif
    if (!LOC_FIXED(r)) {
        #if NEMA_DEBUG
        LOGW("Location is not fixed, ignored callback\n");
        #endif
    } else if (r->fix.flags != 0 && gps_nmea_end_tag) {
        #if NEMA_DEBUG
            char   temp[256];
            char*  p   = temp;
            char*  end = p + sizeof(temp);
            struct tm   utc;

        p += snprintf(p, end-p, "2sending fix");
        if (r->fix.flags & MTK_GPS_LOCATION_HAS_LAT_LONG) {
            p += snprintf(p, end-p, " lat=%g lon=%g", r->fix.lat, r->fix.lng);
        }
        if (r->fix.flags & MTK_GPS_LOCATION_HAS_ALT) {
            p += snprintf(p, end-p, " altitude=%g", r->fix.alt);
        }
        if (r->fix.flags & MTK_GPS_LOCATION_HAS_SPEED) {
            p += snprintf(p, end-p, " speed=%g", r->fix.speed);
        }
        if (r->fix.flags & MTK_GPS_LOCATION_HAS_BEARING) {
            p += snprintf(p, end-p, " bearing=%g", r->fix.bearing);
        }
        if (r->fix.flags & MTK_GPS_LOCATION_HAS_HORIZONTAL_ACCURACY) {
            p += snprintf(p, end-p, " accuracy=%g", r->fix.h_accuracy);
            LOGD("GPS accuracy=%g\n", r->fix.h_accuracy);
        }
        gmtime_r((time_t*) &r->fix.timestamp, &utc);
        p += snprintf(p, end-p, " time=%s", asctime(&utc));
        LOGW("%s", temp);
        #endif
        mnl2hal_location(r->fix);
        mnld_gps_update_location(r->fix);
        r->fix.flags = 0;
    }

    #if NEMA_DEBUG
    LOGD("r->sv_status.num_svs = %d, gps_nmea_end_tag = %d", r->sv_status.num_svs, gps_nmea_end_tag);
    #endif
    if (gps_nmea_end_tag) {
        mnl2hal_gps_sv(r->sv_status);
#ifdef CONFIG_GPS_MT3303
        if(mnld_is_gps_meas_enabled()){
            extern gnss_sv_info cts_sv_data;
            cts_sv_data = r->sv_status;
        }
#endif

        r->sv_count = r->sv_status.num_svs = 0;
        memset((void*)nmea_cash, 0x00, sizeof(nmea_cash));
    }
}

static int mtk_mnld_nmea_paser_filter(char* const nmea_stnc, int length)
{
    char* buf;
    int ret = 0;

    if(nmea_stnc == NULL) {
        LOGE("pointer fail!");
        return 0;
    }

    if(length < 6) {
        LOGE("length fail:%d", length);
        return 0;
    }

    buf = nmea_stnc+3; //skip "$GP", "$GL", "$BD", "$GA"
#ifdef CONFIG_GPS_MT3303
if ((!memcmp(buf, "GGA", 3)) ||
     (!memcmp(buf, "GSA", 3)) ||
     (!memcmp(buf, "RMC", 3)) ||
     (!memcmp(buf+3, "EPH", 3)) ||
     (!memcmp(buf+3, "ALM", 3)) ||
     (!memcmp(buf, "GSV", 3)) ||
     (!memcmp(buf, "ACCURACY", 8)) ||
     (!memcmp(buf+2, "001", 3)) ||
     (!memcmp(buf+2, "010", 3)) ||
     (!memcmp(buf+2, "AGC", 3)) ||
     (!memcmp(buf+2, "CLK", 3)) ||
     (!memcmp(buf+2, "SUPL", 4)) ||
     (!memcmp(buf+2, "AG2", 3)) ||
     (!memcmp(buf+2, "GRP", 3))
#else
    if ((!memcmp(buf, "GGA", 3)) ||
        (!memcmp(buf, "GSA", 3)) ||
        (!memcmp(buf, "RMC", 3)) ||
        (!memcmp(buf+3, "EPH", 3)) ||
        (!memcmp(buf+3, "ALM", 3)) ||
        (!memcmp(buf, "GSV", 3)) ||
        (!memcmp(buf, "ACCURACY", 8)) ||
        (!memcmp(buf, "VTG", 3)) ||
        (!memcmp(buf+2, "TIM", 3)) ||
        (!memcmp(buf+2, "840", 3)) ||
        (!memcmp(buf+2, "AGC", 3)) ||
        (!memcmp(buf+2, "CLK", 3)) ||
        (!memcmp(buf+2, "SUPL", 4)) ||
        (!memcmp(buf+2, "AG2", 3)) ||
        (!memcmp(buf+2, "DOP", 3))
#endif
    ) {
        ret = 1;
    } else {
        ret = 0;
    }

    return ret;
}

void extract_rmc_time_and_date(const char *nmea_sentences, NmeaReader* const r){
    const char *rmc_header = "RMC";
    char    rmc[ NMEA_MAX_SIZE+1] = {};
    size_t rmc_length = 0;
    NmeaTokenizer  tzer[1];

    const char *rmc_start= strstr(nmea_sentences, rmc_header);
//  rmc_start = rmc_start -3;
    if (rmc_start) {
        // Ensure that moving back 3 characters does not go before the start of the buffer
        if (rmc_start > nmea_sentences + 3) {
        rmc_start -= 3;
        } else {
        // If the header is too close to the start, adjust to the start of the buffer
            rmc_start = nmea_sentences;
        }
        const char *rmc_end = strchr(rmc_start, '\n');
        if (rmc_end) {
            rmc_length = rmc_end - rmc_start;
            if (rmc_length < sizeof(rmc)) {
                strncpy(rmc, rmc_start, rmc_length);
                #if NEMA_DEBUG
                LOGD("get rmc[%s]\n", rmc);
                #endif
                rmc[rmc_length] = '\0'; // Ensure null-termination
                nmea_tokenizer_init(tzer, rmc, rmc + rmc_length);
                Token  tok_time          = nmea_tokenizer_get(tzer, 1);
                Token  tok_fixStatus     = nmea_tokenizer_get(tzer, 2);
                Token  tok_date          = nmea_tokenizer_get(tzer, 9);
                if (tok_fixStatus.p[0] == 'A') {
                    nmea_reader_update_date(r, tok_date, tok_time);
                }
            } else {
                LOGD("RMC sentence is too long for the buffer.\n");
            }
	}else {
            LOGD("Failed to find the end of the RMC sentence.\n");
        }
    } else {
        LOGD("RMC header not found in NMEA sentences.\n");
    }
}


void nmea_reader_addc(NmeaReader* const r, int  c) {
    if (r->overflow) {
        r->overflow = (c != '\n');
        return;
    }

    if (r->pos >= NMEA_MAX_SIZE) {
        r->overflow = 1;
        r->pos      = 0;
        return;
    }

    r->in[r->pos] = (char)c;
    r->pos += 1;

    if (c == '\n') {
        if(mtk_mnld_nmea_paser_filter(r->in, r->pos)) {
            nmea_reader_parse(r);
        }
        // LOGD("the structure include nmea_cb address is %p\n", r);
#ifndef CONFIG_GPS_MT3303
        mnl2hal_nmea(r->fix.timestamp, r->in, r->pos);
#else
        if(is_ygps_delete_data == 0){
            mnl2hal_nmea(r->fix.timestamp, r->in, r->pos);
        }
#endif
        // LOGD("length: %d, nmea sentence: %s\n", r->pos, r->in);
        r->pos = 0;
        memset(r->in, 0x00, NMEA_MAX_SIZE+1);
    }
}

FILE *file = NULL;
char line[10*1024] = {};

void mtk_mnl_nmea_parser_process(const char * buffer, UINT32 length) {
    UINT32  nn;
#ifndef CONFIG_GPS_MT3303
    NmeaReader  reader[1];
#else
    static NmeaReader  reader[1]={0};
#endif

#ifndef CONFIG_GPS_MT3303
    nmea_reader_init(reader);
    gps_nmea_end_tag = 0;
#endif

#if 1
    // Open the file
    if (file == NULL){
        mkdir("mkdir /data/misc/gps", 0775);
        file = fopen("/data/misc/gps/parser.log", "w");
        if (file == NULL) {
            LOGD("Error opening file");
        }
        #if NEMA_DEBUG
        LOGD("[ZY]open file");
        #endif

    }
        if (file != NULL){
            #if NEMA_DEBUG
            LOGD("[ZY]write to file");
            #endif
            memset(line, 0, sizeof(line));
            snprintf(line, length, "%s\n", buffer);
            #if NEMA_DEBUG
            LOGD("[ZY]write to file[%s]", buffer);
            #endif
            
fputs("line begin\n", file);
            fputs(line, file);
            fputs("line end\n", file);
            
        }



#endif

    for (nn = 0; nn < length; nn++) {
#ifndef CONFIG_GPS_MT3303
        if (nn == (length-1)) {
            gps_nmea_end_tag = 1;
            #if NEMA_DEBUG
            LOGD("[ZY]gps_nmea_end_tag[%d]", gps_nmea_end_tag);
            #endif
        }
#endif  
        extract_rmc_time_and_date(buffer, reader);
        nmea_reader_addc(reader, buffer[nn]);
    }
}

int get_gps_nmea_parser_end_status() {
    int ret = 0;

    ret = gps_nmea_end_tag;
    return ret;
}
