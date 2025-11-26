/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qti.gtp;

import android.os.Parcel;
import android.os.Parcelable;
import android.os.Bundle;
import android.util.Log;
import com.qti.gtp.GTPAccuracy;

public class GtpRequestData implements Parcelable {
    private static String TAG = "GtpRequestData";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private int mMinIntervalMillis;
    private GTPAccuracy mAccuracy;

    public GtpRequestData(Parcel source) {
        mMinIntervalMillis = source.readInt();
        mAccuracy = GTPAccuracy.values()[source.readInt()];
    }

    public GtpRequestData(int minIntervalMillis, GTPAccuracy accuracy) {
        mMinIntervalMillis = minIntervalMillis;
        mAccuracy = accuracy;
    }

    public int getMinIntervalMillis() {
        return mMinIntervalMillis;
    }

    public GTPAccuracy getAccuracy() {
        return mAccuracy;
    }
    @Override
    public int describeContents() {
        return 0;
    }

    public void setMinIntervalMillis(int minIntervalMillis) {
        mMinIntervalMillis = minIntervalMillis;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        if (VERBOSE) {
            Log.v(TAG, "in GtpRequestData: writeToParcel(); minIntervalMillis is " +
                    mMinIntervalMillis + ", mAccuracy is " + mAccuracy.ordinal());
        }

        dest.writeInt(mMinIntervalMillis);
        dest.writeInt(mAccuracy.ordinal());
    }

    public static final Parcelable.Creator<GtpRequestData> CREATOR =
            new Parcelable.Creator<GtpRequestData>() {
        @Override
        public GtpRequestData createFromParcel(Parcel source) {
             return new GtpRequestData(source);
        }
        @Override
        public GtpRequestData[] newArray(int size) {
            return new GtpRequestData[size];
        }
    };
}
