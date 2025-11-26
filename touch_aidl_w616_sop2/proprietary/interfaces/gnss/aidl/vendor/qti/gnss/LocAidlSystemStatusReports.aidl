/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlSystemStatusReports;

import vendor.qti.gnss.LocAidlSystemStatusBestPosition;
import vendor.qti.gnss.LocAidlSystemStatusEphemeris;
import vendor.qti.gnss.LocAidlSystemStatusErrRecovery;
import vendor.qti.gnss.LocAidlSystemStatusInjectedPosition;
import vendor.qti.gnss.LocAidlSystemStatusLocation;
import vendor.qti.gnss.LocAidlSystemStatusNavData;
import vendor.qti.gnss.LocAidlSystemStatusPdr;
import vendor.qti.gnss.LocAidlSystemStatusPositionFailure;
import vendor.qti.gnss.LocAidlSystemStatusRfAndParams;
import vendor.qti.gnss.LocAidlSystemStatusSvHealth;
import vendor.qti.gnss.LocAidlSystemStatusTimeAndClock;
import vendor.qti.gnss.LocAidlSystemStatusXoState;
import vendor.qti.gnss.LocAidlSystemStatusXtra;

@VintfStability
parcelable LocAidlSystemStatusReports {
    boolean mSuccess;
    LocAidlSystemStatusLocation[] mLocationVec;
    LocAidlSystemStatusTimeAndClock[] mTimeAndClockVec;
    LocAidlSystemStatusXoState[] mXoStateVec;
    LocAidlSystemStatusErrRecovery[] mErrRecoveryVec;
    LocAidlSystemStatusInjectedPosition[] mInjectedPositionVec;
    LocAidlSystemStatusBestPosition[] mBestPositionVec;
    LocAidlSystemStatusXtra[] mXtraVec;
    LocAidlSystemStatusEphemeris[] mEphemerisVec;
    LocAidlSystemStatusSvHealth[] mSvHealthVec;
    LocAidlSystemStatusPdr[] mPdrVec;
    LocAidlSystemStatusNavData[] mNavDataVec;
    LocAidlSystemStatusPositionFailure[] mPositionFailureVec;
    LocAidlSystemStatusRfAndParams[] mRfAndParamsVec;
}

