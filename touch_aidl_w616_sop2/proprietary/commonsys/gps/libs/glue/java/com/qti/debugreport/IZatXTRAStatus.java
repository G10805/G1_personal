/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qti.debugreport;

import android.os.Parcel;
import android.os.Parcelable;


/**
 * Class IZatXTRAStatus.
 *
 * This class contains the XTRA data validity and the
 *  age of the XTRA data for each GNSS constellation.
 */
public class IZatXTRAStatus implements Parcelable {

    private boolean mEnabled;
    private int mXtraDataStatus;
    private int mValidityHrs;
    private String mLastDownloadStatus;


    public IZatXTRAStatus(boolean enable, int status, int validity, String dlStatus) {
        mEnabled = enable;
        mXtraDataStatus = status;
        mValidityHrs = validity;
        mLastDownloadStatus = dlStatus;
    }

    public IZatXTRAStatus(Parcel source) {
        mEnabled = source.readBoolean();
        mXtraDataStatus = source.readInt();
        mValidityHrs = source.readInt();
        mLastDownloadStatus = source.readString();
    }

    public boolean getEnabled() { return mEnabled; }
    public int getXtraDataStatus() { return mXtraDataStatus; }
    public int getValidityHrs() { return mValidityHrs; }
    public String getLastDownloadStatus() { return mLastDownloadStatus; }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeBoolean(mEnabled);
        dest.writeInt(mXtraDataStatus);
        dest.writeInt(mValidityHrs);
        dest.writeString(mLastDownloadStatus);
    }

    public void readFromParcel(Parcel in) {
        mEnabled = in.readBoolean();
        mXtraDataStatus = in.readInt();
        mValidityHrs = in.readInt();
        mLastDownloadStatus = in.readString();
    }

    public static final Parcelable.Creator<IZatXTRAStatus> CREATOR =
            new Parcelable.Creator<IZatXTRAStatus>() {
        @Override
        public IZatXTRAStatus createFromParcel(Parcel source) {
             return new IZatXTRAStatus(source);
        }
        @Override
        public IZatXTRAStatus[] newArray(int size) {
            return new IZatXTRAStatus[size];
        }
    };
};
