/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.wrappers;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

import vendor.qti.imsdatachannel.impls.ImsDataChannelServiceImpl;
import vendor.qti.imsdatachannel.wrappers.ImsDataChannelTransportWrapper;
import vendor.qti.imsdatachannel.ImsDataChannelService;
import vendor.qti.ims.datachannelservice.ServiceState;
import vendor.qti.ims.datachannelservice.IDataChannelServiceListener.Stub;
import vendor.qti.ims.datachannelservice.IDataChannelServiceListener;
import vendor.qti.ims.datachannelservice.IDataChannelTransportListener;
import vendor.qti.ims.datachannelservice.IDataChannelTransport;
import vendor.qti.ims.datachannelservice.IDataChannelService;
import android.os.IBinder;
import java.util.*;
import android.os.ServiceManager;
import android.util.Log;
import android.os.RemoteException;
import java.util.concurrent.ConcurrentHashMap;

public class ImsDataChannelServiceWrapper {
    private static final String LOG_TAG = ImsDataChannelService.LOG_TAG + "_ImsDataChannelServiceWrapper";
    private vendor.qti.ims.datachannelservice.IDataChannelService mAidlDcService;
    private int nextId = 0;
    private Map<Integer,DataChannelServiceListener> mDcServiceListenerMap = new ConcurrentHashMap<>();

    public ImsDataChannelServiceWrapper(vendor.qti.ims.datachannelservice.IDataChannelService mAidlDcService) {
        this.mAidlDcService = mAidlDcService;
        Log.d(LOG_TAG, "ImsDataChannelServiceWrapper() created " + mAidlDcService.toString());
    }

    public DataChannelServiceListener getAidlDcServiceListener(int slotId){
        return mDcServiceListenerMap.get(slotId);
    }

    public void setAidlDcService(vendor.qti.ims.datachannelservice.IDataChannelService mAidlDcService) {
        this.mAidlDcService = mAidlDcService;
        Log.i(LOG_TAG, "setAidlDcService");
    }

    public vendor.qti.ims.datachannelservice.IDataChannelService getAidlDcService(){
      return mAidlDcService;
    }

    public void getDcServiceAvailability(int slotId, ImsDataChannelServiceListener mImsDcServiceCbListener) {
        try {
            Log.d(LOG_TAG, "getDcServiceAvailability mImsDcServiceCbListener addr:" +  mImsDcServiceCbListener.toString());
            DataChannelServiceListener mAidlDcServiceListener = mDcServiceListenerMap.get(slotId);
            if (mAidlDcServiceListener == null) {
                Log.i(LOG_TAG, "getDcServiceAvailability for slotID:" + slotId + "IDataChannelServiceListener is null ");
                mAidlDcServiceListener = new DataChannelServiceListener();
                Log.i(LOG_TAG, "getDcServiceAvailability for slotID:" + slotId + "IDataChannelServiceListener is : " + mAidlDcServiceListener.toString());
                mAidlDcServiceListener.setImsDcServiceCbListener(mImsDcServiceCbListener);
                mDcServiceListenerMap.put(slotId,mAidlDcServiceListener);
            } else {
                //update the new cblistener from client and return serviceavailability state if already cached!!
                Log.i(LOG_TAG, "getDcServiceAvailability for slotID:" + slotId + "IDataChannelServiceListener: " + mAidlDcServiceListener.toString());
                mAidlDcServiceListener.setImsDcServiceCbListener(mImsDcServiceCbListener);
                if (mAidlDcServiceListener.svcState != -1) {
                    mImsDcServiceCbListener.handleOnServiceStatus(mAidlDcServiceListener.svcState);
                }
                return;
            }

            //debug
            if (mAidlDcServiceListener.getImsDcServiceCbListener() == null) {
                Log.d(LOG_TAG, "mAidlDcServiceListener.getImsDcServiceCbListener()");
            }
            if (mAidlDcService == null)
                Log.d(LOG_TAG, "mAidlDcService is null for getDcServiceAvailability for slotID:" + slotId + "is null");
            else {
                Log.d(LOG_TAG, "mAidlDcService calling the aidl api" + mAidlDcService.toString());
                mAidlDcService.getDataChannelServiceAvailability(slotId + 1, mAidlDcServiceListener);
                Log.d(LOG_TAG, "mAidlDcService called the aidl api");
            }
        } catch (RemoteException e) {
            Log.i(LOG_TAG, "mAidlDcService exception");
            e.printStackTrace();
        }
   }

