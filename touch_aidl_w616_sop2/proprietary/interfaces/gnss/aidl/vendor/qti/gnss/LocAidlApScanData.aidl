/*
* Copyright (c) 2021, 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;
import vendor.qti.gnss.LocAidlRangingBandWidth;
import vendor.qti.gnss.LocAidlApServingStatus;


@VintfStability
parcelable LocAidlApScanData {

    long mac_R48b;
    int rssi;
    long age_usec;
    byte channel_id;
    String ssid;
    byte ssid_valid_byte_count;
    LocAidlApServingStatus isServing;
    int frequency;
    LocAidlRangingBandWidth rxBandWidth;
}

