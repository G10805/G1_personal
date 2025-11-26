/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppMode;

@VintfStability
parcelable VppCtrlCade {
    VppMode mode;
    /*
     * ! Range: VPP_CADE_LEVEL_MIN - VPP_CADE_LEVEL_MAX
     */
    int cadeLevel;
    /*
     * ! Range: VPP_CADE_CONTRAST_MIN - VPP_CADE_CONTRAST_MAX
     */
    int contrast;
    /*
     * ! Range: VPP_CADE_SATURATION_MIN - VPP_CADE_SATURATION_MAX
     */
    int saturation;
}
