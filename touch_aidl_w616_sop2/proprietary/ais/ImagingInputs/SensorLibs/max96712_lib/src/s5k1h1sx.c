/**
 * @file s5k1h1sx.c
 *
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "s5k1h1sx.h"

#define INIT_SENSOR \
{ \
    { 0x102A, 0x0005, _s5k1h1sx_delay_}, \
    { 0x102C, 0x00F0, _s5k1h1sx_delay_}, \
    { 0x102E, 0x0000, _s5k1h1sx_delay_}, \
    { 0x101C, 0x0005, _s5k1h1sx_delay_}, \
    { 0x101E, 0x00D4, _s5k1h1sx_delay_}, \
    { 0x70C4, 0x000A, _s5k1h1sx_delay_}, \
    { 0x70D8, 0x0034, _s5k1h1sx_delay_}, \
    { 0x70DA, 0x000F, _s5k1h1sx_delay_}, \
    { 0x70DC, 0x000B, _s5k1h1sx_delay_}, \
    { 0x70C2, 0x000D, _s5k1h1sx_delay_}, \
    { 0x70C0, 0x0013, _s5k1h1sx_delay_}, \
    { 0x70D6, 0x000E, _s5k1h1sx_delay_}, \
    { 0x70D4, 0x0009, _s5k1h1sx_delay_}, \
    { 0x70B0, 0x000F, _s5k1h1sx_delay_}, \
    { 0x3500, 0x025F, _s5k1h1sx_delay_}, \
    { 0x3508, 0x0000, _s5k1h1sx_delay_}, \
    { 0x3528, 0x0020, _s5k1h1sx_delay_}, \
    { 0x3532, 0x0001, _s5k1h1sx_delay_}, \
    { 0x354C, 0x0088, _s5k1h1sx_delay_}, \
    { 0x354E, 0x0088, _s5k1h1sx_delay_}, \
    { 0x3558, 0x0006, _s5k1h1sx_delay_}, \
    { 0x35CC, 0x0050, _s5k1h1sx_delay_}, \
    { 0x35DA, 0x004C, _s5k1h1sx_delay_}, \
    { 0x35E4, 0x000B, _s5k1h1sx_delay_}, \
    { 0x35E6, 0x000F, _s5k1h1sx_delay_}, \
    { 0x35E8, 0x0007, _s5k1h1sx_delay_}, \
    { 0x35EE, 0x000A, _s5k1h1sx_delay_}, \
    { 0x35C8, 0x0030, _s5k1h1sx_delay_}, \
    { 0x35D6, 0x0073, _s5k1h1sx_delay_}, \
    { 0x35DE, 0x0060, _s5k1h1sx_delay_}, \
    { 0x3648, 0x0080, _s5k1h1sx_delay_}, \
    { 0x3516, 0x004C, _s5k1h1sx_delay_}, \
    { 0x3606, 0x0006, _s5k1h1sx_delay_}, \
    { 0x361C, 0x0070, _s5k1h1sx_delay_}, \
    { 0x3620, 0x0001, _s5k1h1sx_delay_}, \
    { 0x3622, 0x0001, _s5k1h1sx_delay_}, \
    { 0x3624, 0x0001, _s5k1h1sx_delay_}, \
    { 0x3626, 0x0001, _s5k1h1sx_delay_}, \
    { 0x6014, 0x0000, _s5k1h1sx_delay_}, \
    { 0x6016, 0x0000, _s5k1h1sx_delay_}, \
    { 0x0120, 0x0003, _s5k1h1sx_delay_}, \
    { 0x3016, 0x0000, _s5k1h1sx_delay_}, \
    { 0x325A, 0x0F10, _s5k1h1sx_delay_}, \
    { 0x32EA, 0x07B2, _s5k1h1sx_delay_}, \
    { 0x8006, 0x0000, _s5k1h1sx_delay_}, \
    { 0x800A, 0x0011, _s5k1h1sx_delay_}, \
    { 0x800C, 0x0002, _s5k1h1sx_delay_}, \
    { 0x8010, 0x0004, _s5k1h1sx_delay_}, \
    { 0x8012, 0x0001, _s5k1h1sx_delay_}, \
    { 0x8022, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8026, 0x9C46, _s5k1h1sx_delay_}, \
    { 0x8028, 0x05A6, _s5k1h1sx_delay_}, \
    { 0x802C, 0x0004, _s5k1h1sx_delay_}, \
    { 0x803A, 0x0001, _s5k1h1sx_delay_}, \
    { 0x8300, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8302, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8304, 0x0022, _s5k1h1sx_delay_}, \
    { 0x8306, 0x0F10, _s5k1h1sx_delay_}, \
    { 0x8308, 0x0794, _s5k1h1sx_delay_}, \
    { 0x830A, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8400, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8402, 0x01BF, _s5k1h1sx_delay_}, \
    { 0x8410, 0x0100, _s5k1h1sx_delay_}, \
    { 0x842E, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8800, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8802, 0x017C, _s5k1h1sx_delay_}, \
    { 0x8808, 0x0100, _s5k1h1sx_delay_}, \
    { 0x880C, 0x0100, _s5k1h1sx_delay_}, \
    { 0x880E, 0x0000, _s5k1h1sx_delay_}, \
    { 0x894E, 0x0800, _s5k1h1sx_delay_}, \
    { 0x8950, 0x04CD, _s5k1h1sx_delay_}, \
    { 0x89AE, 0x0800, _s5k1h1sx_delay_}, \
    { 0x89B0, 0x04CD, _s5k1h1sx_delay_}, \
    { 0x88AC, 0x0085, _s5k1h1sx_delay_}, \
    { 0x98AC, 0x0085, _s5k1h1sx_delay_}, \
    { 0x8A00, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8A02, 0x0001, _s5k1h1sx_delay_}, \
    { 0x8A04, 0x00FB, _s5k1h1sx_delay_}, \
    { 0x8A06, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8A08, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8A0A, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8A0C, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8A0E, 0x0800, _s5k1h1sx_delay_}, \
    { 0x8A10, 0x0E70, _s5k1h1sx_delay_}, \
    { 0x8A12, 0x1880, _s5k1h1sx_delay_}, \
    { 0x8A14, 0x2040, _s5k1h1sx_delay_}, \
    { 0x8A16, 0x2BD0, _s5k1h1sx_delay_}, \
    { 0x8A18, 0x3460, _s5k1h1sx_delay_}, \
    { 0x8A1A, 0x40E0, _s5k1h1sx_delay_}, \
    { 0x8A1C, 0x4A00, _s5k1h1sx_delay_}, \
    { 0x8A1E, 0x5120, _s5k1h1sx_delay_}, \
    { 0x8A20, 0x5700, _s5k1h1sx_delay_}, \
    { 0x8A22, 0x6050, _s5k1h1sx_delay_}, \
    { 0x8A24, 0x67A0, _s5k1h1sx_delay_}, \
    { 0x8A26, 0x6DA0, _s5k1h1sx_delay_}, \
    { 0x8A28, 0x7720, _s5k1h1sx_delay_}, \
    { 0x8A2A, 0x8490, _s5k1h1sx_delay_}, \
    { 0x8A2C, 0x8E20, _s5k1h1sx_delay_}, \
    { 0x8A2E, 0x9590, _s5k1h1sx_delay_}, \
    { 0x8A30, 0x9BB0, _s5k1h1sx_delay_}, \
    { 0x8A32, 0xA540, _s5k1h1sx_delay_}, \
    { 0x8A34, 0xACC0, _s5k1h1sx_delay_}, \
    { 0x8A36, 0xB2D0, _s5k1h1sx_delay_}, \
    { 0x8A38, 0xBC70, _s5k1h1sx_delay_}, \
    { 0x8A3A, 0xC3F0, _s5k1h1sx_delay_}, \
    { 0x8A3C, 0xCA00, _s5k1h1sx_delay_}, \
    { 0x8A3E, 0xD3A0, _s5k1h1sx_delay_}, \
    { 0x8A40, 0xDB20, _s5k1h1sx_delay_}, \
    { 0x8A42, 0xE660, _s5k1h1sx_delay_}, \
    { 0x8A44, 0xEED0, _s5k1h1sx_delay_}, \
    { 0x8A46, 0xF590, _s5k1h1sx_delay_}, \
    { 0x8A48, 0xFFF0, _s5k1h1sx_delay_}, \
    { 0x8A4A, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8A4C, 0x0000, _s5k1h1sx_delay_}, \
    { 0x8A4E, 0x0002, _s5k1h1sx_delay_}, \
    { 0x8A50, 0x0004, _s5k1h1sx_delay_}, \
    { 0x8A52, 0x0008, _s5k1h1sx_delay_}, \
    { 0x8A54, 0x000C, _s5k1h1sx_delay_}, \
    { 0x8A56, 0x0014, _s5k1h1sx_delay_}, \
    { 0x8A58, 0x001C, _s5k1h1sx_delay_}, \
    { 0x8A5A, 0x002C, _s5k1h1sx_delay_}, \
    { 0x8A5C, 0x003C, _s5k1h1sx_delay_}, \
    { 0x8A5E, 0x004C, _s5k1h1sx_delay_}, \
    { 0x8A60, 0x005C, _s5k1h1sx_delay_}, \
    { 0x8A62, 0x007C, _s5k1h1sx_delay_}, \
    { 0x8A64, 0x009C, _s5k1h1sx_delay_}, \
    { 0x8A66, 0x00BC, _s5k1h1sx_delay_}, \
    { 0x8A68, 0x00FC, _s5k1h1sx_delay_}, \
    { 0x8A6A, 0x017C, _s5k1h1sx_delay_}, \
    { 0x8A6C, 0x01FC, _s5k1h1sx_delay_}, \
    { 0x8A6E, 0x027C, _s5k1h1sx_delay_}, \
    { 0x8A70, 0x02FC, _s5k1h1sx_delay_}, \
    { 0x8A72, 0x03FC, _s5k1h1sx_delay_}, \
    { 0x8A74, 0x04FC, _s5k1h1sx_delay_}, \
    { 0x8A76, 0x05FC, _s5k1h1sx_delay_}, \
    { 0x8A78, 0x07FC, _s5k1h1sx_delay_}, \
    { 0x8A7A, 0x09FC, _s5k1h1sx_delay_}, \
    { 0x8A7C, 0x0BFC, _s5k1h1sx_delay_}, \
    { 0x8A7E, 0x0FFC, _s5k1h1sx_delay_}, \
    { 0x8A80, 0x13FC, _s5k1h1sx_delay_}, \
    { 0x8A82, 0x1BFC, _s5k1h1sx_delay_}, \
    { 0x8A84, 0x23FC, _s5k1h1sx_delay_}, \
    { 0x8A86, 0x2BFC, _s5k1h1sx_delay_}, \
    { 0x8A88, 0x3BFC, _s5k1h1sx_delay_}, \
    { 0x8A8A, 0x0080, _s5k1h1sx_delay_}, \
    { 0x8422, 0x0400, _s5k1h1sx_delay_}, \
    { 0x8428, 0x0400, _s5k1h1sx_delay_}, \
    { 0x8424, 0x0800, _s5k1h1sx_delay_}, \
    { 0x8426, 0x0800, _s5k1h1sx_delay_}, \
    { 0x842A, 0x0002, _s5k1h1sx_delay_}, \
    { 0x0202, 0x0095, _s5k1h1sx_delay_}, \
    { 0x0200, 0x0E80, _s5k1h1sx_delay_}, \
    { 0x0216, 0x0012, _s5k1h1sx_delay_}, \
    { 0x0300, 0x1698, _s5k1h1sx_delay_}, \
    { 0x0218, 0x0002, _s5k1h1sx_delay_}, \
    { 0x0400, 0x0B23, _s5k1h1sx_delay_}, \
    { 0x0204, 0x0040, _s5k1h1sx_delay_}, \
    { 0x0206, 0x0040, _s5k1h1sx_delay_}, \
    { 0x0208, 0x0040, _s5k1h1sx_delay_}, \
    { 0x70F6, 0x0003, _s5k1h1sx_delay_}, \
    { 0x70FC, 0x0004, _s5k1h1sx_delay_}, \
    { 0x32B0, 0x0001, _s5k1h1sx_delay_}, \
    { 0x32B8, 0x0002, _s5k1h1sx_delay_}, \
    { 0x32BA, 0x000E, _s5k1h1sx_delay_}, \
    { 0x3220, 0x0003, _s5k1h1sx_delay_}, \
    { 0x3228, 0x0028, _s5k1h1sx_delay_}, \
    { 0x322A, 0x0F10, _s5k1h1sx_delay_}, \
    { 0x32E8, 0x000E, _s5k1h1sx_delay_}, \
    { 0x3628, 0x07C2, _s5k1h1sx_delay_}, \
    { 0x362A, 0x07C6, _s5k1h1sx_delay_}, \
    { 0x362C, 0x07CA, _s5k1h1sx_delay_}, \
    { 0x362E, 0x07CE, _s5k1h1sx_delay_}, \
    { 0x328A, 0x0000, _s5k1h1sx_delay_}, \
    { 0x3298, 0x07DC, _s5k1h1sx_delay_}, \
    { 0x329A, 0x0010, _s5k1h1sx_delay_}, \
    { 0x70B8, 0x0F10, _s5k1h1sx_delay_}, \
    { 0x70F2, 0x0790, _s5k1h1sx_delay_}, \
    { 0x70BA, 0x0790, _s5k1h1sx_delay_}, \
    { 0x7038, 0x1698, _s5k1h1sx_delay_}, \
    { 0x700A, 0x0142, _s5k1h1sx_delay_}, \
    { 0x7042, 0x0E06, _s5k1h1sx_delay_}, \
    { 0x70B4, 0x000C, _s5k1h1sx_delay_}, \
    { 0x331A, 0x0F10, _s5k1h1sx_delay_}, \
    { 0x0340, 0x0D44, _s5k1h1sx_delay_}, \
    { 0x3018, 0x0000, _s5k1h1sx_delay_}, \
    { 0x70A0, 0x007D, _s5k1h1sx_delay_}, \
    { 0x9A0C, 0x0000, _s5k1h1sx_delay_}, \
    { 0x9A0E, 0x0800, _s5k1h1sx_delay_}, \
    { 0x9A10, 0x0E70, _s5k1h1sx_delay_}, \
    { 0x9A12, 0x1880, _s5k1h1sx_delay_}, \
    { 0x9A14, 0x2040, _s5k1h1sx_delay_}, \
    { 0x9A16, 0x2BD0, _s5k1h1sx_delay_}, \
    { 0x9A18, 0x3460, _s5k1h1sx_delay_}, \
    { 0x9A1A, 0x40E0, _s5k1h1sx_delay_}, \
    { 0x9A1C, 0x4A00, _s5k1h1sx_delay_}, \
    { 0x9A1E, 0x5120, _s5k1h1sx_delay_}, \
    { 0x9A20, 0x5700, _s5k1h1sx_delay_}, \
    { 0x9A22, 0x6050, _s5k1h1sx_delay_}, \
    { 0x9A24, 0x67A0, _s5k1h1sx_delay_}, \
    { 0x9A26, 0x6DA0, _s5k1h1sx_delay_}, \
    { 0x9A28, 0x7720, _s5k1h1sx_delay_}, \
    { 0x9A2A, 0x8490, _s5k1h1sx_delay_}, \
    { 0x9A2C, 0x8E20, _s5k1h1sx_delay_}, \
    { 0x9A2E, 0x9590, _s5k1h1sx_delay_}, \
    { 0x9A30, 0x9BB0, _s5k1h1sx_delay_}, \
    { 0x9A32, 0xA540, _s5k1h1sx_delay_}, \
    { 0x9A34, 0xACC0, _s5k1h1sx_delay_}, \
    { 0x9A36, 0xB2D0, _s5k1h1sx_delay_}, \
    { 0x9A38, 0xBC70, _s5k1h1sx_delay_}, \
    { 0x9A3A, 0xC3F0, _s5k1h1sx_delay_}, \
    { 0x9A3C, 0xCA00, _s5k1h1sx_delay_}, \
    { 0x9A3E, 0xD3A0, _s5k1h1sx_delay_}, \
    { 0x9A40, 0xDB20, _s5k1h1sx_delay_}, \
    { 0x9A42, 0xE660, _s5k1h1sx_delay_}, \
    { 0x9A44, 0xEED0, _s5k1h1sx_delay_}, \
    { 0x9A46, 0xF590, _s5k1h1sx_delay_}, \
    { 0x9A48, 0xFFF0, _s5k1h1sx_delay_}, \
    { 0x9A4A, 0x0000, _s5k1h1sx_delay_}, \
    { 0x9A4C, 0x0000, _s5k1h1sx_delay_}, \
    { 0x9A4E, 0x0002, _s5k1h1sx_delay_}, \
    { 0x9A50, 0x0004, _s5k1h1sx_delay_}, \
    { 0x9A52, 0x0008, _s5k1h1sx_delay_}, \
    { 0x9A54, 0x000C, _s5k1h1sx_delay_}, \
    { 0x9A56, 0x0014, _s5k1h1sx_delay_}, \
    { 0x9A58, 0x001C, _s5k1h1sx_delay_}, \
    { 0x9A5A, 0x002C, _s5k1h1sx_delay_}, \
    { 0x9A5C, 0x003C, _s5k1h1sx_delay_}, \
    { 0x9A5E, 0x004C, _s5k1h1sx_delay_}, \
    { 0x9A60, 0x005C, _s5k1h1sx_delay_}, \
    { 0x9A62, 0x007C, _s5k1h1sx_delay_}, \
    { 0x9A64, 0x009C, _s5k1h1sx_delay_}, \
    { 0x9A66, 0x00BC, _s5k1h1sx_delay_}, \
    { 0x9A68, 0x00FC, _s5k1h1sx_delay_}, \
    { 0x9A6A, 0x017C, _s5k1h1sx_delay_}, \
    { 0x9A6C, 0x01FC, _s5k1h1sx_delay_}, \
    { 0x9A6E, 0x027C, _s5k1h1sx_delay_}, \
    { 0x9A70, 0x02FC, _s5k1h1sx_delay_}, \
    { 0x9A72, 0x03FC, _s5k1h1sx_delay_}, \
    { 0x9A74, 0x04FC, _s5k1h1sx_delay_}, \
    { 0x9A76, 0x05FC, _s5k1h1sx_delay_}, \
    { 0x9A78, 0x07FC, _s5k1h1sx_delay_}, \
    { 0x9A7A, 0x09FC, _s5k1h1sx_delay_}, \
    { 0x9A7C, 0x0BFC, _s5k1h1sx_delay_}, \
    { 0x9A7E, 0x0FFC, _s5k1h1sx_delay_}, \
    { 0x9A80, 0x13FC, _s5k1h1sx_delay_}, \
    { 0x9A82, 0x1BFC, _s5k1h1sx_delay_}, \
    { 0x9A84, 0x23FC, _s5k1h1sx_delay_}, \
    { 0x9A86, 0x2BFC, _s5k1h1sx_delay_}, \
    { 0x9A88, 0x3BFC, _s5k1h1sx_delay_}, \
    { 0x9A8A, 0x0080, _s5k1h1sx_delay_}, \
    { 0x0140, 0x0003, _s5k1h1sx_delay_}, \
    { 0x0244, 0x0040, _s5k1h1sx_delay_}, \
    { 0x0246, 0x0040, _s5k1h1sx_delay_}, \
    { 0x0248, 0x0040, _s5k1h1sx_delay_}, \
    { 0x0222, 0x0095, _s5k1h1sx_delay_}, \
    { 0x0220, 0x0E80, _s5k1h1sx_delay_}, \
    { 0x0226, 0x0012, _s5k1h1sx_delay_}, \
    { 0x0320, 0x1698, _s5k1h1sx_delay_}, \
    { 0x0228, 0x0002, _s5k1h1sx_delay_}, \
    { 0x0420, 0x0B23, _s5k1h1sx_delay_}, \
    { 0x7122, 0x0031, _s5k1h1sx_delay_}, \
    { 0x720A, 0x0142, _s5k1h1sx_delay_}, \
    { 0x7238, 0x1698, _s5k1h1sx_delay_}, \
    { 0x7242, 0x0E06, _s5k1h1sx_delay_}, \
    { 0x72B8, 0x0F10, _s5k1h1sx_delay_}, \
    { 0x72BA, 0x0790, _s5k1h1sx_delay_}, \
    { 0x72FC, 0x0790, _s5k1h1sx_delay_}, \
    { 0x72F6, 0x0003, _s5k1h1sx_delay_}, \
    { 0x9026, 0x9C46, _s5k1h1sx_delay_}, \
    { 0x9028, 0x05A6, _s5k1h1sx_delay_}, \
    { 0x902C, 0x0004, _s5k1h1sx_delay_}, \
    { 0x903A, 0x0001, _s5k1h1sx_delay_}, \
    { 0x9300, 0x0000, _s5k1h1sx_delay_}, \
    { 0x9302, 0x0000, _s5k1h1sx_delay_}, \
    { 0x9304, 0x0022, _s5k1h1sx_delay_}, \
    { 0x9306, 0x0F10, _s5k1h1sx_delay_}, \
    { 0x9308, 0x0794, _s5k1h1sx_delay_}, \
    { 0x930A, 0x0000, _s5k1h1sx_delay_}, \
    { 0x980E, 0x0000, _s5k1h1sx_delay_}, \
    { 0x994E, 0x0800, _s5k1h1sx_delay_}, \
    { 0x9950, 0x04CD, _s5k1h1sx_delay_}, \
    { 0x99AE, 0x0800, _s5k1h1sx_delay_}, \
    { 0x99B0, 0x04CD, _s5k1h1sx_delay_}, \
}

#define INIT_SER \
{ \
    { 0x0330, 0x00, _s5k1h1sx_delay_}, \
    { 0x0331, 0x30, _s5k1h1sx_delay_}, \
    { 0x0332, 0xE0, _s5k1h1sx_delay_}, \
    { 0x0311, 0x70, _s5k1h1sx_delay_}, \
    { 0x0308, 0x67, _s5k1h1sx_delay_}, \
    { 0x0002, 0x73, _s5k1h1sx_delay_}, \
    { 0x0314, 0x6C, _s5k1h1sx_delay_}, \
    { 0x0316, 0x52, _s5k1h1sx_delay_}, \
    { 0x0318, 0x70, _s5k1h1sx_delay_}, \
    { 0x0102, 0x0C, _s5k1h1sx_delay_}, \
    { 0x010A, 0x0C, _s5k1h1sx_delay_}, \
    { 0x0112, 0x0C, _s5k1h1sx_delay_}, \
    { 0x0312, 0x02, _s5k1h1sx_delay_}, \
    { 0x0313, 0x10, _s5k1h1sx_delay_}, \
    { 0x031C, 0x38, _s5k1h1sx_delay_}, \
    { 0x031D, 0x30, _s5k1h1sx_delay_}, \
}
#define START_SER {}
#define START_SENSOR \
{ \
    { 0x0100, 0x0001, _s5k1h1sx_delay_}, \
}
#define START_SENSOR_TPG_MODE0 \
{ \
    { 0x6040, 0x0001, _s5k1h1sx_delay_}, \
    { 0x6148, 0x0790, _s5k1h1sx_delay_}, \
    { 0x0100, 0x0001, _s5k1h1sx_delay_}, \
}
#define START_SENSOR_TPG_MODE1 \
{ \
    { 0x6040, 0x0005, _s5k1h1sx_delay_}, \
    { 0x6148, 0x0790, _s5k1h1sx_delay_}, \
    { 0x0100, 0x0001, _s5k1h1sx_delay_}, \
}
#define START_SENSOR_TPG_MODE2 \
{ \
    { 0x6040, 0x0006, _s5k1h1sx_delay_}, \
    { 0x6148, 0x0790, _s5k1h1sx_delay_}, \
    { 0x0100, 0x0001, _s5k1h1sx_delay_}, \
}

#define STOP_SER {}

static struct camera_i2c_reg_array s5k1h1sx_init_reg[] = INIT_SER;
static struct camera_i2c_reg_array s5k1h1sx_sensor_init_reg[] = INIT_SENSOR;
static struct camera_i2c_reg_array s5k1h1sx_sensor_start_reg[] = START_SENSOR;
static struct camera_i2c_reg_array s5k1h1sx_sensor_tpg_mode0_start_reg[] = START_SENSOR_TPG_MODE0;
static struct camera_i2c_reg_array s5k1h1sx_sensor_tpg_mode1_start_reg[] = START_SENSOR_TPG_MODE1;
static struct camera_i2c_reg_array s5k1h1sx_sensor_tpg_mode2_start_reg[] = START_SENSOR_TPG_MODE2;
static maxim_pipeline_t s5k1h1sx_isp_pipelines[S5K1H1SX_MODE_MAX] =
{
    [S5K1H1SX_MODE_8MP] =
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = SENSOR_FORMAT,
            .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 36.6f},
            .channel_info = {.vc = 0, .dt = SENSOR_DT, .cid = 0,},
        }
    },
    [S5K1H1SX_MODE_TPG_MODE0] =
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = SENSOR_FORMAT,
            .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 36.6f},
            .channel_info = {.vc = 0, .dt = SENSOR_DT, .cid = 0,},
        }
    },
    [S5K1H1SX_MODE_TPG_MODE1] =
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = SENSOR_FORMAT,
            .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 36.6f},
            .channel_info = {.vc = 0, .dt = SENSOR_DT, .cid = 0,},
        }
    },
    [S5K1H1SX_MODE_TPG_MODE2] =
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = SENSOR_FORMAT,
            .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 36.6f},
            .channel_info = {.vc = 0, .dt = SENSOR_DT, .cid = 0,},
        }
    },
};

static int s5k1h1sx_detect(max96712_context_t* ctxt, uint32 link);
static int s5k1h1sx_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg);
static int s5k1h1sx_init_link(max96712_context_t* ctxt, uint32 link);
static int s5k1h1sx_start_link(max96712_context_t* ctxt, uint32 link);
static int s5k1h1sx_stop_link(max96712_context_t* ctxt, uint32 link);

static max96712_sensor_t s5k1h1sx_info = {
    .id = MAXIM_SENSOR_ID_S5K1H1SX,
    .detect = s5k1h1sx_detect,
    .get_link_cfg = s5k1h1sx_get_link_cfg,

    .init_link = s5k1h1sx_init_link,
    .start_link = s5k1h1sx_start_link,
    .stop_link = s5k1h1sx_stop_link,
};

max96712_sensor_t* s5k1h1sx_get_sensor_info(void)
{
    return &s5k1h1sx_info;
}

typedef struct
{
    struct camera_i2c_reg_setting s5k1h1sx_reg_setting;
    struct camera_i2c_reg_setting s5k1h1sx_sensor_reg_setting;
}s5k1h1sx_contxt_t;

static int s5k1h1sx_create_ctxt(max96712_sensor_info_t* pSensor)
{
    int rc = 0;

    if (pSensor->pPrivCtxt)
    {
        SLOW("Ctxt already created");
        rc = 0;
    }
    else if (pSensor->mode >= S5K1H1SX_MODE_MAX)
    {
        SERR("Unsupported sensor mode %d", pSensor->mode);
        rc = -1;
    }
    else
    {
        s5k1h1sx_contxt_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(s5k1h1sx_contxt_t));
        if (pCtxt)
        {
            memset(pCtxt, 0x0, sizeof(*pCtxt));

            pCtxt->s5k1h1sx_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
            pCtxt->s5k1h1sx_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

            pCtxt->s5k1h1sx_sensor_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
            pCtxt->s5k1h1sx_sensor_reg_setting.data_type = CAMERA_I2C_WORD_DATA;

            pSensor->pPrivCtxt = pCtxt;
        }
        else
        {
            SERR("Failed to allocate sensor context");
            rc = -1;
        }
    }
    return rc;
}

static int s5k1h1sx_detect(max96712_context_t* ctxt, uint32 link)
{
    SHIGH("s5k1h1sx_detect() E");
    int rc = 0;
    int i = 0;
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    uint16 supported_ser_addr[] = {0x80, 0};// List of serializer addresses we support
    int num_addr = STD_ARRAY_SIZE(supported_ser_addr);
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};
    struct camera_i2c_reg_array rfclk_en[] = {{0, 0, 0}};
    struct camera_i2c_reg_array read_sensor_reg[] = {{0, 0, 0}};
    s5k1h1sx_contxt_t* s5k1h1sx_ctxt;
    
    rc = s5k1h1sx_create_ctxt(pSensor);
    if (rc)
    {
        SERR("Failed to create ctxt for link %d", link);
        return rc;
    }

    s5k1h1sx_ctxt = pSensor->pPrivCtxt;

    /*read chip id for serializer*/
    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array = read_reg;
    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.size = STD_ARRAY_SIZE(read_reg);
    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array[0].delay = 0;

    supported_ser_addr[num_addr-1] = pSensor->serializer_alias;
    
    /* Detect far end serializer */
    for (i = 0; i < num_addr; i++)
    {
        s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array[0].reg_addr = MSM_SER_CHIP_ID_REG_ADDR;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, supported_ser_addr[i], &s5k1h1sx_ctxt->s5k1h1sx_reg_setting);
        if (!rc)
        {
            pSensor->serializer_addr = supported_ser_addr[i];
            break;
        }
    }
    if (i == num_addr)
    {
        SENSOR_WARN("No Camera connected to Link %d of max96712 0x%x", link, pCtxt->slave_addr);
    }
    else if (pSensor->serializer_alias == pSensor->serializer_addr)
    {
        SENSOR_WARN("LINK %d already re-mapped 0x%x", link, pSensor->serializer_addr);
        rc = 0;
    }
    else
    {
        //remap serializer
        struct camera_i2c_reg_array remap_ser[] = {
            {0x0, pSensor->serializer_alias, _s5k1h1sx_delay_}
        };
        s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array = remap_ser;
        s5k1h1sx_ctxt->s5k1h1sx_reg_setting.size = STD_ARRAY_SIZE(remap_ser);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pSensor->serializer_addr,
                &s5k1h1sx_ctxt->s5k1h1sx_reg_setting);
        if (rc)
        {
            SERR("Failed to change serializer address (0x%x) of max96712 0x%x Link %d",
                pSensor->serializer_addr, pCtxt->slave_addr, link);
            rc = -1;
        }
        if(!rc)
        {
            /*read chip id for remapped serializer*/
            s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array = read_reg;
            s5k1h1sx_ctxt->s5k1h1sx_reg_setting.size = STD_ARRAY_SIZE(read_reg);
            s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array[0].reg_addr = 0x00;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                    pCtxt->ctrl,
                    pSensor->serializer_alias,
                    &s5k1h1sx_ctxt->s5k1h1sx_reg_setting);
            if (rc)
            {
                SERR("Failed to read S5K1H1SX SER (0x%x) after remap", pSensor->serializer_alias);
            }
            else
            {
                SHIGH("Detected S5K1H1SX SER alias 0x%x ",read_reg[0].reg_data);
            }
            if (pSensor->serializer_alias != s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array[0].reg_data)
            {
                SENSOR_WARN("Remote SER address remap failed: 0x%x, should be 0x%x",
                    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array[0].reg_data, pSensor->serializer_alias);
            }

            //link reset, remap sensor/camera, create broadcast addr,
            struct camera_i2c_reg_array remap_ser_2[] = {
                {0x0042, pSensor->sensor_alias, _s5k1h1sx_delay_},
                {0x0043, DEFAULT_SENSOR_ADDR, _s5k1h1sx_delay_}
            };

            s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array = remap_ser_2;
            s5k1h1sx_ctxt->s5k1h1sx_reg_setting.size = STD_ARRAY_SIZE(remap_ser_2);
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pSensor->serializer_alias,
                &s5k1h1sx_ctxt->s5k1h1sx_reg_setting);
            if (rc)
            {
                SERR("Failed to reset link %d and remap camera on serializer(0x%x)", link, pSensor->serializer_alias);
                return -1;
            }
        }
    }
 
    pSensor->state = SENSOR_STATE_DETECTED;
    
    /*enable rfclk to sensors connected to serializer*/ 
    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array = rfclk_en;
    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.size = STD_ARRAY_SIZE(rfclk_en);
    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array[0].delay = 0;
    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array[0].reg_addr = 0x0006;
    s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array[0].reg_data = 0xB0;
    
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &s5k1h1sx_ctxt->s5k1h1sx_reg_setting))) {
        SERR("S5K1H1SX SER (0x%x) unable to write to ser", pSensor->serializer_alias);
        rc = 0;
        //@TODO return -1;
    }
    else {
        /*try to read back from the sensor*/
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array = read_sensor_reg;
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.size = STD_ARRAY_SIZE(read_sensor_reg);
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array[0].delay = 0;
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array[0].reg_addr = 0x80F6;
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array[0].reg_data = 0;
        
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                pCtxt->ctrl,
                pSensor->sensor_alias,
                &s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting))) {
            SERR("S5K1H1SX SENSOR (0x%x) unable to read", pSensor->sensor_alias);
            rc = 0;
            //@TODO return -1;
        }
        SHIGH("Detected S5K1H1SX SENSOR default addr 0x%x with version 0x%x", pSensor->sensor_alias, s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array[0].reg_data);
    }    
    return rc;
}

