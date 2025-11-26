/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018, 2020-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.gnssconfig;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.Bundle;
import android.os.HwBinder;
import android.content.pm.PackageManager;
import android.content.SharedPreferences;

import android.util.Log;
import java.lang.IndexOutOfBoundsException;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;

import com.qti.gnssconfig.*;
import com.qualcomm.location.izat.*;
import com.qualcomm.location.utils.IZatServiceContext;

import vendor.qti.gnss.V4_0.ILocHidlGnssConfigService;
import vendor.qti.gnss.V4_0.ILocHidlGnssConfigService.NtripConnectionParams;
import vendor.qti.gnss.V4_0.ILocHidlGnssConfigServiceCallback;
import vendor.qti.gnss.V4_0.ILocHidlGnss;
import com.qualcomm.location.idlclient.LocIDLClientBase.*;
import com.qualcomm.location.idlclient.*;
import com.qualcomm.location.izat.IzatService;
import com.qualcomm.location.izat.IzatService.*;
import vendor.qti.gnss.ILocAidlGnss;
import vendor.qti.gnss.ILocAidlGnssConfigService;
import vendor.qti.gnss.ILocAidlGnssConfigServiceCallback;
import vendor.qti.gnss.LocAidlNtripConnectionParams;
import vendor.qti.gnss.LocAidlRobustLocationInfo;

public class GnssConfigService implements ISsrNotifier, ISystemEventListener {
    private static final String TAG = "GnssConfigService";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sCallBacksLock = new Object();
    private RemoteCallbackList<IGnssConfigCallback> mGnssConfigCallbacks
        = new RemoteCallbackList<IGnssConfigCallback>();
    private final Context mContext;
    private IZatServiceContext mIZatServiceCtx;
    private final UserConsentManager mUserConsentManager;

    private static final String ACCESS_SV_CONFIG_API =
            "com.qualcomm.qti.permission.ACCESS_SV_CONFIG_API";
    private static final String ACCESS_ROBUST_LOCATION =
            "com.qualcomm.qti.permission.ACCESS_ROBUST_LOCATION_API";
    private static final String ACCESS_PRECISE_LOCATION_API =
            "com.qualcomm.qti.permission.ACCESS_PRECISE_LOCATION_API";
    private static final String USER_CONSENT_PERMISSION =
            "com.qualcomm.permission.ACCESS_USER_CONSENT_API";

    private ConfigData mConfigData = new ConfigData();
    private class ClientGnssConfigData extends CallbackData {
        private IGnssConfigCallback mCallback;

        public ClientGnssConfigData(IGnssConfigCallback callback) {
            mCallback = callback;
            super.mCallback = callback;
        }
    }

    private DataPerPackageAndUser<ClientGnssConfigData> mDataPerPackageAndUser;

