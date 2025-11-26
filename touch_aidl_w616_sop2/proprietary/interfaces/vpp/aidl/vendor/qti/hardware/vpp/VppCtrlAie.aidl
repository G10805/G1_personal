/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppHueMode;
import vendor.qti.hardware.vpp.VppMode;

@VintfStability
parcelable VppCtrlAie {
    VppMode mode;
    VppHueMode hueMode;
    /*
     * ! Range: VPP_AIE_CADE_LEVEL_MIN - VPP_AIE_CADE_LEVEL_MAX
     */
    int cadeLevel;
    /*
     * ! Range: VPP_AIE_LTM_LEVEL_MIN - VPP_AIE_LTM_LEVEL_MAX
     */
    int ltmLevel;
    /*
     * ! Range: VPP_AIE_LTM_SAT_GAIN_MIN - VPP_AIE_LTM_SAT_GAIN_MAX
     */
    int ltmSatGain;
    /*
     * ! Range: VPP_AIE_LTM_SAT_OFFSET_MIN - VPP_AIE_LTM_SAT_OFFSET_MAX
     */
    int ltmSatOffset;
    /*
     * ! Range: VPP_AIE_LTM_ACE_STR_MIN - VPP_AIE_LTM_ACE_STR_MAX
     */
    int ltmAceStr;
    /*
     * ! Range: VPP_AIE_LTM_ACE_BRI_L_MIN - VPP_AIE_LTM_ACE_BRI_L_MAX
     */
    int ltmAceBriL;
    /*
     * ! Range: VPP_AIE_LTM_ACE_BRI_H_MIN - VPP_AIE_LTM_ACE_BRI_H_MAX
     */
    int ltmAceBriH;
}
