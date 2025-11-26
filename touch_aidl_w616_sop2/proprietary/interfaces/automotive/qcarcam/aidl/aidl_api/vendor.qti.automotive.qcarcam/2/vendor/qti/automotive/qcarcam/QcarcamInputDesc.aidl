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
enum QcarcamInputDesc {
  QCARCAM_INPUT_TYPE_EXT_REAR = 0,
  QCARCAM_INPUT_TYPE_EXT_FRONT = 1,
  QCARCAM_INPUT_TYPE_EXT_LEFT = 2,
  QCARCAM_INPUT_TYPE_EXT_RIGHT = 3,
  QCARCAM_INPUT_TYPE_DRIVER = 4,
  QCARCAM_INPUT_TYPE_LANE_WATCH = 5,
  QCARCAM_INPUT_TYPE_DIGITAL_MEDIA = 6,
  QCARCAM_INPUT_TYPE_ANALOG_MEDIA = 7,
  QCARCAM_INPUT_TYPE_GESTURE = 8,
  QCARCAM_INPUT_TYPE_IRIS = 9,
  QCARCAM_INPUT_TYPE_FINGERPRINT = 10,
  QCARCAM_INPUT_TYPE_TUNER = 11,
  QCARCAM_INPUT_TYPE_TESTPATTERN = 255,
  QCARCAM_INPUT_TYPE_USER_DEFINED_START = 256,
  QCARCAM_INPUT_NUM,
  QCARCAM_INPUT_MAX = 0x7FFFFFFF,
}
