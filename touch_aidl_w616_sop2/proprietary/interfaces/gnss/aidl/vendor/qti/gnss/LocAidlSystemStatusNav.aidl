/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlEphemerisSource;
import vendor.qti.gnss.LocAidlEphemerisType;

@VintfStability
parcelable LocAidlSystemStatusNav {
    LocAidlEphemerisType mType;
    LocAidlEphemerisSource mSource;
    int mAgeSec;
}

