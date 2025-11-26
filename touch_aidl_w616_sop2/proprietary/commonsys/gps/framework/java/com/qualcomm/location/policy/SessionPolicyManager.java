/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.policy;

import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.OnPermissionsChangedListener;
import android.location.LocationManager;
import android.os.Binder;
import android.util.Log;
import android.util.Pair;
import android.os.Handler;
import android.os.Message;
import android.os.Looper;

import android.app.AppOpsManager;
import android.app.AppOpsManager.OnOpChangedListener;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Iterator;

import com.android.server.LocalServices;
import android.os.PowerManagerInternal;
import android.os.PowerSaveState;
import android.os.PowerManager;
import java.util.function.Consumer;

import com.qualcomm.location.izat.CallbackData;
import com.qualcomm.location.izat.DataPerPackageAndUser;
import com.qualcomm.location.izat.IzatService;
import com.qualcomm.location.izat.esstatusreceiver.EsStatusReceiver;
import com.qualcomm.location.utils.IZatServiceContext;
import com.qualcomm.location.policy.SessionRequest.RequestPrecision;
import com.qualcomm.location.policy.SessionRequest.SessionType;

import android.content.Intent;
import android.content.IntentFilter;
import android.os.UserHandle;
import android.content.BroadcastReceiver;

public class SessionPolicyManager implements Handler.Callback {

    private static final String TAG = "SessionPolicyManager";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sLocationSettingsLock = new Object();
    private final Context mContext;
    private final DataPerPackageAndUser<UserData> mDataPerPackageAndUser;

    private AppOpsManager mAppOpsMgr;
    private final Looper mLooper;
    private Handler mHandler;
    private IZatServiceContext mIZatServiceCtx;
    private boolean mIsLocationSettingsOn;
    private boolean mIsPowerSaveModeOn;
    private boolean mIsScreenOff;

    private final UserProfileChangePolicy mUserProfileChangePolicy;
    private final LocationSettingChangePolicy mLocationSettingChangePolicy;
    private final BackgroundThrottlingPolicy mBackgroundThrottlingPolicy;
    private final BackgroundDeniedPolicy mBackgroundDeniedPolicy;
    private final PowerBlameReportingPolicy mPowerBlameReportingPolicy;
    private final PermissionChangePolicy mPermissionChangePolicy;
    private final PowerSaveModePolicy mPowerSaveModePolicy;
    private final ScreenInactivePolicy mScreenInactivePolicy;
    private final AppOpsChangePolicy mAppOpsChangePolicy;

    private static final int MSG_UPDATE_HIGHPOWER_MONITOR =
            IZatServiceContext.MSG_POLICY_MANAGER_BASE + 1;
    private static final int MSG_HANDLE_USER_CHANGE =
            IZatServiceContext.MSG_POLICY_MANAGER_BASE + 2;
    private static final int MSG_HANDLE_PROCESS_REQUEST =
            IZatServiceContext.MSG_POLICY_MANAGER_BASE + 3;
    private static final int MSG_HANDLE_LOCATION_MODE_CHANGE =
            IZatServiceContext.MSG_POLICY_MANAGER_BASE + 4;
    private static final int MSG_HANDLE_UID_IMPORTANCE_CHANGE =
            IZatServiceContext.MSG_POLICY_MANAGER_BASE + 5;

    private static final int BACKGROUND_APP_MIN_INTERVAL_MILLIS = 1800000;

    // Will be switched accordingly on user changes.
    private UserData mUserData = new UserData();

    //=============================================================
    // Internal Class Definitions
    //=============================================================
    private class SessionStatus {

        private SessionRequest mRequest;

        private volatile boolean mIsRunning;
        private boolean mIsAllowedInBackground;
        private boolean mIsInBackground;
        private long mOriginalMinIntervalMillis;
        private boolean mHasFineLocationAccess;
        private boolean mHasCoarseLocationAccess;

        SessionStatus(SessionRequest request,
                      boolean isAllowedInBackground) {

            mRequest = request;
            mIsRunning = false;
            mIsAllowedInBackground = isAllowedInBackground;
            mIsInBackground = false;
            mOriginalMinIntervalMillis = -1;
        }
    }

    // Data related to each user.
    private class UserData extends CallbackData {

        private Map<Long, SessionStatus> mSessionMap = new HashMap<Long, SessionStatus>();
    }

