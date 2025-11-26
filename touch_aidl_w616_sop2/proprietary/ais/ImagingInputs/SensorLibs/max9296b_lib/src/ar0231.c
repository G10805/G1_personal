/**
 * @file ar0231.c
 *
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ar0231.h"

/*CONFIGURATION OPTIONS*/
//#define AR0231_12BIT_LINEAR
//#define AR0231_20BIT_HDR_MODE
#define AR0231_12BIT_HDR_MODE
//#define AR0231_16BIT_HDR_MODE

#ifndef AR0231_20BIT_HDR_MODE
#define SENSOR_WIDTH    1928
#define SENSOR_HEIGHT   1208
#else
#define SENSOR_WIDTH    1928*2
#define SENSOR_HEIGHT   1208
#endif /*AR0231_20BIT_HDR_MODE*/

#define COARSE_INTEGRATION_TIME 0x0288
#define FINE_INTEGRATION_TIME 0x032A

#define _max9295_delay_ 0
#define _ar0231_delay_ 0
#define MAX9295_LINK_RESET_DELAY 100000
#define MAX9295_START_DELAY 20000

#define CAM_SENSOR_DEFAULT_ADDR    0x20

#if defined(AR0231_12BIT_LINEAR) || defined(AR0231_12BIT_HDR_MODE)
#define FMT_LINK    QCARCAM_FMT_MIPIRAW_12
#define DT_LINK     CSI_DT_RAW12
#elif defined(AR0231_16BIT_HDR_MODE)
#define FMT_LINK    QCARCAM_FMT_MIPIRAW_16
#define DT_LINK     CSI_DT_RAW16
#elif defined(AR0231_20BIT_HDR_MODE)
#define FMT_LINK    QCARCAM_FMT_MIPIRAW_20
#define DT_LINK     CSI_DT_RAW20
#endif /*AR0231_20BIT_HDR_MODE*/

#define CID_VC0        0
#define CID_VC1        4


#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#define AR0231_PACK_ANALOG_GAIN(_g_) \
    (((_g_) << 12) | ((_g_) << 8) | ((_g_) << 4) | (_g_))


#define AR0231_REG_GROUPED_PARAMETER_HOLD 0x3022
#define AR0231_REG_COARSE_INTEGRATION_TIME 0x3012
#define AR0231_REG_COARSE_INTEGRATION_TIME2 0x3212
#define AR0231_REG_COARSE_INTEGRATION_TIME3 0x3216
#define AR0231_REG_FINE_INTEGRATION_TIME 0x3014
#define AR0231_REG_FINE_INTEGRATION_TIME2 0x321E
#define AR0231_REG_FINE_INTEGRATION_TIME3 0x3222

#define AR0231_REG_ANALOG_GAIN 0x3366
#define AR0231_REG_DC_GAIN 0x3362
#define AR0231_REG_DIG_GLOBAL_GAIN 0x3308


#ifdef DEBUG_SINGLE_SENSOR

#define CAM_SER_INIT_A \
{ \
    { 0x0002, 0x03, _max9295_delay_ }, \
    { 0x02BE, 0x90, _max9295_delay_ }, /*set sensor_rst GPIO*/ \
    { 0x03F1, 0x89, _max9295_delay_ }, \
    { 0x0308, 0x7F, _max9295_delay_ }, /*set port for max9295b*/ \
    { 0x0330, 0x00, _max9295_delay_ }, /*set 1x4 input mode for max9295b*/ \
    { 0x0332, 0xE0, _max9295_delay_ }, /*set 1x4 input mode for max9295b*/ \
    { 0x0331, 0x33, _max9295_delay_ } /*set 1x4 input mode for max9295b*/ \
}

#else

/* Difference between link A and link B is the stream ID set in register 0x53 */
#define CAM_SER_INIT_A \
{ \
    { 0x0002, 0x03, _max9295_delay_ }, \
    { 0x0330, 0x00, _max9295_delay_ }, \
    { 0x0332, 0xEE, _max9295_delay_ }, \
    { 0x0333, 0xE4, _max9295_delay_ }, \
    { 0x0331, 0x33, _max9295_delay_ }, \
    { 0x0311, 0x10, _max9295_delay_ }, \
    { 0x0308, 0x7F, _max9295_delay_ }, \
    { 0x0314, 0x6C, _max9295_delay_ }, \
    { 0x02BE, 0x90, _max9295_delay_ }, /*set sensor_rst GPIO*/ \
    { 0x03F1, 0x89, _max9295_delay_ } \
}

#endif /*DEBUG_SINGLE_SENSOR*/

#define CAM_SER_INIT_B \
{ \
    { 0x0002, 0x03, _max9295_delay_ }, \
    { 0x0053, 0x01, _max9295_delay_ }, \
    { 0x0330, 0x00, _max9295_delay_ }, \
    { 0x0332, 0xEE, _max9295_delay_ }, \
    { 0x0333, 0xE4, _max9295_delay_ }, \
    { 0x0331, 0x33, _max9295_delay_ }, \
    { 0x0311, 0x10, _max9295_delay_ }, \
    { 0x0308, 0x7F, _max9295_delay_ }, \
    { 0x0314, 0x6C, _max9295_delay_ }, \
    { 0x02BE, 0x90, _max9295_delay_ }, /*set sensor_rst GPIO*/ \
    { 0x03F1, 0x89, _max9295_delay_ } \
}


#define CAM_SER_START \
{ \
    { 0x0002, 0xF3, MAX9295_START_DELAY }, \
}

#define CAM_SER_STOP \
{ \
    { 0x0002, 0x03, MAX9295_START_DELAY }, \
}

#define CAM_SER_ADDR_CHNG_A \
{ \
    { 0x006B, 0x10, _max9295_delay_ }, \
    { 0x0073, 0x11, _max9295_delay_ }, \
    { 0x007B, 0x30, _max9295_delay_ }, \
    { 0x0083, 0x30, _max9295_delay_ }, \
    { 0x0093, 0x30, _max9295_delay_ }, \
    { 0x009B, 0x30, _max9295_delay_ }, \
    { 0x00A3, 0x30, _max9295_delay_ }, \
    { 0x00AB, 0x30, _max9295_delay_ }, \
    { 0x008B, 0x30, _max9295_delay_ }, \
}

#define CAM_SER_ADDR_CHNG_B \
{ \
    { 0x006B, 0x12, _max9295_delay_ }, \
    { 0x0073, 0x13, _max9295_delay_ }, \
    { 0x007B, 0x32, _max9295_delay_ }, \
    { 0x0083, 0x32, _max9295_delay_ }, \
    { 0x0093, 0x32, _max9295_delay_ }, \
    { 0x009B, 0x32, _max9295_delay_ }, \
    { 0x00A3, 0x32, _max9295_delay_ }, \
    { 0x00AB, 0x32, _max9295_delay_ }, \
    { 0x008B, 0x32, _max9295_delay_ }, \
}

#if defined(AR0231_12BIT_LINEAR)

#define AR0231_PCLK 99000000 /*99MHz*/
#define AR0231_LINE_LENGTH_PCK 2200 /*0x0898*/
#define AR0231_FRAME_LENGTH_LINES 1500 /*0x0A8C*/
#define AR0231_EXTRA_DELAY 0x0
#define X_ADDR_START 0
#define X_ADDR_END   0x787
#define Y_ADDR_START 0x0
#define Y_ADDR_END   0x4B7

