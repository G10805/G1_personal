/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import android.os.Parcel;
import android.os.Parcelable;

public class ImsDataChannelAttributes implements Parcelable {
    private String mDcId;
    private int mDataChannelStreamId;
    private String mDataChannelLabel;
    private String mSubProtocol;
    private int mMaxMessageSize;

    public ImsDataChannelAttributes(){}

    private ImsDataChannelAttributes(Parcel in){
        readFromParcel(in);
    }

    public void readFromParcel(Parcel in) {
        mDcId = in.readString();
        mDataChannelStreamId = in.readInt();
        mDataChannelLabel = in.readString();
        mSubProtocol = in.readString();
        mMaxMessageSize = in.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mDcId);
        dest.writeInt(mDataChannelStreamId);
        dest.writeString(mDataChannelLabel);
        dest.writeString(mSubProtocol);
        dest.writeInt(mMaxMessageSize);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<ImsDataChannelAttributes> CREATOR = new Creator<ImsDataChannelAttributes>() {
        @Override
        public ImsDataChannelAttributes createFromParcel(Parcel in) {
            return new ImsDataChannelAttributes(in);
        }

        @Override
        public ImsDataChannelAttributes[] newArray(int size) {
            return new ImsDataChannelAttributes[size];
        }
    };

    public String getDcId() {
        return mDcId;
    }

    public void setDcId(String mDcId) {
        this.mDcId = mDcId;
    }

    public int getDataChannelStreamId() {
        return mDataChannelStreamId;
    }

    public void setDataChannelStreamId(int mDataChannelStreamId) {
        this.mDataChannelStreamId = mDataChannelStreamId;
    }

    public String getDataChannelLabel() {
        return mDataChannelLabel;
    }

    public void setDataChannelLabel(String mDataChannelLabel) {
        this.mDataChannelLabel = mDataChannelLabel;
    }

    public String getSubProtocol() {
        return mSubProtocol;
    }

    public void setSubProtocol(String mSubProtocol) {
        this.mSubProtocol = mSubProtocol;
    }

    public int getMaxMessageSize() {
        return mMaxMessageSize;
    }

    public void setMaxMessageSize(int mMaxMessageSize) {
        this.mMaxMessageSize = mMaxMessageSize;
    }
}
