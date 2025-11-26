/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlLocationTechnologyBits {
    GNSS_BIT = (1 << 0),
    CELL_BIT = (1 << 1),
    WIFI_BIT = (1 << 2),
    SENSORS_BIT = (1 << 3),
}