#define CAM_SNSR_INIT \
{ \
    { 0x301A, 0x0018, 100 },         \
    { 0x3056, 0x0080, _ar0231_delay_ }, \
    { 0x3058, 0x0080, _ar0231_delay_ }, \
    { 0x305A, 0x0080, _ar0231_delay_ }, \
    { 0x305C, 0x0080, _ar0231_delay_ }, \
    { 0x3138, 0x000B, _ar0231_delay_ }, \
    { 0x30FE, 0x0020, _ar0231_delay_ }, \
    { 0x3372, 0xF54F, _ar0231_delay_ }, \
    { 0x337A, 0x0D70, _ar0231_delay_ }, \
    { 0x337E, 0x1FFD, _ar0231_delay_ }, \
    { 0x3382, 0x00C0, _ar0231_delay_ }, \
    { 0x3092, 0x0024, _ar0231_delay_ }, \
    { 0x3C04, 0x0E80, _ar0231_delay_ }, \
    { 0x3F90, 0x06E1, _ar0231_delay_ }, \
    { 0x3F92, 0x06E1, _ar0231_delay_ }, \
    { 0x350E, 0xFF14, _ar0231_delay_ }, \
    { 0x3506, 0x4444, _ar0231_delay_ }, \
    { 0x3508, 0x4444, _ar0231_delay_ }, \
    { 0x350A, 0x4465, _ar0231_delay_ }, \
    { 0x350C, 0x055F, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x3566, 0x1D28, _ar0231_delay_ }, \
    { 0x3518, 0x1FFE, _ar0231_delay_ }, \
    { 0x318E, 0x0200, _ar0231_delay_ }, \
    { 0x3190, 0x5000, _ar0231_delay_ }, \
    { 0x319E, 0x6060, _ar0231_delay_ }, \
    { 0x3520, 0x4688, _ar0231_delay_ }, \
    { 0x3522, 0x8840, _ar0231_delay_ }, \
    { 0x3524, 0x4046, _ar0231_delay_ }, \
    { 0x352C, 0xC6C6, _ar0231_delay_ }, \
    { 0x352A, 0x089F, _ar0231_delay_ }, \
    { 0x352E, 0x0011, _ar0231_delay_ }, \
    { 0x352E, 0x0011, _ar0231_delay_ }, \
    { 0x3530, 0x4400, _ar0231_delay_ }, \
    { 0x3530, 0x4400, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3536, 0xFF06, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x3538, 0xFFFF, _ar0231_delay_ }, \
    { 0x353A, 0x9000, _ar0231_delay_ }, \
    { 0x353C, 0x3F00, _ar0231_delay_ }, \
    { 0x353C, 0x3F00, _ar0231_delay_ }, \
    { 0x353C, 0x3F00, _ar0231_delay_ }, \
    { 0x353C, 0x3F00, _ar0231_delay_ }, \
    { 0x353C, 0x3F00, _ar0231_delay_ }, \
    { 0x353C, 0x3F00, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x3540, 0xC659, _ar0231_delay_ }, \
    { 0x3540, 0xC659, _ar0231_delay_ }, \
    { 0x3556, 0x101F, _ar0231_delay_ }, \
    { 0x3566, 0x1D28, _ar0231_delay_ }, \
    { 0x3566, 0x1D28, _ar0231_delay_ }, \
    { 0x3566, 0x1D28, _ar0231_delay_ }, \
    { 0x3566, 0x1128, _ar0231_delay_ }, \
    { 0x3566, 0x1328, _ar0231_delay_ }, \
    { 0x3566, 0x3328, _ar0231_delay_ }, \
    { 0x3528, 0xDDDD, _ar0231_delay_ }, \
    { 0x3540, 0xC63E, _ar0231_delay_ }, \
    { 0x3542, 0x545B, _ar0231_delay_ }, \
    { 0x3544, 0x645A, _ar0231_delay_ }, \
    { 0x3546, 0x5A5A, _ar0231_delay_ }, \
    { 0x3548, 0x6400, _ar0231_delay_ }, \
    { 0x301A, 0x10D8, 10 },          \
    { 0x2512, 0x8000, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x3350, _ar0231_delay_ }, \
    { 0x2510, 0x2004, _ar0231_delay_ }, \
    { 0x2510, 0x1460, _ar0231_delay_ }, \
    { 0x2510, 0x1578, _ar0231_delay_ }, \
    { 0x2510, 0x1360, _ar0231_delay_ }, \
    { 0x2510, 0x7B24, _ar0231_delay_ }, \
    { 0x2510, 0xFF24, _ar0231_delay_ }, \
    { 0x2510, 0xFF24, _ar0231_delay_ }, \
    { 0x2510, 0xEA24, _ar0231_delay_ }, \
    { 0x2510, 0x1022, _ar0231_delay_ }, \
    { 0x2510, 0x2410, _ar0231_delay_ }, \
    { 0x2510, 0x155A, _ar0231_delay_ }, \
    { 0x2510, 0x1342, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x24FF, _ar0231_delay_ }, \
    { 0x2510, 0x24FF, _ar0231_delay_ }, \
    { 0x2510, 0x24EA, _ar0231_delay_ }, \
    { 0x2510, 0x2324, _ar0231_delay_ }, \
    { 0x2510, 0x647A, _ar0231_delay_ }, \
    { 0x2510, 0x2404, _ar0231_delay_ }, \
    { 0x2510, 0x052C, _ar0231_delay_ }, \
    { 0x2510, 0x400A, _ar0231_delay_ }, \
    { 0x2510, 0xFF0A, _ar0231_delay_ }, \
    { 0x2510, 0xFF0A, _ar0231_delay_ }, \
    { 0x2510, 0x1808, _ar0231_delay_ }, \
    { 0x2510, 0x3851, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0004, _ar0231_delay_ }, \
    { 0x2510, 0x0801, _ar0231_delay_ }, \
    { 0x2510, 0x0408, _ar0231_delay_ }, \
    { 0x2510, 0x1180, _ar0231_delay_ }, \
    { 0x2510, 0x15DC, _ar0231_delay_ }, \
    { 0x2510, 0x134C, _ar0231_delay_ }, \
    { 0x2510, 0x1002, _ar0231_delay_ }, \
    { 0x2510, 0x1016, _ar0231_delay_ }, \
    { 0x2510, 0x1181, _ar0231_delay_ }, \
    { 0x2510, 0x1189, _ar0231_delay_ }, \
    { 0x2510, 0x1056, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x0D08, _ar0231_delay_ }, \
    { 0x2510, 0x0913, _ar0231_delay_ }, \
    { 0x2510, 0x13C8, _ar0231_delay_ }, \
    { 0x2510, 0x092B, _ar0231_delay_ }, \
    { 0x2510, 0x1588, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x1388, _ar0231_delay_ }, \
    { 0x2510, 0x0909, _ar0231_delay_ }, \
    { 0x2510, 0x11D9, _ar0231_delay_ }, \
    { 0x2510, 0x091D, _ar0231_delay_ }, \
    { 0x2510, 0x1441, _ar0231_delay_ }, \
    { 0x2510, 0x0903, _ar0231_delay_ }, \
    { 0x2510, 0x1214, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x10D6, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x1212, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x11DD, _ar0231_delay_ }, \
    { 0x2510, 0x11D9, _ar0231_delay_ }, \
    { 0x2510, 0x1056, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x11DB, _ar0231_delay_ }, \
    { 0x2510, 0x092B, _ar0231_delay_ }, \
    { 0x2510, 0x119B, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x121A, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x1460, _ar0231_delay_ }, \
    { 0x2510, 0x1250, _ar0231_delay_ }, \
    { 0x2510, 0x1076, _ar0231_delay_ }, \
    { 0x2510, 0x10E6, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x15AB, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x13A8, _ar0231_delay_ }, \
    { 0x2510, 0x1240, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x0923, _ar0231_delay_ }, \
    { 0x2510, 0x158D, _ar0231_delay_ }, \
    { 0x2510, 0x138D, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x0B09, _ar0231_delay_ }, \
    { 0x2510, 0x0108, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x091D, _ar0231_delay_ }, \
    { 0x2510, 0x1588, _ar0231_delay_ }, \
    { 0x2510, 0x1388, _ar0231_delay_ }, \
    { 0x2510, 0x092D, _ar0231_delay_ }, \
    { 0x2510, 0x1066, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x0C08, _ar0231_delay_ }, \
    { 0x2510, 0x090B, _ar0231_delay_ }, \
    { 0x2510, 0x1441, _ar0231_delay_ }, \
    { 0x2510, 0x090D, _ar0231_delay_ }, \
    { 0x2510, 0x10E6, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x1262, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x11BF, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x1066, _ar0231_delay_ }, \
    { 0x2510, 0x11FB, _ar0231_delay_ }, \
    { 0x2510, 0x0935, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x1263, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x1510, _ar0231_delay_ }, \
    { 0x2510, 0x11B8, _ar0231_delay_ }, \
    { 0x2510, 0x12A0, _ar0231_delay_ }, \
    { 0x2510, 0x1200, _ar0231_delay_ }, \
    { 0x2510, 0x1026, _ar0231_delay_ }, \
    { 0x2510, 0x1000, _ar0231_delay_ }, \
    { 0x2510, 0x1342, _ar0231_delay_ }, \
    { 0x2510, 0x1100, _ar0231_delay_ }, \
    { 0x2510, 0x7A06, _ar0231_delay_ }, \
    { 0x2510, 0x0913, _ar0231_delay_ }, \
    { 0x2510, 0x0507, _ar0231_delay_ }, \
    { 0x2510, 0x0841, _ar0231_delay_ }, \
    { 0x2510, 0x3750, _ar0231_delay_ }, \
    { 0x2510, 0x2C2C, _ar0231_delay_ }, \
    { 0x2510, 0xFE05, _ar0231_delay_ }, \
    { 0x2510, 0xFE13, _ar0231_delay_ }, \
    { 0x1008, 0x0361, _ar0231_delay_ }, \
    { 0x100C, 0x0589, _ar0231_delay_ }, \
    { 0x100E, 0x07B1, _ar0231_delay_ }, \
    { 0x1010, 0x0139, _ar0231_delay_ }, \
    { 0x3230, 0x0304, _ar0231_delay_ }, \
    { 0x3232, 0x052C, _ar0231_delay_ }, \
    { 0x3234, 0x0754, _ar0231_delay_ }, \
    { 0x3236, 0x00DC, _ar0231_delay_ }, \
    { 0x3566, 0x3328, _ar0231_delay_ }, \
    { 0x350C, 0x055F, _ar0231_delay_ }, \
    { 0x32D0, 0x3A02, _ar0231_delay_ }, \
    { 0x32D2, 0x3508, _ar0231_delay_ }, \
    { 0x32D4, 0x3702, _ar0231_delay_ }, \
    { 0x32D6, 0x3C04, _ar0231_delay_ }, \
    { 0x32DC, 0x370A, _ar0231_delay_ }, \
    { 0x302A, 0x0006, _ar0231_delay_ }, \
    { 0x302C, 0x0001, _ar0231_delay_ }, \
    { 0x302E, 0x0002, _ar0231_delay_ }, \
    { 0x3030, 0x002C, _ar0231_delay_ }, \
    { 0x3036, 0x000C, _ar0231_delay_ }, \
    { 0x3038, 0x0001, _ar0231_delay_ }, \
    { 0x30B0, 0x0A00, _ar0231_delay_ }, \
    { 0x30A2, 0x0001, _ar0231_delay_ }, \
    { 0x30A6, 0x0001, _ar0231_delay_ }, \
    { 0x3040, 0x0000, _ar0231_delay_ }, \
    { 0x3040, 0x0000, _ar0231_delay_ }, \
    { 0x3082, 0x0001, _ar0231_delay_ }, \
    { 0x3082, 0x0001, _ar0231_delay_ }, \
    { 0x3082, 0x0001, _ar0231_delay_ }, \
    { 0x3082, 0x0001, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3064, 0x1882, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x3180, 0x0080, _ar0231_delay_ }, \
    { 0x33E4, 0x0080, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x3004, X_ADDR_START, _ar0231_delay_ }, \
    { 0x3008, X_ADDR_END, _ar0231_delay_ }, \
    { 0x3002, Y_ADDR_START, _ar0231_delay_ }, \
    { 0x3006, Y_ADDR_END, _ar0231_delay_ }, \
    { 0x3032, 0x0000, _ar0231_delay_ }, \
    { 0x3400, 0x0010, _ar0231_delay_ }, \
    { 0x3402, 0x0788, _ar0231_delay_ }, \
    { 0x3402, 0x0F10, _ar0231_delay_ }, \
    { 0x3404, 0x04B8, _ar0231_delay_ }, \
    { 0x3404, 0x0970, _ar0231_delay_ }, \
    { 0x3082, 0x0001, _ar0231_delay_ }, \
    { 0x30BA, 0x11F1, 10},           \
    { 0x30BA, 0x11F0, _ar0231_delay_ }, /*extra delay off*/ \
    { 0x300C, AR0231_LINE_LENGTH_PCK, _ar0231_delay_ }, \
    { 0x300A, AR0231_FRAME_LENGTH_LINES, _ar0231_delay_ }, \
    { 0x3042, AR0231_EXTRA_DELAY, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3012, COARSE_INTEGRATION_TIME, _ar0231_delay_ }, \
    { 0x3014, FINE_INTEGRATION_TIME, _ar0231_delay_ }, \
    { 0x30B0, 0x0A00, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0C, _ar0231_delay_ }, \
    { 0x32EA, 0x3C08, _ar0231_delay_ }, \
    { 0x32EA, 0x3C08, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x31D0, 0x0000, _ar0231_delay_ }, \
    { 0x31AE, 0x0004, _ar0231_delay_ }, \
    { 0x31AE, 0x0304, _ar0231_delay_ }, \
    { 0x31AC, 0x140C, _ar0231_delay_ }, \
    { 0x31AC, 0x0C0C, _ar0231_delay_ }, \
    { 0x301A, 0x1098, _ar0231_delay_ }, \
    { 0x301A, 0x1018, _ar0231_delay_ }, \
    { 0x301A, 0x0018, _ar0231_delay_ }, \
    { 0x31AE, 0x0204, _ar0231_delay_ }, \
    { 0x3342, 0x122C, _ar0231_delay_ }, \
    { 0x3346, 0x122C, _ar0231_delay_ }, \
    { 0x334A, 0x122C, _ar0231_delay_ }, \
    { 0x334E, 0x122C, _ar0231_delay_ }, \
    { 0x3344, 0x0011, _ar0231_delay_ }, \
    { 0x3348, 0x0111, _ar0231_delay_ }, \
    { 0x334C, 0x0211, _ar0231_delay_ }, \
    { 0x3350, 0x0311, _ar0231_delay_ }, \
    { 0x31B0, 0x003A, _ar0231_delay_ }, \
    { 0x31B2, 0x0020, _ar0231_delay_ }, \
    { 0x31B4, 0x2876, _ar0231_delay_ }, \
    { 0x31B6, 0x2193, _ar0231_delay_ }, \
    { 0x31B8, 0x3048, _ar0231_delay_ }, \
    { 0x31BA, 0x0188, _ar0231_delay_ }, \
    { 0x31BC, 0x8006, _ar0231_delay_ }, \
    { 0x3366, 0x000A, _ar0231_delay_ }, \
    { 0x3306, 0x0400, _ar0231_delay_ }, \
    { 0x301A, 0x001C, 100 }, \
}

#elif defined(AR0231_20BIT_HDR_MODE)

