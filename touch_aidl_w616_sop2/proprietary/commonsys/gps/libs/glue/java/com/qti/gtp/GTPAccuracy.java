/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qti.gtp;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

public enum GTPAccuracy implements Parcelable {
        NOMINAL, HIGH;
    private static String TAG = "GTPAccuracy";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        if (VERBOSE) {
            Log.v(TAG, "in GTPAccuracy: writeToParcel()" +
               "; GTP accuracy is " + ordinal());
        }
        dest.writeInt(ordinal());
    }

    public static final Parcelable.Creator<GTPAccuracy> CREATOR =
            new Parcelable.Creator<GTPAccuracy>() {
        @Override
        public GTPAccuracy createFromParcel(Parcel in) {
             return GTPAccuracy.values()[in.readInt()];
        }
        @Override
        public GTPAccuracy[] newArray(int size) {
            return new GTPAccuracy[size];
        }
    };
}
