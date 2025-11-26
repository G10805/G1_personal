/* ======================================================================
*  Copyright (c) 2023 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbprovider;

import android.location.Location;
import android.os.Parcel;
import android.os.Parcelable;

import com.qti.wifidbprovider.APRangingMeasurement;

public final class APRttScan implements Parcelable {
    public String mMacAdress;
    public int mDeltaTime;
    public int mNumAttempted;
    public APRangingMeasurement[] mRangingMeasurements;

    public APRttScan() {

    }

    public static final Creator<APRttScan> CREATOR =
        new Creator<APRttScan>() {
        public APRttScan createFromParcel(Parcel in) {
            return new APRttScan(in);
        }

        public APRttScan[] newArray(int size) {
            return new APRttScan[size];
        }
    };

    private APRttScan(Parcel in) {
        readFromParcel(in);
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mMacAdress);
        out.writeInt(mDeltaTime);
        out.writeInt(mNumAttempted);
        out.writeTypedArray(mRangingMeasurements, 0);
    }

    public void readFromParcel(Parcel in) {
        mMacAdress = in.readString();
        mDeltaTime = in.readInt();
        mNumAttempted = in.readInt();
        mRangingMeasurements = in.createTypedArray(APRangingMeasurement.CREATOR);
    }

    public int describeContents() {
        return 0;
    }

}
