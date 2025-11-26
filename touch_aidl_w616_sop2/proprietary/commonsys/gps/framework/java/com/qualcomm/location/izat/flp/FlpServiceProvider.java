/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2015-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.flp;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationManager;
import android.os.IBinder;
import android.os.Binder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.UserHandle;
import android.util.Log;
import android.util.Pair;
import android.os.Handler;
import android.os.Message;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Bundle;

import android.app.AppOpsManager;
import android.app.ActivityManager;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Set;

import com.qti.flp.*;
import com.qualcomm.location.izat.CallbackData;
import com.qualcomm.location.izat.DataPerPackageAndUser;
import com.qualcomm.location.izat.IzatService;
import com.qualcomm.location.izat.esstatusreceiver.EsStatusReceiver;

import vendor.qti.gnss.V4_0.ILocHidlGnss;
import vendor.qti.gnss.V4_0.ILocHidlFlpService;
import vendor.qti.gnss.V4_0.ILocHidlFlpServiceCallback;
import vendor.qti.gnss.V1_0.LocHidlLocation;
import vendor.qti.gnss.V1_0.LocHidlBatchOptions;
import vendor.qti.gnss.V1_0.LocHidlBatchStatusInfo;
import com.qualcomm.location.idlclient.IDLClientUtils;
import com.qualcomm.location.utils.IZatServiceContext;
import com.qualcomm.location.idlclient.LocIDLClientBase.*;
import com.qualcomm.location.idlclient.*;
import vendor.qti.gnss.ILocAidlFlpServiceCallback;
import vendor.qti.gnss.LocAidlLocationFlagsBits;

