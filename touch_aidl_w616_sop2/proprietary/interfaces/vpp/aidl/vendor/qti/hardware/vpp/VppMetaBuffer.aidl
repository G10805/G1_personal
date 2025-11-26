/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppPortParam;
import android.hardware.common.NativeHandle;

@VintfStability
parcelable VppMetaBuffer {
    String key;
    NativeHandle handleFd;
    int allocLen;
    VppPortParam params;
}
