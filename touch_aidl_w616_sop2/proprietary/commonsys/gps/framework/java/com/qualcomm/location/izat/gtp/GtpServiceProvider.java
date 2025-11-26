/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.location.izat.gtp;

import android.content.Context;
import android.location.Location;
import android.os.Binder;
import android.os.RemoteException;
import android.util.Log;
import com.qti.gtp.IGTPService;
import com.qti.gtp.IGTPServiceCallback;
import com.qti.gtp.GtpRequestData;
import com.qti.gtp.GTPAccuracy;

import com.qualcomm.location.izat.CallbackData;
import com.qualcomm.location.izat.UserConsentManager;
import com.qualcomm.location.izatprovider.IzatProvider;
import com.qualcomm.location.policy.ISessionOwner;
import com.qualcomm.location.policy.Policy;
import com.qualcomm.location.policy.SessionPolicyManager;
import com.qualcomm.location.policy.SessionRequest;
import com.qualcomm.location.izat.IzatService;

import static com.qualcomm.location.policy.SessionRequest.RequestPrecision.REQUEST_PRECISION_COARSE;
import static com.qualcomm.location.policy.SessionRequest.RequestPrecision.REQUEST_PRECISION_FINE;
import static com.qualcomm.location.policy.SessionRequest.RequestType.*;
import static com.qualcomm.location.policy.SessionRequest.SessionType.*;

public class GtpServiceProvider implements IzatService.ISystemEventListener, ISessionOwner {

    private static final String TAG = "GtpServiceProvider";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private final SessionPolicyManager mSessionPolicyManager;
    private final UserConsentManager mUserConsentManager;

    private final IzatProvider mProvider;
    private static GtpServiceProvider sInstance = null;
    private final Context mContext;

    public static GtpServiceProvider getInstance(
            Context ctx,
            SessionPolicyManager sessionPolicyManager) {
        if (null == sInstance) {
            sInstance = new GtpServiceProvider(ctx, sessionPolicyManager);
        }
        return sInstance;
    }

    private GtpServiceProvider(Context ctx,
                               SessionPolicyManager sessionPolicyManager) {
        this.mSessionPolicyManager = sessionPolicyManager;
        this.mProvider = IzatProvider.getNetworkProvider(ctx);
        this.mProvider.onLoad();
        this.mContext = ctx;
        this.mUserConsentManager = UserConsentManager.getInstance(ctx);
        IzatService.AidlClientDeathNotifier.getInstance().registerAidlClientDeathCb(this);
    }

    public IGTPService getGtpBinder() {
        return mBinder;
    }

    @Override
    public void onAidlClientDied(String packageName, int pid, int uid) {
        Log.d(TAG, "onAidlClientDied - package " + packageName);
        mSessionPolicyManager.stopSessionsByPiduid(pid, uid);
    }

    private ISessionOwner sessionOwner() {
        return this;
    }