public class FlpServiceProvider implements IzatService.ISystemEventListener,
                                           Handler.Callback {
    private static final String TAG = "FlpServiceProvider";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    private static final int FLP_RESULT_SUCCESS = 0;
    private static final int FLP_RESULT_ERROR = -1;
    private static final int LOCATION_REPORT_ON_FULL_INDICATION = 1;
    private static final int LOCATION_REPORT_ON_FIX_INDICATION = 2;
    private static final int LOCATION_REPORT_ON_QUERY = 4;
    private static final int LOCATION_REPORT_ON_INDICATIONS = 8; // for legacy
    private static final int FLP_PASSIVE_LISTENER_HW_ID = -1;
    private static final Object sCallBacksLock = new Object();
    private static final Object sStatusCallbacksLock = new Object();
    private static final Object sLocationSettingsLock = new Object();
    private final Context mContext;
    private final DataPerPackageAndUser<FlpUserData> mDataPerPackageAndUser;
    private AppOpsManager mAppOpsMgr;
    private final Looper mLooper;
    private Handler mHandler;
    private FlpServiceProviderIDLClient mFlpIDLClient;
    private IZatServiceContext mIZatServiceCtx;
    private int mFlpFeaturMasks = -1;
    private boolean mIsLocationSettingsOn;
    private boolean mIsUserEmergency;

    public static final int FLP_SESSION_BACKGROUND = 1;
    public static final int FLP_SESSION_FOREGROUND = 2;
    public static final int FLP_SESSION_PASSIVE = 4;

    public static final int FLP_BG_NOTIFICATION_ROUTINE = 1;
    public static final int FLP_BG_NOTIFICATION_OUTDOOR_TRIP = 3;

    public static final int BATCHING_STATUS_TRIP_COMPLETED = 0;
    public static final int BATCHING_STATUS_POSITION_AVAILABE = 1;
    public static final int BATCHING_STATUS_POSITION_UNAVAILABLE = 2;

    public static final int BATCHING_MODE_ROUTINE = 0;
    public static final int BATCHING_MODE_OUTDOOR_TRIP = 1;

    public static final int POWER_MODE_INVALID = 0;
    public static final int POWER_MODE_M1 = 0;
    public static final int POWER_MODE_M2 = 0;
    public static final int POWER_MODE_M3 = 0;
    public static final int POWER_MODE_M4 = 0;
    public static final int POWER_MODE_M5 = 0;

    public static final int MSG_UPDATE_HIGHPOWER_MONITOR = IZatServiceContext.MSG_FLP_BASE + 1;
    private static final String ACCESS_BACKGROUND_LOCATION =
            android.Manifest.permission.ACCESS_BACKGROUND_LOCATION;
    private static final String ACCESS_ROBUST_LOCATION =
            "com.qualcomm.qti.permission.ACCESS_ROBUST_LOCATION_API";

    // Maximum delta beetwen most recent location in batch and session start time
    private static final int MAX_STARTTIME_DELTA_MS = 60 * 1000;

    private static final long MilliToNano(long millis) {
        return (millis > (Long.MAX_VALUE/1000000)) ? Long.MAX_VALUE : millis*1000000;
    }

    private class BgSessionData {
        private int mBgNotificationType;
        private long mSessionStartTime;

        BgSessionData() {
            mBgNotificationType = FLP_BG_NOTIFICATION_ROUTINE;
            mSessionStartTime = 0;
        }
    }


    // Used for data transfer into native layer.
    // Note the period difference in units: nanoseconds in native layer, milliseconds above.
    private class FlpSessionData {
        private int mId;
        private int mFlags;
        private long mPeriodNs;
        private int mDistanceIntervalMps;
        private long mTripDistanceM;
        private int mPowerMode;
        private long mTbmMs;
        private int mCallingUid;
        private String mCallingPackage;
        private boolean mIsRunning;
        private boolean mIsAllowedInBackground;

        FlpSessionData(int id,
                       int flags,
                       long period_ms,
                       int distance_interval_mps,
                       long trip_distance_m,
                       int power_mode,
                       long tbm_ms,
                       int calling_uid,
                       String calling_package,
                       boolean isAllowedInBackground) {
            mId = id;
            mFlags = flags;
            mPeriodNs = MilliToNano(period_ms);
            mDistanceIntervalMps = distance_interval_mps;
            mTripDistanceM = trip_distance_m;
            mPowerMode = power_mode;
            mTbmMs = tbm_ms;
            mCallingUid = calling_uid;
            mCallingPackage = calling_package;
            mIsRunning = false;
            mIsAllowedInBackground = isAllowedInBackground;
        }
    }

    private class CbCookie {
        long id;
        long sessionStartTime;
        boolean isRobustLocationAllowed = false;
        String packageName;

        CbCookie(String pkg) {
            packageName = pkg;
        }
    }

    private RemoteCallbackList<ILocationCallback> mCallbacksForBg
            = new RemoteCallbackList<ILocationCallback>();
    private RemoteCallbackList<ILocationCallback> mCallbacksForFg
            = new RemoteCallbackList<ILocationCallback>();
    private RemoteCallbackList<ILocationCallback> mCallbacksForPassive
            = new RemoteCallbackList<ILocationCallback>();
    private RemoteCallbackList<IMaxPowerAllocatedCallback> mMaxPowerCallbacks
            = new RemoteCallbackList<IMaxPowerAllocatedCallback>();
    private RemoteCallbackList<ISessionStatusCallback> mCallbacksForStatus
            = new RemoteCallbackList<ISessionStatusCallback>();

    // Used to store FLP data related to each user.
    private class FlpUserData extends CallbackData {
        private Map<Long, BgSessionData> mBgSessionMap = new ConcurrentHashMap<Long,
                BgSessionData>();
        private Map<Long, FlpSessionData> mFlpSessionMap =
                new ConcurrentHashMap<Long, FlpSessionData>();

        private Queue<Pair<ILocationCallback,Long>> mQueryCbQueue
                = new LinkedList<Pair<ILocationCallback,Long>>();

        // Backup copy to allow batch register/deregister on the corresponding RemoteCallbackList
        private Map<ILocationCallback, CbCookie> mCallbacksForBgMap =
                new HashMap<ILocationCallback, CbCookie>();
        private Map<ILocationCallback, CbCookie> mCallbacksForFgMap =
                new HashMap<ILocationCallback, CbCookie>();
        private Map<ILocationCallback, CbCookie> mCallbacksForPassiveMap =
                new HashMap<ILocationCallback, CbCookie>();
        private Map<ISessionStatusCallback, CbCookie> mCallbacksForStatusMap
                = new HashMap<ISessionStatusCallback, CbCookie>();

        private Map<IMaxPowerAllocatedCallback, CbCookie> mMaxPowerCallbacksMap
                = new ConcurrentHashMap<IMaxPowerAllocatedCallback, CbCookie>();
    }

    // For FLP only 1 instance per user.
    // Will be switched accordingly on user changes.
    private FlpUserData mFlpUserData = new FlpUserData();

    public boolean handleMessage(Message msg) {
        int msgID = msg.what;
        switch(msgID){
            case MSG_UPDATE_HIGHPOWER_MONITOR:
            {
                Log.d(TAG, "MSG_UPDATE_HIGHPOWER_MONITOR");
                ArrayList<Pair<Integer,String>> sList =
                        (ArrayList<Pair<Integer,String>>)msg.obj;
                boolean start = (msg.arg1 == 1) ? true:false;
                updateHighPowerLocationMonitoring(sList, start);
            }
            break;
            default:
                Log.w(TAG, "Unhandled Message " + msg.what);
        }
        return true;
    }

    private static FlpServiceProvider sInstance = null;
    public synchronized static FlpServiceProvider getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new FlpServiceProvider(ctx);
        }
        return sInstance;
    }

    @Override
    public void onAidlClientDied(String packageName, int pid, int uid) {
        Log.d(TAG, "aidl client crash: " + packageName);
        //Background callback removal
        for (Iterator it = mFlpUserData.mCallbacksForBgMap.entrySet().iterator(); it.hasNext();) {
            Map.Entry item = (Map.Entry)it.next();
            CbCookie cookie = (CbCookie)item.getValue();
            if (cookie.packageName.equals(packageName)) {
                ILocationCallback cb = (ILocationCallback)item.getKey();
                mCallbacksForBg.unregister(cb);
                it.remove();
                mFlpUserData.mBgSessionMap.remove(cookie.id);
                try {
                    mBinder.stopFlpSession((int)cookie.id);
                } catch (RemoteException e) {
                }
            }
        }
        //Foreground callback removal
        for (Iterator it = mFlpUserData.mCallbacksForFgMap.entrySet().iterator(); it.hasNext();) {
            Map.Entry item = (Map.Entry)it.next();
            CbCookie cookie = (CbCookie)item.getValue();
            if (cookie.packageName.equals(packageName)) {
                ILocationCallback cb = (ILocationCallback)item.getKey();
                mCallbacksForPassive.unregister(cb);
                it.remove();
                try {
                    mBinder.stopFlpSession((int)cookie.id);
                } catch (RemoteException e) {
                }
            }
        }
        //passive callback removal
        for (Iterator it = mFlpUserData.mCallbacksForPassiveMap.entrySet().iterator();
                it.hasNext();) {
            Map.Entry item = (Map.Entry)it.next();
            CbCookie cookie = (CbCookie)item.getValue();
            if (cookie.packageName.equals(packageName)) {
                ILocationCallback cb = (ILocationCallback)item.getKey();
                mCallbacksForFg.unregister(cb);
                it.remove();
                int id = (int)cookie.id;
                if (id != FLP_PASSIVE_LISTENER_HW_ID) {
                    try {
                        mBinder.stopFlpSession(id);
                    } catch (RemoteException e) {
                    }
                }
            }
        }
        //status callback removal
        for (Iterator it = mFlpUserData.mCallbacksForStatusMap.entrySet().iterator();
                it.hasNext();) {
            Map.Entry item = (Map.Entry)it.next();
            CbCookie cookie = (CbCookie)item.getValue();
            if (cookie.packageName.equals(packageName)) {
                ISessionStatusCallback cb = (ISessionStatusCallback)item.getKey();
                mCallbacksForStatus.unregister(cb);
                it.remove();
            }
        }
        //max power callback removal
        for (Iterator it = mFlpUserData.mMaxPowerCallbacksMap.entrySet().iterator();
                it.hasNext();) {
            Map.Entry item = (Map.Entry)it.next();
            CbCookie cookie = (CbCookie)item.getValue();
            if (cookie.packageName.equals(packageName)) {
                IMaxPowerAllocatedCallback cb = (IMaxPowerAllocatedCallback)item.getKey();
                mMaxPowerCallbacks.unregister(cb);
                it.remove();
            }
        }
    }

    private FlpServiceProvider(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "FlpServiceProvider construction");
        }
        mContext = ctx;
        if (LocIDLClientBase.getIDLServiceVersion().compareTo(IDLServiceVersion.V_AIDL) >= 0) {
            mFlpIDLClient = new FlpServiceProviderIDLClient(this);
        } else {
            mFlpIDLClient = new FlpServiceProviderHIDLClient(this);
        }

        LocationManager locationManager =
                (LocationManager) ctx.getSystemService(Context.LOCATION_SERVICE);
        mIsLocationSettingsOn = locationManager
                .isLocationEnabledForUser(Binder.getCallingUserHandle());

        mDataPerPackageAndUser = new DataPerPackageAndUser<FlpUserData>(mContext,
                new UserChanged());
        mDataPerPackageAndUser.useCommonPackage();
        mDataPerPackageAndUser.setData(mFlpUserData);

        mLooper = IZatServiceContext.getInstance(mContext).getLooper();
        mHandler = new Handler(mLooper, this);
        mAppOpsMgr = mContext.getSystemService(AppOpsManager.class);

        IzatService.AidlClientDeathNotifier.getInstance().registerAidlClientDeathCb(this);

        //Register Location mode change and uid importance change event
        mIZatServiceCtx = IZatServiceContext.getInstance(mContext);
        mIZatServiceCtx.registerSystemEventListener(MSG_LOCATION_MODE_CHANGE, this);
        mIZatServiceCtx.registerSystemEventListener(MSG_UID_IMPORTANCE_CHANGE, this);
    }

    @Override
    public void notify(int msgId, Object... args) {
        if (msgId == MSG_LOCATION_MODE_CHANGE) {
            boolean locationSettingsIsOn = (boolean)args[0];
            Log.d(TAG, "location mode change: " + locationSettingsIsOn);
            synchronized (sLocationSettingsLock) {
                mIsLocationSettingsOn = locationSettingsIsOn;
                if (mIsLocationSettingsOn) {
                    startFlpSessions();
                } else {
                    stopFlpSessions();
                }
            }
        } else if (msgId == MSG_UID_IMPORTANCE_CHANGE) {
            int uid = (int)args[0];
            boolean isImportanceForeground = (boolean)args[1];
            ArrayList<Pair<Integer, String>> startList = new ArrayList<Pair<Integer, String>>();
            ArrayList<Pair<Integer, String>> stopList = new ArrayList<Pair<Integer, String>>();
            for (FlpSessionData flpSessionData : mFlpUserData.mFlpSessionMap.values()) {
                if (flpSessionData.mCallingUid == uid) {
                    Log.d(TAG, "uid: " + uid + " pid: " + flpSessionData.mCallingUid +
                            ", session goes: " +
                            (isImportanceForeground? "foreground": "background"));
                    if (flpSessionData.mIsRunning) {
                        //stop background session if without ACCESS_BACKGROUND_LOCATION
                        if (!flpSessionData.mIsAllowedInBackground && !isImportanceForeground) {
                            mFlpIDLClient.stopSession(flpSessionData.mId);
                            stopList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                        flpSessionData.mCallingPackage));
                            flpSessionData.mIsRunning = false;
                        }
                    } else if (isImportanceForeground) {
                        //restart session when it changes to foreground
                        mFlpIDLClient.startSession(flpSessionData.mId,
                                flpSessionData.mFlags,
                                flpSessionData.mPeriodNs,
                                flpSessionData.mDistanceIntervalMps,
                                flpSessionData.mTripDistanceM,
                                flpSessionData.mPowerMode,
                                flpSessionData.mTbmMs);
                        startList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                    flpSessionData.mCallingPackage));
                        flpSessionData.mIsRunning = true;
                    }
                }
            }
            if (!stopList.isEmpty()) {
                mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 0, 1, stopList)
                        .sendToTarget();
            }
            if (!startList.isEmpty()) {
                mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 1, 1, startList)
                        .sendToTarget();
            }
        }
    }

    public void registerEsStatusUpdate() {
        EsStatusReceiver.getInstance(mContext).registerEsStatusListener(
                new EsStatusReceiver.IEsStatusListener() {
            public void onStatusChanged(boolean isEmergencyMode) {
                Log.d(TAG, "Emergency mode changed to : " + isEmergencyMode);
                synchronized (sLocationSettingsLock) {
                    mIsUserEmergency = isEmergencyMode;
                    // restore the cached flp sessions when goes to User Emergency status
                    if (mIsUserEmergency) {
                        startFlpSessions();
                    }
                }
           }});
    }

    public void updateHighPowerLocationMonitoring(ArrayList<Pair<Integer, String>> sList,
                                                  boolean start) {
        for(Pair<Integer,String> item : sList) {
            if (start) {
                Log.d(TAG, "startOpNoThrow");
                mAppOpsMgr.startOpNoThrow(AppOpsManager.OP_MONITOR_HIGH_POWER_LOCATION,
                                          item.first, item.second, false, null, null);
            } else {
                Log.d(TAG, "finishOp");
                mAppOpsMgr.finishOp(AppOpsManager.OP_MONITOR_HIGH_POWER_LOCATION,
                                    item.first, item.second);
            }
        }
    }

    private void startFlpSessions() {
        ArrayList<Pair<Integer, String>> sList = new ArrayList<Pair<Integer, String>>();
        for (FlpSessionData flpSessionData : mFlpUserData.mFlpSessionMap.values()) {
            if (VERBOSE) {
                Log.d(TAG, "Starting flp session id: " + flpSessionData.mId);
            }
            if (!flpSessionData.mIsRunning) {
                mFlpIDLClient.startSession(flpSessionData.mId,
                        flpSessionData.mFlags,
                        flpSessionData.mPeriodNs,
                        flpSessionData.mDistanceIntervalMps,
                        flpSessionData.mTripDistanceM,
                        flpSessionData.mPowerMode,
                        flpSessionData.mTbmMs);
                sList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                                    flpSessionData.mCallingPackage));
                flpSessionData.mIsRunning = true;
            }

        }
        mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 1, 1, sList).sendToTarget();
    }

    private void stopFlpSessions() {
        ArrayList<Pair<Integer, String>> sList = new ArrayList<Pair<Integer, String>>();
        for (FlpSessionData flpSessionData : mFlpUserData.mFlpSessionMap.values()) {
            if (VERBOSE) {
                Log.d(TAG, "Stoping flp session id: " + flpSessionData.mId);
            }
            if (flpSessionData.mIsRunning) {
                mFlpIDLClient.stopSession(flpSessionData.mId);
                sList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                                    flpSessionData.mCallingPackage));
                flpSessionData.mIsRunning = false;
            }
        }
        mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 0, 1, sList).sendToTarget();
    }

    /* Remote binder */
    private final IFlpService.Stub mBinder = new IFlpService.Stub() {
        public void registerCallback(final int sessionType,
                                     final int id,
                                     final ILocationCallback cb,
                                     final long sessionStartTime) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): registerCallback()," +
                           " sessionType is " + sessionType + "; id is " + id +
                           "; sessionStartTime is " + sessionStartTime +
                           "; cb:" + cb);
            }
            if (cb != null) {
                if (mCallbacksForBg != null &&
                    mCallbacksForFg != null &&
                    mCallbacksForPassive != null) {
                    String packageName =
                            mContext.getPackageManager().getNameForUid(Binder.getCallingUid());
                    switch (sessionType) {
                        case FLP_SESSION_BACKGROUND:
                            synchronized (sCallBacksLock) {
                                CbCookie cookie = new CbCookie(packageName);
                                cookie.id = id;
                                mCallbacksForBg.register(cb, cookie);
                                mFlpUserData.mCallbacksForBgMap.put(cb, cookie);

                                BgSessionData bgSessData;
                                Long idVal = Long.valueOf(id);
                                if (mFlpUserData.mBgSessionMap.containsKey(idVal)) {
                                    bgSessData = mFlpUserData.mBgSessionMap.get(idVal);
                                } else {
                                    bgSessData = new BgSessionData();
                                }

                                bgSessData.mSessionStartTime = sessionStartTime;
                                mFlpUserData.mBgSessionMap.put(idVal, bgSessData);
                            }
                            break;
                        case FLP_SESSION_FOREGROUND:
                            int calling_uid = Binder.getCallingUid();
                            int calling_pid = Binder.getCallingPid();
                            //Robust Location information only report to
                            //clients who has ACCESS_ROBUST_LOCATION permission &
                            //the device is in bootloader locked status
                            boolean isRobustLocationAllowed =
                                    (mContext.checkPermission(ACCESS_ROBUST_LOCATION,
                                    calling_pid, calling_uid) == PackageManager.PERMISSION_GRANTED)
                                    && (!IzatService.isDeviceOEMUnlocked(mContext));

                            synchronized (sCallBacksLock) {
                                CbCookie cookie = new CbCookie(packageName);
                                cookie.sessionStartTime = sessionStartTime;
                                cookie.isRobustLocationAllowed = isRobustLocationAllowed;
                                cookie.id = id;
                                mCallbacksForFg.register(cb, cookie);
                                mFlpUserData.mCallbacksForFgMap.put(cb, cookie);
                            }
                            break;
                        case FLP_SESSION_PASSIVE:
                            synchronized (sCallBacksLock) {
                                CbCookie cookie = new CbCookie(packageName);
                                cookie.sessionStartTime = sessionStartTime;
                                cookie.id = id;
                                mCallbacksForPassive.register(cb, cookie);
                                mFlpUserData.mCallbacksForPassiveMap.put(cb, cookie);
                            }
                            break;
                        default:
                            Log.e(TAG, "unknown sessionType");
                            break;
                    }
                } else {
                    Log.e(TAG, "one of the callback list is not created");
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }

        public void unregisterCallback(final int sessionType,
                                       final ILocationCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): unregisterCallback(): cb:" + cb);
            }
            if (cb != null) {
                if (mCallbacksForBg != null &&
                    mCallbacksForFg != null &&
                    mCallbacksForPassive != null) {
                    synchronized (sCallBacksLock) {
                        switch (sessionType) {
                            case FLP_SESSION_BACKGROUND:
                                mCallbacksForBg.unregister(cb);
                                mFlpUserData.mCallbacksForBgMap.remove(cb);
                                break;
                            case FLP_SESSION_FOREGROUND:
                                mCallbacksForFg.unregister(cb);
                                mFlpUserData.mCallbacksForFgMap.remove(cb);
                                break;
                            case FLP_SESSION_PASSIVE:
                                mCallbacksForPassive.unregister(cb);
                                mFlpUserData.mCallbacksForPassiveMap.remove(cb);
                                break;
                            default:
                                Log.e(TAG, "unknown sessionType");
                                break;
                        }
                    }
                } else {
                    Log.e(TAG, "one of the callback list is not created");
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }

        public void registerForSessionStatus(final int id, final ISessionStatusCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): registerForSessionStatus() cb:" + cb);
            }
            if (cb != null) {
                if (mCallbacksForStatus != null) {
                    String packageName =
                            mContext.getPackageManager().getNameForUid(Binder.getCallingUid());
                    synchronized (sStatusCallbacksLock) {
                        CbCookie cookie = new CbCookie(packageName);
                        cookie.id = (long)id;
                        mCallbacksForStatus.register(cb, cookie);
                        mFlpUserData.mCallbacksForStatusMap.put(cb, cookie);
                    }
                } else {
                      Log.e(TAG, "cb is null");
                }
            }
        }

        public void unregisterForSessionStatus(final ISessionStatusCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in FlpService.Stub unregisterForSessionStatus() cb = : " + cb);
            }
            if (cb != null) {
                if (mCallbacksForStatus != null) {
                    synchronized(sStatusCallbacksLock) {
                        mCallbacksForStatus.unregister(cb);
                        mFlpUserData.mCallbacksForStatusMap.remove(cb);
                    }
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }

        public int getAllSupportedFeatures() {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): getAllSupportedFeatures()");
            }
            mFlpFeaturMasks = mFlpIDLClient.getAllSupportedFeatures();
            Log.d(TAG, "FlpServiceProvider feature mask: " + mFlpFeaturMasks);
            return mFlpFeaturMasks;
        }

        public int startFlpSession(int id,
                                   int flags,
                                   long period_ms,
                                   int distance_interval_mps,
                                   long trip_distance_m) {

            return startFlpSessionWithPower(
                    // No power mode and tbm value
                    id, flags, period_ms, distance_interval_mps,
                    trip_distance_m, POWER_MODE_INVALID, 0);
        }

        public int startFlpSessionWithPower(int id,
                int flags,
                long period_ms,
                int distance_interval_mps,
                long trip_distance_m,
                int power_mode,
                long tbm_ms) {

            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): startFlpSession()" +
                           "; id is " + id +
                           "; period_ms is " + period_ms +
                           "; flags is " + flags +
                           "; distance interval is " + distance_interval_mps +
                           "; trip distance is " + trip_distance_m +
                           "; power mode is " + power_mode +
                           "; tbm is " + tbm_ms);
            }

            // Ensure mFlpUserData is from current user.
            // Calls to start from running Apps can arrive before onUserChange
            mFlpUserData = mDataPerPackageAndUser.getData();

            // BgSessionMap only cares about BG sessions i.e. routine / trip
            synchronized(sCallBacksLock) {
                if ((flags == FLP_BG_NOTIFICATION_ROUTINE) ||
                    (flags == FLP_BG_NOTIFICATION_OUTDOOR_TRIP)) {
                    BgSessionData bgSessData = mFlpUserData.mBgSessionMap.get(Long.valueOf(id));

                    if (bgSessData != null) {
                        bgSessData.mBgNotificationType = flags;
                    } else {
                        Log.e(TAG, "No registered callback for this session id.");
                    }
                }
            }

            int calling_uid = Binder.getCallingUid();
            int calling_pid = Binder.getCallingPid();
            String calling_package = mContext.getPackageManager().getNameForUid(calling_uid);
            boolean isAllowedInBackground = mContext.checkPermission(ACCESS_BACKGROUND_LOCATION,
                    calling_pid, calling_uid) == PackageManager.PERMISSION_GRANTED;

            FlpSessionData flpSessionData = new FlpSessionData(id,
                                                               flags,
                                                               period_ms,
                                                               distance_interval_mps,
                                                               trip_distance_m,
                                                               power_mode,
                                                               tbm_ms,
                                                               calling_uid,
                                                               calling_package,
                                                               isAllowedInBackground);
            int result;
            synchronized (sLocationSettingsLock) {
                mFlpUserData.mFlpSessionMap.put(Long.valueOf(id), flpSessionData);
                // For QC FLP, start session when emergency even though Location OFF
                if ((!mIsLocationSettingsOn && !mIsUserEmergency) || flpSessionData.mIsRunning) {
                    if (VERBOSE) {
                        Log.d(TAG, "Location setting is OFF or already running, no need to start.");
                    }
                    result = FLP_RESULT_SUCCESS;
                } else {
                    result = mFlpIDLClient.startSession(flpSessionData.mId,
                                                      flpSessionData.mFlags,
                                                      flpSessionData.mPeriodNs,
                                                      flpSessionData.mDistanceIntervalMps,
                                                      flpSessionData.mTripDistanceM,
                                                      flpSessionData.mPowerMode,
                                                      flpSessionData.mTbmMs);
                    ArrayList<Pair<Integer, String>> sList = new ArrayList<Pair<Integer, String>>();
                    sList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                                        flpSessionData.mCallingPackage));
                    mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 1, 1, sList)
                            .sendToTarget();
                    flpSessionData.mIsRunning = true;
                }
            }

            return result;
        }

        public int updateFlpSession(int id,
                int flags,
                long period_ms,
                int distance_interval_mps,
                long trip_distance_m) {

            return updateFlpSessionWithPower(id, flags, period_ms,
                    distance_interval_mps, trip_distance_m, POWER_MODE_INVALID, 0);
        }

        public int updateFlpSessionWithPower(int id,
                                    int flags,
                                    long period_ms,
                                    int distance_interval_mps,
                                    long trip_distance_m,
                                    int power_mode,
                                    long tbm_ms) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): updateFlpSession()" +
                           "; id is " + id +
                           "; period_ms is " + period_ms +
                           "; flags is " + flags +
                           "; distance_ms is " + distance_interval_mps +
                           "; trip distance " + trip_distance_m +
                           "; power mode " + power_mode +
                           "; tbm_ms " + tbm_ms);
            }

            // BgSessionMap only cares about BG sessions i.e. routine / trip
            synchronized(sCallBacksLock) {
                Long idVal = Long.valueOf(id);
                BgSessionData bgSessData = mFlpUserData.mBgSessionMap.get(idVal);

                if ((flags == FLP_BG_NOTIFICATION_ROUTINE) ||
                    (flags == FLP_BG_NOTIFICATION_OUTDOOR_TRIP)) {
                    if (bgSessData != null) {
                        bgSessData.mBgNotificationType = flags;
                    } else {
                        // may be the update is happening from a foreground session,
                        // hence callback will be registered after the update call
                        bgSessData = new BgSessionData();
                        bgSessData.mBgNotificationType = flags;
                        mFlpUserData.mBgSessionMap.put(idVal, bgSessData);
                    }
                }
            }

            if (!mFlpUserData.mFlpSessionMap.containsKey(Long.valueOf(id))) {
                Log.e(TAG, "Invalid FlpSession id: " + id);
                return FLP_RESULT_ERROR;
            }

            int result;
            FlpSessionData flpSessionData = mFlpUserData.mFlpSessionMap.get(Long.valueOf(id));
            synchronized (sLocationSettingsLock) {
                flpSessionData.mId = id;
                flpSessionData.mFlags = flags;
                flpSessionData.mPeriodNs = MilliToNano(period_ms);
                flpSessionData.mDistanceIntervalMps = distance_interval_mps;
                flpSessionData.mTripDistanceM = trip_distance_m;
                flpSessionData.mPowerMode = power_mode;
                flpSessionData.mTbmMs = tbm_ms;

                if (!mIsLocationSettingsOn) {
                    result =  FLP_RESULT_SUCCESS;
                } else {
                    result =  mFlpIDLClient.updateSession(flpSessionData.mId,
                                                        flpSessionData.mFlags,
                                                        flpSessionData.mPeriodNs,
                                                        flpSessionData.mDistanceIntervalMps,
                                                        flpSessionData.mTripDistanceM,
                                                        flpSessionData.mPowerMode,
                                                        flpSessionData.mTbmMs);
                }
            }

            return result;
        }

        public int stopFlpSession(int id) {
            // Ensure mFlpUserData is from current user.
            // Calls to stop from destroyed Apps can arrive before onUserChange
            mFlpUserData = mDataPerPackageAndUser.getData();

            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): stopFlpSession(); id is " + id);
            }

            synchronized(sCallBacksLock) {
                mFlpUserData.mBgSessionMap.remove(Long.valueOf(id));
            }

            int result;
            Long idVal = Long.valueOf(id);
            if (!mFlpUserData.mFlpSessionMap.containsKey(idVal)) {
                Log.e(TAG, "Invalid FlpSession id: " + id);
                return FLP_RESULT_ERROR;
            }

            FlpSessionData flpSessionData = mFlpUserData.mFlpSessionMap.get(idVal);
            if (!flpSessionData.mIsRunning) {
                result = FLP_RESULT_SUCCESS;
            } else {
                result = mFlpIDLClient.stopSession(id);
                ArrayList<Pair<Integer, String>> sList = new ArrayList<Pair<Integer, String>>();
                sList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                                    flpSessionData.mCallingPackage));
                mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 0, 1, sList)
                            .sendToTarget();
            }

            mFlpUserData.mFlpSessionMap.remove(idVal);

            return result;
        }

        public int pullLocations(final ILocationCallback cb,
                                 final long sessionStartTime,
                                 final int id) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): pullLocations(), sessionStartTime is "
                           + sessionStartTime);
            }
            if (cb == null) {
                Log.e(TAG, "in IFlpService.Stub(): cb is null.");
                return FLP_RESULT_ERROR;
            }

            if (!mFlpUserData.mFlpSessionMap.containsKey(Long.valueOf(id))) {
                Log.e(TAG, "Invalid FlpSession id: " + id);
                return FLP_RESULT_ERROR;
            }

            int result;
            synchronized (sLocationSettingsLock) {
                if (!mIsLocationSettingsOn) {
                    result = FLP_RESULT_SUCCESS;
                } else {
                    synchronized (sCallBacksLock) {
                        // save the cb
                        mFlpUserData.mQueryCbQueue.add(
                                new Pair<ILocationCallback, Long>(cb, sessionStartTime));
                    }
                    result = mFlpIDLClient.getAllLocations(id);
                }
            }

            return result;
        }
    };

    /* Remote binder */
    private final ITestService.Stub mTestingBinder = new ITestService.Stub() {
        public void deleteAidingData(long flags) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): deleteAidingData(). Flags: " + flags);
            }
            mFlpIDLClient.deleteAidingData(flags);
        }

        public void updateXtraThrottle(boolean enabled) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): updateXtraThrottle(). enabled: " + enabled);
            }
            mFlpIDLClient.updateXtraThrottle(enabled);
        }

        public void registerMaxPowerChangeCallback(final IMaxPowerAllocatedCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): registerMaxPowerChangeCallback()");
            }
            if (cb != null) {
                if (mMaxPowerCallbacks != null) {
                    String packageName =
                            mContext.getPackageManager().getNameForUid(Binder.getCallingUid());
                    CbCookie cookie = new CbCookie(packageName);
                    mMaxPowerCallbacks.register(cb, cookie);
                    mFlpUserData.mMaxPowerCallbacksMap.put(cb, cookie);

                    mFlpIDLClient.getMaxPowerAllocatedInMw();
                } else {
                    Log.e(TAG, "mMaxPowerCallbacks is null");
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }

        public void unregisterMaxPowerChangeCallback(IMaxPowerAllocatedCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): unregisterMaxPowerChangeCallback()");
            }
            if (cb != null) {
                if (mMaxPowerCallbacks != null) {
                    mMaxPowerCallbacks.unregister(cb);
                } else {
                    Log.e(TAG, "mMaxPowerCallbacks is null");
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }
    };

    private void onLocationReport(Location[] locations, int reportTrigger, int batchingMode) {
        if (VERBOSE) {
            Log.v(TAG, "entering onLocationReport() reportTrigger is " + reportTrigger +
                       "; Batching Mode is " + batchingMode +
                       "; and the first timestamp is " + locations[0].getTime());
        }
        mIZatServiceCtx.diagLogBatchedFixes(locations);
        long elapsedTime = SystemClock.elapsedRealtimeNanos();
        long timeStamp = locations[locations.length - 1].getTime();
        //Populate the elapasedTime (nanosec) for each fix based on the elapsedTime of the last fix
        //and the timestamp (millisec) of each fix
        for (int i = 0; i < locations.length; ++i) {
            locations[i].setElapsedRealtimeNanos(elapsedTime -
                    (timeStamp - locations[i].getTime()) * 1000000L);
        }

        if (reportTrigger == LOCATION_REPORT_ON_FULL_INDICATION) {
            // Broadcast to all batching callbacks the new value.
            synchronized (sCallBacksLock) {
                int index = mCallbacksForBg.beginBroadcast();
                for (int i = 0; i < index; i++) {
                    try {
                        Long sessionId =
                                (Long) ((CbCookie)mCallbacksForBg.getBroadcastCookie(i)).id;
                        BgSessionData bgSessData = mFlpUserData.mBgSessionMap.get(sessionId);
                        if (bgSessData == null) {
                            Log.e(TAG, "No Background session data found for id:" + index);
                            continue;
                        }
                        long sessionStartTime = (long) bgSessData.mSessionStartTime;
                        if (sessionStartTime >
                                 locations[locations.length-1].getTime() + MAX_STARTTIME_DELTA_MS) {
                            if (VERBOSE) {
                                Log.v(TAG, "Future start time detected, returning whole batch");
                            }
                            sessionStartTime = locations[0].getTime();
                        }

                        if (VERBOSE) {
                            Log.v(TAG, "in the mCallbacksForBg loop : " + i +
                                "; timestamp is " + sessionStartTime +
                                "; notification Type is " + bgSessData.mBgNotificationType);
                        }
                        if (((bgSessData.mBgNotificationType == FLP_BG_NOTIFICATION_ROUTINE) &&
                            (batchingMode == BATCHING_MODE_ROUTINE)) ||
                            (bgSessData.mBgNotificationType == FLP_BG_NOTIFICATION_OUTDOOR_TRIP) &&
                            (batchingMode == BATCHING_MODE_OUTDOOR_TRIP)) {
                            if (sessionStartTime<=locations[0].getTime()) {
                                // return the whole batch
                                if (VERBOSE) {
                                    Log.v(TAG, "return whole batch");
                                }
                                mCallbacksForBg.getBroadcastItem(i).onLocationAvailable(locations);
                            } else if (sessionStartTime > locations[locations.length-1].getTime()) {
                                if (VERBOSE) {
                                    Log.v(TAG, "no need to return");
                                }
                            } else {
                                // find the offset
                                int offset = getOffset(sessionStartTime, locations);
                                Location[] newResult = new Location[locations.length - offset];
                                System.arraycopy(locations,
                                                 offset,
                                                 newResult,
                                                 0,
                                                 locations.length - offset);
                                mCallbacksForBg.getBroadcastItem(i).onLocationAvailable(newResult);
                            }
                        }
                    } catch (RemoteException e) {
                    // The RemoteCallbackList will take care of removing
                    // the dead object.
                    }
                }
                mCallbacksForBg.finishBroadcast();
            }
         } else if (reportTrigger == LOCATION_REPORT_ON_FIX_INDICATION) {
             HashMap<IBinder, String> sdkClients =
                    IzatService.AidlClientDeathNotifier.getInstance().getCallingPackage();
             boolean isLocationOffButEs = false;
             synchronized (sLocationSettingsLock) {
                 if (!mIsLocationSettingsOn && mIsUserEmergency) {
                     isLocationOffButEs = true;
                 }
             }

            // Broadcast to all tracking callbacks the new value.
            synchronized (sCallBacksLock) {
                int index = mCallbacksForFg.beginBroadcast();
                for (int i = 0; i < index; i++) {
                    try {
                        // When Location OFF & emergency, fix should report to QC Flp only
                        if (isLocationOffButEs) {
                            String pkgName =
                                    ((CbCookie)mCallbacksForFg.getBroadcastCookie(i)).packageName;
                            // Ignore SDK client's registered callback
                            if (sdkClients.containsValue(pkgName)) {
                                continue;
                            }
                        }
                        boolean isRobustLocationAllowed =
                            ((CbCookie)mCallbacksForFg.getBroadcastCookie(i))
                            .isRobustLocationAllowed;
                        long sessionStartTime =
                                ((CbCookie)mCallbacksForFg.getBroadcastCookie(i)).sessionStartTime;
                        // Handle case of sessionStartTime in the future (device missconfiguration?)
                        if (sessionStartTime >
                                 locations[locations.length-1].getTime() + MAX_STARTTIME_DELTA_MS) {
                            if (VERBOSE) {
                                Log.v(TAG, "Future start time detected, returning whole batch");
                            }
                            sessionStartTime = locations[0].getTime();
                        }
                        if (!isRobustLocationAllowed) {
                            for (int j = 0; j < locations.length; ++j) {
                                //Clear the robust location info if client has no permission
                                Bundle extra = locations[j].getExtras();
                                if (null != extra) {
                                    extra.remove("Conformity_index");
                                }
                            }
                        }
                        if (VERBOSE) {
                            Log.v(TAG, "in the mCallbacksForFg loop : " + i
                                       + "; cd timestamp is" + sessionStartTime);
                        }
                        if (sessionStartTime <= locations[0].getTime()) {
                            // return the whole batch
                            if (VERBOSE) {
                                Log.v(TAG, "return whole batch");
                            }
                            mCallbacksForFg.getBroadcastItem(i).onLocationAvailable(locations);
                        } else if (sessionStartTime>locations[locations.length-1].getTime()) {
                            if (VERBOSE) {
                                Log.v(TAG, "no need to return");
                            }
                        } else {
                            // find the offset
                            int offset = getOffset(sessionStartTime, locations);
                            Location[] newResult = new Location[locations.length - offset];
                            System.arraycopy(locations,
                                             offset,
                                             newResult,
                                             0,
                                             locations.length - offset);
                            mCallbacksForFg.getBroadcastItem(i).onLocationAvailable(newResult);
                        }
                    } catch (RemoteException e) {
                        // The RemoteCallbackList will take care of removing
                        // the dead object.
                    }
                }
                mCallbacksForFg.finishBroadcast();
            }
        } else if (reportTrigger == LOCATION_REPORT_ON_QUERY) {
            synchronized (sCallBacksLock) {
                if (!mFlpUserData.mQueryCbQueue.isEmpty()) {
                    Pair<ILocationCallback, Long> cbPair = mFlpUserData.mQueryCbQueue.remove();
                    if (cbPair != null) {
                        try {
                            // check the timestamp, find the offset
                            ILocationCallback callback = cbPair.first;
                            long sessionStartTime = cbPair.second;
                            if (VERBOSE) {
                                Log.v(TAG, "calling callback for" +
                                           " pulling, sessionStartTime is " +
                                            sessionStartTime);
                            }
                            if (sessionStartTime <= locations[0].getTime()) {
                                // return the whole batch
                                if (VERBOSE) {
                                    Log.v(TAG, "return whole batch");
                                }
                                callback.onLocationAvailable(locations);
                            } else if (sessionStartTime>locations[locations.length-1].getTime()) {
                                if (VERBOSE) {
                                    Log.v(TAG, "no need to return");
                                }
                            } else {
                                int offset = getOffset(sessionStartTime, locations);
                                Location[] newResult = new Location[locations.length - offset];
                                System.arraycopy(locations,
                                                 offset,
                                                 newResult,
                                                 0,
                                                 locations.length - offset);
                                callback.onLocationAvailable(newResult);
                            }
                            // update the timestamp of the callback
                            Long sessionId =
                                    Long.valueOf(mFlpUserData.mCallbacksForBgMap.get(callback).id);
                            BgSessionData bgSessData = mFlpUserData.mBgSessionMap.get(sessionId);
                            if (bgSessData != null) {
                                bgSessData.mSessionStartTime = System.currentTimeMillis();
                            }
                            if (mCallbacksForFg.unregister(callback)) {
                                CbCookie cookie =
                                        mFlpUserData.mCallbacksForFgMap.get(callback);
                                cookie.sessionStartTime = System.currentTimeMillis();
                                mCallbacksForFg.register(callback, cookie);
                                mFlpUserData.mCallbacksForFgMap.put(callback, cookie);
                            }
                            if (mCallbacksForPassive.unregister(callback)) {
                                CbCookie cookie =
                                        mFlpUserData.mCallbacksForPassiveMap.get(callback);
                                cookie.sessionStartTime = System.currentTimeMillis();
                                mCallbacksForPassive.register(callback, cookie);
                                mFlpUserData.mCallbacksForPassiveMap.put(callback, cookie);
                            }
                        } catch (RemoteException e) {
                            // The RemoteCallbackList will take care of removing
                            // the dead object.
                        }
                    }
                } else {
                    Log.e(TAG, "no available callback on query");
                }
            }
        } else if (reportTrigger == LOCATION_REPORT_ON_INDICATIONS) {
            /*
               For legacy behaviour, the callback are in the passive
               listeners already, so do nothing here.
            */
        } else {
            Log.e(TAG, "unknown reportTrigger");
        }

        // passive listeners
        if (mCallbacksForPassive.getRegisteredCallbackCount() > 0) {
            // Broadcast to all passive listeners
            synchronized (sCallBacksLock) {
                int index = mCallbacksForPassive.beginBroadcast();
                for (int i = 0; i < index; i++) {
                    try {
                        long sessionStartTime =
                                ((CbCookie)mCallbacksForPassive.getBroadcastCookie(i))
                                .sessionStartTime;
                        // Handle case of sessionStartTime in the future (device missconfiguration?)
                        if (sessionStartTime >
                                 locations[locations.length-1].getTime() + MAX_STARTTIME_DELTA_MS) {
                            if (VERBOSE) {
                                Log.v(TAG, "Future start time detected, returning whole batch");
                            }
                            sessionStartTime = locations[0].getTime();
                        }
                        if (VERBOSE) {
                            Log.v(TAG, "in the mCallbacksForPassive loop : " + i
                                       + "; cd timestamp is" + sessionStartTime);
                        }
                        if (sessionStartTime<=locations[0].getTime()) {
                            // return the whole batch
                            if (VERBOSE) {
                                Log.v(TAG, "return whole batch");
                            }
                            mCallbacksForPassive.getBroadcastItem(i).onLocationAvailable(locations);
                        } else if (sessionStartTime>locations[locations.length-1].getTime()) {
                            if (VERBOSE) {
                                Log.v(TAG, "no need to return");
                            }
                        } else {
                            // find the offset
                            int offset = getOffset(sessionStartTime, locations);
                            Location[] newResult = new Location[locations.length - offset];
                            System.arraycopy(locations,
                                             offset,
                                             newResult,
                                             0,
                                             locations.length - offset);
                            mCallbacksForPassive.getBroadcastItem(i).onLocationAvailable(newResult);
                        }
                    } catch (RemoteException e) {
                        // The RemoteCallbackList will take care of removing
                        // the dead object.
                    }
                }
                mCallbacksForPassive.finishBroadcast();
            }
        }
    }

    private void onBatchingStatusCb(int batchingStatus, int[] completedTripClientIds) {
        if (VERBOSE) {
            Log.d(TAG, "entering onBatchingStatusCb batchingStatus :" + batchingStatus);
        }
        synchronized(sStatusCallbacksLock) {
            List<Long> completedTripClientList = null;

            if (completedTripClientIds != null) {
                completedTripClientList = new ArrayList<Long>(completedTripClientIds.length);
                for (int index = 0; index < completedTripClientIds.length; index++) {
                    completedTripClientList.add(Long.valueOf(completedTripClientIds[index]));
                }
            }

            int broadcastCount = mCallbacksForStatus.beginBroadcast();
            for (int broadcastIndex = 0; broadcastIndex < broadcastCount; broadcastIndex++) {
                try {
                    if (batchingStatus != BATCHING_STATUS_TRIP_COMPLETED) {
                        mCallbacksForStatus.getBroadcastItem(broadcastIndex).onBatchingStatusCb(
                                batchingStatus);
                    } else {
                        Long sessionId =
                                (Long) ((CbCookie)mCallbacksForStatus
                                .getBroadcastCookie(broadcastIndex)).id;
                        if ((completedTripClientList != null) &&
                                (completedTripClientList.contains(sessionId))) {
                            mCallbacksForStatus.getBroadcastItem(broadcastIndex).onBatchingStatusCb(
                                    batchingStatus);
                            mFlpUserData.mFlpSessionMap.remove(sessionId);
                        }
                    }
                } catch(RemoteException e) {
                  // The RemoteCallbackList will take care of removing
                  // the dead object.
                }
            }
            mCallbacksForStatus.finishBroadcast();
        }
    }

    private void onMaxPowerAllocatedChanged(int power_mW) {
        if (VERBOSE) {
            Log.d(TAG, "entering onMaxPowerAllocatedChanged() power: " + power_mW);
        }
        // Broadcast to all clients the new value.
        int index = mMaxPowerCallbacks.beginBroadcast();
        for (int i = 0; i < index; i++) {
            if (VERBOSE) {
                Log.v(TAG, "in the mMaxPowerCallbacks loop : " + i);
            }
            try {
                IMaxPowerAllocatedCallback cb =
                    mMaxPowerCallbacks.getBroadcastItem(i);
                cb.onMaxPowerAllocatedChanged((double)power_mW);
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object.
            }
        }
        mMaxPowerCallbacks.finishBroadcast();
    }

    class UserChanged implements DataPerPackageAndUser.UserChangeListener<FlpUserData> {
        @Override
        public void onUserChange(Map<String, FlpUserData> prevUserData,
                                 Map<String, FlpUserData> currentUserData) {
            if (VERBOSE) {
                Log.d(TAG, "Active user has changed, updating FLP callbacks...");
            }

            synchronized (sLocationSettingsLock) {
                // New user switched may be first time running
                if (currentUserData.isEmpty()) {
                    Log.d(TAG, "FLP data for new user is empty, creating new instance.");
                    // mDataPerPackageAndUser.setData would dead-lock in this case
                    currentUserData.put(mDataPerPackageAndUser.getPackageName(null),
                            new FlpUserData());
                }
                FlpUserData currentFlpUserData =
                        currentUserData.get(mDataPerPackageAndUser.getPackageName(null)) ;

                FlpUserData prevFlpUserData =
                        prevUserData.get(mDataPerPackageAndUser.getPackageName(null)) ;
                // mFlpUserData may have been updated if start/stop was called before
                mFlpUserData = prevFlpUserData;

                synchronized (sCallBacksLock) {
                    // Unregister all previous callbacks
                    for (ILocationCallback callback : mFlpUserData.mCallbacksForFgMap.keySet()) {
                        mCallbacksForFg.unregister(callback);
                    }

                    for (ILocationCallback callback : mFlpUserData.mCallbacksForBgMap.keySet()) {
                        mCallbacksForBg.unregister(callback);
                    }

                    for (ILocationCallback callback :
                            mFlpUserData.mCallbacksForPassiveMap.keySet()) {
                        mCallbacksForPassive.unregister(callback);
                    }
                }

                synchronized (sStatusCallbacksLock) {
                    for (ISessionStatusCallback callback :
                            mFlpUserData.mCallbacksForStatusMap.keySet()) {
                        mCallbacksForStatus.unregister(callback);
                    }
                }

                for (IMaxPowerAllocatedCallback callback :
                        mFlpUserData.mMaxPowerCallbacksMap.keySet()) {
                    mMaxPowerCallbacks.unregister(callback);
                }

                // Remove any pending callback queue. Wont make sense any more after user switch
                mFlpUserData.mQueryCbQueue.clear();

                if (mIsLocationSettingsOn) {
                    stopFlpSessions();
                }

                // Update to new current user
                mFlpUserData = currentFlpUserData;

                if (mIsLocationSettingsOn) {
                    startFlpSessions();
                }

                synchronized (sCallBacksLock) {
                    // Re-register all previous callbacks
                    for (ILocationCallback callback : mFlpUserData.mCallbacksForFgMap.keySet()) {
                        mCallbacksForFg.register(callback,
                                mFlpUserData.mCallbacksForFgMap.get(callback));
                    }

                    for (ILocationCallback callback : mFlpUserData.mCallbacksForBgMap.keySet()) {
                        mCallbacksForBg.register(callback,
                                mFlpUserData.mCallbacksForBgMap.get(callback));
                    }

                    for (ILocationCallback callback :
                            mFlpUserData.mCallbacksForPassiveMap.keySet()) {
                        mCallbacksForPassive.register(callback,
                                mFlpUserData.mCallbacksForPassiveMap.get(callback));
                    }
                }

                synchronized (sStatusCallbacksLock) {
                    for (ISessionStatusCallback callback :
                            mFlpUserData.mCallbacksForStatusMap.keySet()) {
                        mCallbacksForStatus.register(callback,
                                mFlpUserData.mCallbacksForPassiveMap.get(callback));
                    }
                }

                for (IMaxPowerAllocatedCallback callback :
                        mFlpUserData.mMaxPowerCallbacksMap.keySet()) {
                    mMaxPowerCallbacks.register(callback,
                            mFlpUserData.mMaxPowerCallbacksMap.get(callback));
                }
            }
        }

    }

    private int getOffset (long sessionStartTime, Location[] locations) {
        int offset = -1, left = 0, right = locations.length-1;
        while(left <= right) {
            int mid = (left+right)/2;
            // found the exactly match
            if (sessionStartTime ==
                locations[mid].getTime()) {
                offset = mid;
                break;
            }
            if (sessionStartTime >
                locations[mid].getTime()) {
                left = mid + 1;
            }
            if (sessionStartTime <
                locations[mid].getTime()) {
                right = mid - 1;
            }
        }
        if (offset == -1) {
            offset = left;
        }

        if (VERBOSE) {
           Log.v(TAG, "offset : " + offset);
        }
        return offset;
    }

    public IFlpService getFlpBinder() {
        return mBinder;
    }

    public ITestService getFlpTestingBinder() {
        return mTestingBinder;
    }

    /**
     * Called from native code to report HIDL death
     */
    private void reportFLPServiceDied() {
        if (VERBOSE) {
            Log.d(TAG, "FLPServiceDied, restarting sessions...");
        }
        stopFlpSessions();
        if (mIsLocationSettingsOn) {
            startFlpSessions();
        }
    }



    //=============================================================
    //Java IDL client
    //=============================================================
    static class FlpServiceProviderIDLClient extends LocIDLClientBase
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "FlpIDLClient";
        public static final int FLP_SUCCESS = 0;
        public static final int FLP_ERROR = 1;
        public static final int LOCATION_REPORT_ON_FULL_INDICATION = 1;
        public static final int LOCATION_REPORT_ON_FIX_INDICATION = 2;
        public static final int BATCHING_MODE_NONE = 2;
        public static final int BATCH_STATUS_TRIP_COMPLETED = 0;
        private FlpIDLCallback mFlpIDLCallback = null;
        public vendor.qti.gnss.ILocAidlFlpService mFlpIface = null;

        @Override
        public void onServiceDied() {
            mFlpIface = null;
            getFlpServiceIface();
            try {
                if (mFlpIface != null) {
                    mFlpIface.init(mFlpIDLCallback);
                    mFlpIDLCallback.mFlpServiceProvider.reportFLPServiceDied();
                }
            } catch (RemoteException e) {
            }
        }

        private FlpServiceProviderIDLClient(FlpServiceProvider provider) {
            getFlpServiceIface();
            try {
                if (mFlpIface != null) {
                    registerServiceDiedCb(this);
                    mFlpIDLCallback = new FlpIDLCallback(provider);
                    mFlpIface.init(mFlpIDLCallback);
                }
            } catch (RemoteException e) {
            }
        }

        public void getFlpServiceIface() {
            if (null == mFlpIface) {
                try {
                    mFlpIface = getGnssAidlService().getExtensionLocAidlFlpService();
                } catch (RuntimeException e) {
                    Log.e(TAG, "Exception getting flp wrapper: " + e);
                    mFlpIface = null;
                } catch (RemoteException e) {
                }
            }
        }

        public int getAllSupportedFeatures() {
            int mask = -1;
            if (null == mFlpIface) {
                return mask;
            }
            IDLClientUtils.toIDLService(TAG);
            try {
                mask = mFlpIface.getAllSupportedFeatures();
            } catch (RemoteException e) {
            }
            return mask;
        }

        public int startSession(int id, int flags, long period_ns,
                int distance_interval_mps, long trip_distance_m, int power_mode, long tbm_ms) {
            int result = -1;
            if (null == mFlpIface) {
                return FLP_ERROR;
            }
            IDLClientUtils.toIDLService(TAG);
            try {
                result = mFlpIface.startFlpSession(id, flags, period_ns,
                        distance_interval_mps, (int)trip_distance_m, power_mode, (int)tbm_ms);
            } catch (RemoteException e) {
            }
            if (result != 0) {
                return FLP_ERROR;
            }
            return FLP_SUCCESS;
        }

        public int updateSession(int id, int flags, long period_ns,
                int distance_interval_mps, long trip_distance_m, int power_mode, long tbm_ms) {
            int result = -1;
            if (null == mFlpIface) {
                return FLP_ERROR;
            }
            IDLClientUtils.toIDLService(TAG);
            try {
                result = mFlpIface.updateFlpSession(id, flags, period_ns,
                        distance_interval_mps, (int)trip_distance_m, power_mode, (int)tbm_ms);
            } catch (RemoteException e) {
            }
            if (result != 0) {
                return FLP_ERROR;
            }
            return FLP_SUCCESS;
        }

        public int stopSession(int id) {
            int result = -1;
            if (null == mFlpIface) {
                return FLP_ERROR;
            }
            IDLClientUtils.toIDLService(TAG);
            try {
                result = mFlpIface.stopFlpSession(id);
            } catch (RemoteException e) {
            }
            if (result != 0) {
                return FLP_ERROR;
            }
            return FLP_SUCCESS;
        }

        public int getAllLocations(int id) {
            int result = -1;
            if (null == mFlpIface) {
                return FLP_ERROR;
            }
            IDLClientUtils.toIDLService(TAG);
            try {
                result = mFlpIface.getAllBatchedLocations(id);
            } catch (RemoteException e) {
            }
            if (result != 0) {
                return FLP_ERROR;
            }
            return FLP_SUCCESS;
        }

        public void deleteAidingData(long flags) {
            try {
                if (null != mFlpIface) {
                    IDLClientUtils.toIDLService(TAG);
                    mFlpIface.deleteAidingData(flags);
                }
            } catch (RemoteException e) {
            }
        }

        public void updateXtraThrottle(boolean enabled) {
            try {
                if (null != mFlpIface) {
                    IDLClientUtils.toIDLService(TAG);
                    mFlpIface.updateXtraThrottle(enabled);
                }
            } catch (RemoteException e) {
            }
        }

        public void getMaxPowerAllocatedInMw() {
            try {
                if (null != mFlpIface) {
                    IDLClientUtils.toIDLService(TAG);
                    mFlpIface.getMaxPowerAllocated();
                }
            } catch (RemoteException e) {
            }
        }
        //==============================================================
        // AIDL Callback class
        //==============================================================
        class FlpIDLCallback extends ILocAidlFlpServiceCallback.Stub {
            private FlpServiceProvider mFlpServiceProvider;

            public FlpIDLCallback(FlpServiceProvider provider) {
                mFlpServiceProvider = provider;
            }

            public void gnssBatchingStatusCb(vendor.qti.gnss.LocAidlBatchStatusInfo batchStatusInfo,
                    int[] listOfCompletedTrips) {
                IDLClientUtils.fromIDLService(TAG);
                int[] TripIds = null;
                if (batchStatusInfo.batchStatus == BATCH_STATUS_TRIP_COMPLETED) {
                    int listSize = listOfCompletedTrips.length;
                    TripIds = new int[listSize];
                    for (int i = 0; i < listSize; i++) {
                        TripIds[i] = listOfCompletedTrips[i];
                    }
                }
                mFlpServiceProvider.onBatchingStatusCb(batchStatusInfo.batchStatus, TripIds);
            }
            public void gnssLocationBatchingCb(vendor.qti.gnss.LocAidlBatchOptions batchOptions,
                    vendor.qti.gnss.LocAidlLocation[] locations) {
                IDLClientUtils.fromIDLService(TAG);
                Location[] locationList = new Location[locations.length];
                for (int i = 0; i < locations.length; i++) {
                    locationList[i] = IDLClientUtils.translateAidlLocation(locations[i]);
                }

                mFlpServiceProvider.onLocationReport(locationList,
                        LOCATION_REPORT_ON_FULL_INDICATION, batchOptions.batchMode);
            }
            public void gnssLocationTrackingCb(vendor.qti.gnss.LocAidlLocation location) {
                IDLClientUtils.fromIDLService(TAG);
                Location[] locationList = new Location[1];
                locationList[0] = IDLClientUtils.translateAidlLocation(location);
                Bundle extras = new Bundle();
                if ((location.locationFlagsMask &
                            LocAidlLocationFlagsBits.CONFORMITY_INDEX_BIT) != 0) {
                    extras.putFloat("Conformity_index", location.conformityIndex);
                }
                if ((location.locationFlagsMask &
                            LocAidlLocationFlagsBits.TECH_MASK_BIT) != 0) {
                    extras.putInt("Tech_mask", location.locationTechnologyMask);
                }
                if ((location.locationFlagsMask &
                            LocAidlLocationFlagsBits.QUALITY_TYPE_BIT) != 0) {
                    extras.putInt("Quality_type", location.qualityType);
                }
                // New locationTechnologyMask definition TBD to avoid HIDL bump at this point
                if ((location.locationTechnologyMask & (1<<11)) != 0) {
                    extras.putByte("Precise_position", (byte) 1);
                }
                if (extras.size() > 0) {
                    locationList[0].setExtras(extras);
                }
                mFlpServiceProvider.onLocationReport(locationList,
                        LOCATION_REPORT_ON_FIX_INDICATION, BATCHING_MODE_NONE);
            }
            public void gnssMaxPowerAllocatedCb(int powerInMW) {
                IDLClientUtils.fromIDLService(TAG);
                mFlpServiceProvider.onMaxPowerAllocatedChanged(powerInMW);
            }
            @Override
            public final int getInterfaceVersion() {
                return ILocAidlFlpServiceCallback.VERSION;
                }
            @Override
            public final String getInterfaceHash() {
                return ILocAidlFlpServiceCallback.HASH;
            }
        }
    }

    //=============================================================
    //Java HIDL client
    //=============================================================
    static class FlpServiceProviderHIDLClient extends FlpServiceProviderIDLClient {
        private FlpHidlCallback mFlpHidlCallback = null;
        public FlpHidlServiceWrapper mFlpHidlWrapper = null;

        @Override
        public void onServiceDied() {
            mFlpHidlWrapper = null;
            getFlpServiceIface();
            if (mFlpHidlWrapper != null) {
                    mFlpHidlWrapper.init((ILocHidlFlpServiceCallback.Stub)mFlpHidlCallback);
                    mFlpHidlCallback.mFlpServiceProvider.reportFLPServiceDied();
            }
        }

        private FlpServiceProviderHIDLClient(FlpServiceProvider provider) {
            super(provider);
            getFlpServiceIface();
            if (mFlpHidlWrapper != null) {
                registerServiceDiedCb(this);
                    mFlpHidlCallback = new FlpHidlCallback(provider);
                    mFlpHidlWrapper.init((ILocHidlFlpServiceCallback.Stub)mFlpHidlCallback);
            }
        }

        public void getFlpServiceIface() {
            if (null == mFlpHidlWrapper) {
                try {
                    mFlpHidlWrapper = FlpIDLServiceIfaceFactory.getWrapper(getIDLServiceVersion(),
                                getGnssService(), getGnssAidlService());
                } catch (RuntimeException e) {
                    Log.e(TAG, "Exception getting flp wrapper: " + e);
                    mFlpHidlWrapper = null;
                }
            }
        }

        public int getAllSupportedFeatures() {
            int mask = -1;
            if (null == mFlpHidlWrapper) {
                return mask;
            }
            IDLClientUtils.toIDLService(TAG);
            mask = mFlpHidlWrapper.getAllSupportedFeatures();
            return mask;
        }

        public int startSession(int id, int flags, long period_ns,
                int distance_interval_mps, long trip_distance_m, int power_mode, long tbm_ms) {
            int result = -1;
            if (null == mFlpHidlWrapper) {
                return FLP_ERROR;
            }
            IDLClientUtils.toIDLService(TAG);
            result = mFlpHidlWrapper.startSession(id, flags, period_ns,
                   distance_interval_mps, (int)trip_distance_m, power_mode, tbm_ms);
            if (result != 0) {
                return FLP_ERROR;
            }
            return FLP_SUCCESS;
        }

        public int updateSession(int id, int flags, long period_ns,
                int distance_interval_mps, long trip_distance_m, int power_mode, long tbm_ms) {
            int result = -1;
            if (null == mFlpHidlWrapper) {
                return FLP_ERROR;
            }
            IDLClientUtils.toIDLService(TAG);
            result = mFlpHidlWrapper.updateSession(id, flags, period_ns,
                    distance_interval_mps, (int)trip_distance_m, power_mode, tbm_ms);
            if (result != 0) {
                return FLP_ERROR;
            }
            return FLP_SUCCESS;
        }

        public int stopSession(int id) {
            int result = -1;
            if (null == mFlpHidlWrapper) {
                return FLP_ERROR;
            }
            IDLClientUtils.toIDLService(TAG);
            result = mFlpHidlWrapper.stopSession(id);
            if (result != 0) {
                return FLP_ERROR;
            }
            return FLP_SUCCESS;
        }

        public int getAllLocations(int id) {
            int result = -1;
            if (null == mFlpHidlWrapper) {
                return FLP_ERROR;
            }
            IDLClientUtils.toIDLService(TAG);
            result = mFlpHidlWrapper.getAllLocations(id);
            if (result != 0) {
                return FLP_ERROR;
            }
            return FLP_SUCCESS;
        }

        public void deleteAidingData(long flags) {
            if (null != mFlpHidlWrapper) {
                IDLClientUtils.toIDLService(TAG);
                mFlpHidlWrapper.deleteAidingData(flags);
            }
        }

        public void updateXtraThrottle(boolean enabled) {
            if (null != mFlpHidlWrapper) {
                IDLClientUtils.toIDLService(TAG);
                mFlpHidlWrapper.updateXtraThrottle(enabled);
            }
        }

        public void getMaxPowerAllocatedInMw() {
            if (null != mFlpHidlWrapper) {
                IDLClientUtils.toIDLService(TAG);
                mFlpHidlWrapper.getMaxPowerAllocatedInMw();
            }
        }
        //==================================================================
        //IDL wrapper factory
        //==================================================================
        static class FlpIDLServiceIfaceFactory {
            static FlpHidlServiceWrapper getWrapper(IDLServiceVersion ver,
                    vendor.qti.gnss.V1_0.ILocHidlGnss hidlSer,
                    vendor.qti.gnss.ILocAidlGnss aidlSer) {
                FlpHidlServiceWrapper instance = null;
                IDLServiceVersion hidlVer = ver;
                vendor.qti.gnss.V1_0.ILocHidlGnss service = hidlSer;
                if (null != service) {
                    try {
                        if (hidlVer.compareTo(IDLServiceVersion.V4_0) >= 0) {
                            instance = new FlpServiceProviderHIDLClient.FlpHidlServiceWrapper40(
                                    ((vendor.qti.gnss.V4_0.ILocHidlGnss)service)
                                    .getExtensionLocHidlFlpService_4_0());
                        } else if (hidlVer.compareTo(IDLServiceVersion.V2_1) >= 0) {
                            instance = new FlpServiceProviderHIDLClient.FlpHidlServiceWrapper21(
                                    ((vendor.qti.gnss.V2_1.ILocHidlGnss)service)
                                    .getExtensionLocHidlFlpService_2_1());
                        } else if (hidlVer.compareTo(IDLServiceVersion.V1_1) >= 0) {
                            instance = new FlpServiceProviderHIDLClient.FlpHidlServiceWrapper11(
                                    ((vendor.qti.gnss.V1_1.ILocHidlGnss)service)
                                    .getExtensionLocHidlFlpService_1_1());
                        } else if (hidlVer.compareTo(IDLServiceVersion.V1_0) >= 0) {
                            instance = new FlpServiceProviderHIDLClient.FlpHidlServiceWrapper(
                                    ((vendor.qti.gnss.V1_0.ILocHidlGnss)service)
                                    .getExtensionLocHidlFlpService());
                        }
                    } catch (RemoteException e) {
                    }
                } else {
                    throw new RuntimeException("gnssService is null!");
                }

                if (null == instance) {
                    throw new RuntimeException("flpService is null!");
                }
                return instance;
            }
        }

        //==============================================================
        //HIDL version 1.0 wrapper class
        //==============================================================
        static class FlpHidlServiceWrapper  {
            protected vendor.qti.gnss.V1_0.ILocHidlFlpService mFlpHidlServiceIface = null;

            public FlpHidlServiceWrapper(vendor.qti.gnss.V1_0.ILocHidlFlpService service) {
                mFlpHidlServiceIface = service;
            }

            public void init(ILocHidlFlpServiceCallback.Stub cb) {
                if (null != mFlpHidlServiceIface) {
                    try {
                        mFlpHidlServiceIface
                                .init((vendor.qti.gnss.V1_0.ILocHidlFlpServiceCallback)cb);
                    } catch (RemoteException e) {
                    }
                }
            }

            public int getAllSupportedFeatures() {
                try {
                    return mFlpHidlServiceIface.getAllSupportedFeatures();
                } catch (RemoteException e) {
                    return -1;
                }
            }

            public int startSession(int id, int flags, long period_ns, int distance_interval_mps,
                    long trip_distance_m, int power_mode, long tbm_ms) {
                try {
                    return mFlpHidlServiceIface.startFlpSession(id, flags, period_ns,
                            distance_interval_mps, (int)trip_distance_m);
                } catch (RemoteException e) {
                    return -1;
                }
            }

            public int updateSession(int id, int flags, long period_ns, int distance_interval_mps,
                    long trip_distance_m, int power_mode, long tbm_ms) {
                try {
                    return mFlpHidlServiceIface.updateFlpSession(id, flags, period_ns,
                            distance_interval_mps, (int)trip_distance_m);
                } catch (RemoteException e) {
                    return -1;
                }
            }

            public int stopSession(int id) {
                try {
                    return mFlpHidlServiceIface.stopFlpSession(id);
                } catch (RemoteException e) {
                    return -1;
                }
            }

            public int getAllLocations(int id) {
                try {
                    return mFlpHidlServiceIface.getAllBatchedLocations(id);
                } catch (RemoteException e) {
                    return -1;
                }
            }

            public void deleteAidingData(long flags) {
                try {
                    mFlpHidlServiceIface.deleteAidingData(flags);
                } catch (RemoteException e) {
                }
            }

            public void updateXtraThrottle(boolean enabled) {
                Log.d(TAG, "not supported");
            }

            public void getMaxPowerAllocatedInMw() {
                try {
                    mFlpHidlServiceIface.getMaxPowerAllocated();
                } catch (RemoteException e) {
                }
            }
        }

        //==============================================================
        //HIDL version 1.1 wrapper class
        //==============================================================
        static class FlpHidlServiceWrapper11 extends FlpHidlServiceWrapper {
            private FlpHidlServiceWrapper11(vendor.qti.gnss.V1_1.ILocHidlFlpService service) {
                super(service);
            }

            @Override
            public void updateXtraThrottle(boolean enabled) {
            }
        }

        //==============================================================
        //HIDL version 2.1 wrapper class
        //==============================================================
        static class FlpHidlServiceWrapper21 extends FlpHidlServiceWrapper11 {
            private FlpHidlServiceWrapper21(vendor.qti.gnss.V2_1.ILocHidlFlpService service) {
                super(service);
            }

            @Override
            public int startSession(int id, int flags, long period_ns, int distance_interval_mps,
                    long trip_distance_m, int power_mode, long tbm_ms) {
                try {
                    return ((vendor.qti.gnss.V2_1.ILocHidlFlpService)mFlpHidlServiceIface)
                            .startFlpSession_2_1(id, flags, period_ns, distance_interval_mps,
                            (int)trip_distance_m, power_mode, (int)tbm_ms);
                } catch (RemoteException e) {
                    return -1;
                }
            }

            @Override
            public int updateSession(int id, int flags, long period_ns, int distance_interval_mps,
                    long trip_distance_m, int power_mode, long tbm_ms) {
                try {
                    return ((vendor.qti.gnss.V2_1.ILocHidlFlpService)mFlpHidlServiceIface)
                            .updateFlpSession_2_1(id, flags, period_ns, distance_interval_mps,
                            (int)trip_distance_m, power_mode, (int)tbm_ms);
                } catch (RemoteException e) {
                    return -1;
                }
            }
        }

        //==============================================================
        //HIDL version 4.0 wrapper class
        //==============================================================
        static class FlpHidlServiceWrapper40 extends FlpHidlServiceWrapper21 {
            private FlpHidlServiceWrapper40(vendor.qti.gnss.V4_0.ILocHidlFlpService service) {
                super(service);
            }

            @Override
            public void init(ILocHidlFlpServiceCallback.Stub cb) {
                if (null != mFlpHidlServiceIface) {
                    try {
                        ((vendor.qti.gnss.V4_0.ILocHidlFlpService)mFlpHidlServiceIface)
                                .init_4_0(cb);
                    } catch (RemoteException e) {
                    }
                }
            }
        }

        //==============================================================
        //HIDL Callback class
        //==============================================================
        class FlpHidlCallback extends ILocHidlFlpServiceCallback.Stub {
            private FlpServiceProvider mFlpServiceProvider;
            private FlpIDLCallback mFlpIDLCb = null;

            public FlpHidlCallback(FlpServiceProvider provider) {
                mFlpServiceProvider = provider;
                mFlpIDLCb = new FlpIDLCallback(provider);
            }

            public void gnssLocationTrackingCb(LocHidlLocation location) {
                IDLClientUtils.fromIDLService(TAG);
                Location[] locationList = new Location[1];
                locationList[0] = IDLClientUtils.translateHidlLocation(location);
                mFlpServiceProvider.onLocationReport(locationList,
                        LOCATION_REPORT_ON_FIX_INDICATION, BATCHING_MODE_NONE);
            }

            public void gnssLocationBatchingCb(LocHidlBatchOptions batchOptions,
                    ArrayList<LocHidlLocation> locations) {
                vendor.qti.gnss.LocAidlBatchOptions aidlBatchOptions =
                        new vendor.qti.gnss.LocAidlBatchOptions();
                aidlBatchOptions.batchMode = batchOptions.batchMode;
                vendor.qti.gnss.LocAidlLocation[] aidlLocations =
                        new vendor.qti.gnss.LocAidlLocation[locations.size()];
                for (int i = 0; i < locations.size(); i++) {
                    aidlLocations[i] = IDLClientUtils.translateHidlLocToAidl(locations.get(i));
                }
                mFlpIDLCb.gnssLocationBatchingCb(aidlBatchOptions, aidlLocations);

            }

            public void gnssBatchingStatusCb(LocHidlBatchStatusInfo batchStatusInfo,
                    ArrayList<Integer> listOfCompletedTrips) {
                int[] TripIds = null;
                if (batchStatusInfo.batchStatus == BATCH_STATUS_TRIP_COMPLETED) {
                    int listSize = listOfCompletedTrips.size();
                    TripIds = new int[listSize];
                    for (int i = 0; i < listSize; i++) {
                        TripIds[i] = listOfCompletedTrips.get(i);
                    }
                }
                vendor.qti.gnss.LocAidlBatchStatusInfo aidlBatchInfo =
                        new vendor.qti.gnss.LocAidlBatchStatusInfo();
                aidlBatchInfo.batchStatus = batchStatusInfo.batchStatus;
                mFlpServiceProvider.onBatchingStatusCb(batchStatusInfo.batchStatus, TripIds);
                mFlpIDLCb.gnssBatchingStatusCb(aidlBatchInfo, TripIds);
            }

            public void gnssMaxPowerAllocatedCb(int powerInMW) {
                mFlpIDLCb.gnssMaxPowerAllocatedCb(powerInMW);
            }

            public void gnssLocationTrackingCb_4_0(vendor.qti.gnss.V4_0.LocHidlLocation location) {
                Log.d(TAG, "gnssLocationTrackingCb_4_0");
                mFlpIDLCb.gnssLocationTrackingCb(IDLClientUtils.translateHidlLoc40ToAidl(
                            location));
            }
        }

    }
}
