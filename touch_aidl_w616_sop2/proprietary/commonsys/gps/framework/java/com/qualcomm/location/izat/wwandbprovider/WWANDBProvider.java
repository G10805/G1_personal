/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.wwandbprovider;

import android.app.PendingIntent;
import android.content.Context;
import android.location.Location;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.Binder;
import android.util.Log;

import com.qualcomm.location.izat.GTPClientHelper;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import com.qti.wwandbprovider.*;

import com.qti.wwandbprovider.*;
import com.qti.wwandbreceiver.BSInfo;
import com.qualcomm.location.idlclient.LocIDLClientBase;
import com.qualcomm.location.idlclient.IDLClientUtils;
import com.qualcomm.location.izat.IzatService;
import vendor.qti.gnss.V2_1.ILocHidlGnss;
import vendor.qti.gnss.V2_1.ILocHidlWWANDBProvider;
import vendor.qti.gnss.V2_1.ILocHidlWWANDBProviderCallback;
import vendor.qti.gnss.V2_1.ILocHidlWWANDBProviderCallback.LocHidlBSObsData;
import vendor.qti.gnss.V2_1.ILocHidlWWANDBProviderCallback.LocHidlBsCellInfo;
import vendor.qti.gnss.V2_1.LocHidlUlpLocation;

import com.qualcomm.location.idlclient.LocIDLClientBase.*;
import com.qualcomm.location.idlclient.*;
import vendor.qti.gnss.ILocAidlGnss;
import vendor.qti.gnss.ILocAidlWWANDBProvider;
import vendor.qti.gnss.ILocAidlWWANDBProviderCallback;
import vendor.qti.gnss.LocAidlBSObsData;

import static android.location.LocationManager.FUSED_PROVIDER;
import static android.location.LocationManager.GPS_PROVIDER;

public class WWANDBProvider implements IzatService.ISystemEventListener {
    private static final String TAG = "WWANDBProvider";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sCallBacksLock = new Object();
    private final Context mContext;

    private volatile IWWANDBProviderResponseListener mWWANDBProviderResponseListener = null;

    private PendingIntent mListenerIntent = null;
    private WWANDBProviderIdlClient mIdlClient = null;

    private String mPackageName;

