/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
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

package vendor.qti.gnss;
@Backing(type="int") @VintfStability
enum LocAidlLocationFlagsBits {
  LAT_LONG_BIT = 1,
  ALTITUDE_BIT = 2,
  SPEED_BIT = 4,
  BEARING_BIT = 8,
  ACCURACY_BIT = 16,
  VERTICAL_ACCURACY_BIT = 32,
  SPEED_ACCURACY_BIT = 64,
  BEARING_ACCURACY_BIT = 128,
  SPOOF_MASK_BIT = 256,
  ELAPSED_REAL_TIME_BIT = 512,
  CONFORMITY_INDEX_BIT = 1024,
  QUALITY_TYPE_BIT = 2048,
  TECH_MASK_BIT = 4096,
}
