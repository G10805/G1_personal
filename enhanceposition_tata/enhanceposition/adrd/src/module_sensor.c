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

//sensor driver
#include <time.h>
#include <sched.h>

#include "module.h"
#include "adr_data_struct.h"
#include "module_sensor_helper.h"
#include "adr_data_convert.h"
#include "config-parser.h"
#include <sensors_io.h>

#if ADR_SUPPORT_SENSOR_V2
//#include <cutils/log.h>
#include <HfManagerWrapper.h>
#endif

extern struct adr_config *config;

double baro_d_last;
double baro_t_last;
double acc_t_last;
double gyro_t_last;

#define ACC_POLL_NS_DEFAULT 20000000LL
#define GYRO_POLL_NS_DEFAULT 20000000LL
#define BARO_POLL_NS_DEFAULT 20000000LL
#define OPEN_FAKE_ODOM_DEGAULT 0

typedef struct {
    char *sensor_lib_name;
    char *acc_gyro_dev;
    char *baro_dev;
    int64_t acc_poll_ns;
    int64_t gyro_poll_ns;
    int64_t baro_poll_ns;

    /*for muli-sensor*/
    int32_t main_dev_id;//sensor with main_dev_id will transfer its data to adr algo
    int32_t count_sensors;//record how many set of sensor platform support

    int32_t acc_gyro_debug_mode;
    int32_t baro_debug_mode;

    int32_t open_baro;//whether support baro
    int32_t open_fake_odom;
}SENSOR_CONFIG_PARAM;

typedef struct sensor_baro_stat_msg {
    sensor_iic_data data;
    struct module_list link;
    int32_t busy;
} sensor_baro_stat_msg;

typedef struct sensor_mems_stat_msg {
    adr_mems_msg_struct data;
    struct module_list link;
    int32_t busy;
    int32_t dev_id;
} sensor_mems_stat_msg;

typedef struct sensor_device {
    char *name;
    struct module *mod;
    void (*upload)(struct module *, module_box *);
    void *sensor_hal;

    int baro_fd;
    pthread_t baro_io_thread;
    int baro_io_thread_exit;
    pthread_t baro_dataready_thread;
    int baro_dataready_thread_exit;
    pthread_mutex_t baro_dataready_mutex;
    pthread_cond_t baro_dataready_cond;
    sensor_baro_stat_msg *baro_stat_msg[FIFO_DATA_CHUNK_NUM];
    struct module_list baro_ready_list;
    pthread_mutex_t baro_ready_list_mutex;

    int acc_gyro_fd;
    pthread_t acc_gyro_io_thread;
    int acc_gyro_io_thread_exit;
    pthread_t mems_dataready_thread;
    int mems_dataready_thread_exit;
    pthread_mutex_t mems_dataready_mutex;
    pthread_cond_t mems_dataready_cond;
    sensor_mems_stat_msg *mems_stat_msg[FIFO_DATA_CHUNK_NUM];
    struct module_list mems_ready_list;
    pthread_mutex_t mems_ready_list_mutex;

    sensor_mems_stat_msg *mems[FIFO_DATA_CHUNK_NUM];   //sensor data to adr(cache data)
    sensor_baro_stat_msg *baro[FIFO_DATA_CHUNK_NUM];   //sensor data to gnss(cache data)

    struct timespec spec;
    pthread_mutex_t spec_mutex;

    float odom;
    float rudder;
    int gear;
    struct timespec odom_boottime;
    pthread_mutex_t odom_mutex;

    SENSOR_CONFIG_PARAM sensor_config;
#if ADR_SUPPORT_SENSOR_V2
    void *HfManager;
    void *HfLooper;
#endif
} sensor_device;

static sensor_device *g_sensor_ctx = NULL;

static sensor_baro_stat_msg *get_next_baro_data_stat_msg(sensor_device *dev)
{
    int32_t cnt = 0;

    while(cnt < FIFO_DATA_CHUNK_NUM) {
        if (!dev->baro[cnt]) {
            dev->baro[cnt] =
                (sensor_baro_stat_msg *)zalloc(sizeof(sensor_baro_stat_msg));
            module_list_init(&dev->baro[cnt]->link);
            return dev->baro[cnt];
        } else {
            if (dev->baro[cnt]->busy == 0)
                 return dev->baro[cnt];
        }
        cnt += 1;
    }
    return NULL;
}

static void free_baro_data_stat_msg(sensor_device *dev)
{
    int32_t cnt = 0;

    while(cnt < FIFO_DATA_CHUNK_NUM) {
        if (dev->baro[cnt]) {
            free(dev->baro[cnt]);
            dev->baro[cnt] = NULL;
        }
        cnt += 1;
    }
}

static sensor_mems_stat_msg *get_next_mems_data_stat_msg(sensor_device *dev)
{
    int32_t cnt = 0;

    while(cnt < FIFO_DATA_CHUNK_NUM) {
        if (!dev->mems[cnt]) {
            dev->mems[cnt] =
                (sensor_mems_stat_msg *)zalloc(sizeof(sensor_mems_stat_msg));
            module_list_init(&dev->mems[cnt]->link);
            return dev->mems[cnt];
        } else {
            if (dev->mems[cnt]->busy == 0)
                 return dev->mems[cnt];
        }
        cnt += 1;
    }
    return NULL;
}

static void free_mems_data_stat_msg(sensor_device *dev)
{
    int32_t cnt = 0;

    while(cnt < FIFO_DATA_CHUNK_NUM) {
        if (dev->mems[cnt]) {
            free(dev->mems[cnt]);
            dev->mems[cnt] = NULL;
        }
        cnt += 1;
    }
}

static void sensor_align_timer(sensor_device *dev, struct timespec *spec)
{
    pthread_mutex_lock(&dev->spec_mutex);
    memcpy(&dev->spec, spec, sizeof(*spec));
    LOG_INFO("difftime [s ns][%ld, %ld]", spec->tv_sec, spec->tv_nsec);
    pthread_mutex_unlock(&dev->spec_mutex);
}

static void sensor_align_odom(sensor_device *dev, odom_uart_data *data)
{
    pthread_mutex_lock(&dev->odom_mutex);
    dev->odom = data->odom;
    dev->rudder = data->rudder;
    dev->gear = data->gear;
    dev->odom_boottime = data->odom_boottime;
    pthread_mutex_unlock(&dev->odom_mutex);
}

static int sensor_acc_gyro_deinit(sensor_device *dev)
{
    if(dev->acc_gyro_fd >= 0){
        close(dev->acc_gyro_fd);
        dev->acc_gyro_fd = -1;
    }else{
        LOG_ERROR("something wrong happened to acc gyro fd");
        return -1;
    }

    return 0;
}

static int sensor_baro_deinit(sensor_device *dev)
{
    if(dev->baro_fd >= 0){
        close(dev->baro_fd);
        dev->baro_fd = -1;
    }else{
        LOG_ERROR("something wrong happened to baro fd");
        return -1;
    }

    return 0;
}

