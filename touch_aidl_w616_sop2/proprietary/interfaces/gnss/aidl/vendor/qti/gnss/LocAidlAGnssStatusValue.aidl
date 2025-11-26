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
enum LocAidlAGnssStatusValue {
    REQUEST_AGNSS_DATA_CONN  = 1,
    RELEASE_AGNSS_DATA_CONN  = 2,
    AGNSS_DATA_CONNECTED     = 3,
    AGNSS_DATA_CONN_DONE     = 4,
    AGNSS_DATA_CONN_FAILED   = 5,
}
