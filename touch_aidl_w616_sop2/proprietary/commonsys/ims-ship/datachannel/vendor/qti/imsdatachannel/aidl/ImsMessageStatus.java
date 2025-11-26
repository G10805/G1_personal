/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import android.os.Parcel;
import android.os.Parcelable;

public class ImsMessageStatus implements Parcelable {
    public static final int MESSAGE_STATUS_SUCCESS = 0;
    public static final int MESSAGE_STATUS_FAILURE= 1;
    public static final int MESSAGE_STATUS_BLOCKED = 2;
    public static final int MESSAGE_STATUS_UNBLOCKED = 3;
    private int mImsMessageStatus = MESSAGE_STATUS_SUCCESS;

    public ImsMessageStatus(){}
    private ImsMessageStatus(Parcel in) {
        readFromParcel(in);
    }

    public void readFromParcel(Parcel in){
        mImsMessageStatus = in.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mImsMessageStatus);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<ImsMessageStatus> CREATOR = new Creator<ImsMessageStatus>() {
        @Override
        public ImsMessageStatus createFromParcel(Parcel in) {
            return new ImsMessageStatus(in);
        }

        @Override
        public ImsMessageStatus[] newArray(int size) {
            return new ImsMessageStatus[size];
        }
    };

    public int getImsMessageStatus() {
        return mImsMessageStatus;
    }

    public void setImsMessageStatus(int mImsMessageStatus) {
        this.mImsMessageStatus = mImsMessageStatus;
    }
}
