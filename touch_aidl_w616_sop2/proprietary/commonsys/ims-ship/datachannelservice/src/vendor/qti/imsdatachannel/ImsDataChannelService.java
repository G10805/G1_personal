/**********************************************************************
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.content.Context;
import android.content.Intent;
import android.app.Service;
import android.os.IBinder;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.os.RemoteException;
import android.util.Log;
import android.util.SparseArray;

import java.util.List;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelServiceAvailabilityCallback;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelEventListener;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;
import vendor.qti.imsdatachannel.impls.ImsDataChannelTransportImpl;
import vendor.qti.imsdatachannel.ImsDataChannelServiceMgr;
import vendor.qti.imsdatachannel.impls.ImsDataChannelServiceImpl;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;


public class ImsDataChannelService extends Service {

    public static final String LOG_TAG = "ImsDataChannelService";
    public static final int DATA_CHANNEL_SERVICE_DISABLED = 0;
    public static final int DATA_CHANNEL_SERVICE_ENABLED = 1;
    private int mDataChannelServiceEnable = DATA_CHANNEL_SERVICE_DISABLED;

    private static final int INVALID_SLOT_ID = -1;
    private static final int UNINITIALIZED_VALUE = -1;
    public static int MAX_SLOTS = -1;
    private static ImsDataChannelServiceMgr mImsDcServiceMgr;
    private int mNumPhonesCache = UNINITIALIZED_VALUE;

//@TODO: uncomment later
    //private static final String DATA_CHANNEL_SERVICE_PROPERTY_STRING = "persist.vendor.ims.datachannel.feature";

    public static final String SERVICE_INTERFACE = "vendor.qti.imsdatachannel.service";

    Executor mServiceExecutor = new ScheduledThreadPoolExecutor(1);


    @Override
    public IBinder onBind(Intent intent) {
         //if(SERVICE_INTERFACE.equals(intent.getAction())) {
             Log.i(LOG_TAG, "ImsDatachannelService Bound.");
             if(mImsDcServiceMgr == null){
               Log.d(LOG_TAG, "ImsDataChannelService property is not enabled, mImsDcServiceMgr is null so return without initializing");
               return null;
             }

             return mImsDcServiceMgr.getImsDataChannelServiceBinder();
         //}
        // return null;
     }

    @Override
    public void onCreate() {
        Log.i(LOG_TAG, "ImsDatachannelService created.");
        setup();
     }

     public @NonNull Executor getExecutor() {
        Log.i (LOG_TAG, "getExecutor======");
        return mServiceExecutor;
     }

     private void setup() {
         Log.i(LOG_TAG, "ImsDatachannelService Setup.");
         final int numSlots = getNumSlots();
         MAX_SLOTS = numSlots;
         Log.i (LOG_TAG, "ImsDataChannelService "+ String.valueOf(MAX_SLOTS));

          // @TODO: uncomment below later
         /*mDataChannelServiceEnable = SystemProperties.getInt(DATA_CHANNEL_SERVICE_PROPERTY_STRING, DATA_CHANNEL_SERVICE_DISABLED);
         Log.i(LOG_TAG, "Data Channel service enable Property: " + mDataChannelServiceEnable);
         if(mDataChannelServiceEnable==DATA_CHANNEL_SERVICE_DISABLED) {
             Log.d(LOG_TAG, "ImsDataChannelService not enabled, so return without initializing");
             // @TODO: uncomment below later
            // return;
         } */
         mImsDcServiceMgr = ImsDataChannelServiceMgr.getInstance();
         Log.d(LOG_TAG, "mImsDcServiceMgr created: " + mImsDcServiceMgr.toString());
         mImsDcServiceMgr.createImsDataServiceImpl(numSlots);
         mImsDcServiceMgr.initialize(mServiceExecutor);

     }

     @Override
     public void onDestroy() {
     Log.i(LOG_TAG, "ImsDatachannelService onDestroy gets called().");
      mImsDcServiceMgr.cleanup();
     }

     private int getNumSlots() {
        if (mNumPhonesCache == UNINITIALIZED_VALUE) {
            mNumPhonesCache = ((TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE)).getSupportedModemCount();
        }
        Log.i (LOG_TAG, "ImsDatachannelService getNumSlots"+
               String.valueOf(mNumPhonesCache));
        return mNumPhonesCache;
    }
}
