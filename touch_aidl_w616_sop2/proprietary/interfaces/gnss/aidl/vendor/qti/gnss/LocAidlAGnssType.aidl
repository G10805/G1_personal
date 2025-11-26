/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

/**
 * LocAidl AGNSS type
 */
@VintfStability
@Backing(type="byte")
enum LocAidlAGnssType {
    TYPE_ANY = 0,
    TYPE_SUPL = 1,
    TYPE_C2K = 2,
    TYPE_WWAN_ANY = 3,
    TYPE_WIFI = 4,
    TYPE_SUPL_ES = 5,
}
