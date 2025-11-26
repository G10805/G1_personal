/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017, 2020-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.location.izat.debugreport;

import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.qti.debugreport.*;
import com.qualcomm.location.idlclient.IDLClientUtils;
import com.qualcomm.location.izat.CallbackData;
import com.qualcomm.location.izat.DataPerPackageAndUser;
import com.qualcomm.location.izat.IzatService;
import java.lang.IndexOutOfBoundsException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusReports;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService;
import vendor.qti.gnss.V1_0.ILocHidlGnss;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlApTimeStamp;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlGnssSvUsedInPosition;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlLocationExtended;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusBestPosition;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusEphemeris;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusErrRecovery;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusInjectedPosition;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusLocation;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusPdr;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusPositionFailure;
import vendor.qti.gnss.V1_1.ILocHidlDebugReportService.LocHidlSystemStatusRfAndParams;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusSvHealth;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusTimeAndClock;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusXoState;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlSystemStatusXtra;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlTime;
import vendor.qti.gnss.V1_0.ILocHidlDebugReportService.LocHidlUlpLocation;
import com.qualcomm.location.idlclient.*;
import com.qualcomm.location.idlclient.LocIDLClientBase.*;
import vendor.qti.gnss.ILocAidlGnss;
import vendor.qti.gnss.ILocAidlDebugReportService;
import vendor.qti.gnss.ILocAidlDebugReportServiceCallback;
import vendor.qti.gnss.LocAidlXtraStatus;
import vendor.qti.gnss.LocAidlSystemStatusReports;
import vendor.qti.gnss.LocAidlSystemStatusXoState;
import vendor.qti.gnss.LocAidlSystemStatusBestPosition;
import vendor.qti.gnss.LocAidlSystemStatusErrRecovery;
import vendor.qti.gnss.LocAidlSystemStatusSvHealth;
import vendor.qti.gnss.LocAidlSystemStatusEphemeris;
import vendor.qti.gnss.LocAidlSystemStatusPdr;
import vendor.qti.gnss.LocAidlSystemStatusPositionFailure;
import vendor.qti.gnss.LocAidlSystemStatusXtra;
import vendor.qti.gnss.LocAidlSystemStatusTimeAndClock;
import vendor.qti.gnss.LocAidlSystemStatusInjectedPosition;
import vendor.qti.gnss.LocAidlSystemStatusRfAndParams;

public class DebugReportService implements IzatService.ISystemEventListener {
    private static final String TAG = "DebugReportService";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sCallBacksLock = new Object();
    private RemoteCallbackList<IDebugReportCallback> mDebugReportCallbacks
            = new RemoteCallbackList<IDebugReportCallback>();
    private RemoteCallbackList<IXtraStatusCallback> mXtraStatusCallbacks
            = new RemoteCallbackList<IXtraStatusCallback>();
    private final Context mContext;
    Timer mDebugReportTimer;

    private DebugReportServiceIDLClient mDebugReportServiceIDLClient;

    private class ClientDebugReportData extends CallbackData {
        private IDebugReportCallback mCallback;
        private boolean mReportPeriodic;

        public ClientDebugReportData(IDebugReportCallback callback) {
            mCallback = callback;
            super.mCallback = callback;
            mReportPeriodic = false;
        }
    }

    private class ClientXtraStatusData extends CallbackData {
        private IXtraStatusCallback mCallback;

        public ClientXtraStatusData(IXtraStatusCallback callback) {
            mCallback = callback;
            super.mCallback = callback;
        }
    }
    private DataPerPackageAndUser<ClientDebugReportData> mDataPerPackageAndUser;
    private DataPerPackageAndUser<ClientXtraStatusData> mXtraDataPerPackageAndUser;

    // DebugReport Data classes
    ArrayList<IZatEphmerisDebugReport> mListOfEphmerisReports =
            new ArrayList<IZatEphmerisDebugReport>();
    ArrayList<IZatFixStatusDebugReport> mListOfFixStatusReports =
            new ArrayList<IZatFixStatusDebugReport>();
    ArrayList<IZatLocationReport> mListOfBestLocationReports = new ArrayList<IZatLocationReport>();
    ArrayList<IZatLocationReport> mListOfEPIReports = new ArrayList<IZatLocationReport>();
    ArrayList<IZatGpsTimeDebugReport> mListGpsTimeOfReports =
            new ArrayList<IZatGpsTimeDebugReport>();
    ArrayList<IZatXoStateDebugReport> mListXoStateOfReports =
            new ArrayList<IZatXoStateDebugReport>();
    ArrayList<IZatRfStateDebugReport> mListRfStateOfReports =
            new ArrayList<IZatRfStateDebugReport>();
    ArrayList<IZatErrorRecoveryReport> mListOfErrorRecoveries =
            new ArrayList<IZatErrorRecoveryReport>();
    ArrayList<IZatPDRDebugReport> mListOfPDRReports = new ArrayList<IZatPDRDebugReport>();
    ArrayList<IZatSVHealthDebugReport> mListOfSVHealthReports =
            new ArrayList<IZatSVHealthDebugReport>();
    ArrayList<IZatXTRADebugReport> mListOfXTRAReports = new ArrayList<IZatXTRADebugReport>();