static int sensor_acc_gyro_stop(sensor_device *dev)
{
    int *fd = &dev->acc_gyro_fd, cmd, enable;
#if ADR_SUPPORT_SENSOR_V2
    int err = 0;
#endif

    dev->acc_gyro_io_thread_exit = 1;
    dev->mems_dataready_thread_exit = 1;
    pthread_cond_broadcast(&dev->mems_dataready_cond);
    pthread_join(dev->acc_gyro_io_thread, NULL);
    LOG_INFO("acc_gyro_io_thread exit ok~");
    pthread_join(dev->mems_dataready_thread, NULL);
    LOG_INFO("mems_dataready_thread exit ok~");

    /*********** disable sensor    ***********/
#if ADR_SUPPORT_SENSOR_V2
    if (HfManagerFindSensor(dev->HfManager, SENSOR_TYPE_ACCELEROMETER) < 0 ||
        HfManagerFindSensor(dev->HfManager, SENSOR_TYPE_GYROSCOPE) < 0 ) {
        LOG_ERROR("can't find sensor\n");
        err = -1;
        goto find_sensor_fail;
    }

    HfManagerDisableSensor(dev->HfManager, SENSOR_TYPE_ACCELEROMETER);
    HfManagerDisableSensor(dev->HfManager, SENSOR_TYPE_GYROSCOPE);
    LOG_INFO("disable acc gyro ok~");

find_sensor_fail:
    HfLooperDestroy(dev->HfLooper);
    HfManagerDestroy(dev->HfManager);
    return err;
#else
    cmd = ACC_GYRO_IOCTL_SET_ENABLE;
    enable = 0; // 1 is enable, 0 is disable
    if (ioctl(*fd, cmd, &enable) < 0){
        LOG_ERROR("disable acc gyro failed, %s", dev->sensor_config.acc_gyro_dev);
        return -1;
    }
    LOG_INFO("disable acc gyro ok~");

    return 0;
#endif
}

static int sensor_baro_stop(sensor_device *dev)
{
    int *fd = &dev->baro_fd, cmd, enable;

    dev->baro_io_thread_exit = 1;
    dev->baro_dataready_thread_exit = 1;
    pthread_cond_broadcast(&dev->baro_dataready_cond);
    pthread_join(dev->baro_io_thread, NULL);
    LOG_INFO("baro_io_thread exit ok~");
    pthread_join(dev->baro_dataready_thread, NULL);
    LOG_INFO("baro_dataready_thread exit ok~");

    /*********** enable sensor    ***********/
    cmd =BARO_TEMP_IOCTL_SET_ENABLE;
    enable = 0; // 1 is enable, 0 is disable
    if (ioctl(*fd, cmd, &enable) < 0){
        LOG_ERROR("disable baro failed, %s", dev->sensor_config.baro_dev);
        return -1;
    }
    LOG_INFO("disable baro ok~");

    return 0;
}

//just cache baro data
void  sensor_baro_prefunc(sensor_device *dev,
              const struct hwm_sensor_data* buffer, int count)
{
    int i = 0;
    sensor_baro_stat_msg *stat_msg;
    sensor_iic_data *data;

    for (i = 0; i < count; i++) {
        stat_msg = get_next_baro_data_stat_msg(dev);
        if(!stat_msg){
            LOG_WARN("baro data ready thread process too slow");
            pthread_mutex_unlock(&dev->baro_dataready_mutex);
            sleep(1);
            break;
        }
        pthread_mutex_lock(&dev->baro_dataready_mutex);
        stat_msg->busy = 1;
        pthread_mutex_unlock(&dev->baro_dataready_mutex);

        if(buffer[i].value_divide == 0){
            LOG_ERROR("[baro_io_t_%d] baro:%d, t:%d, divide %d", i,buffer[i].values[0], buffer[i].values[1], buffer[i].value_divide);
            abort();
        }

        data = &stat_msg->data;
        data->baro = (double)buffer[i].values[0] / buffer[i].value_divide;
        data->baroT = (double)buffer[i].values[1] / 10000;
        data->baro_boottime.tv_sec = buffer[i].time / nsec_2_sec;
        data->baro_boottime.tv_nsec = buffer[i].time % nsec_2_sec;

        pthread_mutex_lock(&dev->baro_ready_list_mutex);
        module_list_insert(&dev->baro_ready_list, &stat_msg->link);
        pthread_mutex_unlock(&dev->baro_ready_list_mutex);
    }
}

static int get_mems_utc_time(struct timespec timediff, adr_mems_msg_struct *sensordata,
    int64_t gyro_ns)
{
    double *ts_sec;
    int *ts_msec, ms;
    time_t sensor_time_sec, tt_sec, timediff_sec = 0;
    long sensor_time_nsec, nsec, timediff_nsec = 00;
    struct tm *tm, tm_res;
    long long timetmp;
    char dtime[30] = {0};

    ts_sec = &sensordata->ts_sec;
    ts_msec = &sensordata->ts_millisec;
    sensor_time_sec = gyro_ns/nsec_2_sec;
    sensor_time_nsec = gyro_ns%nsec_2_sec;

    //if use GNSS to get UTC time
    timediff_sec = timediff.tv_sec;
    timediff_nsec = timediff.tv_nsec;

    nsec = (sensor_time_nsec + timediff_nsec);
    if (nsec >= nsec_2_sec)
    {
        nsec -= nsec_2_sec;
        timediff_sec += 1;
    }
    tt_sec = timediff_sec + sensor_time_sec;
    ms = (nsec + nsec_2_msec / 2)/nsec_2_msec;

    if(ms >= 1000){
        tt_sec += 1;
        ms -= 1000;
    }

    tm = gmtime_r(&tt_sec, &tm_res);
    sprintf(dtime, "%04d%02d%02d%02d%02d%02d\n",
            tm->tm_year + 1900, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min,
            tm->tm_sec);
    timetmp = atoll(dtime);

    /*static long long last_sec = 0;
    static int last_sensor_time_nsec = 0, last_timediff_nsec = 0, last_msec = 0;
    static time_t last_sensor_time_sec = 0,last_timediff_sec = 0;

    if((timetmp < last_sec) || ((timetmp == last_sec && ms <= last_msec))){
        LOG_ERROR("tm :%04d%02d%02d%02d%02d%02d %p\n",
                    tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min,
                    tm->tm_sec, tm);
        LOG_ERROR("the dev[%ld sec, %ld ns]\n",dev->spec.tv_sec, dev->spec.tv_nsec);
        LOG_ERROR("the new diff time [%ld sec, %d ns],the last diff time [%ld sec, %d ns]\n",
                      timediff_sec, timediff_nsec,last_timediff_sec, last_timediff_nsec);
        LOG_ERROR("the new gyro time [%ld sec, %d ns],the last gyro time [%ld sec, %d ns]\n",
        sensor_time_sec, sensor_time_nsec,last_sensor_time_sec, last_sensor_time_nsec);
        LOG_ERROR("dtime %s \n", dtime);

         LOG_ERROR("the new time [%lld sec, %d ms] <= the last [%lld sec, %d ms]\n",
                         timetmp, ms, last_sec, last_msec);*/
        /*abort();*/
    /*}*/

    *ts_sec = timetmp;
    *ts_msec = ms;
    LOG_DEBUG("sensor UTC time[s ms][%lf, %d]", *ts_sec, *ts_msec);

    /*last_sec = timetmp;
    last_msec = ms;
    last_sensor_time_sec = sensor_time_sec;
    last_sensor_time_nsec = sensor_time_nsec;
    last_timediff_sec = timediff_sec;
    last_timediff_nsec = timediff_nsec;*/

    return 0;
}