/*20 BIT MODE*/
#define CAM_SNSR_INIT \
{ \
    { 0x301A, 0x10D8, 100 }, \
    { 0x3092, 0x0C24, _ar0231_delay_ }, \
    { 0x337A, 0x0C80, _ar0231_delay_ }, \
    { 0x3520, 0x1288, _ar0231_delay_ }, \
    { 0x3522, 0x880C, _ar0231_delay_ }, \
    { 0x3524, 0x0C12, _ar0231_delay_ }, \
    { 0x352C, 0x1212, _ar0231_delay_ }, \
    { 0x354A, 0x007F, _ar0231_delay_ }, \
    { 0x350C, 0x055C, _ar0231_delay_ }, \
    { 0x3506, 0x3333, _ar0231_delay_ }, \
    { 0x3508, 0x3333, _ar0231_delay_ }, \
    { 0x3100, 0x4000, _ar0231_delay_ }, \
    { 0x3280, 0x0FA0, _ar0231_delay_ }, \
    { 0x3282, 0x0FA0, _ar0231_delay_ }, \
    { 0x3284, 0x0FA0, _ar0231_delay_ }, \
    { 0x3286, 0x0FA0, _ar0231_delay_ }, \
    { 0x3288, 0x0FA0, _ar0231_delay_ }, \
    { 0x328A, 0x0FA0, _ar0231_delay_ }, \
    { 0x328C, 0x0FA0, _ar0231_delay_ }, \
    { 0x328E, 0x0FA0, _ar0231_delay_ }, \
    { 0x3290, 0x0FA0, _ar0231_delay_ }, \
    { 0x3292, 0x0FA0, _ar0231_delay_ }, \
    { 0x3294, 0x0FA0, _ar0231_delay_ }, \
    { 0x3296, 0x0FA0, _ar0231_delay_ }, \
    { 0x3298, 0x0FA0, _ar0231_delay_ }, \
    { 0x329A, 0x0FA0, _ar0231_delay_ }, \
    { 0x329C, 0x0FA0, _ar0231_delay_ }, \
    { 0x329E, 0x0FA0, _ar0231_delay_ }, \
    { 0x301A, 0x10D8, 10 }, \
    { 0x2512, 0x8000, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x3350, _ar0231_delay_ }, \
    { 0x2510, 0x2004, _ar0231_delay_ }, \
    { 0x2510, 0x1460, _ar0231_delay_ }, \
    { 0x2510, 0x1578, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x7B24, _ar0231_delay_ }, \
    { 0x2510, 0xFF24, _ar0231_delay_ }, \
    { 0x2510, 0xFF24, _ar0231_delay_ }, \
    { 0x2510, 0xEA24, _ar0231_delay_ }, \
    { 0x2510, 0x1022, _ar0231_delay_ }, \
    { 0x2510, 0x2410, _ar0231_delay_ }, \
    { 0x2510, 0x155A, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x24FF, _ar0231_delay_ }, \
    { 0x2510, 0x24FF, _ar0231_delay_ }, \
    { 0x2510, 0x24EA, _ar0231_delay_ }, \
    { 0x2510, 0x2324, _ar0231_delay_ }, \
    { 0x2510, 0x647A, _ar0231_delay_ }, \
    { 0x2510, 0x2404, _ar0231_delay_ }, \
    { 0x2510, 0x052C, _ar0231_delay_ }, \
    { 0x2510, 0x400A, _ar0231_delay_ }, \
    { 0x2510, 0xFF0A, _ar0231_delay_ }, \
    { 0x2510, 0xFF0A, _ar0231_delay_ }, \
    { 0x2510, 0x1008, _ar0231_delay_ }, \
    { 0x2510, 0x3851, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0004, _ar0231_delay_ }, \
    { 0x2510, 0x0801, _ar0231_delay_ }, \
    { 0x2510, 0x0408, _ar0231_delay_ }, \
    { 0x2510, 0x1180, _ar0231_delay_ }, \
    { 0x2510, 0x2652, _ar0231_delay_ }, \
    { 0x2510, 0x1518, _ar0231_delay_ }, \
    { 0x2510, 0x0906, _ar0231_delay_ }, \
    { 0x2510, 0x1348, _ar0231_delay_ }, \
    { 0x2510, 0x1002, _ar0231_delay_ }, \
    { 0x2510, 0x1016, _ar0231_delay_ }, \
    { 0x2510, 0x1181, _ar0231_delay_ }, \
    { 0x2510, 0x1189, _ar0231_delay_ }, \
    { 0x2510, 0x1056, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x0D09, _ar0231_delay_ }, \
    { 0x2510, 0x1413, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x2B15, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x0311, _ar0231_delay_ }, \
    { 0x2510, 0xD909, _ar0231_delay_ }, \
    { 0x2510, 0x1214, _ar0231_delay_ }, \
    { 0x2510, 0x4109, _ar0231_delay_ }, \
    { 0x2510, 0x0312, _ar0231_delay_ }, \
    { 0x2510, 0x1409, _ar0231_delay_ }, \
    { 0x2510, 0x0110, _ar0231_delay_ }, \
    { 0x2510, 0xD612, _ar0231_delay_ }, \
    { 0x2510, 0x1012, _ar0231_delay_ }, \
    { 0x2510, 0x1212, _ar0231_delay_ }, \
    { 0x2510, 0x1011, _ar0231_delay_ }, \
    { 0x2510, 0xDD11, _ar0231_delay_ }, \
    { 0x2510, 0xD910, _ar0231_delay_ }, \
    { 0x2510, 0x5609, _ar0231_delay_ }, \
    { 0x2510, 0x1511, _ar0231_delay_ }, \
    { 0x2510, 0xDB09, _ar0231_delay_ }, \
    { 0x2510, 0x1511, _ar0231_delay_ }, \
    { 0x2510, 0x9B09, _ar0231_delay_ }, \
    { 0x2510, 0x0F11, _ar0231_delay_ }, \
    { 0x2510, 0xBB12, _ar0231_delay_ }, \
    { 0x2510, 0x1A12, _ar0231_delay_ }, \
    { 0x2510, 0x1014, _ar0231_delay_ }, \
    { 0x2510, 0x6012, _ar0231_delay_ }, \
    { 0x2510, 0x5010, _ar0231_delay_ }, \
    { 0x2510, 0x7610, _ar0231_delay_ }, \
    { 0x2510, 0xE609, _ar0231_delay_ }, \
    { 0x2510, 0x0812, _ar0231_delay_ }, \
    { 0x2510, 0x4012, _ar0231_delay_ }, \
    { 0x2510, 0x6009, _ar0231_delay_ }, \
    { 0x2510, 0x290B, _ar0231_delay_ }, \
    { 0x2510, 0x0904, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0923, _ar0231_delay_ }, \
    { 0x2510, 0x15C8, _ar0231_delay_ }, \
    { 0x2510, 0x13C8, _ar0231_delay_ }, \
    { 0x2510, 0x092C, _ar0231_delay_ }, \
    { 0x2510, 0x1588, _ar0231_delay_ }, \
    { 0x2510, 0x1388, _ar0231_delay_ }, \
    { 0x2510, 0x0C09, _ar0231_delay_ }, \
    { 0x2510, 0x0C14, _ar0231_delay_ }, \
    { 0x2510, 0x4109, _ar0231_delay_ }, \
    { 0x2510, 0x1112, _ar0231_delay_ }, \
    { 0x2510, 0x6212, _ar0231_delay_ }, \
    { 0x2510, 0x6011, _ar0231_delay_ }, \
    { 0x2510, 0xBF11, _ar0231_delay_ }, \
    { 0x2510, 0xBB10, _ar0231_delay_ }, \
    { 0x2510, 0x6611, _ar0231_delay_ }, \
    { 0x2510, 0xFB09, _ar0231_delay_ }, \
    { 0x2510, 0x3511, _ar0231_delay_ }, \
    { 0x2510, 0xBB12, _ar0231_delay_ }, \
    { 0x2510, 0x6312, _ar0231_delay_ }, \
    { 0x2510, 0x6014, _ar0231_delay_ }, \
    { 0x2510, 0x0015, _ar0231_delay_ }, \
    { 0x2510, 0x0011, _ar0231_delay_ }, \
    { 0x2510, 0xB812, _ar0231_delay_ }, \
    { 0x2510, 0xA012, _ar0231_delay_ }, \
    { 0x2510, 0x0010, _ar0231_delay_ }, \
    { 0x2510, 0x2610, _ar0231_delay_ }, \
    { 0x2510, 0x0013, _ar0231_delay_ }, \
    { 0x2510, 0x0011, _ar0231_delay_ }, \
    { 0x2510, 0x0008, _ar0231_delay_ }, \
    { 0x2510, 0x3053, _ar0231_delay_ }, \
    { 0x2510, 0x4215, _ar0231_delay_ }, \
    { 0x2510, 0x4013, _ar0231_delay_ }, \
    { 0x2510, 0x4010, _ar0231_delay_ }, \
    { 0x2510, 0x0210, _ar0231_delay_ }, \
    { 0x2510, 0x1611, _ar0231_delay_ }, \
    { 0x2510, 0x8111, _ar0231_delay_ }, \
    { 0x2510, 0x8910, _ar0231_delay_ }, \
    { 0x2510, 0x5612, _ar0231_delay_ }, \
    { 0x2510, 0x1009, _ar0231_delay_ }, \
    { 0x2510, 0x010D, _ar0231_delay_ }, \
    { 0x2510, 0x0815, _ar0231_delay_ }, \
    { 0x2510, 0xC015, _ar0231_delay_ }, \
    { 0x2510, 0xD013, _ar0231_delay_ }, \
    { 0x2510, 0x5009, _ar0231_delay_ }, \
    { 0x2510, 0x1313, _ar0231_delay_ }, \
    { 0x2510, 0xD009, _ar0231_delay_ }, \
    { 0x2510, 0x0215, _ar0231_delay_ }, \
    { 0x2510, 0xC015, _ar0231_delay_ }, \
    { 0x2510, 0xC813, _ar0231_delay_ }, \
    { 0x2510, 0xC009, _ar0231_delay_ }, \
    { 0x2510, 0x0515, _ar0231_delay_ }, \
    { 0x2510, 0x8813, _ar0231_delay_ }, \
    { 0x2510, 0x8009, _ar0231_delay_ }, \
    { 0x2510, 0x0213, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x0411, _ar0231_delay_ }, \
    { 0x2510, 0xC909, _ar0231_delay_ }, \
    { 0x2510, 0x0814, _ar0231_delay_ }, \
    { 0x2510, 0x0109, _ar0231_delay_ }, \
    { 0x2510, 0x0B11, _ar0231_delay_ }, \
    { 0x2510, 0xD908, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x091A, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0903, _ar0231_delay_ }, \
    { 0x2510, 0x1214, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x10D6, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x1212, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x11DD, _ar0231_delay_ }, \
    { 0x2510, 0x11D9, _ar0231_delay_ }, \
    { 0x2510, 0x1056, _ar0231_delay_ }, \
    { 0x2510, 0x0917, _ar0231_delay_ }, \
    { 0x2510, 0x11DB, _ar0231_delay_ }, \
    { 0x2510, 0x0913, _ar0231_delay_ }, \
    { 0x2510, 0x11FB, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x121A, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x1460, _ar0231_delay_ }, \
    { 0x2510, 0x1250, _ar0231_delay_ }, \
    { 0x2510, 0x1076, _ar0231_delay_ }, \
    { 0x2510, 0x10E6, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x15A8, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x13A8, _ar0231_delay_ }, \
    { 0x2510, 0x1240, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x0925, _ar0231_delay_ }, \
    { 0x2510, 0x13AD, _ar0231_delay_ }, \
    { 0x2510, 0x0902, _ar0231_delay_ }, \
    { 0x2510, 0x0907, _ar0231_delay_ }, \
    { 0x2510, 0x1588, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x138D, _ar0231_delay_ }, \
    { 0x2510, 0x0B09, _ar0231_delay_ }, \
    { 0x2510, 0x0914, _ar0231_delay_ }, \
    { 0x2510, 0x4009, _ar0231_delay_ }, \
    { 0x2510, 0x0B13, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x1C0C, _ar0231_delay_ }, \
    { 0x2510, 0x0920, _ar0231_delay_ }, \
    { 0x2510, 0x1262, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x11BF, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x1066, _ar0231_delay_ }, \
    { 0x2510, 0x090A, _ar0231_delay_ }, \
    { 0x2510, 0x11FB, _ar0231_delay_ }, \
    { 0x2510, 0x093B, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x1263, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x1508, _ar0231_delay_ }, \
    { 0x2510, 0x11B8, _ar0231_delay_ }, \
    { 0x2510, 0x12A0, _ar0231_delay_ }, \
    { 0x2510, 0x1200, _ar0231_delay_ }, \
    { 0x2510, 0x1026, _ar0231_delay_ }, \
    { 0x2510, 0x1000, _ar0231_delay_ }, \
    { 0x2510, 0x1300, _ar0231_delay_ }, \
    { 0x2510, 0x1100, _ar0231_delay_ }, \
    { 0x2510, 0x437A, _ar0231_delay_ }, \
    { 0x2510, 0x0609, _ar0231_delay_ }, \
    { 0x2510, 0x0B05, _ar0231_delay_ }, \
    { 0x2510, 0x0708, _ar0231_delay_ }, \
    { 0x2510, 0x4137, _ar0231_delay_ }, \
    { 0x2510, 0x502C, _ar0231_delay_ }, \
    { 0x2510, 0x2CFE, _ar0231_delay_ }, \
    { 0x2510, 0x15FE, _ar0231_delay_ }, \
    { 0x2510, 0x0C2C, _ar0231_delay_ }, \
    { 0x32E6, 0x00E0, _ar0231_delay_ }, \
    { 0x1008, 0x036F, _ar0231_delay_ }, \
    { 0x100C, 0x058F, _ar0231_delay_ }, \
    { 0x100E, 0x07AF, _ar0231_delay_ }, \
    { 0x1010, 0x014F, _ar0231_delay_ }, \
    { 0x3230, 0x0312, _ar0231_delay_ }, \
    { 0x3232, 0x0532, _ar0231_delay_ }, \
    { 0x3234, 0x0752, _ar0231_delay_ }, \
    { 0x3236, 0x00F2, _ar0231_delay_ }, \
    { 0x3566, 0x3328, _ar0231_delay_ }, \
    { 0x32D0, 0x3A02, _ar0231_delay_ }, \
    { 0x32D2, 0x3508, _ar0231_delay_ }, \
    { 0x32D4, 0x3702, _ar0231_delay_ }, \
    { 0x32D6, 0x3C04, _ar0231_delay_ }, \
    { 0x32DC, 0x370A, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x302A, 0x000A, _ar0231_delay_ }, \
    { 0x302C, 0x0001, _ar0231_delay_ }, \
    { 0x302E, 0x0003, _ar0231_delay_ }, \
    { 0x3030, 0x005D, _ar0231_delay_ }, \
    { 0x3036, 0x000A, _ar0231_delay_ }, \
    { 0x3038, 0x0001, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x30A2, 0x0001, _ar0231_delay_ }, \
    { 0x30A6, 0x0001, _ar0231_delay_ }, \
    { 0x3040, 0x0000, _ar0231_delay_ }, \
    { 0x3040, 0x0000, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x3180, 0x0080, _ar0231_delay_ }, \
    { 0x33E4, 0x0080, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x3004, 0x0000, _ar0231_delay_ }, \
    { 0x3008, 0x0787, _ar0231_delay_ }, \
    { 0x3002, 0x0000, _ar0231_delay_ }, \
    { 0x3006, 0x04B7, _ar0231_delay_ }, \
    { 0x3032, 0x0000, _ar0231_delay_ }, \
    { 0x3400, 0x0010, _ar0231_delay_ }, \
    { 0x3402, 0x0788, _ar0231_delay_ }, \
    { 0x3402, 0x0F10, _ar0231_delay_ }, \
    { 0x3404, 0x04B8, _ar0231_delay_ }, \
    { 0x3404, 0x0970, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x300C, 0x068A, _ar0231_delay_ }, \
    { 0x300A, 0x0522, _ar0231_delay_ }, \
    { 0x3042, 0x0000, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3012, 0x0163, _ar0231_delay_ }, \
    { 0x3014, 0x0752, _ar0231_delay_ }, \
    { 0x321E, 0x0752, _ar0231_delay_ }, \
    { 0x3222, 0x0752, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x31D0, 0x0001, _ar0231_delay_ }, \
    { 0x31AE, 0x0004, _ar0231_delay_ }, \
    { 0x31AE, 0x0304, _ar0231_delay_ }, \
    { 0x31AC, 0x1414, _ar0231_delay_ }, \
    { 0x31AC, 0x1414, _ar0231_delay_ }, \
    { 0x301A, 0x1098, _ar0231_delay_ }, \
    { 0x301A, 0x1018, _ar0231_delay_ }, \
    { 0x301A, 0x0018, _ar0231_delay_ }, \
    { 0x31AE, 0x0204, _ar0231_delay_ }, \
    { 0x3342, 0x122B, _ar0231_delay_ }, \
    { 0x3346, 0x122B, _ar0231_delay_ }, \
    { 0x334A, 0x122B, _ar0231_delay_ }, \
    { 0x334E, 0x122B, _ar0231_delay_ }, \
    { 0x3344, 0x0011, _ar0231_delay_ }, \
    { 0x3348, 0x0111, _ar0231_delay_ }, \
    { 0x334C, 0x0211, _ar0231_delay_ }, \
    { 0x3350, 0x0311, _ar0231_delay_ }, \
    { 0x31B0, 0x0066, _ar0231_delay_ }, \
    { 0x31B2, 0x0045, _ar0231_delay_ }, \
    { 0x31B4, 0x2207, _ar0231_delay_ }, \
    { 0x31B6, 0x220A, _ar0231_delay_ }, \
    { 0x31B8, 0x404A, _ar0231_delay_ }, \
    { 0x31BA, 0x028A, _ar0231_delay_ }, \
    { 0x31BC, 0x0C08, _ar0231_delay_ }, \
    { 0x300A, 0x05C8, _ar0231_delay_ }, \
    { 0x301A, 0x001C, 100 }, \
}

#elif defined(AR0231_12BIT_HDR_MODE)

//OnSemi Recommended

#define AR0231_REF_CLK 27000000
#define AR0231_PLL_MULTIPLIER 0x2C
#define AR0231_PRE_PLL_CLK_DIV 2
#define AR0231_VT_SYS_CLK_DIV  1
#define AR0231_VT_PIX_CLK_DIV  6

#define AR0231_PCLK 99000000 /*99MHz*/

#define AR0231_LINE_LENGTH_PCK 0x9CE /*0x300C*/
#define AR0231_FRAME_LENGTH_LINES 0x522 /*0x300A*/
#define AR0231_EXTRA_DELAY 0x0
#define X_ADDR_START 0
#define X_ADDR_END   0x787
#define Y_ADDR_START 0x0
#define Y_ADDR_END   0x4B7

