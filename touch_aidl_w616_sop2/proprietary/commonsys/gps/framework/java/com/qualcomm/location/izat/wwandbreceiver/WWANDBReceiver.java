/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.wwandbreceiver;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.os.IBinder;
import android.os.Binder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import com.qti.wwandbreceiver.*;
import com.qualcomm.location.idlclient.LocIDLClientBase;
import com.qualcomm.location.idlclient.IDLClientUtils;
import com.qualcomm.location.izat.GTPClientHelper;
import com.qualcomm.location.izat.IzatService;
import vendor.qti.gnss.V2_1.ILocHidlGnss;
import vendor.qti.gnss.V2_1.ILocHidlWWANDBReceiver;
import vendor.qti.gnss.V1_2.ILocHidlWWANDBReceiver.LocHidlBsLocationData;
import vendor.qti.gnss.V1_2.ILocHidlWWANDBReceiver.LocHidlBsSpecialInfo;
import vendor.qti.gnss.V2_1.ILocHidlWWANDBReceiverCallback;
import vendor.qti.gnss.V1_2.ILocHidlWWANDBReceiverCallback.LocHidlBsInfo;
import vendor.qti.gnss.V2_1.ILocHidlWWANDBReceiverCallback.LocHidlBsInfoExt;
import vendor.qti.gnss.V2_1.LocHidlUlpLocation;
import vendor.qti.gnss.V1_0.LocHidlLocationFlagsBits;

import com.qualcomm.location.idlclient.LocIDLClientBase.*;
import com.qualcomm.location.idlclient.*;
import vendor.qti.gnss.ILocAidlGnss;
import vendor.qti.gnss.ILocAidlWWANDBReceiver;
import vendor.qti.gnss.ILocAidlWWANDBReceiverCallback;
import vendor.qti.gnss.LocAidlBsInfo;
import vendor.qti.gnss.LocAidlBsSpecialInfo;
import vendor.qti.gnss.LocAidlBsLocationData;
import vendor.qti.gnss.LocAidlUlpLocation;


public class WWANDBReceiver implements IzatService.ISystemEventListener {
    private static final String TAG = "WWANDBReceiver";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sCallBacksLock = new Object();
    private final Context mContext;
    // In case and old SDK client < 7.0 is used
    private boolean mIsLegacySDKClient = false;

    private volatile IWWANDBReceiverResponseListener mWWANDBReceiverResponseListener = null;
    private PendingIntent mListenerIntent = null;
    private WWANDBReceiverIdlClient mIdlClient = null;

    private String mPackageName;

