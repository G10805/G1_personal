/**
 * @file max96712_pattern_gen.h
 *
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MAXIM_PATTERN_GEN_H__
#define __MAXIM_PATTERN_GEN_H__

#include "max96712_lib.h"

#define SENSOR_WIDTH_2MP_10    1920
#define SENSOR_HEIGHT_2MP_10   1200

#define SENSOR_WIDTH_2MP_30    1920
#define SENSOR_HEIGHT_2MP_30   1080

#define SENSOR_WIDTH_8MP    3840
#define SENSOR_HEIGHT_8MP   2160

#define SENSOR_FORMAT   QCARCAM_FMT_RGB_888
#define SENSOR_DT       CSI_DT_RGB888

/*CONFIGURATION OPTIONS*/
#define _maxim_pattern_gen_delay_ 0

// Generate pattern 1920 x 1080 30fps color bar 2 lanes
#define START_REG_ARRAY_TPG_2MP_2LN \
{ \
{ 0x1050, 0xF3, _maxim_pattern_gen_delay_}, \
{ 0x1051, 0x20, _maxim_pattern_gen_delay_}, \
{ 0x1052, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x1053, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x1054, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x1055, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x1056, 0x30, _maxim_pattern_gen_delay_}, \
{ 0x1057, 0x48, _maxim_pattern_gen_delay_}, \
{ 0x1058, 0x22, _maxim_pattern_gen_delay_}, \
{ 0x1059, 0xE4, _maxim_pattern_gen_delay_}, \
{ 0x105A, 0x8, _maxim_pattern_gen_delay_}, \
{ 0x105B, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x105C, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x105D, 0x1, _maxim_pattern_gen_delay_}, \
{ 0x105E, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x105F, 0x14, _maxim_pattern_gen_delay_}, \
{ 0x1060, 0x7, _maxim_pattern_gen_delay_}, \
{ 0x1061, 0xF8, _maxim_pattern_gen_delay_}, \
{ 0x1062, 0x4, _maxim_pattern_gen_delay_}, \
{ 0x1063, 0x5C, _maxim_pattern_gen_delay_}, \
{ 0x1064, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x1065, 0xA9, _maxim_pattern_gen_delay_}, \
{ 0x1066, 0x12, _maxim_pattern_gen_delay_}, \
{ 0x1067, 0x7, _maxim_pattern_gen_delay_}, \
{ 0x1068, 0x80, _maxim_pattern_gen_delay_}, \
{ 0x1069, 0x0, _maxim_pattern_gen_delay_}, \
{ 0x106A, 0x8C, _maxim_pattern_gen_delay_}, \
{ 0x106B, 0x4, _maxim_pattern_gen_delay_}, \
{ 0x106C, 0x38, _maxim_pattern_gen_delay_}, \
{ 0x106D, 0x4, _maxim_pattern_gen_delay_}, \
{ 0x1074, 0x50, _maxim_pattern_gen_delay_}, \
{ 0x1075, 0xA0, _maxim_pattern_gen_delay_}, \
{ 0x1076, 0x50, _maxim_pattern_gen_delay_}, \
{ 0x09, 0x01, _maxim_pattern_gen_delay_}, \
{ 0x1DC, 0x00, _maxim_pattern_gen_delay_}, \
{ 0x09, 0x01, _maxim_pattern_gen_delay_}, \
{ 0x1FC, 0x00, _maxim_pattern_gen_delay_}, \
{ 0x09, 0x01, _maxim_pattern_gen_delay_}, \
{ 0x21C, 0x00, _maxim_pattern_gen_delay_}, \
{ 0x09, 0x01, _maxim_pattern_gen_delay_}, \
{ 0x23C, 0x00, _maxim_pattern_gen_delay_}, \
{ 0xF4, 0x01, _maxim_pattern_gen_delay_}, \
{ 0x8A3, 0x44, _maxim_pattern_gen_delay_}, \
{ 0x8A4, 0x44, _maxim_pattern_gen_delay_}, \
{ 0x415, 0x2F, _maxim_pattern_gen_delay_}, \
{ 0x90A, 0x40, _maxim_pattern_gen_delay_}, \
{ 0x8A0, 0x81, _maxim_pattern_gen_delay_}, \
}

