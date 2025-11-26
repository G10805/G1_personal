/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import android.os.Parcel;
import android.os.Parcelable;

public class ImsDataChannelCommandErrorCode implements Parcelable {
    public static final int DCS_COMMAND_SUCCESS = 0;
    public static final int DCS_COMMAND_FAILURE = 1;
    public static final int DCS_COMMAND_SUCCESS_ASYC_UPDATE = 2;
    public static final int DCS_COMMAND_INVALID_PARAM = 3;
    public static final int DCS_COMMAND_REQUEST_TIMEOUT = 4;
    public static final int DCS_COMMAND_INSUFFICIENT_MEMORY = 5;
    public static final int DCS_COMMAND_LOST_NET = 6;
    public static final int DCS_COMMAND_NOT_SUPPORTED = 7;
    public static final int DCS_COMMAND_NOT_FOUND = 8;
    public static final int DCS_COMMAND_INVALID_CALL_ID = 9;
    public static final int DCS_COMMAND_REQUEST_UNKNOWN = 10;
    public static final int DCS_COMMAND_INSTANCE_NOT_READY = 11;
    public static final int DCS_COMMAND_REQUEST_NOTALLOWED = 12;
    public static final int DCS_COMMAND_DNSQUERY_PENDING = 13;
    public static final int DCS_COMMAND_MESSAGE_NOTALLOWED = 14;
    public static final int DCS_COMMAND_RESOURCE_UNAVAILABLE = 15;
    public static final int DCS_COMMAND_RETRY_ATTEMPTS_MAXED_OUT = 16;
    public static final int DCS_COMMAND_MESSAGE_TOO_BIG = 17;
    private int mImsDataChannelCommandErrorCode = DCS_COMMAND_SUCCESS;

    public ImsDataChannelCommandErrorCode() {
    }

    private ImsDataChannelCommandErrorCode(Parcel in) {
        readFromParcel(in);
    }

    public void readFromParcel(Parcel in) {
        mImsDataChannelCommandErrorCode = in.readInt();
    }

    public static final Creator<ImsDataChannelCommandErrorCode> CREATOR = new Creator<ImsDataChannelCommandErrorCode>() {
        @Override
        public ImsDataChannelCommandErrorCode createFromParcel(Parcel in) {
            return new ImsDataChannelCommandErrorCode(in);
        }

        @Override
        public ImsDataChannelCommandErrorCode[] newArray(int size) {
            return new ImsDataChannelCommandErrorCode[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mImsDataChannelCommandErrorCode);
    }

    public int getImsDataChannelCommandErrorCode() {
        return mImsDataChannelCommandErrorCode;
    }

    public void setImsDataChannelCommandErrorCode(int mImsDataChannelCommandErrorCode) {
        this.mImsDataChannelCommandErrorCode = mImsDataChannelCommandErrorCode;
    }
}
