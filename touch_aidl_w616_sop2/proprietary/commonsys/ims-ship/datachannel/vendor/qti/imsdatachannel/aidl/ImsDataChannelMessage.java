/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;
import android.os.Parcel;
import android.os.Parcelable;

public class ImsDataChannelMessage implements Parcelable {
    private String mDcId;
    private String mProtocolId;
    private String mMessageId;
    private byte[] mMessage;

    public ImsDataChannelMessage(){}

    private ImsDataChannelMessage(Parcel in){
        readFromParcel(in);
    }
    public void readFromParcel(Parcel in) {
        mDcId = in.readString();
        mProtocolId = in.readString();
        mMessageId = in.readString();
        mMessage = in.createByteArray();
    }

    public static final Creator<ImsDataChannelMessage> CREATOR = new Creator<ImsDataChannelMessage>() {
        @Override
        public ImsDataChannelMessage createFromParcel(Parcel in) {
            return new ImsDataChannelMessage(in);
        }

        @Override
        public ImsDataChannelMessage[] newArray(int size) {
            return new ImsDataChannelMessage[size];
        }
    };

    public String getDcId() {
        return mDcId;
    }

    public void setDcId(String mDcId) {
        this.mDcId = mDcId;
    }

    public String getProtocolId() {
        return mProtocolId;
    }

    public void setProtocolId(String mProtocolId) {
        this.mProtocolId = mProtocolId;
    }

    public String getMessageId() {
        return mMessageId;
    }

    public void setMessageId(String mMessageId) {
        this.mMessageId = mMessageId;
    }

    public byte[] getMessage() {
        return mMessage;
    }

    public void setMessage(byte[] mMessage) {
        this.mMessage = mMessage;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mDcId);
        dest.writeString(mProtocolId);
        dest.writeString(mMessageId);
        dest.writeByteArray(mMessage);
    }
}
