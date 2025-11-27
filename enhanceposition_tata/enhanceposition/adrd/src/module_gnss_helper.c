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
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>  /*struct sockaddr_un*/
#include <netinet/in.h>

#include "zalloc.h"
#include "adr_util.h"
#include "adr_data_struct.h"
#include "module_gnss_helper.h"

#if defined(__ANDROID_OS__)
#include <hardware/gps.h>
#elif defined(__LINUX_OS__)
#include "gps.h"
#endif

extern int gps_gsa_fix_3dmode;
extern int upload_number;
#define NEMA_DEBUG 0

void update_gnss_time_diff(struct date_time *dtime,
            struct timespec *drvtime, struct timespec *result)
{
    struct tm gnss_tm;
    long int tv_nsec = 0;

    memset(&gnss_tm, 0, sizeof(gnss_tm));
    gnss_tm.tm_year = dtime->date_yyyy - 1900;
    gnss_tm.tm_mon = dtime->date_mm - 1;
    gnss_tm.tm_mday = dtime->date_dd;
    gnss_tm.tm_hour = dtime->time_hh;
    gnss_tm.tm_min = dtime->time_mm;
    gnss_tm.tm_sec = dtime->time_ss;
    gnss_tm.tm_isdst = 0;

    tv_nsec = dtime->time_ms * 1000000;

    if(tv_nsec >= drvtime->tv_nsec){
        result->tv_sec = mktime(&gnss_tm) - drvtime->tv_sec;
        result->tv_nsec = tv_nsec - drvtime->tv_nsec;
    } else{
        result->tv_sec = mktime(&gnss_tm) - drvtime->tv_sec - 1;
        result->tv_nsec = tv_nsec - drvtime->tv_nsec + 1000000000;
    }
}

void nmea_reader_init(gnss_module_data* const r)
{
    uint32_t idx;

    memset( r, 0, sizeof(*r) );

    r->flags = FRAME_TYPE_UPDATE_NO;
    r->frame_flag = 0;
    r->pmtkchl.gnss_chl.num_sv = 0;
    r->nmeagsa.fix_mode = 0;
    r->nmeagsa.sate_num = 0;
    r->nmeagsv.sv_status.num_svs = 0;
    r->sv_out.nmea.len = 0;
    r->sv_out.nsv = 0;
    r->sv_out.pmtk_cmd_ack = NULL;
    r->sv_out.nmea.sentence[r->sv_out.nmea.len]='\0';
    memset((void*)&r->nmeagsv.sv_status, 0x00, sizeof(r->nmeagsv.sv_status));
    memset((void*)r->in, 0x00, sizeof(r->in));

    for (idx = 0; idx < MTK_MNLD_GNSS_MAX_SVS; idx++)
    {
        r->pmtkchl.gnss_chl.sv_chl[idx].used_in_fix = 0;
        r->nmeagsa.gps_sv_used_in_fix[idx].used_in_fix = 0;;
    }

    for (idx = 0; idx < GNSS_MAX_SVID; idx++)
    {
        r->nmeagsa.gps_sv_used_in_fix[idx].used_in_fix = 0;;
    }

}

static int nmea_reader_update_altitude(double* const retalt,
                                       Token sea_altitude, Token geo_altitude)
{
    Token   tok_sea = sea_altitude;
    Token   tok_geo = geo_altitude;

    if ((tok_sea.p >= tok_sea.end) || (tok_geo.p >= tok_geo.end))
        return -1;

    *retalt = str2float(tok_sea.p, tok_sea.end);
    *retalt += str2float(tok_geo.p, tok_geo.end);
    return 0;
}

static int nmea_reader_update_time(gnss_module_data* const r, Token  tok)
{
    int        hour, minute,seconds,msec;
    char dtime[32] = {0};
    char *p = dtime;
    long long timetmp = 0;

    if (tok.p + 6 > tok.end)
        return -1;

    hour    = str2int(tok.p,   tok.p + 2);
    minute  = str2int(tok.p + 2, tok.p + 4);
    seconds = str2int(tok.p + 4, tok.p + 6);

    msec = str2int(tok.p + 7, tok.end);

    r->dtime.time_hh = hour;
    r->dtime.time_mm = minute;
    r->dtime.time_ss = seconds;
    r->dtime.time_ms = msec;

    p += snprintf(p, sizeof(struct date_time),
                  "%04d%02d%02d%02d%02d%02d", r->dtime.date_yyyy,
                  r->dtime.date_mm, r->dtime.date_dd, r->dtime.time_hh,
                  r->dtime.time_mm, r->dtime.time_ss);

    timetmp = atoll(dtime);
    r->nmearmc.utc_data_time = timetmp;
    r->nmearmc.utc_time_msec = r->dtime.time_ms;

    return 0;
}