//just cache acc gyro data
void  sensor_acc_gyro_prefunc(sensor_device *dev,
              const struct ACC_GYRO_DATA* buffer, int count)
{
    int  i = 0;
    sensor_mems_stat_msg *stat_msg;
    adr_mems_msg_struct *data;
    struct timespec diff, spec;

    for (i = 0; i < count; i++) {
        //if use GNSS to get UTC time
        pthread_mutex_lock(&dev->spec_mutex);
        spec.tv_sec = dev->spec.tv_sec;
        spec.tv_nsec = dev->spec.tv_nsec;
        pthread_mutex_unlock(&dev->spec_mutex);
        if(!spec.tv_sec && !spec.tv_nsec){
            LOG_INFO("did not get time diff form GNSS, discard acc_gyro %d", i);
            break;
        }
        stat_msg = get_next_mems_data_stat_msg(dev);
        if(!stat_msg){
            LOG_WARN("acc gyro data ready thread process too slow");
            sleep(1);
            break;
        }
        pthread_mutex_lock(&dev->mems_dataready_mutex);
        stat_msg->busy = 1;
        pthread_mutex_unlock(&dev->mems_dataready_mutex);

        //record sensor id
        stat_msg->dev_id = buffer[i].acc.reserved;
        //stat_msg->dev_id = 0;

        data = &stat_msg->data;
        /*if(buffer[i].acc.value_divide == 0 || buffer[i].gyro.value_divide == 0){
            LOG_ERROR("[acc_gyro_p_%d] x:%d, y:%d, z:%d, t:%d, divide %d", i, buffer[i].acc.values[0], buffer[i].acc.values[1], buffer[i].acc.values[2],buffer[i].acc.values[3], buffer[i].acc.value_divide);
            LOG_ERROR("[acc_gyro_p_%d] ax:%d, y:%d, z:%d, t:%d, divide %d", i, buffer[i].gyro.values[0], buffer[i].gyro.values[1], buffer[i].gyro.values[2],buffer[i].gyro.values[3], buffer[i].gyro.value_divide);
            LOG_ERROR("[acc_gyro_p_%d] time:%"PRId64, i, buffer[i].gyro.time);
            abort();
        }*/
#if ADR_SUPPORT_SENSOR_V2
        data->acc.x = (double)buffer[i].acc.values[0];
        data->acc.y = (double)buffer[i].acc.values[1];
        data->acc.z = (double)buffer[i].acc.values[2];
        data->acc.t = (double)buffer[i].acc.values[3];

        data->gyro.x = (double)buffer[i].gyro.values[0];
        data->gyro.y = (double)buffer[i].gyro.values[1];
        data->gyro.z = (double)buffer[i].gyro.values[2];
        data->gyro.t = (double)buffer[i].gyro.values[3];
        data->monotonic_raw = buffer[i].gyro.time;
#else
        data->acc.x = (double)buffer[i].acc.values[0] / buffer[i].acc.value_divide;
        data->acc.y = (double)buffer[i].acc.values[1] / buffer[i].acc.value_divide;
        data->acc.z = (double)buffer[i].acc.values[2] / buffer[i].acc.value_divide;
        data->acc.t = (double)buffer[i].acc.values[3] / 10000;

        data->gyro.x = (double)buffer[i].gyro.values[0] / buffer[i].gyro.value_divide;
        data->gyro.y = (double)buffer[i].gyro.values[1] / buffer[i].gyro.value_divide;
        data->gyro.z = (double)buffer[i].gyro.values[2] / buffer[i].gyro.value_divide;
        data->gyro.t = (double)buffer[i].gyro.values[3] / 10000;
        data->monotonic_raw = buffer[i].gyro.time;
#endif

        get_mems_utc_time(spec, data, buffer[i].gyro.time);

        spec.tv_sec = buffer[i].gyro.time / nsec_2_sec;
        spec.tv_nsec = buffer[i].gyro.time % nsec_2_sec;
        if(dev->sensor_config.open_fake_odom){
            pthread_mutex_lock(&dev->odom_mutex);
            data->can1 = 0;
            data->can2 = -9999;
            data->can3 = -9999;
            pthread_mutex_unlock(&dev->odom_mutex);
        }else{
            diff_timespec_timespec(&spec, &dev->odom_boottime, &diff);
            if (diff.tv_sec <= 1) {
                pthread_mutex_lock(&dev->odom_mutex);
                data->can1 = dev->odom;
                data->can2 = dev->gear;
                data->can3 = dev->rudder;
                pthread_mutex_unlock(&dev->odom_mutex);
                LOG_DEBUG("receive odom [%f] time [%ld sec, %ld nsec],sensor time [%ld sec, %ld nsec]",
                    data->can1, dev->odom_boottime.tv_sec, dev->odom_boottime.tv_nsec, spec.tv_sec,spec.tv_nsec);
            } else {
                pthread_mutex_lock(&dev->odom_mutex);
                data->can1 = -9999;
                data->can2 = -9999;
                data->can3 = -9999;
                pthread_mutex_unlock(&dev->odom_mutex);
                LOG_WARN("the time interval between odom[%ld sec, %ld nsec] and sensor [%ld sec, %ld nsec]> 1s",
                    dev->odom_boottime.tv_sec, dev->odom_boottime.tv_nsec, spec.tv_sec,spec.tv_nsec);
            }
        }
        pthread_mutex_lock(&dev->mems_ready_list_mutex);
        module_list_insert(&dev->mems_ready_list, &stat_msg->link);
        pthread_mutex_unlock(&dev->mems_ready_list_mutex);
    }
}

#if ADR_SUPPORT_SENSOR_V2
static int sensor_acc_gyro_init(sensor_device *dev)
{
    LOG_INFO("init acc gyro");
    return 0;
}

#else
static int sensor_acc_gyro_init(sensor_device *dev)
{
    LOG_INFO("init acc gyro");

    int *fd = &dev->acc_gyro_fd;

/***********open sensor dev***********/
    *fd = open(dev->sensor_config.acc_gyro_dev, O_RDWR);
    if (*fd < 0){
        LOG_ERROR("open acc_gyro_dev %s failed", dev->sensor_config.acc_gyro_dev);
        return -1;
    }

    return 0;
}
#endif

