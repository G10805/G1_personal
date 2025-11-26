/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlIzatStreamType {
    FUSED = 0x1,
    NETWORK = 0x2,
    GNSS = 0x4,
    NMEA = 0x8,
    DR = 0x10,
    GNSS_SVINFO = 0x20,
    DR_SVINFO = 0x40,
    ALL = 0x7f,
}
