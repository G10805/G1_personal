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
#include "module.h"
#include "module_sensor_helper.h"
#include "module_gnss_helper.h"
#include "config-parser.h"
#include "adr_release_type.h"
#include "log.h"

extern struct adr_config *config;

typedef struct {
    /*for muli-sensor*/
    int32_t count_sensors;//record how many set of sensor platform support
}STORE_CONFIG_PARAM;

typedef struct store_device {
    struct module *mod;
    void (*upload)(module *, module_box *);

    pthread_mutex_t algo_mutex;
    pthread_mutex_t gnss_mutex;
    pthread_mutex_t nmea_raw_mutex;
    pthread_mutex_t sensor_mutex;

    FILE *store_algo_fp;
    FILE *store_gnss_fp;
    FILE *store_nmea_raw_fp;
    FILE **store_sensor_fp;
    STORE_CONFIG_PARAM store_config;
} store_device;


static int store_load_config_file(STORE_CONFIG_PARAM *store_config)
{
    struct adr_config_section *section;

    section = adr_config_get_section(config, "sensor", NULL, NULL);

    if (section) {
        adr_config_section_get_int32_t(section, "count_sensors",
                    &store_config->count_sensors, 1);
    }
    else {
        LOG_ERROR("store get sensor section failed");
        return -1;
    }
    return 0;
}

static int adr_data_store(store_device *dev, module_box *box)
{
    adr_pvt_msg_struct *adr_pvt = (adr_pvt_msg_struct *)(box->data);
    int ret;
    //char buf[128] = {0};

    ret = pthread_mutex_lock(&dev->algo_mutex);
    if(ret < 0){
        LOG_ERROR("lock algo_mutex error[%d]:%s", ret, strerror(errno));
        return -1;
    }

    if (!adr_pvt || !dev->store_algo_fp){
        pthread_mutex_unlock(&dev->algo_mutex);
        return -1;
    }

    /*snprintf(buf, 128, "%"PRIu64" %03d %0.5f %0.5f %0.2f %0.2f %0.2f %0.3f %d %d %d %d %d\n",
             (uint64_t)adr_pvt->ts_sec, adr_pvt->ts_millisec, adr_pvt->lat, adr_pvt->lon,
             adr_pvt->alt, adr_pvt->velocity, adr_pvt->odom, adr_pvt->heading,
             adr_pvt->fix_state, adr_pvt->pos_init_state, adr_pvt->vel_init_state,
             adr_pvt->heading_init_state, adr_pvt->mu_state);*/
    ret = fprintf(dev->store_algo_fp, "%.0lf %03d %0.5lf %0.5lf %0.2f %0.2lf %0.2lf %0.3lf %d %d %d %d %d\n",
                 adr_pvt->ts_sec, adr_pvt->ts_millisec, adr_pvt->lat, adr_pvt->lon,
                 adr_pvt->alt, adr_pvt->velocity, adr_pvt->odom, adr_pvt->heading,
                 adr_pvt->fix_state, adr_pvt->pos_init_state, adr_pvt->vel_init_state,
                 adr_pvt->heading_init_state, adr_pvt->mu_state);

    //ret = fprintf(dev->store_algo_fp,"%s", buf);
    if (ret < 0)
    {
        goto write_error;
    }

    pthread_mutex_unlock(&dev->algo_mutex);
    return 0;

write_error:
    LOG_ERROR("fprintf adr data error: %s", strerror(errno));
    pthread_mutex_unlock(&dev->algo_mutex);
    return -1;
}