static int nmea_reader_update_date(gnss_module_data* const r, Token  date, Token  time)
{

    Token  tok = date;
    int    day, mon, year;

    if (tok.p + 6 != tok.end)
    {
        LOG_ERROR("date not properly formatted: '%.*s'", (int)(tok.end-tok.p), tok.p);
        return -1;
    }
    day  = str2int(tok.p, tok.p + 2);
    mon  = str2int(tok.p + 2, tok.p + 4);
    year = str2int(tok.p + 4, tok.p + 6) + 2000;
    if (year == 2080)
    {
        year = 1980;
    }

    if ((day|mon|year) < 0)
    {
        LOG_ERROR("date not properly formatted: '%.*s'", (int)(tok.end-tok.p), tok.p);
        return -1;
    }

    r->dtime.date_yyyy = year;
    r->dtime.date_mm   = mon;
    r->dtime.date_dd   = day;

    return nmea_reader_update_time(r, time);
}

static double convert_from_hhmm(Token  tok)
{
    double  val     = str2float(tok.p, tok.end);
    int     degrees = (int)(floor(val) / 100);
    double  minutes = val - degrees*100.;
    double  dcoord  = degrees + minutes / 60.0;
    return dcoord;
}

static int nmea_reader_update_latlong(double* const retlat,
                                      double* const retlng,
                                      Token        latitude,
                                      char         latitudeHemi,
                                      Token        longitude,
                                      char         longitudeHemi)
{
    double   lat, lon;
    Token    tok;

    tok = latitude;
    if (tok.p + 6 > tok.end) {
        LOG_ERROR("latitude is too short: '%.*s'", (int)(tok.end-tok.p), tok.p);
        return -1;
    }
    lat = convert_from_hhmm(tok);
    if (latitudeHemi == 'S')
        lat = -lat;

    tok = longitude;
    if (tok.p + 6 > tok.end) {
        LOG_ERROR("longitude is too short: '%.*s'", (int)(tok.end-tok.p), tok.p);
        return -1;
    }
    lon = convert_from_hhmm(tok);
    if (longitudeHemi == 'W')
        lon = -lon;

    *retlat  = lat;
    *retlng = lon;
    return 0;
}

static int nmea_reader_update_bearing(double* const retbearing,
                                      Token bearing)
{
    Token    tok = bearing;

    if (tok.p >= tok.end)
        return -1;

    *retbearing = str2float(tok.p, tok.end);
    return 0;
}

static int nmea_reader_update_speed(double* const retspeed,
                                    Token speed)
{
    Token tok = speed;

    if (tok.p >= tok.end)
        return -1;

    *retspeed = str2float(tok.p, tok.end) / 1.942795467;
    return 0;
}

// Add by LCH for accuracy
static int nmea_reader_update_accuracy(double* const retaccuracy,
                                       Token accuracy)
{
    Token   tok = accuracy;

    if (tok.p >= tok.end)
        return -1;

    *retaccuracy = str2float(tok.p, tok.end);
    return 0;
}

static int nmea_reader_update_sv_status(gnss_module_data* r, int sv_index,
                                        int id, Token elevation,
                                        Token azimuth, Token snr)
{
    int prn = id;

    sv_index = r->nmeagsv.sv_count + r->nmeagsv.sv_status.num_svs;
    if (MTK_MNLD_GNSS_MAX_SVS <= sv_index) {
        LOG_ERROR("ERR: sv_index=[%d] is larger than MTK_MNLD_GNSS_MAX_SVS=[%d].",
            sv_index, MTK_MNLD_GNSS_MAX_SVS);
        return 0;
    }
    r->nmeagsv.sv_status.sv_list[sv_index].svid = prn;
    r->nmeagsv.sv_status.sv_list[sv_index].c_n0_dbhz = str2float(snr.p, snr.end);
    r->nmeagsv.sv_status.sv_list[sv_index].elevation = str2int(elevation.p, elevation.end);
    r->nmeagsv.sv_status.sv_list[sv_index].azimuth = str2int(azimuth.p, azimuth.end);
    r->nmeagsv.sv_count++;

    return 0;
}