    private static GnssConfigService sInstance = null;
    public static GnssConfigService getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new GnssConfigService(ctx);
        }
        return sInstance;
    }
    private GnssConfigServiceIdlClient mRemoteServiceClient;
    private GnssConfigService(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "GnssConfigService construction");
        }

        mContext = ctx;
        mDataPerPackageAndUser = new DataPerPackageAndUser<ClientGnssConfigData>(mContext,
                new UserChanged());
        if (LocIDLClientBase.getIDLServiceVersion().compareTo(IDLServiceVersion.V_AIDL) >= 0) {
            mRemoteServiceClient = new GnssConfigServiceIdlClient(this);
        } else {
            mRemoteServiceClient = new GnssConfigServiceHidlClient(this);
        }
        IzatService.AidlClientDeathNotifier.getInstance().registerAidlClientDeathCb(this);
        mIZatServiceCtx = IZatServiceContext.getInstance(mContext);
        mUserConsentManager = UserConsentManager.getInstance(mContext);
    }

    @Override
    public void onAidlClientDied(String packageName, int pid, int uid) {
        Log.d(TAG, "aidl client crash: " + packageName);
        synchronized (sCallBacksLock) {
            ClientGnssConfigData clData =
                    mDataPerPackageAndUser.getDataByPkgName(packageName);

            if (null != clData) {
                if (VERBOSE) {
                    Log.d(TAG, "Package died: " + clData.mPackageName);
                }
                mGnssConfigCallbacks.unregister(clData.mCallback);
            }

        }
    }

    enum PrecisePositionOptInStatus {
        UNKNOWN, OFF, ON
    }
    enum RobustLocationStatus {
        UNKNOWN, OFF, LOW, HIGH
    }
    /* =================================================
     *   retrieve config when SSR or boot up
     *  ================================================*/
    private class ConfigData extends JSONizable {
        public RobustLocationStatus rlStatus = RobustLocationStatus.UNKNOWN;

        public PrecisePositionOptInStatus ppOptInStatus = PrecisePositionOptInStatus.UNKNOWN;
        public boolean ppEnabled = false;   // true when precise position param configured
        public String ppHostNameOrIP = "";
        public String ppMountPointName = "";
        public int ppPort = 0;
        public String ppUserName = "";
        public String ppPassword = "";
        public boolean ppRequiresInitialNMEA = false;
        public boolean ppUseSSL = false;
    }

    private void SaveConfigData() {
        SsrHandler.get().registerDataForSSREvents(mContext,
                GnssConfigService.class.getName(), mConfigData.toJSON());
    }

    public void restoreConfigData() {
        if (mConfigData.rlStatus != RobustLocationStatus.UNKNOWN) {
            // RobustLocationStatus <--> rlEnable,rlForE911Enable:
            // OFF <--> F, F;   LOW <--> T, F;   HIGH <--> T, T
            boolean rlEnable = (mConfigData.rlStatus != RobustLocationStatus.OFF);
            boolean rlForE911Enable = (mConfigData.rlStatus == RobustLocationStatus.HIGH);
            mRemoteServiceClient.setRobustLocationConfig(rlEnable, rlForE911Enable);
        }

        if (mConfigData.ppOptInStatus != PrecisePositionOptInStatus.UNKNOWN) {
            boolean ppOptIn = (mConfigData.ppOptInStatus == PrecisePositionOptInStatus.ON);
            mRemoteServiceClient.updateNtripGgaConsent(ppOptIn);
        }
        if (mIZatServiceCtx.isPreciseLocationSupported() && mConfigData.ppEnabled) {
            NtripConfigData data = new NtripConfigData();
            data.mHostNameOrIP = mConfigData.ppHostNameOrIP;
            data.mMountPointName = mConfigData.ppMountPointName;
            data.mPort = mConfigData.ppPort;
            data.mUserName = mConfigData.ppUserName;
            data.mPassword = mConfigData.ppPassword;
            data.mRequiresInitialNMEA = mConfigData.ppRequiresInitialNMEA;
            data.mUseSSL = mConfigData.ppUseSSL;

            mRemoteServiceClient.enablePPENtripStream(data);
        } else {
            mRemoteServiceClient.disablePPENtripStream();
        }

    }

    @Override
    public void bootupAndSsrNotifier(String jsonStr) {
        Log.d(TAG, "bootupAndSsrNotifier");
        mConfigData.fromJSON(jsonStr);
        restoreConfigData();
    }


    /* Remote binder */
    private final IGnssConfigService.Stub mBinder = new IGnssConfigService.Stub() {

        /**
         *
         * Register Callback
         *
         */
        public void registerCallback(final IGnssConfigCallback callback) {
            if (!(mContext.checkCallingPermission(ACCESS_SV_CONFIG_API) ==
                    PackageManager.PERMISSION_GRANTED ||
                    mContext.checkCallingPermission(ACCESS_ROBUST_LOCATION) ==
                    PackageManager.PERMISSION_GRANTED ||
                    mContext.checkCallingPermission(ACCESS_PRECISE_LOCATION_API) ==
                    PackageManager.PERMISSION_GRANTED)) {
                throw new SecurityException("Requires ACCESS_SV_CONFIG_API or" +
                        "ACCESS_ROBUST_LOCATION or ACCESS_PRECISE_LOCATION_API permission");
            }

            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }

            synchronized (sCallBacksLock) {
                if (VERBOSE) {
                    Log.d(TAG, "getGnssSvTypeConfig: " +
                            mDataPerPackageAndUser.getPackageName(null));
                }


                ClientGnssConfigData clData = mDataPerPackageAndUser.getData();
                if (null == clData) {
                    clData = new ClientGnssConfigData(callback);
                    mDataPerPackageAndUser.setData(clData);
                } else {
                    if (null != clData.mCallback) {
                        mGnssConfigCallbacks.unregister(clData.mCallback);
                    }
                    clData.mCallback = callback;
                }

                mGnssConfigCallbacks.register(callback);
            }
        }

        /**
         *
         * SV config
         *
         */
        public void getGnssSvTypeConfig() {
            if (mContext.checkCallingPermission(ACCESS_SV_CONFIG_API) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_SV_CONFIG_API permission");
            }

            if (VERBOSE) {
                Log.d(TAG, "getGnssSvTypeConfig: " + mDataPerPackageAndUser.getPackageName(null));
            }

            synchronized(sCallBacksLock) {

                mRemoteServiceClient.getGnssSvTypeConfig();
            }
        }

        public void setGnssSvTypeConfig(int[] disabledSvTypeArray) {
            if (mContext.checkCallingPermission(ACCESS_SV_CONFIG_API) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_SV_CONFIG_API permission");
            }

            synchronized (sCallBacksLock) {

                Log.d(TAG, "setGnssSvTypeConfig: arrLen: " + disabledSvTypeArray.length +
                           ", package: " + mDataPerPackageAndUser.getPackageName(null));

                mRemoteServiceClient.setGnssSvTypeConfig(disabledSvTypeArray);
            }
        }

        public void resetGnssSvTypeConfig() {
            if (mContext.checkCallingPermission(ACCESS_SV_CONFIG_API) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_SV_CONFIG_API permission");
            }

            synchronized(sCallBacksLock) {
                if (VERBOSE) {
                    Log.d(TAG, "resetGnssSvTypeConfig: " +
                                mDataPerPackageAndUser.getPackageName(null));
                }
                mRemoteServiceClient.resetGnssSvTypeConfig();
            }
        }

        /**
         *
         * Robust location config
         *
         */
        public void getRobustLocationConfig() {
            if (mContext.checkCallingPermission(ACCESS_ROBUST_LOCATION) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_ROBUST_LOCATION_API permission");
            }
            if (IzatService.isDeviceOEMUnlocked(mContext)) {
                throw new SecurityException(
                        "Robust location only supported on bootloader locked device!");
            }

            synchronized(sCallBacksLock) {
                if (VERBOSE) {
                    Log.d(TAG, "getRobustLocationConfig: " +
                                mDataPerPackageAndUser.getPackageName(null));
                }
                mRemoteServiceClient.getRobustLocationConfig();
            }
        }

        public void setRobustLocationConfig(boolean enable, boolean enableForE911) {
            if (mContext.checkCallingPermission(ACCESS_ROBUST_LOCATION) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_ROBUST_LOCATION_API permission");
            }
            if (IzatService.isDeviceOEMUnlocked(mContext)) {
                throw new SecurityException(
                        "Robust location only supported on bootloader locked device!");
            }

            synchronized(sCallBacksLock) {
                if (VERBOSE) {
                    Log.d(TAG, "setRobustLocationConfig: " +
                                mDataPerPackageAndUser.getPackageName(null));
                }
                mRemoteServiceClient.setRobustLocationConfig(enable, enableForE911);
            }
            if (enableForE911) { // enableForE911 == true implies enable == true
                mConfigData.rlStatus = RobustLocationStatus.HIGH;
            } else if (enable) {
                mConfigData.rlStatus = RobustLocationStatus.LOW;
            } else { // !enable && !enableForE911
                mConfigData.rlStatus = RobustLocationStatus.OFF;
            }
            SaveConfigData();
        }

        /**
         *
         * Precise location config
         *
         */
        public void enablePreciseLocation(NtripConfigData data) {
            if (mContext.checkCallingPermission(ACCESS_PRECISE_LOCATION_API) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_PRECISE_LOCATION_API permission");
            }
            if (!mIZatServiceCtx.isPreciseLocationSupported()) {
                Log.e(TAG, "Izat Precise Location is not supported on this device.");
                throw new RuntimeException(
                        "Izat Precise Location is not supported on this device.");
            }
            mConfigData.ppEnabled = true;
            mConfigData.ppHostNameOrIP = data.mHostNameOrIP;
            mConfigData.ppMountPointName = data.mMountPointName;
            mConfigData.ppPort = data.mPort;
            mConfigData.ppUserName = data.mUserName;
            mConfigData.ppPassword = data.mPassword;
            mConfigData.ppRequiresInitialNMEA = data.mRequiresInitialNMEA;
            mConfigData.ppUseSSL = data.mUseSSL;

            SaveConfigData();
            mRemoteServiceClient.enablePPENtripStream(data);
        }

        public void disablePreciseLocation() {
            if (mContext.checkCallingPermission(ACCESS_PRECISE_LOCATION_API) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_PRECISE_LOCATION_API permission");
            }

            mConfigData.ppEnabled = true;
            mConfigData.ppHostNameOrIP = null;
            mConfigData.ppMountPointName = null;
            mConfigData.ppPort = 0;
            mConfigData.ppUserName = null;
            mConfigData.ppPassword = null;
            mConfigData.ppRequiresInitialNMEA = false;
            mConfigData.ppUseSSL = false;

            SaveConfigData();
            mRemoteServiceClient.disablePPENtripStream();
        }

        public void updateNtripGgaConsent(boolean optIn) {
            if (mContext.checkCallingPermission(ACCESS_PRECISE_LOCATION_API) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_PRECISE_LOCATION_API permission");
            }
            Log.d(TAG, "EDGNSS updateNtripGgaConsent: " + optIn);

            mConfigData.ppOptInStatus = optIn? PrecisePositionOptInStatus.ON:
                    PrecisePositionOptInStatus.OFF;
            SaveConfigData();
            mRemoteServiceClient.updateNtripGgaConsent(optIn);
        }

        @Override
        public void setNetworkLocationUserConsent(boolean b) {
            Log.d(TAG, "setNetworkLocationUserConsent: " + b);
            if (mContext.checkCallingPermission(USER_CONSENT_PERMISSION) !=
                    PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Requires ACCESS_USER_CONSENT_API");
            } else {
                mUserConsentManager.setAospUserConsent(b);
            }
        }

    };

    private void onGnssSvTypeConfigCb(int[] disabledSvTypeArray) {

        synchronized (sCallBacksLock) {

            Log.d(TAG, "onGnssSvTypeConfigCb: " + mDataPerPackageAndUser.getPackageName(null));

            for (ClientGnssConfigData clData : mDataPerPackageAndUser.getAllData()) {
                if (VERBOSE) {
                    Log.d(TAG, "Invoking cb for client: " + clData.mPackageName);
                }
                try {
                    clData.mCallback.getGnssSvTypeConfigCb(disabledSvTypeArray);
                } catch (RemoteException e) {
                    Log.e(TAG, "onGnssSvTypeConfigCb: Failed to invoke cb");
                }
            }
        }
    }

    private void onRobustLocationConfigCb(RLConfigData rlConfigData) {

        synchronized (sCallBacksLock) {

            Log.d(TAG, "onRobustLocationConfigCb: " + mDataPerPackageAndUser.getPackageName(null));

            for (ClientGnssConfigData clData : mDataPerPackageAndUser.getAllData()) {
                if (VERBOSE) {
                    Log.d(TAG, "Invoking cb for client: " + clData.mPackageName);
                }
                try {
                    clData.mCallback.getRobustLocationConfigCb(rlConfigData);
                } catch (RemoteException e) {
                    Log.e(TAG, "onRobustLocationConfigCb: Failed to invoke cb");
                }
            }
        }
    }

    class UserChanged implements DataPerPackageAndUser.UserChangeListener<ClientGnssConfigData> {
        @Override
        public void onUserChange(Map<String, ClientGnssConfigData> prevUserData,
                                 Map<String, ClientGnssConfigData> currentUserData) {
            if (VERBOSE) {
                Log.d(TAG, "Active user has changed, updating gnssConfig callbacks...");
            }

            synchronized (sCallBacksLock) {
                // Remove prevUser callbacks
                for (ClientGnssConfigData gnssConfigData: prevUserData.values()) {
                    mGnssConfigCallbacks.unregister(gnssConfigData.mCallback);
                }

                // Add back current user callbacks
                for (ClientGnssConfigData gnssConfigData: currentUserData.values()) {
                    mGnssConfigCallbacks.register(gnssConfigData.mCallback);
                }
            }
        }
    }

    public IGnssConfigService getGnssConfigBinder() {
        return mBinder;
    }

    /* =================================================
     *   HIDL Client
     * =================================================*/
    static private class GnssConfigServiceHidlClient extends GnssConfigServiceIdlClient
            implements LocIDLClientBase.IServiceDeathCb {
        private static final int GPS_SV_TYPE = 1;
        private static final int IRNSS_SV_TYPE = 7;
        private gnssConfigServiceWrapper mGnssConfigServiceIface;
        private GnssConfigServiceCallback mGnssCfgServiceCallback;
        private GnssConfigService mService;

        private GnssConfigServiceHidlClient(GnssConfigService service) {
            super(service);
            mGnssConfigServiceIface = gnssConfigServiceWrapperFactory.getWrapper(
                    getIDLServiceVersion(), getGnssService());
            mGnssCfgServiceCallback = new GnssConfigServiceCallback();
            mService = service;
            if (null != mGnssConfigServiceIface) {
                boolean ret = mGnssConfigServiceIface.init(mGnssCfgServiceCallback);
                if (!ret) {
                    Log.e(TAG, "Failed to init GnssConfigService");
                }
                registerServiceDiedCb(this);
            }
        }

        private int validGnssConstellationTypeCount(ArrayList<Byte> disabledSvTypeList) {
            int validTypes = 0;
            for (Byte type: disabledSvTypeList) {
                if ((type >= GPS_SV_TYPE && type <= IRNSS_SV_TYPE) ||
                        (~type >= GPS_SV_TYPE && ~type <= IRNSS_SV_TYPE)) {
                    ++validTypes;
                }
            }
            return validTypes;
        }

        @Override
        public void onServiceDied() {
            Log.e(TAG, "ILocHidlGnssConfigService died");
            mGnssConfigServiceIface = null;
            mGnssConfigServiceIface = gnssConfigServiceWrapperFactory.getWrapper(
                    getIDLServiceVersion(), getGnssService());
            if (null != mGnssConfigServiceIface) {
                boolean ret = mGnssConfigServiceIface.init(mGnssCfgServiceCallback);
            }
            mService.restoreConfigData();
        }
        @Override
        public void getGnssSvTypeConfig() {
            IDLClientUtils.toIDLService(TAG);
            if (null != mGnssConfigServiceIface) {
                mGnssConfigServiceIface.getGnssSvTypeConfig();
            }
        }
        @Override
        public void setGnssSvTypeConfig(int[] disabledSvTypeArray) {
            IDLClientUtils.toIDLService(TAG);
            ArrayList<Byte> list = new ArrayList<>();
            for (int svType: disabledSvTypeArray) {
                if (svType >= GPS_SV_TYPE && svType <= IRNSS_SV_TYPE) {
                    // GnssConstellationType, GPS: 1, NAVIC: 7
                    list.add((byte)svType);
                } else {
                    byte enableType = (byte)~svType;
                    if (enableType >= GPS_SV_TYPE && enableType <= IRNSS_SV_TYPE) {
                        // Enabled SV Types are pushed as negative numbers in the disabled list
                        list.add((byte)svType);
                    } else {
                        Log.e(TAG, "Invalid sv type: " + svType);
                    }
                }
            }
            if (null != mGnssConfigServiceIface) {
                mGnssConfigServiceIface.setGnssSvTypeConfig(list);
            }
        }
        @Override
        public void resetGnssSvTypeConfig() {
            IDLClientUtils.toIDLService(TAG);
            if (null != mGnssConfigServiceIface) {
                mGnssConfigServiceIface.resetGnssSvTypeConfig();
            }
        }
        @Override
        public void getRobustLocationConfig() {
            IDLClientUtils.toIDLService(TAG);
            if (null != mGnssConfigServiceIface) {
                mGnssConfigServiceIface.getRobustLocationConfig();
            }
        }
        @Override
        public void setRobustLocationConfig(boolean enable, boolean enableForE911) {
            IDLClientUtils.toIDLService(TAG);
            if (null != mGnssConfigServiceIface) {
                mGnssConfigServiceIface.setRobustLocationConfig(enable, enableForE911);
            }
        }
        @Override
        public void updateNtripGgaConsent(boolean consentAccepted) {
            IDLClientUtils.toIDLService(TAG);
            if (null != mGnssConfigServiceIface) {
                mGnssConfigServiceIface.updateNtripGgaConsent(consentAccepted);
            }
        }
        @Override
        public void enablePPENtripStream(NtripConfigData data) {
            IDLClientUtils.toIDLService(TAG);
            if (null != mGnssConfigServiceIface) {
                mGnssConfigServiceIface.enablePPENtripStream(data);
            }
        }
        @Override
        public void disablePPENtripStream() {
            IDLClientUtils.toIDLService(TAG);
            if (null != mGnssConfigServiceIface) {
                mGnssConfigServiceIface.disablePPENtripStream();
            }
        }

        /* =================================================
        *   HIDL Callback
        * =================================================*/
        private class GnssConfigServiceCallback extends ILocHidlGnssConfigServiceCallback.Stub {

            public GnssConfigServiceCallback() {
            }
            @Override
            public void getGnssSvTypeConfigCb(ArrayList<Byte> disabledSvTypeList)
                    throws android.os.RemoteException {
                int validCount = validGnssConstellationTypeCount(disabledSvTypeList);
                IDLClientUtils.fromIDLService(TAG);
                int[] disabledSvTypeArray = new int[validCount];
                int arrIdx = 0;
                for (Byte sv: disabledSvTypeList) {
                    if (sv >= GPS_SV_TYPE && sv <= IRNSS_SV_TYPE) {
                        Log.v(TAG, "Disabled SV type: " + sv);
                        disabledSvTypeArray[arrIdx++] = sv;
                    } else {
                        byte enabledSv = (byte)~sv;
                        if (enabledSv >= GPS_SV_TYPE && enabledSv <= IRNSS_SV_TYPE) {
                            Log.v(TAG, "Enabled SV type: " + enabledSv);
                            disabledSvTypeArray[arrIdx++] = sv;
                        } else {
                            Log.e(TAG, "Invalid sv type: " + sv);
                        }
                    }
                }

                mService.onGnssSvTypeConfigCb(disabledSvTypeArray);
                return;
            }

            public void getGnssSvTypeConfigCb_4_0(ArrayList<Byte> disabledSvTypeList)
                    throws android.os.RemoteException {
                int validCount = validGnssConstellationTypeCount(disabledSvTypeList);
                IDLClientUtils.fromIDLService(TAG);
                int[] disabledSvTypeArray = new int[validCount];
                int arrIdx = 0;
                for (Byte sv: disabledSvTypeList) {
                    if (sv >= GPS_SV_TYPE && sv <= IRNSS_SV_TYPE) {
                        Log.v(TAG, "Disabled SV type: " + sv);
                        disabledSvTypeArray[arrIdx++] = sv;
                    } else {
                        byte enabledSv = (byte)~sv;
                        if (enabledSv >= GPS_SV_TYPE && enabledSv <= IRNSS_SV_TYPE) {
                            Log.v(TAG, "Enabled SV type: " + enabledSv);
                            disabledSvTypeArray[arrIdx++] = sv;
                        } else {
                            Log.e(TAG, "Invalid sv type: " + sv);
                        }
                    }
                }

                mService.onGnssSvTypeConfigCb(disabledSvTypeArray);
                return;
            }

            public void getRobustLocationConfigCb(
                    ILocHidlGnssConfigServiceCallback.RobustLocationInfo info) {
                RLConfigData rlConfigData = new RLConfigData();
                rlConfigData.validMask = info.validMask;
                rlConfigData.enableStatus = info.enable;
                rlConfigData.enableStatusForE911 = info.enableForE911;
                rlConfigData.major = info.major;
                rlConfigData.minor = info.minor;

                mService.onRobustLocationConfigCb(rlConfigData);
            }
        }

        /* =================================================
        *   HIDL wrapper factory
        * =================================================*/
        static class gnssConfigServiceWrapperFactory {
            static gnssConfigServiceWrapper getWrapper(IDLServiceVersion idlVer,
                    vendor.qti.gnss.V1_0.ILocHidlGnss service) {
                IDLServiceVersion hidlVer = getIDLServiceVersion();
                gnssConfigServiceWrapper instance = null;
                if (null != service) {
                    try {
                        if (idlVer.compareTo(IDLServiceVersion.V4_0) >= 0) {
                            instance = new gnssConfigServiceWrapper40(
                                    ((vendor.qti.gnss.V4_0.ILocHidlGnss)service)
                                    .getExtensionLocHidlGnssConfigService_4_0());
                        } else if (idlVer.compareTo(IDLServiceVersion.V2_1) >= 0) {
                            instance = new gnssConfigServiceWrapper(
                                    ((vendor.qti.gnss.V2_1.ILocHidlGnss)service)
                                    .getExtensionLocHidlGnssConfigService_2_1());
                        }
                    } catch (RemoteException e) {
                        Log.d(TAG, "RemoteException: " + e);
                    }
                } else {
                    throw new RuntimeException("gnssService is null!");
                }

                if (null == instance) {
                    throw new RuntimeException("gnssConfigService is null!");
                }
                return instance;
            }
        }

        /* =================================================
        *   HIDL version 2.1 wrapper class
        * =================================================*/
        static class gnssConfigServiceWrapper {
            protected vendor.qti.gnss.V2_1.ILocHidlGnssConfigService mIface = null;

            public gnssConfigServiceWrapper(vendor.qti.gnss.V2_1.ILocHidlGnssConfigService service) {
                mIface = service;
            }

            public boolean init(GnssConfigServiceCallback cb) {
                boolean ret = false;
                try {
                    ret = mIface.init((vendor.qti.gnss.V2_1.ILocHidlGnssConfigServiceCallback)cb);
                } catch (RemoteException e) {
                }
                return ret;
            }

            public void getGnssSvTypeConfig() {
                try {
                    mIface.getGnssSvTypeConfig();
                } catch (RemoteException e) {
                }
            }

            public void setGnssSvTypeConfig(ArrayList<Byte> disabledSvTypeArray) {
                try {
                    mIface.setGnssSvTypeConfig(disabledSvTypeArray);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception setGnssSvTypeConfig: " + e);
                }
            }

            public void resetGnssSvTypeConfig() {
                try {
                    mIface.resetGnssSvTypeConfig();
                } catch (RemoteException e) {
                }
            }

            public void getRobustLocationConfig() {
                Log.e(TAG, "getRobustLocationConfig not supported in 2.1 API");
            }

            public void setRobustLocationConfig(boolean enable, boolean enableForE911) {
                Log.e(TAG, "configRobustLocation not supported in 2.1 API");
            }

            public void updateNtripGgaConsent(boolean consentAccepted) {
                Log.e(TAG, "getRobustLocationConfig not supported in 2.1 API");
            }

            public void enablePPENtripStream(NtripConfigData data) {
                Log.e(TAG, "enablePPENtripStream not supported in 2.1 API");
            }

            public void disablePPENtripStream() {
                Log.e(TAG, "disablePPENtripStream not supported in 2.1 API");
            }
        }

        /* =================================================
        *   HIDL version 4.0 wrapper class
        * =================================================*/
        static class gnssConfigServiceWrapper40 extends gnssConfigServiceWrapper {
            public gnssConfigServiceWrapper40(
                    vendor.qti.gnss.V4_0.ILocHidlGnssConfigService service) {
                super(service);
            }

            public boolean init(GnssConfigServiceCallback cb) {
                boolean ret = false;
                try {
                    ret = ((vendor.qti.gnss.V4_0.ILocHidlGnssConfigService)mIface).init_4_0(cb);
                } catch (RemoteException e) {
                }
                return ret;
            }

            public void setGnssSvTypeConfig(ArrayList<Byte> disabledSvTypeArray) {
                try {
                    ((vendor.qti.gnss.V4_0.ILocHidlGnssConfigService)mIface)
                            .setGnssSvTypeConfig_4_0(disabledSvTypeArray);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception setGnssSvTypeConfig: " + e);
                }
            }

            public void getRobustLocationConfig() {
                try {
                    ((vendor.qti.gnss.V4_0.ILocHidlGnssConfigService)mIface)
                            .getRobustLocationConfig();
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception setGnssSvTypeConfig: " + e);
                }
            }

            public void setRobustLocationConfig(boolean enable, boolean enableForE911) {
                try {
                    ((vendor.qti.gnss.V4_0.ILocHidlGnssConfigService)mIface)
                            .setRobustLocationConfig(enable, enableForE911);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception setGnssSvTypeConfig: " + e);
                }
            }

            public void updateNtripGgaConsent(boolean consentAccepted) {
                try {
                    Log.d(TAG, "EDGNSS updateNtripGgaConsent: " + consentAccepted);
                    ((vendor.qti.gnss.V4_0.ILocHidlGnssConfigService)mIface)
                            .updateNTRIPGGAConsent(consentAccepted);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception updateNtripGgaConsent: " + e);
                }
            }

            public void enablePPENtripStream(NtripConfigData data) {
                try {
                    NtripConnectionParams params = new NtripConnectionParams();
                    params.requiresNmeaLocation = data.mRequiresInitialNMEA;
                    params.hostNameOrIp = data.mHostNameOrIP;
                    params.mountPoint = data.mMountPointName;
                    params.username = data.mUserName;
                    params.password = data.mPassword;
                    params.port = data.mPort;

                    ((vendor.qti.gnss.V4_0.ILocHidlGnssConfigService)mIface)
                            .enablePPENtripStream(params, false);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception updateNtripGgaConsent: " + e);
                }
            }

            public void disablePPENtripStream() {
                try {
                    ((vendor.qti.gnss.V4_0.ILocHidlGnssConfigService)mIface)
                            .disablePPENtripStream();
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception disablePPENtripStream: " + e);
                }
            }
        }
    }

    /* =================================================
     *   AIDL Client
     * =================================================*/
    static private class GnssConfigServiceIdlClient extends LocIDLClientBase
            implements LocIDLClientBase.IServiceDeathCb {
        private static final int GPS_SV_TYPE = 1;
        private static final int IRNSS_SV_TYPE = 7;
        private GnssConfigServiceCallback mGnssCfgServiceCallback;
        private ILocAidlGnssConfigService mGnssConfigServiceIface;
        private GnssConfigService mService;

        private GnssConfigServiceIdlClient(GnssConfigService service) {
            getGnssConfigServiceIface();
            mGnssCfgServiceCallback = new GnssConfigServiceCallback();
            mService = service;
            try {
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.init(mGnssCfgServiceCallback);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception on GnssConfigService init: " + e);
            }
            registerServiceDiedCb(this);
        }

        private void getGnssConfigServiceIface() {
            if (null == mGnssConfigServiceIface) {
                ILocAidlGnss gnssService = (ILocAidlGnss) getGnssAidlService();
                if (null != gnssService) {
                    try {
                        mGnssConfigServiceIface = gnssService.getExtensionLocAidlGnssConfigService();
                    } catch (RemoteException e) {
                        Log.e(TAG, "Exception getting gnss config service: " + e);
                        mGnssConfigServiceIface = null;
                    }
                }
            }
        }

        private int validGnssConstellationTypeCount(byte[] disabledSvTypeList) {
            int validTypes = 0;
            for (byte type: disabledSvTypeList) {
                if ((type >= GPS_SV_TYPE && type <= IRNSS_SV_TYPE) ||
                        (~type >= GPS_SV_TYPE && ~type <= IRNSS_SV_TYPE)) {
                    ++validTypes;
                }
            }
            return validTypes;
        }

        @Override
        public void onServiceDied() {
            Log.e(TAG, "ILocAidlGnssConfigService died");
            mGnssConfigServiceIface = null;
            getGnssConfigServiceIface();
            try {
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.init(mGnssCfgServiceCallback);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception on GnssConfig Service init: " + e);
            }
            mService.restoreConfigData();
        }

        public void getGnssSvTypeConfig() {
            IDLClientUtils.toIDLService(TAG);
            try {
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.getGnssSvTypeConfig();
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception on getGnssSvTypeConfig: " + e);
            }
        }

        public void setGnssSvTypeConfig(int[] disabledSvTypeArray) {
            IDLClientUtils.toIDLService(TAG);
            byte[] list = new byte[disabledSvTypeArray.length];
            for (int i = 0; i < disabledSvTypeArray.length; i++) {
                int svType = disabledSvTypeArray[i];
                if (svType >= GPS_SV_TYPE && svType <= IRNSS_SV_TYPE) {
                    // GnssConstellationType, GPS: 1, NAVIC: 7
                    list[i] = (byte)svType;
                } else {
                    byte enableType = (byte)~svType;
                    if (enableType >= GPS_SV_TYPE && enableType <= IRNSS_SV_TYPE) {
                        // Enabled SV Types are pushed as negative numbers in the disabled list
                        list[i] = (byte)svType;
                    } else {
                        Log.e(TAG, "Invalid sv type: " + svType);
                    }
                }
            }
            try {
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.setGnssSvTypeConfig(list);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception on setGnssSvTypeConfig: " + e);
            }
        }

        public void resetGnssSvTypeConfig() {
            IDLClientUtils.toIDLService(TAG);
            try {
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.resetGnssSvTypeConfig();
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception resetGnssSvTypeConfig: " + e);
            }
        }

        public void getRobustLocationConfig() {
            try {
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.getRobustLocationConfig();
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception getRobustLocationConfig: " + e);
            }
        }

        public void setRobustLocationConfig(boolean enable, boolean enableForE911) {
            IDLClientUtils.toIDLService(TAG);
            try {
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.setRobustLocationConfig(enable, enableForE911);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception setRobustLocationConfig: " + e);
            }
        }

        public void updateNtripGgaConsent(boolean consentAccepted) {
            try {
                Log.d(TAG, "EDGNSS updateNtripGgaConsent: " + consentAccepted);
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.updateNTRIPGGAConsent(consentAccepted);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception updateNtripGgaConsent: " + e);
            }
        }

        public void enablePPENtripStream(NtripConfigData data) {
            try {
                LocAidlNtripConnectionParams params = new LocAidlNtripConnectionParams();
                params.requiresNmeaLocation = data.mRequiresInitialNMEA;
                params.hostNameOrIp = data.mHostNameOrIP;
                params.mountPoint = data.mMountPointName;
                params.username = data.mUserName;
                params.password = data.mPassword;
                params.port = data.mPort;
                params.useSSL = data.mUseSSL;
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.enablePPENtripStream(params, false);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception enablePPENtripStream: " + e);
            }
        }

        public void disablePPENtripStream() {
            IDLClientUtils.toIDLService(TAG);
            try {
                if (null != mGnssConfigServiceIface) {
                    mGnssConfigServiceIface.disablePPENtripStream();
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception disablePPENtripStream: " + e);
            }
        }

        /* =================================================
        *   AIDL Callback
        * =================================================*/
        private class GnssConfigServiceCallback extends ILocAidlGnssConfigServiceCallback.Stub {

            public GnssConfigServiceCallback() {
            }
            @Override
            public void getGnssSvTypeConfigCb(byte[] disabledSvTypeList)
                    throws android.os.RemoteException {
                int validCount = validGnssConstellationTypeCount(disabledSvTypeList);
                IDLClientUtils.fromIDLService(TAG);
                int[] disabledSvTypeArray = new int[validCount];
                int arrIdx = 0;
                for (Byte sv: disabledSvTypeList) {
                    if (sv >= GPS_SV_TYPE && sv <= IRNSS_SV_TYPE) {
                        Log.v(TAG, "Disabled SV type: " + sv);
                        disabledSvTypeArray[arrIdx++] = sv;
                    } else {
                        byte enabledSv = (byte)~sv;
                        if (enabledSv >= GPS_SV_TYPE && enabledSv <= IRNSS_SV_TYPE) {
                            Log.v(TAG, "Enabled SV type: " + enabledSv);
                            disabledSvTypeArray[arrIdx++] = sv;
                        } else {
                            Log.e(TAG, "Invalid sv type: " + sv);
                        }
                    }
                }

                mService.onGnssSvTypeConfigCb(disabledSvTypeArray);
                return;
            }

            public void getRobustLocationConfigCb(LocAidlRobustLocationInfo info) {
                RLConfigData rlConfigData = new RLConfigData();
                rlConfigData.validMask = info.validMask;
                rlConfigData.enableStatus = info.enable;
                rlConfigData.enableStatusForE911 = info.enableForE911;
                rlConfigData.major = info.major;
                rlConfigData.minor = info.minor;

                mService.onRobustLocationConfigCb(rlConfigData);
            }
            @Override
            public final int getInterfaceVersion() {
                return ILocAidlGnssConfigServiceCallback.VERSION;
            }
            @Override
            public final String getInterfaceHash() {
                return ILocAidlGnssConfigServiceCallback.HASH;
            }
        }
    }
}