    //=============================================================
    // Handler.Callback Interface
    //=============================================================
    public boolean handleMessage(Message msg) {
        int msgID = msg.what;
        switch(msgID){

        case MSG_HANDLE_PROCESS_REQUEST:
            handleProcessRequest((SessionRequest)msg.obj);
            break;

        case MSG_UPDATE_HIGHPOWER_MONITOR:
            Log.d(TAG, "MSG_UPDATE_HIGHPOWER_MONITOR");
            int uid = msg.arg2;
            String packageName = (String)msg.obj;
            boolean start = (msg.arg1 == 1) ? true:false;
            mPowerBlameReportingPolicy.updateHighPowerLocationMonitoring(uid, packageName, start);
            break;

        case MSG_HANDLE_USER_CHANGE:
            Pair<Map<String, UserData>, Map<String, UserData>> arg =
                (Pair<Map<String, UserData>, Map<String, UserData>>)msg.obj;
            Map<String, UserData> prevUserDataMap = arg.first;
            Map<String, UserData> currentUserDataMap = arg.second;
            mUserProfileChangePolicy.handleUserChange(prevUserDataMap, currentUserDataMap);
            break;

        case MSG_HANDLE_LOCATION_MODE_CHANGE:
            boolean locationSettingsIsOn = (msg.arg1 == 1);
            mLocationSettingChangePolicy.handleLocationModeChange(locationSettingsIsOn);
            break;

        case MSG_HANDLE_UID_IMPORTANCE_CHANGE:
            uid = msg.arg1;
            boolean isImportanceForeground = (msg.arg2 == 1);
            mBackgroundThrottlingPolicy.handleUidImportanceChange(uid, isImportanceForeground);
            mBackgroundDeniedPolicy.handleUidImportanceChange(uid, isImportanceForeground);
            break;

        default:
            Log.w(TAG, "Unhandled Message " + msg.what);
            break;
        }
        return true;
    }

    //=============================================================
    // Utility Methods
    //=============================================================
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

    //=============================================================
    // Ctor
    //=============================================================
    public SessionPolicyManager(Context ctx) {

        logv("SessionPolicyManager construction");

        mContext = ctx;

        mLooper = IZatServiceContext.getInstance(mContext).getLooper();
        mHandler = new Handler(mLooper, this);
        mAppOpsMgr = mContext.getSystemService(AppOpsManager.class);
        mIZatServiceCtx = IZatServiceContext.getInstance(mContext);

        // Location Setting changed policy
        mLocationSettingChangePolicy = new LocationSettingChangePolicy();

        // User profile change policy
        mUserProfileChangePolicy = new UserProfileChangePolicy();
        mDataPerPackageAndUser = new DataPerPackageAndUser<UserData>(
                                        mContext, mUserProfileChangePolicy);
        mDataPerPackageAndUser.useCommonPackage();
        mDataPerPackageAndUser.setData(mUserData);

        // Background Throttling policy
        mBackgroundThrottlingPolicy = new BackgroundThrottlingPolicy();

        // Background Denied policy
        mBackgroundDeniedPolicy = new BackgroundDeniedPolicy();

        // Power Blame Reporting
        mPowerBlameReportingPolicy = new PowerBlameReportingPolicy();

        // Permission change policy
        mPermissionChangePolicy = new PermissionChangePolicy();

        // Power save mode policy
        mPowerSaveModePolicy = new PowerSaveModePolicy();

        // Screen inactive policy
        mScreenInactivePolicy = new ScreenInactivePolicy();
        // App Ops change policy
        mAppOpsChangePolicy = new AppOpsChangePolicy();
        mAppOpsMgr.startWatchingMode(AppOpsManager.OP_FINE_LOCATION, null, mAppOpsChangePolicy);
        mAppOpsMgr.startWatchingMode(AppOpsManager.OP_COARSE_LOCATION, null, mAppOpsChangePolicy);
    }

    //=============================================================
    // SessionPolicyManager APIs
    //=============================================================
    public void process(SessionRequest request) {

        Log.d(TAG, "process: request type " + request.mIdentity.requestType.name());
        logi("process SessionRequest");
        mHandler.obtainMessage(MSG_HANDLE_PROCESS_REQUEST, request).sendToTarget();
    }
    private void handleProcessRequest(SessionRequest request) {

        logi("handleProcessRequest: " + request.mIdentity.requestType);

        switch(request.mIdentity.requestType) {

        case REQUEST_LOCATION_UPDATES:
        case REQUEST_PASSIVE_LOCATION_UPDATES:
            requestLocationUpdates(request);
            break;

        case REMOVE_LOCATION_UPDATES:
        case REMOVE_PASSIVE_LOCATION_UPDATES:
            removeLocationUpdates(request);
            break;

        default:
            loge("Invalid request type.");
            break;
        }
    }

