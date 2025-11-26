/**
 * @file ar0820.c
 *
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ar0820.h"


typedef enum
{
    AR0820_MODE_DEFAULT = 0,
    AR0820_MODE_12BIT_LINEAR_30FPS = AR0820_MODE_DEFAULT,
    AR0820_MODE_12BIT_LINEAR_60FPS = 1, /*not verified*/
    AR0820_MODE_MAX = 2
}ar0820_mode_id_t;

typedef struct
{
    maxim_pipeline_t pipeline;

    struct camera_i2c_reg_array* init_array;
    uint32_t         size_init_array;

    uint64_t         pclk;
    uint32_t         line_length_pck;
    uint32_t         frame_length_lines;

}ar0820_mode_t;

typedef struct
{
    float exp;       //exp in ms
    float real_gain; //total real gain
}ar0820_exp_table_t;

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
}ar0820_hdr_exp_t;

typedef struct
{
    struct camera_i2c_reg_array exp_reg_array[15];
    struct camera_i2c_reg_setting cam_ser_reg_setting;
    struct camera_i2c_reg_setting cam_sensor_reg_setting;

    //init params
    float max_exp;
    float min_exp;
    float max_gain;
    float min_gain;
    float step_ratio;
    uint32 num_steps;

    //exposure table
    ar0820_exp_table_t* exp_table;

    const ar0820_mode_t* p_mode;

    uint32 current_step;

}ar0820_contxt_t;


struct gain_table_t
{
    float real_gain;
    char analog_gain;
    char analog_fine_gain;
    char lcg_hcg;
};

#define COARSE_INTEGRATION_TIME 0x0090
#define FINE_INTEGRATION_TIME 0x0
#define FINE_INTEGRATION_TIME_12BIT_HDR_MODE 0x0752

#define _max9295_delay_ 0
#define _ar0820_delay_ 0
#define _ar0820_reset_delay_ 200 /* 200usec */

#define MAX9295_LINK_RESET_DELAY 100000
#define MAX9295_START_DELAY 20000

#define AR0820_SENSOR_DEFAULT_ADDR    0x20


#define CID_VC0        0
#define CID_VC1        4


#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#define AR0820_PACK_ANALOG_GAIN(_g_) \
    (((_g_) << 12) | ((_g_) << 8) | ((_g_) << 4) | (_g_))


#define AR0820_RESET_LOW_DELAY   75 /*75us from spec*/
#define AR0820_RESET_HIGH_DELAY  25000 /*25ms corresponds to roughly 650000 EXTCLK as per spec*/

#define AR0820_REG_GROUPED_PARAMETER_HOLD 0x3022
#define AR0820_REG_COARSE_INTEGRATION_TIME 0x3012
#define AR0820_REG_COARSE_INTEGRATION_TIME2 0x3212
#define AR0820_REG_COARSE_INTEGRATION_TIME3 0x3216
#define AR0820_REG_FINE_INTEGRATION_TIME 0x3014

#define AR0820_REG_ANALOG_GAIN 0x3366
#define AR0820_REG_ANALOG_FINE_GAIN 0x336A
#define AR0820_REG_DC_GAIN 0x3362
#define AR0820_REG_DIG_GLOBAL_GAIN 0x305E


#ifdef DEBUG_SINGLE_SENSOR

#define CAM_SER_INIT_A \
{ \
    { 0x0002, 0x03, _max9295_delay_ }, \
    { 0x02BE, 0x90, _max9295_delay_ }, /*set sensor_rst GPIO*/ \
    { 0x03F1, 0x89, _max9295_delay_ } \
}

#else

/* Difference between link A and link B is the stream ID set in register 0x53 */
#define CAM_SER_INIT_A \
{ \
    { 0x0002, 0x03, _max9295_delay_ }, \
    { 0X0040, 0X06, _max9295_delay_ }, \
    { 0X0041, 0X66, _max9295_delay_ }, \
    { 0X0005, 0XC0, _max9295_delay_ }, \
    { 0x0330, 0x00, _max9295_delay_ }, \
    { 0x0332, 0xBE, _max9295_delay_ }, \
    { 0x0333, 0xE1, _max9295_delay_ }, \
    { 0x0331, 0x33, _max9295_delay_ }, \
    { 0x0311, 0xF0, _max9295_delay_ }, \
    { 0x0308, 0x7F, _max9295_delay_ }, \
    { 0x0314, 0x6C, _max9295_delay_ }, \
    { 0x0316, 0x62, _max9295_delay_ }, \
    { 0x0318, 0x62, _max9295_delay_ }, \
    { 0x031A, 0x62, _max9295_delay_ }, \
}

#endif /*DEBUG_SINGLE_SENSOR*/