static int s5k1h1sx_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg)
{
    (void)ctxt;
    unsigned int mode;
    if (ctxt->max96712_sensors[link].mode < S5K1H1SX_MODE_MAX)
    {
        mode = ctxt->max96712_sensors[link].mode;
    }
    else
    {
        SERR("The mode set is greater than tha MAX mode supported. Setting mode to Default.");
        mode = S5K1H1SX_MODE_DEFAULT;
    }
    p_cfg->num_pipelines = 1;
    p_cfg->pipelines[0] = s5k1h1sx_isp_pipelines[mode];

    return 0;
}

static int s5k1h1sx_init_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    s5k1h1sx_contxt_t* s5k1h1sx_ctxt = pSensor->pPrivCtxt;
    int rc = 0;
    
    SHIGH("s5k1h1sx_init_link() E");    
    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array = s5k1h1sx_init_reg;
        s5k1h1sx_ctxt->s5k1h1sx_reg_setting.size = STD_ARRAY_SIZE(s5k1h1sx_init_reg);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pSensor->serializer_alias,
                    &s5k1h1sx_ctxt->s5k1h1sx_reg_setting);
        if (rc)
        {
            SERR("Failed to init camera serializer(0x%x)", pSensor->serializer_alias);
            return -1;
        }
        pSensor->state = SENSOR_STATE_INITIALIZED;
        SHIGH("s5k1h1sx serilaizer init state %d", pSensor->state);
    }
    else
    {
        SERR("s5k1h1sx serilaizer at link %d init in wrong state %d", link, pSensor->state);
        rc = -1;
    }
    
    /* do the init for sensor connected to serializer */
    s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array = s5k1h1sx_sensor_init_reg;
    s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.size = STD_ARRAY_SIZE(s5k1h1sx_sensor_init_reg);
    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                DEFAULT_SENSOR_ADDR,
                &s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting);
    if (rc)
    {
        SERR("Failed to init camera serializer sensor(0x%x)", DEFAULT_SENSOR_ADDR);
        return -1;
    }

    return rc;
}