/*the function should be called circularly by app*/
static int raw_data_store(store_device *dev, module_box *box)
{
    int64_t monotonic_raw = 0;
    int time_ms = 0;
    int ret, cnt;
    gnss_module_data *r = (gnss_module_data *)box->data;

    ret = pthread_mutex_lock(&dev->nmea_raw_mutex);
    if(ret < 0){
        LOG_ERROR("lock nmea_raw_mutex error[%d]:%s", ret, strerror(errno));
        return -1;
    }

    if (!r || !dev->store_nmea_raw_fp || r->sv_out.nsv < 0){
        LOG_ERROR("data or fp is NULL, or sv_out.nsv <= 0");
        pthread_mutex_unlock(&dev->nmea_raw_mutex);
        return -1;
    }

    if (r->sv_out.nsv > GPS_MAX_RAW){
        LOG_ERROR("sv_out.nsv[%d] > GPS_MAX_RAW[%d]", r->sv_out.nsv, GPS_MAX_RAW);
        pthread_mutex_unlock(&dev->nmea_raw_mutex);
        return -1;
    }
    //time_ymdhms = (int64_t)(r->nmearmc.utc_data_time);
    time_ms = r->nmearmc.utc_time_msec;
    monotonic_raw = (int64_t)r->drvtime.tv_sec * nsec_2_sec + r->drvtime.tv_nsec;

    ret = fprintf(dev->store_nmea_raw_fp,"%.0lf %03d %"PRId64" %d", r->nmearmc.utc_data_time,
                time_ms, monotonic_raw, r->sv_out.nsv);

    if (ret < 0){
        goto write_error;
    }

     for (cnt = 0; cnt < r->sv_out.nsv; cnt++) {
        ret = fprintf(dev->store_nmea_raw_fp," %d %0.2lf %0.1f %d %0.1f %d %d %d %d %d",
                  r->sv_out.satedatas[cnt].prn, r->sv_out.satedatas[cnt].ps_range,
                  r->sv_out.satedatas[cnt].carrier_phase, r->sv_out.satedatas[cnt].cycle_slip,
                  r->sv_out.satedatas[cnt].doppler, r->sv_out.satedatas[cnt].cn0,
                  r->sv_out.satedatas[cnt].elevation, r->sv_out.satedatas[cnt].azimuth,
                  r->sv_out.satedatas[cnt].iono, r->sv_out.satedatas[cnt].fixed_flag);
        if (ret < 0){
            goto write_error;
        }
    }

    ret = fprintf(dev->store_nmea_raw_fp,"\n");
    if (ret < 0){
        goto write_error;
    }

    pthread_mutex_unlock(&dev->nmea_raw_mutex);
    return 0;

write_error:
    LOG_ERROR("fprintf raw data error: %s", strerror(errno));
    pthread_mutex_unlock(&dev->nmea_raw_mutex);
    return -1;
}

static int gnss_data_store(store_device *dev, module_box *box)
{
    int64_t monotonic_raw = 0;
    int ret;
    //char buf[256] = {0};
    gnss_module_data *r = (gnss_module_data *)box->data;

    ret = pthread_mutex_lock(&dev->gnss_mutex);
    if(ret < 0){
        LOG_ERROR("lock gnss_mutex error[%d]:%s", ret, strerror(errno));
        return -1;
    }

    if (!r || !dev->store_gnss_fp){
        LOG_ERROR("data or fp is NULL");
        pthread_mutex_unlock(&dev->gnss_mutex);
        return -1;
    }
    //time_ymdhms = (int64_t)(r->nmearmc.utc_data_time);
    monotonic_raw = (int64_t)r->drvtime.tv_sec * nsec_2_sec + r->drvtime.tv_nsec;

    /*snprintf(buf, 256, "%"PRId64" %03d %"PRId64" %0.7f %0.7f %0.2f %0.2f %0.2f %0.2f %0.3f %d %0.2f "
             "%0.2f %0.2f %0.2f %0.2f %0.2f %d %0.2f %0.2f %0.2f %0.2lf %0.2f %0.2f\n",
             time_ymdhms, r->nmearmc.utc_time_msec, monotonic_raw, r->nmeagga.lat, r->nmeagga.lng,
             r->nmeagga.alt, r->pmtkvned.velocity_n, r->pmtkvned.velocity_e,
             r->pmtkvned.velocity_d, r->nmearmc.bearing, r->nmeagsa.fix_mode, r->nmeaepe.epe_2d,
             r->nmeaepe.epe_vert, r->pmtkvned.vn_acc, r->pmtkvned.ve_acc, r->pmtkvned.vd_acc,
             r->pmtkvned.heading_acc, r->nmeagsa.sate_num, r->nmeagsa.pdop, r->nmeagsa.hdop,
             r->nmeagsa.vdop, r->sensor.baro, r->sensor.baroT, r->sensor.odo);*/
    ret = fprintf(dev->store_gnss_fp, "%.0lf %03d %"PRId64" %0.7lf %0.7lf %0.2lf %0.2lf %0.2lf %0.2lf %0.3lf %d %0.2lf "
                 "%0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %d %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf\n",
                 r->nmearmc.utc_data_time, r->nmearmc.utc_time_msec, monotonic_raw, r->nmeagga.lat, r->nmeagga.lng,
                 r->nmeagga.alt, r->pmtkvned.velocity_n, r->pmtkvned.velocity_e,
                 r->pmtkvned.velocity_d, r->nmearmc.bearing, r->nmeagsa.fix_mode, r->nmeaepe.epe_2d,
                 r->nmeaepe.epe_vert, r->pmtkvned.vn_acc, r->pmtkvned.ve_acc, r->pmtkvned.vd_acc,
                 r->pmtkvned.heading_acc, r->nmeagsa.sate_num, r->nmeagsa.pdop, r->nmeagsa.hdop,
                 r->nmeagsa.vdop, r->sensor.baro, r->sensor.baroT, r->sensor.odo);

    //ret = fprintf(dev->store_gnss_fp,"%s",buf);
    if (ret < 0)
    {
        goto write_error;
    }

    pthread_mutex_unlock(&dev->gnss_mutex);
    return 0;

write_error:
    LOG_ERROR("fprintf gnss data error: %s", strerror(errno));
    pthread_mutex_unlock(&dev->gnss_mutex);
    return -1;
}

