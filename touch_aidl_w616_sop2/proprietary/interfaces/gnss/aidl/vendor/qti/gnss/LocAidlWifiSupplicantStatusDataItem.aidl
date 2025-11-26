/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlWifiSupplicantStatusDataItem {
    int state;
    boolean apMacAddressValid;
    boolean apSsidValid;
    byte[] apMacAddress;
    String apSsid;
}

