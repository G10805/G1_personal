/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlRobustLocationInfo {
    int validMask;
    boolean enable;
    boolean enableForE911;
    byte major;
    int minor;
}

