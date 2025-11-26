/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlBatchStatus {
    BATCH_STATUS_TRIP_COMPLETED = 0,
    BATCH_STATUS_POSITION_AVAILABLE = 1,
    BATCH_STATUS_POSITION_UNAVAILABLE = 2,
}
