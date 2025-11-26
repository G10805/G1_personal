/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import android.os.Parcel;
import android.os.Parcelable;

public class ImsDataChannelResponse implements Parcelable {
    private String mDcId;
    private boolean mAcceptStatus;

    public ImsDataChannelResponse(){}
    private ImsDataChannelResponse(Parcel in){
        readFromParcel(in);
    }
    public void readFromParcel(Parcel in) {
        mDcId = in.readString();
        mAcceptStatus = in.readByte() != 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mDcId);
        dest.writeByte((byte) (mAcceptStatus ? 1 : 0));
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<ImsDataChannelResponse> CREATOR = new Creator<ImsDataChannelResponse>() {
        @Override
        public ImsDataChannelResponse createFromParcel(Parcel in) {
            return new ImsDataChannelResponse(in);
        }

        @Override
        public ImsDataChannelResponse[] newArray(int size) {
            return new ImsDataChannelResponse[size];
        }
    };

    public String getDcId() {
        return mDcId;
    }

    public void setDcId(String mDcId) {
        this.mDcId = mDcId;
    }

    public boolean isAcceptStatus() {
        return mAcceptStatus;
    }

    public void setAcceptStatus(boolean mAcceptStatus) {
        this.mAcceptStatus = mAcceptStatus;
    }
}
