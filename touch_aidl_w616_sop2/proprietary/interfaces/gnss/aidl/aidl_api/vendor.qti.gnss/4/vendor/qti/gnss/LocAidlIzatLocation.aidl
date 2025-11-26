/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.

package vendor.qti.gnss;
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
  vendor.qti.gnss.LocAidlNetworkPositionSourceType networkPositionSource;
  boolean hasNavSolutionMask;
  int navSolutionMask;
  boolean hasPositionTechMask;
  int positionTechMask;
}
