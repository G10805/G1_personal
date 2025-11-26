/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.location.idlclient;

import android.util.Log;
import android.os.RemoteException;
import java.util.NoSuchElementException;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.HashSet;
import android.os.HwBinder;
import android.os.IBinder;
import vendor.qti.gnss.V4_3.ILocHidlGnss;
import vendor.qti.gnss.ILocAidlGnss;
import android.os.ServiceManager;

abstract public class LocIDLClientBase {
    public static final String LOCAIDL_SERVICE_NAME = "vendor.qti.gnss.ILocAidlGnss/default";
    protected static vendor.qti.gnss.V1_0.ILocHidlGnss mGnssService = null;
    protected static vendor.qti.gnss.ILocAidlGnss mGnssAidlService = null;
    protected static IBinder mGnssAidlBinder = null;
    protected static IDLServiceVersion mIDLServiceVer = IDLServiceVersion.V0_0;
    protected static Runnable mGetGnssService = null;
    protected static CountDownLatch mCountDownLatch;
    protected static final String TAG = "LocIDLClientBase";

    //Izat modules wait for gnss hidl service maximum 30s
    protected static final long WAIT_GETSERVICE_TIME_MS = 30000;
    protected static HashSet<IServiceDeathCb> mServiceDiedCbs = new HashSet<IServiceDeathCb>();
    private static LocAidlDeathRecipient mAidlDeathRecipient = null;
    private static LocHidlDeathRecipient mHidlDeathRecipient = null;

    public interface IServiceDeathCb {
        void onServiceDied();
    }

    public static enum IDLServiceVersion {
        V0_0,
        V1_0,
        V1_1,
        V1_2,
        V2_0,
        V2_1,
        V3_0,
        V4_0,
        V4_1,
        V4_2,
        V4_3,
        V_AIDL,
        V_AIDL_2_0
    }

    static {
        mCountDownLatch = new CountDownLatch(1);
        mGetGnssService = new Runnable() {
            @Override
            public void run() {
                do {
                    Log.d(TAG, "try to get LOCAIDL service");
                    try {
                        mGnssAidlBinder =
                            ServiceManager.waitForDeclaredService(LOCAIDL_SERVICE_NAME);
                    } catch (Exception e) {
                        Log.e(TAG, "failed to start LOCAIDL service via service manager");
                        mGnssAidlBinder = null;
                    }
                    if (null != mGnssAidlBinder) {
                        Log.d(TAG, "LOCAIDL service available");
                        mGnssAidlService = ILocAidlGnss.Stub.asInterface(mGnssAidlBinder);
                    } else {
                        Log.d(TAG, "LOCAIDL service not available");
                    }
                    if (mGnssAidlService != null) {
                        try {
                            mIDLServiceVer = IDLServiceVersion.V_AIDL;
                            if (mGnssAidlService.getInterfaceVersion() > 1) {
                                mIDLServiceVer = IDLServiceVersion.V_AIDL_2_0;
                            }
                        } catch(RemoteException e) {
                            Log.e(TAG, "RemoteException: " + e);
                        }
                        break;
                    }

                    Log.d(TAG, "try to get 4.3 service");
                    try {
                        mGnssService = vendor.qti.gnss.V4_3.ILocHidlGnss
                            .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V4_3;
                        break;
                    }

                    Log.d(TAG, "try to get 4.2 service");
                    try {
                        mGnssService = vendor.qti.gnss.V4_2.ILocHidlGnss
                            .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V4_2;
                        break;
                    }

                    Log.d(TAG, "try to get 4.1 service");
                    try {
                        mGnssService = vendor.qti.gnss.V4_1.ILocHidlGnss
                                .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V4_1;
                        break;
                    }

                    Log.d(TAG, "try to get 4.0 service");
                    try {
                        mGnssService = vendor.qti.gnss.V4_0.ILocHidlGnss
                                .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V4_0;
                        break;
                    }

                    Log.d(TAG, "try to get 3.0 service");
                    try {
                        mGnssService = vendor.qti.gnss.V3_0.ILocHidlGnss
                                .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V3_0;
                        break;
                    }

                    Log.d(TAG, "try to get 2.1 service");
                    try {
                        mGnssService = vendor.qti.gnss.V2_1.ILocHidlGnss
                                .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V2_1;
                        break;
                    }

                    Log.d(TAG, "try to get 1.2 service");
                    try {
                        mGnssService = vendor.qti.gnss.V2_0.ILocHidlGnss
                                .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V2_0;
                        break;
                    }

                    Log.d(TAG, "try to get 1.1 service");
                    try {
                        mGnssService = vendor.qti.gnss.V1_1.ILocHidlGnss
                                .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V1_1;
                        break;
                    }

                    Log.d(TAG, "try to get 1.0 service");
                    try {
                        mGnssService = vendor.qti.gnss.V1_0.ILocHidlGnss
                                .getService("gnss_vendor", true);
                    } catch (RemoteException e) {
                        mGnssService = null;
                        Log.d(TAG, "RemoteException: " + e);
                    } catch (NoSuchElementException e) {
                        mGnssService = null;
                        Log.d(TAG, "NoSuchElementException: " + e);
                    }
                    if (mGnssService != null) {
                        mIDLServiceVer = IDLServiceVersion.V1_0;
                        break;
                    }
                } while (false);

                try {
                    if (mGnssAidlBinder != null) {
                        mAidlDeathRecipient = new LocAidlDeathRecipient();
                        mGnssAidlBinder.linkToDeath(mAidlDeathRecipient, 0);
                    } else if (mGnssService != null) {
                        mHidlDeathRecipient = new LocHidlDeathRecipient();
                        mGnssService.linkToDeath(mHidlDeathRecipient, 0);
                    }

                } catch (RemoteException e) {
                    Log.d(TAG, "RemoteException: " + e);
                } catch (NoSuchElementException e) {
                    Log.d(TAG, "NoSuchElementException: " + e);
                }
                mCountDownLatch.countDown();
            }
        };
        new Thread(mGetGnssService).start();
    }

    public static IDLServiceVersion getIDLServiceVersion() {
        try {
            mCountDownLatch.await(WAIT_GETSERVICE_TIME_MS, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
        }
        return mIDLServiceVer;
    }

    protected static void binderDiedReset() {
            mGnssService = null;
            mGnssAidlService = null;
            mIDLServiceVer = IDLServiceVersion.V0_0;
            mCountDownLatch = new CountDownLatch(1);
            new Thread(mGetGnssService).start();
            for (IServiceDeathCb item : mServiceDiedCbs) {
                item.onServiceDied();
            }
    }

    protected void registerServiceDiedCb(IServiceDeathCb cb) {
            mServiceDiedCbs.add(cb);
    }

    protected void unregisterServiceDiedCb(IServiceDeathCb cb) {
            mServiceDiedCbs.remove(cb);
    }

    public static vendor.qti.gnss.V1_0.ILocHidlGnss getGnssService() {
        try {
            mCountDownLatch.await(WAIT_GETSERVICE_TIME_MS, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
        }
        return mGnssService;
    }

    public static vendor.qti.gnss.ILocAidlGnss getGnssAidlService() {
        try {
            mCountDownLatch.await(WAIT_GETSERVICE_TIME_MS, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
        }
        return mGnssAidlService;
    }

    static final class LocAidlDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            Log.d(TAG, "LocAidlDeathRecipient.binderDied()");
            binderDiedReset();
        }
    }

    static final class LocHidlDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            Log.d(TAG, "LocHidlDeathRecipient.binderDied()");
            binderDiedReset();
        }
    }
}
