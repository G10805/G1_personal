/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppSplitDirection {
    VPP_SPLIT_LEFT_TO_RIGHT,
    VPP_SPLIT_RIGHT_TO_LEFT,
    VPP_SPLIT_TOP_TO_BOTTOM,
    VPP_SPLIT_BOTTOM_TO_TOP,
}
