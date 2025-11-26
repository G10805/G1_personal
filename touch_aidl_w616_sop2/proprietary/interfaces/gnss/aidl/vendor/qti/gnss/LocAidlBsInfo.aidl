/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlBsInfo {
    byte cell_type;
    int cell_id1;
    int cell_id2;
    int cell_id3;
    int cell_id4;
    long timestamp;
}