    private static WWANDBReceiver sInstance = null;
    public static WWANDBReceiver getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new WWANDBReceiver(ctx);
        }
        return sInstance;
    }

    private WWANDBReceiver(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "WWANDBReceiver construction");
        }
        mContext = ctx;
        if (LocIDLClientBase.getIDLServiceVersion().compareTo(IDLServiceVersion.V_AIDL) >= 0) {
            mIdlClient = new WWANDBReceiverIdlClient(this);
        } else {
            mIdlClient = new WWANDBReceiverHidlClient(this);
        }
        if (null == mIdlClient) {
            Log.e(TAG, "WWANDBReceiver construction fail: " + mIdlClient);
        }
        IzatService.AidlClientDeathNotifier.getInstance().registerAidlClientDeathCb(this);
    }

    @Override
    public void onAidlClientDied(String packageName, int pid, int uid) {
        if (mPackageName != null && mPackageName.equals(packageName)) {
            Log.d(TAG, "aidl client crash: " + packageName);
            synchronized (sCallBacksLock) {
                mWWANDBReceiverResponseListener = null;
            }
        }
    }

    /* Remote binder */
    private final IWWANDBReceiver.Stub mBinder = new IWWANDBReceiver.Stub() {

        @Deprecated
        // For backwards compatibility with possible static-linked SDK
        public boolean registerResponseListener(final IWWANDBReceiverResponseListener callback) {
            mIsLegacySDKClient = true;
            return registerResponseListenerExt(callback, null);
        }

        public boolean registerResponseListenerExt(final IWWANDBReceiverResponseListener callback,
                                                   PendingIntent notifyIntent) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return false;
            }

            if (notifyIntent == null) {
                Log.w(TAG, "notifyIntent is null");
            }

            synchronized (sCallBacksLock) {
                if (null != mWWANDBReceiverResponseListener) {
                    Log.e(TAG, "Response listener already provided.");
                    return false;
                }

                mWWANDBReceiverResponseListener = callback;
                mListenerIntent = notifyIntent;
            }

            mPackageName = mContext.getPackageManager().getNameForUid(Binder.getCallingUid());

            return true;
        }

        public void removeResponseListener(final IWWANDBReceiverResponseListener callback) {
            if (null == callback) {
                Log.e(TAG, "callback is null");
                return;
            }
            synchronized (sCallBacksLock) {
                mWWANDBReceiverResponseListener = null;
                mListenerIntent = null;
            }
            mPackageName = null;
        }


        public void requestBSList(int expireInDays) {
            if (VERBOSE) {
                Log.d(TAG, "in IWWANDBReceiver.Stub(): requestBSList()");
            }
            mIdlClient.requestBSList(expireInDays);
        }

        public void pushWWANDB(List<BSLocationData> locData,
                               List<BSSpecialInfo> splData,
                               int daysValid) {
            if (VERBOSE) {
                Log.d(TAG, "in IWWANDBReceiver.Stub(): pushWWANDB() ");
            }
            mIdlClient.pushWWANDB(locData, splData, daysValid);
        }
    };

    private void onBSListAvailable(ArrayList<BSInfo> bsList, int status, Location location) {
        if (VERBOSE) {
            Log.d(TAG, "onBSListAvailable");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBReceiverResponseListener) {
                try {
                    if (mIsLegacySDKClient) {
                        Log.d(TAG, "TESTDEBUG onBSListAvailable listener");
                        mWWANDBReceiverResponseListener.onBSListAvailable(bsList);
                    } else {
                        Log.d(TAG, "TESTDEBUG onBSListAvailableExt listener");
                        mWWANDBReceiverResponseListener.onBSListAvailableExt(
                                bsList, status, location);
                    }
                } catch (RemoteException e) {
                    Log.w(TAG, "onBSListAvailable remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBReceiver");
                }
            }
        }
    }

    private void onStatusUpdate(boolean isSuccess, String error) {
        if (VERBOSE) {
            Log.d(TAG, "onStatusUpdate");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBReceiverResponseListener) {
                try {
                    mWWANDBReceiverResponseListener.onStatusUpdate(isSuccess, error);
                    if (VERBOSE) {
                        Log.d(TAG, "onStatusUpdate: send update to listener");
                    }
                } catch (RemoteException e) {
                    Log.w(TAG, "onStatusUpdate remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBReceiver");
                }
            }
        }
    }

    private void onServiceRequest() {
        if (VERBOSE) {
            Log.d(TAG, "onServiceRequest");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBReceiverResponseListener) {
                try {
                    mWWANDBReceiverResponseListener.onServiceRequest();
                } catch (RemoteException e) {
                    Log.w(TAG, "onServiceRequest remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBReceiver");
                }
            }
        }
    }

    public IWWANDBReceiver getWWANDBReceiverBinder() {
        return mBinder;
    }

    // ======================================================================
    // HIDL client
    // ======================================================================
    static class WWANDBReceiverHidlClient extends WWANDBReceiverIdlClient
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "WWANDBReceiverHidlClient";
        private final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        private LocHidlWWANDBReceiverCallback mLocHidlWWANDBProvicerCallback;
        private ILocHidlWWANDBReceiver mLocHidlWWANDBReceiver;

        public WWANDBReceiverHidlClient(WWANDBReceiver Receiver) {
            super(Receiver);
            getWWANDBReceiverIface();
            mLocHidlWWANDBProvicerCallback = new LocHidlWWANDBReceiverCallback(Receiver);

            if (null != mLocHidlWWANDBReceiver) {
                try {
                    mLocHidlWWANDBReceiver.init(mLocHidlWWANDBProvicerCallback);
                    mLocHidlWWANDBReceiver.registerWWANDBUpdater(mLocHidlWWANDBProvicerCallback);
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on Receiver init: " + e);
                }
            }
        }

        private void getWWANDBReceiverIface() {
            Log.i(TAG, "getWWANDBReceiverIface");
            ILocHidlGnss gnssService = (vendor.qti.gnss.V2_1.ILocHidlGnss) getGnssService();
            IDLServiceVersion idlVer = getIDLServiceVersion();

            if (null != gnssService) {
                try {
                    if (idlVer.compareTo(IDLServiceVersion.V2_1) >= 0) {
                        Log.d(TAG, "LocHidlWiFiDBReceiverWrapper factory calling new 21");
                        mLocHidlWWANDBReceiver = ((vendor.qti.gnss.V2_1.ILocHidlGnss)gnssService)
                                .getExtensionLocHidlWWANDBReceiver_2_1();
                    } else {
                        Log.i(TAG, "LocHidlWiFiDBReceiverWrapper ftry calling new base 10");
                        mLocHidlWWANDBReceiver = (vendor.qti.gnss.V2_1.ILocHidlWWANDBReceiver)
                                gnssService.getExtensionLocHidlWWANDBReceiver();
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception getting wifidb receiver: " + e);
                }
            } else {
                throw new RuntimeException("gnssService is null!");
            }
        }

        @Override
        public void onServiceDied() {
            mLocHidlWWANDBReceiver = null;
            getWWANDBReceiverIface();

            if (null != mLocHidlWWANDBReceiver) {
                try {
                    mLocHidlWWANDBReceiver.init(mLocHidlWWANDBProvicerCallback);
                    mLocHidlWWANDBReceiver.registerWWANDBUpdater(mLocHidlWWANDBProvicerCallback);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on Receiver init: " + e);
                }
            }
        }

        @Override
        public void requestBSList(int expireInDays) {
            if (null != mLocHidlWWANDBReceiver) {
                try {
                    mLocHidlWWANDBReceiver.sendBSListRequest(expireInDays);
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception on sendAPObsLocDataRequest: " + e);
                }
            } else {
                throw new RuntimeException("mLocHidlWWANDBReceiver is null!");
            }
        }

        @Override
        public void pushWWANDB(List<BSLocationData> locData,
                               List<BSSpecialInfo> splData,
                               int daysValid) {
            ArrayList<LocHidlBsLocationData> bsLocationDataList =
                    new ArrayList<LocHidlBsLocationData>();
            for (BSLocationData loc: locData) {
                LocHidlBsLocationData bsLocationData = new LocHidlBsLocationData();
                bsLocationData.cellType = (byte) loc.mCellType;
                bsLocationData.cellRegionID1 = loc.mCellRegionID1;
                bsLocationData.cellRegionID2 = loc.mCellRegionID2;
                bsLocationData.cellRegionID3 = loc.mCellRegionID3;
                bsLocationData.cellRegionID4 = loc.mCellRegionID4;

                bsLocationData.latitude = loc.mLatitude;
                bsLocationData.longitude = loc.mLongitude;

                bsLocationData.valid_bits = (byte) loc.mValidBits;
                bsLocationData.horizontal_coverage_radius = loc.mHorizontalCoverageRadius;
                bsLocationData.horizontal_confidence = (byte) loc.mHorizontalConfidence;
                bsLocationData.horizontal_reliability = (byte) loc.mHorizontalReliability;
                bsLocationData.altitude = loc.mAltitude;
                bsLocationData.altitude_uncertainty = loc.mAltitudeUncertainty;
                bsLocationData.altitude_confidence = (byte) loc.mAltitudeConfidence;
                bsLocationData.altitude_reliability = (byte) loc.mAltitudeReliability;

                bsLocationDataList.add(bsLocationData);
            }

            ArrayList<LocHidlBsSpecialInfo> splList = new ArrayList<LocHidlBsSpecialInfo>();
            for (BSSpecialInfo sp: splData) {
                LocHidlBsSpecialInfo spl = new LocHidlBsSpecialInfo();
                spl.cellType = (byte) sp.mCellType;
                spl.cellRegionID1 = sp.mCellRegionID1;
                spl.cellRegionID2 = sp.mCellRegionID2;
                spl.cellRegionID3 = sp.mCellRegionID3;
                spl.cellRegionID4 = sp.mCellRegionID4;
                spl.info = (byte) sp.mInfo;

                splList.add(spl);
            }

            if (null != mLocHidlWWANDBReceiver) {
                try {
                    mLocHidlWWANDBReceiver.pushBSWWANDB(bsLocationDataList,
                            (byte) bsLocationDataList.size(), splList, (byte) splList.size(),
                            daysValid);
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception on sendAPObsLocDataRequest: " + e);
                }
            } else {
                throw new RuntimeException("mLocHidlWWANDBReceiver is null!");
            }
        }

        // ======================================================================
        // Callbacks
        // ======================================================================

        class LocHidlWWANDBReceiverCallback extends ILocHidlWWANDBReceiverCallback.Stub {

            private WWANDBReceiver mWWANDBReceiver;

            private LocHidlWWANDBReceiverCallback(WWANDBReceiver wiFiDBReceiver) {
                mWWANDBReceiver = wiFiDBReceiver;
            }

            public void attachVmOnCallback() {
                // ???
            }

            public void serviceRequestCallback() {
                mWWANDBReceiver.onServiceRequest();
            }

            public void bsListUpdateCallback(ArrayList<LocHidlBsInfo> bsInfoList,
                    int bsInfoListSize) {
                ArrayList<LocHidlBsInfoExt> bsHidlInfoList = new ArrayList<LocHidlBsInfoExt>();
                for (LocHidlBsInfo bsObsData: bsInfoList) {
                    LocHidlBsInfoExt bsObsDataExt = new LocHidlBsInfoExt();
                    bsObsDataExt.base_bsinfo_1_2 = bsObsData;
                    bsHidlInfoList.add(bsObsDataExt);
                }
                bsListUpdateCallback_2_1(bsHidlInfoList, bsHidlInfoList.size(), (byte) 0, null);
            }

            public void bsListUpdateCallback_2_1(ArrayList<LocHidlBsInfoExt> bsInfoList,
                    int apObsLocDataListSize, byte apListStatus, LocHidlUlpLocation ulpLocation) {

                Log.i(TAG, "TESTDEBUG bsListUpdateCallback_2_1");

                ArrayList<BSInfo> bsHidlInfoList = new ArrayList<BSInfo>();

                for (LocHidlBsInfoExt bsObsData: bsInfoList) {
                    BSInfo bsInfo = new BSInfo();
                    bsInfo.mCellType =
                            IDLClientUtils.FDCLtoIZatCellTypes(bsObsData.base_bsinfo_1_2.cell_type);
                    bsInfo.mCellRegionID1 = bsObsData.base_bsinfo_1_2.cell_id1;
                    bsInfo.mCellRegionID2 = bsObsData.base_bsinfo_1_2.cell_id2;
                    bsInfo.mCellRegionID3 = bsObsData.base_bsinfo_1_2.cell_id3;
                    bsInfo.mCellRegionID4 = bsObsData.base_bsinfo_1_2.cell_id4;
                    bsInfo.mCellLocalTimestamp = (int) bsObsData.timestamp;

                    bsHidlInfoList.add(bsInfo);
                }

                mWWANDBReceiver.onBSListAvailable(bsHidlInfoList, apListStatus,
                        IDLClientUtils.translateHidlLocation(ulpLocation.gpsLocation));
            }

            public void statusUpdateCallback(boolean status, String reason) {
                mWWANDBReceiver.onStatusUpdate(status, reason);
            }
        }
    }
    // ======================================================================
    // AIDL client
    // ======================================================================
    static class WWANDBReceiverIdlClient extends LocIDLClientBase
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "WWANDBReceiverIdlClient";
        private final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        private LocAidlWWANDBReceiverCallback mLocAidlWWANDBProvicerCallback;
        private ILocAidlWWANDBReceiver mLocAidlWWANDBReceiver;

        public WWANDBReceiverIdlClient(WWANDBReceiver Receiver) {
            getWWANDBReceiverIface();
            mLocAidlWWANDBProvicerCallback = new LocAidlWWANDBReceiverCallback(Receiver);

            if (null != mLocAidlWWANDBReceiver) {
                try {
                    mLocAidlWWANDBReceiver.init(mLocAidlWWANDBProvicerCallback);
                    mLocAidlWWANDBReceiver.registerWWANDBUpdater(mLocAidlWWANDBProvicerCallback);
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on Receiver init: " + e);
                }
            }
        }

        private void getWWANDBReceiverIface() {
            Log.i(TAG, "getWWANDBReceiverIface");
            ILocAidlGnss gnssService = (ILocAidlGnss) getGnssAidlService();

            if (null != gnssService) {
                try {
                        mLocAidlWWANDBReceiver = gnssService.getExtensionLocAidlWWANDBReceiver();
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception getting wifidb receiver: " + e);
                }
            }
        }

        @Override
        public void onServiceDied() {
            mLocAidlWWANDBReceiver = null;
            getWWANDBReceiverIface();

            if (null != mLocAidlWWANDBReceiver) {
                try {
                    mLocAidlWWANDBReceiver.init(mLocAidlWWANDBProvicerCallback);
                    mLocAidlWWANDBReceiver.registerWWANDBUpdater(mLocAidlWWANDBProvicerCallback);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on Receiver init: " + e);
                }
            }
        }

        public void requestBSList(int expireInDays) {
            if (null != mLocAidlWWANDBReceiver) {
                try {
                    mLocAidlWWANDBReceiver.sendBSListRequest(expireInDays);
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception on sendAPObsLocDataRequest: " + e);
                }
            } else {
                throw new RuntimeException("mLocAidlWWANDBReceiver is null!");
            }
        }

        public void pushWWANDB(List<BSLocationData> locData,
                               List<BSSpecialInfo> splData,
                               int daysValid) {
            int i = 0;
            LocAidlBsLocationData[] bsLocationDataList =
                    new LocAidlBsLocationData[locData.size()];
            for (BSLocationData loc: locData) {
                LocAidlBsLocationData bsLocationData = new LocAidlBsLocationData();
                bsLocationData.cellType = (byte) loc.mCellType;
                bsLocationData.cellRegionID1 = loc.mCellRegionID1;
                bsLocationData.cellRegionID2 = loc.mCellRegionID2;
                bsLocationData.cellRegionID3 = loc.mCellRegionID3;
                bsLocationData.cellRegionID4 = loc.mCellRegionID4;

                bsLocationData.latitude = loc.mLatitude;
                bsLocationData.longitude = loc.mLongitude;

                bsLocationData.valid_bits = (byte) loc.mValidBits;
                bsLocationData.horizontal_coverage_radius = (float) loc.mHorizontalCoverageRadius;
                bsLocationData.horizontal_confidence = (byte) loc.mHorizontalConfidence;
                bsLocationData.horizontal_reliability = (byte) loc.mHorizontalReliability;
                bsLocationData.altitude = loc.mAltitude;
                bsLocationData.altitude_uncertainty = loc.mAltitudeUncertainty;
                bsLocationData.altitude_confidence = (byte) loc.mAltitudeConfidence;
                bsLocationData.altitude_reliability = (byte) loc.mAltitudeReliability;

                bsLocationDataList[i] = bsLocationData;
                i++;
            }

            i = 0;
            LocAidlBsSpecialInfo[] splList = new LocAidlBsSpecialInfo[splData.size()];
            for (BSSpecialInfo sp: splData) {
                LocAidlBsSpecialInfo spl = new LocAidlBsSpecialInfo();
                spl.cellType = (byte) sp.mCellType;
                spl.cellRegionID1 = sp.mCellRegionID1;
                spl.cellRegionID2 = sp.mCellRegionID2;
                spl.cellRegionID3 = sp.mCellRegionID3;
                spl.cellRegionID4 = sp.mCellRegionID4;
                spl.info = (byte) sp.mInfo;

                splList[i] = spl;
                i++;
            }

            if (null != mLocAidlWWANDBReceiver) {
                try {
                    mLocAidlWWANDBReceiver.pushBSWWANDB(bsLocationDataList,
                            bsLocationDataList.length, splList, splList.length,
                            daysValid);
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception on sendAPObsLocDataRequest: " + e);
                }
            } else {
                throw new RuntimeException("mLocAidlWWANDBReceiver is null!");
            }
        }

        // ======================================================================
        // Callbacks
        // ======================================================================

        class LocAidlWWANDBReceiverCallback extends ILocAidlWWANDBReceiverCallback.Stub {

            private WWANDBReceiver mWWANDBReceiver;

            private LocAidlWWANDBReceiverCallback(WWANDBReceiver wiFiDBReceiver) {
                mWWANDBReceiver = wiFiDBReceiver;
            }

            public void attachVmOnCallback() {
                // ???
            }

            public void serviceRequestCallback() {
                mWWANDBReceiver.onServiceRequest();
            }

            public void bsListUpdateCallback(LocAidlBsInfo[] bsInfoList,
                    int apObsLocDataListSize, byte apListStatus, LocAidlUlpLocation ulpLocation) {

                Log.i(TAG, "TESTDEBUG bsListUpdateCallback_2_1");

                ArrayList<BSInfo> bsHidlInfoList = new ArrayList<BSInfo>();

                for (LocAidlBsInfo bsObsData: bsInfoList) {
                    BSInfo bsInfo = new BSInfo();
                    bsInfo.mCellType = IDLClientUtils.FDCLtoIZatCellTypes(bsObsData.cell_type);
                    bsInfo.mCellRegionID1 = bsObsData.cell_id1;
                    bsInfo.mCellRegionID2 = bsObsData.cell_id2;
                    bsInfo.mCellRegionID3 = bsObsData.cell_id3;
                    bsInfo.mCellRegionID4 = bsObsData.cell_id4;
                    bsInfo.mCellLocalTimestamp = (int) bsObsData.timestamp;

                    bsHidlInfoList.add(bsInfo);
                }

                mWWANDBReceiver.onBSListAvailable(bsHidlInfoList, apListStatus,
                        IDLClientUtils.translateAidlLocation(ulpLocation.gpsLocation));
            }

            public void statusUpdateCallback(boolean status, String reason) {
                mWWANDBReceiver.onStatusUpdate(status, reason);
            }
            @Override
            public final int getInterfaceVersion() {
                return ILocAidlWWANDBReceiverCallback.VERSION;
            }
            @Override
            public final String getInterfaceHash() {
                return ILocAidlWWANDBReceiverCallback.HASH;
            }
        }
    }
}