#define CAM_SNSR_INIT \
{ \
    { 0x301A, 0x0018, 100 }, \
    { 0x3092, 0x0C24, _ar0231_delay_ }, \
    { 0x337A, 0x0C80, _ar0231_delay_ }, \
    { 0x3520, 0x1288, _ar0231_delay_ }, \
    { 0x3522, 0x880C, _ar0231_delay_ }, \
    { 0x3524, 0x0C12, _ar0231_delay_ }, \
    { 0x352C, 0x1212, _ar0231_delay_ }, \
    { 0x354A, 0x007F, _ar0231_delay_ }, \
    { 0x350C, 0x055C, _ar0231_delay_ }, \
    { 0x3506, 0x3333, _ar0231_delay_ }, \
    { 0x3508, 0x3333, _ar0231_delay_ }, \
    { 0x3100, 0x4000, _ar0231_delay_ }, \
    { 0x3280, 0x0FA0, _ar0231_delay_ }, \
    { 0x3282, 0x0FA0, _ar0231_delay_ }, \
    { 0x3284, 0x0FA0, _ar0231_delay_ }, \
    { 0x3286, 0x0FA0, _ar0231_delay_ }, \
    { 0x3288, 0x0FA0, _ar0231_delay_ }, \
    { 0x328A, 0x0FA0, _ar0231_delay_ }, \
    { 0x328C, 0x0FA0, _ar0231_delay_ }, \
    { 0x328E, 0x0FA0, _ar0231_delay_ }, \
    { 0x3290, 0x0FA0, _ar0231_delay_ }, \
    { 0x3292, 0x0FA0, _ar0231_delay_ }, \
    { 0x3294, 0x0FA0, _ar0231_delay_ }, \
    { 0x3296, 0x0FA0, _ar0231_delay_ }, \
    { 0x3298, 0x0FA0, _ar0231_delay_ }, \
    { 0x329A, 0x0FA0, _ar0231_delay_ }, \
    { 0x329C, 0x0FA0, _ar0231_delay_ }, \
    { 0x329E, 0x0FA0, _ar0231_delay_ }, \
    { 0x301A, 0x10D8, 100 }, \
    { 0x2512, 0x8000, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x3350, _ar0231_delay_ }, \
    { 0x2510, 0x2004, _ar0231_delay_ }, \
    { 0x2510, 0x1460, _ar0231_delay_ }, \
    { 0x2510, 0x1578, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x7B24, _ar0231_delay_ }, \
    { 0x2510, 0xFF24, _ar0231_delay_ }, \
    { 0x2510, 0xFF24, _ar0231_delay_ }, \
    { 0x2510, 0xEA24, _ar0231_delay_ }, \
    { 0x2510, 0x1022, _ar0231_delay_ }, \
    { 0x2510, 0x2410, _ar0231_delay_ }, \
    { 0x2510, 0x155A, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x24FF, _ar0231_delay_ }, \
    { 0x2510, 0x24FF, _ar0231_delay_ }, \
    { 0x2510, 0x24EA, _ar0231_delay_ }, \
    { 0x2510, 0x2324, _ar0231_delay_ }, \
    { 0x2510, 0x647A, _ar0231_delay_ }, \
    { 0x2510, 0x2404, _ar0231_delay_ }, \
    { 0x2510, 0x052C, _ar0231_delay_ }, \
    { 0x2510, 0x400A, _ar0231_delay_ }, \
    { 0x2510, 0xFF0A, _ar0231_delay_ }, \
    { 0x2510, 0xFF0A, _ar0231_delay_ }, \
    { 0x2510, 0x1008, _ar0231_delay_ }, \
    { 0x2510, 0x3851, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0004, _ar0231_delay_ }, \
    { 0x2510, 0x0801, _ar0231_delay_ }, \
    { 0x2510, 0x0408, _ar0231_delay_ }, \
    { 0x2510, 0x1180, _ar0231_delay_ }, \
    { 0x2510, 0x2652, _ar0231_delay_ }, \
    { 0x2510, 0x1518, _ar0231_delay_ }, \
    { 0x2510, 0x0906, _ar0231_delay_ }, \
    { 0x2510, 0x1348, _ar0231_delay_ }, \
    { 0x2510, 0x1002, _ar0231_delay_ }, \
    { 0x2510, 0x1016, _ar0231_delay_ }, \
    { 0x2510, 0x1181, _ar0231_delay_ }, \
    { 0x2510, 0x1189, _ar0231_delay_ }, \
    { 0x2510, 0x1056, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x0D09, _ar0231_delay_ }, \
    { 0x2510, 0x1413, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x2B15, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x0311, _ar0231_delay_ }, \
    { 0x2510, 0xD909, _ar0231_delay_ }, \
    { 0x2510, 0x1214, _ar0231_delay_ }, \
    { 0x2510, 0x4109, _ar0231_delay_ }, \
    { 0x2510, 0x0312, _ar0231_delay_ }, \
    { 0x2510, 0x1409, _ar0231_delay_ }, \
    { 0x2510, 0x0110, _ar0231_delay_ }, \
    { 0x2510, 0xD612, _ar0231_delay_ }, \
    { 0x2510, 0x1012, _ar0231_delay_ }, \
    { 0x2510, 0x1212, _ar0231_delay_ }, \
    { 0x2510, 0x1011, _ar0231_delay_ }, \
    { 0x2510, 0xDD11, _ar0231_delay_ }, \
    { 0x2510, 0xD910, _ar0231_delay_ }, \
    { 0x2510, 0x5609, _ar0231_delay_ }, \
    { 0x2510, 0x1511, _ar0231_delay_ }, \
    { 0x2510, 0xDB09, _ar0231_delay_ }, \
    { 0x2510, 0x1511, _ar0231_delay_ }, \
    { 0x2510, 0x9B09, _ar0231_delay_ }, \
    { 0x2510, 0x0F11, _ar0231_delay_ }, \
    { 0x2510, 0xBB12, _ar0231_delay_ }, \
    { 0x2510, 0x1A12, _ar0231_delay_ }, \
    { 0x2510, 0x1014, _ar0231_delay_ }, \
    { 0x2510, 0x6012, _ar0231_delay_ }, \
    { 0x2510, 0x5010, _ar0231_delay_ }, \
    { 0x2510, 0x7610, _ar0231_delay_ }, \
    { 0x2510, 0xE609, _ar0231_delay_ }, \
    { 0x2510, 0x0812, _ar0231_delay_ }, \
    { 0x2510, 0x4012, _ar0231_delay_ }, \
    { 0x2510, 0x6009, _ar0231_delay_ }, \
    { 0x2510, 0x290B, _ar0231_delay_ }, \
    { 0x2510, 0x0904, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0923, _ar0231_delay_ }, \
    { 0x2510, 0x15C8, _ar0231_delay_ }, \
    { 0x2510, 0x13C8, _ar0231_delay_ }, \
    { 0x2510, 0x092C, _ar0231_delay_ }, \
    { 0x2510, 0x1588, _ar0231_delay_ }, \
    { 0x2510, 0x1388, _ar0231_delay_ }, \
    { 0x2510, 0x0C09, _ar0231_delay_ }, \
    { 0x2510, 0x0C14, _ar0231_delay_ }, \
    { 0x2510, 0x4109, _ar0231_delay_ }, \
    { 0x2510, 0x1112, _ar0231_delay_ }, \
    { 0x2510, 0x6212, _ar0231_delay_ }, \
    { 0x2510, 0x6011, _ar0231_delay_ }, \
    { 0x2510, 0xBF11, _ar0231_delay_ }, \
    { 0x2510, 0xBB10, _ar0231_delay_ }, \
    { 0x2510, 0x6611, _ar0231_delay_ }, \
    { 0x2510, 0xFB09, _ar0231_delay_ }, \
    { 0x2510, 0x3511, _ar0231_delay_ }, \
    { 0x2510, 0xBB12, _ar0231_delay_ }, \
    { 0x2510, 0x6312, _ar0231_delay_ }, \
    { 0x2510, 0x6014, _ar0231_delay_ }, \
    { 0x2510, 0x0015, _ar0231_delay_ }, \
    { 0x2510, 0x0011, _ar0231_delay_ }, \
    { 0x2510, 0xB812, _ar0231_delay_ }, \
    { 0x2510, 0xA012, _ar0231_delay_ }, \
    { 0x2510, 0x0010, _ar0231_delay_ }, \
    { 0x2510, 0x2610, _ar0231_delay_ }, \
    { 0x2510, 0x0013, _ar0231_delay_ }, \
    { 0x2510, 0x0011, _ar0231_delay_ }, \
    { 0x2510, 0x0008, _ar0231_delay_ }, \
    { 0x2510, 0x3053, _ar0231_delay_ }, \
    { 0x2510, 0x4215, _ar0231_delay_ }, \
    { 0x2510, 0x4013, _ar0231_delay_ }, \
    { 0x2510, 0x4010, _ar0231_delay_ }, \
    { 0x2510, 0x0210, _ar0231_delay_ }, \
    { 0x2510, 0x1611, _ar0231_delay_ }, \
    { 0x2510, 0x8111, _ar0231_delay_ }, \
    { 0x2510, 0x8910, _ar0231_delay_ }, \
    { 0x2510, 0x5612, _ar0231_delay_ }, \
    { 0x2510, 0x1009, _ar0231_delay_ }, \
    { 0x2510, 0x010D, _ar0231_delay_ }, \
    { 0x2510, 0x0815, _ar0231_delay_ }, \
    { 0x2510, 0xC015, _ar0231_delay_ }, \
    { 0x2510, 0xD013, _ar0231_delay_ }, \
    { 0x2510, 0x5009, _ar0231_delay_ }, \
    { 0x2510, 0x1313, _ar0231_delay_ }, \
    { 0x2510, 0xD009, _ar0231_delay_ }, \
    { 0x2510, 0x0215, _ar0231_delay_ }, \
    { 0x2510, 0xC015, _ar0231_delay_ }, \
    { 0x2510, 0xC813, _ar0231_delay_ }, \
    { 0x2510, 0xC009, _ar0231_delay_ }, \
    { 0x2510, 0x0515, _ar0231_delay_ }, \
    { 0x2510, 0x8813, _ar0231_delay_ }, \
    { 0x2510, 0x8009, _ar0231_delay_ }, \
    { 0x2510, 0x0213, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x0411, _ar0231_delay_ }, \
    { 0x2510, 0xC909, _ar0231_delay_ }, \
    { 0x2510, 0x0814, _ar0231_delay_ }, \
    { 0x2510, 0x0109, _ar0231_delay_ }, \
    { 0x2510, 0x0B11, _ar0231_delay_ }, \
    { 0x2510, 0xD908, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x091A, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0903, _ar0231_delay_ }, \
    { 0x2510, 0x1214, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x10D6, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x1212, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x11DD, _ar0231_delay_ }, \
    { 0x2510, 0x11D9, _ar0231_delay_ }, \
    { 0x2510, 0x1056, _ar0231_delay_ }, \
    { 0x2510, 0x0917, _ar0231_delay_ }, \
    { 0x2510, 0x11DB, _ar0231_delay_ }, \
    { 0x2510, 0x0913, _ar0231_delay_ }, \
    { 0x2510, 0x11FB, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x121A, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x1460, _ar0231_delay_ }, \
    { 0x2510, 0x1250, _ar0231_delay_ }, \
    { 0x2510, 0x1076, _ar0231_delay_ }, \
    { 0x2510, 0x10E6, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x15A8, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x13A8, _ar0231_delay_ }, \
    { 0x2510, 0x1240, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x0925, _ar0231_delay_ }, \
    { 0x2510, 0x13AD, _ar0231_delay_ }, \
    { 0x2510, 0x0902, _ar0231_delay_ }, \
    { 0x2510, 0x0907, _ar0231_delay_ }, \
    { 0x2510, 0x1588, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x138D, _ar0231_delay_ }, \
    { 0x2510, 0x0B09, _ar0231_delay_ }, \
    { 0x2510, 0x0914, _ar0231_delay_ }, \
    { 0x2510, 0x4009, _ar0231_delay_ }, \
    { 0x2510, 0x0B13, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x1C0C, _ar0231_delay_ }, \
    { 0x2510, 0x0920, _ar0231_delay_ }, \
    { 0x2510, 0x1262, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x11BF, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x1066, _ar0231_delay_ }, \
    { 0x2510, 0x090A, _ar0231_delay_ }, \
    { 0x2510, 0x11FB, _ar0231_delay_ }, \
    { 0x2510, 0x093B, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x1263, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x1508, _ar0231_delay_ }, \
    { 0x2510, 0x11B8, _ar0231_delay_ }, \
    { 0x2510, 0x12A0, _ar0231_delay_ }, \
    { 0x2510, 0x1200, _ar0231_delay_ }, \
    { 0x2510, 0x1026, _ar0231_delay_ }, \
    { 0x2510, 0x1000, _ar0231_delay_ }, \
    { 0x2510, 0x1300, _ar0231_delay_ }, \
    { 0x2510, 0x1100, _ar0231_delay_ }, \
    { 0x2510, 0x437A, _ar0231_delay_ }, \
    { 0x2510, 0x0609, _ar0231_delay_ }, \
    { 0x2510, 0x0B05, _ar0231_delay_ }, \
    { 0x2510, 0x0708, _ar0231_delay_ }, \
    { 0x2510, 0x4137, _ar0231_delay_ }, \
    { 0x2510, 0x502C, _ar0231_delay_ }, \
    { 0x2510, 0x2CFE, _ar0231_delay_ }, \
    { 0x2510, 0x15FE, _ar0231_delay_ }, \
    { 0x2510, 0x0C2C, _ar0231_delay_ }, \
    { 0x32E6, 0x00E0, _ar0231_delay_ }, \
    { 0x1008, 0x036F, _ar0231_delay_ }, \
    { 0x100C, 0x058F, _ar0231_delay_ }, \
    { 0x100E, 0x07AF, _ar0231_delay_ }, \
    { 0x1010, 0x014F, _ar0231_delay_ }, \
    { 0x3230, 0x0312, _ar0231_delay_ }, \
    { 0x3232, 0x0532, _ar0231_delay_ }, \
    { 0x3234, 0x0752, _ar0231_delay_ }, \
    { 0x3236, 0x00F2, _ar0231_delay_ }, \
    { 0x3566, 0x3328, _ar0231_delay_ }, \
    { 0x32D0, 0x3A02, _ar0231_delay_ }, \
    { 0x32D2, 0x3508, _ar0231_delay_ }, \
    { 0x32D4, 0x3702, _ar0231_delay_ }, \
    { 0x32D6, 0x3C04, _ar0231_delay_ }, \
    { 0x32DC, 0x370A, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x302A, AR0231_VT_PIX_CLK_DIV, _ar0231_delay_ }, \
    { 0x302C, AR0231_VT_SYS_CLK_DIV, _ar0231_delay_ }, \
    { 0x302E, AR0231_PRE_PLL_CLK_DIV, _ar0231_delay_ }, \
    { 0x3030, AR0231_PLL_MULTIPLIER, _ar0231_delay_ }, \
    { 0x3036, 0x000C, _ar0231_delay_ }, \
    { 0x3038, 0x0001, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x30A2, 0x0001, _ar0231_delay_ }, \
    { 0x30A6, 0x0001, _ar0231_delay_ }, \
    { 0x3040, 0x0000, _ar0231_delay_ }, \
    { 0x3040, 0x0000, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x3180, 0x0080, _ar0231_delay_ }, \
    { 0x33E4, 0x0080, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x3004, X_ADDR_START, _ar0231_delay_ }, \
    { 0x3008, X_ADDR_END, _ar0231_delay_ }, \
    { 0x3002, Y_ADDR_START, _ar0231_delay_ }, \
    { 0x3006, Y_ADDR_END, _ar0231_delay_ }, \
    { 0x3032, 0x0000, _ar0231_delay_ }, \
    { 0x3400, 0x0010, _ar0231_delay_ }, \
    { 0x3402, 0x0788, _ar0231_delay_ }, \
    { 0x3402, 0x0F10, _ar0231_delay_ }, \
    { 0x3404, 0x04B8, _ar0231_delay_ }, \
    { 0x3404, 0x0970, _ar0231_delay_ }, \
    { 0x3082, 0x0008, _ar0231_delay_ }, \
    { 0x30BA, 0x11F2, _ar0231_delay_ }, \
    { 0x300C, AR0231_LINE_LENGTH_PCK, _ar0231_delay_ }, \
    { 0x300A, AR0231_FRAME_LENGTH_LINES, _ar0231_delay_ }, \
    { 0x3042, 0x0000, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3012, 0x0163, _ar0231_delay_ }, \
    { 0x3014, 0x0752, _ar0231_delay_ }, \
    { 0x321E, 0x0752, _ar0231_delay_ }, \
    { 0x3222, 0x0752, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x33C0, 0x2000, _ar0231_delay_ }, /*OC_LUT_00*/\
    { 0x33C2, 0x3440, _ar0231_delay_ }, \
    { 0x33C4, 0x4890, _ar0231_delay_ }, \
    { 0x33C6, 0x5CE0, _ar0231_delay_ }, \
    { 0x33C8, 0x7140, _ar0231_delay_ }, \
    { 0x33CA, 0x8590, _ar0231_delay_ }, \
    { 0x33CC, 0x99E0, _ar0231_delay_ }, \
    { 0x33CE, 0xAE40, _ar0231_delay_ }, \
    { 0x33D0, 0xC290, _ar0231_delay_ }, \
    { 0x33D2, 0xD6F0, _ar0231_delay_ }, \
    { 0x33D4, 0xEB40, _ar0231_delay_ }, \
    { 0x33D6, 0x0000, _ar0231_delay_ }, \
    { 0x33DA, 0x0000, _ar0231_delay_ }, /*OC_LUT_CONTROL*/\
    { 0x31D0, 0x0001, _ar0231_delay_ }, \
    { 0x31AE, 0x0004, _ar0231_delay_ }, \
    { 0x31AE, 0x0304, _ar0231_delay_ }, \
    { 0x31AC, 0x140C, _ar0231_delay_ }, \
    { 0x31AC, 0x140C, _ar0231_delay_ }, \
    { 0x301A, 0x1098, _ar0231_delay_ }, \
    { 0x301A, 0x1018, _ar0231_delay_ }, \
    { 0x301A, 0x0018, _ar0231_delay_ }, \
    { 0x31AE, 0x0204, _ar0231_delay_ }, \
    { 0x3342, 0x122C, _ar0231_delay_ }, \
    { 0x3346, 0x122C, _ar0231_delay_ }, \
    { 0x334A, 0x122C, _ar0231_delay_ }, \
    { 0x334E, 0x122C, _ar0231_delay_ }, \
    { 0x3344, 0x0011, _ar0231_delay_ }, \
    { 0x3348, 0x0111, _ar0231_delay_ }, \
    { 0x334C, 0x0211, _ar0231_delay_ }, \
    { 0x3350, 0x0311, _ar0231_delay_ }, \
    { 0x31B0, 0x0049, _ar0231_delay_ }, \
    { 0x31B2, 0x0033, _ar0231_delay_ }, \
    { 0x31B4, 0x2185, _ar0231_delay_ }, \
    { 0x31B6, 0x1146, _ar0231_delay_ }, \
    { 0x31B8, 0x3047, _ar0231_delay_ }, \
    { 0x31BA, 0x0186, _ar0231_delay_ }, \
    { 0x31BC, 0x8805, _ar0231_delay_ }, \
    { 0x301A, 0x001C, _ar0231_delay_ }, \
}