static int mems_data_store(store_device *dev, module_box *box)
{
    adr_mems_msg_struct *pMems = (adr_mems_msg_struct *)box->data;
    int32_t dev_id = box->index;
    int ret, i;

    ret = pthread_mutex_lock(&dev->sensor_mutex);
    if (ret < 0){
        LOG_ERROR("lock sensor_mutex error[%d]:%s", ret, strerror(errno));
        return -1;
    }

    if (!pMems || !dev->store_sensor_fp[dev_id]){
        LOG_ERROR("data or sensor[%d] fp is null", dev_id);
        pthread_mutex_unlock(&dev ->sensor_mutex);
        return -1;
    }

    for(i = 0; i < box->count; i++){
        ret = fprintf(dev->store_sensor_fp[dev_id],
                "%.0lf %03d %"PRId64" %0.6lf %0.6lf %0.6lf %0.2lf %0.6lf %0.6lf %0.6lf %0.2lf "
                "%0.2lf %d %0.1f\n", pMems[i].ts_sec, pMems[i].ts_millisec, pMems[i].monotonic_raw, pMems[i].acc.x,
                pMems[i].acc.y, pMems[i].acc.z, pMems[i].acc.t, pMems[i].gyro.x, pMems[i].gyro.y,
                pMems[i].gyro.z, pMems[i].gyro.t, pMems[i].can1, pMems[i].can2, pMems[i].can3);

        if (ret < 0)
        {
            goto write_error;
        }
    }
    pthread_mutex_unlock(&dev->sensor_mutex);
    return 0;

write_error:
    LOG_ERROR("fprintf mems data error: %s", strerror(errno));
    pthread_mutex_unlock(&dev->sensor_mutex);
    return -1;
}

static void store_driver_download(struct module *mod, module_box *box)
{
    struct store_device *dev;
    dev = (store_device*)module_get_driver_data(mod);

    LOG_DEBUG("Recieve data from:%s, data type id %d", box->from, box->type);

    switch(box->type) {
        case DATA_GNSS_STORE:
            raw_data_store(dev, box);
            gnss_data_store(dev, box);
            break;
        case DATA_SENSOR_STORE:
            mems_data_store(dev, box);
            break;
        case DATA_ADR_ALGO_PVT:
            adr_data_store(dev, box);
            break;
        default :
            LOG_ERROR("can not process this type");
    }
}

