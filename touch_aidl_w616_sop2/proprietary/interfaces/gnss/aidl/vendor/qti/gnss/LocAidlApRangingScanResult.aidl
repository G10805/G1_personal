/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;
import vendor.qti.gnss.LocAidlRangingBandWidth;


@VintfStability
parcelable LocAidlApRangingScanResult {
    /** distance of the device and AP in milli-meters **/
    int distanceMm;
    /** Signal strength in dBm **/
    int rssi;
    /** Bandwidth in MHz of the transmitted ack from the device to Wifi Node */
    LocAidlRangingBandWidth txBandWidth;
    /** Bandwidth in MHz of the received frame from the Wifi Node */
    LocAidlRangingBandWidth rxBandWidth;
    /** Number of Chain(Antenna) **/
    int chainNo;
}
