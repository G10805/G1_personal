/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppError;
import vendor.qti.hardware.vpp.VppKeyValueDouble;
import vendor.qti.hardware.vpp.VppKeyValueFloat;
import vendor.qti.hardware.vpp.VppKeyValueInt;
import vendor.qti.hardware.vpp.VppKeyValueString;

@VintfStability
parcelable VppRequirements {
    VppError eResult;
    VppKeyValueInt[] vppReqInt;
    VppKeyValueDouble[] vppReqDouble;
    VppKeyValueFloat[] vppReqFloat;
    VppKeyValueString[] vppReqString;
}
