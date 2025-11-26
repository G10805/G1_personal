/**
 * @file ar0234_ext_fpga.h
 *
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AR0234_EXT_FPGA_H__
#define __AR0234_EXT_FPGA_H__

#include "max96712_lib.h"

typedef enum
{
    AR0234_EXT_FPGA_MODE_YUV8_FULL_RES = 0,          /**< 8bit yuv 30fps */
    AR0234_EXT_FPGA_MODE_YUV8_FULL_RES_60FPS,        /**< 8bit yuv 60fps */
    AR0234_EXT_FPGA_MODE_MAX
}ar0234_ext_fpga_mode_t;

/**
 * AR0234 with External FPGA
 */
max96712_sensor_t* ar0234_ext_fpga_get_sensor_info(void);

#endif /* __AR0234_EXT_FPGA_H__ */
