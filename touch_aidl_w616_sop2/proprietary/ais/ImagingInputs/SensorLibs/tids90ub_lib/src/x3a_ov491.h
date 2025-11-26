/**
 * @file x3a_ov491.h
 *
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __X3A_OV491_H__
#define __X3A_OV491_H__

#include "tids90ub_lib.h"

#define X3A_OV491_SENSOR_FORMAT   QCARCAM_FMT_UYVY_8

#define X3A_OV491_SENSOR_DT       CSI_DT_YUV422_8

#define X3A_OV491_SENSOR_WIDTH    1620
#define X3A_OV491_SENSOR_HEIGHT   1284

#define X3A_OV491_CID_VC0 0
#define X3A_OV491_CID_VC1 4
#define X3A_OV491_CID_VC2 8
#define X3A_OV491_CID_VC3 12

#define X3A_OV491_VC0 0
#define X3A_OV491_VC1 1
#define X3A_OV491_VC2 2
#define X3A_OV491_VC3 3

/*CONFIGURATION OPTIONS*/
#define _x3a_ov491_delay_ 0

#define X3A_OV491_SENSOR_DEFAULT_ADDR 0x30

typedef enum
{
    X3A_OV491_TI935_MODE_DEFAULT = 0,
    X3A_OV491_TI935_MODE_2MP     = X3A_OV491_TI935_MODE_DEFAULT,
    X3A_OV491_TI935_MODE_MAX
}x3a_ov491_ti935_mode_t;

#endif /* __X3A_OV491__ */