#if ADR_SUPPORT_SENSOR_V2
static void *sensor_acc_gyro_io_thread(void *data)
{
    sensor_device *dev = (sensor_device *)data;
    int *fd = &dev->acc_gyro_fd, cmd, len, sensor_event_len;
    struct ACC_GYRO_DATA a_g_data[FIFO_DATA_CHUNK_NUM];
    sensors_event_t sensor_event[FIFO_DATA_CHUNK_NUM] = {0};
    sensors_event_t sensor_event_check_lost[3] = {0};
    int check_lost_num = 0;
    int pos = 0;

    LOG_INFO("sensor_acc_gyro_io_thread entry!");

/***********get sensor data***********/
    while(!dev->acc_gyro_io_thread_exit)
    {
        memset(&sensor_event, 0, sizeof(sensors_event_t) * FIFO_DATA_CHUNK_NUM);
        sensor_event_len = HfLooperEventLooper(dev->HfLooper, sensor_event, FIFO_DATA_CHUNK_NUM);

        switch (sensor_event_len) {
            case -ENODEV:
            LOG_ERROR("cwrapper looper stop nodevice error\n");
            return -3;
        }

        //if(len <= 0){
            //LOG_ERROR("get acc gyro data failed");
            //break;
        //}
        /*for(int i = 0; i < len; i++){
            if(a_g_data[i].acc.value_divide == 0 || a_g_data[i].gyro.value_divide == 0){
                LOG_ERROR("[acc_gyro_io_t_%d] x:%d, y:%d, z:%d, t:%d, divide %d", i, a_g_data[i].acc.values[0], a_g_data[i].acc.values[1], a_g_data[i].acc.values[2],a_g_data[i].acc.values[3], a_g_data[i].acc.value_divide);
                LOG_ERROR("[acc_gyro_io_t_%d] x:%d, y:%d, z:%d, t:%d, divide %d", i, a_g_data[i].gyro.values[0], a_g_data[i].gyro.values[1], a_g_data[i].gyro.values[2],a_g_data[i].gyro.values[3], a_g_data[i].gyro.value_divide);
                LOG_ERROR("[acc_gyro_io_t_%d] time:%"PRId64, i, a_g_data[i].gyro.time);
                //abort();
            }
        }*/

        if(sensor_event_len == 1) {
            switch(sensor_event[0].type) {
            case SENSOR_TYPE_ACCELEROMETER:
                //save Acce to temp buffer
                sensor_event_check_lost[0].type = sensor_event[0].type;
                sensor_event_check_lost[0].reserved0 = (double)sensor_event[0].reserved0;
                sensor_event_check_lost[0].timestamp= sensor_event[0].timestamp;
                sensor_event_check_lost[0].data[0] = (double)sensor_event[0].data[0];
                sensor_event_check_lost[0].data[1] = (double)sensor_event[0].data[1];
                sensor_event_check_lost[0].data[2] = (double)sensor_event[0].data[2];
                check_lost_num = 1;
                break;
            case SENSOR_TYPE_GYROSCOPE:
                if((check_lost_num == 1) && (abs(sensor_event[0].timestamp - sensor_event_check_lost[0].timestamp) < 10000000)) {
                    //save Gyro to temp buffer
                    sensor_event_check_lost[1].type = sensor_event[0].type;
                    sensor_event_check_lost[1].reserved0 = (double)sensor_event[0].reserved0;
                    sensor_event_check_lost[1].timestamp= sensor_event[0].timestamp;
                    sensor_event_check_lost[1].data[0] = (double)sensor_event[0].data[0];
                    sensor_event_check_lost[1].data[1] = (double)sensor_event[0].data[1];
                    sensor_event_check_lost[1].data[2] = (double)sensor_event[0].data[2];
                    check_lost_num = 2;
                } else {
                    check_lost_num = 0;
                }
                break;
            case SENSOR_TYPE_GYRO_TEMPERATURE + SENSOR_TYPE_DEVICE_PRIVATE_BASE:
                if((check_lost_num == 2) &&
                    (abs(sensor_event[0].timestamp - sensor_event_check_lost[0].timestamp) < 10000000) &&
                    (abs(sensor_event[0].timestamp - sensor_event_check_lost[1].timestamp) < 10000000)) {
                    //save Temperature to temp buffer
                    sensor_event_check_lost[2].type = sensor_event[0].type;
                    sensor_event_check_lost[2].reserved0 = (double)sensor_event[0].reserved0;
                    sensor_event_check_lost[2].timestamp= sensor_event[0].timestamp;
                    sensor_event_check_lost[2].data[0] = (double)sensor_event[0].data[0];
                    sensor_event_check_lost[2].data[1] = (double)sensor_event[0].data[1];
                    sensor_event_check_lost[2].data[2] = (double)sensor_event[0].data[2];
                    check_lost_num = 3;
                } else {
                    check_lost_num = 0;
                }
                break;
            }
        }

        if(check_lost_num == 3) {
            a_g_data[0].acc.reserved = sensor_event_check_lost[0].reserved0;
            a_g_data[0].acc.values[0] = (double)sensor_event_check_lost[0].acceleration.x;
            a_g_data[0].acc.values[1] = (double)sensor_event_check_lost[0].acceleration.y;
            a_g_data[0].acc.values[2] = (double)sensor_event_check_lost[0].acceleration.z;
            a_g_data[0].acc.values[3] = -9999;
            a_g_data[0].acc.time = sensor_event_check_lost[0].timestamp;

            a_g_data[0].gyro.reserved = sensor_event_check_lost[1].reserved0;
            a_g_data[0].gyro.values[0] = (double)sensor_event_check_lost[1].gyro.x;
            a_g_data[0].gyro.values[1] = (double)sensor_event_check_lost[1].gyro.y;
            a_g_data[0].gyro.values[2] = (double)sensor_event_check_lost[1].gyro.z;
            a_g_data[0].gyro.values[3] = (double)sensor_event_check_lost[2].data[0];
            a_g_data[0].gyro.time = sensor_event_check_lost[1].timestamp;
            check_lost_num = 0;
            pos = 1;
        } 

        if(sensor_event_len == 2) {
            switch(sensor_event[0].type) {
            case SENSOR_TYPE_ACCELEROMETER:
                if((sensor_event[0].type == SENSOR_TYPE_ACCELEROMETER) &&
                    (sensor_event[1].type == SENSOR_TYPE_GYROSCOPE) &&
                    (abs(sensor_event[0].timestamp - sensor_event[1].timestamp) < 10000000)){
                //save Acce to temp buffer
                sensor_event_check_lost[0].type = sensor_event[0].type;
                sensor_event_check_lost[0].reserved0 = (double)sensor_event[0].reserved0;
                sensor_event_check_lost[0].timestamp= sensor_event[0].timestamp;
                sensor_event_check_lost[0].data[0] = (double)sensor_event[0].data[0];
                sensor_event_check_lost[0].data[1] = (double)sensor_event[0].data[1];
                sensor_event_check_lost[0].data[2] = (double)sensor_event[0].data[2];

                //save Gyro to temp buffer
                sensor_event_check_lost[1].type = sensor_event[1].type;
                sensor_event_check_lost[1].reserved0 = (double)sensor_event[1].reserved0;
                sensor_event_check_lost[1].timestamp= sensor_event[1].timestamp;
                sensor_event_check_lost[1].data[0] = (double)sensor_event[1].data[0];
                sensor_event_check_lost[1].data[1] = (double)sensor_event[1].data[1];
                sensor_event_check_lost[1].data[2] = (double)sensor_event[1].data[2];
                check_lost_num = 2;
                }
                break;

            case SENSOR_TYPE_GYROSCOPE:
                if((check_lost_num == 1) && (sensor_event[0].type == SENSOR_TYPE_GYROSCOPE) &&
                    (sensor_event[1].type == SENSOR_TYPE_GYRO_TEMPERATURE + SENSOR_TYPE_DEVICE_PRIVATE_BASE) &&
                    (abs(sensor_event[0].timestamp - sensor_event_check_lost[0].timestamp) < 10000000) &&
                    (abs(sensor_event[1].timestamp - sensor_event_check_lost[0].timestamp) < 10000000)) {
                    a_g_data[0].acc.reserved = sensor_event_check_lost[0].reserved0;
                    a_g_data[0].acc.values[0] = (double)sensor_event_check_lost[0].acceleration.x;
                    a_g_data[0].acc.values[1] = (double)sensor_event_check_lost[0].acceleration.y;
                    a_g_data[0].acc.values[2] = (double)sensor_event_check_lost[0].acceleration.z;
                    a_g_data[0].acc.values[3] = -9999;
                    a_g_data[0].acc.time = sensor_event_check_lost[0].timestamp;

                    a_g_data[0].gyro.reserved = sensor_event[0].reserved0;
                    a_g_data[0].gyro.values[0] = (double)sensor_event[0].gyro.x;
                    a_g_data[0].gyro.values[1] = (double)sensor_event[0].gyro.y;
                    a_g_data[0].gyro.values[2] = (double)sensor_event[0].gyro.z;
                    a_g_data[0].gyro.values[3] = (double)sensor_event[1].data[0];
                    a_g_data[0].gyro.time = sensor_event[0].timestamp;
                    check_lost_num = 0;
                    pos = 1;
                } else {
                    check_lost_num = 0;
                }
                break;

            case SENSOR_TYPE_GYRO_TEMPERATURE + SENSOR_TYPE_DEVICE_PRIVATE_BASE:
                if((check_lost_num == 2) && (sensor_event[1].type == SENSOR_TYPE_ACCELEROMETER) &&
                    (abs(sensor_event[0].timestamp - sensor_event_check_lost[0].timestamp) < 10000000) &&
                    (abs(sensor_event[0].timestamp - sensor_event_check_lost[1].timestamp) < 10000000)) {
                    a_g_data[0].acc.reserved = sensor_event_check_lost[0].reserved0;
                    a_g_data[0].acc.values[0] = (double)sensor_event_check_lost[0].acceleration.x;
                    a_g_data[0].acc.values[1] = (double)sensor_event_check_lost[0].acceleration.y;
                    a_g_data[0].acc.values[2] = (double)sensor_event_check_lost[0].acceleration.z;
                    a_g_data[0].acc.values[3] = -9999;
                    a_g_data[0].acc.time = sensor_event_check_lost[0].timestamp;

                    a_g_data[0].gyro.reserved = sensor_event_check_lost[1].reserved0;
                    a_g_data[0].gyro.values[0] = (double)sensor_event_check_lost[1].gyro.x;
                    a_g_data[0].gyro.values[1] = (double)sensor_event_check_lost[1].gyro.y;
                    a_g_data[0].gyro.values[2] = (double)sensor_event_check_lost[1].gyro.z;
                    a_g_data[0].gyro.values[3] = (double)sensor_event[0].data[0];
                    a_g_data[0].gyro.time = sensor_event_check_lost[1].timestamp;
                    pos = 1;

                    //save Acce to temp buffer
                    sensor_event_check_lost[0].type = sensor_event[1].type;
                    sensor_event_check_lost[0].reserved0 = (double)sensor_event[1].reserved0;
                    sensor_event_check_lost[0].timestamp= sensor_event[1].timestamp;
                    sensor_event_check_lost[0].data[0] = (double)sensor_event[1].data[0];
                    sensor_event_check_lost[0].data[1] = (double)sensor_event[1].data[1];
                    sensor_event_check_lost[0].data[2] = (double)sensor_event[1].data[2];
                    check_lost_num = 1;
                } else {
                    check_lost_num = 0;
                }
                break;

            }
        }

        if(sensor_event_len > 2 && check_lost_num == 1){
            if((sensor_event[0].type == SENSOR_TYPE_GYROSCOPE) && 
                (sensor_event[1].type == SENSOR_TYPE_GYRO_TEMPERATURE + SENSOR_TYPE_DEVICE_PRIVATE_BASE) &&
                (abs(sensor_event[0].timestamp - sensor_event_check_lost[0].timestamp) < 10000000) &&
                (abs(sensor_event[1].timestamp - sensor_event_check_lost[0].timestamp) < 10000000)){
                a_g_data[0].acc.reserved = sensor_event_check_lost[0].reserved0;
                a_g_data[0].acc.values[0] = (double)sensor_event_check_lost[0].acceleration.x;
                a_g_data[0].acc.values[1] = (double)sensor_event_check_lost[0].acceleration.y;
                a_g_data[0].acc.values[2] = (double)sensor_event_check_lost[0].acceleration.z;
                a_g_data[0].acc.values[3] = -9999;
                a_g_data[0].acc.time = sensor_event_check_lost[0].timestamp;

                a_g_data[0].gyro.reserved = sensor_event[0].reserved0;
                a_g_data[0].gyro.values[0] = (double)sensor_event[0].gyro.x;
                a_g_data[0].gyro.values[1] = (double)sensor_event[0].gyro.y;
                a_g_data[0].gyro.values[2] = (double)sensor_event[0].gyro.z;
                a_g_data[0].gyro.values[3] = (double)sensor_event[1].data[0];
                a_g_data[0].gyro.time = sensor_event[0].timestamp;
                check_lost_num = 0;
                pos = 1;
            }
        }

        if(sensor_event_len > 2 && check_lost_num == 2){
            if((sensor_event[0].type == SENSOR_TYPE_GYRO_TEMPERATURE + SENSOR_TYPE_DEVICE_PRIVATE_BASE) &&
                (abs(sensor_event[0].timestamp - sensor_event_check_lost[0].timestamp) < 10000000) &&
                (abs(sensor_event[0].timestamp - sensor_event_check_lost[1].timestamp) < 10000000)){
                a_g_data[0].acc.reserved = sensor_event_check_lost[0].reserved0;
                a_g_data[0].acc.values[0] = (double)sensor_event_check_lost[0].acceleration.x;
                a_g_data[0].acc.values[1] = (double)sensor_event_check_lost[0].acceleration.y;
                a_g_data[0].acc.values[2] = (double)sensor_event_check_lost[0].acceleration.z;
                a_g_data[0].acc.values[3] = -9999;
                a_g_data[0].acc.time = sensor_event_check_lost[0].timestamp;

                a_g_data[0].gyro.reserved = sensor_event_check_lost[1].reserved0;
                a_g_data[0].gyro.values[0] = (double)sensor_event_check_lost[1].gyro.x;
                a_g_data[0].gyro.values[1] = (double)sensor_event_check_lost[1].gyro.y;
                a_g_data[0].gyro.values[2] = (double)sensor_event_check_lost[1].gyro.z;
                a_g_data[0].gyro.values[3] = (double)sensor_event[0].data[0];
                a_g_data[0].gyro.time = sensor_event_check_lost[1].timestamp;
                check_lost_num = 0;
                pos = 1;
            }
        }

        for (int p = 0; p < sensor_event_len-2; p++) {
            if((sensor_event[p].type == SENSOR_TYPE_ACCELEROMETER) &&
                (sensor_event[p+1].type == SENSOR_TYPE_GYROSCOPE) &&
                (sensor_event[p+2].type == SENSOR_TYPE_GYRO_TEMPERATURE + SENSOR_TYPE_DEVICE_PRIVATE_BASE) &&
                (abs(sensor_event[p+1].timestamp - sensor_event[p].timestamp) < 10000000)) {
                a_g_data[pos].acc.reserved = sensor_event[p].reserved0;
                a_g_data[pos].acc.values[0] = (double)sensor_event[p].acceleration.x;
                a_g_data[pos].acc.values[1] = (double)sensor_event[p].acceleration.y;
                a_g_data[pos].acc.values[2] = (double)sensor_event[p].acceleration.z;
                a_g_data[pos].acc.values[3] = -9999;
                a_g_data[pos].acc.time = sensor_event[p].timestamp;

                a_g_data[pos].gyro.reserved = sensor_event[p+1].reserved0;
                a_g_data[pos].gyro.values[0] = (double)sensor_event[p+1].gyro.x;
                a_g_data[pos].gyro.values[1] = (double)sensor_event[p+1].gyro.y;
                a_g_data[pos].gyro.values[2] = (double)sensor_event[p+1].gyro.z;
                a_g_data[pos].gyro.values[3] = (double)sensor_event[p+2].data[0];
                a_g_data[pos].gyro.time = sensor_event[p+1].timestamp;
                pos++;
            }
        }

        if((sensor_event_len > 0) && (sensor_event[sensor_event_len-1].type == SENSOR_TYPE_ACCELEROMETER)) {
            //save the last sensor event for lost check
            sensor_event_check_lost[0].type = sensor_event[sensor_event_len-1].type;
            sensor_event_check_lost[0].reserved0 = (double)sensor_event[sensor_event_len-1].reserved0;
            sensor_event_check_lost[0].timestamp = sensor_event[sensor_event_len-1].timestamp;
            sensor_event_check_lost[0].data[0] = (double)sensor_event[sensor_event_len-1].data[0];
            sensor_event_check_lost[0].data[1] = (double)sensor_event[sensor_event_len-1].data[1];
            sensor_event_check_lost[0].data[2] = (double)sensor_event[sensor_event_len-1].data[2];

            check_lost_num = 1;
        } else if((sensor_event_len > 1) && (sensor_event[sensor_event_len-2].type == SENSOR_TYPE_ACCELEROMETER) &&
            (sensor_event[sensor_event_len-1].type == SENSOR_TYPE_GYROSCOPE)) {
            //save the last sensor event for lost check
            sensor_event_check_lost[0].type = sensor_event[sensor_event_len-2].type;
            sensor_event_check_lost[0].reserved0 = (double)sensor_event[sensor_event_len-2].reserved0;
            sensor_event_check_lost[0].timestamp= sensor_event[sensor_event_len-2].timestamp;
            sensor_event_check_lost[0].data[0] = (double)sensor_event[sensor_event_len-2].data[0];
            sensor_event_check_lost[0].data[1] = (double)sensor_event[sensor_event_len-2].data[1];
            sensor_event_check_lost[0].data[2] = (double)sensor_event[sensor_event_len-2].data[2];

            sensor_event_check_lost[1].type = sensor_event[sensor_event_len-1].type;
            sensor_event_check_lost[1].reserved0 = (double)sensor_event[sensor_event_len-1].reserved0;
            sensor_event_check_lost[1].timestamp= sensor_event[sensor_event_len-1].timestamp;
            sensor_event_check_lost[1].data[0] = (double)sensor_event[sensor_event_len-1].data[0];
            sensor_event_check_lost[1].data[1] = (double)sensor_event[sensor_event_len-1].data[1];
            sensor_event_check_lost[1].data[2] = (double)sensor_event[sensor_event_len-1].data[2];
            check_lost_num = 2;
        }

        if(pos == 0) {
              continue;
        }

        sensor_acc_gyro_prefunc(dev, a_g_data, pos);
        /*signal to mems data thread to upload*/
        pos = 0;
        pthread_cond_signal(&dev->mems_dataready_cond);
    }

    LOG_INFO("sensor_acc_gyro_io_thread exit!");
    pthread_exit(0);
}

