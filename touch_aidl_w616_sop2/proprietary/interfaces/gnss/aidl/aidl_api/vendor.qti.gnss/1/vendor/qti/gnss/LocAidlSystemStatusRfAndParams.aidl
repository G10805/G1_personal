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
@VintfStability
parcelable LocAidlSystemStatusRfAndParams {
  vendor.qti.gnss.LocAidlTime mUtcTime;
  vendor.qti.gnss.LocAidlTime mUtcReported;
  int mPgaGain;
  int mGpsBpAmpI;
  int mGpsBpAmpQ;
  int mAdcI;
  int mAdcQ;
  int mJammerGps;
  int mJammerGlo;
  int mJammerBds;
  int mJammerGal;
  double mAgcGps;
  double mAgcGlo;
  double mAgcBds;
  double mAgcGal;
  int mGloBpAmpI;
  int mGloBpAmpQ;
  int mBdsBpAmpI;
  int mBdsBpAmpQ;
  int mGalBpAmpI;
  int mGalBpAmpQ;
}
