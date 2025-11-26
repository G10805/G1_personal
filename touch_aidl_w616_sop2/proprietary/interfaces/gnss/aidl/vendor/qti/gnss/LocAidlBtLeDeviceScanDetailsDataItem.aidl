/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlBtLeDeviceScanDetailsDataItem {
    boolean validSrnData;
    int apSrnRssi;
    byte[] apSrnMacAddress;
    long apSrnTimestamp;
    long requestTimestamp;
    long receiveTimestamp;
    int errorCause;
}

