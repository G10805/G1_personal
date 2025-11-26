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
@VintfStability
union QcarcamStreamConfigData {
  long ptr_value;
  float float_value;
  int uint_value;
  vendor.qti.automotive.qcarcam.QcarcamResolution res_value;
  vendor.qti.automotive.qcarcam.QcarcamColorFmt color_value;
  vendor.qti.automotive.qcarcam.QcarcamExposureConfig exposure_config;
  vendor.qti.automotive.qcarcam.QcarcamHDRExposureConfig hdr_exposure_config;
  vendor.qti.automotive.qcarcam.QcarcamFrameRate frame_rate_config;
  vendor.qti.automotive.qcarcam.QcarcamGammaConfig gamma_config;
  int[32] arr_padding;
  vendor.qti.automotive.qcarcam.QcarcamIspCtrls isp_ctrls;
  vendor.qti.automotive.qcarcam.QcarcamVendorParam vendor_param;
  vendor.qti.automotive.qcarcam.QcarcamBatchModeConfig batch_config;
  long uint64_value;
  vendor.qti.automotive.qcarcam.QcarcamIspUsecaseConfig isp_config;
}
