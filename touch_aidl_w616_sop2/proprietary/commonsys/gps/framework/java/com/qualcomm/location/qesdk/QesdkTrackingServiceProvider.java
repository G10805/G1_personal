/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.qesdk;

import android.content.Context;
import android.os.RemoteException;
import android.util.Log;
import android.location.Location;

import com.qualcomm.location.idlclient.IDLClientUtils;
import com.qualcomm.location.idlclient.LocIDLClientBase.*;
import com.qualcomm.location.idlclient.*;

import vendor.qti.gnss.ILocAidlQesdkTrackingCallback;
import vendor.qti.gnss.LocAidlQesdkSessionParams;
import vendor.qti.gnss.LocAidlQesdkSessionPrecision;
import vendor.qti.gnss.LocAidlQesdkSessionType;
import vendor.qti.gnss.LocAidlLocation;

import com.qualcomm.location.policy.SessionPolicyManager;
import com.qualcomm.location.policy.ISessionOwner;
import com.qualcomm.location.policy.SessionRequest;
import com.qualcomm.location.policy.SessionRequest.RequestPrecision;
import com.qualcomm.location.policy.SessionRequest.RequestType;
import com.qualcomm.location.policy.SessionRequest.SessionType;
import com.qualcomm.location.policy.Policy;

import com.qualcomm.location.izatprovider.IzatProvider;
import com.qualcomm.location.izat.UserConsentManager;

import com.qualcomm.location.utils.IZatServiceContext;

public class QesdkTrackingServiceProvider implements ISessionOwner {

    private static final String TAG = "QesdkTrackingServiceProvider";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private QesdkTrackingIDLClient mQesdkIDLClient;
    private SessionPolicyManager mSessionPolicyManager;
    private IzatProvider mGtp;
    private UserConsentManager mUserConsentManager;

    private final Context mContext;

    private void logv(String msg) {
        if (VERBOSE) {
            Log.v(TAG, msg);
        }
    }
    private void logi(String msg) {
        Log.i(TAG, msg);
    }
    private void loge(String msg) {
        Log.e(TAG, msg);
    }

    public QesdkTrackingServiceProvider(Context ctx, SessionPolicyManager sessionPolicyManager) {

        mContext = ctx;

        if (LocIDLClientBase.getIDLServiceVersion().compareTo(IDLServiceVersion.V_AIDL) >= 0) {

            mQesdkIDLClient = new QesdkTrackingIDLClient(this);
            mSessionPolicyManager = sessionPolicyManager;
            mGtp = IzatProvider.getNetworkProvider(ctx);
            mUserConsentManager = UserConsentManager.getInstance(ctx);
            mGtp.onLoad();

        } else {
            loge("Unsupported IDL Service Version.");
        }
    }

    private void reportQesdkServiceDied() {

        logi("ILocAidlQesdkPreciseTracking service died.");

        // no-op
    }

