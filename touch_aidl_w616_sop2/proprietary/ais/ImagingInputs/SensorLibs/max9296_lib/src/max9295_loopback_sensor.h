/**
 * @file max9295_loopback_sensor.h
 *
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MAX9295_LOOPBACK_SENSOR__
#define __MAX9295_LOOPBACK_SENSOR__

#include "max9296_lib.h"

typedef enum
{
    MAX9295_LOOPBACK_FULL_RES,          /**< 8bit RGB888 with full resolution */
    MAX9295_LOOPBACK_MODE_MAX
}max9295_loopback_mode_t;

/**
 * MAX9295 with Loopback setup
 */
max9296_sensor_t* max9295_loopback_get_info(void);

#endif /* __MAX9295_LOOPBACK_SENSOR__ */
