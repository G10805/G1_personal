/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlNetworkPositionSourceType;
import vendor.qti.gnss.LocAidlAltitudeRefType;

@VintfStability
parcelable LocAidlIzatLocation {
    boolean hasUtcTimestampInMsec;
    long utcTimestampInMsec;
    boolean hasElapsedRealTimeInNanoSecs;
    long elapsedRealTimeInNanoSecs;
    boolean hasLatitude;
    double latitude;
    boolean hasLongitude;
    double longitude;
    boolean hasHorizontalAccuracy;
    float horizontalAccuracy;
    boolean hasAltitudeWrtEllipsoid;
    double altitudeWrtEllipsoid;
    boolean hasAltitudeWrtMeanSeaLevel;
    double altitudeWrtMeanSeaLevel;
    boolean hasBearing;
    float bearing;
    boolean hasSpeed;
    float speed;
    int position_source;
    boolean hasAltitudeMeanSeaLevel;
    float altitudeMeanSeaLevel;
    boolean hasDop;
    float pDop;
    float hDop;
    float vDop;
    boolean hasMagneticDeviation;
    float magneticDeviation;
    boolean hasVertUnc;
    float vertUnc;
    boolean hasSpeedUnc;
    float speedUnc;
    boolean hasBearingUnc;
    float bearingUnc;
    boolean hasHorizontalReliability;
    int horizontalReliability;
    boolean hasVerticalReliability;
    int verticalReliability;
    boolean hasHorUncEllipseSemiMajor;
    float horUncEllipseSemiMajor;
    boolean hasHorUncEllipseSemiMinor;
    float horUncEllipseSemiMinor;
    boolean hasHorUncEllipseOrientAzimuth;
    float horUncEllipseOrientAzimuth;
    boolean hasNetworkPositionSource;
    LocAidlNetworkPositionSourceType networkPositionSource;
    boolean hasNavSolutionMask;
    int navSolutionMask;
    boolean hasPositionTechMask;
    int positionTechMask;
    boolean hasAltitudeRefType;
    LocAidlAltitudeRefType altitudeRefType;
}