    private SessionRequest sessionParamsToRequest(
        LocAidlQesdkSessionParams params, RequestType requestType) {
        SessionRequest request = new SessionRequest(params.pid, params.uid,
                mContext.getPackageManager().getNameForUid(params.uid),
                SessionType.SESSION_TYPE_INVALID, requestType, this);
        request.mParams.minIntervalMillis = params.minIntervalMillis;
        Log.i("SessionPolicyManager", "request mininterval = " + request.mParams.minIntervalMillis);
        if ((int)request.mParams.minIntervalMillis == -1) {
            request.mIdentity.requestType = (requestType == RequestType.REQUEST_LOCATION_UPDATES)?
                RequestType.REQUEST_PASSIVE_LOCATION_UPDATES :
                RequestType.REMOVE_PASSIVE_LOCATION_UPDATES;
        }
        switch(params.precision) {
        case LocAidlQesdkSessionPrecision.FINE:
            request.mParams.precision = RequestPrecision.REQUEST_PRECISION_FINE;
            break;
        case LocAidlQesdkSessionPrecision.COARSE:
            request.mParams.precision = RequestPrecision.REQUEST_PRECISION_COARSE;
            break;
        default:
            loge("Invalid params precision: " + params.precision);
            request.mParams.precision = RequestPrecision.REQUEST_PRECISION_INVALID;
            break;
        }
        switch(params.sessionType) {
        case LocAidlQesdkSessionType.PPE:
            request.mIdentity.sessionType = SessionType.SESSION_TYPE_PPE;
            break;
        case LocAidlQesdkSessionType.GTP:
            request.mIdentity.sessionType = SessionType.SESSION_TYPE_GTP;
            break;
        default:
            loge("Invalid params sessionType: " + params.sessionType);
            request.mIdentity.sessionType = SessionType.SESSION_TYPE_INVALID;
            break;
        }
        return request;
    }
    private LocAidlQesdkSessionParams sessionRequestToParams(SessionRequest request) {
        LocAidlQesdkSessionParams params = new LocAidlQesdkSessionParams();
        params.pid = request.mIdentity.pid;
        params.uid = request.mIdentity.uid;
        params.minIntervalMillis = request.mParams.minIntervalMillis;
        switch(request.mParams.precision) {
        case REQUEST_PRECISION_FINE:
            params.precision = LocAidlQesdkSessionPrecision.FINE;
            break;
        case REQUEST_PRECISION_COARSE:
            params.precision = LocAidlQesdkSessionPrecision.COARSE;
            break;
        default:
            loge("Invalid request precision: " + request.mParams.precision);
            break;
        }
        switch(request.mIdentity.sessionType) {
        case SESSION_TYPE_PPE:
            params.sessionType = LocAidlQesdkSessionType.PPE;
            break;
        case SESSION_TYPE_GTP:
            params.sessionType = LocAidlQesdkSessionType.GTP;
            break;
        default:
            loge("Invalid session type: " + request.mIdentity.sessionType);
            params.sessionType = LocAidlQesdkSessionType.INVALID;
            break;
        }
        return params;
    }

    //=============================================================
    // Requests down to QESDK Vendor Service
    //=============================================================
    private void startSession(SessionRequest request) {

        logi("Starting session (pid: " + request.mIdentity.pid +
                ", uid: " + request.mIdentity.uid +
                ", sessionType: " + request.mIdentity.sessionType +
                ", requestType: " + request.mIdentity.requestType +
                ", minIntervalMillis: " + request.mParams.minIntervalMillis +
                ", precision: " + request.mParams.precision + ")");

        if (request.mIdentity.sessionType == SessionType.SESSION_TYPE_PPE) {
            mQesdkIDLClient.requestLocationUpdates(request);
        } else if (request.mIdentity.sessionType == SessionType.SESSION_TYPE_GTP) {
            SessionRequest r = request;
            if (r.mIdentity.requestType == RequestType.REQUEST_PASSIVE_LOCATION_UPDATES ||
                    r.mIdentity.requestType == RequestType.REMOVE_PASSIVE_LOCATION_UPDATES) {
                r = new SessionRequest(request);
                r.mParams.minIntervalMillis = -1;
            }
            mQesdkIDLClient.requestLocationUpdates(r);
            mGtp.onSetSessionRequest(r);
        } else {
            //sessionType equals invalid
            //this case is used to compatible with older version of vendor in OTA update situation
            if (request.mParams.precision == RequestPrecision.REQUEST_PRECISION_FINE) {
                mQesdkIDLClient.requestLocationUpdates(request);
            } else if (request.mParams.precision == RequestPrecision.REQUEST_PRECISION_COARSE) {
                SessionRequest r = request;
                if (r.mIdentity.requestType == RequestType.REQUEST_PASSIVE_LOCATION_UPDATES ||
                        r.mIdentity.requestType == RequestType.REMOVE_PASSIVE_LOCATION_UPDATES) {
                    r = new SessionRequest(request);
                    r.mParams.minIntervalMillis = -1;
                }
                mQesdkIDLClient.requestLocationUpdates(r);
                mGtp.onSetSessionRequest(r);
            }
        }
    }
    private void stopSession(SessionRequest request) {

        logi("Stopping session (pid: " + request.mIdentity.pid +
                ", uid: " + request.mIdentity.uid +
                ", sessionType: " + request.mIdentity.sessionType +
                ", requestType: " + request.mIdentity.requestType +
                ", minIntervalMillis: " + request.mParams.minIntervalMillis +
                ", precision: " + request.mParams.precision + ")");

        if (request.mIdentity.sessionType == SessionType.SESSION_TYPE_PPE) {
            mQesdkIDLClient.removeLocationUpdates(request);
        } else if (request.mIdentity.sessionType == SessionType.SESSION_TYPE_GTP) {
            SessionRequest r = request;
            if (r.mIdentity.requestType == RequestType.REQUEST_PASSIVE_LOCATION_UPDATES ||
                    r.mIdentity.requestType == RequestType.REMOVE_PASSIVE_LOCATION_UPDATES) {
                r = new SessionRequest(request);
                r.mParams.minIntervalMillis = -1;
            }
            mQesdkIDLClient.removeLocationUpdates(r);
            mGtp.onSetSessionRequest(r);
        } else {
            //sessionType equals invalid
            //this case is used to compatible with older version of vendor in OTA update situation
            if (request.mParams.precision == RequestPrecision.REQUEST_PRECISION_FINE) {
                mQesdkIDLClient.removeLocationUpdates(request);
            } else if (request.mParams.precision == RequestPrecision.REQUEST_PRECISION_COARSE) {
                SessionRequest r = request;
                if (r.mIdentity.requestType == RequestType.REQUEST_PASSIVE_LOCATION_UPDATES ||
                        r.mIdentity.requestType == RequestType.REMOVE_PASSIVE_LOCATION_UPDATES) {
                    r = new SessionRequest(request);
                    r.mParams.minIntervalMillis = -1;
                }
                mQesdkIDLClient.removeLocationUpdates(r);
                mGtp.onSetSessionRequest(r);
            }
        }
    }