    //=============================================================
    // New Incoming Session Requests
    //=============================================================
    private void requestLocationUpdates(SessionRequest request) {

        logv("requestLocationUpdates: package " + request.mIdentity.packageName +
             ", uid " + request.mIdentity.uid + ", pid " + request.mIdentity.pid +
             ", precision: " + request.mParams.precision);

        if (request.mParams.precision != SessionRequest.RequestPrecision.REQUEST_PRECISION_FINE &&
                request.mParams.precision !=
                    SessionRequest.RequestPrecision.REQUEST_PRECISION_COARSE) {
            loge("Invalid precision.");
            return;
        }

        mUserData = mDataPerPackageAndUser.getDataForUid(request.mIdentity.uid);
        if (mUserData == null) {
            loge("Failed to fetch user data, can't process start request!");
            return;
        }

        int calling_uid = request.mIdentity.uid;
        int calling_pid = request.mIdentity.pid;
        request.mIdentity.packageName = mContext.getPackageManager().getNameForUid(calling_uid);
        boolean isAllowedInBackground = mPermissionChangePolicy.
                isAllowedInBackground(calling_pid, calling_uid);

        SessionStatus session = mUserData.mSessionMap.get(request.getUniqueId());
        if (session == null) {
            logv("Adding new session for [" + request.mIdentity.packageName + "]");
            session = new SessionStatus(request, isAllowedInBackground);
            mUserData.mSessionMap.put(request.getUniqueId(), session);
        } else {
            session.mRequest = request;
            session.mIsAllowedInBackground = isAllowedInBackground;
        }

        logv("requestLocationUpdates: mUserData.mSessionMap size: " +
                mUserData.mSessionMap.size());

        if (Policy.validateAllPolicies(request)) {

            startSession(session.mRequest);

        } else {
            logv("Session blocked due to policy check.");
        }
    }

    private void removeLocationUpdates(SessionRequest request) {

        logv("removeLocationUpdates: package " + request.mIdentity.packageName +
             ", uid " + request.mIdentity.uid + ", pid " + request.mIdentity.pid +
             ", precision: " + request.mParams.precision);

        if (request.mParams.precision != SessionRequest.RequestPrecision.REQUEST_PRECISION_FINE &&
                request.mParams.precision !=
                    SessionRequest.RequestPrecision.REQUEST_PRECISION_COARSE) {
            loge("Invalid precision.");
            return;
        }

        // Ensure mUserData is from current user.
        // Calls to stop from destroyed Apps can arrive before onUserChange
        mUserData = mDataPerPackageAndUser.getDataForUid(request.mIdentity.uid);
        if (mUserData == null) {
            loge("Failed to fetch user data, can't process remove request!");
            return;
        }

        Log.d(TAG, "removeLocationUpdates: mUserData.mSessionMap size: "
                + mUserData.mSessionMap.size());
        Long idVal = request.getUniqueId();
        if (!mUserData.mSessionMap.containsKey(idVal)) {
            loge("Invalid session id: " + idVal);
            return;
        }

        SessionStatus session = mUserData.mSessionMap.get(idVal);

        Log.d(TAG, "removeLocationUpdates: session.mIsRunning: " + session.mIsRunning);
        if (session.mIsRunning) {
            stopSession(session.mRequest);
        }

        mUserData.mSessionMap.remove(idVal);
    }

    //=============================================================
    // Start/Stop sessions for all handlers
    //=============================================================
    private void startAllSessions() {

        logv("startAllSessions");

        for (SessionStatus session : mUserData.mSessionMap.values()) {

            if (!session.mIsRunning) {
                if (Policy.validateAllPolicies(session.mRequest)) {
                    startSession(session.mRequest);
                }
            }
        }
    }
    private void stopAllSessions() {

        logv("stopAllSessions");

        for (SessionStatus session : mUserData.mSessionMap.values()) {

            if (session.mIsRunning) {
                stopSession(session.mRequest);
            }
        }
    }

