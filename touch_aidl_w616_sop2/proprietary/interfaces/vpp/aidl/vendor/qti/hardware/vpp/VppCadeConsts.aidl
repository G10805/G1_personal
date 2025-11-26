/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppCadeConsts {
    VPP_CADE_LEVEL_MIN = 0,
    VPP_CADE_LEVEL_MAX = 100,
    VPP_CADE_CONTRAST_MIN = -50,
    VPP_CADE_CONTRAST_MAX = 50,
    VPP_CADE_SATURATION_MIN = -50,
    VPP_CADE_SATURATION_MAX = 50,
}