    //=============================================================
    // Requests from QESDK Vendor Service for Policy Management
    //=============================================================
    private void handleSessionRequest(SessionRequest request) {

        if (request.mParams.precision == RequestPrecision.REQUEST_PRECISION_FINE ||
                    request.mParams.precision == RequestPrecision.REQUEST_PRECISION_COARSE) {

            // Check user consent for this request
            if (request.mIdentity.sessionType == SessionType.SESSION_TYPE_GTP) {
                boolean consent = mUserConsentManager.getPackageUserConsent(
                    request.mIdentity.packageName, request.mIdentity.uid);
                mQesdkIDLClient.notifyUserConsent(request, consent);
            }

            // Now process the request
            mSessionPolicyManager.process(request);

        } else {
            loge("Unsupported request precision.");
        }
    }
    private void setUserConsent(int uid, boolean userConsent) {

        mUserConsentManager.setPackageUserConsent(userConsent,
            mContext.getPackageManager().getNameForUid(uid), uid);
    }

    //=============================================================
    // ISessionOwner methods
    //=============================================================
    @Override
    public void handle(SessionRequest request) {

        switch(request.mIdentity.requestType) {
        case REQUEST_LOCATION_UPDATES:
        case REQUEST_PASSIVE_LOCATION_UPDATES:
            startSession(request);
            break;

        case REMOVE_LOCATION_UPDATES:
        case REMOVE_PASSIVE_LOCATION_UPDATES:
            stopSession(request);
            break;

        default:
            loge("Unsupported session request type.");
            break;
        }
    }

    @Override
    public boolean isPolicyApplicable(Policy.PolicyName policyName, SessionRequest request) {

        switch (policyName) {

        case POLICY_NAME_BACKGROUND_THROTTLING:
            return (request.mParams.precision == RequestPrecision.REQUEST_PRECISION_COARSE);

        case POLICY_NAME_BACKGROUND_DENIED:
            return (request.mParams.precision == RequestPrecision.REQUEST_PRECISION_FINE);

        case POLICY_NAME_USER_PROFILE_CHANGE:
        case POLICY_NAME_LOCATION_SETTING_CHANGE:
        case POLICY_NAME_PERMISSION_CHANGE:
        case POLICY_NAME_SCREEN_INACTIVE:
        case POLICY_NAME_POWER_SAVE_MODE:
        case POLICY_NAME_POWER_BLAME_REPORTING:
        case POLICY_NAME_APPOPS_CHANGE:
            return true;

        default:
            return false;
        }
    }

