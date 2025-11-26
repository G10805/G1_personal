/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import vendor.qti.imsdatachannel.aidl.ImsMessageStatus;

import android.os.Parcel;
import android.os.Parcelable;

public class ImsMessageStatusInfo implements Parcelable {
    private String mMsgTransactionId;
    private ImsMessageStatus mMsgStatus;
    private int mMsgStatusErrorCode;
    public ImsMessageStatusInfo(){}

    private ImsMessageStatusInfo(Parcel in){
        readFromParcel(in);
    }
    public void readFromParcel(Parcel in) {
        mMsgTransactionId = in.readString();
        mMsgStatus = in.readParcelable(ImsMessageStatus.class.getClassLoader());
        mMsgStatusErrorCode = in.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mMsgTransactionId);
        dest.writeParcelable(mMsgStatus, flags);
        dest.writeInt(mMsgStatusErrorCode);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<ImsMessageStatusInfo> CREATOR = new Creator<ImsMessageStatusInfo>() {
        @Override
        public ImsMessageStatusInfo createFromParcel(Parcel in) {
            return new ImsMessageStatusInfo(in);
        }

        @Override
        public ImsMessageStatusInfo[] newArray(int size) {
            return new ImsMessageStatusInfo[size];
        }
    };

    public String getMsgTransactionId() {
        return mMsgTransactionId;
    }

    public void setMsgTransactionId(String mMsgTransactionId) {
        this.mMsgTransactionId = mMsgTransactionId;
    }

    public ImsMessageStatus getMsgStatus() {
        return mMsgStatus;
    }

    public void setMsgStatus(ImsMessageStatus mMsgStatus) {
        this.mMsgStatus = mMsgStatus;
    }

    public int getMsgStatusErrorCode() {
        return mMsgStatusErrorCode;
    }

    public void setMsgStatusErrorCode(int mMsgStatusErrorCode) {
        this.mMsgStatusErrorCode = mMsgStatusErrorCode;
    }
}

