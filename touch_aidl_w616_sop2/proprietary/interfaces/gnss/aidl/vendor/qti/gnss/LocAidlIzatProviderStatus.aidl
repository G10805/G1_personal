/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlIzatProviderStatus {
    OUT_OF_SERVICE = 0,
    TEMPORARILY_UNAVAILABLE = 1,
    AVAILABLE = 2,
    GNSS_STATUS_NONE = 3,
    GNSS_STATUS_SESSION_BEGIN = 4,
    GNSS_STATUS_SESSION_END = 5,
    GNSS_STATUS_ENGINE_ON = 6,
    GNSS_STATUS_ENGINE_OFF = 7,
}