static int s5k1h1sx_start_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    s5k1h1sx_contxt_t* s5k1h1sx_ctxt = pSensor->pPrivCtxt;
    int rc = 0;
    
    SHIGH("s5k1h1sx_start_link()");
    
    if (S5K1H1SX_MODE_TPG_MODE0 == ctxt->max96712_sensors[link].mode)
    {
        SHIGH("TPG mode0 enter");
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array = s5k1h1sx_sensor_tpg_mode0_start_reg;
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.size = STD_ARRAY_SIZE(s5k1h1sx_sensor_tpg_mode0_start_reg);
    }
    else if (S5K1H1SX_MODE_TPG_MODE1 == ctxt->max96712_sensors[link].mode)
    {
        SHIGH("TPG mode1 enter");
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array = s5k1h1sx_sensor_tpg_mode1_start_reg;
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.size = STD_ARRAY_SIZE(s5k1h1sx_sensor_tpg_mode1_start_reg);        
    }
    else if (S5K1H1SX_MODE_TPG_MODE2 == ctxt->max96712_sensors[link].mode)
    {
        SHIGH("TPG mode2 enter");
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array = s5k1h1sx_sensor_tpg_mode2_start_reg;
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.size = STD_ARRAY_SIZE(s5k1h1sx_sensor_tpg_mode2_start_reg);
    }
    else
    {
        SHIGH("normal enter");
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.reg_array = s5k1h1sx_sensor_start_reg;
        s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting.size = STD_ARRAY_SIZE(s5k1h1sx_sensor_start_reg);
    }
    
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
        pCtxt->ctrl,
        DEFAULT_SENSOR_ADDR,
        &s5k1h1sx_ctxt->s5k1h1sx_sensor_reg_setting)))
    {
        SERR("Failed to start serializer sensor(0x%x)", DEFAULT_SENSOR_ADDR);
    }
    else
    {
        SHIGH("starting sensor (0x%x)", DEFAULT_SENSOR_ADDR);
    }
    
    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        //@TODO add start reg array here.
        SHIGH("starting serializer ");
    }
    else
    {
        SERR("serializer at link %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int s5k1h1sx_stop_link(max96712_context_t* ctxt, uint32 link)
{    
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];

    int rc = 0;
    
    SHIGH("s5k1h1sx_stop_link()");    
    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
#if 0
        s5k1h1sx_ctxt->s5k1h1sx_reg_setting.reg_array = s5k1h1sx_stop_reg;
        s5k1h1sx_ctxt->s5k1h1sx_reg_setting.size = STD_ARRAY_SIZE(s5k1h1sx_stop_reg);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &s5k1h1sx_ctxt->s5k1h1sx_reg_setting)))
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }
#endif
        SLOW("serilaizer stop (0x%x)", pSensor->serializer_alias);
    }
    else
    {
        SERR("serializer at link %d stop in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}
