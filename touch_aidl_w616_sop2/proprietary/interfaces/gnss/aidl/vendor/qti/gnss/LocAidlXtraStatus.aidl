/*
* Copyright (c) 2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlXtraStatus {
    boolean mEnabled;
    vendor.qti.gnss.LocAidlXtraDataStatus mStatus;
    int mValidityHrs;
    String mLastDownloadStatus;
}

