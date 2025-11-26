/**
 * @file ar0820_ext_isp_gw5300.h
 *
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AR0820_EXT_ISP_GW5300_H__
#define __AR0820_EXT_ISP_GW5300_H__

#include "max96712_lib.h"
#include <string.h>

#define SENSOR_WIDTH    3840
#define SENSOR_HEIGHT   2165

#define SENSOR_FORMAT   QCARCAM_FMT_UYVY_8
#define SENSOR_DT       CSI_DT_YUV422_8

#define MSM_SER_CHIP_ID_REG_ADDR 0x0D

/*CONFIGURATION OPTIONS*/
#define _ar0820_ext_isp_gw5300_delay_ 0

typedef enum
{
    AR0820_EXT_ISP_GW5300_MODE_DEFAULT  = 0,
    AR0820_EXT_ISP_GW5300_MODE_8MP      = AR0820_EXT_ISP_GW5300_MODE_DEFAULT,
    AR0820_EXT_ISP_GW5300_MODE_MAX
}ar0820_ext_isp_gw5300_mode_t;

#endif /* __AR0820_EXT_ISP_GW5300_H__ */