#else
static void *sensor_acc_gyro_io_thread(void *data)
{
    sensor_device *dev = (sensor_device *)data;
    int *fd = &dev->acc_gyro_fd, cmd, len;
    struct ACC_GYRO_DATA a_g_data[FIFO_DATA_CHUNK_NUM];

    LOG_INFO("sensor_acc_gyro_io_thread entry!");

/***********get sensor data***********/
    while(!dev->acc_gyro_io_thread_exit)
    {
        cmd = ACC_GYRO_IOCTL_GET_SENSORDATA;
        len = ioctl(*fd, cmd, a_g_data);
        if (len <= 0){
            LOG_ERROR("get acc gyro data failed");
            break;
        }
        /*for(int i = 0; i < len; i++){
            if(a_g_data[i].acc.value_divide == 0 || a_g_data[i].gyro.value_divide == 0){
                LOG_ERROR("[acc_gyro_io_t_%d] x:%d, y:%d, z:%d, t:%d, divide %d", i, a_g_data[i].acc.values[0], a_g_data[i].acc.values[1], a_g_data[i].acc.values[2],a_g_data[i].acc.values[3], a_g_data[i].acc.value_divide);
                LOG_ERROR("[acc_gyro_io_t_%d] x:%d, y:%d, z:%d, t:%d, divide %d", i, a_g_data[i].gyro.values[0], a_g_data[i].gyro.values[1], a_g_data[i].gyro.values[2],a_g_data[i].gyro.values[3], a_g_data[i].gyro.value_divide);
                LOG_ERROR("[acc_gyro_io_t_%d] time:%"PRId64, i, a_g_data[i].gyro.time);
                //abort();
            }
        }*/
        sensor_acc_gyro_prefunc(dev, a_g_data, len);
        /*signal to mems data thread to upload*/
        pthread_cond_signal(&dev->mems_dataready_cond);
    }

    LOG_INFO("sensor_acc_gyro_io_thread exit!");
    pthread_exit(0);
}


