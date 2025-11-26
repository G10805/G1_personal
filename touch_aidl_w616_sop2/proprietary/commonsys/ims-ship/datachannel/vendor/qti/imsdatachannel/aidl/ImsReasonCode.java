/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import android.os.Parcel;
import android.os.Parcelable;

public class ImsReasonCode implements Parcelable {
    public static final int SUCCESS = 0;
    public static final int FAILURE_UNSPECIFIED= 1;
    private int mImsReasonCode = SUCCESS;

    public ImsReasonCode(){}
    private ImsReasonCode(Parcel in){
        readFromParcel(in);
    }
    public void readFromParcel(Parcel in) {
        mImsReasonCode = in.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mImsReasonCode);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<ImsReasonCode> CREATOR = new Creator<ImsReasonCode>() {
        @Override
        public ImsReasonCode createFromParcel(Parcel in) {
            return new ImsReasonCode(in);
        }

        @Override
        public ImsReasonCode[] newArray(int size) {
            return new ImsReasonCode[size];
        }
    };

    public int getImsReasonCode() {
        return mImsReasonCode;
    }

    public void setImsReasonCode(int mImsReasonCode) {
        this.mImsReasonCode = mImsReasonCode;
    }
}
