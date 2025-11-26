/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="byte")
enum LocAidlGnssNiType {
    VOICE = 1,
    UMTS_SUPL = 2,
    UMTS_CTRL_PLANE = 3,
    EMERGENCY_SUPL = 4,
}