    private static WWANDBProvider sInstance = null;
    public static WWANDBProvider getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new WWANDBProvider(ctx);
        }
        return sInstance;
    }

    private WWANDBProvider(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "WWANDBProvider construction");
        }
        mContext = ctx;
        if (LocIDLClientBase.getIDLServiceVersion().compareTo(IDLServiceVersion.V_AIDL) >= 0) {
            mIdlClient = new WWANDBProviderIdlClient(this);
        } else {
            mIdlClient = new WWANDBProviderHidlClient(this);
        }
        if (null == mIdlClient) {
            Log.e(TAG, "WWANDBProvider construction fail: " + mIdlClient);
        }
        IzatService.AidlClientDeathNotifier.getInstance().registerAidlClientDeathCb(this);
    }

    @Override
    public void onAidlClientDied(String packageName, int pid, int uid) {
        if (mPackageName != null && mPackageName.equals(packageName)) {
            Log.d(TAG, "aidl client crash: " + packageName);
            synchronized (sCallBacksLock) {
                mWWANDBProviderResponseListener = null;
            }
        }
    }

    /* Remote binder */
    private final IWWANDBProvider.Stub mBinder = new IWWANDBProvider.Stub() {

        public boolean registerResponseListener(final IWWANDBProviderResponseListener callback,
                                                PendingIntent notifyIntent) {
            if (VERBOSE) {
                Log.d(TAG, "WWANDBProvider registerResponseListener");
            }

            if (callback == null) {
                Log.e(TAG, "callback is null");
                return false;
            }

            if (notifyIntent == null) {
                Log.w(TAG, "notifyIntent is null");
            }

            synchronized (sCallBacksLock) {
                if (null != mWWANDBProviderResponseListener) {
                    Log.e(TAG, "Response listener already provided.");
                    return false;
                }

                mWWANDBProviderResponseListener = callback;
                mListenerIntent = notifyIntent;
            }

            mPackageName = mContext.getPackageManager().getNameForUid(Binder.getCallingUid());

            return true;
        }

        public void removeResponseListener(final IWWANDBProviderResponseListener callback) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }

            synchronized (sCallBacksLock) {
                mWWANDBProviderResponseListener = null;
                mListenerIntent = null;
            }
            mPackageName = null;
        }


        public int requestBSObsLocData() {
            if (VERBOSE) {
                Log.d(TAG, "in IWWANDBProvider.Stub(): requestBSObsLocData()");
            }

            mIdlClient.requestBSObsLocData();
            return 0;
        }
    };

    private void onBsObsLocDataAvailable(ArrayList<BSObsLocationData> obsDataList, int status) {
        if (VERBOSE) {
            Log.d(TAG, "onBsObsLocDataAvailable status: " + status);
        }

        synchronized (sCallBacksLock) {
            if (null != mWWANDBProviderResponseListener) {
                try {
                    mWWANDBProviderResponseListener.onBsObsLocDataAvailable(
                            obsDataList, status);
                } catch (RemoteException e) {
                    Log.w(TAG, "onBsObsLocDataAvailable remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBProvider");
                }
            }
        }
    }

    private void onServiceRequest() {
        if (VERBOSE) {
            Log.d(TAG, "onServiceRequest");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBProviderResponseListener) {
                try {
                    mWWANDBProviderResponseListener.onServiceRequest();
                } catch (RemoteException e) {
                    Log.w(TAG, "onServiceRequest remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBProvider");
                }
            }
        }
    }

    public IWWANDBProvider getWWANDBProviderBinder() {
        return mBinder;
    }

    // ======================================================================
    // HIDL client
    // ======================================================================
    static class WWANDBProviderHidlClient extends WWANDBProviderIdlClient
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "WWANDBProviderHidlClient";
        private final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        private LocHidlWWANDBProviderCallback mLocHidlWWANDBProvicerCallback;
        private ILocHidlWWANDBProvider mLocHidlWWANDBProvider;

        public WWANDBProviderHidlClient(WWANDBProvider Provider) {
            super(Provider);
            getWWANDBProviderIface();
            mLocHidlWWANDBProvicerCallback = new LocHidlWWANDBProviderCallback(Provider);

            if (null != mLocHidlWWANDBProvider) {
                try {
                    mLocHidlWWANDBProvider.init(mLocHidlWWANDBProvicerCallback);
                    mLocHidlWWANDBProvider.registerWWANDBProvider(mLocHidlWWANDBProvicerCallback);
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on Provider init: " + e);
                }
            }
        }

        private void getWWANDBProviderIface() {
            Log.i(TAG, "getWWANDBProviderIface");
            ILocHidlGnss gnssService = (vendor.qti.gnss.V2_1.ILocHidlGnss) getGnssService();

            if (null != gnssService) {
                try {
                    mLocHidlWWANDBProvider = gnssService.getExtensionLocHidlWWANDBProvider();
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception getting wifidb Provider: " + e);
                }
            } else {
                throw new RuntimeException("gnssService is null!");
            }
        }

        @Override
        public void onServiceDied() {
            mLocHidlWWANDBProvider = null;
            getWWANDBProviderIface();
            if (null != mLocHidlWWANDBProvider) {
                try {
                    mLocHidlWWANDBProvider.init(mLocHidlWWANDBProvicerCallback);
                    mLocHidlWWANDBProvider.registerWWANDBProvider(mLocHidlWWANDBProvicerCallback);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on Provider init: " + e);
                }
            }
        }

        @Override
        public void requestBSObsLocData() {
            if (null != mLocHidlWWANDBProvider) {
                try {
                    mLocHidlWWANDBProvider.sendBSObsLocDataRequest();
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception on sendAPObsLocDataRequest: " + e);
                }
            } else {
                throw new RuntimeException("mLocHidlWWANDBProvider is null!");
            }
        }

        // ======================================================================
        // Callbacks
        // ======================================================================
        class LocHidlWWANDBProviderCallback extends ILocHidlWWANDBProviderCallback.Stub {

            private WWANDBProvider mWWANDBProvider;

            private LocHidlWWANDBProviderCallback(WWANDBProvider wiFiDBProvider) {
                mWWANDBProvider = wiFiDBProvider;
            }

            public void attachVmOnCallback() {
                // ???
            }

            public void serviceRequestCallback() {
                mWWANDBProvider.onServiceRequest();
            }

            public void bsObsLocDataUpdateCallback(ArrayList<LocHidlBSObsData> bsInfoList,
                    int apObsLocDataListSize, byte bsListStatus) {

                Log.i(TAG, "TESTDEBUG bsObsLocDataUpdateCallback");

                ArrayList<BSObsLocationData> obsDataList = new ArrayList<BSObsLocationData>();

                for (LocHidlBSObsData bsObsData: bsInfoList) {
                    BSInfo cellInfo = new BSInfo();
                    cellInfo.mCellType =
                            IDLClientUtils.FDCLtoIZatCellTypes(bsObsData.cellInfo.cell_type);
                    cellInfo.mCellRegionID1 = bsObsData.cellInfo.cell_id1;
                    cellInfo.mCellRegionID2 = bsObsData.cellInfo.cell_id2;
                    cellInfo.mCellRegionID3 = bsObsData.cellInfo.cell_id3;
                    cellInfo.mCellRegionID4 = bsObsData.cellInfo.cell_id4;

                    BSObsLocationData obsData = new BSObsLocationData();
                    obsData.mScanTimestamp = (int) bsObsData.scanTimestamp_ms;
                    obsData.mLocation =
                            IDLClientUtils.translateHidlLocation(bsObsData.gpsLoc.gpsLocation);
                    if (bsObsData.gpsLoc.position_source == 2) {
                        //This is a GNSS position report
                        obsData.mLocation.setProvider(GPS_PROVIDER);
                    } else {
                        //Mark it as fused provider if it is not a GNSS position report
                        obsData.mLocation.setProvider(FUSED_PROVIDER);
                    }

                    obsData.mCellInfo = cellInfo;

                    obsDataList.add(obsData);
                }

                mWWANDBProvider.onBsObsLocDataAvailable(obsDataList, bsListStatus);
            }
        }
    }
    // ======================================================================
    // AIDL client
    // ======================================================================
    static class WWANDBProviderIdlClient extends LocIDLClientBase
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "WWANDBProviderIdlClient";
        private final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        private LocAidlWWANDBProviderCallback mLocAidlWWANDBProvicerCallback;
        private ILocAidlWWANDBProvider mLocAidlWWANDBProvider;

        public WWANDBProviderIdlClient(WWANDBProvider Provider) {
            getWWANDBProviderIface();
            mLocAidlWWANDBProvicerCallback = new LocAidlWWANDBProviderCallback(Provider);
            if (null != mLocAidlWWANDBProvider) {
                try {
                    mLocAidlWWANDBProvider.init(mLocAidlWWANDBProvicerCallback);
                    mLocAidlWWANDBProvider.registerWWANDBProvider(mLocAidlWWANDBProvicerCallback);
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on Provider init: " + e);
                }
            }
        }

        private void getWWANDBProviderIface() {
            Log.i(TAG, "getWWANDBProviderIface");
            ILocAidlGnss gnssService = (ILocAidlGnss) getGnssAidlService();

            if (null != gnssService) {
                try {
                    mLocAidlWWANDBProvider = gnssService.getExtensionLocAidlWWANDBProvider();
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception getting wifidb Provider: " + e);
                }
            }
        }

        @Override
        public void onServiceDied() {
            mLocAidlWWANDBProvider = null;
            getWWANDBProviderIface();
            if (null != mLocAidlWWANDBProvider) {
                try {
                    mLocAidlWWANDBProvider.init(mLocAidlWWANDBProvicerCallback);
                    mLocAidlWWANDBProvider.registerWWANDBProvider(mLocAidlWWANDBProvicerCallback);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on Provider init: " + e);
                }
            }
        }

        public void requestBSObsLocData() {
            if (null != mLocAidlWWANDBProvider) {
                try {
                    mLocAidlWWANDBProvider.sendBSObsLocDataRequest();
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception on sendAPObsLocDataRequest: " + e);
                }
            } else {
                throw new RuntimeException("mLocAidlWWANDBProvider is null!");
            }
        }

        // ======================================================================
        // Callbacks
        // ======================================================================
        class LocAidlWWANDBProviderCallback extends ILocAidlWWANDBProviderCallback.Stub {

            private WWANDBProvider mWWANDBProvider;

            private LocAidlWWANDBProviderCallback(WWANDBProvider wiFiDBProvider) {
                mWWANDBProvider = wiFiDBProvider;
            }

            public void attachVmOnCallback() {
                // ???
            }

            public void serviceRequestCallback() {
                mWWANDBProvider.onServiceRequest();
            }

            public void bsObsLocDataUpdateCallback(LocAidlBSObsData[] bsInfoList,
                    int apObsLocDataListSize, byte bsListStatus) {

                Log.i(TAG, "TESTDEBUG bsObsLocDataUpdateCallback");

                ArrayList<BSObsLocationData> obsDataList = new ArrayList<BSObsLocationData>();

                for (LocAidlBSObsData bsObsData: bsInfoList) {
                    BSInfo cellInfo = new BSInfo();
                    cellInfo.mCellType =
                            IDLClientUtils.FDCLtoIZatCellTypes(bsObsData.cellInfo.cell_type);
                    cellInfo.mCellRegionID1 = bsObsData.cellInfo.cell_id1;
                    cellInfo.mCellRegionID2 = bsObsData.cellInfo.cell_id2;
                    cellInfo.mCellRegionID3 = bsObsData.cellInfo.cell_id3;
                    cellInfo.mCellRegionID4 = bsObsData.cellInfo.cell_id4;

                    BSObsLocationData obsData = new BSObsLocationData();
                    obsData.mScanTimestamp = (int) bsObsData.scanTimestamp_ms;
                    obsData.mLocation =
                            IDLClientUtils.translateAidlLocation(bsObsData.gpsLoc.gpsLocation);
                    if (bsObsData.gpsLoc.position_source == 2) {
                        //This is a GNSS position report
                        obsData.mLocation.setProvider(GPS_PROVIDER);
                    } else {
                        //Mark it as fused provider if it is not a GNSS position report
                        obsData.mLocation.setProvider(FUSED_PROVIDER);
                    }

                    obsData.mCellInfo = cellInfo;

                    obsDataList.add(obsData);
                }

                mWWANDBProvider.onBsObsLocDataAvailable(obsDataList, bsListStatus);
            }
            @Override
            public final int getInterfaceVersion() {
                return ILocAidlWWANDBProviderCallback.VERSION;
            }
            @Override
            public final String getInterfaceHash() {
                return ILocAidlWWANDBProviderCallback.HASH;
            }
        }
    }
}