// Generate pattern 1920 x 1080 10fps
#define START_REG_ARRAY_TPG_2MP_10 \
{ \
{ 0x0013,0x40,   _max96712_reset_delay_}, \
{ 0x1050,0xF3,   _maxim_pattern_gen_delay_}, \
{ 0x1051,0x10,   _maxim_pattern_gen_delay_}, \
{ 0x1052,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1053,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1054,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1055,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1056,0x1F,   _maxim_pattern_gen_delay_}, \
{ 0x1057,0x94,   _maxim_pattern_gen_delay_}, \
{ 0x1058,0x26,   _maxim_pattern_gen_delay_}, \
{ 0x1059,0x05,   _maxim_pattern_gen_delay_}, \
{ 0x105A,0xF5,   _maxim_pattern_gen_delay_}, \
{ 0x105B,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x105C,0xEC,   _maxim_pattern_gen_delay_}, \
{ 0x105D,0x94,   _maxim_pattern_gen_delay_}, \
{ 0x105E,0x07,   _maxim_pattern_gen_delay_}, \
{ 0x105F,0xDB,   _maxim_pattern_gen_delay_}, \
{ 0x1060,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1061,0x0A,   _maxim_pattern_gen_delay_}, \
{ 0x1062,0x04,   _maxim_pattern_gen_delay_}, \
{ 0x1063,0xD5,   _maxim_pattern_gen_delay_}, \
{ 0x1064,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1065,0xEC,   _maxim_pattern_gen_delay_}, \
{ 0x1066,0xD6,   _maxim_pattern_gen_delay_}, \
{ 0x1067,0x07,   _maxim_pattern_gen_delay_}, \
{ 0x1068,0x80,   _maxim_pattern_gen_delay_}, \
{ 0x1069,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x106A,0x65,   _maxim_pattern_gen_delay_}, \
{ 0x106B,0x04,   _maxim_pattern_gen_delay_}, \
{ 0x106C,0xB0,   _maxim_pattern_gen_delay_}, \
{ 0x106E,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x106F,0xA9,   _maxim_pattern_gen_delay_}, \
{ 0x1070,0xB2,   _maxim_pattern_gen_delay_}, \
{ 0x1071,0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1072,0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1073,0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1074,0x50,   _maxim_pattern_gen_delay_}, \
{ 0x1075,0x50,   _maxim_pattern_gen_delay_}, \
{ 0x1076,0x50,   _maxim_pattern_gen_delay_}, \
{ 0x0009,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x00F4,0x0F,   _maxim_pattern_gen_delay_}, \
{ 0x08A3,0xE4,   _maxim_pattern_gen_delay_}, \
{ 0x08A4,0xE4,   _maxim_pattern_gen_delay_}, \
{ 0x01D0,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D1,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D2,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D3,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D4,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D5,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D6,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D7,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E0,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E1,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E2,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E3,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E4,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E5,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E6,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E7,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x0208,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x0209,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020A,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020B,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020C,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020D,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020E,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020F,0x20,   _maxim_pattern_gen_delay_}, \
{ 0x090B,0x07,   _maxim_pattern_gen_delay_}, \
{ 0x092D,0x15,   _maxim_pattern_gen_delay_}, \
{ 0x090D,0x24,   _maxim_pattern_gen_delay_}, \
{ 0x090E,0x24,   _maxim_pattern_gen_delay_}, \
{ 0x090F,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0910,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0911,0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0912,0x01,   _maxim_pattern_gen_delay_}, \
{ 0x094B,0x07,   _maxim_pattern_gen_delay_}, \
{ 0x096D,0x15,   _maxim_pattern_gen_delay_}, \
{ 0x094D,0x24,   _maxim_pattern_gen_delay_}, \
{ 0x094E,0x64,   _maxim_pattern_gen_delay_}, \
{ 0x094F,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0950,0x40,   _maxim_pattern_gen_delay_}, \
{ 0x0951,0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0952,0x41,   _maxim_pattern_gen_delay_}, \
{ 0x098B,0x07,   _maxim_pattern_gen_delay_}, \
{ 0x09AD,0x2A,   _maxim_pattern_gen_delay_}, \
{ 0x098D,0x24,   _maxim_pattern_gen_delay_}, \
{ 0x098E,0x24,   _maxim_pattern_gen_delay_}, \
{ 0x098F,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0990,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0991,0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0992,0x01,   _maxim_pattern_gen_delay_}, \
{ 0x09CB,0x07,   _maxim_pattern_gen_delay_}, \
{ 0x09ED,0x2A,   _maxim_pattern_gen_delay_}, \
{ 0x09CD,0x24,   _maxim_pattern_gen_delay_}, \
{ 0x09CE,0x64,   _maxim_pattern_gen_delay_}, \
{ 0x09CF,0x00,   _maxim_pattern_gen_delay_}, \
{ 0x09D0,0x40,   _maxim_pattern_gen_delay_}, \
{ 0x09D1,0x01,   _maxim_pattern_gen_delay_}, \
{ 0x09D2,0x41,   _maxim_pattern_gen_delay_}, \
{ 0x0418,0x24,   _maxim_pattern_gen_delay_}, \
{ 0x041B,0x24,   _maxim_pattern_gen_delay_}, \
{ 0x08A0,0x84,   _maxim_pattern_gen_delay_}, \
}