#define CAM_SER_INIT_B \
{ \
    { 0x0002, 0x03, _max9295_delay_ }, \
    { 0X0040, 0X06, _max9295_delay_ }, \
    { 0X0041, 0X66, _max9295_delay_ }, \
    { 0X0005, 0XC0, _max9295_delay_ }, \
    { 0x0053, 0x01, _max9295_delay_ }, \
    { 0x0330, 0x00, _max9295_delay_ }, \
    { 0x0332, 0xBE, _max9295_delay_ }, \
    { 0x0333, 0xE1, _max9295_delay_ }, \
    { 0x0331, 0x33, _max9295_delay_ }, \
    { 0x0311, 0xF0, _max9295_delay_ }, \
    { 0x0308, 0x7F, _max9295_delay_ }, \
    { 0x0314, 0x6C, _max9295_delay_ }, \
    { 0x0316, 0x62, _max9295_delay_ }, \
    { 0x0318, 0x62, _max9295_delay_ }, \
    { 0x031A, 0x62, _max9295_delay_ }, \
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

////////////////////////////////////////////////////////////////////////////////
//                     AR0820_MODE_12BIT_LINEAR_30FPS
////////////////////////////////////////////////////////////////////////////////
#define AR0820_12BIT_LINEAR_PCLK                 156000000 /*156MHz*/
#define AR0820_12BIT_LINEAR_LINE_LENGTH_PCK_30FPS  4440 /*0x1158*/
#define AR0820_12BIT_LINEAR_LINE_LENGTH_PCK_60FPS  2220 /*0x1158*/
#define AR0820_12BIT_LINEAR_FRAME_LENGTH_LINES     2336 /*0x920*/
#define AR0820_X_ADDR_START         0
#define AR0820_X_ADDR_END           0x0F07
#define AR0820_Y_ADDR_START         0x0
#define AR0820_Y_ADDR_END           0x0877
#define AR0820_ROPS                 4

#define AR0820_12BIT_LINEAR_SNSR_INIT \
{ \
    {0x301A, 0x0059, _ar0820_reset_delay_ }, /* RESET_REGISTER */ \
    {0x301A, 0x0058, _ar0820_delay_ }, /* RESET_REGISTER */ \
    {0x301A, 0x0058, _ar0820_delay_ }, /* RESET_REGISTER */ \
    {0x2512, 0x8000, _ar0820_delay_ }, /* SEQ_CTRL_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFF07, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xFFFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3001, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3010, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3006, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3020, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3008, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xB031, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xA824, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x003C, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x001F, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xB2F9, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x006F, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0078, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x005C, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x106F, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xC013, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x006E, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0079, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x007B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xC806, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x106E, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0017, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0013, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x004B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0002, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x90F2, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x90FF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xD034, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1032, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0000, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0033, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x00D1, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x092E, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1333, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x123D, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x045B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x11BB, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x133A, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1013, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1017, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1015, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1099, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x14DB, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x00DD, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3088, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3084, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x2003, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x11F9, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x02DA, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xD80C, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x2006, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x017A, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x01F0, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x14F0, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x008B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x10F8, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x118B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x00ED, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x00E4, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0072, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x203B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x8828, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x2003, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1064, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0063, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1072, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x003E, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xC00A, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x05CD, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x006E, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x100E, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0019, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0015, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x16EE, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0071, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x10BE, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1063, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1671, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1095, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1019, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3088, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3084, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x2003, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x018B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x110B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x117B, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x00E4, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0072, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x20C4, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1064, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x107A, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1072, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3041, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xD800, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x881A, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x100C, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x000E, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x100D, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3081, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x10CB, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1052, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x0038, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xC200, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xCA00, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xD230, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x8200, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x11AE, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xB041, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xD000, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x106D, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x101F, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x100E, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x100A, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3042, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3086, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x102F, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x3090, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x9010, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0xB000, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x30A0, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x1016, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x2510, 0x7FFF, _ar0820_delay_ }, /* SEQ_DATA_PORT */ \
    {0x3508, 0xAA80, _ar0820_delay_ }, /* RESERVED_MFR_3508 */ \
    {0x350A, 0xC5C0, _ar0820_delay_ }, /* RESERVED_MFR_350A */ \
    {0x350C, 0xC8C4, _ar0820_delay_ }, /* RESERVED_MFR_350C */ \
    {0x350E, 0x8C8C, _ar0820_delay_ }, /* RESERVED_MFR_350E */ \
    {0x3510, 0x8C88, _ar0820_delay_ }, /* RESERVED_MFR_3510 */ \
    {0x3512, 0x8C8C, _ar0820_delay_ }, /* RESERVED_MFR_3512 */ \
    {0x3514, 0xA0A0, _ar0820_delay_ }, /* RESERVED_MFR_3514 */ \
    {0x3518, 0x0040, _ar0820_delay_ }, /* RESERVED_MFR_3518 */ \
    {0x351A, 0x8600, _ar0820_delay_ }, /* RESERVED_MFR_351A */ \
    {0x351E, 0x0E40, _ar0820_delay_ }, /* RESERVED_MFR_351E */ \
    {0x3506, 0x004A, _ar0820_delay_ }, /* RESERVED_MFR_3506 */ \
    {0x3520, 0x0E19, _ar0820_delay_ }, /* RESERVED_MFR_3520 */ \
    {0x3522, 0x7F7F, _ar0820_delay_ }, /* RESERVED_MFR_3522 */ \
    {0x3524, 0x7F7F, _ar0820_delay_ }, /* RESERVED_MFR_3524 */ \
    {0x3526, 0x7F7F, _ar0820_delay_ }, /* RESERVED_MFR_3526 */ \
    {0x3528, 0x7F7F, _ar0820_delay_ }, /* RESERVED_MFR_3528 */ \
    {0x30FE, 0x00A8, _ar0820_delay_ }, /* RESERVED_MFR_30FE */ \
    {0x3584, 0x0000, _ar0820_delay_ }, /* RESERVED_MFR_3584 */ \
    {0x3540, 0x8308, _ar0820_delay_ }, /* RESERVED_MFR_3540 */ \
    {0x354C, 0x0031, _ar0820_delay_ }, /* RESERVED_MFR_354C */ \
    {0x354E, 0x535C, _ar0820_delay_ }, /* RESERVED_MFR_354E */ \
    {0x3550, 0x5C7F, _ar0820_delay_ }, /* RESERVED_MFR_3550 */ \
    {0x3552, 0x0011, _ar0820_delay_ }, /* RESERVED_MFR_3552 */ \
    {0x3370, 0x0111, _ar0820_delay_ }, /* DBLC_CONTROL */ \
    {0x337A, 0x0F50, _ar0820_delay_ }, /* RESERVED_MFR_337A */ \
    {0x337E, 0xFFF8, _ar0820_delay_ }, /* DBLC_OFFSET0 */ \
    {0x3110, 0x0011, _ar0820_delay_ }, /* HDR_CONTROL0 */ \
    {0x3100, 0x4000, _ar0820_delay_ }, /* DLO_CONTROL0 */ \
    {0x3364, 0x0173, _ar0820_delay_ }, /* DCG_TRIM */ \
    {0x3180, 0x0021, _ar0820_delay_ }, /* RESERVED_MFR_3180 */ \
    {0x3E4C, 0x0404, _ar0820_delay_ }, /* RESERVED_MFR_3E4C */ \
    {0x3E52, 0x0060, _ar0820_delay_ }, /* RESERVED_MFR_3E52 */ \
    {0x3180, 0x0021, _ar0820_delay_ }, /* RESERVED_MFR_3180 */ \
    {0x37A0, 0x0001, _ar0820_delay_ }, /* COARSE_INTEGRATION_AD_TIME */ \
    {0x37A4, 0x0000, _ar0820_delay_ }, /* COARSE_INTEGRATION_AD_TIME2 */ \
    {0x37A8, 0x0000, _ar0820_delay_ }, /* COARSE_INTEGRATION_AD_TIME3 */ \
    {0x37AC, 0x0000, _ar0820_delay_ }, /* COARSE_INTEGRATION_AD_TIME4 */ \
    {0x3E94, 0x3010, _ar0820_delay_ }, /* RESERVED_MFR_3E94 */ \
    {0x3372, 0xF50F, _ar0820_delay_ }, /* DBLC_FS0_CONTROL */ \
    {0x302A, 0x0003, _ar0820_delay_ }, /* VT_PIX_CLK_DIV */ \
    {0x302C, 0x0701, _ar0820_delay_ }, /* VT_SYS_CLK_DIV */ \
    {0x302E, 0x0009, _ar0820_delay_ }, /* PRE_PLL_CLK_DIV */ \
    {0x3030, 0x009C, _ar0820_delay_ }, /* PLL_MULTIPLIER */ \
    {0x3036, 0x0006, _ar0820_delay_ }, /* OP_WORD_CLK_DIV */ \
    {0x3038, 0x0001, _ar0820_delay_ }, /* OP_SYS_CLK_DIV */ \
    {0x303A, 0x0085, _ar0820_delay_ }, /* PLL_MULTIPLIER_ANA */ \
    {0x303C, 0x0003, _ar0820_delay_ }, /* PRE_PLL_CLK_DIV_ANA */ \
    {0x31B0, 0x0047, _ar0820_delay_ }, /* FRAME_PREAMBLE */ \
    {0x31B2, 0x0026, _ar0820_delay_ }, /* LINE_PREAMBLE */ \
    {0x31B4, 0x5187, _ar0820_delay_ }, /* RESERVED_MFR_31B4 */ \
    {0x31B6, 0x5248, _ar0820_delay_ }, /* RESERVED_MFR_31B6 */ \
    {0x31B8, 0x70CA, _ar0820_delay_ }, /* RESERVED_MFR_31B8 */ \
    {0x31BA, 0x028A, _ar0820_delay_ }, /* RESERVED_MFR_31BA */ \
    {0x31BC, 0x8A88, _ar0820_delay_ }, /* MIPI_TIMING_4 */ \
    {0x31BE, 0x0023, _ar0820_delay_ }, /* MIPI_CONFIG_STATUS */ \
    {0x3002, AR0820_Y_ADDR_START, _ar0820_delay_ }, /* Y_ADDR_START_ */ \
    {0x3004, AR0820_X_ADDR_START, _ar0820_delay_ }, /* X_ADDR_START_ */ \
    {0x3006, AR0820_Y_ADDR_END, _ar0820_delay_ }, /* Y_ADDR_END_ */ \
    {0x3008, AR0820_X_ADDR_END, _ar0820_delay_ }, /* X_ADDR_END_ */ \
    {0x32FC, 0x0000, _ar0820_delay_ }, /* READ_MODE2 */ \
    {0x37E0, 0x8421, _ar0820_delay_ }, /* ROW_TX_RO_ENABLE */ \
    {0x37E2, 0x8421, _ar0820_delay_ }, /* ROW_TX_RO_ENABLE_CB */ \
    {0x323C, 0x8421, _ar0820_delay_ }, /* ROW_TX_ENABLE */ \
    {0x323E, 0x8421, _ar0820_delay_ }, /* ROW_TX_ENABLE_CB */ \
    {0x3040, 0x0001, _ar0820_delay_ }, /* READ_MODE */ \
    {0x301D, 0x00, _ar0820_delay_ }, /* IMAGE_ORIENTATION_ */ \
    {0x3082, 0x0000, _ar0820_delay_ }, /* OPERATION_MODE_CTRL */ \
    {0x30BA, 0x1110, _ar0820_delay_ }, /* DIGITAL_CTRL */ \
    {0x3012, 0x0090, _ar0820_delay_ }, /* COARSE_INTEGRATION_TIME_ */ \
    {0x3362, 0x00FF, _ar0820_delay_ }, /* DC_GAIN */ \
    {0x3366, 0x0000, _ar0820_delay_ }, /* ANALOG_GAIN */ \
    {0x336A, 0x0000, _ar0820_delay_ }, /* ANALOG_GAIN2 */ \
    {0x300C, AR0820_12BIT_LINEAR_LINE_LENGTH_PCK_30FPS, _ar0820_delay_ }, /* LINE_LENGTH_PCK_ */ \
    {0x300A, 0x0920, _ar0820_delay_ }, /* FRAME_LENGTH_LINES_ */ \
    {0x31AE, 0x0204, _ar0820_delay_ }, /* SERIAL_FORMAT */ \
    {0x31AC, 0x0C0C, _ar0820_delay_ }, /* DATA_FORMAT_BITS */ \
    {0x301A, 0x0058, _ar0820_reset_delay_ }, /* RESET_REGISTER */ \
    {0x3064, 0x0000, _ar0820_delay_ }, /* SMIA_TEST */ \
    {0x301A, 0x005C, _ar0820_reset_delay_ }, /* RESET_REGISTER */ \
    {0x301A, 0x0058, _ar0820_delay_ }, /* RESET_REGISTER */ \
    {0x3064, 0x0000, _ar0820_delay_ }, /* SMIA_TEST */ \
    {0x301A, 0x005C, _ar0820_reset_delay_ }, /* RESET_REGISTER */ \
    {0x301A, 0x005C, _ar0820_reset_delay_ }, /* RESET_REGISTER */ \
}

static struct camera_i2c_reg_array ar0820_12bit_linear_init_array[] = AR0820_12BIT_LINEAR_SNSR_INIT;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define _ar0820_start_delay_ 100

#define AR0820_SNSR_START \
{ \
    { 0x301A, 0x001C, _ar0820_start_delay_ }, \
}

#define AR0820_SNSR_STOP \
{ \
    { 0x301A, 0x0018, _ar0820_delay_ }, \
}

#define ADD_I2C_REG_ARRAY(_a_, _size_, _addr_, _val_, _delay_) \
do { \
    _a_[_size_].reg_addr = _addr_; \
    _a_[_size_].reg_data = _val_; \
    _a_[_size_].delay = _delay_; \
    _size_++; \
} while(0)


static int ar0820_detect(max9296_context_t* ctxt, uint32 link);
static int ar0820_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg);
static int ar0820_init_link(max9296_context_t* ctxt, uint32 link);
static int ar0820_start_link(max9296_context_t* ctxt, uint32 link);
static int ar0820_stop_link(max9296_context_t* ctxt, uint32 link);
static int ar0820_calculate_exposure(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);
static int ar0820_apply_exposure(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);
static int ar0820_apply_hdr_exposure(max9296_context_t* ctxt, uint32 link, qcarcam_hdr_exposure_config_t* hdr_exposure);

