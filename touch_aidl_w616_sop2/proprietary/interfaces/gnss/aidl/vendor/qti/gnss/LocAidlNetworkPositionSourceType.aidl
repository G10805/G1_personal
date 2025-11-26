/*
* Copyright (c) 2021, 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlNetworkPositionSourceType {
    CELL = 0,
    WIFI = 1,
    WIFI_RTT_SERVER = 2,
    WIFI_RTT_FTM = 3,
}
