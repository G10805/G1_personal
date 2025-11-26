/*
* Copyright (c) 2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

/** Specify the XTRA assistance data status. */
@VintfStability
@Backing(type="int")
enum LocAidlXtraDataStatus {
    /** If XTRA feature is disabled or if XTRA feature is enabled,
     *  but XTRA daemon has not yet retrieved the assistance data
     *  status from modem on early stage of device bootup, xtra data
     *  status will be unknown.   */
    XTRA_DATA_STATUS_UNKNOWN = 0,
    /** If XTRA feature is enabled, but XTRA data is not present
     *  on the device. */
    XTRA_DATA_STATUS_NOT_AVAIL = 1,
    /** If XTRA feature is enabled, XTRA data has been downloaded
     *  but it is no longer valid. */
    XTRA_DATA_STATUS_NOT_VALID = 2,
    /** If XTRA feature is enabled, XTRA data has been downloaded
     *  and is currently valid. */
    XTRA_DATA_STATUS_VALID = 3,
}