static max9296_sensor_t ar0820_info = {
    .id = MAXIM_SENSOR_ID_AR0820,
    .detect = ar0820_detect,
    .get_link_cfg = ar0820_get_link_cfg,

    .init_link = ar0820_init_link,
    .start_link = ar0820_start_link,
    .stop_link = ar0820_stop_link,

    .calculate_exposure = ar0820_calculate_exposure,
    .apply_exposure = ar0820_apply_exposure,
    .apply_hdr_exposure = ar0820_apply_hdr_exposure
};

static struct camera_i2c_reg_array max9295_gmsl_0[] = CAM_SER_ADDR_CHNG_A;
static struct camera_i2c_reg_array max9295_gmsl_1[] = CAM_SER_ADDR_CHNG_B;
static struct camera_i2c_reg_array max9295_init_0[] = CAM_SER_INIT_A;
static struct camera_i2c_reg_array max9295_init_1[] = CAM_SER_INIT_B;
static struct camera_i2c_reg_array max9295_start_reg[] = CAM_SER_START;
static struct camera_i2c_reg_array max9295_stop_reg[] = CAM_SER_STOP;
static struct camera_i2c_reg_array ar0820_start_array[] = AR0820_SNSR_START;
static struct camera_i2c_reg_array ar0820_stop_array[] = AR0820_SNSR_STOP;

