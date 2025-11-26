/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlTime;

@VintfStability
parcelable LocAidlSystemStatusPositionFailure {
    LocAidlTime mUtcTime;
    LocAidlTime mUtcReported;
    int mFixInfoMask;
    int mHepeLimit;
}