    public void createDataChannelTransport(int slotId, String callId, ImsDataChannelTransportWrapper imsDcTransportWrapperObj) {
        try {
            Log.i(LOG_TAG, "createDataChannelTransport for slotId: "+ slotId + " callID: " + callId);
            if (mAidlDcService == null) {
                Log.e(LOG_TAG, "mAidlDcService is null for createDataChannelTransport ");
                return;
            }
            mAidlDcService.createDataChannelTransport(slotId + 1, callId, imsDcTransportWrapperObj.getAidlDcTransportListener());
        } catch (RemoteException e) {
            e.printStackTrace();
            //Log.i(LOG_TAG, e);
        }
    }

   public void closeDataChannelTransport(String dcTransportHandle){
        try {
            Log.i(LOG_TAG, "closeDataChannelTransport" + dcTransportHandle);
            if(mAidlDcService == null){
                Log.e(LOG_TAG, "mAidlDcService is null for closeDataChannelTransport");
                return;
            }
            mAidlDcService.closeDataChannelTransport(dcTransportHandle);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
   }

    public void cleanUp(int slotId) {
        Log.i(LOG_TAG, "cleanUp called slotId:" + slotId);
        try {
            if (mAidlDcService == null) {
                Log.e(LOG_TAG, "mAidlDcService is null for cleanup");
            } else {
                mAidlDcService.cleanUp(slotId + 1);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        mDcServiceListenerMap.remove(slotId);
    }

    synchronized public void cleanUp() {
        if (mAidlDcService == null) {
            Log.e(LOG_TAG, "mAidlDcService is null for cleanup");
        } else {
            for (Map.Entry<Integer, DataChannelServiceListener> itr : mDcServiceListenerMap.entrySet()) {
                cleanUp(itr.getKey());
            }
        }
    }

    public void notifyServiceDied(){
        for (DataChannelServiceListener listnr : mDcServiceListenerMap.values()) {
            if (listnr.getImsDcServiceCbListener() != null){
                Log.d (LOG_TAG, "notifyServiceDied mImsDcServiceCbListener addr:" +  listnr.getImsDcServiceCbListener().toString());
                listnr.getImsDcServiceCbListener().handleOnServiceStatus(ServiceState.UNAVAILABLE);
            }
        }
    }

    public void getAllServiceAvailability() {
        // Executor exec = listnr.getImsDcServiceCbListener().getExecutor();
        Log.d(LOG_TAG, "getAllServiceAvailability");
        for (int slotId : mDcServiceListenerMap.keySet()) {
            DataChannelServiceListener listnr = mDcServiceListenerMap.get(slotId);
            try {
                mAidlDcService.getDataChannelServiceAvailability(slotId + 1, listnr);
            } catch(RemoteException e) {
                e.printStackTrace();
                Log.e(LOG_TAG, "getDataChannelServiceAvailability remote exception!!!");
            }
        }
    }

    private class DataChannelServiceListener extends IDataChannelServiceListener.Stub {
        private final static String LOG_TAG = ImsDataChannelServiceWrapper.LOG_TAG + "_IDataChannelServiceListener";
        private ImsDataChannelServiceListener mImsDcServiceCbListener = null;
        public int svcState = -1;
        public DataChannelServiceListener() {
        }

        public void setImsDcServiceCbListener(ImsDataChannelServiceListener imsDcServiceCbListener) {
            mImsDcServiceCbListener = imsDcServiceCbListener;
        }

        public ImsDataChannelServiceListener getImsDcServiceCbListener() {
            return mImsDcServiceCbListener;
        }

        @Override
        public void onServiceStatus(int svcState){
            this.svcState = svcState;
            if (mImsDcServiceCbListener != null) {
                Log.i(LOG_TAG, "onServiceStatus svcState: " + svcState);
                mImsDcServiceCbListener.handleOnServiceStatus(svcState);
            } else {
               Log.i(LOG_TAG, "onServiceStatus svcState: " + svcState + "mImsDcServiceCbListener is null and datachannelservicelistener is: " + this.toString());
            }
        }

        @Override
        public final int getInterfaceVersion() { return IDataChannelServiceListener.VERSION; }

        @Override
        public final String getInterfaceHash() { return IDataChannelServiceListener.HASH; }

    }


    public static abstract class ImsDataChannelServiceListener{
        protected Executor mExecutor;
        public ImsDataChannelServiceListener(Executor e) {
            mExecutor = e;
        }

        public Executor getExecutor(){
            return mExecutor;
        }

        public final void handleOnServiceStatus(int svcState) {
            mExecutor.execute(() -> {
                onGetServiceStatus(svcState);
            });
        }

        public void onGetServiceStatus(int svcState) {
            // To be overwritten
        }
    }







}