#elif defined(AR0231_16BIT_HDR_MODE)

#define CAM_SNSR_INIT \
{ \
    { 0x301A, 0x0018, 100 }, \
    { 0x3092, 0x0C24, _ar0231_delay_ }, \
    { 0x337A, 0x0C80, _ar0231_delay_ }, \
    { 0x3520, 0x1288, _ar0231_delay_ }, \
    { 0x3522, 0x880C, _ar0231_delay_ }, \
    { 0x3524, 0x0C12, _ar0231_delay_ }, \
    { 0x352C, 0x1212, _ar0231_delay_ }, \
    { 0x354A, 0x007F, _ar0231_delay_ }, \
    { 0x350C, 0x055C, _ar0231_delay_ }, \
    { 0x3506, 0x3333, _ar0231_delay_ }, \
    { 0x3508, 0x3333, _ar0231_delay_ }, \
    { 0x3100, 0x4000, _ar0231_delay_ }, \
    { 0x3280, 0x0FA0, _ar0231_delay_ }, \
    { 0x3282, 0x0FA0, _ar0231_delay_ }, \
    { 0x3284, 0x0FA0, _ar0231_delay_ }, \
    { 0x3286, 0x0FA0, _ar0231_delay_ }, \
    { 0x3288, 0x0FA0, _ar0231_delay_ }, \
    { 0x328A, 0x0FA0, _ar0231_delay_ }, \
    { 0x328C, 0x0FA0, _ar0231_delay_ }, \
    { 0x328E, 0x0FA0, _ar0231_delay_ }, \
    { 0x3290, 0x0FA0, _ar0231_delay_ }, \
    { 0x3292, 0x0FA0, _ar0231_delay_ }, \
    { 0x3294, 0x0FA0, _ar0231_delay_ }, \
    { 0x3296, 0x0FA0, _ar0231_delay_ }, \
    { 0x3298, 0x0FA0, _ar0231_delay_ }, \
    { 0x329A, 0x0FA0, _ar0231_delay_ }, \
    { 0x329C, 0x0FA0, _ar0231_delay_ }, \
    { 0x329E, 0x0FA0, _ar0231_delay_ }, \
    { 0x301A, 0x0018, 100 }, \
    { 0x2512, 0x8000, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x3350, _ar0231_delay_ }, \
    { 0x2510, 0x2004, _ar0231_delay_ }, \
    { 0x2510, 0x1460, _ar0231_delay_ }, \
    { 0x2510, 0x1578, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x7B24, _ar0231_delay_ }, \
    { 0x2510, 0xFF24, _ar0231_delay_ }, \
    { 0x2510, 0xFF24, _ar0231_delay_ }, \
    { 0x2510, 0xEA24, _ar0231_delay_ }, \
    { 0x2510, 0x1022, _ar0231_delay_ }, \
    { 0x2510, 0x2410, _ar0231_delay_ }, \
    { 0x2510, 0x155A, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x24FF, _ar0231_delay_ }, \
    { 0x2510, 0x24FF, _ar0231_delay_ }, \
    { 0x2510, 0x24EA, _ar0231_delay_ }, \
    { 0x2510, 0x2324, _ar0231_delay_ }, \
    { 0x2510, 0x647A, _ar0231_delay_ }, \
    { 0x2510, 0x2404, _ar0231_delay_ }, \
    { 0x2510, 0x052C, _ar0231_delay_ }, \
    { 0x2510, 0x400A, _ar0231_delay_ }, \
    { 0x2510, 0xFF0A, _ar0231_delay_ }, \
    { 0x2510, 0xFF0A, _ar0231_delay_ }, \
    { 0x2510, 0x1008, _ar0231_delay_ }, \
    { 0x2510, 0x3851, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0004, _ar0231_delay_ }, \
    { 0x2510, 0x0801, _ar0231_delay_ }, \
    { 0x2510, 0x0408, _ar0231_delay_ }, \
    { 0x2510, 0x1180, _ar0231_delay_ }, \
    { 0x2510, 0x2652, _ar0231_delay_ }, \
    { 0x2510, 0x1518, _ar0231_delay_ }, \
    { 0x2510, 0x0906, _ar0231_delay_ }, \
    { 0x2510, 0x1348, _ar0231_delay_ }, \
    { 0x2510, 0x1002, _ar0231_delay_ }, \
    { 0x2510, 0x1016, _ar0231_delay_ }, \
    { 0x2510, 0x1181, _ar0231_delay_ }, \
    { 0x2510, 0x1189, _ar0231_delay_ }, \
    { 0x2510, 0x1056, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x0D09, _ar0231_delay_ }, \
    { 0x2510, 0x1413, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x2B15, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x0311, _ar0231_delay_ }, \
    { 0x2510, 0xD909, _ar0231_delay_ }, \
    { 0x2510, 0x1214, _ar0231_delay_ }, \
    { 0x2510, 0x4109, _ar0231_delay_ }, \
    { 0x2510, 0x0312, _ar0231_delay_ }, \
    { 0x2510, 0x1409, _ar0231_delay_ }, \
    { 0x2510, 0x0110, _ar0231_delay_ }, \
    { 0x2510, 0xD612, _ar0231_delay_ }, \
    { 0x2510, 0x1012, _ar0231_delay_ }, \
    { 0x2510, 0x1212, _ar0231_delay_ }, \
    { 0x2510, 0x1011, _ar0231_delay_ }, \
    { 0x2510, 0xDD11, _ar0231_delay_ }, \
    { 0x2510, 0xD910, _ar0231_delay_ }, \
    { 0x2510, 0x5609, _ar0231_delay_ }, \
    { 0x2510, 0x1511, _ar0231_delay_ }, \
    { 0x2510, 0xDB09, _ar0231_delay_ }, \
    { 0x2510, 0x1511, _ar0231_delay_ }, \
    { 0x2510, 0x9B09, _ar0231_delay_ }, \
    { 0x2510, 0x0F11, _ar0231_delay_ }, \
    { 0x2510, 0xBB12, _ar0231_delay_ }, \
    { 0x2510, 0x1A12, _ar0231_delay_ }, \
    { 0x2510, 0x1014, _ar0231_delay_ }, \
    { 0x2510, 0x6012, _ar0231_delay_ }, \
    { 0x2510, 0x5010, _ar0231_delay_ }, \
    { 0x2510, 0x7610, _ar0231_delay_ }, \
    { 0x2510, 0xE609, _ar0231_delay_ }, \
    { 0x2510, 0x0812, _ar0231_delay_ }, \
    { 0x2510, 0x4012, _ar0231_delay_ }, \
    { 0x2510, 0x6009, _ar0231_delay_ }, \
    { 0x2510, 0x290B, _ar0231_delay_ }, \
    { 0x2510, 0x0904, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0923, _ar0231_delay_ }, \
    { 0x2510, 0x15C8, _ar0231_delay_ }, \
    { 0x2510, 0x13C8, _ar0231_delay_ }, \
    { 0x2510, 0x092C, _ar0231_delay_ }, \
    { 0x2510, 0x1588, _ar0231_delay_ }, \
    { 0x2510, 0x1388, _ar0231_delay_ }, \
    { 0x2510, 0x0C09, _ar0231_delay_ }, \
    { 0x2510, 0x0C14, _ar0231_delay_ }, \
    { 0x2510, 0x4109, _ar0231_delay_ }, \
    { 0x2510, 0x1112, _ar0231_delay_ }, \
    { 0x2510, 0x6212, _ar0231_delay_ }, \
    { 0x2510, 0x6011, _ar0231_delay_ }, \
    { 0x2510, 0xBF11, _ar0231_delay_ }, \
    { 0x2510, 0xBB10, _ar0231_delay_ }, \
    { 0x2510, 0x6611, _ar0231_delay_ }, \
    { 0x2510, 0xFB09, _ar0231_delay_ }, \
    { 0x2510, 0x3511, _ar0231_delay_ }, \
    { 0x2510, 0xBB12, _ar0231_delay_ }, \
    { 0x2510, 0x6312, _ar0231_delay_ }, \
    { 0x2510, 0x6014, _ar0231_delay_ }, \
    { 0x2510, 0x0015, _ar0231_delay_ }, \
    { 0x2510, 0x0011, _ar0231_delay_ }, \
    { 0x2510, 0xB812, _ar0231_delay_ }, \
    { 0x2510, 0xA012, _ar0231_delay_ }, \
    { 0x2510, 0x0010, _ar0231_delay_ }, \
    { 0x2510, 0x2610, _ar0231_delay_ }, \
    { 0x2510, 0x0013, _ar0231_delay_ }, \
    { 0x2510, 0x0011, _ar0231_delay_ }, \
    { 0x2510, 0x0008, _ar0231_delay_ }, \
    { 0x2510, 0x3053, _ar0231_delay_ }, \
    { 0x2510, 0x4215, _ar0231_delay_ }, \
    { 0x2510, 0x4013, _ar0231_delay_ }, \
    { 0x2510, 0x4010, _ar0231_delay_ }, \
    { 0x2510, 0x0210, _ar0231_delay_ }, \
    { 0x2510, 0x1611, _ar0231_delay_ }, \
    { 0x2510, 0x8111, _ar0231_delay_ }, \
    { 0x2510, 0x8910, _ar0231_delay_ }, \
    { 0x2510, 0x5612, _ar0231_delay_ }, \
    { 0x2510, 0x1009, _ar0231_delay_ }, \
    { 0x2510, 0x010D, _ar0231_delay_ }, \
    { 0x2510, 0x0815, _ar0231_delay_ }, \
    { 0x2510, 0xC015, _ar0231_delay_ }, \
    { 0x2510, 0xD013, _ar0231_delay_ }, \
    { 0x2510, 0x5009, _ar0231_delay_ }, \
    { 0x2510, 0x1313, _ar0231_delay_ }, \
    { 0x2510, 0xD009, _ar0231_delay_ }, \
    { 0x2510, 0x0215, _ar0231_delay_ }, \
    { 0x2510, 0xC015, _ar0231_delay_ }, \
    { 0x2510, 0xC813, _ar0231_delay_ }, \
    { 0x2510, 0xC009, _ar0231_delay_ }, \
    { 0x2510, 0x0515, _ar0231_delay_ }, \
    { 0x2510, 0x8813, _ar0231_delay_ }, \
    { 0x2510, 0x8009, _ar0231_delay_ }, \
    { 0x2510, 0x0213, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x0411, _ar0231_delay_ }, \
    { 0x2510, 0xC909, _ar0231_delay_ }, \
    { 0x2510, 0x0814, _ar0231_delay_ }, \
    { 0x2510, 0x0109, _ar0231_delay_ }, \
    { 0x2510, 0x0B11, _ar0231_delay_ }, \
    { 0x2510, 0xD908, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x091A, _ar0231_delay_ }, \
    { 0x2510, 0x1440, _ar0231_delay_ }, \
    { 0x2510, 0x0903, _ar0231_delay_ }, \
    { 0x2510, 0x1214, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x10D6, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x1212, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x11DD, _ar0231_delay_ }, \
    { 0x2510, 0x11D9, _ar0231_delay_ }, \
    { 0x2510, 0x1056, _ar0231_delay_ }, \
    { 0x2510, 0x0917, _ar0231_delay_ }, \
    { 0x2510, 0x11DB, _ar0231_delay_ }, \
    { 0x2510, 0x0913, _ar0231_delay_ }, \
    { 0x2510, 0x11FB, _ar0231_delay_ }, \
    { 0x2510, 0x0905, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x121A, _ar0231_delay_ }, \
    { 0x2510, 0x1210, _ar0231_delay_ }, \
    { 0x2510, 0x1460, _ar0231_delay_ }, \
    { 0x2510, 0x1250, _ar0231_delay_ }, \
    { 0x2510, 0x1076, _ar0231_delay_ }, \
    { 0x2510, 0x10E6, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x15A8, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x13A8, _ar0231_delay_ }, \
    { 0x2510, 0x1240, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x0925, _ar0231_delay_ }, \
    { 0x2510, 0x13AD, _ar0231_delay_ }, \
    { 0x2510, 0x0902, _ar0231_delay_ }, \
    { 0x2510, 0x0907, _ar0231_delay_ }, \
    { 0x2510, 0x1588, _ar0231_delay_ }, \
    { 0x2510, 0x0901, _ar0231_delay_ }, \
    { 0x2510, 0x138D, _ar0231_delay_ }, \
    { 0x2510, 0x0B09, _ar0231_delay_ }, \
    { 0x2510, 0x0914, _ar0231_delay_ }, \
    { 0x2510, 0x4009, _ar0231_delay_ }, \
    { 0x2510, 0x0B13, _ar0231_delay_ }, \
    { 0x2510, 0x8809, _ar0231_delay_ }, \
    { 0x2510, 0x1C0C, _ar0231_delay_ }, \
    { 0x2510, 0x0920, _ar0231_delay_ }, \
    { 0x2510, 0x1262, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x11BF, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x1066, _ar0231_delay_ }, \
    { 0x2510, 0x090A, _ar0231_delay_ }, \
    { 0x2510, 0x11FB, _ar0231_delay_ }, \
    { 0x2510, 0x093B, _ar0231_delay_ }, \
    { 0x2510, 0x11BB, _ar0231_delay_ }, \
    { 0x2510, 0x1263, _ar0231_delay_ }, \
    { 0x2510, 0x1260, _ar0231_delay_ }, \
    { 0x2510, 0x1400, _ar0231_delay_ }, \
    { 0x2510, 0x1508, _ar0231_delay_ }, \
    { 0x2510, 0x11B8, _ar0231_delay_ }, \
    { 0x2510, 0x12A0, _ar0231_delay_ }, \
    { 0x2510, 0x1200, _ar0231_delay_ }, \
    { 0x2510, 0x1026, _ar0231_delay_ }, \
    { 0x2510, 0x1000, _ar0231_delay_ }, \
    { 0x2510, 0x1300, _ar0231_delay_ }, \
    { 0x2510, 0x1100, _ar0231_delay_ }, \
    { 0x2510, 0x437A, _ar0231_delay_ }, \
    { 0x2510, 0x0609, _ar0231_delay_ }, \
    { 0x2510, 0x0B05, _ar0231_delay_ }, \
    { 0x2510, 0x0708, _ar0231_delay_ }, \
    { 0x2510, 0x4137, _ar0231_delay_ }, \
    { 0x2510, 0x502C, _ar0231_delay_ }, \
    { 0x2510, 0x2CFE, _ar0231_delay_ }, \
    { 0x2510, 0x15FE, _ar0231_delay_ }, \
    { 0x2510, 0x0C2C, _ar0231_delay_ }, \
    { 0x32E6, 0x00E0, _ar0231_delay_ }, \
    { 0x1008, 0x036F, _ar0231_delay_ }, \
    { 0x100C, 0x058F, _ar0231_delay_ }, \
    { 0x100E, 0x07AF, _ar0231_delay_ }, \
    { 0x1010, 0x014F, _ar0231_delay_ }, \
    { 0x3230, 0x0312, _ar0231_delay_ }, \
    { 0x3232, 0x0532, _ar0231_delay_ }, \
    { 0x3234, 0x0752, _ar0231_delay_ }, \
    { 0x3236, 0x00F2, _ar0231_delay_ }, \
    { 0x3566, 0x3328, _ar0231_delay_ }, \
    { 0x32D0, 0x3A02, _ar0231_delay_ }, \
    { 0x32D2, 0x3508, _ar0231_delay_ }, \
    { 0x32D4, 0x3702, _ar0231_delay_ }, \
    { 0x32D6, 0x3C04, _ar0231_delay_ }, \
    { 0x32DC, 0x370A, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x302A, 0x0006, _ar0231_delay_ }, \
    { 0x302C, 0x0001, _ar0231_delay_ }, \
    { 0x302E, 0x0002, _ar0231_delay_ }, \
    { 0x3030, 0x002C, _ar0231_delay_ }, \
    { 0x3036, 0x000C, _ar0231_delay_ }, \
    { 0x3038, 0x0001, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x30A2, 0x0001, _ar0231_delay_ }, \
    { 0x30A6, 0x0001, _ar0231_delay_ }, \
    { 0x3040, 0x0000, _ar0231_delay_ }, \
    { 0x3040, 0x0000, _ar0231_delay_ }, \
    { 0x3082, 0x000C, _ar0231_delay_ }, \
    { 0x3082, 0x000C, _ar0231_delay_ }, \
    { 0x3082, 0x000C, _ar0231_delay_ }, \
    { 0x3082, 0x000C, _ar0231_delay_ }, \
    { 0x30BA, 0x11F3, _ar0231_delay_ }, \
    { 0x30BA, 0x11F3, _ar0231_delay_ }, \
    { 0x30BA, 0x11F3, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3044, 0x0400, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x3064, 0x1802, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x3180, 0x0080, _ar0231_delay_ }, \
    { 0x33E4, 0x0080, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x33E0, 0x0C80, _ar0231_delay_ }, \
    { 0x3004, 0x0000, _ar0231_delay_ }, \
    { 0x3008, 0x0787, _ar0231_delay_ }, \
    { 0x3002, 0x0000, _ar0231_delay_ }, \
    { 0x3006, 0x04B7, _ar0231_delay_ }, \
    { 0x3032, 0x0000, _ar0231_delay_ }, \
    { 0x3400, 0x0010, _ar0231_delay_ }, \
    { 0x3402, 0x0F10, _ar0231_delay_ }, \
    { 0x3402, 0x0F10, _ar0231_delay_ }, \
    { 0x3404, 0x0970, _ar0231_delay_ }, \
    { 0x3404, 0x0970, _ar0231_delay_ }, \
    { 0x3082, 0x000C, _ar0231_delay_ }, \
    { 0x30BA, 0x11F3, _ar0231_delay_ }, \
    { 0x300C, 0x08C6, _ar0231_delay_ }, \
    { 0x300A, 0x051A, _ar0231_delay_ }, \
    { 0x3042, 0x0000, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3238, 0x0222, _ar0231_delay_ }, \
    { 0x3012, 0x0131, _ar0231_delay_ }, \
    { 0x3014, 0x098E, _ar0231_delay_ }, \
    { 0x321E, 0x098E, _ar0231_delay_ }, \
    { 0x3222, 0x098E, _ar0231_delay_ }, \
    { 0x3226, 0x098E, _ar0231_delay_ }, \
    { 0x30B0, 0x0800, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EA, 0x3C0E, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x32EC, 0x72A1, _ar0231_delay_ }, \
    { 0x31D0, 0x0001, _ar0231_delay_ }, \
    { 0x31AE, 0x0204, _ar0231_delay_ }, \
    { 0x31AE, 0x0304, _ar0231_delay_ }, \
    { 0x31AC, 0x1410, _ar0231_delay_ }, \
    { 0x31AC, 0x1410, _ar0231_delay_ }, \
    { 0x301A, 0x0018, _ar0231_delay_ }, \
    { 0x301A, 0x0018, _ar0231_delay_ }, \
    { 0x301A, 0x0018, _ar0231_delay_ }, \
    { 0x31AE, 0x0204, _ar0231_delay_ }, \
    { 0x3342, 0x122E, _ar0231_delay_ }, \
    { 0x3346, 0x122E, _ar0231_delay_ }, \
    { 0x334A, 0x122E, _ar0231_delay_ }, \
    { 0x334E, 0x122E, _ar0231_delay_ }, \
    { 0x3344, 0x0011, _ar0231_delay_ }, \
    { 0x3348, 0x0111, _ar0231_delay_ }, \
    { 0x334C, 0x0211, _ar0231_delay_ }, \
    { 0x3350, 0x0311, _ar0231_delay_ }, \
    { 0x31B0, 0x0049, _ar0231_delay_ }, \
    { 0x31B2, 0x0033, _ar0231_delay_ }, \
    { 0x31B4, 0x2185, _ar0231_delay_ }, \
    { 0x31B6, 0x1146, _ar0231_delay_ }, \
    { 0x31B8, 0x3047, _ar0231_delay_ }, \
    { 0x31BA, 0x0186, _ar0231_delay_ }, \
    { 0x31BC, 0x0805, _ar0231_delay_ }, \
    { 0x31BC, 0x8805, _ar0231_delay_ }, \
    { 0x301A, 0x001C, 100 }, \
}

