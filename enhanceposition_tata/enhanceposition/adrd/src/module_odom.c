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
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>

#include "module.h"
#include "adr_data_convert.h"
#include "module_odom_helper.h"
#include "config-parser.h"

extern int  log_vehicle_level;

#define  VEHICLE_INFO_SLEEP_MS   50

/**
 *  @enum VEHICLE_RET
 *  @brief Return Value of Vehicle Information API
 */
typedef enum
{
    V_SUCCESS = 0, V_ERROR = -1, V_NOTSUPPORT = -2
}VEHICLE_RET;

typedef struct odom_car_stat_msg {
    odom_uart_data data;
    int32_t busy;
} odom_car_stat_msg;

typedef struct odom_device {
    char *name;
    struct module *mod;
    void (*upload)(module *, module_box *);

    odom_module_data *odomdata;
    odom_car_stat_msg *carInfo[ODOM_CAR_DATA_LEN];

    pthread_t recv_thread;
    int rcv_thread_exit;

    void *odom_handle;//store handle module form vehicle_info.so
    void *odom_handle_data;//store data from vehicle_info.so
} odom_device;

static odom_device *g_odom_ctx = NULL;

static odom_car_stat_msg *get_next_car_data_stat_msg(odom_device *dev)
{
    int32_t cnt = 0;

    while(cnt < ODOM_CAR_DATA_LEN) {
        if (!dev->carInfo[cnt]) {
            dev->carInfo[cnt] = (odom_car_stat_msg *)zalloc(sizeof(odom_car_stat_msg));
            return dev->carInfo[cnt];
        } else {
            if (dev->carInfo[cnt]->busy == 0)
                 return dev->carInfo[cnt];
        }
        cnt += 1;
    }
    return NULL;
}

static void free_car_data_stat_msg(odom_device *dev)
{
    int32_t cnt = 0;

    while(cnt < ODOM_CAR_DATA_LEN) {
        if (dev->carInfo[cnt]) {
            free(dev->carInfo[cnt]);
            dev->carInfo[cnt] = NULL;
        }
        cnt += 1;
    }
}

static int recieve_data(odom_device *dev, odom_uart_data *v_info)
{
    int32_t (*vehicle_update)(void *handle);
    int32_t (*vehicle_get_vehicle_speed)(void *handle, float *vehicle_speed,
        struct timespec *timestamp);
    int32_t (*vehicle_get_gear)(void *handle, int32_t *gear, struct timespec *timestamp);
    int32_t (*vehicle_get_steering_angle)(void *handle, float *angle,
        struct timespec *timestamp);
    int32_t (*vehicle_get_wheel_speed)(void *handle, float *l_front_speed,
        float *r_front_speed, float *l_rear_speed, float *r_rear_speed,
        struct timespec *timestamp);

    int ret;

    vehicle_update = dlsym(dev->odom_handle, "vehicle_update");
    if (!vehicle_update)
    {
        LOG_ERROR("%s", dlerror());
        return -1;
    }
    vehicle_get_vehicle_speed = dlsym(dev->odom_handle, "vehicle_get_vehicle_speed");
    if (!vehicle_get_vehicle_speed)
    {
        LOG_ERROR("%s", dlerror());
        return -1;
    }
    vehicle_get_gear = dlsym(dev->odom_handle, "vehicle_get_gear");
    if (!vehicle_get_gear)
    {
        LOG_ERROR("%s", dlerror());
        return -1;
    }
    vehicle_get_steering_angle = dlsym(dev->odom_handle, "vehicle_get_steering_angle");
    if (!vehicle_get_steering_angle)
    {
        LOG_ERROR("%s", dlerror());
        return -1;
    }
    vehicle_get_wheel_speed = dlsym(dev->odom_handle, "vehicle_get_wheel_speed");
    if (!vehicle_get_wheel_speed)
    {
        LOG_ERROR("%s", dlerror());
        return -1;
    }

    /*update vehicle date*/
    if(vehicle_update(dev->odom_handle_data) == V_ERROR)
    {
        LOG_ERROR("vehicle_update failed");
        return -1;
    }

    /*begin to get vehicle data*/
    ret = vehicle_get_vehicle_speed(dev->odom_handle_data, &v_info->odom,
            &v_info->vehicle_speed_boottime);
    switch (ret){
        case V_ERROR:
            LOG_ERROR("vehicle_get_vehicle_speed failed");
            return -1;
        case V_NOTSUPPORT:
            v_info->odom = -9999;
    }

    ret = vehicle_get_gear(dev->odom_handle_data, &v_info->gear, &v_info->gear_boottime);
    switch (ret){
        case V_ERROR:
            LOG_ERROR("vehicle_get_gear failed");
            return -1;
        case V_NOTSUPPORT:
            v_info->gear= -9999;
    }

    ret = vehicle_get_steering_angle(dev->odom_handle_data, &v_info->rudder,
            &v_info->angle_boottime);
    switch (ret){
        case V_ERROR:
            LOG_ERROR("vehicle_get_steering_angle failed");
            return -1;
        case V_NOTSUPPORT:
            v_info->rudder = -9999;
    }

    ret = vehicle_get_wheel_speed(dev->odom_handle_data, &v_info->l_front_speed,
            &v_info->r_front_speed, &v_info->l_rear_speed, &v_info->r_rear_speed,
            &v_info->wheel_speed_boottime);
    switch (ret){
        case V_ERROR:
            LOG_ERROR("vehicle_get_wheel_speed failed");
            return -1;
        case V_NOTSUPPORT:
            v_info->l_front_speed = -9999;
            v_info->r_front_speed = -9999;
            v_info->l_rear_speed = -9999;
            v_info->r_rear_speed = -9999;
    }

    return 0;
}