// List of serializer addresses we support
static uint16 supported_ser_addr[] = {0xC4, 0x88, 0x80, 0x0};

#define AR0820_SENSOR_WIDTH    3848
#define AR0820_SENSOR_HEIGHT   2168

static ar0820_mode_t ar0820_modes[AR0820_MODE_MAX] =
{
        [AR0820_MODE_12BIT_LINEAR_30FPS] =
        {
                .pipeline = {
                        .id = MAXIM_PIPELINE_X,
                        .mode =
                        {
                            .fmt = QCARCAM_FMT_MIPIRAW_12,
                            .res = {.width = AR0820_SENSOR_WIDTH, .height = AR0820_SENSOR_HEIGHT, .fps = 30.0f},
                            .channel_info = {.vc = 0, .dt = CSI_DT_RAW12, .cid = CID_VC0},
                        },
                },

                .init_array = ar0820_12bit_linear_init_array,
                .size_init_array = STD_ARRAY_SIZE(ar0820_12bit_linear_init_array),
                .pclk = AR0820_12BIT_LINEAR_PCLK,
                .line_length_pck = AR0820_12BIT_LINEAR_LINE_LENGTH_PCK_30FPS,
                .frame_length_lines = AR0820_12BIT_LINEAR_FRAME_LENGTH_LINES
        },
        [AR0820_MODE_12BIT_LINEAR_60FPS] =
        {
                .pipeline = {
                        .id = MAXIM_PIPELINE_X,
                        .mode =
                        {
                            .fmt = QCARCAM_FMT_MIPIRAW_12,
                            .res = {.width = AR0820_SENSOR_WIDTH, .height = AR0820_SENSOR_HEIGHT, .fps = 60.0f},
                            .channel_info = {.vc = 0, .dt = CSI_DT_RAW12, .cid = CID_VC0},
                        },
                },

                .init_array = ar0820_12bit_linear_init_array,
                .size_init_array = STD_ARRAY_SIZE(ar0820_12bit_linear_init_array),
                .pclk = AR0820_12BIT_LINEAR_PCLK,
                .line_length_pck = AR0820_12BIT_LINEAR_LINE_LENGTH_PCK_60FPS,
                .frame_length_lines = AR0820_12BIT_LINEAR_FRAME_LENGTH_LINES
        },
};


