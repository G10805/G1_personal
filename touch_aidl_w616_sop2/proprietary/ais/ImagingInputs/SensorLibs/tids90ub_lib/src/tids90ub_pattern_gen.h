/**
 * @file tids90ub_pattern_gen.h
 *
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __TIDS90UB_PATTERN_GEN_H__
#define __TIDS90UB_PATTERN_GEN_H__

#include "tids90ub_lib.h"

#define TPG_SENSOR_WIDTH_2MP    1920
#define TPG_SENSOR_HEIGHT_2MP   1080

#define TPG_SENSOR_WIDTH_4MP    2688
#define TPG_SENSOR_HEIGHT_4MP   1520

#define TPG_SENSOR_WIDTH_8MP    3840
#define TPG_SENSOR_HEIGHT_8MP   1920

#define TPG_SENSOR_WIDTH_RGB_888_FHD  1920
#define TPG_SENSOR_HEIGHT_RGB_888_FHD 1080

#define TPG_SENSOR_WIDTH_RGB_888_HD  1280
#define TPG_SENSOR_HEIGHT_RGB_888_HD 720

#define TPG_SENSOR_FORMAT   QCARCAM_FMT_UYVY_8
#define TPG_SENSOR_DT       CSI_DT_YUV422_8

#define TPG_SENSOR_FORMAT_RGB_888 QCARCAM_FMT_RGB_888
#define TPG_SENSOR_DT_RGB_888     CSI_DT_RGB888

/*CONFIGURATION OPTIONS*/
#define _tids90ub_pattern_gen_delay_ 0

#define TPG_SENSOR_DEFAULT_ADDR 0x30

// Generate pattern 1920 x 1080
#define START_REG_ARRAY_TPG_2MP \
{ \
    {0xB0, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x23, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x1E, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0F, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x00, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x03, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xC0, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x04, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x38, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x04, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x48, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0B, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xE1, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x07, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x08, _tids90ub_pattern_gen_delay_}, \
}

// Generate pattern 2688 x 1520
#define START_REG_ARRAY_TPG_4MP \
{ \
    {0xB0, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x23, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x1E, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x15, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x00, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x05, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x40, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x05, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xF0, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x06, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x00, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x08, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x7A, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x07, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x08, _tids90ub_pattern_gen_delay_}, \
}

// Generate pattern 3840 x 1920
#define START_REG_ARRAY_TPG_8MP \
{ \
    {0xB0, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x23, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x1E, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x1E, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x00, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x07, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x80, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x07, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x80, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x07, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x90, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x06, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xBA, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x07, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x08, _tids90ub_pattern_gen_delay_}, \
}

// Generate RGB 888 color bar pattern 1920 x 1080
#define START_REG_ARRAY_TPG_RGB_888_FHD_COLOR_BAR \
{ \
    {0xB0, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x33, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x24, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x16, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x80, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xD0, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x04, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x38, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x04, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x65, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0B, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x93, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x21, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0A, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
}

// Generate RGB 888 color bar pattern 1280 x 720
#define START_REG_ARRAY_TPG_RGB_888_HD_COLOR_BAR \
{ \
    {0xB0, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x33, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x24, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0F, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x00, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xE0, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xD0, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xFD, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x11, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x05, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x21, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0A, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
}

// Generate RGB 888 fixed color pattern 1920 x 1080
#define START_REG_ARRAY_TPG_RGB_888_FHD_FIXED_COLOR \
{ \
    {0xB0, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xB3, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x24, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x16, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x80, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xD0, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x04, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x38, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x04, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x65, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0B, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x93, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x21, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0A, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xA2, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x72, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xE2, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
}

// Generate RGB 888 fixed color pattern 1280 x 720
#define START_REG_ARRAY_TPG_RGB_888_HD_FIXED_COLOR \
{ \
    {0xB0, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xB3, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x24, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0F, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x00, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xE0, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xD0, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x02, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xFD, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x11, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x05, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x21, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x0A, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xA2, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x72, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0xE2, _tids90ub_pattern_gen_delay_}, \
    {0xB1, 0x01, _tids90ub_pattern_gen_delay_}, \
    {0xB2, 0x01, _tids90ub_pattern_gen_delay_}, \
}

typedef enum
{
    TIDS90UB_TPG_MODE_DEFAULT = 0,
    TIDS90UB_TPG_MODE_2MP = TIDS90UB_TPG_MODE_DEFAULT,
    TIDS90UB_TPG_MODE_4MP,
    TIDS90UB_TPG_MODE_8MP,
    TIDS90UB_TPG_MODE_RGB_888_FHD_COLOR_BAR,
    TIDS90UB_TPG_MODE_RGB_888_HD_COLOR_BAR,
    TIDS90UB_TPG_MODE_RGB_888_FHD_FIXED_COLOR,
    TIDS90UB_TPG_MODE_RGB_888_HD_FIXED_COLOR,
    TIDS90UB_TPG_MODE_MAX
}tids90ub_tpg_mode_t;

#endif /* __TIDS90UB_PATTERN_GEN_H__ */
