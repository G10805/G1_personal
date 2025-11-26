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

package vendor.qti.hardware.vpp;
@Backing(type="int") @VintfStability
enum VppFrcMode {
  VPP_FRC_MODE_OFF,
  VPP_FRC_MODE_SMOOTH_MOTION,
  VPP_FRC_MODE_SLOMO,
  VPP_FRC_MODE_VIDEO_CUSTOM,
  VPP_FRC_MODE_GAME,
  VPP_FRC_MODE_GAME_CUSTOM,
  VPP_FRC_MODE_CAMERA_LOW_LIGHT,
  VPP_FRC_MODE_CAMERA_SLOMO,
  VPP_FRC_MODE_CAMERA_CUSTOM,
}
