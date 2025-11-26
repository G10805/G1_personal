/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
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
