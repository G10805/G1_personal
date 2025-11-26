/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlApInfo {
    long mac_R48b;
    byte cell_type;
    int cell_id1;
    int cell_id2;
    int cell_id3;
    int cell_id4;
    String ssid;
    byte ssid_valid_byte_count;
    long utc_time;
    byte fdal_status;
}

