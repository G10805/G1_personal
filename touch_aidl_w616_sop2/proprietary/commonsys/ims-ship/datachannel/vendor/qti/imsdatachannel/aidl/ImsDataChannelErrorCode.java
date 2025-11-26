/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.aidl;

import android.os.Parcel;
import android.os.Parcelable;

public class ImsDataChannelErrorCode implements Parcelable {
    public static final int DCS_DC_ERROR_NONE = 0;
    public static final int DCS_DC_ERROR_QOS_NOT_MET = 1;
    public static final int DCS_DC_ERROR_INVALID_STREAM_ID = 2;
    public static final int DCS_DC_ERROR_PARAM_MISSING = 3;
    public static final int DCS_DC_ERROR_COOKIE_ERROR = 4;
    public static final int DCS_DC_ERROR_NO_RESOURCE = 5;
    public static final int DCS_DC_ERROR_UNRESOLVABLE_ADDRESS = 6;
    public static final int DCS_DC_ERROR_UNKNOWN_CHUCK_TYPE = 7;
    public static final int DCS_DC_ERROR_INVALID_PARAM = 8;
    public static final int DCS_DC_ERROR_NO_USER_DATA = 9;
    public static final int DCS_DC_ERROR_NO_PROTOCOL_VIOLATION = 10;
    public static final int DCS_DC_ERROR_NO_CERTIFICATE = 11;
    public static final int DCS_DC_ERROR_AUTH_FAILURE = 12;
    public static final int DCS_DC_ERROR_HANDSHAKE_FAILURE = 13;
    public static final int DCS_DC_ERROR_REMOTE_REJECTED = 14;
    public static final int DCS_DC_ERROR_REMOTE_REQUESTED = 15;
    public static final int DCS_DC_ERROR_NETWORK_ISSUE = 16;
    public static final int DCS_DC_ERROR_ABORT_RECEIVED = 17;
    public static final int DCS_DC_ERROR_HEARTBEAT_TIMEOUT = 18;
    public static final int DCS_DC_ERROR_DTLS_ERROR = 19;
    public static final int DCS_DC_ERROR_CALL_ENDED = 20;
    public static final int DCS_DC_ERROR_MESSAGE_TOO_BIG = 21;
    private int mImsDataChannelErrorCode = DCS_DC_ERROR_NONE;

    public ImsDataChannelErrorCode(){}

    private ImsDataChannelErrorCode(Parcel in){
        readFromParcel(in);
    }

    public void readFromParcel(Parcel in) {
        mImsDataChannelErrorCode = in.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mImsDataChannelErrorCode);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<ImsDataChannelErrorCode> CREATOR = new Creator<ImsDataChannelErrorCode>() {
        @Override
        public ImsDataChannelErrorCode createFromParcel(Parcel in) {
            return new ImsDataChannelErrorCode(in);
        }

        @Override
        public ImsDataChannelErrorCode[] newArray(int size) {
            return new ImsDataChannelErrorCode[size];
        }
    };

    public int getImsDataChannelErrorCode() {
        return mImsDataChannelErrorCode;
    }

    public void setImsDataChannelErrorCode(int mImsDataChannelErrorCode) {
        this.mImsDataChannelErrorCode = mImsDataChannelErrorCode;
    }
}