// Generate pattern 1920 x 1080 30fps
#define START_REG_ARRAY_TPG_2MP_30 \
{ \
{ 0x0013, 0x40,   _max96712_reset_delay_}, \
{ 0x1050, 0xF3,   _maxim_pattern_gen_delay_}, \
{ 0x1051, 0x10,   _maxim_pattern_gen_delay_}, \
{ 0x1052, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1053, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1054, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1055, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1056, 0x21,   _maxim_pattern_gen_delay_}, \
{ 0x1057, 0xC0,   _maxim_pattern_gen_delay_}, \
{ 0x1058, 0x25,   _maxim_pattern_gen_delay_}, \
{ 0x1059, 0x99,   _maxim_pattern_gen_delay_}, \
{ 0x105A, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x105B, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x105C, 0x5f,   _maxim_pattern_gen_delay_}, \
{ 0x105D, 0xC4,   _maxim_pattern_gen_delay_}, \
{ 0x105E, 0x08,   _maxim_pattern_gen_delay_}, \
{ 0x105F, 0x6C,   _maxim_pattern_gen_delay_}, \
{ 0x1060, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1061, 0x2C,   _maxim_pattern_gen_delay_}, \
{ 0x1062, 0x04,   _maxim_pattern_gen_delay_}, \
{ 0x1063, 0xD5,   _maxim_pattern_gen_delay_}, \
{ 0x1064, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x1065, 0x60,   _maxim_pattern_gen_delay_}, \
{ 0x1066, 0x58,   _maxim_pattern_gen_delay_}, \
{ 0x1067, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x1068, 0x80,   _maxim_pattern_gen_delay_}, \
{ 0x1069, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x106A, 0x18,   _maxim_pattern_gen_delay_}, \
{ 0x106B, 0x04,   _maxim_pattern_gen_delay_}, \
{ 0x106C, 0x38,   _maxim_pattern_gen_delay_}, \
{ 0x106E, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x106F, 0xA9,   _maxim_pattern_gen_delay_}, \
{ 0x1070, 0xB2,   _maxim_pattern_gen_delay_}, \
{ 0x1071, 0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1072, 0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1073, 0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1074, 0x50,   _maxim_pattern_gen_delay_}, \
{ 0x1075, 0x50,   _maxim_pattern_gen_delay_}, \
{ 0x1076, 0x50,   _maxim_pattern_gen_delay_}, \
{ 0x0009, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x00F4, 0x0F,   _maxim_pattern_gen_delay_}, \
{ 0x08A3, 0xE4,   _maxim_pattern_gen_delay_}, \
{ 0x08A4, 0xE4,   _maxim_pattern_gen_delay_}, \
{ 0x01D0, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D1, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D2, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D3, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D4, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D5, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D6, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D7, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E0, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E1, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E2, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E3, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E4, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E5, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E6, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E7, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x0208, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x0209, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020A, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020B, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020C, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020D, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020E, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020F, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x090B, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x092D, 0x15,   _maxim_pattern_gen_delay_}, \
{ 0x090D, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x090E, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x090F, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0910, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0911, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0912, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x094B, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x096D, 0x15,   _maxim_pattern_gen_delay_}, \
{ 0x094D, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x094E, 0x64,   _maxim_pattern_gen_delay_}, \
{ 0x094F, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0950, 0x40,   _maxim_pattern_gen_delay_}, \
{ 0x0951, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0952, 0x41,   _maxim_pattern_gen_delay_}, \
{ 0x098B, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x09AD, 0x2A,   _maxim_pattern_gen_delay_}, \
{ 0x098D, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x098E, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x098F, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0990, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0991, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0992, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x09CB, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x09ED, 0x2A,   _maxim_pattern_gen_delay_}, \
{ 0x09CD, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x09CE, 0x64,   _maxim_pattern_gen_delay_}, \
{ 0x09CF, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x09D0, 0x40,   _maxim_pattern_gen_delay_}, \
{ 0x09D1, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x09D2, 0x41,   _maxim_pattern_gen_delay_}, \
{ 0x0418, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x041B, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x08A0, 0x84,   _maxim_pattern_gen_delay_}, \
}

