/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppFrcLevel;
import vendor.qti.hardware.vpp.VppFrcInterp;
import vendor.qti.hardware.vpp.VppFrcInterpRatio;
import vendor.qti.hardware.vpp.VppFrcLatency;
import vendor.qti.hardware.vpp.VppFrcMode;

@VintfStability
parcelable VppCtrlFrcSegment {
    VppFrcMode mode;
    VppFrcLevel level;
    VppFrcInterp interp;
    /*
     * ! Start of a new segment, in terms of timestamp of the input buffer
     */
    long tsStart;
    /*
     * ! Valid values: true, false
     */
    boolean frameCopyOnFallback;
    /*
     * ! Valid values: true, false
     */
    boolean frameCopyInput;
    /*
     * ! Range: VPP_FRC_SMART_FALLBACK_MIN - VPP_FRC_SMART_FALLBACK_MAX
     */
    int smartFallback;
    VppFrcInterpRatio custInterpRatio;
    VppFrcLatency latency;
}
