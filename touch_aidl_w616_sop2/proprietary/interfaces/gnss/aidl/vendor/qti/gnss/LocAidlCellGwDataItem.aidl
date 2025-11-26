/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlCellGwDataItem {
    int type;
    int status;
    int mcc;
    int mnc;
    int lac;
    int cid;
    int mask;
}

