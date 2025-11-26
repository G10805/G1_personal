/**
 * @file max9295_sensor.h
 *
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MAX9295_SENSOR_H__
#define __MAX9295_SENSOR_H__

#include "max9296_lib.h"

#define SENSOR_WIDTH    1920
#define SENSOR_HEIGHT   1024

#define CAM_SENSOR_DEFAULT_ADDR         0xE0
#define CAM_EXT_ISP_DEFAULT_ADDR        0xBA

/*CONFIGURATION OPTIONS*/

#define _max9295_delay_ 0
#define MAX9295_LINK_RESET_DELAY 100000
#define MAX9295_START_DELAY 20000
#define MAX9295_STOP_DELAY 20000

#define CID_VC0        0
#define CID_VC1        4

#define CAM_SER9295_INIT_A \
{ \
    { 0x0002, 0x03, _max9295_delay_ }, \
    { 0x0053, 0x10, _max9295_delay_ }, \
    { 0x0057, 0x11, _max9295_delay_ }, \
    { 0x0311, 0xF0, _max9295_delay_ }, \
}

#define CAM_SER9295_INIT_B \
{ \
    { 0x0002, 0x03, _max9295_delay_ }, \
    { 0x0053, 0x01, _max9295_delay_ }, \
    { 0x0311, 0x10, _max9295_delay_ }, \
}

#define CAM_SER9295_START \
{ \
    { 0x0002, 0x13, MAX9295_START_DELAY }, \
}

#define CAM_SER9295_STOP \
{ \
    { 0x0002, 0x03, MAX9295_STOP_DELAY }, \
}

#define CAM_SER9295_ADDR_CHNG_A \
{ \
    { 0x006B, 0x10, _max9296_delay_ }, \
    { 0x0073, 0x11, _max9296_delay_ }, \
    { 0x007B, 0x30, _max9296_delay_ }, \
    { 0x0083, 0x30, _max9296_delay_ }, \
    { 0x0093, 0x30, _max9296_delay_ }, \
    { 0x009B, 0x30, _max9296_delay_ }, \
    { 0x00A3, 0x30, _max9296_delay_ }, \
    { 0x00AB, 0x30, _max9296_delay_ }, \
    { 0x008B, 0x30, _max9296_delay_ }, \
}

#define CAM_SER9295_ADDR_CHNG_B \
{ \
    { 0x006B, 0x12, _max9296_delay_ }, \
    { 0x0073, 0x13, _max9296_delay_ }, \
    { 0x007B, 0x32, _max9296_delay_ }, \
    { 0x0083, 0x32, _max9296_delay_ }, \
    { 0x0093, 0x32, _max9296_delay_ }, \
    { 0x009B, 0x32, _max9296_delay_ }, \
    { 0x00A3, 0x32, _max9296_delay_ }, \
    { 0x00AB, 0x32, _max9296_delay_ }, \
    { 0x008B, 0x32, _max9296_delay_ }, \
}

#define FLOAT_TO_FIXEDPOINT(b, f) \
  (((f)*(1<<(b))))

typedef enum
{
    MAX9295_MODE_RECORDER_YUV8_FULL_RES = 0, /**< 8bit yuv with metadata; NOTE: this mode is used when a video recorder is attached to de-serializer instead of actual camera */
    MAX9295_MODE_MAX
}max9295_mode_t;

/**
 * MAX9295
 */
max9296_sensor_t* ser9295_get_info(void);

#endif /* __AR0231_EXT_ISP_H__ */
