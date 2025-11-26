/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlTime;

@VintfStability
parcelable LocAidlSystemStatusSvHealth {
    LocAidlTime mUtcTime;
    LocAidlTime mUtcReported;
    int mGpsUnknownMask;
    int mGloUnknownMask;
    long mBdsUnknownMask;
    long mGalUnknownMask;
    byte mQzssUnknownMask;
    int mGpsGoodMask;
    int mGloGoodMask;
    long mBdsGoodMask;
    long mGalGoodMask;
    byte mQzssGoodMask;
    int mGpsBadMask;
    int mGloBadMask;
    long mBdsBadMask;
    long mGalBadMask;
    byte mQzssBadMask;
}