    //This method is used for SSR handling
    public void stopSessionsByPiduid(int pid, int uid) {
        logv("stopAllSessions by pid " + pid + " uid " + uid);

        mUserData = mDataPerPackageAndUser.getDataForUid(uid);
        Long id;
        for (Iterator it = mUserData.mSessionMap.entrySet().iterator(); it.hasNext();) {
            SessionStatus session = ((Map.Entry<Long, SessionStatus>)it.next()).getValue();
            if (session.mIsRunning && session.mRequest.mIdentity.pid == pid &&
                    session.mRequest.mIdentity.uid == uid) {
                stopSession(session.mRequest);
                it.remove();
            }
        }
    }
    //=============================================================
    // Start/Stop the session specified by the session data
    //=============================================================
    private void startSession(SessionRequest request) {
        // update session status and add power blame
        SessionStatus session = mUserData.mSessionMap.get(request.getUniqueId());
        synchronized (this) {
            if (!session.mIsRunning) {
                if (request.mParams.precision != RequestPrecision.REQUEST_PRECISION_COARSE) {
                    mPowerBlameReportingPolicy.addPowerBlame(request);
                }
                session.mIsRunning = true;
            } else {
                logv(" session, pid: " + request.mIdentity.pid + " already started");
                return;
            }
        }
        logv("Starting session, pid: " + request.mIdentity.pid +
                ", uid: " + request.mIdentity.uid);
        // Send request to owner for handling
        SessionRequest newRequest = new SessionRequest(request);
        newRequest.mIdentity.requestType = request.mIdentity.requestType;
        newRequest.mIdentity.owner.handle(newRequest);

    }

    private void stopSession(SessionRequest request) {
        // update session status and remove power blame
        SessionStatus session = mUserData.mSessionMap.get(request.getUniqueId());
        synchronized (this) {
            if (session.mIsRunning) {
                mPowerBlameReportingPolicy.removePowerBlame(request);
                session.mIsRunning = false;
            } else {
                logv(" session, pid: " + request.mIdentity.pid + " already stopped");
                return;
            }
        }
        logv("Stopping session, pid: " + request.mIdentity.pid +
                ", uid: " + request.mIdentity.uid);

        // Send request to owner for handling
        SessionRequest newRequest = new SessionRequest(request);
        if (newRequest.mIdentity.requestType ==
                SessionRequest.RequestType.REQUEST_LOCATION_UPDATES) {
            newRequest.mIdentity.requestType = SessionRequest.RequestType.REMOVE_LOCATION_UPDATES;
        } else if (newRequest.mIdentity.requestType ==
                SessionRequest.RequestType.REQUEST_PASSIVE_LOCATION_UPDATES) {
            newRequest.mIdentity.requestType =
                SessionRequest.RequestType.REMOVE_PASSIVE_LOCATION_UPDATES;
        }
        newRequest.mIdentity.owner.handle(newRequest);
    }

    //=============================================================
    // POLICY_NAME_USER_PROFILE_CHANGE
    //=============================================================
    class UserProfileChangePolicy extends Policy
            implements DataPerPackageAndUser.UserChangeListener<UserData> {

        public UserProfileChangePolicy() {
            super(PolicyName.POLICY_NAME_USER_PROFILE_CHANGE);
        }

        @Override
        public void onUserChange(Map<String, UserData> prevUserDataMap,
                                 Map<String, UserData> currentUserDataMap) {

            logv("onUserChange");

            mHandler.obtainMessage(MSG_HANDLE_USER_CHANGE,
                    Pair.create(prevUserDataMap, currentUserDataMap)).sendToTarget();
        }

        @Override
        public boolean isSessionAllowed(SessionRequest request) {

            // This policy never blocks a session
            return true;
        }

        public void handleUserChange(Map<String, UserData> prevUserDataMap,
                                      Map<String, UserData> currentUserDataMap) {

            // New user switched may be first time running
            if (currentUserDataMap.isEmpty()) {
                logv("current user data empty, creating new instance.");
                currentUserDataMap.put(mDataPerPackageAndUser.getPackageName(null),
                        new UserData());
            }

            UserData currentUserData =
                    currentUserDataMap.get(mDataPerPackageAndUser.getPackageName(null));
            UserData prevUserData =
                    prevUserDataMap.get(mDataPerPackageAndUser.getPackageName(null));

            // mUserData may have been updated if start/stop was called before
            mUserData = prevUserData;

            if (mIsLocationSettingsOn) {
                stopAllSessions();
            }

            // Update to new current user
            mUserData = currentUserData;

            if (mIsLocationSettingsOn) {
                startAllSessions();
            }
        }
    }