static void nmea_reader_parse(gnss_module_data* const r)
{
    /* we received a complete sentence, now parse it to generate
    * a new GPS fix...
    */
    int32_t index;

    ProtoTokenizer  tzer[1];
    Token          tok;
    Token          mtok;
    SV_TYPE sv_type = 0;

#if NEMA_DEBUG
    LOG_DEBUG("Received: '%.*s'", (int)r->pos, r->in);
#endif
    if (r->pos < 9)
    {
        LOG_ERROR("Too short. discarded. '%.*s'", (int)r->pos, r->in);
        return;
    }

    proto_tokenizer_init(tzer, r->in, r->in + r->pos);
#if NEMA_DEBUG
    {
        int  n;
        LOG_DEBUG("Found %d tokens", tzer->count);
        for (n = 0; n < tzer->count; n++)
        {
            Token  tok = proto_tokenizer_get(tzer, n);
            LOG_DEBUG("%2d: '%.*s'", n, (int)(tok.end-tok.p), tok.p);
        }
    }
#endif

    tok = proto_tokenizer_get(tzer, 0);
    if (tok.p + 5 > tok.end)
    {
        LOG_ERROR("sentence id '%.*s' too short, ignored.", (int)(tok.end-tok.p), tok.p);
        return;
    }

    // ignore first two characters.
    mtok.p = tok.p; // Mark the first two char for GPS,GLONASS,BDS , GALILEO SV parse.
    if (!memcmp(mtok.p, "BD", 2))
        sv_type = BDS_SV;
    else if (!memcmp(mtok.p, "GP", 2))
        sv_type = GPS_SV;
    else if (!memcmp(mtok.p, "GL", 2))
        sv_type = GLONASS_SV;
    else if (!memcmp(mtok.p, "GA", 2))
        sv_type = GALILEO_SV;

    tok.p += 2;
    if (!memcmp(tok.p, "GGA", 3)) {
        // GPS fix
        Token  tok_time          = proto_tokenizer_get(tzer, 1);
        Token  tok_latitude      = proto_tokenizer_get(tzer, 2);
        Token  tok_latitudeHemi  = proto_tokenizer_get(tzer, 3);
        Token  tok_longitude     = proto_tokenizer_get(tzer, 4);
        Token  tok_longitudeHemi = proto_tokenizer_get(tzer, 5);
        Token  tok_fixstate      = proto_tokenizer_get(tzer, 6);
        Token  tok_numsatefix 	 = proto_tokenizer_get(tzer, 7);
        Token  tok_hordop        = proto_tokenizer_get(tzer, 8);
        Token  tok_altitude      = proto_tokenizer_get(tzer, 9);
        //Token  tok_altitudeUnits = proto_tokenizer_get(tzer, 10);
        Token  tok_geo_altitude      = proto_tokenizer_get(tzer, 11);
        //Token  tok_geo_altitudeUnits = proto_tokenizer_get(tzer, 12);
        Token  tok_difftime = proto_tokenizer_get(tzer, 13);
        Token  tok_diffstationId = proto_tokenizer_get(tzer, 14);

        r->nmeagga.fix_state = str2int(tok_fixstate.p, tok_fixstate.end);
        //if ((r->nmeagga.fix_state == 1) ||(r->nmeagga.fix_state == 2) ||
        //	(r->nmeagga.fix_state == 6))
        {
            nmea_reader_update_latlong(&r->nmeagga.lat, &r->nmeagga.lng,
                                       tok_latitude,
                                       tok_latitudeHemi.p[0],
                                       tok_longitude,
                                       tok_longitudeHemi.p[0]);
            nmea_reader_update_altitude(&r->nmeagga.alt, tok_altitude, tok_geo_altitude);

            r->nmeagga.utc_time = str2float(tok_time.p, tok_time.end);
            r->nmeagga.nsf = str2int(tok_numsatefix.p, tok_numsatefix.end);
            r->nmeagga.hdop = str2int(tok_hordop.p, tok_hordop.end);
            r->nmeagga.diff_time = str2int(tok_difftime.p, tok_difftime.end);
            r->nmeagga.diff_stationId = str2int(tok_diffstationId.p,
                                                tok_diffstationId.end);

            r->frame_flag |= FRAME_START;
            r->flags |= FRAME_TYPE_UPDATE_GGA;
        }
    } else if (!memcmp(tok.p, "GSA", 3)) {
        Token tok_fix_pattern = proto_tokenizer_get(tzer, 1);
        Token tok_fix_type = proto_tokenizer_get(tzer, 2);
        int idx, max = 12;  /*the number of satellites in GPGSA*/
        int numsates = 0;
        int fix_mode = 0;

        r->nmeagsa.fix_pattern = str2int(tok_fix_pattern.p, tok_fix_pattern.end);
        fix_mode = str2int(tok_fix_type.p, tok_fix_type.end);
        //r->fix_mode = str2int(tok_fix_type.p, tok_fix_type.end);

        //if ((fix_mode == 2) || (fix_mode == 3))/* 1: No fix; 2: 2D; 3: 3D*/
        {
            for (idx = 0; idx < max; idx++)
            {
                Token tok_satellite = proto_tokenizer_get(tzer, idx+3);
                if (tok_satellite.p == tok_satellite.end)
                {
                    LOG_DEBUG("GSA: found %d active satellites\n", idx);
                    break;
                }
                int sate_id = str2int(tok_satellite.p, tok_satellite.end);
                /*GPS:1~32, GLONASS:101~124, BDS:201~214*/
                if (sv_type == BDS_SV) {
                    sate_id += 200;
                }
                else if (sv_type == GLONASS_SV) {
                    sate_id += 36;
                }
                else if (sv_type == GALILEO_SV) {
                    sate_id += 400;
                }
                if (sate_id >= 0 && sate_id < GNSS_MAX_SVID) {
                    r->nmeagsa.gps_sv_used_in_fix[sate_id].prn = sate_id;
                    r->nmeagsa.gps_sv_used_in_fix[sate_id].used_in_fix = 1;
                    numsates += 1;
                }
            }
            /*only GPGSA update fixmode/dop*/
            if ( !memcmp(tok.p - 2, "GPGSA", 5) )
            {
                /*pdop hdop vdop*/
                Token tok_pdop = proto_tokenizer_get(tzer, 15);
                Token tok_hdop = proto_tokenizer_get(tzer, 16);
                Token tok_vdop = proto_tokenizer_get(tzer, 17);

                r->nmeagsa.pdop = str2float(tok_pdop.p, tok_pdop.end);
                r->nmeagsa.hdop = str2float(tok_hdop.p, tok_hdop.end);
                r->nmeagsa.vdop = str2float(tok_vdop.p, tok_vdop.end);

                r->nmeagsa.fix_mode = fix_mode;
            }
            /*sum of GL/GP/BD*/
            r->nmeagsa.sate_num += numsates;
            r->flags |= FRAME_TYPE_UPDATE_GSA;
        }
    } else if (!memcmp(tok.p, "RMC", 3)) {
        Token  tok_time          = proto_tokenizer_get(tzer, 1);
        Token  tok_fixStatus     = proto_tokenizer_get(tzer, 2);
        Token  tok_latitude      = proto_tokenizer_get(tzer, 3);
        Token  tok_latitudeHemi  = proto_tokenizer_get(tzer, 4);
        Token  tok_longitude     = proto_tokenizer_get(tzer, 5);
        Token  tok_longitudeHemi = proto_tokenizer_get(tzer, 6);
        Token  tok_speed         = proto_tokenizer_get(tzer, 7);
        Token  tok_bearing       = proto_tokenizer_get(tzer, 8);
        Token  tok_date          = proto_tokenizer_get(tzer, 9);
        Token  tok_magBiasDeg    = proto_tokenizer_get(tzer, 10);
        Token  tok_magBiasDir    = proto_tokenizer_get(tzer, 11);

        //nmea_reader_update_bearing(r->nmearmc.bearing, tok_bearing);
        r->nmearmc.fix_mode = str2int(tok_fixStatus.p, tok_fixStatus.end);
        nmea_reader_update_date(r, tok_date, tok_time);

        /*static double last_utc_data_time = 0;
        static int last_utc_time_msec = 0;
        if (gps_gsa_fix_3dmode == 1) {

        if(last_utc_data_time > r->nmearmc.utc_data_time || (last_utc_data_time == r->nmearmc.utc_data_time && last_utc_time_msec >= r->nmearmc.utc_time_msec))
        {
            LOG_ERROR("RMC new datetime [%f, %dms] <= last datetime [%f, %dms]\n",
                   r->nmearmc.utc_data_time, r->nmearmc.utc_time_msec, last_utc_data_time, last_utc_time_msec);
            abort();
        }else {
            LOG_DEBUG("RMC new datetime [%f, %dms] > last datetime [%f, %dms]\n",
                   r->nmearmc.utc_data_time, r->nmearmc.utc_time_msec, last_utc_data_time, last_utc_time_msec);
           last_utc_data_time = r->nmearmc.utc_data_time;
           last_utc_time_msec = r->nmearmc.utc_time_msec ;
        }
    }*/
        nmea_reader_update_latlong(&r->nmearmc.lat, &r->nmearmc.lng,
                                   tok_latitude,
                                   tok_latitudeHemi.p[0],
                                   tok_longitude,
                                   tok_longitudeHemi.p[0]);
        nmea_reader_update_bearing(&r->nmearmc.bearing, tok_bearing);
        nmea_reader_update_speed(&r->nmearmc.speed, tok_speed);
        r->nmearmc.magn_bias_deg = str2int(tok_magBiasDeg.p, tok_magBiasDeg.end);
        r->nmearmc.magn_bias_deg_dir =
            str2int(tok_magBiasDir.p, tok_magBiasDir.end);
        r->flags |= FRAME_TYPE_UPDATE_RMC;
    } else if (!memcmp(tok.p, "GSV", 3)) {
        Token tok_num = proto_tokenizer_get(tzer, 1);  // number of messages
        Token tok_seq = proto_tokenizer_get(tzer, 2);  // sequence number
        Token tok_cnt = proto_tokenizer_get(tzer, 3);  // Satellites in view
        int num = str2int(tok_num.p, tok_num.end);
        int seq = str2int(tok_seq.p, tok_seq.end);
        int cnt = str2int(tok_cnt.p, tok_cnt.end);
        int sv_base = (seq - 1)*NMEA_MAX_SV_INFO;
        int sv_num = cnt - sv_base;
        int idx, base = 4, base_idx;

        if (sv_num > NMEA_MAX_SV_INFO)
            sv_num = NMEA_MAX_SV_INFO;
        if (seq == 1)    /*if sequence number is 1, a new set of GSV will be parsed*/
            r->nmeagsv.sv_count = 0;
        for (idx = 0; idx < sv_num; idx++) {
            base_idx = base * (idx + 1);
            Token tok_id  = proto_tokenizer_get(tzer, base_idx + 0);
            int sv_id = str2int(tok_id.p, tok_id.end);
            if (sv_type == BDS_SV) {
                sv_id += 200;
            } else if (sv_type == GLONASS_SV) {
                sv_id += 36;
            }else if (sv_type == GALILEO_SV) {
                sv_id += 400;
            }
            Token tok_ele = proto_tokenizer_get(tzer, base_idx + 1);
            Token tok_azi = proto_tokenizer_get(tzer, base_idx + 2);
            Token tok_snr = proto_tokenizer_get(tzer, base_idx + 3);

            nmea_reader_update_sv_status(r, sv_base+idx, sv_id, tok_ele, tok_azi, tok_snr);
        }
        if (seq == num) {
            if (r->nmeagsv.sv_count <= cnt) {
                r->nmeagsv.sv_status.num_svs += r->nmeagsv.sv_count;
                r->flags |= FRAME_TYPE_UPDATE_GSV;
            } else {
                LOG_ERROR("nmeagsv incomplete (%d/%d), ignored!", r->nmeagsv.sv_count, cnt);
                r->nmeagsv.sv_count = r->nmeagsv.sv_status.num_svs = 0;
            }
        }
    } else if (!memcmp(tok.p, "EPE", 3)) {
        Token tok_2d = proto_tokenizer_get(tzer, 1);    //estimated horizontal position error
        Token tok_vert = proto_tokenizer_get(tzer, 2);  //estimated vertical position error

        r->nmeaepe.epe_2d = str2float(tok_2d.p, tok_2d.end);
        r->nmeaepe.epe_vert = str2float(tok_vert.p, tok_vert.end);

        r->frame_flag |= FRAME_END;
        r->flags |= FRAME_TYPE_UPDATE_EPE;
    } else if (!memcmp(tok.p, "ACCURACY", 8)) {
        Token  tok_haccuracy = proto_tokenizer_get(tzer, 1);
        nmea_reader_update_accuracy(&r->nmeaaccuracy.accuracy, tok_haccuracy);
        r->flags |= FRAME_TYPE_UPDATE_ACCURACY;
    } else if (!memcmp(tok.p, "TKTIM", 5)) {
        Token  tok_boottime = proto_tokenizer_get(tzer, 1);
        char snsec[10] = {0};
        char ssec[12] = {0};
        int insec = 0;
        long lsec = 0;
        strncpy(snsec,tok_boottime.end-9,9);
        strncpy(ssec,tok_boottime.p,tok_boottime.end - tok_boottime.p - 9);
        insec = atoi(snsec);
        lsec = atol(ssec);
        r->drvtime.tv_sec = lsec;
        r->drvtime.tv_nsec = insec;

        /*static time_t last_lsec = 0;
        static long last_insec = 0;
        if(last_lsec > r->drvtime.tv_sec || (last_lsec == r->drvtime.tv_sec && last_insec > r->drvtime.tv_nsec)){
            LOG_ERROR("TKTIM new time [%ld sec, %ld ns] < last time [%ld sec, %ld ns]\n",
                       r->drvtime.tv_sec, r->drvtime.tv_nsec, last_lsec, last_insec);
                    abort();
        }else {
            LOG_DEBUG("TKTIM new time [%ld sec, %ld ns] >= last time [%ld sec, %ld ns]\n",
                      r->drvtime.tv_sec, r->drvtime.tv_nsec, last_lsec, last_insec);
            last_lsec = r->drvtime.tv_sec;
            last_insec = r->drvtime.tv_nsec ;
        }*/

        //gps_kernel_time  = lsec*1000000000LL + insec;
        if (r->nmeagsa.fix_mode == 3 && gps_gsa_fix_3dmode == 0)
        {
            gps_gsa_fix_3dmode = 1;
        }

        //if (gps_gsa_fix_3dmode == 1) 
        {
            struct timespec *available_timespec = get_available_timespec();
            if(available_timespec){
                update_gnss_time_diff(&r->dtime, &r->drvtime, available_timespec);
                if(upload_number > 2){
                    struct timespec avg_time_diff;
                    if(get_gnss_average_time_diff(&avg_time_diff)){
                        struct timespec diff_uploadtime_avg_time;
                        if(!get_diff_uploadtime_avg_time(&avg_time_diff, &diff_uploadtime_avg_time)){
                            LOG_ERROR("get_diff_uploadtime_avg_time failed, abort the program..." );
                            abort();
                        } else if (diff_uploadtime_avg_time.tv_sec || (diff_uploadtime_avg_time.tv_nsec /1000000 >= 10)){
                            struct timespec std_dev_time_diff;
                            if(get_gnss_std_deviation_time_diff(&avg_time_diff, &std_dev_time_diff)){
                                if(!std_dev_time_diff.tv_sec && (std_dev_time_diff.tv_nsec /1000000 < 3)){
                                    gnss_notify_time_msg(available_timespec);
                                }
                            }
                        }
                    } else {
                        LOG_ERROR("get gnss average of cached time diff failed");
                    }
                }else if(upload_number == 1 && gps_gsa_fix_3dmode == 1){
                    gnss_notify_time_msg(available_timespec);
                    upload_number += 1;
                }else if(upload_number == 0){
                    gnss_notify_time_msg(available_timespec);
                    upload_number += 1;
                }
            } else {
                LOG_ERROR("get time buffer failed, abort the program");
                abort();
            }
        }
    } else if (!memcmp(tok.p, "TKCHL", 5)) {
        int32_t cnt = r->pmtkchl.gnss_chl.num_sv;
        int32_t sys_type = -1;
        int32_t sate_base = 0;

        Token  tok_sysId         = proto_tokenizer_get(tzer, 1);
        Token  tok_sateId        = proto_tokenizer_get(tzer, 2);
        Token  tok_presudo       = proto_tokenizer_get(tzer, 3);
        Token  tok_syncCarPh     = proto_tokenizer_get(tzer, 4);
        Token  tok_doppler       = proto_tokenizer_get(tzer, 5);
        Token  tok_cycleslip     = proto_tokenizer_get(tzer, 6);
        Token  tok_snr           = proto_tokenizer_get(tzer, 7);
        Token  tok_ecefX         = proto_tokenizer_get(tzer, 8);
        Token  tok_ecefY         = proto_tokenizer_get(tzer, 9);
        Token  tok_ecefZ         = proto_tokenizer_get(tzer, 10);
        Token  tok_freChl        = proto_tokenizer_get(tzer, 11);
        Token  tok_iodeTb        = proto_tokenizer_get(tzer, 12);
        Token  tok_ionoCorr      = proto_tokenizer_get(tzer, 13);
        Token  tok_ionoSrc       = proto_tokenizer_get(tzer, 14);
        Token  tok_syncStatus    = proto_tokenizer_get(tzer, 15);

        r->pmtkchl.gnss_chl.sv_chl[cnt].sysId =
            str2int(tok_sysId.p, tok_sysId.end);
        sys_type = r->pmtkchl.gnss_chl.sv_chl[cnt].sysId;
        int sv_id = str2int(tok_sateId.p, tok_sateId.end);

        if (sys_type == GPS_SV)
            sate_base = 0;
        if (sys_type == GLONASS_SV)
            sate_base = 100;
        if (sys_type == BDS_SV)
            sate_base = 0;

        r->pmtkchl.gnss_chl.sv_chl[cnt].sateId = sate_base +sv_id;
        r->pmtkchl.gnss_chl.sv_chl[cnt].psedorange =
            str2float(tok_presudo.p, tok_presudo.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].sync_carr_phase =
            str2float(tok_syncCarPh.p, tok_syncCarPh.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].doppler =
            str2float(tok_doppler.p, tok_doppler.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].cycle_slip =
            str2float(tok_cycleslip.p, tok_cycleslip.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].snr =
            str2int(tok_snr.p, tok_snr.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].ecef_x =
            str2float(tok_ecefX.p, tok_ecefX.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].ecef_y =
            str2float(tok_ecefY.p, tok_ecefY.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].ecef_z =
            str2float(tok_ecefZ.p, tok_ecefZ.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].channel_hz =
            str2int(tok_freChl.p, tok_freChl.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].iode_tb =
            str2int(tok_iodeTb.p, tok_iodeTb.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].iono_corr =
            str2float(tok_ionoCorr.p, tok_ionoCorr.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].iode_src =
            str2int(tok_ionoSrc.p, tok_ionoSrc.end);
        r->pmtkchl.gnss_chl.sv_chl[cnt].sync_status =
            str2int(tok_syncStatus.p, tok_syncStatus.end);

        r->pmtkchl.gnss_chl.num_sv ++;
        r->flags |= FRAME_TYPE_UPDATE_CHL;
    } else if (!memcmp(tok.p, "TKGRP", 5)) {
        Token  tok_clktime       = proto_tokenizer_get(tzer, 1);
        Token  tok_timeweek      = proto_tokenizer_get(tzer, 2);
        Token  tok_weeknum       = proto_tokenizer_get(tzer, 3);
        Token  tok_clkstatus     = proto_tokenizer_get(tzer, 4);
        Token  tok_utcoffset     = proto_tokenizer_get(tzer, 5);
        Token  tok_clockbias     = proto_tokenizer_get(tzer, 6);
        Token  tok_offsetA       = proto_tokenizer_get(tzer, 7);
        Token  tok_offsetB       = proto_tokenizer_get(tzer, 8);

        r->pmtkgrp.clocktime     = str2int(tok_clktime.p, tok_clktime.end);
        r->pmtkgrp.timeweeks     = str2float(tok_timeweek.p, tok_timeweek.end);
        r->pmtkgrp.weeknum       = str2int(tok_weeknum.p, tok_weeknum.end);
        r->pmtkgrp.clk_status    = str2int(tok_clkstatus.p, tok_clkstatus.end);
        r->pmtkgrp.utc_offset    = str2int(tok_utcoffset.p, tok_utcoffset.end);
        r->pmtkgrp.clk_bias      = str2int(tok_clockbias.p, tok_clockbias.end);
        r->pmtkgrp.clk_offsetA   = str2int(tok_offsetA.p, tok_offsetA.end);
        r->pmtkgrp.clk_offsetB   = str2int(tok_offsetB.p, tok_offsetB.end);

        r->flags |= FRAME_TYPE_UPDATE_GRP;
    } else if (!memcmp(tok.p, "TKVNED", 6)) {
        Token  tok_clktime       = proto_tokenizer_get(tzer, 1);
        Token  tok_vn            = proto_tokenizer_get(tzer, 2);
        Token  tok_ve            = proto_tokenizer_get(tzer, 3);
        Token  tok_vu            = proto_tokenizer_get(tzer, 4);
        Token  tok_horspeed      = proto_tokenizer_get(tzer, 5);
        Token  tok_speed         = proto_tokenizer_get(tzer, 6);
        Token  tok_vnacc         = proto_tokenizer_get(tzer, 7);
        Token  tok_veacc         = proto_tokenizer_get(tzer, 8);
        Token  tok_vuacc         = proto_tokenizer_get(tzer, 9);
        Token  tok_headacc       = proto_tokenizer_get(tzer, 10);

        r->pmtkvned.clocktime    = str2int(tok_clktime.p, tok_clktime.end);
        r->pmtkvned.velocity_n   = str2float(tok_vn.p, tok_vn.end);
        r->pmtkvned.velocity_e   = str2float(tok_ve.p, tok_ve.end);
        r->pmtkvned.velocity_d   = (-1) * str2float(tok_vu.p, tok_vu.end);//transform
        r->pmtkvned.hor_speed    = str2float(tok_horspeed.p, tok_horspeed.end);
        r->pmtkvned.speed        = str2float(tok_speed.p, tok_speed.end);
        r->pmtkvned.vn_acc       = str2float(tok_vnacc.p, tok_vnacc.end);
        r->pmtkvned.ve_acc       = str2float(tok_veacc.p, tok_veacc.end);
        r->pmtkvned.vd_acc       = str2float(tok_vuacc.p, tok_vuacc.end);//waiting for verify whether need to add -
        r->pmtkvned.heading_acc  = str2float(tok_headacc.p, tok_headacc.end);

        r->flags |= FRAME_TYPE_UPDATE_VNED;
     }else if (!memcmp(tok.p, "TK840", 5)) {
        Token  tok_epx       = proto_tokenizer_get(tzer, 1);
        Token  tok_epy            = proto_tokenizer_get(tzer, 2);
        Token  tok_epv            = proto_tokenizer_get(tzer, 3);
        Token  tok_climb            = proto_tokenizer_get(tzer, 4);
        Token  tok_epd         = proto_tokenizer_get(tzer, 5);
        Token  tok_eps         = proto_tokenizer_get(tzer, 6);
        Token  tok_epdc         = proto_tokenizer_get(tzer, 7);

        r->pmtk840.epx   = str2float(tok_epx.p, tok_epx.end);
        r->pmtk840.epy   = str2float(tok_epy.p, tok_epy.end);
        r->pmtk840.epv   = str2float(tok_epv.p, tok_epv.end);
        r->pmtk840.climb = str2float(tok_climb.p, tok_climb.end);//transform
        r->pmtk840.epd   = str2float(tok_epd.p, tok_epd.end);
        r->pmtk840.eps   = str2float(tok_eps.p, tok_eps.end);
        r->pmtk840.epc   = str2float(tok_epdc.p, tok_epdc.end);

        r->flags |= FRAME_TYPE_UPDATE_EP;
    }else {
        tok.p -= 2;
        #if NEMA_DEBUG //yatish
        LOG_INFO("unknown sentence '%.*s", (int)(tok.end-tok.p), tok.p);
        #endif
    }

    if (r->frame_flag & FRAME_END)
    {
        for (index = 0; index < r->pmtkchl.gnss_chl.num_sv; index++)
        {
            int prn = r->pmtkchl.gnss_chl.sv_chl[index].sateId;
            r->pmtkchl.gnss_chl.sv_chl[index].used_in_fix =
                r->nmeagsa.gps_sv_used_in_fix[prn].used_in_fix;
        }
    }
}

