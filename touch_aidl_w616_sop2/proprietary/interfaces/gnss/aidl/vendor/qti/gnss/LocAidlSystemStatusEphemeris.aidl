/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlTime;

@VintfStability
parcelable LocAidlSystemStatusEphemeris {
    LocAidlTime mUtcTime;
    LocAidlTime mUtcReported;
    int mGpsEpheValid;
    int mGloEpheValid;
    long mBdsEpheValid;
    long mGalEpheValid;
    byte mQzssEpheValid;
}