#endif

#define ADD_I2C_REG_ARRAY(_a_, _size_, _addr_, _val_, _delay_) \
do { \
    _a_[_size_].reg_addr = _addr_; \
    _a_[_size_].reg_data = _val_; \
    _a_[_size_].delay = _delay_; \
    _size_++; \
} while(0)


static int ar0231_detect(max9296_context_t* ctxt, uint32 link);
static int ar0231_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg);
static int ar0231_init_link(max9296_context_t* ctxt, uint32 link);
static int ar0231_start_link(max9296_context_t* ctxt, uint32 link);
static int ar0231_stop_link(max9296_context_t* ctxt, uint32 link);
static int ar0231_calculate_exposure(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);
static int ar0231_apply_exposure(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);
static int ar0231_apply_hdr_exposure(max9296_context_t* ctxt, uint32 link, qcarcam_hdr_exposure_config_t* hdr_exposure);

max9296_sensor_t ar0231_info = {
    .id = MAXIM_SENSOR_ID_AR0231,
    .detect = ar0231_detect,
    .get_link_cfg = ar0231_get_link_cfg,

    .init_link = ar0231_init_link,
    .start_link = ar0231_start_link,
    .stop_link = ar0231_stop_link,

    .calculate_exposure = ar0231_calculate_exposure,
    .apply_exposure = ar0231_apply_exposure,
    .apply_hdr_exposure = ar0231_apply_hdr_exposure
};


static struct camera_i2c_reg_setting cam_ser_reg_setting =
{
    .reg_array = NULL,
    .size = 0,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
};

static struct camera_i2c_reg_setting cam_sensor_reg_setting =
{
    .reg_array = NULL,
    .size = 0,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_WORD_DATA,
};

static struct camera_i2c_reg_array max9295_gmsl_0[] = CAM_SER_ADDR_CHNG_A;
static struct camera_i2c_reg_array max9295_gmsl_1[] = CAM_SER_ADDR_CHNG_B;
static struct camera_i2c_reg_array max9295_init_0[] = CAM_SER_INIT_A;
static struct camera_i2c_reg_array max9295_init_1[] = CAM_SER_INIT_B;
static struct camera_i2c_reg_array max9295_start_reg[] = CAM_SER_START;
static struct camera_i2c_reg_array max9295_stop_reg[] = CAM_SER_STOP;
static struct camera_i2c_reg_array ar0231_init_array[] = CAM_SNSR_INIT;

// List of serializer addresses we support
static uint16 supported_ser_addr[] = {0xC4, 0x88, 0x80};

static maxim_pipeline_t ar0231_isp_pipelines[MAXIM_LINK_MAX] =
{
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = FMT_LINK,
            .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
            .channel_info = {.vc = 0, .dt = DT_LINK, .cid = CID_VC0},
        },
    },
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = FMT_LINK,
            .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
            .channel_info = {.vc = 1, .dt = DT_LINK, .cid = CID_VC1},
        },
    }
};


typedef struct
{
    float exp;       //exp in ms
    float real_gain; //total real gain
}ar0231_exp_table_t;

