/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlCellCdmaDataItem {
    int type;
    int status;
    int sid;
    int nid;
    int bsid;
    int bslat;
    int bslong;
    int timeOffset;
    int mask;
    boolean inDST;
}

