/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlNetworkInfoDataItem {
    int type;
    String typeName;
    String subTypeName;
    boolean available;
    boolean connected;
    boolean roaming;
}

