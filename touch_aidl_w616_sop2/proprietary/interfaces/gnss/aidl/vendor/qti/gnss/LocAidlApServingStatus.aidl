/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;


@VintfStability
@Backing(type="int")
enum LocAidlApServingStatus {
    UNKNOWN = 0,
    SERVING = 1,
    NOT_SERVING = 2,
}
