/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppColorFormat {
    VPP_COLOR_FORMAT_NV12_VENUS,
    VPP_COLOR_FORMAT_NV21_VENUS,
    VPP_COLOR_FORMAT_P010,
    VPP_COLOR_FORMAT_UBWC_NV12,
    VPP_COLOR_FORMAT_UBWC_NV21,
    VPP_COLOR_FORMAT_UBWC_TP10,
    VPP_COLOR_FORMAT_RGBA8,
    VPP_COLOR_FORMAT_BGRA8,
    VPP_COLOR_FORMAT_UBWC_RGBA8,
    VPP_COLOR_FORMAT_UBWC_BGRA8,
    VPP_COLOR_FORMAT_UBWC_RGB565,
    VPP_COLOR_FORMAT_UBWC_BGR565,
}
