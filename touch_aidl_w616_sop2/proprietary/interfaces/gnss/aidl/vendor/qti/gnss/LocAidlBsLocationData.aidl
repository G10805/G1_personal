/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlBsLocationData {
    byte cellType;
    int cellRegionID1;
    int cellRegionID2;
    int cellRegionID3;
    int cellRegionID4;
    float latitude;
    float longitude;
    byte valid_bits;
    float horizontal_coverage_radius;
    byte horizontal_confidence;
    byte horizontal_reliability;
    float altitude;
    float altitude_uncertainty;
    byte altitude_confidence;
    byte altitude_reliability;
}