    private final IGTPService.Stub mBinder = new IGTPService.Stub() {

        @Override
        public void requestLocationUpdates(
                IGTPServiceCallback gtpServiceCallback,
                GtpRequestData gtpReq) {
            SessionRequest sessionRequest = new SessionRequest(
                    Binder.getCallingPid(),
                    Binder.getCallingUid(),
                    mContext.getPackageManager()
                            .getNameForUid(Binder.getCallingUid()),
                    SESSION_TYPE_GTP,
                    REQUEST_LOCATION_UPDATES,
                    sessionOwner());

            sessionRequest.mParams.minIntervalMillis = gtpReq.getMinIntervalMillis();
            sessionRequest.mParams.callback
                    = new GtpCallbackData(gtpServiceCallback);
            sessionRequest.mParams.precision = (gtpReq.getAccuracy() == GTPAccuracy.HIGH) ?
                    REQUEST_PRECISION_FINE : REQUEST_PRECISION_COARSE;

            if (VERBOSE) {
                Log.v(TAG, "requestLocationUpdates: calling SPM.process with" +
                        " session ID: " + sessionRequest.getUniqueId());
            }
            mSessionPolicyManager.process(sessionRequest);
        }

        @Override
        public void removeLocationUpdates() {
            SessionRequest sessionRequest = new SessionRequest(
                    Binder.getCallingPid(),
                    Binder.getCallingUid(),
                    mContext.getPackageManager()
                            .getNameForUid(Binder.getCallingUid()),
                    SESSION_TYPE_GTP,
                    REMOVE_LOCATION_UPDATES,
                    sessionOwner());

            sessionRequest.mParams.precision = REQUEST_PRECISION_COARSE;

            if (VERBOSE) {
                Log.v(TAG, "removeLocationUpdates: calling SPM.process with" +
                        " session ID: " + sessionRequest.getUniqueId());
            }
            mSessionPolicyManager.process(sessionRequest);
        }

        @Override
        public void requestPassiveLocationUpdates(
                IGTPServiceCallback gtpServiceCallback) {

            SessionRequest sessionRequest = new SessionRequest(
                    Binder.getCallingPid(),
                    Binder.getCallingUid(),
                    mContext.getPackageManager()
                            .getNameForUid(Binder.getCallingUid()),
                    SESSION_TYPE_GTP,
                    REQUEST_PASSIVE_LOCATION_UPDATES,
                    sessionOwner());

            sessionRequest.mParams.callback
                    = new GtpCallbackData(gtpServiceCallback);
            sessionRequest.mParams.precision = REQUEST_PRECISION_COARSE;

            if (VERBOSE) {
                Log.d(TAG, "requestPassiveLocationUpdates: calling SPM.process" +
                        " with session ID: " + sessionRequest.getUniqueId());
            }
            mSessionPolicyManager.process(sessionRequest);
        }

        @Override
        public void removePassiveLocationUpdates() {

            SessionRequest sessionRequest = new SessionRequest(
                    Binder.getCallingPid(),
                    Binder.getCallingUid(),
                    mContext.getPackageManager()
                            .getNameForUid(Binder.getCallingUid()),
                    SESSION_TYPE_GTP,
                    REMOVE_PASSIVE_LOCATION_UPDATES,
                    sessionOwner());
            sessionRequest.mParams.precision = REQUEST_PRECISION_COARSE;

            if (VERBOSE) {
                Log.d(TAG, "removePassiveLocationUpdates: calling SPM.process" +
                        " with session ID: " + sessionRequest.getUniqueId());
            }
            mSessionPolicyManager.process(sessionRequest);
        }

        @Override
        public void setUserConsent(boolean b) {

            if (VERBOSE) {
                Log.d(TAG, "Calling mUserConsentManager.setPackageUserConsent: " + b);
            }
            int callingUid = Binder.getCallingUid();
            mUserConsentManager.setPackageUserConsent(
                    b,
                    mContext.getPackageManager().getNameForUid(callingUid),
                    callingUid);
        }
    };

    @Override
    public void handle(SessionRequest sessionRequest) {

        if (VERBOSE) {
            Log.d(TAG, "handle:  " + sessionRequest.mIdentity.requestType.name()
                    + " session id : " + sessionRequest.getUniqueId());
        }
        mProvider.onSetSessionRequest(sessionRequest);
    }

    @Override
    public boolean isPolicyApplicable(Policy.PolicyName policyName, SessionRequest request) {
        switch (policyName) {

            case POLICY_NAME_USER_PROFILE_CHANGE:
            case POLICY_NAME_SCREEN_INACTIVE:
            case POLICY_NAME_POWER_SAVE_MODE:
            case POLICY_NAME_PERMISSION_CHANGE:
            case POLICY_NAME_BACKGROUND_THROTTLING:
            case POLICY_NAME_LOCATION_SETTING_CHANGE:
            case POLICY_NAME_POWER_BLAME_REPORTING:
            case POLICY_NAME_APPOPS_CHANGE:
                return true;

            default:
                return false;
        }
    }


    private static class GtpCallbackData
            extends CallbackData implements SessionRequest.SessionCallback {

        public GtpCallbackData(IGTPServiceCallback callback) {

            super.mCallback = callback;
        }

        @Override
        public void onLocationAvailable(Location location) {

            try {
                Log.d(TAG, "GtpCallbackData onLocationAvailable");
                ((IGTPServiceCallback) mCallback).onLocationAvailable(location);
            } catch (RemoteException e) {
                throw new RuntimeException(e);
            }
        }
    }
}