static ar0820_exp_table_t g_ar0820_exp_table[] =
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

static struct gain_table_t gain_table[] =
{
    { 2.90f, 0, 0, 1 },
    { 3.08f, 0, 1, 1 },
    { 3.26f, 0, 2, 1 },
    { 3.44f, 0, 3, 1 },
    { 3.63f, 0, 4, 1 },
    { 3.81f, 0, 5, 1 },
    { 3.99f, 0, 6, 1 },
    { 4.17f, 0, 7, 1 },
    { 4.35f, 0, 8, 1 },
    { 4.53f, 0, 9, 1 },
    { 4.71f, 0, 10, 1 },
    { 4.89f, 0, 11, 1 },
    { 5.08f, 0, 12, 1 },
    { 5.26f, 0, 13, 1 },
    { 5.44f, 0, 14, 1 },
    { 5.62f, 0, 15, 1 },
    { 5.80f, 1, 0, 1 },
    { 6.16f, 1, 1, 1 },
    { 6.53f, 1, 2, 1 },
    { 6.89f, 1, 3, 1 },
    { 7.25f, 1, 4, 1 },
    { 7.61f, 1, 5, 1 },
    { 7.98f, 1, 6, 1 },
    { 8.34f, 1, 7, 1 },
    { 8.70f, 1, 8, 1 },
    { 9.06f, 1, 9, 1 },
    { 9.43f, 1, 10, 1 },
    { 9.79f, 1, 11, 1 },
    { 10.15f, 1, 12, 1 },
    { 10.51f, 1, 13, 1 },
    { 10.88f, 1, 14, 1 },
    { 11.24f, 1, 15, 1 },
    { 11.60f, 2, 0, 1 },
    { 12.33f, 2, 1, 1 },
    { 13.05f, 2, 2, 1 },
    { 13.78f, 2, 3, 1 },
    { 14.50f, 2, 4, 1 },
    { 15.23f, 2, 5, 1 },
    { 15.95f, 2, 6, 1 },
    { 16.68f, 2, 7, 1 },
    { 17.40f, 2, 8, 1 },
    { 18.13f, 2, 9, 1 },
    { 18.85f, 2, 10, 1 },
    { 19.58f, 2, 11, 1 },
    { 20.30f, 2, 12, 1 },
    { 21.03f, 2, 13, 1 },
    { 21.75f, 2, 14, 1 },
    { 22.48f, 2, 15, 1 },
};

max9296_sensor_t* ar0820_get_sensor_info(void)
{
    return &ar0820_info;
}

static int ar0820_create_ctxt(max9296_sensor_info_t* pSensor)
{
    int rc = 0;

    if (pSensor->pPrivCtxt)
    {
        SLOW("Ctxt already created");
        return 0;
    }

    if(pSensor->mode >= AR0820_MODE_12BIT_LINEAR_60FPS)
    {
        SERR("Unsupported sensor mode %d", pSensor->mode);
        return CAMERA_EBADPARM;
    }

    ar0820_contxt_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(ar0820_contxt_t));
    if (pCtxt)
    {
        memset(pCtxt, 0x0, sizeof(*pCtxt));

        pCtxt->cam_ser_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->cam_ser_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        pCtxt->cam_sensor_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->cam_sensor_reg_setting.data_type = CAMERA_I2C_WORD_DATA;

        pCtxt->p_mode = &ar0820_modes[pSensor->mode];

        pCtxt->current_step = 100;

        pCtxt->min_exp = 1.0f;
        pCtxt->max_exp = 33.0f;
        pCtxt->min_gain = 1.0f;
        pCtxt->max_gain = 40.0f;

        pCtxt->step_ratio = 1.03f;
        pCtxt->num_steps = STD_ARRAY_SIZE(g_ar0820_exp_table);
        pCtxt->exp_table = g_ar0820_exp_table;

        pSensor->pPrivCtxt = pCtxt;
    }

    return rc;
}

