/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlBsSpecialInfo {
    byte cellType;
    int cellRegionID1;
    int cellRegionID2;
    int cellRegionID3;
    int cellRegionID4;
    byte info;
}

