/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlIzatHorizontalAccuracy;
import vendor.qti.gnss.LocAidlIzatOtherAccuracy;
import vendor.qti.gnss.LocAidlIzatStreamType;

@VintfStability
parcelable LocAidlIzatRequest {
    LocAidlIzatStreamType provider;
    int numUpdates;
    long suggestedResponseTimeForFirstFix;
    long timeIntervalBetweenFixes;
    float smallestDistanceBetweenFixes;
    LocAidlIzatHorizontalAccuracy suggestedHorizontalAccuracy;
    LocAidlIzatOtherAccuracy suggestedAltitudeAccuracy;
    LocAidlIzatOtherAccuracy suggestedBearingAccuracy;
}

