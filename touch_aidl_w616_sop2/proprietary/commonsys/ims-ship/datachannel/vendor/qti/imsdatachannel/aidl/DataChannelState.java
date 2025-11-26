/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import android.os.Parcel;
import android.os.Parcelable;

public class DataChannelState implements Parcelable {
    public static final int DATA_CHANNEL_INIT = 0;
    public static final int DATA_CHANNEL_CONNECTING = 1;
    public static final int DATA_CHANNEL_CONNECTED = 2;
    public static final int DATA_CHANNEL_CLOSING = 3;
    public static final int DATA_CHANNEL_CLOSED = 4;
    private int mDataChannelState = DATA_CHANNEL_INIT;

    public DataChannelState(){}

    private DataChannelState(Parcel in){
        readFromParcel(in);
    }
    public void readFromParcel(Parcel in) {
        mDataChannelState = in.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mDataChannelState);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<DataChannelState> CREATOR = new Creator<DataChannelState>() {
        @Override
        public DataChannelState createFromParcel(Parcel in) {
            return new DataChannelState(in);
        }

        @Override
        public DataChannelState[] newArray(int size) {
            return new DataChannelState[size];
        }
    };

    public int getDataChannelState() {
        return mDataChannelState;
    }

    public void setDataChannelState(int mDataChannelState) {
        this.mDataChannelState = mDataChannelState;
    }
}
