/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppMode;

@VintfStability
parcelable VppCtrlCnr {
    VppMode mode;
    /*
     * ! Range: VPP_CNR_LEVEL_MIN - VPP_CNR_LEVEL_MAX
     */
    int level;
}