#endif

#if ADR_SUPPORT_SENSOR_V2
static int sensor_baro_init(sensor_device *dev)
{
    LOG_INFO("init baro");

    return 0;
}
#else
static int sensor_baro_init(sensor_device *dev)
{
    LOG_INFO("init baro");

    int *fd = &dev->baro_fd;

/***********open sensor dev***********/
    *fd = open(dev->sensor_config.baro_dev, O_RDWR);
    if (*fd < 0){
        LOG_ERROR("open baro_dev %s failed", dev->sensor_config.baro_dev);
        return -1;
    }
    return 0;
}
#endif
static void * sensor_baro_io_thread(void *data)
{
    sensor_device *dev = (sensor_device *)data;
    struct hwm_sensor_data baro_data[FIFO_DATA_CHUNK_NUM];
    int *fd = &dev->baro_fd, cmd, len, i;

    LOG_INFO("sensor_baro_io_thread entry");

/***********get sensor data***********/
    while (!dev->baro_io_thread_exit) {
        cmd = BARO_TEMP_IOCTL_GET_SENSORDATA;
        len = ioctl(*fd, cmd, baro_data);
        if (len <= 0){
            LOG_ERROR("get baro data failed");
            break;
        }
        for(i = 0; i < len; i++){
            if(baro_data[i].value_divide == 0){
                LOG_ERROR("[baro_io_t_%d] baro:%d, t:%d, divide %d", i, baro_data[i].values[0], baro_data[i].values[1], baro_data[i].value_divide);
                abort();
            }
        }

        sensor_baro_prefunc(dev, baro_data, len);
        pthread_cond_signal(&dev->baro_dataready_cond);
    }

    LOG_INFO("sensor_baro_io_thread exit!");
    pthread_exit(0);
}

static void* sensor_mems_data_ready_thread(void *data)
{
    sensor_device *dev = (sensor_device *)data;
    sensor_mems_stat_msg *stat_msg;
    adr_mems_msg_struct *mems;

    LOG_INFO("sensor_mems_data_ready_thread starting");
    while (!dev->mems_dataready_thread_exit)
    {
        pthread_mutex_lock(&dev->mems_dataready_mutex);
        pthread_cond_wait(&dev->mems_dataready_cond, &dev->mems_dataready_mutex);
        LOG_DEBUG("begin upload mems");
        pthread_mutex_unlock(&dev->mems_dataready_mutex);
        while(1) {
            pthread_mutex_lock(&dev->mems_ready_list_mutex);
            if (module_list_empty(&dev->mems_ready_list)) {
                pthread_mutex_unlock(&dev->mems_ready_list_mutex);
                break;
            }
            stat_msg = module_container_of(dev->mems_ready_list.prev, stat_msg, link);
            pthread_mutex_unlock(&dev->mems_ready_list_mutex);

            mems = &stat_msg->data;
            if(dev->sensor_config.acc_gyro_debug_mode){
                checkAcc(mems->acc.x, mems->acc.y, mems->acc.z,
                            mems->acc.t, mems->monotonic_raw);
                checkGyro(mems->gyro.x, mems->gyro.y, mems->gyro.z,
                            mems->gyro.t, mems->monotonic_raw);
            }

            module_box *box = get_available_module_box(dev->mod);

            if(stat_msg->dev_id == dev->sensor_config.main_dev_id){
                box->data = (void *)&stat_msg->data;
                box->count = 1;
                box->type = DATA_SENSOR_MEMS;
                dev->upload(dev->mod, box);
            }

            box->data = (void *)&stat_msg->data;
            box->count = 1;
            box->type = DATA_SENSOR_STORE;
            box->index = stat_msg->dev_id;
            dev->upload(dev->mod, box);

            release_module_box(box);

            pthread_mutex_lock(&dev->mems_ready_list_mutex);
            module_list_remove(&stat_msg->link);
            module_list_init(&stat_msg->link);
            pthread_mutex_unlock(&dev->mems_ready_list_mutex);
            
            pthread_mutex_lock(&dev->mems_dataready_mutex);
            stat_msg->busy = 0;
            pthread_mutex_unlock(&dev->mems_dataready_mutex);
        }
    }
    LOG_INFO("sensor_mems_data_ready_thread exit");
    pthread_exit(0);
}

