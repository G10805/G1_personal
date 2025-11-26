/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlQesdkSessionPrecision;
import vendor.qti.gnss.LocAidlQesdkSessionType;

@VintfStability
parcelable LocAidlQesdkSessionParams {
    LocAidlQesdkSessionPrecision precision;
    int pid;
    int uid;
    long minIntervalMillis;
    LocAidlQesdkSessionType sessionType;
}

