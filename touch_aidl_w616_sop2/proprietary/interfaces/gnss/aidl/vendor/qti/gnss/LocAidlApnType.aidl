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
enum LocAidlApnType {
    INVALID = 0,
    IPV4 = 1,
    IPV6 = 2,
    IPV4V6 = 3,
}
