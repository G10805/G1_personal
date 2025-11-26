/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.AiScalerRoi;
import vendor.qti.hardware.vpp.VppKeyValueInt;
import vendor.qti.hardware.vpp.VppMode;

@VintfStability
parcelable VppCtrlAis {
    VppMode mode;
    AiScalerRoi roi;
    /*
     * ! Range: VPP_AIS_CLASSIFICATION_MIN - VPP_AIS_CLASSIFICATION_MAX
     */
    int classification;
    VppKeyValueInt[] ctrls;
}
