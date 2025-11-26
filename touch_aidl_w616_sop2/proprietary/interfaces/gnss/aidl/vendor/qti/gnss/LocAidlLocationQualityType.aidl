/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="byte")
enum LocAidlLocationQualityType {
    STANDALONE_QUALITY_TYPE = 0,
    DGNSS_QUALITY_TYPE = 1,
    FLOAT_QUALITY_TYPE = 2,
    FIXED_QUALITY_TYPE = 3,
}