int cache_nmea_sentence(gnss_module_data* const r)
{
    gnss_nmea_data *nmea;
    nmea = &(r->sv_out.nmea);
    int len = nmea->len + r->pos;

    if(len >= GNSS_NMEA_SENTENCE_SIZE){
        LOG_ERROR("the frame has too many sentences, the cache buffer is not enough");
        return 0;
    }

    int32_t copy_len = r->pos;
    memcpy(&nmea->sentence[nmea->len], r->in, copy_len);
    nmea->len = len;
    return 1;
}

static void nmea_reader_addc(gnss_module_data* const r, int  c)
{
    if (r->overflow)
    {
        r->overflow = (c != '\n');
        return;
    }

    if (r->pos >= (int) sizeof(r->in)-1)
    {
        r->overflow = 1;
        r->pos      = 0;
        return;
    }

    r->in[r->pos] = (char)c;
    r->pos += 1;

    if (c == '\n')
    {
        if(cache_nmea_sentence(r)){
           nmea_reader_parse(r);
           LOG_DEBUG("nmea sentence: %.*s", (int)r->pos, r->in);
           r->pos = 0;
       } else {
         LOG_ERROR("there is not enough buffer,nmea sentence cache failed");
         abort();
     }
    }
}

void gnss_parser_postfunc(gnss_module_data* r)
{
    int32_t idx = 0, idx2 = 0;
    for (idx = 0; idx < r->pmtkchl.gnss_chl.num_sv; idx++)
    {
        for (idx2 = 0; idx2 < r->nmeagsv.sv_status.num_svs; idx2++){
            if (r->pmtkchl.gnss_chl.sv_chl[idx].sateId ==
                    r->nmeagsv.sv_status.sv_list[idx2].svid) {
                r->sv_out.satedatas[r->sv_out.nsv].prn = r->pmtkchl.gnss_chl.sv_chl[idx].sateId;
                r->sv_out.satedatas[r->sv_out.nsv].ps_range = r->pmtkchl.gnss_chl.sv_chl[idx].psedorange;
                r->sv_out.satedatas[r->sv_out.nsv].carrier_phase = r->pmtkchl.gnss_chl.sv_chl[idx].sync_carr_phase;
                r->sv_out.satedatas[r->sv_out.nsv].cycle_slip = r->pmtkchl.gnss_chl.sv_chl[idx].cycle_slip;
                r->sv_out.satedatas[r->sv_out.nsv].doppler = r->pmtkchl.gnss_chl.sv_chl[idx].doppler;
                r->sv_out.satedatas[r->sv_out.nsv].cn0 = r->pmtkchl.gnss_chl.sv_chl[idx].snr;
                r->sv_out.satedatas[r->sv_out.nsv].iono = r->pmtkchl.gnss_chl.sv_chl[idx].iono_corr;
                r->sv_out.satedatas[r->sv_out.nsv].fixed_flag = r->pmtkchl.gnss_chl.sv_chl[idx].used_in_fix;
                r->sv_out.satedatas[r->sv_out.nsv].elevation = r->nmeagsv.sv_status.sv_list[idx2].elevation;
                r->sv_out.satedatas[r->sv_out.nsv].azimuth = r->nmeagsv.sv_status.sv_list[idx2].azimuth;
                r->sv_out.nsv++;
            }
        }
    }
}

