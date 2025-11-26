/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlLocationQualityType;

@VintfStability
parcelable LocAidlLocation {
    int locationFlagsMask;
    long timestamp;
    double latitude;
    double longitude;
    double altitude;
    float speed;
    float bearing;
    float accuracy;
    float verticalAccuracy;
    float speedAccuracy;
    float bearingAccuracy;
    int locationTechnologyMask;
    float conformityIndex;
    LocAidlLocationQualityType qualityType;
    long elapsedRealTime;
    long elapsedRealTimeUnc;
}

