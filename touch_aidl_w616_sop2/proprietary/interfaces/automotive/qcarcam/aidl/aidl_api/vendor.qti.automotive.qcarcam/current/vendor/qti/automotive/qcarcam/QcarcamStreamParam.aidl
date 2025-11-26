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
enum QcarcamStreamParam {
  QCARCAM_CB_EVENT_MASK = 1,
  QCARCAM_COLOR_FMT,
  QCARCAM_RESOLUTION,
  QCARCAM_BRIGHTNESS,
  QCARCAM_CONTRAST,
  QCARCAM_MIRROR_H,
  QCARCAM_MIRROR_V,
  QCARCAM_FRAME_RATE,
  QCARCAM_VID_STD,
  QCARCAM_CURRENT_VID_STD,
  QCARCAM_STATUS,
  QCARCAM_LATENCY_MAX,
  QCARCAM_LATENCY_REDUCE_RATE,
  QCARCAM_PRIVATE_DATA,
  QCARCAM_INJECTION_START,
  QCARCAM_EXPOSURE,
  QCARCAM_HUE,
  QCARCAM_SATURATION,
  QCARCAM_GAMMA,
  QCARCAM_HDR_EXPOSURE,
  QCARCAM_OPMODE,
  QCARCAM_RECOVERY,
  QCARCAM_ISP_CTRLS,
  QCARCAM_PARAM_NUM,
  QCARCAM_PARAM_MAX = 0x7FFFFFFF,
  QCARCAM_VENDOR = 24,
  QCARCAM_INPUT_MODE,
  QCARCAM_MASTER,
  QCARCAM_EVENT_CHANGE_SUBSCRIBE,
  QCARCAM_EVENT_CHANGE_UNSUBSCRIBE,
  QCARCAM_BATCH_MODE,
  QCARCAM_ISP_USECASE,
}
