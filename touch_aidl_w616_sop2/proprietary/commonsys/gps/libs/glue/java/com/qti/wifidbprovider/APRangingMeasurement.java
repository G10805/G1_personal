/* ======================================================================
*  Copyright (c) 2023 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbprovider;

import android.location.Location;
import android.os.Parcel;
import android.os.Parcelable;

public final class APRangingMeasurement implements Parcelable {
    public int mDistanceInMm;
    public float mRssi;
    public int mTxBandWidth;
    public int mRxBandWidth;
    public int mChainNumber;

    public APRangingMeasurement() {

    }

    public static final Creator<APRangingMeasurement> CREATOR =
        new Creator<APRangingMeasurement>() {
        public APRangingMeasurement createFromParcel(Parcel in) {
            return new APRangingMeasurement(in);
        }

        public APRangingMeasurement[] newArray(int size) {
            return new APRangingMeasurement[size];
        }
    };

    private APRangingMeasurement(Parcel in) {
        readFromParcel(in);
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mDistanceInMm);
        out.writeFloat(mRssi);
        out.writeInt(mTxBandWidth);
        out.writeInt(mRxBandWidth);
        out.writeInt(mChainNumber);
    }

    public void readFromParcel(Parcel in) {
        mDistanceInMm = in.readInt();
        mRssi = in.readFloat();
        mTxBandWidth = in.readInt();
        mRxBandWidth = in.readInt();
        mChainNumber = in.readInt();
    }

    public int describeContents() {
        return 0;
    }
}