    //=============================================================
    // POLICY_NAME_LOCATION_SETTING_CHANGE
    //=============================================================
    private class LocationSettingChangePolicy extends Policy
                                              implements IzatService.ISystemEventListener {

        public LocationSettingChangePolicy() {

            super(PolicyName.POLICY_NAME_LOCATION_SETTING_CHANGE);

            LocationManager locationManager =
                    (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
            mIsLocationSettingsOn = locationManager.
                    isLocationEnabledForUser(Binder.getCallingUserHandle());

            mIZatServiceCtx.registerSystemEventListener(MSG_LOCATION_MODE_CHANGE, this);
        }

        @Override
        public void notify(int msgId, Object... args) {

            switch (msgId) {

            case MSG_LOCATION_MODE_CHANGE:
                boolean locationSettingsIsOn = (boolean)args[0];
                mHandler.obtainMessage(MSG_HANDLE_LOCATION_MODE_CHANGE, locationSettingsIsOn?1:0, 0)
                        .sendToTarget();
                break;

            default:
                loge("Unsupported msg id: " + msgId);
                break;
            }
        }

        @Override
        public boolean isSessionAllowed(SessionRequest request) {

            return mIsLocationSettingsOn;
        }

        public void handleLocationModeChange(boolean locationSettingsIsOn) {

            logi("handleLocationModeChange: " + locationSettingsIsOn);

            mIsLocationSettingsOn = locationSettingsIsOn;
            if (mIsLocationSettingsOn) {
                startAllSessions();
            } else {
                stopAllSessions();
            }
        }
    }

    //=============================================================
    // POLICY_NAME_BACKGROUND_THROTTLING
    //=============================================================
    private class BackgroundThrottlingPolicy extends Policy
                                             implements IzatService.ISystemEventListener {

        public BackgroundThrottlingPolicy() {

            super(PolicyName.POLICY_NAME_BACKGROUND_THROTTLING);
            mIZatServiceCtx.registerSystemEventListener(MSG_UID_IMPORTANCE_CHANGE, this);
        }

        @Override
        public void notify(int msgId, Object... args) {

            switch (msgId) {

            case MSG_UID_IMPORTANCE_CHANGE:
                int uid = (int)args[0];
                boolean isImportanceForeground = (boolean)args[1];
                mHandler.obtainMessage(MSG_HANDLE_UID_IMPORTANCE_CHANGE,
                        uid, isImportanceForeground?1:0).sendToTarget();
                break;

            default:
                loge("Unsupported msg id: " + msgId);
                break;
            }
        }

        @Override
        public boolean isSessionAllowed(SessionRequest request) {

            // This policy only throttles request tbf, never blocks a session.
            return true;
        }

        public void handleUidImportanceChange(int uid, boolean isImportanceForeground) {

            for (SessionStatus session : mUserData.mSessionMap.values()) {

                if (session.mRequest.mIdentity.uid == uid) {

                    logi("uid: " + uid + " pid: " + session.mRequest.mIdentity.uid +
                            ", session goes: " +
                            (isImportanceForeground? "foreground": "background"));

                    session.mIsInBackground = !isImportanceForeground;

                    // A running session went to background
                    if (session.mIsRunning && session.mIsInBackground) {

                        // If background session allowed and throttling policy applicable,
                        //  change min interval, or else stop the session.
                        session.mOriginalMinIntervalMillis =
                                session.mRequest.mParams.minIntervalMillis;
                        if (session.mRequest.mIdentity.owner.isPolicyApplicable(
                                    Policy.PolicyName.POLICY_NAME_BACKGROUND_THROTTLING,
                                    session.mRequest)) {
                            session.mRequest.mParams.minIntervalMillis = Math.max(
                                    BACKGROUND_APP_MIN_INTERVAL_MILLIS,
                                    session.mOriginalMinIntervalMillis);
                            if (session.mIsAllowedInBackground) {
                                startSession(session.mRequest);
                            }
                        }

                    } else if (isImportanceForeground) {

                        // restore session when it changes to foreground
                        session.mRequest.mParams.minIntervalMillis =
                                session.mOriginalMinIntervalMillis;
                        if (Policy.validateAllPolicies(session.mRequest)) {
                            startSession(session.mRequest);
                        }
                    }
                }
            }
        }
    }

    //=============================================================
    // POLICY_NAME_BACKGROUND_DENIED
    //=============================================================
    private class BackgroundDeniedPolicy extends Policy {

        public BackgroundDeniedPolicy() {

            super(PolicyName.POLICY_NAME_BACKGROUND_DENIED);
        }

        @Override
        public boolean isSessionAllowed(SessionRequest request) {

            SessionStatus session = mUserData.mSessionMap.get(request.getUniqueId());
            return !session.mIsInBackground;
        }

        public void handleUidImportanceChange(int uid, boolean isImportanceForeground) {

            for (SessionStatus session : mUserData.mSessionMap.values()) {

                if (session.mRequest.mIdentity.uid == uid &&
                    session.mRequest.mIdentity.owner.isPolicyApplicable(
                        Policy.PolicyName.POLICY_NAME_BACKGROUND_DENIED, session.mRequest)) {

                    session.mIsInBackground = !isImportanceForeground;

                    // A running session went to background
                    if (session.mIsRunning && session.mIsInBackground) {

                        stopSession(session.mRequest);

                    } else if (isImportanceForeground) {

                        if (Policy.validateAllPolicies(session.mRequest)) {
                            startSession(session.mRequest);
                        }
                    }
                }
            }
        }
    }

    //=============================================================
    // POLICY_NAME_POWER_BLAME_REPORTING
    //=============================================================
    private class PowerBlameReportingPolicy extends Policy {

        public PowerBlameReportingPolicy() {

            super(PolicyName.POLICY_NAME_POWER_BLAME_REPORTING);
        }

        @Override
        public boolean isSessionAllowed(SessionRequest request) {

            // This policy never blocks a session
            return true;
        }

        public void addPowerBlame(SessionRequest request) {

            // todo - should we allow power blame for apps in background ?

            mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 1, request.mIdentity.uid,
                    request.mIdentity.packageName).sendToTarget();
        }

        public void removePowerBlame(SessionRequest request) {

            mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 0, request.mIdentity.uid,
                    request.mIdentity.packageName).sendToTarget();
        }

