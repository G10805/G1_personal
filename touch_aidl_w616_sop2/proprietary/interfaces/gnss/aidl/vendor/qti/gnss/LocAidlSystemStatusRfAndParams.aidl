/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlSystemStatusRfAndParams;

import vendor.qti.gnss.LocAidlTime;

@VintfStability
parcelable LocAidlSystemStatusRfAndParams {
    LocAidlTime mUtcTime;
    LocAidlTime mUtcReported;
    int mPgaGain;
    int mGpsBpAmpI;
    int mGpsBpAmpQ;
    int mAdcI;
    int mAdcQ;
    int mJammerGps;
    int mJammerGlo;
    int mJammerBds;
    int mJammerGal;
    double mAgcGps;
    double mAgcGlo;
    double mAgcBds;
    double mAgcGal;
    int mGloBpAmpI;
    int mGloBpAmpQ;
    int mBdsBpAmpI;
    int mBdsBpAmpQ;
    int mGalBpAmpI;
    int mGalBpAmpQ;
    long mJammedSignalsMask;
    @nullable long[] mJammerInd;
    @nullable long[] mAgc;
}

