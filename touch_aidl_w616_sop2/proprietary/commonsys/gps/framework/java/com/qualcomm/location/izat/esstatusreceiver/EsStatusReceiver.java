/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.esstatusreceiver;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.os.IBinder;
import android.os.Binder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.Handler;
import android.util.Log;
import android.util.ArraySet;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyCallback.CallStateListener;
import android.telephony.TelephonyManager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.HashMap;
import java.util.Set;
import com.qualcomm.location.utils.IZatServiceContext;

import com.qualcomm.location.idlclient.*;
import com.qualcomm.location.idlclient.LocIDLClientBase.*;
import com.qualcomm.location.idlclient.IDLClientUtils;

import vendor.qti.gnss.V4_1.ILocHidlGnss;
import vendor.qti.gnss.V4_1.ILocHidlEsStatusReceiver;
import vendor.qti.gnss.V4_1.ILocHidlEsStatusReceiverCallback;
import vendor.qti.gnss.ILocAidlEsStatusReceiver;
import vendor.qti.gnss.ILocAidlEsStatusCallback;
import vendor.qti.gnss.ILocAidlGnss;

public class EsStatusReceiver {
    private static final String TAG = "EsStatusReceiver";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);
    private Set<IEsStatusListener> mListeners;
    private Object mListenersLock;
    private Handler mHandler;

    private static final Object mLock = new Object();
    private static EsStatusReceiver mEsStatusReceiver;
    private Context mContext;
    private EsStatusReceiverAidlClient mIdlClient;
    private EsStatusReceiverAFWClient mAFWClient;

    public static EsStatusReceiver getInstance(Context ctx) {
        synchronized (mLock) {
            if (null == mEsStatusReceiver) {
                mEsStatusReceiver = new EsStatusReceiver(ctx);
            }
        }
        return mEsStatusReceiver;
    }

    private EsStatusReceiver(Context ctx) {
        mListeners = new ArraySet<IEsStatusListener>();
        mListenersLock = new Object();
        mContext = ctx;
        mHandler = new Handler(IZatServiceContext.getInstance(ctx).getLooper());
        mHandler.post(()->init(ctx));
    }

    private void init(Context ctx) {
        IDLServiceVersion idlVer = LocIDLClientBase.getIDLServiceVersion();
        if (idlVer.compareTo(IDLServiceVersion.V_AIDL) >= 0) {
            Log.d(TAG, "LOCAIDL presented, initialize aidl client for es status listener");
            mIdlClient = new EsStatusReceiverAidlClient(this);
        } else if (idlVer.compareTo(IDLServiceVersion.V4_1) == 0) {
            Log.d(TAG, "LOCHIDL version is 4.1, initialize hidl client for es status listener");
            mIdlClient = new EsStatusReceiverHidlClient(this);
        } else {
            Log.d(TAG, "LOCHIDL version is lower than 4.1, or use LOCAIDL, initialize AFW client");
            mAFWClient = new EsStatusReceiverAFWClient(this, ctx);
        }
    }

    public void registerEsStatusListener(IEsStatusListener listener) {
        synchronized(mListenersLock) {
            mListeners.add(listener);
        }
    }

    public void deRegisterEsStatusListener(IEsStatusListener listener) {
        synchronized(mListenersLock) {
            mListeners.remove(listener);
        }
    }

    private void broadcastToListener(boolean isEmergencyMode) {
        synchronized(mListenersLock) {
            for (IEsStatusListener listener : mListeners) {
                listener.onStatusChanged(isEmergencyMode);
            }
        }
    }

    public interface IEsStatusListener {
        public void onStatusChanged(boolean isEmergencyMode);
    }

    // ======================================================================
    // AFW ES status client
    // ======================================================================
    static class EsStatusReceiverAFWClient {
        private final TelephonyManager mTelephonyManager;
        private final RilStateListener mRilStateListener;
        private boolean mIsInEmergencyCall = false;
        private EsStatusReceiver mEsStatusReceiver;
        private Context mContext;

        class RilStateListener extends TelephonyCallback implements CallStateListener {
            @Override
                public void onCallStateChanged(int state) {
                    // listening for emergency call ends
                    if (state == TelephonyManager.CALL_STATE_IDLE) {
                        Log.d(TAG, "onCallStateChanged(): state is "+ state);
                        if (mIsInEmergencyCall) {
                            mIsInEmergencyCall = false;
                            mEsStatusReceiver.broadcastToListener(mIsInEmergencyCall);
                        }
                    }
                }
        }
        public EsStatusReceiverAFWClient(EsStatusReceiver receiver, Context ctx) {
            mEsStatusReceiver = receiver;
            mContext = ctx;
            mTelephonyManager =
                    (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
            mRilStateListener = new RilStateListener();
            mTelephonyManager.registerTelephonyCallback(mContext.getMainExecutor(),
                    mRilStateListener);
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(Intent.ACTION_NEW_OUTGOING_CALL);
            mContext.registerReceiver(mBroadcastReciever, intentFilter);
        }

        private final BroadcastReceiver mBroadcastReciever = new BroadcastReceiver() {

            @Override public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action.equals(Intent.ACTION_NEW_OUTGOING_CALL)) {
                    String phoneNumber = intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER);
                    /*
                       Tracks the emergency call:
                           mIsInEmergencyCall records if the phone is in emergency call or not.
                           It will
                           be set to true when the phone is having emergency call, and then will
                           be set to false by mPhoneStateListener when the emergency call ends.
                    */

                    if (!mIsInEmergencyCall) {
                        mIsInEmergencyCall = mTelephonyManager.isEmergencyNumber(phoneNumber);
                        if (mIsInEmergencyCall) {
                            mEsStatusReceiver.broadcastToListener(mIsInEmergencyCall);
                        }
                        Log.d(TAG, "ACTION_NEW_OUTGOING_CALL - " + mIsInEmergencyCall);
                    }
                }
            }
        };

    }

    // ======================================================================
    // AIDL client
    // ======================================================================
    static class EsStatusReceiverAidlClient extends LocIDLClientBase
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "EsStatusReceiverAidlClient";
        private final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        private LocAidlEsStatusReceiverCallback mLocAidlEsStatusReceiverCallback;
        private ILocAidlEsStatusReceiver mLocAidlEsStatusReceiver;

        public EsStatusReceiverAidlClient(EsStatusReceiver receiver) {
            getEsStatusReceiverIface();
            mLocAidlEsStatusReceiverCallback = new LocAidlEsStatusReceiverCallback(receiver);
            if (mLocAidlEsStatusReceiver != null) {
                try {
                    mLocAidlEsStatusReceiver.setCallback(mLocAidlEsStatusReceiverCallback);
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on es status receiver init: " + e);
                }
            }
        }

        void getEsStatusReceiverIface() {
            Log.i(TAG, "getEsStatusReceiverIface");
            ILocAidlGnss gnssService = (vendor.qti.gnss.ILocAidlGnss) getGnssAidlService();

            if (null != gnssService) {
                try {
                    mLocAidlEsStatusReceiver = gnssService.getExtensionLocAidlEsStatusReceiver();
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception getting emergency status receiver: " + e);
                }
            } else {
                throw new RuntimeException("gnssService is null!");
            }
        }

        @Override
        public void onServiceDied() {
            mLocAidlEsStatusReceiver = null;
            getEsStatusReceiverIface();
            try {
                mLocAidlEsStatusReceiver.setCallback(mLocAidlEsStatusReceiverCallback);
            } catch (RemoteException e) {
                Log.e(TAG, "Exception on es status receiver init: " + e);
            }
        }
        // ======================================================================
        // Callbacks
        // ======================================================================

        class LocAidlEsStatusReceiverCallback extends ILocAidlEsStatusCallback.Stub {

            private EsStatusReceiver mEsStatusReceiver;

            private LocAidlEsStatusReceiverCallback(EsStatusReceiver esStatusReceiver) {
                mEsStatusReceiver = esStatusReceiver;
            }
            public void onEsStatusChanged(boolean isEmergencyMode) {
                mEsStatusReceiver.broadcastToListener(isEmergencyMode);
            }
            @Override
            public final int getInterfaceVersion() {
                return ILocAidlEsStatusCallback.VERSION;
            }
            @Override
            public final String getInterfaceHash() {
                return ILocAidlEsStatusCallback.HASH;
            }
        }

    }


    // ======================================================================
    // HIDL client
    // ======================================================================
    static class EsStatusReceiverHidlClient extends EsStatusReceiverAidlClient
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "EsStatusReceiverHidlClient";
        private final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        private LocHidlEsStatusReceiverCallback mLocHidlEsStatusReceiverCallback;
        private ILocHidlEsStatusReceiver mLocHidlEsStatusReceiver;

        public EsStatusReceiverHidlClient(EsStatusReceiver receiver) {
            super(receiver);
            getEsStatusReceiverIface();
            mLocHidlEsStatusReceiverCallback = new LocHidlEsStatusReceiverCallback(receiver);
            if (null != mLocHidlEsStatusReceiver) {
                try {
                    mLocHidlEsStatusReceiver.setCallback(mLocHidlEsStatusReceiverCallback);
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on es status receiver init: " + e);
                }
            }
        }

        void getEsStatusReceiverIface() {
            Log.i(TAG, "getEsStatusReceiverIface");
            ILocHidlGnss gnssService = (vendor.qti.gnss.V4_1.ILocHidlGnss) getGnssService();

            if (null != gnssService) {
                try {
                    mLocHidlEsStatusReceiver = gnssService.getExtensionLocHidlEsStatusReceiver();
                } catch (RemoteException e) {
                    throw new RuntimeException("Exception getting emergency status receiver: " + e);
                }
            } else {
                throw new RuntimeException("gnssService is null!");
            }
        }

        @Override
        public void onServiceDied() {
            mLocHidlEsStatusReceiver = null;
            getEsStatusReceiverIface();
            if (null != mLocHidlEsStatusReceiver) {
                try {
                    mLocHidlEsStatusReceiver.setCallback(mLocHidlEsStatusReceiverCallback);
                } catch (RemoteException e) {
                    Log.e(TAG, "Exception on es status receiver init: " + e);
                }
            }
        }
        // ======================================================================
        // Callbacks
        // ======================================================================

        class LocHidlEsStatusReceiverCallback extends ILocHidlEsStatusReceiverCallback.Stub {

            private EsStatusReceiver mEsStatusReceiver;

            private LocHidlEsStatusReceiverCallback(EsStatusReceiver esStatusReceiver) {
                mEsStatusReceiver = esStatusReceiver;
            }
            public void onEsStatusChanged(boolean isEmergencyMode) {
                mEsStatusReceiver.broadcastToListener(isEmergencyMode);
            }
        }
    }
}