static void* sensor_baro_data_ready_thread(void *data)
{
    sensor_device *dev = (sensor_device *)data;
    sensor_baro_stat_msg *stat_msg;
    sensor_iic_data *iic_data;

    LOG_DEBUG("baro_data_ready_thread starting");
    while (!dev->baro_dataready_thread_exit)
    {
        pthread_mutex_lock(&dev->baro_dataready_mutex);
        pthread_cond_wait(&dev->baro_dataready_cond, &dev->baro_dataready_mutex);
        LOG_DEBUG("begin upload baro");
        pthread_mutex_unlock(&dev->baro_dataready_mutex);
        while(1) {
            pthread_mutex_lock(&dev->baro_ready_list_mutex);
            if (module_list_empty(&dev->baro_ready_list)) {
                pthread_mutex_unlock(&dev->baro_ready_list_mutex);
                break;
            }
            stat_msg = module_container_of(dev->baro_ready_list.prev, stat_msg, link);
            pthread_mutex_unlock(&dev->baro_ready_list_mutex);

            iic_data = &stat_msg->data;
            if(dev->sensor_config.baro_debug_mode){
                checkBaro(iic_data->baro, iic_data->baroT,
                    iic_data->baro_boottime.tv_sec * nsec_2_sec + iic_data->baro_boottime.tv_nsec);
            }

            module_box *box = get_available_module_box(dev->mod);
            stat_msg->busy = 1;
            box->data = (void *)iic_data;
            box->count = sizeof(*iic_data);
            box->type = DATA_SENSOR_BARO;
            dev->upload(dev->mod, box);

            release_module_box(box);

            pthread_mutex_lock(&dev->baro_ready_list_mutex);
            module_list_remove(&stat_msg->link);
            module_list_init(&stat_msg->link);
            pthread_mutex_unlock(&dev->baro_ready_list_mutex);

            pthread_mutex_lock(&dev->baro_dataready_mutex);
            stat_msg->busy = 0;
            pthread_mutex_unlock(&dev->baro_dataready_mutex);

        }
    }
    LOG_INFO("sensor_baro_data_ready_thread exit");
    pthread_exit(0);
}

static int sensor_load_config_file(SENSOR_CONFIG_PARAM *sensor_config)
{
    struct adr_config_section *section;

    section = adr_config_get_section(config, "sensor", NULL, NULL);

    if (section) {
        adr_config_section_get_string(section, "acc_gyro_dev",
                    &sensor_config->acc_gyro_dev, ACC_GYRO_DEV_DEFAULT);
        adr_config_section_get_int64_t(section, "acc_poll_ns",
                    &sensor_config->acc_poll_ns, ACC_POLL_NS_DEFAULT);
        adr_config_section_get_int64_t(section, "gyro_poll_ns",
                    &sensor_config->gyro_poll_ns, GYRO_POLL_NS_DEFAULT);
        adr_config_section_get_int32_t(section, "main_dev_id",
                    &sensor_config->main_dev_id, 0);
        adr_config_section_get_int32_t(section, "count_sensors",
                    &sensor_config->count_sensors, 1);
        adr_config_section_get_int32_t(section, "acc_gyro_debug_mode",
                    &sensor_config->acc_gyro_debug_mode, 1);
        adr_config_section_get_int32_t(section, "open_baro",
                    &sensor_config->open_baro, 1);

        if(sensor_config->open_baro){
            adr_config_section_get_string(section, "baro_dev",
                &sensor_config->baro_dev, BARO_TEMP_DEV_DEFAULT);
            adr_config_section_get_int64_t(section, "baro_poll_ns",
                &sensor_config->baro_poll_ns, BARO_POLL_NS_DEFAULT);
            adr_config_section_get_int32_t(section, "baro_debug_mode",
                &sensor_config->baro_debug_mode, 1);
        }
    }
    else {
        LOG_ERROR("get sensor section failed");
        return -1;
    }

    section = adr_config_get_section(config, "odom", NULL, NULL);
    if (section) {
        adr_config_section_get_int32_t(section, "open_fake_odom",
                    &sensor_config->open_fake_odom, OPEN_FAKE_ODOM_DEGAULT);
    }
    else {
        LOG_ERROR("get odom section failed");
        return -1;
    }

    return 0;
}

static int sensor_acc_gyro_start(sensor_device *dev)
{
    int *fd = &dev->acc_gyro_fd, cmd, enable, delay, ret;
#if ADR_SUPPORT_SENSOR_V2
    int err = 0;
    int len = 0;
    void *mHfManager = HfManagerCreate();
    void *mHfLooper = HfLooperCreate(HfManagerGetFd(mHfManager), 40);

    /*********** set polling rate ***********/
    delay = dev->sensor_config.acc_poll_ns; // unit is ns

    if (HfManagerFindSensor(mHfManager, SENSOR_TYPE_ACCELEROMETER) < 0 ||
        HfManagerFindSensor(mHfManager, SENSOR_TYPE_GYROSCOPE) < 0 ) {
        LOG_ERROR("can't find sensor\n");
        err = -1;
        goto find_sensor_fail;
    }

    HfManagerEnableSensor(mHfManager, SENSOR_TYPE_ACCELEROMETER, 20000000, 0);
    HfManagerEnableSensor(mHfManager, SENSOR_TYPE_GYROSCOPE, 20000000, 0);
    HfManagerEnableSensor(mHfManager, SENSOR_TYPE_GYRO_TEMPERATURE, 20000000, 0);

    dev->acc_gyro_io_thread_exit = 0;
    dev->HfManager = mHfManager;
    dev->HfLooper = mHfLooper;

    ret = pthread_create(&dev->acc_gyro_io_thread, NULL, sensor_acc_gyro_io_thread, (void *)dev);
    if (ret != 0) {
        LOG_ERROR("can't create sensor_acc_gyro_io_thread!");
        err = -1;
        goto enable_sensor_fail;
    }

    dev->mems_dataready_thread_exit = 0;
    ret = pthread_create(&dev->mems_dataready_thread, NULL, sensor_mems_data_ready_thread, (void *)dev);
    if (ret != 0) {
        LOG_ERROR("can't create sensor_mems_dataready_thread!");
        err = -1;
        goto enable_sensor_fail;
    }else{
        return err;
    }

enable_sensor_fail:
    HfManagerDisableSensor(mHfManager, SENSOR_TYPE_ACCELEROMETER);
    HfManagerDisableSensor(mHfManager, SENSOR_TYPE_GYROSCOPE);

find_sensor_fail:
    HfLooperDestroy(dev->HfLooper);
    HfManagerDestroy(mHfManager);

    return err;

#else
    /*********** enable sensor  ***********/
    cmd = ACC_GYRO_IOCTL_SET_ENABLE;
    enable = 1; // 1 is enable, 0 is disable
    if (ioctl(*fd, cmd, &enable) < 0){
        LOG_ERROR("enable acc gyro sensor %s failed", dev->sensor_config.acc_gyro_dev);
        return -1;
    }
    /*********** get sensor odr ***********/
    /*
    same as before, please read from:
    /sys/class/misc/m_acc_misc/accgetodr
    /sys/class/misc/m_gyro_misc/gyrogetodr
    */
    
    /*********** set polling rate ***********/
    cmd = ACC_GYRO_IOCTL_SET_POLLING_RATE;
    delay = dev->sensor_config.acc_poll_ns; // unit is ns
    if (ioctl(*fd, cmd, &delay) < 0){
        LOG_ERROR("acc gyro set polling rate failed");
        return -1;
    }

    dev->acc_gyro_io_thread_exit = 0;
    ret = pthread_create(&dev->acc_gyro_io_thread, NULL, sensor_acc_gyro_io_thread, (void *)dev);
    if (ret != 0) {
        LOG_ERROR("can't create sensor_acc_gyro_io_thread!");
        return -1;
    }

    dev->mems_dataready_thread_exit = 0;
    ret = pthread_create(&dev->mems_dataready_thread, NULL, sensor_mems_data_ready_thread, (void *)dev);
    if (ret != 0) {
        LOG_ERROR("can't create sensor_mems_dataready_thread!");
        return -1;
    }

    return 0;
#endif
}

