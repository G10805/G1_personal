/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;
import vendor.qti.gnss.LocAidlRangingBandWidth;
import vendor.qti.gnss.LocAidlApRangingScanResult;

@VintfStability
parcelable LocAidlApRangingData {
    /** BSSID of the self mac address*/
    long mac_R48b;
    /** Age of this ranging data since first ranging data received in us **/
    long age_usec;
    /** number of attempted RTT measurements **/
    int num_attempted;
    LocAidlApRangingScanResult[] ap_ranging_scan_info;
}
