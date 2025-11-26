/**
 * @file s5k1h1sx.h
 *
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __S5K1H1SX_H__
#define __S5K1H1SX_H__

#include "max96712_lib.h"
#include <string.h>

#define SENSOR_WIDTH    3856
#define SENSOR_HEIGHT   1936

#define SENSOR_FORMAT   QCARCAM_FMT_MIPIRAW_12
#define SENSOR_DT       CSI_DT_RAW12

#define DEFAULT_SER_ADDR 0x80
#define DEFAULT_SENSOR_ADDR 0x20
#define MSM_SER_CHIP_ID_REG_ADDR 0x0D

/*CONFIGURATION OPTIONS*/
#define _s5k1h1sx_delay_ 0

typedef enum
{
    S5K1H1SX_MODE_DEFAULT   = 0,
    S5K1H1SX_MODE_8MP       = S5K1H1SX_MODE_DEFAULT,
    S5K1H1SX_MODE_TPG_MODE0 = 250,                            //SOLID test pattern generator from sensor
    S5K1H1SX_MODE_TPG_MODE1 = 251,                            //Horizontal gradient test pattern generator from sensor
    S5K1H1SX_MODE_TPG_MODE2 = 252,                            //Vertical gradient test pattern generator from sensor
    S5K1H1SX_MODE_MAX
}s5k1h1sx_mode_t;

#endif /* __S5K1H1SX_H__ */
