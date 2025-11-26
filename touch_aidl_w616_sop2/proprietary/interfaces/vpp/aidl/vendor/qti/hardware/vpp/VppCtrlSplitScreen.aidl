/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppSplitDirection;

@VintfStability
parcelable VppCtrlSplitScreen {
    /*
     * ! Range: VPP_SPLIT_PERCENT_MIN - VPP_SPLIT_PERCENT_MAX
     */
    int processPercent;
    VppSplitDirection processDirection;
}
