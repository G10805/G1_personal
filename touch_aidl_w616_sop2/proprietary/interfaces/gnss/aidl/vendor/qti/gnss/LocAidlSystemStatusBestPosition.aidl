/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlTime;

@VintfStability
parcelable LocAidlSystemStatusBestPosition {
    LocAidlTime mUtcTime;
    LocAidlTime mUtcReported;
    float mBestLat;
    float mBestLon;
    float mBestAlt;
    float mBestHepe;
    float mBestAltUnc;
}

