/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlGeofenceBreachType {
    ENTER = 0,
    EXIT = 1,
    DWELL_IN = 2,
    DWELL_OUT = 3,
    UNKNOWN = 4,
}