void gnss_parser_prefunc(gnss_module_data* p,
                              const char *buffer, uint32_t length)
{
    uint32_t  nn;
    gnss_module_data* reader = p;

    reader->pos = 0;
    reader->overflow = 0;

    for (nn = 0; nn < length; nn++)
    {
        nmea_reader_addc(reader, buffer[nn]);
    }
}

/*
Function:gnss_command_socket_open
Description:open and connect a INET socket by given port number
Param:[IN] port, the port number of socket
Param:[OUT] fd, the socket fd
Return:NULL -- some thing is incorrect; Other value -- open and connect sokcet successfully
*/
int gnss_command_socket_open(int port)
{
    struct sockaddr_in servaddr;
    int socketfd = -1;

    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_ERROR("Create socket error:%d,%s\n", errno, strerror(errno));
        return socketfd;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if(connect(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0 )
    {
        LOG_ERROR("connect error:%d,%s\n", errno, strerror(errno));
        close(socketfd);
        socketfd = -1;
    }

    return socketfd;
}

int send_buf_to_libmnl(int *sockfd, char *buf, int len)
{
    int ret = 0;
    int cur_pos = 0;

    LOG_INFO("start %s %d",buf, len);
    if (*sockfd != -1) {
        while(cur_pos < len) {
            ret = send(*sockfd, &buf[cur_pos], len - cur_pos, 0);
            if (ret == len - cur_pos)
                break;

            if (ret <= 0) {
                LOG_ERROR("SOCKET ERROR errno:%d", errno);
                if (errno == EAGAIN || errno == EINTR) {
                    usleep(100000);
                    LOG_ERROR("buffer full");
                    continue;
                }
                if (errno == ECONNRESET || errno == EPIPE ){
                    //*sockfd = -1;
                    LOG_ERROR("buffer client connect is reset");
                }
                break;
            }
            else
                cur_pos += ret;

            LOG_INFO("cur_pos/len[%d %d]\n", cur_pos, len);
        }
    }else{
        LOG_ERROR("sockfd is wrong:%d", *sockfd);
    }
    LOG_INFO("finish tranfer & ret is %d", ret);
    return ret;
}

