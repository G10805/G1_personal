/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlTime;

@VintfStability
parcelable LocAidlSystemStatusTimeAndClock {
    LocAidlTime mUtcTime;
    LocAidlTime mUtcReported;
    int mGpsWeek;
    int mGpsTowMs;
    byte mTimeValid;
    byte mTimeSource;
    int mTimeUnc;
    int mClockFreqBias;
    int mClockFreqBiasUnc;
    int mLeapSeconds;
    int mLeapSecUnc;
}