// Generate pattern 3840 x 2160 30fps
#define START_REG_ARRAY_TPG_8MP \
{ \
{ 0x0013, 0x40,   _max96712_reset_delay_}, \
{ 0x1050, 0xF3,   _maxim_pattern_gen_delay_}, \
{ 0x1051, 0x10,   _maxim_pattern_gen_delay_}, \
{ 0x1052, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1053, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1054, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1055, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1056, 0x5d,   _maxim_pattern_gen_delay_}, \
{ 0x1057, 0x48,   _maxim_pattern_gen_delay_}, \
{ 0x1058, 0x84,   _maxim_pattern_gen_delay_}, \
{ 0x1059, 0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x105A, 0xA8,   _maxim_pattern_gen_delay_}, \
{ 0x105B, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x105C, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x105D, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x105E, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x105F, 0x14,   _maxim_pattern_gen_delay_}, \
{ 0x1060, 0x0F,   _maxim_pattern_gen_delay_}, \
{ 0x1061, 0x78,   _maxim_pattern_gen_delay_}, \
{ 0x1062, 0x08,   _maxim_pattern_gen_delay_}, \
{ 0x1063, 0x94,   _maxim_pattern_gen_delay_}, \
{ 0x1064, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x1065, 0x46,   _maxim_pattern_gen_delay_}, \
{ 0x1066, 0x92,   _maxim_pattern_gen_delay_}, \
{ 0x1067, 0x0F,   _maxim_pattern_gen_delay_}, \
{ 0x1068, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x1069, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x106A, 0x8C,   _maxim_pattern_gen_delay_}, \
{ 0x106B, 0x08,   _maxim_pattern_gen_delay_}, \
{ 0x106C, 0x70,   _maxim_pattern_gen_delay_}, \
{ 0x106E, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x106F, 0xA9,   _maxim_pattern_gen_delay_}, \
{ 0x1070, 0xB2,   _maxim_pattern_gen_delay_}, \
{ 0x1071, 0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1072, 0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1073, 0xFF,   _maxim_pattern_gen_delay_}, \
{ 0x1074, 0x50,   _maxim_pattern_gen_delay_}, \
{ 0x1075, 0x50,   _maxim_pattern_gen_delay_}, \
{ 0x1076, 0x50,   _maxim_pattern_gen_delay_}, \
{ 0x0009, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x00F4, 0x0F,   _maxim_pattern_gen_delay_}, \
{ 0x08A3, 0xE4,   _maxim_pattern_gen_delay_}, \
{ 0x08A4, 0xE4,   _maxim_pattern_gen_delay_}, \
{ 0x01D0, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D1, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D2, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D3, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D4, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D5, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D6, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01D7, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E0, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E1, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E2, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E3, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E4, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E5, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E6, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x01E7, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x0208, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x0209, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020A, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020B, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020C, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020D, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020E, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x020F, 0x20,   _maxim_pattern_gen_delay_}, \
{ 0x090B, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x092D, 0x15,   _maxim_pattern_gen_delay_}, \
{ 0x090D, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x090E, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x090F, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0910, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0911, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0912, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x094B, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x096D, 0x15,   _maxim_pattern_gen_delay_}, \
{ 0x094D, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x094E, 0x64,   _maxim_pattern_gen_delay_}, \
{ 0x094F, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0950, 0x40,   _maxim_pattern_gen_delay_}, \
{ 0x0951, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0952, 0x41,   _maxim_pattern_gen_delay_}, \
{ 0x098B, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x09AD, 0x2A,   _maxim_pattern_gen_delay_}, \
{ 0x098D, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x098E, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x098F, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0990, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x0991, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x0992, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x09CB, 0x07,   _maxim_pattern_gen_delay_}, \
{ 0x09ED, 0x2A,   _maxim_pattern_gen_delay_}, \
{ 0x09CD, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x09CE, 0x64,   _maxim_pattern_gen_delay_}, \
{ 0x09CF, 0x00,   _maxim_pattern_gen_delay_}, \
{ 0x09D0, 0x40,   _maxim_pattern_gen_delay_}, \
{ 0x09D1, 0x01,   _maxim_pattern_gen_delay_}, \
{ 0x09D2, 0x41,   _maxim_pattern_gen_delay_}, \
{ 0x0418, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x041B, 0x24,   _maxim_pattern_gen_delay_}, \
{ 0x08A0, 0x84,   _maxim_pattern_gen_delay_}, \
}

typedef enum
{
    MAXIM_TPG_MODE_DEFAULT = 0,
    MAXIM_TPG_MODE_2MP_10 = MAXIM_TPG_MODE_DEFAULT,
    MAXIM_TPG_MODE_2MP_30,
    MAXIM_TPG_MODE_8MP,
    MAXIM_TPG_MODE_MAX
}maxim_tpg_mode_t;

#endif /* __MAXIM_PATTERN_GEN_H__ */