static void *odom_recv_thread(void *data)
{

    
    struct timespec req, rem;
    struct odom_device *dev = (struct odom_device *)data;
    int ret = -1;
    odom_uart_data v_info;

    //sleep 50ms
    req.tv_sec = 0;
    req.tv_nsec = VEHICLE_INFO_SLEEP_MS * 1000000L;   // 50ms

    while (!dev->rcv_thread_exit)
    {
        ret = recieve_data(dev, &v_info);
        LOG_DEBUG("receive data from odom, ret[%d]....", ret);

        if (ret >= 0) {
            dev->odomdata->gear = v_info.gear;
            dev->odomdata->rudder = v_info.rudder;
            dev->odomdata->odom = v_info.odom;
            dev->odomdata->timestamp = v_info.vehicle_speed_boottime;

            module_box *odom_box = get_available_module_box(dev->mod);
            odom_car_stat_msg *odom_car = get_next_car_data_stat_msg(dev);
            odom_to_carInfo_msg((void *)dev->odomdata, (void *)&odom_car->data);

            //get_boot_time(&odom_car->data.odom_boottime);

            odom_car->busy = 1;
            odom_box->data = (void *)&odom_car->data;
            odom_box->count = sizeof(odom_car->data);
            odom_box->type = DATA_ODOM;
            dev->upload(dev->mod, odom_box);
            release_module_box(odom_box);
            odom_car->busy = 0;
        }
        if (nanosleep(&req, &rem) < 0) {
            LOG_DEBUG("nanosleep fail");
    }
    }
    LOG_INFO("odom_recv_thread exit ....");
    pthread_exit(0);
}

static int odom_hal_start(odom_device *dev)
{
    int (*vehicle_start)( void *handle);

    vehicle_start = dlsym(dev->odom_handle, "vehicle_start");
    if (!vehicle_start)
    {
        LOG_ERROR( "%s", dlerror());
        return -1;
    }
    return vehicle_start(dev->odom_handle_data);
}

static int odom_hal_stop(odom_device *dev)
{
    int (*vehicle_stop)(void *handle);

    vehicle_stop = dlsym(dev->odom_handle, "vehicle_stop");
    if (!vehicle_stop)
    {
        LOG_ERROR( "%s", dlerror());
        return -1;
    }
    return vehicle_stop(dev->odom_handle_data);
}

static void odom_driver_stop(struct module *mod)
{
    struct odom_device *dev =
        (odom_device*)module_get_driver_data(mod);

    /*receive thread is alive, means odom setup ok, so need disable odom and receive thread*/
    if(!dev->rcv_thread_exit) {
        /*odom stop */
        if(odom_hal_stop(dev) < 0)
            LOG_ERROR("odom can stop failed");
        dev->rcv_thread_exit = 1;
        pthread_join(dev->recv_thread, NULL);
        LOG_INFO("stop odom receive thread successfully!");
    }
}