static int sensor_baro_start(sensor_device *dev)
{
    int *fd = &dev->baro_fd, cmd, enable, delay, ret;

    /*********** enable sensor    ***********/
    cmd =BARO_TEMP_IOCTL_SET_ENABLE;
    enable = 1; // 1 is enable, 0 is disable
    if (ioctl(*fd, cmd, &enable) < 0){
        LOG_ERROR("enable baro failed, %s", dev->sensor_config.baro_dev);
        return -1;
    }

    /*********** set polling rate ***********/
    cmd = BARO_TEMP_IOCTL_SET_POLLING_RATE;
    delay = dev->sensor_config.baro_poll_ns; // unit is ns
    if (ioctl(*fd, cmd, &delay) < 0){
        LOG_ERROR("baro set polling rate failed");
        return -1;
    }

    dev->baro_io_thread_exit = 0;
    ret = pthread_create(&dev->baro_io_thread, NULL, sensor_baro_io_thread, (void *)dev);
    if (ret != 0) {
        LOG_ERROR("can't create baro_temp_io_thread");
        return -1;
    }

    dev->baro_dataready_thread_exit = 0;
    ret = pthread_create(&dev->baro_dataready_thread, NULL, sensor_baro_data_ready_thread, (void *)dev);
    if (ret != 0) {
        LOG_ERROR("can't create sensor_baro_dataready_thread_exit");
        return -1;
    }

    return 0;
}

static void sensor_driver_start(struct module *mod)
{
    struct sensor_device *dev;
    dev = (sensor_device*)module_get_driver_data(mod);

    sensor_acc_gyro_start(dev);

    if(dev->sensor_config.open_baro)
        sensor_baro_start(dev);
}

static int32_t sensor_driver_init(struct module *mod)
{
    int32_t ret;
    struct sensor_device *dev = (sensor_device*)module_get_driver_data(mod);

    dev->odom = -9999;
    dev->gear = -9999;
    dev->rudder = -9999;
    pthread_mutex_init(&dev->spec_mutex, NULL);
    pthread_mutex_init(&dev->odom_mutex, NULL);

    baro_d_last = 0;
    baro_t_last = 1024;
    acc_t_last = 1024;
    gyro_t_last = 1024;

    ret = sensor_load_config_file(&dev->sensor_config);
    if (ret != 0) {
        LOG_ERROR("something wrong in load sensor config file");
        return -1;
    }

    if(sensor_acc_gyro_init(dev) != 0){
        LOG_ERROR("init acc gyro failed");
        return -1;
    }

    module_list_init(&dev->mems_ready_list);
    pthread_mutex_init(&dev->mems_ready_list_mutex, NULL);
    pthread_cond_init(&dev->mems_dataready_cond, NULL);
    pthread_mutex_init(&dev->mems_dataready_mutex, NULL);

    if(dev->sensor_config.open_baro){
        if(sensor_baro_init(dev) != 0){
            LOG_ERROR("init baro failed");
            return -1;
        }

        module_list_init(&dev->baro_ready_list);
        pthread_mutex_init(&dev->baro_ready_list_mutex, NULL);
        pthread_cond_init(&dev->baro_dataready_cond, NULL);
        pthread_mutex_init(&dev->baro_dataready_mutex, NULL);
    }

    return 0;
}

static void sensor_driver_download(struct module *mod, module_box *box)
{
    struct sensor_device *dev;
    dev = (sensor_device*)module_get_driver_data(mod);

    switch(box->type) {
        case DATA_GNSS_TIME:
            sensor_align_timer(dev, (struct timespec *)(box->data));
            break;
        case DATA_ODOM:
            sensor_align_odom(dev, (odom_uart_data *)(box->data));
            break;
    }
}

static void sensor_driver_stop(struct module *mod)
{
    struct sensor_device *dev;
    dev = (sensor_device*)module_get_driver_data(mod);

    sensor_acc_gyro_stop(dev);
    if(dev->sensor_config.open_baro)
        sensor_baro_stop(dev);
}

static void sensor_driver_deinit(struct module *mod)
{
    struct sensor_device *dev;
    dev = (sensor_device*)module_get_driver_data(mod);

    pthread_cond_destroy(&dev->mems_dataready_cond);
    pthread_mutex_destroy(&dev->mems_dataready_mutex);
    sensor_acc_gyro_deinit(dev);
    free_mems_data_stat_msg(dev);
    pthread_mutex_destroy(&dev->spec_mutex);
    pthread_mutex_destroy(&dev->odom_mutex);
    pthread_mutex_destroy(&dev->mems_ready_list_mutex);
    acc_t_last = 1024;
    gyro_t_last = 1024;

    if(dev->sensor_config.open_baro){
        pthread_cond_destroy(&dev->baro_dataready_cond);
        pthread_mutex_destroy(&dev->baro_dataready_mutex);
        sensor_baro_deinit(dev);
        free_baro_data_stat_msg(dev);
        pthread_mutex_destroy(&dev->baro_ready_list_mutex);
        baro_d_last = 0;
        baro_t_last = 1024;
    }

    module *store_mod = dev->mod;
    memset(dev, 0, sizeof(sensor_device));
    dev->mod = store_mod;

    LOG_INFO("sensor driver deinit ok");
}

static void sensor_driver_registry(struct module *mod, process_data callback)
{
    struct sensor_device *dev;
    dev = (sensor_device*)module_get_driver_data(mod);

    dev->upload = callback;
}

static module_driver sensor_driver = {
    .name = "mtk_sensor",
    .capability = DATA_GNSS_TIME | DATA_ODOM,
    .start = sensor_driver_start,
    .stop = sensor_driver_stop,
    .init = sensor_driver_init,
    .download = sensor_driver_download,
    .deinit = sensor_driver_deinit,
    .registry = sensor_driver_registry,
};

void module_sensor_deinit(module *mod)
{
    struct sensor_device *dev;
    dev = (sensor_device*)module_get_driver_data(mod);
    
    free(dev);
    module_register_driver(mod, NULL, NULL);
}

int module_sensor_init(module *mod)
{
    struct sensor_device *dev;

    dev = (sensor_device*)zalloc(sizeof(sensor_device));
    if (!dev) {
        LOG_ERROR("Fail to create sensor_device");
        return -1;
    }
    g_sensor_ctx = dev;
    dev->mod = mod;
    module_set_name(mod, sensor_driver.name);
    module_register_driver(mod, &sensor_driver, (void *)dev);
    return 0;
}