    private static DebugReportService sInstance = null;
    public static DebugReportService getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new DebugReportService(ctx);
        }
        return sInstance;
    }

    private DebugReportService(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "DebugReportService construction");
        }

        if (LocIDLClientBase.getIDLServiceVersion().compareTo(IDLServiceVersion.V_AIDL) >= 0) {
            mDebugReportServiceIDLClient = new DebugReportServiceIDLClient(this);
        } else {
            mDebugReportServiceIDLClient = new DebugReportServiceHidlClient(this);
        }
        mContext = ctx;
        mDataPerPackageAndUser = new DataPerPackageAndUser<ClientDebugReportData>(mContext,
                new UserChanged());
        mXtraDataPerPackageAndUser = new DataPerPackageAndUser<ClientXtraStatusData>(mContext,
                new XtraUserChanged());
        IzatService.AidlClientDeathNotifier.getInstance().registerAidlClientDeathCb(this);
    }

    @Override
    public void onAidlClientDied(String packageName, int pid, int uid) {
        Log.d(TAG, "aidl client crash: " + packageName);
        synchronized (sCallBacksLock) {
            ClientDebugReportData clData =
                    mDataPerPackageAndUser.getDataByPkgName(packageName);

            if (null != clData) {
                if (VERBOSE) {
                    Log.d(TAG, "Package died: " + clData.mPackageName);
                }
                mDataPerPackageAndUser.removeData(clData);
                mDebugReportCallbacks.unregister(clData.mCallback);
            }
            checkOnPeriodicReporting();

            ClientXtraStatusData clXtraData =
                    mXtraDataPerPackageAndUser.getDataByPkgName(packageName);
            if (null != clXtraData) {
                if (VERBOSE) {
                    Log.d(TAG, "Package died: " + clXtraData.mPackageName);
                }
                mXtraDataPerPackageAndUser.removeData(clXtraData);
                mXtraStatusCallbacks.unregister(clXtraData.mCallback);
                clXtraData.mCallback = null;
            }
        }
    }

    public void reportXtraStatusUpdate(IZatXTRAStatus xtraStatus) {
        for (ClientXtraStatusData clData : mXtraDataPerPackageAndUser.getAllData()) {
            if (VERBOSE) {
                Log.d(TAG, "Sending XtraStatus to " + clData.mPackageName);
            }
            try {
                if (clData.mCallback != null) {
                    clData.mCallback.onXtraStatusChanged(xtraStatus);
                } else {
                    Log.e(TAG, "IXtraStatusCallback is null for " + clData.mPackageName);
                }
            } catch (RemoteException e) {
                // do nothing
            }
        }
    }
    /* Remote binder */
    private final IDebugReportService.Stub mBinder = new IDebugReportService.Stub() {

        public void registerForDebugReporting(final IDebugReportCallback callback) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }

            synchronized (sCallBacksLock) {
                if (VERBOSE) {
                    Log.d(TAG, "registerForDebugReporting: " +
                            mDataPerPackageAndUser.getPackageName(null));
                }


                ClientDebugReportData clData = mDataPerPackageAndUser.getData();
                if (null == clData) {
                    clData = new ClientDebugReportData(callback);
                    mDataPerPackageAndUser.setData(clData);
                } else {
                    if (null != clData.mCallback) {
                        mDebugReportCallbacks.unregister(clData.mCallback);
                    }
                    clData.mCallback = callback;
                }

                mDebugReportCallbacks.register(callback);
            }
        }

        public void unregisterForDebugReporting(IDebugReportCallback callback) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }

            synchronized (sCallBacksLock) {
                 if (VERBOSE) {
                    Log.d(TAG, "unregisterForDebugReporting: " +
                            mDataPerPackageAndUser.getPackageName(null));
                }

                mDataPerPackageAndUser.removeData(mDataPerPackageAndUser.getData());
                mDebugReportCallbacks.unregister(callback);
                checkOnPeriodicReporting();
            }
        }

        public Bundle getDebugReport() {
            if (VERBOSE) {
                Log.d(TAG, "getDebugReport JAVA: " + mDataPerPackageAndUser.getPackageName(null));
            }

            synchronized(sCallBacksLock) {
                mListOfEphmerisReports.clear();
                mListOfFixStatusReports.clear();
                mListOfEPIReports.clear();
                mListOfBestLocationReports.clear();
                mListGpsTimeOfReports.clear();
                mListXoStateOfReports.clear();
                mListRfStateOfReports.clear();
                mListOfErrorRecoveries.clear();
                mListOfPDRReports.clear();
                mListOfSVHealthReports.clear();
                mListOfXTRAReports.clear();

                if (null != mDebugReportServiceIDLClient) {
                    mDebugReportServiceIDLClient.getReport(30, mListOfEphmerisReports,
                                              mListOfFixStatusReports,
                                              mListOfEPIReports,
                                              mListOfBestLocationReports,
                                              mListGpsTimeOfReports,
                                              mListXoStateOfReports,
                                              mListRfStateOfReports,
                                              mListOfErrorRecoveries,
                                              mListOfPDRReports,
                                              mListOfSVHealthReports,
                                              mListOfXTRAReports);
                }

                Bundle bundleDebugReportObj = new Bundle();
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_EPH_STATUS_KEY,
                        mListOfEphmerisReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_FIX_STATUS_KEY,
                        mListOfFixStatusReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_EXTERNAL_POSITION_INJECTION_KEY,
                        mListOfEPIReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_BEST_POSITION_KEY,
                        mListOfBestLocationReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_GPS_TIME_KEY,
                        mListGpsTimeOfReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_RF_STATE_KEY,
                        mListRfStateOfReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_XO_STATE_KEY,
                        mListXoStateOfReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_LAST_ERROR_RECOVERIES_KEY,
                        mListOfErrorRecoveries);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_PDR_INFO_KEY,
                        mListOfPDRReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_SV_HEALTH_KEY,
                        mListOfSVHealthReports);
                bundleDebugReportObj.putParcelableArrayList(
                        IZatDebugConstants.IZAT_DEBUG_XTRA_STATUS_KEY,
                        mListOfXTRAReports);
                return bundleDebugReportObj;
            }
        }

        public void startReporting() {
            if (VERBOSE) {
                Log.d(TAG, "Request to start periodic reporting by package:"
                           + mDataPerPackageAndUser.getPackageName(null) );
            }

            // update ClientGeofenceData for this package
            synchronized(sCallBacksLock) {
                ClientDebugReportData clData = mDataPerPackageAndUser.getData();
                if (null != clData) {
                    clData.mReportPeriodic = true;
                }
            }

            if (mDebugReportTimer != null) {
                if (VERBOSE) {
                    Log.d(TAG, "Periodic reporting already in progress");
                }
                return;
            }

            mDebugReportTimer = new Timer();
            mDebugReportTimer.schedule(new TimerTask() {
                @Override
                public void run() {
                    synchronized (sCallBacksLock) {
                        mListOfEphmerisReports.clear();
                        mListOfFixStatusReports.clear();
                        mListOfEPIReports.clear();
                        mListOfBestLocationReports.clear();
                        mListGpsTimeOfReports.clear();
                        mListXoStateOfReports.clear();
                        mListRfStateOfReports.clear();
                        mListOfErrorRecoveries.clear();
                        mListOfPDRReports.clear();
                        mListOfSVHealthReports.clear();
                        mListOfXTRAReports.clear();

                        if (null != mDebugReportServiceIDLClient) {
                            mDebugReportServiceIDLClient.getReport(1, mListOfEphmerisReports,
                                                     mListOfFixStatusReports,
                                                     mListOfEPIReports,
                                                     mListOfBestLocationReports,
                                                     mListGpsTimeOfReports,
                                                     mListXoStateOfReports,
                                                     mListRfStateOfReports,
                                                     mListOfErrorRecoveries,
                                                     mListOfPDRReports,
                                                     mListOfSVHealthReports,
                                                     mListOfXTRAReports);
                        }

                        if (mListOfEphmerisReports.isEmpty() &&
                                mListOfFixStatusReports.isEmpty() &&
                                mListOfEPIReports.isEmpty() &&
                                mListOfBestLocationReports.isEmpty() &&
                                mListGpsTimeOfReports.isEmpty() &&
                                mListXoStateOfReports.isEmpty() &&
                                mListRfStateOfReports.isEmpty() &&
                                mListOfErrorRecoveries.isEmpty() &&
                                mListOfPDRReports.isEmpty() &&
                                mListOfSVHealthReports.isEmpty() &&
                                mListOfXTRAReports.isEmpty()) {
                            if (VERBOSE) {
                                Log.d(TAG, "Empty debug report");
                            }
                            return;
                        }

                        Bundle bundleDebugReportObj = new Bundle();

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_EPH_STATUS_KEY,
                                    mListOfEphmerisReports.get(0) );
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_FIX_STATUS_KEY,
                                    mListOfFixStatusReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_EXTERNAL_POSITION_INJECTION_KEY,
                                    mListOfEPIReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_BEST_POSITION_KEY,
                                    mListOfBestLocationReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_GPS_TIME_KEY,
                                    mListGpsTimeOfReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_RF_STATE_KEY,
                                    mListRfStateOfReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_XO_STATE_KEY,
                                    mListXoStateOfReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_LAST_ERROR_RECOVERIES_KEY,
                                    mListOfErrorRecoveries.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_PDR_INFO_KEY,
                                    mListOfPDRReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_SV_HEALTH_KEY,
                                    mListOfSVHealthReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        try {
                            bundleDebugReportObj.putParcelable(
                                    IZatDebugConstants.IZAT_DEBUG_XTRA_STATUS_KEY,
                                    mListOfXTRAReports.get(0));
                        } catch (IndexOutOfBoundsException ioobe) {}

                        String ownerPackage = null;
                        for (ClientDebugReportData clData : mDataPerPackageAndUser.getAllData()) {
                            if (true == clData.mReportPeriodic) {
                                if (VERBOSE) {
                                    Log.d(TAG, "Sending report to " + clData.mPackageName);
                                }

                                try {
                                    clData.mCallback.onDebugDataAvailable(
                                            bundleDebugReportObj);
                                } catch (RemoteException e) {
                                             // do nothing
                                }
                            }
                        }
                    }
                }}, 0, 1000);
            }

        public void stopReporting() {
            if (VERBOSE) {
                Log.d(TAG, "Request to stop periodic reporting by package:"
                           + mDataPerPackageAndUser.getPackageName(null));
            }

            // update ClientGeofenceData for this package
            synchronized (sCallBacksLock) {
                ClientDebugReportData clData = mDataPerPackageAndUser.getData();
                if (null != clData) {
                    clData.mReportPeriodic = false;
                }

                checkOnPeriodicReporting();
            }
        }
        public void registerXtraStatusListener(IXtraStatusCallback callback) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }
                 if (VERBOSE) {
                    Log.d(TAG, "registerXtraStatusListener: " +
                            mXtraDataPerPackageAndUser.getPackageName(null));
                }
            synchronized (sCallBacksLock) {
                ClientXtraStatusData clData = mXtraDataPerPackageAndUser.getData();
                if (null == clData) {
                    clData = new ClientXtraStatusData(callback);
                    mXtraDataPerPackageAndUser.setData(clData);
                } else {
                    if (null != clData.mCallback) {
                        mXtraStatusCallbacks.unregister(clData.mCallback);
                    }
                    clData.mCallback = callback;
                }
                mXtraStatusCallbacks.register(callback);
            }
            if (null != mDebugReportServiceIDLClient) {
                mDebugReportServiceIDLClient.registerXtraStatusListener();
            }
        }

        public void unregisterXtraStatusListener() {
            if (null != mDebugReportServiceIDLClient) {
                mDebugReportServiceIDLClient.unregisterXtraStatusListener();
            }
        }

        public void getXtraStatus() {
            synchronized (sCallBacksLock) {
                ClientXtraStatusData clData = mXtraDataPerPackageAndUser.getData();
                if (null == clData || null == clData.mCallback) {
                    Log.e(TAG, "IXtraStatusCallback not registered for: " +
                            mXtraDataPerPackageAndUser.getPackageName(null));
                    return;
                }
            }
            if (null != mDebugReportServiceIDLClient) {
                mDebugReportServiceIDLClient.getXtraStatus();
            }
        }
    };

    private void checkOnPeriodicReporting() {
        boolean continuePeriodicReporting = false;

        if (mDebugReportTimer == null) {
            if (VERBOSE) {
                Log.d(TAG, "No peridoc reporting in progress !!");
            }
            return;
        }

        for (ClientDebugReportData clData : mDataPerPackageAndUser.getAllData()) {
            if (clData.mReportPeriodic == true) {
                continuePeriodicReporting = true;
                break;
            }
        }

        if (continuePeriodicReporting == false) {
            if (VERBOSE) {
                Log.d(TAG, "Service is stopping periodic debug reports");
            }

            mDebugReportTimer.cancel();
            mDebugReportTimer = null;
        }
    }

    class UserChanged implements DataPerPackageAndUser.UserChangeListener<ClientDebugReportData> {
        @Override
        public void onUserChange(Map<String, ClientDebugReportData> prevUserData,
                                 Map<String, ClientDebugReportData> currentUserData) {
            if (VERBOSE) {
                Log.d(TAG, "Active user has changed, updating debugReport callbacks...");
            }

            synchronized (sCallBacksLock) {
                // Remove prevUser callbacks
                for (ClientDebugReportData debugReportDataData: prevUserData.values()) {
                    mDebugReportCallbacks.unregister(debugReportDataData.mCallback);
                }

                // Add back current user callbacks
                for (ClientDebugReportData debugReportDataData: currentUserData.values()) {
                    mDebugReportCallbacks.register(debugReportDataData.mCallback);
                }

                checkOnPeriodicReporting();
            }
        }
    }
    class XtraUserChanged implements DataPerPackageAndUser.UserChangeListener<ClientXtraStatusData> {
        @Override
        public void onUserChange(Map<String, ClientXtraStatusData> prevUserData,
                                 Map<String, ClientXtraStatusData> currentUserData) {
            if (VERBOSE) {
                Log.d(TAG, "Active user has changed, updating xtraStatus callbacks...");
            }

            synchronized (sCallBacksLock) {
                // Remove prevUser callbacks
                for (ClientXtraStatusData xtraStatusDataData: prevUserData.values()) {
                    mXtraStatusCallbacks.unregister(xtraStatusDataData.mCallback);
                }

                // Add back current user callbacks
                for (ClientXtraStatusData xtraStatusDataData: currentUserData.values()) {
                    mXtraStatusCallbacks.register(xtraStatusDataData.mCallback);
                }

            }
        }
    }

    public IDebugReportService getDebugReportBinder() {
        return mBinder;
    }

    private class DebugReportServiceHidlClient extends DebugReportServiceIDLClient
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "DebugReportServiceHidlClient";
        private final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        private ILocHidlDebugReportService sDebugService;

        private boolean sInstanceStarted = false;


        void getDebugReportIface() {
            if (null == sDebugService) {
                ILocHidlGnss service = getGnssService();
                if (null != service) {
                    try {
                        sDebugService = service.getExtensionLocHidlDebugReportService();
                        Log.d(TAG, "getDebugReportIface sDebugService=" + sDebugService);
                    } catch (RemoteException e) {
                        Log.e(TAG, "Exception getting debug report extension: " + e);
                    }
                } else {
                    Log.e(TAG, "Debug report service is null!");
                }
            }
        }

        private DebugReportServiceHidlClient(DebugReportService provider) {
            super(provider);
            getDebugReportIface();
            if (null != sDebugService) {
                try {
                    Log.d(TAG, "DebugReportServiceHidlClient hidl init");
                    sDebugService.init();
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception getting debug report extension: " + e);
                }
            }
        }

        @Override
        public void onServiceDied() {
            sDebugService = null;
            getDebugReportIface();
            if (null != sDebugService) {
                try {
                    Log.d(TAG, "DebugReportServiceHidlClient hidl init");
                    sDebugService.init();
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception getting debug report extension: " + e);
                }
            }
        }

        @Override
        public void getReport(int maxReports,
                ArrayList<IZatEphmerisDebugReport> ephmerisReports,
                ArrayList<IZatFixStatusDebugReport> fixStatusReports,
                ArrayList<IZatLocationReport> epiReports,
                ArrayList<IZatLocationReport> bestLocationReports,
                ArrayList<IZatGpsTimeDebugReport> gpsTimeReports,
                ArrayList<IZatXoStateDebugReport> xoStateReports,
                ArrayList<IZatRfStateDebugReport> rfStateReports,
                ArrayList<IZatErrorRecoveryReport> errorRecoveries,
                ArrayList<IZatPDRDebugReport> pdrReports,
                ArrayList<IZatSVHealthDebugReport> svHealthReports,
                ArrayList<IZatXTRADebugReport> xtraReports) {

            vendor.qti.gnss.V1_1.ILocHidlDebugReportService ifaceV1_1 =
                    vendor.qti.gnss.V1_1.ILocHidlDebugReportService.castFrom(sDebugService);

            if (null != ifaceV1_1) {
                Log.d(TAG, "getReport 1.1...");
                try {
                    vendor.qti.gnss.V1_1.ILocHidlDebugReportService.LocHidlSystemStatusReports
                            reports = ifaceV1_1.getReport_1_1(maxReports);

                    Log.d(TAG, "getReport 1.1 success: " + reports.base_1_0.mSuccess);
                    populateV1_0Reports(reports.base_1_0, ephmerisReports, fixStatusReports,
                            epiReports, bestLocationReports, gpsTimeReports, xoStateReports,
                            rfStateReports, errorRecoveries, pdrReports, svHealthReports,
                            xtraReports);

                    for (LocHidlSystemStatusRfAndParams status: reports.mRfAndParamsVec_1_1) {
                        IZatRfStateDebugReport rfReport = new IZatRfStateDebugReport(
                                new IZatUtcSpec(status.base_1_0.mUtcTime.tv_sec,
                                        status.base_1_0.mUtcTime.tv_nsec),
                                new IZatUtcSpec(status.base_1_0.mUtcReported.tv_sec,
                                        status.base_1_0.mUtcReported.tv_nsec),
                                status.base_1_0.mPgaGain,
                                status.base_1_0.mAdcI,
                                status.base_1_0.mAdcQ,
                                status.base_1_0.mJammerGps, status.base_1_0.mJammerGlo,
                                status.base_1_0.mJammerBds, status.base_1_0.mJammerGal,
                                status.base_1_0.mGpsBpAmpI, status.base_1_0.mGpsBpAmpQ,
                                status.mGloBpAmpI, status.mGloBpAmpQ,
                                status.mBdsBpAmpI, status.mBdsBpAmpQ,
                                status.mGalBpAmpI, status.mGalBpAmpQ);
                        rfStateReports.add(rfReport);
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception getting report: " + e);
                }
            } else {
                Log.d(TAG, "getReport 1.0...");
                try {
                    LocHidlSystemStatusReports reports = sDebugService.getReport(maxReports);
                    populateV1_0Reports(reports, ephmerisReports, fixStatusReports, epiReports,
                            bestLocationReports, gpsTimeReports, xoStateReports, rfStateReports,
                            errorRecoveries, pdrReports, svHealthReports, xtraReports);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception getting report: " + e);
                }
            }
        }

        private void populateV1_0Reports(LocHidlSystemStatusReports reports,
                ArrayList<IZatEphmerisDebugReport> ephmerisReports,
                ArrayList<IZatFixStatusDebugReport> fixStatusReports,
                ArrayList<IZatLocationReport> epiReports,
                ArrayList<IZatLocationReport> bestLocationReports,
                ArrayList<IZatGpsTimeDebugReport> gpsTimeReports,
                ArrayList<IZatXoStateDebugReport> xoStateReports,
                ArrayList<IZatRfStateDebugReport> rfStateReports,
                ArrayList<IZatErrorRecoveryReport> errorRecoveries,
                ArrayList<IZatPDRDebugReport> pdrReports,
                ArrayList<IZatSVHealthDebugReport> svHealthReports,
                ArrayList<IZatXTRADebugReport> xtraReports) {
            Log.d(TAG, "getReport 1.0 success: " + reports.mSuccess);

            populateXtraReportList(xtraReports, reports.mXtraVec);
            populateGpsTimeReportList(gpsTimeReports, reports.mTimeAndClockVec);
            populateXoStateReports(xoStateReports, reports.mXoStateVec);
            populateErrRecoveryReports(errorRecoveries, reports.mErrRecoveryVec);
            populateInjectedPositionReports(epiReports, reports.mInjectedPositionVec);
            populateBestPositionReports(bestLocationReports, reports.mBestPositionVec);
            populateEphemerisReports(ephmerisReports, reports.mEphemerisVec);
            populateSvHealthReports(svHealthReports, reports.mSvHealthVec);
            populatePdrReports(pdrReports, reports.mPdrVec);
            populatePositionFailureReports(fixStatusReports, reports.mPositionFailureVec);
        }

        private void populateXtraReportList(ArrayList<IZatXTRADebugReport> xtraReports,
                ArrayList<ILocHidlDebugReportService.LocHidlSystemStatusXtra> hidlXtraReports ) {
            for (ILocHidlDebugReportService.LocHidlSystemStatusXtra status: hidlXtraReports) {
                IZatXTRADebugReport xtraReport = new  IZatXTRADebugReport(
                        new IZatUtcSpec(status.mUtcTime.tv_sec, status.mUtcTime.tv_nsec),
                        new IZatUtcSpec(status.mUtcReported.tv_sec, status.mUtcReported.tv_nsec),
                        status.mXtraValidMask, status.mGpsXtraValid, status.mGpsXtraAge,
                        status.mGloXtraValid, status.mGloXtraAge,
                        status.mBdsXtraValid, status.mBdsXtraAge,
                        status.mGalXtraValid, status.mGalXtraAge,
                        status.mQzssXtraValid, status.mQzssXtraAge);

                xtraReports.add(xtraReport);
            }
        }

        private void populateGpsTimeReportList(ArrayList<IZatGpsTimeDebugReport> izatReports,
                ArrayList<ILocHidlDebugReportService.LocHidlSystemStatusTimeAndClock> hidlReports) {
            for (ILocHidlDebugReportService.LocHidlSystemStatusTimeAndClock hidlReport:
                    hidlReports) {
                IZatGpsTimeDebugReport izatReport = new  IZatGpsTimeDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mGpsWeek,
                        hidlReport.mGpsTowMs,
                        hidlReport.mTimeValid == 1,
                        hidlReport.mTimeSource,
                        hidlReport.mTimeUnc,
                        hidlReport.mClockFreqBias,
                        hidlReport.mClockFreqBiasUnc,
                        hidlReport.mLeapSeconds,
                        hidlReport.mLeapSecUnc);

                izatReports.add(izatReport);
            }
        }

        private void populateXoStateReports(ArrayList<IZatXoStateDebugReport> izatReports,
                ArrayList<LocHidlSystemStatusXoState> hidlReports) {
            for (LocHidlSystemStatusXoState hidlReport: hidlReports) {
                IZatXoStateDebugReport izatReport = new  IZatXoStateDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mXoState);
                izatReports.add(izatReport);
            }
        }

        private void populateErrRecoveryReports(ArrayList<IZatErrorRecoveryReport> izatReports,
                ArrayList<LocHidlSystemStatusErrRecovery> hidlReports) {
            for (LocHidlSystemStatusErrRecovery hidlReport: hidlReports) {
                IZatErrorRecoveryReport izatReport = new  IZatErrorRecoveryReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec));
                izatReports.add(izatReport);
            }
        }

        private void populateInjectedPositionReports(ArrayList<IZatLocationReport> izatReports,
                ArrayList<LocHidlSystemStatusInjectedPosition> hidlReports) {
            for (LocHidlSystemStatusInjectedPosition hidlReport: hidlReports) {
                IZatLocationReport izatReport = new  IZatLocationReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mEpiValidity | HAS_SOURCE,
                        hidlReport.mEpiLat,
                        hidlReport.mEpiLon,
                        hidlReport.mEpiHepe,
                        hidlReport.mEpiAlt,
                        hidlReport.mEpiAltUnc,
                        hidlReport.mEpiSrc);
                izatReports.add(izatReport);
            }
        }

        private void populateBestPositionReports(ArrayList<IZatLocationReport> izatReports,
                ArrayList<LocHidlSystemStatusBestPosition> hidlReports) {
            for (LocHidlSystemStatusBestPosition hidlReport: hidlReports) {
                IZatLocationReport izatReport = new  IZatLocationReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        HAS_HORIZONTAL_COMPONENT | HAS_VERTICAL_COMPONENT,
                        hidlReport.mBestLat,
                        hidlReport.mBestLon,
                        hidlReport.mBestHepe,
                        hidlReport.mBestAlt,
                        hidlReport.mBestAltUnc, 0);
                izatReports.add(izatReport);
            }
        }

        private void populateEphemerisReports(ArrayList<IZatEphmerisDebugReport> izatReports,
                ArrayList<LocHidlSystemStatusEphemeris> hidlReports) {
            for (LocHidlSystemStatusEphemeris hidlReport: hidlReports) {
                IZatEphmerisDebugReport izatReport = new  IZatEphmerisDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mGpsEpheValid,
                        hidlReport.mGloEpheValid,
                        hidlReport.mBdsEpheValid,
                        hidlReport.mGalEpheValid,
                        hidlReport.mQzssEpheValid);
                izatReports.add(izatReport);
            }
        }

        private void populateSvHealthReports(ArrayList<IZatSVHealthDebugReport> izatReports,
                ArrayList<LocHidlSystemStatusSvHealth> hidlReports) {
            for (LocHidlSystemStatusSvHealth hidlReport: hidlReports) {
                IZatSVHealthDebugReport izatReport = new  IZatSVHealthDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mGpsGoodMask,
                        hidlReport.mGpsUnknownMask,
                        hidlReport.mGpsBadMask,
                        hidlReport.mGloGoodMask,
                        hidlReport.mGloUnknownMask,
                        hidlReport.mGloBadMask,
                        hidlReport.mBdsGoodMask,
                        hidlReport.mBdsUnknownMask,
                        hidlReport.mBdsBadMask,
                        hidlReport.mGalGoodMask,
                        hidlReport.mGalUnknownMask,
                        hidlReport.mGalBadMask,
                        hidlReport.mQzssGoodMask,
                        hidlReport.mQzssUnknownMask,
                        hidlReport.mQzssBadMask);
                izatReports.add(izatReport);
            }
        }

        private void populatePdrReports(ArrayList<IZatPDRDebugReport> izatReports,
                ArrayList<LocHidlSystemStatusPdr> hidlReports) {
            for (LocHidlSystemStatusPdr hidlReport: hidlReports) {
                IZatPDRDebugReport izatReport = new  IZatPDRDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mFixInfoMask);
                izatReports.add(izatReport);
            }
        }

        private void populatePositionFailureReports(ArrayList<IZatFixStatusDebugReport> izatReports,
                ArrayList<LocHidlSystemStatusPositionFailure> hidlReports) {
            for (LocHidlSystemStatusPositionFailure hidlReport: hidlReports) {
                IZatFixStatusDebugReport izatReport = new  IZatFixStatusDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mFixInfoMask, hidlReport.mHepeLimit);
                izatReports.add(izatReport);
            }
        }
    }

    private class DebugReportServiceIDLClient extends LocIDLClientBase
            implements LocIDLClientBase.IServiceDeathCb {

        private static final String TAG = "DebugReportServiceIDLClient";
        public final int HAS_HORIZONTAL_COMPONENT = 1;
        public final int HAS_VERTICAL_COMPONENT = 2;
        public final int HAS_SOURCE = 4;
        private ILocAidlDebugReportService sDebugService;
        private DebugReportServiceCallback mXtraStatusCb;

        private boolean sInstanceStarted = false;


        void getDebugReportIface() {
            if (null == sDebugService) {
                ILocAidlGnss service = getGnssAidlService();
                if (null != service) {
                    try {
                        sDebugService = service.getExtensionLocAidlDebugReportService();
                        Log.d(TAG, "getDebugReportIface sDebugService=" + sDebugService);
                    } catch (RemoteException e) {
                        Log.e(TAG, "Exception getting debug report extension: " + e);
                    }
                } else {
                    Log.e(TAG, "Debug report service is null!");
                }
            }
        }

        private DebugReportServiceIDLClient(DebugReportService provider) {
            getDebugReportIface();
            if (null != sDebugService) {
                try {
                    Log.d(TAG, "DebugReportServiceIDLClient hidl init");
                    mXtraStatusCb = new DebugReportServiceCallback(provider);
                    sDebugService.init();
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception getting debug report extension: " + e);
                }
            }
        }

        @Override
        public void onServiceDied() {
            sDebugService = null;
            getDebugReportIface();
            if (null != sDebugService) {
                try {
                    Log.d(TAG, "DebugReportServiceIDLClient init");
                    sDebugService.init();
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception getting debug report extension: " + e);
                }
            }
        }
        public void registerXtraStatusListener() {
            vendor.qti.gnss.ILocAidlDebugReportService iface = sDebugService;

            if (null != iface) {
                Log.d(TAG, "registerXtraStatusListener ...");
                try {
                    iface.registerXtraStatusListener(mXtraStatusCb);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception when registerXtraStatusListener(): " + e);
                }
            }
        }

        public void unregisterXtraStatusListener() {
            vendor.qti.gnss.ILocAidlDebugReportService iface = sDebugService;

            if (null != iface) {
                Log.d(TAG, "unregisterXtraStatusListener ...");
                try {
                    iface.unregisterXtraStatusListener();
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception when unregisterXtraStatusListener(): " + e);
                }
            }
        }
        public void getXtraStatus() {
            vendor.qti.gnss.ILocAidlDebugReportService iface = sDebugService;

            if (null != iface) {
                Log.d(TAG, "getXtraStatus ...");
                try {
                    iface.getXtraStatus();
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception when getXtraStatus(): " + e);
                }
            }
        }
        public void getReport(int maxReports,
                ArrayList<IZatEphmerisDebugReport> ephmerisReports,
                ArrayList<IZatFixStatusDebugReport> fixStatusReports,
                ArrayList<IZatLocationReport> epiReports,
                ArrayList<IZatLocationReport> bestLocationReports,
                ArrayList<IZatGpsTimeDebugReport> gpsTimeReports,
                ArrayList<IZatXoStateDebugReport> xoStateReports,
                ArrayList<IZatRfStateDebugReport> rfStateReports,
                ArrayList<IZatErrorRecoveryReport> errorRecoveries,
                ArrayList<IZatPDRDebugReport> pdrReports,
                ArrayList<IZatSVHealthDebugReport> svHealthReports,
                ArrayList<IZatXTRADebugReport> xtraReports) {

            vendor.qti.gnss.ILocAidlDebugReportService iface = sDebugService;

            if (null != iface) {
                Log.d(TAG, "getReport ...");
                try {
                    LocAidlSystemStatusReports reports = iface.getReport(maxReports);

                    Log.d(TAG, "getReport success: " + reports.mSuccess);
                    populateV1_0Reports(reports, ephmerisReports, fixStatusReports,
                            epiReports, bestLocationReports, gpsTimeReports, xoStateReports,
                            rfStateReports, errorRecoveries, pdrReports, svHealthReports,
                            xtraReports);

                    for (LocAidlSystemStatusRfAndParams status: reports.mRfAndParamsVec) {
                        IZatRfStateDebugReport rfReport = new IZatRfStateDebugReport(
                                new IZatUtcSpec(status.mUtcTime.tv_sec,
                                        status.mUtcTime.tv_nsec),
                                new IZatUtcSpec(status.mUtcReported.tv_sec,
                                        status.mUtcReported.tv_nsec),
                                status.mPgaGain,
                                status.mAdcI,
                                status.mAdcQ,
                                status.mJammerGps, status.mJammerGlo,
                                status.mJammerBds, status.mJammerGal,
                                status.mGpsBpAmpI, status.mGpsBpAmpQ,
                                status.mGloBpAmpI, status.mGloBpAmpQ,
                                status.mBdsBpAmpI, status.mBdsBpAmpQ,
                                status.mGalBpAmpI, status.mGalBpAmpQ,
                                status.mJammedSignalsMask, status.mJammerInd,
                                status.mAgc);
                        rfStateReports.add(rfReport);
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception getting report: " + e);
                }
            }
        }

        private void populateV1_0Reports(LocAidlSystemStatusReports reports,
                ArrayList<IZatEphmerisDebugReport> ephmerisReports,
                ArrayList<IZatFixStatusDebugReport> fixStatusReports,
                ArrayList<IZatLocationReport> epiReports,
                ArrayList<IZatLocationReport> bestLocationReports,
                ArrayList<IZatGpsTimeDebugReport> gpsTimeReports,
                ArrayList<IZatXoStateDebugReport> xoStateReports,
                ArrayList<IZatRfStateDebugReport> rfStateReports,
                ArrayList<IZatErrorRecoveryReport> errorRecoveries,
                ArrayList<IZatPDRDebugReport> pdrReports,
                ArrayList<IZatSVHealthDebugReport> svHealthReports,
                ArrayList<IZatXTRADebugReport> xtraReports) {
            Log.d(TAG, "getReport 1.0 success: " + reports.mSuccess);

            populateXtraReportList(xtraReports, reports.mXtraVec);
            populateGpsTimeReportList(gpsTimeReports, reports.mTimeAndClockVec);
            populateXoStateReports(xoStateReports, reports.mXoStateVec);
            populateErrRecoveryReports(errorRecoveries, reports.mErrRecoveryVec);
            populateInjectedPositionReports(epiReports, reports.mInjectedPositionVec);
            populateBestPositionReports(bestLocationReports, reports.mBestPositionVec);
            populateEphemerisReports(ephmerisReports, reports.mEphemerisVec);
            populateSvHealthReports(svHealthReports, reports.mSvHealthVec);
            populatePdrReports(pdrReports, reports.mPdrVec);
            populatePositionFailureReports(fixStatusReports, reports.mPositionFailureVec);
        }

        private void populateXtraReportList(ArrayList<IZatXTRADebugReport> xtraReports,
                LocAidlSystemStatusXtra[] hidlXtraReports ) {
            for (LocAidlSystemStatusXtra status: hidlXtraReports) {
                IZatXTRADebugReport xtraReport = new  IZatXTRADebugReport(
                        new IZatUtcSpec(status.mUtcTime.tv_sec, status.mUtcTime.tv_nsec),
                        new IZatUtcSpec(status.mUtcReported.tv_sec, status.mUtcReported.tv_nsec),
                        status.mXtraValidMask, status.mGpsXtraValid, status.mGpsXtraAge,
                        status.mGloXtraValid, status.mGloXtraAge,
                        status.mBdsXtraValid, status.mBdsXtraAge,
                        status.mGalXtraValid, status.mGalXtraAge,
                        status.mQzssXtraValid, status.mQzssXtraAge);

                xtraReports.add(xtraReport);
            }
        }

        private void populateGpsTimeReportList(ArrayList<IZatGpsTimeDebugReport> izatReports,
                LocAidlSystemStatusTimeAndClock[] hidlReports) {
            for (LocAidlSystemStatusTimeAndClock hidlReport:
                    hidlReports) {
                IZatGpsTimeDebugReport izatReport = new  IZatGpsTimeDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mGpsWeek,
                        hidlReport.mGpsTowMs,
                        hidlReport.mTimeValid == 1,
                        hidlReport.mTimeSource,
                        hidlReport.mTimeUnc,
                        hidlReport.mClockFreqBias,
                        hidlReport.mClockFreqBiasUnc,
                        hidlReport.mLeapSeconds,
                        hidlReport.mLeapSecUnc);

                izatReports.add(izatReport);
            }
        }

        private void populateXoStateReports(ArrayList<IZatXoStateDebugReport> izatReports,
                LocAidlSystemStatusXoState[] hidlReports) {
            for (LocAidlSystemStatusXoState hidlReport: hidlReports) {
                IZatXoStateDebugReport izatReport = new  IZatXoStateDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mXoState);
                izatReports.add(izatReport);
            }
        }

        private void populateErrRecoveryReports(ArrayList<IZatErrorRecoveryReport> izatReports,
                LocAidlSystemStatusErrRecovery[] hidlReports) {
            for (LocAidlSystemStatusErrRecovery hidlReport: hidlReports) {
                IZatErrorRecoveryReport izatReport = new  IZatErrorRecoveryReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec));
                izatReports.add(izatReport);
            }
        }

        private void populateInjectedPositionReports(ArrayList<IZatLocationReport> izatReports,
                LocAidlSystemStatusInjectedPosition[] hidlReports) {
            for (LocAidlSystemStatusInjectedPosition hidlReport: hidlReports) {
                IZatLocationReport izatReport = new  IZatLocationReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mEpiValidity | HAS_SOURCE,
                        hidlReport.mEpiLat,
                        hidlReport.mEpiLon,
                        hidlReport.mEpiHepe,
                        hidlReport.mEpiAlt,
                        hidlReport.mEpiAltUnc,
                        hidlReport.mEpiSrc);
                izatReports.add(izatReport);
            }
        }

        private void populateBestPositionReports(ArrayList<IZatLocationReport> izatReports,
                LocAidlSystemStatusBestPosition[] hidlReports) {
            for (LocAidlSystemStatusBestPosition hidlReport: hidlReports) {
                IZatLocationReport izatReport = new  IZatLocationReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        HAS_HORIZONTAL_COMPONENT | HAS_VERTICAL_COMPONENT,
                        hidlReport.mBestLat,
                        hidlReport.mBestLon,
                        hidlReport.mBestHepe,
                        hidlReport.mBestAlt,
                        hidlReport.mBestAltUnc, 0);
                izatReports.add(izatReport);
            }
        }

        private void populateEphemerisReports(ArrayList<IZatEphmerisDebugReport> izatReports,
                LocAidlSystemStatusEphemeris[] hidlReports) {
            for (LocAidlSystemStatusEphemeris hidlReport: hidlReports) {
                IZatEphmerisDebugReport izatReport = new  IZatEphmerisDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mGpsEpheValid,
                        hidlReport.mGloEpheValid,
                        hidlReport.mBdsEpheValid,
                        hidlReport.mGalEpheValid,
                        hidlReport.mQzssEpheValid);
                izatReports.add(izatReport);
            }
        }

        private void populateSvHealthReports(ArrayList<IZatSVHealthDebugReport> izatReports,
                LocAidlSystemStatusSvHealth[] hidlReports) {
            for (LocAidlSystemStatusSvHealth hidlReport: hidlReports) {
                IZatSVHealthDebugReport izatReport = new  IZatSVHealthDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mGpsGoodMask,
                        hidlReport.mGpsUnknownMask,
                        hidlReport.mGpsBadMask,
                        hidlReport.mGloGoodMask,
                        hidlReport.mGloUnknownMask,
                        hidlReport.mGloBadMask,
                        hidlReport.mBdsGoodMask,
                        hidlReport.mBdsUnknownMask,
                        hidlReport.mBdsBadMask,
                        hidlReport.mGalGoodMask,
                        hidlReport.mGalUnknownMask,
                        hidlReport.mGalBadMask,
                        hidlReport.mQzssGoodMask,
                        hidlReport.mQzssUnknownMask,
                        hidlReport.mQzssBadMask);
                izatReports.add(izatReport);
            }
        }

        private void populatePdrReports(ArrayList<IZatPDRDebugReport> izatReports,
                LocAidlSystemStatusPdr[] hidlReports) {
            for (LocAidlSystemStatusPdr hidlReport: hidlReports) {
                IZatPDRDebugReport izatReport = new  IZatPDRDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mFixInfoMask);
                izatReports.add(izatReport);
            }
        }

        private void populatePositionFailureReports(ArrayList<IZatFixStatusDebugReport> izatReports,
                LocAidlSystemStatusPositionFailure[] hidlReports) {
            for (LocAidlSystemStatusPositionFailure hidlReport: hidlReports) {
                IZatFixStatusDebugReport izatReport = new  IZatFixStatusDebugReport(
                        new IZatUtcSpec(hidlReport.mUtcTime.tv_sec, hidlReport.mUtcTime.tv_nsec),
                        new IZatUtcSpec(hidlReport.mUtcReported.tv_sec,
                                        hidlReport.mUtcReported.tv_nsec),
                        hidlReport.mFixInfoMask, hidlReport.mHepeLimit);
                izatReports.add(izatReport);
            }
        }
        /* =================================================
         *   AIDL Callbacks : ILocAidlDebugReportServiceCallback.hal
         * =================================================*/
        private class DebugReportServiceCallback extends ILocAidlDebugReportServiceCallback.Stub {
            private DebugReportService mProvider;
            public DebugReportServiceCallback(DebugReportService provider) {
                mProvider = provider;
            }
            @Override
            public void onXtraStatusChanged(LocAidlXtraStatus xtra)
                    throws android.os.RemoteException {
                IDLClientUtils.fromIDLService(TAG);
                IZatXTRAStatus xtraStatus = new IZatXTRAStatus(xtra.mEnabled, xtra.mStatus,
                        xtra.mValidityHrs, xtra.mLastDownloadStatus);
                mProvider.reportXtraStatusUpdate(xtraStatus);
            }
            @Override
            public final String getInterfaceHash() {
                return ILocAidlDebugReportServiceCallback.HASH;
            }
            @Override
            public final int getInterfaceVersion() {
                return ILocAidlDebugReportServiceCallback.VERSION;
            }
        }
    }
}
