/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppEarMode {
    VPP_EAR_MODE_OFF,
    VPP_EAR_MODE_BYPASS,
    VPP_EAR_MODE_LOW,
    VPP_EAR_MODE_MEDIUM,
    VPP_EAR_MODE_HIGH,
    VPP_EAR_MODE_STREAM_ADAPTIVE,
    VPP_EAR_MODE_FRAME_ADAPTIVE,
}
