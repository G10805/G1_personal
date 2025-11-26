/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlBatchMode {
    BATCH_MODE_ROUTINE = 0,
    BATCH_MODE_TRIP = 1,
    BATCH_MODE_NONE = 2,
}