static int ar0820_detect(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;
    int i = 0;
    int num_addr = STD_ARRAY_SIZE(supported_ser_addr);
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    sensor_platform_func_table_t* sensor_fcn_tbl = &pCtxt->platform_fcn_tbl;
    ar0820_contxt_t* ar0820_ctxt;

    rc = ar0820_create_ctxt(pSensor);

    if (rc)
    {
        SERR("Failed to create ctxt for link %d", link);
        return rc;
    }

    ar0820_ctxt = pSensor->pPrivCtxt;

    ar0820_ctxt->cam_ser_reg_setting.reg_array = read_reg;
    ar0820_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
    supported_ser_addr[num_addr-1] = pSensor->serializer_alias;

    /* Detect far end serializer */
    for (i = 0; i < num_addr; i++)
    {
        ar0820_ctxt->cam_ser_reg_setting.reg_array[0].reg_addr = MSM_SER_CHIP_ID_REG_ADDR;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, supported_ser_addr[i], &ar0820_ctxt->cam_ser_reg_setting);
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
            {0x0, pSensor->serializer_alias, _ar0820_delay_}
        };

        //link reset, remap cam, create broadcast addr,
        struct camera_i2c_reg_array remap_ser_2[] = {
            {0x0010, 0x31, MAX9295_LINK_RESET_DELAY },
            {0x0042, pSensor->sensor_alias, _ar0820_delay_},
            {0x0043, AR0820_SENSOR_DEFAULT_ADDR, _ar0820_delay_},
            {0x0044, CAM_SER_BROADCAST_ADDR, _ar0820_delay_},
            {0x0045, pSensor->serializer_alias, _ar0820_delay_}
        };

        SENSOR_WARN("Detected Camera connected to Link %d, Ser ID[0x%x]: 0x%x",
            link, MSM_SER_CHIP_ID_REG_ADDR, ar0820_ctxt->cam_ser_reg_setting.reg_array[0].reg_data);

        ar0820_ctxt->cam_ser_reg_setting.reg_array = remap_ser;
        ar0820_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl,pSensor->serializer_addr, &ar0820_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to change serializer address (0x%x) of MAX9296 0x%x Link %d",
                pSensor->serializer_addr, pCtxt->slave_addr, link);
            return rc;
        }

        ar0820_ctxt->cam_ser_reg_setting.reg_array = remap_ser_2;
        ar0820_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser_2);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &ar0820_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            return rc;
        }

        ar0820_ctxt->cam_ser_reg_setting.reg_array = link ? max9295_gmsl_1 : max9295_gmsl_0;
        ar0820_ctxt->cam_ser_reg_setting.size = link ? STD_ARRAY_SIZE(max9295_gmsl_1) : STD_ARRAY_SIZE(max9295_gmsl_0);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &ar0820_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            return rc;
        }

        // Read mapped SER to double check if successful
        ar0820_ctxt->cam_ser_reg_setting.reg_array = read_reg;
        ar0820_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
        ar0820_ctxt->cam_ser_reg_setting.reg_array[0].reg_addr = 0x0000;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, pSensor->serializer_alias, &ar0820_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to read serializer(0x%x) after remap", pSensor->serializer_alias);
            return rc;
        }

        if (pSensor->serializer_alias != ar0820_ctxt->cam_ser_reg_setting.reg_array[0].reg_data)
        {
            SENSOR_WARN("Remote SER address remap failed: 0x%x, should be 0x%x",
                ar0820_ctxt->cam_ser_reg_setting.reg_array[0].reg_data, pSensor->serializer_alias);
        }
    }

    return rc;
}

static int ar0820_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg)
{
    int rc = 0;
    unsigned int mode = ctxt->max9296_sensors[link].mode;

    p_cfg->num_pipelines = 1;
    p_cfg->pipelines[0] = ar0820_modes[mode].pipeline;

    return rc;
}

static int ar0820_init_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    ar0820_contxt_t* ar0820_ctxt = pSensor->pPrivCtxt;
    int rc;

    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        struct camera_i2c_reg_array tmp_reg[] = {{0, 0, 0}};

        ar0820_ctxt->cam_ser_reg_setting.reg_array = link ? max9295_init_1 : max9295_init_0;
        ar0820_ctxt->cam_ser_reg_setting.size = link ? STD_ARRAY_SIZE(max9295_init_1) : STD_ARRAY_SIZE(max9295_init_0);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl,
                    pSensor->serializer_alias, &ar0820_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to init camera serializer(0x%x)", pSensor->serializer_alias);
            return rc;
        }

        /*Wait for sensor to be ready for first i2c command*/
        CameraMicroSleep(AR0820_RESET_HIGH_DELAY);

        ar0820_ctxt->cam_sensor_reg_setting.reg_array = tmp_reg;
        ar0820_ctxt->cam_sensor_reg_setting.size = STD_ARRAY_SIZE(tmp_reg);
        ar0820_ctxt->cam_sensor_reg_setting.reg_array[0].reg_addr = 0x3000;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl,
                    pSensor->sensor_alias, &ar0820_ctxt->cam_sensor_reg_setting);
        if (rc)
        {
            SERR("Failed to read sensor (0x%x)", pSensor->sensor_alias);
            return rc;
        }

        SHIGH("Remote SENSOR addr 0x%x", ar0820_ctxt->cam_sensor_reg_setting.reg_array[0].reg_data);

        ar0820_ctxt->cam_sensor_reg_setting.reg_array = ar0820_ctxt->p_mode->init_array;
        ar0820_ctxt->cam_sensor_reg_setting.size = ar0820_ctxt->p_mode->size_init_array;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0820_ctxt->cam_sensor_reg_setting);
        if (rc)
        {
            SERR("Failed to init sensor 0x%x", pSensor->sensor_alias);
            return rc;
        }

        if (pSensor->mode == AR0820_MODE_12BIT_LINEAR_60FPS)
        {
            tmp_reg[0].reg_addr = 0x300C;
            tmp_reg[0].reg_data = ar0820_ctxt->p_mode->line_length_pck;
            ar0820_ctxt->cam_sensor_reg_setting.reg_array = tmp_reg;
            ar0820_ctxt->cam_sensor_reg_setting.size = STD_ARRAY_SIZE(tmp_reg);
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pSensor->sensor_alias,
                &ar0820_ctxt->cam_sensor_reg_setting);
            if (rc)
            {
                SERR("Failed to init sensor pclk 0x%x", pSensor->sensor_alias);
                return rc;
            }
        }

        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("ar0820 %d init in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADPARM;
    }

    return rc;
}

