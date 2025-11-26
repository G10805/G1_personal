/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlAltitudeRefType {
    ALT_REF_UNKNOWN      = 0,
    ALT_REF_WGS84        = 1,
    ALT_REF_MSL          = 2,
    ALT_REF_AGL          = 3,
    ALT_REF_FLOOR_LEVEL  = 4,
}

