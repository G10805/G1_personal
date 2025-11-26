/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlWifiDBListStatus {
    E_STD_CONT = 0,
    E_STD_FINAL = 1,
    E_SCAN_FINAL = 2,
    E_LOOKUP = 3,
}
