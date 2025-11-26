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

package vendor.qti.automotive.qcarcam;
@Backing(type="int") @VintfStability
enum QcarcamColorFmt {
  QCARCAM_FMT_MIPIRAW_8 = 0x1080000,
  QCARCAM_FMT_MIPIRAW_10 = 0x10a0000,
  QCARCAM_FMT_MIPIRAW_12 = 0x10c0000,
  QCARCAM_FMT_MIPIRAW_14 = 0x10e0000,
  QCARCAM_FMT_MIPIRAW_16 = 0x1100000,
  QCARCAM_FMT_MIPIRAW_20 = 0x1140000,
  QCARCAM_FMT_RGB_888 = 0x7080300,
  QCARCAM_FMT_UYVY_8 = 0x7080102,
  QCARCAM_FMT_UYVY_10 = 0x70a0102,
  QCARCAM_FMT_UYVY_12 = 0x70c0102,
  QCARCAM_FMT_YUYV_8 = 0x7080100,
  QCARCAM_FMT_YUYV_10 = 0x70a0100,
  QCARCAM_FMT_YUYV_12 = 0x70c0100,
  QCARCAM_FMT_NV12 = 0x7080104,
  QCARCAM_FMT_NV21 = 0x7080105,
  QCARCAM_FMT_MAX = 0x7FFFFFFF,
  QCARCAM_FMT_PLAIN16_10 = 0x50a0000,
  QCARCAM_FMT_PLAIN16_12 = 0x50c0000,
  QCARCAM_FMT_PLAIN16_14 = 0x50e0000,
  QCARCAM_FMT_PLAIN16_16 = 0x5100000,
  QCARCAM_FMT_PLAIN32_20 = 0x6140000,
  QCARCAM_FMT_YU12 = 0x7080106,
  QCARCAM_FMT_YV12 = 0x7080107,
  QCARCAM_FMT_MIPIUYVY_10 = 0x10a0102,
  QCARCAM_FMT_P010 = 0x50a0104,
  QCARCAM_FMT_BGR_888 = 0x7180301,
  QCARCAM_FMT_RGB_565 = 0x7100302,
  QCARCAM_FMT_RGBX_8888 = 0x7200303,
  QCARCAM_FMT_BGRX_8888 = 0x7200304,
  QCARCAM_FMT_RGBX_1010102 = 0x7200305,
  QCARCAM_FMT_BGRX_1010102 = 0x7200306,
}
