/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.policy;

import android.location.Location;

public class SessionRequest {

    public class RequestIdentity {
        public int pid;
        public int uid;
        public String packageName;
        public SessionType sessionType;
        public RequestType requestType;
        public ISessionOwner owner;
    }

    public class RequestParams {
        public long minIntervalMillis;
        public RequestPrecision precision;
        public SessionCallback callback;
    }

    public RequestIdentity mIdentity;
    public RequestParams mParams;

    public SessionRequest(int pid, int uid, String packageName, SessionType sessionType,
            RequestType requestType, ISessionOwner owner) {

        mIdentity = new RequestIdentity();
        mParams = new RequestParams();

        mIdentity.pid = pid;
        mIdentity.uid = uid;
        mIdentity.packageName = packageName;
        mIdentity.sessionType = sessionType;
        mIdentity.requestType = requestType;
        mIdentity.owner = owner;
    }
    public SessionRequest(SessionRequest request) {

        mIdentity = new RequestIdentity();
        mParams = new RequestParams();

        mIdentity.pid = request.mIdentity.pid;
        mIdentity.uid = request.mIdentity.uid;
        mIdentity.packageName = request.mIdentity.packageName;
        mIdentity.sessionType = request.mIdentity.sessionType;
        mIdentity.requestType = request.mIdentity.requestType;
        mIdentity.owner = request.mIdentity.owner;

        mParams.minIntervalMillis = request.mParams.minIntervalMillis;
        mParams.precision = request.mParams.precision;
        mParams.callback = request.mParams.callback;
    }

    public long getUniqueId() {
        boolean isPassive = (mIdentity.requestType == RequestType.REQUEST_PASSIVE_LOCATION_UPDATES
                || mIdentity.requestType == RequestType.REMOVE_PASSIVE_LOCATION_UPDATES);

        return (isPassive ? 1 : 0) + (10L * mIdentity.sessionType.ordinal()) +
            (100L * mIdentity.pid) + (1000_000_00L * mIdentity.uid);
    }

    public boolean isValidUpdate(SessionRequest newRequest) {

        return (newRequest.getUniqueId() == getUniqueId() &&
                newRequest.mIdentity.packageName != null &&
                newRequest.mIdentity.packageName.equals(mIdentity.packageName) &&
                newRequest.mIdentity.requestType == mIdentity.requestType &&
                newRequest.mIdentity.sessionType == mIdentity.sessionType &&
                newRequest.mIdentity.owner == mIdentity.owner);
    }

    public boolean isValid() {

        return (mIdentity.pid != 0 && mIdentity.uid != 0 && mIdentity.packageName != null &&
                mIdentity.requestType != RequestType.REQUEST_TYPE_INVALID &&
                mIdentity.owner != null && mIdentity.packageName != null &&
                mParams.precision != RequestPrecision.REQUEST_PRECISION_INVALID);
    }

    public boolean isPassiveRequestType() {
        return (mIdentity.requestType == RequestType.REQUEST_PASSIVE_LOCATION_UPDATES ||
                mIdentity.requestType == RequestType.REMOVE_PASSIVE_LOCATION_UPDATES);
    }

    @Override
    public String toString() {
        return "[SessionRequest]"+
               " pid: " + mIdentity.pid +
               " uid: " + mIdentity.uid +
               " packageName: " + mIdentity.packageName +
               " sessionType: " + mIdentity.sessionType +
               " requestType: " + mIdentity.requestType +
               " owner: " + mIdentity.owner +
               " minIntervalMillis: " + mParams.minIntervalMillis +
               " precision: " + mParams.precision +
               " callback: " + mParams.callback;
    }

    // Session Request Types
    public enum RequestType {
        REQUEST_TYPE_INVALID,
        REQUEST_LOCATION_UPDATES,
        REMOVE_LOCATION_UPDATES,
        REQUEST_PASSIVE_LOCATION_UPDATES,
        REMOVE_PASSIVE_LOCATION_UPDATES
    }

    // Session Request Precision
    public enum RequestPrecision {
        REQUEST_PRECISION_INVALID,
        REQUEST_PRECISION_FINE,
        REQUEST_PRECISION_COARSE
    }

    // Session Types
    public enum SessionType {
        SESSION_TYPE_INVALID,
        SESSION_TYPE_PPE,
        SESSION_TYPE_GTP
    }

    // Callback Interface
    public interface SessionCallback {
        public void onLocationAvailable(Location location);
    }
}
