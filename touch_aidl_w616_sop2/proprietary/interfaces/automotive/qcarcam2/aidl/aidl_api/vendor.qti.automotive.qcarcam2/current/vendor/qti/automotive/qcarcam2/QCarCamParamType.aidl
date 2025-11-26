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
enum QCarCamParamType {
  QCARCAM_STREAM_CONFIG_PARAM_BASE = 0x00000000,
  QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK,
  QCARCAM_STREAM_CONFIG_PARAM_SET_CROP,
  QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL,
  QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL,
  QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE,
  QCARCAM_STREAM_CONFIG_PARAM_MASTER,
  QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE,
  QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE,
  QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE,
  QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE,
  QCARCAM_SENSOR_PARAM_BASE = 0x00000100,
  QCARCAM_SENSOR_PARAM_MIRROR_H,
  QCARCAM_SENSOR_PARAM_MIRROR_V,
  QCARCAM_SENSOR_PARAM_VID_STD,
  QCARCAM_SENSOR_PARAM_CURRENT_VID_STD,
  QCARCAM_SENSOR_PARAM_SIGNAL_STATUS,
  QCARCAM_SENSOR_PARAM_EXPOSURE,
  QCARCAM_SENSOR_PARAM_GAMMA,
  QCARCAM_SENSOR_PARAM_BRIGHTNESS,
  QCARCAM_SENSOR_PARAM_CONTRAST,
  QCARCAM_SENSOR_PARAM_HUE,
  QCARCAM_SENSOR_PARAM_SATURATION,
  QCARCAM_SENSOR_PARAM_COLOR_SPACE,
  QCARCAM_VENDOR_PARAM_BASE = 0x00000300,
  QCARCAM_VENDOR_PARAM,
  QCARCAM_PARAM_MAX = 0x7FFFFFFF,
}