    //=============================================================
    // ILocAidlQesdkPreciseTracking AIDL client
    //=============================================================
    class QesdkTrackingIDLClient extends LocIDLClientBase
            implements LocIDLClientBase.IServiceDeathCb {

        private final String TAG = "QesdkTrackingIDLClient";

        public static final int LOCATION_REPORT_ON_FULL_INDICATION = 1;
        public static final int LOCATION_REPORT_ON_FIX_INDICATION = 2;
        public static final int BATCHING_MODE_NONE = 2;
        public static final int BATCH_STATUS_TRIP_COMPLETED = 0;
        private QesdkTrackingIDLCallback mQesdkTrackingCallback = null;
        public vendor.qti.gnss.ILocAidlQesdkTracking mQesdkTrackingIface = null;

        @Override
        public void onServiceDied() {
            mQesdkTrackingIface = null;
            getQesdkTrackingIface();
            try {
                if (mQesdkTrackingIface != null) {
                    mQesdkTrackingIface.setCallback(mQesdkTrackingCallback);
                    reportQesdkServiceDied();
                }
            } catch (RemoteException e) {
            }
        }

        private QesdkTrackingIDLClient(QesdkTrackingServiceProvider provider) {
            getQesdkTrackingIface();
            try {
                if (mQesdkTrackingIface != null) {
                    registerServiceDiedCb(this);
                    mQesdkTrackingCallback = new QesdkTrackingIDLCallback(provider);
                    mQesdkTrackingIface.setCallback(mQesdkTrackingCallback);
                }
            } catch (RemoteException e) {
            }
        }

        public void getQesdkTrackingIface() {
            if (null == mQesdkTrackingIface) {
                try {
                    mQesdkTrackingIface =
                        getGnssAidlService().getExtensionLocAidlQesdkTracking();
                } catch (RuntimeException e) {
                    Log.e(TAG, "Exception getting mQesdkTrackingIface: " + e);
                    mQesdkTrackingIface = null;
                } catch (RemoteException e) {
                }
            }
        }

        public int requestLocationUpdates(SessionRequest request) {

            if (null == mQesdkTrackingIface) {
                return -1;
            }
            IDLClientUtils.toIDLService(TAG);
            try {
                LocAidlQesdkSessionParams params = sessionRequestToParams(request);
                return mQesdkTrackingIface.requestLocationUpdates(params);
            } catch (RemoteException e) {
            }
            return -1;
        }

        public int removeLocationUpdates(SessionRequest request) {

            if (null == mQesdkTrackingIface) {
                return -1;
            }
            IDLClientUtils.toIDLService(TAG);
            try {
                LocAidlQesdkSessionParams params = sessionRequestToParams(request);
                return mQesdkTrackingIface.removeLocationUpdates(params);
            } catch (RemoteException e) {
            }
            return -1;
        }

        public int notifyUserConsent(SessionRequest request, boolean userConsent) {

            if (null == mQesdkTrackingIface) {
                return -1;
            }
            IDLClientUtils.toIDLService(TAG);
            try {
                LocAidlQesdkSessionParams params = sessionRequestToParams(request);
                // Overloading negative minInterval values to notify user consent
                params.minIntervalMillis = userConsent? -2 : -3;
                return mQesdkTrackingIface.requestLocationUpdates(params);
            } catch (RemoteException e) {
            }
            return -1;
        }

        //==============================================================
        // ILocAidlQesdkTrackingCallback Callback Implementation
        //==============================================================
        class QesdkTrackingIDLCallback extends ILocAidlQesdkTrackingCallback.Stub {

            private QesdkTrackingServiceProvider mQesdkTrackingServiceProvider;

            public QesdkTrackingIDLCallback(QesdkTrackingServiceProvider provider) {
                mQesdkTrackingServiceProvider = provider;
            }

            public void requestLocationUpdatesCb(LocAidlQesdkSessionParams params) {
                IDLClientUtils.fromIDLService(TAG);
                mQesdkTrackingServiceProvider.handleSessionRequest(
                    sessionParamsToRequest(params, RequestType.REQUEST_LOCATION_UPDATES));
            }
            public void removeLocationUpdatesCb(LocAidlQesdkSessionParams params) {
                IDLClientUtils.fromIDLService(TAG);
                mQesdkTrackingServiceProvider.handleSessionRequest(
                    sessionParamsToRequest(params, RequestType.REMOVE_LOCATION_UPDATES));
            }
            public void setUserConsent(LocAidlQesdkSessionParams params, boolean userConsent) {
                IDLClientUtils.fromIDLService(TAG);
                mQesdkTrackingServiceProvider.setUserConsent(params.uid, userConsent);
            }

            @Override
            public final int getInterfaceVersion() {
                return ILocAidlQesdkTrackingCallback.VERSION;
                }
            @Override
            public final String getInterfaceHash() {
                return ILocAidlQesdkTrackingCallback.HASH;
            }
        }
    }
}
