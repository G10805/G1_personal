/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlCellLteDataItem {
    int type;
    int status;
    int mcc;
    int mnc;
    int cid;
    int pci;
    int tac;
    int mask;
}

