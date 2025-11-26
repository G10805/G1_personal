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
enum QcarcamCtrlControlSceneMode {
  QCARCAM_HAL_SCENE_MODE_DISABLED,
  QCARCAM_HAL_SCENE_MODE_FACE_PRIORITY,
  QCARCAM_HAL_SCENE_MODE_ACTION,
  QCARCAM_HAL_SCENE_MODE_PORTRAIT,
  QCARCAM_HAL_SCENE_MODE_LANDSCAPE,
  QCARCAM_HAL_SCENE_MODE_NIGHT,
  QCARCAM_HAL_SCENE_MODE_NIGHT_PORTRAIT,
  QCARCAM_HAL_SCENE_MODE_THEATRE,
  QCARCAM_HAL_SCENE_MODE_BEACH,
  QCARCAM_HAL_SCENE_MODE_SNOW,
  QCARCAM_HAL_SCENE_MODE_SUNSET,
  QCARCAM_HAL_SCENE_MODE_STEADYPHOTO,
  QCARCAM_HAL_SCENE_MODE_FIREWORKS,
  QCARCAM_HAL_SCENE_MODE_SPORTS,
  QCARCAM_HAL_SCENE_MODE_PARTY,
  QCARCAM_HAL_SCENE_MODE_CANDLELIGHT,
  QCARCAM_HAL_SCENE_MODE_BARCODE,
  QCARCAM_HAL_SCENE_MODE_HIGH_SPEED_VIDEO,
  QCARCAM_HAL_SCENE_MODE_HDR,
  QCARCAM_HAL_SCENE_MODE_FACE_PRIORITY_LOW_LIGHT,
  QCARCAM_HAL_SCENE_MODE_DEVICE_CUSTOM_START = 100,
  QCARCAM_HAL_SCENE_MODE_DEVICE_CUSTOM_END = 127,
}