static int ar0820_start_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc;

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        ar0820_contxt_t* ar0820_ctxt = pSensor->pPrivCtxt;

        SHIGH("start link %d ar0820", link);

        ar0820_ctxt->cam_sensor_reg_setting.reg_array = ar0820_start_array;
        ar0820_ctxt->cam_sensor_reg_setting.size = STD_ARRAY_SIZE(ar0820_start_array);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pSensor->sensor_alias,
                &ar0820_ctxt->cam_sensor_reg_setting);
        if (rc)
        {
            SERR("Failed to start link %d sensor 0x%x", link, pSensor->sensor_alias);
            return rc;
        }

        SHIGH("starting serializer");
        ar0820_ctxt->cam_ser_reg_setting.reg_array = max9295_start_reg;
        ar0820_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_start_reg);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &ar0820_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("serializer 0x%x failed to start", pSensor->serializer_alias);
        }
        else
        {
            pSensor->state = SENSOR_STATE_STREAMING;
        }
    }
    else
    {
        SERR("ar0820 %d start in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}

static int ar0820_stop_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc;

    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        ar0820_contxt_t* ar0820_ctxt = pSensor->pPrivCtxt;

        SHIGH("stop link %d ar0820", link);

        ar0820_ctxt->cam_ser_reg_setting.reg_array = max9295_stop_reg;
        ar0820_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_stop_reg);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &ar0820_ctxt->cam_ser_reg_setting)))
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }

        ar0820_ctxt->cam_sensor_reg_setting.reg_array = ar0820_stop_array;
        ar0820_ctxt->cam_sensor_reg_setting.size = STD_ARRAY_SIZE(ar0820_stop_array);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pSensor->sensor_alias,
                &ar0820_ctxt->cam_sensor_reg_setting);
        if (rc)
        {
            SERR("Failed to stop link %d sensor 0x%x", link, pSensor->sensor_alias);
        }

        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("ar0820 %d stop in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}

static int ar0820_get_gain_table_idx(float real_gain)
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

static int ar0820_calculate_exposure(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info)
{
    ar0820_contxt_t* ar0820_ctxt = ctxt->max9296_sensors[link].pPrivCtxt;

    exposure_info->exp_pclk_count = exposure_info->exposure_time*ar0820_ctxt->p_mode->pclk/1000;
    exposure_info->line_count = STD_MIN((unsigned int)((exposure_info->exposure_time * ar0820_ctxt->p_mode->pclk) /
            (AR0820_ROPS * ar0820_ctxt->p_mode->line_length_pck * 1000)), (ar0820_ctxt->p_mode->frame_length_lines - 1));

    SLOW("AEC - AR0820 : exp_time=%f => line_cnt=0x%x", exposure_info->exposure_time, exposure_info->line_count);

    exposure_info->reg_gain = ar0820_get_gain_table_idx(exposure_info->real_gain);

    exposure_info->sensor_analog_gain = gain_table[exposure_info->reg_gain].real_gain;
    exposure_info->sensor_digital_gain = exposure_info->real_gain / exposure_info->sensor_analog_gain;

    SLOW("AEC - AR0820 :idx = %d, gain=%0.2f, a_gain=%0.2f + d_gain=%0.2f",
            exposure_info->reg_gain, exposure_info->real_gain,
            exposure_info->sensor_analog_gain, exposure_info->sensor_digital_gain);

    return 0;
}


static int ar0820_apply_exposure(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    ar0820_contxt_t* ar0820_ctxt = (ar0820_contxt_t*)pSensor->pPrivCtxt;
    struct camera_i2c_reg_array *exp_reg_array = ar0820_ctxt->exp_reg_array;
    int rc = 0;

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SERR("ar0820 %d start in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }
    else if(exposure_info->exposure_mode_type == QCARCAM_EXPOSURE_MANUAL ||
       exposure_info->exposure_mode_type == QCARCAM_EXPOSURE_AUTO)
    {
        uint32 exp_size = 0;

        float real_gain = gain_table[exposure_info->reg_gain].real_gain;
        char analog_gain = gain_table[exposure_info->reg_gain].analog_gain;
        char analog_fine_gain = gain_table[exposure_info->reg_gain].analog_fine_gain;
        char lcg_hcg = gain_table[exposure_info->reg_gain].lcg_hcg;

        SLOW("AEC - AR0820 : coarse=0x%x, real_gain=%0.2f analog_gain %d analog_fine_gain %d lcg_hcg %d",
                exposure_info->line_count, real_gain, analog_gain, analog_fine_gain, lcg_hcg);

        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_GROUPED_PARAMETER_HOLD, 0x01, _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_COARSE_INTEGRATION_TIME, (exposure_info->line_count & 0xffff), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_FINE_INTEGRATION_TIME, FINE_INTEGRATION_TIME, _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_ANALOG_GAIN, AR0820_PACK_ANALOG_GAIN(analog_gain & 0xf), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_ANALOG_FINE_GAIN, AR0820_PACK_ANALOG_GAIN(analog_fine_gain & 0xf), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_DC_GAIN, (lcg_hcg ? 0xFF : 0x00), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size,
            AR0820_REG_DIG_GLOBAL_GAIN, STD_MIN(FLOAT_TO_Q(9, exposure_info->sensor_digital_gain), 0x7FF), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_GROUPED_PARAMETER_HOLD, 0x00, _ar0820_delay_);

        ar0820_ctxt->cam_sensor_reg_setting.reg_array = exp_reg_array;
        ar0820_ctxt->cam_sensor_reg_setting.size = exp_size;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0820_ctxt->cam_sensor_reg_setting);
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

static int ar0820_apply_hdr_exposure(max9296_context_t* ctxt, uint32 link, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    ar0820_contxt_t* ar0820_ctxt = (ar0820_contxt_t*)pSensor->pPrivCtxt;
    struct camera_i2c_reg_array *exp_reg_array = ar0820_ctxt->exp_reg_array;
    int rc = 0;

    /* Temporary disable hdr exposure configuration */
    if (1)
    {
        SENSOR_WARN("HDR exposure config not implemented.");
        return rc;
    }

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {

        SERR("ar0820 %d start in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    switch(hdr_exposure->exposure_mode_type)
    {
    case QCARCAM_EXPOSURE_LUX_IDX:
    {
        unsigned int lux_idx = (unsigned int)hdr_exposure->lux_index;

        if (lux_idx >= ar0820_ctxt->num_steps)
        {
            SENSOR_ERROR("lux_index (%d) out of bounds (max = %d)", lux_idx, ar0820_ctxt->num_steps);
            return CAMERA_EBADPARM;
        }

        float exposure_time = ar0820_ctxt->exp_table[lux_idx].exp;
        float real_gain = ar0820_ctxt->exp_table[lux_idx].real_gain;

        unsigned int pclk_count = exposure_time * ar0820_ctxt->p_mode->pclk / 1000;
        unsigned int line_count = STD_MIN((unsigned int)(pclk_count / ar0820_ctxt->p_mode->line_length_pck)-1,
                (ar0820_ctxt->p_mode->frame_length_lines - 1));

        SLOW("AEC - AR0820 : exp_time=%f => line_cnt=0x%x", exposure_time, line_count);

        unsigned int gain_idx = ar0820_get_gain_table_idx(real_gain);
        float dig_gain = real_gain / gain_table[gain_idx].real_gain;
        char analog_gain = gain_table[gain_idx].analog_gain;
        char lcg_hcg = gain_table[gain_idx].lcg_hcg;

        SLOW("AEC - AR0820 :idx = %d, gain=%0.2f, a_gain=%0.2f + d_gain=%0.2f",
            gain_idx, real_gain, analog_gain, dig_gain);

        uint32 exp_size = 0;

        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_GROUPED_PARAMETER_HOLD, 0x01, _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_COARSE_INTEGRATION_TIME, (line_count & 0xffff), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_FINE_INTEGRATION_TIME, FINE_INTEGRATION_TIME, _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_ANALOG_GAIN, AR0820_PACK_ANALOG_GAIN(analog_gain & 0xf), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_DC_GAIN, (lcg_hcg ? 0xFF : 0x00), _ar0820_delay_);
        //GLOBAL_GAIN (0x305E) : Q7 <- USE ONLY FOR WHITE BALANCING
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_DIG_GLOBAL_GAIN,
            STD_MIN(FLOAT_TO_Q(9, dig_gain), 0x7FF), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_GROUPED_PARAMETER_HOLD, 0x00, _ar0820_delay_);

        ar0820_ctxt->cam_sensor_reg_setting.reg_array = exp_reg_array;
        ar0820_ctxt->cam_sensor_reg_setting.size = exp_size;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0820_ctxt->cam_sensor_reg_setting);
        if (rc)
        {
            SERR("sensor 0x%x failed to start", pSensor->sensor_alias);
        }

        break;
    }
    case QCARCAM_EXPOSURE_MANUAL:
    case QCARCAM_EXPOSURE_AUTO:
    {
        unsigned int i;
        ar0820_hdr_exp_t hdr_exp[3];

        for (i = 0; i < 3; i++)
        {
            hdr_exp[i].pclk_count = hdr_exposure->exposure_time[i] * ar0820_ctxt->p_mode->pclk / 1000 - FINE_INTEGRATION_TIME_12BIT_HDR_MODE;
            hdr_exp[i].line_count = STD_MIN((unsigned int)(hdr_exp[i].pclk_count / ar0820_ctxt->p_mode->line_length_pck) + 1,
                    (ar0820_ctxt->p_mode->frame_length_lines - 1));
            SLOW("AEC - AR0820 : [%d] exp_time=%f => line_cnt=0x%x", i, hdr_exposure->exposure_time[i], hdr_exp[i].line_count);

            unsigned int gain_idx = ar0820_get_gain_table_idx(hdr_exposure->gain[i]);
            hdr_exp[i].analog_gain = gain_table[gain_idx].analog_gain;
            hdr_exp[i].dig_gain = hdr_exposure->gain[i] / gain_table[gain_idx].real_gain;
            hdr_exp[i].lcg_hcg = gain_table[gain_idx].lcg_hcg;

            SLOW("AEC - AR0820 :gain_idx = %d, gain=%0.2f, a_gain=%0.2f + d_gain=%0.2f",
                gain_idx, hdr_exposure->gain[i], hdr_exp[i].analog_gain, hdr_exp[i].dig_gain);
        }

        uint32 exp_size = 0;

        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_GROUPED_PARAMETER_HOLD, 0x01, _ar0820_delay_);

        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_COARSE_INTEGRATION_TIME, (hdr_exp[0].line_count & 0xffff), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_COARSE_INTEGRATION_TIME2, (hdr_exp[1].line_count & 0xffff), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_COARSE_INTEGRATION_TIME3, (hdr_exp[2].line_count & 0xffff), _ar0820_delay_);

        unsigned short analog_gain = hdr_exp[0].analog_gain | (hdr_exp[1].analog_gain << 4) | (hdr_exp[2].analog_gain << 8);
        unsigned short lcg_hcg = hdr_exp[0].lcg_hcg | (hdr_exp[1].lcg_hcg << 1) | (hdr_exp[2].lcg_hcg << 2);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_ANALOG_GAIN, AR0820_PACK_ANALOG_GAIN(analog_gain & 0xf), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_DC_GAIN, (lcg_hcg ? 0xFF : 0x00), _ar0820_delay_);


        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_DIG_GLOBAL_GAIN, STD_MIN(FLOAT_TO_Q(7, hdr_exp[0].dig_gain), 0x7FF), _ar0820_delay_);
        ADD_I2C_REG_ARRAY(exp_reg_array, exp_size, AR0820_REG_GROUPED_PARAMETER_HOLD, 0x00, _ar0820_delay_);

        ar0820_ctxt->cam_sensor_reg_setting.reg_array = exp_reg_array;
        ar0820_ctxt->cam_sensor_reg_setting.size = exp_size;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0820_ctxt->cam_sensor_reg_setting);
        if (rc)
        {
            SERR("sensor 0x%x failed to set manual exp", pSensor->sensor_alias);
        }

        break;
    }
    case QCARCAM_EXPOSURE_SEMI_AUTO:
    default:
        SENSOR_WARN("Not Implemented");
        break;

    }

    return rc;
}