static ar0231_exp_table_t g_ar0231_exp_table[] =
{
    { 1.000000f,  1.000000f },
    { 1.030000f,  1.000000f },
    { 1.060900f,  1.000000f },
    { 1.092727f,  1.000000f },
    { 1.125509f,  1.000000f },
    { 1.159274f,  1.000000f },
    { 1.194052f,  1.000000f },
    { 1.229874f,  1.000000f },
    { 1.266770f,  1.000000f },
    { 1.304773f,  1.000000f },
    { 1.343916f,  1.000000f },
    { 1.384234f,  1.000000f },
    { 1.425761f,  1.000000f },
    { 1.468533f,  1.000000f },
    { 1.512589f,  1.000000f },
    { 1.557967f,  1.000000f },
    { 1.604706f,  1.000000f },
    { 1.652847f,  1.000000f },
    { 1.702432f,  1.000000f },
    { 1.753505f,  1.000000f },
    { 1.806110f,  1.000000f },
    { 1.860294f,  1.000000f },
    { 1.916102f,  1.000000f },
    { 1.973585f,  1.000000f },
    { 2.032793f,  1.000000f },
    { 2.093777f,  1.000000f },
    { 2.156590f,  1.000000f },
    { 2.221288f,  1.000000f },
    { 2.287926f,  1.000000f },
    { 2.356564f,  1.000000f },
    { 2.427261f,  1.000000f },
    { 2.500078f,  1.000000f },
    { 2.575081f,  1.000000f },
    { 2.652333f,  1.000000f },
    { 2.731903f,  1.000000f },
    { 2.813860f,  1.000000f },
    { 2.898276f,  1.000000f },
    { 2.985224f,  1.000000f },
    { 3.074780f,  1.000000f },
    { 3.167024f,  1.000000f },
    { 3.262034f,  1.000000f },
    { 3.359895f,  1.000000f },
    { 3.460692f,  1.000000f },
    { 3.564513f,  1.000000f },
    { 3.671448f,  1.000000f },
    { 3.781591f,  1.000000f },
    { 3.895039f,  1.000000f },
    { 4.011890f,  1.000000f },
    { 4.132246f,  1.000000f },
    { 4.256214f,  1.000000f },
    { 4.383900f,  1.000000f },
    { 4.515417f,  1.000000f },
    { 4.650879f,  1.000000f },
    { 4.790406f,  1.000000f },
    { 4.934118f,  1.000000f },
    { 5.082141f,  1.000000f },
    { 5.234605f,  1.000000f },
    { 5.391644f,  1.000000f },
    { 5.553393f,  1.000000f },
    { 5.719995f,  1.000000f },
    { 5.891594f,  1.000000f },
    { 6.068342f,  1.000000f },
    { 6.250392f,  1.000000f },
    { 6.437904f,  1.000000f },
    { 6.631041f,  1.000000f },
    { 6.829972f,  1.000000f },
    { 7.034871f,  1.000000f },
    { 7.245917f,  1.000000f },
    { 7.463294f,  1.000000f },
    { 7.687192f,  1.000000f },
    { 7.917808f,  1.000000f },
    { 8.155342f,  1.000000f },
    { 8.400002f,  1.000000f },
    { 8.652002f,  1.000000f },
    { 8.911562f,  1.000000f },
    { 9.178908f,  1.000000f },
    { 9.454275f,  1.000000f },
    { 9.737903f,  1.000000f },
    { 10.030040f, 1.000000f },
    { 10.330940f, 1.000000f },
    { 10.640868f, 1.000000f },
    { 10.960093f, 1.000000f },
    { 11.288896f, 1.000000f },
    { 11.627563f, 1.000000f },
    { 11.976389f, 1.000000f },
    { 12.335680f, 1.000000f },
    { 12.705750f, 1.000000f },
    { 13.086923f, 1.000000f },
    { 13.479530f, 1.000000f },
    { 13.883916f, 1.000000f },
    { 14.300433f, 1.000000f },
    { 14.729445f, 1.000000f },
    { 15.171329f, 1.000000f },
    { 15.626468f, 1.000000f },
    { 16.095261f, 1.000000f },
    { 16.578117f, 1.000000f },
    { 17.075460f, 1.000000f },
    { 17.587725f, 1.000000f },
    { 18.115356f, 1.000000f },
    { 18.658817f, 1.000000f },
    { 19.218582f, 1.000000f },
    { 19.795139f, 1.000000f },
    { 20.388992f, 1.000000f },
    { 21.000662f, 1.000000f },
    { 21.630682f, 1.000000f },
    { 22.279602f, 1.000000f },
    { 22.947990f, 1.000000f },
    { 23.636429f, 1.000000f },
    { 24.345522f, 1.000000f },
    { 25.075888f, 1.000000f },
    { 25.828163f, 1.000000f },
    { 26.603006f, 1.000000f },
    { 27.401096f, 1.000000f },
    { 28.223129f, 1.000000f },
    { 29.069822f, 1.000000f },
    { 29.941916f, 1.000000f },
    { 30.840172f, 1.000000f },
    { 31.765375f, 1.000000f },
    { 32.718334f, 1.000000f },
    { 33.000000f, 1.021209f },
    { 33.000000f, 1.051845f },
    { 33.000000f, 1.083400f },
    { 33.000000f, 1.115902f },
    { 33.000000f, 1.149379f },
    { 33.000000f, 1.183860f },
    { 33.000000f, 1.219376f },
    { 33.000000f, 1.255957f },
    { 33.000000f, 1.293636f },
    { 33.000000f, 1.332445f },
    { 33.000000f, 1.372418f },
    { 33.000000f, 1.413591f },
    { 33.000000f, 1.455999f },
    { 33.000000f, 1.499678f },
    { 33.000000f, 1.544669f },
    { 33.000000f, 1.591009f },
    { 33.000000f, 1.638739f },
    { 33.000000f, 1.687901f },
    { 33.000000f, 1.738538f },
    { 33.000000f, 1.790694f },
    { 33.000000f, 1.844415f },
    { 33.000000f, 1.899747f },
    { 33.000000f, 1.956740f },
    { 33.000000f, 2.015442f },
    { 33.000000f, 2.075905f },
    { 33.000000f, 2.138182f },
    { 33.000000f, 2.202328f },
    { 33.000000f, 2.268398f },
    { 33.000000f, 2.336449f },
    { 33.000000f, 2.406543f },
    { 33.000000f, 2.478739f },
    { 33.000000f, 2.553101f },
    { 33.000000f, 2.629694f },
    { 33.000000f, 2.708585f },
    { 33.000000f, 2.789842f },
    { 33.000000f, 2.873538f },
    { 33.000000f, 2.959743f },
    { 33.000000f, 3.048536f },
    { 33.000000f, 3.139992f },
    { 33.000000f, 3.234191f },
    { 33.000000f, 3.331217f },
    { 33.000000f, 3.431154f },
    { 33.000000f, 3.534088f },
    { 33.000000f, 3.640111f },
    { 33.000000f, 3.749314f },
    { 33.000000f, 3.861793f },
    { 33.000000f, 3.977647f },
    { 33.000000f, 4.096976f },
    { 33.000000f, 4.219885f },
    { 33.000000f, 4.346482f },
    { 33.000000f, 4.476876f },
    { 33.000000f, 4.611182f },
    { 33.000000f, 4.749517f },
    { 33.000000f, 4.892003f },
    { 33.000000f, 5.038763f },
    { 33.000000f, 5.189926f },
    { 33.000000f, 5.345623f },
    { 33.000000f, 5.505992f },
    { 33.000000f, 5.671172f },
    { 33.000000f, 5.841307f },
    { 33.000000f, 6.016546f },
    { 33.000000f, 6.197042f },
    { 33.000000f, 6.382953f },
    { 33.000000f, 6.574441f },
    { 33.000000f, 6.771675f },
    { 33.000000f, 6.974825f },
    { 33.000000f, 7.184070f },
    { 33.000000f, 7.399591f },
    { 33.000000f, 7.621579f },
    { 33.000000f, 7.850226f },
    { 33.000000f, 8.085733f },
    { 33.000000f, 8.328305f },
    { 33.000000f, 8.578155f },
    { 33.000000f, 8.835499f },
    { 33.000000f, 9.100563f },
    { 33.000000f, 9.373580f },
    { 33.000000f, 9.654787f },
    { 33.000000f, 9.944430f },
    { 33.000000f, 10.24276f },
    { 33.000000f, 10.55004f },
    { 33.000000f, 10.86654f },
    { 33.000000f, 11.19254f },
    { 33.000000f, 11.52831f },
    { 33.000000f, 11.87416f },
    { 33.000000f, 12.23039f },
    { 33.000000f, 12.59730f },
    { 33.000000f, 12.97522f },
    { 33.000000f, 13.36448f },
    { 33.000000f, 13.76541f },
    { 33.000000f, 14.17837f },
    { 33.000000f, 14.60372f },
    { 33.000000f, 15.04183f },
    { 33.000000f, 15.49309f },
    { 33.000000f, 15.95788f },
    { 33.000000f, 16.43662f },
    { 33.000000f, 16.92972f },
    { 33.000000f, 17.43761f },
    { 33.000000f, 17.96073f },
    { 33.000000f, 18.49956f },
    { 33.000000f, 19.05454f },
    { 33.000000f, 19.62618f },
    { 33.000000f, 20.21497f },
    { 33.000000f, 20.82141f },
    { 33.000000f, 21.44606f },
    { 33.000000f, 22.08944f },
    { 33.000000f, 22.75212f },
    { 33.000000f, 23.43468f },
    { 33.000000f, 24.13772f },
    { 33.000000f, 24.86185f },
    { 33.000000f, 25.60771f },
    { 33.000000f, 26.37594f },
    { 33.000000f, 27.16722f },
    { 33.000000f, 27.98223f },
    { 33.000000f, 28.82170f },
    { 33.000000f, 29.68635f },
    { 33.000000f, 30.57694f },
    { 33.000000f, 31.49425f },
    { 33.000000f, 32.43907f },
    { 33.000000f, 33.41225f },
    { 33.000000f, 34.41461f },
    { 33.000000f, 35.44705f },
    { 33.000000f, 36.51046f },
    { 33.000000f, 37.60577f },
    { 33.000000f, 38.73395f },
    { 33.000000f, 39.89596f },
};

typedef struct
{
    unsigned int pclk_count;
    unsigned int line_count;
    unsigned int fine_integration_time;

    float exposure_time;
    float real_gain;

    unsigned int gain_idx;
    float dig_gain;
    char analog_gain;
    char lcg_hcg;
}ar0231_hdr_exp_t;

typedef struct
{
    //init params
    float max_exp;
    float min_exp;
    float max_gain;
    float min_gain;
    float step_ratio;
    uint32 num_steps;

    //exposure table
    ar0231_exp_table_t* exp_table;

    uint32 current_step;

}ar0231_contxt_t;


struct gain_table_t
{
    float real_gain;
    char analog_gain;
    char lcg_hcg;
};

static struct gain_table_t gain_table[] =
{
    { 1.0f,  0x7, 0 },
    { 1.25f, 0x8, 0 },
    { 1.5f,  0x9, 0 },
    { 2.0f,  0xA, 0 },
    { 2.33f, 0xB, 0 },
    { 3.50f, 0xC, 0 },
    { 4.0f,  0xD, 0 },
    { 5.0f,  0x7, 1 },
    { 6.25f, 0x8, 1 },
    { 7.5f,  0x9, 1 },
    { 10.0f, 0xA, 1 },
    { 11.67f,0xB, 1 },
    { 17.5f, 0xC, 1 },
    { 20.0f, 0xD, 1 },
    { 40.0f, 0xE, 1 },
};

static void ar0132_init_ctxt(void** ppCtxt)
{
    unsigned int i = 0;

    ar0231_contxt_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(ar0231_contxt_t));

    if (pCtxt)
    {
        memset(pCtxt, 0x0, sizeof(*pCtxt));

        pCtxt->current_step = 100;

        pCtxt->min_exp = 1.0f;
        pCtxt->max_exp = 33.0f;
        pCtxt->min_gain = 1.0f;
        pCtxt->max_gain = 40.0f;

        pCtxt->step_ratio = 1.03f;
        pCtxt->num_steps = STD_ARRAY_SIZE(g_ar0231_exp_table);
        pCtxt->exp_table = g_ar0231_exp_table;

        for (i = 0; i < pCtxt->num_steps; i++)
        {
            SENSOR_WARN("[AEC] exp_table[%3d] %f,%f", i, pCtxt->exp_table[i].exp, pCtxt->exp_table[i].real_gain);
        }

        *ppCtxt = pCtxt;
    }
}

static int ar0231_detect(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;
    int i = 0;
    int num_addr = STD_ARRAY_SIZE(supported_ser_addr);
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    sensor_platform_func_table_t* sensor_fcn_tbl = &pCtxt->platform_fcn_tbl;

    cam_ser_reg_setting.reg_array = read_reg;
    cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);

    /* Detect far end serializer */
    for (i = 0; i < num_addr; i++)
    {
        cam_ser_reg_setting.reg_array[0].reg_addr = MSM_SER_CHIP_ID_REG_ADDR;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, supported_ser_addr[i], &cam_ser_reg_setting);
        if (!rc)
        {
            pSensor->serializer_addr = supported_ser_addr[i];
            break;
        }
    }

    if (i == num_addr)
    {
        SENSOR_WARN("No Camera connected to Link %d of MAX9296 0x%x", link, pCtxt->slave_addr);
    }
    else if (pSensor->serializer_alias == pSensor->serializer_addr)
    {
        SENSOR_WARN("LINK %d already re-mapped", link);
        rc = 0;
    }
    else
    {
        struct camera_i2c_reg_array remap_ser[] = {
            {0x0, pSensor->serializer_alias, _ar0231_delay_}
        };

        //link reset, remap cam, create broadcast addr,
        struct camera_i2c_reg_array remap_ser_2[] = {
            {0x0010, 0x31, MAX9295_LINK_RESET_DELAY },
            {0x0042, pSensor->sensor_alias, _ar0231_delay_},
            {0x0043, CAM_SENSOR_DEFAULT_ADDR, _ar0231_delay_},
            {0x0044, CAM_SER_BROADCAST_ADDR, _ar0231_delay_},
            {0x0045, pSensor->serializer_alias, _ar0231_delay_}
        };

        SENSOR_WARN("Detected Camera connected to Link %d, Ser ID[0x%x]: 0x%x",
            link, MSM_SER_CHIP_ID_REG_ADDR, cam_ser_reg_setting.reg_array[0].reg_data);

        cam_ser_reg_setting.reg_array = remap_ser;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl,pSensor->serializer_addr, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to change serializer address (0x%x) of MAX9296 0x%x Link %d",
                pSensor->serializer_addr, pCtxt->slave_addr, link);
            return -1;
        }

        cam_ser_reg_setting.reg_array = remap_ser_2;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser_2);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            return -1;
        }

        cam_ser_reg_setting.reg_array = link ? max9295_gmsl_1 : max9295_gmsl_0;
        cam_ser_reg_setting.size = link ? STD_ARRAY_SIZE(max9295_gmsl_1) : STD_ARRAY_SIZE(max9295_gmsl_0);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            return -1;
        }

        // Read mapped SER to double check if successful
        cam_ser_reg_setting.reg_array = read_reg;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
        cam_ser_reg_setting.reg_array[0].reg_addr = 0x0000;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, pSensor->serializer_alias, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to read serializer(0x%x) after remap", pSensor->serializer_alias);
            return -1;
        }

        if (pSensor->serializer_alias != cam_ser_reg_setting.reg_array[0].reg_data)
        {
            SENSOR_WARN("Remote SER address remap failed: 0x%x, should be 0x%x",
                cam_ser_reg_setting.reg_array[0].reg_data, pSensor->serializer_alias);
        }

        ar0132_init_ctxt(&pSensor->pSensorCtxt);
    }

    return rc;
}