        public void updateHighPowerLocationMonitoring(int uid, String packageName, boolean start) {

            // Package name should not be null while reporting
            if (packageName != null) {

                if (start) {
                    logi("startOpNoThrow: uid: " + uid + ", packageName: " + packageName);
                    mAppOpsMgr.startOpNoThrow(AppOpsManager.OP_MONITOR_HIGH_POWER_LOCATION,
                            uid, packageName, false, null, null);
                } else {
                    logi("finishOp: uid: " + uid + ", packageName: " + packageName);
                    mAppOpsMgr.finishOp(AppOpsManager.OP_MONITOR_HIGH_POWER_LOCATION,
                            uid, packageName);
                }

            } else {
                loge("Null packagename for uid: " + uid +
                        ", not reporting power, start: " + start);
            }
        }
    }

    //=============================================================
    // POLICY_NAME_PERMISSION_CHANGE
    //=============================================================
    private class PermissionChangePolicy extends Policy implements OnPermissionsChangedListener {

        public PermissionChangePolicy() {

            super(PolicyName.POLICY_NAME_PERMISSION_CHANGE);
            mContext.getPackageManager().addOnPermissionsChangeListener(this);
        }

        @Override
        public boolean isSessionAllowed(SessionRequest request) {

            SessionStatus session = mUserData.mSessionMap.get(request.getUniqueId());

            session.mIsAllowedInBackground = isAllowedInBackground(
                    request.mIdentity.pid, request.mIdentity.uid);
            session.mHasFineLocationAccess = isFineLocationAccessAllowed(
                    request.mIdentity.pid, request.mIdentity.uid);
            session.mHasCoarseLocationAccess = isCoarseLocationAccessAllowed(
                    request.mIdentity.pid, request.mIdentity.uid);

            //Only precise positioning APIs need FINE_LOCATION permission
            //For GTP APIs, allow high accuracy request even when clients only
            //have COARSE_LOCATION, only fuzzed location will be reported to
            //COARSE_LOCATION clients.
            boolean requiresFinePermission =
                    (session.mRequest.mIdentity.sessionType ==
                        SessionRequest.SessionType.SESSION_TYPE_PPE) ||
                    ((session.mRequest.mIdentity.sessionType ==
                        SessionRequest.SessionType.SESSION_TYPE_INVALID) &&
                    (session.mRequest.mParams.precision ==
                        SessionRequest.RequestPrecision.REQUEST_PRECISION_FINE));
            boolean requiresCoarsePermission = (session.mRequest.mParams.precision ==
                    SessionRequest.RequestPrecision.REQUEST_PRECISION_COARSE) ||
                    (session.mRequest.mIdentity.sessionType ==
                     SessionRequest.SessionType.SESSION_TYPE_GTP);

            return (!session.mIsInBackground || session.mIsAllowedInBackground) &&
                   ((requiresFinePermission && session.mHasFineLocationAccess) ||
                    (requiresCoarsePermission && session.mHasCoarseLocationAccess));
        }

        @Override
        public void onPermissionsChanged(int uid) {

            for (SessionStatus session : mUserData.mSessionMap.values()) {

                if (session.mRequest.mIdentity.uid == uid &&
                        session.mRequest.mIdentity.owner.isPolicyApplicable(
                                Policy.PolicyName.POLICY_NAME_PERMISSION_CHANGE,
                                session.mRequest)) {

                    logv("uid: " + uid + " pid: " + session.mRequest.mIdentity.pid +
                            ", session permission change, session Running: " + session.mIsRunning);

                    if (!isSessionAllowed(session.mRequest) && session.mIsRunning) {
                        stopSession(session.mRequest);
                    } else if (!session.mIsRunning &&
                            Policy.validateAllPolicies(session.mRequest)) {
                        startSession(session.mRequest);
                    }
                }
            }
        }

        public boolean isAllowedInBackground(int pid, int uid) {
            return mContext.checkPermission(
                    android.Manifest.permission.ACCESS_BACKGROUND_LOCATION, pid, uid) ==
                            PackageManager.PERMISSION_GRANTED;
        }
        public boolean isFineLocationAccessAllowed(int pid, int uid) {
            return mContext.checkPermission(
                    android.Manifest.permission.ACCESS_FINE_LOCATION, pid, uid) ==
                            PackageManager.PERMISSION_GRANTED;
        }
        public boolean isCoarseLocationAccessAllowed(int pid, int uid) {
            return mContext.checkPermission(
                    android.Manifest.permission.ACCESS_COARSE_LOCATION, pid, uid) ==
                            PackageManager.PERMISSION_GRANTED;
        }
    }

    //=============================================================
    // POLICY_NAME_APPOPS_CHANGE
    //=============================================================
    private class AppOpsChangePolicy extends Policy implements OnOpChangedListener {
        private int mLastOpMode = AppOpsManager.MODE_DEFAULT;
        public AppOpsChangePolicy() {
            super(PolicyName.POLICY_NAME_APPOPS_CHANGE);
        }
        @Override
        public void onOpChanged(String op, String packageName) {
            for (SessionStatus session : mUserData.mSessionMap.values()) {
                if (packageName.equals(session.mRequest.mIdentity.packageName) &&
                        session.mRequest.mIdentity.owner.isPolicyApplicable(
                                Policy.PolicyName.POLICY_NAME_APPOPS_CHANGE, session.mRequest)) {
                    logv("OnOpChangedListener  packageName: " + packageName + " pid: " +
                            session.mRequest.mIdentity.pid + "op: " + op +
                            ", session AppOps changed, session running: " + session.mIsRunning);
                    if (!isSessionAllowed(session.mRequest) && session.mIsRunning) {
                        stopSession(session.mRequest);
                    } else if (!session.mIsRunning &&
                            Policy.validateAllPolicies(session.mRequest)) {
                        startSession(session.mRequest);
                    }
                    break;
                }
            }
        }
        @Override
        public boolean isSessionAllowed(SessionRequest request) {
            SessionStatus session = mUserData.mSessionMap.get(request.getUniqueId());
            session.mHasFineLocationAccess = mAppOpsMgr.checkOpNoThrow(
                    AppOpsManager.OPSTR_FINE_LOCATION, request.mIdentity.uid,
                    request.mIdentity.packageName) == AppOpsManager.MODE_ALLOWED;
            session.mHasCoarseLocationAccess = mAppOpsMgr.checkOpNoThrow(
                    AppOpsManager.OPSTR_COARSE_LOCATION, request.mIdentity.uid,
                    request.mIdentity.packageName) == AppOpsManager.MODE_ALLOWED;

            // PPE APIs need FINE_LOCATION permission
            // For GTP APIs, allow high accuracy request even when clients only
            // have COARSE_LOCATION, only fuzzed location will be reported to
            // COARSE_LOCATION clients.
            boolean requiresFinePermission =
                    session.mRequest.mIdentity.sessionType ==
                    SessionRequest.SessionType.SESSION_TYPE_PPE;
            boolean requiresCoarsePermission = (session.mRequest.mParams.precision ==
                    SessionRequest.RequestPrecision.REQUEST_PRECISION_COARSE) ||
                    (session.mRequest.mIdentity.sessionType ==
                     SessionRequest.SessionType.SESSION_TYPE_GTP);
            logv("OnOpChangedListener FINE op MODE: " + session.mHasFineLocationAccess +
                    ", COARSE op MODE: " + session.mHasCoarseLocationAccess + ", req FINE perm: " +
                    requiresFinePermission + ", req COARSE perm: " + requiresCoarsePermission);
            return ((requiresFinePermission && session.mHasFineLocationAccess) ||
                    (requiresCoarsePermission && session.mHasCoarseLocationAccess));
        }
    }
    //=============================================================
    // POLICY_NAME_POWER_SAVE_MODE
    //=============================================================
    private class PowerSaveModePolicy extends Policy {

        public PowerSaveModePolicy() {

            super(PolicyName.POLICY_NAME_POWER_SAVE_MODE);

            IntentFilter powerSaveModeIntentFilter = new IntentFilter();
            powerSaveModeIntentFilter.addAction(PowerManager.ACTION_POWER_SAVE_MODE_CHANGED);
            mContext.registerReceiverAsUser(
                new BroadcastReceiver() {
                    @Override
                    public void onReceive(Context context, Intent intent) {
                        if (PowerManager.ACTION_POWER_SAVE_MODE_CHANGED.equals(
                                intent.getAction())) {
                            mIsPowerSaveModeOn = isPowerSaveModeOn();
                            handlePowerSaveModeChange();
                        } else {
                            loge("Unexpected intent action: " + intent.getAction());
                        }
                    }
                }, UserHandle.ALL, powerSaveModeIntentFilter, null, null);

            mIsPowerSaveModeOn = isPowerSaveModeOn();
        }

        public boolean isPowerSaveModeOn() {

            int mode = mContext.getSystemService(PowerManager.class).getLocationPowerSaveMode();
            return (mode != PowerManager.LOCATION_MODE_NO_CHANGE);
        }

        @Override
        public boolean isSessionAllowed(SessionRequest request) {

            return !mIsPowerSaveModeOn;
        }

        public void handlePowerSaveModeChange() {

            logi("handlePowerSaveModeChange: mIsPowerSaveModeOn: " + mIsPowerSaveModeOn);

            for (SessionStatus session : mUserData.mSessionMap.values()) {

                if (session.mRequest.mIdentity.owner.
                        isPolicyApplicable(Policy.PolicyName.POLICY_NAME_POWER_SAVE_MODE,
                            session.mRequest)) {
                    if (session.mIsRunning && mIsPowerSaveModeOn) {
                        stopSession(session.mRequest);
                    } else if (!session.mIsRunning && !mIsPowerSaveModeOn) {
                        if (Policy.validateAllPolicies(session.mRequest)) {
                            startSession(session.mRequest);
                        }
                    }
                }
            }
        }
    }

    //=============================================================
    // POLICY_NAME_SCREEN_INACTIVE
    //=============================================================
    private class ScreenInactivePolicy extends Policy {

        public ScreenInactivePolicy() {

            super(PolicyName.POLICY_NAME_SCREEN_INACTIVE);

            IntentFilter screenIntentFilter = new IntentFilter();
            screenIntentFilter.addAction(Intent.ACTION_SCREEN_OFF);
            screenIntentFilter.addAction(Intent.ACTION_SCREEN_ON);
            mContext.registerReceiverAsUser(
                new BroadcastReceiver() {
                    @Override
                    public void onReceive(Context context, Intent intent) {
                        if (Intent.ACTION_SCREEN_ON.equals(intent.getAction())) {
                            mIsScreenOff = false;
                        } else if (Intent.ACTION_SCREEN_OFF.equals(intent.getAction())) {
                            mIsScreenOff = true;
                        }
                        screenStateChanged();
                    }
                }, UserHandle.ALL, screenIntentFilter, null, null);
        }

        @Override
        public boolean isSessionAllowed(SessionRequest request) {

            return !mIsScreenOff;
        }

        private void screenStateChanged() {

            for (SessionStatus session : mUserData.mSessionMap.values()) {

                if (session.mRequest.mIdentity.owner.
                        isPolicyApplicable(Policy.PolicyName.POLICY_NAME_SCREEN_INACTIVE, session.mRequest)) {

                    logi("screen state changed: mIsScreenOff: " + mIsScreenOff);

                    if (session.mIsRunning && mIsScreenOff) {
                        stopSession(session.mRequest);
                    } else if (!session.mIsRunning && !mIsScreenOff) {

                        if (Policy.validateAllPolicies(session.mRequest)) {
                            startSession(session.mRequest);
                        }
                    }
                }
            }
        }
    }
}
