/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlLocationFlagsBits {
    LAT_LONG_BIT = (1 << 0),
    ALTITUDE_BIT = (1 << 1),
    SPEED_BIT = (1 << 2),
    BEARING_BIT = (1 << 3),
    ACCURACY_BIT = (1 << 4),
    VERTICAL_ACCURACY_BIT = (1 << 5),
    SPEED_ACCURACY_BIT = (1 << 6),
    BEARING_ACCURACY_BIT = (1 << 7),
    SPOOF_MASK_BIT = (1 << 8),
    ELAPSED_REAL_TIME_BIT = (1 << 9),
    CONFORMITY_INDEX_BIT = (1 << 10),
    QUALITY_TYPE_BIT = (1 << 11),
    TECH_MASK_BIT = (1 << 12),
}
