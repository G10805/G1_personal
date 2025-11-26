/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppError {
    VPP_OK = 0,
    VPP_ERR,
    VPP_PENDING,
    VPP_ERR_STATE,
    VPP_ERR_INVALID_CFG,
    VPP_ERR_PARAM,
    VPP_ERR_NO_MEM,
    VPP_ERR_RESOURCES,
    VPP_ERR_HW,
    VPP_ERR_FATAL,
}
