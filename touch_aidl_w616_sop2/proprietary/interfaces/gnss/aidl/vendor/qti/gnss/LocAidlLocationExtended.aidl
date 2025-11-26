/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlApTimeStamp;
import vendor.qti.gnss.LocAidlGnssSvUsedInPosition;

@VintfStability
parcelable LocAidlLocationExtended {
    int flags;
    float altitudeMeanSeaLevel;
    float pdop;
    float hdop;
    float vdop;
    float magneticDeviation;
    float vert_unc;
    float speed_unc;
    float bearing_unc;
    int horizontal_reliability;
    int vertical_reliability;
    float horUncEllipseSemiMajor;
    float horUncEllipseSemiMinor;
    float horUncEllipseOrientAzimuth;
    LocAidlApTimeStamp apTimeStamp;
    LocAidlGnssSvUsedInPosition svUsedIds;
    int navSolutionMask;
    int tech_mask;
}

