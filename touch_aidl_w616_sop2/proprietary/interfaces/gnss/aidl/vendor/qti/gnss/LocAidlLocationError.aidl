/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlLocationError {
    SUCCESS = 0,
    GENERAL_FAILURE,
    CALLBACK_MISSING,
    INVALID_PARAMETER,
    ID_EXISTS,
    ID_UNKNOWN,
    ALREADY_STARTED,
    GEOFENCES_AT_MAX,
    NOT_SUPPORTED,
}