static int32_t store_driver_init(struct module *mod)
{
    int32_t ret, i, j;
    struct store_device *dev =
        (store_device*)module_get_driver_data(mod);
    char filename[PATH_LEN]= {'\0'};

    ret = store_load_config_file(&dev->store_config);
    if (ret != 0) {
        LOG_ERROR("something wrong in load store config file");
        return -1;
    }

    dev->store_sensor_fp = (FILE **)zalloc(dev->store_config.count_sensors * sizeof(FILE *));

    for(i = 0; i < dev->store_config.count_sensors; i++) {
        snprintf(filename, sizeof filename, "MEMS%d.LOG", i);
        if(open_log_file(filename, &dev->store_sensor_fp[i]) != 0) {
                LOG_ERROR("open log file:%s failed", filename);
                for (j = 0; j < i; j++) {
                    close_log_file(dev->store_sensor_fp[j]);
                }
                free(dev->store_sensor_fp);
                return -1;
        }
    }

    snprintf(filename, sizeof filename, "RAW.LOG");
    if(open_log_file(filename,&dev->store_nmea_raw_fp) != 0) {
        LOG_ERROR("open log file:%s failed", filename);
        for(i = 0; i < dev->store_config.count_sensors; i++) {
            close_log_file(dev->store_sensor_fp[i]);
        }
        free(dev->store_sensor_fp);
        return -1;
    }

    snprintf(filename, sizeof filename, "GNSS.LOG");
    if(open_log_file(filename,&dev->store_gnss_fp) != 0) {
        LOG_ERROR("open log file:%s failed", filename);
        for(i = 0; i < dev->store_config.count_sensors; i++) {
            close_log_file(dev->store_sensor_fp[i]);
        }
        free(dev->store_sensor_fp);
        close_log_file(dev->store_nmea_raw_fp);
        return -1;
    }

    snprintf(filename, sizeof filename, "ALGO.LOG");
    if(open_log_file(filename,&dev->store_algo_fp) != 0) {
        LOG_ERROR("open log file:%s failed", filename);
        for(i = 0; i < dev->store_config.count_sensors; i++) {
            close_log_file(dev->store_sensor_fp[i]);
        }
        free(dev->store_sensor_fp);
        close_log_file(dev->store_nmea_raw_fp);
        close_log_file(dev->store_gnss_fp);
        return -1;
    }

    pthread_mutex_init(&dev->algo_mutex, NULL);
    pthread_mutex_init(&dev->gnss_mutex, NULL);
    pthread_mutex_init(&dev->nmea_raw_mutex, NULL);
    pthread_mutex_init(&dev->sensor_mutex, NULL);

    return 0;
}

static void store_driver_deinit(struct module *mod)
{
    struct store_device *dev;
    dev = (store_device*)module_get_driver_data(mod);

    pthread_mutex_lock(&dev->sensor_mutex);
    for(int i = 0; i < dev->store_config.count_sensors; i++) {
        close_log_file(dev->store_sensor_fp[i]);
    }
    pthread_mutex_unlock(&dev->sensor_mutex);
    pthread_mutex_destroy(&dev->sensor_mutex);
    LOG_INFO("close sensor fp ok");

    pthread_mutex_lock(&dev->gnss_mutex);
    close_log_file(dev->store_gnss_fp);
    pthread_mutex_unlock(&dev->gnss_mutex);
    pthread_mutex_destroy(&dev->gnss_mutex);
    LOG_INFO("close gnss fp ok");

    pthread_mutex_lock(&dev->nmea_raw_mutex);
    close_log_file(dev->store_nmea_raw_fp);
    pthread_mutex_unlock(&dev->nmea_raw_mutex);
    pthread_mutex_destroy(&dev->nmea_raw_mutex);
    LOG_INFO("close nmea fp ok");

    pthread_mutex_lock(&dev->algo_mutex);
    close_log_file(dev->store_algo_fp);
    pthread_mutex_unlock(&dev->algo_mutex);
    pthread_mutex_destroy(&dev->algo_mutex);
    LOG_INFO("close algo fp ok");

    module *store_mod = dev->mod;
    memset(dev, 0, sizeof(store_device));
    dev->mod = store_mod;

    LOG_INFO("deinit data store driver ok");
}

static void store_driver_registry(struct module *mod, process_data callback)
{
    struct store_device *dev;
    dev = (store_device*)module_get_driver_data(mod);

    dev->upload = callback;
}

static module_driver store_driver = {
    .name = "mtk_data_store",
    .capability = DATA_GNSS_STORE | DATA_SENSOR_STORE | DATA_ADR_ALGO_PVT,
    .init = store_driver_init,
    .download = store_driver_download,
    .deinit = store_driver_deinit,
    .registry = store_driver_registry,
};

void module_store_deinit(module *mod)
{
    struct store_device *dev;
    dev = (store_device*)module_get_driver_data(mod);

    free(dev);
    module_register_driver(mod, NULL, NULL);
}

int module_store_init(module *mod)
{
    struct store_device *dev;

    dev = (store_device*)zalloc(sizeof(store_device));
    if (!dev) {
        LOG_ERROR("Fail to create sanity_device");
        return -1;
    }
    dev->mod = mod;
    module_set_name(mod, store_driver.name);
    module_register_driver(mod, &store_driver, (void *)dev);
    return 0;
}

