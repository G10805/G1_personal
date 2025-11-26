/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

/**
 * LocAidl AGNSS SubId
 */
@VintfStability
@Backing(type="byte")
enum LocAidlAGnssSubId {
    LOC_DEFAULT_SUB = 0,
    LOC_PRIMARY_SUB = 1,
    LOC_SECONDARY_SUB = 2,
    LOC_TERTIARY_SUB = 3,
}