static int ar0231_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg)
{
    (void)ctxt;

    p_cfg->num_pipelines = 1;
    p_cfg->pipelines[0] = ar0231_isp_pipelines[link];
    return 0;
}

static int ar0231_init_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc;

    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};

        cam_ser_reg_setting.reg_array = link ? max9295_init_1 : max9295_init_0;
        cam_ser_reg_setting.size = link ? STD_ARRAY_SIZE(max9295_init_1) : STD_ARRAY_SIZE(max9295_init_0);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl,
                    pSensor->serializer_alias, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to init camera serializer(0x%x)", pSensor->serializer_alias);
            return -1;
        }

        cam_sensor_reg_setting.reg_array = read_reg;
        cam_sensor_reg_setting.size = STD_ARRAY_SIZE(read_reg);
        cam_sensor_reg_setting.reg_array[0].reg_addr = 0x3000;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl,
                    pSensor->sensor_alias, &cam_sensor_reg_setting);
        if (rc)
        {
            SERR("Failed to read sensor (0x%x)", pSensor->sensor_alias);
            return -1;
        }
        else
        {
            SHIGH("Remote SENSOR addr 0x%x", cam_sensor_reg_setting.reg_array[0].reg_data);
        }

        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("ar0231 %d init in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ar0231_start_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc;

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SHIGH("starting serializer");
        cam_ser_reg_setting.reg_array = max9295_start_reg;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_start_reg);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &cam_ser_reg_setting);
        if (rc)
        {
            SERR("serializer 0x%x failed to start", pSensor->serializer_alias);
        }
        else
        {
            SHIGH("starting sensor");

            cam_sensor_reg_setting.reg_array = ar0231_init_array;
            cam_sensor_reg_setting.size = STD_ARRAY_SIZE(ar0231_init_array);
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pSensor->sensor_alias,
                &cam_sensor_reg_setting);
            if (rc)
            {
                SERR("sensor 0x%x failed to start", pSensor->sensor_alias);
            }
            else
            {
                pSensor->state = SENSOR_STATE_STREAMING;
            }
        }
    }
    else
    {
        SERR("ar0231 %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ar0231_stop_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc;

    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        cam_ser_reg_setting.reg_array = max9295_stop_reg;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_stop_reg);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &cam_ser_reg_setting)))
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }

        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("ar0231 %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ar0231_get_gain_table_idx(float real_gain)
{
    int gain_table_size = STD_ARRAY_SIZE(gain_table);
    int last_idx = gain_table_size - 1;
    int idx = 0;

    if (real_gain >= gain_table[last_idx].real_gain)
    {
        idx = last_idx;
    }
    else
    {
        for (idx = 0; idx < last_idx; idx++)
        {
            if (real_gain < gain_table[idx+1].real_gain)
            {
                break;
            }
        }
    }

    return idx;
}

static int ar0231_calculate_exposure(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info)
{
    (void)ctxt;
    (void)link;

    exposure_info->exp_pclk_count = exposure_info->exposure_time*AR0231_PCLK/1000;
    exposure_info->line_count = STD_MIN((unsigned int)((exposure_info->exposure_time*AR0231_PCLK)/(AR0231_LINE_LENGTH_PCK*1000)), (AR0231_FRAME_LENGTH_LINES-1));

    SLOW("AEC - AR0143 : exp_time=%f => line_cnt=0x%x", exposure_info->exposure_time, exposure_info->line_count);

    exposure_info->reg_gain = ar0231_get_gain_table_idx(exposure_info->real_gain);

    exposure_info->sensor_analog_gain = gain_table[exposure_info->reg_gain].real_gain;
    exposure_info->sensor_digital_gain = exposure_info->real_gain / exposure_info->sensor_analog_gain;

    SLOW("AEC - AR0143 :idx = %d, gain=%0.2f, a_gain=%0.2f + d_gain=%0.2f",
            exposure_info->reg_gain, exposure_info->real_gain,
            exposure_info->sensor_analog_gain, exposure_info->sensor_digital_gain);

    return 0;
}


static int ar0231_apply_exposure(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc = 0;

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SERR("ar0231 %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }
    else if(exposure_info->exposure_mode_type == QCARCAM_EXPOSURE_MANUAL ||
       exposure_info->exposure_mode_type == QCARCAM_EXPOSURE_AUTO)
    {
        static struct camera_i2c_reg_array ar0231_exp_reg_array[15];
        uint32 exp_size = 0;

        float real_gain = gain_table[exposure_info->reg_gain].real_gain;
        char analog_gain = gain_table[exposure_info->reg_gain].analog_gain;
        char lcg_hcg = gain_table[exposure_info->reg_gain].lcg_hcg;

        unsigned int fine_integration_time = STD_MIN(exposure_info->exp_pclk_count - (exposure_info->line_count*AR0231_LINE_LENGTH_PCK),
                (AR0231_LINE_LENGTH_PCK-1));

        SLOW("AEC - AR0231 : coarse=0x%x, fine=0x%x, real_gain=%0.2f",
                exposure_info->line_count, fine_integration_time, real_gain);

        ADD_I2C_REG_ARRAY(ar0231_exp_reg_array, exp_size, AR0231_REG_GROUPED_PARAMETER_HOLD, 0x01, _ar0231_delay_);
        ADD_I2C_REG_ARRAY(ar0231_exp_reg_array, exp_size, AR0231_REG_COARSE_INTEGRATION_TIME, (exposure_info->line_count & 0xffff), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(ar0231_exp_reg_array, exp_size, AR0231_REG_FINE_INTEGRATION_TIME, FINE_INTEGRATION_TIME, _ar0231_delay_);
        ADD_I2C_REG_ARRAY(ar0231_exp_reg_array, exp_size, AR0231_REG_ANALOG_GAIN, AR0231_PACK_ANALOG_GAIN(analog_gain & 0xf), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(ar0231_exp_reg_array, exp_size, AR0231_REG_DC_GAIN, (lcg_hcg ? 0xFF : 0x00), _ar0231_delay_);
        //TODO: GLOBAL_GAIN (0x305E) : Q7 <- USE ONLY FOR WHITE BALANCING
        ADD_I2C_REG_ARRAY(ar0231_exp_reg_array, exp_size,
            AR0231_REG_DIG_GLOBAL_GAIN, STD_MIN(FLOAT_TO_Q(9, exposure_info->sensor_digital_gain), 0x7FF), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(ar0231_exp_reg_array, exp_size, AR0231_REG_GROUPED_PARAMETER_HOLD, 0x00, _ar0231_delay_);

        cam_sensor_reg_setting.reg_array = ar0231_exp_reg_array;
        cam_sensor_reg_setting.size = exp_size;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &cam_sensor_reg_setting);
        if (rc)
        {
            SERR("sensor 0x%x failed to start", pSensor->sensor_alias);
        }
    }
    else
    {
        //TODO: pass-through for now
        SERR("not implemented AEC mode %d", exposure_info->exposure_mode_type);
        rc = 0;
    }


    return rc;
}

static int ar0231_apply_hdr_exposure(max9296_context_t* ctxt, uint32 link, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    ar0231_contxt_t* pSensorCtxt = (ar0231_contxt_t*)pSensor->pSensorCtxt;
    int rc = 0;

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {

        SERR("ar0231 %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    switch(hdr_exposure->exposure_mode_type)
    {
    case QCARCAM_EXPOSURE_LUX_IDX:
    {
        unsigned int lux_idx = (unsigned int)hdr_exposure->lux_index;

        if (lux_idx >= pSensorCtxt->num_steps)
        {
            SENSOR_ERROR("lux_index (%d) out of bounds (max = %d)", lux_idx, pSensorCtxt->num_steps);
            return -1;
        }

        float exposure_time = pSensorCtxt->exp_table[lux_idx].exp;
        float real_gain = pSensorCtxt->exp_table[lux_idx].real_gain;

        unsigned int pclk_count = exposure_time*AR0231_PCLK/1000;
        unsigned int line_count = STD_MIN((unsigned int)(pclk_count/AR0231_LINE_LENGTH_PCK)-1, (AR0231_FRAME_LENGTH_LINES-1));

        SLOW("AEC - AR0231 : exp_time=%f => line_cnt=0x%x", exposure_time, line_count);

        unsigned int gain_idx = ar0231_get_gain_table_idx(real_gain);
        float dig_gain = real_gain / gain_table[gain_idx].real_gain;
        char analog_gain = gain_table[gain_idx].analog_gain;
        char lcg_hcg = gain_table[gain_idx].lcg_hcg;

        SLOW("AEC - AR0231 :idx = %d, gain=%0.2f, a_gain=%0.2f + d_gain=%0.2f",
            gain_idx, real_gain, analog_gain, dig_gain);

        static struct camera_i2c_reg_array exp_reg_array[15];
        uint32 exp_size = 0;

        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_GROUPED_PARAMETER_HOLD, 0x01, _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_COARSE_INTEGRATION_TIME, (line_count & 0xffff), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_FINE_INTEGRATION_TIME, FINE_INTEGRATION_TIME, _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_ANALOG_GAIN, AR0231_PACK_ANALOG_GAIN(analog_gain & 0xf), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_DC_GAIN, (lcg_hcg ? 0xFF : 0x00), _ar0231_delay_);
        //GLOBAL_GAIN (0x305E) : Q7 <- USE ONLY FOR WHITE BALANCING
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_DIG_GLOBAL_GAIN,
            STD_MIN(FLOAT_TO_Q(9, dig_gain), 0x7FF), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_GROUPED_PARAMETER_HOLD, 0x00, _ar0231_delay_);

        cam_sensor_reg_setting.reg_array = exp_reg_array;
        cam_sensor_reg_setting.size = exp_size;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &cam_sensor_reg_setting);
        if (rc)
        {
            SERR("sensor 0x%x failed to start", pSensor->sensor_alias);
        }

        break;
    }
    case QCARCAM_EXPOSURE_MANUAL:
    {
        unsigned int i;
        ar0231_hdr_exp_t hdr_exp[3];

        for (i = 0; i < 3; i++)
        {
            hdr_exp[i].pclk_count = hdr_exposure->exposure_time[i] * AR0231_PCLK/1000;
            hdr_exp[i].line_count = STD_MIN((unsigned int)(hdr_exp[i].pclk_count/AR0231_LINE_LENGTH_PCK)-1, (AR0231_FRAME_LENGTH_LINES-1));
            SLOW("AEC - AR0231 : [%d] exp_time=%f => line_cnt=0x%x", i, hdr_exposure->exposure_time[i], hdr_exp[i].line_count);

            unsigned int gain_idx = ar0231_get_gain_table_idx(hdr_exposure->gain[i]);
            hdr_exp[i].analog_gain = gain_table[gain_idx].analog_gain;
            hdr_exp[i].dig_gain = hdr_exposure->gain[i] / gain_table[gain_idx].real_gain;
            hdr_exp[i].lcg_hcg = gain_table[gain_idx].lcg_hcg;

            SLOW("AEC - AR0231 :gain_idx = %d, gain=%0.2f, a_gain=%0.2f + d_gain=%0.2f",
                gain_idx, hdr_exposure->gain[i], hdr_exp[i].analog_gain, hdr_exp[i].dig_gain);
        }

        static struct camera_i2c_reg_array exp_reg_array[15];
        uint32 exp_size = 0;

        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_GROUPED_PARAMETER_HOLD, 0x01, _ar0231_delay_);

        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_COARSE_INTEGRATION_TIME, (hdr_exp[0].line_count & 0xffff), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_COARSE_INTEGRATION_TIME2, (hdr_exp[1].line_count & 0xffff), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_COARSE_INTEGRATION_TIME3, (hdr_exp[2].line_count & 0xffff), _ar0231_delay_);

        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_FINE_INTEGRATION_TIME, FINE_INTEGRATION_TIME, _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_FINE_INTEGRATION_TIME2, FINE_INTEGRATION_TIME, _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_FINE_INTEGRATION_TIME3, FINE_INTEGRATION_TIME, _ar0231_delay_);

        unsigned short analog_gain = hdr_exp[0].analog_gain | (hdr_exp[1].analog_gain << 4) | (hdr_exp[2].analog_gain << 8);
        unsigned short lcg_hcg = hdr_exp[0].lcg_hcg | (hdr_exp[1].lcg_hcg << 1) | (hdr_exp[2].lcg_hcg << 2);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_ANALOG_GAIN, AR0231_PACK_ANALOG_GAIN(analog_gain & 0xf), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_DC_GAIN, (lcg_hcg ? 0xFF : 0x00), _ar0231_delay_);

        //GLOBAL_GAIN (0x305E) : Q7 <- USE ONLY FOR WHITE BALANCING

        //@todo: for now set global gain to T0
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_DIG_GLOBAL_GAIN, STD_MIN(FLOAT_TO_Q(9, hdr_exp[0].dig_gain), 0x7FF), _ar0231_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0231_REG_GROUPED_PARAMETER_HOLD, 0x00, _ar0231_delay_);

        cam_sensor_reg_setting.reg_array = exp_reg_array;
        cam_sensor_reg_setting.size = exp_size;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &cam_sensor_reg_setting);
        if (rc)
        {
            SERR("sensor 0x%x failed to start", pSensor->sensor_alias);
        }

        break;
    }
    case QCARCAM_EXPOSURE_AUTO:
    case QCARCAM_EXPOSURE_SEMI_AUTO:
    default:
        SENSOR_WARN("Not Implemented");
        break;

    }

    return rc;
}
