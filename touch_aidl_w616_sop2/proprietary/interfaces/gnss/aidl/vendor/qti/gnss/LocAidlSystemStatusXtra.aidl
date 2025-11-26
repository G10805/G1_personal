/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlTime;

@VintfStability
parcelable LocAidlSystemStatusXtra {
    LocAidlTime mUtcTime;
    LocAidlTime mUtcReported;
    byte mXtraValidMask;
    int mGpsXtraAge;
    int mGloXtraAge;
    int mBdsXtraAge;
    int mGalXtraAge;
    int mQzssXtraAge;
    int mGpsXtraValid;
    int mGloXtraValid;
    long mBdsXtraValid;
    long mGalXtraValid;
    byte mQzssXtraValid;
}

