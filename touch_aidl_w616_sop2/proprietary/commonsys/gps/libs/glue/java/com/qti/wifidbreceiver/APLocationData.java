/* ======================================================================
*  Copyright (c) 2017, 2023 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbreceiver;

import android.os.Parcel;
import android.os.Parcelable;

public final class APLocationData implements Parcelable {
    public String mMacAddress;
    public float mLatitude;
    public float mLongitude;
    public float mMaxAntenaRange;
    public float mHorizontalError;

    public int mReliability;

    public int mRttCapability;
    public int mPositionQuality;
    public int mAltRefType;
    public float mAltitude;
    public float mRttRangeBiasInMm;

    public static final int AP_LOC_MAR_VALID                = 0x1;
    public static final int AP_LOC_HORIZONTAL_ERR_VALID     = 0x2;
    public static final int AP_LOC_RELIABILITY_VALID        = 0x4;
    public static final int AP_LOC_WITH_LAT_LON             = 0x8;
    public static final int AP_LOC_ALT_VALID                = 0x10;
    public static final int AP_LOC_POSITION_QUALITY_VALID    = 0x20;
    public static final int AP_LOC_RTT_CAPABILITY_VALID     = 0x40;
    public static final int AP_LOC_RTT_BIAS_VALID           = 0x80;
    public int mValidBits;

    public APLocationData() {

    }

    public static final Parcelable.Creator<APLocationData> CREATOR =
        new Parcelable.Creator<APLocationData>() {
        public APLocationData createFromParcel(Parcel in) {
            return new APLocationData(in);
        }

        public APLocationData[] newArray(int size) {
            return new APLocationData[size];
        }
    };

    private APLocationData(Parcel in) {
        readFromParcel(in);
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mMacAddress);
        out.writeFloat(mLatitude);
        out.writeFloat(mLongitude);
        out.writeFloat(mMaxAntenaRange);
        out.writeFloat(mHorizontalError);
        out.writeInt(mReliability);
        out.writeInt(mValidBits);
        out.writeInt(mRttCapability);
        out.writeInt(mPositionQuality);
        out.writeInt(mAltRefType);
        out.writeFloat(mAltitude);
        out.writeFloat(mRttRangeBiasInMm);
    }

    public void readFromParcel(Parcel in) {
        mMacAddress = in.readString();
        mLatitude = in.readFloat();
        mLongitude = in.readFloat();
        mMaxAntenaRange = in.readFloat();
        mHorizontalError = in.readFloat();
        mReliability = in.readInt();
        mValidBits = in.readInt();
        mRttCapability = in.readInt();
        mPositionQuality = in.readInt();
        mAltRefType = in.readInt();
        mAltitude = in.readFloat();
        mRttRangeBiasInMm = in.readFloat();
    }

    public int describeContents() {
        return 0;
    }

}
