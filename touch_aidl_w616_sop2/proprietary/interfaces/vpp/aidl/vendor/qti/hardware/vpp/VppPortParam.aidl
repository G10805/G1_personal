/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

import vendor.qti.hardware.vpp.VppColorFormat;

@VintfStability
parcelable VppPortParam {
    int height;
    int width;
    int stride;
    int scanlines;
    VppColorFormat fmt;
}
