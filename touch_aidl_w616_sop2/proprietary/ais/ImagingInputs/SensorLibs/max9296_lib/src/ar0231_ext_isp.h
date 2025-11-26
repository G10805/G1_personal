/**
 * @file ar0231_ext_isp.h
 *
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AR0231_EXT_ISP_H__
#define __AR0231_EXT_ISP_H__

#include "max9296_lib.h"

typedef enum
{
    AR0231_EXT_ISP_MODE_YUV8_CROP_METADATA = 0, /**< 8bit yuv without metadata (set crop of embedded lines) */
    AR0231_EXT_ISP_MODE_YUV8_FULL_RES,          /**< 8bit yuv with metadata; NOTE: the 2 LSBs are dropped so metadata may not make sense */
    AR0231_EXT_ISP_MODE_YUV10_CROP_METADATA,    /**< 10bit yuv without metadata (set crop of embedded lines) */
    AR0231_EXT_ISP_MODE_YUV10_FULL_RES,         /**< 10bit yuv with metadata */
    AR0231_EXT_ISP_MODE_MAX
}ar0231_ext_isp_mode_t;

/**
 * AR0231 with External ISP
 */
max9296_sensor_t* ar0231_ext_isp_get_sensor_info(void);

#endif /* __AR0231_EXT_ISP_H__ */