static void odom_driver_start(struct module *mod)
{
    struct odom_device *dev = (odom_device*)module_get_driver_data(mod);

    if(pthread_create(&dev->recv_thread, NULL, odom_recv_thread, (void*)dev) != 0)
    {
        LOG_ERROR("can't create thread for recv can odom data ,%s",strerror(errno));
    }
    /*odom start */
    if (odom_hal_start(dev) < 0){
        dev->rcv_thread_exit = 1;
        pthread_join(dev->recv_thread, NULL);
        LOG_ERROR("odom start failed, stop odom receive thread successfully!");
    }
}

static int odom_hal_init(odom_device *dev)
{
    void *module = dlopen(V_LIBDIR, RTLD_LAZY);
    int (*vehicle_init)(void **handle);

    LOG_DEBUG("Loading module '%s'", V_LIBDIR);

    if (!module)
    {
        LOG_ERROR("Failed to load module: %s", dlerror());
        return -1;
    }
    dev->odom_handle = module;

    vehicle_init = dlsym(module, "vehicle_init");
    if (!vehicle_init)
    {
        LOG_ERROR( "%s", dlerror());
        return -1;
    }
    return vehicle_init(&dev->odom_handle_data);
}

static int odom_driver_init(struct module *mod)
{
    struct odom_device *dev = (odom_device*)module_get_driver_data(mod);

    if (odom_hal_init(dev) < 0) {
        LOG_ERROR("odom_hal_init failed");
        return -1;
    }

    int32_t (*vehicle_set_log_level)(void *handle, int32_t level);
    vehicle_set_log_level = dlsym(dev->odom_handle, "vehicle_set_log_level");
    if (!vehicle_set_log_level)
    {
        LOG_ERROR( "%s", dlerror());
        return -1;
    }
    vehicle_set_log_level(dev->odom_handle_data, log_vehicle_level);

    dev->odomdata = (odom_module_data *)zalloc(sizeof(odom_module_data));
    dev->rcv_thread_exit = 0;

    LOG_INFO("odom driver init ok");
    return 0;
}

static int odom_hal_deinit(odom_device *dev)
{
    int (*odom_deinit)(void **pri);
    odom_deinit = dlsym(dev->odom_handle, "vehicle_deinit");
    if (!odom_deinit)
    {
        LOG_ERROR( "%s", dlerror());
        return -1;
    }
    return odom_deinit(&dev->odom_handle_data);
}

static void odom_driver_deinit(struct module *mod)
{
    LOG_INFO("odom driver deiniting ......");

    struct odom_device *dev;
    dev = (odom_device*)module_get_driver_data(mod);

    if (odom_hal_deinit(dev) < 0)
        LOG_ERROR("odom_hal_init failed");

    free_car_data_stat_msg(dev);

    if (dev->odomdata)
        free(dev->odomdata);

    if (dev->odom_handle)
        dlclose(dev->odom_handle);

    module *store_mod = dev->mod;
    memset(dev, 0, sizeof(odom_device));
    dev->mod = store_mod;
    LOG_INFO("odom driver deinit ok");
}

static void odom_driver_registry(struct module *mod, process_data callback)
{
    struct odom_device *dev;
    dev = (odom_device*)module_get_driver_data(mod);

    dev->upload = callback;
}

static module_driver odom_driver = {
    .name = "mtk_odom",
    .init = odom_driver_init,
    .start = odom_driver_start,
    .stop = odom_driver_stop,
    .deinit = odom_driver_deinit,
    .registry = odom_driver_registry,
};

void module_odom_deinit(module *mod)
{
    struct odom_device *dev;
    dev = (odom_device*)module_get_driver_data(mod);
    free(dev);
    module_register_driver(mod, NULL, NULL);
}

int module_odom_init(module *mod)
{
    struct odom_device *dev;

    dev = (odom_device*)zalloc(sizeof(odom_device));
    if (!dev) {
        LOG_ERROR("Fail to create odom_device");
        return -1;
    }
    g_odom_ctx = dev;
    dev->mod = mod;
    module_set_name(mod, odom_driver.name);
    module_register_driver(mod, &odom_driver, (void *)dev);
    return 0;
}
