/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.

package vendor.qti.automotive.qcarcam2;
@Backing(type="int") @VintfStability
enum QCarCamColorFmt {
  QCARCAM_FMT_MIPIRAW_8 = 0x1080000,
  QCARCAM_FMT_MIPIRAW_10 = 0x10a0000,
  QCARCAM_FMT_MIPIRAW_12 = 0x10c0000,
  QCARCAM_FMT_MIPIRAW_14 = 0x10e0000,
  QCARCAM_FMT_MIPIRAW_16 = 0x1100000,
  QCARCAM_FMT_MIPIRAW_20 = 0x1140000,
  QCARCAM_FMT_PLAIN16_10 = 0x30a0000,
  QCARCAM_FMT_PLAIN16_12 = 0x30c0000,
  QCARCAM_FMT_PLAIN16_14 = 0x30e0000,
  QCARCAM_FMT_PLAIN16_16 = 0x3100000,
  QCARCAM_FMT_PLAIN32_20 = 0x4140000,
  QCARCAM_FMT_UYVY_8 = 0x80102,
  QCARCAM_FMT_UYVY_10 = 0xa0102,
  QCARCAM_FMT_UYVY_12 = 0xc0102,
  QCARCAM_FMT_YUYV_8 = 0x80100,
  QCARCAM_FMT_YUYV_10 = 0xa0100,
  QCARCAM_FMT_YUYV_12 = 0xc0100,
  QCARCAM_FMT_NV12 = 0x80104,
  QCARCAM_FMT_NV21 = 0x80105,
  QCARCAM_FMT_YU12 = 0x80106,
  QCARCAM_FMT_YV12 = 0x80107,
  QCARCAM_FMT_MIPIUYVY_10 = 0x10a0102,
  QCARCAM_FMT_P010 = 0x30a0104,
  QCARCAM_FMT_RGB_888 = 0x180300,
  QCARCAM_FMT_BGR_888 = 0x180301,
  QCARCAM_FMT_RGB_565 = 0x100302,
  QCARCAM_FMT_RGBX_8888 = 0x200303,
  QCARCAM_FMT_BGRX_8888 = 0x200304,
  QCARCAM_FMT_RGBX_1010102 = 0x200305,
  QCARCAM_FMT_BGRX_1010102 = 0x200306,
  QCARCAM_FMT_MAX = 0x7FFFFFFF,
}
